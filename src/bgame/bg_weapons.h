#pragma once
#include <universal/q_shared.h>

struct XModel;
struct FxEffectDef;
struct flameTable;
struct Material;

enum weaponstate_t : __int32
{                                       // XREF: CG_StopWeaponSound/r
    WEAPON_READY                 = 0x0,
    WEAPON_RAISING               = 0x1,
    WEAPON_RAISING_ALTSWITCH     = 0x2,
    WEAPON_DROPPING              = 0x3,
    WEAPON_DROPPING_QUICK        = 0x4,
    WEAPON_DROPPING_ALTSWITCH    = 0x5,
    WEAPON_FIRING                = 0x6,
    WEAPON_RECHAMBERING          = 0x7,
    WEAPON_RELOADING_RIGHT       = 0x8,
    WEAPON_RELOADING_LEFT        = 0x9,
    WEAPON_RELOADING             = 0xA,
    WEAPON_RELOADING_INTERUPT    = 0xB,
    WEAPON_RELOAD_START          = 0xC,
    WEAPON_RELOAD_START_INTERUPT = 0xD,
    WEAPON_RELOAD_END            = 0xE,
    WEAPON_RELOAD_QUICK          = 0xF,
    WEAPON_RELOAD_QUICK_EMPTY    = 0x10,
    WEAPON_MELEE_INIT            = 0x11,
    WEAPON_MELEE_FIRE            = 0x12,
    WEAPON_MELEE_END             = 0x13,
    WEAPON_OFFHAND_INIT          = 0x14,
    WEAPON_OFFHAND_PREPARE       = 0x15,
    WEAPON_OFFHAND_HOLD          = 0x16,
    WEAPON_OFFHAND_START         = 0x17,
    WEAPON_OFFHAND               = 0x18,
    WEAPON_OFFHAND_END           = 0x19,
    WEAPON_DETONATING            = 0x1A,
    WEAPON_SPRINT_RAISE          = 0x1B,
    WEAPON_SPRINT_LOOP           = 0x1C,
    WEAPON_SPRINT_DROP           = 0x1D,
    WEAPON_CONT_FIRE_IN          = 0x1E,
    WEAPON_CONT_FIRE_LOOP        = 0x1F,
    WEAPON_CONT_FIRE_OUT         = 0x20,
    WEAPON_NIGHTVISION_WEAR      = 0x21,
    WEAPON_NIGHTVISION_REMOVE    = 0x22,
    WEAPON_DEPLOYING             = 0x23,
    WEAPON_DEPLOYED              = 0x24,
    WEAPON_BREAKING_DOWN         = 0x25,
    WEAPON_SWIM_IN               = 0x26,
    WEAPON_SWIM_OUT              = 0x27,
    WEAPON_DTP_IN                = 0x28,
    WEAPON_DTP_LOOP              = 0x29,
    WEAPON_DTP_OUT               = 0x2A,
    WEAPON_SLIDE_IN              = 0x2B,
    WEAPON_FIRING_LEFT           = 0x2C,
    WEAPON_FIRING_BOTH           = 0x2D,
    WEAPON_JAMMED                = 0x2E,
    WEAPON_LOWREADY_RAISE        = 0x2F,
    WEAPON_LOWREADY_LOOP         = 0x30,
    WEAPON_LOWREADY_DROP         = 0x31,
    WEAPONSTATES_NUM             = 0x32,
};

enum weapType_t : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPTYPE_BULLET = 0x0,
    WEAPTYPE_GRENADE = 0x1,
    WEAPTYPE_PROJECTILE = 0x2,
    WEAPTYPE_BINOCULARS = 0x3,
    WEAPTYPE_GAS = 0x4,
    WEAPTYPE_BOMB = 0x5,
    WEAPTYPE_MINE = 0x6,
    WEAPTYPE_MELEE = 0x7,
    WEAPTYPE_NUM = 0x8,
};

enum weapInventoryType_t : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPINVENTORY_PRIMARY = 0x0,
    WEAPINVENTORY_OFFHAND = 0x1,
    WEAPINVENTORY_ITEM = 0x2,
    WEAPINVENTORY_ALTMODE = 0x3,
    WEAPINVENTORY_MELEE = 0x4,
    WEAPINVENTORYCOUNT = 0x5,
};


enum weapClass_t : __int32
{                                                                             // XREF: WeaponDef/r
                                                                                // ?BG_GetWeaponClassName@@YAPBDW4weapClass_t@@@Z/r
    WEAPCLASS_RIFLE = 0x0,
    WEAPCLASS_MG = 0x1,
    WEAPCLASS_SMG = 0x2,
    WEAPCLASS_SPREAD = 0x3,
    WEAPCLASS_PISTOL = 0x4,
    WEAPCLASS_GRENADE = 0x5,
    WEAPCLASS_ROCKETLAUNCHER = 0x6,
    WEAPCLASS_TURRET = 0x7,
    WEAPCLASS_NON_PLAYER = 0x8,
    WEAPCLASS_GAS = 0x9,
    WEAPCLASS_ITEM = 0xA,
    WEAPCLASS_MELEE = 0xB,
    WEAPCLASS_KILLSTREAK_ALT_STORED_WEAPON = 0xC,
    WEAPCLASS_NUM = 0xD,
};

enum PenetrateType : __int32
{                                                                             // XREF: WeaponDef/r
    PENETRATE_TYPE_NONE = 0x0,
    PENETRATE_TYPE_SMALL = 0x1,
    PENETRATE_TYPE_MEDIUM = 0x2,
    PENETRATE_TYPE_LARGE = 0x3,
    PENETRATE_TYPE_COUNT = 0x4,
};

enum ImpactType : __int32
{                                                                             // XREF: WeaponDef/r
    IMPACT_TYPE_NONE = 0x0,
    IMPACT_TYPE_BULLET_SMALL = 0x1,
    IMPACT_TYPE_BULLET_LARGE = 0x2,
    IMPACT_TYPE_BULLET_AP = 0x3,
    IMPACT_TYPE_BULLET_XTREME = 0x4,
    IMPACT_TYPE_SHOTGUN = 0x5,
    IMPACT_TYPE_GRENADE_BOUNCE = 0x6,
    IMPACT_TYPE_GRENADE_EXPLODE = 0x7,
    IMPACT_TYPE_RIFLE_GRENADE = 0x8,
    IMPACT_TYPE_ROCKET_EXPLODE = 0x9,
    IMPACT_TYPE_ROCKET_EXPLODE_XTREME = 0xA,
    IMPACT_TYPE_PROJECTILE_DUD = 0xB,
    IMPACT_TYPE_MORTAR_SHELL = 0xC,
    IMPACT_TYPE_TANK_SHELL = 0xD,
    IMPACT_TYPE_BOLT = 0xE,
    IMPACT_TYPE_BLADE = 0xF,
    IMPACT_TYPE_COUNT = 0x10,
};

enum weapFireType_t : __int32
{                                                                             // XREF: WeaponDef/r
                                                                                // ?CG_UpdateMinigunTags@@YAXPAUcg_s@@PAUcentity_s@@PBUDObj@@QAH@Z/r
    WEAPON_FIRETYPE_FULLAUTO = 0x0,
    WEAPON_FIRETYPE_SINGLESHOT = 0x1,
    WEAPON_FIRETYPE_BURSTFIRE2 = 0x2,
    WEAPON_FIRETYPE_BURSTFIRE3 = 0x3,
    WEAPON_FIRETYPE_BURSTFIRE4 = 0x4,
    WEAPON_FIRETYPE_STACKED = 0x5,
    WEAPON_FIRETYPE_MINIGUN = 0x6,
    WEAPON_FIRETYPECOUNT = 0x7,
};

enum weapClipType_t : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPON_CLIPTYPE_BOTTOM = 0x0,
    WEAPON_CLIPTYPE_TOP = 0x1,
    WEAPON_CLIPTYPE_LEFT = 0x2,
    WEAPON_CLIPTYPE_DP28 = 0x3,
    WEAPON_CLIPTYPE_PTRS = 0x4,
    WEAPON_CLIPTYPE_LMG = 0x5,
    WEAPON_CLIPTYPECOUNT = 0x6,
};

enum OffhandClass : __int32
{                                                                             // XREF: WeaponDef/r
                                                                                // ?CG_OwnerDraw@@YAXHUrectDef_s@@MMMMHHHHQAMPAUMaterial@@@Z/r ...
    OFFHAND_CLASS_NONE = 0x0,
    OFFHAND_CLASS_FRAG_GRENADE = 0x1,
    OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
    OFFHAND_CLASS_FLASH_GRENADE = 0x3,
    OFFHAND_CLASS_GEAR = 0x4,
    OFFHAND_CLASS_COUNT = 0x5,
};

enum weapStance_t : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPSTANCE_STAND = 0x0,
    WEAPSTANCE_DUCK = 0x1,
    WEAPSTANCE_PRONE = 0x2,
    WEAPSTANCE_NUM = 0x3,
};

enum OffhandSlot : __int32
{                                                                             // XREF: WeaponDef/r
                                                                                // Bot_UpdateGrenadeThrow/r ...
    OFFHAND_SLOT_NONE = 0x0,
    OFFHAND_SLOT_LETHAL_GRENADE = 0x1,
    OFFHAND_SLOT_TACTICAL_GRENADE = 0x2,
    OFFHAND_SLOT_EQUIPMENT = 0x3,
    OFFHAND_SLOT_SPECIFIC_USE = 0x4,
    OFFHAND_SLOT_COUNT = 0x5,
};

enum activeReticleType_t : __int32
{                                                                             // XREF: WeaponDef/r
    VEH_ACTIVE_RETICLE_NONE = 0x0,
    VEH_ACTIVE_RETICLE_PIP_ON_A_STICK = 0x1,
    VEH_ACTIVE_RETICLE_BOUNCING_DIAMOND = 0x2,
    VEH_ACTIVE_RETICLE_COUNT = 0x3,
};

enum weaponIconRatioType_t : __int32
{                                                                             // XREF: WeaponDef/r
                                                                                // WeaponDef/r ...
    WEAPON_ICON_RATIO_1TO1 = 0x0,
    WEAPON_ICON_RATIO_2TO1 = 0x1,
    WEAPON_ICON_RATIO_4TO1 = 0x2,
    WEAPON_ICON_RATIO_COUNT = 0x3,
};

enum ammoCounterClipType_t : __int32
{                                                                             // XREF: WeaponDef/r
    AMMO_COUNTER_CLIP_NONE = 0x0,
    AMMO_COUNTER_CLIP_MAGAZINE = 0x1,
    AMMO_COUNTER_CLIP_SHORTMAGAZINE = 0x2,
    AMMO_COUNTER_CLIP_SHOTGUN = 0x3,
    AMMO_COUNTER_CLIP_ROCKET = 0x4,
    AMMO_COUNTER_CLIP_BELTFED = 0x5,
    AMMO_COUNTER_CLIP_ALTWEAPON = 0x6,
    AMMO_COUNTER_CLIP_COUNT = 0x7,
};

enum weapOverlayReticle_t : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPOVERLAYRETICLE_NONE = 0x0,
    WEAPOVERLAYRETICLE_CROSSHAIR = 0x1,
    WEAPOVERLAYRETICLE_NUM = 0x2,
};

enum weapProjExposion_t : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPPROJEXP_GRENADE = 0x0,
    WEAPPROJEXP_ROCKET = 0x1,
    WEAPPROJEXP_FLASHBANG = 0x2,
    WEAPPROJEXP_NONE = 0x3,
    WEAPPROJEXP_DUD = 0x4,
    WEAPPROJEXP_SMOKE = 0x5,
    WEAPPROJEXP_HEAVY = 0x6,
    WEAPPROJEXP_FIRE = 0x7,
    WEAPPROJEXP_NAPALMBLOB = 0x8,
    WEAPPROJEXP_BOLT = 0x9,
    WEAPPROJEXP_NUM = 0xA,
};

enum WeapOverlayInteface_t : __int32 // no r?
{                                                                             // XREF: WeaponDef/r
    WEAPOVERLAYINTERFACE_NONE = 0x0,
    WEAPOVERLAYINTERFACE_JAVELIN = 0x1,
    WEAPOVERLAYINTERFACE_TURRETSCOPE = 0x2,
    WEAPOVERLAYINTERFACECOUNT = 0x3,
};

enum WeapStickinessType : __int32
{                                                                             // XREF: WeaponDef/r
    WEAPSTICKINESS_NONE = 0x0,
    WEAPSTICKINESS_ALL = 0x1,
    WEAPSTICKINESS_ALL_NO_SENTIENTS = 0x2,
    WEAPSTICKINESS_GROUND = 0x3,
    WEAPSTICKINESS_GROUND_WITH_YAW = 0x4,
    WEAPSTICKINESS_FLESH = 0x5,
    WEAPSTICKINESS_COUNT = 0x6,
};

enum WeapRotateType : __int32
{                                                                             // XREF: WeaponDef/r
                                                                                // ?G_InitGrenadeMovement@@YAXPAUgentity_s@@QBM1HW4WeapRotateType@@@Z/r
    WEAPROTATE_GRENADE_ROTATE = 0x0,
    WEAPROTATE_BLADE_ROTATE = 0x1,
    WEAPROTATE_CYLINDER_ROTATE = 0x2,
    WEAPROTATE_COUNT = 0x3,
};

enum guidedMissileType_t : __int32
{                                                                             // XREF: WeaponDef/r
    MISSILE_GUIDANCE_NONE = 0x0,
    MISSILE_GUIDANCE_SIDEWINDER = 0x1,
    MISSILE_GUIDANCE_HELLFIRE = 0x2,
    MISSILE_GUIDANCE_JAVELIN = 0x3,
    MISSILE_GUIDANCE_BALLISTIC = 0x4,
    MISSILE_GUIDANCE_WIREGUIDED = 0x5,
    MISSILE_GUIDANCE_TVGUIDED = 0x6,
    MISSILE_GUIDANCE_COUNT = 0x7,
};

struct BulletFireParams // sizeof=0x40
{                                                                             // XREF: ?VEH_DebugAim@@YAXPAUgentity_s@@QBMH@Z/r
                                                                                // ?Bullet_Fire@@YAXPAUgentity_s@@MPBUweaponParms@@PBU1@H@Z/r ...
        int weaponEntIndex;                                 // XREF: DrawBulletImpacts+13F7/w
                                                                                // Bullet_Fire(gentity_s *,float,weaponParms const *,gentity_s const *,int)+176/w ...
        int ignoreEntIndex;                                 // XREF: DrawBulletImpacts+1407/w
                                                                                // DrawBulletImpacts+1427/w ...
        float damageMultiplier;                         // XREF: DrawBulletImpacts+1436/w
                                                                                // DrawBulletImpacts+159A/w ...
        int methodOfDeath;                                    // XREF: DrawBulletImpacts+1451/w
                                                                                // Bullet_Fire(gentity_s *,float,weaponParms const *,gentity_s const *,int)+1C8/w
        float origStart[3];                                 // XREF: DrawBulletImpacts+14AF/w
                                                                                // DrawBulletImpacts+14BC/w ...
        float start[3];                                         // XREF: FireBulletPenetrate+C3A/w
                                                                                // FireBulletPenetrate+C4A/w ...
        float end[3];                                             // XREF: FireBulletPenetrate+C74/w
                                                                                // FireBulletPenetrate+C8B/w ...
        float dir[3];                                             // XREF: FireBulletPenetrate+BF1/w
                                                                                // FireBulletPenetrate+C08/w ...
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(BulletFireParams) == 64);
#endif

struct BulletTraceResults;

struct WeaponDef // sizeof=0x808
{                                                                             // XREF: WeaponFullDef/r
        const char *szOverlayName;
        XModel **gunXModel;
        XModel *handXModel;
        const char *szModeName;
        unsigned __int16 *notetrackSoundMapKeys;
        unsigned __int16 *notetrackSoundMapValues;
        int playerAnimType;
        weapType_t weapType;
        weapClass_t weapClass;
        PenetrateType penetrateType;
        ImpactType impactType;
        weapInventoryType_t inventoryType;
        weapFireType_t fireType;
        weapClipType_t clipType;
        int itemIndex;
        const char *parentWeaponName;
        int iJamFireTime;
        int tracerFrequency;
        float tracerWidth;
        float tracerLength;
        int overheatWeapon;
        float overheatRate;
        float cooldownRate;
        float overheatEndVal;
        bool coolWhileFiring;
        bool fuelTankWeapon;
        // padding byte
        // padding byte
        int iTankLifeTime;
        OffhandClass offhandClass;
        OffhandSlot offhandSlot;
        weapStance_t stance;
        const FxEffectDef *viewFlashEffect;
        const FxEffectDef *worldFlashEffect;
        const char *pickupSound;
        const char *pickupSoundPlayer;
        const char *ammoPickupSound;
        const char *ammoPickupSoundPlayer;
        const char *projectileSound;
        const char *pullbackSound;
        const char *pullbackSoundPlayer;
        const char *fireSound;
        const char *fireSoundPlayer;
        const char *fireLoopSound;
        const char *fireLoopSoundPlayer;
        const char *fireLoopEndSound;
        const char *fireLoopEndSoundPlayer;
        const char *fireStopSound;
        const char *fireStopSoundPlayer;
        const char *fireLastSound;
        const char *fireLastSoundPlayer;
        const char *emptyFireSound;
        const char *emptyFireSoundPlayer;
        const char *crackSound;
        const char *whizbySound;
        const char *meleeSwipeSound;
        const char *meleeSwipeSoundPlayer;
        const char *meleeHitSound;
        const char *meleeMissSound;
        const char *rechamberSound;
        const char *rechamberSoundPlayer;
        const char *reloadSound;
        const char *reloadSoundPlayer;
        const char *reloadEmptySound;
        const char *reloadEmptySoundPlayer;
        const char *reloadStartSound;
        const char *reloadStartSoundPlayer;
        const char *reloadEndSound;
        const char *reloadEndSoundPlayer;
        const char *rotateLoopSound;
        const char *rotateLoopSoundPlayer;
        const char *deploySound;
        const char *deploySoundPlayer;
        const char *finishDeploySound;
        const char *finishDeploySoundPlayer;
        const char *breakdownSound;
        const char *breakdownSoundPlayer;
        const char *finishBreakdownSound;
        const char *finishBreakdownSoundPlayer;
        const char *detonateSound;
        const char *detonateSoundPlayer;
        const char *nightVisionWearSound;
        const char *nightVisionWearSoundPlayer;
        const char *nightVisionRemoveSound;
        const char *nightVisionRemoveSoundPlayer;
        const char *altSwitchSound;
        const char *altSwitchSoundPlayer;
        const char *raiseSound;
        const char *raiseSoundPlayer;
        const char *firstRaiseSound;
        const char *firstRaiseSoundPlayer;
        const char *putawaySound;
        const char *putawaySoundPlayer;
        const char *overheatSound;
        const char *overheatSoundPlayer;
        const char *adsZoomSound;
        const char **bounceSound;
        const char *standMountedWeapdef;
        const char *crouchMountedWeapdef;
        const char *proneMountedWeapdef;
        int standMountedIndex;
        int crouchMountedIndex;
        int proneMountedIndex;
        const FxEffectDef *viewShellEjectEffect;
        const FxEffectDef *worldShellEjectEffect;
        const FxEffectDef *viewLastShotEjectEffect;
        const FxEffectDef *worldLastShotEjectEffect;
        Material *reticleCenter;
        Material *reticleSide;
        int iReticleCenterSize;
        int iReticleSideSize;
        int iReticleMinOfs;
        activeReticleType_t activeReticleType;
        float vStandMove[3];
        float vStandRot[3];
        float vDuckedOfs[3];
        float vDuckedMove[3];
        float vDuckedSprintOfs[3];
        float vDuckedSprintRot[3];
        float vDuckedSprintBob[2];
        float fDuckedSprintCycleScale;
        float vSprintOfs[3];
        float vSprintRot[3];
        float vSprintBob[2];
        float fSprintCycleScale;
        float vLowReadyOfs[3];
        float vLowReadyRot[3];
        float vDtpOfs[3];
        float vDtpRot[3];
        float vDtpBob[2];
        float fDtpCycleScale;
        float vMantleOfs[3];
        float vMantleRot[3];
        float vSlideOfs[3];
        float vSlideRot[3];
        float vDuckedRot[3];
        float vProneOfs[3];
        float vProneMove[3];
        float vProneRot[3];
        float vStrafeMove[3];
        float vStrafeRot[3];
        float fPosMoveRate;
        float fPosProneMoveRate;
        float fStandMoveMinSpeed;
        float fDuckedMoveMinSpeed;
        float fProneMoveMinSpeed;
        float fPosRotRate;
        float fPosProneRotRate;
        float fStandRotMinSpeed;
        float fDuckedRotMinSpeed;
        float fProneRotMinSpeed;
        XModel **worldModel;
        XModel *worldClipModel;
        XModel *rocketModel;
        XModel *mountedModel;
        XModel *additionalMeleeModel;
        Material *hudIcon;
        weaponIconRatioType_t hudIconRatio;
        Material *indicatorIcon;
        weaponIconRatioType_t indicatorIconRatio;
        Material *ammoCounterIcon;
        weaponIconRatioType_t ammoCounterIconRatio;
        ammoCounterClipType_t ammoCounterClip;
        int iStartAmmo;
        int iHeatIndex;
        int iMaxAmmo;
        int shotCount;
        const char *szSharedAmmoCapName;
        int iSharedAmmoCapIndex;
        int iSharedAmmoCap;
        bool unlimitedAmmo;
        bool ammoCountClipRelative;
        // padding byte
        // padding byte
        int damage;
        float damageDuration;
        float damageInterval;
        int playerDamage;
        int iMeleeDamage;
        int iDamageType;
        unsigned __int16 explosionTag;
        // padding byte
        // padding byte
        int iFireDelay;
        int iMeleeDelay;
        int meleeChargeDelay;
        int iDetonateDelay;
        int iSpinUpTime;
        int iSpinDownTime;
        float spinRate;
        const char *spinLoopSound;
        const char *spinLoopSoundPlayer;
        const char *startSpinSound;
        const char *startSpinSoundPlayer;
        const char *stopSpinSound;
        const char *stopSpinSoundPlayer;
        int iFireTime;
        int iLastFireTime;
        int iRechamberTime;
        int iRechamberBoltTime;
        int iHoldFireTime;
        int iDetonateTime;
        int iMeleeTime;
        int meleeChargeTime;
        int iReloadTimeRight;
        int iReloadTimeLeft;
        int reloadShowRocketTime;
        int iReloadEmptyTimeLeft;
        int iReloadAddTime;
        int iReloadEmptyAddTime;
        int iReloadQuickAddTime;
        int iReloadQuickEmptyAddTime;
        int iReloadStartTime;
        int iReloadStartAddTime;
        int iReloadEndTime;
        int iDropTime;
        int iRaiseTime;
        int iAltDropTime;
        int quickDropTime;
        int quickRaiseTime;
        int iFirstRaiseTime;
        int iEmptyRaiseTime;
        int iEmptyDropTime;
        int sprintInTime;
        int sprintLoopTime;
        int sprintOutTime;
        int lowReadyInTime;
        int lowReadyLoopTime;
        int lowReadyOutTime;
        int contFireInTime;
        int contFireLoopTime;
        int contFireOutTime;
        int dtpInTime;
        int dtpLoopTime;
        int dtpOutTime;
        int slideInTime;
        int deployTime;
        int breakdownTime;
        int nightVisionWearTime;
        int nightVisionWearTimeFadeOutEnd;
        int nightVisionWearTimePowerUp;
        int nightVisionRemoveTime;
        int nightVisionRemoveTimePowerDown;
        int nightVisionRemoveTimeFadeInStart;
        int fuseTime;
        int aiFuseTime;
        int lockOnRadius;
        int lockOnSpeed;
        bool requireLockonToFire;
        bool noAdsWhenMagEmpty;
        bool avoidDropCleanup;
        // padding byte
        unsigned int stackFire;
        float stackFireSpread;
        float stackFireAccuracyDecay;
        const char *stackSound;
        float autoAimRange;
        float aimAssistRange;
        bool mountableWeapon;
        // padding byte
        // padding byte
        // padding byte
        float aimPadding;
        float enemyCrosshairRange;
        bool crosshairColorChange;
        // padding byte
        // padding byte
        // padding byte
        float moveSpeedScale;
        float adsMoveSpeedScale;
        float sprintDurationScale;
        weapOverlayReticle_t overlayReticle;
        WeapOverlayInteface_t overlayInterface;
        float overlayWidth;
        float overlayHeight;
        float fAdsBobFactor;
        float fAdsViewBobMult;
        float fHipSpreadStandMin;
        float fHipSpreadDuckedMin;
        float fHipSpreadProneMin;
        float hipSpreadStandMax;
        float hipSpreadDuckedMax;
        float hipSpreadProneMax;
        float fHipSpreadDecayRate;
        float fHipSpreadFireAdd;
        float fHipSpreadTurnAdd;
        float fHipSpreadMoveAdd;
        float fHipSpreadDuckedDecay;
        float fHipSpreadProneDecay;
        float fHipReticleSidePos;
        float fAdsIdleAmount;
        float fHipIdleAmount;
        float adsIdleSpeed;
        float hipIdleSpeed;
        float fIdleCrouchFactor;
        float fIdleProneFactor;
        float fGunMaxPitch;
        float fGunMaxYaw;
        float swayMaxAngle;
        float swayLerpSpeed;
        float swayPitchScale;
        float swayYawScale;
        float swayHorizScale;
        float swayVertScale;
        float swayShellShockScale;
        float adsSwayMaxAngle;
        float adsSwayLerpSpeed;
        float adsSwayPitchScale;
        float adsSwayYawScale;
        bool sharedAmmo;
        bool bRifleBullet;
        bool armorPiercing;
        bool bBoltAction;
        bool bUseAltTagFlash;
        bool bUseAntiLagRewind;
        bool bIsCarriedKillstreakWeapon;
        bool aimDownSight;
        bool bRechamberWhileAds;
        bool bReloadWhileAds;
        // padding byte
        // padding byte
        float adsViewErrorMin;
        float adsViewErrorMax;
        bool bCookOffHold;
        bool bClipOnly;
        bool bCanUseInVehicle;
        bool bNoDropsOrRaises;
        bool adsFireOnly;
        bool cancelAutoHolsterWhenEmpty;
        bool suppressAmmoReserveDisplay;
        bool laserSightDuringNightvision;
        bool bHideThirdPerson;
        bool bHasBayonet;
        bool bDualWield;
        bool bExplodeOnGround;
        bool bThrowBack;
        bool bRetrievable;
        bool bDieOnRespawn;
        bool bNoThirdPersonDropsOrRaises;
        bool bContinuousFire;
        bool bNoPing;
        bool bForceBounce;
        bool bUseDroppedModelAsStowed;
        bool bNoQuickDropWhenEmpty;
        bool bKeepCrosshairWhenADS;
        bool bUseOnlyAltWeaoponHideTagsInAltMode;
        // padding byte
        Material *killIcon;
        weaponIconRatioType_t killIconRatio;
        bool flipKillIcon;
        bool bNoPartialReload;
        bool bSegmentedReload;
        bool bNoADSAutoReload;
        int iReloadAmmoAdd;
        int iReloadStartAdd;
        const char *szSpawnedGrenadeWeaponName;
        const char *szDualWieldWeaponName;
        unsigned int dualWieldWeaponIndex;
        int iDropAmmoMin;
        int iDropAmmoMax;
        int iDropClipAmmoMin;
        int iDropClipAmmoMax;
        bool blocksProne;
        bool bShowIndicator;
        // padding byte
        // padding byte
        int isRollingGrenade;
        int iExplosionRadius;
        int iExplosionRadiusMin;
        int iIndicatorRadius;
        int iExplosionInnerDamage;
        int iExplosionOuterDamage;
        float damageConeAngle;
        int iProjectileSpeed;
        int iProjectileSpeedUp;
        int iProjectileSpeedRelativeUp;
        int iProjectileSpeedForward;
        int iProjectileActivateDist;
        float projLifetime;
        float timeToAccelerate;
        float projectileCurvature;
        XModel *projectileModel;
        weapProjExposion_t projExplosion;
        const FxEffectDef *projExplosionEffect;
        bool projExplosionEffectForceNormalUp;
        // padding byte
        // padding byte
        // padding byte
        const FxEffectDef *projExplosionEffect2;
        bool projExplosionEffect2ForceNormalUp;
        // padding byte
        // padding byte
        // padding byte
        const FxEffectDef *projExplosionEffect3;
        bool projExplosionEffect3ForceNormalUp;
        // padding byte
        // padding byte
        // padding byte
        const FxEffectDef *projExplosionEffect4;
        bool projExplosionEffect4ForceNormalUp;
        // padding byte
        // padding byte
        // padding byte
        const FxEffectDef *projExplosionEffect5;
        bool projExplosionEffect5ForceNormalUp;
        // padding byte
        // padding byte
        // padding byte
        const FxEffectDef *projDudEffect;
        const char *projExplosionSound;
        const char *projDudSound;
        const char *mortarShellSound;
        const char *tankShellSound;
        bool bProjImpactExplode;
        bool bBulletImpactExplode;
        // padding byte
        // padding byte
        WeapStickinessType stickiness;
        WeapRotateType rotateType;
        bool plantable;
        bool hasDetonator;
        bool timedDetonation;
        bool bNoCrumpleMissile;
        bool rotate;
        bool bKeepRolling;
        bool holdButtonToThrow;
        bool offhandHoldIsCancelable;
        bool freezeMovementWhenFiring;
        // padding byte
        // padding byte
        // padding byte
        float lowAmmoWarningThreshold;
        float meleeChargeRange;
        bool bUseAsMelee;
        bool isCameraSensor;
        bool isAcousticSensor;
        // padding byte
        float *parallelBounce;
        float *perpendicularBounce;
        const FxEffectDef *projTrailEffect;
        float vProjectileColor[3];
        guidedMissileType_t guidedMissileType;
        float maxSteeringAccel;
        int projIgnitionDelay;
        const FxEffectDef *projIgnitionEffect;
        const char *projIgnitionSound;
        float fAdsAimPitch;
        float fAdsCrosshairInFrac;
        float fAdsCrosshairOutFrac;
        int adsGunKickReducedKickBullets;
        float adsGunKickReducedKickPercent;
        float fAdsGunKickPitchMin;
        float fAdsGunKickPitchMax;
        float fAdsGunKickYawMin;
        float fAdsGunKickYawMax;
        float fAdsGunKickAccel;
        float fAdsGunKickSpeedMax;
        float fAdsGunKickSpeedDecay;
        float fAdsGunKickStaticDecay;
        float fAdsViewKickPitchMin;
        float fAdsViewKickPitchMax;
        float fAdsViewKickYawMin;
        float fAdsViewKickYawMax;
        float fAdsViewScatterMin;
        float fAdsViewScatterMax;
        float fAdsSpread;
        int hipGunKickReducedKickBullets;
        float hipGunKickReducedKickPercent;
        float fHipGunKickPitchMin;
        float fHipGunKickPitchMax;
        float fHipGunKickYawMin;
        float fHipGunKickYawMax;
        float fHipGunKickAccel;
        float fHipGunKickSpeedMax;
        float fHipGunKickSpeedDecay;
        float fHipGunKickStaticDecay;
        float fHipViewKickPitchMin;
        float fHipViewKickPitchMax;
        float fHipViewKickYawMin;
        float fHipViewKickYawMax;
        float fHipViewScatterMin;
        float fHipViewScatterMax;
        float fightDist;
        float maxDist;
        const char *accuracyGraphName[2];
        float (*accuracyGraphKnots[2])[2];
        float (*originalAccuracyGraphKnots[2])[2];
        int accuracyGraphKnotCount[2];
        int originalAccuracyGraphKnotCount[2];
        int iPositionReloadTransTime;
        float leftArc;
        float rightArc;
        float topArc;
        float bottomArc;
        float accuracy;
        float aiSpread;
        float playerSpread;
        float minTurnSpeed[2];
        float maxTurnSpeed[2];
        float pitchConvergenceTime;
        float yawConvergenceTime;
        float suppressTime;
        float maxRange;
        float fAnimHorRotateInc;
        float fPlayerPositionDist;
        const char *szUseHintString;
        const char *dropHintString;
        int iUseHintStringIndex;
        int dropHintStringIndex;
        float horizViewJitter;
        float vertViewJitter;
        const char *szScript;
        int minDamage;
        int minPlayerDamage;
        float fMaxDamageRange;
        float fMinDamageRange;
        float destabilizationRateTime;
        float destabilizationCurvatureMax;
        int destabilizeDistance;
        float *locationDamageMultipliers;
        const char *fireRumble;
        const char *meleeImpactRumble;
        const char *reloadRumble;
        float adsDofStart;
        float adsDofEnd;
        float hipDofStart;
        float hipDofEnd;
        float scanSpeed;
        float scanAccel;
        int scanPauseTime;
        const char *flameTableFirstPerson;
        const char *flameTableThirdPerson;
        flameTable *flameTableFirstPersonPtr;
        flameTable *flameTableThirdPersonPtr;
        const FxEffectDef *tagFx_preparationEffect;
        const FxEffectDef *tagFlash_preparationEffect;
        bool doGibbing;
        // padding byte
        // padding byte
        // padding byte
        float maxGibDistance;
};

struct WeaponVariantDef // sizeof=0xE4
{                                                                             // XREF: XAssetPoolEntry<WeaponVariantDef>/r
    const char *szInternalName;
    int iVariantCount;
    WeaponDef *weapDef;
    const char *szDisplayName;
    const char **szXAnims;
    const char *szAltWeaponName;
    unsigned __int16 *hideTags;
    unsigned int altWeaponIndex;
    int iClipSize;
    int iReloadTime;
    int iReloadEmptyTime;
    int iReloadQuickTime;
    int iReloadQuickEmptyTime;
    int iAdsTransInTime;
    int iAdsTransOutTime;
    int iAltRaiseTime;
    const char *szAmmoName;
    int iAmmoIndex;
    const char *szClipName;
    int iClipIndex;
    float fAimAssistRangeAds;
    float fAdsSwayHorizScale;
    float fAdsSwayVertScale;
    float fAdsViewKickCenterSpeed;
    float fHipViewKickCenterSpeed;
    float fAdsZoomFov1;
    float fAdsZoomFov2;
    float fAdsZoomFov3;
    float fAdsZoomInFrac;
    float fAdsZoomOutFrac;
    float fOverlayAlphaScale;
    float fOOPosAnimLength[2];
    bool bSilenced;
    bool bDualMag;
    bool bFullMetalJacket;
    bool bHollowPoint;
    bool bRapidFire;
    // padding byte
    // padding byte
    // padding byte
    Material *overlayMaterial;
    Material *overlayMaterialLowRes;
    Material *dpadIcon;
    weaponIconRatioType_t dpadIconRatio;
    float ikLeftHandOffset[3];
    float ikLeftHandRotation[3];
    float ikLeftHandProneOffset[3];
    float ikLeftHandProneRotation[3];
    float ikLeftHandUiViewerOffset[3];
    float ikLeftHandUiViewerRotation[3];
};

struct weaponParms // sizeof=0x44
{                                       // XREF: ?Actor_Melee@@YAPAUgentity_s@@PAUactor_s@@QBM@Z/r
                                        // Fire_Lead/r ...
    float forward[3];                   // XREF: GScr_MagicBullet(void)+16D/w
                                        // GScr_MagicBullet(void)+17A/w ...
    float right[3];                     // XREF: GScr_MagicBullet(void)+194/w
                                        // GScr_MagicBullet(void)+1A1/w ...
    float up[3];                        // XREF: GScr_MagicBullet(void)+1BB/w
                                        // GScr_MagicBullet(void)+1C8/w ...
    float muzzleTrace[3];               // XREF: GScr_MagicBullet(void)+F6/w
                                        // GScr_MagicBullet(void)+103/w ...
    float gunForward[3];                // XREF: VEH_DebugAim(gentity_s *,float const * const,int)+83/w
                                        // VEH_DebugAim(gentity_s *,float const * const,int)+90/w ...
    const WeaponVariantDef *weapVariantDef;
                                        // XREF: FireWeapon(gentity_s *,int,int):loc_697B7A/r
    const WeaponDef *weapDef;           // XREF: G_UpdateWeapons+72/r
                                        // G_UpdateWeapons+7B/r ...
};

struct WeaponVariantDefHash // sizeof=0x8
{                                       // XREF: .data:bg_weaponVariantNameHashTable/r
                                        // ?BG_FindWeaponIndexForName@@YAHPBD@Z/r
    int hash;                           // XREF: BG_InitDefaultWeaponDef(void)+5C/w
                                        // BG_FindWeaponIndexForName(char const *)+52/w
    int weaponIndex;                    // XREF: BG_InitDefaultWeaponDef(void)+3C/w
};

struct pmove_t;
struct playerState_s;
struct pml_t;
struct WeaponVariantDef;

void __cdecl BG_LoadPenetrationDepthTable();
void __cdecl BG_ParsePenetrationDepthTable(const char *penetrateType, float *depthTable, char *buffer);
char __cdecl BG_AdvanceTrace(BulletFireParams *bp, BulletTraceResults *br, float dist);
double __cdecl BG_GetSurfacePenetrationDepth(const WeaponDef *weapDef, unsigned int surfaceType);
void __cdecl PM_StartWeaponAnim(playerState_s *ps, int anim, int leftAnim);
const char *__cdecl BG_GetImpactTypeName(int impactTypeIndex);
unsigned int __cdecl BG_GetViewmodelWeaponIndex(const playerState_s *ps);
int __cdecl BG_GetFirstAvailableOffhand(const playerState_s *ps, int offhandSlot);
int __cdecl BG_GetFirstEquippedOffhand(const playerState_s *ps, int offhandSlot);
int *__cdecl BG_GetWeaponState(playerState_s *ps, bool leftGun);
bool __cdecl BG_IsAimDownSightWeapon(unsigned int weaponIndex);
bool __cdecl BG_CanPlayerHaveWeapon(unsigned int weaponIndex);
bool __cdecl BG_ValidateWeaponNumber(unsigned int weaponIndex);
bool __cdecl BG_IsWeaponValid(const playerState_s *ps, unsigned int weaponIndex);
bool __cdecl BG_WeaponBlocksProne(unsigned int weapIndex);
bool __cdecl BG_IsOverheatingWeapon(unsigned int weapIndex);
bool __cdecl BG_IsBayonetWeapon(unsigned int weapIndex);
bool __cdecl BG_IsUseAsMeleeWeapon(unsigned int weapIndex);
int __cdecl BG_TakePlayerWeapon(playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_TakeHeldWeapon(playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_GetSpreadForWeapon(
                const playerState_s *ps,
                const WeaponDef *weapDef,
                float *minSpread,
                float *maxSpread);
void __cdecl PM_UpdateAimDownSightFlag(pmove_t *pm, pml_t *pml);
bool __cdecl PM_IsAdsAllowed(pmove_t *pm, pml_t *pml);
void __cdecl PM_ExitAimDownSight(playerState_s *ps);
void __cdecl PM_UpdateScopeZoom(pmove_t *pm);
void __cdecl PM_UpdateSpinLerp(pmove_t *pm, pml_t *pml);
void __cdecl PM_UpdateAimDownSightLerp(pmove_t *pm, pml_t *pml);
bool __cdecl BG_UsingVehicleWeapon(const playerState_s *ps);
bool __cdecl BG_ADSFullyUp(const playerState_s *ps);
bool __cdecl BG_UsingSniperScope(const playerState_s *ps);
int __cdecl PM_InteruptWeaponWithProneMove(playerState_s *ps);
bool __cdecl BG_PlayerWeaponOverheating(const playerState_s *ps, unsigned int weapon);
void __cdecl BG_PlayerWeaponOverheatUpdate(playerState_s *ps, unsigned int weapon, float amount);
struct PlayerHeldWeapon *__cdecl BG_GetHeldWeapon(playerState_s *ps, unsigned int weaponIndex);
bool __cdecl PM_IsWeaponMinFireTimeElapsed(playerState_s *ps, bool leftGun);
unsigned int __cdecl PM_GetWeaponIndexForHand(playerState_s *ps);
bool __cdecl BG_IsAltSwitch(playerState_s *ps, unsigned int lastWeapon, unsigned int weapon);
bool __cdecl BG_CanFastSwitch(const WeaponDef *weapDef, int weaponState);
char __cdecl BG_IsQuickSwitch(unsigned int oldweapon, unsigned int newweapon, int weaponState);
void __cdecl PM_AdjustAimSpreadScale(pmove_t *pm, pml_t *pml);
bool __cdecl ShotLimitReached(playerState_s *ps, const WeaponDef *weapDef);
unsigned int *__cdecl BG_GetWeaponShotCount(playerState_s *ps, bool leftGun);
int __cdecl PM_GetWeaponFireButton(unsigned int weapon);
void __cdecl PM_Weapon_Idle(playerState_s *ps);
void __cdecl PM_Weapon(pmove_t *pm, pml_t *pml);
void __cdecl PM_UpdateHoldBreath(pmove_t *pm, pml_t *pml);
void __cdecl PM_StartHoldBreath(playerState_s *ps);
void __cdecl PM_EndHoldBreath(playerState_s *ps);
int __cdecl PM_Weapon_CheckForRechamber(playerState_s *ps, int delayedAction);
bool __cdecl BG_PlayerWeaponNeedsRechambered(const playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_PlayerWeaponSetNeedsRechamber(playerState_s *ps, unsigned int weaponIndex, bool needsRechamber);
void __cdecl PM_Weapon_FinishRechamber(playerState_s *ps);
void __cdecl PM_ContinueWeaponAnim(playerState_s *ps, int anim, int leftAnim);
void __cdecl PM_Weapon_FinishWeaponChange(pmove_t *pm, int previousWeaponState);
bool __cdecl BG_PlayerEverHadWeapon(const playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_PlayerSetEverHadWeapon(playerState_s *ps, unsigned int weaponIndex, bool hadWeapon);
void __cdecl PM_Weapon_BeginWeaponRaise(
                playerState_s *ps,
                unsigned int anim,
                unsigned int time,
                float aim,
                int altSwitch);
void __cdecl BG_TakeClipOnlyWeaponIfEmpty(playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetAmmoNotInClip(const playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetAmmoNotInClipForAmmoIndex(const playerState_s *ps, int ammoIndex);
unsigned __int16 __cdecl BG_GetValidPrimaryWeaponForAltMode(
                const playerState_s *ps,
                unsigned int newWeapon,
                unsigned int lastWeaponAltModeSwitch);
void __cdecl PM_Weapon_FinishWeaponRaise(playerState_s *ps);
void __cdecl PM_Weapon_FinishReloadStart(pmove_t *pm, int delayedAction);
void __cdecl PM_SetReloadingState(playerState_s *ps);
bool __cdecl BG_PlayerWeaponQuickReload(const playerState_s *ps, unsigned int weaponIndex);
int *__cdecl BG_GetWeaponTime(playerState_s *ps, bool leftGun);
void __cdecl PM_SetWeaponReloadAddAmmoDelay(playerState_s *ps);
int *__cdecl BG_GetWeaponDelay(playerState_s *ps, bool leftGun);
int __cdecl PM_Weapon_AllowReload(playerState_s *ps);
int __cdecl BG_GetClipSize(unsigned int weaponIndex);
void __cdecl PM_Weapon_ReloadDelayedAction(playerState_s *ps);
void __cdecl BG_PlayerSetWeaponQuickReload(playerState_s *ps, unsigned int weaponIndex, bool quickReload);
void __cdecl PM_Weapon_FinishReload(pmove_t *pm, int delayedAction);
void __cdecl PM_Weapon_FinishReloadEnd(playerState_s *ps);
void __cdecl PM_Weapon_CheckFuelTime(pmove_t *pm, pml_t *pml);
void __cdecl PM_Weapon_CheckForReload(pmove_t *pm);
int __cdecl BG_PlayerFuelTankTime(const playerState_s *ps, unsigned int weaponIndex);
void __cdecl PM_BeginWeaponReload(pmove_t *pm);
bool __cdecl BG_PlayDWThirdPersonReload(playerState_s *ps);
bool __cdecl PM_CheckForReloadInterferingWeaponStates(pmove_t *pm, playerState_s *ps, int weaponState);
bool __cdecl BurstFirePending(playerState_s *ps);
void __cdecl UpdatePendingTriggerPull(pmove_t *pm);
void __cdecl CheckStackedFire(pmove_t *pm);
int __cdecl PM_Weapon_WeaponTimeAdjust(pmove_t *pm, pml_t *pml);
bool __cdecl WeaponUsesBurstCooldown(unsigned int weaponIdx);
void __cdecl PM_Weapon_CheckForChangeWeapon(pmove_t *pm);
void __cdecl PM_BeginWeaponChange(pmove_t *pm, unsigned int newweapon, bool quick);
int __cdecl PM_Weapon_ShouldBeFiring(pmove_t *pm, int delayedAction, bool testOnly);
void __cdecl PM_Weapon_FireWeapon(pmove_t *pm, int delayedAction);
void __cdecl PM_HoldBreathFire(playerState_s *ps);
void __cdecl BG_SwitchWeaponsIfEmpty(playerState_s *ps);
bool __cdecl BG_CheckIfDualWieldEmpty(playerState_s *ps);
void __cdecl PM_Weapon_StartFiring(pmove_t *pm, int delayedAction);
char __cdecl BG_CheckIfLastShot(playerState_s *ps, unsigned int weaponIndex);
int __cdecl PM_Weapon_CheckFiringAmmo(pmove_t *pm);
void __cdecl PM_Weapon_Jam(playerState_s *ps);
void __cdecl PM_Weapon_SetFPSFireAnim(playerState_s *ps);
void __cdecl PM_Weapon_AddFiringAimSpreadScale(playerState_s *ps);
char __cdecl PM_WeaponAllowPlant(pmove_t *pm);
void __cdecl Vec2NormalizeFast(float *v);
void __cdecl DrawPlantDebug(float *start, float *end);
bool __cdecl PM_Weapon_IsHoldingGrenade(pmove_t *pm);
void __cdecl PM_Weapon_MeleeEnd(playerState_s *ps);
const WeaponDef *__cdecl PM_Weapon_GetMeleeWeapDef(playerState_s *ps);
void __cdecl PM_Weapon_MeleeFire(playerState_s *ps);
void __cdecl PM_Weapon_CheckForMelee(pmove_t *pm, pml_t *pml, int delayedAction);
void __cdecl PM_Weapon_MeleeInit(pmove_t *pm);
void __cdecl PM_Weapon_OffHandPrepare(pmove_t *pm);
void __cdecl PM_Weapon_OffHandHold(playerState_s *ps);
void __cdecl PM_Weapon_OffHandStart(pmove_t *pm);
void __cdecl PM_Weapon_OffHand(pmove_t *pm);
void __cdecl PM_Weapon_OffHandEnd(playerState_s *ps);
void __cdecl PM_Weapon_CheckForOffHand(pmove_t *pm);
void __cdecl PM_Weapon_OffHandInit(playerState_s *ps);
void __cdecl PM_SendEmtpyOffhandEvent(playerState_s *ps, OffhandSlot offhandSlot);
char __cdecl PM_UpdateGrenadeThrow(playerState_s *ps, pml_t *pml);
char __cdecl PM_Weapon_CheckGrenadeHold(pmove_t *pm, int delayedAction);
void __cdecl PM_Weapon_CheckForDetonation(pmove_t *pm);
void __cdecl PM_Weapon_CheckForGrenadeThrowCancel(pmove_t *pm);
void __cdecl PM_Detonate(playerState_s *ps, int delayedAction);
void __cdecl PM_Weapon_CheckForNightVision(pmove_t *pm);
void __cdecl PM_Weapon_FinishNightVisionWear(playerState_s *ps);
void __cdecl PM_Weapon_FinishNightVisionRemove(playerState_s *ps);
void __cdecl PM_WeaponSwimIn(playerState_s *ps);
void __cdecl PM_WeaponSwimOut(playerState_s *ps);
void __cdecl Sprint_State_Loop(playerState_s *ps);
void __cdecl ContinuousFire_State_Loop(playerState_s *ps);
void __cdecl Dtp_State_Out(playerState_s *ps);
void __cdecl Dtp_State_Loop(playerState_s *ps);
void __cdecl PM_Weapon_CheckForSprint(pmove_t *pm);
void __cdecl Sprint_State_Raise(playerState_s *ps);
void __cdecl Sprint_State_Drop(playerState_s *ps);
void __cdecl PM_Weapon_CheckForContinuousFire(pmove_t *pm, int delayedAction);
void __cdecl ContinuousFire_State_In(playerState_s *ps);
void __cdecl ContinuousFire_State_Out(playerState_s *ps);
void __cdecl PM_Weapon_CheckForDtp(pmove_t *pm);
void __cdecl Dtp_State_In(playerState_s *ps);
void __cdecl PM_Weapon_CheckForSlide(pmove_t *pm);
void __cdecl Slide_State_In(playerState_s *ps);
void __cdecl PM_Weapon_CheckForDeploy(pmove_t *pm);
void __cdecl Deploy_State_Deploy(playerState_s *ps);
void __cdecl Deploy_State_Breakdown(playerState_s *ps);
void __cdecl PM_Weapon_CheckForMantle(pmove_t *pm);
int __cdecl BG_CheckForUniversalActionDW(playerState_s *ps);
void __cdecl PM_Weapon_Turret(pmove_t *pm, pml_t *pml);
void __cdecl PM_ResetWeaponState(playerState_s *ps);
void __cdecl BG_WeaponFireRecoil(const playerState_s *ps, float *recoilSpeed, float *kickAVel);
int __cdecl BG_PlayerWeaponCountPrimaryTypes(const playerState_s *ps);
bool __cdecl BG_PlayerWeaponsFull_Primaries(const playerState_s *ps);
char __cdecl BG_PlayerHasCompatibleWeapon(const playerState_s *ps, unsigned int weaponIndex);
bool __cdecl BG_ThrowingBackGrenade(const playerState_s *ps);
void __cdecl BG_CalcVehicleTurretWeaponPosOffset(float positionFrac, const WeaponDef *weapDef, float *outOffset);
WeaponVariantDef *__cdecl BG_LoadWeaponVariantDef(const char *name);
WeaponVariantDef *__cdecl BG_LoadWeaponVariantDef_LoadObj(char *name);
WeaponVariantDef *__cdecl BG_LoadWeaponVariantDef_FastFile(const char *name);
void __cdecl BG_AssertOffhandIndexOrNone(unsigned int offHandIndex);
int __cdecl BG_GetFuelTankTime(const playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_SetFuelTankTime(playerState_s *ps, unsigned int weaponIndex, int time);
void __cdecl BG_AddFuelTankTime(playerState_s *ps, unsigned int weaponIndex, int time);
char __cdecl BG_HoldWeapon(playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetFreeHeldWeaponSlot(const playerState_s *ps);
const char *__cdecl BG_WeaponName(unsigned int weapon);
void __cdecl BG_StringCopy(unsigned __int8 *member, const char *keyValue);
