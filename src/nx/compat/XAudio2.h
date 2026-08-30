// XAudio2.h -- XAudio2 2.7 compatibility declarations for the Switch build.
//
// The real driver TUs (snd_driver_xaudio2.cpp / snd_driver_xaudio2_dsp.cpp)
// are excluded from the Switch build and replaced by the null driver in
// src/nx/nx_snd_null.cpp; this header only has to make the sound HEADERS
// (snd_driver_xaudio2.h, snd_driver_xaudio2_dsp.h -> snd_dsp.h -> snd.h,
// included nearly everywhere) compile. No ABI fidelity required.
#ifndef NX_COMPAT_XAUDIO2_H
#define NX_COMPAT_XAUDIO2_H

#include "windows.h"

// ----- mmreg wave formats (shared with dsound.h) ---------------------------
#ifndef NX_WAVEFORMATEX_DEFINED
#define NX_WAVEFORMATEX_DEFINED
#pragma pack(push, 1)
typedef struct tWAVEFORMATEX {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
#pragma pack(pop)
#define WAVE_FORMAT_PCM 1
#endif

#pragma pack(push, 1)
typedef struct {
    WAVEFORMATEX Format;
    union {
        WORD wValidBitsPerSample;
        WORD wSamplesPerBlock;
        WORD wReserved;
    } Samples;
    DWORD dwChannelMask;
    GUID  SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;

typedef struct adpcmcoef_tag {
    short iCoef1;
    short iCoef2;
} ADPCMCOEFSET;
#pragma pack(pop)

#define WAVE_FORMAT_ADPCM      2
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

// ----- constants ------------------------------------------------------------
#define XAUDIO2_COMMIT_NOW      0
#define XAUDIO2_COMMIT_ALL      0
#define XAUDIO2_INVALID_OPSET   0xFFFFFFFFu
#define XAUDIO2_NO_LOOP_REGION  0
#define XAUDIO2_LOOP_INFINITE   255
#define XAUDIO2_DEFAULT_CHANNELS 0
#define XAUDIO2_DEFAULT_SAMPLERATE 0
#define XAUDIO2_DEFAULT_FREQ_RATIO 2.0f
#define XAUDIO2_MAX_FREQ_RATIO 1024.0f
#define XAUDIO2_END_OF_STREAM   0x0040u
#define XAUDIO2_SEND_USEFILTER  0x0080u
#define XAUDIO2_VOICE_NOPITCH   0x0002u
#define XAUDIO2_VOICE_NOSRC     0x0004u
#define XAUDIO2_VOICE_USEFILTER 0x0008u
#define XAUDIO2_DEBUG_ENGINE    0x0001u
#define XAUDIO2_ANY_PROCESSOR   0xFFFFFFFFu
#define XAUDIO2_MAX_BUFFER_BYTES 0x80000000u
#define XAUDIO2_MAX_QUEUED_BUFFERS 64
#define XAUDIO2_MAX_AUDIO_CHANNELS 64
#define XAUDIO2_MIN_SAMPLE_RATE 1000
#define XAUDIO2_MAX_SAMPLE_RATE 200000

typedef UINT32 XAUDIO2_PROCESSOR;
typedef XAUDIO2_PROCESSOR XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER;

typedef enum XAUDIO2_DEVICE_ROLE {
    NotDefaultDevice            = 0x0,
    DefaultConsoleDevice        = 0x1,
    DefaultMultimediaDevice     = 0x2,
    DefaultCommunicationsDevice = 0x4,
    DefaultGameDevice           = 0x8,
    GlobalDefaultDevice         = 0xf,
    InvalidDeviceRole           = ~GlobalDefaultDevice
} XAUDIO2_DEVICE_ROLE;

// ----- structs --------------------------------------------------------------
typedef struct XAUDIO2_DEVICE_DETAILS {
    WCHAR DeviceID[256];
    WCHAR DisplayName[256];
    XAUDIO2_DEVICE_ROLE Role;
    WAVEFORMATEXTENSIBLE OutputFormat;
} XAUDIO2_DEVICE_DETAILS;

typedef struct XAUDIO2_VOICE_DETAILS {
    UINT32 CreationFlags;
    UINT32 InputChannels;
    UINT32 InputSampleRate;
} XAUDIO2_VOICE_DETAILS;

class IXAudio2Voice;

typedef struct XAUDIO2_SEND_DESCRIPTOR {
    UINT32 Flags;
    IXAudio2Voice *pOutputVoice;
} XAUDIO2_SEND_DESCRIPTOR;

typedef struct XAUDIO2_VOICE_SENDS {
    UINT32 SendCount;
    XAUDIO2_SEND_DESCRIPTOR *pSends;
} XAUDIO2_VOICE_SENDS;

typedef struct XAUDIO2_EFFECT_DESCRIPTOR {
    IUnknown *pEffect;
    BOOL InitialState;
    UINT32 OutputChannels;
} XAUDIO2_EFFECT_DESCRIPTOR;

typedef struct XAUDIO2_EFFECT_CHAIN {
    UINT32 EffectCount;
    XAUDIO2_EFFECT_DESCRIPTOR *pEffectDescriptors;
} XAUDIO2_EFFECT_CHAIN;

typedef struct XAUDIO2_BUFFER {
    UINT32 Flags;
    UINT32 AudioBytes;
    const BYTE *pAudioData;
    UINT32 PlayBegin;
    UINT32 PlayLength;
    UINT32 LoopBegin;
    UINT32 LoopLength;
    UINT32 LoopCount;
    void *pContext;
} XAUDIO2_BUFFER;

typedef struct XAUDIO2_BUFFER_WMA {
    const UINT32 *pDecodedPacketCumulativeBytes;
    UINT32 PacketCount;
} XAUDIO2_BUFFER_WMA;

typedef struct XAUDIO2_VOICE_STATE {
    void *pCurrentBufferContext;
    UINT32 BuffersQueued;
    UINT64 SamplesPlayed;
} XAUDIO2_VOICE_STATE;

typedef struct XAUDIO2_PERFORMANCE_DATA {
    UINT64 AudioCyclesSinceLastQuery;
    UINT64 TotalCyclesSinceLastQuery;
    UINT32 MinimumCyclesPerQuantum;
    UINT32 MaximumCyclesPerQuantum;
    UINT32 MemoryUsageInBytes;
    UINT32 CurrentLatencyInSamples;
    UINT32 GlitchesSinceEngineStarted;
    UINT32 ActiveSourceVoiceCount;
    UINT32 TotalSourceVoiceCount;
    UINT32 ActiveSubmixVoiceCount;
    UINT32 TotalSubmixVoiceCount;
    UINT32 ActiveXmaSourceVoices;
    UINT32 ActiveXmaStreams;
} XAUDIO2_PERFORMANCE_DATA;

typedef struct XAUDIO2_DEBUG_CONFIGURATION {
    UINT32 TraceMask;
    UINT32 BreakMask;
    BOOL LogThreadID;
    BOOL LogFileline;
    BOOL LogFunctionName;
    BOOL LogTiming;
} XAUDIO2_DEBUG_CONFIGURATION;

typedef struct XAUDIO2_FILTER_PARAMETERS {
    int   Type;
    float Frequency;
    float OneOverQ;
} XAUDIO2_FILTER_PARAMETERS;

// ----- callback interface ---------------------------------------------------
class IXAudio2VoiceCallback {
public:
    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) PURE;
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) PURE;
    STDMETHOD_(void, OnStreamEnd) (THIS) PURE;
    STDMETHOD_(void, OnBufferStart) (THIS_ void *pBufferContext) PURE;
    STDMETHOD_(void, OnBufferEnd) (THIS_ void *pBufferContext) PURE;
    STDMETHOD_(void, OnLoopEnd) (THIS_ void *pBufferContext) PURE;
    STDMETHOD_(void, OnVoiceError) (THIS_ void *pBufferContext, HRESULT Error) PURE;
};

class IXAudio2EngineCallback {
public:
    STDMETHOD_(void, OnProcessingPassStart) (THIS) PURE;
    STDMETHOD_(void, OnProcessingPassEnd) (THIS) PURE;
    STDMETHOD_(void, OnCriticalError) (THIS_ HRESULT Error) PURE;
};

// ----- voice interfaces (implementations never exist on Switch) -------------
class IXAudio2Voice {
public:
    HRESULT GetVoiceDetails(XAUDIO2_VOICE_DETAILS *details);
    HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *sendList);
    HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *effectChain);
    HRESULT EnableEffect(UINT32 effectIndex, UINT32 operationSet = 0);
    HRESULT DisableEffect(UINT32 effectIndex, UINT32 operationSet = 0);
    HRESULT SetEffectParameters(UINT32 effectIndex, const void *params, UINT32 paramsSize, UINT32 operationSet = 0);
    HRESULT SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *params, UINT32 operationSet = 0);
    HRESULT SetVolume(float volume, UINT32 operationSet = 0);
    HRESULT SetChannelVolumes(UINT32 channels, const float *volumes, UINT32 operationSet = 0);
    HRESULT SetOutputMatrix(IXAudio2Voice *destination, UINT32 sourceChannels, UINT32 destinationChannels,
                            const float *levelMatrix, UINT32 operationSet = 0);
    void DestroyVoice();
};

class IXAudio2SourceVoice : public IXAudio2Voice {
public:
    HRESULT Start(UINT32 flags = 0, UINT32 operationSet = 0);
    HRESULT Stop(UINT32 flags = 0, UINT32 operationSet = 0);
    HRESULT SubmitSourceBuffer(const XAUDIO2_BUFFER *buffer, const XAUDIO2_BUFFER_WMA *bufferWMA = 0);
    HRESULT FlushSourceBuffers();
    HRESULT Discontinuity();
    HRESULT ExitLoop(UINT32 operationSet = 0);
    void GetState(XAUDIO2_VOICE_STATE *state);
    HRESULT SetFrequencyRatio(float ratio, UINT32 operationSet = 0);
    void GetFrequencyRatio(float *ratio);
};

class IXAudio2SubmixVoice : public IXAudio2Voice {};
class IXAudio2MasteringVoice : public IXAudio2Voice {};

class IXAudio2 : public IUnknown {
public:
    HRESULT GetDeviceCount(UINT32 *count);
    HRESULT GetDeviceDetails(UINT32 index, XAUDIO2_DEVICE_DETAILS *details);
    HRESULT Initialize(UINT32 flags = 0, XAUDIO2_PROCESSOR processor = XAUDIO2_ANY_PROCESSOR);
    HRESULT RegisterForCallbacks(IXAudio2EngineCallback *callback);
    void UnregisterForCallbacks(IXAudio2EngineCallback *callback);
    HRESULT CreateSourceVoice(IXAudio2SourceVoice **voice, const WAVEFORMATEX *sourceFormat,
                              UINT32 flags = 0, float maxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO,
                              IXAudio2VoiceCallback *callback = 0, const XAUDIO2_VOICE_SENDS *sendList = 0,
                              const XAUDIO2_EFFECT_CHAIN *effectChain = 0);
    HRESULT CreateSubmixVoice(IXAudio2SubmixVoice **voice, UINT32 inputChannels, UINT32 inputSampleRate,
                              UINT32 flags = 0, UINT32 processingStage = 0,
                              const XAUDIO2_VOICE_SENDS *sendList = 0, const XAUDIO2_EFFECT_CHAIN *effectChain = 0);
    HRESULT CreateMasteringVoice(IXAudio2MasteringVoice **voice, UINT32 inputChannels = XAUDIO2_DEFAULT_CHANNELS,
                                 UINT32 inputSampleRate = XAUDIO2_DEFAULT_SAMPLERATE, UINT32 flags = 0,
                                 UINT32 deviceIndex = 0, const XAUDIO2_EFFECT_CHAIN *effectChain = 0);
    HRESULT StartEngine();
    void StopEngine();
    HRESULT CommitChanges(UINT32 operationSet);
    void GetPerformanceData(XAUDIO2_PERFORMANCE_DATA *perfData);
    void SetDebugConfiguration(const XAUDIO2_DEBUG_CONFIGURATION *debugConfiguration, void *reserved = 0);
};

#endif // NX_COMPAT_XAUDIO2_H
