#include "cl_cin.h"
#include <gfx_d3d/r_cinematic.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <server_mp/sv_init_mp.h>
#include "client.h"
#include <client_mp/cl_cgame_mp.h>
#include <qcommon/cmd.h>

bool cin_skippable = true;

void __cdecl SCR_DrawCinematic()
{
    if ( R_Cinematic_IsNextReady() )
        R_Cinematic_StartNextPlayback();
    if ( R_Cinematic_IsFinished() )
    {
        if ( Dvar_GetBool("com_movieIsPlaying") )
            Dvar_SetBool((dvar_s *)com_movieIsPlaying, 0);
        SCR_StopCinematic();
    }
    if ( R_Cinematic_IsStarted() && !Dvar_GetBool("com_movieIsPlaying") )
        Dvar_SetBool((dvar_s *)com_movieIsPlaying, 1);
}

void __cdecl SCR_StopCinematic()
{
    const char *v0; // eax
    int clientNum; // [esp+0h] [ebp-8h]
    bool oneClientWasChanged; // [esp+7h] [ebp-1h]

    if ( cin_skippable || R_Cinematic_IsFinished() )
    {
        R_Cinematic_StopPlayback();
        oneClientWasChanged = 0;
        for ( clientNum = 0; clientNum < 1; ++clientNum )
        {
            if ( CL_GetLocalClientConnectionState(clientNum) == 1 || CL_GetLocalClientConnectionState(clientNum) == 2 )
            {
                oneClientWasChanged = 1;
                CL_SetLocalClientConnectionState(clientNum, CA_DISCONNECTED);
            }
        }
        if ( oneClientWasChanged && *(_BYTE *)nextmap->current.string )
        {
            v0 = va("%s\n", nextmap->current.string);
            Cbuf_AddText(0, v0);
            Dvar_SetString((dvar_s *)nextmap, "");
        }
    }
}

