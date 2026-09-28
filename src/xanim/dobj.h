#pragma once

#include <qcommon/bitarray.h>

#define DOBJ_HANDLE_NONE -1

#define NO_BONEINDEX 255

#define DOBJ_MAX_PARTS 160

struct SavedDObjModel // sizeof=0x2
{                                       // XREF: SavedDObj/r
    unsigned __int16 boneName;
};

struct SavedDObj // sizeof=0x68
{                                       // XREF: ?DObjArchive@@YAXPAUDObj@@@Z/r
    SavedDObjModel dobjModels[32];
    struct XModel **models;                    // XREF: DObjArchive(DObj *)+43/w
                                        // DObjUnarchive(DObj *)+8A/r ...
    unsigned int ignoreCollision;       // XREF: DObjArchive(DObj *)+33/w
                                        // DObjArchive(DObj *)+111/r ...
    unsigned __int16 numModels;         // XREF: DObjArchive(DObj *)+19/o
                                        // DObjArchive(DObj *):loc_7E550E/r ...
    unsigned __int16 entnum;            // XREF: DObjArchive(DObj *)+B/o
                                        // DObjUnarchive(DObj *)+102/r
    unsigned __int8 flags;              // XREF: DObjArchive(DObj *)+4F/w
                                        // DObjUnarchive(DObj *)+E0/r ...
    char localClientNum;                // XREF: DObjArchive(DObj *)+5B/w
                                        // DObjUnarchive(DObj *)+D8/r
    // padding byte
    // padding byte
    struct XAnimTree_s *tree;                  // XREF: DObjArchive(DObj *)+12/o
                                        // DObjUnarchive(DObj *)+10E/r
    unsigned int hidePartBits[5];       // XREF: DObjArchive(DObj *)+69/w
                                        // DObjArchive(DObj *)+72/w ...
};

struct DObjAnimMat // sizeof=0x20
{                                                                             // XREF: XAnimCalcAnimInfo/r
        float quat[4];                                            // XREF: CG_DoBaseOriginController+1D7/w
        float trans[3];                                         // XREF: FX_GenerateMarkVertsForMark_MatrixFromPlacement+10/w
        float transWeight;                                    // XREF: CG_DoBaseOriginController+23B/w
};

// R_SkinXSurfaceRigid() checks if alignment = 16
struct alignas(16) DObjSkelMat // sizeof=0x40
{                                       // XREF: FX_GenerateMarkVertsForMark_MatrixFromPlacement/r
                                        // FX_GenerateMarkVertsForMark_MatrixFromAnim/r ...
    float axis[3][4];
    float origin[4];
};

struct DSkelPartBits // sizeof=0x3C
{                                                                             // XREF: DSkel/r
        //int anim[5];                                                // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+A0/w
        //int control[5];                                         // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+A7/w
        //int skel[5];                                                // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+92/w
        bitarray<160> anim;
        bitarray<160> control;
        bitarray<160> skel;
                                                                                // DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+9C/r
};



struct DSkel // sizeof=0x44
{                                                                             // XREF: DObj/r
                                                                                // ?DObjCalcBaseSkel@@YAXPBUDObj@@PAUDObjAnimMat@@QAH@Z/r
        DSkelPartBits partBits;                         // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+92/w
                                                                                // DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+9C/r ...
        int timeStamp;                                            // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+67/w
                                                                                // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+64/w
        union //$E37E2D40D980A1CD669FE5FFE5D2594A // sizeof=0x4
        {                                                                             // XREF: DObjCalcBaseSkel(DObj const *,DObjAnimMat *,int * const)+64/w
                                                                                        // DSkel/r
            DObjAnimMat *mat;
            DObjAnimMat *localMat;
        };
};

struct DObj // sizeof=0x7C
{
        //$505CB099FB1B2F089675B7DC2E6E811F ___u0;
        union //$505CB099FB1B2F089675B7DC2E6E811F // sizeof=0x4
        {                                                                             // XREF: DObj/r
                struct XAnimTree_s *localTree;
                struct XAnimTree_s *tree;
        };
        unsigned __int16 duplicateParts;
        unsigned __int16 entnum;
        unsigned __int8 duplicatePartsSize;
        unsigned __int8 numModels;
        unsigned __int8 numBones;
        // padding byte
        unsigned int ignoreCollision;
        volatile unsigned int locked;
        DSkel skel;
        float radius;
        unsigned int hidePartBits[5];
        char localClientIndex;
        unsigned __int8 flags;
        // padding byte
        // padding byte
        unsigned __int8 *ikState;
        //$5CBEAE0A843BDDEAC35D3349B6AE42EB ___u14;
        union //$5CBEAE0A843BDDEAC35D3349B6AE42EB // sizeof=0x4
        {                                                                             // XREF: DObj/r
                struct XModel **localModels;
                struct XModel **models;
        };
};

// localModels holds numModels model pointers followed by numModels parent
// bone bytes. nx-port: was 5 * n (4-byte x86 pointers).
#define DOBJ_MODELS_SIZE(n) ((int)((sizeof(struct XModel *) + 1) * (n)))

struct __declspec(align(2)) DObjModel_s // sizeof=0x8
{                                                                             // XREF: DObjModel/r
                                                                                // ?DObjArchive@@YAXPAUDObj@@@Z/r ...
        XModel *model;                                            // XREF: BG_UpdatePlayerDObj(int,DObj *,entityState_s *,clientInfo_t *,int)+287/w
                                                                                // BG_UpdatePlayerDObj(int,DObj *,entityState_s *,clientInfo_t *,int)+294/r ...
        unsigned __int16 boneName;                    // XREF: BG_UpdatePlayerDObj(int,DObj *,entityState_s *,clientInfo_t *,int)+2CC/w
                                                                                // BG_UpdatePlayerDObj(int,DObj *,entityState_s *,clientInfo_t *,int)+494/w ...
        bool ignoreCollision;                             // XREF: BG_UpdatePlayerDObj(int,DObj *,entityState_s *,clientInfo_t *,int)+2DA/w
                                                                                // BG_UpdatePlayerDObj(int,DObj *,entityState_s *,clientInfo_t *,int)+4B8/w ...
        // padding byte
};

struct DObjTrace_s // sizeof=0x20
{                                                                             // XREF: DObjTrace/r
                                                                                // ?SV_TracePointToEntity@@YAXPBUpointtrace_t@@PAUsvEntity_s@@PAUtrace_t@@@Z/r ...
        float fraction;                                         // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+579/w
                                                                                // CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+716/r ...
        int sflags;                                                 // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7AA/r
                                                                                // SV_TracePointToEntity(pointtrace_t const *,svEntity_s *,trace_t *)+64D/r
        float normal[3];                                        // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7E8/r
                                                                                // CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7F7/r ...
        unsigned __int16 modelIndex;                // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7B6/r
                                                                                // SV_TracePointToEntity(pointtrace_t const *,svEntity_s *,trace_t *)+656/r
        unsigned __int16 partName;                    // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7C4/r
                                                                                // SV_TracePointToEntity(pointtrace_t const *,svEntity_s *,trace_t *)+661/r
        unsigned __int16 partGroup;                 // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7D2/r
                                                                                // SV_TracePointToEntity(pointtrace_t const *,svEntity_s *,trace_t *)+677/r
        // padding byte
        // padding byte
        int localBoneIndex;                                 // XREF: CG_PointTraceToEntity(pointtrace_t const *,int,trace_t *)+7DD/r
                                                                                // SV_TracePointToEntity(pointtrace_t const *,svEntity_s *,trace_t *)+66C/r
};

struct PhysGeomList;
struct PhysPreset;
struct XAnimTree_s;
struct XModel;

void __cdecl DObjInit();
void __cdecl DObjShutdown();
void __cdecl DObjDumpInfo(const DObj *obj);
bool __cdecl DObjIgnoreCollision(const DObj *obj, char modelIndex);
void __cdecl DObjGetHierarchyBits(DObj *obj, int boneIndex, int *partBits);
bool __cdecl DObjSkelIsBoneUpToDate(DObj *obj, int boneIndex);
void __cdecl DObjSetTree(DObj *obj, XAnimTree_s *tree);
void __cdecl DObjCreate(
                DObjModel_s *dobjModels,
                unsigned int numModels,
                XAnimTree_s *tree,
                unsigned __int8 *buf,
                __int16 entnum);
void __cdecl DObjCreateDuplicateParts(DObj *obj, DObjModel_s *dobjModels, unsigned int numModels);
void __cdecl DObjDumpCreationInfo(DObjModel_s *dobjModels, unsigned int numModels);
void __cdecl DObjComputeBounds(DObj *obj);
void __cdecl DObjCreateExt(
                DObjModel_s *dobjModels,
                unsigned int numModels,
                XAnimTree_s *tree,
                unsigned __int8 *buf,
                __int16 entnum,
                bool isServer,
                bool isLocalPlayer,
                unsigned __int8 localClientIndex);
void __cdecl DObjFree(DObj *obj);
void __cdecl DObjGetCreateParms(
                const DObj *obj,
                DObjModel_s *dobjModels,
                unsigned __int16 *numModels,
                XAnimTree_s **tree,
                unsigned __int16 *entnum);
void __cdecl DObjCalcBounds(DObj *dobj, float *mins, float *maxs);
void __cdecl DObjArchive(DObj *obj);
void __cdecl DObjUnarchive(DObj *obj);
void __cdecl DObjSkelClear(const DObj *obj);
bool __cdecl DObjIsServer(const DObj *obj);
bool __cdecl DObjIsLocalPlayer(const DObj *obj);
bool __cdecl DObjIsPlayerShadow(const DObj *obj);
bool __cdecl DObjIsLeftHandGripIKActive(const DObj *obj);
int __cdecl DObjGetLocalClientIndex(const DObj *obj);
void __cdecl DObjGetBounds(const DObj *obj, float *mins, float *maxs);
void __cdecl DObjPhysicsGetBounds(const DObj *obj, float *mins, float *maxs);
double __cdecl DObjGetRadius(const DObj *obj);
PhysPreset *__cdecl DObjGetPhysPreset(const DObj *obj);
const char *__cdecl DObjGetName(const DObj *obj);
char *__cdecl DObjGetBoneName(const DObj *obj, int boneIndex);
char *__cdecl DObjGetModelParentBoneName(const DObj *obj, int modelIndex);
XAnimTree_s *__cdecl DObjGetTree(const DObj *obj);
int __cdecl DObjBad(const DObj *obj);
void __cdecl DObjTracelinePartBits(DObj *obj, int *partBits);
void __cdecl DObjTraceline(DObj *obj, float *start, float *end, unsigned __int8 *priorityMap, DObjTrace_s *trace);
void __cdecl InvMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out);
void __cdecl DObjGeomTraceline(DObj *obj, float *localStart, float *localEnd, int contentmask, DObjTrace_s *results);
void __cdecl DObjGeomTracelinePartBits(DObj *obj, int contentmask, int *partBits);
int __cdecl DObjHasContents(DObj *obj, int contentmask);
int __cdecl DObjGetContents(const DObj *obj);
bool __cdecl DObjHasCollmap(const DObj *obj);
PhysGeomList *__cdecl DObjGetCollmap(DObj *obj);
XModel *__cdecl DObjGetXModel(DObj *obj, int index);
int __cdecl DObjSetLocalBoneIndex(DObj *obj, int *partBits, int boneIndex, const float *trans, const float *angles);
int __cdecl DObjGetModelBoneIndex(const DObj *obj, const char *modelName, unsigned int name, unsigned __int8 *index);
void __cdecl DObjGetBasePoseMatrix(DObj *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat);
void __cdecl DObjSetHidePartBits(DObj *obj, const unsigned int *partBits);
bool __cdecl DObjHasFlag(const DObj *obj, unsigned __int8 flag);
void __cdecl DObjSetFlag(DObj *obj, unsigned __int8 flag, int val);
bool __cdecl DObjHasIKActiveFlag(const DObj *obj);
void __cdecl DObjSetIKActiveFlag(DObj *obj, int val);
void __cdecl DObjSetIKState(DObj *obj, unsigned __int8 *pIKState);
