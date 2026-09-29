#pragma once

#include <universal/q_shared.h>

enum entityType_t : __int16;

struct LerpEntityStateTurret // sizeof=0x20
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float gunAngles[3];
        int ownerNum;
        float heatVal;
        int overheating;
        float pivotOffset;
        int flags;
};

struct LerpEntityStateLoopFx // sizeof=0x8
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float cullDist;
        int period;
};

struct __declspec(align(4)) LerpEntityStateActor // sizeof=0x14
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        // LerpEntityStateActor::<unnamed_type_index> index;
        union //LerpEntityStateActor_unnamed_type_index // sizeof=0x4
        {                                                                             // XREF: LerpEntityStateActor/r
                int actorNum;
                int corpseNum;
        };
        int species;
        int team;
        int enemy;
        struct
        {
                __int16 fBodyPitch;
        } proneInfo;
        //LerpEntityStateActor::<unnamed_type_proneInfo> proneInfo;
        // padding byte
        // padding byte
};

struct LerpEntityStatePrimaryLight // sizeof=0x14
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        unsigned __int8 colorAndExp[4];
        float intensity;
        float radius;
        float cosHalfFovOuter;
        float cosHalfFovInner;
};

struct __declspec(align(4)) LerpEntityStatePlayer // sizeof=0x1C
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float leanf;
        int movementDir;
        int moveType;
        unsigned __int16 primaryWeapon;
        unsigned __int16 stowedWeapon;
        unsigned __int16 offhandWeapon;
        unsigned __int16 meleeWeapon;
        unsigned __int8 offhandWeaponModel;
        unsigned __int8 meleeWeaponModel;
        unsigned __int8 vehicleType;
        unsigned __int8 vehicleAnimBoneIndex;
        unsigned __int8 vehicleSeat;
        unsigned __int8 stowedWeaponCamo;
        // padding byte
        // padding byte
};

struct LerpEntityStateVehicleGunnerAngles // sizeof=0x4
{                                                                             // XREF: LerpEntityStateVehicle/r
        __int16 pitch;
        __int16 yaw;
};



struct __declspec(align(2)) LerpEntityStateVehicle // sizeof=0x20
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float steerYaw;
        float bodyRoll;
        LerpEntityStateVehicleGunnerAngles gunnerAngles[4];
        //$ABF14481B6CB3C635AAEF83DF0F35947 ___u3;
        union //$ABF14481B6CB3C635AAEF83DF0F35947 // sizeof=0x2
        {                                                                             // XREF: LerpEntityStateVehicle/r
            __int16 throttle;
            __int16 bodyPitch;
        };
        __int16 gunPitch;
        __int16 gunYaw;
        unsigned __int8 drawOnCompass;
        // padding byte
};

struct LerpEntityStateMissile // sizeof=0x10
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        int launchTime;
        int parentClientNum;
        int fuseTime;
        int forcedDud;
};

struct LerpEntityStateScriptMover // sizeof=0x18
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        unsigned __int8 attachTagIndex[4];
        int attachedTagIndex;
        __int16 attachModelIndex[4];
        __int16 animScriptedAnim;
        __int16 attachedEntNum;
        __int16 exploderIndex;
        unsigned __int16 scale;
};

struct LerpEntityStateBulletHit // sizeof=0xC
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float start[3];
};

struct LerpEntityStateEarthquake // sizeof=0xC
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float scale;
        float radius;
        int duration;
};

struct LerpEntityStateCustomExplode // sizeof=0x8
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        int startTime;
        int effectIndex;
};

struct LerpEntityStateExplosion // sizeof=0x14
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float innerRadius;
        float magnitude;
        float innerDamage;
        int dummy;
        float outerDamage;
};

struct LerpEntityStateExplosionJolt // sizeof=0x10
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float innerRadius;
        float impulse[3];
};

struct LerpEntityStatePhysicsJitter // sizeof=0xC
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float innerRadius;
        float minDisplacement;
        float maxDisplacement;
};

struct LerpEntityStateCreateDynEnt // sizeof=0xC
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        int model;
        int fxId;
        int mature;
};

struct LerpEntityStateStreamerHint // sizeof=0x4
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        float factor;
};

struct LerpEntityStateAnonymous // sizeof=0x20
{                                                                             // XREF: LerpEntityStateTypeUnion/r
        int data[8];
};

struct LerpEntityStateDestructibleHit // sizeof=0x18
{                                       // XREF: LerpEntityStateTypeUnion/r
    int modelState0;                    // XREF: CG_DestructibleRewindToTime(int,Destructible *,int)+60/w
    int modelState1;                    // XREF: CG_DestructibleRewindToTime(int,Destructible *,int)+66/w
    int modelState2;                    // XREF: CG_DestructibleRewindToTime(int,Destructible *,int)+6C/w
    int dummy;                          // XREF: CG_DestructibleRewindToTime(int,Destructible *,int)+72/w
    int modelState3;                    // XREF: CG_DestructibleRewindToTime(int,Destructible *,int)+78/w
    int modelState4;                    // XREF: CG_DestructibleRewindToTime(int,Destructible *,int)+7E/w
};

union LerpEntityStateTypeUnion // sizeof=0x20
{                                                                             // XREF: PlayerCmd_finishPlayerDamage+A/o
                                                                                // PlayerCmd_finishPlayerDamage+11/o ...
        LerpEntityStateTurret turret;
        LerpEntityStateLoopFx loopFx;
        LerpEntityStateActor actor;
        LerpEntityStatePrimaryLight primaryLight;
        LerpEntityStatePlayer player;
        LerpEntityStateVehicle vehicle;
        LerpEntityStateMissile missile;
        LerpEntityStateScriptMover scriptMover;
        LerpEntityStateBulletHit bulletHit;
        LerpEntityStateEarthquake earthquake;
        LerpEntityStateCustomExplode customExplode;
        LerpEntityStateExplosion explosion;
        LerpEntityStateExplosionJolt explosionJolt;
        LerpEntityStatePhysicsJitter physicsJitter;
        LerpEntityStateDestructibleHit destructibleHit;
        LerpEntityStateCreateDynEnt createDynEnt;
        LerpEntityStateStreamerHint streamerHint;
        LerpEntityStateAnonymous anonymous;
};

struct LerpEntityState // sizeof=0x74
{                                                                             // XREF: entityState_s/r
                                                                                // centity_s/r
        int eFlags;                                                 // XREF: ClientEndFrame(gentity_s *)+9A0/r
        int eFlags2;
        trajectory_t pos;                                     // XREF: CScr_LaunchRagdoll+11F/w
                                                                                // CScr_LaunchRagdoll+12F/w ...
        trajectory_t apos;                                    // XREF: CM_GetWaterHeight(float const * const,float,float)+107/o
                                                                                // CM_UnlinkEntity(svEntity_s *)+75/o ...
        LerpEntityStateTypeUnion u;                 // XREF: PlayerCmd_finishPlayerDamage+A/o
                                                                                // PlayerCmd_finishPlayerDamage+11/o ...
        int useCount;
};

struct renderOptions_s // sizeof=0x4
{                                                                             // XREF: PlayerHeldWeapon/r
    //$9E8B35E2EA66D9A293E1BFD9EEB1088A ___u0;
    union //$9E8B35E2EA66D9A293E1BFD9EEB1088A // sizeof=0x4
    {                                                                             // XREF: BG_PlayerToEntitySetMisc+176/w
                                                                                    // BG_PlayerWeaponOptions(playerState_s const *,uint):loc_426373/w ...
            unsigned int i;
            //renderOptions_s::<unnamed_tag>::<unnamed_type_s> s;
            struct //renderOptions_s
            {                                                                             // XREF: $9E8B35E2EA66D9A293E1BFD9EEB1088A/r
                    unsigned __int32 camo : 6;
                    unsigned __int32 lens : 4;
                    unsigned __int32 reticle : 6;
                    unsigned __int32 reticleColor : 3;
                    unsigned __int32 emblem : 1;
                    unsigned __int32 tag : 1;
                    unsigned __int32 player : 5;
                    unsigned __int32 facepaintPattern : 6;
            };
    };

    renderOptions_s()
    {

    }
    renderOptions_s(unsigned int init)
    {
        this->i = init;
    }

    void CopyWeaponOptions(const renderOptions_s *o)
    {
        this->i = o->i & 0x3F | this->i & 0xFFFFFFC0;
        this->i = (((o->i >> 6) & 0xF) << 6) | this->i & 0xFFFFFC3F;
        this->i = ((unsigned __int8)HIBYTE(LOWORD(o->i)) >> 2 << 10) | this->i & 0xFFFF03FF;
        this->i = ((HIWORD(o->i) & 7) << 16) | this->i & 0xFFF8FFFF;
        this->i = (((o->i & 0x80000) != 0) << 19) | this->i & 0xFFF7FFFF;
        this->i = (((o->i & 0x100000) != 0) << 20) | this->i & 0xFFEFFFFF;
        this->i = (((o->i >> 21) & 0x1F) << 21) | this->i & 0xFC1FFFFF;
    }
};

struct clientLinkInfo_t // sizeof=0x4
{                                                                             // XREF: entityState_s/r
        __int16 parentEnt;
        unsigned __int8 tagIndex;
        unsigned __int8 flags;
};

struct actorAnimState_t // sizeof=0x10
{                                                                             // XREF: entityState_s::<unnamed_type_un2>/r
        int state;
        float fLeanAmount;
        float fAimUpDown;
        float fAimLeftRight;
};

struct playerAnimState_t // sizeof=0x10
{                                                                             // XREF: entityState_s::<unnamed_type_un2>/r
        int legsAnim;
        int torsoAnim;
        float fTorsoPitch;
        float fWaistPitch;
};

struct vehicleState_t // sizeof=0x8
{                                                                             // XREF: entityState_s::<unnamed_type_un2>/r
        unsigned __int8 vehicleInfoIndex;
        // padding byte
        __int16 attachModelIndex[2];
        unsigned __int8 attachTagIndex[2];
};

struct __declspec(align(4)) entityState_s // sizeof=0xE0
{                                                                             // XREF: .data:entityState_s g_defaultEntityState/r
                                                                                // snapshot_s/r ...
        int number;                                                 // XREF: Demo_GenerateUncompressedSnapshot(int,msg_t *,int,int,int,int,int,bool)+4E1/r
                                                                                // Demo_WritePacketEntities(msg_t *)+1BC/r
        LerpEntityState lerp;                             // XREF: CScr_LaunchRagdoll+11F/w
                                                                                // CScr_LaunchRagdoll+12F/w ...
        int time2;
        unsigned int loopSoundId;
        int solid;
        renderOptions_s renderOptions;            // XREF: ClientDisconnect(int)+251/r
                                                                                // ClientDisconnect(int)+270/o
        //entityState_s::<unnamed_type_un2> un2;
        union //unnamed_type_un2 //entityState_s::<unnamed_type_un2> // sizeof=0x10
        {                                                                             // XREF: entityState_s/r
            actorAnimState_t animState;
            playerAnimState_t anim;
            vehicleState_t vehicleState;
        }; //un2;
        //entityState_s::<unnamed_type_un3> un3;
        union unnamed_type_un3 //entityState_s::<unnamed_type_un3> // sizeof=0x4
        {                                                                             // XREF: entityState_s/r
                int item;
                int hintString;
                int vehicleXModel;
                unsigned int secondBcAlias;
                unsigned int soundTag;
        } un3;
        unsigned int partBits[5];
        unsigned __int8 events[4];
        unsigned __int16 eventParms[4];
        unsigned __int16 eventParm;
        //__int16 eType;                                            // XREF: BounceMissile+34D/r
        entityType_t eType; // LWSS: enforce enum usage
                                                                                // BounceMissile+3C5/r ...
        __int16 groundEntityNum;
        //entityState_s::<unnamed_type_index> index;
        union unnamed_type_index//entityState_s::<unnamed_type_index> // sizeof=0x2
        {                                                                             // XREF: entityState_s/r
                __int16 brushmodel;
                __int16 xmodel;
                __int16 primaryLight;
                unsigned __int16 bone;
        } index;
        __int16 otherEntityNum;
        __int16 attackerEntityNum;
        __int16 enemyModel;                                 // XREF: MissileImpact:loc_631B13/o
                                                                                // JavelinClimbOffset:loc_638B15/o ...
        unsigned __int16 weapon;
        unsigned __int16 lastStandPrevWeapon;
        unsigned __int16 targetname;
        __int16 loopSoundFade;
        __int16 eventSequence;
        unsigned __int8 surfType;
        unsigned __int8 clientNum;
        unsigned __int8 iHeadIcon;
        //entityState_s::<unnamed_type_faction> faction;
        union unnamed_type_faction//entityState_s::<unnamed_type_faction> // sizeof=0x1
        {                                                                             // XREF: entityState_s/r
                unsigned __int8 iHeadIconTeam;
                unsigned __int8 teamAndOwnerIndex;
        } faction;
        unsigned __int8 weaponModel;
        //entityState_s::<unnamed_type_un1> un1;
        union unnamed_type_un1//entityState_s::<unnamed_type_un1> // sizeof=0x1
        {                                                                             // XREF: entityState_s/r
                unsigned __int8 scale;
                unsigned __int8 eventParm2;
                unsigned __int8 helicopterStage;
                unsigned __int8 destructibleid;
        } un1;
        clientLinkInfo_t clientLinkInfo;
#ifdef KISAK_SP
        // Reconstruction storage for retail SP entityState+0xD0. Retail's
        // loopSoundFade is a separate short at +0xCE; our MP layout keeps it
        // at +0xD0. Use trailing padding, never overlay the sound fade.
        unsigned __int8 animTreeIndex;
#else
        // padding byte
#endif
        // padding byte
};

static_assert(sizeof(entityState_s) == 0xE0, "entity snapshot stride changed");
#ifdef KISAK_SP
static_assert(offsetof(entityState_s, loopSoundFade) == 0xD0, "sound fade offset changed");
static_assert(offsetof(entityState_s, clientLinkInfo) == 0xDA, "entity netfield offsets changed");
static_assert(offsetof(entityState_s, animTreeIndex) == 0xDE, "SP animation byte must occupy trailing padding");
#endif
