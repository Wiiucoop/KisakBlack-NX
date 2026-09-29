#pragma once
#include "actor.h"

enum scriptAnimAIFunctionTypes_t : __int32
{                                       // XREF: ?Actor_SetAnimScript@@YIXPAUactor_s@@PAUscr_animscript_t@@EW4ai_animmode_t@@W4scriptAnimAIFunctionTypes_t@@@Z/r
    AI_ANIM_FUNCTION_STOP   = 0x0,
    AI_ANIM_FUNCTION_MOVE   = 0x1,
    AI_ANIM_FUNCTION_COMBAT = 0x2,
    AI_ANIM_FUNCTION_PAIN   = 0x3,
    AI_ANIM_FUNCTION_REACT  = 0x4,
    AI_ANIM_FUNCTION_DEATH  = 0x5,
};

// RETAIL SP LAYOUT, recovered 2026-08-28 from the animscript SET pass (0x007eeda0 human,
// 0x007ef000 dog, 0x007ef090 zombie, 0x007ef190 zombie_dog), each of which pushes the absolute
// address of the member it fills. Member offsets, all stride 8 (sizeof(scr_animscript_t)):
//   +0x00 combat              +0x40 cover_prone          +0x80 pain
//   +0x08 concealment_crouch  +0x48 cover_right          +0x88 react
//   +0x10 concealment_prone   +0x50 cover_stand          +0x90 move
//   +0x18 concealment_stand   +0x58 cover_wide_left      +0x98 scripted
//   +0x20 cover_arrival       +0x60 cover_wide_right     +0xa0 stop
//   +0x28 cover_crouch        +0x68 death                +0xa8 grenade_cower
//   +0x30 cover_left          +0x70 grenade_return_throw +0xb0 flashed
//   +0x38 cover_pillar        +0x78 init                 +0xb8 turn
//
// cover_pillar is the one member this MP-derived struct was missing; every member after it sat
// 8 bytes low against SP. Confirmed by a READER rather than the loaders (independent second
// fact): Actor_AnimTryRun at 0x0051ac94 computes g_animScriptTable[species] then ADD EDX,0x90
// for ->move and ADD EDX,0xa0 for ->stop, which are this table's offsets and NOT the MP ones
// (+0x88 / +0x98). animscripts/cover_pillar.gsc ships in common.ff and frontend.ff.
//
// DELIBERATE DEVIATION, recorded rather than silently taken: retail SP has no `jump` member
// (nothing in SP loads dog_jump; the slot retail uses at +0xb8 is `turn`). `jump` is kept here
// for both configs because Game/Server/game/actor_dog_exposed.cpp compares against its ADDRESS
// at three sites, and removing it under KISAK_SP would break those for no functional gain --
// nothing serialises AnimScriptList, so only self-consistency is load-bearing. The cost is that
// SP's `turn` lands at +0xc0 here instead of retail's +0xb8. Same reasoning for weapons[]:
// retail SP's stride between species lists is 0x4C0, which leaves 0x400 after the 24-member
// head and so implies weapons[128], but that is gap arithmetic rather than a positive
// observation, and under-sizing an array indexed by weapon number is the dangerous direction --
// so it stays at 2048 for both configs.
struct AnimScriptList // sizeof=0x40C0 (MP); SP retail is 0x4C0, see note above
{                                       // XREF: scr_data_t/r
    scr_animscript_t combat;
    scr_animscript_t concealment_crouch;
    scr_animscript_t concealment_prone;
    scr_animscript_t concealment_stand;
    scr_animscript_t cover_arrival;
    scr_animscript_t cover_crouch;
    scr_animscript_t cover_left;
#ifdef KISAK_SP
    scr_animscript_t cover_pillar;      // retail SP +0x38; absent from the MP set
#endif
    scr_animscript_t cover_prone;
    scr_animscript_t cover_right;
    scr_animscript_t cover_stand;
    scr_animscript_t cover_wide_left;
    scr_animscript_t cover_wide_right;
    scr_animscript_t death;             // XREF: .text:006418CE/o
    scr_animscript_t grenade_return_throw;
    scr_animscript_t init;              // XREF: .text:006418E4/o
    scr_animscript_t pain;              // XREF: .text:006418FA/o
    scr_animscript_t react;
    scr_animscript_t move;              // XREF: .text:00641910/o
    scr_animscript_t scripted;
    scr_animscript_t stop;              // XREF: .text:00641926/o
    scr_animscript_t grenade_cower;
    scr_animscript_t flashed;           // XREF: .text:0064193C/o
    scr_animscript_t jump;              // XREF: .text:00641952/o
                                        // Actor_Dog_Exposed_Think(actor_s *)+39F/o
    scr_animscript_t turn;              // XREF: .text:00641968/o
                                        // Actor_Dog_Exposed_Think(actor_s *)+3E0/o ...
    scr_animscript_t weapons[2048];
};

void __fastcall Actor_InitAnim(actor_s *self);
bool __fastcall Actor_IsAnimScriptAlive(actor_s *self);
void __fastcall Actor_KillAnimScript(actor_s *self);
void __fastcall Actor_SetAnimScript(
                actor_s *self,
                scr_animscript_t *pAnimScriptFunc,
                ai_movemode_t moveMode,
                ai_animmode_t animMode,
                scriptAnimAIFunctionTypes_t animScript);
void __fastcall Actor_AnimStop(actor_s *self, scr_animscript_t *pAnimScriptFunc);
void __fastcall Actor_AnimMoveAway(actor_s *self, scr_animscript_t *pAnimScriptFunc);
scr_animscript_t *__fastcall Actor_GetStopAnim(actor_s *self);
void __fastcall Actor_AnimTryWalk(actor_s *self);
void __fastcall Actor_AnimTryRun(actor_s *self);
void __fastcall Actor_AnimPain(actor_s *self);
void __fastcall Actor_AnimDeath(actor_s *self);
void __fastcall Actor_AnimSpecific(actor_s *self, scr_animscript_t *func, ai_animmode_t eAnimMode, bool bUseGoalWeight);
void __stdcall Actor_AnimSetCompleteGoalWeight(
                XAnimTree_s *tree,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart);
void __stdcall Actor_AnimClearGoalWeight(unsigned int animIndex, float blendTime);

extern AnimScriptList *g_animScriptTable[MAX_AI_SPECIES];