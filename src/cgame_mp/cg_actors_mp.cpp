#include "cg_actors_mp.h"
#include "cg_local_mp.h"
#include <client/splitscreen.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <EffectsCore/fx_marks.h>
#include <qcommon/dobj_management.h>
#include "cg_ents_mp.h"
#include "cg_animtree_mp.h"
#include <xanim/dobj_utils.h>
#include "cg_main_mp.h"
#include <cgame/cg_scr_main.h>
#include <bgame/bg_dog.h>
#include <ragdoll/ragdoll.h>
#include <cgame/cg_world.h>
#include "cg_players_mp.h"
#include <gfx_d3d/r_scene.h>

void __cdecl CG_ActorProcessSnapshot(int localClientNum, centity_s *cent)
{
    bool v2; // [esp+3h] [ebp-5h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_UpdateActorDObj(localClientNum, cent, &cgameGlob->bgs.actorinfo[cent->nextState.lerp.u.actor.actorNum]);
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        if ( cent )
            v2 = ((cent->clientFlags >> 8) & 1) != 0;
        else
            v2 = 0;
    }
    else
    {
        v2 = 0;
    }
    if ( v2 && cent->currentState.u.turret.ownerNum != cent->nextState.lerp.u.turret.ownerNum )
        CScr_NotifyNum(localClientNum, cent->nextState.number, 0, cscr_const.enemy, 0);
}

void __cdecl CG_UpdateActorDObj(int localClientNum, centity_s *cent, actorInfo_t *ai)
{
    XAnimTree_s *Tree; // eax
    DObj *v4; // eax
    float *v5; // [esp+0h] [ebp-230h]
    XModel *xmodel; // [esp+4h] [ebp-22Ch]
    cg_s *cgameGlob; // [esp+Ch] [ebp-224h]
    DObj *pDObj; // [esp+10h] [ebp-220h]
    int model; // [esp+14h] [ebp-21Ch]
    entityState_s *p_nextState; // [esp+18h] [ebp-218h]
    const cgs_t *cgs; // [esp+1Ch] [ebp-214h]
    FxMarkDObjUpdateContext markUpdateContext; // [esp+20h] [ebp-210h] BYREF
    int objExists; // [esp+128h] [ebp-108h]
    XAnimTree_s *pAnimTree; // [esp+12Ch] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+130h] [ebp-100h] BYREF

    if ( ((cent->clientFlags >> 1) & 1) != 0 )
    {
        p_nextState = &cent->nextState;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cent->nextState.lerp.u.actor.actorNum >= 0x10u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        123,
                        0,
                        "es->lerp.u.actor.actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                        cent->nextState.lerp.u.actor.actorNum,
                        16) )
        {
            __debugbreak();
        }
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        pDObj = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, pDObj, 0, 0);
        objExists = pDObj != 0;
        pAnimTree = ai->pXAnimTree;
        model = CG_WhatModelShouldLocalPlayerSee(
                            localClientNum,
                            cgameGlob,
                            cent,
                            cent->nextState.lerp.u.actor.team,
                            cent->nextState.index.brushmodel,
                            cent->nextState.enemyModel);
        if ( pAnimTree && !model )
        {
            XAnimClearTree(pAnimTree);
            if ( objExists )
                CG_SafeDObjFree(localClientNum, p_nextState->number);
            return;
        }
        if ( objExists )
        {
            xmodel = DObjGetModel(pDObj, 0);
            if ( !ai->dobjDirty )
            {
                Tree = DObjGetTree(pDObj);
                if ( pAnimTree == Tree && cgs->gameModels[model] == xmodel )
                    return;
            }
            CG_SafeDObjFree(localClientNum, p_nextState->number);
        }
        dobjModels[0].model = cgs->gameModels[model];
        if ( !dobjModels[0].model
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        167,
                        0,
                        "%s",
                        "dobjModels[iNumModels].model") )
        {
            __debugbreak();
        }
        dobjModels[0].boneName = 0;
        dobjModels[0].ignoreCollision = 0;
        if ( pAnimTree )
        {
            v4 = Com_ClientDObjCreate(dobjModels, 1u, pAnimTree, p_nextState->number, localClientNum);
            ai->dobjDirty = 0;
            v5 = cg_entityOriginArray[localClientNum][p_nextState->number];
            *v5 = 131072.0f;
            v5[1] = 131072.0f;
            v5[2] = 131072.0f;
            FX_MarkEntUpdateEnd(&markUpdateContext, localClientNum, p_nextState->number, v4, 0, 0);
        }
    }
}

//cgs_t *__cdecl CG_GetLocalClientStaticGlobals(int localClientNum)
//{
//    if ( localClientNum
//        && !Assert_MyHandler(
//                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
//                    1843,
//                    0,
//                    "%s\n\t(localClientNum) = %i",
//                    "(localClientNum == 0)",
//                    localClientNum) )
//    {
//        __debugbreak();
//    }
//    return cgsArray;
//}

void __cdecl CG_ResetActorEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent)
{
    actorInfo_t *ai; // [esp+8h] [ebp-10h]
    XAnimTree_s *pAnimTree; // [esp+14h] [ebp-4h]

    if ( cent->nextState.lerp.u.actor.actorNum >= 0x10u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                    201,
                    0,
                    "es->lerp.u.actor.actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                    cent->nextState.lerp.u.actor.actorNum,
                    16) )
    {
        __debugbreak();
    }
    ai = &cgameGlob->bgs.actorinfo[cent->nextState.lerp.u.actor.actorNum];
    CG_GetLocalClientStaticGlobals(localClientNum);
    pAnimTree = ai->pXAnimTree;
    ai->dobjDirty = 1;
    if ( !pAnimTree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp", 209, 0, "%s", "pAnimTree") )
    {
        __debugbreak();
    }
    XAnimClearTreeGoalWeights(pAnimTree, 0, 0.0, -1);
    CG_UpdateActorDObj(localClientNum, cent, ai);
}

void __cdecl CG_Actor_PreControllers(int localClientNum, centity_s *cent)
{
    cent->pose.player.nextWaterHeightCheck = 0;
    cent->pose.turret.barrelPitch = 0.0f;
    cent->pose.fx.triggerTime = 1;
    cent->pose.player.nextWaterHeightCheck = 0;
}

void __cdecl CG_Actor(int localClientNum, centity_s *cent)
{
    unsigned int renderFxFlags; // [esp+2Ch] [ebp-5Ch]
    unsigned __int16 t; // [esp+30h] [ebp-58h]
    const DObj *obj; // [esp+34h] [ebp-54h]
    entityState_s *s1; // [esp+38h] [ebp-50h]
    float mins[3]; // [esp+40h] [ebp-48h] BYREF
    const cgs_t *cgs; // [esp+4Ch] [ebp-3Ch]
    float bounds[2][3]; // [esp+50h] [ebp-38h] BYREF
    float maxs[3]; // [esp+68h] [ebp-20h] BYREF
    float lightingOrigin[3]; // [esp+74h] [ebp-14h] BYREF
    actorInfo_t *actorInfo; // [esp+84h] [ebp-4h]

    s1 = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        obj = Com_GetClientDObj(s1->number, localClientNum);
        if ( obj )
        {
            if ( CG_EntityNeedsScriptThread(localClientNum, cent) )
            {
                cent->clientFlags |= 0x100u;
                Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                t = CScr_ExecEntThread(cent, cg_scr_data.entityspawned, 1u);
                Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
            }
            actorInfo = &CG_GetLocalClientGlobals(localClientNum)->bgs.actorinfo[cent->nextState.lerp.u.actor.actorNum];
            BG_Dog_UpdateAnimationState(localClientNum, &cent->nextState, actorInfo);
            CG_Actor_PreControllers(localClientNum, cent);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2] + 32.0;
            if (cent->pose.isRagdoll && cent->pose.ragdollHandle > 0)
            {
                Ragdoll_GetRootOrigin(cent->pose.ragdollHandle, cent->pose.origin);
            }
            CG_DoFootsteps(localClientNum, cent);
            CG_GetEntityDobjBounds(cent, obj, mins, maxs, bounds[0], bounds[1]);
            if ( !R_CullBoxCurDpvs(bounds[0], localClientNum) )
            {
                CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
                renderFxFlags = 4194308;
                if ( CG_IsInfrared(localClientNum) )
                    renderFxFlags = 4194436;
                R_AddDObjToScene(
                    obj,
                    &cent->pose,
                    s1->number,
                    renderFxFlags,
                    lightingOrigin,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    -1,
                    -1,
                    &cent->pose.constantSet,
                    0,
                    0.0,
                    1.0);
            }
            if ( cent->nextState.eType == 17 )
            {
                if ( CL_LocalClient_IsFirstActive(localClientNum) )
                    CG_DoTouchTriggers(cent, localClientNum);
            }
        }
    }
}

bool __cdecl CG_EntityNeedsScriptThread(int localClientNum, centity_s *cent)
{
    if ( !cg_loadScripts || !cg_loadScripts->current.enabled )
        return 0;
    if ( !CL_LocalClient_IsFirstActive(localClientNum) )
        return 0;
    if ( cent )
        return ((cent->clientFlags >> 8) & 1) == 0;
    return 0;
}

void __cdecl CG_ActorCorpse(int localClientNum, centity_s *cent)
{
    actorInfo_t *ai; // [esp+2Ch] [ebp-20h]
    const DObj *obj; // [esp+30h] [ebp-1Ch]
    entityState_s *p_nextState; // [esp+38h] [ebp-14h]
    unsigned int corpseIndex; // [esp+3Ch] [ebp-10h]
    float lightingOrigin[3]; // [esp+40h] [ebp-Ch] BYREF

    p_nextState = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x40000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                    338,
                    0,
                    "%s",
                    "es->lerp.eFlags & EF_DEAD") )
    {
        __debugbreak();
    }
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        corpseIndex = p_nextState->number - 36;
        if ( corpseIndex >= 8
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        345,
                        0,
                        "%s",
                        "(unsigned)corpseIndex < MAX_ACTOR_CORPSES") )
        {
            __debugbreak();
        }
        ai = (actorInfo_t *)&CG_GetLocalClientStaticGlobals(localClientNum)->actorCorpseInfo[corpseIndex].animInfo.legs.pitchAngle;
        CG_UpdateActorDObj(localClientNum, cent, ai);
        obj = Com_GetClientDObj(p_nextState->number, localClientNum);
        if ( obj )
        {
            BG_Dog_UpdateAnimationState(localClientNum, &cent->nextState, ai);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2];
            if ( (cent->nextState.lerp.eFlags & 8) != 0 )
            {
                lightingOrigin[2] = lightingOrigin[2] + 12.0;
            }
            else if ( (cent->nextState.lerp.eFlags & 4) != 0 )
            {
                lightingOrigin[2] = lightingOrigin[2] + 20.0;
            }
            else
            {
                lightingOrigin[2] = lightingOrigin[2] + 32.0;
            }
            CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
            R_AddDObjToScene(
                obj,
                &cent->pose,
                p_nextState->number,
                0x400000u,
                lightingOrigin,
                0.0,
                0.0,
                0.0,
                0.0,
                -1,
                -1,
                &cent->pose.constantSet,
                0,
                0.0,
                1.0);
        }
    }
}

