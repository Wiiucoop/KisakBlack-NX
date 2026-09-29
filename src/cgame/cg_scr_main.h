#pragma once
#include <qcommon/common.h>
#include <game_mp/g_main_mp.h>

struct BuiltinMethodDef // sizeof=0xC
{                                       // XREF: .rdata:client_methods/r
                                        // .rdata:client_project_methods/r ...
    const char *actionString;           // XREF: CScr_GetMethod(char const * *,int *)+30/r
                                        // CScr_GetMethod(char const * *,int *)+99/r ...
    void (__cdecl *actionFunc)(scr_entref_t);
                                        // XREF: CScr_GetMethod(char const * *,int *)+B8/r
                                        // CScr_GetMethodProjectSpecific(char const * *,int *)+B8/r ...
    int type;                           // XREF: CScr_GetMethod(char const * *,int *)+AA/r
                                        // CScr_GetMethodProjectSpecific(char const * *,int *)+AA/r ...
};

struct BuiltinFunctionDef // sizeof=0xC
{                                       // XREF: .data:BuiltinFunctionDef * client_functions/r
    const char *actionString;           // XREF: CScr_GetFunction(char const * *,int *)+33/r
                                        // CScr_GetFunction(char const * *,int *)+9C/r
    void (__cdecl *actionFunc)();       // XREF: CScr_GetFunction(char const * *,int *)+BB/r
    int type;                           // XREF: CScr_GetFunction(char const * *,int *)+AD/r
};

struct cg_level_locals_t // sizeof=0x3A1C
{                                       // XREF: .data:cg_level_locals_t cg_level/r
    int scriptPrintChannel;             // XREF: CScr_StructContents(uint)+2E/r
                                        // CScr_StructContents(uint)+80/r ...
    SpawnVar spawnVar;                  // XREF: CScr_LoadStructs(void):loc_4FB5D0/o
                                        // CScr_LoadStructs(void)+83/o ...
    int triggerIndex;                   // XREF: CG_ProcessTriggerDebug(centity_s *,trigger_info_t *):loc_4C8328/r
                                        // CG_ProcessTriggerDebug(centity_s *,trigger_info_t *)+188/r ...
    trigger_info_t pendingTriggerList[256];
                                        // XREF: CG_Trigger(centity_s *,centity_s *)+AF/o
                                        // CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+E5E/o
    trigger_info_t currentTriggerList[256];
                                        // XREF: CG_NotifyTriggers(void)+48/o
                                        // CG_NotifyTriggers(void)+15C/o ...
    int pendingTriggerListSize;         // XREF: CG_Trigger(centity_s *,centity_s *):loc_4C8648/r
                                        // CG_Trigger(centity_s *,centity_s *):loc_4C8696/r ...
    int currentTriggerListSize;         // XREF: CG_NotifyTriggers(void)+36/r
                                        // CG_NotifyTriggers(void):loc_4C859B/r ...
    int entTriggerIndex[1536];          // XREF: CG_ProcessTriggerDebug(centity_s *,trigger_info_t *)+A8/r
                                        // CG_ProcessTriggerDebug(centity_s *,trigger_info_t *)+181/r ...
};

struct cent_field_s // sizeof=0x18
{                                       // XREF: .rdata:cent_fields/r
    const char *name;
    int ofs;
    int size[1];
    fieldtype_t type;
    void (__cdecl *setter)(centity_s *, const cent_field_s *);
    void (__cdecl *getter)(centity_s *, const cent_field_s *);
};

struct cscr_data_t // sizeof=0x68
{                                       // XREF: .data:cscr_data_t cg_scr_data/r
    int delete_;                        // XREF: CG_FreeEntityDelay(centity_s *)+4/r
                                        // CG_FreeEntityDelay(centity_s *)+F/r ...
    int initstructs;                    // XREF: CScr_LoadStructs(void)+6/r
                                        // CScr_LoadStructs(void)+37/r ...
    int createstruct;                   // XREF: CG_SpawnStruct(SpawnVar const &):loc_4FB456/r
                                        // CG_SpawnStruct(SpawnVar const &)+67/r ...
    int findstruct;                     // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+72/w
                                        // CG_FindStruct(SpawnVar &,int):loc_6ECC16/r ...
    int levelscript;                    // XREF: CScr_LoadLevel(void)+4/r
                                        // CScr_LoadLevel(void)+11/r ...
    int clientsysstatechange;           // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+8F/w
                                        // CG_ParseClientSystemStateChange(int,int,char const *)+13/r ...
    int maprestart;                     // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+AC/w
                                        // CG_MapRestart(int,int)+2E2/r ...
    int localclientconnect;             // XREF: CG_Init(int,int,int,int)+C38/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+C9/w ...
    int localclientdisconnect;          // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+E6/w
    int entityspawned;                  // XREF: CG_Actor(int,centity_s *)+8F/r
                                        // CG_ProcessFakeEntity+6B3/r ...
    int scriptmodelspawned;
    int dogSoundNotify;                 // XREF: CG_ProcessClientNote+39C/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+25F/w
    int clientFlagCB;                   // XREF: CG_ClientFlagCallback:loc_4B09A7/r
                                        // CG_ClientFlagCallback+20/r ...
    int clientFlagAsValCB;              // XREF: CG_ClientFlagCallback:loc_4B09DD/r
                                        // CG_ClientFlagCallback+56/r ...
    int destructible_callback;          // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+177/w
                                        // CScr_DestructibleCallback(centity_s *,ushort,int,float,int)+4/r ...
    int corpse_callback;                // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+194/w
                                        // CG_ResetEntity+5FB/r
    int playerFootstep;                 // XREF: CScr_PlayerFootstepEvent(int,centity_s *,bool,int,int,bool)+98/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+1B1/w
    int playerJump;                     // XREF: CScr_PlayerJumpEvent(int,centity_s *,bool,int,bool)+62/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+1CE/w
    int playerLand;                     // XREF: CScr_PlayerLandEvent(int,centity_s *,bool,int,bool,bool)+8A/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+1EB/w
    int playerFoliage;                  // XREF: CScr_PlayerFoliageEvent(int,centity_s *,bool,bool)+43/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+208/w
    int playerspawned;                  // XREF: CG_Player(int,centity_s *)+1FA/r
                                        // CScr_SetUniqueClientScripts(ScriptFunctions *)+120/w
    int levelnotify;                    // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+242/w
                                        // ProcessWeaponNoteTracks+107/r ...
    int airsupport;
    int entityshutdownCB;               // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+2D3/w
                                        // CG_ShutdownEntity(int,centity_s *,bool)+C3/r
    int glassSmash;                     // XREF: CScr_GlassSmash(float const * const,float const * const)+4/r
                                        // CScr_GlassSmash(float const * const,float const * const)+2B/r ...
    int gibEvent;
};

void __cdecl CScr_AddStruct();
void __cdecl CScr_SetClientScripts(ScriptFunctions *functions);
unsigned __int16 __cdecl CScr_GetMeansOfDeathConstString(int mod);
void __cdecl CScr_Notify(int localClientNum, centity_s *ent, unsigned __int16 stringValue, unsigned int paramcount);
void __cdecl CScr_ToLower();
void __cdecl CScr_ReadOnly(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetType(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetWeaponName(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_SetOrigin(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_SetAngles(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetModel(centity_s *cent, const cent_field_s *pField);
void __cdecl VehicleCScr_GetVehicleInfoField(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetVehicleClass(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetVehicleTreadFxArray(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetVehicleTreadFx(centity_s *cent, const cent_field_s *pField);
unsigned int __cdecl CScr_GetFakeEntity(unsigned int localClientNum);
void __cdecl CG_SetFakeEntInUse(int localClientNum, fake_centity_s *ent);
void CScr_SpawnFakeEnt();
int __cdecl CScr_GetLocalClientNum(unsigned int index);
void CScr_DeleteFakeEnt();
void __cdecl CScr_FreeFakeEntity(unsigned int localClientNum, unsigned int entNum);
void CScr_SetFakeEntOrg();
void __cdecl CScr_StructContents(unsigned int index);
void CScr_StructInfo();
void CScr_IsDefined();
void CScr_IsString();
void CScr_IsArray();
void assertCmd();
void assertexCmd();
void assertmsgCmd();
void print();
void println();
void CScr_IPrintLnBold();
void CScr_Print3D();
void CScr_RandomInt();
void CScr_RandomFloat();
void CScr_RandomIntRange();
void CScr_RandomFloatRange();
void CScr_VectorFromLineToPoint();
void CScr_PointOnSegmentNearestToPoint();
void CScr_Distance();
void CScr_Distance2D();
void CScr_DistanceSquared();
void CScr_Length();
void CScr_LengthSquared();
void CScr_Closer();
void CScr_VectorDot();
void CScr_VectorCross();
void CScr_VectorNormalize();
void CScr_VectorToAngles();
void CScr_VectorLerp();
void CScr_LerpFloat();
void CScr_LerpVector();
void CScr_AnglesToUp();
void CScr_AnglesToRight();
void CScr_AnglesToForward();
void CScr_CombineAngles();
void CScr_ClampAngle180();
void CScr_AbsAngleClamp180();
int __cdecl Effect_Index(int clientNum, const char *name);
void CScr_LoadedFX();
void CScr_PlayLoopedFX();
void __cdecl CScr_BulletTracer();
void __cdecl CScr_AnimateUI();
void __cdecl CScr_GetAnimLength();
void __cdecl CScr_ShowUI();
void __cdecl CScr_GetSystemTime();
void __cdecl CScr_GetServerTime();
void __cdecl CScr_ClientHasSnapShot();
void CScr_GetDvar();
void CScr_SetDvar();
void CScr_SetSavedDvar();
void CScr_GetDvarColorRed();
void CScr_GetDvarColorGreen();
void CScr_GetDvarColorBlue();
void CScr_GetDvarColorAlpha();
void CScr_GetDvarInt();
void CScr_GetDvarFloat();
void CScr_GetDvarVector();
void CScr_GetDebugDvar();
void CScr_GetDebugDvarInt();
void CScr_GetDebugDvarFloat();
void CScr_SetDvarBool();
void CScr_SetDvarFloat();
void CScr_PlaySound();
void __cdecl CScr_PlaySoundOnEntity(scr_entref_t entref);
void CScr_PlayLoopSound();
void CScr_StopLoopSound();
void __cdecl CScr_PlayLoopSoundOnEntity(scr_entref_t entref);
void __cdecl CScr_StopLoopSoundOnEntity(scr_entref_t entref);
void __cdecl CScr_IsPlayingLoopSound(scr_entref_t entref);
void CScr_SetSoundContext();
void CScr_SoundLoopEmitter();
void CScr_SoundStopLoopEmitter();
void CScr_SoundLineEmitter();
void CScr_SoundStopLineEmitter();
void CScr_GetAliasName();
void CScr_GetAliasCount();
void CScr_IsAliasLoop();
void CScr_StopLocalSound();
void CScr_SoundTimescale();
void CScr_SoundPlaying();
void CScr_SetSoundVolume();
void CScr_SetSoundPitch();
void CScr_SetSoundVolumeRate();
void CScr_SetSoundPitchRate();
void CScr_GetRealTime();
void CScr_GetPlaybackTime();
void CScr_GetKnownLength();
void CScr_SetGroupSnapshot();
void CScr_SetAmbientSnapshot();
void CScr_SetHealthSnapshot();
void CScr_SetLevelFadeSnapshot();
void __cdecl CScr_StopSound();
void __cdecl CScr_ScaleSpeed();
void CScr_DeactivateReverb();
void CScr_SetReverb();
void CScr_GetArrayKeys();
void CScr_GetLocalClientPos();
void CScr_AimingAtFriendly();
void CScr_SplitArgs();
void CScr_sin();
void CScr_cos();
void CScr_tan();
void CScr_asin();
void CScr_acos();
void CScr_atan();
void CScr_abs();
void CScr_min();
void CScr_max();
void CScr_floor();
void CScr_ceil();
void CScr_sqrt();
void CScr_CastInt();
void CScr_CastFloat();
void CScr_GetLocalClientEyePos();
void CScr_GetLocalClientAngles();
void CScr_Spawn();
void CScr_SpawnPlane();
void __cdecl CScr_IsAlive();
void CScr_IsSubStr();
void __cdecl CScr_GetSubStr();
void __cdecl CScr_StrTok();
void __cdecl CScr_GetSpecies(centity_s *cent, const cent_field_s *pField);
void __cdecl CScr_GetIsDog(centity_s *cent, const cent_field_s *pField);
unsigned int __cdecl GetFootstepString(int event);
void __cdecl ReportBadFootsteps(int localClientNum, centity_s *cent, int surfaceType, bool isPlayerView);
void __cdecl CScr_PlayerFootstepEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int event,
                int surfaceType,
                bool quiet);
void __cdecl CScr_PlayerJumpEvent(int localClientNum, centity_s *cent, bool isPlayerView, int surfaceType, bool quiet);
void __cdecl CScr_PlayerLandEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int surfaceType,
                bool quiet,
                bool damagePlayer);
void __cdecl CScr_PlayerFoliageEvent(int localClientNum, centity_s *cent, bool isPlayerView, bool quiet);
void __cdecl CScrCmd_GetThrottle(scr_entref_t entref);
void __cdecl CScrCmd_GetBrake(scr_entref_t entref);
void __cdecl CScrCmd_GetMaxSpeed(scr_entref_t entref);
void __cdecl CScrCmd_GetMaxReverseSpeed(scr_entref_t entref);
void __cdecl CScrCmd_IsDriving(scr_entref_t entref);
void __cdecl CScrCmd_IsLocalClientDriver(scr_entref_t entref);
void __cdecl CScrCmd_GetLocalClientDriver(scr_entref_t entref);
void __cdecl CScrCmd_GetWheelSurface(scr_entref_t entref);
void __cdecl CScrCmd_IsWheelSliding(scr_entref_t entref);
void __cdecl CScrCmd_IsWheelColliding(scr_entref_t entref);
void __cdecl CScrCmd_GetLocalGunnerAngles(scr_entref_t entref);
void __cdecl CScrCmd_SetStunned(scr_entref_t entref);
void CScr_IsInVehicle();
void CScr_PrecacheRumble();
void __cdecl CScrCmd_IsVehiclePeelingOut(scr_entref_t entref);
void __cdecl CScr_line();
void __cdecl CScr_box();
void __cdecl CScr_debugstar();
void CScr_GetMaxLocalClients();
void CScr_LocalClientActive();
void __cdecl CScr_IsSplitScreenHost();
void __cdecl CScr_IsSplitScreen();
void __cdecl CScr_PlayBink();
void __cdecl CScr_StopBink();
void __cdecl CScr_GetBinkLength();
void __cdecl CScr_IsBinkFinished();
void __cdecl CScr_FxParamError(int localClientNum, unsigned int paramIndex, const char *errorString, int fxId);
void __cdecl CScr_SetFxAngles(unsigned int givenAxisCount, float (*axis)[3], float *angles);
unsigned int __cdecl CScr_PlayFXInternal(int localClientNum, int fxId, float *angles, float *pos, int time);
void __cdecl CScr_PlayFX();
void __cdecl CScr_StopFX();
void __cdecl CScr_IsFXPlaying();
void __cdecl CScr_DisableImpactMarks();
void __cdecl CScr_EnableImpactMarks();
void __cdecl CScr_GetEnt();
void __cdecl CScr_GetGenericEnt(int localClientNum, unsigned int offset, const char *pName);
void __cdecl CScrCmd_IsAlive(scr_entref_t entref);
void __cdecl CScr_GetEntNum();
void __cdecl CScr_GetEntArray();
void __cdecl CScr_GetLocalPlayers();
void __cdecl CScr_GetLocalPlayer();
void CScr_BulletTrace();
void CScr_TracePoint();
void __cdecl CScr_OpenFile();
void CScr_CloseFile();
void CScr_FPrintln();
void __cdecl CScr_FPrint_internal(bool commaBetweenFields);
void CScr_FPrintFields();
void CScr_FReadLn();
void CScr_FGetArg();
void CScr_SetUI3DTextureWindow();
void CScr_GetWeaponModel();
void (__cdecl *__cdecl CScr_GetFunction(const char **pName, int *type))();
void __cdecl CScr_Print(scr_entref_t entref);
void __cdecl CScrCmd_Delete(scr_entref_t entref);
void __cdecl CScr_FreeEntity(centity_s *cent, int clientNum);
void __cdecl CScrCmd_ForceDelete(scr_entref_t entref);
void __cdecl CScrCmd_SetModel(scr_entref_t entref);
void __cdecl CScr_Attach(scr_entref_t entref);
unsigned int __cdecl CScr_GetConstServerString(unsigned int index);
void __cdecl CScrCmd_GetSpeed(scr_entref_t entref);
char __cdecl CG_IsLocalPlayer(int entnumber);
void __cdecl CScrCmd_GetEntNum(scr_entref_t entref);
void __cdecl CScrCmd_GetStance(scr_entref_t entref); // LWSS ADD
void __cdecl CScrCmd_GetEntityNumber(scr_entref_t entref);
void __cdecl CScrCmd_Earthquake(scr_entref_t entref);
void __cdecl CScrCmd_MakeLight(scr_entref_t entref);
void __cdecl Byte4PackRgba(const float *from, unsigned __int8 *to);
void __cdecl CScr_GetLightColor(scr_entref_t entref);
void __cdecl CScr_SetLightColor(scr_entref_t entref);
void __cdecl CScr_GetLightIntensity(scr_entref_t entref);
void __cdecl CScr_SetLightIntensity(scr_entref_t entref);
void __cdecl CScr_GetLightRadius(scr_entref_t entref);
void __cdecl CScr_SetLightRadius(scr_entref_t entref);
void __cdecl CScr_GetLightFovInner(scr_entref_t entref);
void __cdecl CScr_GetLightFovOuter(scr_entref_t entref);
void __cdecl CScr_SetLightFovRange(scr_entref_t entref);
void __cdecl CScr_GetLightExponent(scr_entref_t entref);
void __cdecl CScr_SetLightExponent(scr_entref_t entref);
void __cdecl CScrCmdGetCommandTimes(float *pfTotalTime, float *pfAccelTime, float *pfDecelTime);
void __cdecl CScrCmd_MoveGravity(scr_entref_t entref);
void __cdecl CScriptMover_MoveGravity(trajectory_t *pTr, const float *velocity, float fTotalTime, float *vCurrPos);
void __cdecl CScrCmd_MoveTo(scr_entref_t entref);
void __cdecl CScriptMover_Move(
                centity_s *pEnt,
                const float *vPos,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime);
void __cdecl CScriptMover_SetupMove(
                trajectory_t *pTr,
                const float *vPos,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime,
                float *vCurrPos,
                float *pfSpeed,
                float *pfMidTime,
                float *pfDecelTime,
                float *vPos1,
                float *vPos2,
                float *vPos3);
void __cdecl CScrEnt_MoveAxis(scr_entref_t entref, int iAxis);
void __cdecl CScrEntCmd_MoveX(scr_entref_t entref);
void __cdecl CScrEntCmd_MoveY(scr_entref_t entref);
void __cdecl CScrEntCmd_MoveZ(scr_entref_t entref);
void __cdecl CScriptEnt_RotateAxis(scr_entref_t entref, int iAxis);
void __cdecl CScriptMover_Rotate(
                centity_s *cent,
                const float *vRot,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime);
void __cdecl CScrCmd_RotatePitch(scr_entref_t entref);
void __cdecl CScrCmd_RotateYaw(scr_entref_t entref);
void __cdecl CScrCmd_RotateRoll(scr_entref_t entref);
void __cdecl CScrCmd_RotateTo(scr_entref_t entref);
void __cdecl CScrCmd_RotateVelocity(scr_entref_t entref);
void __cdecl CScriptMover_RotateSpeed(
                centity_s *pEnt,
                const float *vRotSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime);
void __cdecl CScriptMover_SetupMoveSpeed(
                trajectory_t *pTr,
                const float *vSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime,
                float *vCurrPos,
                float *pfSpeed,
                float *pfMidTime,
                float *pfDecelTime,
                float *vPos1,
                float *vPos2,
                float *vPos3);
void __cdecl CScr_StartPoisoning(scr_entref_t entref);
void __cdecl CScr_StopPoisoning(scr_entref_t entref);
void __cdecl CScr_IsPoisoned(scr_entref_t entref);
void __cdecl CScrCmd_GetCamPos(scr_entref_t entref);
void __cdecl CScrCmd_GetCamAngles(scr_entref_t entref);
void __cdecl CScr_UseAnimTree(scr_entref_t entref);
XAnimTree_s *__cdecl CScr_GetEntAnimTree(centity_s *ent);
void __cdecl CScr_HasAnimTree(scr_entref_t entref);
void __cdecl CScr_ClearAnim(scr_entref_t entref);
void __cdecl CScr_ClearAnimLimited(scr_entref_t entref);
void __cdecl CScr_SetAnimKnob(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobInternal(scr_entref_t entref, char flags, bool bFlagged);
void __cdecl CScr_HandleAnimError(int error);
void __cdecl CScr_SetAnimKnobLimited(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobRestart(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobLimitedRestart(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobAll(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobAllInternal(scr_entref_t entref, char flags, bool bFlagged);
void __cdecl CScr_SetAnimKnobAllLimited(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobAllRestart(scr_entref_t entref);
void __cdecl CScr_SetAnimKnobAllLimitedRestart(scr_entref_t entref);
void __cdecl CScr_SetAnim(scr_entref_t entref);
void __cdecl CScr_SetAnimInternal(scr_entref_t entref, char flags, bool bFlagged);
void __cdecl CScr_SetAnimLimited(scr_entref_t entref);
void __cdecl CScr_SetAnimRestart(scr_entref_t entref);
void __cdecl CScr_SetAnimLimitedRestart(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnim(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimLimited(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimRestart(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimLimitedRestart(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimKnob(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimKnobLimited(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimKnobRestart(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimKnobLimitedRestart(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimKnobAll(scr_entref_t entref);
void __cdecl CScr_SetFlaggedAnimKnobAllRestart(scr_entref_t entref);
void __cdecl CScr_AnimGetChildAt(scr_entref_t entref);
void __cdecl CScr_AnimGetNumChildren(scr_entref_t entref);
void __cdecl CScr_GetAnimTime(scr_entref_t entref);
void __cdecl CScr_GetAnimCurrFrameCount(scr_entref_t entref);
void __cdecl CScr_SetAnimTime(scr_entref_t entref);
void __cdecl CScrCmd_IsPlayer(scr_entref_t entref);
void __cdecl CScrCmd_IsLocalPlayer(scr_entref_t entref);
void __cdecl CScrCmd_IsAI(scr_entref_t entref);
void __cdecl CScr_HasDObj(scr_entref_t entref);
void __cdecl CScr_SetCompassIcon(scr_entref_t entref);
void __cdecl CScrCmd_IsSplitScreenHost(scr_entref_t entref);
void __cdecl CScrCmd_IsTouching(scr_entref_t entref);
void __cdecl CScrCmd_UnderWater(scr_entref_t entref);
void __cdecl CScrCmd_LinkTo(scr_entref_t entref);
void __cdecl CScrCmd_Unlink(scr_entref_t entref);
void __cdecl CScr_Map_Material(scr_entref_t entref);
void __cdecl CScr_Set_Filter_Pass_Material(scr_entref_t entref);
void __cdecl CScr_Set_Filter_Pass_Enabled(scr_entref_t entref);
void __cdecl CScr_Set_Filter_Pass_Quads(scr_entref_t entref);
void __cdecl CScr_Set_Filter_Pass_Constant(scr_entref_t entref);
void __cdecl CScr_Set_Overlay_Material(scr_entref_t entref);
void __cdecl CScr_Set_Overlay_Constant(scr_entref_t entref);
void __cdecl CScr_SetInfraredVisionset(scr_entref_t entref);
void __cdecl CScr_Set_Overlay_Enabled(scr_entref_t entref);
void __cdecl CScr_LaunchRagdoll(scr_entref_t entref);
void __cdecl CScr_Show(scr_entref_t entref);
void __cdecl CScr_Hide(scr_entref_t entref);
void (__cdecl *__cdecl CScr_GetMethod(const char **pName, int *type))(scr_entref_t);
void __cdecl CScr_LoadLevel();
int __cdecl CScr_SetScriptAndLabel(
                ScriptFunctions *functions,
                const char *filename,
                const char *label,
                int bEnforceExists);
void __cdecl CScr_SetLevelScript(ScriptFunctions *functions);
void __cdecl CG_FreeEntityDelay(centity_s *ed);
void __cdecl CScr_FreeScripts();
unsigned int __cdecl CG_SetEntityScriptVariableInternal(const char *key, char *value);
VariableUnion __cdecl CG_SpawnStruct(const SpawnVar *spawnVar);
void __cdecl CScr_GlassSmash(float *pos, float *dir);
void __cdecl CScr_LoadStructs();
int CG_SpawnNode();
unsigned __int16 __cdecl CScr_ExecEntThread(centity_s *ent, int handle, unsigned int paramcount);
bool __cdecl CG_EntityContact(const float *mins, const float *maxs, const centity_s *cent);
void __cdecl CG_DoTouchTriggers(centity_s *ent, int localClientNum);
void __cdecl CScr_GetEntityField(unsigned int entnum, int clientNum, unsigned int offset);
void __cdecl CScr_GetObjectField(unsigned int classnum, unsigned int entnum, int clientNum, unsigned int offset);
int __cdecl CScr_SetObjectField(unsigned int classnum, unsigned int entnum, int clientNum, unsigned int offset);
int __cdecl CScr_SetEntityField(unsigned int entnum, int clientNum, unsigned int offset);
void __cdecl CScr_AddFieldsForEntity();
void __cdecl CScr_AddFieldsForRadiant();
void __cdecl CScr_PostLoadScripts();


extern cg_level_locals_t cg_level;
extern cscr_data_t cg_scr_data;