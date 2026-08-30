#pragma once
#include <universal/com_math.h>
#include <physics/phys_local.h>
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/fxprimitives.h>
#include <gfx_d3d/r_dpvs.h>
#include <gfx_d3d/r_model_lighting.h>

struct GfxPackedVertex;

struct ray2_t // sizeof=0x14
{                                       // XREF: GlassShard::Outline::Vertex/r
                                        // OutlineEdge/r
    float origin[2];
    float dir[2];                       // XREF: GlassClient::PlayShatterFX(int,float const * const,float const * const)+5AC/o
    float len;                          // XREF: GlassClient::PlayShatterFX(int,float const * const,float const * const)+481/r
                                        // GlassClient::PlayShatterFX(int,float const * const,float const * const)+4B3/r ...
};

struct __declspec(align(16)) GlassPhysics // sizeof=0xA0
{
    phys_mat44 m_mat;
    phys_vec3 m_inv_inertia;
    phys_vec3 m_t_vel;
    phys_vec3 m_a_vel;
    phys_vec3 m_force_sum;
    phys_vec3 m_torque_sum;
    float m_inv_mass;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void CreateAxis(
        float *position,
        const float (*axis)[3],
        float *tVel,
        float *aVel,
        float mass,
        float *localBBoxMin,
        float *localBBoxMax);
    void AddForce(float *worldPos, float *impulse);
    void SetVelocity(float *t_vel);
    void SetAngularVelocity(float *a_vel);
    void Step(float deltaTime);
    void tensor_transform_principle(
        const phys_vec3 *diag,
        const phys_mat44 *mat,
        phys_mat44 *tensor);
    void IntegrateVelocity(float deltaTime);
    void IntegratePos(float deltaTime);
    void GetPosition(float *position, float (*axis)[3]);
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(GlassPhysics) == 160);
#endif

struct GlassDef;

struct ShardGroup // sizeof=0x54
{
    unsigned int packedPos;
    const GlassDef *glassDef;
    struct GlassShard *head;
    float worldBBoxMin[3];
    float worldBBoxMax[3];
    float origin[3];
    float extent[3];
    bool invalidBBox;
    bool visible;
    bool highLod;
    // padding byte
    __int16 numShards;
    __int16 numIndices;
    __int16 numVerts;
    unsigned __int16 lightingHandle;
    GfxLightingInfo lightingInfo;
    unsigned __int16 *renderIndices;

    void __thiscall Init(unsigned int p, const GlassDef *gd);
    void __thiscall Reset();
    void __thiscall Add(GlassShard *shard);
    void __thiscall Remove(GlassShard *shard);
    void __thiscall RemoveGlassShards(unsigned int glassIndex);
    void __thiscall UpdateBBox();
    void __thiscall Update(float deltaTime);
    void __thiscall GenerateVerts(bool firstView, unsigned int localClientNum);
    void __thiscall FreeRenderMemory();
    r_double_index_t *__thiscall AllocateIndices(int numIndices);
    GfxPackedVertex *__thiscall AllocateVerts(int numVerts, unsigned __int16 *vertsBaseIndex);
    void __thiscall ExplosionEvent(
        const float *origin,
        float damageInner,
        float damageOuter,
        float radius,
        int mod);
    int __thiscall TracePoint(float *p0, const float *p1);
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(ShardGroup) == 84);
#endif

struct GlassShardMeshVertex // sizeof=0x2
{
    unsigned __int8 pos;
    unsigned __int8 norm;
};

struct GlassShard // sizeof=0x90
{
    enum RemoveReason : __int32
    {                                       // XREF: ?Remove@GlassShard@@QAEXW4RemoveReason@1@_N@Z/r
        REMOVE_HIT_BOTTOM           = 0x0,
        REMOVE_OUT_OF_SHARDS        = 0x1,
        REMOVE_OUT_OF_VERTEX_MEMORY = 0x2,
        REMOVE_OUT_OF_SHARD_MEMORY  = 0x3,
        REMOVE_OUT_OF_PHYSICS       = 0x4,
        REMOVE_ROLLBACK_TIME        = 0x5,
        NUM_REMOVE_REASONS          = 0x6,
        REMOVE_DONT_TRACK           = 0x7,
        // LWSS ADD
        KISAK_I_HAVE_NO_CLUE_WHY    = 0x8,
        KISAK_TOTAL
        // LWSS END
    };

    struct Outline;
    struct Triangles // sizeof=0x108
    {                                       // XREF: ?Create@GlassShard@@QAE_NPBUGlass@@@Z/r
        const struct Outline *outline;
        unsigned __int8 triangleIndices[256];
        unsigned int nIndices;

        Triangles(const struct GlassShard::Outline *ol);
        double CalcCross(int idx1, int idx2, int idx3);
        char AddTri(
            unsigned __int8 v1,
            unsigned __int8 v2,
            unsigned __int8 v3);
        char Triangulate();
    };

    struct Outline
    {
        struct EdgeDistance // sizeof=0x14
        {                                       // XREF: ?TracePoint@GlassShard@@QAE_NQBM00M00@Z/r
            unsigned int edgeIndex;             // XREF: GlassShard::TracePoint(float const * const,float const * const,float const * const,float,float const * const,float const * const)+67B/r
                                                // GlassShard::TracePoint(float const * const,float const * const,float const * const,float,float const * const,float const * const)+A8A/r
            float edgeParam;                    // XREF: GlassShard::TracePoint(float const * const,float const * const,float const * const,float,float const * const,float const * const)+675/r
                                                // GlassShard::TracePoint(float const * const,float const * const,float const * const,float,float const * const,float const * const)+A84/r
            float dist;                         // XREF: GlassShard::TracePoint(float const * const,float const * const,float const * const,float,float const * const,float const * const)+570/r
            float closestPoint[2];              // XREF: GlassShard::TracePoint(float const * const,float const * const,float const * const,float,float const * const,float const * const)+602/r
        };
        struct Vertex
        {
            ray2_t edge;
            bool isOriginalEdge;                // XREF: GlassShard::Outline::Reverse(void)+1E7/w
            // padding byte
            // padding byte
            // padding byte

            void operator=(const GlassShard::Outline::Vertex *other);
        };
#ifndef KISAK_NX // nx-port: x86 layout assert
        static_assert(sizeof(Outline::Vertex) == 24);
#endif
        GlassShard::Outline::Vertex *verts; // XREF: GlassClient::Outlines::InitShards(GlassShard const *,GlassShard * * const,int)+4F/w
                                            // GlassShard::Create(Glass const *)+35/w ...
        float length;
        float area;
        unsigned __int8 numVerts;           // XREF: GlassClient::Outlines::InitShards(GlassShard const *,GlassShard * * const,int)+55/w
                                            // GlassShard::Create(Glass const *)+3B/w ...
        unsigned __int8 maxVerts;           // XREF: GlassClient::Outlines::InitShards(GlassShard const *,GlassShard * * const,int)+5C/w
                                            // GlassShard::Create(Glass const *)+42/w ...
        bool isClosed;                      // XREF: GlassClient::Outlines::InitShards(GlassShard const *,GlassShard * * const,int)+63/w
                                            // GlassShard::Create(Glass const *)+49/w ...
        // padding byte

        void MarkAllEdge(bool e);

        int Init(Vertex *ptr,unsigned __int8 num);
        int SetPointers(unsigned __int8 *ptr);
        GlassShard::Outline &operator=(const GlassShard::Outline &o);
        double Length() const;
        inline float Area() const
        {
            return this->area;
        }
        Vertex *Add(const float *pos);
        Vertex *Add(float *dir, float len);

        char CloseOutline();

        void GetBBox(float *mn, float *mx);
        double Extent();
        void Recenter(bool flip, float *offset);
        bool HasNarrowAngle();
        bool IsNarrow();
        void Reverse();
        int GetNumIntersections(
            const float *start,
            const float *dir,
            float *nearestDist,
            int *nearestedge) const;
        char DoesIntersect(
            const float *start,
            const float *dir,
            float len,
            float padding);
        void GetNearestDistances(
            const float *p,
            EdgeDistance *dists,
            int nDists);
        void Verify() const;

        static void Defrag(GlassShard *ptr);
        void Defrag();
    };

    struct __declspec(align(4)) Mesh // sizeof=0x10
    {                                       // XREF: GlassShard/r
        PackedUnitVec *normArray;
        unsigned __int8 *indices;
        unsigned __int8 numNorm;
        unsigned __int8 numVerts;
        unsigned __int8 numVertsLow;
        unsigned __int8 numIndices;
        unsigned __int8 numIndicesLow;
        // padding byte
        // padding byte
        // padding byte

        void Clear();
        void SetTriangles(
            unsigned __int8 *ptr,
            unsigned __int8 *triangles,
            unsigned int numTriIdxs);
        int SetPointers(PackedUnitVec *ptr);
        void Init(
            const GlassShard::Outline *outline,
            float thickness,
            const float *tangt);
        static void __cdecl InitVertexList(unsigned __int8 numOutlineVerts, GlassShardMeshVertex *verts);
        static unsigned int __cdecl GetMemorySize(unsigned int numOutineVerts);
    };

    GlassShard *groupNext;
    unsigned __int8 *memoryPtr;
    unsigned __int16 memorySize;
    unsigned __int16 glassIndex;
    GlassShard::Outline outline;
    float thickness;
    float uvScale;
    GlassShard::Mesh mesh;
    float axis[3][3];
    float origin[3];
    int physObjId;
    GlassPhysics *glassPhysics;
    float timeUntilAction;
    float worldBBoxMin[3];
    float worldBBoxMax[3];
    ShardGroup *group;
    bool isOriginalEdge;
    bool remove;
    bool inGroupChange;
    bool delayedDrop;


    void Init();
    void Destroy();
    double EdgeRatio();
    void UnEdge();
    char Create(const Glass *glass);
    bool CanSplit(float maxShardSize, float minShardSize) const;
    bool CanSplit(bool shatter) const;
    int Shatter(const GlassShard **newShards, int maxNewShards);
    void InitPhysics(
        const GlassShard **newShards,
        int numNewShards,
        float glassExtent,
        const float *pos,
        const float *dir);
    void InitMesh();
    char AllocateMemory(
        unsigned int nHull,
        const Triangles *triangles);
    void FreeMemory();
    void Defrag();
    void ToWorldPos(float *pLocal, float *pWorld, bool is3D);
    void ToWorldDir(float *dLocal, float *dWorld, bool is3D);
    void ToLocal(float *pos, float *dir, float *localPos, float *localDir);
    bool Intersect(float *pos, float *dir, float *hitPoint);
    void ExplosionEvent(
        const float *expOrigin,
        float damageInner,
        float damageOuter,
        float radius,
        int mod);
    char TracePoint(
        float *p0,
        const float *p1,
        float *dir,
        float length,
        const float *mins,
        float *maxs);
    void GetLocalBBox(float *mins, float *maxs);
    void UpdateBBox();
    void Update(float deltaTime);
    void ChangeGroup();
    void Remove(RemoveReason reason, bool delay);
    void GenerateVerts(
        bool highLod,
        GfxPackedVertex *baseVerts,
        unsigned __int16 vertsBaseIndex,
        unsigned __int16 *idxOut);
    int Split(
        const GlassShard **newShards,
        float minShardSize,
        unsigned int startEdge,
        float startEdgeParam) const;
    int Chip(
        const Outline::EdgeDistance *dist,
        const float *hitPoint,
        const GlassShard **newShards,
        float minShardSize);
    int InitSplitShards(
        Outline *outline1,
        Outline *outline2,
        const GlassShard **newShards,
        float minShardSize) const;
    char Init(
        const GlassShard *other,
        const GlassShard::Outline *newOutline,
        const float *const offset);
    double GetMass();
    bool InitPhysicsObj(bool enableCollisions);
    void DestroyPhysicsObj();
    void DisableCollisions();
    void AddForce(float *pos, const float *forceIn);
    void SetVelocity(float *t_vel, float *a_vel);
    bool LeaveOnEdge(float stickiness);
    bool IsOnBottomEdge();
    void Recenter();
    char InitPhysics(
        bool enableCollisions,
        const float *hitPos,
        const float *hitDir,
        float glassSize,
        float stickiness);
    void DrawOutline();

    //static void __cdecl Defrag(GlassShard *ptr);

    static int splitFailCount[8];
    static int lastFreeMemorySize;
    static int removeReasonsCount[KISAK_TOTAL];
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(GlassShard) == 144);
#endif

void GlassShard_Defrag(void *ptr); // changed from static member func

struct TempOutline : GlassShard::Outline // sizeof=0x610
{                                       // XREF: ?Create@GlassShard@@QAE_NPBUGlass@@@Z/r
    GlassShard::Outline::Vertex v[64];  // XREF: GlassClient::Outlines::InitShards(GlassShard const *,GlassShard * * const,int)+49/o
};

struct OutlineEdge // sizeof=0x18
{                                       // XREF: ?PlayShatterFX@GlassClient@@QBEXHQBM0@Z/r
    int index;                          // XREF: GlassClient::PlayShatterFX(int,float const * const,float const * const)+96/w
    ray2_t ray;                         // XREF: GlassClient::PlayShatterFX(int,float const * const,float const * const)+D8/o
};

bool __cdecl IsInside(const float *v1, const float *v2, const float *v3, const float *p);
double __cdecl GetSegmentParam(const float *a1, const float *a2, const float *p);
bool __cdecl Vec2IntersectSegments(const float *a1, const float *a2, const float *b1, const float *b2, float *ret);
char __cdecl Vec2IntesectLines(const float *a1, const float *a2, const float *b1, const float *b2, float *ret);
int compareShards(const void *s1, const void *s2);
