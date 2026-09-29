#pragma once
#include "actor.h"

void __fastcall Actor_Exposed_FindReacquireNode(actor_s *self);
pathnode_t *__fastcall Actor_Exposed_GetReacquireNode(actor_s *self);
char __fastcall Actor_Exposed_UseReacquireNode(actor_s *self, pathnode_t *pNode);
char __fastcall Actor_Exposed_ReacquireStepMove(actor_s *self, float fDist);
void __fastcall Actor_Exposed_FindReacquireDirectPath(actor_s *self, bool ignoreSuppression);
void __fastcall Actor_Exposed_FindReacquireProximatePath(actor_s *self, bool ignoreSuppression);
char __fastcall Actor_Exposed_StartReacquireMove(actor_s *self);
void __fastcall Actor_Exposed_FlashBanged(actor_s *self);
int __cdecl Path_IsValidClaimNode(const pathnode_t *node);
void __fastcall Actor_Exposed_Touch(actor_s *self, gentity_s *pOther);
#ifdef KISAK_SP
bool __fastcall Actor_Exposed_Start_SP(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Exposed_Finish_SP(actor_s *self, ai_state_t eNextState);
bool __fastcall Actor_Exposed_Resume_SP(actor_s *self, ai_state_t ePrevState);
actor_think_result_t __fastcall Actor_Exposed_Think_SP(actor_s *self);
#endif
