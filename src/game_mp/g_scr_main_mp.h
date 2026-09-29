#pragma once
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_variable.h>
#include <database/db_registry.h>
#include <qcommon/common.h>
#include <game/actor_animapi.h>
#include <game/g_player_corpse.h>
#include <bgame/bg_actor_constants.h>

struct gentity_s;
struct scr_animscript_t;
struct pathnode_t;
enum objectiveState_t;
struct objective_t;
struct XAnimTree_s;
struct scr_animtree_t;

struct gameTypeScript_t // sizeof=0x84
{                                       // XREF: scr_data_t::<unnamed_type_gametype>/r
    char pszScript[64];
    char pszName[64];                   // XREF: Scr_GetGameTypeNameForScript(char const *)+49/o
    int bTeamBased;
};

struct scr_data_t_unnamed_type_gametype // sizeof=0x10BC
{                                       // XREF: scr_data_t/r
    int main;                           // XREF: GScr_LoadGameTypeScript(void)+41/w
                                        // Scr_LoadGameType(void)+4/r ...
    int startupgametype;                // XREF: GScr_LoadGameTypeScript(void)+5C/w
                                        // Scr_StartupGameType(void)+6/r
    int playerconnect;                  // XREF: GScr_LoadGameTypeScript(void)+77/w
                                        // Scr_PlayerConnect(gentity_s *)+6/r
    int playerdisconnect;               // XREF: GScr_LoadGameTypeScript(void)+92/w
                                        // Scr_PlayerDisconnect(gentity_s *)+6/r
    int playerdamage;                   // XREF: GScr_LoadGameTypeScript(void)+AD/w
                                        // Scr_PlayerDamage(gentity_s *,gentity_s *,gentity_s *,int,int,int,int,float const * const,float const * const,hitLocation_t,int)+C6/r
    int playerkilled;                   // XREF: GScr_LoadGameTypeScript(void)+C8/w
                                        // Scr_PlayerKilled(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int,int)+BA/r
    int votecalled;
    int playervote;
    int playerrevive;
    int actordamage;                    // XREF: GScr_LoadGameTypeScript(void)+E3/w
                                        // Scr_ActorDamage(gentity_s *,gentity_s *,gentity_s *,int,int,int,int,float const * const,float const * const,hitLocation_t,int)+C6/r
    int actorkilled;                    // XREF: GScr_LoadGameTypeScript(void)+FE/w
                                        // Scr_ActorKilled(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int)+AC/r
    int vehicledamage;                  // XREF: GScr_LoadGameTypeScript(void)+119/w
                                        // Scr_VehicleDamage(gentity_s *,gentity_s *,gentity_s *,int,int,int,int,float const * const,float const * const,hitLocation_t,int,int,uint,uint)+F0/r
    int vehicleradiusdamage;            // XREF: GScr_LoadGameTypeScript(void)+134/w
                                        // Scr_VehicleRadiusDamage(gentity_s *,gentity_s *,gentity_s *,int,float,float,int,int,int,float const * const,float,float,float const * const,int)+FD/r
    int playerlaststand;                // XREF: GScr_LoadGameTypeScript(void)+14F/w
                                        // Scr_PlayerLastStand(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int)+B8/r
    int iNumGameTypes;                  // XREF: Scr_ParseGameTypeList_LoadObj+30B/w
                                        // Scr_ParseGameTypeList_FastFile+261/w ...
    gameTypeScript_t list[32];          // XREF: Scr_ParseGameTypeList_LoadObj+1E/o
                                        // Scr_ParseGameTypeList_LoadObj+14A/o ...
};

struct scr_data_t
{
    int levelscript;
    int gametypescript;
    scr_data_t_unnamed_type_gametype gametype;
    int delete_;
    int initstructs;
    int createstruct;
    int findstruct;
    AnimScriptList dogAnim;
#ifdef KISAK_SP
    // Retail SP keeps FOUR animscript lists, contiguous at 0x01c79b64 with stride 0x4C0, and
    // publishes their addresses into g_animScriptTable (0x01a48c30) indexed by AISpecies:
    //   [0] human 0x01c79b64   [1] dog 0x01c7a024   [2] zombie 0x01c7a4e4   [3] zombie_dog 0x01c7a9a4
    // dogAnim above is slot 1. The other three are SP-only; MP ships dogs and nothing else.
    // Field ORDER here is deliberately not retail's (retail puts human first) -- dogAnim has
    // to stay where it is so the MP layout is untouched, and nothing serialises scr_data_t.
    AnimScriptList humanAnim;
    AnimScriptList zombieAnim;
    AnimScriptList zombieDogAnim;
    // Two species-independent handles retail stores as standalone globals next to the lists.
    // `scripted` is written by BOTH branches -- animscripts/scripted::init when zombiemode is
    // off (0x007eef75), animscripts/zombie_scripted::init when it is on (0x007ef15a) -- so it
    // is the mode's generic scripted-anim entry point, not a per-species one.
    int scripted;                       // retail global 0x01c79b2c
    int init_mode_sp;                   // retail global 0x01c7ae74, human branch only
#endif
    corpseInfo_t playerCorpseInfo[4];
    XAnimTree_s *actorXAnimTrees[MAX_ACTORS];
    corpseInfo_t actorCorpseInfo[8];
    //_BYTE gap97F8[96];
    int destructible_callback;
    int levelnotify;
    int faceeventnotify;
    int updatespawnpoints;
    int pregamescript;
    int glassSmash;
#ifdef KISAK_SP
    // SP menu-notification callback handles. Appended to scr_data_t (NOT to
    // scr_data_t_unnamed_type_gametype, whose sizeof=0x10BC is load-bearing and would break).
    // Retail SP keeps them as standalone globals: CodeCallback_MenuMessage at 0x01c87514 and
    // CodeCallback_Dec20Message at 0x01c87518, written by GScr_LoadGameTypeScript (0x006124b0)
    // at 0x00612aea and 0x00612b48 respectively -- verified by disassembly this pass.
    int menumessage;                    // CodeCallback_MenuMessage  -- retail global 0x01c87514
    int dec20message;                   // CodeCallback_Dec20Message -- retail global 0x01c87518
#endif
};


struct cached_tag_mat_t // sizeof=0x3C
{                                       // XREF: .data:cached_tag_mat_t cg_cachedTagMat/r
    int time;
    int entnum;
    unsigned __int16 name;              // XREF: GScr_Shutdown(void)+3/r
                                        // GScr_Shutdown(void)+12/o
    // padding byte
    // padding byte
    float tagMat[4][3];                 // XREF: CScr_GetTagOrigin(scr_entref_t)+117/o
};

void assertCmd();
void assertexCmd();
void assertmsgCmd();
void print();
void println();
void GScr_IsCollectors();
unsigned int __cdecl    GScr_AllocString(const char *s);
void __cdecl Scr_LoadLevel();
void __cdecl Scr_LoadPreGame();
void __cdecl GScr_LoadGameTypeScript();
int __cdecl GScr_LoadScriptAndLabel(scriptInstance_t inst, const char *filename, const char *label, int bEnforceExists);
void __cdecl    GScr_LoadScripts(scriptInstance_t inst);
void __cdecl    GScr_LoadDogAnimScripts(scriptInstance_t inst);
#ifdef KISAK_SP
void __cdecl    GScr_LoadHumanAnimScripts(scriptInstance_t inst);
void __cdecl    GScr_LoadZombieAnimScripts(scriptInstance_t inst);
void __cdecl    GScr_LoadZombieDogAnimScripts(scriptInstance_t inst);
void __cdecl    GScr_LoadScriptsAndAnimsForEntities(scriptInstance_t inst);
#endif
void __cdecl    GScr_LoadSingleAnimScript(scriptInstance_t inst, scr_animscript_t *pAnim, const char *name);
void GScr_SetScriptsForPathNodes();
void __cdecl GScr_SetScriptsForPathNode(scriptInstance_t inst, pathnode_t *loadNode);
unsigned __int8 *__cdecl GScr_AnimscriptAlloc(unsigned int size);
int GScr_LoadLevelScript();
int GScr_LoadPreGameScript();
void __cdecl GScr_PostLoadScripts(scriptInstance_t inst);
void __cdecl GScr_FreeScripts(scriptInstance_t inst);
void __cdecl ScrCmd_GetClanId(scr_entref_t entref);
void __cdecl ScrCmd_GetClanName(scr_entref_t entref);
void GScr_CreatePrintChannel();
void GScr_printChannelSet();
void __cdecl Scr_LocalizationError(unsigned int iParm, const char *pszErrorMessage);
void __cdecl Scr_ConstructMessageString(
                int firstParmIndex,
                int lastParmIndex,
                const char *errorContext,
                char *string,
                unsigned int stringLimit);
void __cdecl Scr_ValidateLocalizedStringRef(unsigned int parmIndex, const char *token, int tokenLen);
void __cdecl Scr_MakeGameMessage(int iClientNum, const char *pszCmd);
void __cdecl Scr_VerifyWeaponIndex(int weaponIndex, const char *weaponName);
void iprintln();
void iprintlnbold();
void GScr_print3d();
void GScr_line();
void GScr_box();
void GScr_debugstar();
void GScr_circle();
void GScr_sphere();
int __cdecl Scr_GetArrayValues_Vector(
                unsigned int parameter_index,
                unsigned int parent_id,
                float (*vector_array)[3],
                int vector_array_size,
                const char *array_type_description);
void GScr_linelist();
void GScr_IsDefined();
void GScr_AddDebugCommand();
void GScr_IsMP();
void GScr_IsFloat();
void GScr_IsInt();
void GScr_IsVec();
void GScr_IsString();
void GScr_IsArray();
void GScr_IsAlive();
void GScr_GetDvar();
void GScr_GetDvarInt();
void GScr_GetDvarFloat();
void GScr_GetDvarColorRed();
void GScr_GetDvarColorGreen();
void GScr_GetDvarColorBlue();
void GScr_SetDvar();
void __cdecl CleanDvarValue(const char *dvarValue, char *outString, int size);
void GScr_GetTime();
void GScr_GetAttachmentIndex();
void GScr_GetCorpseArray();
void Scr_GetEntByNum();
void Scr_GetWeaponStowedModel();
void Scr_GetWeaponModel();
void __cdecl GScr_GetAmmoCount(scr_entref_t entref);
gentity_s *__cdecl GetPlayerEntity(scr_entref_t entref);
gentity_s *__cdecl GetEntity(scr_entref_t entref);
void GScr_GetAnimLength();
void GScr_AnimHasNotetrack();
void GScr_GetNotetrackTimes();
void GScr_GetBrushModelCenter();
void GScr_Spawn();
void GScr_SpawnCollision();
void GScr_SpawnVehicle();
void GScr_SpawnPlane();
void GScr_SpawnTimedFX();
gentity_s *__cdecl SpawnTurretInternal(unsigned int classname, float *origin, const char *weaponinfoname);
void GScr_SpawnTurret();
void GScr_SpawnHelicopter();
void __cdecl GScr_SetTurretCarried(scr_entref_t entref);
void GScr_GetAnimTreesLoaded();
void GScr_FindAnimByName();
void GScr_PrecacheTurret();
void __cdecl ScrCmd_SetMoveSpeedScale(scr_entref_t entref);
void __cdecl ScrCmd_GetMoveSpeedScale(scr_entref_t entref);
void __cdecl ScrCmd_attach(scr_entref_t entref);
void __cdecl ScrCmd_detach(scr_entref_t entref);
void __cdecl ScrCmd_detachAll(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachSize(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachModelName(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachTagName(scr_entref_t entref);
void __cdecl ScrCmd_GetAttachIgnoreCollision(scr_entref_t entref);
void __cdecl G_EntityStateSetPartBits(gentity_s *ent, const unsigned int *partBits);
void __cdecl G_EntityStateGetPartBits(const gentity_s *ent, unsigned int *partBits);
void __cdecl ScrCmd_hidepart(scr_entref_t entref);
void __cdecl ScrCmd_showpart(scr_entref_t entref);
void __cdecl ScrCmd_showallparts(scr_entref_t entref);
void __cdecl ScrCmd_SetVisibleToPlayer(scr_entref_t entref);
void __cdecl ScrCmd_SetInvisibleToPlayer(scr_entref_t entref);
void __cdecl ScrCmd_SetVisibleToAll(scr_entref_t entref);
void __cdecl ScrCmd_SetForceNoCull(scr_entref_t entref);
void __cdecl ScrCmd_OverrideLightingOrigin(scr_entref_t entref);
void __cdecl ScrCmd_SetInvisibleToAll(scr_entref_t entref);
void __cdecl ScrCmd_SetVisibleToTeam(scr_entref_t entref);
void __cdecl ScrCmd_IsLinkedTo(scr_entref_t entref);
void __cdecl ScrCmd_LinkTo(scr_entref_t entref);
void __cdecl ScrCmd_PlayerLinkToDelta(scr_entref_t entref);
void __cdecl ScrCmd_Unlink(scr_entref_t entref);
void __cdecl ScrCmd_EnableLinkTo(scr_entref_t entref);
void __cdecl ScrCmd_GetOrigin(scr_entref_t entref);
void __cdecl ScrCmd_GetAngles(scr_entref_t entref);
void __cdecl ScrCmd_GetMins(scr_entref_t entref);
void __cdecl ScrCmd_GetMaxs(scr_entref_t entref);
void __cdecl ScrCmd_GetAbsMins(scr_entref_t entref);
void __cdecl ScrCmd_GetAbsMaxs(scr_entref_t entref);
void __cdecl ScrCmd_GetPointInBounds(scr_entref_t entref);
void __cdecl ScrCmd_GetEye(scr_entref_t entref);
void __cdecl ScrCmd_GetEyeApprox(scr_entref_t entref);
void __cdecl ScrCmd_UseBy(scr_entref_t entref);
void __cdecl ScrCmd_IsTouching(scr_entref_t entref);
void __cdecl ScrCmd_IsTouchingSwept(scr_entref_t entref);
void __cdecl ScrCmd_IsTouchingVolume(scr_entref_t entref);
void ScrCmd_SoundExists();
void __cdecl ScrCmd_PlaySound(scr_entref_t entref);
#ifdef KISAK_SP
void __cdecl ScrCmd_StopSound(scr_entref_t entref);                     // SP 0x00807B30
#endif
void __cdecl ScrCmd_PlaySoundOnTag(scr_entref_t entref);
void __cdecl ScrCmd_PlaySoundToTeam(scr_entref_t entref);
void __cdecl ScrCmd_PlayBattleChatterToTeam(scr_entref_t entref);
gentity_s *__cdecl StartScriptPlayBattleChatterOnEnt(scr_entref_t entref);
void __cdecl ScrCmd_PlaySoundToPlayer(scr_entref_t entref);
void Scr_PlaySoundAtPosition();
void __cdecl ScrCmd_PlayLoopSound(scr_entref_t entref);
void __cdecl ScrCmd_StopLoopSound(scr_entref_t entref);
void __cdecl ScrCmd_Delete(scr_entref_t entref);
void __cdecl SetModelInternal(gentity_s *ent, char *modelName);
void __cdecl ScrCmd_SetModel(scr_entref_t entref);
void __cdecl ScrCmd_SetEnemyModel(scr_entref_t entref);
void __cdecl ScrCmd_GetNormalHealth(scr_entref_t entref);
void __cdecl ScrCmd_SetNormalHealth(scr_entref_t entref);
void __cdecl ScrCmd_DoDamage(scr_entref_t entref);
void __cdecl ScrCmd_GetVelocity(scr_entref_t entref);
void __cdecl GScr_FakeFire(scr_entref_t entref);
void __cdecl GScr_SetCameraSpikeActive(scr_entref_t entref);
void __cdecl ScrCmd_MakeUsable(scr_entref_t entref);
void __cdecl ScrCmd_MakeUnusable(scr_entref_t entref);
void __cdecl ScrCmd_Show(scr_entref_t entref);
void __cdecl ScrCmd_Hide(scr_entref_t entref);
void __cdecl ScrCmd_Ghost(scr_entref_t entref);
void __cdecl ScrCmd_ShowToPlayer(scr_entref_t entref);
void __cdecl ScrCmd_SetContents(scr_entref_t entref);
void __cdecl GScr_StartFiring(scr_entref_t entref);
void __cdecl GScr_StopFiring(scr_entref_t entref);
void __cdecl GScr_ShootTurret(scr_entref_t entref);
void __cdecl GScr_StopShootTurret(scr_entref_t entref);
void __cdecl GScr_SetMode(scr_entref_t entref);
void __cdecl GScr_GetTurretOwner(scr_entref_t entref);
void __cdecl GScr_SetTargetEntity(scr_entref_t entref);
void __cdecl GScr_SetAiSpread(scr_entref_t entref);
void __cdecl GScr_SetPlayerSpread(scr_entref_t entref);
void __cdecl GScr_SetConvergenceTime(scr_entref_t entref);
void __cdecl GScr_SetSuppressionTime(scr_entref_t entref);
void __cdecl GScr_ClearTargetEntity(scr_entref_t entref);
void __cdecl GScr_SetTurretTeam(scr_entref_t entref);
void __cdecl GScr_SetTurretIgnoreGoals(scr_entref_t entref);
void __cdecl GScr_MakeTurretUsable(scr_entref_t entref);
void __cdecl GScr_MakeTurretUnusable(scr_entref_t entref);
void __cdecl GScr_SetTurretAccuracy(scr_entref_t entref);
void __cdecl GScr_GetTurretTarget(scr_entref_t entref);
void __cdecl GScr_DisconnectPaths(scr_entref_t entref);
void __cdecl GScr_ConnectPaths(scr_entref_t entref);
void __cdecl ScrCmd_GetStance(scr_entref_t entref);
void __cdecl ScrCmd_SetStance(scr_entref_t entref); // LWSS ADD
void __cdecl Scr_SetStableMissile(scr_entref_t entref);
void __cdecl GScr_SetCursorHint(scr_entref_t entref);
int __cdecl G_GetHintStringIndex(int *piIndex, char *pszString);
void __cdecl GScr_SetHintString(scr_entref_t entref);
void __cdecl GScr_SetHintStringForPerk(scr_entref_t entref);
void __cdecl GScr_SetHintLowPriority(scr_entref_t entref);
void __cdecl GScr_SetReviveHintString(scr_entref_t entref);
void __cdecl GScr_UseTriggerRequireLookAt(scr_entref_t entref);
void __cdecl GScr_IsMartyrdomGrenade(scr_entref_t entref);
void __cdecl GScr_GetEntityNumber(scr_entref_t entref);
void __cdecl GScr_EnableGrenadeTouchDamage(scr_entref_t entref);
void __cdecl GScr_DisableGrenadeTouchDamage(scr_entref_t entref);
void __cdecl GScr_MissileSetTarget(scr_entref_t entref);
void __cdecl GScr_EnableAimAssist(scr_entref_t entref);
void __cdecl GScr_DisableAimAssist(scr_entref_t entref);
void __cdecl G_InitObjectives();
int __cdecl ObjectiveStateIndexFromString(objectiveState_t *piStateIndex, unsigned int stateString);
void __cdecl ClearObjective(objective_t *obj);
void Scr_Objective_Add();
void __cdecl ClearObjective_OnEntity(objective_t *obj);
void __cdecl SetObjectiveIcon(objective_t *obj, unsigned int paramNum);
void Scr_Objective_Delete();
void Scr_Objective_State();
void Scr_Objective_Icon();
void Scr_Objective_Position();
void Scr_Objective_OnEntity();
void Scr_Objective_Current();
void Scr_Objective_SetVisibleToPlayer();
void Scr_Objective_SetInvisibleToPlayer();
void Scr_Objective_SetVisibleToAll();
void Scr_Objective_SetInvisibleToAll();
void Scr_Objective_SetSize();
void Scr_Objective_SetColor();
void GScr_Objective_Team();
void __cdecl GetNormalised2DMapPosition(float *inPos, float *outPos);
void __cdecl SetArtilleryIconLocation();
void GScr_LogPrint();
void GScr_WorldEntNumber();
void GScr_Obituary();
void __cdecl GScr_ReviveObituary();
void GScr_LerpFloat();
void GScr_LerpVector();
void GScr_AddDemoBookmark();
void __cdecl Scr_UpdateSpawnPoints();
void GScr_positionWouldTelefrag();
void __cdecl Scr_BoundsWouldTelefrag(float *mins, float *maxs);
void GScr_BoundsWouldTelefrag();
int __cdecl GScr_ReadTeamForSpawnPoints(unsigned int index);
void GScr_RecordUsedSpawnPoint();
void GScr_getStartTime();
void GScr_PrecacheMenu();
int __cdecl GScr_GetScriptMenuIndex(const char *pszMenu);
void GScr_PrecacheStatusIcon();
int __cdecl GScr_GetStatusIconIndex(const char *pszIcon);
void GScr_PrecacheHeadIcon();
int __cdecl GScr_GetHeadIconIndex(const char *pszIcon);
void GScr_WeaponClipSize();
void GScr_WeaponIsSemiAuto();
void GScr_WeaponIsBoltAction();
void GScr_WeaponType();
void GScr_WeaponClass();
void GScr_WeaponIsMountable();
void GScr_WeaponInventoryType();
void GScr_WeaponStartAmmo();
void GScr_WeaponMaxAmmo();
void GScr_WeaponAltWeaponName();
void GScr_GetWatcherWeapons();
void GScr_GetRetrievableWeapons();
void GScr_GetWeaponIndexFromName();
void GScr_GetWeaponFireSound();
void GScr_GetWeaponFireSoundPlayer();
void GScr_GetWeaponPickupSound();
void GScr_GetWeaponPickupSoundPlayer();
void GScr_IsTurretFiring();
void __cdecl GScr_SetDefaultDropPitch(scr_entref_t entref);
void __cdecl GScr_SetScanningPitch(scr_entref_t entref);
void GScr_WeaponFireTime();
void GScr_WeaponReloadTime();
void GScr_IsWeaponClipOnly();
void GScr_IsWeaponDetonationTimed();
void GScr_PrecacheLocationSelector();
int __cdecl GScr_GetLocSelIndex(const char *mtlName);
void Scr_BulletTrace();
void Scr_BulletTracePassed();
void __cdecl Scr_SightTracePassed();
void Scr_PhysicsTrace();
void Scr_PlayerPhysicsTrace();
void Scr_PlayerBulletTrace(); // LWSS ADD
void Scr_RandomInt();
void Scr_RandomFloat();
void Scr_RandomIntRange();
void Scr_RandomFloatRange();
void GScr_log();
void GScr_sin();
void GScr_cos();
void GScr_tan();
void GScr_asin();
void GScr_acos();
void GScr_atan();
void GScr_abs();
void GScr_min();
void GScr_max();
void GScr_floor();
void GScr_ceil();
void GScr_sqrt();
void GScr_CastInt();
void GScr_CastFloat();
void GScr_VectorFromLineToPoint();
void GScr_PointOnSegmentNearestToPoint();
void Scr_Distance();
void Scr_Distance2D();
void Scr_DistanceSquared();
void Scr_Length();
void Scr_LengthSquared();
void Scr_Closer();
void Scr_VectorDot();
void Scr_VectorCross();
void Scr_VectorNormalize();
void Scr_VectorToAngles();
void Scr_VectorLerp();
void Scr_AnglesToUp();
void Scr_AnglesToRight();
void Scr_AnglesToForward();
void Scr_CombineAngles();
void Scr_ClampAngle180();
void Scr_AbsAngleClamp180();
void Scr_RotatePoint();
void Scr_IsSubStr();
void Scr_GetSubStr();
void Scr_ToLower();
void Scr_StrTok();
void __cdecl GScr_NeedsRevive(scr_entref_t entref);
void __cdecl GScr_IsInSecondChance(scr_entref_t entref);
void __cdecl GScr_SetBurn(scr_entref_t entref);
void __cdecl GScr_SetElectrified(scr_entref_t entref);
void __cdecl GScr_StartTanning(scr_entref_t entref);
void __cdecl GScr_StopBurning(scr_entref_t entref);
void __cdecl GScr_SetWaterDrops(scr_entref_t entref);
void __cdecl GScr_SpawnNapalmGroundFlame(scr_entref_t entref);
void __cdecl GScr_RestoreDefaultDropPitch(scr_entref_t entref);
void __cdecl GScr_clearCenterPopups(scr_entref_t entref);
void __cdecl GScr_clearPopups(scr_entref_t entref);
void __cdecl GScr_DisplayGameModeMessage(scr_entref_t entref);
void __cdecl GScr_DisplayTeamMessage(scr_entref_t entref);
void __cdecl GScr_DisplayMedal(scr_entref_t entref);
void __cdecl GScr_DisplayContract(scr_entref_t entref);
void __cdecl GScr_DisplayChallengeComplete(scr_entref_t entref);
void __cdecl GScr_DisplayEndGameMilestoneComplete(scr_entref_t entref);
void __cdecl GScr_DisplayEndGame(scr_entref_t entref);
void __cdecl GScr_ClearEndGameComplete(scr_entref_t entref);
void __cdecl GScr_DisplayKillstreak(scr_entref_t entref);
void __cdecl GScr_DisplayRankUp(scr_entref_t entref);
void __cdecl GScr_DisplayWagerPopup(scr_entref_t entref);
void __cdecl GScr_DisplayHudAnim(scr_entref_t entref);
void __cdecl GScr_IsFiringTurret(scr_entref_t entref);
void __cdecl GScr_IsTurretLockedOn(scr_entref_t entref);
void __cdecl GScr_ShootUp(scr_entref_t entref);
void __cdecl GScr_GetWaterHeight();
void __cdecl GScr_DepthInWater(scr_entref_t entref);
void __cdecl GScr_DepthOfPlayerInWater(scr_entref_t entref);
void Scr_SoundFade();
void Scr_PrecacheModel();
void __cdecl Scr_ErrorOnDefaultAsset(XAssetType type, char *assetName);
void Scr_PrecacheShellShock();
void Scr_PrecacheItem();
void Scr_PrecacheShader();
void Scr_PrecacheString();
void Scr_GrenadeExplosionEffect();
void GScr_RadiusDamage();
void __cdecl GScr_RadiusDamageInternal(gentity_s *inflictor);
void __cdecl GScr_EntityRadiusDamage(scr_entref_t entref);
void GScr_GlassRadiusDamage();
void __cdecl GScr_Detonate(scr_entref_t entref);
void GScr_SetPlayerIgnoreRadiusDamage();
void __cdecl GScr_DamageConeTrace(scr_entref_t entref);
void __cdecl GScr_DamageConeTraceInternal(scr_entref_t entref, int contentMask);
void __cdecl GScr_SightConeTrace(scr_entref_t entref);
void __cdecl GScr_PlayerSightTrace(scr_entref_t entref);
void __cdecl GScr_HeliTurretSightTrace(scr_entref_t entref);
void __cdecl GScr_HeliTurretDogTrace(scr_entref_t entref);
void __cdecl GScr_VisionSetLerpRatio(scr_entref_t entref);
void __cdecl GScr_DirectionalHitIndicator(scr_entref_t entref);
void __cdecl GScr_DoCowardsWayAnims(scr_entref_t entref);
void __cdecl GScr_StartPoisoning(scr_entref_t entref);
void __cdecl GScr_StopPoisoning(scr_entref_t entref);
void __cdecl GScr_StartBinocs(scr_entref_t entref);
void __cdecl GScr_StopBinocs(scr_entref_t entref);
void __cdecl GScr_IsFlared(scr_entref_t entref);
void __cdecl GScr_IsPoisoned(scr_entref_t entref);
void GScr_GetMoveDelta();
void GScr_GetAngleDelta();
void GScr_GetNorthYaw();
void Scr_LoadFX();
void Scr_PlayFX();
void __cdecl Scr_SetFxAngles(unsigned int givenAxisCount, float (*axis)[3], float *angles);
void __cdecl Scr_FxParamError(unsigned int paramIndex, const char *errorString, int fxId);
void Scr_PlayFXOnTag();
void Scr_PlayLoopedFX();
void Scr_SpawnFX();
void Scr_TriggerFX();
void __cdecl ScrCmd_SpawnActor(scr_entref_t entref);
void __cdecl GScr_CreateDynEntAndLaunch();
void Scr_PhysicsExplosionSphere();
void Scr_CreateStreamerHint();
void Scr_PhysicsRadiusJolt();
void Scr_PhysicsExplosionCylinder();
void Scr_SetExponentialFog();
void __cdecl Scr_SetFog(
                const char *cmd,
                float start,
                float density,
                float heightDensity,
                float baseHeight,
                float r,
                float g,
                float b,
                float time,
                float colorScale,
                float sunColR,
                float sunColG,
                float sunColB,
                float sunDirX,
                float sunDirY,
                float sunDirZ,
                float sunStartAng,
                float sunEndAng,
                float maxFogOpacity);
void Scr_SetVolumetricFog();
void Scr_SetCullDist();
void Scr_VisionSetNaked();
void Scr_VisionSetNight();
void Scr_TableLookupRowNum();
void Scr_TableLookupColumnForRow();
void Scr_TableLookup();
void Scr_TableLookupIString();
void __cdecl Scr_GetReflectionLocs();
void __cdecl Scr_GetReflectionOrigin();
void GScr_IsPlayer();
void GScr_IsPlayerNumber();
void GScr_SetWinningPlayer();
void GScr_SetWinningTeam();
void GScr_Announcement();
void GScr_ClientAnnouncement();
void GScr_GetTeamScore();
void GScr_SetTeamScore();
void GScr_SetClientNameMode();
void GScr_UpdateClientNames();
void GScr_GetTeamPlayersAlive();
void GScr_GetDroppedWeapons();
void __cdecl GScr_GetNumParts();
void __cdecl GScr_GetPartName();
void GScr_Earthquake();
void __cdecl GScr_ShellShock(scr_entref_t entref);
void __cdecl GScr_StopShellShock(scr_entref_t entref);
void __cdecl GScr_GetTagOrigin(scr_entref_t entref);
int __cdecl GScr_UpdateTagInternal(
                gentity_s *ent,
                unsigned int tagName,
                cached_tag_mat_t *cachedTag,
                int showScriptError);
void __cdecl GScr_GetTagAngles(scr_entref_t entref);
void __cdecl GScr_GetEntnum(scr_entref_t entref);
void __cdecl GScr_SetDepthOfField(scr_entref_t entref);
void __cdecl GScr_SetViewModelDepthOfField(scr_entref_t entref);
void __cdecl GScr_ViewKick(scr_entref_t entref);
void __cdecl GScr_LocalToWorldCoords(scr_entref_t entref);
void __cdecl GScr_SetRightArc(scr_entref_t entref);
void __cdecl GScr_SetLeftArc(scr_entref_t entref);
void __cdecl GScr_SetTopArc(scr_entref_t entref);
void __cdecl GScr_SetBottomArc(scr_entref_t entref);
void __cdecl GScr_PlaceSpawnPoint(scr_entref_t entref);
void __cdecl ScrCmd_SendFaceEvent(scr_entref_t entref);
void GScr_TestSpawnPoint();
void GScr_MapRestart();
void GScr_LoadMap();
void GScr_ExitLevel();
void GScr_KillServer();
void GScr_AddTestClient();
void GScr_MakeDvarServerInfo();
void GScr_SetBombTimer();
void GScr_SetMatchTalkFlag();
void GScr_SetMatchFlag();
void GScr_AllClientsPrint();
void GScr_MapExists();
void GScr_IsValidGameType();
void GScr_SetVoteString();
void GScr_SetVoteTime();
void GScr_SetVoteYesCount();
void GScr_SetVoteNoCount();
void GScr_KickPlayer();
void GScr_BanPlayer();
void GScr_ClientPrint();
void GScr_OpenFile();
void GScr_CloseFile();
void GScr_FPrintln();
void __cdecl Scr_FPrint_internal(bool commaBetweenFields);
void GScr_FPrintFields();
void GScr_FReadLn();
void GScr_FGetArg();
void GScr_ExecDevgui();
void Scr_IsGlobalStatsServer();
void GScr_SetPlayerStatsForMatchRecording();
void GScr_SetPlayerFinalForMatchRecording();
void GScr_SetBeginForMatchRecording();
void GScr_GetAssignedTeam();
void __cdecl GScr_SendLeaderboards(scr_entref_t entref);
void __cdecl GScr_IsItemPurchased(scr_entref_t entref);
bool __cdecl GScr_IsItemLockedForChallenge(scr_entref_t entref, bool purchaseRequired);
bool __cdecl GScr_IsItemPurchasedForClientNum(unsigned int clientNum, unsigned int itemIndex);
void __cdecl GScr_IsItemLocked(scr_entref_t entref);
void GScr_GetRefFromItemIndex();
void GScr_GetItemGroupFromItemIndex();
void GScr_GetBaseWeaponItemIndex();
void GScr_GetGameTypeEnumFromName();
void GScr_GetWagerGametypeList();
void __cdecl GScr_GetLoadoutItemFromProfile(scr_entref_t entref);
void __cdecl GScr_GetDStat(scr_entref_t entref);
void GScr_GetMaxActiveContracts();
void __cdecl GScr_GetIndexForActiveContract(scr_entref_t entref);
void GScr_GetContractStatType();
void GScr_GetContractStatName();
void GScr_GetContractRewardXP();
void GScr_GetContractRewardCP();
void GScr_GetContractRequirements();
void GScr_GetContractName();
void GScr_GetContractRequiredCount();
void GScr_GetContractResetConditions();
void __cdecl GScr_GetActiveContractProgress(scr_entref_t entref);
void __cdecl GScr_IncrementActiveContractProgress(scr_entref_t entref);
void __cdecl GScr_ResetActiveContractProgress(scr_entref_t entref);
void __cdecl GScr_IncrementActiveContractTime(scr_entref_t entref);
void __cdecl GScr_IsActiveContractComplete(scr_entref_t entref);
void __cdecl GScr_HasActiveContractExpired(scr_entref_t entref);
void __cdecl GScr_GetActiveContractTimePassed(scr_entref_t entref);
void GScr_GetFogSettings();
void GScr_EnableOccluder();
void Gscr_GetCustomClassLoadoutItem();
void Gscr_GetCustomClassLoadoutModifier();
void __cdecl GScr_SetDStat(scr_entref_t entref);
void GScr_UploadStats();
void GScr_GetItemAttachment();
void GScr_GetDefaultClassSlot();
void __cdecl GScr_SetTeamForTrigger(scr_entref_t entref);
void __cdecl GScr_SetPerkForTrigger(scr_entref_t entref);
void __cdecl GScr_SetIgnoreEntForTrigger(scr_entref_t entref);
void __cdecl GScr_ClientClaimTrigger(scr_entref_t entref);
void __cdecl GScr_ClientReleaseTrigger(scr_entref_t entref);
void __cdecl GScr_ReleaseClaimedTrigger(scr_entref_t entref);
void GScr_SetMapCenter();
void GScr_SetDemoIntermissionPoint();
void GScr_StartDemoRecording();
void GScr_StopDemoRecording();
void GScr_IsDemoRecording();
void isDemoEnabled();
void __cdecl GScr_isTestClient(scr_entref_t entref);
void __cdecl GScr_isDemoClient(scr_entref_t entref);
void GScr_SetGameEndTime();
void GScr_SetTimeScale();
void GScr_SetMiniMap();
void GScr_IncrementEscrow();
void GScr_SetTeamSpyplane();
void GScr_GetTeamSpyplane();
void GScr_SetTeamSatellite();
void GScr_GetTeamSatellite();
void GScr_GetArrayKeys();
void __cdecl GScr_Launch(scr_entref_t entref);
void __cdecl GScr_MagicBullet();
void __cdecl GScr_LaunchBomb(scr_entref_t entref);
void __cdecl GScr_MakeGrenadeDud(scr_entref_t entref);
void __cdecl GScr_IsOnLadder(scr_entref_t entref);
void __cdecl GScr_IsMantling(scr_entref_t entref);
void __cdecl GScr_StartDoorBreach(scr_entref_t entref);
void __cdecl GScr_StopDoorBreach(scr_entref_t entref);
void __cdecl GScr_GetLightColor(scr_entref_t entref);
gentity_s *__cdecl GScr_SetupLightEntity(scr_entref_t entref);
void __cdecl GScr_SetLightColor(scr_entref_t entref);
void __cdecl GScr_GetLightIntensity(scr_entref_t entref);
void __cdecl GScr_SetLightIntensity(scr_entref_t entref);
void __cdecl GScr_GetLightRadius(scr_entref_t entref);
void __cdecl GScr_SetLightRadius(scr_entref_t entref);
void __cdecl GScr_GetLightFovInner(scr_entref_t entref);
void __cdecl GScr_GetLightFovOuter(scr_entref_t entref);
void __cdecl GScr_SetLightFovRange(scr_entref_t entref);
void __cdecl GScr_GetLightExponent(scr_entref_t entref);
void __cdecl GScr_SetLightExponent(scr_entref_t entref);
void __cdecl GScr_StartRagdoll(scr_entref_t entref);
void __cdecl GScr_IsRagdoll(scr_entref_t entref);
void __cdecl GScr_RagdollLaunch(scr_entref_t entref);
void __cdecl GScr_VehicleLaunch(scr_entref_t entref);
void __cdecl GScr_GiveAchievement(scr_entref_t entref);
void __cdecl GScr_SetOwner(scr_entref_t entref);
void __cdecl GScr_SetTurretOwner(scr_entref_t entref);
void __cdecl GScr_SetTurretType(scr_entref_t entref);
void __cdecl GScr_SetTeam(scr_entref_t entref);
void __cdecl GScr_GetTeam(scr_entref_t entref);
void __cdecl GScr_GetCorpseAnim(scr_entref_t entref);
void __cdecl ScrCmd_ItemWeaponSetAmmo(scr_entref_t entref);
void __cdecl Scr_AddStruct();
void __cdecl Scr_ResetTimeout();
void GScr_ClientSysRegister();
void GScr_ClientSysSetState();
void GScr_IsAI();
void GScr_GetAITriggerFlags();
void GScr_IsVehicle();
void __cdecl ScrCmd_GetShootAtPosition(scr_entref_t entref);
void __cdecl GScr_GetMaxVehicles();
void __cdecl GScr_DisableDestructiblePieces();
void __cdecl GScr_EnableAllDestructiblePieces();
void __cdecl GScr_ClearSpawnPoints();
void __cdecl GScr_SetSpawnPointRandomVariation();
void __cdecl GScr_ClearSpawnPointsBaseWeight();
void __cdecl GScr_SetSpawnPointsBaseWeight();
void __cdecl GScr_AddSpawnPoints();
void __cdecl GScr_GetSortedSpawnPoints();
void __cdecl GScr_IsSpawnPointVisible();
void __cdecl GScr_PredictGrenade(scr_entref_t entref);
void __cdecl GScr_AddSphereInfluencer();
void __cdecl InfluencerTypeValidation(int type, gentity_s *ent, const char *function_name);
void __cdecl GScr_AddCylinderInfluencer();
void __cdecl GScr_RemoveInfluencer();
void __cdecl GScr_EnableInfluencer();
void __cdecl GScr_SetInfluencerTeamMask();
void __cdecl GScr_SetDebugSideSwitch();
void GScr_CollisionTestPointsInSphere();
void GScr_CollisionTestPointsInCylinder();
void GScr_CollisionTestPointsInPill();
void GScr_CollisionTestPointsInCone();
void GScr_CollisionTestPointsInBox();
void GScr_QSortScoredSpawnPointArrayAscending();
int __cdecl sort_scored_spawn_point_vectors_ascending(float *a, float *b);
void GScr_Matrix4x4TransformPoints();
void GScr_GetNumGVRules();
void GScr_GetGVRule();
void GScr_GetWeaponMinDamageRange();
void GScr_GetWeaponMaxDamageRange();
void GScr_GetWeaponMinDamage();
void GScr_GetWeaponMaxDamage();
void GScr_GetWeaponFuseTime();
void GScr_GetWeaponProjExplosionSound();
void GScr_IsWeaponSpecificUse();
void GScr_IsWeaponEquipment();
void GScr_IsWeaponPrimary();
void GScr_IsWeaponScopeOverlay();
void GScr_PCServerUpdatePlaylist();
void __cdecl GScr_GetPregameTeam(scr_entref_t entref);
void __cdecl GScr_GetPregameClass(scr_entref_t entref);
void __cdecl GScr_SetPregameTeam(scr_entref_t entref);
void __cdecl GScr_SetPregameClass(scr_entref_t entref);
void GScr_PixBeginEvent();
void GScr_PixMarker();
void GScr_IncrementCounter();
void GScr_GetCounterTotal();
void GScr_SetScoreboardColumns();
void __cdecl GScr_SetNemesisXuid(scr_entref_t entref);
void GScr_IsPregameEnabled();
void GScr_ResetPregameData();
void GScr_IsPregameGameStarted();
void GScr_PregameStartGame();
void (__cdecl *__cdecl Scr_GetFunction(const char **pName, int *type))();
void __cdecl GScr_SetClientFlag(scr_entref_t entref);
void __cdecl GScr_GetClientFlag(scr_entref_t entref); // LWSS ADD
void __cdecl GScr_ClearClientFlag(scr_entref_t entref);
void __cdecl GScr_IsMissileInsideHeightLock(scr_entref_t entref);
void __cdecl GScr_IsOnGround(scr_entref_t entref);
void __cdecl GScr_GetGroundEnt(scr_entref_t entref); // LWSS ADD
XAnimTree_s *__cdecl GScr_GetEntAnimTree(gentity_s *ent);
void __cdecl G_FlagAnimForUpdate(gentity_s *ent);
void __cdecl GScr_SetAnim(scr_entref_t entref);
void __cdecl GScr_SetAnimInternal(scr_entref_t entref, char flags);
void __cdecl GScr_HandleAnimError(int error);
double __cdecl GScr_GetOptionalFloat(unsigned int iParamIndex, float fDefault);
#ifdef KISAK_SP
// SP-only server anim script methods. Retail SP registers all of these in its
// builtin method table at 0x00A54218; the MP reconstruction only ever had
// "setanim" (SP methods_3 idx 114, handler 0x00801640 -> GScr_SetAnimInternal).
void __cdecl GScr_ClearAnim(scr_entref_t entref);                       // SP 0x00800660
void __cdecl GScr_SetAnimKnobInternal(scr_entref_t entref, char flags); // SP 0x00800880
void __cdecl GScr_SetAnimKnob(scr_entref_t entref);                     // SP 0x00800CC0
void __cdecl GScr_SetAnimKnobLimited(scr_entref_t entref);              // SP 0x00800CE0
void __cdecl GScr_SetAnimKnobRestart(scr_entref_t entref);              // SP 0x00800D00
void __cdecl GScr_SetAnimKnobAllInternal(scr_entref_t entref, char flags); // SP 0x00800D40
void __cdecl GScr_SetAnimKnobAll(scr_entref_t entref);                  // SP 0x00801170
void __cdecl GScr_SetAnimKnobAllRestart(scr_entref_t entref);           // SP 0x008011B0
void __cdecl GScr_SetAnimLimited(scr_entref_t entref);                  // SP 0x00801660
void __cdecl GScr_SetAnimRestart(scr_entref_t entref);                  // SP 0x00801680
void __cdecl GScr_GetAnimTime(scr_entref_t entref);                     // SP 0x008016C0
void __cdecl GScr_SetFlaggedAnimKnobInternal(scr_entref_t entref, char flags); // SP 0x00801870
void __cdecl GScr_SetFlaggedAnimKnob(scr_entref_t entref);              // SP 0x00801CF0
void __cdecl GScr_SetFlaggedAnimKnobRestart(scr_entref_t entref);       // SP 0x00801D30
void __cdecl GScr_SetFlaggedAnimKnobLimitedRestart(scr_entref_t entref);// SP 0x00801D50
void __cdecl GScr_SetFlaggedAnimKnobAllInternal(scr_entref_t entref, char flags, const char *pszUsageError); // SP 0x00801D70
void __cdecl GScr_SetFlaggedAnimKnobAll(scr_entref_t entref);           // SP 0x008021C0
void __cdecl GScr_SetFlaggedAnimKnobAllRestart(scr_entref_t entref);    // SP 0x008021F0
void __cdecl GScr_SetFlaggedAnimInternal(scr_entref_t entref, char flags); // SP 0x00802220
void __cdecl GScr_SetFlaggedAnim(scr_entref_t entref);                  // SP 0x008026A0
void __cdecl GScr_SetFlaggedAnimLimited(scr_entref_t entref);           // SP 0x008026C0
void __cdecl GScr_SetFlaggedAnimRestart(scr_entref_t entref);           // SP 0x008026E0
void __cdecl GScr_SetAnimTime(scr_entref_t entref);                     // SP 0x008027D0
void __cdecl GScr_StartScriptedAnim_SP(scr_entref_t entref, bool startActorThread, bool dontInterpolate);
bool __cdecl GScr_IsScriptedAnimActive_SP(const gentity_s *ent);
void __cdecl GScr_UpdateScriptedAnim_SP(gentity_s *ent);
bool __cdecl GScr_RunScriptedMover_SP(gentity_s *ent);
void __cdecl GScr_ClearScriptedAnim_SP(gentity_s *ent);
void __cdecl GScr_ResetGroundReferenceState_SP();
void __cdecl GScr_ResetChangeLevel_SP();
void __cdecl GScr_UpdateChangeLevel_SP();
#endif
void __cdecl G_SetAnimTree(gentity_s *ent, scr_animtree_t *animtree);
void __cdecl GScr_UseAnimTree(scr_entref_t entref);
void (__cdecl *__cdecl Scr_GetMethod(const char **pName, int *type))(scr_entref_t);
void (__cdecl *__cdecl BuiltIn_GetMethod(const char **pName, int *type))(scr_entref_t);
void __cdecl Scr_SetOrigin(gentity_s *ent, int);
void __cdecl Scr_SetAngles(gentity_s *ent, int);
void __cdecl Scr_SetExposureIndex(gentity_s *ent, int);
void __cdecl Scr_SetExposureLerpToLighter(gentity_s *ent, int);
void __cdecl Scr_SetExposureLerpToDarker(gentity_s *ent, int);
void __cdecl Scr_SetHealth(gentity_s *ent, int);
void __cdecl GScr_AddVector(float *vVec);
void __cdecl GScr_AddEntity(gentity_s *pEnt);
void __cdecl Scr_ParseGameTypeList();
int Scr_ParseGameTypeList_LoadObj();
void Scr_ParseGameTypeList_FastFile();
char *__cdecl Scr_GetGameTypeNameForScript(const char *pszGameTypeScript);
bool __cdecl Scr_IsValidGameType(const char *pszGameType);
void __cdecl Scr_LoadGameType();
void __cdecl Scr_StartupGameType();
void __cdecl Scr_PlayerConnect(gentity_s *self);
void __cdecl Scr_PlayerDisconnect(gentity_s *self);
void __cdecl Scr_PlayerDamage(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                int dflags,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vPoint,
                float *vDir,
                hitLocation_t hitLoc,
#ifdef KISAK_SP
                int modelIndex,
#endif
                int timeOffset);
void __cdecl Scr_PlayerKilled(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vDir,
                hitLocation_t hitLoc,
                int psTimeOffset,
                int deathAnimDuration);
void __cdecl Scr_ActorDamage(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                int dflags,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vPoint,
                float *vDir,
                hitLocation_t hitLoc,
#ifdef KISAK_SP
                int modelIndex,
#endif
                int timeOffset);
void __cdecl Scr_ActorKilled(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vDir,
                hitLocation_t hitLoc,
                int psTimeOffset);
void __cdecl Scr_VehicleRadiusDamage(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                float fInnerDamage,
                float fOuterDamage,
                int dflags,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vPoint,
                float fRadius,
                float coneAngleCos,
                float *coneDirection,
                int timeOffset);
void __cdecl Scr_VehicleDamage(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                int dflags,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vPoint,
                float *vDir,
                hitLocation_t hitLoc,
                int timeOffset,
                unsigned int damageFromUnderneath,
                unsigned int modelIndex,
                unsigned int partName);
void __cdecl Scr_PlayerLastStand(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                unsigned int meansOfDeath,
                unsigned int iWeapon,
                float *vDir,
                hitLocation_t hitLoc,
                int psTimeOffset);
void __cdecl Scr_VoteCalled(gentity_s *self, char *command, char *param1, char *param2);
void __cdecl Scr_PlayerVote(gentity_s *self, char *option);
void __cdecl GScr_Shutdown();
void __cdecl GScr_Gdt_Update(char *asset, char *keyValue);
void __cdecl Scr_GlassSmash(float *pos, float *dir);


extern scr_data_t g_scr_data;
