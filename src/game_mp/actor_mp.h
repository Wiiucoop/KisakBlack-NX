#pragma once
#include <game/sentient.h>
#include <game/actor.h>
#include <game/actor_fields.h>

#define MAX_ACTORS 16
#define ACTOR_STOP_TIME 500

struct PhysicsInputs // sizeof=0x1C
{                                       // XREF: Actor_PhysicsAndDodge/r
    float vVelocity[3];
    unsigned __int16 groundEntNum;
    // padding byte
    // padding byte
    int bHasGroundPlane;
    float groundplaneSlope;
    int iFootstepTimer;
};

struct AITypeScript // sizeof=0xC
{
    int main;
    int precache;
    int spawner;
};

void __fastcall VisCache_Update(vis_cache_t *pCache, bool bVisible);
void __fastcall SentientInfo_Clear(sentient_info_t *pInfo);
void __fastcall SentientInfo_Copy(actor_s *pTo, const actor_s *pFrom, int index);
void __fastcall SentientInfo_ForceCopy(sentient_info_t *pTo, const sentient_info_t *pFrom);
int __cdecl Actor_droptofloor(gentity_s *ent);
int __cdecl SP_actor(gentity_s *ent, SpawnVar *spawnVar);
void __cdecl Actor_InitActorState(gentity_s *ent);
actor_s *__cdecl Actor_Alloc();
void __cdecl Scr_FreeFields(const actor_field_t *fields, unsigned __int8 *base);
void __cdecl Scr_FreeActorFields(actor_s *pActor);
void __cdecl Actor_Free(actor_s *actor);
void __cdecl Actor_FreeExpendable();
bool __cdecl Actor_IsDeletable(actor_s *actor);
void __cdecl G_InitActors();
int __cdecl G_GetActorIndex(actor_s *actor);
XAnimTree_s *__cdecl G_GetActorAnimTree(actor_s *actor);
void __cdecl Actor_SetDefaults(actor_s *actor);
void __cdecl Actor_FinishSpawning(actor_s *self);
void __cdecl Actor_InitAnimScript(actor_s *self);
void __cdecl Actor_FinishSpawningAll();
void __cdecl Actor_DissociateSentient(actor_s *self, sentient_s *other);
actor_s *__fastcall Actor_FirstActor(int iTeamFlags);
actor_s *__fastcall Actor_NextActor(actor_s *pPrevActor, int iTeamFlags);
void __fastcall Actor_NodeClaimRevoked(actor_s *self, int invalidTime);
char __fastcall Actor_KeepClaimedNode(actor_s *self);
void __fastcall Actor_ClearKeepClaimedNode(actor_s *self);
void __fastcall Actor_ClearArrivalPos(actor_s *self);
void __fastcall Actor_PreThink(actor_s *self);
void __fastcall Actor_PostThink(actor_s *self);
void __fastcall Actor_CheckClearNodeClaimCloseEnt(actor_s *self);
void __fastcall Actor_CheckNodeClaim(actor_s *self);
bool __fastcall Actor_IsDying(const actor_s *self);
gentity_s *__fastcall Actor_GetTargetEntity(actor_s *self);
sentient_s *__fastcall Actor_GetTargetSentient(actor_s *self);
void __fastcall Actor_GetTargetPosition(actor_s *self, float *position);
void __fastcall Actor_GetTargetLookPosition(actor_s *self, float *position);
gentity_s *__fastcall Actor_GetScriptTargetEntity(actor_s *self);
void __cdecl Actor_Think(gentity_s *self);
actor_think_result_t __fastcall Actor_CallThink(actor_s *self);
void __fastcall Actor_UpdateCloseEnt(actor_s *self);
void __cdecl Vec2Sub(const float *a, const float *b, float *diff);
double __cdecl Vec2DistanceSq(const float *v0, const float *v1);
void __fastcall Actor_UpdatePlayerPush(actor_s *self, gentity_s *player);
void __fastcall Actor_CheckNotify(actor_s *self);
bool __fastcall Actor_CheckGoalNotify(actor_s *self);
int __cdecl Actor_UpdateActorInfo(gentity_s *ent);
bool __cdecl Actor_InScriptedState(const actor_s *self);
void __cdecl Actor_Touch(gentity_s *self, gentity_s *other, int);
void __cdecl Actor_Pain(
                gentity_s *self,
                gentity_s *pAttacker,
                int iDamage,
                const float *vPoint,
                int iMod,
                const float *vDir,
                hitLocation_t hitLoc,
                int weaponIdx);
void __cdecl Actor_Die(
    gentity_s *self,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int iDamage,
    int iMod,
    const int iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc,
    int timeOffset);
bool __cdecl usingCodeGoal(actor_s *actor);
bool __cdecl isNodeInRegion(pathnode_t *node, gentity_s *volume);
void __fastcall Actor_DebugDrawNodesInVolume(actor_s *self);
void __cdecl Actor_EntInfo(gentity_s *self, float *source);
void __fastcall Actor_InitMove(actor_s *self);
bool __cdecl Actor_IsDodgeEntity(actor_s *self, int entnum);
void __fastcall Actor_DoMove(actor_s *self);
int __fastcall Actor_PhysicsAndDodge(actor_s *self);
int __fastcall Actor_Physics_GetLeftOrRightDodge(actor_s *self, bool dodgeRight, float length);
void __cdecl Actor_PhysicsBackupInputs(actor_s *self, PhysicsInputs *inputs);
void __cdecl Actor_PhysicsRestoreInputs(actor_s *self, PhysicsInputs *inputs);
bool __fastcall Actor_ShouldMoveAwayFromCloseEnt(actor_s *self);
int __fastcall Actor_PhysicsMoveAway(actor_s *self);
double __cdecl Actor_CalcultatePlayerPushDelta(const actor_s *self, const gentity_s *pusher, float *pushDir);
int __fastcall Actor_PhysicsCheckMoveAwayNoWorse(
                actor_s *self,
                gentity_s *other,
                int flags,
                float distanceSqrd,
                float lengthSqrd);
int __cdecl Actor_MoveAwayNoWorse(actor_s *self);
bool __cdecl Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther);
void __cdecl actor_controller(const gentity_s *self, int *partBits);
bool __fastcall Actor_PointNear(const float *vPoint, const float *vGoalPos);
bool __fastcall Actor_PointNearNode(const float *vPoint, const pathnode_t *node);
bool __fastcall Actor_PointAtGoal(const float *vPoint, const actor_goal_s *goal);
bool __fastcall Actor_PointNearPoint(const float *vPoint, const float *vGoalPos, float buffer);
bool __fastcall Actor_PointAt(const float *vPoint, const float *vGoalPos);
bool __fastcall Actor_IsAtGoal(actor_s *self);
bool __fastcall Actor_IsAtScriptGoal(actor_s *self);
bool __fastcall Actor_IsNearClaimedNode(actor_s *self);
gentity_s *__fastcall Actor_IsKnownEnemyInRegion(
                const actor_s *self,
                const gentity_s *volume,
                const float *position,
                float radius);
void __fastcall Actor_FindPathToGoalDirect(actor_s *self);
bool __fastcall Actor_FindPathToGoalDirectInternal(actor_s *self);
void __fastcall Actor_HandleInvalidPath(actor_s *self);
bool __fastcall Actor_FindPath(
                actor_s *self,
                const float *vGoalPos,
                int bAllowNegotiationLinks,
                bool ignoreSuppression);
void __fastcall Actor_RecalcPath(actor_s *self);
bool __fastcall Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable);
bool __fastcall Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable, int bIgnoreBadplaces);
bool __fastcall Actor_FindPathToSentient(actor_s *self, sentient_s *pGoalEnt, int bSuppressable);
void __fastcall Actor_FindPathInGoalWithLOS(
                actor_s *self,
                const float *vGoalPos,
                float fWithinDistSqrd,
                bool ignoreSuppression);
void __cdecl Actor_BadPlacesChanged();
bool __fastcall Actor_HasPath(actor_s *self);
void __fastcall Actor_InitPath(actor_s *self);
void __fastcall Actor_ClearPath(actor_s *self);
void __fastcall Actor_GetAnimDeltas(actor_s *self, float *rotation, float *translation);
void __fastcall Actor_UpdateAnglesAndDelta(actor_s *self);
void __fastcall Actor_PathEndActions(actor_s *self);
bool __fastcall Actor_IsMovingToMeleeAttack(actor_s *self);
bool __fastcall Actor_SkipPathEndActions(actor_s *self);
void __fastcall Actor_UpdateOriginAndAngles(actor_s *self);
void __fastcall Actor_PredictOriginAndAngles(actor_s *self);
void __fastcall Actor_PredictAnim(actor_s *self);
bool __fastcall Actor_AtClaimNode(actor_s *self);
bool __fastcall Actor_NearClaimNode(actor_s *self, float dist);
void __fastcall Actor_CheckCollisions(actor_s *self);
void __fastcall Actor_UpdatePileUp(actor_s *self);
void __fastcall Actor_ClearPileUp(actor_s *self);
void __fastcall Actor_ClipPathToGoal(actor_s *self);
void __fastcall Actor_BeginTrimPath(actor_s *self);
int __fastcall Actor_TrimPathToAttack(actor_s *self);
void __fastcall Actor_ClearMoveHistory(actor_s *self);
bool __fastcall Actor_MayReacquireMove(actor_s *self);
void __fastcall Actor_GetMoveHistoryAverage(actor_s *self, float *vDir);
void __fastcall Actor_UpdateMoveHistory(actor_s *self);
void __fastcall Actor_UpdateNetworkLeanAmount(actor_s *self);
void __fastcall Path_UpdateLeanAmount(actor_s *self, float *vWishDir);
double __fastcall Path_UpdateMomentum(actor_s *self, float *vWishDir, float fMoveDist);
void Path_UpdateMovementDelta(actor_s *self, float fMoveDist);
void __cdecl Actor_SetFlashed(actor_s *self, int flashed, float strength);
void __fastcall Actor_AddStationaryMoveHistory(actor_s *self);
bool __fastcall Actor_IsMoving(actor_s *self);
void __fastcall Actor_UpdateGoalPos(actor_s *self);
void __cdecl Actor_CheckOverridePos(actor_s *self, const float *prevGoalPos);
void __fastcall Actor_SetGoalRadius(actor_goal_s *goal, float radius);
void __fastcall Actor_SetGoalHeight(actor_goal_s *goal, float height);
bool __fastcall Actor_IsInsideArc(
                actor_s *self,
                const float *origin,
                float radius,
                float angle0,
                float angle1,
                float halfHeight);


const float ACTOR_DEFAULT_FOV_COS = 0.0099999998;
const float ACTOR_GOAL_ANGLE_TOLERANCE = 20.0;
const float ACTOR_DEFAULT_WALK_DIST = 128.0;
const float ACTOR_DEFAULT_INTERVAL = 96.0;
const float ACTOR_DEFAULT_GRENADE_AWARENESS = 0.33000001;
const float ACTOR_ENTINFO_SCALE = 0.60000002;
const float ACTOR_DROP_NUDGE_HEIGHT = 1.0;
const float ACTOR_DROP_HEIGHT = 128.0;
const float ACTOR_PATH_RETRY_FRACTION = 0.25;
const float ACTOR_EYE_OFFSET = 64.0;
const int ACTOR_MAX_HEALTH = 100;
const int ACTOR_TEAMMOVE_WAIT_TIME = 500;
const float ACTOR_DEFAULT_BADPLACE_AWARENESS = 0.75;
const float ACTOR_NEAR_NODE_DIST = 15.0;
const float ACTOR_PLAYER_PUSH_LOOKAHEAD = 0.1;
const float ACTOR_PLAYER_PUSH_MIN_SPEED = 60.0;

// means o' death
extern unsigned __int16 *modNames[21];