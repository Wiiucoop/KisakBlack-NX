#pragma once

#include <qcommon/msg_mp.h>
#include <game/enthandle.h>
#include <physics/phys_colgeom.h>
#include <game/teams.h>

#define MAX_VEHICLES 16

enum pmoveVehAnimState_t : __int32
{                                                                             // XREF: pmove_t/r
                                                                                // ?G_VehicleFinishedAnimating@@YAXPAUgentity_s@@W4pmoveVehAnimState_t@@@Z/r
        PMOVE_VEH_ANIM_STATE_PLAYING     = 0x0,
        PMOVE_VEH_ANIM_STATE_END_STAGE = 0x1,
        PMOVE_VEH_ANIM_STATE_COMPLETE    = 0x2,
};

//enum $E02F86DF661B70747453867A72DA739A : __int32
enum gentityFlags_t : uint32_t // LWSS: not a real enum name, used to force usage
{
    FL_GODMODE              = 0x1,
    FL_DEMI_GODMODE         = 0x2,
    FL_NOTARGET             = 0x4,
    FL_NO_KNOCKBACK         = 0x8,
    FL_DROPPED_ITEM         = 0x10,
    FL_NO_BOTS              = 0x20,
    FL_NO_HUMANS            = 0x40,
    FL_TOGGLE               = 0x80,
#ifdef KISAK_SP
    // Retail SP keeps the dynamic-path state as a compact three-bit group.
    // BlackOps.exe uses 0x100 for the dynamic-blocker predicate, 0x200 for
    // automatic path blocking, and 0x400 for the active obstacle state.
    FL_DYNAMICPATH          = 0x100,
    FL_AUTO_BLOCKPATHS      = 0x200,
    FL_OBSTACLE             = 0x400,
#else
    FL_SOFTACTIVATE         = 0x100,
    FL_LOW_PRIORITY_USEABLE = 0x200,
    FL_NO_HEADCHECK         = 0x400,
    FL_DYNAMICPATH          = 0x800,
#endif
    FL_SUPPORTS_LINKTO      = 0x1000,
    FL_NO_AUTO_ANIM_UPDATE  = 0x2000,
    FL_GRENADE_TOUCH_DAMAGE = 0x4000,
    FL_GRENADE_MARTYRDOM    = 0x8000,
    FL_MISSILE_DESTABILIZED = 0x10000,
    FL_STABLE_MISSILES      = 0x20000,
    FL_REPEAT_ANIM_UPDATE   = 0x40000,
    FL_VEHICLE_TARGET       = 0x80000,
    FL_GROUND_ENT           = 0x100000,
    FL_CURSOR_HINT          = 0x200000,
    FL_USE_TURRET           = 0x400000,
    FL_MISSILE_ATTRACTOR    = 0x800000,
    FL_TARGET               = 0x1000000,
    FL_WEAPON_BEING_GRABBED = 0x2000000,
#ifndef KISAK_SP
    FL_OBSTACLE             = 0x4000000,
#endif
    FL_DODGE_LEFT           = 0x8000000,
    FL_DODGE_RIGHT          = 0x10000000,
    FL_BADPLACE_VOLUME      = 0x20000000,
#ifndef KISAK_SP
    FL_AUTO_BLOCKPATHS      = 0x40000000,
#endif
};

//enum $5313C8230143BA0EE39E1AA1829F5562 : __int32
enum EntHandler_t : uint8_t // (not a real enum name)
{
    ENT_HANDLER_NULL               = 0x0,
    ENT_HANDLER_ACTOR_INIT         = 0x1,
    ENT_HANDLER_ACTOR              = 0x2,
    ENT_HANDLER_ACTOR_CORPSE       = 0x3,
    ENT_HANDLER_TRIGGER_MULTIPLE   = 0x4,
    ENT_HANDLER_TRIGGER_HURT       = 0x5,
    ENT_HANDLER_TRIGGER_HURT_TOUCH = 0x6,
    ENT_HANDLER_TRIGGER_DAMAGE     = 0x7,
    ENT_HANDLER_SCRIPT_MOVER       = 0x8,
    ENT_HANDLER_SCRIPT_MODEL       = 0x9,
    ENT_HANDLER_GRENADE            = 0xA,
    ENT_HANDLER_TIMED_OBJECT       = 0xB,
    ENT_HANDLER_ROCKET             = 0xC,
    ENT_HANDLER_CLIENT             = 0xD,
    ENT_HANDLER_CLIENT_SPECTATOR   = 0xE,
    ENT_HANDLER_CLIENT_DEAD        = 0xF,
    ENT_HANDLER_PLAYER_CLONE       = 0x10,
    ENT_HANDLER_TURRET_INIT        = 0x11,
    ENT_HANDLER_TURRET             = 0x12,
    ENT_HANDLER_DROPPED_ITEM       = 0x13,
    ENT_HANDLER_ITEM_INIT          = 0x14,
    ENT_HANDLER_ITEM               = 0x15,
    ENT_HANDLER_TRIGGER_USE        = 0x16,
    ENT_HANDLER_PRIMARY_LIGHT      = 0x17,
    ENT_HANDLER_PLAYER_BLOCK       = 0x18,
    ENT_HANDLER_VEHICLE_INIT       = 0x19,
    ENT_HANDLER_VEHICLE            = 0x1A,
    ENT_HANDLER_VEHICLE_FREE       = 0x1B,
    ENT_HANDLER_HELICOPTER         = 0x1C,
    ENT_HANDLER_IK_PLAYERCLIP_TERRAIN = 0x1D,
    ENT_HANDLER_IK_DISABLE_TERRAIN_MAPPING = 0x1E,
    ENT_HANDLER_COUNT              = 0x1F,
};

struct entityShared_t // sizeof=0x64
{                                       // XREF: gentity_s/r
    unsigned __int8 linked;
    unsigned __int8 bmodel;
    unsigned __int8 svFlags;
    // padding byte
    int clientMask[1];
    unsigned __int8 inuse;              // XREF: EntHandle::isDefined(void)+20/r
                                        // EntHandle::ent(void)+57/r ...
    // padding byte
    // padding byte
    // padding byte
    int broadcastTime;                  // XREF: SP_worldspawn(SpawnVar *)+246/o
    float mins[3];
    float maxs[3];                      // XREF: G_FreeEntities(bool):loc_626404/r
                                        // SP_worldspawn(SpawnVar *)+23F/w
    int contents;                       // XREF: Path_AttemptLink+B2/w
                                        // Path_AttemptLink+E2/w
    float absmin[3];
    float absmax[3];
    float currentOrigin[3];             // XREF: GetFreeDropCueIdx+19B/o
                                        // G_PrintEntities(void)+6B/r ...
    float currentAngles[3];             // XREF: G_FreeEntities(bool):loc_62649F/r
                                        // SpawnSystem_SetPointsBaseWeight(int,float * const,float,float)+9A/o ...
    EntHandle ownerNum;
    int eventTime;
};

struct item_ent_t // sizeof=0xC
{                                       // XREF: $69C5C147512A8F06DC355830F4729073/r
    int ammoCount;
    int clipAmmoCount;
    int index;
};

struct trigger_ent_t // sizeof=0x2C
{                                       // XREF: $69C5C147512A8F06DC355830F4729073/r
    int threshold;
    int accumulate;
    int timestamp;
    int singleUserEntIndex;
    unsigned __int8 perk;
    bool requireLookAt;
    // padding byte
    // padding byte
    int exposureIndex;
    float exposureLerpToLighter;
    float exposureLerpToDarker;
    float exposureFeather[3];
};

struct mover_ent_t // sizeof=0x60
{                                       // XREF: $69C5C147512A8F06DC355830F4729073/r
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

struct corpse_ent_t // sizeof=0x4
{                                       // XREF: $69C5C147512A8F06DC355830F4729073/r
    int deathAnimStartTime;
};


enum MissileStage : __int32
{                                       // XREF: missile_ent_t::<unnamed_tag>::<unnamed_type_missile>/r
    MISSILESTAGE_SOFTLAUNCH = 0x0,
    MISSILESTAGE_ASCENT     = 0x1,
    MISSILESTAGE_DESCENT    = 0x2,
};

enum MissileFlightMode : __int32
{                                       // XREF: missile_ent_t::<unnamed_tag>::<unnamed_type_missile>/r
    MISSILEFLIGHTMODE_TOP    = 0x0,
    MISSILEFLIGHTMODE_DIRECT = 0x1,
};



struct missile_ent_t // sizeof=0x58
{                                       // XREF: $69C5C147512A8F06DC355830F4729073/r
    int timestamp;
    float time;
    int timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    unsigned __int8 flags;
    // padding byte
    // padding byte
    // padding byte
    int antilagTimeOffset;
    int timeAlive;
    //$0D33AF6AB483EB176B99DAC6E021D6CF ___u9;
    union //$0D33AF6AB483EB176B99DAC6E021D6CF // sizeof=0x20
    {                                       // XREF: missile_ent_t/r
        //missile_ent_t::<unnamed_tag>::<unnamed_type_missile> missile;
        struct
        {
            float curvature[3];
            float targetOffset[3];
            MissileStage stage;
            MissileFlightMode flightMode;
        } missile;
        //missile_ent_t::<unnamed_tag>::<unnamed_type_grenade> grenade;
        struct
        {
            int effectIndex;
        } grenade;
    };
    int forcedDud;
    float grenadeWobbleCycle;
    float grenadeCurve;
};

struct spawner_ent_t // sizeof=0x8
{                                       // XREF: $69C5C147512A8F06DC355830F4729073/r
    int team;
    int timestamp;
};

struct flame_timed_damage_t // sizeof=0x1C
{                                       // XREF: gentity_s/r
    struct gentity_s *attacker;
    int damage;
    float damageDuration;
    float damageInterval;
    int start_timestamp;
    int end_timestamp;
    int lastupdate_timestamp;
};

struct tagInfo_s // sizeof=0x70
{                                       // XREF: tagInfo_t/r
    struct gentity_s *parent;
    struct gentity_s *next;
    unsigned __int16 name;
    // padding byte
    // padding byte
    int index;
    float axis[4][3];
    float parentInvAxis[4][3];
};

struct Destructible;
struct XAnimTree_s;
struct gentity_s // sizeof=0x2F8
{                                       // XREF: Svcmd_EntityList_f(void)+6/o
                                        // .data:gentity_s * g_entities/r ...
    entityState_s s;                    // XREF: ClientEndFrame(gentity_s *)+9A0/r
                                        // ClientDisconnect(int)+251/r ...
    entityShared_t r;                   // XREF: EntHandle::isDefined(void)+20/r
                                        // EntHandle::ent(void)+57/r ...
    struct gclient_s *client;                  // XREF: GetFollowPlayerState(int,playerState_s *)+85/r
                                        // G_PlayerEvent(int,int)+F/r ...
    struct actor_s *actor;                     // XREF: Actor_SetBodyPlantAngle+D3/r
                                        // Actor_SetBodyPlantAngle+E5/r ...
    sentient_s *sentient;               // XREF: Actor_EventListener_Next(int,ai_event_t,int)+55/r
    struct TurretInfo *pTurretInfo;            // XREF: turret_clientaim+397/r
                                        // turret_clientaim+3C5/r
    Destructible *destructible;
    struct scr_vehicle_s *scr_vehicle;
    unsigned __int16 model;
    unsigned __int8 physicsObject;
    unsigned __int8 takedamage;
    unsigned __int8 active;
    unsigned __int8 nopickup;
    unsigned __int8 handler;            // XREF: ShowEntityInfo(void)+1AF/r
    unsigned __int8 team;
    unsigned __int16 classname;         // XREF: G_RunFrame(int)+A68/r
                                        // SP_worldspawn(SpawnVar *)+232/o ...
    unsigned __int16 target;
    unsigned __int16 targetname;        // XREF: GetEntity(ushort const &)+2A/r
                                        // GetEntityIndex(ushort const &)+2A/r
    unsigned __int16 script_noteworthy;
    unsigned int attachIgnoreCollision;
    int spawnflags;                     // XREF: SP_worldspawn(SpawnVar *)+219/w
    int flags;                          // XREF: FinishSpawningItem(gentity_s *)+362/r
                                        // FinishSpawningItem(gentity_s *)+37E/w ...
    int eventTime;
    int freeAfterEvent;
    int unlinkAfterEvent;
    int clipmask;
    int processedFrame;
    EntHandle parent;
    int nextthink;
    int health;                         // XREF: Scr_Vehicle_Think(gentity_s *)+163/r
    int maxHealth;
    int damage;
    flame_timed_damage_t flame_timed_damage[4];
    int last_timed_radius_damage;
    int count;
    gentity_s *chain;
    //$69C5C147512A8F06DC355830F4729073 ___u36;
    union //$69C5C147512A8F06DC355830F4729073 // sizeof=0x60
    {                                       // XREF: gentity_s/r
        item_ent_t item[2];
        trigger_ent_t trigger;
        mover_ent_t mover;
        corpse_ent_t corpse;
        missile_ent_t missile;
        spawner_ent_t spawner;
    };
    EntHandle missileTargetEnt;
    tagInfo_s *tagInfo;
    gentity_s *tagChildren;
    unsigned __int16 attachModelNames[19];
    unsigned __int16 attachTagNames[19];
    XAnimTree_s *pAnimTree;
    unsigned __int16 disconnectedLinks;
    // padding byte
    // padding byte
    int iDisconnectTime;
    int useCount;
    intptr_t physObjId;   // nx-port: holds a PhysObjUserData pointer
    gentity_s *nextFree;
    int birthTime;
    int ikPlayerclipTerrainTime;
    int ikDisableTerrainMappingTime;
};












struct pmoveHandler_t // sizeof=0x20
{                                       // XREF: .data:pmoveHandler_t * pmoveHandlers/r
    void (__cdecl *trace)(struct trace_t *, const float *, const float *, const float *, const float *, int, int, struct col_context_t *);
    void (__cdecl *playerEvent)(int, int);
    bool (__cdecl *getEntityOriginAngles)(int, int, float *, float *);
    unsigned __int16 (__cdecl *getVehicleTypeString)(int, int);
    void (__cdecl *entityLinkFromPMove)(unsigned int, int, int);
    int (__cdecl *getPlayerWeapon)(const struct playerState_s *, const int);
    void (__cdecl *setVehDriverInputs)(const int, const int, struct usercmd_s *);
    bool (__cdecl *isEntWalkable)(int, int);
};


// duplicate prototypes
void __cdecl CG_TraceCapsule(
    struct trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentMask,
    struct col_context_t *context);
bool __cdecl CG_GetEntityOriginAngles(int localClientNum, int entityNum, float *origin, float *angles);
unsigned __int16 __cdecl CG_GetVehicleTypeString(int clientNum, int entityNum);
int __cdecl CG_GetPlayerWeapon(const struct playerState_s *ps, int localClientNum);
void __cdecl CG_SetVehDriverInputs(int localClientNum, int vehEntNum, struct usercmd_s *cmd);
bool __cdecl CG_IsEntWalkable(int localClientNum, int entityNum);

void __cdecl G_TraceCapsule(
    struct trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentmask,
    struct col_context_t *context);
void __cdecl G_PlayerEvent(int clientNum, int event);
bool __cdecl G_GetEntityOriginAngles(int localClientNum, int entityNum, float *origin, float *angles);
unsigned __int16 __cdecl G_GetVehicleTypeString(int clientNum, int entityNum);
void __cdecl G_EntityLinkFromPMove(unsigned int entityNum, int parentEntityNum, int tagName);
int __cdecl G_GetPlayerWeapon(const struct playerState_s *ps, int localClientNum);
void __cdecl G_SetVehDriverInputs(int localClientNum, int vehEntNum, usercmd_s *cmd);
bool __cdecl G_IsEntWalkable(int localClientNum, int entityNum);

static const pmoveHandler_t pmoveHandlers[2] =
{
  {
    CG_TraceCapsule,
    NULL,
    CG_GetEntityOriginAngles,
    CG_GetVehicleTypeString,
    NULL,
    CG_GetPlayerWeapon,
    CG_SetVehDriverInputs,
    CG_IsEntWalkable
  },
  {
    G_TraceCapsule,
    G_PlayerEvent,
    G_GetEntityOriginAngles,
    G_GetVehicleTypeString,
    G_EntityLinkFromPMove,
    G_GetPlayerWeapon,
    G_SetVehDriverInputs,
    G_IsEntWalkable
  }
};
