#include "sv_offline_stats.h"
#include "sv_init_mp.h"
#include <bgame/bg_unlockable_items.h>
#include <bgame/bg_weapons_attachment.h>
#include <server/server.h>
#include <qcommon/common.h>
#include <clientscript/cscr_stringlist.h>

#include <cstring>
#include "sv_main_mp.h"
#include "sv_main_pc_mp.h"
#include <game_mp/g_main_mp.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <universal/q_parse.h>
#include <game_mp/g_utils_mp.h>
#include <qcommon/dvar_cmds.h>
#include <qcommon/com_profilemapload.h>
#include <server/sv_game.h>
#include <win32/win_net.h>
#include <live/live_steam_server.h>
#include <win32/win_shared.h>
#include <client_mp/cl_cgame_mp.h>
#include <qcommon/com_clients.h>
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_world.h>
#include <universal/com_constantconfigstrings.h>
#include <game_mp/pregame.h>
#include <client_mp/sv_client_mp.h>
#include <ik/ik.h>
#include "sv_ccmds_mp.h"
#include "sv_bot_mp.h"
#include <qcommon/com_gamemodes.h>
#include <qcommon/threads.h>
#include <stringed/stringed_hooks.h>
#include <live/live_sessions_win.h>
#include <live/live_win.h>
#include <DW/dwUtils_pc.h>
#include <client_mp/cl_main_pc_mp.h>
#include <qcommon/files.h>
#ifdef KISAK_SP
#include <gfx_d3d/r_cinematic.h>
#include <sound/snd_dvar.h>
#include <sound/snd_public_async.h>
#include <ui_mp/ui_main_mp.h>
#endif

const dvar_t *sv_gametype;
const dvar_t *sv_privateClients;
const dvar_t *sv_hostname;
const dvar_t *sv_noname;
const dvar_t *sv_geolocation;
const dvar_t *sv_maxgrouperrors;
const dvar_t *sv_ownerid;
const dvar_t *sv_numreservedslots;
const dvar_t *sv_clientSideBullets;
const dvar_t *sv_clientSideVehicles;
const dvar_t *sv_penetrationCount;
const dvar_t *sv_axis_penetrationCount;
const dvar_t *sv_allies_penetrationCount;
const dvar_t *sv_bullet_range;
const dvar_t *sv_hitFXFrustumCutoff;
const dvar_t *sv_punkbuster;
const dvar_t *sv_security;
const dvar_t *sv_ranked;
const dvar_t *ui_ranked;
const dvar_t *sv_dedicatedmaxclients;
const dvar_t *sv_maxclients;
const dvar_t *sv_maxRate;
const dvar_t *sv_minPing;
const dvar_t *sv_maxPing;
const dvar_t *sv_timeout;
const dvar_t *sv_connectTimeout;
const dvar_t *sv_floodProtect;
const dvar_t *sv_showCommands;
const dvar_t *sv_writeConfigStrings;
const dvar_t *scr_writeConfigStrings;
const dvar_t *sv_dwlsgerror;
const dvar_t *sv_allowAnonymous;
const dvar_t *sv_disableClientConsole;
const dvar_t *sv_privatePassword;
const dvar_t *sv_allowDownload;
const dvar_t *sv_iwds;
const dvar_t *sv_iwdNames;
const dvar_t *sv_referencedIwds;
const dvar_t *sv_referencedIwdNames;
const dvar_t *sv_FFCheckSums;
const dvar_t *sv_FFNames;
const dvar_t *sv_referencedFFCheckSums;
const dvar_t *sv_referencedFFNames;
const dvar_t *sv_authenticating;
const dvar_t *sv_voice;
const dvar_t *sv_voiceQuality;
const dvar_t *sv_cheats;
const dvar_t *sv_pure;
const dvar_t *rcon_password;
const dvar_t *sv_fps;
const dvar_t *sv_showPingSpam;
const dvar_t *sv_zombietime;
const dvar_t *sv_reconnectlimit;
const dvar_t *sv_padPackets;
const dvar_t *sv_allowedClan1;
const dvar_t *sv_allowedClan2;
const dvar_t *sv_packet_info;
const dvar_t *sv_showAverageBPS;
const dvar_t *sv_kickBanTime;
const dvar_t *sv_debugMessageKey;
const dvar_t *sv_debugPacketContents;
const dvar_t *sv_debugPacketContentsForClientThisFrame;
const dvar_t *sv_showHuffmanData;
const dvar_t *sv_debugConstantConfigStrings;
const dvar_t *sv_loadMyChanges;
const dvar_t *sv_debugPlayerstate;
const dvar_t *sv_debugPacketContentsQuick;
const dvar_t *sv_printMessageSize;
const dvar_t *sv_mapRotation;
const dvar_t *sv_mapRotationCurrent;
const dvar_t *sv_debugRate;
const dvar_t *sv_debugReliableCmds;
const dvar_t *nextmap;
const dvar_t *com_movieIsPlaying;
const dvar_t *sv_wwwDownload;
const dvar_t *sv_wwwBaseURL;
const dvar_t *sv_wwwDlDisconnected;
const dvar_t *sv_smp;
const dvar_t *sv_network_fps;
const dvar_t *sv_assistWorkers;
const dvar_t *sv_clientArchive;

volatile unsigned int sv_thread_owns_game;

void __cdecl SV_SetConfigstring(int index, char *val)
{
    unsigned __int16 v2; // [esp+20h] [ebp-448h]
    client_t *client; // [esp+38h] [ebp-430h]
    int chunkSize; // [esp+3Ch] [ebp-42Ch]
    int maxChunk; // [esp+40h] [ebp-428h]
    int remaining; // [esp+44h] [ebp-424h]
    char buf[1028]; // [esp+48h] [ebp-420h] BYREF
    int len; // [esp+450h] [ebp-18h]
    int overhead; // [esp+454h] [ebp-14h]
    int sent; // [esp+458h] [ebp-10h]
    int caseSensitive; // [esp+45Ch] [ebp-Ch]
    int i; // [esp+460h] [ebp-8h]
    char cmd; // [esp+467h] [ebp-1h]

    if ( (unsigned int)index >= MAX_CONFIGSTRINGS )
        Com_Error(ERR_DROP, "SV_SetConfigstring: bad index %i", index);
    if ( sv.configstrings[index] )
    {
        if ( !val )
            val = (char *)"";
        if ( strcmp(val, SL_ConvertToString(sv.configstrings[index], SCRIPTINSTANCE_SERVER)) )
        {
            SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, sv.configstrings[index]);
            caseSensitive = index < 1547;
#ifdef KISAK_SP
            caseSensitive |= index >= CS_SP_LOOKAT_TEXT;
#endif
            v2 = caseSensitive
                 ? SL_GetString_(SCRIPTINSTANCE_SERVER, val, 0, 19)
                 : SL_GetLowercaseString_(val, 0, 19, SCRIPTINSTANCE_SERVER);
            sv.configstrings[index] = v2;
            if ( SV_Loaded() || sv.restarting )
            {
                len = strlen(val);
                sprintf(buf, "%i", index);
                overhead = &buf[strlen(buf) + 1] - &buf[1] + 4;
                maxChunk = 1024 - overhead;
                i = 0;
                client = svs.clients;
                while ( i < com_maxclients->current.integer )
                {
                    if ( client->header.state >= CS_CLIENTLOADING )
                    {
                        if ( len <= maxChunk )
                        {
                            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c %i %s", 100, index, val);
                        }
                        else
                        {
                            sent = 0;
                            for ( remaining = len; remaining > 0; remaining -= chunkSize )
                            {
                                if ( sent )
                                {
                                    if ( remaining > maxChunk )
                                        cmd = 121;
                                    else
                                        cmd = 122;
                                }
                                else
                                {
                                    cmd = 120;
                                }
                                chunkSize = maxChunk;
                                while ( remaining > chunkSize && val[chunkSize + sent] == 32 )
                                {
                                    if ( !--chunkSize )
                                        Com_Error(ERR_DROP, "SV_SetConfigstring: big config string with %d empty spaces", maxChunk);
                                }
                                I_strncpyz(buf, &val[sent], chunkSize + 1);
                                SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c %i %s", cmd, index, buf);
                                sent += chunkSize;
                            }
                        }
                    }
                    ++i;
                    ++client;
                }
            }
        }
    }
    else if ( val )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp", 156, 0, "%s", "!val") )
            __debugbreak();
    }
}

void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize)
{
    char *v3; // eax

    if ( bufferSize < 1 )
        Com_Error(ERR_DROP, "SV_GetConfigstring: bufferSize == %i", bufferSize);
    if ( index >= MAX_CONFIGSTRINGS )
        Com_Error(ERR_DROP, "SV_GetConfigstring: bad index %i", index);
    if ( !sv.configstrings[index]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    247,
                    0,
                    "%s",
                    "sv.configstrings[index]") )
    {
        __debugbreak();
    }
    v3 = SL_ConvertToString(sv.configstrings[index], SCRIPTINSTANCE_SERVER);
    I_strncpyz(buffer, v3, bufferSize);
}

unsigned int __cdecl SV_GetConfigstringConst(unsigned int index)
{
    iassert((unsigned)index < MAX_CONFIGSTRINGS);
    iassert(sv.configstrings[index]);

    return sv.configstrings[index];
}

void __cdecl SV_SetConfigValueForKey(int start, int max, char *key, char *value)
{
    char *v4; // eax
    unsigned int String; // [esp+0h] [ebp-14h]
    unsigned int name; // [esp+4h] [ebp-10h]
    int i; // [esp+10h] [ebp-4h]

    if ( start < 1547 )
        String = SL_FindString(key, SCRIPTINSTANCE_SERVER);
    else
        String = SL_FindLowercaseString(key, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < max; ++i )
    {
        name = sv.configstrings[i + start];
        if ( name == sv.emptyConfigString )
        {
            SV_SetConfigstring(i + start, key);
            break;
        }
        if ( String == name )
            break;
    }
    if ( i == max )
    {
        Com_Printf(15, "Overflow at config string start value of %i: key values printed below\n", start);
        for ( i = 0; i < max; ++i )
        {
            v4 = SL_ConvertToString(sv.configstrings[i + start], SCRIPTINSTANCE_SERVER);
            Com_Printf(15, "%i: %i ( %s )\n", i + start, sv.configstrings[i + start], v4);
        }
        Com_Error(ERR_DROP, "SV_SetConfigValueForKey: overflow");
    }
    SV_SetConfigstring(i + max + start, value);
}

void __cdecl SV_SetUserinfo(int index, char *val)
{
    char *v2; // eax

    if ( index < 0 || index >= com_maxclients->current.integer )
        Com_Error(ERR_DROP, "SV_SetUserinfo: bad index %i", index);
    if ( !val )
        val = (char *)"";
    I_strncpyz(svs.clients[index].userinfo, val, 1024);
    v2 = Info_ValueForKey(val, "name");
    I_strncpyz(svs.clients[index].name, v2, 32);
}

void __cdecl SV_GetUserinfo(int index, char *buffer, int bufferSize)
{
    if ( (unsigned int)index >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    341,
                    0,
                    "index doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    index,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( bufferSize < 1 )
        Com_Error(ERR_DROP, "SV_GetUserinfo: bufferSize == %i", bufferSize);
    if ( index < 0 || index >= com_maxclients->current.integer )
        Com_Error(ERR_DROP, "SV_GetUserinfo: bad index %i", index);
    I_strncpyz(buffer, svs.clients[index].userinfo, bufferSize);
}

void __cdecl SV_CreateBaseline()
{
    float *absmax; // [esp+8h] [ebp-1Ch]
    float *absmin; // [esp+10h] [ebp-14h]
    gentity_s *svent; // [esp+1Ch] [ebp-8h]
    int entnum; // [esp+20h] [ebp-4h]

    for (entnum = 1; entnum < sv.num_entities; ++entnum)
    {
        svent = (gentity_s *)((char *)sv.gentities + entnum * sv.gentitySize);
        if (svent->r.linked && svent->s.eType != 14 && svent->s.eType != 17)
        {
            svent->s.number = entnum;
            memcpy(&sv.svEntities[entnum].baseline, svent, 0xE0u);
            sv.svEntities[entnum].baseline.r.svFlags = svent->r.svFlags;
            sv.svEntities[entnum].baseline.r.clientMask[0] = svent->r.clientMask[0];
            absmin = sv.svEntities[entnum].baseline.r.absmin;
            *absmin = svent->r.absmin[0];
            absmin[1] = svent->r.absmin[1];
            absmin[2] = svent->r.absmin[2];
            absmax = sv.svEntities[entnum].baseline.r.absmax;
            *absmax = svent->r.absmax[0];
            absmax[1] = svent->r.absmax[1];
            absmax[2] = svent->r.absmax[2];
            if (svent->s.clientNum >= 0x20u)
                svent->s.clientNum = 32;
        }
    }
}

void __cdecl SV_SetXUIDConfigStrings()
{
    char *v0; // eax
    int index; // [esp+0h] [ebp-4Ch]
    int j; // [esp+4h] [ebp-48h]
    char filteredName[32]; // [esp+8h] [ebp-44h] BYREF
    client_t *client; // [esp+28h] [ebp-24h]
    int i; // [esp+2Ch] [ebp-20h]
    char xuidStr[20]; // [esp+30h] [ebp-1Ch] BYREF
    int xuidCount; // [esp+48h] [ebp-4h]

    xuidCount = 0;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        client = &svs.clients[i];
        if ( client->header.state == CS_ACTIVE )
        {
            XUIDToString(&client->dw_userID, xuidStr);
            I_strncpyz(filteredName, client->name, 32);
            for ( j = 0; filteredName[j] && j < 32; ++j )
            {
                if ( filteredName[j] == 32 )
                    filteredName[j] = 1;
            }
            index = xuidCount + 323;
            v0 = va("%s %s", xuidStr, filteredName);
            SV_SetConfigstring(index, v0);
            ++xuidCount;
        }
    }
}

void __cdecl SV_Startup(int controllerIndex)
{
    iassert(!svs.initialized);

    if (IsDedicatedServer())
    {
        SV_ResetDWState();
        Dvar_SetBoolByName("r_gfxopt_water_simulation", 0);

#ifdef KISAK_LIVE
        dwNetStart(1);
        while (g_dwNetStatus == DW_NET_STARTING_ONLINE)
            dwNetPump();

        if (g_svdedicatedauthstate != SV_DWAUTHORIZED)
        {
            DW_DedicatedLogonStart(controllerIndex);
            while (g_svdedicatedauthstate == SV_DWAUTHORIZING)
                DW_DedicatedLogonComplete(0);
            if (g_svdedicatedauthstate != SV_DWAUTHORIZED)
                Com_Error(ERR_DROP, "Dedicated server authentication failure.\n");
            Com_Printf(0, "should be logged in ok\n");
        }
#endif
    }

    //BLOPS_NULLSUB();

    iassert(com_maxclients->current.integer <= 32);
    
    if ( g_entsInSnapshot )
        Dvar_SetInt((dvar_s *)g_entsInSnapshot, 1024);

    svs.initialized = 1;

    Dvar_SetBool((dvar_s *)com_sv_running, 1);
}

void __cdecl SV_SetExpectedHunkUsage(char *mapname)
{
    int handle; // [esp+0h] [ebp-18h] BYREF
    const char *memlistfile; // [esp+4h] [ebp-14h]
    char *buf; // [esp+8h] [ebp-10h]
    int len; // [esp+Ch] [ebp-Ch]
    const char *token; // [esp+10h] [ebp-8h]
    const char *buftrav; // [esp+14h] [ebp-4h] BYREF

    memlistfile = "hunkusage.dat";
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", &handle, FS_READ);
    if ( len >= 0 )
    {
        buf = (char *)Z_Malloc(len + 1, "SV_SetExpectedHunkUsage", 11);
        memset((unsigned __int8 *)buf, 0, len + 1);
        FS_Read((unsigned __int8 *)buf, len, handle);
        FS_FCloseFile(handle);
        buftrav = buf;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&buftrav);
            if ( !token
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp", 724, 0, "%s", "token") )
            {
                __debugbreak();
            }
            if ( !*token )
                break;
            if ( !I_stricmp(token, mapname) )
            {
                token = (const char *)Com_Parse(&buftrav);
                if ( token )
                {
                    if ( *token )
                    {
                        com_expectedHunkUsage = atoi(token);
                        Z_Free(buf, 11);
                        return;
                    }
                }
            }
        }
        Z_Free(buf, 11);
    }
}

void __cdecl SV_ClearServer()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
        if ( sv.configstrings[i] )
            SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, sv.configstrings[i]);
    }
    if ( sv.emptyConfigString )
        SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, sv.emptyConfigString);
    G_ClearCachedModels();
    Com_Memset(&sv, 0, sizeof(server_t));
    com_inServerFrame = 0;
}

void __cdecl SV_InitArchivedSnapshot()
{
    svs.nextArchivedSnapshotFrames = 0;
    svs.nextArchivedSnapshotBuffer = 0;
    svs.nextCachedSnapshotEntities = 0;
    svs.nextCachedSnapshotClients = 0;
    svs.nextCachedSnapshotFrames = 0;
    svs.nextCachedSnapshotMatchStates = 0;
}

void __cdecl SV_SetSystemInfoConfig()
{
    dvar_modifiedFlags &= ~8u;
}

void __cdecl SV_SaveSystemInfo()
{
    char *v0; // eax

    SV_SetSystemInfoConfig();
    v0 = Dvar_InfoString(0, 4);
    SV_SetConfigstring(0, v0);
    dvar_modifiedFlags &= ~4u;
    SV_SetConfig(23, 150, 256);
    dvar_modifiedFlags &= ~0x100u;
    sv.state = SS_GAME;
    sv.restarting = 0;
}

void __cdecl SV_SetServerDvarsBeforeScriptsInit()
{
    int LicenseType; // eax
    int v1; // eax

    LicenseType = SV_GetLicenseType();
    Dvar_SetInt((dvar_s *)sv_ranked, LicenseType);
    v1 = SV_GetLicenseType();
    if ( SV_IsServerRanked(v1) )
        Dvar_SetBoolByName("g_allowvote", 0);
}

void __cdecl    SV_SpawnServer(int controllerIndex, char *server, int mapIsPreloaded, int savegame)
{
    //jpeg_decompress_struct *v10; // [esp+0h] [ebp-84h]
    unsigned int bspVersion; // [esp+1Ch] [ebp-68h]
    client_t *client; // [esp+20h] [ebp-64h]
    char filename[68]; // [esp+24h] [ebp-60h] BYREF
    int checksum; // [esp+6Ch] [ebp-18h] BYREF
    int party; // [esp+70h] [ebp-14h]
    int savepersist; // [esp+74h] [ebp-10h]
    int i; // [esp+80h] [ebp-4h]

    Com_SyncThreads();
#ifdef KISAK_LIVE
    MatchRecord_InitMatchData();
#endif
    iassert(SV_GetServerThreadOwnsGame() == 0);

    if ( useFastFile->current.enabled && !mapIsPreloaded )
    {
        DB_AddUserMapDir(server);
        FS_DisablePureCheck(1);
        Com_LoadMapLoadingScreenFastFile(server);
    }

    if (!mapIsPreloaded && !IsDedicatedServer())
    {
        CL_SetupClientsForIngame();
    }

    CL_AllocatePerLocalClientMemory();
    Scr_ParseGameTypeList();
    SV_SetGametype();

    if ( !mapIsPreloaded && !IsDedicatedServer() )
        CL_InitLoad(server, sv_gametype->current.string);

    if ( useFastFile->current.enabled && !mapIsPreloaded )
        DB_SyncXAssets();

    R_BeginRemoteScreenUpdate();

    if ( fs_debug->current.integer == 2 )
        Dvar_SetInt((dvar_s*)fs_debug, 0);

    ProfLoad_Activate();

    iassert(SV_GetServerThreadOwnsGame() == 0);

    if ( com_sv_running->current.enabled )
    {
        savepersist = G_GetSavePersist();
        i = 0;
        client = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( client->header.state >= CS_CLIENTLOADING )
            {
                Com_sprintf(filename, 64, "loadingnewmap\n%s\n%s", server, sv_gametype->current.string);
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, filename);
            }
            ++i;
            ++client;
        }
        NET_Sleep(250);
    }
    else
    {
        savepersist = 0;
    }

    iassert(!strstr(server, "\\"));

    Dvar_SetString((dvar_s*)sv_mapname, server);
    LiveSteam_Server_Init();
    R_EndRemoteScreenUpdate(0);

    if ( !mapIsPreloaded && !IsDedicatedServer() )
    {
        CL_MapLoading(server);
        R_BeginRemoteScreenUpdate();
        R_EndRemoteScreenUpdate(0);
        CL_ShutdownAll();
#ifdef KISAK_SP
        // Retail SP 0x0050F2A9 -> 0x0046BF30 -> 0x004E27D0/0x00882B00. The
        // cinematic owner runs after CL_ShutdownAll, is suppressed for tool/savegame/networked
        // paths, maps every menu level to the frontend selector, and owns the final audio fade.
        // KISAK_NX: never -- the Bink load movie the solo path plays has always
        // been trouble in this decomp (a black screen here); the online/co-op
        // path, which skips it, shows the map's loadscreen_<map> image
        // (code_post_gfx) instead.
#ifdef KISAK_NX
        if ( false )
#else
        if ( !G_ExitAfterToolComplete()
            && !savegame
            && !onlinegame->current.enabled
            && !Dvar_GetBool("systemlink") )
#endif
        {
            const float menuMaster = snd_menu_master->current.value;
            const float menuCinematic = snd_menu_cinematic->current.value;
            const float volume = menuMaster * menuMaster * menuCinematic * menuCinematic;
            CL_MapLoading_StartCinematic(Com_IsMenuLevel(server) ? "frontend" : server, volume);
        }
        SND_FadeOut();
#endif
    }

    SV_ShutdownGameProgs();
    Com_Printf(15, "------ Server Initialization ------\n");
    Com_Printf(15, "Server: %s\n", server);
    SV_ClearServer();

    if ( !useFastFile->current.enabled )
    {
        FS_Shutdown();
        FS_ClearIwdReferences();
    }
    if ( !mapIsPreloaded )
        Com_Restart();

    if (com_sv_running->current.enabled)
    {
        //BLOPS_NULLSUB(v10);
    }
    else
    {
        SV_Startup(controllerIndex);
    }

    Dvar_ClearModified(com_maxclients);
    I_strncpyz(sv.gametype, sv_gametype->current.string, 64);
    G_srand(Sys_MillisecondsRaw());
    sv.checksumFeed = Sys_Milliseconds() ^ (G_rand() ^ (G_rand() << 16));
    FS_Restart(0, sv.checksumFeed);

    if ( !useFastFile->current.enabled )
    {
        Com_GetBspFilename(filename, 64, server);
        SV_SetExpectedHunkUsage(filename);
    }

    if ( !mapIsPreloaded )
    {
        ProfLoad_Begin("start loading client");
        if (!IsDedicatedServer())
            CL_StartLoading();
        ProfLoad_End();
        if ( useFastFile->current.enabled )
        {
#ifdef KISAK_SP
            // SP-only, Ghidra 0x0050F3F6..0x0050F41C (SV_SpawnServer, retail 0x0050F030): the
            // first of two retail call sites restoring the briefing/loading menu around the level
            // fastfile load (docs/SP_MAIN_MENU_BOOTCHAIN.md Sec.2 row 4). Guard confirmed live by
            // decompiling this function directly: retail wraps this whole block in
            // `*(char*)(DAT_0247fec8 + 0x18) != 0`, which is exactly this reconstruction's
            // `useFastFile->current.enabled` (the condition already wrapping this block), so no
            // extra guard is needed for Com_UnloadFrontEnd/Com_LoadLevelFastFiles themselves. The
            // inner cinematic/menu activation is separately gated on retail's third formal
            // parameter -- Ghidra's decompiler resolves `(char)param_3 == 0` identically at BOTH
            // 0x0050F3FB and 0x0050F433 (the second site, below). That parameter maps to this
            // function's `savegame` argument: retail's own signature drops this reconstruction's
            // leading `controllerIndex` (an already-recorded divergence on this function's
            // machine-proposed-name plate), leaving (server, mapIsPreloaded, savegame)
            // positionally 1:1 against retail's (param_1, param_2, param_3); `savegame` is
            // otherwise unused anywhere in this reconstruction's SV_SpawnServer body, and both of
            // its current call sites (sv_ccmds_mp.cpp:85, :367) already pass literal 0, matching
            // the retail map/spmap/devmap/spdevmap command handler's confirmed hardcoded 0 for the
            // same argument slot (docs/SP_MP_STARTUP_AUDIT.md Sec.5.7, disassembly-confirmed).
            // <ctx> is Com_LocalClients_GetPrimary(): retail calls the unnamed FUN_005BEE40
            // (Ghidra 0x005BEE40) here, whose body is a linear scan over a 0x14-byte-stride table
            // for the first entry with flag bit 0x2 set, returning its index or -1 -- the stride
            // exactly matches sizeof(ClientGameState) (0x14) and the tested bit exactly matches
            // the "primary" bit Com_LocalClient_SetPrimary sets/clears on ClientGameState::flags,
            // so this reconstruction's existing helper is reused rather than porting FUN_005BEE40
            // fresh under a new name.
            if (!IsDedicatedServer())
                Com_UnloadFrontEnd();
            if ( !savegame && !IsDedicatedServer() )
            {
                R_Cinematic_UpdateFrame(1);
                UI_SetActiveMenuSp(Com_LocalClients_GetPrimary(), UISP_BRIEFING);
            }
#endif
            Com_LoadLevelFastFiles(server);

            iassert(sv_loadMyChanges);

            if ( sv_loadMyChanges->current.enabled )
            {
                Cbuf_ExecuteBuffer(0, Com_LocalClient_GetControllerIndex(0), (char*)"loadzone mychanges\n");
            }
        }
    }

    R_BeginRemoteScreenUpdate();
#ifdef KISAK_SP
    // SP-only, Ghidra 0x0050F425..0x0050F447 (SV_SpawnServer): the second retail call site, same
    // guard and <ctx> derivation as the first site above -- see that note for the full evidence.
    // Retail interposes one more setup call here (UI_LoadIngameMenus(0), Ghidra 0x00523090,
    // called at 0x0050F42B) between R_BeginRemoteScreenUpdate and this guard.
    // IDENTITY RESOLVED 2026-08-26 (was "unidentified"): 0x00523090 is named UI_LoadIngameMenus
    // in the live Ghidra database. Its whole body is this tree's UI_LoadIngameMenus
    // (ui_main.cpp:3939) -- the g_ingameMenusLoaded[contextIndex] once-only guard, then
    // va("%singame.txt", SP_UI_DIR) -> UI_LoadMenus(list, 3) -> UI_AddMenuList(contextIndex, dc,
    // list, 1), the same literal 3 and the same trailing 1 -- plus a SECOND identical block for
    // va("%singame_options.txt", SP_UI_DIR) inside the same guard (see the TODO on that function).
    // CAVEAT worth carrying: that symbol was written through a deliberate name-policy bypass. The
    // function carries the name-policy-blocked tag alongside openblops-source-match, and its plate
    // still opens "NOT APPLIED -- function remains FUN_*", which is now stale -- the policy refused
    // UI_LoadIngameMenus as a token-subset duplicate of UI_LoadMenus and a later pass overrode it.
    // The identity is verified (unique "%singame.txt" literal, one referrer); only the write path
    // was irregular.
    // DONE 2026-08-26: ported below. The call POSITION was re-derived from the raw listing of
    // retail SV_SpawnServer (0x0050F030) in this pass rather than taken from the note above:
    //     0050f425  CALL 0x006d7e60   ; R_BeginRemoteScreenUpdate (live Ghidra name)
    //     0050f42a  PUSH EBP          ; EBP == 0 -- zeroed at 0x0050F32D / 0x0050F33A on both
    //                                 ; incoming paths and never written again before here
    //     0050f42b  CALL 0x00523090   ; UI_LoadIngameMenus
    //     0050f430  ADD ESP,0x4
    //     0050f433  CMP byte ptr [ESP+0x78],0x0   ; the `savegame` guard immediately below
    // so the call lands between R_BeginRemoteScreenUpdate() and the !savegame guard, with a
    // literal 0 for contextIndex -- exactly the slot this block already occupies. The
    // declaration reaches this TU through <ui_mp/ui_main_mp.h> (included in the KISAK_SP block
    // at the top of this file), which includes <ui/ui_main.h> where it is declared at :364.
    if (!IsDedicatedServer())
        UI_LoadIngameMenus(0);
    if ( !savegame && !IsDedicatedServer() )
    {
        UI_SetActiveMenuSp(Com_LocalClients_GetPrimary(), UISP_BRIEFING);
    }
#endif
    sv.emptyConfigString = SL_GetString_(SCRIPTINSTANCE_SERVER, "", 0, 19);
    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
        sv.configstrings[i] = SL_GetString_(SCRIPTINSTANCE_SERVER, "", 0, 19);
    }
    Dvar_ResetScriptInfo();
    svs.nextSnapshotEntities = 0;
    svs.nextSnapshotClients = 0;
    SV_InitArchivedSnapshot();
    SV_InitSnapshot();
    svs.snapFlagServerBit ^= 4u;
    Dvar_SetString((dvar_s*)nextmap, "map_restart");
    Dvar_SetInt((dvar_s *)cl_paused, 0);
    Com_GetBspFilename(filename, 64, server);
    if ( !useFastFile->current.enabled )
        Com_LoadBsp(filename);
    if ( G_OnlyConnectingPaths() )
    {
        bspVersion = Com_GetBspVersion();
        if ( bspVersion == 45 )
            Material_SetAlwaysUseDefaultMaterial(1);
        else
            Com_Error(
                ERR_DROP,
                "Can only connect pths with bsp version %i, but the bsp is version %i.    You need to recompile the map.",
                45,
                bspVersion);
    }
    CM_LoadMap(filename, &checksum);
    Com_LoadWorld(filename);
    if ( !G_OnlyConnectingPaths() && !useFastFile->current.enabled )
        Com_UnloadBsp();
    CM_LinkWorld();
    sv_serverId_value = (unsigned __int8)(sv_serverId_value + 16);
    if ( (sv_serverId_value & 0xF0) == 0 )
        sv_serverId_value += 16;
    sv.start_frameTime = com_frameTime;
    sv.state = SS_LOADING;
    if ( !G_ExitAfterConnectPaths() && !useFastFile->current.enabled )
        Com_GetBspFilename(filename, 64, server);
    R_EndRemoteScreenUpdate(0);
#ifdef KISAK_SP
    // Com_LoadLevelFastFiles queues the level zone asynchronously.  In this
    // reconstruction the map's delayed overrides can still be pending here:
    // a traced zombie_theater run loaded the patch copy of
    // configstrings_pc_zombie_theater_zom.csv (checksum 355402927), cached that
    // value below, and only then DB_PostLoadXZone swapped in the level copy
    // (checksum 232304673).  CL_ParseGamestate looked up the level copy and
    // rejected the stale server checksum.  A full row dump proved both parsed
    // tables have 773 rows and differ only at configstrings 3 and 219, ruling
    // out parser corruption; DB xasset diagnostics proved the late
    // patch->zombie_theater override ordering.
    //
    // Synchronize the queued SP level transaction before caching its constant
    // config strings.  This is deliberately a reconstruction scheduling fix,
    // not a checksum bypass: server and client still independently load and
    // validate the same shipped table, and KISAK_MP retains its original
    // asynchronous behavior.
    if ( useFastFile->current.enabled && !mapIsPreloaded )
        DB_SyncXAssets();
#endif
    party = 1;
    if ( CCS_ShouldLoadConstConfigStrings(1) )
        CCS_LoadConstantConfigStrings(server, sv_gametype->current.string);
    else
        CCS_ClearConstantConfigStrings();
    R_BeginRemoteScreenUpdate();

    if (!IsDedicatedServer() && !G_ExitAfterToolComplete())
    {
        if (!g_serverSession.sessionHandle)
        {
            Session_Init();
            Session_StartHost(&g_serverSession, 1, Com_GetPrivateClients(), 0x20 - Com_GetPrivateClients());
        }
        else
        {
            //I_strncpyz(g_matchmakingInfo->m_membermapname, sv_mapname->current.string, 0x21);
            //g_matchmakingInfo->m_memberGAME_TYPE = Com_GametypeToInt(g_gametype->current.string);
            //Session_Modify(0, &g_serverSession, 1, Com_GetPrivateClients(), 0x20 - Com_GetPrivateClients());
        }
    }

    iassert(SV_GetServerThreadOwnsGame() == 0);

    SV_SetXUIDConfigStrings();
    Pregame_Reset();
    SV_SetServerDvarsBeforeScriptsInit();
#ifdef KISAK_MP
    SV_ApplyCustomMatchBots();
#endif

    {
        ProfLoad_Begin("Init game");
#ifndef KISAK_SP
        if (IsDedicatedServer())
        {
            // Unlockable attachment-point lists depend on this table. Clients
            // load it in BG_UnlockableItemsInit before BG_InitUnlockables.
            BG_LoadWeaponAttachmentTable();
            BG_InitUnlockables();
        }
#endif
#if defined(KISAK_DEDICATED) && defined(OPENBLOPS_NO_STEAM_AUTH)
        if (!SV_OfflineStatsBindSchema())
            Com_Error(ERR_DROP, "Offline dedicated stats schema/defaults unavailable (see console)");
#endif
        SV_InitGameProgs(savepersist);
        ProfLoad_End();
    }


    SV_CreateBaseline();
    Demo_SetDemoClientState(0);

    for (i = 0; i < com_maxclients->current.integer; ++i)
    {
        client_t *cl = &svs.clients[i];

        if (cl->header.state >= CS_CONNECTED)
        {
            if (cl->bIsDemoClient || cl->bIsTestClient)
            {
                SV_DropClient(cl, "EXE_PLAYERKICKED", false, true);
                continue;
            }

            const char *denied = ClientConnect(i, cl->scriptId);

            if (denied)
            {
                SV_DropClient(cl, denied, true, true);
                continue;
            }

            // Restore connected state
            cl->header.state = CS_CONNECTED;
        }
    }

    const char *names;
    const char *checksums;

    if (sv_pure->current.enabled)
    {
        FS_LoadedIwds(&checksums, &names);

        if (!*checksums)
            Com_PrintWarning(15, "WARNING: sv_pure set but no IWD files loaded\n");

        Dvar_SetString((dvar_s*)sv_iwds, checksums);
        Dvar_SetString((dvar_s*)sv_iwdNames, names);
    }
    else
    {
        Dvar_SetString((dvar_s *)sv_iwds, "");
        Dvar_SetString((dvar_s *)sv_iwdNames, "");
    }

    FS_ReferencedIwds(&checksums, &names);

    Dvar_SetString((dvar_s*)sv_referencedIwds, checksums);
    Dvar_SetString((dvar_s*)sv_referencedIwdNames, names);

    Dvar_SetString((dvar_s*)sv_referencedFFCheckSums, DB_ReferencedFFChecksums());
    Dvar_SetString((dvar_s*)sv_referencedFFNames, DB_ReferencedFFNameList());

    SV_SaveSystemInfo();

    if (Dvar_GetBool("playlist_enabled"))
    {
        int maxplayers = Dvar_GetInt("party_maxplayers");
        Dvar_SetIntByName("sv_maxclients", maxplayers);
    }

    SV_Heartbeat_f();

    ProfLoad_Deactivate();

    Com_Printf(15, "-----------------------------------\n");

    if (G_ExitAfterToolComplete())
        Dvar_SetBoolByName("sv_punkbuster", false);

    //DisablePbSv();

    R_EndRemoteScreenUpdate(NULL);

    if (G_OnlyConnectingPaths())
        Path_InitPaths();

}

#ifdef KISAK_SP
static int SV_CachedSnapshotEntityCount(int maxClients)
{
    // SP maps can archive every entity, independently of the player count.
    // Keep room for a full frame and its decode successor, even with one slot.
    return 1024 * (maxClients > 2 ? maxClients : 2);
}
#endif

const int ikStateSize = (int)sizeof(IKState);   // nx-port: was 3680 (x86); 5344 on LP64
unsigned __int8 *sv_ikBuf;
char *__cdecl SV_AllocateClientMemory_SizeRequired(int maxLocalClients, int maxClients)
{
    int v3; // [esp+0h] [ebp-Ch]
    int v4; // [esp+4h] [ebp-8h]

    if ( maxClients > 2 )
        v4 = maxClients;
    else
        v4 = 2;
    if ( maxClients > 2 )
        v3 = maxClients;
    else
        v3 = 2;

    // nx-port: was the x86 sizes folded into constants (0x118D00 * maxClients
    // = client_t + 32 MatchStates + 2688 entityStates, ...). The same terms as
    // SV_AllocateClientMemory, from the structs; 0x80 twice is the slack for the
    // two 128-aligned allocations.
    return (char *)((sizeof(client_t) + 32 * sizeof(MatchState) + 2688 * sizeof(entityState_s)) * maxClients
        + 32 * sizeof(clientState_s) * maxClients * maxClients
        + 512 * sizeof(cachedSnapshot_t) + 0x80
        + sizeof(MatchState) * maxClients
#ifdef KISAK_SP
        + sizeof(archivedEntity_s) * SV_CachedSnapshotEntityCount(maxClients)
#else
        + 80 * sizeof(archivedEntity_s) * maxClients
#endif
        + 0x80
        + sizeof(cachedClient_s) * v3 * v4
        + 1200 * sizeof(archivedSnapshot_s) + 0x1000000
        + 0x20 * ikStateSize);
}

void __cdecl SV_AllocateClientMemory(HunkUser *hunk, int maxLocalClients, int maxClients)
{
    int v3; // [esp+0h] [ebp-8h]
    int v4; // [esp+4h] [ebp-4h]

    svs.clients = (client_t *)Hunk_UserAlloc(hunk, sizeof(client_t) * maxClients, 4, "svs.clients");
    memset((unsigned __int8 *)svs.clients, 0, sizeof(client_t) * maxClients);
#if defined(KISAK_DEDICATED) && defined(OPENBLOPS_NO_STEAM_AUTH)
    SV_OfflineStatsResetAll();
#endif
    svs.numSnapshotMatchStates = 32 * maxClients;
    svs.snapshotMatchStates = (MatchState *)Hunk_UserAlloc(hunk, sizeof(MatchState) * svs.numSnapshotMatchStates, 4, "svs.snapshotMatchStates");
    memset((unsigned __int8 *)svs.snapshotMatchStates, 0, sizeof(MatchState) * svs.numSnapshotMatchStates);
    svs.numSnapshotEntities = 2688 * maxClients;
    svs.snapshotEntities = (entityState_s *)Hunk_UserAlloc(hunk, sizeof(entityState_s) * svs.numSnapshotEntities, 4, "svs.snapshotEntities");
    memset((unsigned __int8 *)svs.snapshotEntities, 0, sizeof(entityState_s) * svs.numSnapshotEntities);
    svs.numSnapshotClients = 32 * maxClients * maxClients;
    svs.snapshotClients = (clientState_s *)Hunk_UserAlloc(hunk, sizeof(clientState_s) * svs.numSnapshotClients, 4, "svs.snapshotClients");
    memset((unsigned __int8 *)svs.snapshotClients, 0, sizeof(clientState_s) * svs.numSnapshotClients);
    svs.cachedSnapshotFrames = (cachedSnapshot_t *)Hunk_UserAlloc(hunk, 512 * sizeof(cachedSnapshot_t), 128, "svs.cachedSnapshotFrames");
    memset((unsigned __int8 *)svs.cachedSnapshotFrames, 0, 512 * sizeof(cachedSnapshot_t));
    svs.numCachedSnapshotMatchStates = maxClients;
    svs.cachedSnapshotMatchStates = (MatchState *)Hunk_UserAlloc(
                                                                                                    hunk,
                                                                                                    sizeof(MatchState) * maxClients,
                                                                                                    4,
                                                                                                    "svs.cachedSnapshotMatchStates");
    memset((unsigned __int8 *)svs.cachedSnapshotMatchStates, 0, sizeof(MatchState) * svs.numCachedSnapshotMatchStates);
#ifdef KISAK_SP
    svs.numCachedSnapshotEntities = SV_CachedSnapshotEntityCount(maxClients);
#else
    svs.numCachedSnapshotEntities = 80 * maxClients;
#endif
    svs.cachedSnapshotEntities = (archivedEntity_s *)Hunk_UserAlloc(
                                                                                                         hunk,
                                                                                                         sizeof(archivedEntity_s) * svs.numCachedSnapshotEntities,
                                                                                                         128,
                                                                                                         "svs.cachedSnapshotEntities");
    memset((unsigned __int8 *)svs.cachedSnapshotEntities, 0, sizeof(archivedEntity_s) * svs.numCachedSnapshotEntities);
    if ( maxClients > 2 )
        v4 = maxClients;
    else
        v4 = 2;
    if ( maxClients > 2 )
        v3 = maxClients;
    else
        v3 = 2;
    svs.numCachedSnapshotClients = v3 * v4;
    svs.cachedSnapshotClients = (cachedClient_s *)Hunk_UserAlloc(hunk, sizeof(cachedClient_s) * v3 * v4, 4, "svs.cachedSnapshotClients");
    memset((unsigned __int8 *)svs.cachedSnapshotClients, 0, sizeof(cachedClient_s) * svs.numCachedSnapshotClients);
    svs.archivedSnapshotFrames = (archivedSnapshot_s *)Hunk_UserAlloc(hunk, 1200 * sizeof(archivedSnapshot_s), 4, "svs.archivedSnapshotFrames");
    memset((unsigned __int8 *)svs.archivedSnapshotFrames, 0, 1200 * sizeof(archivedSnapshot_s));   // nx-port: sizes above were x86 literals
    svs.archivedSnapshotBuffer = (unsigned __int8 *)Hunk_UserAlloc(hunk, 0x1000000, 4, "svs.archivedSnapshotBuffer");
    memset(svs.archivedSnapshotBuffer, 0, 0x1000000);
    sv_ikBuf = (unsigned __int8 *)Hunk_UserAlloc(hunk, 32 * ikStateSize, 16, "sv_ikStatesArray");
    memset(sv_ikBuf, 0, 32 * ikStateSize);
    IK_AllocateLocalClientMemory(sv_ikBuf, -1);
}

void __cdecl SV_FreeClientMemory(HunkUser *hunk)
{
#if defined(KISAK_DEDICATED) && defined(OPENBLOPS_NO_STEAM_AUTH)
    SV_OfflineStatsResetAll();
#endif
    if ( sv_ikBuf )
    {
        Hunk_UserFree(hunk, sv_ikBuf);
        IK_AllocateLocalClientMemory(0, -1);
    }
    sv_ikBuf = 0;
    if ( svs.clients )
        Hunk_UserFree(hunk, svs.clients);
    svs.clients = 0;
    if ( svs.snapshotMatchStates )
        Hunk_UserFree(hunk, svs.snapshotMatchStates);
    svs.snapshotMatchStates = 0;
    if ( svs.snapshotEntities )
        Hunk_UserFree(hunk, svs.snapshotEntities);
    svs.snapshotEntities = 0;
    if ( svs.snapshotClients )
        Hunk_UserFree(hunk, svs.snapshotClients);
    svs.snapshotClients = 0;
    if ( svs.cachedSnapshotFrames )
        Hunk_UserFree(hunk, svs.cachedSnapshotFrames);
    svs.cachedSnapshotFrames = 0;
    if ( svs.cachedSnapshotMatchStates )
        Hunk_UserFree(hunk, svs.cachedSnapshotMatchStates);
    svs.cachedSnapshotMatchStates = 0;
    if ( svs.cachedSnapshotEntities )
        Hunk_UserFree(hunk, svs.cachedSnapshotEntities);
    svs.cachedSnapshotEntities = 0;
    if ( svs.cachedSnapshotClients )
        Hunk_UserFree(hunk, svs.cachedSnapshotClients);
    svs.cachedSnapshotClients = 0;
    if ( svs.archivedSnapshotFrames )
        Hunk_UserFree(hunk, svs.archivedSnapshotFrames);
    svs.archivedSnapshotFrames = 0;
    if ( svs.archivedSnapshotBuffer )
        Hunk_UserFree(hunk, svs.archivedSnapshotBuffer);
    svs.archivedSnapshotBuffer = 0;
}

bool __cdecl SV_Loaded()
{
    return sv.state == SS_GAME;
}

void __cdecl SV_Init()
{
    SV_AddOperatorCommands();
    Demo_RegisterDvars();
    SV_BotRegisterDvars();
#ifdef KISAK_SP
    _Dvar_RegisterInt("g_gameskill", 1, 0, 3, 0x1064u, "");
    // Retail SV_Init (BlackOps.exe 0x00698260, site 0x0069XXXX) registers this
    // Bool(true, 0x1004 = SAVED|SERVERINFO). Never registered in this tree, so
    // maps/_load.gsc:270 `SetSavedDvar("sv_saveOnStartMap", ...)` was throwing
    // "does not exist" (after the g_speed SAVED-flag throw above it, both on the
    // frontend map's _load::main()).
    _Dvar_RegisterBool("sv_saveOnStartMap", 1, 0x1004u, "");
#endif
    // This is the CREATING registration for g_gametype: Com_Init calls SV_Init
    // (common.cpp:2013) long before Com_LoadFrontEnd (common.cpp:1699) triggers the
    // frontend SV_SpawnServer. Dvar_Reregister never rewrites dvar->reset (dvar.cpp:1898
    // only ORs flags and sets description), so the reset value stored here is the one every
    // later _Dvar_RegisterString("g_gametype", ...) is compared against for the rest of the
    // process -- namely SV_SetGametype (sv_game.cpp) and G_RegisterDvars
    // (g_main_mp.cpp). If any of the three disagree, Dvar_Reregister's reset-value
    // check fires: (dvar->flags & 0x9200) == 0 is true for 0x24, and Dvar_ValuesEqual for
    // DVAR_TYPE_STRING is a plain strcmp (dvar.cpp:690), so dvar.cpp:1884 calls
    // Assert_MyHandler -- whose real body is #if 0'd in this tree, leaving
    // `__debugbreak(); return 1;` (assertive.cpp:626). That prints nothing and raises an
    // unhandled EXCEPTION_BREAKPOINT, which the SP exception filter turns into a bare
    // "Fatal Error" with no file, line, or message.
#ifdef KISAK_SP
    // Retail SP registers "cmp" with an empty description here. Verified in BlackOps.exe at
    // 0x00698391: push 0x9dd354 ("") / push 0x24 / push 0xa30458 ("cmp") / push 0xa1ca20
    // ("g_gametype"). All three SP registration sites (0x00698391 here, 0x005715e0
    // G_RegisterDvars, 0x00549ac3 SV_SetGametype) push byte-identical operands, which is why
    // retail never trips the assert described above.
    sv_gametype = _Dvar_RegisterString("g_gametype", "cmp", 0x24u, "");
#else
    sv_gametype = _Dvar_RegisterString("g_gametype", "tdm", 0x24u, "Current game type");
#endif
    _Dvar_RegisterInt("protocol", 1044, 1044, 1044, 0x44u, "Protocol version");
    sv_mapname = _Dvar_RegisterString("mapname", (char *)"", 0x44u, "Current map name");
    sv_privateClients = _Dvar_RegisterInt(
                                                "sv_privateClients",
                                                0,
                                                0,
                                                32,
                                                0,
                                                "Maximum number of private clients allowed on the server");
    sv_hostname = _Dvar_RegisterString("sv_hostname", IsDedicatedServer() ? "BlackOpsPublic" : "BlackOpsPrivate", 5u, "Host name of the server");
    sv_noname = _Dvar_RegisterString(
                                "sv_noname",
                                "Unknown Soldier",
                                0,
                                "Player name assigned to players that fail name validation");
    sv_geolocation = _Dvar_RegisterString("sv_geolocation", (char *)"", 0x10u, "geolocation");
    sv_maxgrouperrors = _Dvar_RegisterInt(
                                                "sv_maxgrouperrors",
                                                5,
                                                0,
                                                0x7FFFFFFF,
                                                0,
                                                "Number of group errors before a sys_error");
    sv_ownerid = _Dvar_RegisterString("sv_ownerid", (char *)"", 0, "SteamID of server owner");
    sv_numreservedslots = _Dvar_RegisterInt("maxreservedslots", 6, 0, 6, 0x40u, "");
    sv_clientSideBullets = _Dvar_RegisterBool(
                                                     "sv_clientSideBullets",
                                                     1,
                                                     0x100u,
                                                     "If true, clients will synthesize tracers and bullet impacts");
    sv_clientSideVehicles = _Dvar_RegisterBool(
                                                        "sv_clientSideVehicles",
                                                        1,
                                                        0x100u,
                                                        "If true, vehicles will be predicted on the client reducing response time");
    sv_penetrationCount = _Dvar_RegisterInt(
                                                    "penetrationCount",
                                                    5,
                                                    0,
                                                    5,
                                                    0x100u,
                                                    "Maximum number of private clients allowed on the server");
    sv_axis_penetrationCount = _Dvar_RegisterInt("penetrationCount_axis", 5, 0, 5, 0x100u, "Maximum number for TEAM_AXIS");
    sv_allies_penetrationCount = _Dvar_RegisterInt(
                                                                 "penetrationCount_allies",
                                                                 5,
                                                                 0,
                                                                 5,
                                                                 0x100u,
                                                                 "Maximum number for TEAM_ALLIES");
    sv_bullet_range = _Dvar_RegisterFloat(
                                            "bulletrange",
                                            8192.0,
                                            0.0,
                                            65536.0,
                                            0x100u,
                                            "Defines the how far the bulllets will go.");
    sv_hitFXFrustumCutoff = _Dvar_RegisterFloat(
                                                        "fxfrustumCutoff",
                                                        1000.0,
                                                        0.0,
                                                        5000.0,
                                                        0x100u,
                                                        "Hit effects that are more than <this value> outside of the frustum will be culled.");
    sv_punkbuster = _Dvar_RegisterBool("sv_punkbuster", 0, 0x10u, "Enable PunkBuster on this server");
    sv_security = _Dvar_RegisterInt("sv_security", 1, 0, 2, 0x14u, "Enable security on this server");
    sv_ranked = _Dvar_RegisterInt("sv_ranked", 0, 0, 5, 0x44u, "Server license type.");
    ui_ranked = _Dvar_RegisterBool("ui_ranked", 0, 0x80u, "True if playing in a ranked server");
#if defined(KISAK_SP) && defined(KISAK_DEDICATED)
    sv_dedicatedmaxclients = _Dvar_RegisterInt("sv_dedicatedmaxclients", 4, 1, 4, 0x10u, "Maximum Zombies clients");
    sv_maxclients = _Dvar_RegisterInt("sv_maxclients", 4, 1, 4, 5u, "Maximum Zombies clients");
#else
    sv_dedicatedmaxclients = _Dvar_RegisterInt(
                                                         "sv_dedicatedmaxclients",
                                                         32,
                                                         1,
                                                         32,
                                                         0x10u,
                                                         "The dedicated server max clients");
    sv_maxclients = _Dvar_RegisterInt(
                                        "sv_maxclients",
                                        18,
                                        1,
                                        IsDedicatedServer() ? sv_dedicatedmaxclients->current.integer : 30,
                                        5u,
                                        "The maximum number of clients that can connect to a server");
#endif
    sv_maxRate = _Dvar_RegisterInt("sv_maxRate", 5000, 0, 25000, 5u, "Maximum bit rate");
    sv_minPing = _Dvar_RegisterInt("sv_minPing", 0, 0, 999, 5u, "Minimum ping allowed on the server");
    sv_maxPing = _Dvar_RegisterInt("sv_maxPing", 0, 0, 999, 5u, "Maximum ping allowed on the server");
    sv_timeout = _Dvar_RegisterInt("sv_timeout", 240, 0, 1800, 0, "seconds without any message");
    sv_connectTimeout = _Dvar_RegisterInt(
                                                "sv_connectTimeout",
                                                80,
                                                0,
                                                1800,
                                                0,
                                                "seconds without any message when a client is loading");
    sv_floodProtect = _Dvar_RegisterInt(
                                            "sv_floodprotect",
                                            4,
                                            0,
                                            0x7FFFFFFF,
                                            5u,
                                            "Prevent malicious lagging by flooding the server with commands.    Is the number of client commands "
                                            "the server will process per 800ms.    0 means no flood protection.");
    sv_showCommands = _Dvar_RegisterBool("sv_showCommands", 0, 0, "Print client commands in the log file");
    sv_writeConfigStrings = _Dvar_RegisterBool("sv_writeConfigStrings", 0, 0, "Write out the config string file");
    scr_writeConfigStrings = _Dvar_RegisterBool(
                                                         "scr_writeConfigStrings",
                                                         0,
                                                         0,
                                                         "Special script mode for writing config string files");
    _Dvar_RegisterString("sv_keywords", (char *)"", 0, "Server keywords");
    sv_dwlsgerror = _Dvar_RegisterBool("sv_dwlsgerror", 0, 0, "Demonware LSG error");
    sv_allowAnonymous = _Dvar_RegisterBool("sv_allowAnonymous", 0, 0, "Allow anonymous access");
    sv_disableClientConsole = _Dvar_RegisterBool(
                                                            "sv_disableClientConsole",
                                                            0,
                                                            4u,
                                                            "Disallow remote clients from accessing the console");
    sv_privatePassword = _Dvar_RegisterString(
                                                 "sv_privatePassword",
                                                 (char *)"",
                                                 0,
                                                 "password for the privateClient slots");
    sv_allowDownload = _Dvar_RegisterBool("sv_allowDownload", 1, 1u, "Allow auto download of files");
    sv_iwds = _Dvar_RegisterString("sv_iwds", (char *)"", 0x48u, "IWD server checksums");
    sv_iwdNames = _Dvar_RegisterString(
                                    "sv_iwdNames",
                                    (char *)"",
                                    0x48u,
                                    "Names of IWD files used by the server");
    sv_referencedIwds = _Dvar_RegisterString(
                                                "sv_referencedIwds",
                                                (char *)"",
                                                0x48u,
                                                "Checksum of all referenced IWD files");
    sv_referencedIwdNames = _Dvar_RegisterString(
                                                        "sv_referencedIwdNames",
                                                        (char *)"",
                                                        0x48u,
                                                        "Names of all referenced IWD files");
    sv_FFCheckSums = _Dvar_RegisterString("sv_FFCheckSums", (char *)"", 0x48u, "Fast File server checksums");
    sv_FFNames = _Dvar_RegisterString(
                                 "sv_FFNames",
                                 (char *)"",
                                 0x48u,
                                 "Names of Fast Files used by the server");
    sv_referencedFFCheckSums = _Dvar_RegisterString(
                                                             "sv_referencedFFCheckSums",
                                                             (char *)"",
                                                             0x48u,
                                                             "Checksum of all referenced Fast Files");
    sv_referencedFFNames = _Dvar_RegisterString(
                                                     "sv_referencedFFNames",
                                                     (char *)"",
                                                     0x48u,
                                                     "Names of all referenced Fast Files");
    sv_authenticating = _Dvar_RegisterBool("sv_authenticating", 0, 0x40u, "");
    sv_voice = _Dvar_RegisterBool("sv_voice", 0, 0x105u, "Use server side voice communications");
    sv_voiceQuality = _Dvar_RegisterInt("sv_voiceQuality", 3, 0, 9, 0x100u, "Voice quality");
    sv_cheats = _Dvar_RegisterBool("sv_cheats", 0, 0x18u, "Enable cheats on the server");
    sv_pure = _Dvar_RegisterBool("sv_pure", 0, 0x104u, "Cannot use modified IWD files");
    rcon_password = _Dvar_RegisterString("rcon_password", (char *)"", 0, "Password for the rcon command");
    sv_fps = _Dvar_RegisterInt("sv_fps", 20, 10, 1000, 0, "Server frames per second");
    sv_showPingSpam = _Dvar_RegisterInt("sv_showPingSpam", 0, 0, 1, 0, "Turns on ping info spam.");
    sv_zombietime = _Dvar_RegisterInt("sv_zombietime", 2, 0, 1800, 0, "seconds to sync messages after disconnect");
    sv_reconnectlimit = _Dvar_RegisterInt("sv_reconnectlimit", 3, 0, 1800, 1u, "minimum seconds between connect messages");
    sv_padPackets = _Dvar_RegisterInt("sv_padPackets", 0, 0, 0x7FFFFFFF, 0, "add nop bytes to messages");
    sv_allowedClan1 = _Dvar_RegisterString(
                                            "sv_allowedClan1",
                                            (char *)"",
                                            0,
                                            "Allow this clan to join the server");
    sv_allowedClan2 = _Dvar_RegisterString(
                                            "sv_allowedClan2",
                                            (char *)"",
                                            0,
                                            "Allow this clan to join the server");
    sv_packet_info = _Dvar_RegisterBool("sv_packet_info", 0, 0, "Enable packet info debugging information");
    sv_showAverageBPS = _Dvar_RegisterBool("sv_showAverageBPS", 0, 0, "Show average bytes per second for net debugging");
    sv_kickBanTime = _Dvar_RegisterFloat(
                                         "sv_kickBanTime",
                                         300.0,
                                         0.0,
                                         3600.0,
                                         0,
                                         "Time in seconds for a player to be banned from the server after being kicked");
    sv_debugMessageKey = _Dvar_RegisterBool("sv_debugMessageKey", 0, 0, "net message key generation debugging");
    sv_debugPacketContents = _Dvar_RegisterBool(
                                                         "sv_debugPacketContents",
                                                         0,
                                                         0,
                                                         "print out the contents of every snapshot (VERY SLOW)");
    sv_debugPacketContentsForClientThisFrame = _Dvar_RegisterBool(
                                                                                             "sv_debugPacketContentsForClientThisFrame",
                                                                                             0,
                                                                                             0,
                                                                                             "set to true to get the next snapshot for this client");
    sv_showHuffmanData = _Dvar_RegisterBool(
                                                 "sv_showHuffmanData",
                                                 0,
                                                 0,
                                                 "To enable or disable the printing of the huffman data byte count");
    sv_debugConstantConfigStrings = _Dvar_RegisterBool(
                                                                        "sv_debugConstantConfigStrings",
                                                                        0,
                                                                        0,
                                                                        "const config strings debugging");
    sv_loadMyChanges = _Dvar_RegisterBool("sv_loadMyChanges", 0, 0, "Load my changes fast file on devmap.");
    sv_debugPlayerstate = _Dvar_RegisterBool(
                                                    "sv_debugPlayerstate",
                                                    0,
                                                    0,
                                                    "Print out what fields are changing in the playerstate");
    sv_debugPacketContentsQuick = _Dvar_RegisterInt(
                                                                    "sv_debugPacketContentsQuick",
                                                                    0,
                                                                    0,
                                                                    2,
                                                                    0,
                                                                    "print out snapshot entity changed fields");
    sv_printMessageSize = _Dvar_RegisterBool("sv_printMessageSize", 0, 0, "print out size of client messages");
    sv_mapRotation = _Dvar_RegisterString(
                                         "sv_mapRotation",
                                         (char *)"",
                                         0,
                                         "List of maps for the server to play");
    sv_mapRotationCurrent = _Dvar_RegisterString(
                                                        "sv_mapRotationCurrent",
                                                        (char *)"",
                                                        0,
                                                        "Current map in the map rotation");
    sv_debugRate = _Dvar_RegisterBool("sv_debugRate", 0, 0, "Enable snapshot rate debugging info");
    sv_debugReliableCmds = _Dvar_RegisterBool(
                                                     "sv_debugReliableCmds",
                                                     0,
                                                     0,
                                                     "Enable debugging information for 'reliable' commands");
    nextmap = _Dvar_RegisterString("nextmap", (char *)"", 0, "Next map to play");
    com_movieIsPlaying = _Dvar_RegisterBool("com_movieIsPlaying", 0, 0, "Is a movie playiner.");
    sv_wwwDownload = _Dvar_RegisterBool("sv_wwwDownload", 0, 1u, "Enable http downloads");
    sv_wwwBaseURL = _Dvar_RegisterString(
                                        "sv_wwwBaseURL",
                                        (char *)"",
                                        1u,
                                        "The base url for files downloaded via http");
    sv_wwwDlDisconnected = _Dvar_RegisterBool(
                                                     "sv_wwwDlDisconnected",
                                                     0,
                                                     1u,
                                                     "Should clients stay connected while downloading?");
    sv_smp = _Dvar_RegisterBool("sv_smp", 1, 0x80u, "Enable server multithreading");
    sv_network_fps = _Dvar_RegisterInt(
                                         "sv_network_fps",
                                         100,
                                         20,
                                         200,
                                         0x80u,
                                         "Number of times per second the server checks for net messages");
    sv_assistWorkers = _Dvar_RegisterBool("sv_assistWorkers", 0, 0x80u, "Enable server worker thread assist when idle");
    sv_clientArchive = _Dvar_RegisterBool(
                                             "sv_clientArchive",
                                             1,
                                             0,
                                             "Have the clients archive data to save bandwidth on the server");
}

void __cdecl SV_DropAllClients()
{
    client_t *drop; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    drop = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( drop->header.state >= CS_CONNECTED )
            SV_DropClient(drop, "EXE_DISCONNECTED", 1, !xblive_wagermatch->current.enabled);
        ++i;
        ++drop;
    }
}

void __cdecl SV_Shutdown(const char *finalmsg)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    2039,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        Com_SyncThreads();
        if ( Demo_IsRecording() )
            Demo_End(0);
        Com_Printf(15, "----- Server Shutdown -----\n");
        SV_FinalMessage(finalmsg);
        if (IsDedicatedServer())
        {
            SV_SysLog_LogMessage(5, va("shutting down: %s", finalmsg));
        }
        //BLOPS_NULLSUB();
        SV_ShutdownGameProgs();
        SV_DropAllClients();
        LiveSteam_Server_Shutdown();
        SV_FreeClients();
        SV_ClearServer();
        Dvar_SetBool((dvar_s *)com_sv_running, 0);
#ifndef KISAK_SP
        // SP retail SV_Shutdown (0x005142b0) leaves this to its caller.
        // Com_ShutdownInternal frees client memory after Com_Restart releases
        // collision; freeing here unloads frontend while cm.isInUse is set.
        CL_FreePerLocalClientMemory();
#endif
        memset((unsigned __int8 *)&svs, 0, sizeof(svs));
        if (!IsDedicatedServer())
        {
            Session_DeleteSession(&g_serverSession);
        }
        //*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8) = 0;
        bgs = 0;
        Com_Printf(15, "---------------------------\n");
    }
}

void __cdecl SV_FinalMessage(const char *message)
{
    int j; // [esp+0h] [ebp-40h]
    client_t *client; // [esp+4h] [ebp-3Ch]
    bool translationForReason; // [esp+Bh] [ebp-35h]
    msg_t msg; // [esp+Ch] [ebp-34h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    translationForReason = SEH_StringEd_GetString((char*)message) != 0;
    for ( j = 0; j < 2; ++j )
    {
        i = 0;
        client = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( client->header.state >= CS_CONNECTED )
            {
                if ( client->header.netchan.remoteAddress.type != NA_LOOPBACK )
                    SV_SendDisconnect(client, client->header.state, message, translationForReason, client->name);
                client->nextSnapshotTime = -1;
                client->lastSnapshotTime = -1;
                SV_SetServerStaticHeader();
                SV_BeginClientSnapshot(client, &msg);
                if ( client->header.state == CS_ACTIVE || client->header.state == CS_ZOMBIE )
                    SV_WriteSnapshotToClient(client, &msg);
                SV_EndClientSnapshot(client, &msg);
                SV_GetServerStaticHeader();
            }
            ++i;
            ++client;
        }
    }
}

void __cdecl SV_ClearServerThreadOwnsGame()
{
    _InterlockedExchange(&sv_thread_owns_game, 0);
}

void __cdecl SV_IncServerThreadOwnsGame()
{
    _InterlockedExchangeAdd(&sv_thread_owns_game, 1u);
}

void __cdecl SV_DecServerThreadOwnsGame()
{
    if ( sv_thread_owns_game <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    2163,
                    0,
                    "%s",
                    "sv_thread_owns_game > 0") )
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(&sv_thread_owns_game, 0xFFFFFFFF);
}

int __cdecl SV_GetServerThreadOwnsGame()
{
    return sv_thread_owns_game;
}

void __cdecl SV_CheckThread()
{
    if ( SV_GetServerThreadOwnsGame() <= 0 )
    {
        if ( !Sys_IsMainThread()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                        2190,
                        0,
                        "%s",
                        "Sys_IsMainThread()") )
        {
            __debugbreak();
        }
    }
    else if ( !Sys_IsServerThread()
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                             2188,
                             0,
                             "%s",
                             "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
}

