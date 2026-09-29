#pragma once

#include <clientscript/cscr_animtree.h>
#include <bgame/bg_local.h>

#define ANIM_TOGGLEBIT (1 << 10)

enum aistateEnum_t : __int32
{                                       // XREF: ?BG_AnimScriptAnimation@@YAHPAUpmove_t@@W4aistateEnum_t@@W4scriptAnimMoveTypes_t@@H@Z/r
    AISTATE_COMBAT = 0x0,
    MAX_AISTATES   = 0x1,
};
inline aistateEnum_t &operator--(aistateEnum_t &e) {
    e = static_cast<aistateEnum_t>(static_cast<int>(e) - 1);
    return e;
}
inline aistateEnum_t &operator--(aistateEnum_t &e, int i)
{
    --e;
    return e;
}

enum animScriptParseMode_t : __int32
{                                       // XREF: BG_AnimParseAnimScript/r
    PARSEMODE_DEFINES           = 0x0,
    PARSEMODE_ANIMATION         = 0x1,
    PARSEMODE_CANNED_ANIMATIONS = 0x2,
    PARSEMODE_STATECHANGES      = 0x3,
    PARSEMODE_EVENTS            = 0x4,
    PARSEMODE_FORCELOAD         = 0x5,
    NUM_PARSEMODES              = 0x6,
};

enum animScriptConditionTypes_t : __int32
{                                       // XREF: animConditionTable_t/r
    ANIM_CONDTYPE_BITFLAGS   = 0x0,     // XREF: .data:animConditionsTable/s
    ANIM_CONDTYPE_VALUE      = 0x1,     // XREF: .data:animConditionsTable/s
    ANIM_CONDTYPE_ENABLED    = 0x2,     // XREF: .data:animConditionsTable/s
    ANIM_CONDTYPE_STRINGHASH = 0x3,     // XREF: .data:animConditionsTable/s
    NUM_ANIM_CONDTYPES       = 0x4,
};

enum scriptAnimMoveTypes_t : __int32
{                                       // XREF: PM_Footsteps_NotMoving/r
                                        // PM_ApplyMovementAnimations/r ...
    ANIM_MT_UNUSED           = 0x0,
    ANIM_MT_IDLE             = 0x1,
    ANIM_MT_SHUFFLE          = 0x2,     // XREF: .rdata:moveAnimTable/s
    ANIM_MT_WALK             = 0x3,     // XREF: .rdata:moveAnimTable/s
    ANIM_MT_RUN              = 0x4,     // XREF: .rdata:moveAnimTable/s
    ANIM_MT_CLIMBUP          = 0x5,
    ANIM_MT_CLIMBDOWN        = 0x6,
    ANIM_MT_SPRINT           = 0x7,
    ANIM_MT_MANTLE_ROOT      = 0x8,
    ANIM_MT_MANTLE_UP_57     = 0x9,
    ANIM_MT_MANTLE_UP_51     = 0xA,
    ANIM_MT_MANTLE_UP_45     = 0xB,
    ANIM_MT_MANTLE_UP_39     = 0xC,
    ANIM_MT_MANTLE_UP_33     = 0xD,
    ANIM_MT_MANTLE_UP_27     = 0xE,
    ANIM_MT_MANTLE_UP_21     = 0xF,
    ANIM_MT_MANTLE_OVER_HIGH = 0x10,
    ANIM_MT_MANTLE_OVER_MID  = 0x11,
    ANIM_MT_MANTLE_OVER_LOW  = 0x12,
    ANIM_MT_TURNRIGHT        = 0x13,
    ANIM_MT_TURNLEFT         = 0x14,
    ANIM_MT_STUMBLE          = 0x15,    // XREF: .rdata:moveAnimTable/s
    ANIM_MT_STUMBLE_WALK     = 0x16,    // XREF: .rdata:moveAnimTable/s
    ANIM_MT_STUMBLE_SPRINT   = 0x17,
    ANIM_MT_SWIM             = 0x18,
    ANIM_MT_DTP              = 0x19,
    ANIM_MT_SLIDE            = 0x1A,
    NUM_ANIM_MOVETYPES       = 0x1B,
};

enum scriptAnimEventTypes_t : __int32
{                                       // XREF: ?BG_AnimScriptEventGetCommand@@YAPAUanimScriptCommand_t@@PAUplayerState_s@@W4scriptAnimEventTypes_t@@@Z/r
                                        // ?BG_AnimScriptEvent@@YAHPAUpmove_t@@W4scriptAnimEventTypes_t@@HH@Z/r
    ANIM_ET_UNUSED               = 0x0,
    ANIM_ET_DEATH                = 0x1,
    ANIM_ET_FIREWEAPON           = 0x2,
    ANIM_ET_JUMP                 = 0x3,
    ANIM_ET_LAND                 = 0x4,
    ANIM_ET_DROPWEAPON           = 0x5,
    ANIM_ET_RAISEWEAPON          = 0x6,
    ANIM_ET_RELOAD               = 0x7,
    ANIM_ET_CROUCH_TO_PRONE      = 0x8,
    ANIM_ET_PRONE_TO_CROUCH      = 0x9,
    ANIM_ET_STAND_TO_CROUCH      = 0xA,
    ANIM_ET_CROUCH_TO_STAND      = 0xB,
    ANIM_ET_PRONE_TO_STAND       = 0xC,
    ANIM_ET_PRONE_TO_SPRINT      = 0xD,
    ANIM_ET_MELEEATTACK          = 0xE,
    ANIM_ET_KNIFE_MELEE          = 0xF,
    ANIM_ET_KNIFE_MELEE_CHARGE   = 0x10,
    ANIM_ET_FLINCH               = 0x11,
    ANIM_ET_SHELLSHOCK           = 0x12,
    ANIM_ET_VEHICLE_ENTER        = 0x13,
    ANIM_ET_VEHICLE_CHANGE_SEATS = 0x14,
    ANIM_ET_VEHICLE_EXIT         = 0x15,
    ANIM_ET_SCRIPTEVENT          = 0x16,
    ANIM_ET_DTP_TAKEOFF          = 0x17,
    ANIM_ET_DTP_LAND             = 0x18,
    ANIM_ET_STAND_TO_LASTSTAND   = 0x19,
    ANIM_ET_CROUCH_TO_LASTSTAND  = 0x1A,
    ANIM_ET_PRONE_TO_LASTSTAND   = 0x1B,
    ANIM_ET_LASTSTAND_TO_STAND   = 0x1C,
    ANIM_ET_LASTSTAND_TO_CROUCH  = 0x1D,
    ANIM_ET_LASTSTAND_TO_PRONE   = 0x1E,
    ANIM_ET_FLARED               = 0x1F,
    ANIM_ET_LASTSTAND_SUICIDE    = 0x20,
    ANIM_ET_PRIME_GRENADE        = 0x21,
    NUM_ANIM_EVENTTYPES          = 0x22,
};

enum animBodyPart_t : __int32
{                                       // XREF: ?BG_PlayAnim@@YAHPAUplayerState_s@@HW4animBodyPart_t@@HHHH@Z/r
    ANIM_BP_UNUSED     = 0x0,
    ANIM_BP_LEGS       = 0x1,
    ANIM_BP_TORSO      = 0x2,
    ANIM_BP_BOTH       = 0x3,
    NUM_ANIM_BODYPARTS = 0x4,
};

enum scriptAnimMoveStatusStates_t : __int32
{
    ANIM_MOVESTATUS_STATIONARY = 0x0,
    ANIM_MOVESTATUS_WALK       = 0x1,
    ANIM_MOVESTATUS_RUN        = 0x2,
    NUM_ANIM_MOVESTATUSES      = 0x3,
};

enum scriptAnimStances_t : __int32
{
    ANIM_STANCE_STAND  = 0x0,
    ANIM_STANCE_CROUCH = 0x1,
    ANIM_STANCE_PRONE  = 0x2,
    NUM_ANIM_STANCES   = 0x3,
};

enum scriptAnimDirections_t : __int32
{                                       // XREF: ?BG_LocalEvalDirection@@YAXPAUpmove_t@@@Z/r
    ANIM_DIRECTION_NONE     = 0x0,
    ANIM_DIRECTION_FORWARD  = 0x1,
    ANIM_DIRECTION_BACKWARD = 0x2,
    ANIM_DIRECTION_LEFT     = 0x3,
    ANIM_DIRECTION_RIGHT    = 0x4,
    NUM_ANIM_DIRECTIONS     = 0x5,
};

enum scriptAnimSlopeStates_t : __int32
{                                       // XREF: ?BG_LocalEvalSlope@@YAXPAUpmove_t@@@Z/r
    ANIM_SLOPE_NONE      = 0x0,
    ANIM_SLOPE_UP        = 0x1,
    ANIM_SLOPE_DOWN      = 0x2,
    NUM_ANIM_SLOPESTATES = 0x3,
};

struct animConditionTable_t // sizeof=0x10
{                                       // XREF: .data:animConditionsTable/r
    animScriptConditionTypes_t type;    // XREF: BG_EvaluateConditions+269/r
                                        // BG_SetConditionBit(int,int,int)+4C/r ...
    struct animStringItem_t *values;           // XREF: BG_AnimParseAnimScript+4C4/r
                                        // BG_ParseConditions+C0/r ...
    int minValue;
    int maxValue;
};

struct __declspec(align(8)) animation_s // sizeof=0x78
{                                                                             // XREF: animScriptData_t/r
                                                                                // animation_t/r
    char name[64];
    int initialLerp;
    int finalLerp;
    float forceAnimRate;
    float moveSpeed;
    float rotSpeed;
    int duration;
    int nameHash;
    int flags;
    __int64 movetype;
    int stance;
    int movestatus;
    int noteType;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};


struct animStringItem_t // sizeof=0x8
{                                                                             // XREF: BG_AddVehicleName(char const *,int,int,int,int)+19A/w
        const char *string;                                 // XREF: BG_InitWeaponString(int,char const *)+9/w
        int hash;                                                     // XREF: BG_InitWeaponString(int,char const *)+1F/w
};

struct animScriptCondition_t // sizeof=0x8
{                                                                             // XREF: animScriptItem_t/r
        int index;
        unsigned int value;
};

struct animScriptCommand_t // sizeof=0x14
{                                                                             // XREF: animScriptItem_t/r
        __int16 bodyPart[2];
        __int16 animIndex[2];
        unsigned __int16 animDuration[2];
        snd_alias_list_t *soundAlias;
        unsigned __int16 tagName;                     // XREF: _Com_InitHunkMemory+BD/o
                                                                                // Sys_GetPhysicalCpuCount+131/o ...
        unsigned __int16 flags;
};

struct animScriptItem_t // sizeof=0xC8
{                                                                             // XREF: animScriptData_t/r
                                                                                // BG_AnimParseAnimScript/r
        int numConditions;
        animScriptCondition_t conditions[4];
        int numCommands;
        animScriptCommand_t commands[8];        // XREF: _Com_InitHunkMemory+BD/o
                                                                                // Sys_GetPhysicalCpuCount+131/o ...
};

struct animVehicleSeats_t // sizeof=0x4
{                                       // XREF: .data:animVehicleSeats/r
    int seats;                          // XREF: BG_AddVehicleName(char const *,int,int,int,int)+DB/w
};

struct animScript_t // sizeof=0x204
{                                       // XREF: animScriptData_t/r
    int numItems;
    animScriptItem_t *items[128];
};

struct __declspec(align(8)) animScriptData_t // sizeof=0x8D388
{                                       // XREF: bgsAnim_s/r
    animation_s animations[1024];
    unsigned int numAnimations;
    animScript_t scriptAnims[1][27];
    animScript_t scriptCannedAnims[1][27];
    animScript_t scriptStateChange[1][1];
    animScript_t scriptEvents[34];
    animScriptItem_t scriptItems[2048]; // XREF: _Com_InitHunkMemory+BD/o
    // Sys_GetPhysicalCpuCount+131/o ...
    int numScriptItems;
    scr_animtree_t animTree;
    unsigned __int16 bodyAnim;
    unsigned __int16 mainAnim;
    unsigned __int16 torsoAnim;
    unsigned __int16 legsAnim;
    unsigned __int16 turningAnim;
    // padding byte
    // padding byte
    snd_alias_list_t *(__cdecl *soundAlias)(const char *);
    void(__cdecl *playSoundAlias)(int, snd_alias_list_t *);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct controller_info_t // sizeof=0x60
{                                       // XREF: ?BG_Player_DoControllersSetup@@YAXPBUentityState_s@@PAUclientInfo_t@@H@Z/r
    float angles[6][3];
    float tag_origin_angles[3];         // XREF: BG_Player_DoControllersSetup(entityState_s const *,clientInfo_t *,int)+13C/o
    float tag_origin_offset[3];         // XREF: BG_Player_DoControllersSetup(entityState_s const *,clientInfo_t *,int)+165/o
};

struct bgsAnim_s // sizeof=0x8D3A8
{                                       // XREF: .data:bgsAnim_s cg_bgsAnim/r
                                        // .data:bgsAnim_s level_bgsAnim/r ...
    animScriptData_t animScriptData;    // XREF: _Com_InitHunkMemory+BD/o
    // Sys_GetPhysicalCpuCount+131/o ...
//bgsAnim_s::<unnamed_type_generic_human> generic_human;
    struct //bgsAnim_s::<unnamed_type_generic_human> // sizeof=0x14
    {                                       // XREF: bgsAnim_s/r
        scr_animtree_t tree;
        scr_anim_s body;
        scr_anim_s main;
        scr_anim_s torso;
        scr_anim_s legs;
    } generic_human;

    //bgsAnim_s::<unnamed_type_generic_dog> generic_dog;
    struct //bgsAnim_s::<unnamed_type_generic_dog> // sizeof=0x8
    {                                       // XREF: bgsAnim_s/r
        scr_animtree_t tree;
        scr_anim_s main;
    } generic_dog;

    unsigned int done_notify;
};

struct playerState_s;
struct loadAnim_t;
struct clientInfo_t;
struct entityState_s;
struct lerpFrame_t;
struct pmove_t;
struct DObj;

int __cdecl BG_StringHashValue(const char *fname);
void BG_AnimParseError(const char *msg, ...);
unsigned int __cdecl BG_AnimationIndexForString(const char *string);
int __cdecl BG_IndexForString(const char *token, animStringItem_t *strings, int allowFail);
const char *__cdecl BG_StringForIndex(int index, animStringItem_t *strings, int allowFail);
void __cdecl BG_InitWeaponString(int index, const char *name);
void __cdecl BG_InitWeaponStrings();
void __cdecl BG_AddVehicleName(const char *vehName, int vehType, int drivers, int gunners, int passengers);
unsigned int __cdecl BG_AnimGetConditionValue(animScriptItem_t *scriptItem, int conditionIndex);
void __cdecl BG_InitVehicles();
int __cdecl BG_GetVehicleAnimSetIndex(unsigned __int16 animSetStr);
void __cdecl BG_ParseCommands(const char **input, animScriptItem_t *scriptItem, animScriptData_t *scriptData);
int __cdecl GetValueForBitfield(unsigned int bitfield);
bool __cdecl Com_BitCheckAssert(const unsigned int *array, int bitNum, int size);
int __cdecl BG_PlayAnim(
                playerState_s *ps,
                int animNum,
                animBodyPart_t bodyPart,
                int forceDuration,
                int setTimer,
                int isContinue,
                int force);
const char *__cdecl BG_GetCurrentStance(playerState_s *ps);
const char *__cdecl BG_GetCurrentMoveStatus(playerState_s *ps);
const char *__cdecl BG_GetCurrentDirection(playerState_s *ps);
const char *__cdecl BG_GetCurrentDmgDirection(playerState_s *ps);
const char *__cdecl BG_GetCurrentDmgType(playerState_s *ps);
const char *__cdecl BG_GetCurrentSlope(playerState_s *ps);
const char *__cdecl BG_GetCurrentWeaponName(playerState_s *ps);
const char *__cdecl BG_GetCurrentWeaponClass(playerState_s *ps);
char *__cdecl BG_GetCurrentPlayerAnimType(playerState_s *ps);
int __cdecl BG_ExecuteCommand(
                playerState_s *ps,
                animScriptCommand_t *scriptCommand,
                int setTimer,
                int isContinue,
                int force);
int __cdecl BG_AnimScriptAnimation(pmove_t *pm, aistateEnum_t state, scriptAnimMoveTypes_t movetype, int force);
animScriptItem_t *__cdecl BG_FirstValidItem(unsigned int client, animScript_t *script);
int __cdecl BG_EvaluateConditions(clientInfo_t *ci, animScriptItem_t *scriptItem);
const char *__cdecl GetWeaponTypeName(int type);
char *__cdecl BG_GetConditionString(int condition, int value);
const char *__cdecl GetBodyPart(int bodypart);
animScriptCommand_t *__cdecl BG_AnimScriptEventGetCommand(playerState_s *ps, scriptAnimEventTypes_t event);
int __cdecl BG_AnimScriptEvent(pmove_t *pm, scriptAnimEventTypes_t event, int isContinue, int force);
void __cdecl BG_SetConditionValue(unsigned int client, unsigned int condition, unsigned __int64 value);
void __cdecl BG_SetConditionBit(unsigned int client, int condition, int value);
void __cdecl Com_BitSetAssert(unsigned int *array, int bitNum, int size);
unsigned int __cdecl BG_GetConditionValue(clientInfo_t *ci, unsigned int condition);
unsigned int __cdecl BG_GetConditionBit(const clientInfo_t *ci, unsigned int condition);
scriptAnimMoveTypes_t __cdecl BG_GetAnimMoveType(const clientInfo_t *ci);
struct bgsAnim_s *__cdecl BG_GetAnimationForIndex(int client, unsigned int index);
int __cdecl BG_GetPlayerStateWeaponIndex(const pmove_t *pm);
void __cdecl BG_AnimUpdatePlayerStateConditions(pmove_t *pmove);
bool __cdecl BG_IsCrouchingAnim(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsAds(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsProneAnim(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsKnifeMeleeAnim(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsGrenadeAnim(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsSliding(const clientInfo_t *ci);
void __cdecl BG_LerpOffset(float *offset_goal, float maxOffsetChange, float *offset);
void __cdecl BG_Player_DoControllersSetup(const entityState_s *es, clientInfo_t *ci, int frametime);
void __cdecl BG_Player_DoControllersInternal(const entityState_s *es, const clientInfo_t *ci, controller_info_t *info);
void __cdecl BG_LerpAngles(float *angles_goal, float maxAngleChange, float *angles);
void __cdecl BG_PlayerAnimation(int localClientNum, const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_RunLerpFrameRate(
                int localClientNum,
                clientInfo_t *ci,
                lerpFrame_t *lf,
                int newAnimation,
                const entityState_s *es);
void __cdecl BG_SetNewAnimation(
                int localClientNum,
                clientInfo_t *ci,
                lerpFrame_t *lf,
                int newAnimation,
                const entityState_s *es);
void __cdecl BG_PlayerAnimation_VerifyAnim(struct XAnimTree_s *pAnimTree, struct lerpFrame_t *lf);
void __cdecl BG_PlayerAngles(const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_SwingAngles(
                float destination,
                float swingTolerance,
                float clampTolerance,
                float speed,
                float *angle,
                int *swinging);
void __cdecl BG_AnimPlayerConditions(const entityState_s *es, const clientInfo_t *ci);
void __cdecl BG_UpdatePlayerDObj(
                int localClientNum,
                DObj *pDObj,
                entityState_s *es,
                clientInfo_t *ci,
                int attachIgnoreCollision);
int __cdecl BG_GetPlayerWeaponForDObj(int localClientNum, entityState_s *es);
int __cdecl BG_GetPlayerOffhandWeaponForDObj(int localClientNum, entityState_s *es);
void __cdecl BG_FindAnimTrees();
scr_animtree_t __cdecl BG_FindAnimTree(const char *filename, int bEnforceExists);
#ifdef KISAK_SP
XAnim_s *__cdecl BG_GetActorAnims();   // SP-only: the "generic_human" AI actor animtree
#endif
void __cdecl BG_LoadAnim(const char *levelName);
void __cdecl BG_AnimParseAnimScript(
                animScriptData_t *scriptData,
                loadAnim_t *pLoadAnims,
                unsigned int *piNumAnims,
                const char *levelName);
char *__cdecl BG_CopyStringIntoBuffer(const char *string, char *buffer, unsigned int bufSize, unsigned int *offset);
void __cdecl BG_ParseConditionBits(
                const char **text_pp,
                animStringItem_t *stringTable,
                int condIndex,
                unsigned int *result);
void __cdecl BG_SetAnimConditionFlags(int condIndex, unsigned int result);
int __cdecl BG_ParseConditions(const char **text_pp, animScriptItem_t *scriptItem);
void BG_FindAnims();
void __cdecl BG_PostLoadAnim(const char *levelName);
void __cdecl BG_FinalizePlayerAnims(const char *levelName);
loadAnim_t *__cdecl BG_LoadAnimForAnimIndex(unsigned int iAnimIndex);
void __cdecl BG_SetupAnimNoteTypes(animScriptData_t *scriptData);


extern thread_local struct bgs_t *bgs; // tls+8
extern bgs_t level_bgs;