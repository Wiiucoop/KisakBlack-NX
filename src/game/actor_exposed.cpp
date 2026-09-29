#include "actor_exposed.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include "actor_senses.h"
#include "actor_state.h"
#include <game_mp/g_utils_mp.h>
#include "actor_navigation.h"
#include "actor_orientation.h"
#include "actor_events.h"
#include <cgame_mp/cg_local_mp.h>
#ifdef KISAK_SP
#include "actor_animapi.h"
#include "actor_corpse.h"
#include "actor_team_move.h"
#include "actor_badplace.h"
#include "sentient.h"
#include "pathnode.h"
#include <game_mp/g_spawn_mp.h>
#include <server/sv_world.h>
#include <server/sv_game.h>
#include <clientscript/scr_const.h>
#include <universal/com_math.h>
#include <cmath>
#include <cstring>
#endif

#ifdef KISAK_SP
// Local port helper for the zombie specialization of retail 0x00426710;
// this is not a recovered source symbol. The species-zero node selectors
// are unreachable for this controller's AI_SPECIES_ZOMBIE table entry.
static void Actor_Zombie_ExposedCombat_SP(actor_s *self)
{
    iassert(self->species == AI_SPECIES_ZOMBIE);

    Actor_CheckCollisions(self);
    Actor_ClearPileUp(self);
    self->bUseGoalWeight = false;

    scr_animscript_t *combat = &g_animScriptTable[self->species]->combat;
    if ( self->fixedNode && level.time > self->exposedStartTime + 2000
        && self->pAnimScriptFunc != combat )
    {
        self->exposedStartTime = level.time;
    }

    // Reselecting the same script preserves its thread and script-set mode,
    // including melee while safeToChangeScript is false. Actor_AnimStop with
    // ->stop would replace that thread and bypass zombie_combat::TryMelee.
    Actor_SetAnimScript(
        self, combat, AI_MOVE_STOP,
        self->pCloseEnt.isDefined() ? AI_ANIM_MOVE_CODE : AI_ANIM_USE_BOTH_DELTAS,
        AI_ANIM_FUNCTION_COMBAT);
    // Retail 0x007C0EF0 always faces the enemy without a claimed 0x8000
    // node. Its exceptional node/visibility predicates remain unported.
    const pathnode_t *node = self->sentient->pClaimedNode;
    if ( !node || (node->constant.spawnflags & 0x8000) == 0 )
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
}

// ---------------------------------------------------------------------------
// Retail SP exposed goal/path maintenance.  None of the helpers below have an
// attested source name; the names are descriptive and the retail address is
// given for each.  Species-0 (human) cover-node candidate selection
// (0x007BB900/0x00506E40/0x007BB500/0x0046C830) and the fixed-node controller
// (0x00667C80) are not reconstructed; this table entry is zombie-only, where
// retail leaves the candidate node NULL.
// ---------------------------------------------------------------------------
extern badplace_t g_badplaces[256]; // actor_badplace.cpp

// Retail 0x005A29F0: is the goal position inside any active bad place?
static bool Actor_IsGoalInAnyBadPlace_SP(const float *pos)
{
    for ( unsigned int i = 0; i < 0x100; ++i )
    {
        const badplace_t *bp = &g_badplaces[i];
        switch ( bp->type )
        {
        case 1u:
        case 3u:
        case 4u:
            if ( IsPosInsideArc(
                     pos,
                     15.0f,
                     bp->parms.arc.origin,
                     bp->parms.arc.radius,
                     bp->parms.arc.angle0,
                     bp->parms.arc.angle1,
                     bp->parms.arc.halfheight) )
                return true;
            break;
        case 2u:
            if ( SV_EntityContact(pos, pos, bp->parms.brush.volume) )
                return true;
            break;
        default:
            break;
        }
    }
    return false;
}

// Retail 0x007CF300: claimed node has a visibility link to the enemy's node.
static bool Actor_ClaimedNodeSeesEnemyNode_SP(actor_s *self)
{
    pathnode_t *claimed = self->sentient->pClaimedNode;
    if ( !claimed )
        return false;
    pathnode_t *enemyNode = Sentient_NearestNode(Actor_GetTargetSentient(self));
    return enemyNode && Path_NodesVisible(claimed, enemyNode);
}

// Retail 0x0050DE30: enemy position known within the last 10 seconds.
static bool Actor_EnemyPosRecentlyKnown_SP(actor_s *self, int hadPath)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( !enemy )
        return false;
    if ( !hadPath && Actor_ClaimedNodeSeesEnemyNode_SP(self) )
        return true;
    const int t = self->sentientInfo[enemy - level.sentients].lastKnownPosTime;
    return t && level.time - t < 10000;
}

// Retail 0x0061C020: enemy seen within the last 10 seconds.
static bool Actor_EnemyRecentlyVisible_SP(actor_s *self, int checkNode)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( !enemy )
        return Actor_CanSeeEntityEx(self, Actor_GetTargetEntity(self), self->fovDot, self->fMaxSightDistSqrd) != 0;
    if ( checkNode && Actor_ClaimedNodeSeesEnemyNode_SP(self) )
        return true;
    const int t = self->sentientInfo[enemy - level.sentients].VisCache.iLastVisTime;
    return t && level.time - t < 10000;
}

// Retail 0x007BB6A0: is the enemy within pathEnemyLookahead (plus 256 when we
// had no path) of the actor or of the next pathEnemyFightDist of path.
static bool Actor_IsEnemyAlongPath_SP(actor_s *self, const float *enemyPos, const float *lookaheadPos, int hadPath)
{
    const path_t *path = &self->Path;
    float range = self->pathEnemyLookahead;
    if ( !hadPath )
        range += 256.0f;
    const float rangeSq = range * range;

    float dx = enemyPos[0] - self->ent->r.currentOrigin[0];
    float dy = enemyPos[1] - self->ent->r.currentOrigin[1];
    if ( dx * dx + dy * dy < rangeSq )
        return true;

    dx = lookaheadPos[0] - enemyPos[0];
    dy = lookaheadPos[1] - enemyPos[1];
    float along = path->lookaheadDir[0] * dx + path->lookaheadDir[1] * dy;
    if ( along < path->fLookaheadDist )
    {
        float distSq;
        if ( along > 0.0f )
        {
            distSq = path->lookaheadDir[1] * dx - path->lookaheadDir[0] * dy;
            distSq *= distSq;
        }
        else
        {
            distSq = dx * dx + dy * dy;
        }
        if ( distSq < rangeSq )
            return true;
    }

    int i = path->lookaheadNextNode;
    const pathpoint_t *pt = &path->pts[i];
    dx = pt->vOrigPoint[0] - enemyPos[0];
    dy = pt->vOrigPoint[1] - enemyPos[1];
    along = pt->fDir2D[0] * dx + pt->fDir2D[1] * dy;
    if ( along < path->fLookaheadDistToNextNode )
    {
        float distSq;
        if ( along > 0.0f )
        {
            distSq = pt->fDir2D[1] * dx - pt->fDir2D[0] * dy;
            distSq *= distSq;
        }
        else
        {
            distSq = dx * dx + dy * dy;
        }
        if ( distSq < rangeSq )
            return true;
    }

    float total = path->fLookaheadDistToNextNode + path->fLookaheadDist;
    while ( total < self->pathEnemyFightDist && --i >= 0 )
    {
        pt = &path->pts[i];
        dx = pt->vOrigPoint[0] - enemyPos[0];
        dy = pt->vOrigPoint[1] - enemyPos[1];
        along = pt->fDir2D[0] * dx + pt->fDir2D[1] * dy;
        if ( along < pt->fOrigLength )
        {
            float distSq;
            if ( along > 0.0f )
            {
                distSq = pt->fDir2D[1] * dx - pt->fDir2D[0] * dy;
                distSq *= distSq;
            }
            else
            {
                distSq = dx * dx + dy * dy;
            }
            if ( distSq < rangeSq )
                return true;
        }
        total += pt->fOrigLength;
    }
    return false;
}

// Retail 0x007BD830: decide whether the enemy is close enough to the path
// that the actor should stop following it (true => caller clears the path).
// May switch the code goal to the enemy (useEnemyGoal).
static bool Actor_CheckStopForEnemy_SP(actor_s *self, bool enemyKnown, pathnode_t *node, int hadPath)
{
    gentity_s *enemyEnt = Actor_GetTargetEntity(self);
    sentient_s *enemy = enemyEnt ? enemyEnt->sentient : NULL;
    if ( !enemy )
        return false;
    if ( !Actor_HasPath(self) )
        return false;

    // Retail 0x007BB620: enemy inside pathEnemyFightDist / goal height.
    const float *enemyPos = enemy->ent->r.currentOrigin;
    const float *myPos = self->ent->r.currentOrigin;
    {
        const float ex = enemyPos[0] - myPos[0];
        const float ey = enemyPos[1] - myPos[1];
        if ( ex * ex + ey * ey >= self->pathEnemyFightDist * self->pathEnemyFightDist )
            return false;
        if ( fabsf(enemyPos[2] - myPos[2]) > self->codeGoal.height )
            return false;
    }
    if ( node && Actor_PointNearNode(myPos, node) )
        return false;
    // Retail 0x00555B90: within 30 units (80 vertical) of the arrival pos.
    if ( self->arrivalInfo.animscriptOverrideRunTo
        && Actor_PointNearPoint(myPos, self->arrivalInfo.animscriptOverrideRunToPos, 30.0f) )
        return false;
    if ( !Actor_CanSeeEnemy(self) )
        return false;

    const path_t *path = &self->Path;
    float lookaheadPos[2];
    lookaheadPos[0] = path->lookaheadDir[0] * path->fLookaheadDist + myPos[0];
    lookaheadPos[1] = path->lookaheadDir[1] * path->fLookaheadDist + myPos[1];
    const float dx = lookaheadPos[0] - enemyPos[0];
    const float dy = lookaheadPos[1] - enemyPos[1];
    if ( path->lookaheadDir[1] * dy + path->lookaheadDir[0] * dx > 0.0f
        && path->fLookaheadDist * path->fLookaheadDist > dy * dy + dx * dx )
    {
        if ( enemyKnown && !self->useEnemyGoal )
        {
            self->useEnemyGoal = true;
            Actor_UpdateGoalPos(self);
        }
        return true;
    }

    if ( Actor_PointAtGoal(myPos, &self->codeGoal) && Actor_CanSeeEnemy(self) )
    {
        if ( !node )
            return true;
        const float nx = node->constant.vOrigin[0] - enemyPos[0];
        const float ny = node->constant.vOrigin[1] - enemyPos[1];
        if ( Path_DistanceGreaterThan(&self->Path, sqrtf(nx * nx + ny * ny)) )
            return true;
    }

    if ( !enemyKnown
        || self->useEnemyGoal
        || Actor_PointAtGoal(myPos, &self->codeGoal)
        || !Actor_IsEnemyAlongPath_SP(self, enemyPos, lookaheadPos, hadPath) )
    {
        return false;
    }

    self->useEnemyGoal = true;
    Actor_UpdateGoalPos(self);
    pathnode_t *claimed = self->sentient->pClaimedNode;
    if ( claimed && Actor_PointAtGoal(claimed->constant.vOrigin, &self->codeGoal) )
        return node != claimed;
    if ( node && Actor_PointAtGoal(node->constant.vOrigin, &self->codeGoal) )
        return false;
    return true;
}

// Retail 0x007BDA90: path to the animscript-requested arrival position,
// restoring the previous path when that fails.
static void Actor_TryPathToArrivalPos_SP(actor_s *self)
{
    static path_t pathBackup; // retail 0x01A505F0

    memcpy(&pathBackup, &self->Path, sizeof(path_t));
    if ( !Actor_FindPath(self, self->arrivalInfo.animscriptOverrideRunToPos, 1, false) )
    {
        Actor_ClearPath(self);
        memcpy(&self->Path, &pathBackup, sizeof(path_t));
        if ( self->Path.wPathLen > 0 && self->Path.wNegotiationStartNode > 0 )
            ++Path_ConvertIndexToNode(self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum)->dynamic.userCount;
        self->arrivalInfo.animscriptOverrideRunTo = 0;
        self->arrivalInfo.arrivalNotifyRequested = 0;
    }
}

// Retail 0x00488D70: per-think goal and path maintenance for the exposed
// state (repath throttle on iTeamMoveWaitTime, bad-place goals, goal_changed
// notify, enemy-goal switching and the stop-for-enemy decision).
static void Actor_FindPathToGoal_SP(actor_s *self)
{
    pathnode_t *node = NULL; // species-0 candidate selection not reconstructed

    if ( self->ent->tagInfo )
    {
        Actor_ClearPath(self);
        return;
    }
    const int hadPath = Actor_HasPath(self);
    if ( !hadPath && level.time < self->iTeamMoveWaitTime )
        return;

    if ( self->badPlaceAwareness > 0.0f && Actor_IsGoalInAnyBadPlace_SP(self->codeGoal.pos) )
        goto clear_path;

    if ( self->fixedNode )
    {
        // Retail tail-calls the fixed-node controller 0x00667C80 (not
        // reconstructed; zombies never use fixed nodes).  Keep the previous
        // bounded behaviour for that case.
        self->useEnemyGoal = false;
        Actor_UpdateGoalPos(self);
        Actor_FindPathToGoalDirect(self);
        return;
    }

    {
        const bool enemyKnown = Actor_EnemyPosRecentlyKnown_SP(self, hadPath);
        if ( !enemyKnown )
            self->useEnemyGoal = false;

        const float prevGoal[3] = { self->codeGoal.pos[0], self->codeGoal.pos[1], self->codeGoal.pos[2] };
        Actor_UpdateGoalPos(self);
        self->goalPosChanged = self->codeGoal.pos[0] != prevGoal[0]
                            || self->codeGoal.pos[1] != prevGoal[1]
                            || self->codeGoal.pos[2] != prevGoal[2];
        if ( self->goalPosChanged )
            Scr_Notify(self->ent, scr_const.goal_changed, 0);

        if ( Actor_KeepClaimedNode(self) )
            goto clear_path;

        sentient_s *sentient = self->sentient;
        pathnode_t *prevClaimed = sentient->pClaimedNode;

        if ( self->arrivalInfo.animscriptOverrideRunTo )
        {
            Actor_TryPathToArrivalPos_SP(self);
        }
        else if ( !self->useEnemyGoal )
        {
            Actor_FindPathToGoalDirect(self);
        }
        else
        {
            // Retail 0x00489022: with no candidate node the enemy goal is
            // dropped again and any path to it discarded.
            self->useEnemyGoal = false;
            Actor_UpdateGoalPos(self);
            if ( Actor_HasPath(self) )
            {
                Actor_ClearPath(self);
                Actor_TeamMoveBlocked(self);
            }
        }

        if ( Actor_CheckStopForEnemy_SP(self, enemyKnown, node, hadPath) )
        {
            // Retail 0x00488E20 (unconditional clear + block here).
            Actor_ClearPath(self);
            Actor_TeamMoveBlocked(self);
            return;
        }

        if ( node || Actor_HasPath(self) )
            Sentient_ClaimNode(sentient, node);
        if ( self->goalPosChanged )
            return;
        if ( sentient->pClaimedNode && prevClaimed != sentient->pClaimedNode )
        {
            self->goalPosChanged = true;
            Scr_Notify(self->ent, scr_const.goal_changed, 0);
        }
        return;
    }

clear_path:
    if ( Actor_HasPath(self) )
    {
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
    }
}

// Retail 0x007D30D0: in combat with the enemy visible, should the remaining
// path be abandoned?
static bool Actor_Exposed_ShouldClearCombatPath_SP(actor_s *self)
{
    if ( self->pPileUpActor || self->pCloseEnt.isDefined() )
        return true;
    if ( !Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal) )
        return false;
    if ( self->sentient->pClaimedNode && !Actor_KeepClaimedNode(self) )
        return Actor_PointAt(self->ent->r.currentOrigin, self->sentient->pClaimedNode->constant.vOrigin);

    float shrink = self->codeGoal.radius * 0.5f;
    if ( shrink >= 64.0f )
        shrink = 64.0f;
    // Retail 0x004AC4E0: inside the goal cylinder shrunk by 'shrink'.
    const float *pos = self->ent->r.currentOrigin;
    const float dz = pos[2] - self->codeGoal.pos[2];
    if ( dz * dz > self->codeGoal.height * self->codeGoal.height )
        return false;
    const float dx = self->codeGoal.pos[0] - pos[0];
    const float dy = self->codeGoal.pos[1] - pos[1];
    const float r = self->codeGoal.radius - shrink;
    return dx * dx + dy * dy <= r * r;
}

// Retail 0x007C0E30: standing at the goal but well off its centre; switch to
// a code-driven stop so the actor can be nudged without a script change.
static void Actor_Exposed_CheckCodeStop_SP(actor_s *self)
{
    if ( self->useEnemyGoal
        || self->eAnimMode == AI_ANIM_MOVE_CODE
        || self->eAnimMode == AI_ANIM_USE_ANGLE_DELTAS
        || self->mayMoveTime + 1000 > level.time
        || Actor_KeepClaimedNode(self) )
    {
        return;
    }
    const float r = self->codeGoal.radius - 30.0f;
    if ( r > 0.0f )
    {
        const float dx = self->codeGoal.pos[0] - self->ent->r.currentOrigin[0];
        const float dy = self->codeGoal.pos[1] - self->ent->r.currentOrigin[1];
        if ( dx * dx + dy * dy < r * r )
            return;
    }
    self->moveMode = AI_MOVE_STOP;
    self->eAnimMode = AI_ANIM_MOVE_CODE;
}

// Retail 0x007D2D80: exposed substate transitions, run before dispatch.
static void Actor_Exposed_DecideSubState_SP(actor_s *self)
{
    if ( !Actor_GetTargetEntity(self) )
    {
        Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        return;
    }
    const ai_substate_t subState = self->eSubState[self->stateLevel];
    if ( subState == STATE_EXPOSED_NONCOMBAT )
    {
        if ( Actor_EnemyRecentlyVisible_SP(self, 1) )
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    }
    else if ( subState == STATE_EXPOSED_REACQUIRE_MOVE )
    {
        if ( !Actor_HasPath(self) || self->pPileUpActor || self->pCloseEnt.isDefined() )
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
        // Retail additionally requires a clear muzzle-to-enemy capsule trace
        // (0x0059BAA0, not reconstructed) before leaving the reacquire move
        // early; without it the move simply runs to the end of its path.
    }
}

// Retail 0x007D2E70: noncombat substate controller.
static void Actor_Exposed_NonCombat_SP(actor_s *self)
{
    const pathnode_t *prevClaimed = self->sentient->pClaimedNode;
    const bool prevClaimedValid = prevClaimed && (prevClaimed->constant.spawnflags & 0x8000) != 0;

    Actor_FindPathToGoal_SP(self);
    if ( Actor_HasPath(self) )
    {
        self->pszDebugInfo = "noncombat_move";
        const float *pos = self->ent->r.currentOrigin;
        const float dx = pos[0] - self->Path.vFinalGoal[0];
        const float dy = pos[1] - self->Path.vFinalGoal[1];
        const float dz = pos[2] - self->Path.vFinalGoal[2];
        // Run unless the remaining straight-line distance is under fWalkDist.
        Actor_MoveAlongPathWithTeam(
            self, self->fWalkDist * self->fWalkDist <= dx * dx + dy * dy + dz * dz, true, true);

        ai_orient_mode_t mode = AI_ORIENT_TO_MOTION;
        // Retail 0x007D2E30: a fresh (<2s) path shorter than 60 units into a
        // valid claimed node orients to the goal instead of to motion.
        if ( prevClaimedValid
            && level.time - self->Path.iPathTime < 2000
            && !Path_DistanceGreaterThan(&self->Path, 60.0f)
            && !self->arrivalInfo.animscriptOverrideRunTo )
        {
            mode = AI_ORIENT_TO_GOAL;
        }
        if ( Actor_IsAtGoal(self) && !prevClaimedValid )
            mode = AI_ORIENT_TO_ENEMY_OR_MOTION;
        Actor_SetOrientMode(self, mode);
        return;
    }

    self->pszDebugInfo = "noncombat_stop";
    if ( !Actor_IsAtGoal(self) )
    {
        if ( Actor_GetTargetEntity(self) )
            Actor_Zombie_ExposedCombat_SP(self);
        else
            Actor_AnimStop(self, Actor_GetStopAnim(self));
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
        return;
    }
    if ( prevClaimedValid )
    {
        Actor_AnimStop(self, Actor_GetStopAnim(self));
        Actor_SetOrientMode(self, AI_ORIENT_TO_GOAL);
        return;
    }
    Actor_Exposed_CheckCodeStop_SP(self);
    if ( Actor_GetTargetEntity(self) )
        Actor_Zombie_ExposedCombat_SP(self);
    else
        Actor_AnimStop(self, Actor_GetStopAnim(self));

    const float goalYaw = self->codeGoal.ang[1];
    if ( goalYaw == 0.0f )
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
        return;
    }
    // Consume the script goal yaw (SetGoalPos( pos, angles )).
    self->bNotifyTurnDone = 1;
    self->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
    Actor_SetDesiredAngles(&self->ScriptOrient, 0.0f, goalYaw);
    self->codeGoal.ang[1] = 0.0f;
    self->scriptGoal.ang[1] = 0.0f;
}

// Retail's human and zombie exposed tables share these three callbacks:
// start 0x0062a110, finish 0x005f8970 and resume 0x004544c0.  The bodies were
// recovered from the table pointers at 0x00a519e8/0x00a51b68, not inferred
// from the MP table.  The retail field at actor_s+0x224c is bProneOK: it gates
// writes to the adjacent SP ProneInfo members in all three bodies.
bool __fastcall Actor_Exposed_Start_SP(actor_s *self, ai_state_t)
{
    if ( self->bProneOK )
    {
        self->ProneInfo.prone = true;
        self->ProneInfo.orientPitch = true;
        self->ProneInfo.iProneTime = level.time;
        self->ProneInfo.iProneTrans = 500;
    }
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    return true;
}

void __fastcall Actor_Exposed_Finish_SP(actor_s *self, ai_state_t)
{
    if ( self->bProneOK )
    {
        self->ProneInfo.prone = false;
        self->ProneInfo.orientPitch = false;
        self->ProneInfo.fTorsoPitch = 0.0f;
    }
}

bool __fastcall Actor_Exposed_Resume_SP(actor_s *self, ai_state_t)
{
    if ( self->bProneOK )
    {
        self->ProneInfo.prone = true;
        self->ProneInfo.orientPitch = true;
        self->ProneInfo.iProneTime = level.time;
        self->ProneInfo.iProneTrans = 500;
    }
    return true;
}

actor_think_result_t __fastcall Actor_Exposed_Think_SP(actor_s *self)
{
    // Retail zombie exposed think is 0x005591f0. This bounded controller
    // preserves the existing goal-maintenance fallback; full SP substate,
    // visibility/path-clear and fixed-node orientation parity remains open.
    // The combat fallback below now follows the reviewed zombie selector
    // at 0x00426710, reached through 0x007c0ef0.
    self->pszDebugInfo = "exposed";
    if ( self->bProneOK )
        Actor_OrientPitchToGround(self->ent, 1);

    Actor_PreThink(self);
    Actor_Exposed_DecideSubState_SP(self);
    switch ( self->eSubState[self->stateLevel] )
    {
    case STATE_EXPOSED_COMBAT:
        self->pszDebugInfo = "exposed_combat";
        // Retail 0x00559245: full goal/path maintenance (0x00488D70).
        Actor_FindPathToGoal_SP(self);
        // Retail 0x0055924C..0x00559270: once the enemy is visible and the
        // path merely ends inside the goal, drop the path and fight.
        if ( Actor_HasPath(self) && Actor_CanSeeEnemy(self) && Actor_Exposed_ShouldClearCombatPath_SP(self) )
            Actor_ClearPath(self);
        if ( Actor_HasPath(self) && self->safeToChangeScript )
        {
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
            Actor_MoveAlongPathWithTeam(self, true, true, true);
        }
        else
        {
            Actor_Zombie_ExposedCombat_SP(self);
            // Retail 0x007C0EF0 -> 0x007C0E30.
            if ( Actor_IsAtGoal(self) && Actor_GetTargetSentient(self) )
                Actor_Exposed_CheckCodeStop_SP(self);
        }
        break;

    case STATE_EXPOSED_NONCOMBAT:
        Actor_Exposed_NonCombat_SP(self);
        break;

    case STATE_EXPOSED_REACQUIRE_MOVE:
        // Retail 0x005592F9 has no outer HasPath gate here; the substate
        // decision above already left this substate when the path vanished,
        // so the guard below is only defensive.
        self->pszDebugInfo = "exposed_reacquire_move";
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP);
        if ( Actor_HasPath(self) )
            Actor_MoveAlongPathWithTeam(self, true, false, true);
        break;

    default:
        // Retail only post-thinks for other substates (incl. flashbanged).
        break;
    }

    Actor_PostThink(self);
    return ACTOR_THINK_DONE;
}
#endif

void __fastcall Actor_Exposed_FindReacquireNode(actor_s *self)
{
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
        self->iPotentialReacquireNodeCount = 0;
    else
        self->iPotentialReacquireNodeCount = 0;
}

pathnode_t *__fastcall Actor_Exposed_GetReacquireNode(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 320, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 321, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
    {
        if ( self->iPotentialReacquireNodeCount )
        {
            if ( Actor_HasPath(self) || level.time >= self->iTeamMoveWaitTime )
            {
                return self->pPotentialReacquireNode[--self->iPotentialReacquireNodeCount];
            }
            else
            {
                self->iPotentialReacquireNodeCount = 0;
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
        self->iPotentialReacquireNodeCount = 0;
        return 0;
    }
}

char __fastcall Actor_Exposed_UseReacquireNode(actor_s *self, pathnode_t *pNode)
{
    float vPoint[3]; // [esp+14h] [ebp-1Ch] BYREF
    gentity_s *targetEnt; // [esp+20h] [ebp-10h]
    float vFrom[3]; // [esp+24h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 355, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 356, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_KeepClaimedNode(self)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp",
                    357,
                    0,
                    "%s",
                    "!Actor_KeepClaimedNode( self )") )
    {
        __debugbreak();
    }
    targetEnt = Actor_GetTargetEntity(self);
    if ( !targetEnt || self->eState[self->stateLevel] != AIS_EXPOSED )
        return 0;
    if ( !Path_CanClaimNode(pNode, self->sentient) )
        return 0;
    vFrom[0] = pNode->constant.vOrigin[0];
    vFrom[1] = pNode->constant.vOrigin[1];
    vFrom[2] = pNode->constant.vOrigin[2];
    vFrom[2] = vFrom[2] + 64.0;
    Actor_GetTargetLookPosition(self, vPoint);
    if ( !Actor_CanSeePointFrom(self, vFrom, vPoint, 0.0, targetEnt->s.number) )
        return 0;
    if ( !Actor_FindPathToNode(self, pNode, 1) )
        return 0;
    Sentient_ClaimNode(self->sentient, pNode);
    self->iPotentialReacquireNodeCount = 0;
    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    return 1;
}

char __fastcall Actor_Exposed_ReacquireStepMove(actor_s *self, float fDist)
{
    float *v4; // ecx
    float *currentOrigin; // [esp+28h] [ebp-80h]
    float fScale; // [esp+30h] [ebp-78h]
    float vStepDir[3]; // [esp+34h] [ebp-74h] BYREF
    float fYaw; // [esp+40h] [ebp-68h]
    float vTraceEndPos[3]; // [esp+44h] [ebp-64h] BYREF
    float stepheight; // [esp+50h] [ebp-58h]
    gentity_s *targetEnt; // [esp+54h] [ebp-54h]
    float vStartPos[3]; // [esp+58h] [ebp-50h] BYREF
    float forward[3]; // [esp+64h] [ebp-44h] BYREF
    float vEyePos[3]; // [esp+70h] [ebp-38h] BYREF
    int iOrder; // [esp+7Ch] [ebp-2Ch]
    int i; // [esp+80h] [ebp-28h]
    float vCheckPos[3]; // [esp+84h] [ebp-24h] BYREF
    float vEnemyPos[3]; // [esp+90h] [ebp-18h] BYREF
    float vMovePos[3]; // [esp+9Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 401, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 402, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    targetEnt = Actor_GetTargetEntity(self);
    if ( !targetEnt || self->eState[self->stateLevel] != AIS_EXPOSED )
        return 0;
    fYaw = self->fDesiredBodyYaw + 90.0;
    Actor_GetTargetLookPosition(self, vEnemyPos);
    YawVectors(fYaw, vStepDir, 0);
    forward[0] = targetEnt->r.currentOrigin[0];
    forward[1] = targetEnt->r.currentOrigin[1];
    forward[2] = targetEnt->r.currentOrigin[2];
    currentOrigin = self->ent->r.currentOrigin;
    forward[0] = forward[0] - *currentOrigin;
    forward[1] = forward[1] - currentOrigin[1];
    forward[2] = forward[2] - currentOrigin[2];
    if ( (float)((float)(forward[0] * forward[0]) + (float)(forward[1] * forward[1])) <= 1.0 )
        return 0;
    Vec2Normalize(forward);
    forward[2] = 0.0f;
    vStepDir[0] = forward[1];
    vStepDir[1] = -forward[0];
    vStepDir[2] = 0.0f;
    Actor_GetEyePosition(self, vEyePos);
    v4 = self->ent->r.currentOrigin;
    vStartPos[0] = *v4;
    vStartPos[1] = v4[1];
    vStartPos[2] = v4[2];
    if ( vStepDir[2] != 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 448, 0, "%s", "!vStepDir[2]") )
    {
        __debugbreak();
    }
    iOrder = G_rand() & 1;
    for ( i = 0; i < 2; ++i )
    {
        fScale = factor[iOrder ^ i] * fDist;
        vCheckPos[0] = (float)(fScale * vStepDir[0]) + vEyePos[0];
        vCheckPos[1] = (float)(fScale * vStepDir[1]) + vEyePos[1];
        vCheckPos[2] = vEyePos[2];
        if ( Actor_CanSeePointFrom(self, vCheckPos, vEnemyPos, self->fMaxSightDistSqrd, targetEnt->s.number) )
        {
            vMovePos[0] = (float)(fScale * vStepDir[0]) + vStartPos[0];
            vMovePos[1] = (float)(fScale * vStepDir[1]) + vStartPos[1];
            vMovePos[2] = vStartPos[2];
            stepheight = self->Physics.prone ? 10.0f : 18.0f;
            if ( Path_PredictionTrace(vStartPos, vMovePos, 1023, self->Physics.iTraceMask | 4, vTraceEndPos, stepheight, 1)
                && Actor_PointAtGoal(vTraceEndPos, &self->codeGoal) )
            {
                Actor_FindPath(self, vTraceEndPos, 0, 0);
                if ( Actor_HasPath(self) )
                {
                    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
                    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
                    return 1;
                }
            }
        }
    }
    return 0;
}

void __fastcall Actor_Exposed_FindReacquireDirectPath(actor_s *self, bool ignoreSuppression)
{
    sentient_s *enemy; // [esp+8h] [ebp-10h]
    float vEnemyPos[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 512, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 513, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
    {
        Actor_GetTargetPosition(self, vEnemyPos);
        enemy = Actor_GetTargetSentient(self);
        if ( enemy )
            Actor_FindPathToSentient(self, enemy, !ignoreSuppression);
        else
            Actor_FindPath(self, vEnemyPos, 1, ignoreSuppression);
        if ( !Actor_PointAtGoal(vEnemyPos, &self->codeGoal) )
            Actor_ClipPathToGoal(self);
        Actor_BeginTrimPath(self);
    }
    else
    {
        Actor_ClearPath(self);
    }
}

void __fastcall Actor_Exposed_FindReacquireProximatePath(actor_s *self, bool ignoreSuppression)
{
    float fWithinDistSqrd; // [esp+1Ch] [ebp-10h]
    float vEnemyPos[3]; // [esp+20h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 549, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 550, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
    {
        Actor_GetTargetPosition(self, vEnemyPos);
        fWithinDistSqrd = Vec3DistanceSq(self->ent->r.currentOrigin, vEnemyPos) * 0.25;
        if ( fWithinDistSqrd > (float)(self->fMaxSightDistSqrd * 0.0625) )
            fWithinDistSqrd = self->fMaxSightDistSqrd * 0.0625;
        Actor_FindPathInGoalWithLOS(self, vEnemyPos, fWithinDistSqrd, ignoreSuppression);
        Actor_BeginTrimPath(self);
    }
    else
    {
        Actor_ClearPath(self);
    }
}

char __fastcall Actor_Exposed_StartReacquireMove(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 585, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 586, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    self->TrimInfo.iIndex = 0;
    self->TrimInfo.iDelta = 0;
    if ( !Actor_GetTargetEntity(self) || self->eState[self->stateLevel] != AIS_EXPOSED )
        return 0;
    if ( !Actor_HasPath(self) )
        return 0;
    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    return 1;
}

void __fastcall Actor_Exposed_FlashBanged(actor_s *self)
{
    PROF_SCOPED("flashbanged");

    if ( !self->flashBanged
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp",
                    766,
                    0,
                    "%s",
                    "self->flashBanged") )
    {
        __debugbreak();
    }
    Actor_SetAnimScript(
        self,
        &g_animScriptTable[self->species]->flashed,
        AI_MOVE_STOP,
        AI_ANIM_USE_BOTH_DELTAS,
        AI_ANIM_FUNCTION_STOP);
    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
}

int __cdecl Path_IsValidClaimNode(const pathnode_t *node)
{
    if ( !node && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.h", 224, 0, "%s", "node") )
        __debugbreak();
    return node->constant.spawnflags & 0x8000;
}

void __fastcall Actor_Exposed_Touch(actor_s *self, gentity_s *pOther)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 920, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 921, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !pOther
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 922, 0, "%s", "pOther") )
    {
        __debugbreak();
    }
    if ( pOther->sentient )
        Actor_GetPerfectInfo(self, pOther->sentient);
}

