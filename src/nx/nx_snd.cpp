// nx_snd.cpp -- sound driver for the Switch build, replacing
// snd_driver_xaudio2.cpp / snd_driver_xaudio2_dsp.cpp (excluded).
//
// The engine's own sound code (snd.cpp and friends) does everything up to the
// voice: picking aliases, 3D panning into a per-channel speaker map, distance
// and occlusion levels, pitch, start delays, and the stream reader
// (snd_stream.cpp) that pages streamed sounds out of the .iwd archives. What
// XAudio2 did below that -- decode, resample, mix, play -- is done here: a
// software mixer on its own thread feeding libnx audout (48 kHz, stereo,
// 16-bit), with the engine talking to it through the same SD_* entry points,
// mirroring snd_driver_xaudio2.cpp call for call.
//
// Formats: PCM 16-bit, MS-ADPCM (the streamed voice lines) and xWMA (most
// loaded sounds), the last through FFmpeg's WMA v2 decoder. Not reproduced
// yet: the reverb bus, the per-voice low-pass / futz DSP and the master EQ /
// limiter.
//
// IMPORTANT: SD_Xaudio2CanInit MUST return true and SD_Init MUST succeed --
// Sys_StreamSleep blocks the shared stream thread on sndInitializedEvent,
// which only gets set when the sound system initializes; killing sound init
// would also kill texture streaming. SD_Init succeeds even when audout does
// not start (the mixer then never runs and the game is silent).
#include <sound/snd_driver_xaudio2.h>
#include <sound/snd.h>
#include <sound/snd_db.h>
#include <sound/snd_dvar.h>
#include <sound/snd_stream.h>
#include <sound/snd_utils.h>
#include <sound/snd_public_async.h>
#include <universal/dvar.h>
#include <qcommon/common.h>

#include <malloc.h>
#include <string.h>

#include <switch.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mem.h>
}

// The WMA v2 decoder by name: avcodec_find_decoder would pull in the whole
// codec list, and every codec with it. FFCodec starts with its AVCodec.
extern "C" const AVCodec ff_wmav2_decoder;

// Set by SND_StopVoice (snd.cpp) to its return address; see s_loopStops.
void *g_nxSndStopCaller;

namespace {

enum {
    NX_SND_RATE = 48000,
    NX_MIX_FRAMES = 1024,                        // ~21 ms per buffer
    NX_OUT_BUFFERS = 4,                          // ~85 ms of output queued
    NX_OUT_BYTES = NX_MIX_FRAMES * 2 * 2,        // stereo int16
    NX_ADPCM_BLOCK_BYTES = 262,                  // per channel; the PC driver's nBlockAlign
    NX_ADPCM_BLOCK_FRAMES = 2 * NX_ADPCM_BLOCK_BYTES - 12,   // 512
    NX_STAGE_FRAMES = 4096,                      // an ADPCM block (512) or a WMA frame (2048)
    NX_WMA_BLOCK_STEREO = 4096,                  // the PC driver's nBlockAlign for xWMA
    NX_WMA_BLOCK_MONO = 2230,
};

struct NxVoice {
    bool active;        // created: g_sd.voices[i] was non-null
    bool running;       // XAudio2 Start / Stop
    bool finished;      // an in-memory sound has played to its end
    bool stream;
    bool looping;
    snd_asset_format format;
    unsigned channels;
    unsigned rate;
    unsigned totalFrames;     // frame_count of the asset
    unsigned framesOut;       // source frames handed to the resampler

    // In-memory source.
    const uint8_t *data;
    unsigned dataSize;
    unsigned nextUnit;        // next ADPCM block, PCM frame or WMA packet
    unsigned totalUnits;      // WMA: packets (the seek table count)

    // Stream source: up to two windows, played in order. A window the mixer
    // has finished is flagged here and released on the main thread.
    struct Window { const uint8_t *data; unsigned size; unsigned offset; int index; } queue[2];
    int queueHead, queueCount;
    char *windows[2];          // what Snd_StreamAcquireWindow handed out, by buffer index
    bool windowDone[2];
    unsigned submitted;        // StreamVoice::bufferQueuedCount

    // Decoded frames not yet consumed.
    int16_t stage[NX_STAGE_FRAMES * 2];
    unsigned stageFrames, stagePos;

    // Resampler.
    float cur[2], next[2];
    float frac;
    bool primed;

    // Set by SD_UpdateVoice.
    float gain[2][2];          // [input channel][L, R]
    float pitch;
    uint64_t samplesPlayed;
};

NxVoice s_voice[SND_MAX_VOICES];
RMutex s_lock;

// A WMA decoder per voice slot, kept across sounds (flushed when the next one
// has the same rate and channels) and apart from NxVoice, which is cleared on
// every start. xWMA is a run of fixed-size packets (the asset's seek table
// counts them) of WMA v2 with no extradata; FFmpeg's own xWMA demuxer fills
// in the same six bytes. Packets are copied to a padded buffer because the
// decoder reads up to AV_INPUT_BUFFER_PADDING_SIZE past the end.
struct NxWma {
    AVCodecContext *ctx;
    AVFrame *frame;
    AVPacket *packet;
    unsigned rate, channels;
    bool draining;
    unsigned frameLeft;   // samples of d.frame not yet handed to the stage
    alignas(16) uint8_t scratch[NX_WMA_BLOCK_STEREO + AV_INPUT_BUFFER_PADDING_SIZE];
};
NxWma s_wma[SND_MAX_VOICES];

// Which aliases start most, for the periodic report.
struct NxStartTally { const snd_alias_t *alias; unsigned count; bool stream; };
NxStartTally s_tally[64];
NxStartTally s_cutTally[64];
unsigned s_nEnded, s_nCut;
unsigned s_nWmaErrors;   // decode errors (the sound goes on with the next packet)

// Who stopped looping voices: SND_StopVoice records its caller
// (g_nxSndStopCaller, snd.cpp). A loop the engine restarts every frame shows
// which engine code keeps stopping it.
struct NxCallerTally { uintptr_t addr; unsigned count; const snd_alias_t *alias; };
NxCallerTally s_loopStops[16];
unsigned s_nLoopStops;

Thread s_thread;
bool s_threadRunning;
volatile bool s_quit;
AudioOutBuffer s_outBuf[NX_OUT_BUFFERS];
int16_t *s_outData[NX_OUT_BUFFERS];
float s_mix[NX_MIX_FRAMES * 2];

// For the periodic report.
unsigned s_nStartPcm, s_nStartAdpcm, s_nStartWma, s_nRefusedWma, s_nRefusedOther, s_nStarved;
unsigned s_peakRunning;
u64 s_mixTicks, s_reportTick;
char s_lastRefused[64];

struct Lock {
    Lock() { rmutexLock(&s_lock); }
    ~Lock() { rmutexUnlock(&s_lock); }
};

// ---------------------------------------------------------------------------
// MS-ADPCM
// ---------------------------------------------------------------------------
const int kAdaptCoeff1[7] = { 256, 512, 0, 192, 240, 460, 392 };
const int kAdaptCoeff2[7] = { 0, -256, 0, 64, 0, -208, -232 };
const int kAdaptation[16] = { 230, 230, 230, 230, 307, 409, 512, 614,
                              768, 614, 512, 409, 307, 230, 230, 230 };

inline int16_t clamp16(int v)
{
    return (int16_t)(v < -32768 ? -32768 : v > 32767 ? 32767 : v);
}

// One block of `channels` interleaved channels into `out` (interleaved).
// Returns the frames decoded (up to 512).
unsigned decodeAdpcmBlock(const uint8_t *in, unsigned channels, int16_t *out)
{
    int pred[2], delta[2], s1[2], s2[2];
    const uint8_t *p = in;
    for (unsigned c = 0; c < channels; ++c)
        pred[c] = p[c] < 7 ? p[c] : 0;
    p += channels;
    for (unsigned c = 0; c < channels; ++c, p += 2)
        delta[c] = (int16_t)(p[0] | (p[1] << 8));
    for (unsigned c = 0; c < channels; ++c, p += 2)
        s1[c] = (int16_t)(p[0] | (p[1] << 8));
    for (unsigned c = 0; c < channels; ++c, p += 2)
        s2[c] = (int16_t)(p[0] | (p[1] << 8));

    // The two header samples come out oldest first.
    for (unsigned c = 0; c < channels; ++c) {
        out[c] = (int16_t)s2[c];
        out[channels + c] = (int16_t)s1[c];
    }
    unsigned frames = 2;
    unsigned nibbles = (NX_ADPCM_BLOCK_BYTES - 7) * 2 * channels;
    unsigned c = 0;
    int16_t *o = out + 2 * channels;
    for (unsigned n = 0; n < nibbles; ++n) {
        int byte = p[n >> 1];
        int nib = (n & 1) ? (byte & 0x0F) : (byte >> 4);
        int signedNib = nib >= 8 ? nib - 16 : nib;
        int predicted = (s1[c] * kAdaptCoeff1[pred[c]] + s2[c] * kAdaptCoeff2[pred[c]]) >> 8;
        int sample = clamp16(predicted + signedNib * delta[c]);
        s2[c] = s1[c];
        s1[c] = sample;
        delta[c] = (delta[c] * kAdaptation[nib]) >> 8;
        if (delta[c] < 16)
            delta[c] = 16;
        *o++ = (int16_t)sample;
        if (++c == channels) {
            c = 0;
            ++frames;
        }
    }
    return frames;
}

unsigned blockBytes(const NxVoice &v)
{
    return NX_ADPCM_BLOCK_BYTES * v.channels;
}

// ---------------------------------------------------------------------------
// Sources: refill the stage with the next decoded frames. False when nothing
// is available (the end of an in-memory sound, or a stream starving).
// ---------------------------------------------------------------------------
// WMA: the next decoded frame into the stage, feeding packets as the decoder
// asks. A looping sound restarts at packet 0 with the decoder flushed; a
// one-shot drains the decoder and ends.
bool refillWma(NxVoice &v)
{
    NxWma &d = s_wma[&v - s_voice];
    if (!d.ctx)
        return false;
    const unsigned blockAlign = v.channels > 1 ? NX_WMA_BLOCK_STEREO : NX_WMA_BLOCK_MONO;
    const unsigned packets = v.dataSize / blockAlign < v.totalUnits ? v.dataSize / blockAlign : v.totalUnits;
    for (int guard = 0; guard < 64; ++guard) {
        // A decoded frame is a whole superframe -- with the bit reservoir about
        // one packet, ~10000 samples for the zombie vocals -- so it is handed
        // out a stage at a time. Keeping only the first 4096 samples of each
        // skipped the rest of the packet: sounds ran fast and short.
        if (d.frameLeft) {
            unsigned n = d.frameLeft < NX_STAGE_FRAMES ? d.frameLeft : NX_STAGE_FRAMES;
            const unsigned at = (unsigned)d.frame->nb_samples - d.frameLeft;
            const bool planar = d.frame->format == AV_SAMPLE_FMT_FLTP;
            for (unsigned c = 0; c < v.channels; ++c) {
                const float *src = (const float *)d.frame->extended_data[planar ? c : 0];
                for (unsigned i = 0; i < n; ++i) {
                    float s = planar ? src[at + i] : src[(at + i) * v.channels + c];
                    v.stage[i * v.channels + c] = clamp16((int)(s * 32767.0f));
                }
            }
            d.frameLeft -= n;
            if (!d.frameLeft)
                av_frame_unref(d.frame);
            v.stageFrames = n;
            v.stagePos = 0;
            return true;
        }
        int r = avcodec_receive_frame(d.ctx, d.frame);
        if (r == 0) {
            d.frameLeft = d.frame->nb_samples > 0 ? (unsigned)d.frame->nb_samples : 0;
            if (!d.frameLeft)
                av_frame_unref(d.frame);
            continue;
        }
        if (r == AVERROR_EOF)
            return false;   // drained
        if (r != AVERROR(EAGAIN))
            ++s_nWmaErrors;   // a packet it could not decode: go on with the next one
        if (v.nextUnit >= packets) {
            if (v.looping && packets) {
                avcodec_flush_buffers(d.ctx);
                d.draining = false;
                v.nextUnit = 0;
                continue;
            }
            if (d.draining)
                return false;
            d.draining = true;
            avcodec_send_packet(d.ctx, nullptr);
            continue;
        }
        memcpy(d.scratch, v.data + v.nextUnit * blockAlign, blockAlign);
        memset(d.scratch + blockAlign, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        ++v.nextUnit;
        d.packet->data = d.scratch;
        d.packet->size = (int)blockAlign;
        avcodec_send_packet(d.ctx, d.packet);   // a packet it refuses is skipped
    }
    return false;
}

bool refillRam(NxVoice &v)
{
    if (v.format == SND_ASSET_FORMAT_WMA) {
        // The decoder pads the tail: stop at the asset's frame count.
        if (!v.looping && v.totalFrames && v.framesOut >= v.totalFrames)
            return false;
        return refillWma(v);
    }
    for (int attempt = 0; attempt < 2; ++attempt) {
        if (v.format == SND_ASSET_FORMAT_MSADPCM) {
            unsigned bb = blockBytes(v);
            unsigned first = v.nextUnit * NX_ADPCM_BLOCK_FRAMES;
            if (first < v.totalFrames && (v.nextUnit + 1) * bb <= v.dataSize) {
                unsigned frames = decodeAdpcmBlock(v.data + v.nextUnit * bb, v.channels, v.stage);
                if (frames > v.totalFrames - first)
                    frames = v.totalFrames - first;
                ++v.nextUnit;
                v.stageFrames = frames;
                v.stagePos = 0;
                return frames != 0;
            }
        } else {
            unsigned frameBytes = 2 * v.channels;
            unsigned avail = v.dataSize / frameBytes;
            if (avail > v.totalFrames)
                avail = v.totalFrames;
            if (v.nextUnit < avail) {
                unsigned frames = avail - v.nextUnit;
                if (frames > NX_STAGE_FRAMES)
                    frames = NX_STAGE_FRAMES;
                memcpy(v.stage, v.data + v.nextUnit * frameBytes, frames * frameBytes);
                v.nextUnit += frames;
                v.stageFrames = frames;
                v.stagePos = 0;
                return true;
            }
        }
        if (!v.looping)
            return false;
        v.nextUnit = 0;   // wrap once, then give up if still empty
    }
    return false;
}

void finishWindow(NxVoice &v)
{
    NxVoice::Window &w = v.queue[v.queueHead];
    v.windowDone[w.index] = true;
    v.queueHead = (v.queueHead + 1) % 2;
    --v.queueCount;
}

bool refillStream(NxVoice &v)
{
    if (!v.looping && v.totalFrames && v.framesOut >= v.totalFrames) {
        while (v.queueCount)
            finishWindow(v);
        return false;
    }
    while (v.queueCount) {
        NxVoice::Window &w = v.queue[v.queueHead];
        unsigned left = w.size - w.offset;
        if (v.format == SND_ASSET_FORMAT_MSADPCM) {
            unsigned bb = blockBytes(v);
            if (left >= bb) {
                v.stageFrames = decodeAdpcmBlock(w.data + w.offset, v.channels, v.stage);
                w.offset += bb;
                v.stagePos = 0;
                return true;
            }
        } else {
            unsigned frameBytes = 2 * v.channels;
            unsigned frames = left / frameBytes;
            if (frames) {
                if (frames > NX_STAGE_FRAMES)
                    frames = NX_STAGE_FRAMES;
                memcpy(v.stage, w.data + w.offset, frames * frameBytes);
                w.offset += frames * frameBytes;
                v.stageFrames = frames;
                v.stagePos = 0;
                return true;
            }
        }
        finishWindow(v);
    }
    return false;
}

bool readFrame(NxVoice &v, float out[2])
{
    if (v.stagePos >= v.stageFrames) {
        bool ok = v.stream ? refillStream(v) : refillRam(v);
        if (!ok || !v.stageFrames)
            return false;
    }
    const int16_t *f = v.stage + v.stagePos * v.channels;
    out[0] = f[0] * (1.0f / 32768.0f);
    out[1] = v.channels > 1 ? f[1] * (1.0f / 32768.0f) : out[0];
    ++v.stagePos;
    ++v.framesOut;
    return true;
}

// Mixes one running voice into s_mix.
void mixVoice(NxVoice &v)
{
    const float step = (float)v.rate * v.pitch / (float)NX_SND_RATE;
    if (!v.primed) {
        if (!readFrame(v, v.cur)) {
            if (!v.stream)
                v.finished = true;
            else
                ++s_nStarved;
            return;
        }
        if (!readFrame(v, v.next)) {
            v.next[0] = v.cur[0];
            v.next[1] = v.cur[1];
        }
        v.frac = 0.0f;
        v.primed = true;
    }
    const float g00 = v.gain[0][0], g01 = v.gain[0][1];
    const float g10 = v.gain[1][0], g11 = v.gain[1][1];
    const bool stereo = v.channels > 1;
    unsigned i = 0;
    for (; i < NX_MIX_FRAMES; ++i) {
        float a = v.cur[0] + (v.next[0] - v.cur[0]) * v.frac;
        if (stereo) {
            float b = v.cur[1] + (v.next[1] - v.cur[1]) * v.frac;
            s_mix[2 * i]     += a * g00 + b * g10;
            s_mix[2 * i + 1] += a * g01 + b * g11;
        } else {
            s_mix[2 * i]     += a * g00;
            s_mix[2 * i + 1] += a * g01;
        }
        v.frac += step;
        bool dry = false;
        while (v.frac >= 1.0f) {
            v.frac -= 1.0f;
            v.cur[0] = v.next[0];
            v.cur[1] = v.next[1];
            if (!readFrame(v, v.next)) {
                dry = true;
                break;
            }
        }
        if (dry) {
            ++i;
            // An in-memory sound has ended; a stream waits for its next window.
            if (!v.stream)
                v.finished = true;
            else
                ++s_nStarved;
            v.primed = false;
            break;
        }
    }
    v.samplesPlayed += i;
}

void mixInto(int16_t *out)
{
    u64 t0 = armGetSystemTick();
    memset(s_mix, 0, sizeof(s_mix));
    unsigned running = 0;
    {
        Lock lock;
        for (NxVoice &v : s_voice) {
            if (!v.active || !v.running || v.finished)
                continue;
            ++running;
            mixVoice(v);
        }
    }
    if (running > s_peakRunning)
        s_peakRunning = running;
    for (unsigned i = 0; i < NX_MIX_FRAMES * 2; ++i)
        out[i] = clamp16((int)(s_mix[i] * 32767.0f));
    s_mixTicks += armGetSystemTick() - t0;
}

void mixerThread(void *)
{
    while (!s_quit) {
        AudioOutBuffer *released = nullptr;
        u32 count = 0;
        if (R_FAILED(audoutWaitPlayFinish(&released, &count, 100000000ull)) || !released)
            continue;
        mixInto((int16_t *)released->buffer);
        released->data_size = NX_OUT_BYTES;
        audoutAppendAudioOutBuffer(released);
    }
}

bool startOutput()
{
    if (R_FAILED(audoutInitialize()))
        return false;
    if (R_FAILED(audoutStartAudioOut())) {
        audoutExit();
        return false;
    }
    for (int i = 0; i < NX_OUT_BUFFERS; ++i) {
        s_outData[i] = (int16_t *)memalign(0x1000, NX_OUT_BYTES);
        memset(s_outData[i], 0, NX_OUT_BYTES);
        s_outBuf[i].next = nullptr;
        s_outBuf[i].buffer = s_outData[i];
        s_outBuf[i].buffer_size = NX_OUT_BYTES;
        s_outBuf[i].data_size = NX_OUT_BYTES;
        s_outBuf[i].data_offset = 0;
        audoutAppendAudioOutBuffer(&s_outBuf[i]);
    }
    s_quit = false;
    // Above the main thread (0x2C) so a long frame does not starve the output.
    if (R_FAILED(threadCreate(&s_thread, mixerThread, nullptr, nullptr, 0x10000, 0x2B, -2))
        || R_FAILED(threadStart(&s_thread))) {
        audoutStopAudioOut();
        audoutExit();
        return false;
    }
    s_threadRunning = true;
    return true;
}

// ---------------------------------------------------------------------------
// The engine side, after snd_driver_xaudio2.cpp
// ---------------------------------------------------------------------------
void noteRefused(const snd_alias_t *alias, snd_asset_format format)
{
    if (format == SND_ASSET_FORMAT_WMA)
        ++s_nRefusedWma;
    else
        ++s_nRefusedOther;
    if (alias && alias->name)
        I_strncpyz(s_lastRefused, alias->name, sizeof(s_lastRefused));
}

// iSND_CreateVoice: the voice exists and, for an in-memory sound, holds its
// data. Returns false for a format this driver cannot play.
// The slot's WMA decoder, ready for this sound: reused and flushed when the
// rate and channels match, else made anew. Called with the lock held.
bool openWma(int voiceIndex, const snd_asset *snd)
{
    NxWma &d = s_wma[voiceIndex];
    if (d.ctx && (d.rate != snd->frame_rate || d.channels != snd->channel_count))
        avcodec_free_context(&d.ctx);
    if (!d.ctx) {
        AVCodecContext *ctx = avcodec_alloc_context3(&ff_wmav2_decoder);
        if (!ctx)
            return false;
        ctx->sample_rate = (int)snd->frame_rate;
        av_channel_layout_default(&ctx->ch_layout, (int)snd->channel_count);
        ctx->bit_rate = 6000 * 8 * (int64_t)snd->channel_count;   // nAvgBytesPerSec in the PC driver
        ctx->block_align = snd->channel_count > 1 ? NX_WMA_BLOCK_STEREO : NX_WMA_BLOCK_MONO;
        ctx->thread_count = 1;
        ctx->extradata = (uint8_t *)av_mallocz(6 + AV_INPUT_BUFFER_PADDING_SIZE);
        if (ctx->extradata) {
            ctx->extradata_size = 6;
            ctx->extradata[4] = 31;   // as FFmpeg's xwma demuxer: exp VLC, bit reservoir, variable blocks
        }
        if (avcodec_open2(ctx, &ff_wmav2_decoder, nullptr) < 0) {
            avcodec_free_context(&ctx);
            return false;
        }
        d.ctx = ctx;
        d.rate = snd->frame_rate;
        d.channels = snd->channel_count;
    } else {
        avcodec_flush_buffers(d.ctx);
    }
    if (!d.frame)
        d.frame = av_frame_alloc();
    if (!d.packet)
        d.packet = av_packet_alloc();
    d.draining = false;
    if (d.frameLeft && d.frame)
        av_frame_unref(d.frame);   // what the slot's previous sound left undelivered
    d.frameLeft = 0;
    return d.frame && d.packet;
}

void noteTally(NxStartTally (&table)[64], const snd_alias_t *alias, bool stream)
{
    if (!alias)
        return;
    NxStartTally *freeSlot = nullptr;
    for (NxStartTally &t : table) {
        if (t.alias == alias) {
            ++t.count;
            return;
        }
        if (!t.alias && !freeSlot)
            freeSlot = &t;
    }
    if (freeSlot) {
        freeSlot->alias = alias;
        freeSlot->count = 1;
        freeSlot->stream = stream;
    }
}

// alias: the one being started. For an in-memory sound g_snd.voice[i].alias is
// not set yet here (SND_SetVoiceStartInfo comes after).
bool createVoice(int voiceIndex, const snd_alias_t *alias, const snd_asset *snd, bool isLooping)
{
    const bool stream = SND_IsStream(voiceIndex);
    const bool wma = snd->format == SND_ASSET_FORMAT_WMA && !stream && snd->seek_table_count;
    bool ok = (snd->format == SND_ASSET_FORMAT_PCMS16 || snd->format == SND_ASSET_FORMAT_MSADPCM || wma)
           && (snd->channel_count == 1 || snd->channel_count == 2) && snd->frame_rate;
    Lock lock;
    if (ok && wma && !openWma(voiceIndex, snd))
        ok = false;
    if (!ok) {
        noteRefused(alias, snd->format);
        return false;
    }
    noteTally(s_tally, alias, stream);
    NxVoice &v = s_voice[voiceIndex];
    memset(&v, 0, sizeof(v));
    v.totalUnits = wma ? snd->seek_table_count : 0;
    v.active = true;
    v.stream = SND_IsStream(voiceIndex);
    v.looping = isLooping;
    v.format = snd->format;
    v.channels = snd->channel_count;
    v.rate = snd->frame_rate;
    v.totalFrames = snd->frame_count;
    v.pitch = 1.0f;
    if (!v.stream) {
        v.data = (const uint8_t *)snd->data;
        v.dataSize = snd->data_size;
    }
    if (snd->format == SND_ASSET_FORMAT_MSADPCM)
        ++s_nStartAdpcm;
    else if (wma)
        ++s_nStartWma;
    else
        ++s_nStartPcm;
    return true;
}

unsigned buffersQueued(const NxVoice &v)
{
    if (v.stream)
        return (unsigned)v.queueCount;
    return v.finished ? 0 : 1;
}

void releaseDoneWindows(int voiceIndex)
{
    NxVoice &v = s_voice[voiceIndex];
    for (int i = 0; i < 2; ++i) {
        if (v.windowDone[i] && v.windows[i]) {
            char *w = v.windows[i];
            v.windows[i] = nullptr;
            v.windowDone[i] = false;
            Snd_StreamReleaseWindow(voiceIndex, w);
        }
    }
}

void channelError(int voiceIndex)
{
    const snd_alias_t *alias = g_snd.voice[voiceIndex].alias;
    SND_LengthNotify(voiceIndex, 0);
    SND_StopVoice(voiceIndex);
    if (alias && alias->soundFile)
        alias->soundFile->exists = 0;
}

void updateStreamVoice(int voiceIndex)
{
    NxVoice &v = s_voice[voiceIndex];
    releaseDoneWindows(voiceIndex);

    snd_stream_status status = Snd_StreamStatus(voiceIndex);
    if (!status || status == SND_STREAM_STARVING || !Snd_StreamGetHeader(voiceIndex))
        return;
    if (status == SND_STREAM_ERROR) {
        channelError(voiceIndex);
        return;
    }
    if (!v.active) {
        if (status != SND_STREAM_OK)
            return;
        const snd_asset *header = Snd_StreamGetHeader(voiceIndex);
        bool isLooping = (g_snd.voice[voiceIndex].alias->flags & 1) != 0;
        if (!createVoice(voiceIndex, g_snd.voice[voiceIndex].alias, header, isLooping)) {
            channelError(voiceIndex);
            return;
        }
        unsigned lengthMS = (unsigned)(1000ull * header->frame_count / header->frame_rate);
        SND_SetSoundFileVoiceInfo(voiceIndex, header->channel_count, header->frame_rate, lengthMS, 0, SFLS_LOADED);
        SND_UpdateVoice(&g_snd.voice[voiceIndex], 0.0f);
    }
    while (Snd_StreamGetFreeWindows(voiceIndex)) {
        if (v.queueCount >= 2)
            break;
        unsigned size = 0, position = 0;
        char *data = nullptr;
        status = Snd_StreamAcquireWindow(voiceIndex, &size, &position, &data);
        switch (status) {
        case SND_STREAM_OK: {
            int index = (int)(v.submitted % 2);
            ++v.submitted;
            v.windows[index] = data;
            v.windowDone[index] = false;
            NxVoice::Window &w = v.queue[(v.queueHead + v.queueCount) % 2];
            w.data = (const uint8_t *)data;
            w.size = size;
            w.offset = 0;
            w.index = index;
            ++v.queueCount;
            break;
        }
        case SND_STREAM_STARVING:
            return;
        case SND_STREAM_EOF:
            if (!v.queueCount)
                SND_StopVoice(voiceIndex);
            return;
        case SND_STREAM_ERROR:
            channelError(voiceIndex);
            return;
        default:
            return;
        }
    }
}

int startAliasStream(SndStartAliasInfo *startAliasInfo, unsigned int voiceIndex)
{
    const snd_alias_t *alias = startAliasInfo->alias;
    char filename[260];
    SND_AliasGetFileName(alias, filename, 256);
    if (!alias->soundFile->exists) {
        Com_PrintError(1, "Tried to play streamed sound '%s' from alias '%s', but it was not found at load time.\n",
                       filename, alias->name);
        return SND_SetPlaybackIdNotPlayed(voiceIndex);
    }
    unsigned primeSize = 0;
    char *primeData = nullptr;
    if ((alias->flags & 0xC000) >> 14 == 3 && alias->soundFile->u.streamSnd->primeSnd) {
        primeData = alias->soundFile->u.streamSnd->primeSnd->buffer;
        primeSize = alias->soundFile->u.streamSnd->primeSnd->size;
    }
    Snd_StreamOpen(voiceIndex, filename, (alias->flags & 1) != 0, primeSize, primeData);
    SND_SetVoiceStartInfo(voiceIndex, startAliasInfo);
    SND_SetSoundFileVoiceInfo(voiceIndex, 0, 0, 0, 0, SFLS_LOADING);
    SD_UpdateVoice(voiceIndex);
    return g_snd.voice[voiceIndex].alias ? startAliasInfo->playbackId : -1;
}

int startAliasRam(SndStartAliasInfo *startAliasInfo, int voiceIndex)
{
    bool isLooping = (startAliasInfo->alias->flags & 1) != 0;
    const snd_asset *snd = &startAliasInfo->alias->soundFile->u.loadSnd->sound;
    if (!createVoice(voiceIndex, startAliasInfo->alias, snd, isLooping))
        return -1;
    unsigned rate = snd->frame_rate;
    unsigned totalMsec = isLooping ? 0
        : startAliasInfo->startDelay + (unsigned)((double)snd->frame_count * 1000.0 / (double)rate);
    SND_SetSoundFileVoiceInfo(voiceIndex, snd->channel_count, rate, totalMsec, 0, SFLS_LOADED);
    SND_SetVoiceStartInfo(voiceIndex, startAliasInfo);
    return startAliasInfo->playbackId;
}

// SDXA2_UpdateVoiceSends, dry path only, folded to stereo.
void updateGains(int voiceIndex)
{
    const snd_voice_t *voice = &g_snd.voice[voiceIndex];
    NxVoice &v = s_voice[voiceIndex];
    unsigned in = voice->pan.input_channel_count;
    unsigned outs = voice->pan.output_channel_count;
    if (in > 2)
        in = 2;
    float gain[2][2] = {};
    for (unsigned j = 0; j < in; ++j) {
        if (outs >= 2) {
            gain[j][0] = (float)Snd_SpeakerMapGetVolume(&voice->pan, j, 0) * voice->dryLevel;
            gain[j][1] = (float)Snd_SpeakerMapGetVolume(&voice->pan, j, 1) * voice->dryLevel;
        } else {
            gain[j][0] = gain[j][1] = (float)Snd_SpeakerMapGetVolume(&voice->pan, j, 0) * voice->dryLevel;
        }
    }
    memcpy(v.gain, gain, sizeof(gain));
}

void noteCaller(uintptr_t addr, const snd_alias_t *alias)
{
    NxCallerTally *freeSlot = nullptr;
    for (NxCallerTally &t : s_loopStops) {
        if (t.addr == addr) {
            ++t.count;
            t.alias = alias;
            return;
        }
        if (!t.addr && !freeSlot)
            freeSlot = &t;
    }
    if (freeSlot) {
        freeSlot->addr = addr;
        freeSlot->count = 1;
        freeSlot->alias = alias;
    }
}

// Caller addresses as offsets into KisakBlack.elf, for addr2line.
void printLoopStops()
{
    MemoryInfo text = {};
    u32 pageInfo;
    svcQueryMemory(&text, &pageInfo, (u64)&noteCaller);
    for (NxCallerTally &t : s_loopStops) {
        if (!t.addr)
            continue;
        printf("[nx-snd]   loops stopped %ux by elf+0x%llx (last: %s)\n", t.count,
               (unsigned long long)(t.addr - text.addr),
               t.alias && t.alias->name ? t.alias->name : "?");
    }
    memset(s_loopStops, 0, sizeof(s_loopStops));
}

void printTop(NxStartTally (&table)[64], const char *what, int count)
{
    for (int n = 0; n < count; ++n) {
        NxStartTally *best = nullptr;
        for (NxStartTally &t : table)
            if (t.alias && t.count && (!best || t.count > best->count))
                best = &t;
        if (!best)
            break;
        printf("[nx-snd]   %s %ux: %s (%s)\n", what, best->count,
               best->alias->name ? best->alias->name : "?", best->stream ? "stream" : "loaded");
        best->count = 0;
    }
    memset(table, 0, sizeof(table));
}

void report()
{
    u64 now = armGetSystemTick();
    if (!s_reportTick) {
        s_reportTick = now;
        return;
    }
    u64 elapsed = now - s_reportTick;
    if (armTicksToNs(elapsed) < 10000000000ull)
        return;
    printf("[nx-snd] 10 s: started %u pcm, %u adpcm, %u wma; refused %u wma, %u other%s%s%s; "
           "peak %u playing; %u starved; mixer %.1f ms/s\n",
           s_nStartPcm, s_nStartAdpcm, s_nStartWma, s_nRefusedWma, s_nRefusedOther,
           s_lastRefused[0] ? " (last '" : "", s_lastRefused, s_lastRefused[0] ? "')" : "",
           s_peakRunning, s_nStarved,
           armTicksToNs(s_mixTicks) / 1e6 / (armTicksToNs(elapsed) / 1e9));
    printf("[nx-snd]   one-shots: %u played to their end, %u stopped early by the engine\n",
           s_nEnded, s_nCut);
    s_nStartPcm = s_nStartAdpcm = s_nStartWma = s_nRefusedWma = s_nRefusedOther = s_nStarved = 0;
    s_nEnded = s_nCut = 0;
    // The aliases started most -- a sound restarted over and over shows up
    // here -- and those the engine cut short most.
    printTop(s_tally, "started", 5);
    printTop(s_cutTally, "cut", 3);
    printf("[nx-snd]   wma decode errors: %u\n", s_nWmaErrors);
    s_nWmaErrors = 0;
    printf("[nx-snd]   looping voices stopped: %u\n", s_nLoopStops);
    s_nLoopStops = 0;
    printLoopStops();
    s_peakRunning = 0;
    s_mixTicks = 0;
    s_lastRefused[0] = 0;
    s_reportTick = now;
}

} // namespace

extern "C++" {

bool __cdecl SD_Xaudio2CanInit()
{
    return true;
}

char __cdecl SD_Init()
{
    rmutexInit(&s_lock);
    memset(s_voice, 0, sizeof(s_voice));

    // SND_InitMasterVoice: the engine pans for the speakers the device has.
    Snd_SetConfigStringsBySpeakerCount(2);
    for (unsigned index = 0; index < SND_GetSpeakerConfigCount(); ++index) {
        if (Snd_GetSpeakerConfig(index)->speakerCount == 2) {
            Dvar_SetInt((dvar_s *)snd_speakerConfiguration, index);
            break;
        }
    }

    bool out = startOutput();
    printf("[nx-snd] %s\n", out ? "audout up: 48 kHz stereo, software mixer running"
                                : "audout failed to start; sound stays silent");
    return 1;   // see the note at the top: must succeed regardless
}

void SD_Shutdown()
{
    if (s_threadRunning) {
        s_quit = true;
        threadWaitForExit(&s_thread);
        threadClose(&s_thread);
        s_threadRunning = false;
        audoutStopAudioOut();
        audoutExit();
        for (int i = 0; i < NX_OUT_BUFFERS; ++i) {
            free(s_outData[i]);
            s_outData[i] = nullptr;
        }
    }
}

void __cdecl SD_TruncateAudioDeviceNames(Font_s *, float, int) {}

int __cdecl SD_StartAlias(SndStartAliasInfo *startAliasInfo, unsigned int voice)
{
    if (SND_IsStream(voice))
        return startAliasStream(startAliasInfo, voice);
    return startAliasRam(startAliasInfo, (int)voice);
}

void __cdecl SD_StopVoice(int voiceIndex)
{
    Lock lock;
    NxVoice &v = s_voice[voiceIndex];
    if (v.active && v.looping) {
        ++s_nLoopStops;
        noteCaller((uintptr_t)g_nxSndStopCaller, g_snd.voice[voiceIndex].alias);
    }
    g_nxSndStopCaller = nullptr;
    if (v.active) {
        // Ended: the driver ran out of sound. Cut: the engine stopped it early.
        if (v.finished || (v.stream && !v.queueCount))
            ++s_nEnded;
        else if (!v.looping) {
            ++s_nCut;
            noteTally(s_cutTally, g_snd.voice[voiceIndex].alias, v.stream);
        }
    }
    v.active = false;
    v.running = false;
    if (SND_IsStream(voiceIndex)) {
        for (int i = 0; i < 2; ++i) {
            if (v.windows[i]) {
                char *w = v.windows[i];
                v.windows[i] = nullptr;
                Snd_StreamReleaseWindow(voiceIndex, w);
            }
            v.windowDone[i] = false;
        }
        v.queueCount = 0;
        if (Snd_StreamStatus(voiceIndex))
            Snd_StreamClose(voiceIndex);
    }
}

void __cdecl SD_UpdateVoice(unsigned int voiceIndex)
{
    Lock lock;
    if (SND_IsStream(voiceIndex)) {
        updateStreamVoice((int)voiceIndex);
        if (!g_snd.voiceAliasHash[voiceIndex])
            return;   // stopped while updating
    }
    snd_voice_t *voice = &g_snd.voice[voiceIndex];
    if (voice->soundFileInfo.loadingState == SFLS_LOADING)
        return;
    NxVoice &v = s_voice[voiceIndex];
    if (!v.active)
        return;
    updateGains((int)voiceIndex);
    unsigned queued = buffersQueued(v);
    if (!voice->startDelay && !v.samplesPlayed && queued && !voice->paused)
        v.running = true;
    if (queued)
        v.pitch = (float)SND_GetPitch(voice);
    else
        SND_StopVoice((int)voiceIndex);
}

void __cdecl SD_PreUpdate()
{
    report();
}

void __cdecl SD_PauseVoice(int voiceIndex)
{
    Lock lock;
    g_snd.voice[voiceIndex].paused = 1;
    if (g_snd.voice[voiceIndex].soundFileInfo.loadingState != SFLS_LOADING)
        s_voice[voiceIndex].running = false;
}

void __cdecl SD_UnpauseVoice(int voiceIndex)
{
    Lock lock;
    if (g_snd.voice[voiceIndex].soundFileInfo.loadingState != SFLS_LOADING)
        s_voice[voiceIndex].running = true;
}

} // extern "C++"
