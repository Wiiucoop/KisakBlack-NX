#pragma once

struct DObj;
struct centity_s;

#ifdef KISAK_SP
struct msg_t;
struct XAnim_s;
namespace SpAnimSnapshot { struct Entity; }
void SV_WriteAnimSnapshot_SP(msg_t *msg, int serverTime, int firstEntity, int numEntities);
void CL_ReadAnimSnapshot_SP(msg_t *msg, int serverTime, int messageNum);
void CG_ResetRemoteAnimSnapshots_SP();
void CG_CaptureRemoteAnimTrees_SP();
void CG_ClearRemoteAnimTrees_SP();
const SpAnimSnapshot::Entity *CG_GetRemoteAnimEntity_SP(int localClientNum, int entNum);
XAnim_s *CG_GetRemoteAnimations_SP(int localClientNum, int entNum);
bool CG_ApplyRemoteAnimSnapshot_SP(int localClientNum, int entNum, DObj *obj, bool force);
// Retail SP's separately snapshotted 44-byte animation command. Keep this
// layout exact even though the reconstruction currently transports it through
// an in-process ring between its integrated server and local client.
struct AnimCommand_SP
{
    int index;
    int type;
    int serverTime;
    int sequence;
    int entNum;
    int animIndex;
    int rootAnimIndex;
    float weight;
    float rate;
    float goalTime;
    int flags;
};
static_assert(sizeof(AnimCommand_SP) == 0x2C, "retail SP animation command layout changed");

int __cdecl CG_StoreServerAnimCommand_SP(
    int entNum,
    int serverTime,
    int type,
    unsigned int animIndex,
    unsigned int rootAnimIndex,
    float weight,
    float goalTime,
    float rate,
    int flags);
void __cdecl CG_ApplyPendingAnimCommands_SP(int localClientNum);
void __cdecl CG_ApplyPendingAnimCommandsForDObj_SP(int localClientNum, int entNum, DObj *obj);
// The entity was freed: its stored commands belong to it, not to whatever
// takes the number next (see the definition).
void __cdecl CG_ForgetAnimCommandsForEnt_SP(int entNum);
#endif

void __cdecl CG_GetTagMatrix(int localClientNum, int linkEntNum, unsigned __int16 tagName, float (*resultTagMat)[3]);
DObj *__cdecl GetLinkEntDObj(int localClientNum, centity_s *centLink);
void __cdecl CG_CalcTagParentAxis(int localClientNum, centity_s *cent, float (*parentAxis)[3]);
void __cdecl CG_LinkTransformForEntity(int localClientNum, centity_s *cent, float *resultOrigin, float *resultAngles);
void __cdecl CG_GenerateLinkInfo(int localClientNum, centity_s *cent, int attachedEntNum, int attachedTagIndex);
void __cdecl CG_UpdateEntityLink(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent, int parentNum, int tagIndex);
void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent);
bool __cdecl CG_EntityLinked(int localClientNum, centity_s *cent);
centity_s *__cdecl CG_EntGetLinkToParent(int localClientNum, centity_s *cent);
