#include "actor.h"

#include "actor_dog_exposed.h"
#include "actor_generic.h"
#include "actor_exposed.h"
#include "actor_death.h"
#include "actor_pain.h"
#include "actor_negotiation.h"
#ifdef KISAK_SP
#include "actor_animapi.h"
#include "actor_orientation.h"
#include "actor_state.h"
#include "sentient.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>

// Retail state 9 uses the same callbacks in all four SP species tables. Its
// start/finish bodies toggle eFlags2 bit 1; its think body runs the species'
// scripted animscript, keeps the actor fixed to the scripted scene, and pops
// the state when either the GSC thread or per-entity scripted anim ends.
static bool __fastcall Actor_ScriptedAnim_Start_SP(actor_s *self, ai_state_t)
{
  Com_Printf(15, "SP scripted anim state start: ent %d species %d\n", self->ent->s.number, self->species);
  self->ent->s.lerp.eFlags2 |= 2u;
  return true;
}

static void __fastcall Actor_ScriptedAnim_Finish_SP(actor_s *self, ai_state_t)
{
  self->ent->s.lerp.eFlags2 &= ~2u;
  GScr_ClearScriptedAnim_SP(self->ent);
}

static actor_think_result_t __fastcall Actor_ScriptedAnim_Think_SP(actor_s *self)
{
  self->pszDebugInfo = "animscripted";
  Actor_ClearKeepClaimedNode(self);
  Sentient_ClaimNode(self->sentient, NULL);
  Actor_ClearPath(self);
  Actor_AnimSpecific(self, &g_animScriptTable[self->species]->scripted, AI_ANIM_USE_BOTH_DELTAS, true);
  self->pushable = false;

  if ( !Actor_IsAnimScriptAlive(self) )
  {
    Com_Printf(
        15,
        "SP scripted anim state ended: ent %d animscript not alive, alignment %d\n",
        self->ent->s.number,
        GScr_IsScriptedAnimActive_SP(self->ent));
    if ( self->eSimulatedState[self->simulatedStateLevel] == AIS_SCRIPTEDANIM )
      Actor_PopState(self);
    return ACTOR_THINK_REPEAT;
  }

  GScr_UpdateScriptedAnim_SP(self->ent);
  if ( !GScr_IsScriptedAnimActive_SP(self->ent) )
  {
    Actor_PopState(self);
    return ACTOR_THINK_REPEAT;
  }

  const float scriptedOrigin[3] = {
      self->ent->r.currentOrigin[0],
      self->ent->r.currentOrigin[1],
      self->ent->r.currentOrigin[2]
  };
  Actor_PreThink(self);
  static int lastOriginProbeTime_SP[1024] = {};
  const int entNum = self->ent->s.number;
  if ( static_cast<unsigned int>(entNum) < 1024u
      && level.time - lastOriginProbeTime_SP[entNum] >= 1000 )
  {
    lastOriginProbeTime_SP[entNum] = level.time;
    Com_Printf(
        15,
        "SP scripted actor post-think: ent %d scripted (%.2f %.2f %.2f) current (%.2f %.2f %.2f) trBase (%.2f %.2f %.2f) trDelta (%.2f %.2f %.2f) aposBase (%.2f %.2f %.2f) aposDelta (%.2f %.2f %.2f)\n",
        entNum,
        scriptedOrigin[0], scriptedOrigin[1], scriptedOrigin[2],
        self->ent->r.currentOrigin[0], self->ent->r.currentOrigin[1], self->ent->r.currentOrigin[2],
        self->ent->s.lerp.pos.trBase[0], self->ent->s.lerp.pos.trBase[1], self->ent->s.lerp.pos.trBase[2],
        self->ent->s.lerp.pos.trDelta[0], self->ent->s.lerp.pos.trDelta[1], self->ent->s.lerp.pos.trDelta[2],
        self->ent->s.lerp.apos.trBase[0], self->ent->s.lerp.apos.trBase[1], self->ent->s.lerp.apos.trBase[2],
        self->ent->s.lerp.apos.trDelta[0], self->ent->s.lerp.apos.trDelta[1], self->ent->s.lerp.apos.trDelta[2]);
  }
  Actor_SetDesiredAngles(&self->CodeOrient, self->ent->r.currentAngles[0], self->ent->r.currentAngles[1]);
  self->Physics.vVelocity[0] = 0.0f;
  self->Physics.vVelocity[1] = 0.0f;
  self->Physics.vVelocity[2] = 0.0f;
  self->Physics.vWishDelta[0] = 0.0f;
  self->Physics.vWishDelta[1] = 0.0f;
  self->Physics.vWishDelta[2] = 0.0f;
  return ACTOR_THINK_DONE;
}
#endif

const ai_funcs_t AIDogFuncTable[12] =
{
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Dog_Exposed_Start,
    Actor_Dog_Exposed_Finish,
    Actor_Dog_Exposed_Suspend,
    Actor_Generic_Resume,
    Actor_Dog_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_BadPlace_Flee_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_BadPlace_Flee_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Death_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Death_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Pain_Start,
    Actor_Pain_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Pain_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
#ifdef KISAK_SP
  {
    Actor_ScriptedAnim_Start_SP,
    Actor_ScriptedAnim_Finish_SP,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_ScriptedAnim_Think_SP,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
#else
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
#endif
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Negotiation_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Negotiation_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  }
};

#ifdef KISAK_SP
// Retail's species pointer table is at 0x00b750d8 and contains four entries:
// human 0x00a519e8, dog 0x00a51e68, zombie 0x00a51b68 and zombie-dog
// 0x00a51ce8.  Each retail table has 12 state rows.  State 1 in the zombie
// table uses the shared exposed start/finish/suspend/resume callbacks plus the
// zombie-specific think body at 0x005591f0; its remaining populated rows match
// the callbacks already reconstructed in AIDogFuncTable.  Keeping a distinct
// table here prevents AI_SPECIES_ZOMBIE (2) from indexing beyond the old MP
// one-entry AIFuncTable, which was the 0x007d7558 runtime fault.
const ai_funcs_t AIZombieFuncTable_SP[12] =
{
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Exposed_Start_SP,
    Actor_Exposed_Finish_SP,
    Actor_Generic_Suspend,
    Actor_Exposed_Resume_SP,
    Actor_Exposed_Think_SP,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_BadPlace_Flee_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_BadPlace_Flee_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Death_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Death_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Pain_Start,
    Actor_Pain_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Pain_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_ScriptedAnim_Start_SP,
    Actor_ScriptedAnim_Finish_SP,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_ScriptedAnim_Think_SP,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Negotiation_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Negotiation_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  }
};

const ai_funcs_t *AIFuncTable[MAX_AI_SPECIES] =
{
  AIZombieFuncTable_SP, // human: safe state-1 bridge; SP-only state 2/3 remain to recover
  AIDogFuncTable,
  AIZombieFuncTable_SP,
  AIDogFuncTable
};
#else
const ai_funcs_t *AIFuncTable[MAX_AI_SPECIES] = { AIDogFuncTable };
#endif

