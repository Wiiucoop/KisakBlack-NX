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
#ifdef KISAK_SP
#include "cg_animscripted_mp.h"
#include <clientscript/cscr_animtree.h>
#include <bgame/bg_animation.h>
#include <cstddef>
#include <bgame/bg_sp_anim_snapshot.h>
#include <clientscript/cscr_stringlist.h>
#endif

#ifdef KISAK_SP
namespace
{
    constexpr int SP_ACTOR_ATTACHMENT_COUNT = 6;

    struct SPRemoteAttachmentRefs
    {
        unsigned short tags[SP_ACTOR_ATTACHMENT_COUNT] = {};
        ~SPRemoteAttachmentRefs()
        {
            for (unsigned short tag : tags)
                if (tag) SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, tag);
        }
    };

    struct SPActorAttachmentState
    {
        unsigned __int16 modelIndices[SP_ACTOR_ATTACHMENT_COUNT];
        unsigned __int16 tagNames[SP_ACTOR_ATTACHMENT_COUNT];
        unsigned int ignoreCollisionBits;
        unsigned int generation;
        bool valid;
    };

    SPActorAttachmentState s_actorAttachments_SP[1024] = {};
    unsigned int s_actorAttachmentAppliedGeneration_SP[MAX_LOCAL_CLIENTS][1024] = {};
}

void __cdecl CG_PublishActorAttachments_SP(
    int entNum,
    const unsigned __int16 *modelIndices,
    const unsigned __int16 *tagNames,
    unsigned int ignoreCollisionBits)
{
    if ( entNum < 0 || entNum >= 1024 )
        return;

    SPActorAttachmentState &state = s_actorAttachments_SP[entNum];
    for ( int i = 0; i < SP_ACTOR_ATTACHMENT_COUNT; ++i )
    {
        state.modelIndices[i] = modelIndices[i];
        state.tagNames[i] = tagNames[i];
    }
    state.ignoreCollisionBits = ignoreCollisionBits;
    ++state.generation;
    if ( !state.generation )
        ++state.generation;
    state.valid = true;

    Com_Printf(
        15,
        "SP actor attachments publish: ent %d generation %u models %u %u %u %u %u %u tags %u %u %u %u %u %u ignore 0x%x\n",
        entNum,
        state.generation,
        state.modelIndices[0], state.modelIndices[1], state.modelIndices[2],
        state.modelIndices[3], state.modelIndices[4], state.modelIndices[5],
        state.tagNames[0], state.tagNames[1], state.tagNames[2],
        state.tagNames[3], state.tagNames[4], state.tagNames[5],
        state.ignoreCollisionBits);
}
#endif

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
#ifdef KISAK_SP
    const SPActorAttachmentState *attachmentState = NULL;
    unsigned int attachmentGeneration = 0;
    SPActorAttachmentState remoteAttachments = {};
    SPRemoteAttachmentRefs remoteAttachmentRefs;
#endif

    if ( ((cent->clientFlags >> 1) & 1) != 0 )
    {
        p_nextState = &cent->nextState;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cent->nextState.lerp.u.actor.actorNum >= MAX_ACTORS
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        123,
                        0,
                        "es->lerp.u.actor.actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                        cent->nextState.lerp.u.actor.actorNum,
                        MAX_ACTORS) )
        {
            __debugbreak();
        }
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
#ifdef KISAK_SP
        // Retail's actor DObj path consumes the same entityState+0xD0
        // animtree index as script movers, but owns its tree in actorInfo.
        // Recreate that independent client tree before comparing/creating the
        // DObj when useAnimTree selected a different server animation set.
        const unsigned int treeIndex = p_nextState->animTreeIndex;
        XAnim_s *desiredAnims = !com_sv_running->current.enabled
                              ? CG_GetRemoteAnimations_SP(localClientNum, p_nextState->number)
                              : treeIndex
                              ? CG_ResolvePublishedAnims_SP(treeIndex, false)
                              : BG_GetActorAnims();
        if ( desiredAnims && (!ai->pXAnimTree || XAnimGetAnims(ai->pXAnimTree) != desiredAnims) )
        {
            if ( ai->pXAnimTree )
                XAnimFreeTree(ai->pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            ai->pXAnimTree = XAnimCreateTree(desiredAnims, Hunk_AllocXAnimClient);
            ai->dobjDirty = 1;
            Com_Printf(
                15,
                "SP actor animtree resolve: client %d ent %d actor %u index %u tree %p anims %p %s\n",
                localClientNum,
                p_nextState->number,
                p_nextState->lerp.u.actor.actorNum,
                treeIndex,
                ai->pXAnimTree,
                desiredAnims,
                desiredAnims->debugName ? desiredAnims->debugName : "<none>");
        }
#endif
        pDObj = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, pDObj, 0, 0);
        objExists = pDObj != 0;
        pAnimTree = ai->pXAnimTree;
#ifdef KISAK_SP
        if (!com_sv_running->current.enabled)
        {
            const SpAnimSnapshot::Entity *remote = CG_GetRemoteAnimEntity_SP(localClientNum, p_nextState->number);
            unsigned generation = 2166136261u;
            if (remote)
            {
                remoteAttachments.ignoreCollisionBits = remote->ignoreCollision;
                generation = (generation ^ remote->ignoreCollision) * 16777619u;
                for (unsigned i = 0; i < SP_ACTOR_ATTACHMENT_COUNT; ++i)
                {
                    const SpAnimSnapshot::Attachment &attachment = remote->attachments[i];
                    remoteAttachments.modelIndices[i] = static_cast<unsigned short>(attachment.model);
                    generation = (generation ^ attachment.model) * 16777619u;
                    for (unsigned char c : attachment.tag) generation = (generation ^ c) * 16777619u;
                    if (attachment.model)
                    {
                        remoteAttachmentRefs.tags[i] = static_cast<unsigned short>(SL_GetString(
                            const_cast<char *>(attachment.tag.c_str()), 0, SCRIPTINSTANCE_SERVER));
                        remoteAttachments.tagNames[i] = remoteAttachmentRefs.tags[i];
                    }
                }
            }
            attachmentGeneration = generation ? generation : 1;
            attachmentState = &remoteAttachments;
        }
        else if ( p_nextState->number >= 0 && p_nextState->number < 1024
            && s_actorAttachments_SP[p_nextState->number].valid )
        {
            attachmentState = &s_actorAttachments_SP[p_nextState->number];
            attachmentGeneration = attachmentState->generation;
        }
#endif
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
                if ( pAnimTree == Tree && cgs->gameModels[model] == xmodel
#ifdef KISAK_SP
                    && s_actorAttachmentAppliedGeneration_SP[localClientNum][p_nextState->number]
                        == attachmentGeneration
#endif
                    )
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
            unsigned int numModels = 1;
#ifdef KISAK_SP
            if ( attachmentState )
            {
                for ( int i = 0; i < SP_ACTOR_ATTACHMENT_COUNT; ++i )
                {
                    const unsigned int attachmentModel = attachmentState->modelIndices[i];
                    if ( !attachmentModel )
                        break;
                    if ( attachmentModel >= 512 || !cgs->gameModels[attachmentModel] )
                    {
                        Com_PrintWarning(
                            15,
                            "SP actor attachment skipped: ent %d slot %d model %u is unavailable on client\n",
                            p_nextState->number,
                            i,
                            attachmentModel);
                        continue;
                    }
                    dobjModels[numModels].model = cgs->gameModels[attachmentModel];
                    dobjModels[numModels].boneName = attachmentState->tagNames[i];
                    dobjModels[numModels].ignoreCollision =
                        (attachmentState->ignoreCollisionBits & (1u << i)) != 0;
                    ++numModels;
                }
            }
#endif
            v4 = Com_ClientDObjCreate(dobjModels, numModels, pAnimTree, p_nextState->number, localClientNum);
#ifdef KISAK_SP
            if (!com_sv_running->current.enabled)
                CG_ApplyRemoteAnimSnapshot_SP(localClientNum, p_nextState->number, v4, true);
#endif
            ai->dobjDirty = 0;
#ifdef KISAK_SP
            s_actorAttachmentAppliedGeneration_SP[localClientNum][p_nextState->number] = attachmentGeneration;
            Com_Printf(
                15,
                "SP actor DObj create: client %d ent %d models %d generation %u",
                localClientNum,
                p_nextState->number,
                DObjGetNumModels(v4),
                attachmentGeneration);
            for ( int i = 0; i < DObjGetNumModels(v4); ++i )
            {
                const XModel *createdModel = DObjGetModel(v4, i);
                Com_Printf(15, " [%d]=%s", i, createdModel && createdModel->name ? createdModel->name : "<none>");
            }
            Com_Printf(15, "\n");
#endif
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

    if ( cent->nextState.lerp.u.actor.actorNum >= MAX_ACTORS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                    201,
                    0,
                    "es->lerp.u.actor.actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                    cent->nextState.lerp.u.actor.actorNum,
                    MAX_ACTORS) )
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
#ifdef KISAK_SP
    CG_ApplyPendingAnimCommandsForDObj_SP(
        localClientNum,
        cent->nextState.number,
        Com_GetClientDObj(cent->nextState.number, localClientNum));
#endif
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
#ifdef KISAK_SP
    // Temporary frontend scripted-animation ownership probe.  Retail SP sends
    // a separate 44-byte animation-command stream alongside snapshots; this
    // MP-derived client currently has no equivalent consumer.  Sample the
    // rendered actor once per second so runtime evidence can distinguish a
    // missing client DObj/tree from a server-only animation update.
    if ( s1->number >= 100 && s1->number <= 120 )
    {
        static int s_lastActorProbeTime[1024] = {};
        const int probeTime = CG_GetLocalClientGlobals(localClientNum)->time;
        if ( probeTime < s_lastActorProbeTime[s1->number]
            || probeTime - s_lastActorProbeTime[s1->number] >= 1000 )
        {
            const DObj *probeObj = Com_GetClientDObj(s1->number, localClientNum);
            const XAnimTree_s *probeTree = probeObj ? DObjGetTree(probeObj) : NULL;
            const XModel *probeModel = probeObj ? DObjGetModel(probeObj, 0) : NULL;
		unsigned int probeHideBits[5] = {};
            if ( probeObj )
                DObjGetHidePartBits(probeObj, probeHideBits);
            Com_Printf(
                15,
                "SP client actor probe: time %d ent %d eType %d actorNum %u model %d (%s) modelCount %d obj %p tree %p animState 0x%x eFlags 0x%x flags2 0x%x hide %08x/%08x/%08x/%08x/%08x part %08x/%08x/%08x/%08x/%08x pose (%.2f %.2f %.2f) snap (%.2f %.2f %.2f)\n",
                probeTime,
                s1->number,
                s1->eType,
                s1->lerp.u.actor.actorNum,
                s1->index.brushmodel,
                probeModel && probeModel->name ? probeModel->name : "<none>",
                probeObj ? DObjGetNumModels(probeObj) : 0,
                probeObj,
                probeTree,
                s1->animState.state,
                s1->lerp.eFlags,
                s1->lerp.eFlags2,
                probeHideBits[0],
                probeHideBits[1],
                probeHideBits[2],
                probeHideBits[3],
                probeHideBits[4],
                s1->partBits[0],
                s1->partBits[1],
                s1->partBits[2],
                s1->partBits[3],
                s1->partBits[4],
                cent->pose.origin[0],
                cent->pose.origin[1],
                cent->pose.origin[2],
                s1->lerp.pos.trBase[0],
                s1->lerp.pos.trBase[1],
                s1->lerp.pos.trBase[2]);
            s_lastActorProbeTime[s1->number] = probeTime;
        }
    }
#endif
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
#ifdef KISAK_SP
            // Retail SP's combined actor renderer at 0x00581260 consumes the
            // actor DObj/tree directly and has no BG_Dog_UpdateAnimationState
            // call. During animscripted, eFlags2 bit 1 says the dedicated SP
            // animation-command stream owns the tree; running the MP dog state
            // machine here can overwrite that humanoid tree from overlapping
            // MP entityState fields. Retain it for non-scripted actors until
            // their full SP animation state path is reconstructed.
            if ( (s1->lerp.eFlags2 & 2u) == 0 )
                BG_Dog_UpdateAnimationState(localClientNum, &cent->nextState, actorInfo);
#else
            BG_Dog_UpdateAnimationState(localClientNum, &cent->nextState, actorInfo);
#endif
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
#ifdef KISAK_SP
            // Retail SP submits the scripted actor DObj here without the MP
            // DPVS box rejection.  Keep measuring the MP result so this
            // exception remains observable, but do not let it hide the first
            // part of a root-motion entrance.
            const bool actorDpvsRejected = R_CullBoxCurDpvs(bounds[0], localClientNum);
            const bool actorCulled = false;
            if ( s1->number >= 100 && s1->number <= 120 )
            {
                static int s_lastActorCullProbeTime[1024] = {};
                const int probeTime = CG_GetLocalClientGlobals(localClientNum)->time;
                if ( probeTime < s_lastActorCullProbeTime[s1->number]
                    || probeTime - s_lastActorCullProbeTime[s1->number] >= 1000 )
                {
                    Com_Printf(
                        15,
                        "SP client actor cull: time %d ent %d culled %d bounds (%.2f %.2f %.2f)-(%.2f %.2f %.2f)\n",
                        probeTime,
                        s1->number,
                        actorDpvsRejected,
                        bounds[0][0],
                        bounds[0][1],
                        bounds[0][2],
                        bounds[1][0],
                        bounds[1][1],
                        bounds[1][2]);
                    s_lastActorCullProbeTime[s1->number] = probeTime;
                }
            }
#else
            const bool actorCulled = R_CullBoxCurDpvs(bounds[0], localClientNum);
#endif
            if ( !actorCulled )
            {
                CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
                renderFxFlags = 4194308;
                if ( CG_IsInfrared(localClientNum) )
                    renderFxFlags = 4194436;
#ifdef KISAK_SP
                const unsigned __int16 actorSceneIndexBefore = scene.dpvs.sceneDObjIndex[s1->number];
                const unsigned int actorSceneCountBefore = scene.sceneDObjCount;
#endif
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
#ifdef KISAK_SP
                if ( s1->number >= 100 && s1->number <= 120 )
                {
                    static int s_lastActorSceneProbeTime[1024] = {};
                    const int probeTime = CG_GetLocalClientGlobals(localClientNum)->time;
                    if ( probeTime < s_lastActorSceneProbeTime[s1->number]
                        || probeTime - s_lastActorSceneProbeTime[s1->number] >= 1000 )
                    {
                        Com_Printf(
                            15,
                            "SP client actor scene: time %d ent %d index %u->%u count %u->%u fx 0x%x\n",
                            probeTime,
                            s1->number,
                            actorSceneIndexBefore,
                            scene.dpvs.sceneDObjIndex[s1->number],
                            actorSceneCountBefore,
                            scene.sceneDObjCount,
                            renderFxFlags);
                        s_lastActorSceneProbeTime[s1->number] = probeTime;
                    }
                }
#endif
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

#ifdef KISAK_NX
extern const dvar_t *nx_physics;   // phys_main.cpp
#endif

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
#ifdef KISAK_NX
        // HACK while the physics solver is unported (nx_physics off): there is
        // no ragdoll to take the body over when the death animation ends, and
        // the corpse's client anim tree starts empty, so it stood in its bind
        // pose (a T-pose). The corpse is not drawn instead: it vanishes as the
        // death animation finishes. The server entity stays, so scripts keep
        // their reference and the corpse limit still clears it. Copying the
        // server's corpse tree over instead crashed: it carries server script
        // strings into client notetrack notifies. See README-SWITCH, physics.
        // The actor's DObj must go too: it still points at the anim tree of
        // the actor slot, and the next zombie in that slot shares it, so
        // CG_UpdateEntInfo advanced that tree twice a frame (double speed).
        if ( !nx_physics || !nx_physics->current.enabled )
        {
            if ( Com_GetClientDObj(p_nextState->number, localClientNum) )
                CG_SafeDObjFree(localClientNum, p_nextState->number);
            return;
        }
#endif
#ifdef KISAK_SP
        // Retail SP uses the corpse slot carried in actorNum because the actor
        // entity itself becomes the corpse; it is not one of MP's fixed 36..43
        // clone entities.
        corpseIndex = p_nextState->lerp.u.actor.actorNum;
#else
        corpseIndex = p_nextState->number - 36;
#endif
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
