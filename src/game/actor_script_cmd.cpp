#include "actor_script_cmd.h"
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_vm.h>
#include "actor_state.h"
#include "actor_orientation.h"
#include <game_mp/actor_mp.h>
#include "actor_aim.h"
#include <game_mp/g_spawn_mp.h>
#include "actor_exposed.h"
#include "actor_lookat.h"
#include "actor_senses.h"
#include <cgame/cg_drawtools.h>
#include "actor_navigation.h"
#include <game_mp/g_utils_mp.h>
#include <clientscript/scr_const.h>
#include "actor_spawner.h"
#include <clientscript/cscr_stringlist.h>
#include <qcommon/dobj_management.h>
#include <bgame/bg_misc.h>
#include <server/sv_game.h>
#include "actor_events.h"
#include "actor_dog_exposed.h"
#include <bgame/bg_dog.h>
#include <game_mp/g_combat_mp.h>
#include "g_weapon.h"
#include <game_mp/g_client_script_cmd_mp.h>
#include <game_mp/g_misc_mp.h>
#include <game_mp/g_trigger_mp.h>
#include <game_mp/g_active_mp.h>
#include "g_scr_mover.h"
#include "g_missile.h"
#include "turret.h"
#include "g_scr_helicopter.h"
#include <cgame/cg_scr_main.h>

static void __cdecl METHOD_NULLSUB(scr_entref_t entref)
{

}


#ifdef KISAK_SP
#include <qcommon/common.h>
#include <game_mp/g_scr_main_mp.h>

// ===========================================================================
// TODO(SP-STUB) -- deliberate no-op stubs for retail-SP ACTOR script methods.
//
// Source: retail SP's actor method table at 0x00A52058, 115 entries, reached
// through the 1st link of SP's Scr_GetMethod chain (SP dispatcher
// FUN_0051AB90; SP Scr_GetMethod is FUN_0068A640). That table is this file's
// methods_2[] plus 40 extra names; the 75 names the two tables share appear in
// IDENTICAL relative order in both, which is what establishes the pairing.
// The 22 registered below are the subset of those 40 that the SP `frontend`
// boot actually compiles.
//
// These are placeholders, NOT implementations. Each does nothing but print one
// warning naming itself. They exist so the SP script set LINKS: an unresolved
// method name falls through EmitMethod's script_method: path
// (cscr_compiler.cpp:3187) and becomes a script-function reference, which
// LinkThread (cscr_compiler.cpp:271) then kills the boot over. Grep
// TODO(SP-STUB) to enumerate every stub in the tree.
//
// The full rationale -- in particular WHY AN EMPTY BODY IS STACK-SAFE, with
// the cscr_vm.cpp line references -- is written out once above
// BuiltinFunctionDef functions[] in game_mp/g_scr_main_mp.cpp. Not repeated
// here.
//
// type == 0 on every row. Unlike the earlier pass, that is now a VERIFIED
// claim rather than merely the permissive choice: all 710 rows across all
// seven SP method tables were read, only 9 carry type 1 (the mover table's
// devadd{pitch,yaw,roll} and 6 others), and every one of the 58 names
// registered by this pass carries 0 in retail SP.
// ===========================================================================

// Channel 24 == "parserscript" (con_channels.cpp:11, builtinChannels[24]), the
// same channel the sibling stubs in game_mp/g_scr_main_mp.cpp report on.
static void ActorSPStub_ReportOnce(const char *name, const char *spHandler, const char *retDesc, bool *pReported)
{
    if ( *pReported )
        return;
    *pReported = true;
    Com_PrintWarning(
        24,
        "WARNING: TODO(SP-STUB) script builtin '%s' (retail SP handler %s) was called "
        "but is an unimplemented stub: it does no work at all and evaluates to %s. "
        "This warning prints once per builtin name.\n",
        name,
        spHandler,
        retDesc);
}

#define ActorSPStub_DEFINE(symbol, gscName, spHandler)                        \
    static void __cdecl symbol(scr_entref_t)                                  \
    {                                                                         \
        static bool s_reported = false;                                       \
        ActorSPStub_ReportOnce(gscName, spHandler, "undefined", &s_reported); \
    }

// Returns a fixed integer instead of undefined. STILL A STUB -- it does no
// work; the value is only a return SHAPE fix. It exists because the VM makes
// `if (x)` on undefined a fatal script error (Scr_CastBool,
// cscr_variable.cpp:4522-4529), and every name using this macro is read as a
// predicate somewhere in the retail SP script corpus. The per-name reason for
// the specific value is on that name's own row below. See the full write-up
// above the SP_STUB_* macros in game_mp/g_scr_main_mp.cpp for the VM stack
// contract that makes this safe.
#define ActorSPStub_DEFINE_INT(symbol, gscName, spHandler, value)             \
    static void __cdecl symbol(scr_entref_t)                                  \
    {                                                                         \
        static bool s_reported = false;                                       \
        ActorSPStub_ReportOnce(gscName, spHandler,                            \
                               "the fixed integer " #value, &s_reported);     \
        Scr_AddInt((value), SCRIPTINSTANCE_SERVER);                           \
    }

// --- TODO(SP-STUB) actor method stubs, registered at the end of methods_2[] ---
ActorSPStub_DEFINE(ActorCmd_SPStub_setproneanimnodes, "setproneanimnodes", "0x007ca720")
    // TODO(SP-STUB) 8 GSC ref(s) in the frontend closure, e.g. animscripts/cover_prone:57 self setProneAnimNodes( -45, 45, %prone_legs_down, %exposed_modern, %prone_le...
ActorSPStub_DEFINE_INT(ActorCmd_SPStub_canshoot, "canshoot", "0x007caba0", 0)
    // RETURNS 0: JUDGEMENT: capability query; 0 = 'cannot shoot at that point'.
    // TODO(SP-STUB) 7 GSC ref(s) in the frontend closure, e.g. animscripts/combat:1044 if ( IsDefined( self.enemy ) && (!self cansee( self.enemy ) || !self canShoot( s...
ActorSPStub_DEFINE_INT(ActorCmd_SPStub_useturret, "useturret", "0x007cc3d0", 0)
    // RETURNS 0: JUDGEMENT: 0 = 'failed to mount the turret'.
    // TODO(SP-STUB) 7 GSC ref(s) in the frontend closure, e.g. animscripts/cover_wall:828 if ( self useTurret( turret ) )
ActorSPStub_DEFINE(ActorCmd_SPStub_shoot, "shoot", "0x007c9b50")
    // TODO(SP-STUB) 5 GSC ref(s) in the frontend closure, e.g. animscripts/shared:1486 self shoot( 1, pos );
ActorSPStub_DEFINE(ActorCmd_SPStub_stopuseturret, "stopuseturret", "0x007cc4c0")
    // TODO(SP-STUB) 5 GSC ref(s) in the frontend closure, e.g. maps/_mgturret:1688 self StopUSeturret();
ActorSPStub_DEFINE_INT(ActorCmd_SPStub_usecovernode, "usecovernode", "0x007c9ec0", 0)
    // RETURNS 0: JUDGEMENT: 0 = 'failed to take the cover node'.
    // TODO(SP-STUB) 4 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:2034 if ( self UseCoverNode( node ) )
ActorSPStub_DEFINE(ActorCmd_SPStub_checkgrenadethrow, "checkgrenadethrow", "0x007cba70")
    // TODO(SP-STUB) 3 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:1575 throwvel = self checkGrenadeThrow(armOffset, "min energy", self.randomGr...
ActorSPStub_DEFINE(ActorCmd_SPStub_checkgrenadethrowpos, "checkgrenadethrowpos", "0x007cbb70")
    // TODO(SP-STUB) 3 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:1562 throwvel = self checkGrenadeThrowPos(armOffset, "min energy", destination);
ActorSPStub_DEFINE_INT(ActorCmd_SPStub_issuppressed, "issuppressed", "0x007cb9d0", 0)
    // RETURNS 0: JUDGEMENT: 0 = 'not suppressed'; nothing in this tree tracks
    //            suppression.
    // TODO(SP-STUB) 3 GSC ref(s) in the frontend closure, e.g. animscripts/squadmanager:798 else if (self issuppressed())
ActorSPStub_DEFINE(ActorCmd_SPStub_updateplayersightaccuracy, "updateplayersightaccuracy", "0x007c9d70")
    // TODO(SP-STUB) 3 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:391 self updatePlayerSightAccuracy();
ActorSPStub_DEFINE(ActorCmd_SPStub_updateprone, "updateprone", "0x007ca820")
    // TODO(SP-STUB) 3 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:872 self UpdateProne(%prone_legs_up, %prone_legs_down, 1, 0.1, 1);
ActorSPStub_DEFINE(ActorCmd_SPStub_dropweapon, "dropweapon", "0x007cae50")
    // TODO(SP-STUB) 2 GSC ref(s) in the frontend closure, e.g. animscripts/shared:227 droppedWeapon = self DropWeapon( dropWeaponName, position, speed );
ActorSPStub_DEFINE(ActorCmd_SPStub_findbestcovernode, "findbestcovernode", "0x007c9e20")
    // NOTE: LEFT UNDEFINED DELIBERATELY AND CORRECTLY: the corpus guards it with IsDefined
    //       (animscripts/combat_utility.gsc:2028/2089), and undefined is what retail returns when
    //       no cover node is found. Same for getturret (maps/_vehicle_aianim.gsc:2286 wraps it in
    //       isdefined()) and dropweapon (result never read).
    // TODO(SP-STUB) 2 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:2028 node = self FindBestCoverNode();
ActorSPStub_DEFINE_INT(ActorCmd_SPStub_canuseturret, "canuseturret", "0x007cc500", 0)
    // RETURNS 0: JUDGEMENT: capability query; 0 = 'cannot use that turret'.
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. maps/_mgturret:911 if( ai[i] IsInGoal( node.origin ) && ai[i] CanUSeturret( turret ) )
ActorSPStub_DEFINE_INT(ActorCmd_SPStub_checkcoverexitposwithpath, "checkcoverexitposwithpath", "0x007c9ae0", 0)
    // RETURNS 0: JUDGEMENT: 0 = 'no valid exit position found'; no path check is performed.
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. animscripts/cover_arrival:2071 if ( !isExposedApproach && checkWithPath && !( self checkCoverExitPosWith...
ActorSPStub_DEFINE(ActorCmd_SPStub_enterprone, "enterprone", "0x007ca610")
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. animscripts/utility:2253 self EnterProne(timer);
ActorSPStub_DEFINE(ActorCmd_SPStub_exitprone, "exitprone", "0x007ca6c0")
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. animscripts/utility:2274 self ExitProne(timer);
ActorSPStub_DEFINE(ActorCmd_SPStub_getturret, "getturret", "0x007ccc80")
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. maps/_vehicle_aianim:2286 if( !isdefined( guy getturret() ) )
ActorSPStub_DEFINE(ActorCmd_SPStub_startactorreact, "startactorreact", "0x007ce8f0")
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. maps/_callbacksetup:314 self startactorreact();
ActorSPStub_DEFINE(ActorCmd_SPStub_stopshoot, "stopshoot", "0x007c9cb0")
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. animscripts/utility:1377 self StopShoot();
ActorSPStub_DEFINE(ActorCmd_SPStub_throwgrenade, "throwgrenade", "0x007cbc90")
    // TODO(SP-STUB) 1 GSC ref(s) in the frontend closure, e.g. animscripts/combat_utility:1761 self throwGrenade();

// --- Added 2026-08-28, when GScr_LoadHumanAnimScripts began compiling the 23 human
// animscripts. Both names are actor methods that no earlier pass registered, and an
// UNREGISTERED builtin is a COMPILE error, not a runtime warning: the boot died with
//     Server script compile error: unknown function (animscripts/grenade_return_throw.gsc:109)
//       self pickUpGrenade();
// A static diff of the newly-compiled script closure against every name this tree
// registers says these two are the ONLY additions the human set needs; the other 113
// entries of the SP actor method table it touches are already present above.
ActorSPStub_DEFINE(ActorCmd_SPStub_pickupgrenade, "pickupgrenade", "0x007cc380")
    // TODO(SP-STUB) 2 GSC ref(s), both animscripts/grenade_return_throw (:109, :121) --
    // the AI picking a live grenade back up to throw it away. Retail 0x007cc380 is short:
    // Actor_Get(entref), then `if (actor-><short +0x215c>) FUN_00647e00();`. Left a stub
    // rather than guessed: +0x215c is not mapped to a field of this tree's actor_s, and
    // FUN_00647e00 is unidentified. Nothing on the frontend path throws grenades.
// Retail 0x007C98F0 is the actor-only wrapper around the same worker used by
// AnimScripted. The binary actor-method table pairs the literal
// "startscriptedanim" with this address, and the body independently proves the
// ABI: Actor_Get, reject more than eight parameters, then worker flags (0, 0).
static void __cdecl ActorCmd_StartScriptedAnim_SP(scr_entref_t entref)
{
    actor_s *actor = Actor_Get(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 8 )
        Scr_Error("too many parameters", SCRIPTINSTANCE_SERVER);
    Com_Printf(15, "SP startscriptedanim method: ent %d actor %p\n", actor->ent->s.number, actor);
    GScr_StartScriptedAnim_SP(entref, false, false);
}

// Retail SP 0x007CD130.  Besides the actor-method table's name/handler pairing,
// the body independently identifies all three existing actor_s string fields:
// current becomes last, parameter 0 becomes current, and parameter 1 records
// the transition reason.  Actor_DumpInfo consumes the same triplet.
static void __cdecl ActorCmd_TrackScriptState_SP(scr_entref_t entref)
{
    actor_s *actor = Actor_Get(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2 )
        Scr_Error("trackScriptState newStateName, reasonForTransition", SCRIPTINSTANCE_SERVER);

    Scr_SetString(&actor->lastScriptState, actor->scriptState, SCRIPTINSTANCE_SERVER);
    Scr_SetString(
        &actor->scriptState,
        (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER),
        SCRIPTINSTANCE_SERVER);
    Scr_SetString(
        &actor->stateChangeReason,
        (unsigned __int16)Scr_GetConstString(1, SCRIPTINSTANCE_SERVER),
        SCRIPTINSTANCE_SERVER);

    if ( actor->scriptState == actor->lastScriptState )
    {
        Scr_Error(
            va(
                "trackScriptState should only be called on script state transitions.  "
                "Called for state %s from state %s.",
                SL_ConvertToString(actor->scriptState, SCRIPTINSTANCE_SERVER),
                SL_ConvertToString(actor->lastScriptState, SCRIPTINSTANCE_SERVER)),
            SCRIPTINSTANCE_SERVER);
    }
}

// Retail SP 0x007CE220.  The actor-method table's name/handler pair is fact 1;
// the body writes the first two members of the contiguous +0x1AD8 engagement
// block, and retail's independent scorer at 0x007BFD40 reads that same block.
static void __cdecl ActorCmd_SetEngagementMinDist_SP(scr_entref_t entref)
{
    actor_s *actor = Actor_Get(entref);
    actor->engageMinDist = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    actor->engageMinFalloffDist = Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);

    if ( actor->engageMinFalloffDist > actor->engageMinDist )
    {
        Scr_Error(
            va(
                "Min dist falloff must be <= min dist. [%f < %f]",
                actor->engageMinDist,
                actor->engageMinFalloffDist),
            SCRIPTINSTANCE_SERVER);
    }
}

// Retail SP 0x007CE2C0, the +0x1AE0/+0x1AE4 twin of the method above.
static void __cdecl ActorCmd_SetEngagementMaxDist_SP(scr_entref_t entref)
{
    actor_s *actor = Actor_Get(entref);
    actor->engageMaxDist = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    actor->engageMaxFalloffDist = Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);

    if ( actor->engageMaxFalloffDist < actor->engageMaxDist )
    {
        Scr_Error(
            va(
                "Max dist falloff must be >= max dist. [%f > %f]",
                actor->engageMaxDist,
                actor->engageMaxFalloffDist),
            SCRIPTINSTANCE_SERVER);
    }
}
#endif // KISAK_SP


const BuiltinMethodDef methods_2[] =
{
  { "startcoverarrival", &ActorCmd_StartCoverArrival, 0 },
  { "starttraversearrival", &ActorCmd_StartTraverseArrival, 0 },
  { "melee", &ActorCmd_Melee, 0 },
  { "reacquirestep", &ActorCmd_ReacquireStep, 0 },
  { "findreacquirenode", &ActorCmd_FindReacquireNode, 0 },
  { "getreacquirenode", &ActorCmd_GetReacquireNode, 0 },
  { "usereacquirenode", &ActorCmd_UseReacquireNode, 0 },
  { "findreacquiredirectpath", &ActorCmd_FindReacquireDirectPath, 0 },
  { "trimpathtoattack", &ActorCmd_TrimPathToAttack, 0 },
  { "reacquiremove", &ActorCmd_ReacquireMove, 0 },
  { "findreacquireproximatepath", &ActorCmd_FindReacquireProximatePath, 0 },
  { "flagenemyunattackable", &ActorCmd_FlagEnemyUnattackable, 0 },
  { "clearpitchorient", &ActorCmd_ClearPitchOrient, 0 },
  { "setlookatanimnodes", &ActorCmd_SetLookAtAnimNodes, 0 },
  { "setlookat", &ActorCmd_SetLookAt, 0 },
  { "setlookatyawlimits", &ActorCmd_SetLookAtYawLimits, 0 },
  { "stoplookat", &ActorCmd_StopLookAt, 0 },
  { "cansee", &ActorCmd_CanSee, 0 },
  { "maymovetopoint", &ActorCmd_MayMoveToPoint, 0 },
  { "maymovefrompointtopoint", &ActorCmd_MayMoveFromPointToPoint, 0 },
  { "teleport", &ActorCmd_Teleport, 0 },
  { "forceteleport", &ActorCmd_ForceTeleport, 0 },
  { "withinapproxpathdist", &ActorCmd_WithinApproxPathDist, 0 },
  { "ispathdirect", &ActorCmd_IsPathDirect, 0 },
  { "allowedstances", &ActorCmd_AllowedStances, 0 },
  { "isstanceallowed", &ActorCmd_IsStanceAllowed, 0 },
  { "traversemode", &ActorCmd_TraverseMode, 0 },
  { "animmode", &ActorCmd_AnimMode, 0 },
  { "orientmode", &ActorCmd_OrientMode, 0 },
  { "getmotionangle", &ActorCmd_GetMotionAngle, 0 },
  { "getanglestolikelyenemypath", &ActorCmd_GetAnglesToLikelyEnemyPath, 0 },
  { "lerpposition", &ActorCmd_LerpPosition, 0 },
  { "predictoriginandangles", &ActorCmd_PredictOriginAndAngles, 0 },
  { "predictanim", &ActorCmd_PredictAnim, 0 },
  { "gethitenttype", &ActorCmd_GetHitEntType, 0 },
  { "gethityaw", &ActorCmd_GetHitYaw, 0 },
  { "getgroundenttype", &ActorCmd_GetGroundEntType, 0 },
  { "isdeflected", &ActorCmd_IsDeflected, 0 },
  { "animcustom", &ScrCmd_animcustom, 0 },
  { "canattackenemynode", &ScrCmd_CanAttackEnemyNode, 0 },
  // LWSS ADD FROM LATEST BLOPS MP RETAIL
  { "getpathlength", &ScrCmd_GetPathLength, 0 },
  { "calcpathlength", &ScrCmd_CalcPathLength, 0 },
  // LWSS END
  { "getnegotiationstartnode", &ScrCmd_GetNegotiationStartNode, 0 },
  { "getnegotiationendnode", &ScrCmd_GetNegotiationEndNode, 0 },
  { "checkprone", &ActorCmd_CheckProne, 0 },
  { "pushplayer", &ActorCmd_PushPlayer, 0 },
  { "setgoalnode", &ActorCmd_SetGoalNode, 0 },
  { "setgoalpos", &ActorCmd_SetGoalPos, 0 },
  { "setgoalentity", &ActorCmd_SetGoalEntity, 0 },
  { "setgoalvolume", &ActorCmd_SetGoalVolume, 0 },
  { "getgoalvolume", &ActorCmd_GetGoalVolume, 0 },
  { "cleargoalvolume", &ActorCmd_ClearGoalVolume, 0 },
  { "setfixednodesafevolume", &ActorCmd_SetFixedNodeSafeVolume, 0 },
  { "getfixednodesafevolume", &ActorCmd_GetFixedNodeSafeVolume, 0 },
  { "clearfixednodesafevolume", &ActorCmd_ClearFixedNodeSafeVolume, 0 },
  { "isingoal", &ActorCmd_IsInGoal, 0 },
  { "setruntopos", &ActorCmd_SetOverrideRunToPos, 0 },
  { "clearruntopos", &ActorCmd_ClearOverrideRunToPos, 0 },
  { "nearnode", &ActorCmd_NearNode, 0 },
  { "clearenemy", &ActorCmd_ClearEnemy, 0 },
  { "setentitytarget", &ActorCmd_SetEntityTarget, 0 },
  { "clearentitytarget", &ActorCmd_ClearEntityTarget, 0 },
  { "setpotentialthreat", &ActorCmd_SetPotentialThreat, 0 },
  { "clearpotentialthreat", &ActorCmd_ClearPotentialThreat, 0 },
  { "getperfectinfo", &ActorCmd_GetPerfectInfo, 0 },
  { "setflashbangimmunity", &ActorCmd_SetFlashbangImmunity, 0 },
  { "setflashbanged", &ActorCmd_SetFlashBanged, 0 },
  { "getflashbangedstrength", &ActorCmd_GetFlashBangedStrength, 0 },
  { "isknownenemyinradius", &ActorCmd_IsKnownEnemyInRadius, 0 },
  { "isknownenemyinvolume", &ActorCmd_IsKnownEnemyInVolume, 0 },
  { "settalktospecies", &ActorCmd_SetTalkToSpecies, 0 },
  { "allowpitchangle", &METHOD_NULLSUB, 0 },
  { "knockback", &ActorCmd_Knockback, 0 },
  { "getdeltaturnyaw", &ActorCmd_GetDeltaTurnYaw, 0 },
  { "setentityowner", &ActorCmd_SetEntityOwner, 0 },
  { "clearentityowner", &ActorCmd_ClearEntityOwner, 0 },
  { "setanimstate", &ActorCmd_SetAnimState, 0 },
  { "setaimanimweights", &ActorCmd_SetAimAnimWeights, 0 },
  { "finishactordamage", &ActorCmd_finishActorDamage, 0 }
#ifdef KISAK_SP
  ,
  { "setproneanimnodes", &ActorCmd_SPStub_setproneanimnodes, 0 },            // TODO(SP-STUB) SP 0x00A52058 idx 26, 0x007ca720
  { "trackscriptstate", &ActorCmd_TrackScriptState_SP, 0 },                  // IMPLEMENTED from SP 0x00A52058 idx 72, 0x007CD130
  { "canshoot", &ActorCmd_SPStub_canshoot, 0 },                              // TODO(SP-STUB) SP 0x00A52058 idx 33, 0x007caba0
  { "useturret", &ActorCmd_SPStub_useturret, 0 },                            // TODO(SP-STUB) SP 0x00A52058 idx 53, 0x007cc3d0
  { "shoot", &ActorCmd_SPStub_shoot, 0 },                                    // TODO(SP-STUB) SP 0x00A52058 idx 4, 0x007c9b50
  { "stopuseturret", &ActorCmd_SPStub_stopuseturret, 0 },                    // TODO(SP-STUB) SP 0x00A52058 idx 54, 0x007cc4c0
  { "usecovernode", &ActorCmd_SPStub_usecovernode, 0 },                      // TODO(SP-STUB) SP 0x00A52058 idx 12, 0x007c9ec0
  { "checkgrenadethrow", &ActorCmd_SPStub_checkgrenadethrow, 0 },            // TODO(SP-STUB) SP 0x00A52058 idx 47, 0x007cba70
  { "checkgrenadethrowpos", &ActorCmd_SPStub_checkgrenadethrowpos, 0 },      // TODO(SP-STUB) SP 0x00A52058 idx 82, 0x007cbb70
  { "issuppressed", &ActorCmd_SPStub_issuppressed, 0 },                      // TODO(SP-STUB) SP 0x00A52058 idx 45, 0x007cb9d0
  { "updateplayersightaccuracy", &ActorCmd_SPStub_updateplayersightaccuracy, 0 }, // TODO(SP-STUB) SP 0x00A52058 idx 8, 0x007c9d70
  { "updateprone", &ActorCmd_SPStub_updateprone, 0 },                        // TODO(SP-STUB) SP 0x00A52058 idx 27, 0x007ca820
  { "dropweapon", &ActorCmd_SPStub_dropweapon, 0 },                          // TODO(SP-STUB) SP 0x00A52058 idx 35, 0x007cae50
  { "findbestcovernode", &ActorCmd_SPStub_findbestcovernode, 0 },            // TODO(SP-STUB) SP 0x00A52058 idx 10, 0x007c9e20
  { "canuseturret", &ActorCmd_SPStub_canuseturret, 0 },                      // TODO(SP-STUB) SP 0x00A52058 idx 55, 0x007cc500
  { "checkcoverexitposwithpath", &ActorCmd_SPStub_checkcoverexitposwithpath, 0 }, // TODO(SP-STUB) SP 0x00A52058 idx 3, 0x007c9ae0
  { "enterprone", &ActorCmd_SPStub_enterprone, 0 },                          // TODO(SP-STUB) SP 0x00A52058 idx 24, 0x007ca610
  { "exitprone", &ActorCmd_SPStub_exitprone, 0 },                            // TODO(SP-STUB) SP 0x00A52058 idx 25, 0x007ca6c0
  { "getturret", &ActorCmd_SPStub_getturret, 0 },                            // TODO(SP-STUB) SP 0x00A52058 idx 62, 0x007ccc80
  { "startactorreact", &ActorCmd_SPStub_startactorreact, 0 },                // TODO(SP-STUB) SP 0x00A52058 idx 114, 0x007ce8f0
  { "stopshoot", &ActorCmd_SPStub_stopshoot, 0 },                            // TODO(SP-STUB) SP 0x00A52058 idx 6, 0x007c9cb0
  { "throwgrenade", &ActorCmd_SPStub_throwgrenade, 0 },                      // TODO(SP-STUB) SP 0x00A52058 idx 51, 0x007cbc90
  { "pickupgrenade", &ActorCmd_SPStub_pickupgrenade, 0 },                    // TODO(SP-STUB) SP 0x00A52058 idx 52, 0x007cc380
  { "startscriptedanim", &ActorCmd_StartScriptedAnim_SP, 0 },                // IMPLEMENTED from SP 0x007C98F0 -> shared worker 0x00624F30
  { "setengagementmindist", &ActorCmd_SetEngagementMinDist_SP, 0 },         // SP 0x00A52058 idx 105, 0x007ce220
  { "setengagementmaxdist", &ActorCmd_SetEngagementMaxDist_SP, 0 },         // SP 0x00A52058 idx 106, 0x007ce2c0
#endif // KISAK_SP
};



static void USE_NULLSUB(gentity_s *, gentity_s *, gentity_s *)
{

}


entityHandler_t entityHandlers[31] =
{
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  {
    NULL,
    NULL,
    NULL,
    Actor_Touch,
    NULL,
    Actor_Pain,
    NULL,
    Actor_Die,
    Actor_EntInfo,
    actor_controller,
    0,
    0
  },
  {
    Actor_Think,
    NULL,
    NULL,
    Actor_Touch,
    NULL,
    Actor_Pain,
    NULL,
    Actor_Die,
    Actor_EntInfo,
    actor_controller,
    0,
    0
  },
  {
    BodyEnd,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    NULL,
    0,
    0
  },
  {
    NULL,
    NULL,
    NULL,
    Touch_Multi,
    NULL,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    NULL,
    0,
    0
  },
  { NULL, NULL, NULL, NULL, hurt_use, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  {
    NULL,
    NULL,
    NULL,
    hurt_touch,
    hurt_use,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0
  },
  {
    NULL,
    NULL,
    NULL,
    NULL,
    Use_trigger_damage,
    Pain_trigger_damage,
    NULL,
    Die_trigger_damage,
    NULL,
    NULL,
    0,
    0
  },
  {
    NULL,
    Reached_ScriptMover,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0
  },
  {
    NULL,
    Reached_ScriptMover,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    NULL,
    0,
    0
  },
  {
    G_ExplodeMissile,
    NULL,
    NULL,
    Touch_Item_Auto,
    NULL,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    NULL,
    3,
    4
  },
  {
    G_TimedObjectThink,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    NULL,
    0,
    0
  },
  {
    G_ExplodeMissile,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    NULL,
    5,
    6
  },
  {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    player_die,
    misc_EntInfo,
    G_PlayerController,
    0,
    0
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, player_die, NULL, NULL, 0, 0 },
  {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    G_PlayerController,
    0,
    0
  },
  { BodyEnd, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  {
    turret_think_init,
    NULL,
    NULL,
    NULL,
    turret_use,
    NULL,
    NULL,
    NULL,
    NULL,
    turret_controller,
    0,
    0
  },
  {
    turret_think,
    NULL,
    NULL,
    NULL,
    turret_use,
    NULL,
    NULL,
    NULL,
    misc_EntInfo,
    turret_controller,
    0,
    0
  },
  {
    DroppedItemClearOwner,
    NULL,
    NULL,
    Touch_Item_Auto,
    NULL,
    NULL,
    NULL,
    NULL,
    EntInfo_Item,
    NULL,
    0,
    0
  },
  {
    FinishSpawningItem,
    NULL,
    NULL,
    Touch_Item_Auto,
    NULL,
    NULL,
    NULL,
    NULL,
    EntInfo_Item,
    NULL,
    0,
    0
  },
  {
    NULL,
    NULL,
    NULL,
    Touch_Item_Auto,
    NULL,
    NULL,
    NULL,
    NULL,
    EntInfo_Item,
    NULL,
    0,
    0
  },
  {
    NULL,
    NULL,
    NULL,
    NULL,
    USE_NULLSUB,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0
  },
  {
    NULL,
    Reached_ScriptMover,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0
  },
  { G_FreeEntity, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  {
    Scr_Vehicle_Init,
    NULL,
    NULL,
    Scr_Vehicle_Touch,
    Scr_Vehicle_Use,
    Scr_Vehicle_Pain,
    NULL,
    Scr_Vehicle_Die,
    misc_EntInfo,
    Scr_Vehicle_Controller,
    0,
    0
  },
  {
    Scr_Vehicle_Think,
    NULL,
    NULL,
    Scr_Vehicle_Touch,
    Scr_Vehicle_Use,
    Scr_Vehicle_Pain,
    NULL,
    Scr_Vehicle_Die,
    EntInfo_Vehicle,
    Scr_Vehicle_Controller,
    0,
    0
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0 },
  {
    Helicopter_Think,
    NULL,
    NULL,
    NULL,
    NULL,
    Helicopter_Pain,
    NULL,
    Helicopter_Die,
    EntInfo_Vehicle,
    Helicopter_Controller,
    0,
    0
  },
  {
    NULL,
    NULL,
    NULL,
    trigger_ik_playerclip_terrain_touch,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0
  },
  {
    NULL,
    NULL,
    NULL,
    trigger_ik_disable_terrain_mapping_touch,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0
  }
};

void(__cdecl *__cdecl Actor_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < ARRAY_COUNT(methods_2); ++i)
    {
        if (!strcmp(*pName, methods_2[i].actionString))
        {
            *pName = methods_2[i].actionString;
            return methods_2[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl ActorCmd_StartCoverArrival(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_StartArrivalState(self, AIS_COVERARRIVAL);
}

actor_s *__fastcall Actor_Get(scr_entref_t entref)
{
    if ( !entref.classnum )
    {
        if ( entref.entnum >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                        89,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        if ( g_entities[entref.entnum].actor )
            return g_entities[entref.entnum].actor;
    }
    Scr_ObjectError("not an actor", SCRIPTINSTANCE_SERVER);
    return 0;
}

void __fastcall Actor_StartArrivalState(actor_s *self, ai_state_t newState)
{
    float *currentOrigin; // [esp+1Ch] [ebp-20h]
    float startPos[3]; // [esp+20h] [ebp-1Ch] BYREF
    float angles[3]; // [esp+2Ch] [ebp-10h]
    gentity_s *ent; // [esp+38h] [ebp-4h]

    ent = self->ent;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 179, 0, "%s", "ent") )
        __debugbreak();
    if ( newState > AIS_DEATH )
    {
        Actor_PushState(self, newState);
        goto LABEL_9;
    }
    if ( !self->simulatedStateLevel )
    {
        Actor_SetState(self, newState);
LABEL_9:
        Scr_GetVector(0, startPos, SCRIPTINSTANCE_SERVER);
        currentOrigin = ent->r.currentOrigin;
        self->arrivalInfo.animscriptOverrideOriginError[0] = startPos[0] - ent->r.currentOrigin[0];
        self->arrivalInfo.animscriptOverrideOriginError[1] = startPos[1] - currentOrigin[1];
        self->arrivalInfo.animscriptOverrideOriginError[2] = startPos[2] - currentOrigin[2];
        angles[0] = 0.0f;
        angles[1] = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        angles[2] = 0.0f;
        self->Physics.vVelocity[0] = 0.0f;
        self->Physics.vVelocity[1] = 0.0f;
        self->Physics.vVelocity[2] = 0.0f;
        self->Physics.vWishDelta[0] = 0.0f;
        self->Physics.vWishDelta[1] = 0.0f;
        self->Physics.vWishDelta[2] = 0.0f;
        self->ScriptOrient.eMode = AI_ORIENT_INVALID;
        Actor_SetDesiredAngles(&self->CodeOrient, angles[0], angles[1]);
        Actor_SetDesiredAngles(&self->ScriptOrient, angles[0], angles[1]);
        Actor_ClearPath(self);
    }
}

void __cdecl ActorCmd_StartTraverseArrival(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_StartArrivalState(self, AIS_NEGOTIATION);
}

void __cdecl ActorCmd_Melee(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-14h]
    gentity_s *hitEnt; // [esp+4h] [ebp-10h]
    float direction[3]; // [esp+8h] [ebp-Ch] BYREF

    self = Actor_Get(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
    {
        Scr_GetVector(0, direction, SCRIPTINSTANCE_SERVER);
        hitEnt = Actor_Melee(self, direction);
    }
    else
    {
        hitEnt = Actor_Melee(self, 0);
    }
    if ( hitEnt )
        Scr_AddEntity(hitEnt, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_ReacquireStep(scr_entref_t entref)
{
    unsigned __int8 v1; // al
    float fDist; // [esp+0h] [ebp-Ch]
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    fDist = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    v1 = Actor_Exposed_ReacquireStepMove(self, fDist);
    Scr_AddBool(v1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_FindReacquireNode(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_Exposed_FindReacquireNode(self);
}

void __cdecl ActorCmd_GetReacquireNode(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]
    pathnode_t *pNode; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    pNode = Actor_Exposed_GetReacquireNode(self);
    if ( pNode )
        Scr_AddPathnode(pNode);
}

void __cdecl ActorCmd_UseReacquireNode(scr_entref_t entref)
{
    pathnode_t *Pathnode; // eax
    unsigned __int8 v2; // al
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Actor_KeepClaimedNode(self) )
        Scr_Error("cannot change node when keepclaimednode is set", 0);
    Pathnode = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
    v2 = Actor_Exposed_UseReacquireNode(self, Pathnode);
    Scr_AddBool(v2, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_FindReacquireDirectPath(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]
    bool ignoreSuppression; // [esp+7h] [ebp-1h]

    self = Actor_Get(entref);
    ignoreSuppression = 0;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        ignoreSuppression = Scr_GetInt(0, SCRIPTINSTANCE_SERVER) != 0;
    Actor_Exposed_FindReacquireDirectPath(self, ignoreSuppression);
}

void __cdecl ActorCmd_FindReacquireProximatePath(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]
    bool ignoreSuppression; // [esp+7h] [ebp-1h]

    self = Actor_Get(entref);
    ignoreSuppression = 0;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        ignoreSuppression = Scr_GetInt(0, SCRIPTINSTANCE_SERVER) != 0;
    Actor_Exposed_FindReacquireProximatePath(self, ignoreSuppression);
}

void __cdecl ActorCmd_TrimPathToAttack(scr_entref_t entref)
{
    unsigned int v1; // eax
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( !Actor_MayReacquireMove(self) )
        Scr_Error("TrimPathToAttack may only called after calling FindReacquireDirectPath or FindReacquireProximatePath", 0);
    v1 = Actor_TrimPathToAttack(self);
    Scr_AddBool(v1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_ReacquireMove(scr_entref_t entref)
{
    unsigned __int8 started; // al
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( !Actor_MayReacquireMove(self) )
        Scr_Error("ReacquireMove may only called after calling FindReacquireDirectPath or FindReacquireProximatePath", 0);
    started = Actor_Exposed_StartReacquireMove(self);
    Scr_AddBool(started, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_FlagEnemyUnattackable(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_FlagEnemyUnattackable(self);
}

void __cdecl ActorCmd_ClearPitchOrient(scr_entref_t entref)
{
    actor_s *v1; // eax

    v1 = Actor_Get(entref);
    v1->ProneInfo.prone = 0;
    v1->ProneInfo.orientPitch = 0;
}

void __cdecl ActorCmd_SetLookAtAnimNodes(scr_entref_t entref)
{
    XAnimTree_s *tree; // [esp+Ch] [ebp-14h]
    actor_s *self; // [esp+10h] [ebp-10h]
    unsigned __int16 animRight; // [esp+14h] [ebp-Ch]
    unsigned __int16 animStraight; // [esp+18h] [ebp-8h]
    unsigned __int16 animLeft; // [esp+1Ch] [ebp-4h]

    self = Actor_Get(entref);
    tree = G_GetActorAnimTree(self);
    animStraight = Scr_GetAnim(0, tree, SCRIPTINSTANCE_SERVER).index;
    animLeft = Scr_GetAnim(1u, tree, SCRIPTINSTANCE_SERVER).index;
    animRight = Scr_GetAnim(2u, tree, SCRIPTINSTANCE_SERVER).index;
    Actor_SetLookAtAnimNodes(self, animStraight, animLeft, animRight);
}

void __cdecl ActorCmd_SetLookAt(scr_entref_t entref)
{
    float pfPosition[3]; // [esp+4h] [ebp-14h] BYREF
    float fTurnAccel; // [esp+10h] [ebp-8h]
    actor_s *self; // [esp+14h] [ebp-4h]

    fTurnAccel = 0.0f;
    self = Actor_Get(entref);
    if ( !self->lookAtInfo.bLookAtSetup )
        Scr_Error("LookAt Called without setLookAtAnimNodes being called first.", 0);
    Scr_GetVector(0, pfPosition, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        fTurnAccel = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    Actor_SetLookAt(self, pfPosition, fTurnAccel);
}

void __cdecl ActorCmd_SetLookAtYawLimits(scr_entref_t entref)
{
    float fAnimYawLimit; // [esp+0h] [ebp-10h]
    float fYawLimit; // [esp+4h] [ebp-Ch]
    float fBlendTime; // [esp+8h] [ebp-8h]
    actor_s *self; // [esp+Ch] [ebp-4h]

    self = Actor_Get(entref);
    fBlendTime = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    fYawLimit = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    fAnimYawLimit = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    Actor_SetLookAtYawLimits(self, fAnimYawLimit, fYawLimit, fBlendTime);
}

void __cdecl ActorCmd_StopLookAt(scr_entref_t entref)
{
    float fTurnAccel; // [esp+4h] [ebp-8h]
    actor_s *self; // [esp+8h] [ebp-4h]

    fTurnAccel = 0.0f;
    self = Actor_Get(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        fTurnAccel = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    Actor_StopLookAt(self, fTurnAccel);
}

void __cdecl ActorCmd_CanSee(scr_entref_t entref)
{
    char CanSeeSentient; // al
    gentity_s *pOther; // [esp+Ch] [ebp-10h]
    unsigned __int8 bVisible; // [esp+13h] [ebp-9h]
    actor_s *self; // [esp+14h] [ebp-8h]
    int iLatency; // [esp+18h] [ebp-4h]

    PROF_SCOPED("ActorCmd_CanSee");

    self = Actor_Get(entref);
    pOther = Scr_GetEntity(0);
    if ( !pOther
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 1152, 0, "%s", "pOther") )
    {
        __debugbreak();
    }
    if ( pOther->sentient )
    {
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
        {
            CanSeeSentient = Actor_CanSeeSentient(self, pOther->sentient, 250);
        }
        else
        {
            iLatency = (int)(Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER) * 1000.0);
            CanSeeSentient = Actor_CanSeeSentient(self, pOther->sentient, iLatency);
        }
        bVisible = CanSeeSentient;
    }
    else
    {
        bVisible = Actor_CanSeeEntity(self, pOther);
    }
    Scr_AddBool(bVisible, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_MayMoveToPoint(scr_entref_t entref)
{
    bool checkDrop; // [esp+48h] [ebp-24h]
    float vPoint[3]; // [esp+50h] [ebp-1Ch] BYREF
    actor_s *self; // [esp+5Ch] [ebp-10h]
    float myPos[3]; // [esp+60h] [ebp-Ch] BYREF

    PROF_SCOPED("MayMoveToPoint");

    self = Actor_Get(entref);
    Scr_GetVector(0, vPoint, SCRIPTINSTANCE_SERVER);
    checkDrop = (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1
                     || Scr_GetInt(1u, SCRIPTINSTANCE_SERVER) != 0;
    if ( self->ent->tagInfo )
    {
        DEBUGMAYMOVE(self->ent->r.currentOrigin, vPoint, colorMagenta, DEBUGMAYMOVE_LIFTED);
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Sentient_GetOrigin(self->sentient, myPos);
        if ( MayMove_TraceCheck(self, myPos, vPoint, 1, checkDrop) )
        {
            self->mayMoveTime = level.time;
            DEBUGMAYMOVE(self->ent->r.currentOrigin, vPoint, colorGreen, DEBUGMAYMOVE_LIFTED);
            Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
        }
        else
        {
            Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
        }
    }
}

void __cdecl DEBUGMAYMOVE(
                const float *vec1,
                const float *vec2,
                const float *color,
                DEBUGMAYMOVE_LIFT_ENUM liftBehavior)
{
    float from[3]; // [esp+0h] [ebp-18h] BYREF
    float to[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( ai_debugMayMove->current.enabled )
    {
        from[0] = *vec1;
        from[1] = vec1[1];
        from[2] = vec1[2];
        to[0] = *vec2;
        to[1] = vec2[1];
        to[2] = vec2[2];
        if ( liftBehavior == DEBUGMAYMOVE_LIFTED )
        {
            from[2] = from[2] + 32.0;
            to[2] = to[2] + 32.0;
        }
        CG_DebugLine(from, to, color, 0, 100);
    }
}

int __cdecl MayMove_TraceCheck(actor_s *self, float *vStart, float *vEnd, int allowStartSolid, int checkDrop)
{
    col_context_t context; // [esp+50h] [ebp-90h] BYREF
    float vTraceEndPos[3]; // [esp+78h] [ebp-68h] BYREF
    float stepheight; // [esp+84h] [ebp-5Ch]
    float vPointHigh[3]; // [esp+88h] [ebp-58h] BYREF
    float MAX_FALL_HEIGHT; // [esp+94h] [ebp-4Ch]
    trace_t results; // [esp+98h] [ebp-48h] BYREF
    float vPointLow[3]; // [esp+D4h] [ebp-Ch] BYREF

    MAX_FALL_HEIGHT = 32.0f;
    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 1360, 0, "%s", "self") )
    {
        __debugbreak();
    }
    if ( !vStart
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 1361, 0, "%s", "vStart") )
    {
        __debugbreak();
    }
    if ( !vEnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 1362, 0, "%s", "vEnd") )
    {
        __debugbreak();
    }
    vPointHigh[0] = *vEnd;
    vPointHigh[1] = vEnd[1];
    vPointHigh[2] = vEnd[2];
    vPointLow[0] = *vEnd;
    vPointLow[1] = vEnd[1];
    vPointLow[2] = vEnd[2];
    vPointHigh[2] = vPointHigh[2] + 48.0;
    vPointLow[2] = vPointLow[2] - 48.0;
    //col_context_t::col_context_t(&context);
    G_TraceCapsule(
        &results,
        vPointHigh,
        vec3_origin,
        vec3_origin,
        vPointLow,
        self->ent->s.number,
        self->Physics.iTraceMask | 0xA000,
        &context);
    if ( results.allsolid )
    {
        DEBUGMAYMOVE(vPointHigh, vPointLow, colorOrange, DEBUGMAYMOVE_LIFTED);
        return 0;
    }
    else
    {
        if ( self->Physics.prone )
            stepheight = 10.0f;
        else
            stepheight = 18.0f;
        if ( checkDrop
            && (Vec3Lerp(vPointHigh, vPointLow, results.fraction, vPointLow),
                    (float)(vStart[2] - vPointLow[2]) > MAX_FALL_HEIGHT) )
        {
            DEBUGMAYMOVE(vEnd, vPointLow, colorOrange, DEBUGMAYMOVE_NOT_LIFTED);
            return 0;
        }
        else if ( Path_PredictionTrace(
                                vStart,
                                vEnd,
                                self->ent->s.number,
                                self->Physics.iTraceMask | 0x8004,
                                vTraceEndPos,
                                stepheight,
                                allowStartSolid) )
        {
            return 1;
        }
        else
        {
            DEBUGMAYMOVE(vStart, vEnd, colorLtOrange, DEBUGMAYMOVE_LIFTED);
            return 0;
        }
    }
}

void __cdecl ActorCmd_MayMoveFromPointToPoint(scr_entref_t entref)
{
    bool checkDrop; // [esp+40h] [ebp-24h]
    float vEnd[3]; // [esp+48h] [ebp-1Ch] BYREF
    actor_s *self; // [esp+54h] [ebp-10h]
    float vStart[3]; // [esp+58h] [ebp-Ch] BYREF

    PROF_SCOPED("MayMoveFromPointToPoint");

    self = Actor_Get(entref);
    Scr_GetVector(0, vStart, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1u, vEnd, SCRIPTINSTANCE_SERVER);
    checkDrop = (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 2
                     || Scr_GetInt(2u, SCRIPTINSTANCE_SERVER) != 0;
    if ( self->ent->tagInfo )
    {
        DEBUGMAYMOVE(vStart, vEnd, colorMagenta, DEBUGMAYMOVE_LIFTED);
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
        return;
    }
    if ( MayMove_TraceCheck(self, vStart, vEnd, 0, checkDrop) )
    {
        self->mayMoveTime = level.time;
        DEBUGMAYMOVE(vStart, vEnd, colorGreen, DEBUGMAYMOVE_LIFTED);
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl ActorCmd_Teleport(scr_entref_t entref)
{
    float *vWishDelta; // [esp+8h] [ebp-4Ch]
    float *vVelocity; // [esp+Ch] [ebp-48h]
    float vAngles[3]; // [esp+1Ch] [ebp-38h] BYREF
    actor_s *self; // [esp+28h] [ebp-2Ch]
    float vEyePos[3]; // [esp+2Ch] [ebp-28h] BYREF
    const float *angles; // [esp+38h] [ebp-1Ch]
    float distSquared; // [esp+3Ch] [ebp-18h]
    float vSpawnPos[3]; // [esp+40h] [ebp-14h] BYREF
    gentity_s *ent; // [esp+4Ch] [ebp-8h]
    gentity_s *player; // [esp+50h] [ebp-4h]

    self = Actor_Get(entref);
    angles = 0;
    ent = self->ent;
    Scr_GetVector(0, vSpawnPos, SCRIPTINSTANCE_SERVER);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
    {
        Scr_GetVector(1u, vAngles, SCRIPTINSTANCE_SERVER);
        angles = vAngles;
    }
    distSquared = Vec3DistanceSq(vSpawnPos, ent->r.currentOrigin);
    if ( self->ent->tagInfo )
    {
        ent->s.lerp.eFlags ^= 2u;
    }
    else if ( distSquared <= 100.0 )
    {
        if ( Actor_HasPath(self) && self->Path.iPathEndTime )
        {
            Com_DPrintf(18, "Teleport failed because actor (%i) in mid-stopping.\n", ent->s.number);
            Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
            return;
        }
    }
    else
    {
        player = G_Find(0, 356, scr_const.player);
        if ( player && player->sentient )
        {
            Sentient_GetEyePosition(player->sentient, vEyePos);
            if ( PointCouldSeeSpawn(vEyePos, vSpawnPos, player->s.number, entref.entnum) )
            {
                Com_DPrintf(18, "Teleport (of actor %i) failed because player could see goal pos.\n", ent->s.number);
                Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
                return;
            }
            if ( PointCouldSeeSpawn(vEyePos, ent->r.currentOrigin, player->s.number, entref.entnum) )
            {
                Com_DPrintf(18, "Teleport failed because player could see actor (%i).\n", ent->s.number);
                Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
                return;
            }
        }
        if ( distSquared > 16384.0 )
            ent->s.lerp.eFlags ^= 2u;
    }
    G_SetOrigin(ent, vSpawnPos);
    vVelocity = self->Physics.vVelocity;
    self->Physics.vVelocity[0] = 0.0f;
    vVelocity[1] = 0.0f;
    vVelocity[2] = 0.0f;
    vWishDelta = self->Physics.vWishDelta;
    self->Physics.vWishDelta[0] = 0.0f;
    vWishDelta[1] = 0.0f;
    vWishDelta[2] = 0.0f;
    if ( self->useEnemyGoal && (!Actor_PointAtGoal(vSpawnPos, &self->codeGoal) || distSquared > 100.0) )
    {
        self->useEnemyGoal = 0;
        Actor_UpdateGoalPos(self);
    }
    if ( angles )
    {
        G_SetAngle(ent, angles);
        Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        if ( self->ScriptOrient.eMode == AI_ORIENT_DONT_CHANGE )
            Actor_SetDesiredAngles(&self->ScriptOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        Actor_SetLookAngles(self, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    }
    if ( !self->ent->tagInfo && distSquared > 100.0 )
    {
        Sentient_InvalidateNearestNode(self->sentient);
        Actor_ClearPath(self);
        Sentient_NearestNode(self->sentient);
    }
    Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_ForceTeleport(scr_entref_t entref)
{
    float *vWishDelta; // [esp+8h] [ebp-3Ch]
    float *vVelocity; // [esp+Ch] [ebp-38h]
    float vAngles[3]; // [esp+1Ch] [ebp-28h] BYREF
    actor_s *self; // [esp+28h] [ebp-1Ch]
    const float *angles; // [esp+2Ch] [ebp-18h]
    float distSquared; // [esp+30h] [ebp-14h]
    float vSpawnPos[3]; // [esp+34h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+40h] [ebp-4h]

    self = Actor_Get(entref);
    angles = 0;
    ent = self->ent;
    Scr_GetVector(0, vSpawnPos, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
    {
        Scr_GetVector(1u, vAngles, SCRIPTINSTANCE_SERVER);
        angles = vAngles;
    }
    distSquared = Vec3DistanceSq(vSpawnPos, ent->r.currentOrigin);
    if ( self->ent->tagInfo )
        ent->s.lerp.eFlags ^= 2u;
    G_SetOrigin(ent, vSpawnPos);
    vVelocity = self->Physics.vVelocity;
    self->Physics.vVelocity[0] = 0.0f;
    vVelocity[1] = 0.0f;
    vVelocity[2] = 0.0f;
    vWishDelta = self->Physics.vWishDelta;
    self->Physics.vWishDelta[0] = 0.0f;
    vWishDelta[1] = 0.0f;
    vWishDelta[2] = 0.0f;
    if ( self->useEnemyGoal && (!Actor_PointAtGoal(vSpawnPos, &self->codeGoal) || distSquared > 100.0) )
    {
        self->useEnemyGoal = 0;
        Actor_UpdateGoalPos(self);
    }
    if ( angles )
    {
        G_SetAngle(ent, angles);
        Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        Actor_SetLookAngles(self, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    }
    if ( !self->ent->tagInfo && distSquared > 100.0 )
    {
        Sentient_InvalidateNearestNode(self->sentient);
        Actor_ClearPath(self);
        Sentient_NearestNode(self->sentient);
    }
    Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_WithinApproxPathDist(scr_entref_t entref)
{
    float v1; // xmm0_4
    float checkDist; // [esp+0h] [ebp-Ch]
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    checkDist = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    v1 = (float)Path_WithinApproxDist(&self->Path, checkDist);
    Scr_AddFloat(v1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_IsPathDirect(scr_entref_t entref)
{
    bool v1; // eax
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Actor_HasPath(self) )
    {
        v1 = Path_CompleteLookahead(&self->Path);
        Scr_AddInt(v1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl ActorCmd_AllowedStances(scr_entref_t entref)
{
    char *v1; // eax
    const char *v2; // eax
    unsigned int str; // [esp+0h] [ebp-10h]
    actor_s *self; // [esp+4h] [ebp-Ch]
    int iNumParam; // [esp+8h] [ebp-8h]
    signed int i; // [esp+Ch] [ebp-4h]

    self = Actor_Get(entref);
    iNumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( iNumParam < 1 )
        Scr_Error("no stances given in allowedStances()\n", 0);
    self->eAllowedStances = STANCE_BAD;
    for ( i = 0; i < iNumParam; ++i )
    {
        str = Scr_GetConstString(i, SCRIPTINSTANCE_SERVER);
        if ( str == scr_const.stand )
        {
            self->eAllowedStances |= 1;
        }
        else if ( str == scr_const.crouch )
        {
            self->eAllowedStances |= 2;
        }
        else if ( str == scr_const.prone )
        {
            self->eAllowedStances |= 4;
        }
        else
        {
            v1 = SL_ConvertToString(str, SCRIPTINSTANCE_SERVER);
            v2 = va("invalid stance '%s' in allowedStances()\n", v1);
            Scr_Error(v2, 0);
        }
    }
    if ( self->eAllowedStances == STANCE_BAD )
    {
        self->eAllowedStances = STANCE_ANY;
        Scr_Error("no allowed stances given", 0);
    }
}

void __cdecl ActorCmd_IsStanceAllowed(scr_entref_t entref)
{
    char *v1; // eax
    const char *v2; // eax
    unsigned int str; // [esp+0h] [ebp-10h]
    ai_stance_e eAllowedStances; // [esp+4h] [ebp-Ch]
    ai_stance_e eAllowedStancesa; // [esp+4h] [ebp-Ch]
    actor_s *self; // [esp+8h] [ebp-8h]
    ai_stance_e eStance; // [esp+Ch] [ebp-4h]

    self = Actor_Get(entref);
    str = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    eStance = STANCE_BAD;
    if ( str == scr_const.stand )
    {
        eStance = STANCE_STAND;
    }
    else if ( str == scr_const.crouch )
    {
        eStance = STANCE_CROUCH;
    }
    else if ( str == scr_const.prone )
    {
        eStance = STANCE_PRONE;
    }
    else
    {
        v1 = SL_ConvertToString(str, SCRIPTINSTANCE_SERVER);
        v2 = va("invalid stance '%s' in isStanceAllowed()\n", v1);
        Scr_Error(v2, 0);
    }
    eAllowedStances = STANCE_ANY;
    if ( Actor_HasPath(self) )
    {
        eAllowedStances = Path_AllowedStancesForPath(&self->Path);
    }
    else if ( self->sentient->pClaimedNode
                 && Actor_PointNearNode(self->ent->r.currentOrigin, self->sentient->pClaimedNode) )
    {
        eAllowedStances = Path_AllowedStancesForNode(self->sentient->pClaimedNode);
    }
    if ( (self->eAllowedStances & eAllowedStances) != 0 )
        eAllowedStancesa = (ai_stance_e)(self->eAllowedStances & eAllowedStances);
    else
        eAllowedStancesa = self->eAllowedStances;
    if ( (eStance & eAllowedStancesa) != 0 )
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_Knockback(scr_entref_t entref)
{
    unsigned int v1; // eax
    gentity_s *pEnt; // [esp+8h] [ebp-4h]

    Actor_Get(entref);
    pEnt = &g_entities[entref.entnum];
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) == 1 )
        v1 = pEnt->flags & 0xFFFFFFF7;
    else
        v1 = pEnt->flags | 8;
    pEnt->flags = v1;
}

void __cdecl ActorCmd_TraverseMode(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]
    unsigned int mode; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    mode = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    if ( mode == scr_const.gravity )
    {
        self->eTraverseMode = AI_TRAVERSE_GRAVITY;
    }
    else if ( mode == scr_const.nogravity )
    {
        self->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    }
    else if ( mode == scr_const.noclip )
    {
        self->eTraverseMode = AI_TRAVERSE_NOCLIP;
    }
    else
    {
        Scr_Error("traverseMode must be 'gravity', 'nogravity', or 'noclip'\n", 0);
    }
}

void __cdecl ActorCmd_AnimMode(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-10h]
    unsigned int mode; // [esp+8h] [ebp-8h]
    bool clearPath; // [esp+Fh] [ebp-1h]

    self = Actor_Get(entref);
    clearPath = 1;
    mode = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        clearPath = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER) != 0;
    if ( mode != scr_const.none && clearPath )
        Actor_ClearPath(self);
    if ( mode == scr_const.nophysics )
    {
        self->eScriptSetAnimMode = AI_ANIM_NOPHYSICS;
        self->ent->flags |= 0x2000u;
    }
    else
    {
        self->ent->flags &= ~0x2000u;
        if ( mode == scr_const.gravity )
        {
            self->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS;
        }
        else if ( mode == scr_const.nogravity )
        {
            self->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY;
        }
        else if ( mode == scr_const.angle_deltas )
        {
            self->eScriptSetAnimMode = AI_ANIM_USE_ANGLE_DELTAS;
        }
        else if ( mode == scr_const.pos_deltas )
        {
            self->eScriptSetAnimMode = AI_ANIM_USE_POS_DELTAS;
        }
        else if ( mode == scr_const.zonly_physics )
        {
            self->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS;
        }
        else if ( mode == scr_const.none )
        {
            self->eScriptSetAnimMode = AI_ANIM_UNKNOWN;
        }
        else if ( mode == scr_const.normal )
        {
            self->eScriptSetAnimMode = AI_ANIM_MOVE_CODE;
        }
        else if ( mode == scr_const.point_relative )
        {
            self->eScriptSetAnimMode = AI_ANIM_POINT_RELATIVE;
        }
        else if ( mode == scr_const.noclip )
        {
            self->eScriptSetAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOCLIP;
        }
        else
        {
            Scr_Error("illegal call to animmode()\n", 0);
        }
    }
}

void __cdecl ActorCmd_OrientMode(scr_entref_t entref)
{
    float fYaw; // [esp+4h] [ebp-34h]
    float *currentOrigin; // [esp+8h] [ebp-30h]
    float delta[3]; // [esp+Ch] [ebp-2Ch] BYREF
    float point[3]; // [esp+18h] [ebp-20h] BYREF
    float v[3]; // [esp+24h] [ebp-14h] BYREF
    actor_s *self; // [esp+30h] [ebp-8h]
    unsigned int mode; // [esp+34h] [ebp-4h]

    self = Actor_Get(entref);
    mode = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    if ( mode == scr_const.face_angle )
    {
        self->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        fYaw = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        Actor_SetDesiredAngles(&self->ScriptOrient, 0.0, fYaw);
    }
    else if ( mode == scr_const.face_current )
    {
        self->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        self->ScriptOrient.fDesiredBodyYaw = self->fDesiredBodyYaw;
    }
    else if ( mode == scr_const.face_direction )
    {
        Scr_GetVector(1u, v, SCRIPTINSTANCE_SERVER);
        if ( v[0] == 0.0 && v[1] == 0.0 )
            Scr_Error("cannot face (0, 0, *)", 0);
        self->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        Actor_FaceVector(&self->ScriptOrient, v);
    }
    else if ( mode == scr_const.face_enemy )
    {
        self->ScriptOrient.eMode = AI_ORIENT_TO_ENEMY;
    }
    else if ( mode == scr_const.face_enemy_or_motion )
    {
        self->ScriptOrient.eMode = AI_ORIENT_TO_ENEMY_OR_MOTION;
    }
    else if ( mode == scr_const.face_goal )
    {
        self->ScriptOrient.eMode = AI_ORIENT_TO_GOAL;
    }
    else if ( mode == scr_const.face_motion )
    {
        self->ScriptOrient.eMode = AI_ORIENT_TO_MOTION;
    }
    else if ( mode == scr_const.face_point )
    {
        Scr_GetVector(1u, point, SCRIPTINSTANCE_SERVER);
        currentOrigin = self->ent->r.currentOrigin;
        delta[0] = point[0] - *currentOrigin;
        delta[1] = point[1] - currentOrigin[1];
        delta[2] = point[2] - currentOrigin[2];
        self->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        if ( (float)((float)(delta[0] * delta[0]) + (float)(delta[1] * delta[1])) >= 1.0 )
            Actor_FaceVector(&self->ScriptOrient, delta);
    }
    else if ( mode == scr_const.face_default )
    {
        Actor_ClearScriptOrient(self);
    }
    else
    {
        Scr_Error(
            "orientMode must be 'face angle', 'face current', 'face direction', 'face enemy', 'face enemy or motion', 'face goa"
            "l', 'face motion', 'face point', or 'face default'\n"
            "'face direction' and 'face point' take a second argument that is a vector giving the way to face\n"
            "'face angle' takes a second argument that is a yaw angle\n",
            0);
    }
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 2 && Scr_GetInt(2u, SCRIPTINSTANCE_SERVER) > 0 )
    {
        Actor_SetDesiredBodyAngle(&self->CodeOrient, self->ScriptOrient.fDesiredBodyYaw);
        Actor_SetBodyAngle(self, self->ScriptOrient.fDesiredBodyYaw);
    }
}

void __cdecl ActorCmd_GetMotionAngle(scr_entref_t entref)
{
    float fMoveYaw; // [esp+28h] [ebp-Ch]
    float fMoveYawa; // [esp+28h] [ebp-Ch]
    float fDeltaYaw; // [esp+2Ch] [ebp-8h]
    actor_s *self; // [esp+30h] [ebp-4h]

    PROF_SCOPED("GetMotionAngle");

    self = Actor_Get(entref);
    if ( (float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
                         + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1])) <= 1.0 )
    {
        if ( Actor_HasPath(self) )
        {
            fMoveYawa = vectoyaw(self->Path.lookaheadDir);
            fDeltaYaw = AngleNormalize180(fMoveYawa - self->ent->r.currentAngles[1]);
        }
        else
        {
            fDeltaYaw = 0.0f;
        }
    }
    else
    {
        fMoveYaw = vectoyaw(self->Physics.vVelocity);
        fDeltaYaw = AngleNormalize180(fMoveYaw - self->ent->r.currentAngles[1]);
    }
    Scr_AddFloat(fDeltaYaw, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_GetAnglesToLikelyEnemyPath(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Actor_GetAnglesToLikelyEnemyPath(self) )
        Scr_AddVector(self->anglesToLikelyEnemyPath, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_LerpPosition(scr_entref_t entref)
{
    float *v1; // [esp+8h] [ebp-3Ch]
    float *vWishDelta; // [esp+Ch] [ebp-38h]
    float *v3; // [esp+10h] [ebp-34h]
    float *v4; // [esp+14h] [ebp-30h]
    float *vVelocity; // [esp+18h] [ebp-2Ch]
    float *currentOrigin; // [esp+1Ch] [ebp-28h]
    float *currentAngles; // [esp+20h] [ebp-24h]
    float origin[3]; // [esp+24h] [ebp-20h] BYREF
    float angles[3]; // [esp+30h] [ebp-14h] BYREF
    actor_s *self; // [esp+3Ch] [ebp-8h]
    gentity_s *ent; // [esp+40h] [ebp-4h]

    self = Actor_Get(entref);
    if ( self->eScriptSetAnimMode != AI_ANIM_NOPHYSICS )
        Scr_Error("cannot lerp position if animMode is not 'nophysics'", 0);
    ent = self->ent;
    Scr_GetVector(0, origin, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1u, angles, SCRIPTINSTANCE_SERVER);
    currentAngles = ent->r.currentAngles;
    ent->r.currentAngles[0] = angles[0];
    currentAngles[1] = angles[1];
    currentAngles[2] = angles[2];
    Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    vVelocity = self->Physics.vVelocity;
    currentOrigin = ent->r.currentOrigin;
    self->Physics.vVelocity[0] = origin[0] - ent->r.currentOrigin[0];
    vVelocity[1] = origin[1] - currentOrigin[1];
    vVelocity[2] = origin[2] - currentOrigin[2];
    v3 = self->Physics.vVelocity;
    v4 = self->Physics.vVelocity;
    self->Physics.vVelocity[0] = 20.0 * self->Physics.vVelocity[0];
    v3[1] = 20.0 * v4[1];
    v3[2] = 20.0 * v4[2];
    vWishDelta = self->Physics.vWishDelta;
    self->Physics.vWishDelta[0] = 0.0f;
    vWishDelta[1] = 0.0f;
    vWishDelta[2] = 0.0f;
    v1 = ent->r.currentOrigin;
    ent->r.currentOrigin[0] = origin[0];
    v1[1] = origin[1];
    v1[2] = origin[2];
    self->Physics.groundEntNum = 1023;
}

void __cdecl ActorCmd_PredictOriginAndAngles(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    if ( !self
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 2829, 0, "%s", "self") )
    {
        __debugbreak();
    }
    ent = self->ent;
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 2831, 0, "%s", "ent") )
    {
        __debugbreak();
    }
    if ( ent->tagInfo )
        Scr_Error("cannot predict when linked to an entity", 0);
    if ( self->eScriptSetAnimMode == AI_ANIM_NOPHYSICS )
        Scr_Error("cannot predict when using no physics", 0);
    if ( self->eScriptSetAnimMode )
        self->eAnimMode = self->eScriptSetAnimMode;
    else
        self->eAnimMode = AI_ANIM_USE_BOTH_DELTAS;
    self->bUseGoalWeight = 1;
    Actor_PredictOriginAndAngles(self);
}

void __cdecl ActorCmd_PredictAnim(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_PredictAnim(self);
}

void __cdecl ActorCmd_GetHitEntType(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_GetEntType(self->Physics.iHitEntnum);
}

void __cdecl Actor_GetEntType(int entnum)
{
    unsigned __int16 obstacle; // [esp+2h] [ebp-2h]

    if ( entnum == 1023 )
    {
        Scr_AddConstString(scr_const.none, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        if ( (FL_OBSTACLE & level.gentities[entnum].flags) != 0 )
            obstacle = scr_const.obstacle;
        else
            obstacle = scr_const.world;
        Scr_AddConstString(obstacle, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl ActorCmd_GetHitYaw(scr_entref_t entref)
{
    float value; // [esp+0h] [ebp-18h]
    float normal[3]; // [esp+8h] [ebp-10h] BYREF
    actor_s *self; // [esp+14h] [ebp-4h]

    self = Actor_Get(entref);
    if ( self->Physics.iHitEntnum == 1023 )
        Scr_Error("nothing was hit", 0);
    normal[0] = -self->Physics.vHitNormal[0];
    normal[1] = -self->Physics.vHitNormal[1];
    normal[2] = 0.0f;
    value = vectoyaw(normal);
    Scr_AddFloat(value, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_GetGroundEntType(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Actor_GetEntType(self->Physics.groundEntNum);
}

void __cdecl ActorCmd_IsDeflected(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Scr_AddInt(self->Physics.bDeflected, SCRIPTINSTANCE_SERVER);
}

void __cdecl ScrCmd_animcustom(scr_entref_t entref)
{
    int func; // [esp+0h] [ebp-8h]
    actor_s *self; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Actor_PushState(self, AIS_CUSTOMANIM) )
    {
        func = Scr_GetFunc(0, SCRIPTINSTANCE_SERVER);
        Actor_KillAnimScript(self);
        self->AnimScriptSpecific.func = func;
        Scr_SetString(&self->AnimScriptSpecific.name, scr_const._custom, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl ScrCmd_CanAttackEnemyNode(scr_entref_t entref)
{
    sentient_s *TargetSentient; // eax
    pathnode_t *enemyNode; // [esp+0h] [ebp-Ch]
    actor_s *self; // [esp+4h] [ebp-8h]
    pathnode_t *myNode; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Actor_GetTargetSentient(self)
        && (myNode = Sentient_NearestNode(self->sentient),
                TargetSentient = Actor_GetTargetSentient(self),
                enemyNode = Sentient_NearestNode(TargetSentient),
                myNode)
        && enemyNode
        && Path_NodesVisible(myNode, enemyNode) )
    {
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}

// LWSS ADD
void ScrCmd_GetPathLength(scr_entref_t entref)
{
    actor_s *self;
    self = Actor_Get(entref);

    if (Actor_HasPath(self))
    {
        Scr_AddInt(self->Path.wPathLen, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}
void ScrCmd_CalcPathLength(scr_entref_t entref)
{
    actor_s *self;
    self = Actor_Get(entref);

    float goalpos[3];
    Scr_GetVector(0, goalpos, SCRIPTINSTANCE_SERVER);
    Actor_FindPath(self, goalpos, true, true);
    if (Actor_HasPath(self))
    {
        Scr_AddInt(self->Path.wPathLen, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}
// LWSS END

void __cdecl ScrCmd_GetNegotiationStartNode(scr_entref_t entref)
{
    pathnode_t *v1; // eax
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Path_HasNegotiationNode(&self->Path) )
    {
        if ( self->Path.wNegotiationStartNode >= self->Path.wPathLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                        3095,
                        0,
                        "%s",
                        "self->Path.wNegotiationStartNode < self->Path.wPathLen") )
        {
            __debugbreak();
        }
        if ( self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                        3096,
                        0,
                        "%s",
                        "self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum >= 0") )
        {
            __debugbreak();
        }
        v1 = Path_ConvertIndexToNode(self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum);
        Scr_AddPathnode(v1);
    }
}

void __cdecl ScrCmd_GetNegotiationEndNode(scr_entref_t entref)
{
    pathnode_t *v1; // eax
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Path_HasNegotiationNode(&self->Path) )
    {
        if ( self->Path.wNegotiationStartNode >= self->Path.wPathLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                        3119,
                        0,
                        "%s",
                        "self->Path.wNegotiationStartNode < self->Path.wPathLen") )
        {
            __debugbreak();
        }
        const int endNodeIndex = self->Path.pts[self->Path.wNegotiationStartNode - 1].iNodeNum;
        if ( endNodeIndex < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                        3120,
                        0,
                        "%s",
                        "self->Path.pts[self->Path.wNegotiationStartNode - 1].iNodeNum >= 0") )
        {
            __debugbreak();
        }
        v1 = Path_ConvertIndexToNode(endNodeIndex);
        Scr_AddPathnode(v1);
    }
}

void __cdecl ActorCmd_CheckProne(scr_entref_t entref)
{
    unsigned __int8 canGoProne; // [esp+2Fh] [ebp-19h]
    float yaw; // [esp+30h] [ebp-18h]
    float position[3]; // [esp+38h] [ebp-10h] BYREF
    bool isProne; // [esp+47h] [ebp-1h]
    int savedregs; // [esp+48h] [ebp+0h] BYREF

    Actor_Get(entref);
    Scr_GetVector(0, position, SCRIPTINSTANCE_SERVER);
    yaw = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    isProne = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER) != 0;
    canGoProne = BG_CheckProne(
                                 0,
                                 entref.entnum,
                                 position,
                                 15.0,
                                 48.0,
                                 yaw,
                                 0,
                                 0,
                                 isProne,
                                 1,
                                 1,
                                 1u,
                                 PCT_ACTOR,
                                 50.0);
    Scr_AddBool(canGoProne, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_PushPlayer(scr_entref_t entref)
{
    unsigned int v1; // ecx
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
        v1 = self->Physics.iTraceMask & 0xFDFFFFFF;
    else
        v1 = self->Physics.iTraceMask | 0x2000000;
    self->Physics.iTraceMask = v1;
}

void __cdecl ActorCmd_SetGoalNode(scr_entref_t entref)
{
    pathnode_t *node; // [esp+18h] [ebp-14h]
    actor_s *self; // [esp+1Ch] [ebp-10h]
    float ang[3]; // [esp+20h] [ebp-Ch] BYREF

    self = Actor_Get(entref);
    memset(ang, 0, sizeof(ang));
    node = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
    if ( !node->constant.totalLinkCount && (node->constant.spawnflags & 1) == 0 )
        Com_PrintError(
            18,
            "AI %d's goal node at (%0.f, %0.f, %0.f) does not have any path links\n",
            self->ent->s.number,
            node->constant.vOrigin[0],
            node->constant.vOrigin[1],
            node->constant.vOrigin[2]);
    Actor_SetScriptGoalPos(self, node->constant.vOrigin, node, ang);
}

void __fastcall Actor_SetScriptGoalPos(actor_s *self, const float *vGoalPos, pathnode_t *node, float *ang)
{
    //EntHandle::setEnt(&self->scriptGoalEnt, 0);
    self->scriptGoalEnt.setEnt(NULL);
    Actor_ClearKeepClaimedNode(self);
    self->scriptGoal.pos[0] = *vGoalPos;
    self->scriptGoal.pos[1] = vGoalPos[1];
    self->scriptGoal.pos[2] = vGoalPos[2];
    self->scriptGoal.ang[0] = *ang;
    self->scriptGoal.ang[1] = ang[1];
    self->scriptGoal.ang[2] = ang[2];
    self->scriptGoal.node = node;
    if ( self->scriptGoal.volume )
    {
        if ( !SV_EntityContact(vGoalPos, vGoalPos, self->scriptGoal.volume) )
            self->scriptGoal.volume = 0;
    }
}

void __cdecl ActorCmd_SetGoalPos(scr_entref_t entref)
{
    float pos[3]; // [esp+0h] [ebp-1Ch] BYREF
    actor_s *self; // [esp+Ch] [ebp-10h]
    float ang[3]; // [esp+10h] [ebp-Ch] BYREF

    self = Actor_Get(entref);
    memset(ang, 0, sizeof(ang));
    Scr_GetVector(0, pos, SCRIPTINSTANCE_SERVER);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 2 )
        Scr_GetVector(1u, ang, SCRIPTINSTANCE_SERVER);
    Actor_SetScriptGoalPos(self, pos, 0, ang);
}

void __cdecl ActorCmd_SetGoalEntity(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    ent = Scr_GetEntity(0);
    Actor_ClearKeepClaimedNode(self);
    Actor_SetScriptGoalEntity(self, ent);
}

void __fastcall Actor_SetScriptGoalEntity(actor_s *self, gentity_s *pGoalEnt)
{
    if ( !pGoalEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 3243, 0, "%s", "pGoalEnt") )
    {
        __debugbreak();
    }
    if ( !pGoalEnt->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    3244,
                    0,
                    "%s",
                    "pGoalEnt->r.inuse") )
    {
        __debugbreak();
    }
    Actor_ClearKeepClaimedNode(self);
    //EntHandle::setEnt(&self->scriptGoalEnt, pGoalEnt);
    self->scriptGoalEnt.setEnt(pGoalEnt);
    self->scriptGoal.node = 0;
    self->scriptGoal.volume = 0;
}

void __cdecl ActorCmd_SetGoalVolume(scr_entref_t entref)
{
    const char *v1; // eax
    gentity_s *volume; // [esp+1Ch] [ebp-8h]
    actor_s *self; // [esp+20h] [ebp-4h]

    self = Actor_Get(entref);
    //if ( EntHandle::isDefined(&self->scriptGoalEnt) )
    if ( self->scriptGoalEnt.isDefined() )
        Scr_Error("cannot set goal volume when a goal entity is set", 0);
    volume = Scr_GetEntity(0);
    if ( !SV_EntityContact(self->scriptGoal.pos, self->scriptGoal.pos, volume) )
    {
        v1 = va(
                     "cannot set goal volume which does not contain goal position (goal pt %f, %f, %f)",
                     self->scriptGoal.pos[0],
                     self->scriptGoal.pos[1],
                     self->scriptGoal.pos[2]);
        Scr_Error(v1, 0);
    }
    Actor_ClearKeepClaimedNode(self);
    Actor_SetScriptGoalVolume(self, volume);
}

void __fastcall Actor_SetScriptGoalVolume(actor_s *self, gentity_s *volume)
{
    if ( !volume
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 3183, 0, "%s", "volume") )
    {
        __debugbreak();
    }
    if ( !volume->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    3184,
                    0,
                    "%s",
                    "volume->r.inuse") )
    {
        __debugbreak();
    }
    //if ( EntHandle::isDefined(&self->scriptGoalEnt)
    if ( self->scriptGoalEnt.isDefined()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    3186,
                    0,
                    "%s",
                    "!self->scriptGoalEnt.isDefined()") )
    {
        __debugbreak();
    }
    if ( !SV_EntityContact(self->scriptGoal.pos, self->scriptGoal.pos, volume)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    3187,
                    0,
                    "%s",
                    "SV_EntityContact( self->scriptGoal.pos, self->scriptGoal.pos, volume )") )
    {
        __debugbreak();
    }
    self->scriptGoal.volume = volume;
}

void __cdecl ActorCmd_GetGoalVolume(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( self->scriptGoal.volume )
        Scr_AddEntity(self->scriptGoal.volume, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_ClearGoalVolume(scr_entref_t entref)
{
    Actor_Get(entref)->scriptGoal.volume = 0;
}

void __cdecl ActorCmd_SetFixedNodeSafeVolume(scr_entref_t entref)
{
    gentity_s *volume; // [esp+0h] [ebp-8h]
    actor_s *self; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    volume = Scr_GetEntity(0);
    //EntHandle::setEnt(&self->fixedNodeSafeVolume, volume);
    self->fixedNodeSafeVolume.setEnt(volume);
    self->fixedNodeSafeVolumeRadiusSq = RadiusFromBounds2DSq(volume->r.mins, volume->r.maxs);
}

void __cdecl ActorCmd_GetFixedNodeSafeVolume(scr_entref_t entref)
{
    gentity_s *v1; // eax
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    //if ( EntHandle::isDefined(&self->fixedNodeSafeVolume) )
    if ( self->fixedNodeSafeVolume.isDefined() )
    {
        //v1 = EntHandle::ent(&self->fixedNodeSafeVolume);
        v1 = self->fixedNodeSafeVolume.ent();
        Scr_AddEntity(v1, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl ActorCmd_ClearFixedNodeSafeVolume(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    //EntHandle::setEnt(&self->fixedNodeSafeVolume, 0);
    self->fixedNodeSafeVolume.setEnt(NULL);
    self->fixedNodeSafeVolumeRadiusSq = 0.0f;
}

void __cdecl ActorCmd_IsInGoal(scr_entref_t entref)
{
    bool v1; // al
    float pos[3]; // [esp+0h] [ebp-14h] BYREF
    actor_s *self; // [esp+Ch] [ebp-8h]
    const char *usage; // [esp+10h] [ebp-4h]

    self = Actor_Get(entref);
    usage = "illegal call to isingoal()\n";
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1 )
    {
        Scr_GetVector(0, pos, SCRIPTINSTANCE_SERVER);
        v1 = Actor_PointAtGoal(pos, &self->codeGoal);
        Scr_AddBool(v1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_Error(usage, 0);
    }
}

void __cdecl ActorCmd_SetOverrideRunToPos(scr_entref_t entref)
{
    bool posEqual; // [esp+0h] [ebp-1Ch]
    float *animscriptOverrideRunToPos; // [esp+4h] [ebp-18h]
    float pos[3]; // [esp+Ch] [ebp-10h] BYREF
    actor_s *self; // [esp+18h] [ebp-4h]

    self = Actor_Get(entref);
    Scr_GetVector(0, pos, SCRIPTINSTANCE_SERVER);
    posEqual = pos[0] == self->arrivalInfo.animscriptOverrideRunToPos[0]
                    && pos[1] == self->arrivalInfo.animscriptOverrideRunToPos[1]
                    && pos[2] == self->arrivalInfo.animscriptOverrideRunToPos[2];
    if ( !posEqual || !self->arrivalInfo.animscriptOverrideRunTo )
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
    self->arrivalInfo.animscriptOverrideRunTo = 1;
    animscriptOverrideRunToPos = self->arrivalInfo.animscriptOverrideRunToPos;
    self->arrivalInfo.animscriptOverrideRunToPos[0] = pos[0];
    animscriptOverrideRunToPos[1] = pos[1];
    animscriptOverrideRunToPos[2] = pos[2];
}

void __cdecl ActorCmd_ClearOverrideRunToPos(scr_entref_t entref)
{
    actor_s *v1; // eax

    v1 = Actor_Get(entref);
#ifdef KISAK_SP
    // CORRECTED 2026-08-28. The #else branch below is this function as the decompiler emitted it,
    // and it is a MIS-TYPED FLOAT STORE, not a species write. The three stores it makes land at
    // actor_s +3836 / +3840 / +3844; animscriptOverrideRunToPos[0] is at +3836, so +3840 and
    // +3844 are pos[1] and pos[2]. The decompiler re-based the pointer (`v1 + 3836`) and then
    // named those two slots after whatever happens to sit at +0x04 and +0x08 of actor_s --
    // `sentient` and `species`. ActorCmd_SetOverrideRunToPos directly above writes the same three
    // components correctly, which is the shape this clear-function mirrors.
    //
    // It was harmless only because NULL and AI_SPECIES_DOG were both 0. Widening AISpecies makes
    // AI_SPECIES_DOG == 1 on SP, which would store 1 into a float (1.4e-45) and, worse, would read
    // as a real species assignment to anyone maintaining this. Fixed for SP only: the MP path must
    // stay byte-identical, and there the emitted stores are already correct by coincidence.
    v1->arrivalInfo.animscriptOverrideRunTo = 0;
    v1->arrivalInfo.animscriptOverrideRunToPos[0] = 0.0f;
    v1->arrivalInfo.animscriptOverrideRunToPos[1] = 0.0f;
    v1->arrivalInfo.animscriptOverrideRunToPos[2] = 0.0f;
#else
    v1->arrivalInfo.animscriptOverrideRunTo = 0;
    v1->arrivalInfo.animscriptOverrideRunToPos[0] = 0.0f;
    v1 = (actor_s *)((char *)v1 + 3836);
    //v1->sentient = *(sentient_s **)&FLOAT_0_0;
    v1->sentient = NULL;
    v1->species = AI_SPECIES_DOG;
#endif
}

void __cdecl ActorCmd_NearNode(scr_entref_t entref)
{
    bool v1; // al
    pathnode_t *node; // [esp+0h] [ebp-8h]
    actor_s *self; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    node = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
    v1 = Actor_PointNearNode(self->ent->r.currentOrigin, node);
    Scr_AddBool(v1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_ClearEnemy(scr_entref_t entref)
{
    gentity_s *TargetEntity; // esi
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( !self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    3570,
                    0,
                    "%s",
                    "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetSentient(self) )
    {
        self->sentientInfo[Actor_GetTargetSentient(self) - level.sentients].lastKnownPosTime = 0;
        self->faceLikelyEnemyPathNode = 0;
    }
    //if ( EntHandle::isDefined(&self->sentient->scriptTargetEnt) )
    if ( self->sentient->scriptTargetEnt.isDefined() )
    {
        TargetEntity = Actor_GetTargetEntity(self);
        //if ( TargetEntity == EntHandle::ent(&self->sentient->scriptTargetEnt) )
        if ( TargetEntity == self->sentient->scriptTargetEnt.ent() )
        {
            //EntHandle::setEnt(&self->sentient->scriptTargetEnt, 0);
            self->sentient->scriptTargetEnt.setEnt(0);
        }
    }
    Sentient_SetEnemy(self->sentient, 0, 1);
}

static const float AI_ENTITY_TARGET_MAX_THREAT = 1.0;
void __cdecl ActorCmd_SetEntityTarget(scr_entref_t entref)
{
    float v1; // [esp+0h] [ebp-20h]
    int NumParam; // [esp+4h] [ebp-1Ch]
    float Float; // [esp+8h] [ebp-18h]
    float v4; // [esp+10h] [ebp-10h]
    float targetThreat; // [esp+14h] [ebp-Ch]
    gentity_s *targetEnt; // [esp+18h] [ebp-8h]
    actor_s *self; // [esp+1Ch] [ebp-4h]

    self = Actor_Get(entref);
    targetEnt = 0;
    targetThreat = AI_ENTITY_TARGET_MAX_THREAT;

    NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( NumParam != 1 )
    {
        if ( NumParam != 2 )
        {
            if ( NumParam != 3 )
            {
                Scr_Error("No target provided for SetEntityTarget", 0);
                goto LABEL_14;
            }
            self->sentient->scriptTargetTag = (unsigned __int16)Scr_GetConstLowercaseString(2u, SCRIPTINSTANCE_SERVER);
        }
        Float = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        if ( (float)(Float - 1.0) < 0.0 )
            v4 = Float;
        else
            v4 = AI_ENTITY_TARGET_MAX_THREAT;
        if ( (float)(0.0 - Float) < 0.0 )
            v1 = v4;
        else
            v1 = 0.0f;
        targetThreat = v1;
    }

    targetEnt = Scr_GetEntity(0);
LABEL_14:
    
    iassert(self->sentient);
    iassert(targetEnt);

    self->sentient->scriptTargetEnt.setEnt(targetEnt);

    self->sentient->entityTargetThreat = targetThreat;

    if ( self->sentient->entityTargetThreat == AI_ENTITY_TARGET_MAX_THREAT )
        Sentient_SetEnemy(self->sentient, targetEnt, 1);
}

void __cdecl ActorCmd_ClearEntityTarget(scr_entref_t entref)
{
    gentity_s *TargetEntity; // esi
    actor_s *self; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    
    //if ( EntHandle::isDefined(&self->sentient->scriptTargetEnt) )
    if ( self->sentient->scriptTargetEnt.isDefined() )
    {
        TargetEntity = Actor_GetTargetEntity(self);
        //if ( TargetEntity == EntHandle::ent(&self->sentient->scriptTargetEnt) )
        if ( TargetEntity == self->sentient->scriptTargetEnt.ent() )
            Sentient_SetEnemy(self->sentient, 0, 1);
    }
    //EntHandle::setEnt(&self->sentient->scriptTargetEnt, 0);
    self->sentient->scriptTargetEnt.setEnt(0);
    self->sentient->scriptTargetTag = 0;
}

void __cdecl ActorCmd_ClearEntityOwner(scr_entref_t entref)
{
    actor_s *self; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    //EntHandle::setEnt(&self->sentient->scriptOwner, 0);
    self->sentient->scriptOwner.setEnt(0);
    self->ent->s.faction.iHeadIconTeam = self->ent->s.faction.iHeadIconTeam & 3 | 0xFC;
}

void __cdecl ActorCmd_SetEntityOwner(scr_entref_t entref)
{
    actor_s *self; // [esp+4h] [ebp-8h]
    gentity_s *ownerEnt; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    ownerEnt = Scr_GetEntity(0);
    if ( !ownerEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 3701, 0, "%s", "ownerEnt") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    3703,
                    0,
                    "%s",
                    "self->sentient") )
    {
        __debugbreak();
    }
    //EntHandle::setEnt(&self->sentient->scriptOwner, ownerEnt);
    self->sentient->scriptOwner.setEnt(ownerEnt);
    self->ent->s.faction.iHeadIconTeam = self->ent->s.faction.iHeadIconTeam & 3 | (4 * LOBYTE(ownerEnt->s.number));
}

void __cdecl ActorCmd_SetPotentialThreat(scr_entref_t entref)
{
    actor_s *self; // [esp+4h] [ebp-Ch]
    float potentialThreatDir; // [esp+8h] [ebp-8h]

    self = Actor_Get(entref);
    if ( !self || Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1 )
        Scr_Error("illegal call to setpotentialthreat()\n", 0);
    potentialThreatDir = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    Actor_SetPotentialThreat(&self->potentialThreat, potentialThreatDir);
}

void __cdecl ActorCmd_ClearPotentialThreat(scr_entref_t entref)
{
    actor_s *self; // [esp+0h] [ebp-8h]

    self = Actor_Get(entref);
    if ( !self || Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        Scr_Error("illegal call to clearpotentialthreat()\n", 0);
    Actor_ClearPotentialThreat(&self->potentialThreat);
}

void __cdecl ActorCmd_GetPerfectInfo(scr_entref_t entref)
{
    gentity_s *targetEnt; // [esp+0h] [ebp-8h]
    actor_s *self; // [esp+4h] [ebp-4h]

    self = Actor_Get(entref);
    if ( !Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        Scr_Error("No entity provided for GetPerfectInfo", 0);
    targetEnt = Scr_GetEntity(0);
    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp", 3787, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( !targetEnt->sentient )
        Scr_Error("Entity provided to GetPerfectInfo is not a sentient", 0);
    Actor_GetPerfectInfo(self, targetEnt->sentient);
}

const char *USAGEMSG = "Invalid call to setFlashBanged().";
void __cdecl ActorCmd_SetFlashBanged(scr_entref_t entref)
{
    int NumParam; // [esp+4h] [ebp-10h]
    VariableUnion flashed; // [esp+8h] [ebp-Ch]
    actor_s *self; // [esp+Ch] [ebp-8h]
    float strength; // [esp+10h] [ebp-4h]

    self = Actor_Get(entref);
    strength = 1.0f;
    NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( NumParam != 1 )
    {
        if ( NumParam != 2 )
        {
            Scr_Error(USAGEMSG, 0);
            return;
        }
        strength = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    }
    flashed.intValue = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    Actor_SetFlashed(self, flashed.intValue, strength);
}

const char *USAGEMSG_0 = "Invalid call to setFlashbangImmunity().";
void __cdecl ActorCmd_SetFlashbangImmunity(scr_entref_t entref)
{
    actor_s *self; // [esp+4h] [ebp-8h]

    self = Actor_Get(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1 )
        self->flashBangImmunity = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    else
        Scr_Error(USAGEMSG_0, 0);
}

void __cdecl ActorCmd_GetFlashBangedStrength(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    Scr_AddFloat(self->flashBangedStrength, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_IsKnownEnemyInRadius(scr_entref_t entref)
{
    float radius; // [esp+0h] [ebp-14h]
    actor_s *self; // [esp+4h] [ebp-10h]
    float position[3]; // [esp+8h] [ebp-Ch] BYREF

    self = Actor_Get(entref);
    Scr_GetVector(0, position, SCRIPTINSTANCE_SERVER);
    radius = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    if ( Actor_IsKnownEnemyInRegion(self, 0, position, radius) )
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_IsKnownEnemyInVolume(scr_entref_t entref)
{
    gentity_s *volume; // [esp+4h] [ebp-8h]
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    volume = Scr_GetEntity(0);
    if ( Actor_IsKnownEnemyInRegion(self, volume, vec3_origin, 0.0) )
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_SetTalkToSpecies(scr_entref_t entref)
{
    int i; // [esp+0h] [ebp-14h]
    int speciesFlag; // [esp+4h] [ebp-10h]
    actor_s *self; // [esp+8h] [ebp-Ch]
    unsigned int species; // [esp+Ch] [ebp-8h]
    unsigned int paramNum; // [esp+10h] [ebp-4h]

    self = Actor_Get(entref);
    speciesFlag = 0;
    for ( paramNum = 0; paramNum < Scr_GetNumParam(SCRIPTINSTANCE_SERVER); ++paramNum )
    {
        species = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
        if ( species == scr_const.all )
        {
            self->talkToSpecies = -1;
            return;
        }
        for ( i = 0; i < MAX_AI_SPECIES; ++i )
        {
            if ( species == *g_AISpeciesNames[i] )
            {
                speciesFlag |= 1 << i;
                break;
            }
        }
    }
    self->talkToSpecies = speciesFlag;
}

void __cdecl ActorCmd_GetDeltaTurnYaw(scr_entref_t entref)
{
    float value; // [esp+0h] [ebp-Ch]
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    if ( !Flame_GetLocalClientSourceRange()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_script_cmd.cpp",
                    4069,
                    0,
                    "%s",
                    "G_IsSpeciesDog( self->species )") )
    {
        __debugbreak();
    }
    value = Actor_Dog_GetDeltaTurnYaw(self);
    Scr_AddFloat(value, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_SetAnimState(scr_entref_t entref)
{
    unsigned __int16 animstate; // [esp+0h] [ebp-Ch]
    actor_s *self; // [esp+4h] [ebp-8h]
    int anim_state_index; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    animstate = (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    anim_state_index = BG_Actor_GetAnimStateIndex(animstate);
    BG_Dog_SetAnimState(&self->ent->s, anim_state_index);
}

void __cdecl ActorCmd_SetAimAnimWeights(scr_entref_t entref)
{
    actor_s *self; // [esp+8h] [ebp-4h]

    self = Actor_Get(entref);
    self->ent->s.animState.fAimUpDown = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    self->ent->s.animState.fAimLeftRight = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
}

void __cdecl ActorCmd_finishActorDamage(scr_entref_t entref)
{
#ifdef KISAK_SP
    typedef void(__cdecl *actor_die_sp_t)(
        gentity_s *,
        gentity_s *,
        gentity_s *,
        int,
        int,
        int,
        const float *,
        hitLocation_t);

    actor_s *self = Actor_Get(entref);
    gentity_s *inflictor = &g_entities[ENTITYNUM_WORLD];
    gentity_s *attacker = &g_entities[ENTITYNUM_WORLD];
    float vPoint[3];
    float vDir[3] = { 0.0f, 0.0f, 0.0f };
    float *point = 0;
    float *dir = 0;
    const int damage = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    if ( damage <= 0 )
        return;

    if ( Scr_GetType(0, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == 19 )
        inflictor = Scr_GetEntity(0);
    if ( Scr_GetType(1u, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(1u, SCRIPTINSTANCE_SERVER) == 19 )
        attacker = Scr_GetEntity(1u);

    const int dflags = Scr_GetInt(3u, SCRIPTINSTANCE_SERVER);
    const meansOfDeath_t mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(4u);
    char *weaponName = Scr_GetString(5u, SCRIPTINSTANCE_SERVER);
    const int iWeapon = G_GetWeaponIndexForName(weaponName);
    if ( Scr_GetType(6u, SCRIPTINSTANCE_SERVER) )
    {
        Scr_GetVector(6u, vPoint, SCRIPTINSTANCE_SERVER);
        point = vPoint;
    }
    if ( Scr_GetType(7u, SCRIPTINSTANCE_SERVER) )
    {
        Scr_GetVector(7u, vDir, SCRIPTINSTANCE_SERVER);
        dir = vDir;
    }

    const unsigned __int16 hitLocString = (unsigned __int16)Scr_GetConstString(8u, SCRIPTINSTANCE_SERVER);
    const hitLocation_t hitLoc = (hitLocation_t)G_GetHitLocationIndexFromString(hitLocString);
    const int modelIndex = Scr_GetInt(9u, SCRIPTINSTANCE_SERVER);
    const int psTimeOffset = Scr_GetInt(10u, SCRIPTINSTANCE_SERVER);

    // Retail 0x007CE570 reports damage received after native death through a
    // separate notification and performs no further health/state mutation.
    if ( !self->Physics.bIsAlive )
    {
        G_DamageNotify(
            scr_const.damage_afterdeath,
            self->ent,
            attacker,
            dir,
            point,
            damage,
            mod,
            dflags,
            modelIndex,
            hitLocString,
            weaponName);
        return;
    }

    self->ent->health -= damage;
    G_DamageNotify(
        scr_const.damage,
        self->ent,
        attacker,
        dir,
        point,
        damage,
        mod,
        dflags,
        modelIndex,
        hitLocString,
        weaponName);

    if ( self->ent->health > 0 )
    {
        void(__cdecl *pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int) =
            entityHandlers[self->ent->handler].pain;
        if ( pain )
            pain(self->ent, attacker, damage, point, mod, vDir, hitLoc, iWeapon);
        return;
    }

    if ( self->ent->health < -999 )
        self->ent->health = -999;
    ScrNotify_FaceEvent(self->ent, scr_const.death);
    if ( iWeapon )
        Scr_AddString((char *)BG_WeaponName(iWeapon), SCRIPTINSTANCE_SERVER);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    Scr_AddConstString(*modNames[mod], SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
    Scr_Notify(self->ent, scr_const.death, 3u);

    // The script callback owns timeOffset.  The native retail die handler is
    // a distinct eight-word cdecl dispatch and contains no script callback.
    Scr_ActorKilled(self->ent, inflictor, attacker, damage, mod, iWeapon, vDir, hitLoc, psTimeOffset);
    if ( self->ent->sentient )
        self->ent->sentient->lastAttacker.setEnt(attacker);
    actor_die_sp_t die = reinterpret_cast<actor_die_sp_t>(entityHandlers[self->ent->handler].die);
    if ( die )
        die(self->ent, inflictor, attacker, damage, mod, iWeapon, vDir, hitLoc);
#else
    char *String; // eax
    unsigned __int16 floatValue; // ax
    unsigned __int8 v3; // al
    gentity_s *attacker; // [esp+Ch] [ebp-5Ch]
    int damage; // [esp+10h] [ebp-58h]
    meansOfDeath_t mod; // [esp+14h] [ebp-54h]
    float *dir; // [esp+18h] [ebp-50h]
    float localdir[3]; // [esp+1Ch] [ebp-4Ch] BYREF
    float vDir[3]; // [esp+28h] [ebp-40h] BYREF
    float vPoint[3]; // [esp+34h] [ebp-34h] BYREF
    gentity_s *tempBulletHitEntity; // [esp+40h] [ebp-28h]
    int iWeapon; // [esp+44h] [ebp-24h]
    int psTimeOffset; // [esp+48h] [ebp-20h]
    actor_s *self; // [esp+4Ch] [ebp-1Ch]
    int dflags; // [esp+50h] [ebp-18h]
    gentity_s *inflictor; // [esp+54h] [ebp-14h]
    hitLocation_t hitLoc; // [esp+58h] [ebp-10h]
    void(__cdecl * die)(gentity_s *, gentity_s *, gentity_s *, int, int, const int, const float *, const hitLocation_t, int); // [esp+5Ch] [ebp-Ch]
    void(__cdecl * pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int); // [esp+60h] [ebp-8h]
    const float *point; // [esp+64h] [ebp-4h]

    self = Actor_Get(entref);
    inflictor = &g_entities[1022];
    attacker = &g_entities[1022];
    dir = 0;
    point = 0;
    tempBulletHitEntity = 0;
    damage = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    if (damage > 0)
    {
        if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == 19)
            inflictor = Scr_GetEntity(0);
        if (Scr_GetType(1u, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(1u, SCRIPTINSTANCE_SERVER) == 19)
            attacker = Scr_GetEntity(1u);
        dflags = Scr_GetInt(3u, SCRIPTINSTANCE_SERVER);
        mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(4u);
        String = Scr_GetString(5u, SCRIPTINSTANCE_SERVER);
        iWeapon = G_GetWeaponIndexForName(String);
        if (Scr_GetType(6u, SCRIPTINSTANCE_SERVER))
        {
            Scr_GetVector(6u, vPoint, SCRIPTINSTANCE_SERVER);
            point = vPoint;
        }
        if (Scr_GetType(7u, SCRIPTINSTANCE_SERVER))
        {
            Scr_GetVector(7u, vDir, SCRIPTINSTANCE_SERVER);
            dir = vDir;
        }
        floatValue = (unsigned __int16)Scr_GetConstString(8u, SCRIPTINSTANCE_SERVER);
        hitLoc = (hitLocation_t)G_GetHitLocationIndexFromString(floatValue);
        psTimeOffset = Scr_GetInt(9u, SCRIPTINSTANCE_SERVER);
        if (self->Physics.bIsAlive)
        {
            if (dir)
                Vec3NormalizeTo(dir, localdir);
            else
                memset(localdir, 0, sizeof(localdir));
            if ((self->ent->flags & 1) == 0)
            {
                if (iWeapon && BG_GetWeaponDef(iWeapon)->weapType == WEAPTYPE_BULLET && IsBulletImpactMOD(mod))
                {
                    tempBulletHitEntity = G_TempEntity(vPoint, EV_BULLET_HIT);
                    AssignToSmallerType<unsigned short>(&tempBulletHitEntity->s.weapon, iWeapon);
                    v3 = DirToByte(localdir);
                    tempBulletHitEntity->s.eventParm = v3;
                    tempBulletHitEntity->s.un1.scale = hitLoc == HITLOC_HEAD;
                    tempBulletHitEntity->s.surfType = 7;
                    AssignToSmallerType<short>(&tempBulletHitEntity->s.otherEntityNum, attacker->s.number);
                }
                if ((self->ent->flags & 2) != 0 && self->ent->health - damage <= 0)
                    damage = self->ent->health - 1;
                if (g_debugDamage->current.enabled)
                    Com_Printf(15, "target:%i health:%i damage:%i\n", self->ent->s.number, self->ent->health, damage);
                self->ent->health -= damage;
                Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
                Scr_AddInt(damage, SCRIPTINSTANCE_SERVER);
                Scr_Notify(self->ent, scr_const.damage, 2u);
                ScrNotify_FaceEvent(self->ent, scr_const.damage);
                if (self->ent->health > 0)
                {
                    pain = entityHandlers[self->ent->handler].pain;
                    if (pain)
                        pain(self->ent, attacker, damage, point, mod, localdir, hitLoc, iWeapon);
                }
                else
                {
                    if (self->ent->health < -999)
                        self->ent->health = -999;
                    ScrNotify_FaceEvent(self->ent, scr_const.death);
                    Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(self->ent, scr_const.death, 1u);
                    die = entityHandlers[self->ent->handler].die;
                    if (die)
                        die(self->ent, inflictor, attacker, damage, mod, iWeapon, localdir, hitLoc, psTimeOffset);
                }
            }
            else
            {
                Scr_Error("Trying to do damage to a actor that is already dead", 0);
            }
        }
    }
#endif
}
