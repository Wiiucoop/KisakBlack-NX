#pragma once
#include <clientscript/cscr_main.h>
#include "dobj.h"

#include <d3d9.h>
#include <flame/flame_system.h>
#include <universal/com_pack.h>
#include <gfx_d3d/r_gfx.h>

struct XAnimParam // sizeof=0x14
{
        char strName[16];
        float fValue;
};

union XAnimIndices // sizeof=0x4
{                                                                             // XREF: XAnimLoadFile(char const *,void * (*)(int),bool)+1808/w
                                                                                // XAnimLoadFile(char const *,void * (*)(int),bool)+1840/w ...
        unsigned __int8 *_1;
        unsigned __int16 *_2;
        void *data;
};

struct XAnimNotifyInfo // sizeof=0x8
{
        unsigned __int16 name;
        // padding byte
        // padding byte
        float time;
};

union XAnimDynamicFrames // sizeof=0x4
{                                                                             // XREF: XAnimCalcNonLoopEnd+9ED/w
                                                                                // XAnimCalcNonLoopEnd+9F3/r ...
        unsigned __int8 (*_1)[3];
        unsigned __int16 (*_2)[3];
};

union XAnimDynamicIndices // sizeof=0x2
{                                                                             // XREF: XAnimPartTransFrames/r
                                                                                // XAnimDeltaPartQuatDataFrames/r ...
        unsigned __int8 _1[1];
        unsigned __int16 _2[1];
};

struct __declspec(align(4)) XAnimPartTransFrames // sizeof=0x20
{                                                                             // XREF: XAnimPartTransData/r
        float mins[3];
        float size[3];
        XAnimDynamicFrames frames;
        XAnimDynamicIndices indices;
        // padding byte
        // padding byte
};

union XAnimPartTransData // sizeof=0x20
{                                                                             // XREF: XAnimPartTrans/r
        XAnimPartTransFrames frames;
        float frame0[3];
};

struct XAnimPartTrans // sizeof=0x24
{
        unsigned __int16 size;
        unsigned __int8 smallTrans;
        // padding byte
        XAnimPartTransData u;
};

struct __declspec(align(4)) XAnimDeltaPartQuatDataFrames // sizeof=0x8
{                                                                             // XREF: XAnimDeltaPartQuatData/r
        __int16 (*frames)[2];
        XAnimDynamicIndices indices;
        // padding byte
        // padding byte
};

union XAnimDeltaPartQuatData // sizeof=0x8
{                                                                             // XREF: XAnimDeltaPartQuat/r
        XAnimDeltaPartQuatDataFrames frames;
        __int16 frame0[2];
};

struct XAnimDeltaPartQuat // sizeof=0xC
{
        unsigned __int16 size;
        // padding byte
        // padding byte
        XAnimDeltaPartQuatData u;
};

struct XAnimDeltaPart // sizeof=0x8
{
        XAnimPartTrans *trans;
        XAnimDeltaPartQuat *quat;
};

struct XAnimParts // sizeof=0x68
{                                                                             // XREF: XAssetPoolEntry<XAnimParts>/r
        const char *name;
        unsigned __int16 dataByteCount;
        unsigned __int16 dataShortCount;
        unsigned __int16 dataIntCount;
        unsigned __int16 randomDataByteCount;
        unsigned __int16 randomDataIntCount;
        unsigned __int16 numframes;
        bool bLoop;
        bool bDelta;
        bool bLeftHandGripIK;
        bool bStreamable;
        unsigned int streamedFileSize;
        unsigned __int8 boneCount[10];
        unsigned __int8 notifyCount;
        unsigned __int8 assetType;
        bool isDefault;
        // padding byte
        // padding byte
        // padding byte
        unsigned int randomDataShortCount;
        unsigned int indexCount;
        float framerate;
        float frequency;
        float primedLength;
        float loopEntryTime;
        unsigned __int16 *names;
        unsigned __int8 *dataByte;
        __int16 *dataShort;
        int *dataInt;
        __int16 *randomDataShort;
        unsigned __int8 *randomDataByte;
        int *randomDataInt;
        XAnimIndices indices;
        XAnimNotifyInfo *notify;
        XAnimDeltaPart *deltaPart;
};

struct XAnimParent // sizeof=0x4
{                                                                             // XREF: $AE2C40205408E7677B548FCC327D21B5/r
        unsigned __int16 flags;
        unsigned __int16 children;
};

struct XAnimEntry // sizeof=0x10
{                                                                             // XREF: XAnim_s/r
        unsigned __int16 bCreated;
        unsigned __int16 numAnims;
        unsigned __int16 firstParamIxPlusOne;
        unsigned __int16 paramCount;
        unsigned __int16 parent;
        // padding byte
        // padding byte
        //$AE2C40205408E7677B548FCC327D21B5 ___u5;
        union //$AE2C40205408E7677B548FCC327D21B5 // sizeof=0x4
        {                                                                             // XREF: XAnimClearTreeGoalWeightsInternal+42/r
                                                                                        // XAnimClearTreeGoalWeightsInternal+5F/r ...
            XAnimParts *parts;
            XAnimParent animParent;
        };
};

struct XAnim_s // sizeof=0x28
{                                                                             // XREF: XAnim/r
        const char *debugName;
        unsigned int size;
        unsigned int paramSize;
        XAnimParam *params;
        const char **debugAnimNames;
        bool *wasLoggedIfMissing;
        XAnimEntry entries[1];
};

struct XAnimTree_s // sizeof=0x8
{                                                                             // XREF: XAnimTree/r
        XAnim_s *anims;
        unsigned __int16 children;
        __int16 inst;
};

struct XSurfaceVertexInfo // sizeof=0x10
{                                                                             // XREF: XSurface/r
        __int16 vertCount[4];
        unsigned __int16 *vertsBlend;
        float *tensionData;
};

struct XSurfaceCollisionAabb // sizeof=0xC
{                                                                             // XREF: XSurfaceCollisionNode/r
        unsigned __int16 mins[3];
        unsigned __int16 maxs[3];
};

struct XSurfaceCollisionNode // sizeof=0x10
{
        XSurfaceCollisionAabb aabb;
        unsigned __int16 childBeginIndex;
        unsigned __int16 childCount;
};

struct XSurfaceCollisionLeaf // sizeof=0x2
{
        unsigned __int16 triangleBeginIndex;
};

struct XSurfaceCollisionTree // sizeof=0x28
{
        float trans[3];
        float scale[3];
        unsigned int nodeCount;
        XSurfaceCollisionNode *nodes;
        unsigned int leafCount;
        XSurfaceCollisionLeaf *leafs;
};

struct XRigidVertList // sizeof=0xC
{                                                                             // XREF: ?XModelReadSurface@@YAXPAUXModel@@PAPBEP6APAXH@ZPAUXSurface@@HPBD@Z/r
        unsigned __int16 boneOffset;                // XREF: XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+28A/r
                                                                                // XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+A3A/r
        unsigned __int16 vertCount;                 // XREF: XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+FDB/r
        unsigned __int16 triOffset;                 // XREF: XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+F8B/w
                                                                                // XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+F9C/r
        unsigned __int16 triCount;                    // XREF: XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+1027/w
                                                                                // XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+1038/r
        XSurfaceCollisionTree *collisionTree;
};

struct GfxPackedVertex // sizeof=0x20
{                                                                             // XREF: GfxPackedVertex0/r
                                                                                // GfxPackedVertex1/r ...
        float xyz[3];
        float binormalSign;
        GfxColor color;                                         // XREF: R_GenerateQuadStampCodeMeshVertsArray(Material *,renderQuad_t *,int,float (* const)[3])+175/o
                                                                                // R_GenerateQuadStampCodeMeshVertsArray(Material *,renderQuad_t *,int,float (* const)[3])+41C/o
        PackedTexCoords texCoord;
        PackedUnitVec normal;                             // XREF: R_GenerateQuadStampCodeMeshVertsArray(Material *,renderQuad_t *,int,float (* const)[3])+2C3/w
        PackedUnitVec tangent;                            // XREF: R_GenerateQuadStampCodeMeshVertsArray(Material *,renderQuad_t *,int,float (* const)[3])+419/w
};

//https://github.com/Laupetin/OpenAssetTools/blob/a74b04259d00ed3823b9cfe00436f96549535c33/src/Common/Game/T5/T5_Assets.h#L548
enum XSurfaceFlag
{
    XSURFACE_FLAG_QUANTIZED = 0x1,
    XSURFACE_FLAG_SKINNED = 0x2,
    XSURFACE_FLAG_CONSTANT_COLOR = 0x4,
    XSURFACE_FLAG_DEFORMED = 0x80,
    XSURFACE_FLAG_STREAMED = 0x8000,
};

struct XSurface // sizeof=0x44
{
        unsigned __int8 tileMode;
        unsigned __int8 vertListCount;
        unsigned __int16 flags;
        unsigned __int16 vertCount;
        unsigned __int16 triCount;
        unsigned __int16 baseTriIndex;
        unsigned __int16 baseVertIndex;
        unsigned __int16 *triIndices;
        XSurfaceVertexInfo vertInfo;
        GfxPackedVertex *verts0;
        IDirect3DVertexBuffer9 *vb0;
        XRigidVertList *vertList;
        IDirect3DIndexBuffer9 *indexBuffer;
        int partBits[5];
};

struct Collmap // sizeof=0x4
{
        PhysGeomList *geomList;
};

struct XAnimSimpleRotPos // sizeof=0x18
{                                                                             // XREF: ?XAnimCalcDelta@@YAXPAUDObj@@IQAM1_N@Z/r
        float rot[2];                                             // XREF: XAnimCalcDelta(DObj *,uint,float * const,float * const,bool)+13B/r
        float posWeight;                                        // XREF: XAnimGetRelDelta(XAnim_s const *,uint,float * const,float * const,float,float)+97/w
        float pos[3];                                             // XREF: XAnimCalcDelta(DObj *,uint,float * const,float * const,bool)+19C/r
};

struct __declspec(align(4)) XAnimState // sizeof=0x20
{                                                                             // XREF: XAnimInfo/r
                                                                                // ?DObjInitServerTime@@YAXPAUDObj@@M@Z/r ...
        float currentAnimTime;                            // XREF: XAnimInit(void)+6A/w
                                                                                // XAnimGetTime(XAnimTree_s const *,uint)+ED/r ...
        float oldTime;                                            // XREF: XAnimInit(void)+70/w
        __int16 cycleCount;                                 // XREF: XAnimInit(void)+76/w
                                                                                // DObjInitServerTime(DObj *,float)+5C/w ...
        __int16 oldCycleCount;
        float goalTime;                                         // XREF: XAnimInit(void)+7C/w
        float goalWeight;                                     // XREF: XAnimInit(void)+82/w
                                                                                // XAnimSetCompleteGoalWeightNode+9A/r ...
        float weight;                                             // XREF: XAnimInit(void)+88/w
                                                                                // XAnimGetWeight(XAnimTree_s const *,uint)+ED/r
        float rate;                                                 // XREF: XAnimInit(void)+8E/w
                                                                                // XAnimSetAnimRate(XAnimTree_s *,uint,float)+12B/w
        bool instantWeightChange;                     // XREF: XAnimInit(void)+94/w
        // padding byte
        // padding byte
        // padding byte
};

struct __declspec(align(4)) XAnimInfo // sizeof=0x40
{                                                                             // XREF: .data:g_xAnimInfo/r
                                                                                // XAnimInfoArray/r
        unsigned __int16 notifyChild;
        __int16 notifyIndex;
        unsigned __int16 notifyName;
        unsigned __int16 notifyType;
        unsigned __int16 prev;                            // XREF: XAnimInit(void)+3E/w
                                                                                // XAnimFreeInfo+269/w ...
        unsigned __int16 next;                            // XREF: XAnimInit(void)+5F/w
                                                                                // XAnimGetInfoIndex_r+7A/r ...
        unsigned __int16 children;                    // XREF: XAnimResetTime+16/r
                                                                                // XAnimCloneInitTime+DC/r ...
        unsigned __int16 parent;                        // XREF: XAnimClearTreeGoalWeightsInternal+55/r
                                                                                // XAnimClearGoalWeightKnobInternal+9F/r ...
        unsigned __int16 animIndex;                 // XREF: XAnimGetInfoIndex_r+BC/r
                                                                                // XAnimClearTreeGoalWeightsInternal+F2/r ...
        unsigned __int16 animToModel;             // XREF: XAnimClearTreeGoalWeightsInternal+31/r
                                                                                // XAnimClearGoalWeightKnobInternal+1D8/r
        union //$AE2C40205408E7677B548FCC327D21B5 // sizeof=0x4
        {                                       // XREF: XAnimClearTreeGoalWeightsInternal+42/r
            XAnimParts *parts;
            XAnimParent animParent;
        };
                                                                                // XREF: XAnimClearTreeGoalWeightsInternal+42/r
                                                                                // XAnimClearTreeGoalWeightsInternal+5F/r ...
        XAnimState state;                                     // XREF: XAnimInit(void)+6A/w
                                                                                // XAnimInit(void)+70/w ...
        XAnimParts *streamedParts;
        unsigned __int16 streamedAnimToModel;
        // padding byte
        // padding byte
};

struct XAnimServerNotify // sizeof=0xC
{                                                                             // XREF: ?G_DObjUpdateServerTime@@YAXPAUgentity_s@@HP6AXXZ@Z/r
        unsigned __int16 entNum;                        // XREF: G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+DE/r
        unsigned __int16 notifyName;                // XREF: G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+CA/r
        unsigned __int16 notetrackName;         // XREF: G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+AE/r
        // padding byte
        // padding byte
        float fPercentTime;
};

struct XAnimServerNotifyList // sizeof=0xC
{                                                                             // XREF: ?G_DObjUpdateServerTime@@YAXPAUgentity_s@@HP6AXXZ@Z/r
        XAnimServerNotify *pList;                     // XREF: G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+41/w
        int iNumElements;                                     // XREF: G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+4B/w
        int iCurrentElement;                                // XREF: G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+44/w
                                                                                // G_DObjUpdateServerTime(gentity_s *,int,void (*)(void))+9E/r
};

struct XAnimDeltaInfo // sizeof=0x4
{                                                                             // XREF: ?XAnimCalcDelta@@YAXPAUDObj@@IQAM1_N@Z/r
                                                                                // XAnimCalcDeltaTree/r ...
        bool bClear;                                                // XREF: XAnimCalcDelta(DObj *,uint,float * const,float * const,bool):loc_7F186B/w
                                                                                // XAnimCalcDelta(DObj *,uint,float * const,float * const,bool)+121/r ...
        bool bNormQuat;                                         // XREF: XAnimCalcDelta(DObj *,uint,float * const,float * const,bool)+10F/w
                                                                                // XAnimCalcDeltaTree+3A1/w ...
        bool bAbs;                                                    // XREF: XAnimCalcDelta(DObj *,uint,float * const,float * const,bool)+113/w
                                                                                // XAnimCalcDeltaTree+125/r ...
        bool bUseGoalWeight;                                // XREF: XAnimCalcDelta(DObj *,uint,float * const,float * const,bool)+11A/w
                                                                                // XAnimCalcDeltaTree+1F1/r ...
};

struct XAnimTime // sizeof=0xC
{                                                                             // XREF: XAnimCalcParts_unsigned_char_/r
        float time;
        float frameFrac;                                        // XREF: XAnimCalcParts_unsigned_char_+29E/w
        int frameIndex;                                         // XREF: XAnimCalcParts_unsigned_char_+28E/r
};

struct __declspec(align(4)) XAnimToXModel // sizeof=0xB8
{                                       // XREF: ?XAnimGetAnimMap@@YAIPBUXAnimParts@@PBUXModelNameMap@@@Z/r
    bitarray<160> partBits;             // XREF: XAnimGetAnimMap(XAnimParts const *,XModelNameMap const *)+3D/w
                                        // XAnimGetAnimMap(XAnimParts const *,XModelNameMap const *)+78/w ...
    unsigned __int8 boneCount;          // XREF: XAnimGetAnimMap(XAnimParts const *,XModelNameMap const *)+192/w
                                        // XAnimGetAnimMap(XAnimParts const *,XModelNameMap const *)+198/r
    unsigned __int8 boneIndex[160];     // XREF: XAnimGetAnimMap(XAnimParts const *,XModelNameMap const *)+130/w
                                        // XAnimGetAnimMap(XAnimParts const *,XModelNameMap const *)+167/w
    // padding byte
    // padding byte
    // padding byte
};

struct XModelNameMap;

int __cdecl XAnimTreeSize();
bool __cdecl XAnimTreeHasInfo(const XAnimTree_s *tree);
void __cdecl XAnimInit();
void __cdecl XAnimShutdown();
void __cdecl XAnimFree(XAnimParts *parts, scriptInstance_t inst);
void *__cdecl Hunk_AllocXAnimPrecache(unsigned int size);
XAnimParts *__cdecl XAnimPrecache(char *name, void *(__cdecl *Alloc)(int));
XAnimParts *__cdecl XAnimFindData_LoadObj(char *name, void *(__cdecl *Alloc)(int));
XAnimParts *__cdecl XAnimFindData_FastFile(const char *name);
XAnimParts *__cdecl XAnimClone(XAnimParts *fromParts, void *(__cdecl *Alloc)(int));
void __cdecl XAnimCreate(XAnim_s *anims, unsigned int animIndex, char *name);
void __cdecl XAnimBlend(
                XAnim_s *anims,
                unsigned int animIndex,
                const char *name,
                unsigned int children,
                unsigned int num,
                unsigned int flags);
bool __cdecl IsNodeAdditive(const XAnimEntry *node);
bool __cdecl IsLeafNode(const XAnimEntry *anim);
void __cdecl XAnimSetParamValue(XAnim_s *anims, unsigned int paramIndex, char *strParamName, float paramValue);
void __cdecl XAnimSetValueSection(
                XAnim_s *anims,
                unsigned int animIndex,
                __int16 iFirstValueIndex,
                unsigned __int16 iValueCount);
XAnim_s *__cdecl XAnimCreateAnims(const char *debugName, int size, void *(__cdecl *Alloc)(unsigned int));
XAnim_s *__cdecl XAnimCreateAnimsWithValues(
                const char *debugName,
                int size,
                unsigned int iTotalValueCount,
                void *(__cdecl *Alloc)(int));
void __cdecl XAnimFreeList(XAnim_s *anims);
XAnimTree_s *__cdecl XAnimCreateTree(XAnim_s *anims, void *(__cdecl *Alloc)(unsigned int));
void __cdecl XAnimFreeTree(
                XAnimTree_s *tree,
                void (__cdecl *Free)(void *, int, scriptInstance_t),
                scriptInstance_t inst);
XAnim_s *__cdecl XAnimGetAnims(const XAnimTree_s *tree);
void __cdecl XAnimInitModelMap(XModel *const *models, unsigned int numModels, XModelNameMap *modelMap);
unsigned int __cdecl XAnimGetAnimMap(const XAnimParts *parts, const XModelNameMap *modelMap);
void __cdecl XAnimCalcRelDeltaParts(
                const XAnimParts *parts,
                float weightScale,
                float time1,
                float time2,
                XAnimSimpleRotPos *rotPos,
                int quatIndex);
void __cdecl TransformToQuatRefFrame(const float *rot, float *trans);
void __cdecl XAnimCalcAbsDeltaParts(const XAnimParts *parts, float weightScale, float time, XAnimSimpleRotPos *rotPos);
void __cdecl XAnimAddClientNotify(unsigned int notetrackName, float frac, unsigned int notifyType);
void __cdecl AddSignalToNoteList(struct XAnimClientNotifyList *pList, const struct ClientNotifyData *ourNote);
void __cdecl XAnimAddClientNotifyNamed(unsigned int notetrackName, float frac, unsigned int notifyName);
void    XAnimResetAnimMap(const DObj *obj, unsigned int infoIndex);
void __cdecl XAnimResetAnimMap_r(XModelNameMap *modelMap, unsigned int infoIndex);
void __cdecl XAnimResetAnimMapLeaf(const XModelNameMap *modelMap, unsigned int infoIndex);
double __cdecl XAnimGetLength(const XAnim_s *anims, unsigned int animIndex);
int __cdecl XAnimGetFrameCount(const XAnim_s *anims, unsigned int animIndex);
int __cdecl XAnimGetLengthMsec(const XAnim_s *anims, unsigned int anim);
unsigned int __cdecl XAnimGetInfoIndex(const XAnimTree_s *tree, unsigned int animIndex);
unsigned int __cdecl XAnimGetInfoIndex_r(unsigned int animIndex, unsigned int infoIndex);
double __cdecl XAnimGetTime(const XAnimTree_s *tree, unsigned int animIndex);
double __cdecl XAnimGetWeight(const XAnimTree_s *tree, unsigned int animIndex);
bool __cdecl XAnimHasFinished(const XAnimTree_s *tree, unsigned int animIndex);
int __cdecl XAnimGetNumChildren(const XAnim_s *anims, unsigned int animIndex);
unsigned int __cdecl XAnimGetChildAt(const XAnim_s *anims, unsigned int animIndex, unsigned int childIndex);
const char *__cdecl XAnimGetAnimName(const XAnim_s *anims, unsigned int animIndex);
char *__cdecl XAnimGetAnimDebugName(const XAnim_s *anims, unsigned int animIndex);
unsigned int __cdecl XAnimGetAnimTreeSize(const XAnim_s *anims);
void __cdecl XAnimInitInfo(XAnimInfo *info);
void __cdecl XAnimUpdateOldTime(
                DObj *obj,
                unsigned int infoIndex,
                XAnimState *syncState,
                float dtime,
                bool parentHasWeight,
                bool *childHasTimeForParent);
unsigned int __cdecl XAnimInitTime(XAnimTree_s *tree, unsigned int infoIndex, float goalTime, int cmdIndex);
void __cdecl XAnimResetTime(unsigned int infoIndex);
void __cdecl XAnimResetTimeInternal(unsigned int infoIndex);
unsigned int __cdecl XAnimCloneInitTime(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                unsigned int parentIndex,
                int cmdIndex);
void __cdecl DObjInitServerTime(DObj *obj, float dtime);
void __cdecl DObjUpdateClientInfo(DObj *obj, float dtime, int notifyFlags);
void __cdecl XAnimUpdateTimeAndNotetrack(const DObj *obj, unsigned int infoIndex, float dtime, int notifyFlags);
void __cdecl XAnimCheckFreeInfo(XAnimTree_s *tree, unsigned int infoIndex, int hasWeight);
void __cdecl XAnimFreeInfo(XAnimTree_s *tree, unsigned int infoIndex);
void __cdecl XAnimClearServerNotify(XAnimInfo *info, scriptInstance_t inst);
double __cdecl XAnimGetAverageRateFrequency(const XAnimTree_s *tree, unsigned int infoIndex);
void __cdecl XAnimUpdateTimeAndNotetrackLeaf(
                const DObj *obj,
                const XAnimParts *parts,
                unsigned int infoIndex,
                float dtime,
                char notifyFlags);
void __cdecl XAnimProcessClientNotify(XAnimInfo *info, float dtime);
#ifdef KISAK_SP
void __cdecl XAnimApplyClientCommandCatchup_SP(DObj *obj, unsigned int animIndex, int elapsedMs);
#endif
signed int __cdecl XAnimGetNextNotifyIndex(const XAnimParts *parts, float time);
double __cdecl XAnimGetNotifyFracLeaf(const XAnimState *state, const XAnimState *nextState, float time, float dtime);
void __cdecl XAnimUpdateTimeAndNotetrackSyncSubTree(
                const DObj *obj,
                unsigned int infoIndex,
                float dtime,
                int notifyFlags);
void __cdecl XAnimUpdateInfoSync(
                const DObj *obj,
                unsigned int infoIndex,
                int notifyFlags,
                XAnimState *syncState,
                float dtime);
void __cdecl XAnimProcessServerNotify(const DObj *obj, XAnimInfo *info, float time);
XAnimParts *__cdecl XAnimGetPartsForNotify(const XAnimTree_s *tree, XAnimInfo *info);
void __cdecl NotifyServerNotetrack(
                const DObj *obj,
                unsigned int notifyName,
                unsigned int notetrackName,
                float fPercentage);
void __cdecl DObjClearServerNotifies();
void __cdecl DObjSetServerNotifies(XAnimServerNotifyList *pNotifyList);
void __cdecl DObjClearClientNotifies();
void __cdecl DObjSetClientNotifies(XAnimClientNotifyList *pNotifyList);
const XAnimClientNotifyList *__cdecl DObjGetClientNotifies();
void __cdecl DObjUpdateServerInfo(DObj *obj, float dtime, int notifyFlags);
void __cdecl DObjDisplayAnimToBuffer(const DObj *obj, const char *header, char *buffer, int bufferSize);
void __cdecl XAnimDisplay(
                const XAnimTree_s *tree,
                unsigned int infoIndex,
                int depth,
                char *buffer,
                int bufferSize,
                int *bufferPos);
void __cdecl DObjDisplayAnim(const DObj *obj, const char *header);
void __cdecl XAnimCalcDelta(DObj *obj, unsigned int animIndex, float *rot, float *trans, bool bUseGoalWeight);
void __cdecl XAnimCalcDeltaTree(
                const DObj *obj,
                unsigned int infoIndex,
                float weightScale,
                XAnimDeltaInfo deltaInfo,
                XAnimSimpleRotPos *rotPos);
void __cdecl XAnimCalcAbsDelta(DObj *obj, unsigned int animIndex, float *rot, float *trans);
void __cdecl XAnimGetRelDelta(
                const XAnim_s *anims,
                unsigned int animIndex,
                float *rot,
                float *trans,
                float time1,
                float time2);
void __cdecl XAnimGetAbsDelta(const XAnim_s *anims, unsigned int animIndex, float *rot, float *trans, float time);
unsigned int __cdecl XAnimAllocInfoWithParent(
                XAnimTree_s *tree,
                unsigned __int16 animToModel,
                unsigned int animIndex,
                unsigned int parentInfoIndex,
                int after);
unsigned int    XAnimAllocInfoIndex(DObj *obj,
                unsigned int animIndex,
                int after,
                int cmdIndex);
unsigned int __cdecl XAnimEnsureGoalWeightParent(DObj *obj, unsigned int animIndex, int cmdIndex);
void __cdecl XAnimClearGoalWeight(
                XAnimTree_s *tree,
                unsigned int animIndex,
                float blendTime,
                unsigned __int16 cmdIndex);
void __cdecl XAnimClearGoalWeightInternal(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float blendTime,
                int forceBlendTime,
                unsigned __int16 cmdIndex);
void __cdecl XAnimClearTreeGoalWeightsInternal(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float blendTime,
                int forceBlendTime,
                int cmdIndex);
void __cdecl XAnimClearTreeGoalWeights(XAnimTree_s *tree, unsigned int animIndex, float blendTime, int cmdIndex);
void __cdecl XAnimClearTreeGoalWeightsStrict(XAnimTree_s *tree, unsigned int animIndex, float blendTime, int cmdIndex);
int __cdecl XAnimSetCompleteGoalWeightKnob(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex);
void __cdecl XAnimClearGoalWeightKnobInternal(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float goalWeight,
                float goalTime,
                unsigned __int16 cmdIndex);
int __cdecl XAnimSetCompleteGoalWeightNode(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                unsigned __int16 cmdIndex);
int __cdecl XAnimSetGoalWeightKnobAll(
                DObj *obj,
                unsigned int animIndex,
                unsigned int rootIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex);
int __cdecl XAnimSetCompleteGoalWeightKnobAll(
                DObj *obj,
                unsigned int animIndex,
                unsigned int rootIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex);
int __cdecl XAnimSetGoalWeightKnob(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex);
void __cdecl XAnimClearTree(XAnimTree_s *tree);
int __cdecl XAnimSetGoalWeightNode(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                unsigned __int16 cmdIndex);
unsigned int __cdecl XAnimGetDescendantWithGreatestWeight(const XAnimTree_s *tree, unsigned int infoIndex);
void __cdecl    XAnimSetupSyncNodes(XAnim_s *anims);
void __cdecl XAnimSetupSyncNodes_r(XAnim_s *anims, unsigned int animIndex, int parFlags);
bool __cdecl XAnimHasTime(const XAnim_s *anims, unsigned int animIndex);
bool __cdecl XAnimIsPrimitive(XAnim_s *anims, unsigned int animIndex);
void __cdecl XAnimSetTime(XAnimTree_s *tree, unsigned int animIndex, float time, unsigned __int16 cmdIndex);
unsigned int __cdecl XAnimRestart(XAnimTree_s *tree, unsigned int infoIndex, float goalTime, int cmdIndex);
int __cdecl XAnimSetGoalWeight(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex);
void __cdecl XAnimSetAnimRate(XAnimTree_s *tree, unsigned int animIndex, float rate);
bool __cdecl XAnimIsLooped(const XAnim_s *anims, unsigned int animIndex);
unsigned __int8 __cdecl XAnimGetBoneCount(const XAnim_s *anims, unsigned int animIndex);
int __cdecl XAnimHasBone(const XAnim_s *anims, unsigned int animIndex, unsigned __int16 boneName);
bool __cdecl XAnimIsClientNode(const XAnim_s *anims, unsigned int animIndex);
bool __cdecl XAnimIsClientNode(XAnimTree_s *tree, unsigned int animIndex);
char __cdecl XAnimNotetrackExists(const XAnim_s *anims, unsigned int animIndex, unsigned int name);
void __cdecl XAnimAddNotetrackTimesToScriptArray(const XAnim_s *anims, unsigned int animIndex, unsigned int name);
int __cdecl XAnimSetCompleteGoalWeight(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex);
void __cdecl XAnimCloneAnimInfo(XAnimTree_s *toTree, const XAnimInfo *from, XAnimInfo *to);
void __cdecl XAnimCloneAnimTree(const XAnimTree_s *from, XAnimTree_s *to);
void __cdecl XAnimCloneAnimTree_r(
                const XAnimTree_s *from,
                XAnimTree_s *to,
                unsigned int fromInfoIndex,
                unsigned int toInfoParentIndex);
XAnimInfo *__cdecl GetAnimInfo(int infoIndex);
char __cdecl XAnimGetParamValue(const XAnim_s *anims, unsigned int animIndex, const char *paramName, float *outValue);
bool __cdecl XAnimGetParamValue(XAnimTree_s *tree, unsigned int animIndex, const char *paramName, float *outValue);
bool __cdecl XAnimGetParamValueTree(
                const XAnim_s *anims,
                unsigned int animIndex,
                const char *paramName,
                float *outValue);
//void __thiscall bitarray<160>::setBit(bitarray<160> *this, unsigned int pos);
void __cdecl XAnim_SetTime(float time, int frameCount, XAnimTime *animTime);
