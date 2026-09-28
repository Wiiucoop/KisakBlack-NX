#include "dobj_management.h"
#include <cgame_mp/cg_local_mp.h>

int objFreeCount;
__int16 clientObjMap[1537];
__int16 serverObjMap[1024];
DObj objBuf[3072];
bool objAlloced[3072];

int com_lastDObjIndex;

int g_bDObjInited;

DObj *__cdecl Com_GetServerDObj(unsigned int handle)
{
    if ( handle >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    84,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < (sizeof( serverObjMap ) / (sizeof( serverObjMap[0] ) * (sizeof( serverObjMap ) != 4 || siz"
                    "eof( serverObjMap[0] ) <= 4))))",
                    handle) )
    {
        __debugbreak();
    }
    if ( (unsigned int)serverObjMap[handle] >= 0xC00
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    85,
                    0,
                    "%s",
                    "(unsigned)serverObjMap[handle] < DOBJ_HANDLE_MAX") )
    {
        __debugbreak();
    }
    if ( serverObjMap[handle] )
        return (DObj *)&objBuf[serverObjMap[handle]];
    else
        return 0;
}

DObj *__cdecl Com_ClientDObjCreate(
                DObjModel_s *dobjModels,
                unsigned __int16 numModels,
                XAnimTree_s *tree,
                unsigned int handle,
                int localClientNum)
{
    bool isLocalPlayer; // [esp+Bh] [ebp-Dh]
    unsigned int index; // [esp+Ch] [ebp-Ch]

    isLocalPlayer = 0;
    if ( !dobjModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp", 162, 0, "%s", "dobjModels") )
    {
        __debugbreak();
    }
    if ( handle >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    167,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < ((((1<<10) + 512)) + 1))",
                    handle) )
    {
        __debugbreak();
    }
    if ( Com_GetClientDObj(handle, localClientNum)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    168,
                    0,
                    "%s",
                    "!Com_GetClientDObj( handle, localClientNum )") )
    {
        __debugbreak();
    }
    if ( CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.clientNum == handle )
        isLocalPlayer = 1;
    index = Com_GetFreeDObjIndex();
    if ( handle >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    181,
                    0,
                    "%s",
                    "(unsigned)handle < ARRAY_COUNT( clientObjMap )") )
    {
        __debugbreak();
    }
    if ( (int)handle < 1537 * localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    183,
                    0,
                    "%s",
                    "handle >= localClientNum * CLIENT_DOBJ_HANDLE_MAX") )
    {
        __debugbreak();
    }
    if ( (int)handle >= 1537 * localClientNum + 1537
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    184,
                    0,
                    "%s",
                    "handle < localClientNum * CLIENT_DOBJ_HANDLE_MAX + CLIENT_DOBJ_HANDLE_MAX") )
    {
        __debugbreak();
    }
    DObjCreateExt(
        dobjModels,
        numModels,
        tree,
        (unsigned __int8 *)&objBuf[index],
        handle + 1,
        0,
        isLocalPlayer,
        localClientNum);
    clientObjMap[handle] = index;
    if ( index >= 0xC00
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    191,
                    0,
                    "%s",
                    "(unsigned)index < DOBJ_HANDLE_MAX") )
    {
        __debugbreak();
    }
    if ( !objFreeCount )
        Com_Error(ERR_DROP, "No free DObjs");
    return (DObj *)&objBuf[index];
}

int __cdecl Com_GetFreeDObjIndex()
{
    int i; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DOBJ_ALLOC);
    for ( i = com_lastDObjIndex + 1; i < 3072; ++i )
    {
        if ( !objAlloced[i] )
        {
            com_lastDObjIndex = i;
            if ( !i && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp", 112, 0, "%s", "i") )
                __debugbreak();
            if ( (unsigned int)i >= 0xC00
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                            113,
                            0,
                            "%s",
                            "(unsigned)i < ARRAY_COUNT( objAlloced )") )
            {
                __debugbreak();
            }
            objAlloced[i] = 1;
            if ( !objFreeCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                            115,
                            0,
                            "%s",
                            "objFreeCount") )
            {
                __debugbreak();
            }
LABEL_13:
            --objFreeCount;
            Sys_LeaveCriticalSection(CRITSECT_DOBJ_ALLOC);
            return i;
        }
    }
    for ( i = 1; i <= com_lastDObjIndex; ++i )
    {
        if ( !objAlloced[i] )
        {
            com_lastDObjIndex = i;
            if ( !i && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp", 130, 0, "%s", "i") )
                __debugbreak();
            if ( (unsigned int)i >= 0xC00
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                            131,
                            0,
                            "%s",
                            "(unsigned)i < ARRAY_COUNT( objAlloced )") )
            {
                __debugbreak();
            }
            objAlloced[i] = 1;
            if ( !objFreeCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                            133,
                            0,
                            "%s",
                            "objFreeCount") )
            {
                __debugbreak();
            }
            goto LABEL_13;
        }
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp", 143, 0, "unreachable") )
        __debugbreak();
    return 0;
}

void __cdecl Com_ClientDObjClearAllSkel()
{
    int handleOffset; // [esp+4h] [ebp-4h]

    for ( handleOffset = 0; handleOffset < 1537; ++handleOffset )
    {
        if ( clientObjMap[handleOffset] )
            DObjSkelClear((const DObj *)&objBuf[clientObjMap[handleOffset]]);
    }
}

DObj *__cdecl Com_ServerDObjCreate(
                DObjModel_s *dobjModels,
                unsigned __int16 numModels,
                XAnimTree_s *tree,
                unsigned int handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if ( !dobjModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp", 244, 0, "%s", "dobjModels") )
    {
        __debugbreak();
    }
    if ( handle >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    245,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < ((1<<10)))",
                    handle) )
    {
        __debugbreak();
    }
    if ( Com_GetServerDObj(handle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    246,
                    0,
                    "%s",
                    "!Com_GetServerDObj( handle )") )
    {
        __debugbreak();
    }
    index = Com_GetFreeDObjIndex();
    if ( handle >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    249,
                    0,
                    "%s",
                    "(unsigned)handle < ARRAY_COUNT( serverObjMap )") )
    {
        __debugbreak();
    }
    serverObjMap[handle] = index;
    if ( index >= 0xC00
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    251,
                    0,
                    "%s",
                    "(unsigned)index < DOBJ_HANDLE_MAX") )
    {
        __debugbreak();
    }
    DObjCreateExt(dobjModels, numModels, tree, (unsigned __int8 *)&objBuf[index], handle + 1, 1, 0, 0xFFu);
    if ( !objFreeCount )
        Com_Error(ERR_DROP, "No free DObjs");
    return (DObj *)&objBuf[index];
}

void __cdecl Com_SafeClientDObjFree(unsigned int handle, int localClientNum)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if ( handle >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    270,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < ((((1<<10) + 512)) + 1))",
                    handle) )
    {
        __debugbreak();
    }
    if ( handle >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    276,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < (sizeof( clientObjMap ) / (sizeof( clientObjMap[0] ) * (sizeof( clientObjMap ) != 4 || siz"
                    "eof( clientObjMap[0] ) <= 4))))",
                    handle) )
    {
        __debugbreak();
    }
    if ( handle - 1537 * localClientNum >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    277,
                    0,
                    "handle - localClientNum * CLIENT_DOBJ_HANDLE_MAX doesn't index CLIENT_DOBJ_HANDLE_MAX\n\t%i not in [0, %i)",
                    handle - 1537 * localClientNum,
                    1537) )
    {
        __debugbreak();
    }
    index = clientObjMap[handle];
    if ( clientObjMap[handle] )
    {
        clientObjMap[handle] = 0;
        if ( index >= 0xC00
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                        284,
                        0,
                        "%s",
                        "(unsigned)index < ARRAY_COUNT( objAlloced )") )
        {
            __debugbreak();
        }
        Sys_EnterCriticalSection(CRITSECT_DOBJ_ALLOC);
        objAlloced[index] = 0;
        ++objFreeCount;
        DObjFree((DObj *)&objBuf[index]);
        Sys_LeaveCriticalSection(CRITSECT_DOBJ_ALLOC);
    }
}

void __cdecl Com_SafeServerDObjFree(unsigned int handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if ( handle >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    314,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < ((1<<10)))",
                    handle) )
    {
        __debugbreak();
    }
    index = serverObjMap[handle];
    if ( serverObjMap[handle] )
    {
        serverObjMap[handle] = 0;
        if ( index >= 0xC00
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                        321,
                        0,
                        "%s",
                        "(unsigned)index < ARRAY_COUNT( objAlloced )") )
        {
            __debugbreak();
        }
        Sys_EnterCriticalSection(CRITSECT_DOBJ_ALLOC);
        objAlloced[index] = 0;
        ++objFreeCount;
        DObjFree((DObj *)&objBuf[index]);
        Sys_LeaveCriticalSection(CRITSECT_DOBJ_ALLOC);
    }
}

void __cdecl Com_InitDObj()
{
    Com_Memset((unsigned int *)objAlloced, 0, 3072);
    objFreeCount = 3071;
    Com_Memset((unsigned int *)clientObjMap, 0, 3074);
    Com_Memset((unsigned int *)serverObjMap, 0, 2048);
    com_lastDObjIndex = 1;
    g_bDObjInited = 1;
}

void __cdecl Com_ShutdownDObj()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int i; // [esp+0h] [ebp-4h]
    unsigned int ia; // [esp+0h] [ebp-4h]
    unsigned int ib; // [esp+0h] [ebp-4h]

    if ( g_bDObjInited )
    {
        g_bDObjInited = 0;
        for ( i = 0; i < 0xC00; ++i )
        {
            if ( objAlloced[i]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                            376,
                            0,
                            "%s\n\t(i) = %i",
                            "(!objAlloced[i])",
                            i) )
            {
                __debugbreak();
            }
        }
        for ( ia = 0; ia < 0x601; ++ia )
        {
            if ( clientObjMap[ia] )
            {
                v0 = va("i: %d, clientObjMap[i]: %d", ia, clientObjMap[ia]);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                                378,
                                0,
                                "%s\n\t%s",
                                "!clientObjMap[i]",
                                v0) )
                    __debugbreak();
            }
        }
        for ( ib = 0; ib < 0x400; ++ib )
        {
            if ( serverObjMap[ib] )
            {
                v1 = va("i: %d, serverObjMap[i]: %d", ib, serverObjMap[ib]);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                                380,
                                0,
                                "%s\n\t%s",
                                "!serverObjMap[i]",
                                v1) )
                    __debugbreak();
            }
        }
        if ( objFreeCount != 3071
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                        381,
                        0,
                        "%s\n\t(objFreeCount) = %i",
                        "(objFreeCount == (sizeof( objAlloced ) / (sizeof( objAlloced[0] ) * (sizeof( objAlloced ) != 4 || sizeof( ob"
                        "jAlloced[0] ) <= 4))) - 1)",
                        objFreeCount) )
        {
            __debugbreak();
        }
    }
}

void __cdecl DB_SaveDObjs()
{
    signed int handle; // [esp+0h] [ebp-Ch]
    signed int handlea; // [esp+0h] [ebp-Ch]
    DObj *obj; // [esp+4h] [ebp-8h]
    DObj *obja; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        for ( handle = 0; handle < 1537; ++handle )
        {
            obj = Com_GetClientDObj(handle, localClientNum);
            if ( obj )
                DObjArchive(obj);
        }
    }
    for ( handlea = 0; handlea < 1024; ++handlea )
    {
        obja = Com_GetServerDObj(handlea);
        if ( obja )
            DObjArchive(obja);
    }
}

void __cdecl DB_LoadDObjs()
{
    signed int handle; // [esp+0h] [ebp-Ch]
    signed int handlea; // [esp+0h] [ebp-Ch]
    DObj *obj; // [esp+4h] [ebp-8h]
    DObj *obja; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        for ( handle = 0; handle < 1537; ++handle )
        {
            obj = Com_GetClientDObj(handle, localClientNum);
            if ( obj )
                DObjUnarchive(obj);
        }
    }
    for ( handlea = 0; handlea < 1024; ++handlea )
    {
        obja = Com_GetServerDObj(handlea);
        if ( obja )
            DObjUnarchive(obja);
    }
}

DObj *__cdecl Com_GetClientDObj(unsigned int handle, int localClientNum)
{
    unsigned int handlea; // [esp+10h] [ebp+8h]

    if ( handle > 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    464,
                    0,
                    "%s\n\t(handle) = %i",
                    "(handle >= 0 && handle < ((((1<<10) + 512)) + 1))",
                    handle) )
    {
        __debugbreak();
    }
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    465,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    handlea = handle + 1537 * localClientNum;
    if ( handlea >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    478,
                    0,
                    "%s\n\t(handle) = %i",
                    "((unsigned)handle < (sizeof( clientObjMap ) / (sizeof( clientObjMap[0] ) * (sizeof( clientObjMap ) != 4 || siz"
                    "eof( clientObjMap[0] ) <= 4))))",
                    handlea) )
    {
        __debugbreak();
    }
    if ( (unsigned int)clientObjMap[handlea] >= 0xC00
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dobj_management.cpp",
                    479,
                    0,
                    "%s\n\t(clientObjMap[handle]) = %i",
                    "((unsigned)clientObjMap[handle] < (2048+1024))",
                    clientObjMap[handlea]) )
    {
        __debugbreak();
    }
    if ( clientObjMap[handlea] )
        return (DObj *)&objBuf[clientObjMap[handlea]];
    else
        return 0;
}

