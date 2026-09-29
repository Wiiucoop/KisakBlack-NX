#include "cg_animscripted_mp.h"
#include "cg_local_mp.h"
#include "cg_ents_mp.h"
#include <qcommon/dobj_management.h>
#include <universal/com_math_anglevectors.h>
#include <clientscript/cscr_memorytree.h>
#include <client_mp/cl_cgame_mp.h>
#include <clientscript/cscr_stringlist.h>
#ifdef KISAK_SP
#include <xanim/xanim.h>
#include <xanim/dobj.h>
#include <xanim/xanim_clientnotify.h>
#include <qcommon/common.h>
#include <mutex>
#include <cstring>
#include "cg_sp_anim_snapshot.inl"
#endif

#ifdef KISAK_SP
namespace
{
constexpr unsigned int SP_ANIM_COMMAND_CAPACITY = 1024;

struct StoredAnimCommand_SP
{
    AnimCommand_SP command;
    bool occupied;
    bool processed;
};

StoredAnimCommand_SP g_animCommands_SP[SP_ANIM_COMMAND_CAPACITY] = {};
unsigned int g_nextAnimCommandSequence_SP = 1;
int g_lastAnimCommandServerTime_SP = -1;
std::mutex g_animCommandMutex_SP;
int g_lastAppliedAnimIndex_SP[1024] = {};
int g_lastAppliedAnimType_SP[1024] = {};

bool CG_ApplyAnimCommand_SP(int localClientNum, const AnimCommand_SP &command, DObj *obj)
{
    if ( !obj || !obj->localTree || !obj->localTree->anims )
        return false;

    XAnimTree_s *tree = obj->localTree;
    XAnim_s *anims = tree->anims;
    if ( command.animIndex < 0 || static_cast<unsigned int>(command.animIndex) >= anims->size )
    {
        Com_PrintWarning(
            15,
            "SP anim command rejected: client %d ent %d type %d anim %d outside tree size %u\n",
            localClientNum,
            command.entNum,
            command.type,
            command.animIndex,
            anims->size);
        return false;
    }

    int error = 0;
    const unsigned int notifyType = command.weight > 0.001f ? 2 : 0;
    XAnimClientNotifyList notifyList;
    DObjSetClientNotifies(&notifyList);
    switch ( command.type )
    {
        case 1:
            if ( command.flags & 1 )
                XAnimClearTreeGoalWeights(tree, command.animIndex, command.goalTime, -1);
            else
                XAnimClearGoalWeight(tree, command.animIndex, command.goalTime, static_cast<unsigned short>(-1));
            break;
        case 2:
            XAnimClearTreeGoalWeightsStrict(tree, command.animIndex, command.goalTime, -1);
            break;
        case 3:
            if ( command.flags & 1 )
            {
                error = XAnimSetCompleteGoalWeight(
                    obj,
                    command.animIndex,
                    command.weight,
                    command.goalTime,
                    command.rate,
                    0,
                    notifyType,
                    (command.flags & 2) != 0,
                    -1);
            }
            else
            {
                error = XAnimSetGoalWeight(
                    obj,
                    command.animIndex,
                    command.weight,
                    command.goalTime,
                    command.rate,
                    0,
                    notifyType,
                    (command.flags & 2) != 0,
                    -1);
            }
            break;
        case 4:
            if ( command.flags & 1 )
            {
                error = XAnimSetCompleteGoalWeightKnob(
                    obj,
                    command.animIndex,
                    command.weight,
                    command.goalTime,
                    command.rate,
                    0,
                    notifyType,
                    (command.flags & 2) != 0,
                    -1);
            }
            else
            {
                error = XAnimSetGoalWeightKnob(
                    obj,
                    command.animIndex,
                    command.weight,
                    command.goalTime,
                    command.rate,
                    0,
                    notifyType,
                    (command.flags & 2) != 0,
                    -1);
            }
            break;
        case 5:
            if ( command.rootAnimIndex < 0 || static_cast<unsigned int>(command.rootAnimIndex) >= anims->size )
            {
                DObjClearClientNotifies();
                return false;
            }
            if ( command.flags & 1 )
            {
                error = XAnimSetCompleteGoalWeightKnobAll(
                    obj,
                    command.animIndex,
                    command.rootAnimIndex,
                    command.weight,
                    command.goalTime,
                    command.rate,
                    0,
                    notifyType,
                    (command.flags & 2) != 0,
                    -1);
            }
            else
            {
                error = XAnimSetGoalWeightKnobAll(
                    obj,
                    command.animIndex,
                    command.rootAnimIndex,
                    command.weight,
                    command.goalTime,
                    command.rate,
                    0,
                    notifyType,
                    (command.flags & 2) != 0,
                    -1);
            }
            break;
        case 6:
            XAnimSetTime(tree, command.animIndex, command.goalTime, 0xFFFFu);
            break;
        default:
            Com_PrintWarning(15, "SP anim command rejected: unknown type %d\n", command.type);
            DObjClearClientNotifies();
            return true;
    }

    const int clientTime = CG_GetLocalClientGlobals(localClientNum)->time;
    const int elapsedMs = clientTime > command.serverTime ? clientTime - command.serverTime : 0;
    if ( command.type >= 3 && command.type <= 5 )
        XAnimApplyClientCommandCatchup_SP(obj, command.animIndex, elapsedMs);
    const float animTime = command.type >= 3 && command.type <= 6
        ? static_cast<float>(XAnimGetTime(tree, command.animIndex))
        : -1.0f;
    CG_ProcessFakeEntClientNoteTracks(localClientNum, command.entNum);
    const int notifyCount = notifyList.m_numNotifies;
    DObjClearClientNotifies();
    if ( command.entNum >= 0 && command.entNum < 1024 && command.type >= 3 && command.type <= 6 )
    {
        g_lastAppliedAnimIndex_SP[command.entNum] = command.animIndex;
        g_lastAppliedAnimType_SP[command.entNum] = command.type;
    }
    Com_Printf(
        15,
        "SP anim command apply: seq %d client %d ent %d type %d anim %d root %d tree %p weight %.3f blend %.3f rate %.3f flags 0x%x lag %d error %d animTime %.3f notifies %d\n",
        command.sequence,
        localClientNum,
        command.entNum,
        command.type,
        command.animIndex,
        command.rootAnimIndex,
        tree,
        command.weight,
        command.goalTime,
        command.rate,
        command.flags,
        clientTime - command.serverTime,
        error,
        animTime,
        notifyCount);
    return true;
}
}

int __cdecl CG_StoreServerAnimCommand_SP(
    int entNum,
    int serverTime,
    int type,
    unsigned int animIndex,
    unsigned int rootAnimIndex,
    float weight,
    float goalTime,
    float rate,
    int flags)
{
    std::lock_guard<std::mutex> lock(g_animCommandMutex_SP);
    if ( serverTime < g_lastAnimCommandServerTime_SP )
    {
        memset(g_animCommands_SP, 0, sizeof(g_animCommands_SP));
        g_nextAnimCommandSequence_SP = 1;
    }
    g_lastAnimCommandServerTime_SP = serverTime;

    const unsigned int sequence = g_nextAnimCommandSequence_SP++;
    const unsigned int slot = sequence % SP_ANIM_COMMAND_CAPACITY;
    StoredAnimCommand_SP &stored = g_animCommands_SP[slot];
    if ( stored.occupied && !stored.processed )
        Com_PrintWarning(15, "SP anim command ring overwrote unprocessed sequence %d\n", stored.command.sequence);

    stored.command.index = static_cast<int>(slot);
    stored.command.type = type;
    stored.command.serverTime = serverTime;
    stored.command.sequence = static_cast<int>(sequence);
    stored.command.entNum = entNum;
    stored.command.animIndex = static_cast<int>(animIndex);
    stored.command.rootAnimIndex = static_cast<int>(rootAnimIndex);
    stored.command.weight = weight;
    stored.command.goalTime = goalTime;
    stored.command.rate = rate;
    stored.command.flags = flags;
    stored.occupied = true;
    stored.processed = false;

    Com_Printf(
        15,
        "SP anim command store: seq %u slot %u ent %d type %d anim %u root %u weight %.3f time %.3f rate %.3f flags 0x%x serverTime %d\n",
        sequence,
        slot,
        entNum,
        type,
        animIndex,
        rootAnimIndex,
        weight,
        goalTime,
        rate,
        flags,
        serverTime);
    return static_cast<int>(slot);
}

void __cdecl CG_ApplyPendingAnimCommandsForDObj_SP(int localClientNum, int entNum, DObj *obj)
{
    if (CG_ApplyRemoteAnimSnapshot_SP(localClientNum, entNum, obj, true))
        return;
    if ( localClientNum != 0 || !obj || !obj->localTree )
        return;

    std::lock_guard<std::mutex> lock(g_animCommandMutex_SP);
    const unsigned int firstSequence = g_nextAnimCommandSequence_SP > SP_ANIM_COMMAND_CAPACITY
                                     ? g_nextAnimCommandSequence_SP - SP_ANIM_COMMAND_CAPACITY
                                     : 1;
    for ( unsigned int sequence = firstSequence; sequence < g_nextAnimCommandSequence_SP; ++sequence )
    {
        StoredAnimCommand_SP &stored = g_animCommands_SP[sequence % SP_ANIM_COMMAND_CAPACITY];
        if ( stored.occupied
            && static_cast<unsigned int>(stored.command.sequence) == sequence
            && stored.command.entNum == entNum )
            CG_ApplyAnimCommand_SP(localClientNum, stored.command, obj);
    }
}

void __cdecl CG_ApplyPendingAnimCommands_SP(int localClientNum)
{
    if (!com_sv_running->current.enabled)
    {
        for (int entNum = 0; entNum < 1023; ++entNum)
            CG_ApplyRemoteAnimSnapshot_SP(localClientNum, entNum, Com_GetClientDObj(entNum, localClientNum), false);
        return;
    }
    if ( localClientNum != 0 )
        return;

    std::lock_guard<std::mutex> lock(g_animCommandMutex_SP);
    const unsigned int firstSequence = g_nextAnimCommandSequence_SP > SP_ANIM_COMMAND_CAPACITY
                                     ? g_nextAnimCommandSequence_SP - SP_ANIM_COMMAND_CAPACITY
                                     : 1;
    for ( unsigned int sequence = firstSequence; sequence < g_nextAnimCommandSequence_SP; ++sequence )
    {
        StoredAnimCommand_SP &stored = g_animCommands_SP[sequence % SP_ANIM_COMMAND_CAPACITY];
        if ( !stored.occupied
            || static_cast<unsigned int>(stored.command.sequence) != sequence
            || stored.processed )
            continue;
        DObj *obj = Com_GetClientDObj(stored.command.entNum, localClientNum);
        if ( obj && obj->localTree )
            stored.processed = CG_ApplyAnimCommand_SP(localClientNum, stored.command, obj);
    }

    static int lastProbeTime_SP[1024] = {};
    const int clientTime = CG_GetLocalClientGlobals(localClientNum)->time;
    for ( int entNum = 100; entNum <= 120; ++entNum )
    {
        const int animIndex = g_lastAppliedAnimIndex_SP[entNum];
        if ( !animIndex || clientTime - lastProbeTime_SP[entNum] < 1000 )
            continue;
        DObj *obj = Com_GetClientDObj(entNum, localClientNum);
        XAnimTree_s *tree = obj ? obj->localTree : NULL;
        if ( !tree || static_cast<unsigned int>(animIndex) >= tree->anims->size )
            continue;
        lastProbeTime_SP[entNum] = clientTime;
        Com_Printf(
            15,
            "SP client anim probe: time %d ent %d type %d anim %d animTime %.3f weight %.3f tree %p\n",
            clientTime,
            entNum,
            g_lastAppliedAnimType_SP[entNum],
            animIndex,
            XAnimGetTime(tree, animIndex),
            XAnimGetWeight(tree, animIndex),
            tree);
    }
}
#endif

void __cdecl CG_GetTagMatrix(int localClientNum, int linkEntNum, unsigned __int16 tagName, float (*resultTagMat)[3])
{
    centity_s *centLink; // [esp+14h] [ebp-8h]
    DObj *objLink; // [esp+18h] [ebp-4h]

    centLink = CG_GetEntity(localClientNum, linkEntNum);
    if ( ((centLink->clientFlags >> 1) & 1) != 0 )
    {
        objLink = GetLinkEntDObj(localClientNum, centLink);
        if ( !objLink || !CG_DObjGetWorldTagMatrix(&centLink->pose, objLink, tagName, resultTagMat, &(*resultTagMat)[9]) )
        {
            MatrixIdentity33(resultTagMat);
            (*resultTagMat)[9] = 0.0f;
            (*resultTagMat)[10] = 0.0f;
            (*resultTagMat)[11] = 0.0f;
        }
    }
    else
    {
        Com_PrintWarning(14, "An actor is linking to an entity that was not included in latest snapshot.\n");
        MatrixIdentity33(resultTagMat);
        (*resultTagMat)[9] = 0.0f;
        (*resultTagMat)[10] = 0.0f;
        (*resultTagMat)[11] = 0.0f;
    }
}

DObj *__cdecl GetLinkEntDObj(int localClientNum, centity_s *centLink)
{
    if ( ((centLink->clientFlags >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    12,
                    0,
                    "%s",
                    "centLink->nextValid") )
    {
        __debugbreak();
    }
    return Com_GetClientDObj(centLink->nextState.number, localClientNum);
}

void __cdecl CG_CalcTagParentAxis(int localClientNum, centity_s *cent, float (*parentAxis)[3])
{
    centity_s *centLink; // [esp+40h] [ebp-8h]
    cLinkInfo_s *linkInfo; // [esp+44h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 54, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    linkInfo = cent->linkInfo;
    if ( !linkInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 57, 0, "%s", "linkInfo") )
    {
        __debugbreak();
    }
    if ( linkInfo->linkTag <= 0 )
    {
        centLink = CG_GetEntity(localClientNum, linkInfo->linkEnt);
        AnglesToAxis(centLink->pose.angles, parentAxis);
        (*parentAxis)[9] = centLink->pose.origin[0];
        (*parentAxis)[10] = centLink->pose.origin[1];
        (*parentAxis)[11] = centLink->pose.origin[2];
    }
    else
    {
        CG_GetTagMatrix(localClientNum, linkInfo->linkEnt, linkInfo->linkTag, parentAxis);
    }
    if ( ((LODWORD((*parentAxis)[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    71,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[0])[0]) && !IS_NAN((parentAxis[0])[1]) && !IS_NAN((parentAxis[0])[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD((*parentAxis)[3]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[4]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[5]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    72,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[1])[0]) && !IS_NAN((parentAxis[1])[1]) && !IS_NAN((parentAxis[1])[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD((*parentAxis)[6]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[7]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[8]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    73,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[2])[0]) && !IS_NAN((parentAxis[2])[1]) && !IS_NAN((parentAxis[2])[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD((*parentAxis)[9]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[10]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[11]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    74,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[3])[0]) && !IS_NAN((parentAxis[3])[1]) && !IS_NAN((parentAxis[3])[2])") )
    {
        __debugbreak();
    }
}

void __cdecl CG_LinkTransformForEntity(int localClientNum, centity_s *cent, float *resultOrigin, float *resultAngles)
{
    float *v4; // [esp+4h] [ebp-80h]
    float matrix[4][3]; // [esp+Ch] [ebp-78h] BYREF
    float origin[3]; // [esp+3Ch] [ebp-48h]
    float angles[3]; // [esp+48h] [ebp-3Ch] BYREF
    float parentAxis[4][3]; // [esp+54h] [ebp-30h] BYREF

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 89, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    CG_CalcTagParentAxis(localClientNum, cent, parentAxis);
    if ( cent->nextState.clientLinkInfo.parentEnt )
    {
        AnglesToAxis(cent->nextState.lerp.apos.trDelta, cent->linkInfo->axis);
        v4 = cent->linkInfo->axis[3];
        *v4 = cent->nextState.lerp.pos.trDelta[0];
        v4[1] = cent->nextState.lerp.pos.trDelta[1];
        v4[2] = cent->nextState.lerp.pos.trDelta[2];
    }
    MatrixMultiply43(cent->linkInfo->axis, parentAxis, matrix);
    AxisToAngles(matrix, angles);
    *(_QWORD *)origin = *(_QWORD *)&matrix[3][0];
    origin[2] = matrix[3][2];
    if ( resultOrigin )
    {
        *resultOrigin = origin[0];
        resultOrigin[1] = origin[1];
        resultOrigin[2] = origin[2];
    }
    if ( resultAngles )
    {
        *resultAngles = angles[0];
        resultAngles[1] = angles[1];
        resultAngles[2] = angles[2];
    }
}

void __cdecl CG_GenerateLinkInfo(int localClientNum, centity_s *cent, int attachedEntNum, int attachedTagIndex)
{
    float *v4; // [esp+0h] [ebp-A4h]
    float *relative_angles; // [esp+10h] [ebp-94h]
    float invParentAxis[4][3]; // [esp+14h] [ebp-90h] BYREF
    float parentAxis[4][3]; // [esp+44h] [ebp-60h] BYREF
    float axis[4][3]; // [esp+74h] [ebp-30h] BYREF

    if ( !cent->linkInfo )
    {
        cent->linkInfo = (cLinkInfo_s *)MT_Alloc(sizeof(cLinkInfo_s), 18, SCRIPTINSTANCE_SERVER);
        cent->linkInfo->linkEnt = 1023;
        relative_angles = cent->linkInfo->relative_angles;
        *relative_angles = 0.0f;
        relative_angles[1] = 0.0f;
        relative_angles[2] = 0.0f;
        cent->linkInfo->angles_set = 0;
    }
    if ( cent->linkInfo->linkEnt != attachedEntNum )
    {
        cent->linkInfo->linkEnt = attachedEntNum;
        if ( attachedTagIndex )
            cent->linkInfo->linkTag = attachedTagIndex;
        else
            cent->linkInfo->linkTag = -1;
        if ( cent->linkInfo->linkTag <= 0 )
        {
            v4 = cent->linkInfo->axis[3];
            *v4 = cent->nextState.lerp.pos.trBase[0];
            v4[1] = cent->nextState.lerp.pos.trBase[1];
            v4[2] = cent->nextState.lerp.pos.trBase[2];
            AnglesToAxis(cent->nextState.lerp.apos.trBase, cent->linkInfo->axis);
        }
        else
        {
            AnglesToAxis(cent->nextState.lerp.apos.trBase, axis);
            axis[3][0] = cent->nextState.lerp.pos.trBase[0];
            axis[3][1] = cent->nextState.lerp.pos.trBase[1];
            axis[3][2] = cent->nextState.lerp.pos.trBase[2];
            CG_GetTagMatrix(localClientNum, cent->linkInfo->linkEnt, cent->linkInfo->linkTag, parentAxis);
            MatrixInverseOrthogonal43(parentAxis, invParentAxis);
            MatrixMultiply43(axis, invParentAxis, cent->linkInfo->axis);
        }
    }
}

void __cdecl CG_UpdateEntityLink(int localClientNum, centity_s *cent)
{
    char *tagName; // [esp+0h] [ebp-Ch]
    __int16 parentEnt; // [esp+4h] [ebp-8h]
    __int16 parentEnta; // [esp+4h] [ebp-8h]
    unsigned int tagIndex; // [esp+8h] [ebp-4h]

    if ( cent->nextState.number < 1024 )
    {
        parentEnt = cent->nextState.clientLinkInfo.parentEnt;
        if ( parentEnt )
        {
            parentEnta = parentEnt - 1;
            if ( cent->nextState.clientLinkInfo.tagIndex >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                            172,
                            0,
                            "%s",
                            "cent->nextState.clientLinkInfo.tagIndex < MAX_TAGS") )
            {
                __debugbreak();
            }
            tagIndex = 0;
            if ( cent->nextState.clientLinkInfo.tagIndex )
            {
                tagName = CL_GetConfigString(cent->nextState.clientLinkInfo.tagIndex + 3115);
                tagIndex = SL_FindString(tagName, SCRIPTINSTANCE_SERVER);
            }
            if ( !cent->linkInfo || cent->linkInfo->linkEnt != parentEnta || cent->linkInfo->linkTag != tagIndex )
                CG_GenerateLinkInfo(localClientNum, cent, parentEnta, tagIndex);
        }
        else if ( cent->linkInfo )
        {
            cent->linkInfo->linkEnt = 1023;
        }
    }
}

void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent, int parentNum, int tagIndex)
{
    cLinkInfo_s *linkInfo; // [esp+0h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 188, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.number >= 0x600u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    189,
                    0,
                    "cent->nextState.number doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    cent->nextState.number,
                    1536) )
    {
        __debugbreak();
    }
    linkInfo = cent->linkInfo;
    if ( parentNum == 1023 )
    {
        if ( linkInfo )
        {
            MT_Free((unsigned char*)linkInfo, sizeof(cLinkInfo_s), SCRIPTINSTANCE_SERVER);
            cent->linkInfo = 0;
        }
    }
    else
    {
        CG_GenerateLinkInfo(localClientNum, cent, parentNum, tagIndex);
    }
}

void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent)
{
    cLinkInfo_s *linkInfo; // [esp+Ch] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 207, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.number >= 0x600u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    208,
                    0,
                    "cent->nextState.number doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    cent->nextState.number,
                    1536) )
    {
        __debugbreak();
    }
    linkInfo = cent->linkInfo;
    if ( linkInfo
        && linkInfo->linkEnt != 1023
        && ((*((unsigned int *)CG_GetEntity(localClientNum, cent->linkInfo->linkEnt) + 201) >> 1) & 1) == 0 )
    {
        MT_Free((unsigned char*)linkInfo, sizeof(cLinkInfo_s), SCRIPTINSTANCE_SERVER);
        cent->linkInfo = 0;
    }
}

bool __cdecl CG_EntityLinked(int localClientNum, centity_s *cent)
{
    return CG_EntGetLinkToParent(localClientNum, cent) != 0;
}

centity_s *__cdecl CG_EntGetLinkToParent(int localClientNum, centity_s *cent)
{
    centity_s *centParent; // [esp+8h] [ebp-4h]

    if ( !cent->linkInfo || cent->linkInfo->linkEnt == 1023 )
        return 0;
    if ( cent->linkInfo->linkEnt >= 0x600u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    234,
                    0,
                    "%s",
                    "cent->linkInfo->linkEnt >= 0 && cent->linkInfo->linkEnt < MAX_LOCAL_CENTITIES") )
    {
        __debugbreak();
    }
    centParent = CG_GetEntity(localClientNum, cent->linkInfo->linkEnt);
    if ( !centParent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    236,
                    0,
                    "%s",
                    "centParent") )
    {
        __debugbreak();
    }
    if ( ((centParent->clientFlags >> 1) & 1) != 0 )
        return centParent;
    Com_PrintWarning(
        14,
        "Entity #%i of type %i: parent ent #%i not in snapshot.\n",
        cent->nextState.number,
        cent->nextState.eType,
        cent->linkInfo->linkEnt);
    return 0;
}

