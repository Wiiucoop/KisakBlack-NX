#pragma once
#include "r_material.h"
#include <tl/jobqueue/jobqueue_all.h>
#include <universal/q_shared.h>

struct GfxWorld;
struct XModel;

enum stream_status : __int32
{                                       // XREF: pendingRequest/r
                                        // pendingRequest/r
    STREAM_STATUS_INVALID    = 0x0,
    STREAM_STATUS_PRE        = 0x1,
    STREAM_STATUS_QUEUED     = 0x2,
    STREAM_STATUS_INPROGRESS = 0x3,
    STREAM_STATUS_CANCELLED  = 0x4,
    STREAM_STATUS_READFAILED = 0x5,
    STREAM_STATUS_EOF        = 0x6,
    STREAM_STATUS_FINISHED   = 0x7,
    STREAM_STATUS_USER1      = 0x8,
    STREAM_STATUS_COUNT      = 0x9,
};

struct streamerHintInfo // sizeof=0x20
{                                       // XREF: .data:s_streamHints/r
    int entNum;
    float origin[3];                    // XREF: R_StreamUpdate_FindImageAndOptimize+311/o
    float angles[3];
    float importance;                   // XREF: R_StreamUpdate_FindImageAndOptimize+2D5/r
                                        // R_StreamUpdate_FindImageAndOptimize+2F4/r
};

struct distance_data // sizeof=0x8
{                                       // XREF: R_StreamUpdateForcedModels/r
                                        // R_StreamUpdateForBModel/r ...
    float importance;                   // XREF: R_StreamUpdateForcedModels+63/w
                                        // R_StreamUpdateForcedModels+11C/r ...
    float distanceForHimip;             // XREF: R_StreamUpdateForcedModels+56/w
                                        // R_StreamUpdateForBModel+E8/w ...
};

struct StreamFrontendGlob
{
    void *mainBuffer;
    int mainBufferSize;
    void *extraBuffer;
    int extraBufferSize;
    unsigned int frame;
    _BYTE gap14[108];
    float materialImportance[4096];
    unsigned int materialImportanceBits[128];
    unsigned int materialTouchBits[128];
    unsigned int materialPreventBits[128];
    float modelDistance[1000];
    _BYTE gap5620[96];
    unsigned int modelDistanceBits[32];
    float dynamicModelDistance[1000];
    _BYTE gap66A0[96];
    unsigned int dynamicModelDistanceBits[32];
    unsigned int modelTouchBits[32];
    unsigned int imageInSortedListBits[128];
    float imageImportance[4096];
    unsigned int imageImportanceBits[128];
    float dynamicImageImportance[4096];
    unsigned int dynamicImageImportanceBits[128];
    unsigned int dummy;
    _BYTE gapEE04[1440];
    unsigned int imageLoading[128];
    unsigned int imageUseBits[128];
    unsigned int imageForceBits[128];
    _BYTE gapF9A4[16];
    unsigned int imageInitialBits[128];
    _BYTE gapFBB4[16];
    unsigned int imageTouchBits[2][132];
    int activeImageTouchBits;
    float touchedImageImportance;
    float initialImageImportance;
    float forcedImageImportance;
    bool imageInitialBitsSet;
    _BYTE gapFFF5[3];
    int initialLoadAllocFailures;
    bool preloadCancelled;
    bool diskOrderImagesNeedSorting;
    __declspec(align(1)) int sortedImages[4096];
    _declspec(align(1)) int sortedImageCount;
    bool calculateTotalBytesWanted;
    _BYTE gap14003[74];
    _BYTE gap1400311[435];
    int totalBytesWanted;
    char syncThing;
    __declspec(align(4)) _BYTE gap14208[4];
    volatile int queryClient;
    volatile int queryInProgress;
    bool diskOrder;
    bool forceDiskOrder;
    bool outputImageList;
    bool ignoreMainView;
};

struct __declspec(align(4)) pendingRequest // sizeof=0x30
{                                       // XREF: .data:s_pendingRequests/r
    int id[1];
    GfxImage *image;
    int imagePart;
    unsigned __int8 *buffer;
    int bufferSize;
    int startTime;
    float importance;
    stream_status status;               // XREF: R_StreamUpdate_ReadTextures(void)+4B/r
                                        // R_Stream_Sync+1AC/r ...
    stream_status partStatus[1];
    int numParts;
    int bytesToCopy;
    bool highMip;
    // padding byte
    // padding byte
    // padding byte
};

struct StreamUpdateCmd // sizeof=0x1C
{                                       // XREF: R_StreamUpdate_FindImageAndOptimize/r
                                        // ?R_StreamUpdatePerClient@@YAXQBM@Z/r
    StreamFrontendGlob *frontend;       // XREF: R_StreamUpdate_FindImageAndOptimize:loc_AA38BC/w
                                        // R_StreamUpdatePerClient(float const * const):loc_AA460A/w
    float viewPos[3];                   // XREF: R_StreamUpdate_FindImageAndOptimize+25A/w
                                        // R_StreamUpdate_FindImageAndOptimize+267/w ...
    float maxDistSq;                    // XREF: R_StreamUpdate_FindImageAndOptimize+27E/w
                                        // R_StreamUpdatePerClient(float const * const)+11C/w
    float distanceScale[2];             // XREF: R_StreamUpdate_FindImageAndOptimize+28B/w
                                        // R_StreamUpdate_FindImageAndOptimize+29B/w ...
};

struct __declspec(align(4)) StreamSortCmd // sizeof=0x8
{                                       // XREF: R_StreamUpdate_FindImageAndOptimize/r
                                        // ?R_StreamUpdate_End@@YAXXZ/r
    StreamFrontendGlob *frontend;       // XREF: R_StreamUpdate_FindImageAndOptimize+154/w
                                        // R_StreamUpdate_End(void)+59/w
    bool diskOrder;                     // XREF: R_StreamUpdate_FindImageAndOptimize+15B/w
                                        // R_StreamUpdate_End(void)+65/w
    // padding byte
    // padding byte
    // padding byte
};

struct StreamCombineCmd // sizeof=0x4
{                                       // XREF: ?R_StreamUpdate_End@@YAXXZ/r
    StreamFrontendGlob *frontend;       // XREF: R_StreamUpdate_End(void)+3F/w
};

struct DObj;

bool __cdecl R_StreamIsEnabled();
void __cdecl R_StreamPushDisable();
void __cdecl R_StreamPopDisable();
void __cdecl R_StreamSetInitData(int buffer_size);
void __cdecl R_StreamSetDefaultConfig(bool clear);
void __cdecl R_StreamSetUIConfig(bool clear);
double __cdecl R_Stream_GetProgress();
void __cdecl R_StreamUpdate_ReadTextures();
bool __cdecl R_StreamUpdate_ProcessFileCallbacks();
void __cdecl R_Stream_InvalidateRequest(pendingRequest *request);
void __cdecl R_StreamUpdate_SetupInitialImageList();
void __cdecl R_Stream_ForceLoadMaterial(Material *material, int part);
void __cdecl R_Stream_ForceLoadImage(GfxImage *image, int part);
void __cdecl R_Stream_ForceLoadModel(XModel *model, int part);
char __cdecl R_StreamTouchDObjAndCheck(const DObj *obj, int level);
void __cdecl R_StreamTouchMaterial(Material *material);
char __cdecl R_StreamTouchMaterialAndCheck(Material *material, int level);
void __cdecl R_StreamTouchImage(GfxImage *image);
bool __cdecl R_StreamTouchImageAndCheck(GfxImage *image, int level);
bool __cdecl R_StreamImageCheck(GfxImage *image, int level);
void __cdecl R_Stream_ResetHintEntities();
void __cdecl R_StreamInit();
void __cdecl R_StreamShutdown();
void __cdecl R_Stream_ReleaseImage(GfxImage *image, bool lock, bool delayDirty);
void __cdecl R_StreamSyncThenFlush(bool flushAll);
void R_Stream_Sync();
void __cdecl R_StreamPushSyncDisable();
void __cdecl R_StreamPopSyncDisable();
void __cdecl R_StreamUpdate_Idle();
void __cdecl R_StreamUpdate_CompletePreload(void (__cdecl *pumpfunc)());
char __cdecl R_StreamUpdate(const float *viewPos);
void __cdecl R_Stream_AddImagePartImportance(int imagePartIndex, float importance);
void __cdecl importance_swap_func(int *a, int *b);
bool __cdecl importance_compare_func(int a, int b);
void __cdecl importance_merge_sort(void **list, int list_count);
void __cdecl R_StreamUpdate_EndQuery();
void R_StreamUpdate_EndQuery_Internal();
char __cdecl R_StreamRequestImageAllocation(
                pendingRequest *request,
                GfxImage *image,
                bool highMip,
                int imagePart,
                float importance);
char __cdecl R_StreamRequestImageRead(pendingRequest *request);
char __cdecl R_StreamUpdate_FindImageAndOptimize(const float *viewPos);
void __cdecl R_StreamUpdate_AddInitialImages(float importance);
void __cdecl R_StreamUpdate_AddForcedImages(float forceImportance, float touchImportance);
void R_StreamUpdatePreventedMaterials();
void R_CheckHighmipAabbs();
void __cdecl R_CheckHighmipAabbs_r(GfxWorld *world, int aabbTreeNode, float *mins, float *maxs);
char __cdecl R_StreamUpdate_TryBeginQuery();
void R_StreamUpdateTouchedModels();
void __cdecl R_StreamUpdateForXModelTouched(const XModel *model);
void __cdecl R_StreamUpdateForcedModels();
void __cdecl R_StreamTouchImagesFromMaterial(const Material *remoteMaterial, float importance);
void __cdecl R_StreamUpdatePerClient(const float *viewPos);
void __cdecl R_StreamUpdateDynamicModels(
                const float *viewPos,
                float maxDistSq,
                unsigned int frame,
                float *distanceScale);
void __cdecl R_StreamUpdateForBModel(
                const float *viewPos,
                unsigned int frame,
                unsigned int surfId,
                const GfxBrushModel *bmodel,
                const float *origin,
                float maxDistSq,
                Material *altMaterial,
                bool isVisible,
                float *distanceScale);
double __cdecl PointDistSqFromBounds(const float *v, const float *mins, const float *maxs);
void __cdecl R_StreamUpdate_AddDynamicXModelDistance(
                const XModel *model,
                const float *viewPos,
                const float *origin,
                float scale,
                bool visible,
                float *distanceScale);
void __cdecl R_StreamUpdate_End();
void __cdecl R_Stream_UpdateStaticModelsCmd(char *data);
void __cdecl R_StreamUpdateStaticModel(
                int staticModelIndex,
                const float *viewPos,
                float maxDistSq,
                float *distanceScale);
void __cdecl R_StreamUpdate_AddXModelDistance(
                const XModel *model,
                const float *viewPos,
                const float *origin,
                float scale,
                bool visible,
                float *distanceScale);
void __cdecl R_Stream_UpdateStaticSurfacesCmd(char *data);
void __cdecl R_StreamUpdateWorldSurface(int surfId, const float *viewPos, float maxDistSq, float *distanceScale);
void __cdecl MultiplePointDistSqFromBounds(
                distance_data *distances,
                const float *v,
                const float *mip0mins,
                const float *mip0maxs,
                float himipRadiusSq,
                float distanceScale);
double __cdecl FastPointDistSqFromBounds(float4 mins, float4 maxs);
void __cdecl R_Stream_SortCmd(_BYTE *data);
void __cdecl R_StreamUpdate_EndQuerySort(bool diskOrder);
void __cdecl R_Stream_CombineCmd();
void R_StreamUpdate_CombineImportance();
void __cdecl R_StreamUpdateForXModel(const XModel *remoteModel, float distSq);
int __cdecl r_stream_update_staticmodelsCallback(jqBatch *batch);
int __cdecl r_stream_update_staticsurfacesCallback(jqBatch *batch);
int __cdecl r_stream_sortCallback(jqBatch *batch);
int __cdecl r_stream_combineCallback(jqBatch *batch);
int __cdecl r_stream_updateCallback(jqBatch *batch);
void __cdecl R_StreamUpdateStatic(const float *viewPos, float maxDistSq, float *distanceScale);

extern StreamFrontendGlob streamFrontendGlob;