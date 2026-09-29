#include "actor_team_move.h"
#include <game_mp/g_main_mp.h>
#include "actor_navigation.h"
#include <game_mp/actor_mp.h>
#include "bullet.h"
#ifdef KISAK_SP
#include <game_mp/g_spawn_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <server/sv_world.h>
#endif

#ifdef KISAK_SP
// Retail SP table at 0x00A50CD4, indexed by the four-value SP AISpecies.
const float g_actorAssumedSpeed[4] = { 190.0f, 300.0f, 190.0f, 300.0f };
#else
const float g_actorAssumedSpeed[1] = { 300.0f };
#endif


void __fastcall Actor_TeamMoveBlocked(actor_s *self)
{
    self->iTeamMoveWaitTime = ACTOR_TEAMMOVE_WAIT_TIME + level.time;
}

void __fastcall Actor_TeamMoveBlockedClear(actor_s *self)
{
    self->iTeamMoveWaitTime = 0;
}

char __cdecl Actor_TeamMoveCalcMovementDir(team_move_context_t *context, ai_teammove_t *result)
{
    float v3; // [esp+4h] [ebp-34h]
    float fVelSelf; // [esp+30h] [ebp-8h]
    actor_s *self; // [esp+34h] [ebp-4h]

    self = context->self;
    *(double *)context->vVelSelf = *(double *)context->self->Physics.vVelocity;
    *(double *)context->vVelDirSelf = *(double *)context->vVelSelf;
    fVelSelf = Vec2Normalize(context->vVelDirSelf);
    if ( fVelSelf >= 1.0 )
        goto LABEL_9;
    *(double *)context->vVelDirSelf = *(double *)self->Path.lookaheadDir;
    if ( context->vVelDirSelf[0] != 0.0 || context->vVelDirSelf[1] != 0.0 )
    {
        v3 = g_actorAssumedSpeed[self->species];
        context->vVelSelf[0] = v3 * context->vVelDirSelf[0];
        context->vVelSelf[1] = v3 * context->vVelDirSelf[1];
        fVelSelf = g_actorAssumedSpeed[self->species];
LABEL_9:
        context->fVelSelfSqrd = fVelSelf * fVelSelf;
        return 0;
    }
    if ( Path_UsesObstacleNegotiation(&self->Path) )
    {
        *result = AI_TEAMMOVE_TRAVEL;
    }
    else
    {
        Actor_ClearPath(self);
        *result = AI_TEAMMOVE_WAIT;
    }
    return 1;
}

#ifdef KISAK_SP
// Retail SP FUN_00492930 (unnamed): true when the actor is heading for a negotiation node or
// for its claimed cover node, and the animscript has not overridden the run-to position.
static bool Actor_WantsCornerApproachNotify(actor_s *self)
{
    const pathnode_t *node; // claimed node

    if ( self->arrivalInfo.animscriptOverrideRunTo )
        return 0;
    if ( Path_HasNegotiationNode(&self->Path) )
        return 1;
    node = self->sentient->pClaimedNode;
    if ( !node || ((1 << (node->constant.type & 0x1F)) & 0x83FFC) == 0 )
        return 0;
    if ( !Actor_HasPath(self) )
        return 0;
    return Vec3Compare(self->Path.vFinalGoal, node->constant.vOrigin) != 0;
}

// Retail SP FUN_00627530 (unnamed): walks the path back from the negotiation/claimed node to find
// the furthest path point with a clear line to it, then notifies "corner_approach" with the
// 2D approach direction.
static void Actor_NotifyCornerApproach(actor_s *self)
{
    const pathnode_t *node;
    path_t *pPath;
    trace_t trace;
    float vEnd[3];
    float vStart[3];
    float vDir[3];
    float dx;
    float dy;
    int firstIndex;
    int i;

    memset(&trace, 0, sizeof(trace));
    pPath = &self->Path;
    if ( Path_HasNegotiationNode(pPath) )
    {
        node = Path_GetNegotiationNode(pPath);
        vEnd[0] = node->constant.vOrigin[0];
        vEnd[1] = node->constant.vOrigin[1];
        vEnd[2] = node->constant.vOrigin[2];
    }
    else
    {
        node = self->sentient->pClaimedNode;
        vEnd[0] = pPath->vFinalGoal[0];
        vEnd[1] = pPath->vFinalGoal[1];
        vEnd[2] = pPath->vFinalGoal[2];
    }
    vEnd[2] = vEnd[2] + 18.0f;
    firstIndex = pPath->wNegotiationStartNode + 1;
    for ( i = firstIndex; i < pPath->wPathLen; ++i )
    {
        dx = vEnd[0] - pPath->pts[i].vOrigPoint[0];
        dy = vEnd[1] - pPath->pts[i].vOrigPoint[1];
        if ( i > firstIndex && (float)(dx * dx + dy * dy) > 250000.0f )
            break;
        if ( (float)(node->constant.forward[0] * dx + node->constant.forward[1] * dy) >= 0.0f )
        {
            vStart[0] = pPath->pts[i].vOrigPoint[0];
            vStart[1] = pPath->pts[i].vOrigPoint[1];
            vStart[2] = pPath->pts[i].vOrigPoint[2] + 18.0f;
            col_context_t context;
            G_TraceCapsule(
                &trace,
                vStart,
                vec3_origin,
                vec3_origin,
                vEnd,
                self->ent->s.number,
                self->Physics.iTraceMask,
                &context);
            if ( trace.allsolid || trace.fraction < 1.0f )
                break;
        }
    }
    if ( i == firstIndex )
        return;
    dx = vEnd[0] - (pPath->lookaheadDir[0] * pPath->fLookaheadDist + self->ent->r.currentOrigin[0]);
    dy = vEnd[1] - (pPath->lookaheadDir[1] * pPath->fLookaheadDist + self->ent->r.currentOrigin[1]);
    if ( (float)(dx * dx + dy * dy) >= 225.0f )
    {
        vDir[0] = vEnd[0] - pPath->pts[i - 1].vOrigPoint[0];
        vDir[1] = vEnd[1] - pPath->pts[i - 1].vOrigPoint[1];
    }
    else
    {
        vDir[0] = pPath->lookaheadDir[0];
        vDir[1] = pPath->lookaheadDir[1];
    }
    Vec2Normalize(vDir);
    vDir[2] = 0.0f;
    Scr_AddVector(vDir, SCRIPTINSTANCE_SERVER);
    Scr_Notify(self->ent, scr_const.corner_approach, 1u);
}
#endif

void __cdecl Actor_MoveAlongPathWithTeam(actor_s *self, bool bRun, bool bUseInterval, bool bAllowGoalPileUp)
{
    const char *v4; // eax
    ai_teammove_t eTeamMove; // [esp+2Ch] [ebp-8h]
    bool wasMoving; // [esp+33h] [ebp-1h]

    PROF_SCOPED("movealongpath");

    if ( !Actor_HasPath(self)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    942,
                    0,
                    "%s",
                    "Actor_HasPath( self )") )
    {
        __debugbreak();
    }
    if ( self->eAnimMode == AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS )
    {
        wasMoving = 0;
        eTeamMove = AI_TEAMMOVE_TRAVEL;
    }
    else
    {
        wasMoving = Actor_IsMoving(self);
        if ( !wasMoving )
        {
            self->eAnimMode = AI_ANIM_MOVE_CODE;
            self->moveMode = AI_MOVE_STOP;
            if ( self->stateLevel < 6 && self->eState[self->stateLevel + 1] != AIS_NEGOTIATION )
            {
                self->prevMoveDir[0] = 0.0f;
                self->prevMoveDir[1] = 0.0f;
            }
        }
        eTeamMove = Actor_GetTeamMoveStatus(self, bUseInterval, bAllowGoalPileUp);
    }
    switch ( eTeamMove )
    {
        case AI_TEAMMOVE_TRAVEL:
LABEL_18:
            if ( bRun )
                Actor_AnimTryRun(self);
            else
                Actor_AnimTryWalk(self);
            if ( self->goalPosChanged && wasMoving || self->arrivalInfo.arrivalNotifyRequested )
            {
#ifdef KISAK_SP
                // Retail SP 0x00464630: send the "corner_approach" arrival notify before clearing the request.
                if ( Actor_WantsCornerApproachNotify(self) )
                    Actor_NotifyCornerApproach(self);
#endif
                self->arrivalInfo.arrivalNotifyRequested = 0;
            }
            if ( !wasMoving && Actor_IsMoving(self) )
            {
                Actor_ClearMoveHistory(self);
                self->yawVeloc = 0.0f;
                self->ent->flags &= 0xE7FFFFFF;
                self->Path.iPathEndTime = 0;
            }
            return;
        case AI_TEAMMOVE_WAIT:
            Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
            self->arrivalInfo.animscriptOverrideRunTo = 0;
            return;
        case AI_TEAMMOVE_SLOW_DOWN:
            bRun = 0;
            goto LABEL_18;
    }

    v4 = va("unhandled case %i for Actor_MoveAlongPathWithTeam", eTeamMove);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 1015, 0, v4) )
        __debugbreak();
}

ai_teammove_t __fastcall Actor_GetTeamMoveStatus(actor_s *self, bool bUseInterval, bool bAllowGoalPileUp)
{
    sentient_s *TargetSentient; // eax
    bool v6; // [esp+28h] [ebp-1B0h]
    float *v9; // [esp+34h] [ebp-1A4h]
    float *currentOrigin; // [esp+40h] [ebp-198h]
    team_move_other_context_t context_other; // [esp+44h] [ebp-194h] BYREF
    sentient_s *other; // [esp+74h] [ebp-164h]
    actor_s *pOtherActor; // [esp+78h] [ebp-160h]
    path_t *pPath; // [esp+7Ch] [ebp-15Ch]
    team_move_context_t context; // [esp+80h] [ebp-158h] BYREF
    float vOrgDodgeStart[3]; // [esp+198h] [ebp-40h] BYREF
    float vVelPerp[2]; // [esp+1A4h] [ebp-34h]
    ai_teammove_t eResult; // [esp+1ACh] [ebp-2Ch] BYREF
    int startIndex; // [esp+1B0h] [ebp-28h]
    float vOrgDodge[3]; // [esp+1B4h] [ebp-24h]
    int mask; // [esp+1C0h] [ebp-18h]
    float vOrgDodgeEnd[3]; // [esp+1C4h] [ebp-14h] BYREF
    int bCheckLookahead; // [esp+1D0h] [ebp-8h]
    bool bCheckWait; // [esp+1D7h] [ebp-1h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 681, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 682, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !Actor_HasPath(self)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    683,
                    0,
                    "%s",
                    "Actor_HasPath( self )") )
    {
        __debugbreak();
    }
    if ( self->eAnimMode != AI_ANIM_MOVE_CODE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    684,
                    0,
                    "%s",
                    "self->eAnimMode == AI_ANIM_MOVE_CODE") )
    {
        __debugbreak();
    }
    eResult = AI_TEAMMOVE_TRAVEL;
    pPath = &self->Path;
    currentOrigin = self->ent->r.currentOrigin;
    context.vOrgSelf[0] = *currentOrigin;
    context.vOrgSelf[1] = currentOrigin[1];
    context.vOrgSelf[2] = currentOrigin[2];
    if ( Actor_TeamMoveCheckWaitTimer(self, &eResult) )
        return eResult;
    context.self = self;
    if ( Actor_TeamMoveCalcMovementDir(&context, &eResult) )
        return eResult;
    Actor_TeamMoveInitializeContext(self, bUseInterval, bAllowGoalPileUp, &context);
    for ( other = level.sentients; other < &level.sentients[48]; ++other )
    {
        if ( other->inuse )
        {
            context_other.other = other;
            if ( Actor_TeamMoveShouldTryDodgeSentient(&context, &context_other)
                && Actor_TeamMoveTryDodge(&context, &context_other) )
            {
                Actor_TeamMoveSetDodge(&context, &context_other);
            }
        }
    }
    if ( context.pSlowDownOther && !Actor_TeamMoveConsiderSlowDown(&context, &eResult) )
        return AI_TEAMMOVE_WAIT;
    Actor_ClearPileUp(self);
    if ( !context.pDodgeOther || self->noDodgeMove || ai_noDodge->current.enabled )
        return Actor_TeamMoveNoDodge(&context, eResult);
    if ( pPath->wDodgeEntity != context.pDodgeOther->ent->s.number )
    {
        self->iTeamMoveDodgeTime = 0;
        if ( pPath->wDodgeCount < 0 )
            pPath->wDodgeCount = 0;
        pPath->wDodgeEntity = context.pDodgeOther->ent->s.number;
    }
    if ( !Actor_IsEnemy(self, context.pDodgeOther) )
    {
        pOtherActor = context.pDodgeOther->ent->actor;
        bCheckWait = Actor_TeamMoveNeedToCheckWait(self->moveMode, pPath);
        if ( !bCheckWait )
            goto LABEL_44;
        while ( 1 )
        {
            if ( Actor_TeamMoveCheckPileup(self, pOtherActor) )
                return AI_TEAMMOVE_WAIT;
LABEL_44:
            if ( level.time < self->iTeamMoveDodgeTime )
                return eResult;
            startIndex = Actor_TeamMoveTrimPath(pPath, &context);
            v6 = pPath->fLookaheadDistToNextNode != 0.0
                && pPath->fLookaheadDist >= (float)(sqrtf(context.fDodgePosDeltaLengthSqrd) + 37.5);
            bCheckLookahead = v6;
            if ( v6
                && pPath->lookaheadNextNode >= pPath->wPathLen - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                            767,
                            0,
                            "%s",
                            "!bCheckLookahead || pPath->lookaheadNextNode < pPath->wPathLen - 1") )
            {
                __debugbreak();
            }
            if ( bCheckLookahead || startIndex >= pPath->wNegotiationStartNode )
            {
                if ( context.vVelDirSelf[0] == 0.0
                    && context.vVelDirSelf[1] == 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                                772,
                                0,
                                "%s",
                                "context.vVelDirSelf[0] || context.vVelDirSelf[1]") )
                {
                    __debugbreak();
                }
                if ( pOtherActor )
                    mask = self->Physics.iTraceMask | 4;
                else
                    // retail SP 0x007D099B ORs in 0x4004: SP actors carry contents 0x4000.
                    // This reconstruction keeps the MP actor bit (0x8000, see SP_actor) in
                    // both builds, so the mask has to keep using it too.
                    mask = self->Physics.iTraceMask | 0x8004;
                Actor_TeamMoveTooCloseMoveAway(self, mask, &context);
                vVelPerp[0] = context.vVelDirSelf[1] * 37.5;
                vVelPerp[1] = context.vVelDirSelf[0] * -37.5;
                v9 = context.pDodgeOther->ent->r.currentOrigin;
                vOrgDodge[0] = *v9;
                vOrgDodge[1] = v9[1];
                vOrgDodge[2] = v9[2];
                vOrgDodgeStart[2] = vOrgDodge[2];
                vOrgDodgeStart[0] = (float)((float)(-37.5 * context.vVelDirSelf[0]) + vOrgDodge[0])
                                                    + (float)(context.vVelDirSelf[1] * 37.5);
                vOrgDodgeStart[1] = (float)((float)(-37.5 * context.vVelDirSelf[1]) + vOrgDodge[1])
                                                    + (float)(context.vVelDirSelf[0] * -37.5);
                vOrgDodgeEnd[0] = (float)(75.0 * context.vVelDirSelf[0]) + vOrgDodgeStart[0];
                vOrgDodgeEnd[1] = (float)(75.0 * context.vVelDirSelf[1]) + vOrgDodgeStart[1];
                vOrgDodgeEnd[2] = vOrgDodge[2];
                if ( Path_AttemptDodge(
                             pPath,
                             context.vOrgSelf,
                             vOrgDodgeStart,
                             vOrgDodgeEnd,
                             startIndex,
                             context.dodgeEntities,
                             context.dodgeEntityCount,
                             self->ent->s.number,
                             mask,
                             bCheckLookahead) )
                {
                    if ( ai_showDodge->current.enabled )
                    {
                        if ( pOtherActor )
                            Com_Printf(18, "AI %d right dodge succeed AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
                        else
                            Com_Printf(18, "AI %d right dodge succeed AI %d\n", self->ent->s.number, 1023);
                    }
                    goto LABEL_136;
                }
                if ( ai_showDodge->current.enabled )
                {
                    if ( pOtherActor )
                        Com_Printf(18, "AI %d failed right dodge pathing AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
                    else
                        Com_Printf(18, "AI %d failed right dodge pathing AI %d\n", self->ent->s.number, 1023);
                }
                vVelPerp[0] = 2.0 * vVelPerp[0];
                vVelPerp[1] = 2.0 * vVelPerp[1];
                vOrgDodgeStart[0] = vOrgDodgeStart[0] - vVelPerp[0];
                vOrgDodgeStart[1] = vOrgDodgeStart[1] - vVelPerp[1];
                vOrgDodgeEnd[0] = vOrgDodgeEnd[0] - vVelPerp[0];
                vOrgDodgeEnd[1] = vOrgDodgeEnd[1] - vVelPerp[1];
                if ( Path_AttemptDodge(
                             pPath,
                             context.vOrgSelf,
                             vOrgDodgeStart,
                             vOrgDodgeEnd,
                             startIndex,
                             context.dodgeEntities,
                             context.dodgeEntityCount,
                             self->ent->s.number,
                             mask,
                             bCheckLookahead) )
                {
                    if ( ai_showDodge->current.enabled )
                    {
                        if ( pOtherActor )
                            Com_Printf(18, "AI %d left dodge succeed AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
                        else
                            Com_Printf(18, "AI %d left dodge succeed AI %d\n", self->ent->s.number, 1023);
                    }
LABEL_136:
                    self->iTeamMoveDodgeTime = level.time + 1000;
                    if ( self->moveMode )
                        return eResult;
                    else
                        return AI_TEAMMOVE_WAIT;
                }
                if ( ai_showDodge->current.enabled )
                {
                    if ( pOtherActor )
                        Com_Printf(18, "AI %d failed left dodge pathing AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
                    else
                        Com_Printf(18, "AI %d failed left dodge pathing AI %d\n", self->ent->s.number, 1023);
                }
            }
            if ( pOtherActor
                && self->eSubState[self->stateLevel] == STATE_EXPOSED_COMBAT
                && (pOtherActor->eSubState[self->stateLevel] != STATE_EXPOSED_COMBAT || !Actor_HasPath(pOtherActor)) )
            {
                goto dodge;
            }
            if ( context.fDodgePosDeltaLengthSqrd >= 1406.25 )
            {
                if ( self->moveMode )
                    return eResult;
                if ( context.fDodgePosDeltaLengthSqrd >= 3600.0
                    && (pPath->fLookaheadDist >= 60.0 || Path_CompleteLookahead(pPath)) )
                {
                    goto dodge;
                }
            }
            if ( pPath->wDodgeCount < 0 )
                break;
            if ( pPath->wDodgeCount || !pOtherActor )
                goto LABEL_107;
            if ( (pOtherActor->eAnimMode == AI_ANIM_MOVE_CODE
                 || pOtherActor->eAnimMode == AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS)
                && !pOtherActor->moveMode
                && Actor_IsAtGoal(pOtherActor) )
            {
                if ( ai_showDodge->current.enabled )
                {
                    if ( pOtherActor )
                        Com_Printf(18, "AI %d failed to dodge stationary AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
                    else
                        Com_Printf(18, "AI %d failed to dodge stationary AI %d\n", self->ent->s.number, 1023);
                }
                goto dodge;
            }
            if ( pOtherActor->eState[pOtherActor->stateLevel] != AIS_SCRIPTEDANIM
                && pOtherActor->eState[pOtherActor->stateLevel] != AIS_CUSTOMANIM )
            {
                goto LABEL_107;
            }
            if ( ai_showDodge->current.enabled )
            {
                if ( pOtherActor )
                    goto LABEL_104;
                Com_Printf(18, "AI %d failed to dodge scripted AI %d\n", self->ent->s.number, 1023);
            }
dodge:
            self->iTeamMoveDodgeTime = level.time + 1000;
            if ( self->moveMode )
                return eResult;
            if ( bCheckWait )
                return AI_TEAMMOVE_WAIT;
            eResult = AI_TEAMMOVE_WAIT;
        }
        if ( pPath->fLookaheadDist >= 60.0 || Path_CompleteLookahead(pPath) )
        {
LABEL_107:
            if ( level.time / ACTOR_TEAMMOVE_WAIT_TIME % (2000 / ACTOR_TEAMMOVE_WAIT_TIME) )
                goto LABEL_123;
            if ( pPath->wDodgeCount >= 0 )
            {
                if ( pPath->wDodgeCount || context.bFailedLookahead )
                    Actor_ClearPath(self);
                else
                    pPath->wDodgeCount = -1;
                goto LABEL_123;
            }
        }
        if ( !pOtherActor )
            goto LABEL_118;
        if ( pOtherActor->eAnimMode == AI_ANIM_MOVE_CODE )
        {
            if ( ai_showDodge->current.enabled )
                Com_Printf(18, "AI %d failed to dodge pathing AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
        }
        else
        {
            if ( pOtherActor->eState[pOtherActor->stateLevel] != AIS_SCRIPTEDANIM
                && pOtherActor->eState[pOtherActor->stateLevel] != AIS_CUSTOMANIM )
            {
LABEL_118:
                pPath->wDodgeCount = 0;
LABEL_123:
                if ( !pOtherActor || pOtherActor->pPileUpEnt != self->ent )
                {
                    self->pPileUpActor = self;
                    self->pPileUpEnt = context.pDodgeOther->ent;
                }
LABEL_40:
                Actor_TeamMoveBlocked(self);
                return AI_TEAMMOVE_WAIT;
            }
            if ( ai_showDodge->current.enabled )
            {
LABEL_104:
                Com_Printf(18, "AI %d failed to dodge scripted AI %d\n", self->ent->s.number, pOtherActor->ent->s.number);
                goto dodge;
            }
        }
        goto dodge;
    }
    if ( context.fDodgePosDeltaLengthSqrd < 1406.25 )
    {
        TargetSentient = Actor_GetTargetSentient(self);
        if ( TargetSentient == context.pDodgeOther )
            Actor_ClearPath(self);
        goto LABEL_40;
    }
    return eResult;
}

bool Actor_TeamMoveCheckWaitTimer(actor_s *self, ai_teammove_t *result)
{
    iassert(self);
    iassert(result);

    if ( level.time >= self->iTeamMoveWaitTime )
    {
        //if ( EntHandle::isDefined(&self->pCloseEnt) )
        if ( self->pCloseEnt.isDefined() )
        {
            self->iTeamMoveWaitTime = level.time + 50;
            *result = AI_TEAMMOVE_WAIT;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        *result = AI_TEAMMOVE_WAIT;
        return 1;
    }
}

char __cdecl Actor_TeamMoveNeedToCheckWait(unsigned __int8 moveMode, path_t *pPath)
{
    if ( moveMode )
    {
        if ( (pPath->flags & 2) == 0 && pPath->fLookaheadDist < 60.0 && !Path_CompleteLookahead(pPath) )
            return 1;
    }
    else if ( (pPath->flags & 2) == 0 || pPath->fLookaheadDist < 60.0 && !Path_CompleteLookahead(pPath) )
    {
        return 1;
    }
    return 0;
}

bool __cdecl Actor_IsEnemy(actor_s *self, sentient_s *other)
{
    return (~(1 << Sentient_EnemyTeam(self->sentient->eTeam)) & (1 << other->eTeam)) == 0;
}

void __cdecl Actor_TeamMoveSetDodge(team_move_context_t *context, team_move_other_context_t *context_other)
{
    context->fDodgePosDeltaLengthSqrd = context_other->fPosDeltaLengthSqrd;
    context->pDodgeOther = context_other->other;
}

char __cdecl Actor_TeamMoveShouldTryDodgeSentient(
                team_move_context_t *context,
                team_move_other_context_t *context_other)
{
    actor_s *pPileUpActor; // ecx
    bool v4; // [esp+0h] [ebp-8Ch]
    float *trBase; // [esp+64h] [ebp-28h]
    actor_s *pOtherActor; // [esp+68h] [ebp-24h]
    float mySpeedSq; // [esp+74h] [ebp-18h]
    bool bOtherClose; // [esp+79h] [ebp-13h]
    bool bPileUpSelf; // [esp+7Ah] [ebp-12h]
    float otherSpeedSq; // [esp+7Ch] [ebp-10h]
    actor_s *self; // [esp+80h] [ebp-Ch]
    sentient_s *other; // [esp+84h] [ebp-8h]
    bool slowDown; // [esp+8Bh] [ebp-1h]

    self = context->self;
    other = context_other->other;
    if ( !context
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 237, 0, "%s", "context") )
    {
        __debugbreak();
    }
    if ( !context_other
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 238, 0, "%s", "context_other") )
    {
        __debugbreak();
    }
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 239, 0, "%s", "self") )
        __debugbreak();
    if ( !other
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp", 240, 0, "%s", "other") )
    {
        __debugbreak();
    }
    pOtherActor = other->ent->actor;
    if ( pOtherActor )
    {
        if ( Actor_IsEnemy(self, other) )
            return 0;
        if ( other == self->sentient )
            return 0;
        if ( pOtherActor->pPileUpEnt == self->ent )
            return 0;
    }
    trBase = other->ent->s.lerp.pos.trBase;
    context_other->vOrgOther[0] = *trBase;
    context_other->vOrgOther[1] = trBase[1];
    context_other->vOrgOther[2] = trBase[2];
    if ( Actor_AtDifferentElevation(context->vOrgSelf, context_other->vOrgOther) )
        return 0;
    context_other->vDelta[0] = context_other->vOrgOther[0] - context->vOrgSelf[0];
    context_other->vDelta[1] = context_other->vOrgOther[1] - context->vOrgSelf[1];
    context_other->fPosDeltaLengthSqrd = (float)(context_other->vDelta[0] * context_other->vDelta[0])
                                                                         + (float)(context_other->vDelta[1] * context_other->vDelta[1]);
    if ( context_other->fPosDeltaLengthSqrd >= context->fMaxIntervalSqrd )
        return 0;
    context_other->fScale = (float)((float)(context->vVelSelf[0] * context_other->vDelta[0])
                                                                + (float)(context->vVelSelf[1] * context_other->vDelta[1]))
                                                - context->fDeltaCorrection;
    if ( context_other->fScale <= 0.0
        && (float)((float)(self->Path.forwardLookaheadDir2D[0] * context_other->vDelta[0])
                         + (float)(self->Path.forwardLookaheadDir2D[1] * context_other->vDelta[1])) <= 0.0
        && !context->bFailedLookahead )
    {
        return 0;
    }
    bOtherClose = context_other->fPosDeltaLengthSqrd < 900.0;
    mySpeedSq = (float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
                        + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1]);
    if ( pOtherActor )
    {
        //if ( pOtherActor->pPileUpActor || EntHandle::isDefined(&pOtherActor->pCloseEnt) )
        if ( pOtherActor->pPileUpActor || pOtherActor->pCloseEnt.isDefined() )
        {
            context_other->vVelOther[0] = 0.0f;
            context_other->vVelOther[1] = 0.0f;
        }
        else
        {
            context_other->vVelOther[0] = 20.0 * pOtherActor->Physics.vWishDelta[0];
            context_other->vVelOther[1] = 20.0 * pOtherActor->Physics.vWishDelta[1];
        }
        if ( bOtherClose
            //&& !EntHandle::isDefined(&self->pCloseEnt)
            && !self->pCloseEnt.isDefined()
            && level.gentities[pOtherActor->Physics.iHitEntnum].sentient )
        {
            //EntHandle::setEnt(&self->pCloseEnt, pOtherActor->ent);
            self->pCloseEnt.setEnt(pOtherActor->ent);
        }
    }
    else
    {
        if ( context_other->fPosDeltaLengthSqrd >= context->fDodgePosDeltaLengthSqrd )
            return 0;
        if ((self->Physics.iTraceMask & 0x2000000) == 0)
            return 0;
        if ( Actor_IsEnemy(self, other) )
            return 1;
        if ( !other->ent->client
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                        304,
                        0,
                        "%s",
                        "other->ent->client") )
        {
            __debugbreak();
        }
        *(double *)context_other->vVelOther = *(double *)other->ent->client->ps.velocity;
    }
    otherSpeedSq = (float)(context_other->vVelOther[0] * context_other->vVelOther[0])
                             + (float)(context_other->vVelOther[1] * context_other->vVelOther[1]);
    v4 = otherSpeedSq >= 1.0 && (float)(otherSpeedSq * 25.0) > mySpeedSq;
    pPileUpActor = self->pPileUpActor;
    bPileUpSelf = pPileUpActor == self;
    slowDown = 1;
    if ( pPileUpActor != self && v4 )
    {
        if ( context_other->fPosDeltaLengthSqrd >= context->fIntervalSqrd )
            return 0;
        if ( (float)((float)(context->vVelSelf[0] * context_other->vVelOther[0])
                             + (float)(context->vVelSelf[1] * context_other->vVelOther[1])) >= 0.0 )
        {
            if ( (float)(context_other->fScale + context->fDeltaCorrection) <= 0.0 )
                return 0;
            if ( pOtherActor
                && self->ent->s.number > other->ent->s.number
                && context->fDeltaCorrection > (float)((float)(context_other->vVelOther[0] * context_other->vDelta[0])
                                                                                         + (float)(context_other->vVelOther[1] * context_other->vDelta[1])) )
            {
                return 0;
            }
        }
        else if ( !context->bPileUp && (mySpeedSq >= 1.0 || !bOtherClose) )
        {
            if ( self->ent->s.number <= other->ent->s.number )
                return 0;
            slowDown = 0;
        }
    }
    if ( pOtherActor )
        Actor_AddToList(context->dodgeEntities, &context->dodgeEntityCount, 48, pOtherActor);
    if ( context_other->fPosDeltaLengthSqrd >= context->fDodgePosDeltaLengthSqrd )
        return 0;
    if ( bPileUpSelf || !v4 )
        return 1;
    if ( context_other->fPosDeltaLengthSqrd >= context->fSlowDownPosDeltaLengthSqrd )
        return 0;
    if ( !slowDown )
        return 1;
    context->fSlowDownPosDeltaLengthSqrd = context_other->fPosDeltaLengthSqrd;
    context->pSlowDownOther = other;
    *(double *)context->vVelSlowDownOther = *(double *)context_other->vVelOther;
    return 0;
}

void __cdecl Actor_AddToList(int *dodgeEntities, int *dodgeEntityCount, int arraysz, actor_s *pOtherActor)
{
    if ( !pOtherActor->ent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    206,
                    0,
                    "%s",
                    "pOtherActor->ent") )
    {
        __debugbreak();
    }
    if ( *dodgeEntityCount >= arraysz
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    207,
                    0,
                    "%s",
                    "(*dodgeEntityCount) < arraysz") )
    {
        __debugbreak();
    }
    dodgeEntities[(*dodgeEntityCount)++] = pOtherActor->ent->s.number;
}

bool __cdecl Actor_TeamMoveTryDodge(team_move_context_t *context, team_move_other_context_t *context_other)
{
    float v3; // [esp+Ch] [ebp-7Ch]
    float v4; // [esp+54h] [ebp-34h]
    path_t *pPath; // [esp+64h] [ebp-24h]
    float vForwardStartPos[2]; // [esp+68h] [ebp-20h] BYREF
    float dist; // [esp+70h] [ebp-18h]
    actor_s *self; // [esp+74h] [ebp-14h]
    sentient_s *other; // [esp+78h] [ebp-10h]
    bool maintainMotion; // [esp+7Fh] [ebp-9h]
    float perpDist; // [esp+80h] [ebp-8h]
    float minDistSq; // [esp+84h] [ebp-4h]

    self = context->self;
    other = context_other->other;
    pPath = &self->Path;
    perpDist = (float)(context->vVelDirSelf[0] * context_other->vDelta[1])
                     - (float)(context->vVelDirSelf[1] * context_other->vDelta[0]);
    minDistSq = 900.0 - (float)(perpDist * perpDist);
    if ( minDistSq <= 0.0 )
        return 0;
    dist = (float)((float)(context->vVelDirSelf[0] * context_other->vDelta[0])
                             + (float)(context->vVelDirSelf[1] * context_other->vDelta[1]))
             - self->Path.fLookaheadDist;
    if ( dist > 0.0 && (float)(dist * dist) >= (float)((float)(minDistSq + 1406.25) - 900.0) )
        return 0;
    if ( context->bFailedLookahead )
        return 1;
    context_other->fScale = context_other->fScale / context->fVelSelfSqrd;
    v4 = -context_other->fScale;
    context_other->vPerp[0] = (float)(v4 * context->vVelSelf[0]) + context_other->vDelta[0];
    context_other->vPerp[1] = (float)(v4 * context->vVelSelf[1]) + context_other->vDelta[1];
    maintainMotion = 0;
    if ( other->ent->s.number == pPath->wDodgeEntity )
    {
        if ( context->bPileUp )
        {
            if ( (pPath->wDodgeCount || (pPath->flags & 2) != 0)
                && !Path_CompleteLookahead(pPath)
                && (float)(sqrtf(context_other->fPosDeltaLengthSqrd) + 37.5) > pPath->fLookaheadDist )
            {
                return 1;
            }
        }
        else if ( self->moveMode && level.time < self->iTeamMoveDodgeTime )
        {
            if ( context_other->fScale <= 0.0
                || (float)((float)(context_other->vPerp[0] * context_other->vPerp[0])
                                 + (float)(context_other->vPerp[1] * context_other->vPerp[1])) >= 900.0 )
            {
                return 0;
            }
            maintainMotion = 1;
        }
    }
    if ( maintainMotion
        || (float)((float)(context_other->vPerp[0] * context_other->vPerp[0])
                         + (float)(context_other->vPerp[1] * context_other->vPerp[1])) < 1406.25 )
    {
        if ( maintainMotion || context_other->fScale > 0.0 )
        {
            if ( !Path_GetForwardStartPos(pPath, context->vOrgSelf, vForwardStartPos) )
                return 1;
            context_other->vDelta[0] = context_other->vOrgOther[0] - vForwardStartPos[0];
            context_other->vDelta[1] = context_other->vOrgOther[1] - vForwardStartPos[1];
            if ( (float)((float)(context->vVelSelf[0] * context_other->vDelta[0])
                                 + (float)(context->vVelSelf[1] * context_other->vDelta[1])) <= 0.0 )
                return 1;
        }
        else
        {
            if ( context_other->fPosDeltaLengthSqrd >= 3600.0 )
                return 0;
            if ( !Path_GetForwardStartPos(pPath, context->vOrgSelf, vForwardStartPos) )
                return 1;
            context_other->vDelta[0] = context_other->vOrgOther[0] - vForwardStartPos[0];
            context_other->vDelta[1] = context_other->vOrgOther[1] - vForwardStartPos[1];
        }
    }
    else
    {
        if ( context_other->fPosDeltaLengthSqrd >= 3600.0 )
            return 0;
        if ( !Path_GetForwardStartPos(pPath, context->vOrgSelf, vForwardStartPos) )
            return 0;
        context_other->vDelta[0] = context_other->vOrgOther[0] - vForwardStartPos[0];
        context_other->vDelta[1] = context_other->vOrgOther[1] - vForwardStartPos[1];
    }
    context_other->fScale = (float)(pPath->forwardLookaheadDir2D[0] * context_other->vDelta[0])
                                                + (float)(pPath->forwardLookaheadDir2D[1] * context_other->vDelta[1]);
    if ( context_other->fScale <= 0.0 )
        return 0;
    v3 = -context_other->fScale;
    context_other->vPerp[0] = (float)(v3 * pPath->forwardLookaheadDir2D[0]) + context_other->vDelta[0];
    context_other->vPerp[1] = (float)(v3 * pPath->forwardLookaheadDir2D[1]) + context_other->vDelta[1];
    return (float)((float)(context_other->vPerp[0] * context_other->vPerp[0])
                             + (float)(context_other->vPerp[1] * context_other->vPerp[1])) < 900.0;
}

char __cdecl Actor_TeamMoveConsiderSlowDown(team_move_context_t *context, ai_teammove_t *eResult)
{
    bool v3; // [esp+0h] [ebp-14h]
    actor_s *pSlowDownOtherActor; // [esp+Ch] [ebp-8h]

    if ( context->fDodgePosDeltaLengthSqrd > context->fSlowDownPosDeltaLengthSqrd )
    {
        pSlowDownOtherActor = context->pSlowDownOther->ent->actor;
        if ( !pSlowDownOtherActor
            || (float)((float)(context->vVelSelf[0] * context->vVelSlowDownOther[0])
                             + (float)(context->vVelSelf[1] * context->vVelSlowDownOther[1])) < 0.0 )
        {
            Actor_TeamMoveBlocked(context->self);
            return 0;
        }
        v3 = pSlowDownOtherActor->moveMode == 2 && context->fSlowDownPosDeltaLengthSqrd >= context->fWalkIntervalSqrd;
        if ( pSlowDownOtherActor->eAnimMode == AI_ANIM_MOVE_CODE && (pSlowDownOtherActor->moveMode == 3 || v3) )
        {
            *eResult = AI_TEAMMOVE_SLOW_DOWN;
        }
        else
        {
            context->fDodgePosDeltaLengthSqrd = context->fSlowDownPosDeltaLengthSqrd;
            context->pDodgeOther = context->pSlowDownOther;
        }
    }
    return 1;
}

ai_teammove_t __cdecl Actor_TeamMoveNoDodge(team_move_context_t *context, ai_teammove_t eResult)
{
    float *currentOrigin; // edx
    char v4; // [esp+0h] [ebp-24h]
    float v5; // [esp+10h] [ebp-14h]
    path_t *pPath; // [esp+1Ch] [ebp-8h]
    actor_s *self; // [esp+20h] [ebp-4h]

    self = context->self;
    pPath = &context->self->Path;
    if ( context->bFailedLookahead
        || !context->pSlowDownOther
        || context->fVelSelfSqrd <= (float)((float)(context->vVelSlowDownOther[0] * context->vVelSlowDownOther[0])
                                                                            + (float)(context->vVelSlowDownOther[1] * context->vVelSlowDownOther[1])) )
    {
        if ( !context->self->Path.wDodgeCount
            || context->self->Path.wDodgeEntity == 1023
            || (context->self->Path.wDodgeCount >= 0 || context->self->Path.fLookaheadDist >= 37.5 ? (v4 = 0) : (v4 = 1),
                    v4
             || (currentOrigin = level.gentities[context->self->Path.wDodgeEntity].r.currentOrigin,
                     v5 = context->vOrgSelf[1] - level.gentities[context->self->Path.wDodgeEntity].r.currentOrigin[1],
                     (float)((float)((float)(context->vOrgSelf[0] - *currentOrigin)
                                                 * (float)(context->vOrgSelf[0] - *currentOrigin))
                                 + (float)(v5 * v5)) >= 3600.0)) )
        {
            if ( context->self->Path.wDodgeCount < 0 )
                context->self->Path.wDodgeCount = 0;
            if ( pPath->wDodgeEntity == 1023
                || (pPath->wDodgeEntity = 1023, self->moveMode)
                || level.time < self->iTeamMoveDodgeTime )
            {
                self->iTeamMoveDodgeTime = 0;
                return eResult;
            }
            else
            {
                return AI_TEAMMOVE_WAIT;
            }
        }
        else
        {
            return eResult;
        }
    }
    else
    {
        if ( !context->self->Path.wDodgeCount )
            context->self->Path.wDodgeCount = -1;
        pPath->wDodgeEntity = context->pSlowDownOther->ent->s.number;
        return AI_TEAMMOVE_TRAVEL;
    }
}

void __cdecl Actor_TeamMoveInitializeContext(
                actor_s *self,
                bool bUseInterval,
                bool bAllowGoalPileUp,
                team_move_context_t *context)
{
    bool v4; // [esp+4h] [ebp-10h]
    float fVelSelfSqrd; // [esp+8h] [ebp-Ch]

    Actor_CalcInterval(self, bUseInterval, &context->fIntervalSqrd, &context->fWalkIntervalSqrd);
    if ( (float)(context->fIntervalSqrd - context->fVelSelfSqrd) < 0.0 )
        fVelSelfSqrd = context->fVelSelfSqrd;
    else
        fVelSelfSqrd = context->fIntervalSqrd;
    context->fMaxIntervalSqrd = fVelSelfSqrd;
    v4 = self->pPileUpActor || bAllowGoalPileUp && Actor_IsAtGoal(self);
    context->bPileUp = v4;
    context->fDodgePosDeltaLengthSqrd = context->fMaxIntervalSqrd;
    context->dodgeEntityCount = 0;
    context->pDodgeOther = 0;
    context->fSlowDownPosDeltaLengthSqrd = context->fIntervalSqrd;
    context->pSlowDownOther = 0;
    context->bFailedLookahead = Path_FailedLookahead(&self->Path);
    context->fDeltaCorrection = Actor_TeamMoveDeltaCorrection(self, context->fVelSelfSqrd);
}

void __cdecl Actor_CalcInterval(actor_s *self, bool bUseInterval, float *fIntervalSqrdOut, float *fWalkIntervalSqrdOut)
{
    float fWalkInterval; // [esp+0h] [ebp-8h]
    float fInterval; // [esp+4h] [ebp-4h]

    if ( bUseInterval )
    {
        fInterval = self->fInterval;
        fWalkInterval = fInterval * 0.5;
        if ( (float)(fInterval * 0.5) < 37.5 )
        {
            fWalkInterval = 37.5f;
            if ( fInterval < 30.0 )
                fInterval = 30.0f;
        }
        *fIntervalSqrdOut = fInterval * fInterval;
        *fWalkIntervalSqrdOut = fWalkInterval * fWalkInterval;
    }
    else
    {
        *fIntervalSqrdOut = 900.0f;
        *fWalkIntervalSqrdOut = 1406.25f;
    }
}

double __cdecl Actor_TeamMoveDeltaCorrection(actor_s *self, float fVelSelfSqrd)
{
    if ( self->moveMode )
        return (float)((float)(fVelSelfSqrd * 0.1) * 0.1);
    else
        return 0.0f;
}

int __cdecl Actor_TeamMoveTrimPath(path_t *pPath, const team_move_context_t *context)
{
    float v3; // [esp+8h] [ebp-1Ch]
    float v4; // [esp+Ch] [ebp-18h]
    float v5; // [esp+18h] [ebp-Ch]
    float v6; // [esp+1Ch] [ebp-8h]
    int startIndex; // [esp+20h] [ebp-4h]

    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    571,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    572,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    startIndex = pPath->wPathLen - 1;
    if ( pPath->wDodgeCount > 0 )
        startIndex -= pPath->wDodgeCount;
    if ( startIndex > pPath->lookaheadNextNode )
        startIndex = pPath->lookaheadNextNode;
    if ( startIndex >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                    581,
                    0,
                    "%s",
                    "startIndex < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( startIndex == pPath->wPathLen - 1 )
    {
        v5 = context->vOrgSelf[0] - pPath->vCurrPoint[0];
        v6 = context->vOrgSelf[1] - pPath->vCurrPoint[1];
        if ( (float)((float)(v5 * v5) + (float)(v6 * v6)) >= context->fDodgePosDeltaLengthSqrd )
            return startIndex;
        --startIndex;
    }
    while ( startIndex >= pPath->wNegotiationStartNode )
    {
        v3 = context->vOrgSelf[0] - pPath->pts[startIndex].vOrigPoint[0];
        v4 = context->vOrgSelf[1] - pPath->pts[startIndex].vOrigPoint[1];
        if ( (float)((float)(v3 * v3) + (float)(v4 * v4)) >= context->fDodgePosDeltaLengthSqrd )
            break;
        --startIndex;
    }
    return startIndex;
}

void __cdecl Actor_TeamMoveTooCloseMoveAway(const actor_s *self, int mask, team_move_context_t *context)
{
    float *currentOrigin; // [esp+10h] [ebp-24h]
    float vNewOrgSelf[3]; // [esp+14h] [ebp-20h] BYREF
    float vDodgeDelta[2]; // [esp+20h] [ebp-14h] BYREF
    float vOrgDodge[3]; // [esp+28h] [ebp-Ch]

    if ( context->fDodgePosDeltaLengthSqrd < 1406.25 )
    {
        if ( !context->pDodgeOther
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_team_move.cpp",
                        611,
                        0,
                        "%s",
                        "context.pDodgeOther") )
        {
            __debugbreak();
        }
        currentOrigin = context->pDodgeOther->ent->r.currentOrigin;
        vOrgDodge[0] = *currentOrigin;
        vOrgDodge[1] = currentOrigin[1];
        vOrgDodge[2] = currentOrigin[2];
        vDodgeDelta[0] = vOrgDodge[0] - context->vOrgSelf[0];
        vDodgeDelta[1] = vOrgDodge[1] - context->vOrgSelf[1];
        if ( Vec2Normalize(vDodgeDelta) < 37.5 )
        {
            vNewOrgSelf[0] = (float)(-37.5 * vDodgeDelta[0]) + vOrgDodge[0];
            vNewOrgSelf[1] = (float)(-37.5 * vDodgeDelta[1]) + vOrgDodge[1];
            vNewOrgSelf[2] = vOrgDodge[2];
            Path_PredictionTraceCheckForEntities(
                context->vOrgSelf,
                vNewOrgSelf,
                context->dodgeEntities,
                context->dodgeEntityCount,
                self->ent->s.number,
                mask,
                context->vOrgSelf);
        }
    }
}

char __cdecl Actor_TeamMoveCheckPileup(actor_s *self, actor_s *pOtherActor)
{
    if ( !pOtherActor )
        return 0;
    Actor_UpdatePileUp(pOtherActor);
    if ( !pOtherActor->pPileUpActor
        || pOtherActor->pPileUpActor == self
        || pOtherActor->pPileUpEnt == self->ent
        || self->eState[self->stateLevel] == AIS_GRENADE_RESPONSE )
    {
        return 0;
    }
    if ( self->Path.wDodgeCount < 0 )
        self->Path.wDodgeCount = 0;
    self->pPileUpActor = pOtherActor->pPileUpActor;
    self->pPileUpEnt = pOtherActor->pPileUpEnt;
    Actor_TeamMoveBlocked(self);
    self->iTeamMoveDodgeTime = 0;
    return 1;
}

