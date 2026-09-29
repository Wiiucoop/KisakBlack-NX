#include "sv_game.h"
#include <bgame/bg_local.h>
#include <server_mp/sv_main_mp.h>
#include <client_mp/sv_client_mp.h>
#include <server_mp/sv_init_mp.h>
#include <qcommon/cm_load.h>
#include <qcommon/dobj_management.h>
#include "sv_world.h"
#include <qcommon/cm_test.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/dvar_cmds.h>
#include <universal/com_memory.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>
#include <server_mp/sv_ccmds_mp.h>
#include <universal/com_files.h>
#include <qcommon/com_bsp_load_obj.h>
#include <client/cl_debugdata.h>
#include <qcommon/threads.h>
#include <game/g_load_utils.h>
#include <win32/win_main.h>
#include <win32/win_shared.h>
#include <game/g_svcmds.h>

int boxVerts_1[24][3] =
{
  { 0, 0, 0 },
  { 1, 0, 0 },
  { 0, 0, 0 },
  { 0, 1, 0 },
  { 1, 1, 0 },
  { 1, 0, 0 },
  { 1, 1, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 },
  { 1, 0, 1 },
  { 0, 0, 1 },
  { 0, 1, 1 },
  { 1, 1, 1 },
  { 1, 0, 1 },
  { 1, 1, 1 },
  { 0, 1, 1 },
  { 0, 0, 0 },
  { 0, 0, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 1, 0 },
  { 1, 1, 1 }
};

char *g_sv_skel_memory_start;
char g_sv_skel_memory[262144];

playerState_s *__cdecl SV_GameClientNum(int num)
{
    return (playerState_s *)((char *)sv.gameClients + num * sv.gameClientSize);
}

svEntity_s *__cdecl SV_SvEntityForGentity(const gentity_s *gEnt)
{
    if ( !gEnt || gEnt->s.number >= 1024 )
    {
        if ( gEnt )
            Com_Error(ERR_DROP, "SV_SvEntityForGentity: bad gEnt %d", gEnt->s.number);
        else
            Com_Error(ERR_DROP, "SV_SvEntityForGentity: bad gEnt %d", 0);
    }

    return &sv.svEntities[gEnt->s.number];
}

gentity_s *__cdecl SV_GentityNum(int num)
{
    return (gentity_s *)((char *)sv.gentities + num * sv.gentitySize);
}

gentity_s *__cdecl SV_GEntityForSvEntity(svEntity_s *svEnt)
{
    return SV_GentityNum(svEnt - sv.svEntities);
}

void __cdecl SV_GameSendServerCommand(int clientNum, svscmd_type type, const char *text)
{
    if ( clientNum == -1 )
    {
        SV_SendServerCommand(0, type, "%s", text);
    }
    else
    {
        if ( (com_maxclients->current.integer < 1 || com_maxclients->current.integer > 32)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                        152,
                        0,
                        "%s\n\t(com_maxclients->current.integer) = %i",
                        "(com_maxclients->current.integer >= 1 && com_maxclients->current.integer <= 32)",
                        com_maxclients->current.integer) )
        {
            __debugbreak();
        }
        if ( clientNum >= 0 && clientNum < com_maxclients->current.integer )
            SV_SendServerCommand(&svs.clients[clientNum], type, "%s", text);
    }
}

void __cdecl SV_GameDropClient(int clientNum, const char *reason)
{
    if ( (com_maxclients->current.integer < 1 || com_maxclients->current.integer > 32)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                    164,
                    0,
                    "%s\n\t(com_maxclients->current.integer) = %i",
                    "(com_maxclients->current.integer >= 1 && com_maxclients->current.integer <= 32)",
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( clientNum >= 0 && clientNum < com_maxclients->current.integer )
        SV_DropClient(&svs.clients[clientNum], reason, 1, 1);
}

void __cdecl SV_SetMapCenter(float *mapCenter)
{
    char *v1; // eax

    svs.mapCenter[0] = *mapCenter;
    svs.mapCenter[1] = mapCenter[1];
    svs.mapCenter[2] = mapCenter[2];
    svs.mapCenter[0] = (float)(int)svs.mapCenter[0];
    svs.mapCenter[1] = (float)(int)svs.mapCenter[1];
    svs.mapCenter[2] = (float)(int)svs.mapCenter[2];
    v1 = va("%g %g %g", svs.mapCenter[0], svs.mapCenter[1], svs.mapCenter[2]);
    SV_SetConfigstring(13, v1);
}

float (*__cdecl SV_GetMapCenter())[3]
{
    return (float (*)[3])svs.mapCenter;
}

void __cdecl SV_SetGameEndTime(int gameEndTime)
{
    char *v1; // eax
    char lastGameEndTime[12]; // [esp+0h] [ebp-10h] BYREF

    SV_GetConfigstring(0xCu, lastGameEndTime, 12);
    if ( (int)abs(atoi(lastGameEndTime) - gameEndTime) > 500 )
    {
        v1 = va("%i", gameEndTime);
        SV_SetConfigstring(12, v1);
    }
}

void __cdecl SV_SetTimeScale(float endTimeScale, int endTime)
{
    char *v2; // eax

    v2 = va("%g %d", endTimeScale, endTime);
    SV_SetConfigstring(14, v2);
}

char __cdecl SV_SetBrushModel(gentity_s *ent)
{
    DObj *obj; // [esp+8h] [ebp-34h]
    float mins[3]; // [esp+Ch] [ebp-30h] BYREF
    float mx[3]; // [esp+18h] [ebp-24h] BYREF
    float maxs[3]; // [esp+24h] [ebp-18h] BYREF
    float mn[3]; // [esp+30h] [ebp-Ch] BYREF
    int savedregs; // [esp+3Ch] [ebp+0h] BYREF

    if ( !ent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 213, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( !CM_ClipHandleIsValid(ent->s.index.brushmodel) )
        return 0;
    CM_ModelBounds(ent->s.index.brushmodel, mins, maxs);
    ent->r.mins[0] = mins[0];
    ent->r.mins[1] = mins[1];
    ent->r.mins[2] = mins[2];
    ent->r.maxs[0] = maxs[0];
    ent->r.maxs[1] = maxs[1];
    ent->r.maxs[2] = maxs[2];
    ent->r.bmodel = 1;
    ent->r.contents = CM_ContentsOfModel(ent->s.index.brushmodel);
    obj = Com_GetServerDObj(ent->s.number);
    if ( obj )
    {
        if ( (ent->r.svFlags & 4) != 0 )
        {
            DObjCalcBounds(obj, mn, mx);
            Vec3Min(ent->r.mins, mn, ent->r.mins);
            Vec3Max(ent->r.maxs, mx, ent->r.maxs);
            ent->r.contents |= DObjGetContents(obj);
        }
    }
    SV_LinkEntity(ent);
    return 1;
}

bool __cdecl SV_inSnapshot(const float *origin, int iEntityNum)
{
    int clientcluster; // [esp+8h] [ebp-24h]
    float fogOpaqueDistSqrd; // [esp+Ch] [ebp-20h]
    svEntity_s *svEnt; // [esp+10h] [ebp-1Ch]
    int l; // [esp+14h] [ebp-18h]
    unsigned int leafnum; // [esp+18h] [ebp-14h]
    gentity_s *ent; // [esp+1Ch] [ebp-10h]
    int i; // [esp+20h] [ebp-Ch]
    unsigned __int8 *bitvector; // [esp+24h] [ebp-8h]

    ent = (gentity_s *)((char *)sv.gentities + iEntityNum * sv.gentitySize);
    if ( !ent->r.linked )
        return 0;
    if ( ent->r.broadcastTime )
        return 1;
    if ( (ent->r.svFlags & 1) != 0 )
        return 0;
    if ( (ent->r.svFlags & 0x18) != 0 )
        return 1;
    svEnt = SV_SvEntityForGentity(ent);
    leafnum = CM_PointLeafnum(origin);
    if ( !svEnt->numClusters )
        return 0;
    clientcluster = CM_LeafCluster(leafnum);
    bitvector = CM_ClusterPVS(clientcluster);
    l = 0;
    for ( i = 0; i < svEnt->numClusters; ++i )
    {
        l = svEnt->clusternums[i];
        if ( ((1 << (l & 7)) & bitvector[l >> 3]) != 0 )
            break;
    }
    if ( i == svEnt->numClusters )
    {
        if ( !svEnt->lastCluster )
            return 0;
        while ( l <= svEnt->lastCluster && ((1 << (l & 7)) & bitvector[l >> 3]) == 0 )
            ++l;
        if ( l == svEnt->lastCluster )
            return 0;
    }
    fogOpaqueDistSqrd = G_GetFogOpaqueDistSqrd();
    return fogOpaqueDistSqrd == 3.4028235e38
            || !BoxDistSqrdExceeds(ent->r.absmin, ent->r.absmax, origin, fogOpaqueDistSqrd);
}

bool __cdecl SV_EntityContact(const float *mins, const float *maxs, const gentity_s *gEnt)
{
    unsigned int model; // [esp+20h] [ebp-50h]
    float dist; // [esp+24h] [ebp-4Ch]
    float dista; // [esp+24h] [ebp-4Ch]
    trace_t trace; // [esp+2Ch] [ebp-44h] BYREF
    float center[2]; // [esp+68h] [ebp-8h]

    if ( (gEnt->r.svFlags & 0x60) != 0 )
    {
        if ( (gEnt->r.svFlags & 0x20) != 0 )
        {
            if ( gEnt->r.mins[2] != 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 336, 0, "%s", "!gEnt->r.mins[2]") )
            {
                __debugbreak();
            }
            if ( gEnt->r.currentOrigin[2] < maxs[2] )
            {
                if ( mins[2] < (float)(gEnt->r.currentOrigin[2] + gEnt->r.maxs[2]) )
                {
                    center[0] = *mins + *maxs;
                    center[1] = mins[1] + maxs[1];
                    center[0] = 0.5 * center[0];
                    center[1] = 0.5 * center[1];
                    dist = (float)(*maxs - center[0]) + gEnt->r.maxs[0];
                    return (float)(dist * dist) > (float)((float)((float)(center[0] - gEnt->r.currentOrigin[0])
                                                                                                            * (float)(center[0] - gEnt->r.currentOrigin[0]))
                                                                                            + (float)((float)(center[1] - gEnt->r.currentOrigin[1])
                                                                                                            * (float)(center[1] - gEnt->r.currentOrigin[1])));
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if ( (gEnt->r.svFlags & 0x40) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                            349,
                            0,
                            "%s",
                            "gEnt->r.svFlags & SVF_DISK") )
            {
                __debugbreak();
            }
            center[0] = *mins + *maxs;
            center[1] = mins[1] + maxs[1];
            center[0] = 0.5 * center[0];
            center[1] = 0.5 * center[1];
            dista = (float)((float)(*maxs - center[0]) + gEnt->r.maxs[0]) - 64.0;
            return (float)((float)((float)(center[0] - gEnt->r.currentOrigin[0])
                                                     * (float)(center[0] - gEnt->r.currentOrigin[0]))
                                     + (float)((float)(center[1] - gEnt->r.currentOrigin[1])
                                                     * (float)(center[1] - gEnt->r.currentOrigin[1]))) >= (float)(dista * dista);
        }
    }
    else
    {
        model = SV_ClipHandleForEntity(gEnt);
        CM_TransformedBoxTraceExternal(
            &trace,
            vec3_origin,
            vec3_origin,
            mins,
            maxs,
            model,
            -1,
            gEnt->r.currentOrigin,
            gEnt->r.currentAngles);
        return trace.startsolid;
    }
}

void __cdecl SV_GetServerinfo(char *buffer, int bufferSize)
{
    char *v2; // eax

    if ( bufferSize < 1 )
        Com_Error(ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize);
    v2 = Dvar_InfoString(0, 4);
    I_strncpyz(buffer, v2, bufferSize);
}

void __cdecl SV_LocateGameData(
                gentity_s *gEnts,
                int numGEntities,
                int sizeofGEntity_t,
                playerState_s *clients,
                int sizeofGameClient)
{
    sv.gentities = gEnts;
    sv.gentitySize = sizeofGEntity_t;
    sv.num_entities = numGEntities;
    sv.gameClients = clients;
    sv.gameClientSize = sizeofGameClient;
}

void __cdecl SV_GetUsercmd(int clientNum, usercmd_s *cmd)
{
    if ( clientNum < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 534, 0, "%s", "clientNum >= 0") )
    {
        __debugbreak();
    }
    if ( (com_maxclients->current.integer < 1 || com_maxclients->current.integer > 32)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                    535,
                    0,
                    "%s\n\t(com_maxclients->current.integer) = %i",
                    "(com_maxclients->current.integer >= 1 && com_maxclients->current.integer <= 32)",
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                    536,
                    0,
                    "%s",
                    "clientNum < com_maxclients->current.integer") )
    {
        __debugbreak();
    }
    memcpy(cmd, &svs.clients[clientNum].lastUsercmd, sizeof(usercmd_s));
}

XModel *__cdecl SV_XModelGet(char *name)
{
    return XModelPrecache(
                     name,
                     (void *(__cdecl *)(int))SV_AllocXModelPrecache,
                     (void *(__cdecl *)(int))SV_AllocXModelPrecacheColl);
}

unsigned __int8 *__cdecl SV_AllocXModelPrecache(unsigned int size)
{
    return Hunk_Alloc(size, "SV_AllocXModelPrecache", 22);
}

unsigned __int8 *__cdecl SV_AllocXModelPrecacheColl(unsigned int size)
{
    return Hunk_Alloc(size, "SV_AllocXModelPrecacheColl", 28);
}

void __cdecl SV_DObjDumpInfo(gentity_s *ent)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    if ( com_developer->current.integer )
    {
        obj = Com_GetServerDObj(ent->s.number);
        if ( obj )
            DObjDumpInfo(obj);
        else
            Com_Printf(15, "no model.\n");
    }
}

void __cdecl SV_ResetSkeletonCache()
{
    if (!++sv.skelTimeStamp)
        sv.skelTimeStamp = 1;
    g_sv_skel_memory_start = (char *)((uintptr_t)&g_sv_skel_memory[15] & ~(uintptr_t)15);   // nx-port: was (unsigned int)
    sv.skelMemPos = 0;
}

bool __cdecl SV_DObjCreateSkelForBone(DObj *obj, int boneIndex)
{
    char *buf; // [esp+0h] [ebp-8h]
    unsigned int len; // [esp+4h] [ebp-4h]

    if (DObjSkelExists(obj, sv.skelTimeStamp))
        return DObjSkelIsBoneUpToDate(obj, boneIndex);
    len = DObjGetAllocSkelSize(obj);
    buf = SV_AllocSkelMemory(len);
    DObjCreateSkel(obj, buf, sv.skelTimeStamp);
    return 0;
}

char *__cdecl SV_AllocSkelMemory(unsigned int size)
{
    char *result; // [esp+0h] [ebp-4h]
    unsigned int sizea; // [esp+Ch] [ebp+8h]

    if (!size && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 628, 0, "%s", "size"))
        __debugbreak();
    sizea = (size + 15) & 0xFFFFFFF0;
    if (sizea > 0x3FFF0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
            631,
            0,
            "%s",
            "size <= sizeof( g_sv_skel_memory ) - SKEL_MEM_ALIGNMENT"))
    {
        __debugbreak();
    }
    if (!g_sv_skel_memory_start
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
            633,
            0,
            "%s",
            "g_sv_skel_memory_start"))
    {
        __debugbreak();
    }
    while (1)
    {
        result = &g_sv_skel_memory_start[sv.skelMemPos];
        sv.skelMemPos += sizea;
        if (sv.skelMemPos <= 262128)
            break;
        static int warnCount_2;
        if (warnCount_2 != sv.skelTimeStamp)
        {
            warnCount_2 = sv.skelTimeStamp;
            Com_PrintWarning(15, "WARNING: SV_SKEL_MEMORY_SIZE exceeded\n");
        }
        SV_ResetSkeletonCache();
    }
    if (!result && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 641, 0, "%s", "result"))
        __debugbreak();
    return result;
}

int __cdecl SV_DObjCreateSkelForBones(DObj *obj, int *partBits)
{
    char *buf; // [esp+0h] [ebp-8h]
    unsigned int len; // [esp+4h] [ebp-4h]

    if (DObjSkelExists(obj, sv.skelTimeStamp))
        return DObjSkelAreBonesUpToDate(obj, partBits);
    len = DObjGetAllocSkelSize(obj);
    buf = SV_AllocSkelMemory(len);
    DObjCreateSkel(obj, buf, sv.skelTimeStamp);
    return 0;
}

void __cdecl SV_DObjUpdateServerTime(gentity_s *ent, float dtime, int bNotify)
{
    DObj *obj; // [esp+8h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( obj )
        DObjUpdateServerInfo(obj, dtime, bNotify);
}

void __cdecl SV_DObjInitServerTime(gentity_s *ent, float dtime)
{
    DObj *obj; // [esp+4h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( obj )
        DObjInitServerTime(obj, dtime);
}

int __cdecl SV_DObjGetBoneIndex(const gentity_s *ent, unsigned int boneName)
{
    const DObj *obj; // [esp+0h] [ebp-8h]
    unsigned __int8 index; // [esp+7h] [ebp-1h] BYREF

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj )
        return -1;
    index = -2;
    if ( DObjGetBoneIndex(obj, boneName, &index, -1) )
        return index;
    else
        return -1;
}

DObjAnimMat *__cdecl SV_DObjGetMatrixArray(const gentity_s *ent)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 783, 0, "%s", "obj") )
        __debugbreak();
    return DObjGetRotTransArray(obj);
}

void __cdecl SV_DObjDisplayAnim(gentity_s *ent, const char *header)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( obj )
        DObjDisplayAnim(obj, header);
}

DObjAnimMat *__cdecl SV_DObjGetRotTransArray(const gentity_s *ent)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 806, 0, "%s", "obj") )
        __debugbreak();
    return DObjGetRotTransArray(obj);
}

int __cdecl SV_DObjSetRotTransIndex(const gentity_s *ent, int *partBits, int boneIndex)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 817, 0, "%s", "obj") )
        __debugbreak();
    return DObjSetRotTransIndex(obj, partBits, boneIndex);
}

void __cdecl SV_DObjGetBounds(gentity_s *ent, float *mins, float *maxs)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 839, 0, "%s", "obj") )
        __debugbreak();
    DObjGetBounds(obj, mins, maxs);
}

XAnimTree_s *__cdecl SV_DObjGetTree(gentity_s *ent)
{
    const DObj *obj; // [esp+4h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( obj )
        return DObjGetTree(obj);
    else
        return 0;
}

void __cdecl SV_XModelDebugBoxes(gentity_s *ent, const float *color, int *partBits, int duration)
{
    DObjAnimMat *boneMatrix; // [esp+58h] [ebp-330h]
    unsigned int j; // [esp+5Ch] [ebp-32Ch]
    XBoneInfo *boneInfoArray[160]; // [esp+60h] [ebp-328h] BYREF
    int numBones; // [esp+2E0h] [ebp-A8h]
    int boneIndex; // [esp+2E4h] [ebp-A4h]
    DObj *obj; // [esp+2E8h] [ebp-A0h]
    float start[3]; // [esp+2ECh] [ebp-9Ch] BYREF
    float end[3]; // [esp+2F8h] [ebp-90h] BYREF
    int size; // [esp+304h] [ebp-84h]
    float boneAxis[4][3]; // [esp+308h] [ebp-80h] BYREF
    int localBoneIndex; // [esp+338h] [ebp-50h]
    int (*tempBoxVerts)[3]; // [esp+33Ch] [ebp-4Ch]
    float org[3]; // [esp+340h] [ebp-48h] BYREF
    XBoneInfo *boneInfo; // [esp+34Ch] [ebp-3Ch]
    float axis[3][3]; // [esp+350h] [ebp-38h] BYREF
    float vec[3]; // [esp+374h] [ebp-14h] BYREF
    int modelCount; // [esp+380h] [ebp-8h]
    int modelIndex; // [esp+384h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 887, 0, "%s", "obj") )
        __debugbreak();
    numBones = DObjNumBones(obj);
    iassert(numBones <= DOBJ_MAX_PARTS);

    DObjGetBoneInfo(obj, boneInfoArray);
    boneMatrix = DObjGetRotTransArray(obj);
    AnglesToAxis(ent->r.currentAngles, axis);
    modelCount = DObjGetNumModels(obj);
    boneIndex = 0;
    for ( modelIndex = 0; modelIndex < modelCount; ++modelIndex )
    {
        size = XModelNumBones(DObjGetModel(obj, modelIndex));
        if ( DObjIgnoreCollision(obj, modelIndex) )
        {
            boneIndex += size;
            boneMatrix += size;
        }
        else
        {
            localBoneIndex = 0;
            while ( localBoneIndex < size )
            {
                if ( !partBits || (partBits[boneIndex >> 5] & (0x80000000 >> (boneIndex & 0x1F))) != 0 )
                {
                    boneInfo = boneInfoArray[boneIndex];
                    tempBoxVerts = boxVerts_1;
                    ConvertQuatToMat(boneMatrix, boneAxis);
                    boneAxis[3][0] = boneMatrix->trans[0];
                    boneAxis[3][1] = boneMatrix->trans[1];
                    boneAxis[3][2] = boneMatrix->trans[2];
                    for ( j = 0; j < 0xC; ++j )
                    {
                        org[0] = boneInfo->bounds[(*tempBoxVerts)[0]][0];
                        org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                        org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                        MatrixTransformVector43(org, boneAxis, vec);
                        MatrixTransformVector(vec, axis, start);
                        start[0] = start[0] + ent->r.currentOrigin[0];
                        start[1] = start[1] + ent->r.currentOrigin[1];
                        start[2] = start[2] + ent->r.currentOrigin[2];
                        org[0] = boneInfo->bounds[(*++tempBoxVerts)[0]][0];
                        org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                        org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                        MatrixTransformVector43(org, boneAxis, vec);
                        MatrixTransformVector(vec, axis, end);
                        end[0] = end[0] + ent->r.currentOrigin[0];
                        end[1] = end[1] + ent->r.currentOrigin[1];
                        end[2] = end[2] + ent->r.currentOrigin[2];
                        ++tempBoxVerts;
                        CL_AddDebugLine(start, end, color, 0, duration);
                    }
                }
                ++localBoneIndex;
                ++boneMatrix;
                ++boneIndex;
            }
        }
    }
}

bool __cdecl SV_MapExists(char *name)
{
    char fullpath[256]; // [esp+0h] [ebp-108h] BYREF
    const char *basename; // [esp+104h] [ebp-4h]

    basename = SV_GetMapBaseName(name);
    Com_GetBspFilename(fullpath, 0x100u, basename);
    return FS_ReadFile(fullpath, 0) >= 0;
}

bool __cdecl SV_DObjExists(gentity_s *ent)
{
    return Com_GetServerDObj(ent->s.number) != 0;
}

void __cdecl SV_track_shutdown()
{
    //track_shutdown(2);
}

void __cdecl SV_SavePaths(unsigned __int8 *buf, unsigned int size)
{
    Com_SaveLump(LUMP_PATHCONNECTIONS, buf, size, COM_SAVE_LUMP_AND_REOPEN);
    sv.checksum = Com_GetBspChecksum();
}

int __cdecl SV_GetGuid(int clientNum)
{
    if ( !com_maxclients
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 1001, 0, "%s", "com_maxclients") )
    {
        __debugbreak();
    }
    if ( clientNum >= 0 && clientNum < com_maxclients->current.integer )
        return svs.clients[clientNum].guid;
    else
        return 0;
}

int __cdecl SV_GetClientPing(unsigned int clientNum)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                    1010,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return svs.clients[clientNum].ping;
}

bool __cdecl SV_IsLocalClient(unsigned int clientNum)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                    1018,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return NET_IsLocalAddress(svs.clients[clientNum].header.netchan.remoteAddress);
}

void __cdecl SV_ShutdownGameProgs()
{
    Com_SyncThreads();
    sv.state = SS_DEAD;
    if ( gameInitialized )
    {
        SV_ShutdownGameVM(1);
        gameInitialized = 0;
    }
}

void __cdecl SV_SetGametype()
{
    char gametype[64]; // [esp+0h] [ebp-48h] BYREF
    char *s; // [esp+44h] [ebp-4h]

    // HISTORICAL NOTE (SP) -- superseded by the "RESOLVED (SP)" block at the end of this
    // comment and by the #ifdef KISAK_SP below. Kept because it records how the SP default was
    // pinned down. Read it as history, not as a description of the current code: the opening
    // "not changed" framing and the "Ghidra is unavailable in this session" line were both
    // true when written and are both false now.
    // What was established at the time:
    //   - "dm"/"tdm" exist in no SP zone. SP's maps/gametypes/ holds exactly cmp, arc, bnk, zom,
    //     sop (.txt for all five, .gsc for cmp only, and that cmp.gsc is empty - see
    //     GScr_LoadGameTypeScript). MP's code_post_gfx_mp.ff does carry
    //     maps/mp/gametypes/{dm,dom,sd,sab,tdm,koth,twar,...}.txt, so this is a clean
    //     both-directions control, not a scanning artefact.
    //   - Scr_IsValidGameType currently returns false for EVERYTHING on SP because the gametype
    //     list is parsed from a rawfile whose path is itself disputed between the two audits
    //     (see the TODO(SP) in Scr_ParseGameTypeList_FastFile, g_scr_main_mp.cpp). So this
    //     function always takes the fallback and stomps g_gametype to "dm".
    //   - Even with that path corrected, "cmp" would STILL fail validation: SP's shipped
    //     maps/gametypes/_gametypes.txt payload is literally "zom\r\nsop" (8 bytes). So retail
    //     SP's SV_SetGametype must differ structurally - different default, no validity check,
    //     or a different list source - and no static evidence distinguishes those.
    // Settled by: decompiling SV_SetGametype in BlackOps.exe and reading (a) the g_gametype
    // registration default and (b) whether the Scr_IsValidGameType/"dm" fallback exists at all.
    // Ghidra is unavailable in this session.
    // Currently harmless: the resulting nonsense gametype only feeds GScr_LoadGameTypeScript,
    // which is bEnforceExists=0 under KISAK_SP, and CG_LoadHudMenu's va("ui/hud_%s.txt", ...),
    // which warns rather than fatals.
    // RESOLVED (SP), 2026-08-09 -- the TODO above is answered; both halves now have direct
    // binary evidence, so the "no static evidence distinguishes those" note no longer holds.
    //   Fact 1: SP's g_* dvar registration block at 0x005715e0 opens with
    //           _Dvar_RegisterString("g_gametype", "cmp", 0x24, "") -- same 0x24 flags as
    //           here, default "cmp" not "tdm". (Description is "" in SP, not "Game Type".)
    //   Fact 2: SP's own fallback literal at 0x009eaae4 reads, byte for byte,
    //           "g_gametype %s is not a valid gametype, defaulting to cmp\n" -- so the
    //           Scr_IsValidGameType check and its fallback DO exist in SP, and the fallback
    //           value is "cmp" as well. (Referenced from exactly one function, 0x00549ac0 --
    //           which the live Ghidra database now names SV_SetGametype, its plate re-deriving
    //           this same RESOLVED block from scratch and confirming the transcription exact.)
    // Both facts read from BlackOps.exe directly. This does NOT resolve the second half of
    // the note above -- SP's shipped maps/gametypes/_gametypes.txt payload is still
    // "zom\r\nsop", so "cmp" will still fail Scr_IsValidGameType and still take the fallback.
    // The fallback now lands on "cmp" instead of "dm", which is the gametype whose script
    // SP actually ships (maps/gametypes/cmp.gsc), so the downstream
    // GScr_LoadGameTypeScript/CG_LoadHudMenu lookups stop asking for a nonexistent "dm".
#ifdef KISAK_SP
    _Dvar_RegisterString("g_gametype", "cmp", 0x24u, "");
#else
    _Dvar_RegisterString("g_gametype", "tdm", 0x24u, "Game Type");
#endif
    // CORRECTION 2026-08-09: the previous note here claimed no ui_gametype seed could be
    // found in SV_SetGametype (Ghidra 0x00549ac0). That was wrong -- the seed is plainly there,
    // immediately after
    // the registration above and before the select below:
    //     pcVar2 = Dvar_GetString("ui_gametype");
    //     if (*pcVar2 == '\0') pcVar2 = "cmp";
    //     Dvar_SetString(sv_gametype, pcVar2);
    // (SV_SetGametype also ends with an extra unmodelled call, FUN_008338f0(&gametype) --
    // 0x008338f0 is still FUN_* in Ghidra as of 2026-08-26, so that call stays unidentified.)
    // Implemented for in-process Zombies loading: SV_Map_f now selects ui_gametype="zom" before
    // SV_SpawnServer. Without retail's seed below, this function would keep the creating dvar's
    // "cmp" value and CCS_LoadConstantConfigStrings would ask for the nonexistent *_cmp.csv even
    // though zombie_theater.ff contains *_zom.csv. This is a direct transcription of retail
    // BlackOps.exe 0x00549AD9..0x00549AFC, not a map-name inference: read ui_gametype, replace an
    // empty value with "cmp", and write the result to sv_gametype before validation.
#ifdef KISAK_SP
    const char *uiGameType = Dvar_GetString("ui_gametype");
    if ( !*uiGameType )
        uiGameType = "cmp";
    Dvar_SetString((dvar_s *)sv_gametype, uiGameType);
#endif
    if (com_sv_running->current.enabled && G_GetSavePersist())
        I_strncpyz(gametype, sv.gametype, 64);
    else
        I_strncpyz(gametype, sv_gametype->current.string, 64);
    for (s = gametype; *s; ++s)
        *s = tolower(*s);
    if (!Scr_IsValidGameType(gametype))
    {
#ifdef KISAK_SP
        Com_Printf(15, "g_gametype %s is not a valid gametype, defaulting to cmp\n", gametype);
        strcpy(gametype, "cmp");
#else
        Com_Printf(15, "g_gametype %s is not a valid gametype, defaulting to dm\n", gametype);
        strcpy(gametype, "dm");
#endif
    }
    Dvar_SetString((dvar_s*)sv_gametype, gametype);
}

void __cdecl SV_ShutdownGameVM(int clearScripts)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp", 1295, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    PROF_SCOPED("SV_ShutdownGameVM");
    G_ShutdownGame(clearScripts);
}

void __cdecl    SV_RestartGameProgs(int savepersist)
{
    Com_SyncThreads();
    SV_ShutdownGameVM(0);
    com_fixedConsolePosition = 0;
    SV_InitGameVM(1, savepersist == 0);
}

void __cdecl    SV_InitGameVM(int restart, int registerDvars)
{
    int i; // [esp+Ch] [ebp-4h]

    PROF_SCOPED("SV_InitGameVM");

    G_ResetEntityParsePoint();
    SV_ResetSkeletonCache();

    if ( (com_maxclients->current.integer < 1 || com_maxclients->current.integer > 32)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_game.cpp",
                    1217,
                    0,
                    "%s\n\t(com_maxclients->current.integer) = %i",
                    "(com_maxclients->current.integer >= 1 && com_maxclients->current.integer <= 32)",
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }

    for ( i = 0; i < com_maxclients->current.integer; ++i )
        svs.clients[i].gentity = 0;

    Sys_LoadingKeepAlive();
    G_InitGame(svs.time, Sys_MillisecondsRaw(), restart, registerDvars);

    if (IsDedicatedServer())
    {
        float sprinttime = Dvar_GetFloat("scr_player_sprinttime");
        Dvar_SetFromStringByNameFromSource("player_sprintTime", va("%f", sprinttime), DVAR_SOURCE_SCRIPT, 0);
    }

    SV_SetConfigstring(3, va("%i", sv_serverId_value));
    Sys_LoadingKeepAlive();

    if (IsDedicatedServer())
    {
        Com_DvarDump(6, NULL);
    }
}

void __cdecl    SV_InitGameProgs(int savepersist)
{
    gameInitialized = 1;
    SV_InitGameVM(0, savepersist == 0);
}

int __cdecl SV_GameCommand()
{
    if ( sv.state == SS_GAME )
        return ConsoleCommand();
    else
        return 0;
}

const char *__cdecl SV_Archived_Dvar_GetVariantString(const char *dvarName)
{
    return Dvar_GetVariantString(dvarName);
}

const char *__cdecl SV_Archived_Dvar_GetVariantString(int dvarHash)
{
    return Dvar_GetVariantString(dvarHash);
}

