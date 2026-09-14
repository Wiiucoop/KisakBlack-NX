#pragma once

#include <qcommon/ent.h>
#include <game/teams.h>
#include <sound/snd.h>
#include "bg_animation.h"
#include <game/g_scr_vehicle.h>
#include "bg_mantle.h"
#include <gfx_d3d/r_shader_constant_set.h>
#include <cgame_mp/cg_scoreboard_mp.h>

#define ACTOR_CORPSES 36

struct GfxSkinCacheEntry // sizeof=0xC
{                                                                             // XREF: cpose_t/r
        unsigned int frameCount;
        int skinnedCachedOffset;
        unsigned __int16 numSkinnedVerts;
        unsigned __int16 ageCount;
};

struct clientControllers_t // sizeof=0x60
{                                                                             // XREF: clientInfo_t/r
        float angles[6][3];
        float tag_origin_angles[3];
        float tag_origin_offset[3];
};

struct CEntPlayerInfo // sizeof=0x18
{                                                                             // XREF: $251974A72D8ACF7EC8C19B3B5F3F224B/r
                                                                                // ?G_PlayerController@@YAXPBUgentity_s@@QAH@Z/r
        clientControllers_t *control;             // XREF: G_PlayerController(gentity_s const *,int * const)+166/w
        unsigned __int8 tag[6];                         // XREF: G_PlayerController(gentity_s const *,int * const)+133/w
                                                                                // G_PlayerController(gentity_s const *,int * const)+13D/o
        // padding byte
        // padding byte
        float waterHeight;
        int nextWaterHeightCheck;
        int nextRippleTime;
};

struct CEntTurretAngles // sizeof=0x8
{                                                                             // XREF: $4EE5A6C5EBB6111004F076A4831D37E2/r
        float pitch;
        float yaw;
};

struct __declspec(align(4)) CEntTurretInfo // sizeof=0x20
{                                                                             // XREF: $251974A72D8ACF7EC8C19B3B5F3F224B/r
        //$4EE5A6C5EBB6111004F076A4831D37E2 ___u0;
        union //$4EE5A6C5EBB6111004F076A4831D37E2 // sizeof=0x8
        {                                                                             // XREF: CEntTurretInfo/r
            CEntTurretAngles angles;
            const float *viewAngles;
        };
        float barrelPitch;
        float pivotOffset;
        bool playerUsing;
        // padding byte
        // padding byte
        // padding byte
        float firingOffset;
        unsigned __int8 tag_aim;
        unsigned __int8 tag_aim_pivot;
        unsigned __int8 tag_aim_animated;
        unsigned __int8 tag_flash;
        unsigned __int8 tag_weapon;
        // padding byte
        // padding byte
        // padding byte
};

struct CEntVehicleInfo // sizeof=0x90
{                                                                             // XREF: $251974A72D8ACF7EC8C19B3B5F3F224B/r
        __int16 pitch;
        __int16 yaw;
        __int16 roll;
        __int16 barrelPitch;
        __int16 steerYaw;
        __int16 steerPitch;
        __int16 steerRoll;
        __int16 gunnerPitch[4];
        __int16 gunnerYaw[4];
        // padding byte
        // padding byte
        float time;
        unsigned __int8 wheelBoneIndex[6];
        // padding byte
        // padding byte
        float nitrousWheelRotation[6];
        float nitrousWheelYaw[6];
        float wheelHeight[6];
        unsigned __int8 tag_body;
        unsigned __int8 tag_turret;
        unsigned __int8 tag_barrel;
        unsigned __int8 tag_barrel_recoil;
        unsigned __int8 tag_gunner_turret[4];
        unsigned __int8 tag_gunner_barrel[4];
        unsigned __int8 tag_extra_tank_wheels[4];
        unsigned __int8 tag_minigun_spin;
        // padding byte
        __int16 minigun_rotation;
        float extra_wheel_rot_scale;
        float barrelRecoil;
};

struct CEntFx // sizeof=0x8
{                                                                             // XREF: $251974A72D8ACF7EC8C19B3B5F3F224B/r
        int triggerTime;
        unsigned int effect;
};


struct CEntActorInfo // sizeof=0x20
{                                                                             // XREF: $251974A72D8ACF7EC8C19B3B5F3F224B/r
        int proneType;
        float pitch;
        float roll;
        unsigned __int8 headBone;
        // padding byte
        // padding byte
        // padding byte
        //$B5C6484BFC4A9A509FCDCE318554591E ___u4;
        union //$B5C6484BFC4A9A509FCDCE318554591E // sizeof=0x4
        {                                                                             // XREF: CEntActorInfo/r
            float height;
            float yaw;
        };
        float waterHeight;
        int nextWaterHeightCheck;
        int nextRippleTime;
};

struct CEntGeneral // sizeof=0x20
{                                                                             // XREF: $251974A72D8ACF7EC8C19B3B5F3F224B/r
        int contents;
        bool linked;
        // padding byte
        // padding byte
        // padding byte
        float mins[3];
        float maxs[3];
};

struct cpose_t // sizeof=0x174
{                                                                             // XREF: centity_s/r cg_s/r ...
        unsigned __int16 lightingHandle;
        unsigned __int8 eType;
        unsigned __int8 eTypeUnion;
        unsigned __int8 localClientNum;
        // padding byte
        // padding byte
        // padding byte
        volatile unsigned int cullIn;
        unsigned __int8 isRagdoll;
        // padding byte
        // padding byte
        // padding byte
        int ragdollHandle;
        int killcamRagdollHandle;
        int physObjId;
        int physUserBody;
        int physUserBodyProneFeet;
        unsigned __int8 destructiblePose;
        // padding byte
        // padding byte
        // padding byte
        int startBurnTime;
        float wetness;
        float origin[3];
        float angles[3];
        float absmin[3];
        float absmax[3];
        GfxSkinCacheEntry skinCacheEntry;
        //$251974A72D8ACF7EC8C19B3B5F3F224B ___u19;
        union //$251974A72D8ACF7EC8C19B3B5F3F224B // sizeof=0x90
        {                                                                             // XREF: cpose_t/r
            CEntPlayerInfo player;
            CEntTurretInfo turret;
            CEntVehicleInfo vehicle;
            CEntFx fx;
            CEntActorInfo actor;
            CEntGeneral general;
        };
        ShaderConstantSet constantSet;
};

struct XAnimTree_s;
struct Destructible;
struct NitrousVehicle;

struct __declspec(align(4)) cLinkInfo_s // sizeof=0x48
{                                                                             // XREF: cLinkInfo_t/r
        float axis[4][3];
        int linkEnt;
        int linkTag;
        float relative_angles[3];
        bool angles_set;
        // padding byte
        // padding byte
        // padding byte
};

struct cached_client_tag_t // sizeof=0x30
{                                                                             // XREF: ClientTagCache/r
                                                                                // ClientTagCache/r
        int time;
        float lastLocalTagOrigin[3];
        bool visible;
        // padding byte
        // padding byte
        // padding byte
        int predictedTime;
        float predictedPosition[3];
        float predictedAngles[3];
};

struct ClientTagCache // sizeof=0x60
{
        cached_client_tag_t aim_head_tag;
        cached_client_tag_t aim_highest_tag;
};

struct AimTargetCache // sizeof=0x8
{
        int lastUpdateTime;
        float targetHeight;
};

struct cgScriptMover_s // sizeof=0x60
{
        float decelTime;
        float aDecelTime;
        float speed;
        float aSpeed;
        float midTime;
        float aMidTime;
        float pos1[3];
        float pos2[3];
        float pos3[3];
        float apos1[3];
        float apos2[3];
        float apos3[3];
};

struct Material;

//enum entityType_t : __int32
enum entityType_t : __int16
{                                       // XREF: ?CG_EntityEvent@@YAXHPAUcentity_s@@H@Z/r
    ET_GENERAL         = 0x0,
    ET_PLAYER          = 0x1,
    ET_PLAYER_CORPSE   = 0x2,
    ET_ITEM            = 0x3,
    ET_MISSILE         = 0x4,
    ET_INVISIBLE       = 0x5,
    ET_SCRIPTMOVER     = 0x6,
    ET_SOUND_BLEND     = 0x7,
    ET_FX              = 0x8,
    ET_LOOP_FX         = 0x9,
    ET_PRIMARY_LIGHT   = 0xA,
    ET_MG42            = 0xB,
    ET_HELICOPTER      = 0xC,
    ET_PLANE           = 0xD,
    ET_VEHICLE         = 0xE,
    ET_VEHICLE_COLLMAP = 0xF,
    ET_VEHICLE_CORPSE  = 0x10,
    ET_ACTOR           = 0x11,
    ET_ACTOR_SPAWNER   = 0x12,
    ET_ACTOR_CORPSE    = 0x13,
    ET_STREAMER_HINT   = 0x14,
    ET_EVENTS          = 0x15,
    // Numbers higher than this are used for Temp Entities
    // (ET_EVENTS + entity_event_t) -- (See G_TempEntity())
};

struct centity_s // sizeof=0x328
{                                                                             // XREF: cg_s/r
        cpose_t pose;
        LerpEntityState currentState;
        entityState_s nextState;
        __int16 previousEventSequence;
        // padding byte
        // padding byte
        int miscTime;
        int lastMuzzleFlash;
        unsigned __int16 attachModelNames[2];
        unsigned __int16 attachTagNames[2];
        XAnimTree_s *tree;
        Destructible *destructible;
        NitrousVehicle *nitrousVeh;
        cLinkInfo_s *linkInfo;
        cgVehicle_s *vehicle;
        ClientTagCache *clientTagCache;
        AimTargetCache *aimTargetInfo;
        cgScriptMover_s *cScriptMover;
        int numBulletImpacts;
        int nextSlideFX;
        int fireTime;
        unsigned __int16 flagIndex;
        unsigned __int16 flagState;
        Material *compassMaterial;
        int lastTrailTime;
        unsigned int fxTrailHandle;
        unsigned int fxProjExplosion;
        unsigned int fxHeartbeat;
        centity_s *updateDelayedNext;
        // nx-port: the decompiler rendered this 25-bit flag group as a raw
        // dword at the x86 word index 201 (byte 804) -- *((unsigned int *)cent
        // + 201) -- all over the cgame. On LP64 the ten pointers above widen
        // and the group moves to byte 848, so that literal index lands on
        // nextSlideFX instead. Alias the word so those accesses can name it and
        // follow the struct on both ABIs. The union does not change the layout.
        union
        {
            unsigned int clientFlags;
            struct
            {
                unsigned __int32 applyLeftHandIK : 1;
                unsigned __int32 nextValid : 1;
                unsigned __int32 bMuzzleFlash : 1;
                unsigned __int32 bMuzzleFlashLeft : 1;
                unsigned __int32 bTrailMade : 1;
                unsigned __int32 isBurning : 1;
                unsigned __int32 skipBloodImpacts : 1;
                unsigned __int32 heroBloodImpact : 1;
                unsigned __int32 scriptThreaded : 1;
                unsigned __int32 clientRumbleLoop : 1;
                unsigned __int32 didOverheatFx : 1;
                unsigned __int32 leftFootstep : 1;
                unsigned __int32 rightFootstep : 1;
                unsigned __int32 leftRearFootstep : 1;
                unsigned __int32 rightRearFootstep : 1;
                unsigned __int32 bIsTrigger : 1;
                unsigned __int32 bIsTriggerRadius : 1;
                unsigned __int32 bIsOneShot : 1;
                unsigned __int32 bIsPlaneOnRadar : 1;
                unsigned __int32 bFloatLonger : 1;
                unsigned __int32 bForceBuoyancy : 1;
                unsigned __int32 bIgnoreRagdoll : 1;
                unsigned __int32 bClientFlagsNeedProcessing : 1;
                unsigned __int32 bUpdatedSinceReset : 1;
                unsigned __int32 bUpdateToggle : 1;
            };
        };
        // padding bit
        // padding bit
        // padding bit
        // padding bit
        // padding bit
        // padding bit
        // padding bit
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(centity_s) == 0x328);
#endif

struct fake_centity_s // sizeof=0x32C
{                                                                             // XREF: fake_centity_t/r
        unsigned int flags;
        centity_s cent;
};

union hudelem_color_t // sizeof=0x4
{                                                                             // XREF: DrawSingleHudElem2d+137/r
    //$0D0CB43DF22755AD856C77DD3F304010 __s0;
    struct
    {
            unsigned __int8 r;
            unsigned __int8 g;
            unsigned __int8 b;
            unsigned __int8 a;
    };
    int rgba;
};

struct __declspec(align(4)) hudelem_s // sizeof=0x70
{                                                                             // XREF: .data:g_dummyHudCurrent/r
                                                                                // .data:g_dummyHudCurrent_0/r ...
        float x;
        float y;
        float z;
        float fontScale;
        hudelem_color_t color;
        hudelem_color_t fromColor;
        int fadeStartTime;
        int scaleStartTime;
        float fromX;
        float fromY;
        int moveStartTime;
        int time;
        int duration;
        float value;
        float sort;
        hudelem_color_t glowColor;
        int fxBirthTime;
        __int16 targetEntNum;
        __int16 fadeTime;
        __int16 label;
        __int16 width;
        __int16 height;
        __int16 fromWidth;
        __int16 fromHeight;
        __int16 scaleTime;
        __int16 moveTime;
        __int16 text;
        unsigned __int16 fxLetterTime;
        unsigned __int16 fxDecayStartTime;
        unsigned __int16 fxDecayDuration;
        unsigned __int16 fxRedactDecayStartTime;
        unsigned __int16 fxRedactDecayDuration;
        unsigned __int16 flags;
        unsigned __int8 type;                             // XREF: HudElem_Alloc(int,int)+25/r
                                                                                // HudElem_ClientDisconnect(gentity_s *)+25/r ...
        unsigned __int8 font;
        unsigned __int8 alignOrg;
        unsigned __int8 alignScreen;
        unsigned __int8 materialIndex;
        unsigned __int8 offscreenMaterialIdx;
        unsigned __int8 fromAlignOrg;
        unsigned __int8 fromAlignScreen;
        unsigned __int8 soundID;
        char ui3dWindow;
        // padding byte
        // padding byte
};


enum OffhandSecondaryClass : __int32
{                                                                             // XREF: playerState_s/r
        PLAYER_OFFHAND_SECONDARY_SMOKE     = 0x0,
        PLAYER_OFFHAND_SECONDARY_FLASH     = 0x1,
        PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
};

enum OffhandPrimaryClass : __int32
{                                                                             // XREF: playerState_s/r
        PLAYER_OFFHAND_PRIMARY_FRAG        = 0x0,
        PLAYER_OFFHAND_PRIMARY_GEAR        = 0x1,
        PLAYER_OFFHAND_PRIMARIES_TOTAL = 0x2,
};


struct __declspec(align(4)) PlayerHeldWeapon // sizeof=0x18
{                                                                             // XREF: playerState_s/r
        unsigned int weapon;
        unsigned __int8 model;
        // padding byte
        // padding byte
        // padding byte
        renderOptions_s options;
        //$687D092DC3F87A5931C7C781A3904E11 ___u3;
        union //$687D092DC3F87A5931C7C781A3904E11 // sizeof=0x4
        {                                                                             // XREF: PlayerHeldWeapon/r
                float heatPercent;
                int fuelTankTime;
        };
        bool overHeating;
        bool needsRechamber;
        bool heldBefore;
        bool quickReload;
        bool blockWeaponPickup;
        // padding byte
        // padding byte
        // padding byte
};

struct AmmoPool // sizeof=0x8
{                                                                             // XREF: playerState_s/r
        int ammoIndex;
        int count;
};

struct AmmoClip // sizeof=0x8
{                                                                             // XREF: playerState_s/r
        int clipIndex;
        int count;
};

enum ViewLockTypes : __int32
{                                                                             // XREF: playerState_s/r
        PLAYERVIEWLOCK_NONE                 = 0x0,
        PLAYERVIEWLOCK_FULL                 = 0x1,
        PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
        PLAYERVIEWLOCKCOUNT                 = 0x3,
};

enum locSel_t : __int32
{                                                                             // XREF: playerState_s/r
                                                                                // PlayerCmd_BeginLocationTypeSelection/r
        LOC_SEL_NONE            = 0x0,
        LOC_SEL_ARTILLERY = 0x1,
        LOC_SEL_AIRSTRIKE = 0x2,
        LOC_SEL_MORTAR        = 0x3,
        LOC_SEL_NAPALM        = 0x4,
        LOC_SEL_COMLINK     = 0x5,
};

struct SprintState // sizeof=0x1C
{                                                                             // XREF: playerState_s/r cg_s/r
        int sprintButtonUpRequired;
        int sprintDelay;
        int lastSprintStart;
        int lastSprintEnd;
        int sprintStartMaxLength;
        int sprintDuration;
        int sprintCooldown;
};

enum ActionSlotType : __int32
{                                                                             // XREF: playerState_s/r
        ACTIONSLOTTYPE_DONOTHING             = 0x0,
        ACTIONSLOTTYPE_SPECIFYWEAPON     = 0x1,
        ACTIONSLOTTYPE_ALTWEAPONTOGGLE = 0x2,
        ACTIONSLOTTYPE_NIGHTVISION         = 0x3,
        ACTIONSLOTTYPECOUNT                        = 0x4,
};

struct ActionSlotParam_SpecifyWeapon // sizeof=0x4
{                                                                             // XREF: ActionSlotParam/r
        unsigned int index;
};

struct ActionSlotParam // sizeof=0x4
{                                                                             // XREF: playerState_s/r
        ActionSlotParam_SpecifyWeapon specifyWeapon;
};

enum objectiveState_t : __int32
{                                                                             // XREF: objective_t/r
                                                                                // Scr_Objective_Add/r ...
        OBJST_EMPTY         = 0x0,
        OBJST_ACTIVE        = 0x1,
        OBJST_INVISIBLE = 0x2,
        OBJST_DONE            = 0x3,
        OBJST_CURRENT     = 0x4,
        OBJST_FAILED        = 0x5,
        OBJST_NUMSTATES = 0x6,
};

struct objective_t // sizeof=0x30
{                                                                             // XREF: playerState_s/r
        objectiveState_t state;
        float origin[3];
        float size[2];
        int entNum;
        int teamNum;
        int icon;
        int use3D;
        int color;
        const char *alt_3D_text;
};

struct playerState_s // sizeof=0x26A4
{                                                                             // XREF: .data:dummy_1/r
                                                                                // .data:playerState_s g_defaultPlayerState/r ...
        int commandTime;                                        // XREF: Demo_SwitchPlayer(int,bool,int,bool)+151/r
                                                                                // Demo_ReadPlayerStates(int,msg_t *,int)+F7/w ...
        int pm_type;                                                // XREF: Demo_IsPlayerDead(int)+E/r
        int bobCycle;
        int pm_flags;
        int weapFlags;
        int otherFlags;
        int pm_time;
        unsigned int loopSoundId;
        int loopSoundFade;
        float origin[3];
        float velocity[3];
        int weaponTime;
        int weaponDelay;
        int weaponTimeLeft;
        int weaponDelayLeft;
        int weaponIdleTime;
        int grenadeTimeLeft;
        int throwBackGrenadeOwner;
        int throwBackGrenadeTimeLeft;
        int weaponRestrictKickTime;
        bool mountAvailable;
        bool bRunLeftGun;
        bool bCarryingTurret;
        // padding byte
        float mountPos[3];
        float mountDir;
        int foliageSoundTime;
        int gravity;
        float leanf;
        int speed;
        float delta_angles[3];
        int groundEntityNum;
        int groundType;
        float vLadderVec[3];
        int jumpTime;
        float jumpOriginZ;
        int slideTime;
        int moveType;
        int legsTimer;
        int torsoTimer;
        __int16 legsAnim;                                     // XREF: Demo_WriteCustomSnapshotInformation(int,msg_t *)+185/r
                                                                                // Demo_ReadCustomSnaphotInformation(int,msg_t *)+1AC/r
        __int16 torsoAnim;                                    // XREF: Demo_WriteCustomSnapshotInformation(int,msg_t *)+C3/r
                                                                                // Demo_ReadCustomSnaphotInformation(int,msg_t *)+110/r
        int legsAnimDuration;
        int torsoAnimDuration;
        int damageTimer;
        int damageDuration;
        int dmgDirection;
        int dmgType;
        int corpseIndex;
        int movementDir;
        int eFlags;
        int eFlags2;
        __int16 predictableEventSequence;
        __int16 predictableEventSequenceOld;
        int predictableEvents[4];
        unsigned int predictableEventParms[4];
        __int16 unpredictableEventSequence;
        __int16 unpredictableEventSequenceOld;
        int unpredictableEvents[4];
        unsigned int unpredictableEventParms[4];
        unsigned __int8 clientNum;                    // XREF: Demo_SwitchPlayer(int,bool,int,bool)+163/r
                                                                                // Demo_GenerateUncompressedSnapshot(int,msg_t *,int,int,int,int,int,bool)+275/r
        // padding byte
        // padding byte
        // padding byte
        int offHandIndex;
        OffhandSecondaryClass offhandSecondary;
        OffhandPrimaryClass offhandPrimary;
        renderOptions_s renderOptions;
        unsigned __int16 weapon;
        unsigned __int16 lastStandPrevWeapon;
        unsigned int lastWeaponAltModeSwitch;
        unsigned int stowedWeapon;
        unsigned __int8 stowedWeaponCamo;
        // padding byte
        // padding byte
        // padding byte
        unsigned int meleeWeapon;
        //int weaponstate;
        weaponstate_t weaponstate;
        int weaponstateLeft;
        unsigned int weaponShotCount;
        unsigned int weaponShotCountLeft;
        float fWeaponPosFrac;
        int adsDelayTime;
        int spreadOverride;
        int spreadOverrideState;
        float weaponSpinLerp;
        int viewmodelIndex;
        float viewangles[3];
        int viewHeightTarget;
        float viewHeightCurrent;
        int viewHeightLerpTime;
        int viewHeightLerpTarget;
        int viewHeightLerpDown;
        float viewAngleClampBase[2];
        float viewAngleClampRange[2];
        unsigned int stackFireCount;
        int damageEvent;
        int damageYaw;
        int damagePitch;
        int damageCount;
        int stats[5];
        PlayerHeldWeapon heldWeapons[15];
        AmmoPool ammoNotInClip[15];
        AmmoClip ammoInClip[15];
        float proneDirection;
        float proneDirectionPitch;
        float proneTorsoPitch;
        ViewLockTypes viewlocked;
        __int16 viewlocked_entNum;
        // padding byte
        // padding byte
        int vehiclePos;
        int vehicleType;
        int vehicleAnimBoneIndex;
        int linkFlags;
        float linkAngles[3];
        int cursorHint;
        int cursorHintString;
        int cursorHintEntIndex;
        int iCompassPlayerInfo;
        unsigned int spyplaneTypeEnabled;
        unsigned int satelliteTypeEnabled;
        locSel_t locationSelectionType;
        int locationSelectionInfo;
        SprintState sprintState;
        int lastDtpEnd;
        float fTorsoPitch;
        float fWaistPitch;
        float holdBreathScale;
        int holdBreathTimer;
        float moveSpeedScaleMultiplier;
        MantleState mantleState;
        int vehicleAnimStage;
        int vehicleEntryPoint;
        unsigned int scriptedAnim;
        int scriptedAnimTime;
        float meleeChargeYaw;
        int meleeChargeDist;
        int meleeChargeTime;
        int weapLockFlags;
        int weapLockedEntnum;
        unsigned int perks[2];
        ActionSlotType actionSlotType[4];
        ActionSlotParam actionSlotParam[4];
        int entityEventSequence;
        int weapAnim;
        int weapAnimLeft;
        float aimSpreadScale;
        int shellshockIndex;
        int shellshockTime;
        int shellshockDuration;
        float dofNearStart;
        float dofNearEnd;
        float dofFarStart;
        float dofFarEnd;
        float dofNearBlur;
        float dofFarBlur;
        float dofViewmodelStart;
        float dofViewmodelEnd;
        int waterlevel;
        int smokeColorIndex;
        int hudElemLastAssignedSoundID;
        int adsZoomSelect;
        int adsZoomLatchTime;
        bool adsZoomLatchState;
        // padding byte
        // padding byte
        // padding byte
        int artilleryInboundIconLocation;
        float visionSetLerpRatio;
        int poisoned;
        int binoculars;
        int scriptCursorHintString;
        objective_t objective[32];
        int deltaTime;
        int killCamEntity;
        int killCamTargetEntity;
        //playerState_s::<unnamed_type_hud> hud;
        struct// playerState_s_s // sizeof=0x1B20
        {                                                                             // XREF: playerState_s/r
                hudelem_s current[31];
                hudelem_s archival[31];
        } hud;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(playerState_s) == 9892);
#endif

struct lerpFrame_t // sizeof=0x34
{                                                                             // XREF: clientInfo_t/r
                                                                                // clientInfo_t/r ...
        float yawAngle;                                         // XREF: ClientEndFrame(gentity_s *)+E72/r
                                                                                // ClientEndFrame(gentity_s *)+E8B/r
        int yawing;
        float pitchAngle;
        int pitching;
        int animationNumber;
        struct animation_s *animation;
        int animationTime;
        float oldFramePos[3];
        float oldFrameYaw;
        float animSpeedScale;
        int oldFrameSnapshotTime;
};

struct __declspec(align(4)) clientInfo_t // sizeof=0x5C8
{                                                                             // XREF: bgs_t/r corpseInfo_t/r
        int infoValid;                                            // XREF: G_RunFrame(int)+906/r
        int nextValid;
        int clientNum;
        char name[32];
        team_t team;                                                // XREF: Weapon_Melee_internal+1F5/r
                                                                                // Weapon_Melee_internal+245/r ...
        team_t oldteam;
        ffa_team_t ffaTeam;
        int rank;
        int prestige;
        int needsRevive;
        unsigned int perks[2];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 xuid;
        char clanAbbrev[8];
        score_s score;
        Material *hStatusIcon;
        Material *hRankIcon;
        int location;
        int health;
        char model[64];
        char attachModelNames[6][64];
        char attachTagNames[6][64];
        lerpFrame_t legs;                                     // XREF: ClientEndFrame(gentity_s *)+E8B/r
        lerpFrame_t torso;                                    // XREF: ClientEndFrame(gentity_s *)+E72/r
        float lerpMoveDir;
        float lerpLean;
        float playerAngles[3];                            // XREF: ClientEndFrame(gentity_s *)+E59/r
        int leftHandGun;
        int dobjDirty;                                            // XREF: ScrCmd_attach+275/w
        clientControllers_t control;
        int lastDamageTime;
        int lastStandStartTime;
        __int64 moveType;
        __int64 eventType;
        unsigned int clientConditions[23];
        XAnimTree_s *pXAnimTree;                        // XREF: G_FreeAnimTreeInstances+2D/r
                                                                                // G_FreeAnimTreeInstances+43/r ...
        unsigned int iDObjWeapon;
        unsigned __int8 weaponModel;
        // padding byte
        // padding byte
        // padding byte
        unsigned int iDObjOffhandWeapon;
        unsigned __int8 offhandWeaponModel;
        // padding byte
        // padding byte
        // padding byte
        unsigned int iDObjMeleeWeapon;
        unsigned __int8 meleeWeaponModel;
        // padding byte
        // padding byte
        // padding byte
        int stanceTransitionTime;
        int turnAnimEndTime;
        char turnAnimType;
        // padding byte
        // padding byte
        // padding byte
        int attachedVehEntNum;
        int attachedVehSeat;
        bool hideWeapon;
        // padding byte
        // padding byte
        // padding byte
        unsigned int weaponHideBits[5];
        bool usingKnife;
        bool usingGrenade;
        // padding byte
        // padding byte
        int nextHideWeaponTime;
        float vehAnimLastOrg[3];
        float vehAnimLastAngle[3];
        int vehAnimLastBone;
        int vehAnimLastTime;
        int vehAnimLerpStartTime;
        int vehAnimLerpEndTime;
        float weaponSpinLerp;
        float lastWeaponSpinLerp;
        float weaponSpin;
        bool minigunWasFiring;
        // padding byte
        // padding byte
        // padding byte
        int clientUIVisibilityFlags;
        int teleportTime;
        bool teleportBit;
        // padding byte
        // padding byte
        // padding byte
};


struct loadAnim_t // sizeof=0x48
{                                                                             // XREF: playerAnims_t/r
        scr_anim_s anim;
        int iNameHash;
        char szAnimName[64];
};

struct actorLerpFrame_t // sizeof=0x8
{                                                                             // XREF: animInfo_t/r
        int bAnimating;
        int animationState;
};

struct animInfo_t // sizeof=0x3C
{                                                                             // XREF: actorInfo_t/r
        actorLerpFrame_t lerp;
        lerpFrame_t legs;
};

struct actorInfo_t // sizeof=0x54
{                                                                             // XREF: bgs_t/r cgs_t/r
        int infoValid;
        int nextValid;
        int actorNum;
        int entityNum;
        animInfo_t animInfo;
        int dobjDirty;
        XAnimTree_s *pXAnimTree;
};

struct pml_t // sizeof=0x8C
{                                       // XREF: ?Update@UIViewer@@QAEXM@Z/r
                                        // PmoveSingle/r
    float forward[3];
    float right[3];                     // XREF: PmoveSingle+D4C/o
                                        // PmoveSingle+DA4/o
    float up[3];                        // XREF: PmoveSingle+D76/w
                                        // PmoveSingle+D86/w ...
    float frametime;                    // XREF: PmoveSingle+CE0/w
                                        // PmoveSingle+157F/r ...
    int msec;                           // XREF: PmoveSingle+C30/w
                                        // PmoveSingle+C33/r ...
    int walking;                        // XREF: PmoveSingle+1133/w
                                        // PmoveSingle+132C/w ...
    int groundPlane;                    // XREF: PmoveSingle+1125/w
                                        // PmoveSingle+131E/w ...
    int almostGroundPlane;              // XREF: PmoveSingle+112C/w
                                        // PmoveSingle+1325/w
    trace_t groundTrace;                // XREF: PmoveSingle+13/o
                                        // UIViewer::Update(float)+5F2/o
    float impactSpeed;
    float previous_origin[3];           // XREF: PmoveSingle+C73/w
                                        // PmoveSingle+C83/w ...
    float previous_velocity[3];         // XREF: PmoveSingle+CAE/w
                                        // PmoveSingle+CBE/w ...
};

struct bgsAnim_s;
struct bgs_t // sizeof=0xBF00
{                                       // XREF: .data:bgs_t level_bgs/r
                                        // cg_s/r ...
    bgsAnim_s *animData;                // XREF: G_PlayerVehiclePositionAndBlend(gentity_s *,gentity_s *)+4F2/r
                                        // G_InitGame(int,int,int,int,int)+127/w ...
    int time;                           // XREF: G_RunFrame(int)+5A/w
    int latestSnapshotTime;             // XREF: G_RunFrame(int)+66/w
    int frametime;                      // XREF: G_RunFrame(int)+71/w
    int anim_user;                      // XREF: G_InitGame(int,int,int,int,int)+16D/w
    int proneTime[32];
    struct XModel *(__cdecl *GetXModel)(char *);
                                        // XREF: G_InitGame(int,int,int,int,int)+131/w
    void (__cdecl *CreateDObj)(struct DObjModel_s *, unsigned __int16, XAnimTree_s *, int, int, clientInfo_t *);
                                        // XREF: G_InitGame(int,int,int,int,int)+13B/w
    unsigned __int16 (__cdecl *AttachWeapon)(struct DObjModel_s *, unsigned __int16, clientInfo_t *);
                                        // XREF: G_InitGame(int,int,int,int,int)+145/w
    struct DObj *(__cdecl *GetDObj)(unsigned int, int); // XREF: G_InitGame(int,int,int,int,int)+14F/w
    void (__cdecl *SafeDObjFree)(unsigned int, int);
                                        // XREF: G_InitGame(int,int,int,int,int)+159/w
    void *(__cdecl *AllocXAnim)(unsigned int);   // XREF: G_InitGame(int,int,int,int,int)+163/w
    int (__cdecl *Rand)();              // XREF: G_InitGame(int,int,int,int,int)+177/w
    float (__cdecl *Random)();          // XREF: G_InitGame(int,int,int,int,int)+181/w
    void (__cdecl *AnimCmdRefCount)(__int16, __int16, int);
    void (__cdecl *AnimCmdAddDependent)(__int16, __int16);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    clientInfo_t clientinfo[32];        // XREF: G_PlayerVehiclePositionAndBlend(gentity_s *,gentity_s *)+A5/o
                                        // G_PlayerController(gentity_s const *,int * const)+62/o ...
    actorInfo_t actorinfo[16];          // XREF: Actor_PostThink(actor_s *)+81/o
                                        // Actor_Negotiation_Think(actor_s *)+2EC/o
};

static const float playerMins[3] = { -15.0, -15.0, 0.0 };
static const float playerMaxs[3] = { 15.0, 15.0, 70.0 };