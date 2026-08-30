#include "snd_public_async.h"
#include <qcommon/common.h>
#include "snd_dvar.h"
#include "snd_public_async_q.h"
#include "snd_utils.h"
#include "snd_bank.h"
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame/cg_sound.h>
#include "snd_globals.h"
#include <win32/win_common.h>
#include <universal/com_workercmds.h>

#include <Windows.h>
#include "snd_local.h"

volatile unsigned int updatesound_workerLimit = 1;
jqModule updatesound_workerModule =
{
    .Name = "updatesound_worker",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))updatesound_workerCallback,
};
jqWorkerCmd updatesound_workerWorkerCmd = { &updatesound_workerModule, 4u, 0, 0, &updatesound_workerLimit, NULL, 0u };



volatile unsigned int entryCount;

void __cdecl SND_PlayInternal(
                unsigned int id,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify,
                snd_playback *playback)
{
    snd_command *cmd; // [esp+44h] [ebp-4h]

    if ( (*(_WORD *)&entHandle.field & 0xFFFu) >= 0x600
        && (*(_WORD *)&entHandle.field & 0xFFF) != 0xFFF
        && (*(_WORD *)&entHandle.field & 0xFFF) != 0xFFE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    66,
                    0,
                    "%s",
                    "entHandle.field.entIndex < MAX_LOCAL_CENTITIES || entHandle.field.entIndex == SND_ENT_NONE || entHandle.field."
                    "entIndex == SND_ENT_NO_STOP") )
    {
        __debugbreak();
    }
    if ( (*(_WORD *)&entHandle.field & 0xFFFu) >= 0x600
        && (*(_WORD *)&entHandle.field & 0xFFF) != 0xFFF
        && (*(_WORD *)&entHandle.field & 0xFFF) != 0xFFE )
    {
        Com_Error(ERR_DROP, "Invalid sound handle");
    }
    if ( SND_Active() && id )
    {
        if ( fadeTimeMs >= 0x7FFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        88,
                        0,
                        "%s",
                        "fadeTimeMs < 32767") )
        {
            __debugbreak();
        }
        if ( fadeTimeMs <= -32767
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        89,
                        0,
                        "%s",
                        "fadeTimeMs > -32767") )
        {
            __debugbreak();
        }
        if ( (LODWORD(attenuation) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        90,
                        0,
                        "%s",
                        "!IS_NAN(attenuation)") )
        {
            __debugbreak();
        }
        if ( attenuation < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        91,
                        0,
                        "%s",
                        "attenuation >= 0.0f") )
        {
            __debugbreak();
        }
        if ( attenuation > 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        92,
                        0,
                        "%s",
                        "attenuation <= 1.0f") )
        {
            __debugbreak();
        }

        if (position)
        {
            nanassertvec3(position);
        }
        if (direction)
        {
            nanassertvec3(direction);
        }

        if ( snd_assert_on_enqueue
            && snd_assert_on_enqueue->current.integer
            && *(_BYTE *)snd_assert_on_enqueue->current.string
            && SND_HashName(snd_assert_on_enqueue->current.string) == id
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        104,
                        0,
                        "%s",
                        "SND_HashName(snd_assert_on_enqueue->current.string) != id") )
        {
            __debugbreak();
        }
        if ( playback
            && playback->id == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        109,
                        0,
                        "%s",
                        "playback->id != SND_PLAYBACKID_NOTPLAYED") )
        {
            __debugbreak();
        }
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_PLAY;
            cmd->context.play.alias = id;
            cmd->context.play.fadeTimeMs = fadeTimeMs;
            cmd->context.play.attenuation = attenuation;
            cmd->context.play.entHandle = entHandle;
            cmd->context.play.notify = notify;
            cmd->context.play.playback = playback;
            if ( position )
            {
                nanassertvec3(position);

                cmd->context.play.position[0] = *position;
                cmd->context.play.position[1] = position[1];
                cmd->context.play.position[2] = position[2];
            }
            else
            {
                cmd->context.play.position[0] = 0.0f;
                cmd->context.play.position[1] = 0.0f;
                cmd->context.play.position[2] = 0.0f;
            }
            if ( direction )
            {
                nanassertvec3(direction);

                cmd->context.play.direction[0] = *direction;
                cmd->context.play.direction[1] = direction[1];
                cmd->context.play.direction[2] = direction[2];
            }
            else
            {
                cmd->context.play.direction[0] = 0.0f;
                cmd->context.play.direction[1] = 0.0f;
                cmd->context.play.direction[2] = 0.0f;
            }
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_Play(
                unsigned int id,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify)
{
    SND_PlayInternal(id, fadeTimeMs, attenuation, entHandle, position, direction, notify, 0);
}

void __cdecl SND_Play(
                char *alias,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify)
{
    unsigned int AliasId; // eax

    if ( SND_Active() )
    {
        AliasId = SND_FindAliasId(alias);
        SND_PlayInternal(AliasId, fadeTimeMs, attenuation, entHandle, position, direction, notify, 0);
    }
}

int __cdecl SND_Playback(
                unsigned int alias,
                int fadeTimeMs,
                float attenuation,
                SndEntHandle entHandle,
                const float *position,
                const float *direction,
                bool notify)
{
    snd_playback *playback; // [esp+1Ch] [ebp-4h]

    if ( !SND_Active() )
        return -1;
    playback = SND_AllocatePlayback();
    SND_PlayInternal(alias, fadeTimeMs, attenuation, entHandle, position, direction, notify, playback);
    if ( playback )
        return playback->id;
    else
        return -1;
}

void __cdecl SND_StopSoundAliasOnEnt(SndEntHandle ent, unsigned int alias_name)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_ALIAS;
            cmd->context.stop_alias.ent = ent;
            cmd->context.play.fadeTimeMs = alias_name;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopSoundsOnEnt(SndEntHandle ent)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_ENT;
            cmd->context.stop_alias.ent = ent;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_NotifyCinematicStart(float volume)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_CINEMATIC_START;
            cmd->context.notify_cinematic_start.volume = volume;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_NotifyCinematicEnd()
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_CINEMATIC_END;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_DisconnectListener(unsigned int listener)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_DISCONNECT_LISTENER;
            cmd->context.play.alias = listener;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetListener(
                unsigned int listener,
                int clientNum,
                team_t team,
                const float *origin,
                const float (*axis)[3])
{
    double v5; // st7
    snd_command *cmd; // [esp+2Ch] [ebp-4h]

    if ( SND_Active() )
    {
        iassert(Vec3Length(axis[0]) > 1.0f - (1.52879e-5f) && Vec3Length(axis[0]) < 1.0f + (1.52879e-5f));
        //if ( Abs((const float *)axis) <= 0.99998474 || Abs((const float *)axis) >= 1.0000153 )
        //{
        //    v5 = Abs((const float *)axis);
        //    if ( !Assert_MyHandler(
        //                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
        //                    335,
        //                    0,
        //                    "%s\n\t(Vec3Length(axis[0])) = %g",
        //                    "(Vec3Length(axis[0]) > 1.0f-(1.52879e-5f) && Vec3Length(axis[0]) < 1.0f+(1.52879e-5f))",
        //                    v5) )
        //        __debugbreak();
        //}
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_LISTENER;
            cmd->context.play.alias = listener;
            cmd->context.play.fadeTimeMs = clientNum;
            cmd->context.set_game_state.cg_time = team;
            cmd->context.set_listener.origin[0] = *origin;
            cmd->context.set_listener.origin[1] = origin[1];
            cmd->context.play.position[0] = origin[2];
            cmd->context.play.position[1] = (*axis)[0];
            cmd->context.play.position[2] = (*axis)[1];
            cmd->context.play.direction[0] = (*axis)[2];
            cmd->context.play.direction[1] = (*axis)[3];
            cmd->context.play.direction[2] = (*axis)[4];
            cmd->context.set_listener.axis[1][2] = (*axis)[5];
            cmd->context.set_listener.axis[2][0] = (*axis)[6];
            cmd->context.set_listener.axis[2][1] = (*axis)[7];
            cmd->context.set_listener.axis[2][2] = (*axis)[8];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopSounds(snd_stop_sound_flags flags)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_SOUNDS;
            cmd->context.play.alias = flags;
            SND_CommandPush(cmd);
        }
        SND_UpdateWait();
    }
}

void __cdecl SND_FadeIn()
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_FADE_IN;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_FadeOut()
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_FADE_OUT;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetEnvironmentEffects(
                unsigned int priority,
                const char *preset,
                float drylevel,
                float wetlevel,
                int fademsec)
{
    const snd_radverb *radverb; // [esp+0h] [ebp-Ch]
    snd_command *cmd; // [esp+4h] [ebp-8h]
    int id; // [esp+8h] [ebp-4h]

    if ( SND_Active() )
    {
        id = SND_HashName(preset);
        radverb = SND_GetRadverb(id);
        if ( !radverb || radverb->id == g_snd.defaultHash && id != g_snd.defaultHash )
            Com_PrintError(9, "Missing radverb %s\n", preset);
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_ENVIRONMENT_EFFECTS;
            cmd->context.play.alias = priority;
            cmd->context.play.fadeTimeMs = id;
            cmd->context.play.attenuation = drylevel;
            cmd->context.set_listener.origin[0] = wetlevel;
            cmd->context.play.notify = fademsec;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_DeactivateEnvironmentEffects(unsigned int priority, int fademsec)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_DEACTIVATE_ENVIRONMENT_EFFECTS;
            cmd->context.play.alias = priority;
            cmd->context.play.fadeTimeMs = fademsec;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackAttenuation(unsigned int id, float attenuation)
{
    snd_command *cmd; // [esp+4h] [ebp-4h]

    if ( SND_Active() )
    {
        if ( (LODWORD(attenuation) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        486,
                        0,
                        "%s",
                        "!IS_NAN(attenuation)") )
        {
            __debugbreak();
        }
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_ATTENUATION;
            cmd->context.play.alias = id;
            cmd->context.set_playback_attenuation.attenuation = attenuation;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackAttenuationRate(unsigned int id, float rate)
{
    snd_command *cmd; // [esp+4h] [ebp-4h]

    if ( SND_Active() )
    {
        if ( (LODWORD(rate) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        511,
                        0,
                        "%s",
                        "!IS_NAN(rate)") )
        {
            __debugbreak();
        }
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_ATTENUATION_RATE;
            cmd->context.play.alias = id;
            cmd->context.set_playback_attenuation.attenuation = rate;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackPitch(unsigned int playbackId, float pitch)
{
    snd_command *cmd; // [esp+4h] [ebp-4h]

    if ( SND_Active() )
    {
        if ( (LODWORD(pitch) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        536,
                        0,
                        "%s",
                        "!IS_NAN(pitch)") )
        {
            __debugbreak();
        }
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_PITCH;
            cmd->context.play.alias = playbackId;
            cmd->context.set_playback_attenuation.attenuation = pitch;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetPlaybackPitchRate(unsigned int playbackId, float rate)
{
    snd_command *cmd; // [esp+4h] [ebp-4h]

    if ( SND_Active() )
    {
        if ( (LODWORD(rate) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        561,
                        0,
                        "%s",
                        "!IS_NAN(rate)") )
        {
            __debugbreak();
        }
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_PLAYBACK_PITCH_RATE;
            cmd->context.play.alias = playbackId;
            cmd->context.set_playback_attenuation.attenuation = rate;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopPlayback(unsigned int playbackId)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_PLAYBACK;
            cmd->context.play.alias = playbackId;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetSnapshot(snd_snapshot_type type, const char *snapshotName, float length, float amount)
{
    snd_command *cmd; // [esp+4h] [ebp-8h]
    unsigned int id; // [esp+8h] [ebp-4h]

    if ( SND_Active() )
    {
        if ( !snapshotName || !*snapshotName )
            snapshotName = "default";
        if ( (LODWORD(length) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        613,
                        0,
                        "%s",
                        "!IS_NAN(length)") )
        {
            __debugbreak();
        }
        id = SND_HashName(snapshotName);
        if ( !SND_GetSnapshotById(id) )
        {
            Com_PrintError(9, "Could not find group snapshot %s\n", snapshotName);
            id = g_snd.defaultHash;
        }
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SNAPSHOT;
            cmd->context.play.alias = type;
            cmd->context.play.fadeTimeMs = id;
            cmd->context.play.attenuation = length;
            cmd->context.set_listener.origin[0] = amount;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetEntState(SndEntHandle handle)
{
    snd_command *cmd; // [esp+4h] [ebp-4h]

    if ( SND_Active()
        && (((unsigned int)handle.handle >> 21) & 1) == 0
        && (*(_WORD *)&handle.field & 0xFFF) != 0xFFF
        && CL_LocalClient_IsCUIFlagSet(((unsigned int)handle.handle >> 19) & 3, 32)
        && CG_SoundEntInUse(handle)
        && (((unsigned int)handle.handle >> 12) & 0x7F) == (CG_SoundGetUseCount(handle) & 0x7F) )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_ENT_STATE;
            cmd->context.stop_alias.ent = handle;
            CG_GetSoundEntityOrientation(
                handle,
                &cmd->context.set_playback_attenuation.attenuation,
                (float (*)[3])&cmd->context.play.position[2],
                &cmd->context.set_listener.origin[1]);
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_PlayLoopAt(unsigned int id, const float *origin)
{
    snd_command *cmd; // [esp+10h] [ebp-4h]

    if ( SND_Active() )
    {
        nanassertvec3(origin);

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_PLAY_LOOP_AT;
            cmd->context.play.alias = id;
            cmd->context.set_playback_attenuation.attenuation = *origin;
            cmd->context.play.attenuation = origin[1];
            cmd->context.set_listener.origin[0] = origin[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopLoopAt(unsigned int id, const float *origin)
{
    snd_command *cmd; // [esp+10h] [ebp-4h]

    if ( SND_Active() )
    {
        nanassertvec3(origin);

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_LOOP_AT;
            cmd->context.play.alias = id;
            cmd->context.set_playback_attenuation.attenuation = *origin;
            cmd->context.play.attenuation = origin[1];
            cmd->context.set_listener.origin[0] = origin[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_PlayLineAt(unsigned int id, const float *origin0, const float *origin1)
{
    snd_command *cmd; // [esp+20h] [ebp-4h]

    if ( SND_Active() )
    {
        nanassertvec3(origin0);
        nanassertvec3(origin1);

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_PLAY_LINE_AT;
            cmd->context.play.alias = id;
            cmd->context.set_playback_attenuation.attenuation = *origin0;
            cmd->context.play.attenuation = origin0[1];
            cmd->context.set_listener.origin[0] = origin0[2];
            cmd->context.set_listener.origin[1] = *origin1;
            cmd->context.play.position[0] = origin1[1];
            cmd->context.play.position[1] = origin1[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_StopLineAt(unsigned int id, const float *origin0, const float *origin1)
{
    snd_command *cmd; // [esp+20h] [ebp-4h]

    if ( SND_Active() )
    {
        nanassertvec3(origin0);
        nanassertvec3(origin1);

        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_STOP_LINE_AT;
            cmd->context.play.alias = id;
            cmd->context.set_playback_attenuation.attenuation = *origin0;
            cmd->context.play.attenuation = origin0[1];
            cmd->context.set_listener.origin[0] = origin0[2];
            cmd->context.set_listener.origin[1] = *origin1;
            cmd->context.play.position[0] = origin1[1];
            cmd->context.play.position[1] = origin1[2];
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_SetContext(const char *type, const char *value)
{
    unsigned int v2; // eax
    unsigned int v3; // eax
    unsigned int v4; // [esp-4h] [ebp-8h]
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        v2 = SND_HashName(type);
        if ( (SND_FindContextIndex(v2) & 0x80000000) == 0 )
        {
            if ( SND_HashName(value)
                && (v4 = SND_HashName(value), v3 = SND_HashName(type), SND_FindContextValueIndex(v3, v4) < 0) )
            {
                Com_PrintError(9, "Unknown sound context %s\n", value);
            }
            else
            {
                cmd = SND_GetNewCommand();
                if ( cmd )
                {
                    cmd->type = SND_COMMAND_SET_CONTEXT;
                    cmd->context.play.alias = SND_HashName(type);
                    cmd->context.play.fadeTimeMs = SND_HashName(value);
                    SND_CommandPush(cmd);
                }
            }
        }
        else
        {
            Com_PrintError(9, "Unknown sound context %s\n", type);
        }
    }
}

void __cdecl SND_SetScriptTimescale(float value)
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SCRIPT_TIMESCALE;
            cmd->context.notify_cinematic_start.volume = value;
            SND_CommandPush(cmd);
        }
    }
}

snd_ent_state *__cdecl SND_FindEntState(SndEntHandle handle, bool createNew)
{
    unsigned int k; // [esp+0h] [ebp-10h]
    unsigned int j; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    snd_ent_state *state; // [esp+Ch] [ebp-4h]

    if ((*(_WORD *)&handle.field & 0xFFF) == 0xFFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
            842,
            0,
            "%s",
            "handle.field.entIndex != SND_ENT_NONE"))
    {
        __debugbreak();
    }
    if ((((unsigned int)handle.handle >> 21) & 1) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
            843,
            0,
            "%s",
            "!handle.field.isStationary"))
    {
        __debugbreak();
    }
    if ((*(_WORD *)&handle.field & 0xFFFu) >= 0x600
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
            844,
            0,
            "handle.field.entIndex doesn't index ARRAY_COUNT(g_snd.entStateIndex)\n\t%i not in [0, %i)",
            *(_WORD *)&handle.field & 0xFFF,
            1536))
    {
        __debugbreak();
    }
    if ((((unsigned int)handle.handle >> 21) & 1) != 0
        || (*(_WORD *)&handle.field & 0xFFF) == 0xFFF
        || (*(_WORD *)&handle.field & 0xFFF) == 0xFFE)
    {
        return 0;
    }
    if ((*(_WORD *)&handle.field & 0xFFFu) >= 0x600
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
            854,
            0,
            "handle.field.entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
            *(_WORD *)&handle.field & 0xFFF,
            1536))
    {
        __debugbreak();
    }
    if ((*(_WORD *)&handle.field & 0xFFFu) > 0x600)
        Com_Error(ERR_DROP, "Invalid sound handle");
    state = g_snd.entStateIndex[*(_WORD *)&handle.field & 0xFFF];
    for (i = 0; state && handle.handle != state->handle.handle && i < 0x64; ++i)
    {
        if (i >= 0x32
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp", 867, 0, "%s", "i<50"))
        {
            __debugbreak();
        }
        if (state == state->next
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                868,
                0,
                "%s",
                "state != state->next"))
        {
            __debugbreak();
        }
        if ((*(_WORD *)&handle.field & 0xFFF) != (state->handle.handle & 0xFFF)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                869,
                0,
                "%s",
                "handle.field.entIndex == state->handle.field.entIndex"))
        {
            __debugbreak();
        }
        if (!state->lastUsed
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                870,
                0,
                "%s",
                "state->lastUsed"))
        {
            __debugbreak();
        }
        state = state->next;
    }
    if (createNew && !state)
    {
        for (j = 0; j < 0x128; ++j)
        {
            if (!g_snd.entState[j].lastUsed)
            {
                state = &g_snd.entState[j];
                if (state == g_snd.entStateIndex[*(_WORD *)&handle.field & 0xFFF]
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        882,
                        0,
                        "%s",
                        "state != g_snd.entStateIndex[handle.field.entIndex]"))
                {
                    __debugbreak();
                }
                state->lastUsed = g_snd.frame;
                state->handle = handle;
                state->next = g_snd.entStateIndex[*(_WORD *)&handle.field & 0xFFF];
                g_snd.entStateIndex[*(_WORD *)&handle.field & 0xFFF] = state;
                break;
            }
        }
        if (!state)
        {
            Com_Printf(9, "Out of ent state cache entries at time %d\n", g_snd.frame);
            for (k = 0; k < 0x128; ++k)
                Com_Printf(
                    9,
                    "%d - %x %d %d\n",
                    k,
                    g_snd.entState[k].handle.handle,
                    g_snd.entState[k].lastUsed,
                    g_snd.entState[k].next - g_snd.entState);
        }
        if (!state
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp", 900, 0, "%s", "state"))
        {
            __debugbreak();
        }
    }
    return state;
}

void __cdecl SND_UpdateEntState(
                SndEntHandle handle,
                const float *origin,
                const float *velocity,
                const float (*orientation)[3])
{
    snd_ent_state *state; // [esp+44h] [ebp-4h]

    if ( (((unsigned int)handle.handle >> 21) & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    914,
                    0,
                    "%s",
                    "!handle.field.isStationary") )
    {
        __debugbreak();
    }
    nanassertvec3(origin);
    nanassertvec3(velocity);
    nanassertvec3(orientation[0]);
    nanassertvec3(orientation[1]);
    nanassertvec3(orientation[2]);

    state = SND_FindEntState(handle, 1);
    if ( state )
    {
        if ( state->handle.handle != handle.handle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                        925,
                        0,
                        "%s",
                        "state->handle.handle == handle.handle") )
        {
            __debugbreak();
        }
        state->origin[0] = *origin;
        state->origin[1] = origin[1];
        state->origin[2] = origin[2];
        state->velocity[0] = *velocity;
        state->velocity[1] = velocity[1];
        state->velocity[2] = velocity[2];
        AxisCopy(orientation, state->orientation);
    }
}

char __cdecl SND_GetEntState(SndEntHandle handle, float *origin, float *velocity, float (*orientation)[3])
{
    snd_ent_state *state; // [esp+8h] [ebp-4h]

    state = SND_FindEntState(handle, 0);
    if ( state )
    {
        *origin = state->origin[0];
        origin[1] = state->origin[1];
        origin[2] = state->origin[2];
        *velocity = state->velocity[0];
        velocity[1] = state->velocity[1];
        velocity[2] = state->velocity[2];
        AxisCopy(state->orientation, orientation);
        return 1;
    }
    else
    {
        SND_EntStateRequest(handle);
        return 0;
    }
}

void __cdecl SNDL_SetEntState(
                SndEntHandle handle,
                const float *origin,
                const float *velocity,
                const float (*orientation)[3])
{
    SND_UpdateEntState(handle, origin, velocity, orientation);
}

void __cdecl SND_EntStateFrame()
{
    unsigned int n; // [esp+8h] [ebp-28h]
    bool removed; // [esp+Fh] [ebp-21h]
    snd_ent_state **list; // [esp+10h] [ebp-20h]
    snd_ent_state *v4; // [esp+14h] [ebp-1Ch]
    unsigned int m; // [esp+18h] [ebp-18h]
    snd_ent_state *EntState; // [esp+1Ch] [ebp-14h]
    unsigned int k; // [esp+20h] [ebp-10h]
    unsigned int j; // [esp+24h] [ebp-Ch]
    snd_ent_state *state; // [esp+28h] [ebp-8h]
    unsigned int i; // [esp+2Ch] [ebp-4h]

    PROF_SCOPED("SND_EntStateFrame");

    for (i = 0; i < 0x600; ++i)
    {
        state = g_snd.entStateIndex[i];
        for (j = 0; state && j < 0x64; ++j)
        {
            if (j >= 0x32
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp", 979, 0, "%s", "j<50"))
            {
                __debugbreak();
            }
            if (i != (state->handle.handle & 0xFFF)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    980,
                    0,
                    "%s",
                    "i == state->handle.field.entIndex"))
            {
                __debugbreak();
            }
            if (state == state->next
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    981,
                    0,
                    "%s",
                    "state != state->next"))
            {
                __debugbreak();
            }
            if (!state->lastUsed
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    982,
                    0,
                    "%s",
                    "state->lastUsed"))
            {
                __debugbreak();
            }
            state = state->next;
        }
    }
    for (k = 0; k < 0x4A; ++k)
    {
        if (g_snd.voiceAliasHash[k])
        {
            if ((((unsigned int)g_snd.voice[k].sndEnt.handle >> 21) & 1) == 0
                && (g_snd.voice[k].sndEnt.handle & 0xFFF) != 0xFFF)
            {
                EntState = SND_FindEntState(g_snd.voice[k].sndEnt, 0);
                if (EntState)
                {
                    if (EntState->lastUsed != g_snd.frame)
                        SND_EntStateRequest(g_snd.voice[k].sndEnt);
                    EntState->lastUsed = g_snd.frame;
                }
            }
        }
    }
    for (m = 0; m < 0x128; ++m)
    {
        v4 = &g_snd.entState[m];
        if (v4->lastUsed && g_snd.frame - v4->lastUsed > 2)
        {
            if (g_snd.frame < v4->lastUsed
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    1014,
                    0,
                    "%s",
                    "g_snd.frame >= state->lastUsed"))
            {
                __debugbreak();
            }
            if ((v4->handle.handle & 0xFFFu) >= 0x600
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                    1015,
                    0,
                    "state->handle.field.entIndex doesn't index ARRAY_COUNT(g_snd.entStateIndex)\n\t%i not in [0, %i)",
                    v4->handle.handle & 0xFFF,
                    1536))
            {
                __debugbreak();
            }
            list = &g_snd.entStateIndex[v4->handle.handle & 0xFFF];
            removed = 0;
            for (n = 0; *list && n < 0x64; ++n)
            {
                if (*list == v4)
                {
                    *list = (*list)->next;
                    removed = 1;
                    break;
                }
                list = &(*list)->next;
            }
            if (!removed
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp", 1033, 0, "%s", "removed"))
            {
                __debugbreak();
            }
            v4->lastUsed = 0;
            v4->next = 0;
        }
    }
}

void __cdecl SND_ResetEntState()
{
    unsigned int j; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 0x128; ++i)
        g_snd.entState[i].lastUsed = 0;
    for (j = 0; j < 0x600; ++j)
        g_snd.entStateIndex[j] = 0;
}

void __cdecl SND_EntStateRequest(SndEntHandle handle)
{
    snd_notify *cmd; // [esp+0h] [ebp-4h]

    cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_ENT_UPDATE;
        cmd->context.ent_update.handle = handle;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_SubtitleNotify(const char *subtitle, unsigned int lengthMs)
{
    snd_notify *cmd; // [esp+0h] [ebp-4h]

    cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_SUBTITLE;
        cmd->context.length.ent = (unsigned int)subtitle;
        cmd->context.ent_update.handle.handle = lengthMs;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_LengthNotify(unsigned int ent, unsigned int lengthMs)
{
    snd_notify *cmd; // [esp+0h] [ebp-4h]

    cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_LENGTH;
        cmd->context.length.ent = ent;
        cmd->context.ent_update.handle.handle = lengthMs;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_FreePlaybackNotify(snd_playback *playback)
{
    snd_notify *cmd; // [esp+0h] [ebp-4h]

    if ( !playback
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp", 1130, 0, "%s", "playback") )
    {
        __debugbreak();
    }
    cmd = SND_GetNewNotify();
    if ( cmd )
    {
        cmd->type = SND_NOTIFY_PLAYBACK_FREE;
        cmd->context.ent_update.handle.handle = (int)playback;
        SND_NotifyPush(cmd);
    }
}

void __cdecl SND_ResetPlaybacks()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x94; ++i )
        g_snd.playbacks[i].id = -1;
}

snd_playback *__cdecl SND_AllocatePlayback()
{
    unsigned int i; // [esp+0h] [ebp-8h]
    snd_playback *playback; // [esp+4h] [ebp-4h]

    playback = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    for (i = 0; i < 0x94; ++i)
    {
        if (g_snd.playbacks[i].id == -1)
        {
            g_snd.playbacks[i].id = SND_AcquirePlaybackId();
            g_snd.playbacks[i].attenuation = 1.0f;
            g_snd.playbacks[i].lengthMs = 0;
            g_snd.playbacks[i].playedMs = 0;
            playback = &g_snd.playbacks[i];
            ++g_snd.playbacksInUse;
            break;
        }
    }
    if (!playback)
        Com_PrintError(9, "ran out of playbacks\n");
    Sys_LeaveCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    return playback;
}

void __cdecl SND_FreePlayback(snd_playback *playback)
{
    if (playback)
    {
        if ((unsigned int)(playback - g_snd.playbacks) >= 0x94
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_public_async.cpp",
                1193,
                0,
                "playback-g_snd.playbacks doesn't index SND_PLAYBACK_COUNT\n\t%i not in [0, %i)",
                playback - g_snd.playbacks,
                148))
        {
            __debugbreak();
        }
        playback->id = -1;
        --g_snd.playbacksInUse;
    }
}

bool __cdecl SND_IsPlaying(int playbackId)
{
    return SND_FindPlayback(playbackId) != 0;
}

snd_playback *__cdecl SND_FindPlayback(int playbackId)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    snd_playback *playback; // [esp+4h] [ebp-4h]

    playback = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    for ( i = 0; i < 0x94; ++i )
    {
        if ( g_snd.playbacks[i].id == playbackId )
        {
            playback = &g_snd.playbacks[i];
            break;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_PLAYBACK_ALLOC);
    return playback;
}

int __cdecl SND_GetPlaybackTime(int playbackId)
{
    snd_playback *playback; // [esp+4h] [ebp-4h]

    playback = SND_FindPlayback(playbackId);
    if ( playback )
        return playback->playedMs;
    else
        return 0;
}

char __cdecl SND_GetKnownLength(int playbackId, int *msec)
{
    char v3; // [esp+0h] [ebp-8h]
    snd_playback *playback; // [esp+4h] [ebp-4h]

    playback = SND_FindPlayback(playbackId);
    v3 = 0;
    if ( playback )
    {
        *msec = playback->lengthMs;
        if ( playback->lengthMs )
            return 1;
    }
    return v3;
}

void SND_Update()
{
    unsigned int unused; // [esp+0h] [ebp-4h] BYREF

    //unused = (unsigned int)this;
    if ( SND_Active() )
    {
        Sys_AssistAndWaitWorkerCmdInternal(&updatesound_workerWorkerCmd);
        Sys_AddWorkerCmdInternal(&updatesound_workerWorkerCmd, (unsigned __int8 *)&unused, 0);
    }
}

void __cdecl SND_UpdateWait()
{
    if ( SND_Active() )
    {
        SND_Update();
        Sys_AssistAndWaitWorkerCmdInternal(&updatesound_workerWorkerCmd);
    }
}

int __cdecl updatesound_workerCallback(jqBatch *batch)
{
    PROF_SCOPED("updatesound_worker");
    jqLockData(batch);
    jqUnlockData(batch);
    SND_Frame();
    return 0;
}

void SND_Frame()
{
    iassert(entryCount == 0);
    _InterlockedExchangeAdd(&entryCount, 1u);
    iassert(entryCount == 1);
    
    {
        PROF_SCOPED("SND_Frame");

        SND_EntStateFrame();
        SND_CommandPump();
        SNDL_Update();
    }
    
    iassert(entryCount == 1);
    
    _InterlockedExchangeAdd(&entryCount, 0xFFFFFFFF);

    iassert(entryCount == 0);
}

void __cdecl SND_GameReset()
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_GAME_RESET;
            SND_CommandPush(cmd);
        }
        SND_UpdateWait();
    }
}

void __cdecl SND_BeginFrame(bool isMature, bool isPaused, float timescale, unsigned int cgTime, unsigned int seed)
{
    PROF_SCOPED("SND_BeginFrame"); // LWSS ADD

    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_SET_GAME_STATE;
            cmd->context.alias_name.name[1] = isMature;
            cmd->context.alias_name.name[0] = isPaused;
            cmd->context.set_playback_attenuation.attenuation = timescale;
            cmd->context.set_game_state.cg_time = cgTime;
            cmd->context.play.entHandle.handle = seed;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_EndFrame()
{
    snd_command *cmd; // [esp+0h] [ebp-4h]

    if ( SND_Active() )
    {
        SND_NotifyPump();
        cmd = SND_GetNewCommand();
        if ( cmd )
        {
            cmd->type = SND_COMMAND_UPDATE_LOOPS;
            SND_CommandPush(cmd);
        }
    }
}

void __cdecl SND_AsyncInit()
{
    SND_InitCommands();
    SNDL_GameReset();
}

