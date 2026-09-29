#include "cg_main_mp.h"
#ifdef KISAK_SP
#include "cg_animscripted_mp.h"
#include <set>
#include <string>
#endif
#include <universal/q_shared.h>
#include <universal/com_memory.h>
#include <universal/assertive.h>
#include <universal/dvar.h>
#include "cg_view_mp.h"
#include <DynEntity/DynEntity_client.h>
#include <cgame/offhandweapons.h>
#include <cgame/cg_compass.h>
#include <cgame/cg_spikeacoustic.h>
#include <cgame/cg_ammocounter.h>
#include <cgame/cg_visionsets.h>
#include "cg_scoreboard_mp.h"
#include <cgame/cg_hudelem.h>
#include <cgame/cg_weapon_options.h>
#include <turret/turret_placement.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_main_mp.h>
#include "cg_local_mp.h"
#include <ragdoll/ragdoll.h>
#include <ik/ik.h>
#include <universal/com_math_anglevectors.h>
#include <client/cl_console.h>
#include <sound/snd_bank.h>
#include <universal/surfaceflags.h>
#include <cgame/cg_sound.h>
#include <sound/snd_public_async.h>
#include <EffectsCore/fx_system.h>
#include <EffectsCore/fx_update.h>
#include <client_mp/cl_cgame_mp.h>
#include <stringed/stringed_hooks.h>
#include <clientscript/scr_const.h>
#include <clientscript/cscr_stringlist.h>
#include <game/g_load_utils.h>
#include <cgame/cg_scr_main.h>
#include "cg_vehicles_mp.h"
#include <qcommon/dobj_management.h>
#include "cg_ents_mp.h"
#include <clientscript/cscr_memorytree.h>
#include <qcommon/threads.h>
#include <bgame/bg_slidemove.h>
#include <cgame/cg_draw_names.h>
#include <cgame/cg_main.h>
#include <client_mp/cl_input_mp.h>
#include <EffectsCore/fx_load_obj.h>
#include "cg_servercmds_mp.h"
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_dvars.h>
#include <client_mp/cl_scrn_mp.h>
#include "cg_newDraw_mp.h"
#include "cg_consolecmds_mp.h"
#include <bgame/bg_mantle.h>
#include <bgame/bg_dog_animations_mp.h>
#include <bgame/bg_vehicle_anim.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_info.h>
#include <gfx_d3d/r_stream.h>
#include <qcommon/com_profilemapload.h>
#include <cgame/cg_bolt.h>
#include <cgame/cg_localents.h>
#include <cgame/cg_drawtools.h>
#include <gfx_d3d/r_water_sim.h>
#include <gfx_d3d/r_bsp_load_obj.h>
#include <aim_assist/aim_target.h>
#include <client/splitscreen.h>
#include <bgame/bg_fire.h>
#include <gfx_d3d/r_model.h>
#include <cgame/cg_effects_load_obj.h>
#include <ui/ui_shared_obj.h>
#include "cg_draw_mp.h"
#include <ragdoll/ragdoll_update.h>
#include <cgame/cg_spawn.h>
#include <gfx_d3d/r_shader_constant_set.h>
#include "cg_animtree_mp.h"
#include <gfx_d3d/r_cinematic.h>
#include "cg_snapshot_mp.h"
#include <glass/glass_client.h>
#include <physics/rope.h>
#include "cg_predict_mp.h"
#include <cgame/cg_local.h>
#include <win32/win_main.h>

const char *cg_thirdPersonModeNames[4] =
{ "Free", "Fixed", "Locked", NULL };

const char *cg_drawMaterialNames[6] =
{
  "Off",
  "CONTENTS_SOLID",
  "NONSOLID",
  "MASK_PLAYERSOLID",
  "MASK_CLIENTEFFECTS",
  NULL
};

const char *snd_drawInfoStrings[5] =
{ "None", "3D", "Stream", "2D", NULL };

const char *snd_drawSortStrings[6] =
{ "priority", "channel", "alias", "dry level", "entity", NULL };

const char *cg_drawBudgetNames[6] =
{ "Off", "Critical Only", "All >=64", "32 to 63", "<32", NULL };

const char *cg_drawFpsNames[5] =
{ "Off", "Simple", "SimpleRanges", "Verbose", NULL };

const char *debugOverlayNames[4] =
{ "Off", "ViewmodelInfo", "FontTest", NULL };

float (*cg_entityOriginArray[1])[3];
unsigned __int8 *cg_ikBuf[1];

centity_s *cg_entitiesArray[1];
fake_centity_s *cg_fakeEntitiesArray;

BattleChatterParams cg_BattleChatters[8];

cgMedia_t cgMedia;
bgsAnim_s cg_bgsAnim;

int cg_usedTriggerCount;
int cg_usedTriggers[300];
bool g_mapLoaded;
bool g_ambientStarted;

bool cg_fakeEntitiesInuseArray[512];
int cg_fakeEntitiesInuseCount[1];
int cg_fakeEntitiesInuseCountFromMap;
int cg_fakeEntitiesInuseCountFromLoadScript;

const dvar_s *cg_loadScripts;
const dvar_s *cg_usingClientScripts;
const dvar_s *cg_drawGun;
const dvar_s *cg_cursorHints;
const dvar_s *cg_retrieveHintTime;
const dvar_s *cg_retrieveHintTimeStuck;
const dvar_s *cg_weaponHintsCoD1Style;
const dvar_s *cg_hintFadeTime;
const dvar_s *cg_seatHintFadeTime;
const dvar_s *cg_fov;
const dvar_s *cg_fov_default;
const dvar_s *cg_fov_default_thirdperson;
const dvar_s *cg_fovScale;
const dvar_s *cg_fovMin;
const dvar_s *cg_fovExtraCam;
const dvar_s *cg_fovCompMax;
const dvar_s *cg_adsZoomToggleStyle;
const dvar_s *cg_viewVehicleInfluenceGunner;
const dvar_s *cg_viewVehicleInfluenceGunnerFiring;
const dvar_s *cg_draw2D;
const dvar_s *cg_drawErrorMessages;
const dvar_s *cg_drawHealth;
const dvar_s *cg_drawBreathHint;
const dvar_s *cg_drawMantleHint;
const dvar_s *cg_wadefps;
const dvar_s *cg_drawFPS;
const dvar_s *cg_drawFPSScale;
const dvar_s *cg_drawBudgets;
const dvar_s *cg_drawDynSModelBudget;
const dvar_s *cg_development;
const dvar_s *cg_drawAnimAttachTags;
const dvar_s *cg_drawFPSOnly;
const dvar_s *cg_profile_physics;
const dvar_s *cg_drawFPSLabels;
const dvar_s *cg_debugInfoCornerOffset;
const dvar_s *cg_drawVersion;
const dvar_s *cg_drawVersionX;
const dvar_s *cg_drawVersionY;
const dvar_s *cg_readTitleStorageLocally;
const dvar_s *snd_drawInfo;
const dvar_s *snd_drawSort;
const dvar_s *cg_drawScriptUsage;
const dvar_s *cg_drawMaterial;
const dvar_s *cg_drawModelAxis;
const dvar_s *cg_drawSnapshot;
const dvar_s *cg_drawSnapshotTime;
const dvar_s *cg_drawCrosshair;
const dvar_s *cg_drawCrosshair3D;
const dvar_s *cg_drawHoldBreathHint;
const dvar_s *cg_drawTurretCrosshair;
const dvar_s *cg_drawCrosshairNames;
const dvar_s *cg_drawCrosshairNamesPosX;
const dvar_s *cg_drawCrosshairNamesPosY;
const dvar_s *cg_drawShellshock;
const dvar_s *cg_hudStanceFlash;
const dvar_s *cg_hudStanceHintPrints;
const dvar_s *cg_hudDamageIconWidth;
const dvar_s *cg_hudDamageIconHeight;
const dvar_s *cg_hudDamageIconOffset;
const dvar_s *cg_hudDamageIconTime;
const dvar_s *cg_hudDamageDirectionalIconTime;
const dvar_s *cg_hudDamageIconInScope;
const dvar_s *cg_hudGrenadeIconMaxRangeFrag;
const dvar_s *cg_hudGrenadeIconMaxRangeFlash;
const dvar_s *cg_hudGrenadeIconMaxHeight;
const dvar_s *cg_hudGrenadeIconInScope;
const dvar_s *cg_hudGrenadeIconOffset;
const dvar_s *cg_hudGrenadeIconHeight;
const dvar_s *cg_hudGrenadeIconWidth;
const dvar_s *cg_hudGrenadeIconEnabledFlash;
const dvar_s *cg_hudGrenadePointerHeight;
const dvar_s *cg_hudGrenadePointerWidth;
const dvar_s *cg_hudGrenadePointerPivot;
const dvar_s *cg_hudGrenadePointerPulseFreq;
const dvar_s *cg_hudGrenadePointerPulseMax;
const dvar_s *cg_hudGrenadePointerPulseMin;
const dvar_s *cg_hudChatPosition;
const dvar_s *cg_hudSayPosition;
const dvar_s *cg_hudChatIntermissionPosition;
const dvar_s *cg_hudVotePosition;
const dvar_s *cg_debugDrawSafeAreas;
const dvar_s *cg_drawLagometer;
const dvar_s *drawEntityCount;
const dvar_s *drawEntityCountPos;
const dvar_s *drawEntityCountSize;
const dvar_s *drawServerBandwidth;
const dvar_s *drawServerBandwidthPos;
const dvar_s *drawServerBandwidthSize;
const dvar_s *drawKillcamData;
const dvar_s *drawKillcamDataPos;
const dvar_s *drawKillcamDataSize;
const dvar_s *cg_hudProneY;
const dvar_s *cg_mapLocationSelectionCursorSpeed;
const dvar_s *cg_mapLocationSelectionRotationSpeed;
const dvar_s *cg_hudGrenadeIndicatorFadeUp;
const dvar_s *cg_hudGrenadeIndicatorTargetColor;
const dvar_s *cg_hudGrenadeIndicatorStartColor;
const dvar_s *cg_weaponCycleDelay;
const dvar_s *cg_crosshairAlpha;
const dvar_s *cg_crosshairAlphaMin;
const dvar_s *cg_crosshairDynamic;
const dvar_s *cg_crosshairEnemyColor;
const dvar_s *cg_brass;
const dvar_s *cg_gun_fovcomp_x;
const dvar_s *cg_gun_fovcomp_y;
const dvar_s *cg_gun_fovcomp_z;
const dvar_s *cg_gun_x;
const dvar_s *cg_gun_y;
const dvar_s *cg_gun_z;
const dvar_s *cg_gun_move_f;
const dvar_s *cg_gun_move_r;
const dvar_s *cg_gun_move_u;
const dvar_s *cg_gun_ofs_f;
const dvar_s *cg_gun_ofs_r;
const dvar_s *cg_gun_ofs_u;
const dvar_s *cg_gun_move_rate;
const dvar_s *cg_gun_move_minspeed;
const dvar_s *cg_centertime;
const dvar_s *cg_debugPosition;
const dvar_s *cg_debugEvents;
const dvar_s *cg_errorDecay;
const dvar_s *cg_nopredict;
const dvar_s *cg_showmiss;
const dvar_s *cg_footsteps;
const dvar_s *cg_footprints;
const dvar_s *cg_footprintsDistortWater;
const dvar_s *cg_footprintsDebug;
const dvar_s *cg_waterTrailRippleFrequency;
const dvar_s *cg_waterTrailRippleVariance;
const dvar_s *cg_treadmarks;
const dvar_s *cg_firstPersonTracerChance;
const dvar_s *cg_laserForceOn;
const dvar_s *cg_laserRange;
const dvar_s *cg_laserRangePlayer;
const dvar_s *cg_laserRadius;
const dvar_s *cg_laserLight;
const dvar_s *cg_laserLightBodyTweak;
const dvar_s *cg_laserLightRadius;
const dvar_s *cg_laserLightBeginOffset;
const dvar_s *cg_laserEndOffset;
const dvar_s *cg_laserLightEndOffset;
const dvar_s *cg_laserFlarePct;
const dvar_s *cg_marks_ents_player_only;
const dvar_s *cg_tracerChance;
const dvar_s *cg_tracerWidth;
const dvar_s *cg_tracerSpeed;
const dvar_s *cg_tracerLength;
const dvar_s *cg_tracerNoDrawTime;
const dvar_s *cg_tracerScale;
const dvar_s *cg_tracerScaleMinDist;
const dvar_s *cg_tracerScaleDistRange;
const dvar_s *cg_tracerScrewDist;
const dvar_s *cg_tracerScrewRadius;
const dvar_s *cg_bulletWidth;
const dvar_s *cg_bulletLength;
const dvar_s *cg_thirdPersonRange;
const dvar_s *cg_thirdPersonAngle;
const dvar_s *cg_thirdPersonFocusDist;
const dvar_s *cg_thirdPerson;
const dvar_s *cg_thirdPersonMode;
const dvar_s *cg_chatTime;
const dvar_s *cg_chatHeight;
const dvar_s *cg_predictItems;
const dvar_s *cg_spectateThirdPerson;
const dvar_s *cg_teamChatsOnly;
const dvar_s *cg_use_colored_smoke;
const dvar_s *cg_fakefireWizbyChance;
const dvar_s *cg_paused;
const dvar_s *cg_drawpaused;
const dvar_s *cg_synchronousClients;
const dvar_s *cg_debug_overlay_viewport;
const dvar_s *cg_fs_debug;
const dvar_s *cg_debugFace;
const dvar_s *cg_dumpAnims;
const dvar_s *cg_developer;
const dvar_s *cg_minicon;
const dvar_s *cg_subtitles;
const dvar_s *cg_subtitleMinTime;
const dvar_s *cg_subtitleWidthStandard;
const dvar_s *cg_subtitleWidthWidescreen;
const dvar_s *cg_gameMessageWidth;
const dvar_s *cg_gameBoldMessageWidth;
const dvar_s *cg_descriptiveText;
const dvar_s *cg_youInKillCamSize;
const dvar_s *cg_scriptIconSize;
const dvar_s *cg_connectionIconSize;
const dvar_s *cg_voiceIconSize;
const dvar_s *cg_constantSizeHeadIcons;
const dvar_s *cg_headIconMinScreenRadius;
const dvar_s *cg_overheadNamesMaxDist;
const dvar_s *cg_overheadNamesNearDist;
const dvar_s *cg_overheadNamesFarDist;
const dvar_s *cg_overheadNamesFarScale;
const dvar_s *cg_overheadNamesSize;
const dvar_s *cg_overheadIconSize;
const dvar_s *cg_overheadRankSize;
const dvar_s *cg_overheadNamesGlow;
const dvar_s *cg_overheadNamesFont;
const dvar_s *cg_drawFriendlyNames;
const dvar_s *cg_enemyNameFadeIn;
const dvar_s *cg_friendlyNameFadeIn;
const dvar_s *cg_enemyNameFadeOut;
const dvar_s *cg_friendlyNameFadeOut;
const dvar_s *cg_drawThroughWalls;
const dvar_s *cg_playerHighlightTargetSize;
const dvar_s *cg_playerHighlightEnemyColor;
const dvar_s *cg_playerHighlightBrightness;
const dvar_s *cg_playerHighlightMinFade;
const dvar_s *cg_playerHighlightBlinkTime;
const dvar_s *cg_corpseHighlightFadeTime;
const dvar_s *cg_cameraSpikeHighlightBrightness;
const dvar_s *cg_cameraSpikeEnemyColor;
const dvar_s *cg_adsZScaleMax;
const dvar_s *cg_infraredHighlightScale;
const dvar_s *cg_infraredHighlightOffset;
const dvar_s *cg_allow_mature;
const dvar_s *cg_mature;
const dvar_s *cg_blood;
const dvar_s *cg_invalidCmdHintDuration;
const dvar_s *cg_invalidCmdHintBlinkInterval;
const dvar_s *cg_viewZSmoothingMin;
const dvar_s *cg_viewZSmoothingMax;
const dvar_s *cg_viewZSmoothingTime;
const dvar_s *overrideNVGModelWithKnife;
const dvar_s *cg_visionSetLerpMaxIncreasePerFrame;
const dvar_s *cg_visionSetLerpMaxDecreasePerFrame;
const dvar_s *cg_flareVisionSetFadeDuration;
const dvar_s *cg_turretBipodOffset;
const dvar_s *cg_AllPlayerNamesVisible;
const dvar_s *cg_ScoresColor_MyTeam;
const dvar_s *cg_ScoresColor_EnemyTeam;
const dvar_s *cg_ScoresColor_Spectator;
const dvar_s *cg_ScoresColor_Free;
const dvar_s *cg_ScoresColor_Allies;
const dvar_s *cg_ScoresColor_Axis;
const dvar_s *cg_TeamName_Allies;
const dvar_s *cg_TeamName_Axis;
const dvar_s *cg_TeamColor_Allies;
const dvar_s *cg_TeamColor_Axis;
const dvar_s *cg_TeamColor_MyTeam;
const dvar_s *cg_TeamColor_EnemyTeam;
const dvar_s *cg_TeamColor_MyTeamAlt;
const dvar_s *cg_TeamColor_EnemyTeamAlt;
const dvar_s *cg_TeamColor_Squad;
const dvar_s *cg_TeamColor_Spectator;
const dvar_s *cg_TeamColor_Free;
const dvar_s *cg_proneFeetCollisionHull;

const dvar_s *g_compassShowEnemies;
const dvar_s *cg_drawWVisDebug;
const dvar_s *debugOverlay;
const dvar_s *cg_motionblur_duration;
const dvar_s *cg_motionblur_fadeout;
const dvar_s *cg_timedDamageDuration;
const dvar_s *cg_MinDownedPulseRate;
const dvar_s *cg_MaxDownedPulseRate;
const dvar_s *cg_playerFrustumHalfHeight;
const dvar_s *cg_overheadNamesTagUpdateInterval;
const dvar_s *cg_canSeeFriendlyFrustumUpdateInterval;
const dvar_s *cg_canSeeFriendlyFrustumExpand;
const dvar_s *cg_canSeeFriendlyFrustumMinDistance;
const dvar_s *cg_watersheeting;
const dvar_s *cg_debug_triggers;
const dvar_s *cg_cameraWaterClip;
#ifdef KISAK_SP
const dvar_s *cg_cameraUseTagCamera;
#endif
const dvar_s *cg_cameraVehicleExitTweenTime;
const dvar_s *cg_vehicle_piece_damagesfx_threshold;
const dvar_s *cg_debugLocHit;
const dvar_s *cg_debugLocHitTime;

cg_s *cgArray;
cgs_t *cgsArray;

int __cdecl CG_GetClientNumForLocalClient(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    625,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    return cgArray[localClientNum].clientNum;
}

bool __cdecl CG_IsRagdollTrajectory(const trajectory_t *trajectory)
{
    if ( !trajectory
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 632, 0, "%s", "trajectory") )
    {
        __debugbreak();
    }
    if ( !ragdoll_enable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 633, 0, "%s", "ragdoll_enable") )
    {
        __debugbreak();
    }
    return ragdoll_enable->current.enabled && trajectory->trType >= 0xCu && trajectory->trType <= 0xEu;
}

void __cdecl CG_SetupSplitscreenDvars()
{
    Dvar_SetFloat((dvar_s *)cg_hudGrenadeIconHeight, 25.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadeIconWidth, 25.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadeIconOffset, 50.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadePointerHeight, 12.0);
    Dvar_SetFloat((dvar_s *)cg_hudGrenadePointerWidth, 25.0);
    Dvar_SetVec2((dvar_s *)cg_hudGrenadePointerPivot, (12.0), (27.0));
    Dvar_SetFloat((dvar_s *)cg_fovScale, 1.0);
}

#include <cgame/cg_draw_names.h>

void __cdecl CG_RegisterDvars()
{
    bool v0; // al
#ifdef KISAK_SP
    CG_RegisterLookAtDvars_SP();
    const float extraCamDefaultFov = 65.0f;
#else
    const float extraCamDefaultFov = 30.0f;
#endif

    v0 = G_ExitAfterToolComplete();
    cg_loadScripts = _Dvar_RegisterBool("g_loadScripts", !v0, 0, "Disable scripts from loading");
    cg_usingClientScripts = _Dvar_RegisterBool("cg_usingClientScripts", 1, 0x80u, "True, if client scripts are enabled.");
    cg_drawGun = _Dvar_RegisterBool("cg_drawGun", 1, 0x80u, "Draw the view model");
    cg_cursorHints = _Dvar_RegisterInt(
                                         "cg_cursorHints",
                                         4,
                                         0,
                                         4,
                                         1u,
                                         "Draw cursor hints where:\n"
                                         " 0: no hints\n"
                                         "\t1:\tsin size pulse\n"
                                         "\t2:\tone way size pulse\n"
                                         "\t3:\talpha pulse\n"
                                         "\t4:\tstatic image");
    cg_retrieveHintTime = _Dvar_RegisterInt(
                                                    "cg_retrieveHintTime",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0x80u,
                                                    "Time in milliseconds between the landing of a retrievable object and the start of the pulse sh"
                                                    "ader to hint that the object is retrievable");
    cg_retrieveHintTimeStuck = _Dvar_RegisterInt(
                                                             "cg_retrieveHintTimeStuck",
                                                             2000,
                                                             0,
                                                             0x7FFFFFFF,
                                                             0x80u,
                                                             "Time in milliseconds between the retrievable object being stuck in an entity and the star"
                                                             "t of the pulse shader to hint that the object is retrievable");
    cg_weaponHintsCoD1Style = _Dvar_RegisterBool(
                                                            "cg_weaponHintsCoD1Style",
                                                            1,
                                                            0x1000u,
                                                            "Draw weapon hints in CoD1 style: with the weapon name, and with the icon below");
    cg_hintFadeTime = _Dvar_RegisterInt(
                                            "cg_hintFadeTime",
                                            100,
                                            0,
                                            0x7FFFFFFF,
                                            1u,
                                            "Time in milliseconds for the cursor hint to fade");
    cg_seatHintFadeTime = _Dvar_RegisterInt(
                                                    "cg_seatHintFadeTime",
                                                    2000,
                                                    0,
                                                    0x7FFFFFFF,
                                                    1u,
                                                    "Time in milliseconds for the seat hint to fade");
    cg_fov = _Dvar_RegisterFloat("cg_fov", 65.0, 1.0, 160.0, 0x80u, "The field of view angle in degrees");
    cg_fov_default = _Dvar_RegisterFloat(
                                         "cg_fov_default",
                                         65.0,
                                         65.0,
                                         80.0,
                                         1u,
                                         "User default field of view angle in degrees");
    cg_fov_default_thirdperson = _Dvar_RegisterFloat(
                                                                 "cg_fov_default_thirdperson",
                                                                 40.0,
                                                                 1.0,
                                                                 160.0,
                                                                 1u,
                                                                 "User default 3rd person field of view angle in degrees");
    cg_fovScale = _Dvar_RegisterFloat("cg_fovScale", 1.0, 0.2, 2.0, 0x80u, "Scale applied to the field of view");
    cg_fovMin = _Dvar_RegisterFloat("cg_fovMin", 10.0, 1.0, 160.0, 0x80u, "The minimum possible field of view");
    cg_fovExtraCam = _Dvar_RegisterFloat(
                                         "cg_fovExtraCam",
                                         extraCamDefaultFov,
                                         1.0,
                                         160.0,
                                         0x80u,
                                         "The field of view angle in degrees for the extra cam");
    cg_fovCompMax = _Dvar_RegisterFloat(
                                        "cg_fovCompMax",
                                        85.0,
                                        1.0,
                                        160.0,
                                        0,
                                        "The maximum field of view to compensate for gun placement");
    cg_adsZoomToggleStyle = _Dvar_RegisterInt(
                                                        "cg_adsZoomToggleStyle",
                                                        1,
                                                        0,
                                                        1,
                                                        1u,
                                                        "Style of zoom toggle - 0=oscillate, 1=rotate");
    cg_viewVehicleInfluenceGunner = _Dvar_RegisterVec3(
                                                                        "cg_viewVehicleInfluenceGunner",
                                                                        (1.0),
                                                                        (1.0),
                                                                        (1.0),
                                                                        0.0,
                                                                        1.0,
                                                                        0x1080u,
                                                                        "The influence on the view from being a vehicle gunner");
    cg_viewVehicleInfluenceGunnerFiring = _Dvar_RegisterVec3(
                                                                                    "cg_viewVehicleInfluenceGunnerFiring",
                                                                                    (0.0),
                                                                                    (0.0),
                                                                                    (0.0),
                                                                                    0.0,
                                                                                    1.0,
                                                                                    0x1080u,
                                                                                    "The influence on the view from being a vehicle gunner while firing");
    cg_draw2D = _Dvar_RegisterBool("cg_draw2D", 1, 0x80u, "Draw 2D screen elements");
    cg_drawErrorMessages = _Dvar_RegisterBool("cg_drawErrorMessages", 1, 0, "Draw error/warning text");
    cg_drawHealth = _Dvar_RegisterBool("cg_drawHealth", 0, 0x80u, "Draw health bar");
    cg_drawBreathHint = _Dvar_RegisterBool("cg_drawBreathHint", 1, 1u, "Draw a 'hold breath to steady' hint");
    cg_drawMantleHint = _Dvar_RegisterBool("cg_drawMantleHint", 1, 1u, "Draw a 'press key to mantle' hint");
    cg_wadefps = _Dvar_RegisterBool(
                                 "cl_wadefps",
                                 0,
                                 0,
                                 "Toggles the display of the conspicuous FPS meter in non-development builds only.");
    cg_drawFPS = _Dvar_RegisterEnum("cg_drawFPS", cg_drawFpsNames, 1, 1u, "Draw frames per second");
    cg_drawFPSScale = _Dvar_RegisterFloat("cg_drawFPSScale", 0.0, 0.0, 100.0, 1u, "Draw FPS size scale");
    cg_drawBudgets = _Dvar_RegisterEnum("cg_drawBudgets", cg_drawBudgetNames, 1, 0, "Draw asset type budgets");
    cg_drawDynSModelBudget = _Dvar_RegisterBool("cg_drawDynSModelBudget", 1, 0, "Draw dynamic static model budget");
    cg_development = _Dvar_RegisterBool(
                                         "cg_development",
                                         1,
                                         0,
                                         "Indicates if we are in DEVELOPMENT (non-release ship builds)");
    cg_drawAnimAttachTags = _Dvar_RegisterBool("cg_drawAnimAttachTags", 0, 0x80u, "Display anim attach tags debug data");
    cg_drawFPSOnly = _Dvar_RegisterBool("cg_drawFPSOnly", 0, 1u, "Draw only the FPS stats in the upper right");
    cg_profile_physics = _Dvar_RegisterBool("profile_physics", 0, 1u, "Draw physics & collision profiltimers");
    cg_drawFPSLabels = _Dvar_RegisterBool("cg_drawFPSLabels", 1, 1u, "Draw FPS Info Labels");
    cg_debugInfoCornerOffset = _Dvar_RegisterVec2(
                                                             "cg_debugInfoCornerOffset",
                                                             (0.0),
                                                             (0.0),
                                                             -200.0,
                                                             640.0,
                                                             1u,
                                                             "Offset from top-right corner, for cg_drawFPS, etc");
    cg_drawVersion = _Dvar_RegisterBool("cg_drawVersion", 1, 0, "Draw the game version");
    cg_drawVersionX = _Dvar_RegisterFloat("cg_drawVersionX", 50.0, 0.0, 512.0, 0, "X offset for the version string");
    cg_drawVersionY = _Dvar_RegisterFloat("cg_drawVersionY", 18.0, 0.0, 512.0, 0, "Y offset for the version string");
    cg_readTitleStorageLocally = _Dvar_RegisterBool(
                                                                 "cg_readTitleStorageLocally",
                                                                 0,
                                                                 0,
                                                                 "Read title storage locally, instead of from the Xbox Live server");
    snd_drawInfo = _Dvar_RegisterEnum("snd_drawInfo", snd_drawInfoStrings, 0, 0, "Draw debugging information for sounds");
    snd_drawSort = _Dvar_RegisterEnum("snd_drawSort", snd_drawSortStrings, 0, 0, "Sort debugging information for sounds");
    cg_drawScriptUsage = _Dvar_RegisterBool("cg_drawScriptUsage", 0, 0, "Draw debugging information for scripts");
    cg_drawMaterial = _Dvar_RegisterEnum(
                                            "cg_drawMaterial",
                                            cg_drawMaterialNames,
                                            0,
                                            0x80u,
                                            "Draw debugging information for materials");
    cg_drawModelAxis = _Dvar_RegisterInt(
                                             "cg_drawModelAxis",
                                             -1,
                                             -1,
                                             256,
                                             0x80u,
                                             "Draw debugging axis for a bone of the model under the crosshair");
    cg_drawSnapshot = _Dvar_RegisterBool("cg_drawSnapshot", 0, 1u, "Draw debugging information for snapshots");
    cg_drawSnapshotTime = _Dvar_RegisterBool("cg_drawSnapshotTime", 1, 1u, "Draw length of snapshot buffer");
    cg_drawCrosshair = _Dvar_RegisterBool("cg_drawCrosshair", 1, 0x81u, "Turn on weapon crosshair");
    cg_drawCrosshair3D = _Dvar_RegisterBool("cg_drawCrosshair3D", 1, 0, "Turn on weapon crosshair in 3D mode.");
    cg_drawHoldBreathHint = _Dvar_RegisterBool(
                                                        "cg_drawHoldBreathHint",
                                                        1,
                                                        1u,
                                                        "Turn on hold breath hint string for the sniper rifles");
    cg_drawTurretCrosshair = _Dvar_RegisterBool("cg_drawTurretCrosshair", 1, 1u, "Draw a cross hair when using a turret");
    cg_drawCrosshairNames = _Dvar_RegisterBool(
                                                        "cg_drawCrosshairNames",
                                                        1,
                                                        0x81u,
                                                        "Draw the name of an enemy under the crosshair");
    cg_drawCrosshairNamesPosX = _Dvar_RegisterInt(
                                                                "cg_drawCrosshairNamesPosX",
                                                                300,
                                                                0,
                                                                640,
                                                                0,
                                                                "Virtual screen space position of the crosshair name");
    cg_drawCrosshairNamesPosY = _Dvar_RegisterInt(
                                                                "cg_drawCrosshairNamesPosY",
                                                                180,
                                                                0,
                                                                480,
                                                                0,
                                                                "Virtual screen space position of the crosshair name");
    cg_drawShellshock = _Dvar_RegisterBool("cg_drawShellshock", 1, 0x80u, "Draw shellshock & flashbang screen effects.");
    cg_hudStanceFlash = _Dvar_RegisterColor(
                                                "cg_hudStanceFlash",
                                                1.0,
                                                1.0,
                                                1.0,
                                                1.0,
                                                0,
                                                "The background color of the flash when the stance changes");
    cg_hudStanceHintPrints = _Dvar_RegisterBool(
                                                         "cg_hudStanceHintPrints",
                                                         0,
                                                         1u,
                                                         "Draw helpful text to say how to change stances");
    cg_hudDamageIconWidth = _Dvar_RegisterFloat(
                                                        "cg_hudDamageIconWidth",
                                                        128.0,
                                                        0.0,
                                                        512.0,
                                                        1u,
                                                        "The width of the damage icon");
    cg_hudDamageIconHeight = _Dvar_RegisterFloat(
                                                         "cg_hudDamageIconHeight",
                                                         64.0,
                                                         0.0,
                                                         512.0,
                                                         1u,
                                                         "The height of the damage icon");
    cg_hudDamageIconOffset = _Dvar_RegisterFloat(
                                                         "cg_hudDamageIconOffset",
                                                         128.0,
                                                         0.0,
                                                         512.0,
                                                         1u,
                                                         "The offset from the center of the damage icon");
    cg_hudDamageIconTime = _Dvar_RegisterInt(
                                                     "cg_hudDamageIconTime",
                                                     2000,
                                                     0,
                                                     0x7FFFFFFF,
                                                     1u,
                                                     "The amount of time for the damage icon to stay on screen after damage is taken");
    cg_hudDamageDirectionalIconTime = _Dvar_RegisterInt(
                                                                            "cg_hudDamageDirectionalIconTime",
                                                                            1000,
                                                                            1,
                                                                            0x7FFFFFFF,
                                                                            1u,
                                                                            "The amount of time for the damage icon to stay on screen after damage is taken");
    cg_hudDamageIconInScope = _Dvar_RegisterBool(
                                                            "cg_hudDamageIconInScope",
                                                            0,
                                                            1u,
                                                            "Draw damage icons when aiming down the sight of a scoped weapon");
    cg_hudGrenadeIconMaxRangeFrag = _Dvar_RegisterFloat(
                                                                        "cg_hudGrenadeIconMaxRangeFrag",
                                                                        250.0,
                                                                        0.0,
                                                                        1000.0,
                                                                        0x1080u,
                                                                        "The minimum distance that a grenade has to be from a player in order to be shown on "
                                                                        "the grenade indicator");
    cg_hudGrenadeIconMaxRangeFlash = _Dvar_RegisterFloat(
                                                                         "cg_hudGrenadeIconMaxRangeFlash",
                                                                         500.0,
                                                                         0.0,
                                                                         2000.0,
                                                                         0x1080u,
                                                                         "The minimum distance that a flashbang has to be from a player in order to be shown "
                                                                         "on the grenade indicator");
    cg_hudGrenadeIconMaxHeight = _Dvar_RegisterFloat(
                                                                 "cg_hudGrenadeIconMaxHeight",
                                                                 104.0,
                                                                 0.0,
                                                                 1000.0,
                                                                 1u,
                                                                 "The minimum height difference between a player and a grenade for the grenade to be show"
                                                                 "n on the grenade indicator");
    cg_hudGrenadeIconInScope = _Dvar_RegisterBool(
                                                             "cg_hudGrenadeIconInScope",
                                                             0,
                                                             1u,
                                                             "Show the grenade indicator when aiming down the sight of a scoped weapon");
    cg_hudGrenadeIconOffset = _Dvar_RegisterFloat(
                                                            "cg_hudGrenadeIconOffset",
                                                            50.0,
                                                            0.0,
                                                            512.0,
                                                            1u,
                                                            "The offset from the center of the screen for a grenade icon");
    cg_hudGrenadeIconHeight = _Dvar_RegisterFloat(
                                                            "cg_hudGrenadeIconHeight",
                                                            25.0,
                                                            0.0,
                                                            512.0,
                                                            1u,
                                                            "The height of the grenade indicator icon");
    cg_hudGrenadeIconWidth = _Dvar_RegisterFloat(
                                                         "cg_hudGrenadeIconWidth",
                                                         25.0,
                                                         0.0,
                                                         512.0,
                                                         1u,
                                                         "The width of the grenade indicator icon");
    cg_hudGrenadeIconEnabledFlash = _Dvar_RegisterBool(
                                                                        "cg_hudGrenadeIconEnabledFlash",
                                                                        0,
                                                                        1u,
                                                                        "Show the grenade indicator for flash grenades");
    cg_hudGrenadePointerHeight = _Dvar_RegisterFloat(
                                                                 "cg_hudGrenadePointerHeight",
                                                                 12.0,
                                                                 0.0,
                                                                 512.0,
                                                                 1u,
                                                                 "The height of the grenade indicator pointer");
    cg_hudGrenadePointerWidth = _Dvar_RegisterFloat(
                                                                "cg_hudGrenadePointerWidth",
                                                                25.0,
                                                                0.0,
                                                                512.0,
                                                                1u,
                                                                "The width of the grenade indicator pointer");
    cg_hudGrenadePointerPivot = _Dvar_RegisterVec2(
                                                                "cg_hudGrenadePointerPivot",
                                                                (12.0),
                                                                (27.0),
                                                                0.0,
                                                                512.0,
                                                                1u,
                                                                "The pivot point of th grenade indicator pointer");
    cg_hudGrenadePointerPulseFreq = _Dvar_RegisterFloat(
                                                                        "cg_hudGrenadePointerPulseFreq",
                                                                        1.7,
                                                                        0.1,
                                                                        50.0,
                                                                        0,
                                                                        "The number of times per second that the grenade indicator flashes in Hertz");
    cg_hudGrenadePointerPulseMax = _Dvar_RegisterFloat(
                                                                     "cg_hudGrenadePointerPulseMax",
                                                                     1.85,
                                                                     0.0,
                                                                     3.0,
                                                                     0,
                                                                     "The maximum alpha of the grenade indicator pulse. Values higher than 1 will cause the"
                                                                     " indicator to remain at full brightness for longer");
    cg_hudGrenadePointerPulseMin = _Dvar_RegisterFloat(
                                                                     "cg_hudGrenadePointerPulseMin",
                                                                     0.30000001,
                                                                     -3.0,
                                                                     1.0,
                                                                     0,
                                                                     "The minimum alpha of the grenade indicator pulse. Values lower than 0 will cause the "
                                                                     "indicator to remain at full transparency for longer");
    cg_hudChatPosition = _Dvar_RegisterVec2(
                                                 "cg_hudChatPosition",
                                                 (5.0),
                                                 (204.0),
                                                 0.0,
                                                 640.0,
                                                 1u,
                                                 "Position of the HUD chat box");
    cg_hudSayPosition = _Dvar_RegisterVec2(
                                                "cg_hudSayPosition",
                                                (5.0),
                                                (180.0),
                                                0.0,
                                                640.0,
                                                1u,
                                                "Position of the HUD say box");
    cg_hudChatIntermissionPosition = _Dvar_RegisterVec2(
                                                                         "cg_hudChatIntermissionPosition",
                                                                         (5.0),
                                                                         (90.0),
                                                                         0.0,
                                                                         640.0,
                                                                         1u,
                                                                         "Position of the HUD chat box during intermission");
    cg_hudVotePosition = _Dvar_RegisterVec2(
                                                 "cg_hudVotePosition",
                                                 (5.0),
                                                 (220.0),
                                                 0.0,
                                                 640.0,
                                                 1u,
                                                 "Position of the HUD vote box");
    cg_debugDrawSafeAreas = _Dvar_RegisterBool(
                                                        "cg_debugDrawSafeAreas",
                                                        0,
                                                        0,
                                                        "Show the safe area outlines for the safe areas on the UI");
    cg_drawLagometer = _Dvar_RegisterBool("drawLagometer", 0, 1u, "Enable the 'lagometer'");
    drawEntityCount = _Dvar_RegisterBool("drawEntityCount", 0, 0, "Enable entity count drawing");
    drawEntityCountPos = _Dvar_RegisterVec2(
                                                 "drawEntityCountPos",
                                                 (-55.0),
                                                 (-180.0),
                                                 -3.4028235e38,
                                                 3.4028235e38,
                                                 0,
                                                 "Where to draw the entity count graph");
    drawEntityCountSize = _Dvar_RegisterInt(
                                                    "drawEntityCountSize",
                                                    32,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "How big to draw the entity count graph");
    drawServerBandwidth = _Dvar_RegisterBool("drawServerBandwidth", 0, 0, "Enable drawing server bandwidth");
    drawServerBandwidthPos = _Dvar_RegisterVec2(
                                                         "drawServerBandwidthPos",
                                                         (-55.0),
                                                         (-280.0),
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0,
                                                         "Where to draw the server bandwidth graph");
    drawServerBandwidthSize = _Dvar_RegisterInt(
                                                            "drawEntityCountSize",
                                                            32,
                                                            0,
                                                            0x7FFFFFFF,
                                                            0,
                                                            "How big to draw the entity count graph");
    drawKillcamData = _Dvar_RegisterBool("drawKillcamData", 0, 0, "Enable drawing server killcam data");
    drawKillcamDataPos = _Dvar_RegisterVec2(
                                                 "drawKillcamDataPos",
                                                 (-55.0),
                                                 (-230.0),
                                                 -3.4028235e38,
                                                 3.4028235e38,
                                                 0,
                                                 "Where to draw the server killcam graph");
    drawKillcamDataSize = _Dvar_RegisterInt(
                                                    "drawKillcamDataSize",
                                                    32,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "How big to draw the killcam data graph");
    cg_hudProneY = _Dvar_RegisterFloat(
                                     "cg_hudProneY",
                                     -160.0,
                                     -10000.0,
                                     10000.0,
                                     1u,
                                     "Virtual screen y coordinate of the prone blocked message");
    cg_mapLocationSelectionCursorSpeed = _Dvar_RegisterFloat(
                                                                                 "cg_mapLocationSelectionCursorSpeed",
                                                                                 0.60000002,
                                                                                 0.001,
                                                                                 1.0,
                                                                                 1u,
                                                                                 "Speed of the cursor when selecting a location on the map");
    cg_mapLocationSelectionRotationSpeed = _Dvar_RegisterInt(
                                                                                     "cg_mapLocationSelectionRotationSpeed",
                                                                                     3,
                                                                                     1,
                                                                                     10,
                                                                                     1u,
                                                                                     "Rotation speed of the cursor when selecting a location on the map");
    cg_hudGrenadeIndicatorFadeUp = _Dvar_RegisterBool(
                                                                     "cg_hudGrenadeIndicatorFadeUp",
                                                                     0,
                                                                     0,
                                                                     "Draw grenade indicator with distance fade(COD3 style)");
    cg_hudGrenadeIndicatorTargetColor = _Dvar_RegisterVec4(
                                                                                "cg_hudGrenadeIndicatorTargetColor",
                                                                                (1.0),
                                                                                (1.0),
                                                                                (1.0),
                                                                                (1.0),
                                                                                0.0,
                                                                                1.0,
                                                                                0,
                                                                                "");
    cg_hudGrenadeIndicatorStartColor = _Dvar_RegisterVec4(
                                                                             "cg_hudGrenadeIndicatorStartColor",
                                                                             (1.0),
                                                                             (1.0),
                                                                             (1.0),
                                                                             (1.0),
                                                                             0.0,
                                                                             1.0,
                                                                             0,
                                                                             "");
    cg_weaponCycleDelay = _Dvar_RegisterInt(
                                                    "cg_weaponCycleDelay",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    1u,
                                                    "The delay after cycling to a new weapon to prevent holding down the cycle weapon button from cycling too fast");
    cg_crosshairAlpha = _Dvar_RegisterFloat("cg_crosshairAlpha", 1.0, 0.0, 1.0, 0x81u, "The alpha value of the crosshair");
    cg_crosshairAlphaMin = _Dvar_RegisterFloat(
                                                     "cg_crosshairAlphaMin",
                                                     0.5,
                                                     0.0,
                                                     1.0,
                                                     0x81u,
                                                     "The minimum alpha value of the crosshair when it fades in");
    cg_crosshairDynamic = _Dvar_RegisterBool("cg_crosshairDynamic", 0, 0x81u, "Crosshair is Dynamic");
    cg_crosshairEnemyColor = _Dvar_RegisterBool(
                                                         "cg_crosshairEnemyColor",
                                                         1,
                                                         0x81u,
                                                         "The crosshair color when over an enemy");
    cg_brass = _Dvar_RegisterBool("cg_brass", 1, 1u, "Weapons eject brass");
    cg_gun_fovcomp_x = _Dvar_RegisterFloat(
                                             "cg_gun_fovcomp_x",
                                             -2.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0,
                                             "x position FOV offset compensation of the viewmodel");
    cg_gun_fovcomp_y = _Dvar_RegisterFloat(
                                             "cg_gun_fovcomp_y",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0,
                                             "y position FOV offset compensation of the viewmodel");
    cg_gun_fovcomp_z = _Dvar_RegisterFloat(
                                             "cg_gun_fovcomp_z",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0,
                                             "z position FOV offset compensation of the viewmodel");
    cg_gun_x = _Dvar_RegisterFloat("cg_gun_x", 0.0, -3.4028235e38, 3.4028235e38, 0x80u, "x position of the viewmodel");
    cg_gun_y = _Dvar_RegisterFloat("cg_gun_y", 0.0, -3.4028235e38, 3.4028235e38, 0x80u, "y position of the viewmodel");
    cg_gun_z = _Dvar_RegisterFloat("cg_gun_z", 0.0, -3.4028235e38, 3.4028235e38, 0x80u, "z position of the viewmodel");
    cg_gun_move_f = _Dvar_RegisterFloat(
                                        "cg_gun_move_f",
                                        0.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        0x80u,
                                        "Weapon movement forward due to player movement");
    cg_gun_move_r = _Dvar_RegisterFloat(
                                        "cg_gun_move_r",
                                        0.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        0x80u,
                                        "Weapon movement right due to player movement");
    cg_gun_move_u = _Dvar_RegisterFloat(
                                        "cg_gun_move_u",
                                        0.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        0x80u,
                                        "Weapon movement up due to player movement");
    cg_gun_ofs_f = _Dvar_RegisterFloat(
                                     "cg_gun_ofs_f",
                                     0.0,
                                     -3.4028235e38,
                                     3.4028235e38,
                                     0x80u,
                                     "Forward weapon offset when prone/ducked");
    cg_gun_ofs_r = _Dvar_RegisterFloat(
                                     "cg_gun_ofs_r",
                                     0.0,
                                     -3.4028235e38,
                                     3.4028235e38,
                                     0x80u,
                                     "Right weapon offset when prone/ducked");
    cg_gun_ofs_u = _Dvar_RegisterFloat(
                                     "cg_gun_ofs_u",
                                     0.0,
                                     -3.4028235e38,
                                     3.4028235e38,
                                     0x80u,
                                     "Up weapon offset when prone/ducked");
    cg_gun_move_rate = _Dvar_RegisterFloat(
                                             "cg_gun_move_rate",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0x80u,
                                             "The base weapon movement rate");
    cg_gun_move_minspeed = _Dvar_RegisterFloat(
                                                     "cg_gun_move_minspeed",
                                                     0.0,
                                                     -3.4028235e38,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "The minimum weapon movement rate");
    cg_centertime = _Dvar_RegisterFloat(
                                        "cg_centertime",
                                        5.0,
                                        0.0,
                                        3.4028235e38,
                                        0x80u,
                                        "The time for a center printed message to fade");
    cg_debugPosition = _Dvar_RegisterBool("cg_debugposition", 0, 0x80u, "Output position debugging information");
    cg_debugEvents = _Dvar_RegisterBool("cg_debugevents", 0, 0x80u, "Output event debug information");
    cg_errorDecay = _Dvar_RegisterFloat("cg_errordecay", 100.0, 0.0, 3.4028235e38, 0, "Decay for predicted error");
    cg_nopredict = _Dvar_RegisterBool("cg_nopredict", 0, 0, "Don't do client side prediction");
    cg_showmiss = _Dvar_RegisterInt("cg_showmiss", 0, 0, 2, 0, "Show prediction errors");
    cg_footsteps = _Dvar_RegisterBool("cg_footsteps", 1, 0x80u, "Play footstep sounds");
    cg_footprints = _Dvar_RegisterBool("cg_footprints", 1, 0x80u, "Draw footprint decals and effects");
    cg_footprintsDistortWater = _Dvar_RegisterInt(
                                                                "cg_footprintsDistortWater",
                                                                0,
                                                                0,
                                                                1,
                                                                0x80u,
                                                                "Distort water on footprint (0 means no distortion)");
    cg_footprintsDebug = _Dvar_RegisterInt(
                                                 "cg_footprintsDebug",
                                                 0,
                                                 0,
                                                 1,
                                                 0x80u,
                                                 "Debug footprint drawing code (0 means no debugging)");
    cg_waterTrailRippleFrequency = _Dvar_RegisterInt(
                                                                     "cg_waterTrailRippleFrequency",
                                                                     400,
                                                                     0,
                                                                     0x7FFFFFFF,
                                                                     0,
                                                                     "How often (in ms) will play the waist ripple fx for actors in water");
    cg_waterTrailRippleVariance = _Dvar_RegisterInt(
                                                                    "cg_waterTrailRippleVariance",
                                                                    200,
                                                                    0,
                                                                    0x7FFFFFFF,
                                                                    0,
                                                                    "How late (in ms) the waist ripple fx can be played");
    cg_treadmarks = _Dvar_RegisterBool("cg_treadmarks", 0, 0x80u, "Draw treadmark decals and effects");
    cg_firstPersonTracerChance = _Dvar_RegisterFloat(
                                                                 "cg_firstPersonTracerChance",
                                                                 0.0,
                                                                 0.0,
                                                                 1.0,
                                                                 0x80u,
                                                                 "The probability that a bullet is a tracer round for your bullets");
    cg_laserForceOn = _Dvar_RegisterBool(
                                            "cg_laserForceOn",
                                            0,
                                            0x80u,
                                            "Force laser sights on in all possible places (for debug purposes).");
    cg_laserRange = _Dvar_RegisterFloat(
                                        "cg_laserRange",
                                        1500.0,
                                        1.0,
                                        3.4028235e38,
                                        0x80u,
                                        "The maximum range of a laser beam");
    cg_laserRangePlayer = _Dvar_RegisterFloat(
                                                    "cg_laserRangePlayer",
                                                    1500.0,
                                                    1.0,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "The maximum range of the player's laser beam");
    cg_laserRadius = _Dvar_RegisterFloat(
                                         "cg_laserRadius",
                                         0.80000001,
                                         0.001,
                                         3.4028235e38,
                                         0x80u,
                                         "The size (radius) of a laser beam");
    cg_laserLight = _Dvar_RegisterBool(
                                        "cg_laserLight",
                                        1,
                                        0,
                                        "Whether to draw the light emitted from a laser (not the laser itself)");
    cg_laserLightBodyTweak = _Dvar_RegisterFloat(
                                                         "cg_laserLightBodyTweak",
                                                         15.0,
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "Amount to add to length of beam for light when laser hits a body (for hitboxes).");
    cg_laserLightRadius = _Dvar_RegisterFloat(
                                                    "cg_laserLightRadius",
                                                    3.0,
                                                    0.001,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "The radius of the light at the far end of a laser beam");
    cg_laserLightBeginOffset = _Dvar_RegisterFloat(
                                                             "cg_laserLightBeginOffset",
                                                             13.0,
                                                             -3.4028235e38,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "How far from the true beginning of the beam the light at the beginning is.");
    cg_laserLightEndOffset = _Dvar_RegisterFloat(
                                                         "cg_laserLightEndOffset",
                                                         -3.0,
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "How far from the true end of the beam the light at the end is.");
    cg_laserEndOffset = _Dvar_RegisterFloat(
                                                "cg_laserEndOffset",
                                                0.5,
                                                -3.4028235e38,
                                                3.4028235e38,
                                                0x80u,
                                                "How far from the point of collision the end of the beam is.");
    cg_laserFlarePct = _Dvar_RegisterFloat(
                                             "cg_laserFlarePct",
                                             0.2,
                                             0.0,
                                             3.4028235e38,
                                             0x80u,
                                             "Percentage laser widens over distance from viewer.");
    cg_marks_ents_player_only = _Dvar_RegisterBool(
                                                                "cg_marks_ents_player_only",
                                                                0,
                                                                1u,
                                                                "Marks on entities from players' bullets only.");
    cg_tracerChance = _Dvar_RegisterFloat(
                                            "cg_tracerchance",
                                            0.2,
                                            0.0,
                                            1.0,
                                            0x80u,
                                            "The probability that a bullet is a tracer round");
    cg_tracerWidth = _Dvar_RegisterFloat("cg_tracerwidth", 3.0, 0.0, 3.4028235e38, 0x80u, "The width of the tracer round");
    cg_tracerSpeed = _Dvar_RegisterFloat(
                                         "cg_tracerSpeed",
                                         7500.0,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "The speed of a tracer round in units per second");
    cg_tracerLength = _Dvar_RegisterFloat(
                                            "cg_tracerlength",
                                            100.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "The length of a tracer round");
    cg_tracerNoDrawTime = _Dvar_RegisterInt(
                                                    "cg_tracerNoDrawTime",
                                                    0,
                                                    0,
                                                    1000,
                                                    0x80u,
                                                    "Delay in milliseconds before a tracer will start rendering");
    cg_tracerScale = _Dvar_RegisterFloat(
                                         "cg_tracerScale",
                                         1.0,
                                         1.0,
                                         3.4028235e38,
                                         0x80u,
                                         "Scale the tracer at a distance, so it's still visible");
    cg_tracerScaleMinDist = _Dvar_RegisterFloat(
                                                        "cg_tracerScaleMinDist",
                                                        5000.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "The minimum distance to scale a tracer");
    cg_tracerScaleDistRange = _Dvar_RegisterFloat(
                                                            "cg_tracerScaleDistRange",
                                                            25000.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "The range at which a tracer is scaled to its maximum amount");
    cg_tracerScrewDist = _Dvar_RegisterFloat(
                                                 "cg_tracerScrewDist",
                                                 100.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "The length a tracer goes as it completes a full corkscrew revolution");
    cg_tracerScrewRadius = _Dvar_RegisterFloat(
                                                     "cg_tracerScrewRadius",
                                                     0.15000001,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "The radius of a tracer's corkscrew motion");
    cg_bulletWidth = _Dvar_RegisterFloat(
                                         "cg_bulletwidth",
                                         2.0,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "The width of the non-tracer round");
    cg_bulletLength = _Dvar_RegisterFloat(
                                            "cg_bulletlength",
                                            80.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "The length of a non-tracer round");
    cg_thirdPersonRange = _Dvar_RegisterFloat(
                                                    "cg_thirdPersonRange",
                                                    120.0,
                                                    0.0,
                                                    1024.0,
                                                    1u,
                                                    "The range of the camera from the player in third person view");
    cg_thirdPersonAngle = _Dvar_RegisterFloat(
                                                    "cg_thirdPersonAngle",
                                                    0.0,
                                                    -180.0,
                                                    360.0,
                                                    1u,
                                                    "The angle of the camera from the player in third person view");
    cg_thirdPersonFocusDist = _Dvar_RegisterFloat(
                                                            "cg_thirdPersonFocusDist",
                                                            512.0,
                                                            0.0,
                                                            1024.0,
                                                            0x80u,
                                                            "The distance infront of the player to aim the 3rd person camera at");
    cg_thirdPerson = _Dvar_RegisterInt("cg_thirdPerson", 0, 0, 2, 0x80u, "Use third person view");
    cg_thirdPersonMode = _Dvar_RegisterEnum(
                                                 "cg_thirdPersonMode",
                                                 cg_thirdPersonModeNames,
                                                 1,
                                                 1u,
                                                 "How the camera behaves in third person");
    cg_chatTime = _Dvar_RegisterInt(
                                    "cg_chatTime",
                                    12000,
                                    0,
                                    60000,
                                    1u,
                                    "The amount of time that a chat message is visible");
    cg_chatHeight = _Dvar_RegisterInt("cg_chatHeight", 5, 0, 8, 1u, "The font height of a chat message");
    cg_predictItems = _Dvar_RegisterBool("cg_predictItems", 1, 3u, "Turn on client side prediction for item pickup");
    cg_spectateThirdPerson = _Dvar_RegisterBool(
                                                         "cg_spectateThirdPerson",
                                                         0,
                                                         0,
                                                         "Default player to thirdperson in spectate");
    cg_teamChatsOnly = _Dvar_RegisterBool("cg_teamChatsOnly", 0, 1u, "Allow chatting only on the same team");
    cg_use_colored_smoke = _Dvar_RegisterBool("cg_use_colored_smoke", 0, 0x80u, "Allow the use of colored smoke grenades");
    cg_fakefireWizbyChance = _Dvar_RegisterFloat(
                                                         "cg_fakefireWizbyChance",
                                                         0.2,
                                                         0.0,
                                                         1.0,
                                                         0x80u,
                                                         "The probability that a fake fire shot plays a wizby to local players round");
    cg_paused = _Dvar_RegisterInt("cl_paused", 0, 0, 1, 0x40u, "Pause the game");
    cg_drawpaused = _Dvar_RegisterBool("cg_drawpaused", 1, 0, "Draw paused screen");
    cg_synchronousClients = _Dvar_RegisterBool(
                                                        "g_synchronousClients",
                                                        0,
                                                        0x100u,
                                                        "Client is synchronized to the server - allows smooth demos");
    cg_debug_overlay_viewport = _Dvar_RegisterBool(
                                                                "cg_debug_overlay_viewport",
                                                                0,
                                                                0x80u,
                                                                "Remove the sniper overlay so you can check that the scissor window is correct.");
    cg_fs_debug = _Dvar_RegisterInt("fs_debug", 0, 0, 2, 0, "Output debugging information for the file system");
    cg_debugFace = _Dvar_RegisterBool("cg_debugFace", 0, 0x80u, "Turn on debug information for face");
    cg_dumpAnims = _Dvar_RegisterInt("cg_dumpAnims", -1, -1, 1535, 0x80u, "Output animation info for the given entity id");
#ifdef _DEBUG // LWSS ADD
    cg_developer = _Dvar_RegisterInt("developer", 1, 0, 2, 0, "Turn on Development systems");
#else
    cg_developer = _Dvar_RegisterInt("developer", 0, 0, 2, 0, "Turn on Development systems");
#endif
    cg_minicon = _Dvar_RegisterBool("con_minicon", 0, 1u, "Display the mini console on screen");
    cg_subtitles = _Dvar_RegisterBool("cg_subtitles", 1, 1u, "Show subtitles");
    cg_subtitleMinTime = _Dvar_RegisterFloat(
                                                 "cg_subtitleMinTime",
                                                 3.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 1u,
                                                 "The minimum time that the subtitles are displayed on screen in seconds");
    cg_subtitleWidthStandard = _Dvar_RegisterInt(
                                                             "cg_subtitleWidthStandard",
                                                             520,
                                                             130,
                                                             1664,
                                                             1u,
                                                             "The width of the subtitles in non wide-screen");
    cg_subtitleWidthWidescreen = _Dvar_RegisterInt(
                                                                 "cg_subtitleWidthWidescreen",
                                                                 520,
                                                                 130,
                                                                 1664,
                                                                 1u,
                                                                 "The width of the subtitles in wide-screen ");
    cg_gameMessageWidth = _Dvar_RegisterInt(
                                                    "cg_gameMessageWidth",
                                                    455,
                                                    130,
                                                    1664,
                                                    1u,
                                                    "The maximum character width of the game messages");
    cg_gameBoldMessageWidth = _Dvar_RegisterInt(
                                                            "cg_gameBoldMessageWidth",
                                                            390,
                                                            130,
                                                            1664,
                                                            1u,
                                                            "The maximum character width of the bold game messages");
    cg_descriptiveText = _Dvar_RegisterBool("cg_descriptiveText", 1, 1u, "Draw descriptive spectator messages");
    cg_youInKillCamSize = _Dvar_RegisterFloat(
                                                    "cg_youInKillCamSize",
                                                    11.0,
                                                    0.0,
                                                    100.0,
                                                    1u,
                                                    "Size of the 'you' Icon in the kill cam");
    cg_scriptIconSize = _Dvar_RegisterFloat("cg_scriptIconSize", 5.0, 0.0, 100.0, 1u, "Size of Icons defined by script");
    cg_connectionIconSize = _Dvar_RegisterFloat(
                                                        "cg_connectionIconSize",
                                                        5.0,
                                                        0.0,
                                                        100.0,
                                                        1u,
                                                        "Size of the connection icon");
    cg_voiceIconSize = _Dvar_RegisterFloat("cg_voiceIconSize", 0.0, 0.0, 100.0, 1u, "Size of the 'voice' icon");
    cg_constantSizeHeadIcons = _Dvar_RegisterBool(
                                                             "cg_constantSizeHeadIcons",
                                                             0,
                                                             0x80u,
                                                             "Head icons are the same size regardless of distance from the player");
    cg_headIconMinScreenRadius = _Dvar_RegisterFloat(
                                                                 "cg_headIconMinScreenRadius",
                                                                 0.02,
                                                                 0.0,
                                                                 1.0,
                                                                 1u,
                                                                 "The minumum radius of a head icon on the screen");
    cg_overheadNamesMaxDist = _Dvar_RegisterFloat(
                                                            "cg_overheadNamesMaxDist",
                                                            10000.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "The maximum distance for showing friendly player names");
    cg_overheadNamesNearDist = _Dvar_RegisterFloat(
                                                             "cg_overheadNamesNearDist",
                                                             64.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "The near distance at which names are full size");
    cg_overheadNamesFarDist = _Dvar_RegisterFloat(
                                                            "cg_overheadNamesFarDist",
                                                            512.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "The far distance at which name sizes are scaled by cg_overheadNamesFarScale");
    cg_overheadNamesFarScale = _Dvar_RegisterFloat(
                                                             "cg_overheadNamesFarScale",
                                                             0.69999999,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "The amount to scale overhead name sizes at cg_overheadNamesFarDist");
    cg_overheadNamesSize = _Dvar_RegisterFloat(
                                                     "cg_overheadNamesSize",
                                                     0.5,
                                                     0.0,
                                                     100.0,
                                                     1u,
                                                     "The maximum size to show overhead names");
    cg_overheadIconSize = _Dvar_RegisterFloat(
                                                    "cg_overheadIconSize",
                                                    0.69999999,
                                                    0.0,
                                                    100.0,
                                                    1u,
                                                    "The maximum size to show overhead icons like 'rank'");
    cg_overheadRankSize = _Dvar_RegisterFloat(
                                                    "cg_overheadRankSize",
                                                    0.5,
                                                    0.0,
                                                    3.4028235e38,
                                                    1u,
                                                    "The size to show rank text");
    cg_overheadNamesGlow = _Dvar_RegisterColor(
                                                     "cg_overheadNamesGlow",
                                                     0.0,
                                                     0.0,
                                                     0.0,
                                                     1.0,
                                                     0x80u,
                                                     "Glow color for overhead names");
    cg_overheadNamesFont = _Dvar_RegisterInt(
                                                     "cg_overheadNamesFont",
                                                     2,
                                                     0,
                                                     6,
                                                     1u,
                                                     "Font for overhead names ( see menudefinition.h )");
    cg_drawFriendlyNames = _Dvar_RegisterBool("cg_drawFriendlyNames", 1, 0x80u, "Whether to show friendly names in game");
    cg_enemyNameFadeIn = _Dvar_RegisterInt(
                                                 "cg_enemyNameFadeIn",
                                                 250,
                                                 0,
                                                 0x7FFFFFFF,
                                                 0x80u,
                                                 "Time in milliseconds to fade in enemy names");
    cg_friendlyNameFadeIn = _Dvar_RegisterInt(
                                                        "cg_friendlyNameFadeIn",
                                                        0,
                                                        0,
                                                        0x7FFFFFFF,
                                                        0x80u,
                                                        "Time in milliseconds to fade in friendly names");
    cg_enemyNameFadeOut = _Dvar_RegisterInt(
                                                    "cg_enemyNameFadeOut",
                                                    250,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0x80u,
                                                    "Time in milliseconds to fade out enemy names");
    cg_friendlyNameFadeOut = _Dvar_RegisterInt(
                                                         "cg_friendlyNameFadeOut",
                                                         1500,
                                                         0,
                                                         0x7FFFFFFF,
                                                         0x80u,
                                                         "Time in milliseconds to fade out friendly names");
    cg_drawThroughWalls = _Dvar_RegisterBool(
                                                    "cg_drawThroughWalls",
                                                    0,
                                                    0x80u,
                                                    "Whether to draw friendly names through walls or not");
    cg_playerHighlightTargetSize = _Dvar_RegisterFloat(
                                                                     "cg_playerHighlightTargetSize",
                                                                     750.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Size of player target highlights.");
    cg_playerHighlightEnemyColor = _Dvar_RegisterColor(
                                                                     "cg_playerHighlightEnemyColor",
                                                                     1.0,
                                                                     0.1,
                                                                     0.1,
                                                                     1.0,
                                                                     0x80u,
                                                                     "Color of enemy player highlights.");
    cg_playerHighlightBrightness = _Dvar_RegisterFloat(
                                                                     "cg_playerHighlightBrightness",
                                                                     3.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Brightness of highlights.");
    cg_playerHighlightMinFade = _Dvar_RegisterFloat(
                                                                "cg_playerHighlightMinFade",
                                                                0.40000001,
                                                                0.0,
                                                                1.0,
                                                                0x80u,
                                                                "The minimum fade for player highlight blinking.");
    cg_playerHighlightBlinkTime = _Dvar_RegisterInt(
                                                                    "cg_playerHighlightBlinkTime",
                                                                    1250,
                                                                    0,
                                                                    0x7FFFFFFF,
                                                                    0x80u,
                                                                    "The speed (in ms) at which the player highlights blink.");
    cg_corpseHighlightFadeTime = _Dvar_RegisterFloat(
                                                                 "cg_corpseHighlightFadeTime",
                                                                 2.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "Time (in seconds) that corpse highlights fade out");
    cg_cameraSpikeHighlightBrightness = _Dvar_RegisterFloat(
                                                                                "cg_cameraSpikeHighlightBrightness",
                                                                                5.0,
                                                                                0.0,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "Brightness of player highlights in the camera spike view");
    cg_cameraSpikeEnemyColor = _Dvar_RegisterColor(
                                                             "cg_cameraSpikeEnemyColor",
                                                             1.0,
                                                             0.0,
                                                             0.0,
                                                             1.0,
                                                             0x80u,
                                                             "Color of enemies in the camera spike view");
    cg_adsZScaleMax = _Dvar_RegisterFloat(
                                            "cg_adsZScaleMax",
                                            1.25,
                                            1.01,
                                            1.99,
                                            0x81u,
                                            "The scale factor for shrinky dinks");
    cg_infraredHighlightScale = _Dvar_RegisterFloat(
                                                                "cg_infraredHighlightScale",
                                                                200.0,
                                                                1.0,
                                                                1000.0,
                                                                0,
                                                                "Scale of the player highlight when using infrared scope");
    cg_infraredHighlightOffset = _Dvar_RegisterFloat(
                                                                 "cg_infraredHighlightOffset",
                                                                 0.75,
                                                                 0.0,
                                                                 1.0,
                                                                 0,
                                                                 "Offset to the player highlight when using infrared scope");
    cg_allow_mature = _Dvar_RegisterBool("cg_allow_mature", g_allowMature, 0x40u, "Controls Mature Content selectability");
    if ( g_allowMature )
        cg_mature = _Dvar_RegisterBool("cg_mature", 1, 1u, "Show Mature Content");
    else
        cg_mature = _Dvar_RegisterBool("cg_mature", 0, 0x40u, "Show Mature Content");
    cg_blood = _Dvar_RegisterBool("cg_blood", 1, 1u, "Show Blood");
    cg_invalidCmdHintDuration = _Dvar_RegisterInt(
                                                                "cg_invalidCmdHintDuration",
                                                                1800,
                                                                0,
                                                                0x7FFFFFFF,
                                                                1u,
                                                                "Duration of an invalid command hint");
    cg_invalidCmdHintBlinkInterval = _Dvar_RegisterInt(
                                                                         "cg_invalidCmdHintBlinkInterval",
                                                                         600,
                                                                         1,
                                                                         0x7FFFFFFF,
                                                                         1u,
                                                                         "Blink rate of an invalid command hint");
    cg_viewZSmoothingMin = _Dvar_RegisterFloat(
                                                     "cg_viewZSmoothingMin",
                                                     1.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "Threshhold for the minimum smoothing distance it must move to smooth");
    cg_viewZSmoothingMax = _Dvar_RegisterFloat(
                                                     "cg_viewZSmoothingMax",
                                                     16.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "Threshhold for the maximum smoothing distance we'll do");
    cg_viewZSmoothingTime = _Dvar_RegisterFloat(
                                                        "cg_viewZSmoothingTime",
                                                        0.1,
                                                        0.0,
                                                        3.4028235e38,
                                                        1u,
                                                        "Amount of time to spread the smoothing over");
    overrideNVGModelWithKnife = _Dvar_RegisterBool(
                                                                "overrideNVGModelWithKnife",
                                                                0,
                                                                0x1000u,
                                                                "When true, nightvision animations will attach the weapDef's knife model instead of the n"
                                                                "ight vision goggles.");
    cg_visionSetLerpMaxIncreasePerFrame = _Dvar_RegisterFloat(
                                                                                    "cg_visionSetLerpMaxIncreasePerFrame",
                                                                                    0.02,
                                                                                    0.0,
                                                                                    1.0,
                                                                                    0x81u,
                                                                                    "Maximum jump of customlerp between 2 frames, used for smoothing for flare visionset");
    cg_visionSetLerpMaxDecreasePerFrame = _Dvar_RegisterFloat(
                                                                                    "cg_visionSetLerpMaxDecreasePerFrame",
                                                                                    0.0099999998,
                                                                                    0.0,
                                                                                    1.0,
                                                                                    0x81u,
                                                                                    "Maximum jump of customlerp between 2 frames, used for smoothing for flare visionset");
    cg_flareVisionSetFadeDuration = _Dvar_RegisterInt(
                                                                        "cg_flareVisionSetFadeDuration",
                                                                        2000,
                                                                        0,
                                                                        0x7FFFFFFF,
                                                                        0x81u,
                                                                        "Duration of fade back to normal vision set when you look away from the flare");
    cg_turretBipodOffset = _Dvar_RegisterFloat(
                                                     "cg_turretBipodOffset",
                                                     17.0,
                                                     -50.0,
                                                     50.0,
                                                     0x80u,
                                                     "Offset bipod mount position on gun by this distance");
    cg_AllPlayerNamesVisible = _Dvar_RegisterBool(
                                                             "cg_allPlayerNamesVisible",
                                                             0,
                                                             0x80u,
                                                             "When true all names are visible within visibility range.");
    cg_ScoresColor_MyTeam = _Dvar_RegisterColor(
                                                        "g_ScoresColor_MyTeam",
                                                        0.25,
                                                        0.72000003,
                                                        0.25,
                                                        1.0,
                                                        0x100u,
                                                        "Player team color on scoreboard");
    cg_ScoresColor_EnemyTeam = _Dvar_RegisterColor(
                                                             "g_ScoresColor_EnemyTeam",
                                                             0.69,
                                                             0.07,
                                                             0.050000001,
                                                             1.0,
                                                             0x100u,
                                                             "Enemy team color on scoreboard");
    cg_ScoresColor_Spectator = _Dvar_RegisterColor(
                                                             "g_ScoresColor_Spectator",
                                                             0.25,
                                                             0.25,
                                                             0.25,
                                                             1.0,
                                                             0x100u,
                                                             "Spectator team color on scoreboard");
    cg_ScoresColor_Free = _Dvar_RegisterColor(
                                                    "g_ScoresColor_Free",
                                                    0.75999999,
                                                    0.77999997,
                                                    0.1,
                                                    1.0,
                                                    0x100u,
                                                    "Free Team color on scoreboard");
    cg_ScoresColor_Allies = _Dvar_RegisterColor(
                                                        "g_ScoresColor_Allies",
                                                        0.090000004,
                                                        0.46000001,
                                                        0.07,
                                                        1.0,
                                                        0x100u,
                                                        "Allies team color on scoreboard");
    cg_ScoresColor_Axis = _Dvar_RegisterColor(
                                                    "g_ScoresColor_Axis",
                                                    0.69,
                                                    0.07,
                                                    0.050000001,
                                                    1.0,
                                                    0x100u,
                                                    "Axis team color on scoreboard");
    cg_TeamName_Allies = _Dvar_RegisterString("g_TeamName_Allies", "GAME_ALLIES", 0x100u, "Allied team name");
    cg_TeamName_Axis = _Dvar_RegisterString("g_TeamName_Axis", "GAME_AXIS", 0x100u, "Axis team name");
    cg_TeamColor_Allies = _Dvar_RegisterColor(
                                                    "g_TeamColor_Allies",
                                                    0.60000002,
                                                    0.63999999,
                                                    0.69,
                                                    1.0,
                                                    0x100u,
                                                    "Allies team color");
    cg_TeamColor_Axis = _Dvar_RegisterColor(
                                                "g_TeamColor_Axis",
                                                0.64999998,
                                                0.56999999,
                                                0.41,
                                                1.0,
                                                0x100u,
                                                "Axis team color");
    cg_TeamColor_MyTeam = _Dvar_RegisterColor(
                                                    "g_TeamColor_MyTeam",
                                                    0.40000001,
                                                    0.69999999,
                                                    0.40000001,
                                                    1.0,
                                                    0x100u,
                                                    "Player team color");
    cg_TeamColor_EnemyTeam = _Dvar_RegisterColor(
                                                         "g_TeamColor_EnemyTeam",
                                                         1.0,
                                                         0.315,
                                                         0.34999999,
                                                         1.0,
                                                         0x100u,
                                                         "Enemy team color");
    cg_TeamColor_MyTeamAlt = _Dvar_RegisterColor(
                                                         "g_TeamColor_MyTeamAlt",
                                                         0.34999999,
                                                         1.0,
                                                         1.0,
                                                         1.0,
                                                         0x100u,
                                                         "Player team color");
    cg_TeamColor_EnemyTeamAlt = _Dvar_RegisterColor(
                                                                "g_TeamColor_EnemyTeamAlt",
                                                                1.0,
                                                                0.5,
                                                                0.0,
                                                                1.0,
                                                                0x100u,
                                                                "Enemy team color");
    cg_TeamColor_Squad = _Dvar_RegisterColor("g_TeamColor_Squad", 0.25, 0.25, 0.75, 1.0, 0x100u, "Squad color");
    cg_TeamColor_Spectator = _Dvar_RegisterColor(
                                                         "g_TeamColor_Spectator",
                                                         0.25,
                                                         0.25,
                                                         0.25,
                                                         1.0,
                                                         0x100u,
                                                         "Spectator team color");
    cg_TeamColor_Free = _Dvar_RegisterColor("g_TeamColor_Free", 0.75, 0.25, 0.25, 1.0, 0x100u, "Free Team color");
    cg_proneFeetCollisionHull = _Dvar_RegisterBool(
                                                                "cg_proneFeetCollisionHull",
                                                                1,
                                                                0x80u,
                                                                "Enables the use of the extra physics collision hulls on the feet while prone.");
    CG_ViewRegisterDvars();
#ifdef KISAK_SP
    // Retail SP helper 0x005e6430, called by CG_RegisterDvars at 0x004a6b4b.
    // The frontend Zombies action tests ui_sp_unlock before opening its next
    // menu; without this registration UI_DvarValueTest rejects the action.
    _Dvar_RegisterInt("mis_01", 0, 0, 50, 1u, "");
    _Dvar_RegisterString("mis_difficulty", "0000000000000000000000000", 1u, "");
    _Dvar_RegisterBool("mis_cheat", 0, 0, "");
    _Dvar_RegisterInt("mis_01_unlock", 21, 0, 50, 1u, "");
    _Dvar_RegisterInt("ui_sp_unlock", 0, 0, 1, 1u, "");
#endif
    DynEntCl_RegisterDvars();
    CG_OffhandRegisterDvars();
    CG_CompassRegisterDvars();
    CG_SpikeAcousticRegisterDvars();
    CG_AmmoCounterRegisterDvars();
    CG_RegisterVisionSetsDvars();
    CG_RegisterScoreboardDvars();
    CG_HudElemRegisterDvars();
    GC_InitWeaponOptionsDvars();
    Turret_PlaceTurret_RegisterDvars();
    g_compassShowEnemies = _Dvar_RegisterBool(
                                                     "g_compassShowEnemies",
                                                     0,
                                                     0x80u,
                                                     "Whether enemies are visible on the compass at all times");
    BG_RegisterDvars();
    cg_drawWVisDebug = _Dvar_RegisterBool("cg_drawWVisDebug", 0, 0, "Display weapon visibility debug info");
    debugOverlay = _Dvar_RegisterEnum(
                                     "debugOverlay",
                                     debugOverlayNames,
                                     0,
                                     0,
                                     "Toggles the display of various debug info.");
    cg_motionblur_duration = _Dvar_RegisterInt(
                                                         "cg_motionblur_duration",
                                                         2500,
                                                         1,
                                                         10000,
                                                         0x81u,
                                                         "Sets radial motion blur duration");
    cg_motionblur_fadeout = _Dvar_RegisterInt(
                                                        "cg_motionblur_fadeout",
                                                        500,
                                                        1,
                                                        5000,
                                                        0x81u,
                                                        "Sets fade time for radial motion blur");
    cg_timedDamageDuration = _Dvar_RegisterInt(
                                                         "cg_timedDamageDuration",
                                                         500,
                                                         1,
                                                         5000,
                                                         0x81u,
                                                         "Sets the time to display a damage friendly indicator");
    cg_MinDownedPulseRate = _Dvar_RegisterFloat(
                                                        "cg_MinDownedPulseRate",
                                                        1.0,
                                                        1.0,
                                                        10.0,
                                                        0x81u,
                                                        "The amount of alpha to fade per second, at minimum for downed allies");
    cg_MaxDownedPulseRate = _Dvar_RegisterFloat(
                                                        "cg_MaxDownedPulseRate",
                                                        4.0,
                                                        1.0,
                                                        10.0,
                                                        0x81u,
                                                        "The amount of alpha to fade per second, at most for downed allies");
    cg_playerFrustumHalfHeight = _Dvar_RegisterFloat(
                                                                 "cg_playerFrustumHalfHeight",
                                                                 36.0,
                                                                 0.0,
                                                                 128.0,
                                                                 0x80u,
                                                                 "The radius used to calculate frustum target center for a player. Used for fast \"is on screen\" tests");
    cg_overheadNamesTagUpdateInterval = _Dvar_RegisterInt(
                                                                                "cg_overheadNamesTagUpdateInterval",
                                                                                1,
                                                                                1,
                                                                                5000,
                                                                                0x80u,
                                                                                "How often the friendly visibility head tag is updated for the on screen frustum check");
    cg_canSeeFriendlyFrustumUpdateInterval = _Dvar_RegisterInt(
                                                                                         "cg_canSeeFriendlyFrustumUpdateInterval",
                                                                                         250,
                                                                                         1,
                                                                                         5000,
                                                                                         0x80u,
                                                                                         "How often the head tag is updated for the overhead names");
    cg_canSeeFriendlyFrustumExpand = _Dvar_RegisterFloat(
                                                                         "cg_canSeeFriendlyFrustumExpand",
                                                                         -30.0,
                                                                         -1000.0,
                                                                         1000.0,
                                                                         0x80u,
                                                                         "The frustum expansion to determine if a friendly is on screen.    Positive is inwards.");
    cg_canSeeFriendlyFrustumMinDistance = _Dvar_RegisterFloat(
                                                                                    "cg_canSeeFriendlyFrustumMinDistance",
                                                                                    100.0,
                                                                                    0.0,
                                                                                    10000.0,
                                                                                    0x80u,
                                                                                    "If target is inside this distance frustum culling is not applied.");
    cg_watersheeting = _Dvar_RegisterBool(
                                             "cg_watersheeting",
                                             1,
                                             0x80u,
                                             "Enables/disables the watersheeting fullscreen effect");
    cg_debug_triggers = _Dvar_RegisterBool(
                                                "cg_debug_triggers",
                                                0,
                                                0x4000u,
                                                "Debug client side Triggers, prints out all the client triggers the first time they are hit.");
    cg_cameraWaterClip = _Dvar_RegisterFloat(
                                                 "cg_cameraWaterClip",
                                                 4.5,
                                                 -100.0,
                                                 100.0,
                                                 0x1080u,
                                                 "Min distance between camera and water surface. To prevent camera seeing water edge-on. Set to -1 to disable");
#ifdef KISAK_SP
    // Retail SP registers this here, immediately after cg_cameraWaterClip and
    // immediately before cg_forceSniperBobHack: 0x004a6d5d-0x004a6d73 pushes
    // "cg_cameraUseTagCamera" (0x00a0ccc8), default 1, flags 0x1000,
    // description "" (0x009dd354), and calls Dvar_RegisterBool (0x0045bb20);
    // the handle is stored to the global at 0x02ff66f4, which is the same
    // global CG_UpdateCameraMode reads at 0x00623cbb. Gates the tag-camera
    // view override -- see CG_ComputeUseTagCamera in cg_camera.cpp.
    cg_cameraUseTagCamera = _Dvar_RegisterBool("cg_cameraUseTagCamera", 1, 0x1000u, "");
#endif
    cg_cameraVehicleExitTweenTime = _Dvar_RegisterFloat(
                                                                        "cg_cameraVehicleExitTweenTime",
                                                                        0.40000001,
                                                                        0.0,
                                                                        5.0,
                                                                        0x1000u,
                                                                        "Time(secs) to tween from gunner/vehicle camera to normal player camera");
    cg_vehicle_piece_damagesfx_threshold = _Dvar_RegisterFloat(
                                                                                     "vehicle_piece_damagesfx_threshold",
                                                                                     10.0,
                                                                                     0.0,
                                                                                     3.4028235e38,
                                                                                     0x80u,
                                                                                     "Minimum amount of damage for which a destructible piece damageSound SFX will be played.");
    cg_debugLocHit = _Dvar_RegisterInt(
                                         "g_debugLocHit",
                                         0,
                                         0,
                                         2,
                                         0x80u,
                                         "Display locational damage info for an entity when the entity is hit");
    cg_debugLocHitTime = _Dvar_RegisterInt(
                                                 "g_debugLocHitTime",
                                                 500,
                                                 0,
                                                 0x7FFFFFFF,
                                                 0x80u,
                                                 "Time duration of g_debugLocHit lines");
#ifdef KISAK_SP
    // Retail SP CG_RegisterDvars registers this saved HUD group together. These
    // dvars are consumed by server-side SetSavedDvar calls in the SP scripts.
    _Dvar_RegisterBool("hud_showTextNoAmmo", 1, 0x1000u, "");
    _Dvar_RegisterBool("hud_showObjectives", 1, 0x1000u, "");
    _Dvar_RegisterBool("hud_showStance", 1, 0x1000u, "");
    _Dvar_RegisterBool("hud_drawHUD", 1, 0x1000u, "");
    _Dvar_RegisterBool("hud_missionFailed", 0, 0x1000u, "");
#endif
    CG_SetupSplitscreenDvars();
}

int __cdecl CG_AllocateClientMemory_SizeRequired(int maxLocalClients)
{
    int localClientNum; // [esp+0h] [ebp-8h]
    int size; // [esp+4h] [ebp-4h]

    // nx-port: were x86 literals (907268 and 1062080); the same terms as
    // CG_AllocateClientMemory, from the structs (the second cgs_t was in the
    // original total), plus 128 for cgArray's alignment.
    size = (int)((sizeof(cg_s) + 2 * sizeof(cgs_t) + 512 * sizeof(fake_centity_s) + sizeof(ViewModelInfo)) * maxLocalClients + 128);
    for ( localClientNum = 0; localClientNum < maxLocalClients; ++localClientNum )
        size += (int)(2048 * sizeof(weaponInfo_s) + 1024 * sizeof(centity_s) + 18432 + 144 * sizeof(Destructible) + 32 * sizeof(IKState));
    return size;
}

void __cdecl CG_AllocateClientMemory(HunkUser *hunk, int maxLocalClients)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    cgArray = (cg_s *)Hunk_UserAlloc(hunk, sizeof(cg_s) * maxLocalClients, 128, "cgArray");
    cgsArray = (cgs_t *)Hunk_UserAlloc(hunk, sizeof(cgs_t) * maxLocalClients, 8, "cgsArray");
    cg_fakeEntitiesArray = (fake_centity_s *)Hunk_UserAlloc(hunk, sizeof(fake_centity_s) * 512 * maxLocalClients, 4, "cg_fakeEntitiesArray");
    cg_viewModelArray = (ViewModelInfo *)Hunk_UserAlloc(hunk, sizeof(ViewModelInfo) * maxLocalClients, 4, "cg_viewModelArray");
    for ( localClientNum = 0; localClientNum < maxLocalClients; ++localClientNum )
    {
        cg_weaponsArray[localClientNum] = (weaponInfo_s *)Hunk_UserAlloc(hunk, 2048 * sizeof(weaponInfo_s), 8, "cg_weaponsArray");
        cg_entitiesArray[localClientNum] = (centity_s *)Hunk_UserAlloc(hunk, 1024 * sizeof(centity_s), 8, "cg_entitiesArray");
        cg_entityOriginArray[localClientNum] = (float (*)[3])Hunk_UserAlloc(hunk, 18432, 4, "cg_entityOriginArray");
        cg_destructibles[localClientNum] = (Destructible *)Hunk_UserAlloc(hunk, 144 * sizeof(Destructible), 8, "cg_destructibles");
        cg_ikBuf[localClientNum] = (unsigned __int8 *)Hunk_UserAlloc(hunk, 32 * sizeof(IKState), 16, "ikStatesArray");   // nx-port: was 117760, 32 x86 IKStates
        memset(cg_ikBuf[localClientNum], 0, 32 * sizeof(IKState));
        IK_AllocateLocalClientMemory(cg_ikBuf[localClientNum], localClientNum);
    }
}

void __cdecl CG_FreeClientMemory(HunkUser *hunk, int maxLocalClients)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = maxLocalClients - 1; localClientNum >= 0; --localClientNum )
    {
        Hunk_UserFree(hunk, cg_ikBuf[localClientNum]);
        IK_AllocateLocalClientMemory(0, localClientNum);
        Hunk_UserFree(hunk, cg_destructibles[localClientNum]);
        Hunk_UserFree(hunk, cg_entityOriginArray[localClientNum]);
        Hunk_UserFree(hunk, cg_entitiesArray[localClientNum]);
        Hunk_UserFree(hunk, cg_weaponsArray[localClientNum]);
    }
    if ( hunk )
    {
        if ( (!cg_viewModelArray || !cg_fakeEntitiesArray || !cgsArray || !cgArray)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                        1226,
                        0,
                        "%s",
                        "cg_viewModelArray && cg_fakeEntitiesArray && cgsArray && cgArray") )
        {
            __debugbreak();
        }
        Hunk_UserFree(hunk, cg_viewModelArray);
        Hunk_UserFree(hunk, cg_fakeEntitiesArray);
        Hunk_UserFree(hunk, cgsArray);
        Hunk_UserFree(hunk, cgArray);
    }
    cgArray = 0;
    cgsArray = 0;
    cg_weaponsArray[0] = 0;
    cg_viewModelArray = 0;
    cg_entitiesArray[0] = 0;
    cg_entityOriginArray[0] = 0;
    cg_destructibles[0] = 0;
}

void __cdecl CG_GetDObjOrientation(int localClientNum, int dobjHandle, float (*axis)[3], float *origin)
{
    const cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    const centity_s *cent; // [esp+14h] [ebp-4h]

    if ( (unsigned int)dobjHandle > 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    1254,
                    0,
                    "%s\n\t(dobjHandle) = %i",
                    "(dobjHandle >= 0 && dobjHandle < ((((1<<10) + 512)) + 1))",
                    dobjHandle) )
    {
        __debugbreak();
    }
    if ( !axis && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1255, 0, "%s", "axis") )
        __debugbreak();
    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1256, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    if ( dobjHandle >= 1536 )
    {
        if ( dobjHandle - 1536 >= 2048
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                        1265,
                        0,
                        "%s\n\t(dobjHandle) = %i",
                        "(dobjHandle >= (((1<<10) + 512)) && dobjHandle - (((1<<10) + 512)) < ( 1 << 11 ))",
                        dobjHandle) )
        {
            __debugbreak();
        }
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        AxisCopy(cgameGlob->viewModelAxis, axis);
        *origin = cgameGlob->viewModelAxis[3][0];
        origin[1] = cgameGlob->viewModelAxis[3][1];
        origin[2] = cgameGlob->viewModelAxis[3][2];
    }
    else
    {
        cent = CG_GetEntity(localClientNum, dobjHandle);
        AnglesToAxis(cent->pose.angles, axis);
        *origin = cent->pose.origin[0];
        origin[1] = cent->pose.origin[1];
        origin[2] = cent->pose.origin[2];
    }
}

const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum)
{
    return &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState;
}

void __cdecl CG_GameMessage(int localClientNum, const char *msg)
{
    CL_ConsolePrint(localClientNum, 2, msg, 0, cg_gameMessageWidth->current.integer, 0);
}

void __cdecl CG_BoldGameMessage(int localClientNum, const char *msg, int duration)
{
    CL_ConsolePrint(localClientNum, 3, msg, duration, cg_gameBoldMessageWidth->current.integer, 0);
}

bool __cdecl CG_IsVehicleMayhemGameType()
{
    const char *String; // eax
    const char *v1; // eax
    bool v3; // [esp+0h] [ebp-4h]

    String = Dvar_GetString("g_gametype");
    v3 = 1;
    if ( I_strcmp(String, "vdm") )
    {
        v1 = Dvar_GetString("g_gametype");
        if ( I_strcmp(v1, "vtdm") )
            return 0;
    }
    return v3;
}

void __cdecl CG_RegisterSounds()
{
    int type; // [esp+0h] [ebp-8Ch]
    char name[132]; // [esp+4h] [ebp-88h] BYREF

    CG_RegisterSurfaceTypeSounds("wpn_grenade_explode", cgMedia.grenadeExplodeSound);
    CG_RegisterSurfaceTypeSounds("wpn_rifle_grenade", cgMedia.rifleGrenadeSound);
    CG_RegisterSurfaceTypeSounds("wpn_rocket_explode", cgMedia.rocketExplodeSound);
    CG_RegisterSurfaceTypeSounds("wpn_rocket_explode_xtreme", cgMedia.rocketExplodeXtremeSound);
    CG_RegisterSurfaceTypeSounds("wpn_tank_shell", cgMedia.tankShellExplodeSound);
    CG_RegisterSurfaceTypeSounds("wpn_mortar_shell", cgMedia.mortarShellExplodeSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_small", cgMedia.bulletHitSmallSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_large", cgMedia.bulletHitLargeSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_ap", cgMedia.bulletHitAPSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_xtreme", cgMedia.bulletHitXTremeSound);
    CG_RegisterSurfaceTypeSounds("prj_bulletspray_small", cgMedia.shotgunHitSound);
    CG_RegisterSurfaceTypeSounds("prj_bolt_impact", cgMedia.boltHitSound);
    CG_RegisterSurfaceTypeSounds("prj_blade_impact", cgMedia.bladeHitSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_small_exit", cgMedia.bulletExitSmallSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_large_exit", cgMedia.bulletExitLargeSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_ap_exit", cgMedia.bulletExitAPSound);
    CG_RegisterSurfaceTypeSounds("prj_bullet_impact_xtreme_exit", cgMedia.bulletExitXTremeSound);
    CG_RegisterSurfaceTypeSounds("prj_bulletspray_impact_small_exit", cgMedia.shotgunExitSound);
    CG_RegisterSurfaceTypeSounds("prj_bolt_impact_exit", cgMedia.boltExitSound);
    CG_RegisterImpactTypeSounds("prj_impact_veh_armor", cgMedia.weaponImpactsTankArmorSound);
    CG_RegisterImpactTypeSounds("prj_impact_veh_locomotion", cgMedia.weaponImpactsTankTreadSound);
    cgMedia.mantleSound = SND_FindAliasId("chr_launch_exert_npc");
    cgMedia.mantleSoundPlayer = SND_FindAliasId("chr_launch_exert_plr");
    cgMedia.dtpLaunchSoundPlayer = SND_FindAliasId("fly_dtp_launch_plr");
    cgMedia.dtpLaunchSound = SND_FindAliasId("fly_dtp_launch_npc");
    cgMedia.dtpCollideSoundPlayer = SND_FindAliasId("fly_dtp_collide_plr");
    cgMedia.dtpCollideSound = SND_FindAliasId("fly_dtp_collide_npc");
    for ( type = 0; type < 9; ++type )
    {
        sprintf(name, "fly_dtp_land_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpLandSoundPlayer[type] = SND_FindAliasId(name);
        sprintf(name, "fly_dtp_land_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpLandSound[type] = SND_FindAliasId(name);
        sprintf(cgMedia.dtpSlideLoopSoundPlayer[type], "fly_dtp_slide_loop_plr_%s", Dtp_SurfaceTypeNames[type]);
        sprintf(cgMedia.dtpSlideLoopSound[type], "fly_dtp_slide_loop_npc_%s", Dtp_SurfaceTypeNames[type]);
        sprintf(name, "fly_dtp_slide_stop_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpSlideStopSoundPlayer[type] = SND_FindAliasId(name);
        sprintf(name, "fly_dtp_slide_stop_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.dtpSlideStopSound[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_loop_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStart_1p[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_loop_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStart_3p[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_stop_plr_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStop_1p[type] = SND_FindAliasId(name);
        sprintf(name, "fly_pslide_stop_npc_%s", Dtp_SurfaceTypeNames[type]);
        cgMedia.playerSlidingStop_3p[type] = SND_FindAliasId(name);
    }
    cgMedia.playerSprintGasp = SND_FindAliasId("fly_sprint_gasp");
    cgMedia.underwaterWhizby = SND_FindAliasId("prj_whizbyuw");
    cgMedia.bulletWhizby = SND_FindAliasId("prj_whizby");
    cgMedia.bulletCrack = SND_FindAliasId("prj_crack");
    cgMedia.deathGurgle = SND_FindAliasId("fly_death_gurgle");
    cgMedia.meleeHit = SND_FindAliasId("wpn_melee_hit");
    cgMedia.meleeHitOther = SND_FindAliasId("wpn_melee_hit_other");
    cgMedia.meleeKnifeHit = SND_FindAliasId("wpn_melee_knife_hit_body");
    cgMedia.meleeKnifeHitOther = SND_FindAliasId("wpn_melee_knife_hit_other");
    cgMedia.meleeDogHit = SND_FindAliasId("chr_melee_dog_hit");
    cgMedia.meleeDogHitOther = SND_FindAliasId("chr_melee_dog_hit_other");
    cgMedia.nightVisionOn = SND_FindAliasId("fly_nightvision_on");
    cgMedia.nightVisionOff = SND_FindAliasId("fly_nightvision_off");
    cgMedia.playerHeartBeatSound = SND_FindAliasId("wpn_sniper_heartbeat");
    cgMedia.playerBreathInSound = SND_FindAliasId("wpn_sniper_breathin");
    cgMedia.playerBreathOutSound = SND_FindAliasId("wpn_sniper_breathout");
    cgMedia.playerBreathGaspSound = SND_FindAliasId("wpn_sniper_breathgasp");
    cgMedia.playerSwapOffhand = SND_FindAliasId("wpn_offhand_select");
}

void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, unsigned int *sound)
{
    const char *v2; // eax
    int i; // [esp+0h] [ebp-10Ch]
    int ia; // [esp+0h] [ebp-10Ch]
    char szAliasName[260]; // [esp+4h] [ebp-108h] BYREF

    if ( !pszType
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1340, 0, "%s", "pszType") )
    {
        __debugbreak();
    }
    if ( *pszType )
    {
        for ( ia = 0; ia < 31; ++ia )
        {
            v2 = Com_SurfaceTypeToName(ia);
            Com_sprintf(szAliasName, 0x100u, "%s_%s", pszType, v2);
            sound[ia] = SND_FindAliasId(szAliasName);
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: no alias prefix defined\n");
        for ( i = 0; i < 31; ++i )
            sound[i] = 0;
    }
}

void __cdecl CG_RegisterImpactTypeSounds(const char *pszType, unsigned int *sound)
{
    const char *ImpactTypeName; // eax
    int i; // [esp+0h] [ebp-10Ch]
    int ia; // [esp+0h] [ebp-10Ch]
    char szAliasName[260]; // [esp+4h] [ebp-108h] BYREF

    if ( !pszType
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 1369, 0, "%s", "pszType") )
    {
        __debugbreak();
    }
    if ( *pszType )
    {
        for ( ia = 0; ia < 16; ++ia )
        {
            ImpactTypeName = BG_GetImpactTypeName(ia);
            Com_sprintf(szAliasName, 0x100u, "%s_%s", pszType, ImpactTypeName);
            sound[ia] = SND_FindAliasId(szAliasName);
        }
    }
    else
    {
        Com_PrintError(9, "ERROR: no alias prefix defined for %s\n", "CG_RegisterImpactTypeSounds");
        for ( i = 0; i < 16; ++i )
            sound[i] = 0;
    }
}

void __cdecl CG_PlayBattleChatter(
                int localClientNum,
                int entitynum,
                float *origin,
                unsigned int firstSoundAlias,
                int secondSoundAlias)
{
    float *SndOrigin; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int SndID; // [esp+10h] [ebp-4h]

    SndID = CG_PlaySoundWithHandle(localClientNum, entitynum, origin, 0, 0, 1.0, firstSoundAlias);
    if (secondSoundAlias)
    {
        for (i = 0; i < 8; ++i)
        {
            if (!cg_BattleChatters[i].WhichSoundIsPlaying)
            {
                cg_BattleChatters[i].CurrentPlayingSound = SndID;
                cg_BattleChatters[i].EntityNum = entitynum;
                cg_BattleChatters[i].WhichSoundIsPlaying = 1;
                cg_BattleChatters[i].LocalClientNum = localClientNum;
                cg_BattleChatters[i].SecondAlias = secondSoundAlias;
                SndOrigin = cg_BattleChatters[i].SndOrigin;
                *SndOrigin = *origin;
                SndOrigin[1] = origin[1];
                SndOrigin[2] = origin[2];
                return;
            }
        }
    }
}

void __cdecl CG_CheckBattleChatter()
{
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 8; ++i)
    {
        if (cg_BattleChatters[i].WhichSoundIsPlaying)
        {
            if (cg_BattleChatters[i].WhichSoundIsPlaying != 1 || SND_IsPlaying(cg_BattleChatters[i].CurrentPlayingSound))
            {
                if (!SND_IsPlaying(cg_BattleChatters[i].CurrentPlayingSound))
                    cg_BattleChatters[i].WhichSoundIsPlaying = 0;
            }
            else
            {
                cg_BattleChatters[i].CurrentPlayingSound = CG_PlaySoundWithHandle(
                    cg_BattleChatters[i].LocalClientNum,
                    cg_BattleChatters[i].EntityNum,
                    cg_BattleChatters[i].SndOrigin,
                    0,
                    0,
                    1.0,
                    cg_BattleChatters[i].SecondAlias);
                cg_BattleChatters[i].WhichSoundIsPlaying = 2;
            }
        }
    }
}

void __cdecl CG_RestartSmokeGrenades(int localClientNum)
{
    int j; // [esp+18h] [ebp-44h]
    int eventIndex; // [esp+1Ch] [ebp-40h]
    const cg_s *cgameGlob; // [esp+20h] [ebp-3Ch]
    const snapshot_s *nextSnap; // [esp+24h] [ebp-38h]
    const WeaponDef *weaponDef; // [esp+28h] [ebp-34h]
    const entityState_s *v6; // [esp+2Ch] [ebp-30h]
    const cgs_t *cgs; // [esp+30h] [ebp-2Ch]
    int i; // [esp+34h] [ebp-28h]
    int ia; // [esp+34h] [ebp-28h]
    float axis[3][3]; // [esp+38h] [ebp-24h] BYREF

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    Com_DPrintf(14, "Playing smoke grenades at time %i\n", cgameGlob->time);
    for ( i = 0; i < 27; ++i )
    {
        if ( !cgs->grenadeFx[i] )
            return;
        FX_KillEffectDef(localClientNum, cgs->grenadeFx[i]);
    }
    FX_RewindTo(localClientNum, cgameGlob->time);
    nextSnap = cgameGlob->nextSnap;
    if ( nextSnap )
    {
        for ( ia = 0; ia < nextSnap->numEntities; ++ia )
        {
            v6 = &nextSnap->entities[ia];
            weaponDef = BG_GetWeaponDef(nextSnap->entities[ia].weapon);
            if ( (nextSnap->entities[ia].lerp.eFlags & 0x4000) != 0
                && nextSnap->entities[ia].time2 >= cgameGlob->time
                && nextSnap->entities[ia].lerp.u.actor.actorNum <= cgameGlob->time
                && !nextSnap->entities[ia].eType )
            {
                eventIndex = ((unsigned __int8)nextSnap->entities[ia].eventSequence - 1) & 3;
                if ( v6->events[eventIndex] != 66
                    && v6->events[eventIndex] != 67
                    && (v6->events[eventIndex] < 0x39u || v6->events[eventIndex] > 0x3Eu)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                1910,
                                0,
                                "es->events[eventIndex] not in [EV_GRENADE_EXPLODE, EV_CUSTOM_EXPLODE_NOMARKS]\n\t%i not in [%i, %i]",
                                v6->events[eventIndex],
                                57,
                                62) )
                {
                    __debugbreak();
                }
                ByteToDir(v6->eventParms[eventIndex], axis[0]);
                Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                Com_DPrintf(
                    14,
                    "Restarting smoke grenade at time %i at ( %f, %f, %f )\n",
                    nextSnap->entities[ia].lerp.u.actor.actorNum,
                    nextSnap->entities[ia].lerp.pos.trBase[0],
                    nextSnap->entities[ia].lerp.pos.trBase[1],
                    nextSnap->entities[ia].lerp.pos.trBase[2]);
                for ( j = 0; j < 27; ++j )
                {
                    if ( weaponDef->projExplosionEffect == cgs->grenadeFx[j] )
                        FX_PlayOrientedEffect(
                            localClientNum,
                            cgs->grenadeFx[j],
                            nextSnap->entities[ia].lerp.u.actor.actorNum,
                            nextSnap->entities[ia].lerp.pos.trBase,
                            axis);
                }
            }
        }
    }
}

void __cdecl CG_InitVote(int localClientNum)
{
    char *ConfigString; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    cgs_t *cgs; // [esp+0h] [ebp-Ch]
    int time; // [esp+4h] [ebp-8h] BYREF
    int serverId; // [esp+8h] [ebp-4h] BYREF

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgs->voteTime = 0;
    ConfigString = CL_GetConfigString(0xFu);
    if ( sscanf(ConfigString, "%d %d", &time, &serverId) == 2 && serverId == cls.serverId )
        cgs->voteTime = time;
    v2 = CL_GetConfigString(0x11u);
    cgs->voteYes = atoi(v2);
    v3 = CL_GetConfigString(0x12u);
    cgs->voteNo = atoi(v3);
    v4 = CL_GetConfigString(0x10u);
    v5 = SEH_LocalizeTextMessage(v4, "vote string", LOCMSG_SAFE);
    I_strncpyz(cgs->voteString, v5, 256);
}

unsigned __int16 __cdecl CG_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType, weapInventoryType_t invType)
{
    if ( weapType == WEAPTYPE_GRENADE )
    {
        if ( invType != WEAPINVENTORY_ITEM )
            return scr_const.tag_inhand;
    }
    else if ( ci->leftHandGun )
    {
        return SL_FindString(bg_weaponleftbone->current.string, SCRIPTINSTANCE_SERVER);
    }
    return SL_FindString(bg_weaponrightbone->current.string, SCRIPTINSTANCE_SERVER);
}

// Client-side mirror of the GSCR_* script-path macros in g_scr_main_mp.cpp. Same zone evidence:
// SP's compiled GSC lives under bare "maps/", the "maps/mp/..." spellings appear in no zone SP
// loads (maps/frontend.gsc = 7926 B in frontend.ff; maps/_destructible = 1793 B in
// code_post_gfx.ff). Unlike the server side these misses are NON-FATAL - CGScr_LoadScriptAndLabel
// only Com_Printf's "Could not find script '%s'" (cg_main_mp.cpp:2384) - so this is not a boot
// blocker; without it the frontend's client-side script simply never runs.
// Audit finding 5e (frontend-map-load audit).
// TODO(SP): the sibling "clientscripts/mp/" prefix (CGScr_LoadClientScripts and the inst!=0
// branches here) is NOT changed - neither audit examined SP's clientscripts/ layout, and no
// evidence was gathered either way. Settle it by scanning the SP zones for
// "clientscripts/frontend" vs "clientscripts/mp/frontend" before touching it.
#ifdef KISAK_SP
#define CGSCR_SERVER_SCRIPT_DIR "maps/"
#else
#define CGSCR_SERVER_SCRIPT_DIR "maps/mp/"
#endif

void __cdecl CGScr_LoadGameTypeScript(scriptInstance_t inst, const char *gametype, ScriptFunctions *functions)
{
    char filename[68]; // [esp+4h] [ebp-48h] BYREF

#ifdef KISAK_SP
    if (inst == SCRIPTINSTANCE_SERVER)
    {
        // Match GScr_LoadGameTypeScript's compiler roots without storing its
        // server callback handles. Loading the file compiles every function.
        CGScr_LoadScriptAndLabel(inst, "maps/_callbacksetup", "CodeCallback_StartGameType", functions);
        if (ui_gametype)
        {
            Com_sprintf(filename, sizeof(filename), "maps/gametypes/%s", ui_gametype->current.string);
            CGScr_LoadScriptAndLabel(inst, filename, "init", functions);
        }
        return;
    }
#endif
    if ( inst )
        Com_sprintf(filename, 64, "%sgametypes/%s", "clientscripts/mp/", gametype);
    else
        Com_sprintf(filename, 64, "%sgametypes/%s", CGSCR_SERVER_SCRIPT_DIR, gametype);
    CGScr_LoadScriptAndLabel(inst, filename, "main", functions);
}

char __cdecl CGScr_LoadScriptAndLabel(
                scriptInstance_t inst,
                const char *filename,
                const char *label,
                ScriptFunctions *functions)
{
    int func; // [esp+4h] [ebp-4h]

    if ( !cg_loadScripts || !cg_loadScripts->current.enabled )
        return 0;
    if ( functions->count < functions->maxSize )
    {
        if ( Scr_LoadScript(inst, (char*)filename) )
        {
            func = Scr_GetFunctionHandle(inst, filename, label);
            functions->address[functions->count++] = func;
            if ( func )
            {
                return 1;
            }
            else
            {
                Com_Printf(15, "Could not find label '%s' in script '%s'\n", label, filename);
                return 0;
            }
        }
        else
        {
            functions->address[functions->count++] = 0;
            Com_Printf(15, "Could not find script '%s'\n", filename);
            return 0;
        }
    }
    else
    {
        Com_PrintError(15, "CODE ERROR: GScr_LoadScriptAndLabel: functions->maxSize exceeded\n");
        return 0;
    }
}

#ifdef KISAK_SP
static void CGScr_LoadActorAnimScripts_SP(ScriptFunctions *functions)
{
    // Retail remote LOAD helpers 0088f800 (zombie) / 0088f450 (human).
    // Compile the same roots as the server without writing server script handles.
    static const char *const human[] = {
        "combat", "concealment_crouch", "concealment_prone", "concealment_stand",
        "cover_arrival", "cover_crouch", "cover_left", "cover_pillar", "cover_prone",
        "cover_right", "cover_stand", "cover_wide_left", "cover_wide_right", "death",
        "grenade_return_throw", "init", "pain", "react", "move", "scripted", "stop",
        "grenade_cower", "flashed"
    };
    static const char *const zombie[] = {
        "zombie_combat", "zombie_death", "zombie_init", "zombie_pain", "zombie_move",
        "zombie_scripted", "zombie_stop"
    };
    static const char *const zombieDog[] = {
        "zombie_dog_combat", "zombie_dog_death", "zombie_dog_init", "zombie_dog_pain",
        "zombie_dog_move", "zombie_dog_scripted", "zombie_dog_stop", "zombie_dog_flashed", "zombie_dog_turn"
    };
    const auto load = [functions](const char *name) {
        char filename[64];
        Com_sprintf(filename, sizeof(filename), "animscripts/%s", name);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, filename, "main", functions);
    };
    if (zombiemode->current.enabled)
    {
        for (const char *name : zombie) load(name);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "animscripts/zombie_scripted", "init", functions);
        for (const char *name : zombieDog) load(name);
    }
    else
    {
        for (const char *name : human) load(name);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "animscripts/scripted", "init", functions);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "animscripts/init_mode_sp", "init", functions);
    }
}

static void CGScr_LoadEntityAnimScripts_SP(ScriptFunctions *functions)
{
    // Remote equivalent of GScr_LoadScriptsAndAnimsForEntities: no AITypeScript
    // registry or server-global handles, just the same compiler roots.
    SpawnVar spawnVar;
    std::set<std::string> seen;
    std::set<std::string> seenTraversal;
    bool dogsLoaded = false;
    G_ResetEntityParsePoint();
    if (!G_ParseSpawnVars(&spawnVar)) Com_Error(ERR_DROP, "CGScr_LoadEntityAnimScripts_SP: no entities");
    while (G_ParseSpawnVars(&spawnVar))
    {
        const char *classname;
        if (!G_SpawnString(&spawnVar, "classname", "", &classname)) continue;
        // Retail005efbb0,005efd08-005efdaa: raw entity negotiation branch.
        if (!I_stricmp(classname, "node_negotiation_begin"))
        {
            const char *animscript;
            if (G_SpawnString(&spawnVar, "animscript", "", &animscript) && *animscript
                && seenTraversal.insert(animscript).second)
            {
                char filename[64];
                Com_sprintf(filename, sizeof(filename), "animscripts/traverse/%s", animscript);
                CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, filename, "main", functions);
                Com_Printf(16, "SP remote path animation root: '%s'\n", filename);
            }
            continue;
        }
        if (I_strnicmp(classname, "actor_", 6)) continue;
        if (!seen.insert(classname+6).second) continue;
        if (!zombiemode->current.enabled && !dogsLoaded && (strstr(classname, "dog") || strstr(classname, "hound")))
        {
            static const char *const dog[] = { "dog_combat", "dog_death", "dog_init", "dog_pain", "dog_move", "dog_scripted", "dog_stop", "dog_flashed" };
            for (const char *name : dog)
            {
                char filename[64];
                Com_sprintf(filename, sizeof(filename), "animscripts/%s", name);
                CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, filename, "main", functions);
            }
            dogsLoaded = true;
        }
        char filename[64];
        Com_sprintf(filename, sizeof(filename), "aitype/%s", classname+6);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, filename, "main", functions);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, filename, "precache", functions);
        CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, filename, "spawner", functions);
    }
    G_ResetEntityParsePoint();
}

#endif

void __cdecl CGScr_LoadScripts(const char *mapname, const char *gametype, ScriptFunctions *functions)
{
    Scr_BeginLoadScripts(SCRIPTINSTANCE_SERVER, 0);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/delete", "main", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/struct", "initstructs", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/struct", "createstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_SERVER, "codescripts/struct", "findstruct", functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_SERVER,
        CGSCR_SERVER_SCRIPT_DIR "_destructible",
        "CodeCallback_DestructibleEvent",
        functions);
    // NOTE(SP): maps/mp/gametypes/_spawning is deliberately left as-is even though it ships in no
    // SP zone. Unlike the server-side twin this call is non-fatal (Com_Printf only), and skipping
    // it would shift every later functions->address[] index by one - CScr_SetUniqueClientScripts
    // reads that array positionally. Costs one "Could not find script" log line.
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_SERVER,
        "maps/mp/gametypes/_spawning",
        "CodeCallback_UpdateSpawnPoints",
        functions);
    CGScr_LoadGameTypeScript(SCRIPTINSTANCE_SERVER, gametype, functions);
#ifdef KISAK_SP
    CGScr_LoadActorAnimScripts_SP(functions);
#endif
    CGScr_LoadLevelScript(SCRIPTINSTANCE_SERVER, mapname, functions);
#ifdef KISAK_SP
    CGScr_LoadEntityAnimScripts_SP(functions);
#endif
    G_ResetEntityParsePoint();
    Scr_PostCompileScripts(SCRIPTINSTANCE_SERVER);
    Scr_EndLoadScripts(SCRIPTINSTANCE_SERVER);
    Scr_PrecacheAnimTrees(SCRIPTINSTANCE_SERVER, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1);
}

void __cdecl CGScr_LoadLevelScript(scriptInstance_t inst, const char *mapname, ScriptFunctions *functions)
{
    char filename[68]; // [esp+4h] [ebp-48h] BYREF

    if ( inst )
#ifdef KISAK_SP
        // SP's client level script lives at clientscripts/<mapname>, with no "mp/" component.
        // Fact 1 (producer): retail SP's CGScr_LoadClientScripts (0x006555a0) builds this path at
        //   0x0065579a-0x006557b3 as Com_sprintf(buf, 0x40, "%s%s", "clientscripts/", mapname),
        //   pushing the literal at 0x00a2ca98 ("clientscripts/") -- NOT 0x00a39480
        //   ("clientscripts/mp/_callbacks", the only "clientscripts/mp/" text in the binary).
        // Fact 2 (corpus): the shipped SP zones contain clientscripts/frontend.csc; there is no
        //   clientscripts/mp/ directory in them at all.
        // Note the CONSUMER half, CScr_SetLevelScript (cg_scr_main.cpp:10111), already builds
        // "clientscripts/<mapname>" in BOTH configs -- it only uses the string for an error
        // message, so MP has always been internally inconsistent here. Producer is ground truth.
        Com_sprintf(filename, 64, "%s%s", "clientscripts/", mapname);
#else
        Com_sprintf(filename, 64, "%s%s", "clientscripts/mp/", mapname);
#endif
    else
        Com_sprintf(filename, 64, "%s%s", CGSCR_SERVER_SCRIPT_DIR, mapname);
    CGScr_LoadScriptAndLabel(inst, filename, "main", functions);
}

#ifdef KISAK_SP
// ===========================================================================
// SP client-script load list, transcribed from retail BlackOps.exe.
//
// PRODUCER: CGScr_LoadScriptAndLabel calls in CGScr_LoadClientScripts
// (Ghidra 0x006555a0). The name is now live in the database and its plate
// cites THIS transcription as its corroborating evidence, so the two agree.
// That function passes `filename` in EDI and
// `functions` in ESI (a compiler regparm optimisation -- the decompiler shows
// only 2 stack args), so the filenames are recoverable ONLY from the
// disassembly's `MOV EDI, <addr>` hoists, not from the decompile. Every string
// address below was read out of the image and is listed in the row comments.
//
// CONSUMER: FUN_00408ee0 (0x00408ee0), SP's CScr_SetUniqueClientScripts --
// still FUN_* in Ghidra as of 2026-08-26, so that name remains this tree's own
// hypothesis and not an attested symbol, unlike the producer above. Its
// 30 rows land 1:1 on the 30 producer rows here, in the same order, writing
// cg_scr_data at 0x00c207d0 with exactly the field layout cscr_data_t already
// declares (delete_@0 ... gibEvent@0x64). That positional agreement is the
// second, independent fact behind this list.
//
// *** DO NOT "CORRECT" THE clientscripts/mp/ STRINGS IN THE CONSUMER. ***
// SP's consumer still passes "clientscripts/mp/_callbacks" (0x00a39480) for
// six rows -- playerspawned, the four CodeCallback_Player* rows and
// glass_smash -- while the PRODUCER for those same six rows has EDI =
// "clientscripts/_callbacks" (0x00a49b8c). CScr_SetScriptAndLabel only ever
// uses filename/label to format an error message, so this is a cosmetic
// retail bug: SP genuinely loads from clientscripts/_callbacks and merely
// misnames it if the label is missing. Reproduced verbatim in
// CScr_SetUniqueClientScripts so the two halves keep matching retail.
//
// DIFFERENCES FROM MP worth naming: SP has no client-side gametype script and
// no maps/mp/gametypes/_spawning row; _dogs (playDogstep/soundNotify),
// CodeCallback_CreatingCorpse, demo_jump and demo_player_switch are absent;
// and SP adds scriptmodelspawned, _footsteps::playAIFootstep,
// callback_(de)activate_exploder, sound_notify, zombie_eye_callback,
// CodeCallback_PlayWeapon{Death,Damage}Effects and CodeCallback_GibEvent.
// 31 rows total (30 + level script) against MP's 27.
//
// _destructible IS SINGULAR HERE ON PURPOSE. The literal at 0x009dd6dc is
// byte-for-byte "clientscripts/_destructible\0" (read directly, not via
// Ghidra's string table). The shipped SP zones contain _destructibleS.csc
// (plural), so retail SP loads a file that does not exist -- harmless,
// because CGScr_LoadScriptAndLabel only Com_Printf's and the consumer row is
// bEnforceExists=0. Transcribed as retail has it, not as the corpus has it.
// ===========================================================================
static void __cdecl CGScr_LoadClientScripts_SP(const char *mapname, ScriptFunctions *functions)
{
    Scr_BeginLoadScripts(SCRIPTINSTANCE_CLIENT, 0);

    // NOTE(SP, RESOLVED+PORTED 2026-08-26; demoted from an open marker by a triage pass that
    // confirmed the Scr_FindAnim call is live at line 2566 below):
    // retail calls Scr_FindAnim (Ghidra 0x004bd6f0) here, at 0x006555dc -- BEFORE any
    // script load. IDENTITY RESOLVED 2026-08-26 (was "not identified; something face/anim
    // related"): 0x004bd6f0 is named Scr_FindAnim in the live Ghidra database (tags
    // openblops-source-match + machine-proposed) and its body is this tree's own Scr_FindAnim
    // (cscr_animtree.cpp:2977) statement for statement -- SL_GetLowercaseString_(animName,0,4,
    // inst), Scr_UsingTreeInternal, Scr_EmitAnimationInternal, SL_RemoveRefToString, same order.
    // Read off the raw listing at 0x006555a6-0x006555dc the five arguments are
    //     Scr_FindAnim(SCRIPTINSTANCE_CLIENT, "generic_human", "body", <anim>, <user>)
    // with both literals read out of the image (0x009f4d50 == "generic_human", 0x00a336b4 ==
    // "body"), <anim> = bgs->animData + 0x8d38c and <user> = bgs[0x10], where bgs is the client
    // BgsGlobals fetched from TLS ([FS:0x2c][DAT_03956508*4] + 0x1c). That is the CLIENT-side
    // counterpart of BG_FindAnims (bg_animation.cpp:4221), which makes the same call three times
    // with ("multiplayer", "main"/"torso"/"legs") on the server instance -- an animtree lookup,
    // not a face registration. So "generic_human" is the animtree FILENAME here and "body" the
    // anim NAME, matching MP's (filename, animName) argument roles.
    // DONE 2026-08-26: ported below, and the two RAW OFFSETS the note above carried are now
    // resolved to real fields rather than transcribed as numbers. Both close exactly against the
    // headers, which is what makes them safe to write:
    //   * animScriptData_t is sizeof=0x8D388 (bg_animation.h:236) and is bgsAnim_s's FIRST member
    //     (bg_animation.h:270), so generic_human starts at 0x8D388. Its members are
    //     tree/body/main/torso/legs, 4 bytes each -> body @ 0x8D38C, main @ 0x8D390,
    //     torso @ 0x8D394, legs @ 0x8D398; then generic_dog (0x8) and done_notify (0x4) land the
    //     end at 0x8D3A8, which IS the declared sizeof(bgsAnim_s). The retail argument
    //     `animData + 0x8d38c` is therefore &animData->generic_human.body, exactly.
    //   * `bgs[0x10]` is bgs_t +0x10, which bg_local.h:906 declares as `int anim_user` (animData,
    //     time, latestSnapshotTime, frametime, anim_user at 0/4/8/0xC/0x10).
    // The 16-bit tail at 0x006557E7-0x00655805 resolves the same way and is ported at the end of
    // this function: 0x8D370 is animScriptData_t::bodyAnim (counting back from the struct end --
    // 4 trailing pad, playSoundAlias @0x8D380, soundAlias @0x8D37C, 2 pad, turningAnim @0x8D378,
    // legsAnim @0x8D376, torsoAnim @0x8D374, mainAnim @0x8D372, bodyAnim @0x8D370), and the
    // 16-bit read at 0x8D38C is scr_anim_s::index (cscr_animtree.h:13, the low half of the
    // union). That makes the tail `animScriptData.bodyAnim = generic_human.body.index`, the exact
    // structural sibling of MP's BG_InitAnimTree lines at bg_animation.cpp:3392-3394
    // (`animScriptData.mainAnim = generic_human.main.index`, ditto torso/legs). It is ported with
    // the call because it reads back the handle this call writes; note that nothing in this tree
    // currently READS bodyAnim, so the tail is inert here and is fidelity work only.
    Scr_FindAnim(
        SCRIPTINSTANCE_CLIENT,
        "generic_human",
        "body",
        &bgs->animData->generic_human.body,
        bgs->anim_user);

    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/delete", "main", functions);          // EDI=0x009e6adc
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "initstructs", functions);   // EDI=0x009d8548
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "createstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "findstruct", functions);
    // --- EDI = 0x00a49b8c "clientscripts/_callbacks" (set at 0x00655626) -------------------
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "statechange", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "maprestart", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "localclientconnect", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "localclientdisconnect", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "entityspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "playerspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "scriptmodelspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "client_flag_callback", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "client_flagasval_callback", functions);
    // --- EDI = 0x009dd6dc "clientscripts/_destructible" (set at 0x0065569d) ----------------
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/_destructible",
        "CodeCallback_DestructibleEvent",
        functions);
    // --- EDI back to "clientscripts/_callbacks" (set at 0x006556ae) ------------------------
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "CodeCallback_PlayerFootstep", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "CodeCallback_PlayerJump", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "CodeCallback_PlayerLand", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "CodeCallback_PlayerFoliage", functions);
    // --- EDI = 0x009ae898 "clientscripts/_footsteps" (set at 0x006556e3) -------------------
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_footsteps", "playAIFootstep", functions);
    // --- EDI back to "clientscripts/_callbacks" (set at 0x006556f4) ------------------------
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "callback_activate_exploder", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "callback_deactivate_exploder", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "level_notify", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "sound_notify", functions);
    // --- 0x00655725-0x00655745: EDI = "clientscripts/_zombietron" (0x009dab44) when the
    //     `zombietron` dvar is set, else "clientscripts/_zombiemode" (0x009cc4d4).
    // TODO(SP): `zombietron` is registered in Com_InitDvars at 0x0082bcf0 with default 0 and
    // flags 0x40, so the DEFAULT path is _zombiemode; that is what is hardcoded here. Wire the
    // real branch up if/when a `zombietron` dvar is added to this reconstruction (it is not
    // registered anywhere in this tree today). Non-fatal either way: consumer row is
    // bEnforceExists=0 and neither .csc ships in the campaign zones.
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_zombiemode", "zombie_eye_callback", functions);
    // --- EDI back to "clientscripts/_callbacks" (set at 0x00655754) ------------------------
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/_callbacks",
        "CodeCallback_PlayWeaponDeathEffects",
        functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/_callbacks",
        "CodeCallback_PlayWeaponDamageEffects",
        functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "airsupport", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "entityshutdown_callback", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "glass_smash", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/_callbacks", "CodeCallback_GibEvent", functions);
    // --- clientscripts/<mapname>, "main" (0x0065579a-0x006557c6) ---------------------------
    CGScr_LoadLevelScript(SCRIPTINSTANCE_CLIENT, mapname, functions);

    // 0x006557cb `PUSH 1; CALL 0x00651a30` sits exactly where MP's
    // Scr_PostCompileScripts(SCRIPTINSTANCE_CLIENT) is -- and 0x00651a30 is this binary's
    // shared do-nothing function (it is also the registered handler for openfile/closefile/
    // fprintln/freadln/fgetarg/playrumble*), so retail SP's call there is a no-op. The
    // reconstruction's Scr_PostCompileScripts is real and needed here, so it is kept.
    Scr_PostCompileScripts(SCRIPTINSTANCE_CLIENT);
    CScr_PostLoadScripts();                                                   // 0x006557d2
    Scr_PrecacheAnimTrees(SCRIPTINSTANCE_CLIENT, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1); // 0x006557e2
    // Retail's tail (0x006557E7-0x00655805): re-reads the TLS bgs, then
    //     MOV CX,  word ptr [EAX + 0x8d38c]      ; generic_human.body.index
    //     MOV word ptr [EAX + 0x8d370], CX       ; animScriptData.bodyAnim
    // BOTH fields are identified now -- see the derivation on the Scr_FindAnim call at the head
    // of this function. This is MP's BG_InitAnimTree pattern (bg_animation.cpp:3392-3394) with
    // "body" in place of main/torso/legs.
    bgs->animData->animScriptData.bodyAnim = bgs->animData->generic_human.body.index;
}
#endif // KISAK_SP

void __cdecl CGScr_LoadClientScripts(const char *mapname, ScriptFunctions *functions)
{
#ifdef KISAK_SP
    CGScr_LoadClientScripts_SP(mapname, functions);
    return;
#else
    Scr_BeginLoadScripts(SCRIPTINSTANCE_CLIENT, 0);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/delete", "main", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "initstructs", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "createstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "codescripts/struct", "findstruct", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "statechange", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "maprestart", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "localclientconnect", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "localclientdisconnect", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "entityspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "playerspawned", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "client_flag_callback", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "client_flagasval_callback", functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_destructible",
        "CodeCallback_DestructibleEvent",
        functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_callbacks",
        "CodeCallback_CreatingCorpse",
        functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_callbacks",
        "CodeCallback_PlayerFootstep",
        functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "CodeCallback_PlayerJump", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "CodeCallback_PlayerLand", functions);
    CGScr_LoadScriptAndLabel(
        SCRIPTINSTANCE_CLIENT,
        "clientscripts/mp/_callbacks",
        "CodeCallback_PlayerFoliage",
        functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_dogs", "playDogstep", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "level_notify", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_dogs", "soundNotify", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "airsupport", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "demo_jump", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "demo_player_switch", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "entityshutdown_callback", functions);
    CGScr_LoadScriptAndLabel(SCRIPTINSTANCE_CLIENT, "clientscripts/mp/_callbacks", "glass_smash", functions);
    CGScr_LoadLevelScript(SCRIPTINSTANCE_CLIENT, mapname, functions);
    Scr_PostCompileScripts(SCRIPTINSTANCE_CLIENT);
    CScr_PostLoadScripts();
    Scr_PrecacheAnimTrees(SCRIPTINSTANCE_CLIENT, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1);
#endif // !KISAK_SP
}

void __cdecl CGScr_LoadClientScriptsAndAnims()
{
    int address[128]; // [esp+0h] [ebp-210h] BYREF
    ScriptFunctions functions; // [esp+200h] [ebp-10h] BYREF
    const char *mapname; // [esp+20Ch] [ebp-4h]

    functions.maxSize = 128;
    functions.count = 0;
    functions.address = address;
    mapname = Dvar_GetString("mapname");
    CGScr_LoadClientScripts(mapname, &functions);
    functions.maxSize = functions.count;
    functions.count = 0;
    CScr_SetClientScripts(&functions);
    if ( functions.maxSize != functions.count )
        Com_Error(ERR_DROP, "Script function count mismatch");
    Scr_EndLoadScripts(SCRIPTINSTANCE_CLIENT);
    Scr_EndLoadAnimTrees(SCRIPTINSTANCE_CLIENT);
}

void __cdecl CG_InitScreenDimensions(int localClientNum)
{
    cgs_t *LocalClientStaticGlobals; // eax

    LocalClientStaticGlobals = CG_GetLocalClientStaticGlobals(localClientNum);
    LocalClientStaticGlobals->viewX = 0;
    LocalClientStaticGlobals->viewY = 0;
    CL_GetScreenDimensions(
        &LocalClientStaticGlobals->viewWidth,
        &LocalClientStaticGlobals->viewHeight,
        &LocalClientStaticGlobals->viewAspect);
}

int __cdecl CG_GetClientNum(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->clientNum;
}

bool __cdecl CG_IsMature()
{
    if ( !cg_mature
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2534,
                    0,
                    "%s\n\t(cg_mature) = %p",
                    "(cg_mature != 0)",
                    0) )
    {
        __debugbreak();
    }
    return cg_mature->current.enabled;
}

bool __cdecl CG_GetEntityOriginAngles(int localClientNum, int entityNum, float *origin, float *angles)
{
    centity_s *cent; // [esp+10h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entityNum);
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2544, 0, "%s", "cent") )
        __debugbreak();
    if ( ((cent->clientFlags >> 1) & 1) == 0 )
        return 0;
    *origin = cent->pose.origin[0];
    origin[1] = cent->pose.origin[1];
    origin[2] = cent->pose.origin[2];
    *angles = cent->pose.angles[0];
    angles[1] = cent->pose.angles[1];
    angles[2] = cent->pose.angles[2];
    return 1;
}

unsigned __int16 __cdecl CG_GetVehicleTypeString(int clientNum, int entityNum)
{
    const vehicle_info_t *info; // [esp+8h] [ebp-14h]
    unsigned __int16 string; // [esp+Ch] [ebp-10h]
    centity_s *cent; // [esp+10h] [ebp-Ch]
    int localClientNum; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    localClientNum = -1;
    for ( i = 0; i < 1; ++i )
    {
        if ( CG_GetLocalClientGlobals(i)->clientNum == clientNum )
        {
            localClientNum = i;
            break;
        }
    }
    if ( localClientNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2573,
                    0,
                    "%s",
                    "localClientNum >= 0") )
    {
        __debugbreak();
    }
    cent = CG_GetEntity(localClientNum, entityNum);
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2576, 0, "%s", "cent") )
        __debugbreak();
    if ( ((cent->clientFlags >> 1) & 1) == 0 )
        return 0;
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    if ( !info )
        return 0;
    string = SL_FindString(info->animSet, SCRIPTINSTANCE_SERVER);
    if ( !string )
        return SL_GetString_(SCRIPTINSTANCE_SERVER, info->animSet, 0, 22);
    return string;
}

int __cdecl CachedTag_GetCachedTagPos(
                const centity_s *ent,
                cached_client_tag_t *cachedTag,
                unsigned int tagName,
                float *pos,
                int updateInterval,
                bool forceUpdate)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2667, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->clientTagCache )
        return CachedTag_NoCache_GetTagPos(ent, tagName, pos);
    CachedTag_UpdateTagInternal(ent, cachedTag, tagName, updateInterval, forceUpdate);
    CachedTag_CalcTagPos(ent, cachedTag, pos);
    return cachedTag->time;
}

int __cdecl CachedTag_UpdateTagInternal(
                const centity_s *ent,
                cached_client_tag_t *cachedTag,
                unsigned int tagName,
                int updateInterval,
                bool forceUpdate)
{
    char *v6; // eax
    float pos[3]; // [esp+8h] [ebp-14h] BYREF
    DObj *obj; // [esp+14h] [ebp-8h]
    cg_s *cgameGlob; // [esp+18h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2598, 0, "%s", "ent") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(ent->pose.localClientNum);
    if ( !forceUpdate && cgameGlob->time < updateInterval + cachedTag->time )
        return 0;
    obj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
    if ( obj )
    {
        if ( CG_DObjGetWorldTagPos(&ent->pose, obj, tagName, pos) )
        {
            cachedTag->lastLocalTagOrigin[0] = pos[0] - ent->pose.origin[0];
            cachedTag->lastLocalTagOrigin[1] = pos[1] - ent->pose.origin[1];
            cachedTag->lastLocalTagOrigin[2] = pos[2] - ent->pose.origin[2];
            cachedTag->time = cgameGlob->time;
            return 1;
        }
        else
        {
            v6 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "CachedTag_UpdateTagInternal: Cannot find tag [%s] on entity\n", v6);
            return 0;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "CachedTag_UpdateTagInternal: Cannot find dobj on entity\n");
        return 0;
    }
}

void __cdecl CachedTag_CalcTagPos(const centity_s *ent, cached_client_tag_t *cachedTag, float *pos)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2636, 0, "%s", "ent") )
        __debugbreak();
    *pos = ent->pose.origin[0] + cachedTag->lastLocalTagOrigin[0];
    pos[1] = ent->pose.origin[1] + cachedTag->lastLocalTagOrigin[1];
    pos[2] = ent->pose.origin[2] + cachedTag->lastLocalTagOrigin[2];
}

int __cdecl CachedTag_NoCache_GetTagPos(const centity_s *ent, unsigned int tagName, float *pos)
{
    char *v4; // eax
    DObj *dobj; // [esp+4h] [ebp-8h]

    dobj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
    if ( !dobj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2646, 0, "%s", "dobj") )
        __debugbreak();
    if ( dobj )
    {
        if ( CG_DObjGetWorldTagPos(&ent->pose, dobj, tagName, pos) )
        {
            return CG_GetLocalClientGlobals(ent->pose.localClientNum)->time;
        }
        else
        {
            v4 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "CachedTag_NoCache_GetTagPos: Cannot find tag [%s] on entity\n", v4);
            return 0;
        }
    }
    else
    {
        *pos = ent->pose.origin[0];
        pos[1] = ent->pose.origin[1];
        pos[2] = ent->pose.origin[2];
        return 0;
    }
}

int __cdecl CachedTag_GetTagPos(
                const centity_s *ent,
                unsigned int tagName,
                float *pos,
                int updateInterval,
                bool forceUpdate)
{
    if ( scr_const.aim_vis_bone != scr_const.j_head
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2686,
                    0,
                    "%s",
                    "scr_const.aim_vis_bone == scr_const.j_head") )
    {
        __debugbreak();
    }
    if ( ent->nextState.eType != 1 || !ent->clientTagCache )
        return CachedTag_NoCache_GetTagPos(ent, tagName, pos);
    if ( scr_const.aim_vis_bone == tagName )
        return CachedTag_GetCachedTagPos(ent, &ent->clientTagCache->aim_head_tag, tagName, pos, updateInterval, forceUpdate);
    if ( scr_const.aim_highest_bone == tagName )
        return CachedTag_GetCachedTagPos(
                         ent,
                         &ent->clientTagCache->aim_highest_tag,
                         tagName,
                         pos,
                         updateInterval,
                         forceUpdate);
    Com_Error(ERR_DROP, "CachedTag_GetTagPos: Called for a tag that it is not set up for\n");
    return CachedTag_GetCachedTagPos(ent, 0, tagName, pos, updateInterval, forceUpdate);
}

void __cdecl CG_InitClientEntityCaches(int localClientNum)
{
    AimTargetCache *aimTargetInfo; // edx
    centity_s *cent; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( !cent->clientTagCache )
        {
            cent->clientTagCache = (ClientTagCache *)MT_Alloc(sizeof(ClientTagCache), 22, SCRIPTINSTANCE_SERVER);
            memset((unsigned __int8 *)cent->clientTagCache, 0, sizeof(ClientTagCache));
        }
        if ( !cent->aimTargetInfo )
        {
            cent->aimTargetInfo = (AimTargetCache *)MT_Alloc(sizeof(AimTargetCache), 22, SCRIPTINSTANCE_SERVER);
            aimTargetInfo = cent->aimTargetInfo;
            aimTargetInfo->lastUpdateTime = 0;
            aimTargetInfo->targetHeight = 0.0;
        }
    }
}

void __cdecl CG_FreeClientEntityCaches(int localClientNum)
{
    centity_s *cent; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( cent->clientTagCache )
        {
            MT_Free((unsigned char*)cent->clientTagCache, sizeof(ClientTagCache), SCRIPTINSTANCE_SERVER);
            cent->clientTagCache = 0;
        }
        if ( cent->aimTargetInfo )
        {
            MT_Free((unsigned char *)cent->aimTargetInfo, sizeof(AimTargetCache), SCRIPTINSTANCE_SERVER);
            cent->aimTargetInfo = 0;
        }
    }
}

void __cdecl CG_Init(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum)
{
    unsigned __int16 t; // [esp+1Ch] [ebp-64h]
    cg_s *cgameGlob; // [esp+20h] [ebp-60h]
    cgs_t *cgs; // [esp+28h] [ebp-58h]
    bool loaded_client_scripts; // [esp+33h] [ebp-4Dh]
    const char *s; // [esp+34h] [ebp-4Ch]
    char mapname[64]; // [esp+38h] [ebp-48h] BYREF
    bool loaded_server_scripts; // [esp+7Fh] [ebp-1h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2851,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    CL_GetLocalClientConnection(localClientNum);
    destroy_client_gjkcc_info(localClientNum);

    memset(cgs, 0, sizeof(cgs_t));
    memset(cgameGlob, 0, sizeof(cg_s));
    memset(&cgDC[localClientNum], 0, sizeof(UiContext));
    memset(cg_entitiesArray[localClientNum], 0, 1024 * sizeof(centity_s)/*0xCA000u*/);
    memset(cg_weaponsArray[localClientNum], 0, 2048 * sizeof(weaponInfo_s) /*0x12000u*/);
    memset(&cg_viewModelArray[localClientNum], 0, sizeof(ViewModelInfo));
    memset(&cg_BattleChatters[0].WhichSoundIsPlaying, 0, sizeof(cg_BattleChatters));
#ifdef KISAK_SP
    // Retail's hideViewModel lives in cg_s and is cleared by the cg_s memset
    // above. Keep the SP sidecar on the same frontend-to-map lifecycle.
    CG_ResetViewModelHidden_SP(localClientNum);
#endif

    cgDC[localClientNum].contextIndex = localClientNum;

    CG_InitDestructibles(localClientNum);
    CG_ClearCompassPingData();
    CG_ClearOverheadFade();
    CG_ClearPlayerDetails();
    CG_InitDof(&cgameGlob->refdef.dof);
    CG_SetupSplitscreenDvars();
    Flame_Init();
    Ragdoll_Init();
    Phys_Init();
    IK_InitSystem();
    if ( !CG_HasClientSystemBeenInitialzed() )
    {
        SND_GameReset();
        CG_SndGameReset();
    }
    cgameGlob->localClientNum = localClientNum;
    cgameGlob->viewModelPose.eType = 21;
    cgameGlob->viewModelPose.localClientNum = localClientNum;

    iassert(cgameGlob->viewModelPose.localClientNum == localClientNum);

    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);

    cgameGlob->objectiveText[0] = 0;
    cgameGlob->bgs.animData = &cg_bgsAnim;
    cgameGlob->bgs.animData->animScriptData.soundAlias = SND_FindAlias;
    cgameGlob->bgs.animData->animScriptData.playSoundAlias = CG_PlayAnimScriptSoundAlias;
    cgameGlob->bgs.GetXModel = FX_RegisterModel;
    cgameGlob->bgs.CreateDObj = CG_CreateDObj;
    cgameGlob->bgs.AttachWeapon = CG_AttachWeapon;
    cgameGlob->bgs.GetDObj = CG_GetDObj;
    cgameGlob->bgs.SafeDObjFree = Com_SafeClientDObjFree;
    cgameGlob->bgs.AllocXAnim = Hunk_AllocXAnimClient;
    cgameGlob->bgs.anim_user = 0;
    cgameGlob->bgs.Rand = CG_rand;
    cgameGlob->bgs.Random = CG_random;
    cgameGlob->clientNum = clientNum;
    cgameGlob->drawHud = 1;

    Dvar_SetBoolByName("r_grassEnable", 0);

    cgameGlob->cameraLinkedEntitiesCount = 0;
    cgameGlob->groundTiltEntNum = -1;
    cgameGlob->lastPlayerStateOverride = -1;
    cgameGlob->extraCamEntity = 1023;
#ifdef KISAK_SP
    // Seed the OTHER extra-cam sentinel -- cg_s::cameraData.extraCamEntNum, not
    // cg_s::extraCamEntity above.  They are two distinct fields (cg+0xa46cc and
    // cg+0xce4d8 in SP; see the shared-facts block in cg_scr_main_mp.cpp:281),
    // and until now only the second was ever initialised on a fresh
    // CL_InitCGame.  cg_s is zeroed there, so extraCamEntNum came up 0, which
    // made CG_IsExtraCamActive() (cg_camera.cpp:569, `!= 1023`) report an active
    // extra cam on entity 0 from frame zero, and made the FIRST
    // `camera isExtraCam( 0 )` in clientscripts/frontend.csc:60 raise
    // "There can be only one extra camera active in the level."
    //
    // THIS IS A TRANSCRIPTION, NOT A CHOSEN SPOT.  Retail SP does it right here,
    // in CG_Init, and this is the only place in the whole binary besides
    // CG_MapRestart that writes the sentinel.  Located by sweeping every
    // instruction referencing cg+0xa46cc program-wide (14 sites, 1.55M
    // instructions scanned): exactly two store 0x3ff, CG_Init @ 0x0064f96d and
    // CG_MapRestart @ 0x00490aa4 (the latter already mirrored at
    // cg_servercmds_mp.cpp:568).
    //
    // The line lands here and not elsewhere in CG_Init because retail's store
    // sits between the CALL to Dvar_SetBoolByName at 0x0064f933 and the CALL to
    // CG_ParseServerInfo at 0x0064f990 -- i.e. between :3071 and :3083 -- with
    // no other call in between.
    //
    // extraCamFov is seeded in the same breath: `MOVSS [EBP+0xa46d0],XMM0` at
    // 0x0064f977, with XMM0 loaded at 0x0064f938 from 0x009ac544, read back as
    // 00 00 82 42 == 0x42820000 == 65.0f.  That is the same constant and the
    // same pair CScr_StopExtraCam restores (cg_scr_main_mp.cpp:424-425).
    //
    // NOTE the MP line above is retained untouched: retail SP does NOT write
    // cg+0xce4d8 in CG_Init (its only writers are CG_AddPacketEntities and
    // CG_Missile), so the two lines are not alternatives -- and extraCamEntity
    // still gates the ammo-counter/camera-sensor logic in this tree.
    cgameGlob->cameraData.extraCamEntNum = 1023;
    cgameGlob->cameraData.extraCamFov = 65.0f;
#endif
    cgameGlob->lastHealthLerpDelay = 1;

    cgs->processedSnapshotNum = serverMessageNum;
    cgs->serverCommandSequence = serverCommandSequence;
    cgs->localServer = com_sv_running->current.color[0];

    CG_ParseServerInfo(localClientNum);
    CG_ParseCodInfo(localClientNum);
    R_BeginRemoteScreenUpdate();

    if ( !r_reflectionProbeGenerate->current.enabled )
        UI_LoadIngameMenus(localClientNum);

    UI_ClearLocalUIVisibilityBits(localClientNum);
    SCR_UpdateLoadScreen();
    cgMedia.whiteMaterial = Material_RegisterHandle("white", 7);
    cgMedia.smallDevFont = CL_RegisterFont("fonts/smallDevFont", 1);
    cgMedia.bigDevFont = CL_RegisterFont("fonts/bigDevFont", 1);
    Material_RegisterHandle("net_disconnect", 7);
    cgMedia.hudDpadLeftHighlight = Material_RegisterHandle("nightvision_overlay_goggles", 7);
    cgMedia.ammoCounterBullet = Material_RegisterHandle("hud_icon_nvg", 7);
    cgMedia.ammoCounterBeltBullet = Material_RegisterHandle("hud_dpad_arrow", 7);
    cgMedia.ammoCounterRifleBullet = Material_RegisterHandle("hud_dpad_eqip_count_backing", 7);
    cgMedia.ammoCounterRocket = Material_RegisterHandle("hud_dpad_outer_frame_highlight_side", 7);
    cgMedia.ammoCounterShotgunShell = Material_RegisterHandle("ammo_counter_bullet", 7);
    cgMedia.ammoCounterSingle = Material_RegisterHandle("ammo_counter_beltbullet", 7);
    cgMedia.lifeCounterAlive = Material_RegisterHandle("ammo_counter_riflebullet", 7);
    cgMedia.lifeCounterDead = Material_RegisterHandle("ammo_counter_rocket", 7);
    cgMedia.textDecodeCharacters = Material_RegisterHandle("ammo_counter_shotgunshell", 7);
    cgMedia.textDecodeCharactersGlow = Material_RegisterHandle("ammo_counter_single", 7);
    cgMedia.lifeCounterAlive = Material_RegisterHandle("life_counter_alive", 7);
    cgMedia.lifeCounterDead = Material_RegisterHandle("life_counter_dead", 7);
    cgMedia.textDecodeCharacters = Material_RegisterHandle("decode_characters", 7);
    cgMedia.textDecodeCharactersGlow = Material_RegisterHandle("decode_characters_glow", 7);

    Material_RegisterHandle("code_warning_soundcpu", 7);
    Material_RegisterHandle("code_warning_snapshotents", 7);
    Material_RegisterHandle("code_warning_maxeffects", 7);
    Material_RegisterHandle("code_warning_models", 7);
    Material_RegisterHandle("code_warning_file", 7);
    Material_RegisterHandle("code_warning_fps", 7);
    Material_RegisterHandle("code_warning_serverfps", 7);
    Material_RegisterHandle("code_warning_collision", 7);
    Material_RegisterHandle("killicondied", 7);
    Material_RegisterHandle("killiconcrush", 7);
    Material_RegisterHandle("killiconfalling", 7);
    Material_RegisterHandle("killiconsuicide", 7);
    Material_RegisterHandle("killiconheadshot", 7);
    Material_RegisterHandle("killiconmelee", 7);

    if ( cg_fs_debug->current.integer == 2 )
        Dvar_SetInt((dvar_s*)cg_fs_debug, 0);

    CG_AntiBurnInHUD_RegisterDvars();
    CG_InitConsoleCommands();
    CG_InitViewDimensions(localClientNum);
    s = CL_GetConfigString(2);
    if ( strcmp(s, "cod") )
        Com_Error(ERR_DROP, "Client/Server game mismatch: %s/%s", "cod", s);
    SCR_UpdateLoadScreen();
    if ( !com_sv_running
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 3023, 0, "%s", "com_sv_running") )
    {
        __debugbreak();
    }
    if ( !com_sv_running->current.enabled )
        Mantle_CreateAnims(Hunk_AllocXAnimClient);
    if ( !com_sv_running->current.enabled )
        Dog_CreateAnims(Hunk_AllocXAnimClient);
    VehAnim_Init();

    iassert(bgs == 0);
    
    bgs = &cgameGlob->bgs;

    if ( !bg_lastParsedWeaponIndex )
    {
        Com_SetWeaponInfoMemory(2);
        BG_ClearWeaponDef();
    }
    loaded_server_scripts = 0;
    loaded_client_scripts = 0;
    if ( !Scr_IsSystemInitied(SCRIPTINSTANCE_CLIENT) )
    {
        memset((unsigned __int8 *)&cg_bgsAnim, 0, sizeof(cg_bgsAnim));
        if ( !com_sv_running->current.enabled )
        {
            CGScr_LoadScriptsAndAnims();
            loaded_server_scripts = 1;
        }
        Scr_InitSystem(SCRIPTINSTANCE_CLIENT, 1);
        Scr_SetLoading(1, SCRIPTINSTANCE_CLIENT);
        Scr_AllocGameVariable(SCRIPTINSTANCE_CLIENT);
        CGScr_LoadClientScriptsAndAnims();
        Scr_SetLoading(0, SCRIPTINSTANCE_CLIENT);
        loaded_client_scripts = 1;
    }
    if ( !g_mapLoaded && !useFastFile->current.enabled )
    {
        CG_LoadingString(localClientNum, "sound aliases");
        //BLOPS_NULLSUB((jpeg_decompress_struct *)cgs->mapname);
    }
    CG_SetupWeaponDef();
    if ( !com_sv_running->current.enabled && !CG_HasClientSystemBeenInitialzed() )
    {
        bg_numVehicleInfos = 0;
        CG_Veh_Init();
    }
    CG_Veh_RegisterMaterials();
    if ( I_strnicmp(cgs->mapname, "maps/", 5)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    3090,
                    0,
                    "%s",
                    "!I_strnicmp( cgs->mapname, \"maps/\", 5 )") )
    {
        __debugbreak();
    }
    Com_StripExtension(&cgs->mapname[5], mapname);
    CG_LoadAnimTrees(localClientNum, cgameGlob, mapname, loaded_client_scripts);
    if ( loaded_server_scripts )
        Scr_FreeScripts(SCRIPTINSTANCE_SERVER, 1u);
    if ( !cgs->localServer )
        GScr_LoadConsts();
    CG_LoadingString(localClientNum, "collision map");
    CL_CM_LoadMap(cgs->mapname);
    Menu_Setup(&cgDC[localClientNum]);
    CG_LoadingString(localClientNum, "graphics");
    CG_ParsePlayerInfos();
    if ( !g_mapLoaded )
    {
        CG_LoadingString(localClientNum, cgs->mapname);
        LoadWorld(cgs->mapname);
        g_mapLoaded = 1;
    }
    CG_LoadingString(localClientNum, "game media");
    if ( !CG_HasClientSystemBeenInitialzed() )
    {
        R_Stream_ResetHintEntities();
        R_PerMap_Init();
        R_Stream_ResetHintEntities();
    }
    ProfLoad_Begin("Init effects system");
    FX_InitSystem(localClientNum);
    FX_RegisterDefaultEffect();
    ProfLoad_End();
    SCR_UpdateLoadScreen();
    CG_RegisterGraphics(localClientNum, mapname);
    CG_LoadingString(localClientNum, "clients");
    if ( !cls.vidConfig.isToolMode )
        GC_InitWeaponOptions();
    CG_SetupGameInformation(localClientNum);
    CG_LoadHudMenu(localClientNum);
    CG_SetGridTable();
    //BLOPS_NULLSUB((jpeg_decompress_struct *)localClientNum);
    CG_InitEntities(localClientNum);
    CG_InitLocalEntities(localClientNum);
    DynEntCl_InitEntities(localClientNum);
    CG_InitVisionSets(localClientNum);
    CG_InitExposure(localClientNum, mapname);
    CG_InitBolt(localClientNum);
    cgameGlob->isLoading = 0;
    CG_SetConfigValues(localClientNum);
    CG_LoadingString(localClientNum, "");
    CG_NorthDirectionChanged(localClientNum);
    CL_FinishLoadingModels();
    if ( !g_mapLoaded )
        SND_StopSounds(SND_STOP_ALL);
    CG_ParseFog(localClientNum);
    R_WaterSimulationRestart();
    R_InitPrimaryLights(cgameGlob->refdef.primaryLights);
    R_ClearShadowedPrimaryLightHistory(localClientNum);
    CL_SetADS(localClientNum, 0);
    AimTarget_Init(localClientNum);
    AimAssist_Init(localClientNum);
    CG_InitClientEntityCaches(localClientNum);
    CG_InitVote(localClientNum);
    Flame_Init_DVars();
    Flame_Init_FlameVars();
    Flame_InitDevGui();
    if ( cg_loadScripts && cg_loadScripts->current.enabled )
    {
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            CScr_LoadStructs();
            CScr_LoadLevel();
            cg_fakeEntitiesInuseCountFromLoadScript = cg_fakeEntitiesInuseCount[localClientNum];
            if ( cg_fakeEntitiesInuseCountFromLoadScript > 412 )
                Com_Error(
                    ERR_DROP,
                    "To many local clientside entities used for the map: %i and script: %i.    Need to reserve %i for dynamic gameplay usage.\n",
                    cg_fakeEntitiesInuseCountFromLoadScript - cg_fakeEntitiesInuseCountFromMap,
                    cg_fakeEntitiesInuseCountFromMap,
                    100);
            Com_PrintWarning(
                16,
                "Fake Ents:    Using %i fake ents from the map and %i from script at level load.\n",
                cg_fakeEntitiesInuseCountFromLoadScript - cg_fakeEntitiesInuseCountFromMap,
                cg_fakeEntitiesInuseCountFromMap);
        }
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.localclientconnect, 1u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
    BG_InitFire();

    iassert(bgs == &cgameGlob->bgs);
    
    bgs = NULL;
    R_EndRemoteScreenUpdate(0);
    CG_InitScoreboard();
}

clientConnection_t *__cdecl CL_GetLocalClientConnection(int localClientNum)
{
    if ( !clientConnections
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../client/client.h",
                    200,
                    0,
                    "%s",
                    "clientConnections") )
    {
        __debugbreak();
    }
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../client/client.h",
                    205,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return clientConnections;
}

void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname)
{
    const FxEffectDef *v2; // eax
    const FxEffectDef *v3; // eax
    shellshock_parms_t *ShellshockParms; // eax
    const char *shellshock; // [esp+0h] [ebp-94h]
    const char *effectname; // [esp+4h] [ebp-90h]
    const char *modelName; // [esp+8h] [ebp-8Ch]
    const char *EffectNames[27]; // [esp+Ch] [ebp-88h]
    cgs_t *cgs; // [esp+80h] [ebp-14h]
    int i; // [esp+84h] [ebp-10h]
    const char *fireEffectNames[3]; // [esp+88h] [ebp-Ch]

    SCR_UpdateLoadScreen();
    CG_LoadingString(localClientNum, " - textures");
    cgMedia.lagometerMaterial = Material_RegisterHandle("lagometer", 7);
    cgMedia.connectionMaterial = Material_RegisterHandle("headicondisconnected", 7);
    cgMedia.youInKillCamMaterial = Material_RegisterHandle("headiconyouinkillcam", 7);
    Material_RegisterHandle("killiconmelee", 7);
    Material_RegisterHandle("killiconsuicide", 7);
    Material_RegisterHandle("killiconfalling", 7);
    Material_RegisterHandle("killiconcrush", 7);
    Material_RegisterHandle("killicondied", 7);
    cgMedia.redTracerMaterial = Material_RegisterHandle("gfx_red_tracer", 6);
    cgMedia.greenTracerMaterial = Material_RegisterHandle("gfx_green_tracer", 6);
    cgMedia.bulletMaterial = Material_RegisterHandle("gfx_bullet", 6);
    cgMedia.laserMaterial = Material_RegisterHandle("gfx_laser", 6);
    cgMedia.laserLightMaterial = Material_RegisterHandle("gfx_laser_light", 6);
    cgMedia.ropeMaterial = Material_RegisterHandle("rope", 6);
    cgMedia.hintMaterials[4] = Material_RegisterHandle("hint_health", 7);
    cgMedia.hintMaterials[5] = Material_RegisterHandle("hint_friendly", 7);
    cgMedia.stanceMaterials[0] = Material_RegisterHandle("stance_stand", 7);
    cgMedia.stanceMaterials[1] = Material_RegisterHandle("stance_crouch", 7);
    cgMedia.stanceMaterials[2] = Material_RegisterHandle("stance_prone", 7);
    cgMedia.stanceMaterials[4] = Material_RegisterHandle("stance_swim", 7);
    cgMedia.stanceMaterials[3] = Material_RegisterHandle("stance_flash", 7);
    cgMedia.objectiveMaterials[0] = Material_RegisterHandle("objective", 7);
    cgMedia.friendMaterials[0] = Material_RegisterHandle("compassping_friendly_mp", 7);
    cgMedia.friendMaterials[1] = Material_RegisterHandle("objective_friendly_chat", 7);
    cgMedia.friendMaterials[2] = Material_RegisterHandle("compass_waypoint_second_chance", 7);
    cgMedia.damageMaterial = Material_RegisterHandle("hit_direction", 7);
    cgMedia.mantleHint = Material_RegisterHandle("hint_mantle", 7);
    cgMedia.compassping_player = Material_RegisterHandle("compassping_player", 7);
    cgMedia.compassping_friendlyfiring = Material_RegisterHandle("compassping_friendlyfiring_mp", 7);
    cgMedia.compassping_friendlyyelling = Material_RegisterHandle("compassping_friendlyyelling_mp", 7);
    cgMedia.compassping_friendlyfakefire = Material_RegisterHandle("compassping_decoyfiring", 7);
    cgMedia.compassping_enemy = Material_RegisterHandle("compassping_enemy", 7);
    cgMedia.compassping_enemydirectional = Material_RegisterHandle("compassping_enemydirectional", 7);
    cgMedia.compassping_enemysatellite = Material_RegisterHandle("compassping_enemysatellite", 7);
    cgMedia.compassping_enemyfiring = Material_RegisterHandle("compassping_enemyfiring", 7);
    cgMedia.compassping_enemyyelling = Material_RegisterHandle("compassping_enemyyelling", 7);
    cgMedia.compassping_grenade = Material_RegisterHandle("compassping_grenade", 7);
    cgMedia.compassping_explosion = Material_RegisterHandle("compassping_explosion", 7);
    cgMedia.compassping_firstplace = Material_RegisterHandle("compassping_firstplace", 7);
    cgMedia.compass_radarline = Material_RegisterHandle("compass_radarline", 7);
    cgMedia.compass_acoustic_ping = Material_RegisterHandle("compass_acoustic_ping", 7);
    cgMedia.watch_face = Material_RegisterHandle("watch_face", 7);
    cgMedia.watch_hour = Material_RegisterHandle("watch_hour", 7);
    cgMedia.watch_minute = Material_RegisterHandle("watch_minute", 7);
    cgMedia.watch_second = Material_RegisterHandle("watch_second", 7);
    cgMedia.acoustic_ping = Material_RegisterHandle("acoustic_ping", 7);
    cgMedia.acoustic_wedge = Material_RegisterHandle("acoustic_wedge", 7);
    cgMedia.acoustic_grid = Material_RegisterHandle("acoustic_grid", 7);
    cgMedia.compass_scrambler_large = Material_RegisterHandle("compass_scrambler_large", 7);
    cgMedia.compass_mortar_selector = Material_RegisterHandle("waypoint_recon_artillery_strike", 7);
    cgMedia.compass_artillery_enemy = Material_RegisterHandle("compass_objpoint_flak_busy", 7);
    cgMedia.compass_artillery_friendly = Material_RegisterHandle("compass_objpoint_flak_friendly", 7);
    cgMedia.compass_mortar_enemy = Material_RegisterHandle("compass_objpoint_mortar_busy", 7);
    cgMedia.compass_mortar_friendly = Material_RegisterHandle("compass_objpoint_mortar_friendly", 7);
    cgMedia.compass_dogs_enemy = Material_RegisterHandle("compassping_dog", 7);
    cgMedia.compass_incoming_artillery = Material_RegisterHandle("waypoint_recon_artillery_strike", 7);
    cgMedia.compass_sentry_friendly = Material_RegisterHandle("compass_turret_green", 7);
    cgMedia.compass_sentry_friendly_firing = Material_RegisterHandle("compass_turret_green_fire", 7);
    cgMedia.compass_sentry_enemy = Material_RegisterHandle("compass_turret_red", 7);
    cgMedia.compass_sentry_enemy_firing = Material_RegisterHandle("compass_turret_red_fire", 7);
    cgMedia.compass_tow_turret_friendly = Material_RegisterHandle("compass_sam_turret_green", 7);
    cgMedia.compass_tow_turret_friendly_firing = Material_RegisterHandle("compass_sam_turret_green_fire", 7);
    cgMedia.compass_tow_turret_enemy = Material_RegisterHandle("compass_sam_turret_red", 7);
    cgMedia.compass_tow_turret_enemy_firing = Material_RegisterHandle("compass_sam_turret_red_fire", 7);
    cgMedia.compass_guided_missile = Material_RegisterHandle("compassping_player_missle", 7);
    cgMedia.grenadeIconFrag = Material_RegisterHandle("hud_grenadeicon", 7);
    cgMedia.grenadeIconFlash = Material_RegisterHandle("hud_flashbangicon", 7);
    cgMedia.grenadeIconThrowBack = Material_RegisterHandle("hud_grenadethrowback", 7);
    cgMedia.grenadePointer = Material_RegisterHandle("hud_grenadepointer", 7);
    cgMedia.offscreenObjectivePointer = Material_RegisterHandle("hud_offscreenobjectivepointer", 7);
    cgMedia.demoTimelineFaded = Material_RegisterHandle("demo_timeline_faded", 7);
    cgMedia.demoTimelineSolid = Material_RegisterHandle("demo_timeline_solid", 7);
    cgMedia.demoTimelineCursor = Material_RegisterHandle("demo_timeline_arrow", 7);
    cgMedia.demoTimelineBookmark = Material_RegisterHandle("demo_timeline_bookmark", 7);
    cgMedia.demoStatePaused = Material_RegisterHandle("demo_pause", 7);
    cgMedia.demoStatePlay = Material_RegisterHandle("demo_play", 7);
    cgMedia.demoStateStop = Material_RegisterHandle("demo_stop", 7);
    cgMedia.demoStateJump = Material_RegisterHandle("demo_step", 7);
    cgMedia.demoStateForwardFast = Material_RegisterHandle("demo_forward_fast", 7);
    cgMedia.demoStateForwardSlow = Material_RegisterHandle("demo_forward_slow", 7);
    cgMedia.theaterUpArrow = Material_RegisterHandle("theater_up_arrow", 7);
    cgMedia.theaterDownArrow = Material_RegisterHandle("theater_down_arrow", 7);
    cgMedia.theaterLeftArrow = Material_RegisterHandle("theater_left_arrow", 7);
    cgMedia.theaterRightArrow = Material_RegisterHandle("theater_right_arrow", 7);
    cgMedia.teamStatusBar = Material_RegisterHandle("hudcolorbar", 7);
    CG_LoadingString(localClientNum, " - models");
    cgMedia.afkLightbulb = Material_RegisterHandle("headicontalkballoon", 7);
    CG_RegisterScoreboardGraphics();
    CG_LoadingString(localClientNum, " - items");
    CG_RegisterItems(localClientNum);
    CG_LoadingString(localClientNum, " - inline models");
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    CG_LoadingString(localClientNum, " - server models");
    for (i = 1; i < 512; ++i)
    {
        modelName = CL_GetConfigString(i + 1568);
        if (*modelName)
        {
            SCR_UpdateLoadScreen();
            cgs->gameModels[i] = R_RegisterModel((char*)modelName);
        }
    }
    for (i = 1; i < 196; ++i)
    {
        effectname = CL_GetConfigString(i + 2080);
        if (*effectname)
        {
            cgs->fxs[i] = FX_Register(effectname);
            if (!cgs->fxs[i]
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    1658,
                    0,
                    "%s",
                    "cgs->fxs[i]"))
            {
                __debugbreak();
            }
        }
    }
    EffectNames[0] = "weapon/grenade/fx_smoke_grenade_11sec_mp";
    EffectNames[1] = "misc/fx_flare_sky_white_10sec_mp";
    EffectNames[2] = "weapon/napalm/fx_napalm_ground_fire_lg_mp";
    EffectNames[3] = "weapon/napalm/fx_napalm_ground_fire_sm_mp";
    EffectNames[4] = "weapon/grenade/fx_exp_incendiary_mp_125r";
    EffectNames[5] = "weapon/grenade/fx_exp_incendiary_mp_75r";
    EffectNames[6] = "weapon/grenade/fx_exp_incendiary_mp_50r";
    EffectNames[7] = "weapon/grenade/fx_exp_incendiary_mp_25r";
    EffectNames[8] = "weapon/grenade/fx_exp_incendiary_mp_center";
    EffectNames[9] = "weapon/rocket/fx_trail_bazooka_geotrail";
    EffectNames[10] = "explosions/fx_default_explosion";
    EffectNames[11] = "env/fire/fx_fire_player_torso_mp";
    EffectNames[12] = "env/fire/fx_fire_player_sm_mp";
    EffectNames[13] = "env/fire/fx_fire_player_md_mp";
    EffectNames[14] = "env/fire/fx_fire_player_sm_smk_2sec";
    EffectNames[15] = "trail/fx_trail_blood_streak_mp";
    EffectNames[16] = "system_elements/fx_blood_drops_decal_emit";
    EffectNames[17] = "impacts/fx_flesh_hit_knife_mp";
    EffectNames[18] = "weapon/grenade/fx_smoke_grenade_mp_125r";
    EffectNames[19] = "weapon/grenade/fx_smoke_grenade_mp_75r";
    EffectNames[20] = "weapon/grenade/fx_smoke_grenade_mp_50r";
    EffectNames[21] = "weapon/grenade/fx_smoke_grenade_mp_25r";
    EffectNames[22] = "weapon/grenade/fx_gas_poison_mp_125r";
    EffectNames[23] = "weapon/grenade/fx_gas_poison_mp_75r";
    EffectNames[24] = "weapon/grenade/fx_gas_poison_mp_50r";
    EffectNames[25] = "weapon/grenade/fx_gas_poison_mp_25r";
    EffectNames[26] = "weapon/grenade/fx_gas_poison_mp";
    for (i = 0; i < 27; ++i)
    {
        v2 = FX_Register(EffectNames[i]);
        cgs->grenadeFx[i] = v2;
        if (!cgs->grenadeFx[i]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                1710,
                0,
                "%s",
                "cgs->grenadeFx[i]"))
        {
            __debugbreak();
        }
    }
    fireEffectNames[0] = "env/fire/fx_fire_player_torso_mp";
    fireEffectNames[1] = "env/fire/fx_fire_player_sm_mp";
    fireEffectNames[2] = "env/fire/fx_fire_player_md_mp";
    for (i = 0; i < 3; ++i)
    {
        v3 = FX_Register(fireEffectNames[i]);
        cgs->playerFireFx[i] = v3;
        if (!cgs->playerFireFx[i]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                1727,
                0,
                "%s",
                "cgs->playerFireFx[i]"))
        {
            __debugbreak();
        }
    }
    for (i = 1; i < 16; ++i)
    {
        shellshock = CL_GetConfigString(i + 2532);
        if (*shellshock)
        {
            if (!BG_LoadShellShockDvars(shellshock))
                Com_Error(ERR_DROP, "couldn't register shellshock '%s' -- see console", shellshock);
            ShellshockParms = BG_GetShellshockParms(i);
            BG_SetShellShockParmsFromDvars(ShellshockParms);
        }
    }
    if (!BG_LoadShellShockDvars("hold_breath_mp"))
        Com_Error(ERR_DROP, "Couldn't find shock file [hold_breath_mp.shock]\n");
    BG_SetShellShockParmsFromDvars(&cgs->holdBreathParams);
    cgMedia.fx = CG_RegisterImpactEffects(mapname);
    if (!cgMedia.fx)
        Com_Error(ERR_DROP, "Error reading CSV files in the fx directory to identify impact effects");
    cgMedia.fxNoBloodFleshHit = FX_Register("impacts/fx_flesh_hit_noblood");
    cgMedia.fxKnifeBlood = FX_Register("impacts/fx_flesh_hit_knife_mp");
    cgMedia.fxKnifeNoBlood = FX_Register("impacts/fx_flesh_hit_knife_noblood");
    cgMedia.fxDogBlood = FX_Register("impacts/fx_deathfx_dogbite");
    cgMedia.fxDogNoBlood = FX_Register("impacts/fx_flesh_hit_knife_noblood");
    cgMedia.fxNonFatalHero = FX_Register("impacts/fx_flesh_hit_body_nonfatal_hero");
    cgMedia.fxBodyArmorSmall = FX_Register("impacts/fx_small_metalhit_bodyarmor");
    cgMedia.fxBodyArmorLarge = FX_Register("impacts/fx_xlarge_metalhit_bodyarmor");
    cgMedia.fxDtpArmSlide1 = FX_Register("bio/player/fx_player_arm_dust_slide");
    cgMedia.fxDtpArmSlide2 = FX_Register("bio/player/fx_player_arm_dust_slide_rk");
    cgMedia.fxPlayerSliding = FX_Register("system_elements/fx_snow_sm_em");
    cgMedia.fxPuff = FX_Register("bio/player/fx_player_dust_inair");
    cgMedia.heliDustEffect = FX_Register("vehicle/treadfx/fx_heli_dust_default");
    cgMedia.heliWaterEffect = FX_Register("vehicle/treadfx/fx_heli_water_spray");
    cgMedia.helicopterLightSmoke = FX_Register("trail/fx_trail_heli_white_smoke");
    cgMedia.helicopterHeavySmoke = FX_Register("trail/fx_trail_heli_black_smoke");
    cgMedia.helicopterOnFire = FX_Register("trail/fx_trail_fire_smoke");
    cgMedia.jetAfterburner = FX_Register("vehicle/exhaust/fx_exhaust_jet_afterburner");
    cgMedia.physicsWaterEffects[0] = FX_Register("impacts/fx_water_hit_sm");
    cgMedia.physicsWaterEffects[1] = FX_Register("impacts/fx_water_hit_md");
    cgMedia.physicsWaterEffects[2] = FX_Register("impacts/fx_water_hit_lg");
    cgMedia.physicsWaterEffects[3] = FX_Register("impacts/fx_water_object_ripple");
    cgMedia.physicsWaterEffects[4] = FX_Register("bio/player/fx_water_hit_player_bubbles");
    cgMedia.physicsWaterEffects[5] = FX_Register("bio/player/fx_player_water_waist_ripple");
    cgMedia.physicsWaterEffects[6] = FX_Register("bio/player/fx_player_water_knee_ripple");
    cgMedia.physicsWaterEffects[7] = FX_Register("bio/player/fx_player_water_splash_impact");
    cgMedia.infraredHeartbeat = FX_Register("weapon/ir_scope/fx_ir_scope_heartbeat");
    CG_LoadingString(localClientNum, " - game media done");
}

void __cdecl CG_LoadHudMenu(int localClientNum)
{
    MenuList *Menus; // eax
    MenuList *v2; // eax
    const char *String; // eax
    const char *v4; // eax
    MenuList *v5; // eax
    cgs_t *cgs; // [esp+0h] [ebp-10h]
    menuDef_t *menu; // [esp+4h] [ebp-Ch]
    MenuList *menuList; // [esp+8h] [ebp-8h]
    MenuList *menuLista; // [esp+8h] [ebp-8h]
    MenuList *menuListb; // [esp+8h] [ebp-8h]
    const rectDef_s *rect; // [esp+Ch] [ebp-4h]

    // SP HUD menufiles live under "ui/", and SP's set is genuinely DIFFERENT from MP's - not
    // just re-prefixed. Evidence (zone scan of all 139 shipped .ff files, plus the retail
    // BlackOps.exe string table):
    //   ui_mp/hud.txt       -> common_mp only | ui/hud.txt        -> code_post_gfx.ff, and the
    //                                            literal "ui/hud.txt" IS in the SP exe
    //   ui_mp/hud_%s.txt    -> ...             | "ui/hud_%s.txt" IS in the SP exe, and
    //                                            ui/hud_sp.txt, ui/hud_zombie.txt,
    //                                            ui/hud_coop.txt, ui/hud_demo.txt,
    //                                            ui/hud_splitscreen.txt all ship in code_post_gfx
    //   ui_mp/hud_hardcore.txt, ui_mp/hud_spectator.txt, ui_mp/hud_popups.txt,
    //   ui_mp/hud_demo.txt  -> common_mp only, and NO literal of any spelling exists in the
    //                          retail SP exe -> SP does not load these at all.
    // Non-fatal either way (a missing menufile only warns), so this is accuracy work, not a boot
    // fix. Audit finding C8 (asset-availability audit) / 5a (frontend-map-load audit).
    // NOTE(SP, RESOLVED+IMPLEMENTED 2026-08-26): the ARGUMENT to the hud_%s.txt format WAS wrong
    // here; it is now known and the fix is implemented below. Demoted from an open marker on
    // 2026-08-26 by a triage pass that re-verified the KISAK_SP block below against retail
    // 0x0088EFD0.
    // SETTLED 2026-08-26 by decompiling 0x0088efd0 -- which the live Ghidra database now names
    // CG_LoadHudMenu (tags openblops-source-match + machine-proposed + custom-abi; localClientNum
    // arrives in EDI). Ghidra is available again; the "Ghidra is unavailable in this session"
    // deferral below no longer applies and the old hypothesis is disproved:
    //   * The format argument is NEITHER g_gametype NOR a mode string. Retail does
    //         0088f07c PUSH 0x9ae25c ("mapname")  ->  Dvar_GetString  ->  sprintf(buf, "ui/hud_%s.txt")
    //     i.e. it formats the MAP NAME. Both candidates previously on the table were wrong.
    //   * The three-way sp/zombie/coop branch that docs/SP_MP_STARTUP_AUDIT.md row #11 / Q7
    //     describes is REAL but is a DIFFERENT load, not the "%s" selector. Retail's actual
    //     sequence is: ui/hud.txt unconditionally; then ui/hud_zombie.txt or ui/hud_sp.txt on a
    //     dvar's current.enabled byte (dvar_s at 0x0243fdd4); then ui/hud_coop.txt if either of
    //     two further dvars (0x0247fed0 / 0x0290bf04) has current.enabled set; then the
    //     ui/hud_<mapname>.txt load; then ui/vs_hud.txt when a dvar string (0x02562a14) equals
    //     "vs". Conflating the two is what produced the mode-string guess.
    //   * There is no IsHardcoreMode gate in retail at all, and retail only calls UI_AddMenuList
    //     for the hud_%s.txt list when UI_LoadMenus returned non-NULL -- a guard this
    //     reconstruction does not have.
    // IMPLEMENTED 2026-08-26 under KISAK_SP. Re-verified against retail 0x0088EFD0 immediately
    // before writing; the sequence above is confirmed exactly, and EVERY UI_LoadMenus call in
    // retail passes 7 as its second argument (checked, because the ingame path uses 3 -- 7 does
    // carry over here).
    // THE FOUR GATING DVARS ARE NOW ALL NAMED, by the registrar string-table walk (agents.md
    // "The registrar string-table walk"), i.e. find the global's sole WRITE xref, then read the
    // name string pushed before the register call one call EARLIER than the store:
    //   0x0243FDD4 = "zombiemode"  -- writer 0x0082BCB1 in Com_InitDvars, name push 0x0082BC88
    //                                -> 0x00A2921C. (Already on record; the chain was re-run.)
    //   0x0247FED0 = "onlinegame"  -- writer 0x0082BDE3 in Com_InitDvars, name push 0x0082BDC6
    //                                -> 0x00A499C4. Corroborated independently: retail registers
    //                                it with value 0 / flags 0 and the very next registration is
    //                                0x00A06548 "xblive_rankedmatch" -- exactly this tree's
    //                                common.cpp:2336-2345 ordering and constants.
    //   0x0290BF04 = "systemlink"  -- writer 0x00590D6A in CL_InitOnceForAllClients, name push
    //                                0x00590D4D -> 0x00A01E4C. The next registration is
    //                                0x00A07188 "systemlink_warning_shown", which is what a
    //                                systemlink flag would sit beside.
    //   0x02562A14 = "ui_gametype" -- writer 0x00836079 in FUN_00835D00, name push 0x00836056
    //                                -> 0x009C69C0. Registered as a STRING dvar with value ""
    //                                and flags 0 (matching ui_main.cpp:3389), and the next
    //                                registration is 0x009C3CB0 "ui_mapname".
    // Three of the four have a live global here; "systemlink" is registered by retail but by
    // NOTHING in this reconstruction, so it is read by name with Dvar_GetBool, which returns
    // false for an unregistered dvar (dvar.cpp:951-957) -- the same result retail gets from its
    // default-false global. That is the one place this transcription is not pointer-for-pointer.
    // HAZARD, live: UI_AddMenu asserts "touchMenu == menu" (ui_shared.cpp:11090) when a menu NAME
    // resolves in the asset DB to a different menuDef than the one in the list just loaded - i.e.
    // when the same menu name exists in two loaded zones. These loads were previously all no-ops
    // on SP (every ui_mp/ file missed), so this is the first time SP feeds real menus through
    // UI_AddMenuList from here. If SP asserts at boot in UI_AddMenu, this block is the first
    // suspect.
#ifdef KISAK_SP
    // ui/hud.txt, unconditional -- retail has NO IsHardcoreMode gate here.
    v2 = UI_LoadMenus(SP_UI_DIR "hud.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], v2, 0);
    // zombie vs sp, on zombiemode->current.enabled (0x0088EFF9 tests the +0x18 byte).
    if ( zombiemode->current.enabled )
        Menus = UI_LoadMenus(SP_UI_DIR "hud_zombie.txt", 7);
    else
        Menus = UI_LoadMenus(SP_UI_DIR "hud_sp.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], Menus, 0);
    // ui/hud_coop.txt when EITHER onlinegame or systemlink is set (0x0088F040 / 0x0088F04F).
    if ( onlinegame->current.enabled || Dvar_GetBool("systemlink") )
    {
        v5 = UI_LoadMenus(SP_UI_DIR "hud_coop.txt", 7);
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], v5, 0);
    }
    // ui/hud_<mapname>.txt -- the %s is Dvar_GetString("mapname") (0x0088F07C), NOT g_gametype.
    // Retail formats into a 0x40 stack buffer with sprintf; va() is kept because the result is
    // consumed immediately by the very next call and it is what this site already used.
    // The non-NULL guard is retail's (0x0088F0A0) and this reconstruction lacked it.
    String = Dvar_GetString("mapname");
    v4 = va(SP_UI_DIR "hud_%s.txt", String);
    menuLista = UI_LoadMenus(v4, 7);
    if ( menuLista )
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuLista, 0);
    // ui/vs_hud.txt when ui_gametype's string is "vs" (I_stricmp against 0x009B04D4 at 0x0088F0C5).
    if ( !I_stricmp(ui_gametype->current.string, "vs") )
    {
        menuListb = UI_LoadMenus(SP_UI_DIR "vs_hud.txt", 7);
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuListb, 0);
    }
#else
    Menus = UI_LoadMenus("ui_mp/hud_hardcore.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], Menus, 0);
    menuList = UI_LoadMenus("ui_mp/hud_spectator.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuList, 0);
    if ( !IsHardcoreMode(localClientNum) )
    {
        v2 = UI_LoadMenus(SP_UI_DIR "hud.txt", 7);
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], v2, 0);
        String = Dvar_GetString("g_gametype");
        v4 = va(SP_UI_DIR "hud_%s.txt", String);
        menuLista = UI_LoadMenus(v4, 7);
        UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuLista, 0);
    }
    v5 = UI_LoadMenus("ui_mp/hud_popups.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], v5, 0);
    menuListb = UI_LoadMenus("ui_mp/hud_demo.txt", 7);
    UI_AddMenuList(localClientNum, &cgDC[localClientNum], menuListb, 0);
#endif
    if ( CL_LocalClient_GetActiveCount() == 1 )
        menu = Menus_FindByName(&cgDC[localClientNum], "Compass");
    else
        menu = Menus_FindByName(&cgDC[localClientNum], "Compass_mp");
    if ( menu )
    {
        rect = Window_GetRect(&menu->window);
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        cgs->compassWidth = rect->w;
        cgs->compassHeight = rect->h;
        cgs->compassY = rect->y;
    }
}

const rectDef_s *__cdecl Window_GetRect(const windowDef_t *w)
{
    if ( !w && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_utils_api.h", 37, 0, "%s", "w") )
        __debugbreak();
    return &w->rect;
}

unsigned __int16 __cdecl CG_AttachWeapon(DObjModel_s *dobjModels, unsigned __int16 numModels, clientInfo_t *ci)
{
    const WeaponDef *weapDefDW; // [esp+4h] [ebp-10h]
    int oldLeftHand; // [esp+8h] [ebp-Ch]
    unsigned __int8 weaponModel; // [esp+Fh] [ebp-5h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]
    const WeaponDef *weapDefa; // [esp+10h] [ebp-4h]
    const WeaponDef *weapDefb; // [esp+10h] [ebp-4h]

    if ( ci->iDObjWeapon )
    {
        weapDef = BG_GetWeaponDef(ci->iDObjWeapon);
        weaponModel = ci->weaponModel;
        if ( weapDef->worldModel[weaponModel] && !weapDef->bHideThirdPerson )
        {
            if ( numModels >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                            2081,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = weapDef->worldModel[weaponModel];
            dobjModels[numModels].boneName = CG_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
            dobjModels[numModels++].ignoreCollision = 0;
        }
        if ( weapDef->bDualWield )
        {
            weapDefDW = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
            if ( weapDefDW->worldModel[weaponModel] )
            {
                if ( !ci->usingKnife )
                {
                    if ( numModels >= 0x20u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                    2099,
                                    0,
                                    "%s",
                                    "numModels < DOBJ_MAX_SUBMODELS") )
                    {
                        __debugbreak();
                    }
                    dobjModels[numModels].model = weapDefDW->worldModel[weaponModel];
                    oldLeftHand = ci->leftHandGun;
                    ci->leftHandGun = 1;
                    dobjModels[numModels].boneName = CG_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
                    ci->leftHandGun = oldLeftHand;
                    dobjModels[numModels++].ignoreCollision = 0;
                }
            }
        }
        if ( weapDef->additionalMeleeModel )
        {
            if ( numModels >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                            2115,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = weapDef->additionalMeleeModel;
            dobjModels[numModels].boneName = scr_const.tag_weapon_left;
            dobjModels[numModels++].ignoreCollision = 0;
        }
        else if ( ci->usingKnife && ci->iDObjMeleeWeapon )
        {
            weapDefa = BG_GetWeaponDef(ci->iDObjMeleeWeapon);
            if ( weapDefa->worldModel )
            {
                if ( numModels >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                2128,
                                0,
                                "%s",
                                "numModels < DOBJ_MAX_SUBMODELS") )
                {
                    __debugbreak();
                }
                dobjModels[numModels].model = weapDefa->worldModel[ci->meleeWeaponModel];
                dobjModels[numModels].boneName = scr_const.tag_weapon_left;
                dobjModels[numModels++].ignoreCollision = 0;
            }
        }
        else if ( ci->usingGrenade )
        {
            if ( ci->iDObjOffhandWeapon )
            {
                weapDefb = BG_GetWeaponDef(ci->iDObjOffhandWeapon);
                if ( weapDefb->worldModel )
                {
                    if ( numModels >= 0x20u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                                    2142,
                                    0,
                                    "%s",
                                    "numModels < DOBJ_MAX_SUBMODELS") )
                    {
                        __debugbreak();
                    }
                    dobjModels[numModels].model = weapDefb->worldModel[ci->offhandWeaponModel];
                    dobjModels[numModels].boneName = scr_const.tag_inhand;
                    dobjModels[numModels++].ignoreCollision = 0;
                }
            }
        }
    }
    return numModels;
}

void __cdecl CG_CreateDObj(
                DObjModel_s *dobjModels,
                unsigned __int16 numModels,
                XAnimTree_s *tree,
                int handle,
                int localClientNum,
                clientInfo_t *ci)
{
    float *v6; // eax
    centity_s *ent; // [esp+Ch] [ebp-4h]

    Com_ClientDObjCreate(dobjModels, numModels, tree, handle, localClientNum);
    ent = CG_GetEntity(localClientNum, handle);
    if ( ent && ent->pose.isRagdoll )
    {
        if ( ent->pose.killcamRagdollHandle > 0 )
            Ragdoll_RebindBody(ent->pose.killcamRagdollHandle);
        if ( ent->pose.ragdollHandle > 0 )
            Ragdoll_RebindBody(ent->pose.ragdollHandle);
    }
    v6 = cg_entityOriginArray[localClientNum][ci->clientNum];
    v6[0] = 131072.0f;
    v6[1] = 131072.0f;
    v6[2] = 131072.0f;
}

DObj *__cdecl CG_GetDObj(unsigned int handle, int localClientNum)
{
    return Com_GetClientDObj(handle, localClientNum);
}

void __cdecl CG_InitEntities(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax
    int entityIndex; // [esp+8h] [ebp-Ch]
    centity_s *cent; // [esp+10h] [ebp-4h]

    for ( entityIndex = 0; entityIndex < 1024; ++entityIndex )
    {
        cent = CG_GetEntity(localClientNum, entityIndex);
        if ( !cent
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp", 2471, 0, "%s", "cent") )
        {
            __debugbreak();
        }
        cent->pose.localClientNum = localClientNum;
    }
    memset((unsigned __int8 *)&cg_fakeEntitiesArray[512 * localClientNum], 0, 512 * sizeof(fake_centity_s));   // nx-port: was 0x65800 (x86)
    CG_InitFakeEntities(localClientNum, 1);
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->predictedPlayerEntity.pose.localClientNum = localClientNum;
    R_InitShaderConstantSet(&LocalClientGlobals->predictedPlayerEntity.pose.constantSet);
}

void __cdecl CG_InitViewDimensions(int localClientNum)
{
    cgs_t *cgs; // [esp+8h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgs->viewX = 0;
    cgs->viewX = 0;
    CL_GetScreenDimensions(&cgs->viewWidth, &cgs->viewHeight, &cgs->viewAspect);
    if ( cgs->viewWidth <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2509,
                    1,
                    "%s\n\t(cgs->viewWidth) = %i",
                    "(cgs->viewWidth > 0)",
                    cgs->viewWidth) )
    {
        __debugbreak();
    }
    if ( cgs->viewHeight <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2510,
                    1,
                    "%s\n\t(cgs->viewHeight) = %i",
                    "(cgs->viewHeight > 0)",
                    cgs->viewHeight) )
    {
        __debugbreak();
    }
    if ( cgs->viewAspect <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2511,
                    1,
                    "%s\n\t(cgs->viewAspect) = %g",
                    "(cgs->viewAspect > 0)",
                    cgs->viewAspect) )
    {
        __debugbreak();
    }
}

void __cdecl CG_InitDof(GfxDepthOfField *dof)
{
    dof->nearStart = 0.0f;
    dof->nearEnd = 0.0f;
    dof->farStart = 5000.0f;
    dof->farEnd = 5000.0f;
    dof->nearBlur = 6.0f;
    dof->farBlur = 0.0f;
}

int CGScr_LoadScriptsAndAnims()
{
    int address[128]; // [esp+0h] [ebp-218h] BYREF
    const char *gametype; // [esp+204h] [ebp-14h]
    ScriptFunctions functions; // [esp+208h] [ebp-10h] BYREF
    const char *mapname; // [esp+214h] [ebp-4h]

    functions.maxSize = 128;
    functions.count = 0;
    functions.address = address;
    mapname = Dvar_GetString("mapname");
    gametype = Dvar_GetString("g_gametype");
    CGScr_LoadScripts(mapname, gametype, &functions);
    BG_LoadAnim(mapname);
    BG_PostLoadAnim(mapname);
#ifdef KISAK_SP
    CG_CaptureRemoteAnimTrees_SP();
#endif
    return functions.count;
}

void __cdecl CG_LoadAnimTrees(int localClientNum, cg_s *cgameGlob, const char *mapname, bool loading_scripts)
{
    if ( loading_scripts && com_sv_running->current.enabled )
        CGScr_LoadAnimTrees();
    if ( !cgameGlob->bgs.animData->generic_human.tree.anims && loading_scripts && com_sv_running->current.enabled )
    {
        BG_LoadAnim(mapname);
        BG_PostLoadAnim(mapname);
    }
    if ( !cgameGlob->bgs.animData->generic_human.tree.anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    2816,
                    0,
                    "%s",
                    "cgameGlob->bgs.animData->generic_human.tree.anims") )
    {
        __debugbreak();
    }
    CG_LoadAnimTreeInstances(localClientNum);
}

void __cdecl CG_LoadAnimTreeInstances(int localClientNum)
{
    XAnim_s *generic_human; // [esp+0h] [ebp-14h]
    cg_s *cgameGlob; // [esp+4h] [ebp-10h]
    cgs_t *cgs; // [esp+8h] [ebp-Ch]
    XAnim_s *anims; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    generic_human = cgameGlob->bgs.animData->generic_human.tree.anims;

    for ( int i = 0; i < com_maxclients->current.integer; ++i )
        cgameGlob->bgs.clientinfo[i].pXAnimTree = XAnimCreateTree(generic_human, Hunk_AllocXAnimClient);

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    for (int i = 0; i < 4; ++i)
    {
        cgs->corpseinfo[i].pXAnimTree = XAnimCreateTree(generic_human, Hunk_AllocXAnimClient);
        //*(unsigned int *)&cgs->corpseinfo[1480 * ia + 1332] = (unsigned int)XAnimCreateTree(generic_human, Hunk_AllocXAnimClient);
    }

#ifdef KISAK_SP
    // SP actors are not dogs. MP ships one AI species so this hardcoded the 60-entry
    // DOG_ANIMS table; SP animscripts index against the "generic_human" tree instead, and
    // feeding a dog tree to one tripped "animIndex < anims->size" (xanim.cpp) the moment an
    // actor played its first animation. "DOG_ANIMS" and "generic_dog" have ZERO hits in the
    // SP binary; SP dogs get animtrees/dog.atr instead, installed per-entity by
    // animscripts/dog_init.gsc's `self useAnimTree( #animtree )`, which goes through
    // G_SetAnimTree and replaces ent->pAnimTree -- not through this shared pointer.
    // Client mirror of G_LoadAnimTreeInstances; same lockstep requirement.
    anims = BG_GetActorAnims();
#else
    anims = Dog_GetAnims();
#endif

    iassert(anims);

    for ( int i = 0; i < MAX_ACTORS; ++i )
        cgameGlob->bgs.actorinfo[i].pXAnimTree = XAnimCreateTree(anims, Hunk_AllocXAnimClient);

    for ( int i = 0; i < 8; ++i )
    {
        cgs->actorCorpseInfo[i].pXAnimTree = XAnimCreateTree(anims, Hunk_AllocXAnimClient);
        cgs->actorCorpseInfo[i].entityNum = -1;
        //cgs->actorCorpseInfo[ic + 1].animInfo.legs.yawing = (int)XAnimCreateTree(anims, Hunk_AllocXAnimClient);
        //cgs->actorCorpseInfo[ic].animInfo.legs.animation = (animation_s *)-1;
    }
}

void __cdecl CG_SetupGameInformation(int localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->matchUIVisibilityFlags = cls.gameState.matchUIVisibilityFlags;
}

void __cdecl CG_Shutdown(int localClientNum)
{
#ifdef KISAK_SP
    CG_ClearRemoteAnimTrees_SP();
#endif
    colgeom_visitor_inlined_t<200> *v1; // [esp+0h] [ebp-28h]
    int i; // [esp+14h] [ebp-14h]
    cg_s *cgameGlob; // [esp+18h] [ebp-10h]
    centity_s *cent; // [esp+1Ch] [ebp-Ch]
    int entnum; // [esp+24h] [ebp-4h]

    g_ropesWithEntsAnchorsCount = 0;
    g_ropeCount = 0;

    for (i = 0; i < 1; ++i)
    {
        //p_proximity_data = &cg_pmove[i].proximity_data;
        //colgeom_visitor_inlined_t<500>::reset(p_proximity_data);
        cg_pmove[i].proximity_data.reset();
    }

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    destroy_client_gjkcc_info(localClientNum);
    R_TrackStatistics(0);
    SND_FadeIn();
    for ( entnum = 0; entnum < 1024; ++entnum )
    {
        cent = CG_GetEntity(localClientNum, entnum);
        if ( cent->pose.ragdollHandle > 0 )
        {
            Ragdoll_Remove(cent->pose.ragdollHandle);
            cent->pose.ragdollHandle = 0;
        }
        if ( cent->pose.physObjId )
        {
            if ( cent->pose.physObjId != -1 )
            {
                Phys_ObjDestroy(0, cent->pose.physObjId);
                cent->pose.physObjId = 0;
            }
        }
    }
    g_ambientStarted = 0;
    g_mapLoaded = 0;
    Mantle_ShutdownAnims();
    Dog_ShutdownAnims();
    if ( !useFastFile->current.enabled )
        Menus_FreeAllMemory(&cgDC[localClientNum]);
    CG_FreeWeapons(localClientNum);
    CG_FreeClientEntityCaches(localClientNum);
    CG_FreeClientDObjInfo(localClientNum);
    CG_FreeEntityDObjInfo(localClientNum);
    CG_GetLocalClientStaticGlobals(localClientNum);
    CG_ShutdownEntities(localClientNum);
    CG_ShutdownEntity(localClientNum, &cgameGlob->predictedPlayerEntity, 1);
    FX_KillAllEffects(localClientNum);
    FX_ShutdownSystem(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    CG_ShutdownFakeEntities(localClientNum);
    GlassCl_Reset(localClientNum);
    num_heli_height_lock_patches = 0;
    CG_FreeAnimTreeInstances(localClientNum);
    cgameGlob->nextSnap = 0;
    memset((unsigned __int8 *)cgameGlob, 0, sizeof(cg_s));
    if ( cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    3408,
                    0,
                    "%s",
                    "!cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
}

void __cdecl CG_FreeAnimTreeInstances(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    cgs_t *cgs; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for (i = 0; i < com_maxclients->current.integer; ++i)
    {
        if (cgameGlob->bgs.clientinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgameGlob->bgs.clientinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgameGlob->bgs.clientinfo[i].pXAnimTree = 0;
        }
    }
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    for (i = 0; i < 4; ++i)
    {
        if (cgs->corpseinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgs->corpseinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgs->corpseinfo[i].pXAnimTree = 0;
        }
    }
    for (i = 0; i < MAX_ACTORS; ++i)
    {
        if (cgameGlob->bgs.actorinfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgameGlob->bgs.actorinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgameGlob->bgs.actorinfo[i].pXAnimTree = 0;
        }
    }
    for (i = 0; i < 8; ++i)
    {
        if (cgs->actorCorpseInfo[i].pXAnimTree)
        {
            XAnimFreeTree(cgs->actorCorpseInfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            cgs->actorCorpseInfo[i].pXAnimTree = 0;
        }
    }
}

void __cdecl CG_ShutdownOnceForAllClients()
{
    R_Cinematic_StopPlayback();
    Com_ShutdownDynamicMemorySystems();
    SND_GameReset();
    CG_SndGameReset();
    Ragdoll_Shutdown();
    Com_FreeWeaponInfoMemory(2);
    Scr_ShutdownSystem(SCRIPTINSTANCE_CLIENT, 1u, 0);
    CScr_FreeScripts();
    Scr_FreeScripts(SCRIPTINSTANCE_CLIENT, 1u);
    Scr_ShutdownGameStrings(SCRIPTINSTANCE_CLIENT);
    Scr_FreeEntityList(SCRIPTINSTANCE_CLIENT);
    if ( !com_sv_running->current.enabled )
        Scr_ShutdownGameStrings(SCRIPTINSTANCE_SERVER);
    BG_ShutdownFire();
    CG_ClearCompassPingData();
    CG_ShutdownConsoleCommands();
    CG_ResetClientInitializationState();
}

void __cdecl CG_ProcessTriggerDebug(centity_s *ent, trigger_info_t *trigger_info)
{
    const char *v2; // [esp+24h] [ebp-14h]
    centity_s *other; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    if ( cg_usedTriggerCount < 300 )
    {
        for ( i = 0; i < cg_usedTriggerCount; ++i )
        {
            if ( cg_usedTriggers[i] == ent->nextState.number )
                return;
        }
        if ( cg_debug_triggers->current.enabled )
        {
            if ( cg_usedTriggerCount >= 300 )
            {
                Com_Printf(5, "CTrigger: Max Triggers Process\n");
                return;
            }
            if ( trigger_info )
                v2 = "";
            else
                v2 = "Immediate Notify Maxed!! ";
            Com_Printf(
                5,
                "CTrigger: %d (%g %g %g) %sIdx %d/%d\n",
                ent->nextState.number,
                ent->currentState.pos.trBase[0],
                ent->currentState.pos.trBase[1],
                ent->currentState.pos.trBase[2],
                v2,
                cg_level.entTriggerIndex[ent->nextState.number],
                cg_level.triggerIndex);
            if ( trigger_info )
            {
                other = CG_GetEntity(0, trigger_info->otherEntnum);
                if ( ent->nextState.lerp.useCount != trigger_info->useCount
                    || other->nextState.lerp.useCount != trigger_info->otherUseCount )
                {
                    Com_Printf(
                        5,
                        "CTrigger notify not being sent because useCount. Trig Info %d, Trig %d, Trig Info Other %d, Other %d\n",
                        trigger_info->useCount,
                        ent->nextState.lerp.useCount,
                        trigger_info->otherUseCount,
                        other->nextState.lerp.useCount);
                }
                if ( cg_level.entTriggerIndex[ent->nextState.number] == cg_level.triggerIndex )
                    Com_Printf(5, "CTrigger already triggered this frame loop\n");
            }
        }
        cg_usedTriggers[cg_usedTriggerCount++] = ent->nextState.number;
    }
}

int __cdecl CG_NotifyTriggers()
{
    trigger_info_t *v0; // ecx
    trigger_info_t *trigger_info; // [esp+14h] [ebp-18h]
    unsigned int *other; // [esp+18h] [ebp-14h]
    centity_s *ent; // [esp+1Ch] [ebp-10h]
    int bMoreTriggered; // [esp+20h] [ebp-Ch]
    int entnum; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    bMoreTriggered = 0;
    ++cg_level.triggerIndex;
    for ( i = 0; i < cg_level.currentTriggerListSize; ++i )
    {
        trigger_info = &cg_level.currentTriggerList[i];
        entnum = trigger_info->entnum;
        ent = CG_GetEntity(0, entnum);
        CG_ProcessTriggerDebug(ent, trigger_info);
        if ( ent->nextState.lerp.useCount == trigger_info->useCount )
        {
            other = (unsigned int *)CG_GetEntity(0, trigger_info->otherEntnum);
            if ( other[151] == trigger_info->otherUseCount
                && ((ent->clientFlags >> 1) & 1) != 0
                && ((other[201] >> 1) & 1) != 0 )
            {
                if ( cg_level.entTriggerIndex[entnum] == cg_level.triggerIndex )
                {
                    bMoreTriggered = 1;
                    continue;
                }
                cg_level.entTriggerIndex[entnum] = cg_level.triggerIndex;
                CScr_AddEntity(ent, 0);
                CScr_NotifyNum(0, other[122], 0, cscr_const.trigger, 1u);
            }
        }
        --cg_level.currentTriggerListSize;
        --i;
        v0 = &cg_level.currentTriggerList[cg_level.currentTriggerListSize];
        *(unsigned int *)&trigger_info->entnum = *(unsigned int *)&v0->entnum;
        trigger_info->useCount = v0->useCount;
        trigger_info->otherUseCount = v0->otherUseCount;
    }
    return bMoreTriggered;
}

void __cdecl CG_Trigger(centity_s *self, centity_s *other)
{
    trigger_info_t *trigger_info; // [esp+4h] [ebp-4h]

    if ( ((other->clientFlags >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_main_mp.cpp",
                    3630,
                    0,
                    "%s",
                    "other->nextValid") )
    {
        __debugbreak();
    }
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_CLIENT) )
    {
        if ( cg_level.pendingTriggerListSize == 256 )
        {
            CG_ProcessTriggerDebug(self, 0);
            CScr_AddEntity(other, 0);
            CScr_NotifyNum(0, self->nextState.number, 0, cscr_const.trigger, 1u);
        }
        else
        {
            trigger_info = &cg_level.pendingTriggerList[cg_level.pendingTriggerListSize++];
            trigger_info->entnum = self->nextState.number;
            trigger_info->otherEntnum = other->nextState.number;
            trigger_info->useCount = self->nextState.lerp.useCount;
            trigger_info->otherUseCount = other->nextState.lerp.useCount;
        }
    }
}

void __cdecl CG_multi_trigger(centity_s *ent)
{
    if ( ((ent->clientFlags >> 17) & 1) != 0 )
        CG_FreeEntityDelay(ent);
}

void __cdecl CG_Touch_Multi(centity_s *self, centity_s *other)
{
    if ( ((other->clientFlags >> 1) & 1) != 0 )
    {
        CG_Trigger(self, other);
        CG_multi_trigger(other);
    }
}

void *__cdecl Hunk_AllocXAnimClient(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXAnimClient", 13);
}

