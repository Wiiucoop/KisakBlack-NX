#pragma once
#include <bgame/bg_local.h>
#include <cstdint>
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <cgame/cg_visionsets.h>
#include <gfx_d3d/r_gfx.h>
#include <cgame/cg_camera.h>
#include "cg_view_mp.h"
#include <cgame/cg_playerstate.h>
#include <gfx_d3d/r_screenshot.h>
#include "cg_compassfriendlies_mp.h"
#include <client_mp/cl_main_mp.h>
#include <cgame_mp/cg_main_mp.h>

#define MAX_LOCAL_CLIENTS 1

enum link_type_e : __int32
{                                       // XREF: cg_s/r
    PITCH_CTRL = 0x0,
    YAW_CTRL   = 0x1,
    ROLL_CTRL  = 0x2,
    SWIM_CTRL  = 0x3,
    FULL_CTRL  = 0x4,
};

enum DemoType : __int32
{                                                                             // XREF: cg_s/r
                                                                                // ?CG_DrawActiveFrame@@YAHHHW4DemoType@@W4CubemapShot@@HH@Z/r ...
        DEMO_TYPE_NONE                        = 0x0,
        DEMO_TYPE_CLIENT                    = 0x1,
        DEMO_TYPE_SERVER                    = 0x2,
        DEMO_TYPE_SERVER_SNAPSHOT = 0x3,
};

enum thirdPersonType : __int32
{                                                                             // XREF: cg_s/r
        TP_OFF                = 0x0,
        TP_FOR_MODEL    = 0x1,
        TP_FOR_SHADOW = 0x2,
};

struct snapshot_s // sizeof=0x20200
{                                                                             // XREF: cg_s/r snapshot_t/r
        int snapFlags;
        int ping;
        int serverTime;
        int physicsTime;
        playerState_s ps;
        int numEntities;
        int numClients;
        entityState_s entities[512];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        clientState_s clients[32];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        MatchState matchState;
        int serverCommandSequence;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct playerEntity_t // sizeof=0x30
{                                                                             // XREF: cg_s/r
        float fLastWeaponPosFrac;
        int bPositionToADS;
        float vPositionLastOrg[3];
        float fLastIdleFactor;
        float baseMoveOrigin[3];
        float baseMoveAngles[3];
};

struct __declspec(align(16)) refdef_s // sizeof=0x18510
{                                                                             // XREF: cg_s/r refdef_t/r
        unsigned int x;
        unsigned int y;
        unsigned int width;
        unsigned int height;
        float tanHalfFovX;
        float tanHalfFovY;
        float fov_x;
        float vieworg[3];
        float yaw;
        float viewaxis[3][3];
        int time;
        float zNear;
        float zFar;
        GfxVisionSet visionset;
        float blurRadius;
        GfxDepthOfField dof;
        GfxDoubleVision doubleVision;
        GfxCompositeFx flameFx;
        GfxCompositeFx waterSheetingFx;
        GfxGenericFilter genericFilter;
        GfxPoison poisonFx;
        GfxCompositeFx electrifiedFx;
        GfxCompositeFx transportedFx;
        GfxSaveScreenFx saveScreenFx;
        float sunVisibility;
        GfxLight primaryLights[255];
        GfxViewport scissorViewport;
        bool useScissorViewport;
        // padding byte
        // padding byte
        // padding byte
        int localClientNum;
        int hideMatureContent;
        int splitscreen;
        int playerTeleported;
        int oldExposureId;
        int newExposureId;
        GfxExposureValue exposureValue;
        float lerpcount;
        int lastTime;
        unsigned int exposureMode;
        float exposure;
        float postEmissiveBrightening;
        bool noLodCullOut;
        // padding byte
        // padding byte
        // padding byte
        WaterFogDef waterFog;
        float extraCamPos[3];
        bool extraCamPosValid;
        bool extraCamLargeFrame;
        // padding byte
        // padding byte
        float preExtraCamVieworg[3];
        float preExtraCamViewaxis[3][3];
        float preExtraCamTanHalfFovX;
        float preExtraCamTanHalfFovY;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct ScriptCamera // sizeof=0x30
{                                       // XREF: Camera/r
    int flags;
    float origin[3];
    float angles[3];
    float lookat[3];
    int originCent;
    int lookAtCent;
};

struct __declspec(align(4)) Camera // sizeof=0xAC
{                                       // XREF: cg_s/r
    float lastViewOrg[3];
    float lastViewAngles[3];
    float lastFOV;
    int lastViewInputTime;
    float lastSpringOffset;
    float lastViewTraceFraction;
    int lastTime;
    int lastClientNum;
    float tweenStartOrg[3];
    float tweenStartAngles[3];
    float tweenStartFOV;
    int tweenStartTime;
    float tweenDuration;
    CameraMode lastCamMode;
    int lastVehicleInfoIndex;
    int lastVehicleSeatPos;
    float missileViewAngles[3];
    bool missileWasKillCam;
    // padding byte
    // padding byte
    // padding byte
    ScriptCamera scriptCam;
    int extraCamEntNum;
    float extraCamFov;
    bool useTagCamera;
    // padding byte
    // padding byte
    // padding byte
};

enum InvalidCmdHintType : __int32
{                                       // XREF: cg_s/r
                                        // CG_SetInvalidCmdHint/r
    INVALID_CMD_NONE                 = 0x0,
    INVALID_CMD_NO_AMMO_BULLETS      = 0x1,
    INVALID_CMD_NO_AMMO_FRAG_GRENADE = 0x2,
    INVALID_CMD_NO_AMMO_SPECIAL_GRENADE = 0x3,
    INVALID_CMD_NO_AMMO_FLASH_GRENADE = 0x4,
    INVALID_CMD_NO_AMMO_EQUIPMENT    = 0x5,
    INVALID_CMD_STAND_BLOCKED        = 0x6,
    INVALID_CMD_CROUCH_BLOCKED       = 0x7,
    INVALID_CMD_TARGET_TOO_CLOSE     = 0x8,
    INVALID_CMD_LOCKON_REQUIRED      = 0x9,
    INVALID_CMD_NOT_ENOUGH_CLEARANCE = 0xA,
    INVALID_CMD_CANT_PLACE_TURRET    = 0xB,
    INVALID_CMD_CANT_EQUIP_WHILE_PRONE = 0xC,
    INVALID_CMD_CANT_PLANT_EQUIPMENT = 0xD,
};

enum invalid_cmd_hint_t : __int32
{                                       // XREF: CG_SetInvalidStanceHint/r
    INVALID_CMD_NO_HINT     = 0x0,
    INVALID_CMD_STAND_HINT  = 0x1,
    INVALID_CMD_CROUCH_HINT = 0x2,
};

enum SuccessfulCmdHintType : __int32
{                                       // XREF: cg_s/r
    SUCCESSFUL_CMD_NONE             = 0x0,
    SUCCESSFUL_CMD_GOD_MODE_ON      = 0x1,
    SUCCESSFUL_CMD_GOD_MODE_OFF     = 0x2,
    SUCCESSFUL_CMD_DEMIGOD_MODE_ON  = 0x3,
    SUCCESSFUL_CMD_DEMIGOD_MODE_OFF = 0x4,
    SUCCESSFUL_CMD_UFO_ON           = 0x5,
    SUCCESSFUL_CMD_UFO_OFF          = 0x6,
    SUCCESSFUL_CMD_WEAPNEXT         = 0x7,
    SUCCESSFUL_CMD_WEAPPREV         = 0x8,
    SUCCESSFUL_CMD_GIVE_ALL         = 0x9,
    SUCCESSFUL_CMD_CAMLOCK_ON       = 0xA,
    SUCCESSFUL_CMD_CAMLOCK_OFF      = 0xB,
    SUCCESSFUL_CMD_LOOKSPRING_ON    = 0xC,
    SUCCESSFUL_CMD_LOOKSPRING_OFF   = 0xD,
};

struct viewDirectionalHitIndicator_t // sizeof=0x10
{                                       // XREF: cg_s/r
    int time;
    float entOrigin[3];
};

struct viewDamage_t // sizeof=0xC
{                                       // XREF: cg_s/r
    int time;
    int duration;
    float yaw;
};

struct hudElemSoundInfo_t // sizeof=0x4
{                                       // XREF: cg_s/r
    int lastPlayedTime;
};

struct __declspec(align(128)) cg_s // sizeof=0x71C80
{                                                                             // XREF: cg_t/r
        int clientNum;
        int localClientNum;
        DemoType demoType;
        CubemapShot cubemapShot;
        int cubemapSize;
        int cameraMode;
        int hiResShotMode;
        int renderScreen;
        int latestSnapshotNum;
        int latestSnapshotTime;
        snapshot_s *snap;
        snapshot_s *nextSnap;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        snapshot_s activeSnapshots[2];
        float frameInterpolation;
        int frametime;
        int time;
        int oldTime;
        int physicsTime;
        int mapRestart;
        thirdPersonType renderingThirdPerson;
        playerState_s predictedPlayerState;
        int lastPlayerStateOverride;
        centity_s predictedPlayerEntity;
        playerEntity_t playerEntity;
        int predictedErrorTime;
        float predictedError[3];
        float landChange;
        int landTime;
        float heightToCeiling;
        int heightToCeilingTS;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        refdef_s refdef;
        float refdefViewAngles[3];
        Camera cameraData;
        float lastPelvisCameraOffset;
        float baseGunAngles[3];
        float swayViewAngles[3];
        float swayAngles[3];
        float swayOffset[3];
        float recoilAngles[3];
        float recoilSpeed[3];
        float movieCameraOrigin[3];
        float movieCameraAngles[3];
        float movieCameraVelocity[3];
        int movieCameraLastInsideHeliPatch;
        float radiantCameraOrigin[3];
        float radiantCameraAngles[3];
        int radiantCamInUse;
        int followCameraClient;
        int iEntityLastType[1536];
        XModel *pEntityLastXModel[1536];
        float zoomSensitivity;
        bool isLoading;
        char objectiveText[1024];
        // padding byte
        // padding byte
        // padding byte
        int vehicleInitView;
        float prevVehicleInvAxis[3][3];
        bool vehicleViewLocked;
        // padding byte
        // padding byte
        // padding byte
        float vehicleViewLockedAngles[3];
        float storedRemoteCameraAngles[3];
        float storedRemoteCameraOrigin[3];
        char scriptMainMenu[256];
        int numScores;
        int teamScores[4];
        int teamPings[4];
        int teamPlayers[4];
        int scoreOrder[32];
        int scoreLimit;
        scoreboardColumnType_t scoreboardColumnTypes[4];
        int matchUIVisibilityFlags;
        unsigned int talkFlags;
        int bombTimer[2];
        int showScores;
        int scoreFadeTime;
        int scoresTop;
        int scoresOffBottom;
        int scoresBottom;
        int drawHud;
        int timeScaleTimeStart;
        int timeScaleTimeEnd;
        float timeScaleStart;
        float timeScaleEnd;
        int crosshairClientNum;
        int crosshairClientLastTime;
        int crosshairClientStartTime;
        int identifyClientNum;
        int cursorHintIcon;
        int cursorHintTime;
        int cursorHintFade;
        int cursorHintString;
        int vehicleHintTime;
        int lastVehicleSeat;
        int driverHintShown;
        int gunnerHintShown;
        int lastClipFlashTime;
        InvalidCmdHintType invalidCmdHintType;
        int invalidCmdHintTime;
        SuccessfulCmdHintType successfulCmdHintType;
        int successfulCmdHintTime;
        int lastHealthPulseTime;
        int lastHealthLerpDelay;
        int lastHealthClient;
        float lastHealth;
        float healthOverlayFromAlpha;
        float healthOverlayToAlpha;
        int healthOverlayPulseTime;
        int healthOverlayPulseDuration;
        int healthOverlayPulsePhase;
        bool healthOverlayHurt;
        // padding byte
        // padding byte
        // padding byte
        int healthOverlayLastHitTime;
        float healthOverlayOldHealth;
        int healthOverlayPulseIndex;
        int proneBlockedEndTime;
        int lastStance;
        int lastStanceChangeTime;
        int lastStanceFlashTime;
        int voiceTime;
        unsigned int weaponSelect;
        int weaponSelectTime;
        bool weaponForcedSelect;
        // padding byte
        // padding byte
        // padding byte
        unsigned int weaponLatestPrimaryIdx;
        unsigned __int16 lastWeaponForAlt[15];
        // padding byte
        // padding byte
        int prevViewmodelWeapon;
        int equippedOffHand;
        viewDirectionalHitIndicator_t directionalHitIndicator[4];
        viewDamage_t viewDamage[8];
        int damageTime;
        float damageX;
        float damageY;
        float damageValue;
        float viewFade;
        int waterDropCount;
        int waterDropStartTime;
        int waterDropStopTime;
        int weapIdleTime;
        int nomarks;
        int clearMarks;
        int newPlayerViewmodel;
        int v_dmg_time;
        float v_dmg_pitch;
        float v_dmg_roll;
        float fBobCycle;
        float xyspeed;
        float kickAVel[3];
        float kickAngles[3];
        float offsetAngles[3];
        float gunPitch;
        float gunYaw;
        float gunXOfs;
        float gunYOfs;
        float gunZOfs;
        float flamethrowerKickOffset[3];
        float minigunSpinAngle;
        float viewModelAxis[4][3];
        float rumbleScale;
        float compassNorthYaw;
        float compassNorth[2];
        Material *compassMapMaterial;
        float compassMapUpperLeft[2];
        float compassMapWorldSize[2];
        int compassLastTime;
        float compassYaw;
        float compassSpeed;
        int compassFadeTime;
        int healthFadeTime;
        int ammoFadeTime;
        int stanceFadeTime;
        int sprintFadeTime;
        int offhandFadeTime;
        int offhandFlashTime;
        int waypointFadeTime;
        int vehicleControlsFadeTime;
        shellshock_t shellshock;
        //cg_s::<unnamed_type_testShock> testShock;
        struct //cg_s::<unnamed_type_testShock> // sizeof=0x8
        {                                       // XREF: cg_s/r
            int time;
            int duration;
        } testShock;
        int shellshockSoundActive;
        int holdBreathTime;
        int holdBreathInTime;
        int holdBreathDelay;
        float holdBreathFrac;
        float radarProgress;
        float satelliteProgress;
        int radarType;
        float selectedLocation[2];
        float selectedYaw[2];
        int typewriterStartTime;
        float selectedLocations[3][2];
        int lastTimeSelected;
        SprintState sprintStates;
        int adsViewErrorDone;
        int inKillCam;
        int wasInKillCam;
        bool killCamFirstFrameRan;
        // padding byte
        // padding byte
        // padding byte
        int killCamEntity;
        int invalidKillCamEntity;
        KillCamEntityType killCamEntityType;
        int killCamLastEntityNum;
        float killCamLastEntityOrg[3];
        float killCamLastEntityAngles[3];
        KillCamEntityRestState killCamEntityRestState;
        int killCamLookAtEntity;
        float killCamLookAt[3];
        float killCamHelicopterOffset[3];
        __int16 killCamLastVehicleInfoIndex;
        // padding byte
        // padding byte
        int killCamStoppedTime;
        float killCamStoppedDecelTime;
        float killCamStoppedPos[3];
        float killCamStoppedVel[3];
        float killCamPrevBombOrigin[3];
        int killCamLerpEndTime;
        float killCamOldViewAngles[3];
        float killCamOldViewOrg[3];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        bgs_t bgs;
        cpose_t viewModelPose;
        visionSetVars_t visionSetPreLoaded[8];
        char visionSetPreLoadedName[8][64];
        visionSetVars_t visionSetFrom[6];
        visionSetVars_t visionSetTo[6];
        visionSetVars_t visionSetCurrent[6];
        visionSetLerpData_t visionSetLerpData[6];
        char visionNameNaked[64];
        char visionNameNight[64];
        char visionNameFlare[64];
        char visionNameInfrared[64];
        char visionNameTvguided[64];
        char visionNameExtraCam[64];
        bitarray<51> extraButton_bits;
        int lastActionSlotTime;
        bool playerTeleported;
        // padding byte
        // padding byte
        // padding byte
        float flareRatioLastFrame;
        int stepViewStart;
        float stepViewChange;
        //cg_s::<unnamed_type_lastFrame> lastFrame;
        struct //cg_s::<unnamed_type_lastFrame> // sizeof=0x4
        {                                       // XREF: cg_s/r
            float aimSpreadScale;
        } lastFrame;
        int oldCommandTime;
        float oldOrigin[3];
        float oldViewangles[3];
        transPlayerState_t oldTransPlayerState;
        hudElemSoundInfo_t hudElemSound[32];
        int nextRippleTime;
        unsigned int impactEffectsNext;
        unsigned int impactEffects[256];
        float warDesiredMomentumProgress;
        float warCurrentMomentumProgress;
        int warMomentumMultiplier;
        int warMomentumBlitzkriegTime;
        int warMomentumFlashTime;
        float gunnerPitchOffset;
        unsigned int centInNextSnapshot[32];
        centity_s *slideFakeEnt;
        int cameraLinkedEntitiesCount;
        int cameraLinkedEntities[4];
        link_type_e cameraLinkedEntitiesType[4];
        int groundTiltEntNum;
        int poisoned;
        Material *genericMaterialMap[64];
        bool genericFilterPassEnabled[3][16];
        Material *genericFilterPassMaterial[3][16];
        int genericFilterPassTarget[3][16];
        int genericFilterPassSampler0[3][16];
        int genericFilterPassSampler1[3][16];
        float genericFilterPassParam[3][16][16];
        int genericFilterPassQuads[3][16];
        float genericFilterPassFlareOcclusion[3][16];
        bool genericOverlayEnabled[3];
        // padding byte
        Material *genericOverlayMaterial[3];
        float genericOverlayParam[3][16];
        int genericOverlaySortIndex[3];
        float genericOverlayParamADS[8];
        bool genericOverlayEnableADS;
        // padding byte
        // padding byte
        // padding byte
        int swimming;
        int swimmingStartTime;
        int missileViewWobbleLastTime;
        float missileViewWobbleFreq[3];
        float missileViewWobbleAmp[3];
        int counterSpinTime;
        float counterSpinTarget;
        float counterSpinAngle;
        unsigned __int8 weaponTracerCount[1024];
        int extraCamEntity;
        int extraCamActive;
        int extraCamStatic;
        float extraCamOrigin[3];
        float extraCamAngles[3];
        int hasLocalRadar;
        float localRadarPos[2];
        float localRadarProgress;
        int globalScramblerActive;
        float scramblerEnemyAlpha;
        float scramblerFriendlyAlpha;
        int popUpSwayStartTime;
        float popupRotationAngle;
        float popUpAngleDelta;
        CompassScrambler compassScrambler[16];
        centity_s *closestEnemyScrambler;
        int alliesFlagAway;
        int axisFlagAway;
        int resetWeaponStateFlag;
#ifdef KISAK_SP
        // Retail SP cg_s +0xCE534. Client-script Use/ClearAlternateAimParams
        // owns this flag; several SP-only aim/view paths consume it. Placing
        // the semantic field in the existing aligned tail preserves cg_s's
        // normal memset lifetime without disturbing the MP member layout.
        int useAlternateAimParams;
#endif
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};
//static_assert(sizeof(cg_s) == 0x71C80);


struct cgMedia_t // sizeof=0x31BC
{                                                                             // XREF: .data:cgMedia_t cgMedia/r
        Material *whiteMaterial;                        // XREF: AimAssist_DrawCenterBox+E6/r
                                                                                // AimAssist_DrawTargets+297/r ...
        Material *teamStatusBar;                        // XREF: CG_DrawChatMessages+399/r
                                                                                // CG_RegisterGraphics+6A5/w
        Material *afkLightbulb;                         // XREF: CG_RegisterGraphics+6CA/w
                                                                                // CG_AddPlayerSpriteDrawSurfs+310/r
        Material *connectionMaterial;             // XREF: CG_RegisterGraphics+42/w
                                                                                // CG_AddPlayerSpriteDrawSurfs+2C0/r
        Material *youInKillCamMaterial;         // XREF: CG_RegisterGraphics+56/w
                                                                                // CG_AddPlayerSpriteDrawSurfs+280/r
        Material *redTracerMaterial;                // XREF: CG_RegisterGraphics+B5/w
                                                                                // CG_DrawTracer(float const * const,float const * const,refdef_s const *,localEntity_s *)+7C/r
        Material *greenTracerMaterial;            // XREF: CG_RegisterGraphics+C9/w
                                                                                // CG_DrawTracer(float const * const,float const * const,refdef_s const *,localEntity_s *)+9A/r
        Material *bulletMaterial;                     // XREF: CG_RegisterGraphics+DD/w
                                                                                // CG_DrawTracer(float const * const,float const * const,refdef_s const *,localEntity_s *)+C4/r
        Material *laserMaterial;                        // XREF: CG_Laser_Add_Core+454/r
                                                                                // CG_RegisterGraphics+F1/w
        Material *laserLightMaterial;             // XREF: CG_Laser_Add_Core+6B4/r
                                                                                // CG_RegisterGraphics+105/w
        Material *lagometerMaterial;                // XREF: CG_DrawLagometer(int)+66/r
                                                                                // CG_RegisterGraphics+2E/w
        Material *ropeMaterial;                         // XREF: CG_RegisterGraphics+119/w
                                                                                // Rope_Init:loc_85DBDD/r
        Material *hintMaterials[2056];            // XREF: CG_RegisterGraphics+12D/w
                                                                                // CG_RegisterGraphics+141/w ...
        Material *stanceMaterials[5];             // XREF: CG_RegisterGraphics+155/w
                                                                                // CG_RegisterGraphics+169/w ...
        Material *objectiveMaterials[1];        // XREF: CG_RegisterGraphics+1B9/w
                                                                                // CG_ObjectiveIcon(int,int,int)+75/r
        Material *friendMaterials[3];             // XREF: CG_CompassDrawFriendlies(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+8AA/r
                                                                                // CG_CompassDrawFriendlies(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+906/r ...
        Material *damageMaterial;                     // XREF: CG_DrawDamageDirectionIndicators(int)+361/r
                                                                                // CG_RegisterGraphics+209/w
        Material *mantleHint;                             // XREF: CG_RegisterGraphics+21D/w
                                                                                // CG_DrawMantleHint+13/r ...
        Material *graphline;
        Font_s *smallDevFont;                             // XREF: AimAssist_DrawTargets+316/r
                                                                                // AimAssist_DrawTargets+387/r ...
        Font_s *bigDevFont;                                 // XREF: CG_DrawBigDevStringColor(ScreenPlacement const *,float,float,char const *,float const * const,int)+3/r
                                                                                // CG_Init(int,int,int,int)+3B7/w
        unsigned int grenadeExplodeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+13/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+277/r
        unsigned int rifleGrenadeSound[31]; // XREF: CG_RegisterSounds(void)+25/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+292/r
        unsigned int rocketExplodeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+37/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+2AD/r ...
        unsigned int rocketExplodeXtremeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+49/o
        unsigned int mortarShellExplodeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+6D/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+32B/r
        unsigned int tankShellExplodeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+5B/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+36B/r
        unsigned int weaponImpactsTankArmorSound[16];
                                                                                // XREF: CG_RegisterSounds(void)+169/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+402/r ...
        unsigned int weaponImpactsTankTreadSound[16];
                                                                                // XREF: CG_RegisterSounds(void)+17B/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+428/r ...
        unsigned int bulletHitSmallSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+7F/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+10B/r ...
        unsigned int bulletHitLargeSound[31];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+33D1/r
                                                                                // CG_EntityEvent(int,centity_s *,int)+3636/r ...
        unsigned int bulletHitAPSound[31];    // XREF: CG_RegisterSounds(void)+A3/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+18D/r ...
        unsigned int bulletHitXTremeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+B5/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+1CE/r ...
        unsigned int shotgunHitSound[31];     // XREF: CG_RegisterSounds(void)+C7/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+20F/r ...
        unsigned int boltHitSound[31];            // XREF: CG_RegisterSounds(void)+D9/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+3B9/r
        unsigned int bladeHitSound[31];         // XREF: CG_RegisterSounds(void)+EB/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+3DA/r
        unsigned int bulletExitSmallSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+FD/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+FC/r
        unsigned int bulletExitLargeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+10F/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+13D/r
        unsigned int bulletExitAPSound[31]; // XREF: CG_RegisterSounds(void)+121/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+17E/r
        unsigned int bulletExitXTremeSound[31];
                                                                                // XREF: CG_RegisterSounds(void)+133/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+1BF/r
        unsigned int shotgunExitSound[31];    // XREF: CG_RegisterSounds(void)+145/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+200/r
        unsigned int boltExitSound[31];         // XREF: CG_RegisterSounds(void)+157/o
                                                                                // CG_ImpactEffectForWeapon(int,int,int,FxEffectDef const * *,uint *)+3AA/r
        unsigned int mantleSound;                     // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B21DB/r
                                                                                // CG_RegisterSounds(void)+19A/w
        unsigned int mantleSoundPlayer;         // XREF: CG_EntityEvent(int,centity_s *,int)+50E/r
                                                                                // CG_RegisterSounds(void)+1AC/w
        unsigned int dtpLaunchSound;                // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B6493/r
                                                                                // CG_RegisterSounds(void)+1D0/w
        unsigned int dtpLaunchSoundPlayer;    // XREF: CG_EntityEvent(int,centity_s *,int)+47C6/r
                                                                                // CG_RegisterSounds(void)+1BE/w
        unsigned int dtpLandSound[9];             // XREF: CG_EntityEvent(int,centity_s *,int)+4826/r
                                                                                // CG_RegisterSounds(void)+296/w
        unsigned int dtpLandSoundPlayer[9]; // XREF: CG_EntityEvent(int,centity_s *,int)+4814/r
                                                                                // CG_RegisterSounds(void)+258/w
        char dtpSlideLoopSound[9][64];            // XREF: CG_EntityEvent(int,centity_s *,int)+487A/o
                                                                                // CG_EntityEvent(int,centity_s *,int)+4A19/o ...
        char dtpSlideLoopSoundPlayer[9][64];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+4866/o
                                                                                // CG_EntityEvent(int,centity_s *,int)+4A06/o ...
        unsigned int dtpSlideStopSound[9];    // XREF: CG_RegisterSounds(void)+368/w
        unsigned int dtpSlideStopSoundPlayer[9];
                                                                                // XREF: CG_RegisterSounds(void)+32A/w
        unsigned int dtpCollideSound;             // XREF: CG_RegisterSounds(void)+1F4/w
        unsigned int dtpCollideSoundPlayer; // XREF: CG_RegisterSounds(void)+1E2/w
        unsigned int playerSlidingStart_1p[9];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+4A7A/r
                                                                                // CG_RegisterSounds(void)+3A6/w
        unsigned int playerSlidingStart_3p[9];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+4A8C/r
                                                                                // CG_RegisterSounds(void)+3E4/w
        unsigned int playerSlidingStop_1p[9];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+4B3E/r
                                                                                // CG_RegisterSounds(void)+422/w
        unsigned int playerSlidingStop_3p[9];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+4B50/r
                                                                                // CG_RegisterSounds(void)+460/w
        unsigned int bulletWhizby;                    // XREF: CG_RegisterSounds(void)+49D/w
                                                                                // CG_SoundWhizby(int,WeaponDef const *,float const * const,float const * const,float const * const):loc_509745/r
        unsigned int bulletCrack;                     // XREF: CG_RegisterSounds(void)+4AF/w
        unsigned int underwaterWhizby;            // XREF: CG_RegisterSounds(void)+48B/w
                                                                                // CG_SoundWhizby(int,WeaponDef const *,float const * const,float const * const,float const * const)+18C/r
        unsigned int deathGurgle;                     // XREF: CG_RegisterSounds(void)+4C1/w
        unsigned int meleeHit;                            // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B394A/r
                                                                                // CG_RegisterSounds(void)+4D3/w ...
        unsigned int meleeHitOther;                 // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B3B8A/r
                                                                                // CG_RegisterSounds(void)+4E5/w
        unsigned int meleeKnifeHit;                 // XREF: CG_EntityEvent(int,centity_s *,int)+1C5E/r
                                                                                // CG_RegisterSounds(void)+4F7/w
        unsigned int meleeKnifeHitOther;        // XREF: CG_EntityEvent(int,centity_s *,int)+1E9D/r
                                                                                // CG_RegisterSounds(void)+509/w ...
        unsigned int meleeDogHit;                     // XREF: CG_EntityEvent(int,centity_s *,int)+1BB6/r
                                                                                // CG_RegisterSounds(void)+51B/w
        unsigned int meleeDogHitOther;            // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B3A31/r
                                                                                // CG_RegisterSounds(void)+52D/w
        unsigned int nightVisionOn;                 // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B3C9C/r
                                                                                // CG_RegisterSounds(void)+53F/w
        unsigned int nightVisionOff;                // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B3D46/r
                                                                                // CG_RegisterSounds(void)+551/w
        unsigned int playerSprintGasp;            // XREF: CG_RegisterSounds(void)+479/w
        unsigned int playerHeartBeatSound;    // XREF: CG_RegisterSounds(void)+563/w
                                                                                // HoldBreathUpdate+13D/r
        unsigned int playerBreathInSound;     // XREF: CG_RegisterSounds(void)+575/w
                                                                                // HoldBreathUpdate+C0/r
        unsigned int playerBreathOutSound;    // XREF: CG_RegisterSounds(void)+587/w
                                                                                // HoldBreathUpdate+215/r
        unsigned int playerBreathGaspSound; // XREF: CG_RegisterSounds(void)+599/w
                                                                                // HoldBreathUpdate+1DC/r
        unsigned int playerSwapOffhand;         // XREF: CG_RegisterSounds(void)+5AB/w
        Material *compassping_player;             // XREF: CG_CompassDrawVehicles(int,CompassType,int,rectDef_s const *,rectDef_s const *,float const * const)+59E/r
                                                                                // CG_CompassDrawVehicles(int,CompassType,int,rectDef_s const *,rectDef_s const *,float const * const)+5D6/r ...
        Material *compassping_friendlyfiring;
                                                                                // XREF: CG_CompassDrawFriendlies(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_48E28E/r
                                                                                // CG_RegisterGraphics+245/w
        Material *compassping_friendlyyelling;
                                                                                // XREF: CG_CompassDrawFriendlies(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_48E342/r
                                                                                // CG_RegisterGraphics+259/w
        Material *compassping_friendlyfakefire;
                                                                                // XREF: CG_CompassDrawFakeFire(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_490288/r
                                                                                // CG_RegisterGraphics+26D/w
        Material *compassping_enemy;                // XREF: GetCompassPingMaterialForClient:loc_48FCAE/r
                                                                                // CG_RegisterGraphics+281/w
        Material *compassping_enemydirectional;
                                                                                // XREF: CG_CompassDrawEnemies(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_48F9F3/r
                                                                                // CG_CompassDrawFakeFire(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_490227/r ...
        Material *compassping_enemyfiring;    // XREF: GetCompassFiringMaterialForClient:loc_48FCFE/r
                                                                                // CG_CompassDrawFakeFire(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+56B/r ...
        Material *compassping_enemyyelling; // XREF: CG_RegisterGraphics+2D1/w
        Material *compassping_enemysatellite;
                                                                                // XREF: CG_CompassDrawEnemies(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+C63/r
                                                                                // CG_RegisterGraphics+2A9/w
        Material *compassping_grenade;            // XREF: CG_RegisterGraphics+2E5/w
        Material *compassping_explosion;        // XREF: CG_RegisterGraphics+2F9/w
        Material *compassping_firstplace;     // XREF: GetCompassPingMaterialForClient+37/r
                                                                                // GetCompassFiringMaterialForClient+37/r ...
        Material *watch_face;                             // XREF: CG_RegisterGraphics+349/w
                                                                                // CG_DrawWristWatch+32/r
        Material *watch_hour;                             // XREF: CG_RegisterGraphics+35D/w
                                                                                // CG_DrawWristWatch+C7/r
        Material *watch_minute;                         // XREF: CG_RegisterGraphics+371/w
                                                                                // CG_DrawWristWatch+13F/r
        Material *watch_second;                         // XREF: CG_RegisterGraphics+385/w
                                                                                // CG_DrawWristWatch+1B7/r
        Material *acoustic_ping;                        // XREF: CG_RegisterGraphics+399/w
        Material *acoustic_wedge;                     // XREF: CG_RegisterGraphics+3AD/w
        Material *acoustic_grid;                        // XREF: CG_RegisterGraphics+3C1/w
        Material *compass_scrambler_large;    // XREF: CG_CompassDrawScramblerFriendly(int,CompassType,rectDef_s const *,rectDef_s const *,float const * const)+174/r
                                                                                // CG_RegisterGraphics+3D5/w
        Material *compass_mortar_selector;    // XREF: CG_CompassDrawPlayerSelectedLocations(int,CompassType,rectDef_s const *,rectDef_s const *,Material *,float * const)+587/r
                                                                                // CG_RegisterGraphics+3E9/w
        Material *compass_acoustic_ping;        // XREF: CG_CompassDrawRadarEffects(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_4909A6/r
                                                                                // CG_RegisterGraphics+335/w
        Material *compass_radarline;                // XREF: CG_CompassDrawRadarEffects(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_490544/r
                                                                                // CG_CompassDrawRadarEffects(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+419/r ...
        Material *compass_artillery_friendly;
                                                                                // XREF: CG_CompassDrawArtilleryIcon(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_48E946/r
                                                                                // CG_RegisterGraphics+411/w
        Material *compass_artillery_enemy;    // XREF: CG_CompassDrawArtilleryIcon(int,CompassType,rectDef_s const *,rectDef_s const *,float * const):loc_48E916/r
                                                                                // CG_RegisterGraphics+3FD/w
        Material *compass_mortar_friendly;    // XREF: CG_CompassDrawArtilleryIcon(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+FC/r
                                                                                // CG_RegisterGraphics+439/w
        Material *compass_mortar_enemy;         // XREF: CG_CompassDrawArtilleryIcon(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+C4/r
                                                                                // CG_RegisterGraphics+425/w
        Material *compass_dogs_enemy;             // XREF: CG_RegisterGraphics+44D/w
                                                                                // CG_OwnerDraw(int,rectDef_s,float,float,float,float,int,int,int,int,float * const,Material *)+44F/r ...
        Material *compass_incoming_artillery;
                                                                                // XREF: CG_CompassDrawIncomingArtilleryIcon(int,CompassType,rectDef_s const *,rectDef_s const *,float * const)+1AD/r
                                                                                // CG_RegisterGraphics+461/w
        Material *compass_sentry_friendly;    // XREF: CG_GetTurretCompassMaterial:loc_49212A/r
                                                                                // CG_RegisterGraphics+475/w
        Material *compass_sentry_friendly_firing;
                                                                                // XREF: CG_GetTurretCompassMaterial+21/r
                                                                                // CG_RegisterGraphics+489/w
        Material *compass_sentry_enemy;         // XREF: CG_GetTurretCompassMaterial:loc_492144/r
                                                                                // CG_RegisterGraphics+49D/w
        Material *compass_sentry_enemy_firing;
                                                                                // XREF: CG_GetTurretCompassMaterial+3B/r
                                                                                // CG_RegisterGraphics+4B1/w
        Material *compass_tow_turret_friendly;
                                                                                // XREF: CG_GetTurretCompassMaterial:loc_492174/r
                                                                                // CG_RegisterGraphics+4C5/w
        Material *compass_tow_turret_friendly_firing;
                                                                                // XREF: CG_GetTurretCompassMaterial+6B/r
                                                                                // CG_RegisterGraphics+4D9/w
        Material *compass_tow_turret_enemy; // XREF: CG_GetTurretCompassMaterial:loc_49218E/r
                                                                                // CG_RegisterGraphics+4ED/w
        Material *compass_tow_turret_enemy_firing;
                                                                                // XREF: CG_GetTurretCompassMaterial+85/r
                                                                                // CG_RegisterGraphics+501/w
        Material *compass_guided_missile;     // XREF: CG_CompassDrawGuidedMissile(int,CompassType,int,rectDef_s const *,rectDef_s const *,float const * const)+3B7/r
                                                                                // CG_RegisterGraphics+515/w
        Material *grenadeIconFrag;                    // XREF: CG_AddHudGrenade(cg_s const *,centity_s const *):loc_498BC7/r
                                                                                // CG_RegisterGraphics+529/w
        Material *grenadeIconFlash;                 // XREF: CG_AddHudGrenade(cg_s const *,centity_s const *):loc_498C1F/r
                                                                                // CG_RegisterGraphics+53D/w
        Material *grenadeIconThrowBack;         // XREF: CG_AddHudGrenade(cg_s const *,centity_s const *)+342/r
                                                                                // CG_RegisterGraphics+551/w
        Material *grenadePointer;                     // XREF: CG_DrawWarningPointer+20F/r
                                                                                // CG_RegisterGraphics+565/w
        Material *offscreenObjectivePointer;
                                                                                // XREF: DrawOffscreenViewableWaypoint+7BE/r
                                                                                // CG_RegisterGraphics+579/w
        Material *demoTimelineFaded;                // XREF: CG_RegisterGraphics+58D/w
                                                                                // CG_DrawDemoControls+4A2/r
        Material *demoTimelineSolid;                // XREF: CG_RegisterGraphics+5A1/w
                                                                                // CG_DrawDemoControls+7DF/r
        Material *demoTimelineCursor;             // XREF: CG_RegisterGraphics+5B5/w
                                                                                // CG_DrawDemoControls+1227/r
        Material *demoTimelineBookmark;         // XREF: CG_RegisterGraphics+5C9/w
                                                                                // CG_DrawDemoControls+CD4/r ...
        Material *demoStatePaused;                    // XREF: CG_RegisterGraphics+5DD/w
                                                                                // CG_DrawDemoControls:loc_4CFC0A/r
        Material *demoStatePlay;                        // XREF: CG_RegisterGraphics+5F1/w
                                                                                // CG_DrawDemoControls+1DB0/r
        Material *demoStateStop;                        // XREF: CG_RegisterGraphics+605/w
                                                                                // CG_DrawDemoControls+1D37/r
        Material *demoStateJump;                        // XREF: CG_RegisterGraphics+619/w
                                                                                // CG_DrawDemoControls+19D6/r ...
        Material *demoStateForwardFast;         // XREF: CG_RegisterGraphics+62D/w
                                                                                // CG_DrawDemoControls+2012/r ...
        Material *demoStateForwardSlow;         // XREF: CG_RegisterGraphics+641/w

        Material *theaterUpArrow;           // XREF: CG_RegisterGraphics+655/w
        Material *theaterDownArrow;
        Material *theaterLeftArrow;
        Material *theaterRightArrow;
                                                                                // CG_DrawDemoControls+1AB9/r ...
        struct FxImpactTable *fx;                                    // XREF: CG_RegisterGraphics+655/w
                                                                                // CG_DrawDemoControls+25CE/r
        const FxEffectDef *fxNoBloodFleshHit;
                                                                                // XREF: CG_RegisterGraphics+669/w
                                                                                // CG_DrawDemoControls+24B7/r
        const FxEffectDef *fxKnifeBlood;        // XREF: CG_RegisterGraphics+67D/w
                                                                                // CG_DrawDemoControls+23A8/r
        const FxEffectDef *fxKnifeNoBlood;    // XREF: CG_RegisterGraphics+691/w
                                                                                // CG_DrawDemoControls+26DE/r
        const FxEffectDef *fxDogBlood;            // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B4266/r
                                                                                // CG_EntityEvent(int,centity_s *,int)+2600/r ...
        const FxEffectDef *fxDogNoBlood;        // XREF: CG_RegisterGraphics+ABC/w
                                                                                // CG_BulletHitEvent_Internal+2A5/r
        const FxEffectDef *fxNonFatalHero;    // XREF: CG_RegisterGraphics+ACE/w
                                                                                // CG_MeleeBloodEvent(int,centity_s const *,int)+131/r ...
        const FxEffectDef *fxBodyArmorSmall;
                                                                                // XREF: CG_RegisterGraphics+AE0/w
                                                                                // CG_MeleeBloodEvent(int,centity_s const *,int)+152/r ...
        const FxEffectDef *fxBodyArmorLarge;
                                                                                // XREF: CG_RegisterGraphics+AF2/w
                                                                                // CG_DogBloodEvent(int,int,float * const,int)+40/r
        const FxEffectDef *fxDtpArmSlide1;    // XREF: CG_RegisterGraphics+B04/w
                                                                                // CG_DogBloodEvent(int,int,float * const,int)+76/r
        const FxEffectDef *fxDtpArmSlide2;    // XREF: CG_RegisterGraphics+B16/w
                                                                                // CG_BulletHitEvent_Internal+277/r
        const FxEffectDef *fxPlayerSliding; // XREF: CG_RegisterGraphics+B28/w
                                                                                // CG_BulletHitEvent_Internal+495/r
        const FxEffectDef *fxPuff;                    // XREF: CG_RegisterGraphics+B3A/w
                                                                                // CG_BulletHitEvent_Internal:loc_537A90/r
        const FxEffectDef *heliDustEffect;    // XREF: CG_EntityEvent(int,centity_s *,int)+48C8/r
                                                                                // CG_RegisterGraphics+B4C/w
        const FxEffectDef *heliWaterEffect; // XREF: CG_EntityEvent(int,centity_s *,int)+48F0/r
                                                                                // CG_RegisterGraphics+B5E/w
        const FxEffectDef *helicopterLightSmoke;
                                                                                // XREF: CG_RegisterGraphics+B70/w
                                                                                // CG_Player(int,centity_s *)+3F6/r
        const FxEffectDef *helicopterHeavySmoke;
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+4913/r
                                                                                // CG_RegisterGraphics+B82/w
        const FxEffectDef *helicopterOnFire;
                                                                                // XREF: CG_RegisterGraphics+B94/w
        const FxEffectDef *jetAfterburner;    // XREF: CG_RegisterGraphics+BA6/w
        Material *nightVisionOverlay;             // XREF: CG_RegisterGraphics+BB8/w
        Material *hudIconNVG;                             // XREF: CG_RegisterGraphics+BCA/w
        Material *hudDpadArrow;                         // XREF: CG_RegisterGraphics+BDC/w
        Material *hudDpadCircle;                        // XREF: CG_RegisterGraphics+BEE/w
        Material *hudDpadLeftHighlight;         // XREF: CG_DrawNightVisionOverlay(int):loc_4A1586/r
                                                                                // CG_DrawNightVisionOverlay(int):loc_4A15A0/r ...
        Material *ammoCounterBullet;                // XREF: CG_Init(int,int,int,int)+3EE/w
        Material *ammoCounterBeltBullet;        // XREF: CG_Init(int,int,int,int)+402/w
        Material *ammoCounterRifleBullet;     // XREF: CG_DrawPlayerActionSlot(int,rectDef_s const *,uint,float * const,Font_s *,float,int)+41C/r
                                                                                // CG_DrawPlayerActionSlot(int,rectDef_s const *,uint,float * const,Font_s *,float,int)+971/r ...
        Material *ammoCounterRocket;                // XREF: CG_DrawPlayerActionSlot(int,rectDef_s const *,uint,float * const,Font_s *,float,int)+311/r
                                                                                // CG_DrawPlayerActionSlot(int,rectDef_s const *,uint,float * const,Font_s *,float,int)+6D2/r ...
        Material *ammoCounterShotgunShell;    // XREF: DrawClipAmmoMagazine+144/r
                                                                                // DrawClipAmmoBeltfed+16F/r ...
        Material *ammoCounterSingle;                // XREF: DrawClipAmmoBeltfed+388/r
                                                                                // CG_Init(int,int,int,int)+452/w
        Material *lifeCounterAlive;                 // XREF: DrawClipAmmoShortMagazine+144/r
                                                                                // CG_Init(int,int,int,int)+466/w
        Material *lifeCounterDead;                    // XREF: DrawClipAmmoRockets+144/r
                                                                                // CG_Init(int,int,int,int)+47A/w
        Material *textDecodeCharacters;         // XREF: DrawClipAmmoShotgunShells+144/r
                                                                                // CG_Init(int,int,int,int)+48E/w
        Material *textDecodeCharactersGlow; // XREF: CG_DrawAmmoCounterSingle(int,rectDef_s *,float * const,bool):loc_4720C6/r
                                                                                // CG_Init(int,int,int,int)+4A2/w
        const FxEffectDef *physicsWaterEffects[8];
                                                                                // XREF: CG_DrawPlayerCOD7TypeWriter(int,rectDef_s const *,Font_s *,float,float * const,Material *,int,itemDef_s *,char const *)+8E/r
                                                                                // CG_DrawPlayerCOD7TypeWriter(int,rectDef_s const *,Font_s *,float,float * const,Material *,int,itemDef_s *,char const *)+95/r ...
        const FxEffectDef *infraredHeartbeat;
                                                                                // XREF: CG_RegisterGraphics+C48/w
};

extern centity_s *cg_entitiesArray[1];
extern fake_centity_s *cg_fakeEntitiesArray;

extern cg_s *cgArray;
extern cgs_t *cgsArray;

inline centity_s *__cdecl CG_GetEntity(int32_t localClientNum, uint32_t entityIndex)
{
    iassert(localClientNum == 0);
    bcassert(entityIndex, MAX_LOCAL_CENTITIES);

    if (entityIndex < 1024)
        return &cg_entitiesArray[localClientNum][entityIndex];
    else
        return &cg_fakeEntitiesArray[512 * localClientNum - 1024 + entityIndex].cent;
}

inline cg_s *CG_GetLocalClientGlobals(int32_t localClientNum)
{
    iassert(localClientNum == 0);

    return &cgArray[localClientNum];
}

inline cgs_t *CG_GetLocalClientStaticGlobals(int32_t localClientNum)
{
    iassert(localClientNum == 0);

    return &cgsArray[localClientNum];
}

#ifdef KISAK_SP
// Retail SP 0x0063dcd0. Verified against the retail disassembly:
//
//   0063dcd0  mov  eax, [esp+8]                 ; entnum
//   0063dcd4  cmp  eax, 0x400                   ; real vs fake centity
//   0063dcd9  mov  ecx, [esp+4]                 ; localClientNum
//   0063dcf9  imul eax, eax, 0x31c              ; sizeof(centity_s), retail SP
//   0063dcff  add  eax, [ecx*4 + 0x2ff6684]     ; cg_entitiesArray[local]
//   0063dd06  cmp  word [eax+0x222], 1          ; nextState.eType == ET_PLAYER
//   0063dd0e  je   0063dd13
//   0063dd10  xor  eax, eax / ret               ;   ...else NULL
//   0063dd13  movzx eax, byte [eax+0x236]       ; nextState.clientNum  <-- index
//   0063dd21  imul eax, eax, 0x600              ; sizeof(clientInfo_t), retail SP
//
// Retail calls this from BOTH tag-camera sites (0x00623cca in
// CG_ComputeUseTagCamera and the CG_OffsetFirstPersonView tail at 0x00792316)
// and from 0x0040b4aa, so it is ported once here rather than duplicated.
//
// The index is cent->nextState.clientNum (the `movzx ... [eax+0x236]` above),
// NOT cgameGlob->clientNum. The two are identical for the local player in SP,
// but the faithful form is also correct when spectating another client.
inline clientInfo_t *CG_GetClientInfoForPlayerEnt(int localClientNum, int entnum)
{
    centity_s *cent = CG_GetEntity(localClientNum, entnum);

    if ( !cent || cent->nextState.eType != ET_PLAYER )
        return 0;
    return &CG_GetLocalClientGlobals(localClientNum)->bgs.clientinfo[cent->nextState.clientNum];
}
#endif


constexpr float factor[2] = { -1.0, 1.0 };
