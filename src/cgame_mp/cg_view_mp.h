#pragma once

#include <cgame/cg_camera.h>

#define POF_FOLLOW 2

enum ClientViewViewportSize : __int32
{                                       // XREF: ClientViewParams/r
    VIEWPORT_LARGE = 0x0,               // XREF: .data:clientViewParamsArray/s
    VIEWPORT_SMALL = 0x1,
};

enum KillCamEntityType : __int32
{                                       // XREF: cg_s/r
                                        // CG_UpdateKillCamEntity/r ...
    KC_NO_ENTITY      = 0x0,
    KC_HELICOPTER     = 0x1,
    KC_DESTRUCTIBLE   = 0x2,
    KC_SCRIPTED       = 0x3,
    KC_EXPLOSIVE      = 0x4,
    KC_FAST_EXPLOSIVE = 0x5,
    KC_ROCKET         = 0x6,
    KC_DOG            = 0x7,
    KC_ARTILLERY      = 0x8,
    KC_VEHICLE        = 0x9,
    KC_TURRET         = 0xA,
};

enum KillCamEntityRestState : __int32
{                                       // XREF: cg_s/r
                                        // CG_KillcamCameraTrace/r
    KC_ENT_MOVING       = 0x0,
    KC_ENT_AT_REST      = 0x1,
    KC_ENT_STUCK_GROUND = 0x2,
    KC_ENT_STUCK_WALL   = 0x3,
};

struct ClientViewParams // sizeof=0x14
{                                       // XREF: .data:clientViewParamsArray/r
    float x;
    float y;
    float width;
    float height;
    ClientViewViewportSize viewportSize;
};

struct TestEffect // sizeof=0x54
{                                       // XREF: .data:s_testEffect/r
    char name[64];
    float pos[3];
    int time;
    int respawnTime;
};

struct vehicle_info_t;
struct centity_s;
struct GfxDepthOfField;
struct GfxDepthcg_sOfField;

enum DemoType : __int32;
enum CubemapShot : __int32;

void __cdecl CG_DrawWaterTrail(int localClientNum, const float *pos, float waterHeight);
int __cdecl CG_GetNextRippleTime(centity_s *cent);
void __cdecl CG_SetNextRippleTime(centity_s *cent, int time, float speed, bool in_water);
int __cdecl CG_GetNextWaterCheck(centity_s *cent);
void __cdecl CG_SetNextWaterCheck(centity_s *cent, int time, bool in_water);
void __cdecl CG_SetCentWaterHeight(centity_s *cent, float height);
double __cdecl CG_GetCentWaterHeight(centity_s *cent);
void __cdecl CG_DrawWaterTrail(int localClientNum, centity_s *cent);
void __cdecl CG_DrawWaterTrail(int localClientNum);
void __cdecl CG_FxSetTestPosition();
void __cdecl CG_FxTest();
void __cdecl CG_PlayTestFx(int localClientNum);
void __cdecl CG_ViewRegisterDvars();
void __cdecl CG_UpdateTurretKillCam(int localClientNum);
void __cdecl CG_GetPlayerKillCamEntityLookAt(int localClientNum, float *lookAtPos);
void __cdecl CG_UpdateTurretKillCam(float distance, GfxDepthOfField *dof);
void __cdecl CG_GetTurretEntityOrgAngles(int localClientNum, float *origin, float *angles);
void __cdecl CG_UpdateVehicleKillCam(int localClientNum);
void __cdecl CG_GetKillCamEntityOrgAngles(int localClientNum, float *origin, float *angles);
const vehicle_info_t *__cdecl CG_GetKillCamVehicleInfo(int localClientNum);
void __cdecl CG_UpdateVehicleKillCamDof(const vehicle_info_t *info, float distance, GfxDepthOfField *dof);
void __cdecl CG_UpdateScriptedKillCam(int localClientNum);
double __cdecl LookAtAxisAndDistance(const float *origin, const float *lookAt, float (*axis)[3]);
void __cdecl CG_UpdateScriptedKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_UpdateDestructibleKillCam(int localClientNum);
void __cdecl CG_UpdateDestructibleKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_UpdateArtilleryKillCam(int localClientNum);
void __cdecl CG_UpdateDogKillCam(int localClientNum);
bool __cdecl CG_KillCamEntityEnabled(int localClientNum);
void __cdecl CG_InitView(int localClientNum);
void __cdecl LerpKillCamView(int localClientNum);
void __cdecl CG_CalcViewValues(int localClientNum);
void __cdecl CG_DevSaveCamera(float *angles, float *origin);
void    CG_OffsetChaseCamView(int localClientNum, CameraMode camMode);
// local variable allocation has failed, the output may be wrong!
void    CG_OffsetThirdPersonView(int localClientNum);
void __cdecl CG_CalcVrect(int localClientNum);
void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob);
#ifdef KISAK_SP
// SP takes localClientNum as well: the tag-camera block appended to the end of
// this function needs it for CG_GetEntity / Com_GetClientDObj. Retail SP's
// CG_OffsetFirstPersonView (0x00792316-0x007923e3) reloads localClientNum from
// [esp+0x84] three times for exactly those three calls. Guarded so the MP
// signature -- and therefore MP's generated code -- is untouched.
void __cdecl CG_OffsetFirstPersonView(int localClientNum, cg_s *cgameGlob);
#else
void __cdecl CG_OffsetFirstPersonView(cg_s *cgameGlob);
#endif
void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob);
void __cdecl CG_CalcTurretViewValues(int localClientNum);
void __cdecl CG_UpdateKillCamEntity(KillCamEntityType killCamEntityType, int localClientNum);
void __cdecl CG_UpdateHelicopterKillCam(KillCamEntityType killCamEntityType, int localClientNum);
void __cdecl CG_UpdateHelicopterKillCamDof(float distance, GfxDepthOfField *dof);
void __cdecl CG_UpdateExplosiveKillCam(int localClientNum, KillCamEntityType killCamEntityType);
double __cdecl LookAtBothPoints(const float *point1, const float *point2, const float *campos, float (*lookaxis)[3]);
void __cdecl CG_KillcamCameraTrace(
                const float *originalBombOrigin,
                const float *bombAngles,
                const float *desiredCamPos,
                int passEntIndex,
                KillCamEntityRestState killCamEntityRestState,
                float *campos);
void __cdecl RewindTrace(trace_t *trace, const float *start, const float *end, float amount);
void __cdecl DebugTraceLine(float *start, float *end, float frac, const float *color);
bool __cdecl CG_ShouldRenderThirdPerson(CameraMode camMode);
void __cdecl CG_UpdateThirdPerson(int localClientNum, bool forExtraCam);
const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int localClientNum);
const ClientViewParams *__cdecl CG_GetLocalClientViewParams(
                int localClientNum,
                int activeClientIndex,
                int activeClientCountArrayIndex);
void __cdecl GetCeilingHeight(cg_s *cgameGlob);
void __cdecl CG_UpdateViewOffset(int localClientNum);
void __cdecl CG_UpdateKillCamEntityViewOffset(int localClientNum);
void __cdecl CG_UpdateRemoteControlEntityViewOffset(int localClientNum);
void __cdecl CG_SetupFlameFX(int localClientNum, int enable);
void __cdecl CG_SetupElectrifiedFX(int localClientNum, int enable);
bool __cdecl CG_IsInfrared(int localClientNum);
bool __cdecl CG_IsTvguided(int localClientNum, bool onlyADS);
int    CG_DrawActiveFrame(
                int localClientNum,
                int serverTime,
                DemoType demoType,
                CubemapShot cubemapShot,
                int cubemapSize,
                int renderScreen);
void __cdecl CG_UpdateTestFX(int localClientNum);
void __cdecl CG_KickAngles(cg_s *cgameGlob);
void __cdecl UpdateKillCamEntityCache(int localClientNum);
void __cdecl CG_CacheKillCamEntityPose(int localClientNum);
void __cdecl CG_CacheKillCamLookAtEntityOrigin(int localClientNum);
void __cdecl CG_UpdateEntInfo(int localClientNum);
void __cdecl CG_UpdateIKTiming(int localClientNum);
void __cdecl DumpAnims(int localClientNum);
void __cdecl DrawShellshockBlend(int localClientNum);
void __cdecl CG_UpdateTimeScale(int localClientNum);
void __cdecl CG_UpdateSceneDepthOfField(int localClientNum);
void __cdecl CG_UpdateAdsDof(int localClientNum, GfxDepthOfField *dof);
double __cdecl CG_UpdateAdsDofValue(float currentValue, float targetValue, float maxChange, float dt);
bool __cdecl ShouldDoDefaultAdsDof(int localClientNum);
void __cdecl CG_UpdatePoisonFX(cg_s *cgameGlob);
void __cdecl CG_UpdateGenericFilter(cg_s *cgameGlob);
void __cdecl CG_UpdateWaterSheetingFX(cg_s *cgameGlob);
void __cdecl CG_UpdateFlameFX(cg_s *cgameGlob);
void __cdecl CG_UpdateElectrifiedFX(cg_s *cgameGlob);
void __cdecl CG_ValidateWeaponSelect(cg_s *cgameGlob);
void __cdecl CG_PickValidWeaponSelect(cg_s *cgameGlob);
int __cdecl CG_DrawExtraCamFrame(
                int localClientNum,
                int serverTime,
                DemoType demoType,
                CubemapShot cubemapShot,
                int cubemapSize,
                int renderScreen);
void __cdecl CG_CalcViewValues_ExtraCam(int localClientNum);
void __cdecl CG_CalcVrect_ExtraCam(int localClientNum);
void __cdecl CG_ClearSaveScreenFx(int localClientNum);
void __cdecl CG_SaveScreen(int localClientNum);
void __cdecl CG_SaveScreenSection(float viewX, float viewY, float viewWidth, float viewHeight, int screenTimerId);
void __cdecl CG_BlendSavedScreenShockBlurred(
                int fadeMsec,
                float viewX,
                float viewY,
                float viewWidth,
                float viewHeight,
                int screenTimerId);
void __cdecl CG_BlendSavedScreenShockFlashed(
                int localClientNum,
                float intensityWhiteout,
                float intensityScreengrab,
                float viewX,
                float viewY,
                float viewWidth,
                float viewHeight);
void __cdecl CG_ResetFullscreenFX(int localClientNum);

float __cdecl CG_GetViewZoomScale();

extern const dvar_t *cg_heliKillCamFov;
extern const dvar_t *cg_heliKillCamNearBlur;
extern const dvar_t *cg_heliKillCamFarBlur;
extern const dvar_t *cg_heliKillCamFarBlurStart;
extern const dvar_t *cg_heliKillCamFarBlurDist;
extern const dvar_t *cg_heliKillCamNearBlurStart;
extern const dvar_t *cg_heliKillCamNearBlurEnd;

extern const dvar_t *cg_scriptedKillCamFov;
extern const dvar_t *cg_scriptedKillCamCloseXYDist;
extern const dvar_t *cg_scriptedKillCamCloseZDist;
extern const dvar_t *cg_scriptedKillCamNearBlur;
extern const dvar_t *cg_scriptedKillCamFarBlur;
extern const dvar_t *cg_scriptedKillCamFarBlurStart;
extern const dvar_t *cg_scriptedKillCamFarBlurDist;
extern const dvar_t *cg_scriptedKillCamNearBlurStart;
extern const dvar_t *cg_scriptedKillCamNearBlurEnd;

extern const dvar_t *cg_destructibleKillCamFov;
extern const dvar_t *cg_destructibleKillCamCloseXYDist;
extern const dvar_t *cg_destructibleKillCamCloseZDist;
extern const dvar_t *cg_destructibleKillCamNearBlur;
extern const dvar_t *cg_destructibleKillCamFarBlur;
extern const dvar_t *cg_destructibleKillCamFarBlurStart;
extern const dvar_t *cg_destructibleKillCamFarBlurDist;
extern const dvar_t *cg_destructibleKillCamNearBlurStart;
extern const dvar_t *cg_destructibleKillCamNearBlurEnd;
extern const dvar_t *cg_destructibleKillCamZIncrease;
extern const dvar_t *cg_destructibleKillCamRegularHeight;

extern const dvar_t *cg_explosiveKillCamUpDist;
extern const dvar_t *cg_explosiveKillCamBackDist;
extern const dvar_t *cg_explosiveKillCamWallOutDist;
extern const dvar_t *cg_explosiveKillCamWallSideDist;
extern const dvar_t *cg_explosiveKillCamGroundUpDist;
extern const dvar_t *cg_explosiveKillCamGroundBackDist;
extern const dvar_t *cg_explosiveKillCamStopDist;
extern const dvar_t *cg_explosiveKillCamStopDecelDist;

extern const dvar_t *cg_rocketKillCamUpDist;
extern const dvar_t *cg_rocketKillCamBackDist;

extern const dvar_t *cg_dogKillCamFov;
extern const dvar_t *cg_dogKillCamForwardDist;
extern const dvar_t *cg_dogKillCamUpDist;
extern const dvar_t *cg_dogKillCamSideDist;
extern const dvar_t *cg_dogKillCamDistFromEyes;
extern const dvar_t *cg_dogKillMinDistFromTarget;

extern const dvar_t *cg_artilleryKillCamFov;
extern const dvar_t *cg_artilleryKillCamUpDist;
extern const dvar_t *cg_artilleryKillCamBackDist;
extern const dvar_t *cg_artilleryKillCamWallOutDist;
extern const dvar_t *cg_artilleryKillCamWallSideDist;
extern const dvar_t *cg_artilleryKillCamGroundUpDist;
extern const dvar_t *cg_artilleryKillCamGroundBackDist;

extern const dvar_t *cg_turretKillCamFov;
extern const dvar_t *cg_turretKillCamCloseXYDist;
extern const dvar_t *cg_turretKillCamCloseZDist;
extern const dvar_t *cg_turretKillCamNearBlur;
extern const dvar_t *cg_turretKillCamFarBlur;
extern const dvar_t *cg_turretKillCamFarBlurStart;
extern const dvar_t *cg_turretKillCamFarBlurDist;
extern const dvar_t *cg_turretKillCamNearBlurStart;
extern const dvar_t *cg_turretKillCamNearBlurEnd;
extern const dvar_t *cg_turretKillCamHeightIncrease;
extern const dvar_t *cg_turretKillCamBackOffset;
extern const dvar_t *cg_turretKillCamSideOffset;
extern const dvar_t *cg_turretKillCamDistanceIncrease;

extern const dvar_t *cg_infraredBlurTime;