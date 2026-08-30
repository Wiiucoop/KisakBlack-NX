#include "sv_autoplaylist.h"
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <universal/q_parse.h>
#include <qcommon/cmd.h>
#include <live/live_storage_win.h>
#include <live/live_storage_pub.h>
#include <client_mp/cl_main_mp.h>

sv_apstate_t s_apstate = AP_SLEEPING;

dwFileOperationInfo s_finfo;

int s_probabilities[64];

void __cdecl SV_AP_DumpTable()
{
    int i; // [esp+0h] [ebp-4h]

    if ( Dvar_GetBool("sv_ap_debug") )
    {
        Com_Printf(15, "\n**************************************\nPlaylist\t\tProbability\n");
        for ( i = 0; i < 64; ++i )
            Com_Printf(15, "%i\t\t%u\n", i, s_probabilities[i]);
        Com_Printf(15, "\n**************************************\n");
    }
}

bool __cdecl SV_AP_ParseControlFile(unsigned __int8 *controlFile)
{
    parseInfo_t *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    int value; // [esp+0h] [ebp-24h]
    unsigned int probability; // [esp+4h] [ebp-20h]
    int playlist; // [esp+8h] [ebp-1Ch]
    parseInfo_t *token; // [esp+Ch] [ebp-18h]
    int totalProb; // [esp+10h] [ebp-14h]
    int numPlaylists; // [esp+14h] [ebp-10h]
    const char *cFile; // [esp+18h] [ebp-Ch] BYREF
    bool retval; // [esp+1Fh] [ebp-5h]
    int line; // [esp+20h] [ebp-4h]

    cFile = (const char *)controlFile;
    numPlaylists = 0;
    totalProb = 0;
    line = 0;
    retval = 0;
    Com_BeginParseSession("AutoPlaylist");
    Com_SetSpaceDelimited(0);
    Com_SetCSV(1);
    while ( 1 )
    {
        ++line;
        token = Com_Parse(&cFile);
        if ( !token || !token->token[0] )
            break;
        if ( token->token[0] == 35 )
        {
            Com_SkipRestOfLine(&cFile);
        }
        else
        {
            playlist = atoi(token->token);
            if ( playlist <= 0 || playlist >= 64 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                {
                    v6 = va("Failed parsing control file at line %i, playlist %i failed sanity check\n", line, playlist);
                    Com_Error(ERR_DROP, "AP error: %s\n", v6);
                }
                else
                {
                    v7 = va("Failed parsing control file at line %i, playlist %i failed sanity check\n", line, playlist);
                    Com_PrintError(15, "AP Error: %s\n", v7);
                }
                break;
            }
            v1 = Com_Parse(&cFile);
            probability = atoi(v1->token);
            if ( probability > 0x64 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                {
                    v4 = va("Failed parsing control file at line %i, probability %i failed sanity check\n", line, probability);
                    Com_Error(ERR_DROP, "AP error: %s\n", v4);
                }
                else
                {
                    v5 = va("Failed parsing control file at line %i, probability %i failed sanity check\n", line, probability);
                    Com_PrintError(15, "AP Error: %s\n", v5);
                }
                break;
            }
            s_probabilities[playlist] = (unsigned __int8)probability;
            totalProb += probability;
            if ( ++numPlaylists == 64 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                    Com_Error(ERR_DROP, "AP error: %s\n", "Too many playlists, truncating!\n");
                else
                    Com_PrintError(15, "AP Error: %s\n", "Too many playlists, truncating!\n");
                break;
            }
            if ( totalProb > 100 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                {
                    v2 = va("We've exceeded 100%% total probability on line %i\n", line);
                    Com_Error(ERR_DROP, "AP error: %s\n", v2);
                }
                else
                {
                    v3 = va("We've exceeded 100%% total probability on line %i\n", line);
                    Com_PrintError(15, "AP Error: %s\n", v3);
                }
                break;
            }
        }
    }
    Com_EndParseSession();
    if ( totalProb == 100 )
    {
        if ( Dvar_GetBool("sv_ap_debug") )
        {
            v10 = va("Successfully parsed %i playlist probabilities, %i lines\n", numPlaylists, line);
            Com_Printf(15, "AP: %s\n", v10);
        }
        retval = 1;
        SV_AP_DumpTable();
        value = SV_AP_PlaylistFromDistribution();
        if ( Dvar_GetBool("sv_ap_debug") )
        {
            v11 = va("Choosing playlist %i\n", value);
            Com_Printf(15, "AP: %s\n", v11);
        }
        Dvar_SetIntByName("playlist", value);
        Cbuf_AddText(0, "map_rotate\n");
    }
    else if ( Dvar_GetBool("sv_ap_fatal") )
    {
        v8 = va("Finished parsing, total probability is %i, should be 100\n", totalProb);
        Com_Error(ERR_DROP, "AP error: %s\n", v8);
    }
    else
    {
        v9 = va("Finished parsing, total probability is %i, should be 100\n", totalProb);
        Com_PrintError(15, "AP Error: %s\n", v9);
    }
    return retval;
}

int __cdecl SV_AP_PlaylistFromDistribution()
{
#ifdef KISAK_LIVE
    const char *v0; // eax
    bdTrulyRandomImpl *Instance; // eax
    unsigned int RandomUInt; // eax
    int n; // [esp+64h] [ebp-4C0h]
    int m; // [esp+68h] [ebp-4BCh]
    int k; // [esp+6Ch] [ebp-4B8h]
    int j; // [esp+74h] [ebp-4B0h]
    int i; // [esp+78h] [ebp-4ACh]
    int probdists[64]; // [esp+7Ch] [ebp-4A8h] BYREF
    int choice; // [esp+17Ch] [ebp-3A8h]
    int activeplaylists; // [esp+180h] [ebp-3A4h]
    int numchoices; // [esp+184h] [ebp-3A0h]
    int totalservers; // [esp+188h] [ebp-39Ch]
    int retvals[101]; // [esp+18Ch] [ebp-398h] BYREF
    int count; // [esp+320h] [ebp-204h]
    goaldiff_t playlists[64]; // [esp+324h] [ebp-200h] BYREF

    totalservers = 0;
    memset(probdists, 0, sizeof(probdists));
    activeplaylists = 0;
    numchoices = 0;
    for ( i = 0; i < 64; ++i )
    {
        totalservers += dword_979CF90[3 * i];
        if ( s_probabilities[i] > 0 )
            ++activeplaylists;
    }
    if ( firstTimeRunning )
        ++totalservers;
    if ( Dvar_GetBool("sv_ap_debug") )
    {
        v0 = va("%i total servers in geogroup\n", totalservers);
        Com_Printf(15, "AP: %s\n", v0);
    }
    if ( totalservers <= activeplaylists )
    {
        memcpy(probdists, s_probabilities, sizeof(probdists));
        numchoices = 100;
    }
    else
    {
        for ( j = 0; j < 64; ++j )
        {
            playlists[j].diff = (int)((double)totalservers * ((double)s_probabilities[j] / 100.0)) - dword_979CF90[3 * j];
            playlists[j].playlist = j;
        }
        qsort(playlists, 0x40u, 8u, (int (__cdecl *)(const void *, const void *))comparePlaylists);
        for ( k = 0; k < 64; ++k )
        {
            if ( playlists[k].diff > 0 )
            {
                probdists[playlists[k].playlist] = (int)((double)playlists[k].diff / (double)totalservers * 100.0);
                numchoices += probdists[playlists[k].playlist];
            }
        }
    }
    if ( numchoices || firstTimeRunning )
    {
        if ( !numchoices && firstTimeRunning )
        {
            memcpy(probdists, s_probabilities, sizeof(probdists));
            for ( m = 0; m < 64; ++m )
            {
                if ( playlists[m].diff < 0 )
                    probdists[m] = 0;
                numchoices += probdists[playlists[m].playlist];
            }
            if ( !numchoices )
                numchoices = 100;
        }
        count = 0;
        memset(retvals, 0, 400);
        for ( n = 0; n < 64; ++n )
        {
            if ( count > numchoices
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_autoplaylist.cpp",
                            184,
                            0,
                            "%s",
                            "count <= numchoices") )
            {
                __debugbreak();
            }
            while ( probdists[n] > 0 )
            {
                retvals[count++] = n;
                --probdists[n];
            }
        }
        firstTimeRunning = 0;
        Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
        RandomUInt = bdTrulyRandomImpl::getRandomUInt(Instance);
        choice = RandomUInt % numchoices;
        return retvals[RandomUInt % numchoices];
    }
    else
    {
        firstTimeRunning = 0;
        return playlist->current.integer;
    }
#else
    return playlist->current.integer;
#endif
}

int __cdecl comparePlaylists(unsigned int *p1, unsigned int *p2)
{
    return *p2 - *p1;
}

void __cdecl SV_AP_GetControlFileComplete()
{
    if ( Dvar_GetBool("sv_ap_debug") )
        Com_Printf(15, "AP: %s\n", "Received control file, attempting to parse\n");
    //operator++(&s_apstate);
    s_apstate++;
}

int __cdecl SV_AP_GetControlFileFailure()
{
    if ( Dvar_GetBool("sv_ap_fatal") )
        Com_Error(ERR_DROP, "AP error: %s\n", "Couldn't get control file.\n");
    else
        Com_PrintError(15, "AP Error: %s\n", "Couldn't get control file.\n");
    return 1;
}

TaskRecord *__cdecl SV_AP_GetControlFile()
{
#ifdef KISAK_LIVE
    TaskRecord *nestedTask; // [esp+0h] [ebp-8h]
    TaskRecord *task; // [esp+4h] [ebp-4h]

    task = 0;
    SV_AP_GetControlFileName(s_controlFileName, 32);
    s_finfo.isUserFile = 0;
    s_finfo.isCompressedFile = 0;
    s_finfo.fileTask.m_filename = s_controlFileName;
    s_finfo.fileBuffer = s_controlFileBuffer;
    s_finfo.bufferSize = 1024;
    s_finfo.fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_AP_GetControlFileComplete;
    s_finfo.fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))SV_AP_GetControlFileFailure;
    nestedTask = LiveStorage_ReadDWFile(0, &s_finfo);
    if ( nestedTask )
    {
        task = TaskManager2_CreateTask(task_SVFetchControlFile, 0, nestedTask, 0);
        if ( task )
        {
            TaskManager2_StartTask(task);
        }
        else if ( Dvar_GetBool("sv_ap_fatal") )
        {
            Com_Error(
                ERR_DROP,
                "AP error: %s\n",
                "Couldn't start fetch control file nested task. Probable connectivity issue\n");
        }
        else
        {
            Com_PrintError(
                15,
                "AP Error: %s\n",
                "Couldn't start fetch control file nested task. Probable connectivity issue\n");
        }
    }
    else if ( Dvar_GetBool("sv_ap_fatal") )
    {
        Com_Error(ERR_DROP, "AP error: %s\n", "Couldn't start fetch control file dw task. Probable connectivity issue\n");
    }
    else
    {
        Com_PrintError(15, "AP Error: %s\n", "Couldn't start fetch control file dw task. Probable connectivity issue\n");
    }
    return task;
#else
    return NULL;
#endif
}

void __cdecl SV_AP_GetControlFileName(char *buf, int buflen)
{
#ifdef KISAK_LIVE
    char *filename; // [esp+0h] [ebp-4h]

    if ( (!sv_geolocation || !*(_BYTE *)sv_geolocation->current.string)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_autoplaylist.cpp",
                    299,
                    0,
                    "%s",
                    "sv_geolocation && sv_geolocation->current.string[0]") )
    {
        __debugbreak();
    }
    if ( sv_geolocation && *(_BYTE *)sv_geolocation->current.string )
    {
        filename = va("%s%s", sv_geolocation->current.string, ".csv");
        I_strncpyz(buf, filename, buflen);
    }
    else if ( Dvar_GetBool("sv_ap_fatal") )
    {
        Com_Error(
            ERR_DROP,
            "AP error: %s\n",
            "sv_geolocation isn't set. This has to be set for the autoplaylist tech to work\n");
    }
    else
    {
        Com_PrintError(
            15,
            "AP Error: %s\n",
            "sv_geolocation isn't set. This has to be set for the autoplaylist tech to work\n");
    }
#endif
}

void __cdecl SV_SetGroupCountsComplete()
{
#ifdef KISAK_LIVE
    Com_Printf(15, "Group set complete\n");
    s_apstate = AP_SLEEPING;
    s_numgroupErrors = 0;
    if ( !Dvar_GetBool("sv_ap_enabled") && SV_ShouldMapRotate() )
    {
        SV_SetShouldMapRotate(0);
        Cbuf_AddText(0, "map_rotate\n");
    }
#endif
}

void __cdecl SV_GetGroupCountsComplete()
{
#ifdef KISAK_LIVE
    Com_Printf(15, "Groups get complete\n");
    s_numgroupErrors = 0;
    if ( !LiveStorage_DoWeHavePlaylists()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_autoplaylist.cpp",
                    388,
                    0,
                    "%s",
                    "LiveStorage_DoWeHavePlaylists()") )
    {
        __debugbreak();
    }
    operator++(&s_apstate);
#endif
}

void __cdecl SV_GroupsFailure(TaskRecord *task)
{
#ifdef KISAK_LIVE
    enum bdLobbyErrorCode ErrorCode; // [esp+0h] [ebp-10h]

    if ( task->remoteTask.m_ptr )
        ErrorCode = bdRemoteTask::getErrorCode(task->remoteTask.m_ptr);
    else
        ErrorCode = -1;
    SV_GroupError("Remote async task failed: %i", ErrorCode);
#endif
}

void SV_GroupError(const char *fmt, ...)
{
    char msg[132]; // [esp+4h] [ebp-88h] BYREF
    va_list va; // [esp+98h] [ebp+Ch] BYREF

    va_start(va, fmt);
    _vsnprintf(msg, 0x80u, fmt, va);
    Com_PrintError(15, "GROUPS: %s\n", msg);
}

TaskRecord *__cdecl SV_GetGroupCounts()
{
#ifdef KISAK_LIVE
    const bdReference<bdCommonAddr> *GroupCounts; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-218h] BYREF
    int j; // [esp+20h] [ebp-214h]
    int i; // [esp+24h] [ebp-210h]
    TaskRecord *task; // [esp+28h] [ebp-20Ch]
    unsigned int groupids[129]; // [esp+2Ch] [ebp-208h] BYREF
    bdGroup *group; // [esp+230h] [ebp-4h]

    task = TaskManager2_CreateTask(task_getGroupCounts, 0, 0, 0);
    group = dwGetGroup(0);
    memset(groupids, 0, 512);
    i = s_geogroupid;
    for ( j = 0; j < 128; ++j )
        groupids[j] = i++;
    GroupCounts = (const bdReference<bdCommonAddr> *)bdGroup::getGroupCounts(
                                                                                                         group,
                                                                                                         (int)&v2,
                                                                                                         (int)groupids,
                                                                                                         0x80u,
                                                                                                         &s_bdGroupCounts,
                                                                                                         0x80u);
    bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, GroupCounts);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
    TaskManager2_StartTask(task);
    return task;
#else
    return NULL;
#endif
}

void __cdecl SV_Groups_SetGroupMembership(bool full)
{
#ifdef KISAK_LIVE
    int LicenseType; // eax
    const bdReference<bdCommonAddr> *v2; // eax
    bdReference<bdCommonAddr> v3; // [esp+1Ch] [ebp-1Ch] BYREF
    TaskRecord *task; // [esp+20h] [ebp-18h]
    int numgroups; // [esp+24h] [ebp-14h]
    unsigned int groupIDs[3]; // [esp+28h] [ebp-10h] BYREF
    bdGroup *group; // [esp+34h] [ebp-4h]

    if ( !playlist
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_autoplaylist.cpp", 420, 0, "%s", "playlist") )
    {
        __debugbreak();
    }
    if ( !cachedFull || !full )
    {
        cachedFull = full;
        if ( s_geogroupid == -1 )
        {
            SV_GroupError("No group to join");
        }
        else
        {
            group = dwGetGroup(0);
            if ( group )
            {
                numgroups = 0;
                groupIDs[0] = playlist->current.integer + s_geogroupid;
                numgroups = 1;
                if ( SV_IsWagerServer() )
                {
                    groupIDs[numgroups++] = 492;
                }
                else
                {
                    LicenseType = SV_GetLicenseType();
                    if ( SV_IsServerRanked(LicenseType) )
                        groupIDs[numgroups] = 490;
                    else
                        groupIDs[numgroups] = 491;
                    ++numgroups;
                }
                if ( full )
                    groupIDs[numgroups++] = s_geogroupid + playlist->current.integer + 64;
                task = TaskManager2_CreateTask(task_SVSetGroups, 0, 0, 0);
                v2 = (const bdReference<bdCommonAddr> *)bdGroup::setGroups(group, (int)&v3, (int)groupIDs, numgroups);
                bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v2);
                bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
                TaskManager2_StartTask(task);
            }
            else
            {
                SV_GroupError("Couldn't get group access, probable connectivity problem");
            }
        }
    }
#endif
}

void __cdecl SV_Groups_ParseGeos(const char *geoblob)
{
#ifdef KISAK_LIVE
    int v1; // eax
    parseInfo_t *country; // [esp+0h] [ebp-10h]
    parseInfo_t *geoID; // [esp+4h] [ebp-Ch]
    const char *myloc; // [esp+8h] [ebp-8h]
    int loc_count; // [esp+Ch] [ebp-4h]

    if ( !geoblob
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_autoplaylist.cpp",
                    474,
                    0,
                    "%s",
                    "NULL != geoblob") )
    {
        __debugbreak();
    }
    if ( sv_geolocation && *(_BYTE *)sv_geolocation->current.string )
    {
        Com_Printf(15, "Setting location to %s\n", sv_geolocation->current.string);
    }
    else if ( sv_geolocation )
    {
        Dvar_SetString((dvar_s *)sv_geolocation, "LAX");
    }
    myloc = sv_geolocation->current.string;
    loc_count = 0;
    Com_BeginParseSession("geogroups");
    Com_SetSpaceDelimited(1);
    while ( 1 )
    {
        while ( 1 )
        {
            geoID = Com_Parse(&geoblob);
            if ( !geoID || geoID->token[0] != 35 )
                break;
            Com_SkipRestOfLine(&geoblob);
        }
        if ( !geoID || !geoID->token[0] )
            break;
        if ( !I_stricmp(geoID->token, myloc) )
        {
            s_geogroupid = (loc_count << 7) + 500;
            Com_DPrintf(15, "GEOGROUPS: Set groupid to %i from geo %s\n", (loc_count << 7) + 500, myloc);
            country = Com_ParseOnLine(&geoblob);
            v1 = atoi(country->token);
            SV_SetRegion(v1);
            break;
        }
        Com_SkipRestOfLine(&geoblob);
        ++loc_count;
    }
    Com_EndParseSession();
    if ( s_geogroupid == -1 )
    {
        SV_GroupError("Failed to find match for geo %s\n", myloc);
    }
    else if ( !Dvar_GetBool("sv_ap_enabled") )
    {
        SV_Groups_SetGroupMembership(0);
    }
#endif
}

void __cdecl SV_AP_Start()
{
#ifdef KISAK_LIVE
    if ( Dvar_GetBool("sv_ap_enabled") )
    {
        if ( s_apstate == AP_SLEEPING )
        {
            s_apstate = AP_START;
        }
        else if ( !Assert_MyHandler(
                                 "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_autoplaylist.cpp",
                                 572,
                                 0,
                                 "Tried starting autoplaylist when not in AP_SLEEPING\n") )
        {
            __debugbreak();
        }
    }
#endif
}

void __cdecl SV_AP_Frame()
{
#ifdef KISAK_LIVE
    switch ( s_apstate )
    {
        case AP_START:
            if ( Dvar_GetBool("sv_ap_debug") )
                Com_Printf(15, "AP: %s\n", "Initing..\n");
            s_groupgetTask = SV_GetGroupCounts();
            s_controlFileTask = SV_AP_GetControlFile();
            if ( s_groupgetTask && s_controlFileTask )
            {
                s_apstate = AP_GETTINGDATA;
                s_aptimeout = Sys_Milliseconds();
                if ( Dvar_GetBool("sv_ap_debug") )
                    Com_Printf(15, "AP: %s\n", "AP_GETTINGDATA\n");
            }
            else
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                    Com_Error(ERR_DROP, "AP error: %s\n", "Couldn't start data tasks\n");
                else
                    Com_PrintError(15, "AP Error: %s\n", "Couldn't start data tasks\n");
                s_apstate = AP_ERROR;
            }
            break;
        case AP_GETTINGDATA:
        case AP_GETONE:
        case AP_SETTINGDATA:
            if ( (int)(Sys_Milliseconds() - s_aptimeout) > 60000 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                    Com_Error(ERR_DROP, "AP error: %s\n", "Data timeout\n");
                else
                    Com_PrintError(15, "AP Error: %s\n", "Data timeout\n");
                s_apstate = AP_ERROR;
            }
            break;
        case AP_CHOOSE:
            if ( Dvar_GetBool("sv_ap_debug") )
                Com_Printf(15, "AP: %s\n", "Fetched data ok\n");
            SV_AP_ParseControlFile(s_controlFileBuffer);
            SV_Groups_SetGroupMembership(0);
            s_aptimeout = Sys_Milliseconds();
            s_apstate = AP_SETTINGDATA;
            break;
        case AP_SLEEPING:
            if ( com_sv_running && com_sv_running->current.enabled )
            {
                if ( SV_AP_ServerIsFull() )
                {
                    if ( !cachedFull_0 )
                    {
                        cachedFull_0 = 1;
                        SV_Groups_SetGroupMembership(1);
                    }
                }
                else if ( cachedFull_0 )
                {
                    cachedFull_0 = 0;
                    SV_Groups_SetGroupMembership(0);
                }
            }
            break;
        default:
            return;
    }
#endif
}

bool __cdecl SV_AP_ServerIsFull()
{
//#ifdef KISAK_LIVE
    int i; // [esp+0h] [ebp-8h]
    bool retval; // [esp+7h] [ebp-1h]

    retval = 1;
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            if ( svs.clients[i].header.state == CS_FREE )
                return 0;
        }
    }
    return retval;
//#else
//    return false;
//#endif
}


