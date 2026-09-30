#include "snd_local.h"
#include "snd_log.h"

#include <cstring>
#include "snd_bank.h"
#include "snd_public_async.h"
#include <win32/win_shared.h>
#include <win32/win_voice.h>
#include "snd_utils.h"
#include <qcommon/common.h>
#include <cgame/cg_sound.h>
#include <win32/win_common.h>
#include "snd_globals.h"

void __cdecl SNDL_AliasName(char *name, unsigned int id)
{
    SND_LogRegisterString(name, id);
}

int __cdecl SNDL_Play(
                unsigned int aliasHash,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                float *position,
                const float *direction,
                bool notify,
                snd_playback *playback)
{
    snd_alias_list_t *list; // [esp+2Ch] [ebp-28h]
    float temp[3]; // [esp+30h] [ebp-24h] BYREF
    snd_occlusion_start_cache ocache; // [esp+3Ch] [ebp-18h] BYREF
    int pId; // [esp+50h] [ebp-4h]

    if ( !aliasHash )
        return -1;
    pId = -1;
    memset(&ocache, 0, sizeof(ocache));
    if ( !position )
    {
        memset(temp, 0, sizeof(temp));
        position = temp;
    }
    list = SND_FindAliasFromId(aliasHash);
    if ( list )
    {
        nanassertvec3(position);
        
        pId = SND_PlaySoundAlias(list, attenuation, entHandle, position, 0, fadeTimeMs, direction, playback, &ocache);
        if ( notify )
            SND_AddLengthNotify(pId, (const char*)entHandle.field.entIndex, SND_LENGTH_NOTIFY_SCRIPT);
    }
    else
    {
        SND_LogMissingAliasId(aliasHash);
    }
    if ( pId != -1 )
    {
        if ( notify )
            SND_AddLengthNotify(pId, (const char *)entHandle.field.entIndex, SND_LENGTH_NOTIFY_SCRIPT);
        if ( playback
            && playback->id != pId
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                        85,
                        0,
                        "%s",
                        "playback->id == pId") )
        {
            __debugbreak();
        }
    }
    if ( pId == -1 )
    {
        if ( playback )
            SND_FreePlaybackNotify(playback);
    }
    return pId;
}

void __cdecl SNDL_StopSoundAliasOnEnt(SndEntHandle sndEnt, unsigned int name)
{
    StopSoundAliasesOnEnt(sndEnt, name);
}

void __cdecl StopSoundAliasesOnEnt(SndEntHandle sndEnt, unsigned int name)
{
    snd_voice_t *voice; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 74; ++i)
    {
        voice = &g_snd.voice[i];
        if (g_snd.voiceAliasHash[i]
            && voice->sndEnt.handle == sndEnt.handle
            && ((voice->alias->flags & 1) != 0 || name || SND_IsAliasStopOnDeath(voice->alias))
            && (!name || name == g_snd.voiceAliasHash[i]))
        {
            SND_StopVoice(i);
        }
    }
}

bool __cdecl SND_IsAliasStopOnDeath(const snd_alias_t *alias)
{
    if ( !alias && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_alias_db.h", 476, 0, "%s", "alias") )
        __debugbreak();
    return (alias->flags & 0x200) >> 9 != 0;
}

void __cdecl SNDL_StopSoundsOnEnt(SndEntHandle sndEnt)
{
    StopSoundAliasesOnEnt(sndEnt, 0);
}

void __cdecl SNDL_NotifyCinematicStart(float volume)
{
    if (!g_snd.inCinematic && volume > 0.0000152879)
    {
        g_snd.inCinematic = 1;
        g_snd.cinematicUpdate = 1;
        g_snd.cinematicTimestamp = Sys_Milliseconds();
    }
}

void __cdecl SNDL_NotifyCinematicEnd()
{
    if (g_snd.inCinematic)
    {
        g_snd.inCinematic = 0;
        g_snd.cinematicUpdate = 1;
        g_snd.cinematicTimestamp = Sys_Milliseconds();
    }
}

void __cdecl SNDL_DisconnectListener(int localClientNum)
{
    bool v2; // [esp+0h] [ebp-14h]
    snd_listener *v3; // [esp+4h] [ebp-10h]
    int voiceIndex; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+Ch] [ebp-8h]
    bool kill; // [esp+13h] [ebp-1h]

    if (localClientNum
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
            182,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1))
    {
        __debugbreak();
    }
    memset((unsigned __int8 *)&g_snd.listeners[localClientNum], 0, sizeof(g_snd.listeners[localClientNum]));
    AxisClear(g_snd.listeners[localClientNum].orient.axis);
    v3 = &g_snd.listeners[localClientNum];
    v3->orient.origin[0] = 0.0f;
    v3->orient.origin[1] = 0.0f;
    v3->orient.origin[2] = 0.0f;
    kill = 1;
    for (i = 0; !i; i = 1)
    {
        v2 = kill && !g_snd.listeners[0].active;
        kill = v2;
    }
    if (kill)
    {
        for (voiceIndex = 0; voiceIndex < 74; ++voiceIndex)
        {
            if (g_snd.voiceAliasHash[voiceIndex] && (g_snd.voice[voiceIndex].alias->flags & 2) >> 1)
                SND_StopVoice(voiceIndex);
        }
    }
}

void __cdecl SNDL_SetListener(
                int localClientNum,
                int clientNum,
                team_t team,
                const float *origin,
                const float (*inAxis)[3])
{
    snd_listener *v5; // [esp+10h] [ebp-88h]
    float b; // [esp+68h] [ebp-30h]
    float axis[3][3]; // [esp+6Ch] [ebp-2Ch] BYREF
    float a; // [esp+90h] [ebp-8h]
    bool independant; // [esp+97h] [ebp-1h]

    if ( !origin && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 213, 0, "%s", "origin") )
        __debugbreak();
    if ( !inAxis && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 214, 0, "%s", "inAxis") )
        __debugbreak();
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                    215,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    nanassertvec3(origin);
    nanassertvec3(inAxis[0]);
    nanassertvec3(inAxis[1]);
    nanassertvec3(inAxis[2]);

    AxisCopy(inAxis, axis);
    axis[0][2] = 0.0f;
    axis[1][2] = 0.0f;
    axis[2][0] = 0.0f;
    //*(_QWORD *)&axis[2][1] = __PAIR64__(LODWORD(1.0f), 0);
    axis[2][1] = 0.0f;
    axis[2][2] = 1.0f;
    a = Vec3Normalize(axis[0]);
    b = Vec3Normalize(axis[1]);
    if ( (LODWORD(a) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 235, 0, "%s", "!IS_NAN(a)") )
    {
        __debugbreak();
    }
    if ( (LODWORD(b) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 236, 0, "%s", "!IS_NAN(b)") )
    {
        __debugbreak();
    }
    independant = (float)((float)((float)(axis[0][0] * axis[1][0]) + (float)(axis[0][1] * axis[1][1]))
                                            + (float)(axis[0][2] * axis[1][2])) < 0.99998474;
    if ( a > 0.0000152879 && b > 0.0000152879 && independant )
    {
        AxisCopy(axis, g_snd.listeners[localClientNum].orient.axis);
        v5 = &g_snd.listeners[localClientNum];
        v5->orient.origin[0] = *origin;
        v5->orient.origin[1] = origin[1];
        v5->orient.origin[2] = origin[2];
        g_snd.listeners[localClientNum].clientNum = clientNum;
        g_snd.listeners[localClientNum].active = 1;
    }
    if ( (float)((float)((float)(g_snd.listeners[localClientNum].orient.axis[0][0]
                                                         * g_snd.listeners[localClientNum].orient.axis[0][0])
                                         + (float)(g_snd.listeners[localClientNum].orient.axis[0][1]
                                                         * g_snd.listeners[localClientNum].orient.axis[0][1]))
                         + (float)(g_snd.listeners[localClientNum].orient.axis[0][2]
                                         * g_snd.listeners[localClientNum].orient.axis[0][2])) <= 0.99998474
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                    250,
                    0,
                    "%s",
                    "Vec3LengthSq(g_snd.listeners[localClientNum].orient.axis[0]) > 1.0f-SND_EPSILON") )
    {
        __debugbreak();
    }
    if ( (float)((float)((float)(g_snd.listeners[localClientNum].orient.axis[1][0]
                                                         * g_snd.listeners[localClientNum].orient.axis[1][0])
                                         + (float)(g_snd.listeners[localClientNum].orient.axis[1][1]
                                                         * g_snd.listeners[localClientNum].orient.axis[1][1]))
                         + (float)(g_snd.listeners[localClientNum].orient.axis[1][2]
                                         * g_snd.listeners[localClientNum].orient.axis[1][2])) <= 0.99998474
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                    251,
                    0,
                    "%s",
                    "Vec3LengthSq(g_snd.listeners[localClientNum].orient.axis[1]) > 1.0f-SND_EPSILON") )
    {
        __debugbreak();
    }
    if ( (float)((float)((float)(g_snd.listeners[localClientNum].orient.axis[2][0]
                                                         * g_snd.listeners[localClientNum].orient.axis[2][0])
                                         + (float)(g_snd.listeners[localClientNum].orient.axis[2][1]
                                                         * g_snd.listeners[localClientNum].orient.axis[2][1]))
                         + (float)(g_snd.listeners[localClientNum].orient.axis[2][2]
                                         * g_snd.listeners[localClientNum].orient.axis[2][2])) <= 0.99998474
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                    252,
                    0,
                    "%s",
                    "Vec3LengthSq(g_snd.listeners[localClientNum].orient.axis[2]) > 1.0f-SND_EPSILON") )
    {
        __debugbreak();
    }
    g_snd.listeners[localClientNum].clientNum = clientNum;
    g_snd.listeners[localClientNum].active = 1;
    g_snd.listeners[localClientNum].team = team;
}

void __cdecl SNDL_StopSounds(snd_stop_sound_flags which)
{
    unsigned int voiceIndex; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( which != SND_EVERY_SINGLE_ONE_DONT_ASK_ANY_QUESTIONS && which )
    {
        if ( which == SND_STOP_STREAMED )
        {
            for ( voiceIndex = 10; voiceIndex < 0x4A; ++voiceIndex )
            {
                if ( g_snd.voiceAliasHash[voiceIndex] )
                    SND_StopVoice(voiceIndex);
            }
        }
    }
    else
    {
        for ( i = 0; i < 0x4A; ++i )
            SND_StopVoice(i);
    }
}

void __cdecl SNDL_FadeOut()
{
    SND_FaderSetRate(&g_snd.volume, 0.5);
    SND_FaderSetGoal(&g_snd.volume, 0.0);
    Voice_StopClientSamples();
}

void __cdecl SNDL_FadeIn()
{
    SND_FaderSetRate(&g_snd.volume, 4.0);
    SND_FaderSetGoal(&g_snd.volume, 1.0);
}

void __cdecl SNDL_SetEnvironmentEffects(
                int priority,
                unsigned int roomstring,
                float drylevel,
                float wetlevel,
                int fademsec)
{
    snd_enveffect *effect; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (SND_ActiveListenerCount() == 1)
    {
        if ((priority <= 0 || priority >= 3)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                318,
                0,
                "%s\n\t(priority) = %i",
                "(priority > SND_ENVEFFECTPRIO_NONE && priority < SND_ENVEFFECTPRIO_COUNT)",
                priority))
        {
            __debugbreak();
        }
        if (!roomstring
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 319, 0, "%s", "roomstring"))
        {
            __debugbreak();
        }
        if (fademsec < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 320, 0, "%s", "fademsec >= 0"))
        {
            __debugbreak();
        }
        if ((drylevel < 0.0 || drylevel > 1.0)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                321,
                0,
                "%s",
                "drylevel >= 0 && drylevel <= 1"))
        {
            __debugbreak();
        }
        if ((wetlevel < 0.0 || wetlevel > 1.0)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                322,
                0,
                "%s",
                "wetlevel >= 0 && wetlevel <= 1"))
        {
            __debugbreak();
        }
        effect = &g_snd.envEffects[priority];
        effect->active = 1;
        if (fademsec < 1)
            fademsec = 1;
        effect->drygoal = drylevel;
        effect->drylevel = g_snd.effect->drylevel;
        effect->dryrate = (float)(drylevel - g_snd.effect->drylevel) / (float)fademsec;
        effect->wetgoal = wetlevel;
        effect->wetlevel = g_snd.effect->wetlevel;
        effect->wetrate = (float)(wetlevel - g_snd.effect->wetlevel) / (float)fademsec;
        effect->reverbId = roomstring;
        if (effect != g_snd.effect)
        {
            for (i = priority + 1; i < 3; ++i)
            {
                if (g_snd.envEffects[i].active)
                    return;
            }
            g_snd.effect = &g_snd.envEffects[priority];
        }
    }
}

void __cdecl SNDL_DeactivateEnvironmentEffects(int priority, int fademsec)
{
    snd_enveffect *effect; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (SND_ActiveListenerCount() == 1)
    {
        if ((priority <= 0 || priority >= 3)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                367,
                0,
                "%s\n\t(priority) = %i",
                "(priority > SND_ENVEFFECTPRIO_NONE && priority < SND_ENVEFFECTPRIO_COUNT)",
                priority))
        {
            __debugbreak();
        }
        if (fademsec < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp", 368, 0, "%s", "fademsec >= 0"))
        {
            __debugbreak();
        }
        effect = &g_snd.envEffects[priority];
        effect->active = 0;
        if (effect == g_snd.effect)
        {
            for (i = priority - 1; i >= 0 && !g_snd.envEffects[i].active; --i)
                ;
            if (i < 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                    381,
                    0,
                    "%s",
                    "i >= SND_ENVEFFECTPRIO_NONE"))
            {
                __debugbreak();
            }
            if (fademsec < 1)
                fademsec = 1;
            g_snd.effect = &g_snd.envEffects[i];
            g_snd.envEffects[i].drylevel = effect->drylevel;
            g_snd.effect->dryrate = (float)(g_snd.effect->drygoal - effect->drylevel) / (float)fademsec;
            g_snd.effect->wetlevel = effect->wetlevel;
            g_snd.effect->wetrate = (float)(g_snd.effect->wetgoal - effect->wetlevel) / (float)fademsec;
        }
    }
}

void __cdecl SNDL_SetPlaybackAttenuation(int playbackId, float attenuation)
{
    float g; // [esp+4h] [ebp-1Ch]
    float v3; // [esp+18h] [ebp-8h]
    snd_voice_t *voice; // [esp+1Ch] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
    {
        if ( (float)(attenuation - 1.0) < 0.0 )
            v3 = attenuation;
        else
            v3 = 1.0f;
        if ( (float)(0.0 - attenuation) < 0.0 )
            g = v3;
        else
            g = 0.0f;
        SND_FaderSetGoal(&voice->script_fade, g);
    }
}

void __cdecl SNDL_SetPlaybackAttenuationRate(int playbackId, float rate)
{
    snd_voice_t *voice; // [esp+8h] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
        SND_FaderSetRate(&voice->script_fade, rate);
}

void __cdecl SNDL_SetPlaybackPitch(int playbackId, float pitch)
{
    float g; // [esp+4h] [ebp-1Ch]
    float v3; // [esp+18h] [ebp-8h]
    snd_voice_t *voice; // [esp+1Ch] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
    {
        if ( (float)(pitch - 2.0) < 0.0 )
            v3 = pitch;
        else
            v3 = 2.0f;
        if ( (float)(0.1 - pitch) < 0.0 )
            g = v3;
        else
            g = 0.1f;
        SND_FaderSetGoal(&voice->script_pitch, g);
    }
}

void __cdecl SNDL_SetPlaybackPitchRate(int playbackId, float rate)
{
    snd_voice_t *voice; // [esp+8h] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
        SND_FaderSetRate(&voice->script_pitch, rate);
}

void __cdecl SNDL_StopPlayback(int playbackId)
{
    snd_voice_t *voice; // [esp+0h] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( voice )
        SND_StopVoice(voice - g_snd.voice);
}

void __cdecl SNDL_SetSnapshot(snd_snapshot_type type, unsigned int id, float length, float amount)
{
    if ((unsigned int)type >= SND_SNAPSHOT_COUNT
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
            464,
            0,
            "type doesn't index SND_SNAPSHOT_COUNT\n\t%i not in [0, %i)",
            type,
            11))
    {
        __debugbreak();
    }
    if (SND_GetSnapshotById(id))
        g_snd.snapshotCategories[type].snapshot = id;
    else
        g_snd.snapshotCategories[type].snapshot = g_snd.defaultHash;
    g_snd.snapshotCategories[type].length = length;
    g_snd.snapshotCategories[type].amount = amount;
}

void __cdecl SNDL_SetGameState(bool isMature, bool isPaused, float timescale, unsigned int cg_time, unsigned int seed)
{
    unsigned int v6; // eax
    unsigned int v7; // [esp-4h] [ebp-4h]

    g_snd.gameState.mature = isMature;
    g_snd.gameState.gamePaused = isPaused;
    g_snd.gameState.timescale = timescale;
    g_snd.gameState.cgTime = cg_time;
    if (seed)
        g_snd.gameState.seed = seed;
    if (isMature)
        v7 = SND_HashName("explicit");
    else
        v7 = SND_HashName("safe");
    v6 = SND_HashName("mature");
    SNDL_SetContext(v6, v7);
}

void __cdecl SNDL_SetScriptTimescale(float value)
{
    g_snd.scriptTimescale = value;
}

void __cdecl SNDL_PlayLoopAt(unsigned int id, float *origin)
{
    float *v2; // [esp+0h] [ebp-Ch]
    unsigned int i; // [esp+4h] [ebp-8h]
    bool gotIt; // [esp+Bh] [ebp-1h]

    gotIt = 0;
    for ( i = 0; i < 0x100; ++i )
    {
        if ( !g_snd.loopEmitters[i].id )
        {
            g_snd.loopEmitters[i].id = id;
            g_snd.loopEmitters[i].alias = 0;
            v2 = g_snd.loopEmitters[i].origin;
            *v2 = *origin;
            v2[1] = origin[1];
            v2[2] = origin[2];
            gotIt = 1;
            break;
        }
    }
    if ( !gotIt )
        Com_PrintError(9, "ERROR: Sound ran out of loop emitters\n");
}

void __cdecl SNDL_StopLoopAt(unsigned int id, const float *origin)
{
    float *v2; // [esp+14h] [ebp-1Ch]
    SndEntHandle sndEnt; // [esp+28h] [ebp-8h]
    unsigned int i; // [esp+2Ch] [ebp-4h]

    for ( i = 0; i < 0x100; ++i )
    {
        if ( g_snd.loopEmitters[i].id == id && Vec3DistanceSq(origin, g_snd.loopEmitters[i].origin) < 0.0000152879 )
        {
            if ( !g_snd.loopEmitters[i].alias
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_local.cpp",
                            548,
                            0,
                            "%s",
                            "g_snd.loopEmitters[i].alias") )
            {
                __debugbreak();
            }
            sndEnt.field = SND_EntHandle(0, i + 3710, 0, 0, 1, TEAM_FREE).field;
            SND_ContinueLoopingSound(g_snd.loopEmitters[i].id, 1.0, sndEnt, 0, -500, 0);
            g_snd.loopEmitters[i].id = 0;
            g_snd.loopEmitters[i].alias = 0;
            v2 = g_snd.loopEmitters[i].origin;
            *v2 = 0.0f;
            v2[1] = 0.0f;
            v2[2] = 0.0f;
        }
    }
}

void __cdecl SNDL_PlayLineAt(unsigned int id, float *origin0, float *origin1)
{
    float *v3; // [esp+0h] [ebp-14h]
    float *v4; // [esp+4h] [ebp-10h]
    unsigned int i; // [esp+8h] [ebp-Ch]
    snd_alias_list_t *list; // [esp+Ch] [ebp-8h]
    bool gotIt; // [esp+13h] [ebp-1h]

    list = SND_FindAliasFromId(id);
    if (list && list->head)
    {
        gotIt = 0;
        for (i = 0; i < 0x80; ++i)
        {
            if (!g_snd.lineEmitters[i].id)
            {
                g_snd.lineEmitters[i].id = id;
                v4 = g_snd.lineEmitters[i].origin[0];
                *v4 = *origin0;
                v4[1] = origin0[1];
                v4[2] = origin0[2];
                v3 = g_snd.lineEmitters[i].origin[1];
                *v3 = *origin1;
                v3[1] = origin1[1];
                v3[2] = origin1[2];
                gotIt = 1;
                break;
            }
        }
        if (!gotIt)
            Com_PrintError(9, "ERROR: Sound ran out of line emitters\n");
    }
}

void __cdecl SNDL_StopLineAt(unsigned int id, const float *origin0, const float *origin1)
{
    float *v4; // [esp+14h] [ebp-2Ch]
    float *v5; // [esp+18h] [ebp-28h]
    SndEntHandle sndEnt; // [esp+38h] [ebp-8h]
    unsigned int i; // [esp+3Ch] [ebp-4h]

    for (i = 0; i < 0x80; ++i)
    {
        if (g_snd.lineEmitters[i].id == id
            && Vec3DistanceSq(origin0, g_snd.lineEmitters[i].origin[0]) < 0.0000152879
            && Vec3DistanceSq(origin1, g_snd.lineEmitters[i].origin[1]) < 0.0000152879)
        {
            sndEnt.field = SND_EntHandle(0, i + 3966, 0, 0, 1, TEAM_FREE).field;
            SND_ContinueLoopingSound(g_snd.lineEmitters[i].id, 1.0, sndEnt, 0, -500, 0);
            g_snd.lineEmitters[i].id = 0;
            g_snd.lineEmitters[i].alias = 0;
            v5 = g_snd.lineEmitters[i].origin[0];
            *v5 = 0.0f;
            v5[1] = 0.0f;
            v5[2] = 0.0f;
            v4 = g_snd.lineEmitters[i].origin[1];
            *v4 = 0.0f;
            v4[1] = 0.0f;
            v4[2] = 0.0f;
        }
    }
}

int __cdecl SNDL_GetPlaybackTime(int playbackId)
{
    snd_voice_t *voice; // [esp+0h] [ebp-4h]

    voice = SND_GetPlaybackVoice(playbackId);
    if ( !voice )
        return 0;
    if ( g_snd.pausetime )
        return g_snd.pausetime - voice->startTime;
    return g_snd.time - voice->startTime;
}

void __cdecl SNDL_GameReset()
{
    const snd_alias_t *alias; // esi
    snd_voice_t *voice; // [esp+1Ch] [ebp-Ch]
    unsigned int i; // [esp+20h] [ebp-8h]

    if (SND_Active())
    {
        SNDL_FadeIn();
        SND_ResetEntState();
        memset((unsigned __int8 *)g_snd.loopEmitters, 0, sizeof(g_snd.loopEmitters));
        memset((unsigned __int8 *)g_snd.lineEmitters, 0, sizeof(g_snd.lineEmitters));
        memset((unsigned __int8 *)g_snd.currentContexts, 0, sizeof(g_snd.currentContexts));
        for (i = 0; i < 0x4A; ++i)
        {
            if (g_snd.voiceAliasHash[i])
            {
                voice = &g_snd.voice[i];
                alias = voice->alias;
                if (alias->id == SND_HashName("mus_mp_frontend"))
                {
                    if ((voice->alias->flags & 1) != 0)
                    {
                        if (SND_IsAliasMusic(voice->alias))
                        {
                            SND_FaderSetRateTime(&voice->fade, 4.0);
                            SND_FaderSetGoal(&voice->fade, 0.0);
                        }
                    }
                }
                else
                {
                    SND_StopVoice(i);
                }
            }
        }
        SNDL_Update();
        Sys_EnterCriticalSection(CRITSEC_SOUND_OCCLUSION);
        Sys_LeaveCriticalSection(CRITSEC_SOUND_OCCLUSION);
        g_snd.traceSequence = 0;
        g_snd.lastTraceSequence = 0;
        SNDL_SetSnapshot(SND_SNAPSHOT_AMBIENT, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_SCRIPT, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_SHELLSHOCK, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_ADS, g_snd.defaultHash, 0.0, 1.0);
        SNDL_SetSnapshot(SND_SNAPSHOT_BREATH, g_snd.defaultHash, 0.0, 1.0);
        g_snd.scriptTimescale = 1.0f;
        memset((unsigned __int8 *)g_snd.envEffects, 0, sizeof(g_snd.envEffects));
        g_snd.effect = g_snd.envEffects;
        g_snd.envEffects[0].reverbId = g_snd.defaultHash;
        g_snd.envEffects[0].drylevel = 1.0f;
        g_snd.effect->drygoal = 1.0f;
        g_snd.effect->dryrate = 0.0f;
        g_snd.effect->wetlevel = 0.0f;
        g_snd.effect->wetgoal = 0.0f;
        g_snd.effect->wetrate = 0.0f;
        g_snd.effect->active = 1;
    }
}

bool __cdecl SND_IsAliasMusic(const snd_alias_t *alias)
{
    if ( !alias && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_alias_db.h", 417, 0, "%s", "alias") )
        __debugbreak();
    return (alias->flags & 0x100) >> 8 != 0;
}

void __cdecl SNDL_SetContext(unsigned int type, unsigned int value)
{
    signed int index; // [esp+0h] [ebp-4h]

    index = SND_FindContextIndex(type);
    if ( index >= 0 )
        g_snd.currentContexts[index] = value;
}

#ifdef KISAK_NX
// SND_Frame pumps every queued command, then runs SNDL_Update, which continues
// the loop emitters (SND_PlayLoopAt: ambient fires, chandeliers). When the
// sound job runs late it pumps two client frames at once -- two UPDATE_LOOPS
// with no SNDL_Update between them -- and the second retired every emitter
// loop as not continued; SNDL_Update then restarted it from the top, over and
// over at the Switch's frame rate. Only the first check after an emitter
// update retires; later ones just advance looptime.
bool g_nxLoopsRetiredThisSndFrame;
#endif

void __cdecl SNDL_UpdateLoopingSounds()
{
    snd_voice_t *voice; // [esp+4h] [ebp-8h]
    unsigned int i; // [esp+8h] [ebp-4h]

#ifdef KISAK_NX
    if (!g_snd.paused && g_nxLoopsRetiredThisSndFrame)
    {
        ++g_snd.looptime;
        return;
    }
    g_nxLoopsRetiredThisSndFrame = true;
#endif
    if (!g_snd.paused)
    {
        for (i = 0; i < 0x4A; ++i)
        {
            voice = &g_snd.voice[i];
            if (g_snd.voiceAliasHash[i]
                && (voice->alias->flags & 1) != 0
                && voice->fade.goal != 0.0
                && voice->looptime != g_snd.looptime
                && (voice->sndEnt.handle & 0xFFF) != 0xFFE)
            {
                SND_StopVoice(i);
            }
        }
        ++g_snd.looptime;
    }
}

