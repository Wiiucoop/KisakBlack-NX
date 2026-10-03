#pragma once
#include <bgame/bg_public.h>
#include <sound/snd.h>

#define MAX_MODELS 512

struct DObjAnimMat;
struct DObjTrace_s;

enum entity_event_t : __int32;

void __cdecl G_ClearCachedModels();
int __cdecl G_FindConfigstringIndex(char *name, int start, int max, int create, const char *errormsg);
int __cdecl G_LocalizedStringIndex(char *string);
int __cdecl G_MaterialIndex(const char *name);
int __cdecl G_ModelIndex(char *name);
bool __cdecl G_GetModelBounds(int index, float *outMins, float *outMaxs);
XModel *__cdecl G_GetModel(int index);
bool __cdecl G_XModelBad(int index);
unsigned int __cdecl G_ModelName(unsigned int index);
void __cdecl G_EntityCentroid(const gentity_s *ent, float *centroid);
void __cdecl G_GetEntityBoundsPoint(
                const gentity_s *ent,
                float ratioFromCenterX,
                float ratioFromCenterY,
                float ratioFromCenterZ,
                float *result);
int __cdecl G_TagIndex(char *name);
int __cdecl G_EffectIndex(char *name);
int __cdecl G_ShellShockIndex(char *name);
XAnimTree_s *__cdecl G_GetEntAnimTree(gentity_s *ent);
void __cdecl G_UpdateVehicleAttachedModels(gentity_s *ent);
void __cdecl G_UpdateScriptMoverAttachedModels(gentity_s *ent);
void __cdecl G_DObjUpdate(gentity_s *ent);
void __cdecl G_SetModel(gentity_s *ent, char *modelName);
void __cdecl G_SetModelIfLoaded(gentity_s *ent, const char *modelName);
void __cdecl G_OverrideModel(unsigned int modelIndex, char *defaultModelName);
int __cdecl G_EntAttach(gentity_s *ent, char *modelName, unsigned int tagName, int ignoreCollision);
int __cdecl G_EntDetach(gentity_s *ent, const char *modelName, unsigned int tagName);
void __cdecl G_EntDetachAll(gentity_s *ent);
int __cdecl G_EntLinkTo(gentity_s *ent, gentity_s *parent, unsigned int tagName);
int __cdecl G_EntLinkToInternal(gentity_s *ent, gentity_s *parent, unsigned int tagName);
int __cdecl G_EntLinkToWithOffset(
                gentity_s *ent,
                gentity_s *parent,
                unsigned int tagName,
                const float *originOffset,
                const float *anglesOffset);
void __cdecl G_EntUnlink(gentity_s *ent);
bool __cdecl G_EntIsLinkedTo(gentity_s *ent, gentity_s *parent);
void __cdecl G_UpdateTagInfo(gentity_s *ent, int bParentHasDObj);
void __cdecl G_UpdateTags(gentity_s *ent, int bHasDObj);
void __cdecl G_UpdateTagInfoOfChildren(gentity_s *parent, int bHasDObj);
void __cdecl G_CalcTagParentAxis(gentity_s *ent, float (*parentAxis)[3]);
void __cdecl G_CalcTagAxis(gentity_s *ent, int bAnglesOnly);
void __cdecl G_SetFixedLink(gentity_s *ent, int eAngles);
void __cdecl G_UpdateViewAngleClamp(gclient_s *client, const float *worldAnglesCenter);
void __cdecl G_SetPlayerFixedLink(gentity_s *ent);
void __cdecl G_GeneralLink(gentity_s *ent);
void __cdecl G_SafeDObjFree(gentity_s *ent);
void __cdecl G_DObjUpdateServerTime(gentity_s *ent, int bNotify, void (__cdecl *CallbackFunc)());
void __cdecl G_DObjCalcPose(gentity_s *ent, int *partBits);
void __cdecl G_DObjCalcBone(const gentity_s *ent, int boneIndex);
DObjAnimMat *__cdecl G_DObjGetLocalTagMatrix(const gentity_s *ent, unsigned int tagName);
int __cdecl G_DObjGetWorldTagMatrix(const gentity_s *ent, unsigned int tagName, float (*tagMat)[3]);
int __cdecl G_DObjGetWorldTagPos(const gentity_s *ent, unsigned int tagName, float *pos);
void __cdecl G_TraceBulletPathForVehTurret(gentity_s *ent, DObjTrace_s *trace, int gunnerIndex);
DObjAnimMat *__cdecl G_DObjGetLocalBoneIndexMatrix(const gentity_s *ent, int boneIndex);
void __cdecl G_DObjGetWorldBoneIndexPos(const gentity_s *ent, int boneIndex, float *pos);
void __cdecl G_DObjGetWorldBoneIndexMatrix(const gentity_s *ent, int boneIndex, float (*tagMat)[3]);
gentity_s *__cdecl G_Find(gentity_s *from, int fieldofs, unsigned __int16 match);
// G_Find's field offset counts from s.number. Callers passed 356, the x86 offset of
// classname; under LP64 that read another field and never found the player
// (ChangeLevel from the frontend menu did nothing).
#define G_FIND_CLASSNAME ((int)(offsetof(gentity_s, classname) - offsetof(gentity_s, s.number)))
void __cdecl G_InitGentity(gentity_s *e);
void __cdecl G_PrintEntities();
gentity_s *__cdecl G_Spawn();
bool __cdecl G_MaySpawnEntity(gentity_s *e);
gentity_s *__cdecl G_SpawnPlayerClone();
gentity_s *__cdecl G_SpawnActorClone();
void __cdecl G_ClearGroundEntity(gentity_s *ent);
void __cdecl G_ClearGroundEntityRefs(gentity_s *ed);
void __cdecl G_FreeEntityRefs(gentity_s *ed);
void __cdecl G_FreeEntity(gentity_s *ed);
void __cdecl G_FreeEntityDelay(gentity_s *ed);
void __cdecl G_BroadcastEntity(gentity_s *ent);
void __cdecl G_FreeEntityAfterEvent(gentity_s *ent);
gentity_s *__cdecl G_TempEntity(const float *origin, entity_event_t event);
void __cdecl G_AddPredictableEvent(gentity_s *ent, unsigned int event, unsigned int eventParm);
void __cdecl G_AddEvent(gentity_s *ent, unsigned int event, unsigned int eventParm);
gentity_s *__cdecl G_PlaySoundAliasAtPoint(const float *origin, unsigned int alias);
gentity_s *__cdecl G_PlaySoundAlias(gentity_s *ent, unsigned int alias, unsigned int notifyString, unsigned int bone);
#ifdef KISAK_SP
void __cdecl G_SPResetSoundNotifyState();
void __cdecl G_SPShutdownSoundNotifyState();
void __cdecl G_SPSetSoundLength(int entNum, int lengthMs);
void __cdecl G_SPUpdateSoundNotify(gentity_s *ent);
bool __cdecl G_SPIsWaitingOnSound(const gentity_s *ent);
#endif
void __cdecl G_AnimScriptSound(int client, snd_alias_list_t *aliasList);
void __cdecl G_SetOrigin(gentity_s *ent, const float *origin);
void __cdecl G_SetAngle(gentity_s *ent, const float *angle);
void __cdecl G_SetConstString(unsigned __int16 *to, char *from);
const char *__cdecl G_GetEntityTypeName(const gentity_s *ent);
void __cdecl G_srand(unsigned int seed);
int __cdecl G_rand();
double __cdecl G_flrand(float min, float max);
int __cdecl G_irand(int min, int max);
float __cdecl G_random();
double __cdecl G_crandom();
