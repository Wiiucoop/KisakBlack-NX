#pragma once
#include "actor.h"

enum ai_teammove_t : __int32
{                                       // XREF: ?Actor_MoveAlongPathWithTeam@@YAXPAUactor_s@@_N11@Z/r
                                        // Actor_GetTeamMoveStatus/r ...
    AI_TEAMMOVE_TRAVEL    = 0x0,
    AI_TEAMMOVE_WAIT      = 0x1,
    AI_TEAMMOVE_SLOW_DOWN = 0x2,
};

struct team_move_context_t // sizeof=0x118
{                                       // XREF: Actor_GetTeamMoveStatus/r
    actor_s *self;                      // XREF: Actor_GetTeamMoveStatus+171/w
    float vVelSelf[2];
    float vOrgSelf[3];                  // XREF: Actor_GetTeamMoveStatus+11B/w
                                        // Actor_GetTeamMoveStatus+12E/w ...
    float vVelDirSelf[2];               // XREF: Actor_GetTeamMoveStatus:loc_91DA09/r
                                        // Actor_GetTeamMoveStatus+52E/r ...
    float fVelSelfSqrd;
    float fDeltaCorrection;
    float fIntervalSqrd;
    float fWalkIntervalSqrd;
    float fMaxIntervalSqrd;
    int bFailedLookahead;               // XREF: Actor_GetTeamMoveStatus+C03/r
    float fDodgePosDeltaLengthSqrd;     // XREF: Actor_GetTeamMoveStatus+350/r
                                        // Actor_GetTeamMoveStatus+448/r ...
    int dodgeEntities[48];              // XREF: Actor_GetTeamMoveStatus+6D2/o
                                        // Actor_GetTeamMoveStatus+7CB/o
    int dodgeEntityCount;               // XREF: Actor_GetTeamMoveStatus+6CE/r
                                        // Actor_GetTeamMoveStatus+7C7/r
    sentient_s *pDodgeOther;            // XREF: Actor_GetTeamMoveStatus+29E/r
                                        // Actor_GetTeamMoveStatus+2E8/r ...
    bool bPileUp;
    // padding byte
    // padding byte
    // padding byte
    float fSlowDownPosDeltaLengthSqrd;
    sentient_s *pSlowDownOther;         // XREF: Actor_GetTeamMoveStatus:loc_91D759/r
    float vVelSlowDownOther[2];
};

struct team_move_other_context_t // sizeof=0x30
{                                       // XREF: Actor_GetTeamMoveStatus/r
    sentient_s *other;                  // XREF: Actor_GetTeamMoveStatus+207/w
    float vOrgOther[3];
    float vVelOther[2];
    float vDelta[2];
    float vPerp[2];
    float fPosDeltaLengthSqrd;
    float fScale;
};

void __fastcall Actor_TeamMoveBlocked(actor_s *self);
void __fastcall Actor_TeamMoveBlockedClear(actor_s *self);
char __cdecl Actor_TeamMoveCalcMovementDir(team_move_context_t *context, ai_teammove_t *result);
void __cdecl Actor_MoveAlongPathWithTeam(actor_s *self, bool bRun, bool bUseInterval, bool bAllowGoalPileUp);
ai_teammove_t __fastcall Actor_GetTeamMoveStatus(actor_s *self, bool bUseInterval, bool bAllowGoalPileUp);
bool Actor_TeamMoveCheckWaitTimer(actor_s *self, ai_teammove_t *result);
char __cdecl Actor_TeamMoveNeedToCheckWait(unsigned __int8 moveMode, path_t *pPath);
bool __cdecl Actor_IsEnemy(actor_s *self, sentient_s *other);
void __cdecl Actor_TeamMoveSetDodge(team_move_context_t *context, team_move_other_context_t *context_other);
char __cdecl Actor_TeamMoveShouldTryDodgeSentient(
                team_move_context_t *context,
                team_move_other_context_t *context_other);
void __cdecl Actor_AddToList(int *dodgeEntities, int *dodgeEntityCount, int arraysz, actor_s *pOtherActor);
bool __cdecl Actor_TeamMoveTryDodge(team_move_context_t *context, team_move_other_context_t *context_other);
char __cdecl Actor_TeamMoveConsiderSlowDown(team_move_context_t *context, ai_teammove_t *eResult);
ai_teammove_t __cdecl Actor_TeamMoveNoDodge(team_move_context_t *context, ai_teammove_t eResult);
void __cdecl Actor_TeamMoveInitializeContext(
                actor_s *self,
                bool bUseInterval,
                bool bAllowGoalPileUp,
                team_move_context_t *context);
void __cdecl Actor_CalcInterval(actor_s *self, bool bUseInterval, float *fIntervalSqrdOut, float *fWalkIntervalSqrdOut);
double __cdecl Actor_TeamMoveDeltaCorrection(actor_s *self, float fVelSelfSqrd);
int __cdecl Actor_TeamMoveTrimPath(path_t *pPath, const team_move_context_t *context);
void __cdecl Actor_TeamMoveTooCloseMoveAway(const actor_s *self, int mask, team_move_context_t *context);
char __cdecl Actor_TeamMoveCheckPileup(actor_s *self, actor_s *pOtherActor);

#ifdef KISAK_SP
extern const float g_actorAssumedSpeed[4];
#else
extern const float g_actorAssumedSpeed[1];
#endif
