#pragma once
#include <sound/snd.h>
#include <bgame/bg_weapons_def.h>

struct gentity_s;
struct XModel;
struct localEntity_s;
struct FxEffectDef;
struct DObj;
struct XAnim_s;
struct GfxScaledPlacement;
struct refdef_s;
struct ViewModelInfo;
struct cg_s;

struct AnimRateOffset // sizeof=0x8
{                                       // XREF: .data:g_animRateOffsets/r
    int fromWeaponDef;
    int fromWeaponVariantDef;
};

struct weaponInfo_s // sizeof=0x24
{                                       // XREF: weaponInfo_t/r
    XModel *handModel;
    XModel *gogglesModel;
    XModel *rocketModel;
    unsigned __int8 weapModelIdx;
    // padding byte
    // padding byte
    // padding byte
    int registered;
    const struct gitem_s *item;
    const char *translatedDisplayName;
    const char *translatedModename;
    const char *translatedAIOverlayDescription;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(weaponInfo_s) == 0x24);
#endif

struct weaponState_t // sizeof=0x90
{                                       // XREF: ?ClientThink_real@@YAXPAUgentity_s@@PAUusercmd_s@@@Z/r
                                        // ?CG_AddViewWeapon@@YAXH@Z/r
    const playerState_s *ps;            // XREF: ClientThink_real(gentity_s *,usercmd_s *):loc_5FA486/r
                                        // ClientThink_real(gentity_s *,usercmd_s *)+B20/r
    float xyspeed;
    float frametime;
    float fLastIdleFactor;
    int time;
    int damageTime;
    float v_dmg_pitch;
    float v_dmg_roll;
    int *weapIdleTime;
    float baseAngles[3];
    float baseOrigin[3];
    float recoilAngles[3];
    float recoilSpeed[3];
    float swayAngles[3];
    float swayOrigin[3];
    float swayViewAngles[3];
    float bobAngles[3];
    int shellShockStart;
    int shellShockDuration;
    int shellShockFadeTime;
};

struct __declspec(align(4)) snd_weapon_shot // sizeof=0x34
{                                       // XREF: snd_autosim_play/r
                                        // snd_autosim/r ...
    int localClientNum;                 // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+182/w
                                        // CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+13A/w
    SndEntHandle shooter;               // XREF: CG_SndKillAutoSimEnt(SndEntHandle)+22/r
                                        // CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+188/w ...
    unsigned int weapon;                // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+18E/w
                                        // CG_SndPingAutoSim+80/r ...
    float origin[3];                    // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1BF/w
                                        // CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1CC/w ...
    float direction[3];                 // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1E5/w
                                        // CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1F2/w ...
    unsigned int tagName;               // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+207/w
                                        // CG_SndPingAutoSim+94/r ...
    unsigned int burstCount;            // XREF: CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+1C2/w
    bool shooterIsPlayer;               // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+194/w
                                        // CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+149/w
    bool shooterIsLocalPlayer;          // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+19A/w
                                        // CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+14D/w
    bool isLastShotInClip;              // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1A0/w
                                        // CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+151/w
    bool isTurret;                      // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1AF/w
                                        // CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool):loc_535D96/r ...
    bool doubleTap;                     // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+1B5/w
                                        // CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+164/w
    bool fakeFire;                      // XREF: CG_WeaponFireSingle(int,SndEntHandle,uint,bool,bool,bool,float const * const,float const * const,uint,bool)+20A/w
                                        // CG_WeaponFireFake(int,SndEntHandle,uint,float const * const,float const * const,int)+1BB/w
    // padding byte
    // padding byte
};

struct snd_autosim_play // sizeof=0x38
{
    unsigned int frame;
    snd_weapon_shot shot;
};

struct snd_autosim // sizeof=0x4C
{                                       // XREF: .data:snd_autosim * g_snd_autosims/r
    snd_weapon_shot shot;               // XREF: CG_SndKillAutoSimEnt(SndEntHandle)+22/r
    // CG_SndPingAutoSim+6C/r ...
    unsigned int lastPing;
    unsigned int lastShot;
    unsigned int fireTime;
    unsigned int shotCount;
    int used;                           // XREF: CG_SndKillAutoSimEnt(SndEntHandle)+33/w
    // CG_SndPingAutoSim+43/r
    int isNew;
};

bool __cdecl CG_JavelinADS(int localClientNum);
bool __cdecl CG_UICheckWeapLockBlink(int localClientNum, float blinkPerSec);
bool __cdecl CG_UICheckWeapLockAttackTop(int localClientNum);
bool __cdecl CG_UICheckWeapLockAttackDirect(int localClientNum);
void __cdecl CG_ResetWeaponAnimTrees(int localClientNum, const playerState_s *ps, int newPlayerstate);
void __cdecl StartWeaponAnim(
                int localClientNum,
                unsigned int weaponNum,
                DObj *obj,
                int animIndex,
                float transitionTime,
                int newPlayerstate);
double __cdecl GetWeaponAnimRate(
                int localClientNum,
                const WeaponVariantDef *weapVariantDef,
                XAnim_s *anims,
                unsigned int animIndex);
double __cdecl GetWeaponAnimTimeFrac(
                int localClientNum,
                const WeaponVariantDef *weapVariantDef,
                XAnim_s *anims,
                unsigned int animIndex);
int __cdecl CG_WeaponDObjHandle(int localClientNum);
void __cdecl CG_WeaponMyChanges();
void __cdecl CG_SetWeaponHidePartBits(
                const WeaponVariantDef *weapVariantDef,
                ViewModelInfo *viewModelInfo,
                DObj *obj,
                int modelNum);
void __cdecl CG_FreeWeapons(int localClientNum);
void __cdecl CG_RegisterWeapon(int localClientNum, unsigned int weaponNum);
bool __cdecl ShouldRebuildTree(
                int localClientNum,
                const weaponInfo_s *weapInfo,
                int weaponNum,
                unsigned __int8 weaponModel,
                XModel *newHands);
bool __cdecl ShouldRebuildAnims(int localClientNum, const ViewModelInfo *viewModelInfo, int weaponNum);
void __cdecl CG_TouchViewModels(int localClientNum);
void __cdecl CG_UpdateWeaponViewmodels(int localClientNum);
void __cdecl ChangeViewmodelDobj(
                int localClientNum,
                const playerState_s *ps,
                unsigned int weaponNum,
                unsigned __int8 weaponModel,
                XModel *newHands,
                XModel *newGoggles,
                XModel *newRocket,
                bool updateClientInfo,
                bool forceRebuildTree);
void __cdecl CG_CreateWeaponViewModelXAnim(ViewModelInfo *viewModelInfo, const WeaponVariantDef *weapVariantDef);
bool __cdecl CG_NonDualWieldAnimsValid(const WeaponVariantDef *weapVariantDef);
void __cdecl CG_UpdateHandViewmodels(int localClientNum);
void __cdecl CG_RegisterItemVisuals(int localClientNum, unsigned int weapIdx);
void __cdecl CG_RegisterItems(int localClientNum);
void __cdecl WeaponFlash(
                int localClientNum,
                int dobjHandle,
                unsigned int weaponNum,
                int bViewFlash,
                unsigned int flashTag);
void __cdecl CG_HoldBreathInit(cg_s *cgameGlob);
void __cdecl CG_UpdateViewModelPose(const DObj *obj, int localClientNum);
void __cdecl CG_UpdateMinigunSpin(int localClientNum, cg_s *cgameGlob, centity_s *cent);
double __cdecl I_fmodulus(float f, float mod);
void __cdecl CG_UpdateMinigunSounds(int localClientNum, cg_s *cgameGlob, centity_s *cent, const WeaponDef *weaponDef);
void __cdecl CG_UpdateMinigunTags(cg_s *cgameGlob, centity_s *cent, const DObj *obj, int *partBits);
void __cdecl UpdateMinigunTag(
                cg_s *cgameGlob,
                centity_s *cent,
                const DObj *obj,
                unsigned __int16 name,
                unsigned int axis,
                int *partBits);
void __cdecl CG_UpdateViewModelMinigun(cg_s *cgameGlob, DObj *obj, const WeaponDef *weapDef, centity_s *cent);
void __cdecl CG_UpdateViewModelStackCounter(
                cg_s *cgameGlob,
                const playerState_s *ps,
                DObj *obj,
                const WeaponDef *weapDef);
void __cdecl BulletTrace_Start(
                int localClientNum,
                BulletFireParams *bp,
                unsigned int weaponIndex,
                const centity_s *attacker,
                float *tracerStart,
                bool drawTracer,
                bool isPlayer);
void __cdecl FireBulletPenetrate(
                int localClientNum,
                BulletFireParams *bp,
                unsigned int weaponIndex,
                const centity_s *attacker,
                float *tracerStart,
                bool drawTracer,
                bool isPlayer,
                BulletTraceResults *br);
char __cdecl BulletTrace(
                int localClientNum,
                const BulletFireParams *bp,
                const WeaponDef *weapDef,
                const centity_s *attacker,
                BulletTraceResults *br,
                unsigned int lastSurfaceType);
bool __cdecl ShouldIgnoreHitEntity(int attackerNum, int hitEntNum);
bool __cdecl IsEntityNotDoingClientSideBullets(int localClientNum, int entityNum);
bool __cdecl CG_IsPlayerCrouching(clientInfo_t *ci, const centity_s *cent);
bool __cdecl CG_IsPlayerProne(clientInfo_t *ci, const centity_s *cent);
bool __cdecl CG_IsPlayerADS(clientInfo_t *ci, const centity_s *cent);
void __cdecl CG_GuessSpreadForWeapon(
                int localClientNum,
                const centity_s *cent,
                const WeaponDef *weapDef,
                float *minSpread,
                float *maxSpread);
char __cdecl CG_GetPlayerViewOrigin(int localClientNum, const playerState_s *ps, float *origin);
cg_s *__cdecl CG_GetLocalClientCGameGlobFromEntNum(int entnumber);
void __cdecl CG_AddPlayerWeapon(
                int localClientNum,
                const GfxScaledPlacement *placement,
                const playerState_s *ps,
                centity_s *cent,
                int bDrawGun);
void __cdecl HoldBreathUpdate(int localClientNum);
void __cdecl HoldBreathSoundLerp(int localClientNum, float lerp);
bool __cdecl CG_ShowMuzzleFlash(cg_s *cgameGlob, centity_s *cent, int bViewModel);
void __cdecl Weapon_DrawAxisOrigin(orientation_t orient, float colorScale);
void __cdecl Weapon_DrawTagAxis(int localClientNum, int entityNum, unsigned int weaponNum, int bViewModel);
void __cdecl Anim_DrawTags(int localClientNum, int entityNum, int weaponNum, cpose_t *pose, char flags);
void __cdecl CG_UpdateViewWeaponAnim(int localClientNum, int newPlayerstate);
void __cdecl WeaponRunXModelAnims(
                int localClientNum,
                const playerState_s *ps,
                ViewModelInfo *viewModelInfo,
                int newPlayerstate);
void __cdecl StartCameraAnim(
                int localClientNum,
                unsigned int weaponNum,
                DObj *obj,
                int animIndex,
                float transitionTime);
void __cdecl StopCameraAnim(DObj *obj);
void __cdecl PlayIdleAnim(
                int localClientNum,
                const playerState_s *ps,
                ViewModelInfo *viewModelInfo,
                DObj *obj,
                unsigned int weaponIndex,
                float transitionTime,
                int newPlayerstate);
void __cdecl PlayADSAnim(float weaponPosFrac, int weaponNum, DObj *obj, int animIndex);
int __cdecl CheckForValidAnimIDDW(int animID);
void __cdecl EnsureValidLeftAnim(
                int localClientNum,
                DObj *obj,
                ViewModelInfo *viewModelInfo,
                XAnimTree_s *pAnimTree,
                unsigned int weaponIndex,
                int newPlayerstate);
void __cdecl ProcessWeaponNoteTracks(int localClientNum, const playerState_s *predictedPlayerState, bool isViewArms);
void __cdecl PlayNoteMappedSoundAliases(int localClientNum, const char *noteName, const WeaponDef *weapDef);
bool __cdecl NeedToRebuildViewmodel(
                int localClientNum,
                unsigned int weaponIndex,
                unsigned __int8 weaponModel,
                const ViewModelInfo *viewModelInfo,
                weaponInfo_s *weapInfo);
bool __cdecl UpdateViewmodelAttachments(
                int localClientNum,
                unsigned int weaponNum,
                unsigned __int8 weaponModel,
                weaponInfo_s *weapInfo);
bool __cdecl ViewmodelRocketShouldBeAttached(int localClientNum, unsigned int weaponIndex);
void __cdecl CG_AddViewWeapon(int localClientNum);
void __cdecl CalculateWeaponPostion_PositionToADS(cg_s *cgameGlob, playerState_s *ps);
void __cdecl CG_CalculateWeaponMovement_ClientSpecific(cg_s *cgameGlob, GfxScaledPlacement *placement, float *angles);
void __cdecl CG_CalculateWeaponMovement_LocalToWorld(const cg_s *cgameGlob, float *origin);
void __cdecl CG_CalculateWeaponMovement_Landing(const cg_s *cgameGlob, float *origin);
void __cdecl CG_CalculateWeaponMovement_SaveOrigin(cg_s *cgameGlob, float *origin);
void __cdecl CG_CalculateWeaponMovement_Debug(const cg_s *cgameGlob, float *origin);
void __cdecl CG_CalculateWeaponMovement_SaveAngles(cg_s *cgameGlob, float *angles, GfxScaledPlacement *placement);
void __cdecl CG_BuildWeaponState(cg_s *cgameGlob, weaponState_t *ws);
void __cdecl CG_SaveWeaponState(const weaponState_t *ws, cg_s *cgameGlob);
void __cdecl CG_NextWeapon_f();
bool __cdecl WeaponCycleAllowed(cg_s *cgameGlob);
char __cdecl CG_CancelOffhand(cg_s *cgameGlob);
void __cdecl CG_PrevWeapon_f();
void __cdecl CG_OutOfAmmoChange(int localClientNum);
char __cdecl VerifyPlayerAltModeWeapon(int localClientNum, unsigned int weaponIndex);
char __cdecl CycleWeapPrimary(int localClientNum, int cycleForward, int bIgnoreEmpty);
unsigned int __cdecl CG_GetLastWeaponForAlt(const cg_s *cgameGlob, const playerState_s *ps, unsigned int weapIndex);
unsigned int __cdecl CG_GetAltWeapon(const cg_s *cgameGlob, unsigned int weapIndex);
unsigned int __cdecl CG_AltWeaponToggleIndex(int localClientNum, const cg_s *cgameGlob, unsigned int *primaryWeapon);
unsigned int __cdecl NextWeapInCycle(
                int localClientNum,
                const playerState_s *ps,
                weapInventoryType_t type,
                unsigned int startWeaponIndex,
                bool cycleForward,
                bool skipEmpties,
                bool skipHaveNoAlts);
void __cdecl CG_ActionSlotDown_f();
char __cdecl ToggleWeaponAltMode(int localClientNum);
bool __cdecl ActionSlotUsageAllowed(cg_s *cgameGlob);
char __cdecl ActionParms(int *slotResult);
bool __cdecl CG_CheckPuttingAwayKillstreakWeapon(unsigned int oldWeapon, unsigned int newWeapon);
void __cdecl CG_ActionSlotUp_f();
void __cdecl CG_EjectWeaponBrass(int localClientNum, const entityState_s *ent, int event, unsigned int weapon);
void __cdecl CG_VehicleJolt(centity_s *cent, float *origin, float *dir);
bool __cdecl CG_FireWeapon_GenerateMark(centity_s *cent, const playerState_s *ps, const WeaponDef *weaponDef);
bool __cdecl CG_FireWeapon_LocalPlayerUsingVehicleWeapon(centity_s *cent, const playerState_s *ps, int event);
bool __cdecl CG_FireWeapon_LocalPlayerUsingTurretWeapon(centity_s *cent, const playerState_s *ps);
void __cdecl CG_FireWeapon_VehicleFire(
                int localClientNum,
                centity_s *cent,
                const playerState_s *ps,
                int event,
                unsigned __int16 tagName);
void __cdecl CG_VehicleFire(centity_s *cent, unsigned __int16 tagName);
void __cdecl CG_FireWeapon_OriginAndDirection(
                int localClientNum,
                centity_s *cent,
                unsigned __int16 tagName,
                bool isPlayer,
                float *origin,
                float *direction);
void __cdecl CG_FireWeapon(
                int localClientNum,
                centity_s *cent,
                int event,
                unsigned __int16 tagName,
                unsigned int weapon,
                const playerState_s *ps,
                bool leftGun);
void __cdecl DrawBulletImpacts(
                int localClientNum,
                const centity_s *ent,
                unsigned __int16 boneName,
                const playerState_s *ps,
                unsigned int weapon,
                int event,
                bool isPlayer);
void __cdecl CG_BulletEndpos(
                int randSeed,
                float spread,
                const float *start,
                float *end,
                float *dir,
                const float *forwardDir,
                const float *rightDir,
                const float *upDir,
                float maxRange);
void __cdecl RandomBulletDir(int randSeed, float *x, float *y);
void __cdecl TakeClipOnlyWeaponIfEmpty(int localClientNum, playerState_s *ps);
bool __cdecl CG_IsCharacter(bool isPlayer, int eType);
void __cdecl CG_FakeFireWeapon(
                int localClientNum,
                centity_s *cent,
                unsigned int weapon,
                float *origin,
                unsigned int count);
void __cdecl CG_SndAutoSimReset();
void __cdecl CG_SndKillAutoSimEnt(SndEntHandle ent);
void __cdecl CG_SndUpdateAutoSim();
void __cdecl CG_SndWeaponFire(snd_weapon_shot *shot);
void __cdecl CG_SndWeaponFakeFire(snd_weapon_shot *shot, const WeaponDef *weaponDef);
void __cdecl CG_SndWeaponFakeFireNotify(int localClientNum, unsigned int entNum, centity_s *shot_ent, float *origin);
void __cdecl CG_WeaponFireSingle(
                int localClientNum,
                SndEntHandle shooter,
                unsigned int weapon,
                bool shooterIsPlayer,
                bool shooterIsLocalPlayer,
                bool isLastShotInClip,
                const float *origin,
                const float *direction,
                unsigned int tagName,
                bool doubleTap);
void __cdecl CG_SndPingAutoSim(snd_weapon_shot *shot);
void __cdecl CG_WeaponFireFake(
                int localClientNum,
                SndEntHandle shooter,
                unsigned int weapon,
                const float *origin,
                const float *direction,
                unsigned int count);
void __cdecl CG_SpawnTracer(
                int localClientNum,
                const float *pstart,
                const float *pend,
                int type,
                float width,
                float length);
void __cdecl CG_DrawTracer(const float *start, const float *finish, const refdef_s *refdef, localEntity_s *le);
void __cdecl ScaleTracer(
                const float *start,
                const float *finish,
                const float *viewOrg,
                float *startWidth,
                float *finishWidth);
double __cdecl CalcTracerFinalScale(float tracerScaleDistRange, float dist, float tracerScale);
cg_s *__cdecl CG_GetLocalClientGlobalsForEnt(int localClientNum, int entityNum);
void __cdecl CG_GetViewDirection(int localClientNum, int entityNum, float *forward, float *right, float *up);
void __cdecl CG_CalcEyePoint(int localClientNum, int entityNum, float *eyePos);
void __cdecl CG_RandomEffectAxis(float *forward, float *left, float *up);
void __cdecl CG_ImpactEffectForWeapon(
                unsigned int weaponIndex,
                unsigned int surfType,
                char impactFlags,
                const FxEffectDef **outFx,
                unsigned int *outSnd);
void __cdecl CG_BulletHitEvent(
                int localClientNum,
                int sourceEntityNum,
                int targetEntityNum,
                int weaponIndex,
                const float *startPos,
                const float *position,
                const float *normal,
                const float *seeThruDecalNormal,
                unsigned int surfType,
                int event,
                unsigned __int8 eventParam,
                int damage,
                __int16 hitContents,
                unsigned __int8 boneIndex);
void __cdecl CG_BulletHitEvent_Internal(
                int localClientNum,
                int sourceEntityNum,
                int targetEntityNum,
                int weaponIndex,
                const float *startPos,
                const float *position,
                const float *normal,
                const float *seeThruDecalNormal,
                unsigned int surfType,
                int event,
                unsigned __int8 eventParam,
                int damage,
                __int16 hitContents,
                unsigned __int8 boneIndex);
void __cdecl BulletTrajectoryEffects(
                int localClientNum,
                int sourceEntityNum,
                unsigned int weaponIndex,
                const float *startPos,
                const float *position,
                int surfType,
                unsigned int flashTag,
                unsigned __int8 impactFlags,
                int damage,
                float *tracerStart);
void __cdecl CalcMuzzlePoint(int localClientNum, int entityNum, float *muzzle, unsigned int flashTag);
void __cdecl CG_BulletHitEvent_GenExitDir(
                int localClientNum,
                int sourceEntityNum,
                const float *position,
                float *outExitDir);
void __cdecl CG_BulletHitClientEvent(
                int localClientNum,
                int sourceEntityNum,
                unsigned int weaponNum,
                int hitEntityNum,
                const float *startPos,
                float *position,
                unsigned int surfType,
                int event,
                int damage);
void __cdecl CG_DogBloodEvent(int localClientNum, unsigned int entnum, float *position, int boneIndex);
void __cdecl CG_MeleeBloodEvent(int localClientNum, const centity_s *cent, int eventParm);
void __cdecl CG_SetBaseWeaponForStats(const WeaponVariantDef *weapVariantDef);
void __cdecl CG_SetupWeaponConfigString(int configStringIndex);
void __cdecl CG_SetupWeaponDef();
unsigned int __cdecl ValidLatestPrimaryWeapIdx(const cg_s *cgameGlob, unsigned int weaponIndex);
char __cdecl CG_SetLastWeaponForAlt(
                cg_s *cgameGlob,
                const playerState_s *ps,
                unsigned int altWeapon,
                unsigned int weapon);
void __cdecl CG_SelectWeaponIndex(int localClientNum, const playerState_s *ps, unsigned int weaponIndex);
char __cdecl CG_UpdateLastWeaponForAlt(
                cg_s *cgameGlob,
                const playerState_s *ps,
                unsigned int oldweapon,
                unsigned int weapon);
void __cdecl CG_SelectWeaponIndex(int localClientNum, unsigned int weaponIndex);
void __cdecl CG_DisplayViewmodelAnim(int localClientNum);
bool __cdecl CG_ScopeIsOverlayed(int localClientNum);
unsigned int __cdecl CG_PlayerTurretWeaponIdx(int localClientNum);
unsigned int __cdecl CG_GetPlayerMeleeWeapon(const playerState_s *ps);
int __cdecl CG_GetPlayerWeapon(const playerState_s *ps, int localClientNum);
int __cdecl CG_GetPlayerVehicleWeapon(const playerState_s *ps, int localClientNum);
unsigned int __cdecl CG_GetClientWeapon(unsigned int clientNum, int localClientNum);
bool __cdecl CG_PlayerUsingScopedTurret(int localClientNum);
void __cdecl CG_ApplyWeaponTurnRateCap(int localClientNum);
bool __cdecl CG_PlayerUsingHandbrakeInVehicle(int localClientNum);
double __cdecl CG_GetPlayerVehicleHandbrakeTurnSpeedValue(int localClientNum);
void __cdecl CG_MolotovFloat(int localClientNum, const centity_s *cent, const entityState_s *es);
Material *__cdecl CG_GetWeaponOverlay(int localClientNum, unsigned int weaponIndex);
bool __cdecl CG_IsInGuidedMissile(const playerState_s *ps);
bool __cdecl CG_IsInGuidedMissileStatic(const playerState_s *ps);


extern unsigned __int8 bulletPriorityMap[20];
extern unsigned __int8 riflePriorityMap[19];