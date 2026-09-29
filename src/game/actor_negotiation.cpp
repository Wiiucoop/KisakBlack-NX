#include "actor_negotiation.h"
#include "actor_navigation.h"
#include "actor_orientation.h"
#include <clientscript/cscr_stringlist.h>
#include "actor_animapi.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <bgame/bg_dog.h>
#include "actor_state.h"

bool __fastcall Actor_Negotiation_Start(actor_s *pSelf, ai_state_t ePrevState)
{
    const bool hasNegotiationNode = Path_HasNegotiationNode(&pSelf->Path);
#ifdef KISAK_SP
    Com_Printf(
        15,
        "ZM_BARRICADE negotiation_start time=%d ent=%d species=%d hasNode=%d flags=0x%X neg=%d len=%d prevState=%d\n",
        level.time,
        pSelf->ent->s.number,
        pSelf->species,
        hasNegotiationNode,
        pSelf->Path.flags,
        pSelf->Path.wNegotiationStartNode,
        pSelf->Path.wPathLen,
        ePrevState);
#endif
    if ( !hasNegotiationNode )
        return 0;
    Actor_SetOrientMode(pSelf, AI_ORIENT_DONT_CHANGE);
    pSelf->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    pSelf->prevMoveDir[0] = 0.0f;
    pSelf->prevMoveDir[1] = 0.0f;
    return 1;
}

actor_think_result_t __fastcall Actor_Negotiation_Think(actor_s *pSelf)
{
    const char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    ai_animmode_t eAnimMode; // [esp+34h] [ebp-4h]

    PROF_SCOPED("negotiationthink");

#ifdef KISAK_SP
    const bool firstDispatch = !pSelf->pszDebugInfo || I_strcmp(pSelf->pszDebugInfo, "negotiation") != 0;
#endif

    eAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY;
    if ( pSelf->eTraverseMode == AI_TRAVERSE_GRAVITY )
    {
        eAnimMode = AI_ANIM_USE_BOTH_DELTAS;
    }
    else if ( pSelf->eTraverseMode == AI_TRAVERSE_NOCLIP )
    {
        eAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOCLIP;
    }
    else if ( pSelf->eTraverseMode != AI_TRAVERSE_NOGRAVITY )
    {
        v1 = va("pSelf->eTraverseMode is unhandled %i", pSelf->eTraverseMode);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp", 55, 0, v1) )
            __debugbreak();
    }
    if ( !pSelf->AnimScriptSpecific.func )
    {
        v2 = SL_ConvertToString(pSelf->AnimScriptSpecific.name, SCRIPTINSTANCE_SERVER);
        v3 = va("Invalid negotiation script %s", v2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp",
                        58,
                        0,
                        "%s\n\t%s",
                        "pSelf->AnimScriptSpecific.func",
                        v3) )
            __debugbreak();
    }
    Actor_AnimSpecific(pSelf, &pSelf->AnimScriptSpecific, eAnimMode, 1);
    pSelf->pushable = 0;
    const bool animScriptAlive = Actor_IsAnimScriptAlive(pSelf);
#ifdef KISAK_SP
    if ( firstDispatch )
    {
        Com_Printf(
            15,
            "ZM_BARRICADE negotiation_think_first time=%d ent=%d species=%d script=%s func=0x%08X handle=%u traverse=%d animMode=%d alive=%d\n",
            level.time,
            pSelf->ent->s.number,
            pSelf->species,
            pSelf->AnimScriptSpecific.name
                ? SL_ConvertToString(pSelf->AnimScriptSpecific.name, SCRIPTINSTANCE_SERVER)
                : "<none>",
            pSelf->AnimScriptSpecific.func,
            pSelf->AnimScriptHandle,
            pSelf->eTraverseMode,
            eAnimMode,
            animScriptAlive);
    }
#endif
    if ( animScriptAlive )
    {
        pSelf->pszDebugInfo = "negotiation";
        Actor_PreThink(pSelf);
        Actor_UpdateOriginAndAngles(pSelf);
#ifndef KISAK_SP
        // Retail SP Actor_Negotiation_Think @ 005189A0 has no MP dog anim-state update here.
        BG_Dog_UpdateAnimationState(-1, &pSelf->ent->s, &level_bgs.actorinfo[pSelf - level.actors]);
#endif
        return ACTOR_THINK_DONE;
    }
    else
    {
#ifdef KISAK_SP
        Com_Printf(
            15,
            "ZM_BARRICADE negotiation_cleanup time=%d ent=%d species=%d hasNode=%d neg=%d len=%d\n",
            level.time,
            pSelf->ent->s.number,
            pSelf->species,
            Path_HasNegotiationNode(&pSelf->Path),
            pSelf->Path.wNegotiationStartNode,
            pSelf->Path.wPathLen);
#endif
        if ( Path_HasNegotiationNode(&pSelf->Path) )
        {
            if ( pSelf->Path.wNegotiationStartNode >= pSelf->Path.wPathLen
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp",
                            68,
                            0,
                            "%s",
                            "pSelf->Path.wNegotiationStartNode < pSelf->Path.wPathLen") )
            {
                __debugbreak();
            }
            const int endNodeIndex = pSelf->Path.pts[pSelf->Path.wNegotiationStartNode - 1].iNodeNum;
            if ( endNodeIndex < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp",
                            69,
                            0,
                            "%s",
                            "pSelf->Path.pts[pSelf->Path.wNegotiationStartNode - 1].iNodeNum >= 0") )
            {
                __debugbreak();
            }
            pSelf->sentient->pNearestNode = Path_ConvertIndexToNode(endNodeIndex);
            pSelf->sentient->bNearestNodeBad = 0;
            pSelf->sentient->bNearestNodeValid = 1;
        }
        pSelf->Physics.vWishDelta[0] = 0.0f;
        pSelf->Physics.vWishDelta[1] = 0.0f;
        pSelf->Physics.vWishDelta[2] = 0.0f;
        pSelf->Physics.vVelocity[0] = 0.0f;
        pSelf->Physics.vVelocity[1] = 0.0f;
        pSelf->Physics.vVelocity[2] = 0.0f;
        YawVectors2D(pSelf->fDesiredBodyYaw, pSelf->prevMoveDir, 0);
        Actor_RecalcPath(pSelf);
        Actor_PopState(pSelf);
        return ACTOR_THINK_REPEAT;
    }
}

