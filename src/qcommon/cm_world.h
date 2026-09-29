#pragma once
#include <server/sv_world.h>
#include "cm_staticmodel.h"
#include "cm_trace.h"

#define SECTOR_HEAD 1

struct moveclip_t;

enum PredictionTraceResult : __int32
{                                       // XREF: ?Path_AttemptDodge@@YI_NPAUpath_t@@QBM11HPBHHHHH@Z/r
    PTR_SUCCESS    = 0x0,
    PTR_HIT_WORLD  = 0x1,
    PTR_HIT_ENTITY = 0x2,
};

struct locTraceWork_t // sizeof=0x34
{                                       // XREF: ?CM_PointTraceStaticModels@@YAXPAUtrace_t@@QBM1H@Z/r
    int contents;                       // XREF: CM_PointTraceStaticModels(trace_t *,float const * const,float const * const,int)+2A/w
    TraceExtents extents;               // XREF: CM_PointTraceStaticModels(trace_t *,float const * const,float const * const,int)+1F/o
};

struct areaParms_t // sizeof=0x18
{                                       // XREF: ?CM_AreaEntities@@YAHQBM0PAHHH@Z/r
    const float *mins;                  // XREF: CM_AreaEntities(float const * const,float const * const,int *,int,int)+18/w
    const float *maxs;                  // XREF: CM_AreaEntities(float const * const,float const * const,int *,int,int)+1E/w
    int *list;                          // XREF: CM_AreaEntities(float const * const,float const * const,int *,int,int)+24/w
    int count;                          // XREF: CM_AreaEntities(float const * const,float const * const,int *,int,int)+27/w
                                        // CM_AreaEntities(float const * const,float const * const,int *,int,int)+48/r
    int maxcount;                       // XREF: CM_AreaEntities(float const * const,float const * const,int *,int,int)+31/w
    int contentmask;                    // XREF: CM_AreaEntities(float const * const,float const * const,int *,int,int)+37/w
};

struct staticmodeltrace_t // sizeof=0x34
{                                       // XREF: ?CM_PointTraceStaticModelsComplete@@YAHQBM0H@Z/r
    TraceExtents extents;               // XREF: CM_PointTraceStaticModelsComplete(float const * const,float const * const,int)+25/w
                                        // CM_PointTraceStaticModelsComplete(float const * const,float const * const,int)+32/w ...
    int contents;                       // XREF: CM_PointTraceStaticModelsComplete(float const * const,float const * const,int)+1B/w
};

struct sightclip_t // sizeof=0x48
{                                       // XREF: ?Path_DisconnectPathsForEntity@@YAXPAUgentity_s@@@Z/r
                                        // ?Path_PredictionTraceCheckForEntities@@YI?AW4PredictionTraceResult@@QBM0PBHHHHQAM@Z/r ...
    float mins[3];                      // XREF: G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+1EF/w
                                        // G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+1FC/w ...
    float maxs[3];                      // XREF: G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+216/w
                                        // G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+223/w ...
    float outerSize[3];                 // XREF: SV_SightTraceCapsule(int *,float const * const,float const * const,float const * const,float const * const,col_context_t &)+477/w
                                        // SV_SightTraceCapsule(int *,float const * const,float const * const,float const * const,float const * const,col_context_t &)+48C/w ...
    float start[3];                     // XREF: G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+1A2/w
                                        // G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+1AF/w ...
    float end[3];                       // XREF: G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+1C8/w
                                        // G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+1D5/w ...
    int passEntityNum[2];               // XREF: G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+23C/w
                                        // G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+243/r ...
    int contentmask;                    // XREF: G_CheckHitTriggerDamage(gentity_s *,float const * const,float const * const,int,int)+235/w
                                        // G_GrenadeTouchTriggerDamage(gentity_s *,float * const,float * const,int,int)+179/w ...
};

struct sightpointtrace_t // sizeof=0x2C
{                                       // XREF: ?SV_SightTracePoint@@YAHPAHQBM1AAUcol_context_t@@@Z/r
    float start[3];                     // XREF: SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+204/w
                                        // SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+211/w ...
    float end[3];                       // XREF: SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+22A/w
                                        // SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+237/w ...
    int passEntityNum[2];               // XREF: SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+24F/w
                                        // SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+258/w
    int contentmask;                    // XREF: SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+1FA/w
    int locational;                     // XREF: SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+261/w
    unsigned __int8 *priorityMap;       // XREF: SV_SightTracePoint(int *,float const * const,float const * const,col_context_t &)+26A/w
};

struct worldContents_s // sizeof=0xC
{                                       // XREF: worldSector_s/r
                                        // worldContents_t/r
    int contentsStaticModels;           // XREF: CM_SortNode+44F/r
                                        // CM_SortNode+45C/w ...
    int contentsEntities;
    unsigned __int16 entities;
    unsigned __int16 staticModels;
};

struct worldTree_s // sizeof=0xC
{                                       // XREF: worldSector_s/r
                                        // worldTree_t/r
    float dist;                         // XREF: CM_ClearWorld+EB/w
    unsigned __int16 axis;              // XREF: CM_ClearWorld+BC/w
                                        // CM_ClearWorld+C3/r ...
    //worldTree_s::<unnamed_type_u> u;    // XREF: CM_ClearWorld+6B/w
    union //worldTree_s::<unnamed_type_u> // sizeof=0x2
    {                                       // XREF: CM_ClearWorld+6B/w
                                            // worldTree_s/r
        unsigned __int16 parent;
        unsigned __int16 nextFree;
    } u;
    unsigned __int16 child[2];          // XREF: CM_ClearWorld+F3/r
                                        // CM_ClearWorld:loc_6C4D84/r
};

struct worldSector_s // sizeof=0x18
{                                       // XREF: cm_world_t/r
                                        // worldSector_t/r
    worldContents_s contents;           // XREF: CM_SortNode+44F/r
                                        // CM_SortNode+45C/w ...
    worldTree_s tree;                   // XREF: CM_ClearWorld+6B/w
                                        // CM_ClearWorld+BC/w ...
};

struct cm_world_t // sizeof=0x601C
{                                       // XREF: .data:cm_world/r
    float mins[3];                      // XREF: CM_ClearWorld+7A/r
                                        // CM_ClearWorld+8F/r ...
    float maxs[3];                      // XREF: CM_ClearWorld+1A/o
                                        // CM_ClearWorld+72/r ...
    unsigned __int16 freeHead;          // XREF: CM_ClearWorld+33/w
                                        // CM_UnlinkEntity(svEntity_s *)+1E5/r ...
    // padding byte
    // padding byte
    worldSector_s sectors[1024];        // XREF: CM_ClearWorld+6B/w
                                        // CM_ClearWorld+BC/w ...
};

void __cdecl CM_LinkWorld();
bool CM_ClearWorld();
void __cdecl CM_UnlinkEntity(svEntity_s *ent);
void __cdecl CM_LinkEntity(svEntity_s *ent, float *absmin, float *absmax, unsigned int clipHandle);
void __cdecl CM_AddEntityToNode(svEntity_s *ent, unsigned __int16 childNodeIndex);
void __cdecl CM_SortNode(unsigned __int16 nodeIndex, float *mins, float *maxs);
unsigned __int16 __cdecl CM_AllocWorldSector(float *mins, float *maxs);
void __cdecl CM_AddStaticModelToNode(cStaticModel_s *staticModel, unsigned __int16 childNodeIndex);
unsigned int CM_LinkAllStaticModels();
void __cdecl CM_LinkStaticModel(cStaticModel_s *staticModel);
int __cdecl CM_AreaEntities(const float *mins, const float *maxs, int *entityList, int maxcount, int contentmask);
void __cdecl CM_AreaEntities_r(unsigned int nodeIndex, areaParms_t *ap);
void __cdecl CM_PointTraceStaticModels(trace_t *results, const float *start, const float *end, int contentmask);
void __cdecl CM_PointTraceStaticModels_r(
                locTraceWork_t *tw,
                unsigned __int16 nodeIndex,
                const float *p1_,
                const float *p2,
                trace_t *trace);
int __cdecl CM_PointTraceStaticModelsComplete(const float *start, const float *end, int contentmask);
int __cdecl CM_PointTraceStaticModelsComplete_r(
                const staticmodeltrace_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1_,
                const float *p2);
void __cdecl CM_ClipMoveToEntities(moveclip_t *clip, trace_t *trace);
void __cdecl CM_ClipMoveToEntities_r(
                const moveclip_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2,
                trace_t *trace);
int __cdecl CM_ClipSightTraceToEntities(sightclip_t *clip);
int __cdecl CM_ClipSightTraceToEntities_r(
                const sightclip_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2);
void __cdecl CM_PointTraceToEntities(pointtrace_t *clip, trace_t *trace, col_context_t *context);
void __cdecl CM_PointTraceToEntities_r(
                pointtrace_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2,
                trace_t *trace,
                col_context_t *context);
int __cdecl CM_PointSightTraceToEntities(sightpointtrace_t *clip);
int __cdecl CM_PointSightTraceToEntities_r(
                const sightpointtrace_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2);


static const float PREDICTION_TRACE_MIN[3] = { -15.0, -15.0, 0.0 };
#ifdef KISAK_SP
// retail SP 0x00a52020: { 15, 15, 72 } (SP actors are 72 tall, see SP_actor @ 004f5a70)
static const float PREDICTION_TRACE_MAX[3] = { 15.0, 15.0, 72.0 };
#else
static const float PREDICTION_TRACE_MAX[3] = { 15.0, 15.0, 48.0 };
#endif

