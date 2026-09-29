#pragma once

#include <universal/dvar.h>
#include <ui/ui_shared.h>
#include <game_mp/g_save_mp.h>
#include <qcommon/msg_mp.h>

enum GVConditionalLhsTypeMask : __int32
{                                       // XREF: GVConditionalLhs/r
    GV_COND_MASK_GENERAL        = 0x1,
    GV_COND_MASK_HIT_BY         = 0x2,
    GV_COND_MASK_PLAYER         = 0x4,
    GV_COND_MASK_ATTACK         = 0x8,
    GV_COND_MASK_ATTACK_CLASS   = 0x10,
    GV_COND_MASK_PLAYER_CLASS   = 0x20,
    GV_COND_MASK_GENERAL_PLAYER = 0x25,
    GV_COND_MASK_GENERAL_ATTACK = 0x19,
    GV_COND_MASK_ALL_CLASS      = 0x30,
};

enum GVType : __int32
{                                       // XREF: GVConditionalRhs/r
    GVTYPE_BOOL   = 0x0,
    GVTYPE_INT    = 0x1,
    GVTYPE_FLOAT  = 0x2,
    GVTYPE_STRING = 0x3,
    GVTYPE_NUM    = 0x4,
};

// Listbox special IDs used by the gametype-variants menus. These are shared with
// ui_feeders_mp.cpp, but intentionally exclude adjacent generic feeder IDs.
enum GameVariantFeeder : __int32
{
    GV_FEEDER_CONDITION_LHS = 35,
    GV_FEEDER_CONDITION_OP = 36,
    GV_FEEDER_CONDITION_RHS = 37,
    GV_FEEDER_RULES = 39,
    GV_FEEDER_EVENTS = 40,
    GV_FEEDER_ACTIONS = 41,
    GV_FEEDER_PARAMETERS = 42,
    GV_FEEDER_TARGETS = 43,
    GV_FEEDER_RULE_SUMMARY = 44,
};

struct GVParameter // sizeof=0x81
{                                       // XREF: GVRule/r GVRule/r
    bool hasParameter;
    char value[128];
};

struct GVConditional // sizeof=0x4
{                                       // XREF: GVRule/r
    bool hasConditional;
    unsigned __int8 m_lhsIndex;
    unsigned __int8 m_operatorIndex;
    unsigned __int8 m_rhsIndex;
};

struct __declspec(align(4)) GVEvent // sizeof=0xC
{
    const char *m_name;
    const char *m_scriptName;
    unsigned __int8 m_targetMask;
    unsigned __int8 m_condMask;
    // padding byte
    // padding byte
};

struct GVRule // sizeof=0x10A
{                                       // XREF: GVGlob/r
    bool isValid;                       // XREF: UI_GameVariants_Reset(void)+2F/w
    unsigned __int8 eventID;
    unsigned __int8 actionID;
    GVParameter parameter;
    GVParameter secondParameter;
    unsigned __int8 targetID;
    GVConditional m_conditional;
};

struct GVGlob // sizeof=0x4284
{                                       // XREF: .data:GVGlob gvGlob/r
    GVRule rules[64];                   // XREF: UI_GameVariants_Reset(void)+2F/w
    int ruleCount;                      // XREF: GetRuleForFeederIndex+F/r
                                        // UI_GV_AddNewRule_f(void)+6/r ...

    GVGlob();
};

struct GVValue // sizeof=0x8
{
    const char *m_displayValue;
    const char *m_value;
};

struct GVAction // sizeof=0x14
{                                       // XREF: .data:stru_98A5D98/r
    const char *m_name;
    const char *m_scriptName;
    unsigned __int8 m_parameterType;
    // padding byte
    // padding byte
    // padding byte
    GVValue *m_parameters;
    int m_parametersSize;

    GVAction(
        const char *name,
        const char *scriptName,
        unsigned __int8 parameterType,
        GVValue *parameters,
        int parametersSize);

    GVAction() {};
};

struct GVConditionalRhs // sizeof=0xC
{                                       // XREF: GVConditionalLhs/r
    GVValue *m_values;
    int m_valuesSize;
    GVType m_type;
};

struct GVConditionalLhs // sizeof=0x18
{
    const char *m_displayName;
    const char *m_scriptName;
    GVConditionalLhsTypeMask m_type;
    GVConditionalRhs m_rhs;
};

struct __declspec(align(4)) GVTarget // sizeof=0xC
{
    char *m_name;
    char *m_scriptName;
    unsigned __int8 m_targetMask;
    // padding byte
    // padding byte
    // padding byte
};

struct _CustomClassData_modifierDef // sizeof=0x8
{
    const char *modifierName;
    int modifierOffset;
};

struct _CustomClassData // sizeof=0x50
{                                       // XREF: CustomClassData/r
    struct modifierDef // sizeof=0x8
    {
        const char *modifierName;
        int modifierOffset;
    };

    static constexpr _CustomClassData::modifierDef modifierIndex[9] =
    {
      { "active", 0 },
      { "team", 44 },
      { "health", 52 },
      { "healthRegeneration", 56 },
      { "healthVampirism", 60 },
      { "movementSpeed", 64 },
      { "movementSprintSpeed", 68 },
      { "damage", 72 },
      { "damageExplosive", 76 }
    };

    static constexpr const char *itemNames[39] =
    {
      "primary",
      "primaryattachment",
      "primaryattachmenttop",
      "primaryattachmentbottom",
      "primaryattachmenttrigger",
      "primaryattachmentmuzzle",
      "primarycamo",
      "primaryreticle",
      "primaryreticlecolor",
      "primarylens",
      "primaryemblem",
      "primarytag",
      "secondary",
      "secondaryattachment",
      "secondaryattachmenttop",
      "secondaryattachmentbottom",
      "secondaryattachmenttrigger",
      "secondaryattachmentmuzzle",
      "secondarycamo",
      "secondaryreticle",
      "secondaryreticlecolor",
      "secondarylens",
      "secondaryemblem",
      "secondarytag",
      "specialty1",
      "specialty2",
      "specialty3",
      "classbonus",
      "head",
      "body",
      "primarygrenade",
      "specialgrenade",
      "equipment",
      "facepaintpattern",
      "facepaintcolor",
      "killstreak1",
      "killstreak2",
      "killstreak3",
      "deathstreak"
    };



    int isActive;
    unsigned __int8 primary;
    unsigned __int8 primaryattachment;
    unsigned __int8 primaryattachmenttop;
    unsigned __int8 primaryattachmentbottom;
    unsigned __int8 primaryattachmenttrigger;
    unsigned __int8 primaryattachmentmuzzle;
    unsigned __int8 primarycamo;
    unsigned __int8 primaryreticle;
    unsigned __int8 primaryreticlecolor;
    unsigned __int8 primarylens;
    unsigned __int8 primaryemblem;
    unsigned __int8 primarytag;
    unsigned __int8 secondary;
    unsigned __int8 secondaryattachment;
    unsigned __int8 secondaryattachmenttop;
    unsigned __int8 secondaryattachmentbottom;
    unsigned __int8 secondaryattachmenttrigger;
    unsigned __int8 secondaryattachmentmuzzle;
    unsigned __int8 secondarycamo;
    unsigned __int8 secondaryreticle;
    unsigned __int8 secondaryreticlecolor;
    unsigned __int8 secondarylens;
    unsigned __int8 secondaryemblem;
    unsigned __int8 secondarytag;
    unsigned __int8 specialty1;
    unsigned __int8 specialty2;
    unsigned __int8 specialty3;
    unsigned __int8 classbonus;
    unsigned __int8 head;
    unsigned __int8 body;
    unsigned __int8 primarygrenade;
    unsigned __int8 specialgrenade;
    unsigned __int8 equipment;
    unsigned __int8 facepaintpattern;
    unsigned __int8 facepaintcolor;
    unsigned __int8 killstreak1;
    unsigned __int8 killstreak2;
    unsigned __int8 killstreak3;
    unsigned __int8 deathstreak;
    // padding byte
    int team;
    int character;
    int health;
    int healthRegeneration;
    int healthVampirism;
    int movementSpeed;
    int movementSprintSpeed;
    int damage;
    int damageExplosive;

    int  GetClassModifierValue(const char *modifierName);
    unsigned __int8  GetLoadoutItem(const char *itemName);
    void  SetLoadoutItem(const char *itemName, unsigned __int8 itemIndex);
    void  PopulateCustomDvarsFromClassData();
    void  SaveClassDataFromDvars();
    void  WriteClassToClientData(msg_t *msg);
    void  ReadClassFromClientData(msg_t *msg);
};

struct _CustomClassDescription // sizeof=0x10
{                                       // XREF: CustomClassDescription/r
                                        // ?UI_Gametype_UpdateClassDataFromDvars_f@@YAXXZ/r
    char name[16];
};

int __cdecl UI_GameVariants_GetClassParameters();
void __cdecl UI_GV_StartAddingEvent_f();
void __cdecl UI_GV_StartEditingFeeder(int localClientNum, GameVariantFeeder feederId, int feederCount);
void __cdecl UI_GV_StartAddingAction_f();
void __cdecl UI_GV_StartAddingParam_f();
void __cdecl UI_GV_StartAddingTarget_f();
void __cdecl UI_GV_StartAddingConditional_f();
void SetNextPopupToAddOrEditConfirm();
void __cdecl UI_GV_EditSelectedRule_f();
GVGlob *__cdecl GetSelectedRule(int localClientNum);
GVGlob *__cdecl GetRuleForFeederIndex(int feederIndex);
void __cdecl UI_GV_UpdateRuleFromDvars(GVRule *rule);
int __cdecl GetEventIndexFromFeederIndex(int feederIndex);
bool __cdecl IsEventValid(GVEvent *ev);
bool __cdecl IsFreeForAllMode();
char __cdecl AreCustomClassesAvailable();
int __cdecl GetActionIndexFromFeederIndex(int feederIndex);
bool __cdecl IsActionValid(GVAction *action);
int __cdecl GetTargetIDFromFeederIndex(int feederIndex);
GVEvent *__cdecl GetEventByDvarIndex(const char *dvarName);
unsigned __int8 __cdecl GetValidTargetMask(unsigned __int8 targetMask);
GVConditional *__cdecl GetConditionalForDvarIndex(
                GVConditional *result,
                const char *lhsDvarIndexName,
                const char *opDvarIndexName,
                const char *rhsDvarIndexName);
unsigned int __cdecl GetCondLhsIndexForFeederIndex(int feederIndex);
unsigned __int8 __cdecl GetValidCondMask(unsigned __int8 condMask);
void __cdecl UI_GV_SetRuleParameterFromDisplayDvars(GVRule *rule);
const char *__cdecl GetParamValueForDvarIndex(int actionIndex, const char *paramIndexDvarName);
const char *__cdecl GetPerkParamValueFromDvarIndex(GVRule *rule, const char *paramDvarIndexName);
void __cdecl UI_GV_AddNewRule_f();
int __cdecl UI_GV_GetFirstInvalidRuleIndex();
void __cdecl UI_GV_DeleteSelected_f();
void __cdecl UI_GV_NavForward_f();
void __cdecl UI_GV_NavBackward_f();
void __cdecl UI_GV_UpdateSelectedRule_f();
void __cdecl UI_InitGametypeVariants();
void __cdecl UI_GV_ResetFeeder_f();
int __cdecl UI_GetGVRulesCount();
int __cdecl UI_GetGVEventCount();
int __cdecl GetValidEventCount();
int __cdecl UI_GetGVActionCount();
int __cdecl GetValidActionCount();
int __cdecl UI_GetGVTargetCount();
int __cdecl GetValidTargetCount(unsigned __int8 targetMask);
int __cdecl UI_GetGVParamCount();
GVAction *__cdecl GetActionByDvarIndex(const char *dvarName);
int __cdecl UI_GetGVCondLhsCount();
int __cdecl GetValidCondLhsCount(unsigned __int8 condMask);
int __cdecl UI_GetGVCondOpCount();
GVConditionalLhs *__cdecl GetCondLhsForDvarIndex(const char *lhsDvarIndexName);
GVConditionalLhs *__cdecl GetCondLhsForFeederIndex(int feederIndex);
int __cdecl UI_GetGVCondRhsCount();
int __cdecl UI_FeederCount_GameVariants(int contextIndex, float feederID);
char *__cdecl GetItemGroupDisplayValue(itemGroup_t itemGroup, const char *value);
char *__cdecl UI_FeederItemText_GameVariantRules(int index, int column);
char *__cdecl UI_GV_GetRuleParamDisplayValue(GVRule *rule);
int __cdecl GetParamIndexForValue(int actionIndex, const char *paramValue);
char *__cdecl UI_FeederItemText_GameVariantEvents(int index, int column);
char *__cdecl UI_FeederItemText_GameVariantActions(int index, int column);
char *__cdecl UI_FeederItemText_GameVariantTargets(int index, int column);
GVTarget *__cdecl GetTargetAtFeederIndex(int feederIndex);
char *__cdecl UI_FeederItemText_GameVariantParams(int index, int column);
char *__cdecl UI_FeederItemText_GameVariantCondLhs(int index, int column);
char *__cdecl UI_FeederItemText_GameVariantCondOp(int index, int column);
char *__cdecl UI_FeederItemText_GameVariantCondRhs(int index, int column);
char *__cdecl UI_FeederItemText_GameVariants(float feederID, int index, int column);
void __cdecl UI_FeederSelection_GameVariantRules(int contextIndex, int index);
int __cdecl GetFeederIndexFromEventIndex(int eventIndex);
int __cdecl GetFeederIndexFromActionIndex(int actionIndex);
int __cdecl GetFeederIndexFromTargetId(int targetId, unsigned __int8 targetMask);
int __cdecl GetFeederIndexForCondLhsIndex(int condLhsIndex, unsigned __int8 condMask);
void __cdecl UI_FeederSelection_GameVariantEvent(int contextIndex, int index);
void __cdecl UI_FeederSelection_GameVariantAction(int localClientNum, int contextIndex, int index);
void __cdecl UI_FeederSelection_GameVariantTarget(int contextIndex, int index);
void __cdecl UI_FeederSelection_GameVariantCondOp(int contextIndex, int index);
void __cdecl UI_FeederSelection_GameVariantCondRhs(int contextIndex, int index);
void __cdecl UI_FeederSelection_GameVariants(int localClientNum, int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_GameVariantParam(int localClientNum, int contextIndex, int index);
void __cdecl UI_FeederSelection_GameVariantCondLhs(int localClientNum, int contextIndex, int index);
void __cdecl UI_GameVariants_ArchiveRule(MemoryFile *memFile, GVRule *rule);
void __cdecl MemFile_ArchiveByte(MemoryFile *memFile, unsigned __int8 *value);
void __cdecl UI_GameVariants_ArchiveParameter(MemoryFile *memFile, GVParameter *param);
unsigned int __cdecl UI_GameVariants_GetRuleSize(const GVRule *rule);
void __cdecl UI_GameVariants_WriteRuleToMemoryFile(MemoryFile *memFile, GVRule *rule);
void __cdecl MemFile_WriteUInt(MemoryFile *memFile, unsigned int value);
void __cdecl UI_GameVariants_WriteToMemoryFile(MemoryFile *memFile);
void __cdecl UI_GameVariants_Reset();
char __cdecl UI_GameVariants_ReadGameVariantRuleChunk(MemoryFile *memFile);
void __cdecl Scr_GameVariants_AddConditional(GVRule *rule);
void __cdecl Scr_GameVariants_AddParams(GVRule *rule);
void __cdecl Scr_GameVariants_AddPerkString(char *paramValue);
void __cdecl Scr_GameVariants_GetRule();

extern const dvar_t *ui_gv_rulecount;
extern const dvar_t *ui_gv_reloadSpeedModifier;
extern const dvar_s *custom_class_mode;

extern _CustomClassData g_customGameModeClasses[10];
