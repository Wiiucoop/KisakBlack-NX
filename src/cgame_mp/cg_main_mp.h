#pragma once
#include "cg_local_mp.h"
#include <win32/win_common.h>
#include <bgame/bg_local.h>
#include <clientscript/cscr_main.h>
#include <client_mp/cl_main_mp.h>
#include <ui/ui_main.h>
#include <xanim/dobj.h>

struct clientInfo_t;
struct GfxDepthOfField;
struct cg_s;
struct trigger_info_t;

struct BattleChatterParams // sizeof=0x20
{                                       // XREF: .data:BattleChatterParams * cg_BattleChatters/r
    unsigned __int8 WhichSoundIsPlaying;
                                        // XREF: CG_PlayBattleChatter(int,int,float const * const,int,int)+57/r
                                        // CG_PlayBattleChatter(int,int,float const * const,int,int)+86/w ...
    // padding byte
    // padding byte
    // padding byte
    unsigned int SecondAlias;           // XREF: CG_PlayBattleChatter(int,int,float const * const,int,int)+A5/w
                                        // CG_CheckBattleChatter(void)+67/r
    int EntityNum;                      // XREF: CG_PlayBattleChatter(int,int,float const * const,int,int)+7A/w
                                        // CG_CheckBattleChatter(void)+8B/r
    int LocalClientNum;                 // XREF: CG_PlayBattleChatter(int,int,float const * const,int,int)+96/w
                                        // CG_CheckBattleChatter(void)+98/r
    int CurrentPlayingSound;            // XREF: CG_PlayBattleChatter(int,int,float const * const,int,int)+6B/w
                                        // CG_CheckBattleChatter(void)+4B/r ...
    float SndOrigin[3];                 // XREF: CG_PlayBattleChatter(int,int,float const * const,int,int)+B1/o
                                        // CG_CheckBattleChatter(void)+7E/o
};

int __cdecl CG_GetClientNumForLocalClient(int localClientNum);
bool __cdecl CG_IsRagdollTrajectory(const trajectory_t *trajectory);
void __cdecl CG_SetupSplitscreenDvars();
void __cdecl CG_RegisterDvars();
int __cdecl CG_AllocateClientMemory_SizeRequired(int maxLocalClients);
void __cdecl CG_AllocateClientMemory(HunkUser *hunk, int maxLocalClients);
void __cdecl CG_FreeClientMemory(HunkUser *hunk, int maxLocalClients);
void __cdecl CG_GetDObjOrientation(int localClientNum, int dobjHandle, float (*axis)[3], float *origin);
const playerState_s *__cdecl CG_GetPredictedPlayerState(int localClientNum);
void __cdecl CG_GameMessage(int localClientNum, const char *msg);
void __cdecl CG_BoldGameMessage(int localClientNum, const char *msg, int duration);
bool __cdecl CG_IsVehicleMayhemGameType();
void __cdecl CG_RegisterSounds();
void __cdecl CG_RegisterSurfaceTypeSounds(const char *pszType, unsigned int *sound);
void __cdecl CG_RegisterImpactTypeSounds(const char *pszType, unsigned int *sound);
void __cdecl CG_PlayBattleChatter(
                int localClientNum,
                int entitynum,
                float *origin,
                unsigned int firstSoundAlias,
                int secondSoundAlias);
void __cdecl CG_CheckBattleChatter();
void __cdecl CG_RestartSmokeGrenades(int localClientNum);
void __cdecl CG_InitVote(int localClientNum);
unsigned __int16 __cdecl CG_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType, weapInventoryType_t invType);
void __cdecl CGScr_LoadGameTypeScript(scriptInstance_t inst, const char *gametype, ScriptFunctions *functions);
char __cdecl CGScr_LoadScriptAndLabel(
                scriptInstance_t inst,
                const char *filename,
                const char *label,
                ScriptFunctions *functions);
void __cdecl CGScr_LoadScripts(const char *mapname, const char *gametype, ScriptFunctions *functions);
void __cdecl CGScr_LoadLevelScript(scriptInstance_t inst, const char *mapname, ScriptFunctions *functions);
void __cdecl CGScr_LoadClientScripts(const char *mapname, ScriptFunctions *functions);
void __cdecl CGScr_LoadClientScriptsAndAnims();
void __cdecl CG_InitScreenDimensions(int localClientNum);
int __cdecl CG_GetClientNum(int localClientNum);
bool __cdecl CG_IsMature();
bool __cdecl CG_GetEntityOriginAngles(int localClientNum, int entityNum, float *origin, float *angles);
unsigned __int16 __cdecl CG_GetVehicleTypeString(int clientNum, int entityNum);
int __cdecl CachedTag_GetCachedTagPos(
                const centity_s *ent,
                cached_client_tag_t *cachedTag,
                unsigned int tagName,
                float *pos,
                int updateInterval,
                bool forceUpdate);
int __cdecl CachedTag_UpdateTagInternal(
                const centity_s *ent,
                cached_client_tag_t *cachedTag,
                unsigned int tagName,
                int updateInterval,
                bool forceUpdate);
void __cdecl CachedTag_CalcTagPos(const centity_s *ent, cached_client_tag_t *cachedTag, float *pos);
int __cdecl CachedTag_NoCache_GetTagPos(const centity_s *ent, unsigned int tagName, float *pos);
int __cdecl CachedTag_GetTagPos(
                const centity_s *ent,
                unsigned int tagName,
                float *pos,
                int updateInterval,
                bool forceUpdate);
void __cdecl CG_InitClientEntityCaches(int localClientNum);
void __cdecl CG_FreeClientEntityCaches(int localClientNum);
void __cdecl CG_Init(int localClientNum, int serverMessageNum, int serverCommandSequence, int clientNum);
clientConnection_t *__cdecl CL_GetLocalClientConnection(int localClientNum);
void __cdecl CG_RegisterGraphics(int localClientNum, const char *mapname);
void __cdecl CG_LoadHudMenu(int localClientNum);
const rectDef_s *__cdecl Window_GetRect(const windowDef_t *w);
unsigned __int16 __cdecl CG_AttachWeapon(DObjModel_s *dobjModels, unsigned __int16 numModels, clientInfo_t *ci);
void __cdecl CG_CreateDObj(
                DObjModel_s *dobjModels,
                unsigned __int16 numModels,
                XAnimTree_s *tree,
                int handle,
                int localClientNum,
                clientInfo_t *ci);
DObj *__cdecl CG_GetDObj(unsigned int handle, int localClientNum);
void __cdecl CG_InitEntities(int localClientNum);
void __cdecl CG_InitViewDimensions(int localClientNum);
void __cdecl CG_InitDof(GfxDepthOfField *dof);
int CGScr_LoadScriptsAndAnims();
void __cdecl CG_LoadAnimTrees(int localClientNum, cg_s *cgameGlob, const char *mapname, bool loading_scripts);
void __cdecl CG_LoadAnimTreeInstances(int localClientNum);
void __cdecl CG_SetupGameInformation(int localClientNum);
void __cdecl CG_Shutdown(int localClientNum);
void __cdecl CG_FreeAnimTreeInstances(int localClientNum);
void __cdecl CG_ShutdownOnceForAllClients();
void __cdecl CG_ProcessTriggerDebug(centity_s *ent, trigger_info_t *trigger_info);
int __cdecl CG_NotifyTriggers();
void __cdecl CG_Trigger(centity_s *self, centity_s *other);
void __cdecl CG_multi_trigger(centity_s *ent);
void __cdecl CG_Touch_Multi(centity_s *self, centity_s *other);
void *__cdecl Hunk_AllocXAnimClient(unsigned int size);

extern struct cgMedia_t cgMedia;

extern bool cg_fakeEntitiesInuseArray[512];
extern int cg_fakeEntitiesInuseCount[1];
extern int cg_fakeEntitiesInuseCountFromMap;
extern int cg_fakeEntitiesInuseCountFromLoadScript;

extern const dvar_s *cg_loadScripts;
extern const dvar_s *cg_usingClientScripts;
extern const dvar_s *cg_drawGun;
extern const dvar_s *cg_cursorHints;
extern const dvar_s *cg_retrieveHintTime;
extern const dvar_s *cg_retrieveHintTimeStuck;
extern const dvar_s *cg_weaponHintsCoD1Style;
extern const dvar_s *cg_hintFadeTime;
extern const dvar_s *cg_seatHintFadeTime;
extern const dvar_s *cg_fov;
extern const dvar_s *cg_fov_default;
extern const dvar_s *cg_fov_default_thirdperson;
extern const dvar_s *cg_fovScale;
extern const dvar_s *cg_fovMin;
extern const dvar_s *cg_fovExtraCam;
extern const dvar_s *cg_fovCompMax;
extern const dvar_s *cg_adsZoomToggleStyle;
extern const dvar_s *cg_viewVehicleInfluenceGunner;
extern const dvar_s *cg_viewVehicleInfluenceGunnerFiring;
extern const dvar_s *cg_draw2D;
extern const dvar_s *cg_drawErrorMessages;
extern const dvar_s *cg_drawHealth;
extern const dvar_s *cg_drawBreathHint;
extern const dvar_s *cg_drawMantleHint;
extern const dvar_s *cg_wadefps;
extern const dvar_s *cg_drawFPS;
extern const dvar_s *cg_drawFPSScale;
extern const dvar_s *cg_drawBudgets;
extern const dvar_s *cg_drawDynSModelBudget;
extern const dvar_s *cg_development;
extern const dvar_s *cg_drawAnimAttachTags;
extern const dvar_s *cg_drawFPSOnly;
extern const dvar_s *cg_profile_physics;
extern const dvar_s *cg_drawFPSLabels;
extern const dvar_s *cg_debugInfoCornerOffset;
extern const dvar_s *cg_drawVersion;
extern const dvar_s *cg_drawVersionX;
extern const dvar_s *cg_drawVersionY;
extern const dvar_s *cg_readTitleStorageLocally;
extern const dvar_s *snd_drawInfo;
extern const dvar_s *snd_drawSort;
extern const dvar_s *cg_drawScriptUsage;
extern const dvar_s *cg_drawMaterial;
extern const dvar_s *cg_drawModelAxis;
extern const dvar_s *cg_drawSnapshot;
extern const dvar_s *cg_drawSnapshotTime;
extern const dvar_s *cg_drawCrosshair;
extern const dvar_s *cg_drawCrosshair3D;
extern const dvar_s *cg_drawHoldBreathHint;
extern const dvar_s *cg_drawTurretCrosshair;
extern const dvar_s *cg_drawCrosshairNames;
extern const dvar_s *cg_drawCrosshairNamesPosX;
extern const dvar_s *cg_drawCrosshairNamesPosY;
extern const dvar_s *cg_drawShellshock;
extern const dvar_s *cg_hudStanceFlash;
extern const dvar_s *cg_hudStanceHintPrints;
extern const dvar_s *cg_hudDamageIconWidth;
extern const dvar_s *cg_hudDamageIconHeight;
extern const dvar_s *cg_hudDamageIconOffset;
extern const dvar_s *cg_hudDamageIconTime;
extern const dvar_s *cg_hudDamageDirectionalIconTime;
extern const dvar_s *cg_hudDamageIconInScope;
extern const dvar_s *cg_hudGrenadeIconMaxRangeFrag;
extern const dvar_s *cg_hudGrenadeIconMaxRangeFlash;
extern const dvar_s *cg_hudGrenadeIconMaxHeight;
extern const dvar_s *cg_hudGrenadeIconInScope;
extern const dvar_s *cg_hudGrenadeIconOffset;
extern const dvar_s *cg_hudGrenadeIconHeight;
extern const dvar_s *cg_hudGrenadeIconWidth;
extern const dvar_s *cg_hudGrenadeIconEnabledFlash;
extern const dvar_s *cg_hudGrenadePointerHeight;
extern const dvar_s *cg_hudGrenadePointerWidth;
extern const dvar_s *cg_hudGrenadePointerPivot;
extern const dvar_s *cg_hudGrenadePointerPulseFreq;
extern const dvar_s *cg_hudGrenadePointerPulseMax;
extern const dvar_s *cg_hudGrenadePointerPulseMin;
extern const dvar_s *cg_hudChatPosition;
extern const dvar_s *cg_hudSayPosition;
extern const dvar_s *cg_hudChatIntermissionPosition;
extern const dvar_s *cg_hudVotePosition;
extern const dvar_s *cg_debugDrawSafeAreas;
extern const dvar_s *cg_drawLagometer;
extern const dvar_s *drawEntityCount;
extern const dvar_s *drawEntityCountPos;
extern const dvar_s *drawEntityCountSize;
extern const dvar_s *drawServerBandwidth;
extern const dvar_s *drawServerBandwidthPos;
extern const dvar_s *drawServerBandwidthSize;
extern const dvar_s *drawKillcamData;
extern const dvar_s *drawKillcamDataPos;
extern const dvar_s *drawKillcamDataSize;
extern const dvar_s *cg_hudProneY;
extern const dvar_s *cg_mapLocationSelectionCursorSpeed;
extern const dvar_s *cg_mapLocationSelectionRotationSpeed;
extern const dvar_s *cg_hudGrenadeIndicatorFadeUp;
extern const dvar_s *cg_hudGrenadeIndicatorTargetColor;
extern const dvar_s *cg_hudGrenadeIndicatorStartColor;
extern const dvar_s *cg_weaponCycleDelay;
extern const dvar_s *cg_crosshairAlpha;
extern const dvar_s *cg_crosshairAlphaMin;
extern const dvar_s *cg_crosshairDynamic;
extern const dvar_s *cg_crosshairEnemyColor;
extern const dvar_s *cg_brass;
extern const dvar_s *cg_gun_fovcomp_x;
extern const dvar_s *cg_gun_fovcomp_y;
extern const dvar_s *cg_gun_fovcomp_z;
extern const dvar_s *cg_gun_x;
extern const dvar_s *cg_gun_y;
extern const dvar_s *cg_gun_z;
extern const dvar_s *cg_gun_move_f;
extern const dvar_s *cg_gun_move_r;
extern const dvar_s *cg_gun_move_u;
extern const dvar_s *cg_gun_ofs_f;
extern const dvar_s *cg_gun_ofs_r;
extern const dvar_s *cg_gun_ofs_u;
extern const dvar_s *cg_gun_move_rate;
extern const dvar_s *cg_gun_move_minspeed;
extern const dvar_s *cg_centertime;
extern const dvar_s *cg_debugPosition;
extern const dvar_s *cg_debugEvents;
extern const dvar_s *cg_errorDecay;
extern const dvar_s *cg_nopredict;
extern const dvar_s *cg_showmiss;
extern const dvar_s *cg_footsteps;
extern const dvar_s *cg_footprints;
extern const dvar_s *cg_footprintsDistortWater;
extern const dvar_s *cg_footprintsDebug;
extern const dvar_s *cg_waterTrailRippleFrequency;
extern const dvar_s *cg_waterTrailRippleVariance;
extern const dvar_s *cg_treadmarks;
extern const dvar_s *cg_firstPersonTracerChance;
extern const dvar_s *cg_laserForceOn;
extern const dvar_s *cg_laserRange;
extern const dvar_s *cg_laserRangePlayer;
extern const dvar_s *cg_laserRadius;
extern const dvar_s *cg_laserLight;
extern const dvar_s *cg_laserLightBodyTweak;
extern const dvar_s *cg_laserLightRadius;
extern const dvar_s *cg_laserLightBeginOffset;
extern const dvar_s *cg_laserEndOffset;
extern const dvar_s *cg_laserLightEndOffset;
extern const dvar_s *cg_laserFlarePct;
extern const dvar_s *cg_marks_ents_player_only;
extern const dvar_s *cg_tracerChance;
extern const dvar_s *cg_tracerWidth;
extern const dvar_s *cg_tracerSpeed;
extern const dvar_s *cg_tracerLength;
extern const dvar_s *cg_tracerNoDrawTime;
extern const dvar_s *cg_tracerScale;
extern const dvar_s *cg_tracerScaleMinDist;
extern const dvar_s *cg_tracerScaleDistRange;
extern const dvar_s *cg_tracerScrewDist;
extern const dvar_s *cg_tracerScrewRadius;
extern const dvar_s *cg_bulletWidth;
extern const dvar_s *cg_bulletLength;
extern const dvar_s *cg_thirdPersonRange;
extern const dvar_s *cg_thirdPersonAngle;
extern const dvar_s *cg_thirdPersonFocusDist;
extern const dvar_s *cg_thirdPerson;
extern const dvar_s *cg_thirdPersonMode;
extern const dvar_s *cg_chatTime;
extern const dvar_s *cg_chatHeight;
extern const dvar_s *cg_predictItems;
extern const dvar_s *cg_spectateThirdPerson;
extern const dvar_s *cg_teamChatsOnly;
extern const dvar_s *cg_use_colored_smoke;
extern const dvar_s *cg_fakefireWizbyChance;
extern const dvar_s *cg_paused;
extern const dvar_s *cg_drawpaused;
extern const dvar_s *cg_synchronousClients;
extern const dvar_s *cg_debug_overlay_viewport;
extern const dvar_s *cg_fs_debug;
extern const dvar_s *cg_debugFace;
extern const dvar_s *cg_dumpAnims;
extern const dvar_s *cg_developer;
extern const dvar_s *cg_minicon;
extern const dvar_s *cg_subtitles;
extern const dvar_s *cg_subtitleMinTime;
extern const dvar_s *cg_subtitleWidthStandard;
extern const dvar_s *cg_subtitleWidthWidescreen;
extern const dvar_s *cg_gameMessageWidth;
extern const dvar_s *cg_gameBoldMessageWidth;
extern const dvar_s *cg_descriptiveText;
extern const dvar_s *cg_youInKillCamSize;
extern const dvar_s *cg_scriptIconSize;
extern const dvar_s *cg_connectionIconSize;
extern const dvar_s *cg_voiceIconSize;
extern const dvar_s *cg_constantSizeHeadIcons;
extern const dvar_s *cg_headIconMinScreenRadius;
extern const dvar_s *cg_overheadNamesMaxDist;
extern const dvar_s *cg_overheadNamesNearDist;
extern const dvar_s *cg_overheadNamesFarDist;
extern const dvar_s *cg_overheadNamesFarScale;
extern const dvar_s *cg_overheadNamesSize;
extern const dvar_s *cg_overheadIconSize;
extern const dvar_s *cg_overheadRankSize;
extern const dvar_s *cg_overheadNamesGlow;
extern const dvar_s *cg_overheadNamesFont;
extern const dvar_s *cg_drawFriendlyNames;
extern const dvar_s *cg_enemyNameFadeIn;
extern const dvar_s *cg_friendlyNameFadeIn;
extern const dvar_s *cg_enemyNameFadeOut;
extern const dvar_s *cg_friendlyNameFadeOut;
extern const dvar_s *cg_drawThroughWalls;
extern const dvar_s *cg_playerHighlightTargetSize;
extern const dvar_s *cg_playerHighlightEnemyColor;
extern const dvar_s *cg_playerHighlightBrightness;
extern const dvar_s *cg_playerHighlightMinFade;
extern const dvar_s *cg_playerHighlightBlinkTime;
extern const dvar_s *cg_corpseHighlightFadeTime;
extern const dvar_s *cg_cameraSpikeHighlightBrightness;
extern const dvar_s *cg_cameraSpikeEnemyColor;
extern const dvar_s *cg_adsZScaleMax;
extern const dvar_s *cg_infraredHighlightScale;
extern const dvar_s *cg_infraredHighlightOffset;
extern const dvar_s *cg_allow_mature;
extern const dvar_s *cg_mature;
extern const dvar_s *cg_blood;
extern const dvar_s *cg_invalidCmdHintDuration;
extern const dvar_s *cg_invalidCmdHintBlinkInterval;
extern const dvar_s *cg_viewZSmoothingMin;
extern const dvar_s *cg_viewZSmoothingMax;
extern const dvar_s *cg_viewZSmoothingTime;
extern const dvar_s *overrideNVGModelWithKnife;
extern const dvar_s *cg_visionSetLerpMaxIncreasePerFrame;
extern const dvar_s *cg_visionSetLerpMaxDecreasePerFrame;
extern const dvar_s *cg_flareVisionSetFadeDuration;
extern const dvar_s *cg_turretBipodOffset;
extern const dvar_s *cg_AllPlayerNamesVisible;
extern const dvar_s *cg_ScoresColor_MyTeam;
extern const dvar_s *cg_ScoresColor_EnemyTeam;
extern const dvar_s *cg_ScoresColor_Spectator;
extern const dvar_s *cg_ScoresColor_Free;
extern const dvar_s *cg_ScoresColor_Allies;
extern const dvar_s *cg_ScoresColor_Axis;
extern const dvar_s *cg_TeamName_Allies;
extern const dvar_s *cg_TeamName_Axis;
extern const dvar_s *cg_TeamColor_Allies;
extern const dvar_s *cg_TeamColor_Axis;
extern const dvar_s *cg_TeamColor_MyTeam;
extern const dvar_s *cg_TeamColor_EnemyTeam;
extern const dvar_s *cg_TeamColor_MyTeamAlt;
extern const dvar_s *cg_TeamColor_EnemyTeamAlt;
extern const dvar_s *cg_TeamColor_Squad;
extern const dvar_s *cg_TeamColor_Spectator;
extern const dvar_s *cg_TeamColor_Free;
extern const dvar_s *cg_proneFeetCollisionHull;
extern const dvar_s *g_compassShowEnemies;
extern const dvar_s *cg_drawWVisDebug;
extern const dvar_s *debugOverlay;
extern const dvar_s *cg_motionblur_duration;
extern const dvar_s *cg_motionblur_fadeout;
extern const dvar_s *cg_timedDamageDuration;
extern const dvar_s *cg_MinDownedPulseRate;
extern const dvar_s *cg_MaxDownedPulseRate;
extern const dvar_s *cg_playerFrustumHalfHeight;
extern const dvar_s *cg_overheadNamesTagUpdateInterval;
extern const dvar_s *cg_canSeeFriendlyFrustumUpdateInterval;
extern const dvar_s *cg_canSeeFriendlyFrustumExpand;
extern const dvar_s *cg_canSeeFriendlyFrustumMinDistance;
extern const dvar_s *cg_watersheeting;
extern const dvar_s *cg_debug_triggers;
extern const dvar_s *cg_cameraWaterClip;
#ifdef KISAK_SP
extern const dvar_s *cg_cameraUseTagCamera;
#endif
extern const dvar_s *cg_cameraVehicleExitTweenTime;
extern const dvar_s *cg_vehicle_piece_damagesfx_threshold;
extern const dvar_s *cg_debugLocHit;
extern const dvar_s *cg_debugLocHitTime;

extern cg_s *cgArray;
extern cgs_t *cgsArray;

extern float (*cg_entityOriginArray[1])[3];