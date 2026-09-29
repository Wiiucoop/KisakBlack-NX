#include "r_staticmodel_load_obj.h"
#include <universal/com_memory.h>
#include <xanim/xmodel.h>
#include <xanim/xmodel_load_obj.h>
#include <xanim/xmodel_utils.h>
#include "r_init.h"
#include "r_bsp.h"
#include "r_reflection_probe.h"

void __cdecl R_AllocStaticModels(GfxAabbTree *tree)
{
    GfxAabbTree *children; // [esp+0h] [ebp-Ch]
    int childIndex; // [esp+4h] [ebp-8h]
    unsigned __int8 *smodelIndexes; // [esp+8h] [ebp-4h]

    if ( tree->smodelIndexCount )
    {
        smodelIndexes = Hunk_AllocAlign(2 * tree->smodelIndexCount, 4, "R_AllocStaticModels", 22);
        memcpy(smodelIndexes, (unsigned __int8 *)tree->smodelIndexes, 2 * tree->smodelIndexCount);
        tree->smodelIndexes = (unsigned __int16 *)smodelIndexes;
    }
    children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
    for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
        R_AllocStaticModels(&children[childIndex]);
}

void __cdecl R_SortGfxAabbTree(GfxWorld *world, GfxAabbTree *tree)
{
    int j; // [esp+Ch] [ebp-78h]
    int ja; // [esp+Ch] [ebp-78h]
    float middle; // [esp+10h] [ebp-74h]
    float middle_4; // [esp+14h] [ebp-70h]
    GfxAabbTree *children; // [esp+1Ch] [ebp-68h]
    int smodelIndexIter; // [esp+20h] [ebp-64h]
    GfxAabbTree *childTreeb; // [esp+24h] [ebp-60h]
    GfxAabbTree *childTree; // [esp+24h] [ebp-60h]
    GfxAabbTree *childTreea; // [esp+24h] [ebp-60h]
    float mins[3]; // [esp+28h] [ebp-5Ch]
    float childMaxs[3]; // [esp+34h] [ebp-50h] BYREF
    unsigned __int16 *smodelIndexes; // [esp+40h] [ebp-44h]
    int childIndex; // [esp+44h] [ebp-40h]
    GfxStaticModelInst *smodelInst; // [esp+48h] [ebp-3Ch]
    float childMins[3]; // [esp+4Ch] [ebp-38h] BYREF
    float maxs[3]; // [esp+58h] [ebp-2Ch]
    int smodelIndexCount; // [esp+64h] [ebp-20h]
    int i; // [esp+68h] [ebp-1Ch]
    int childCount[4]; // [esp+6Ch] [ebp-18h]
    int count; // [esp+7Ch] [ebp-8h]
    int smodelIndex; // [esp+80h] [ebp-4h]

    qsort(
        tree->smodelIndexes,
        tree->smodelIndexCount,
        2u,
        (int (__cdecl *)(const void *, const void *))CompareStaticModels);
    if ( tree->childCount )
    {
        children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
        for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
            R_SortGfxAabbTree(world, &children[childIndex]);
    }
    else
    {
        mins[0] = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;
        maxs[0] = -FLT_MAX;
        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        for ( smodelIndexIter = 0; smodelIndexIter < tree->smodelIndexCount; ++smodelIndexIter )
        {
            smodelIndex = tree->smodelIndexes[smodelIndexIter];
            smodelInst = &world->dpvs.smodelInsts[smodelIndex];
            for ( i = 0; i < 3; ++i )
            {
                if ( mins[i] > smodelInst->mins[i] )
                    mins[i] = smodelInst->mins[i];
                if ( smodelInst->maxs[i] > maxs[i] )
                    maxs[i] = smodelInst->maxs[i];
            }
        }
        if ( !tree->surfaceCount )
        {
            tree->mins[0] = mins[0];
            tree->mins[1] = mins[1];
            tree->mins[2] = mins[2];
            tree->maxs[0] = maxs[0];
            tree->maxs[1] = maxs[1];
            tree->maxs[2] = maxs[2];
        }
        if ( tree->smodelIndexCount >= 8u )
        {
            middle = 0.5 * (float)(mins[0] + maxs[0]);
            middle_4 = 0.5 * (float)(mins[1] + maxs[1]);
            smodelIndexes = tree->smodelIndexes;
            smodelIndexCount = tree->smodelIndexCount;
            childMins[0] = mins[0];
            childMins[1] = mins[1];
            childMins[2] = mins[2];
            childMaxs[1] = maxs[1];
            childMaxs[2] = maxs[2];
            childMaxs[0] = middle;
            childCount[0] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[0];
            smodelIndexCount -= childCount[0];
            childMins[1] = mins[1];
            childMins[2] = mins[2];
            childMaxs[0] = maxs[0];
            childMaxs[1] = maxs[1];
            childMaxs[2] = maxs[2];
            childMins[0] = middle;
            childCount[1] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[1];
            smodelIndexCount -= childCount[1];
            childMins[0] = mins[0];
            childMins[1] = mins[1];
            childMins[2] = mins[2];
            childMaxs[0] = maxs[0];
            childMaxs[2] = maxs[2];
            childMaxs[1] = middle_4;
            childCount[2] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[2];
            smodelIndexCount -= childCount[2];
            childMins[0] = mins[0];
            childMins[2] = mins[2];
            childMaxs[0] = maxs[0];
            childMaxs[1] = maxs[1];
            childMaxs[2] = maxs[2];
            childMins[1] = middle_4;
            childCount[3] = R_SortGfxAabbTreeChildren(world, childMins, childMaxs, smodelIndexes, smodelIndexCount);
            smodelIndexes += childCount[3];
            smodelIndexCount -= childCount[3];
            count = 0;
            for ( j = 0; j < 4; ++j )
                count += childCount[j] != 0;
            if ( count )
            {
                if ( tree->surfaceCount )
                    ++count;
                if ( smodelIndexCount )
                    ++count;
                tree->childrenOffset = Hunk_AllocAlign(sizeof(GfxAabbTree) * count, 8, "R_SortGfxAabbTree", 22) - (unsigned __int8 *)tree;
                if ( tree->surfaceCount )
                {
                    childTreeb = (GfxAabbTree *)((char *)&tree[tree->childCount++] + tree->childrenOffset);
                    childTreeb->mins[0] = tree->mins[0];
                    childTreeb->mins[1] = tree->mins[1];
                    childTreeb->mins[2] = tree->mins[2];
                    childTreeb->maxs[0] = tree->maxs[0];
                    childTreeb->maxs[1] = tree->maxs[1];
                    childTreeb->maxs[2] = tree->maxs[2];
                    childTreeb->startSurfIndex = tree->startSurfIndex;
                    childTreeb->surfaceCount = tree->surfaceCount;
                }
                smodelIndexes = tree->smodelIndexes;
                smodelIndexCount = tree->smodelIndexCount;
                for ( ja = 0; ja < 4; ++ja )
                {
                    count = childCount[ja];
                    if ( count )
                    {
                        childTree = (GfxAabbTree *)((char *)&tree[tree->childCount++] + tree->childrenOffset);
                        childTree->smodelIndexCount = count;
                        if ( childTree->smodelIndexCount != count
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                                        266,
                                        0,
                                        "%s",
                                        "childTree->smodelIndexCount == count") )
                        {
                            __debugbreak();
                        }
                        childTree->smodelIndexes = smodelIndexes;
                        R_SortGfxAabbTree(world, childTree);
                        smodelIndexes += count;
                        smodelIndexCount -= count;
                    }
                }
                if ( smodelIndexCount )
                {
                    childTreea = (GfxAabbTree *)((char *)&tree[tree->childCount++] + tree->childrenOffset);
                    childTreea->smodelIndexCount = smodelIndexCount;
                    if ( childTreea->smodelIndexCount != smodelIndexCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                                    284,
                                    0,
                                    "%s",
                                    "childTree->smodelIndexCount == smodelIndexCount") )
                    {
                        __debugbreak();
                    }
                    childTreea->smodelIndexes = smodelIndexes;
                    R_SortGfxAabbTree(world, childTreea);
                }
            }
        }
    }
}

int __cdecl R_SortGfxAabbTreeChildren(
                GfxWorld *world,
                float *mins,
                float *maxs,
                unsigned __int16 *staticModels,
                int staticModelCount)
{
    GfxStaticModelInst *smodelInst; // [esp+0h] [ebp-14h]
    int smodelSwapIndex; // [esp+4h] [ebp-10h]
    int childCount; // [esp+8h] [ebp-Ch]
    int smodelChildIndex; // [esp+Ch] [ebp-8h]
    int smodelIndex; // [esp+10h] [ebp-4h]

    childCount = 0;
    for ( smodelChildIndex = 0; smodelChildIndex < staticModelCount; ++smodelChildIndex )
    {
        smodelIndex = staticModels[smodelChildIndex];
        smodelInst = &world->dpvs.smodelInsts[smodelIndex];
        if ( *mins <= smodelInst->mins[0]
            && mins[1] <= smodelInst->mins[1]
            && mins[2] <= smodelInst->mins[2]
            && smodelInst->maxs[0] <= *maxs
            && smodelInst->maxs[1] <= maxs[1]
            && smodelInst->maxs[2] <= maxs[2] )
        {
            smodelSwapIndex = staticModels[childCount];
            staticModels[childCount] = smodelIndex;
            if ( staticModels[childCount] != smodelIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                            79,
                            0,
                            "%s",
                            "staticModels[childCount] == smodelIndex") )
            {
                __debugbreak();
            }
            staticModels[smodelChildIndex] = smodelSwapIndex;
            if ( staticModels[smodelChildIndex] != smodelSwapIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                            82,
                            0,
                            "%s",
                            "staticModels[smodelChildIndex] == smodelSwapIndex") )
            {
                __debugbreak();
            }
            ++childCount;
        }
    }
    return childCount < 2 ? 0 : childCount;
}

int __cdecl CompareStaticModels(unsigned __int16 *smodel0, unsigned __int16 *smodel1)
{
    return *smodel0 - *smodel1;
}

void __cdecl R_FilterStaticModelIntoCells_r(
                GfxWorld *world,
                mnode_t *node,
                GfxStaticModelInst *smodelInst,
                const float *mins,
                const float *maxs)
{
    const char *NameFromStaticModelInst; // eax
    const char *v6; // eax
    double v7; // [esp+0h] [ebp-70h]
    double v8; // [esp+8h] [ebp-68h]
    double v9; // [esp+10h] [ebp-60h]
    const cplane_s *v10; // [esp+18h] [ebp-58h]
    float v11; // [esp+1Ch] [ebp-54h]
    float localmaxs[3]; // [esp+20h] [ebp-50h]
    float dist; // [esp+2Ch] [ebp-44h]
    float localmins[3]; // [esp+30h] [ebp-40h] BYREF
    unsigned int type; // [esp+3Ch] [ebp-34h]
    cplane_s *plane; // [esp+40h] [ebp-30h]
    int cellIndex; // [esp+44h] [ebp-2Ch]
    int boxSide; // [esp+48h] [ebp-28h]
    float mins2[3]; // [esp+4Ch] [ebp-24h] BYREF
    int cellCount; // [esp+58h] [ebp-18h]
    float maxs2[3]; // [esp+5Ch] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+68h] [ebp-8h]
    int planeIndex; // [esp+6Ch] [ebp-4h]

    cellCount = world->dpvsPlanes.cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - (world->dpvsPlanes.cellCount + 1);
        if ( planeIndex < 0 )
            break;
        plane = &world->dpvsPlanes.planes[planeIndex];
        boxSide = BoxOnPlaneSide(mins2, maxs2, plane);
        if ( boxSide == 3 )
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if ( type >= 3 )
            {
                R_FilterStaticModelIntoCells_r(world, node + 1, smodelInst, mins2, maxs2);
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[plane->type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[plane->type] = dist;
                if ( BoxOnPlaneSide(localmins, maxs2, plane) != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                                517,
                                0,
                                "%s",
                                "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT") )
                {
                    __debugbreak();
                }
                if ( maxs2[plane->type] > dist )
                    R_FilterStaticModelIntoCells_r(world, node + 1, smodelInst, localmins, maxs2);
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            if ( boxSide != 1 && boxSide != 2 )
            {
                v9 = smodelInst->lightingOrigin[2];
                v8 = smodelInst->lightingOrigin[1];
                v7 = smodelInst->lightingOrigin[0];
                NameFromStaticModelInst = GetNameFromStaticModelInst(smodelInst);
                v6 = va(
                             "boxside of %d in a static model named %s, may be a bad bounding box at or around (%f,%f,%f).\n",
                             boxSide,
                             NameFromStaticModelInst,
                             v7,
                             v8,
                             v9);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                                546,
                                0,
                                "%s\n\t%s",
                                "boxSide == BOXSIDE_FRONT || boxSide == BOXSIDE_BACK",
                                v6) )
                    __debugbreak();
            }
            node = (mnode_t *)((char *)node + 2 * (boxSide - 1) * (node->rightChildOffset - 2) + 4);
        }
    }
    if ( cellIndex )
        R_AddStaticModelToCell(world, smodelInst, cellIndex - 1);
}

void __cdecl R_AddStaticModelToCell(GfxWorld *world, GfxStaticModelInst *smodelInst, int cellIndex)
{
    GfxCell *cell; // [esp+0h] [ebp-Ch]
    GfxAabbTree *tree; // [esp+4h] [ebp-8h]
    int smodelIndex; // [esp+8h] [ebp-4h]

    if ( !smodelInst
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    416,
                    0,
                    "%s",
                    "smodelInst") )
    {
        __debugbreak();
    }
    if ( (cellIndex < 0 || cellIndex >= world->dpvsPlanes.cellCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    417,
                    0,
                    "%s",
                    "cellIndex >= 0 && cellIndex < world->dpvsPlanes.cellCount") )
    {
        __debugbreak();
    }
    cell = &world->cells[cellIndex];
    smodelIndex = smodelInst - world->dpvs.smodelInsts;
    tree = cell->aabbTree;
    if ( !tree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp", 422, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    if ( !tree->smodelIndexCount || tree->smodelIndexes[tree->smodelIndexCount - 1] != smodelIndex )
        R_AddStaticModelToAabbTree_r(world, cell->aabbTree, smodelIndex);
}

void __cdecl R_AddStaticModelToAabbTree_r(GfxWorld *world, GfxAabbTree *tree, int smodelIndex)
{
    int v3; // [esp+4h] [ebp-30h]
    unsigned __int8 *children; // [esp+14h] [ebp-20h]
    GfxAabbTree *childTree; // [esp+18h] [ebp-1Ch]
    GfxAabbTree *childTreea; // [esp+18h] [ebp-1Ch]
    unsigned __int8 *newChildren; // [esp+1Ch] [ebp-18h]
    int childIndex; // [esp+20h] [ebp-14h]
    int childIndexa; // [esp+20h] [ebp-14h]
    int childIndexb; // [esp+20h] [ebp-14h]
    unsigned __int8 *smodelIndexes; // [esp+24h] [ebp-10h]
    GfxStaticModelInst *smodelInst; // [esp+28h] [ebp-Ch]
    int i; // [esp+30h] [ebp-4h]

    if ( ((tree->smodelIndexCount - 1) & tree->smodelIndexCount) == 0 )
    {
        if ( tree->smodelIndexCount )
            v3 = 2 * tree->smodelIndexCount;
        else
            v3 = 1;
        smodelIndexes = (unsigned __int8 *)Hunk_AllocateTempMemory(2 * v3, "R_AddModelToCell");
        memcpy(smodelIndexes, (unsigned __int8 *)tree->smodelIndexes, 2 * tree->smodelIndexCount);
        tree->smodelIndexes = (unsigned __int16 *)smodelIndexes;
    }
    tree->smodelIndexes[tree->smodelIndexCount] = smodelIndex;
    if ( tree->smodelIndexes[tree->smodelIndexCount] != smodelIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    318,
                    0,
                    "%s",
                    "tree->smodelIndexes[tree->smodelIndexCount] == smodelIndex") )
    {
        __debugbreak();
    }
    ++tree->smodelIndexCount;
    if ( tree->childCount )
    {
        smodelInst = &world->dpvs.smodelInsts[smodelIndex];
        for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
        {
            childTree = (GfxAabbTree *)((char *)&tree[childIndex] + tree->childrenOffset);
            if ( childTree->mins[0] <= smodelInst->mins[0]
                && childTree->mins[1] <= smodelInst->mins[1]
                && childTree->mins[2] <= smodelInst->mins[2]
                && smodelInst->maxs[0] <= childTree->maxs[0]
                && smodelInst->maxs[1] <= childTree->maxs[1]
                && smodelInst->maxs[2] <= childTree->maxs[2] )
            {
                goto LABEL_19;
            }
        }
        for ( childIndexa = 0; ; ++childIndexa )
        {
            if ( childIndexa >= tree->childCount )
            {
                newChildren = Hunk_AllocAlign(sizeof(GfxAabbTree) * (tree->childCount + 1), 8, "R_AddStaticModelToAabbTree_r", 22);
                children = (unsigned __int8 *)tree + tree->childrenOffset;
                memcpy(newChildren, children, sizeof(GfxAabbTree) * tree->childCount);
                tree->childrenOffset = newChildren - (unsigned __int8 *)tree;
                for ( childIndexb = 0; childIndexb < tree->childCount; ++childIndexb )
                    {
                        GfxAabbTree *oldChild = (GfxAabbTree *)children + childIndexb;
                        GfxAabbTree *newChild = (GfxAabbTree *)newChildren + childIndexb;
                        newChild->childrenOffset = (int)((char *)oldChild + oldChild->childrenOffset - (char *)newChild);
                    }
                childTreea = (GfxAabbTree *)((char *)&tree[tree->childCount++] + tree->childrenOffset);
                childTreea->mins[0] = smodelInst->mins[0];
                childTreea->mins[1] = smodelInst->mins[1];
                childTreea->mins[2] = smodelInst->mins[2];
                childTreea->maxs[0] = smodelInst->maxs[0];
                childTreea->maxs[1] = smodelInst->maxs[1];
                childTreea->maxs[2] = smodelInst->maxs[2];
                R_AddStaticModelToAabbTree_r(world, childTreea, smodelIndex);
                return;
            }
            childTree = (GfxAabbTree *)((char *)&tree[childIndexa] + tree->childrenOffset);
            if ( !childTree->surfaceCount )
                break;
        }
        for ( i = 0; i < 3; ++i )
        {
            if ( childTree->mins[i] > smodelInst->mins[i] )
                childTree->mins[i] = smodelInst->mins[i];
            if ( smodelInst->maxs[i] > childTree->maxs[i] )
                childTree->maxs[i] = smodelInst->maxs[i];
        }
LABEL_19:
        R_AddStaticModelToAabbTree_r(world, childTree, smodelIndex);
    }
}

const char *__cdecl GetNameFromStaticModelInst(const GfxStaticModelInst *smodelInst)
{
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+4h] [ebp-8h]

    smodelDrawInst = &s_world.dpvs.smodelDrawInsts[smodelInst - s_world.dpvs.smodelInsts];
    if ( smodelDrawInst && smodelDrawInst->model && smodelDrawInst->model->name )
        return smodelDrawInst->model->name;
    else
        return "unknown";
}

void __cdecl R_SetStaticModelReflectionProbe(
                const GfxWorld *world,
                const GfxStaticModelInst *smodelInst,
                GfxStaticModelDrawInst *smodelDrawInst)
{
    unsigned int reflectionProbeIndex; // [esp+4h] [ebp-10h]
    float center[3]; // [esp+8h] [ebp-Ch] BYREF

    center[0] = (float)(smodelInst->mins[0] + smodelInst->maxs[0]) * 0.5;
    center[1] = (float)(smodelInst->mins[1] + smodelInst->maxs[1]) * 0.5;
    center[2] = (float)(smodelInst->mins[2] + smodelInst->maxs[2]) * 0.5;
    reflectionProbeIndex = R_CalcReflectionProbeIndex(world, center);
    smodelDrawInst->reflectionProbeIndex = reflectionProbeIndex;
    if ( smodelDrawInst->reflectionProbeIndex != reflectionProbeIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    671,
                    0,
                    "%s",
                    "smodelDrawInst->reflectionProbeIndex == reflectionProbeIndex") )
    {
        __debugbreak();
    }
}

unsigned int __cdecl R_FindNearestReflectionProbe_0(const GfxWorld *world, const float *origin)
{
    float bestProbeDist; // [esp+Ch] [ebp-Ch]
    unsigned __int8 bestProbe; // [esp+12h] [ebp-6h]
    unsigned __int8 probeIndex; // [esp+13h] [ebp-5h]
    float testProbeDist; // [esp+14h] [ebp-4h]

    if (world->draw.reflectionProbeCount >= 0xFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
            594,
            0,
            "%s",
            "world->draw.reflectionProbeCount < 0xff"))
    {
        __debugbreak();
    }
    bestProbe = 0;
    //bestProbeDist = FLOAT_3_4028235e38;
    bestProbeDist = FLT_MAX;
    for (probeIndex = 1; probeIndex < world->draw.reflectionProbeCount; ++probeIndex)
    {
        testProbeDist = Vec3DistanceSq(world->draw.reflectionProbes[probeIndex].origin, origin);
        if (bestProbeDist > testProbeDist)
        {
            bestProbeDist = testProbeDist;
            bestProbe = probeIndex;
        }
    }
    return bestProbe;
}

unsigned int __cdecl R_FindNearestReflectionProbeInCell_0(
        const GfxWorld *world,
        const GfxCell *cell,
        const float *origin)
{
  float bestProbeDist; // [esp+Ch] [ebp-10h]
  unsigned __int8 bestProbe; // [esp+12h] [ebp-Ah]
  unsigned __int8 probeIndex; // [esp+13h] [ebp-9h]
  float testProbeDist; // [esp+14h] [ebp-8h]
  unsigned int cellProbeIndex; // [esp+18h] [ebp-4h]

  if ( world->draw.reflectionProbeCount >= 0xFF
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
          563,
          0,
          "%s",
          "world->draw.reflectionProbeCount < 0xff") )
  {
    __debugbreak();
  }
  bestProbe = 0;
  //bestProbeDist = FLOAT_3_4028235e38;
  bestProbeDist = FLT_MAX;
  if ( !cell->reflectionProbeCount
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
          569,
          0,
          "%s",
          "cell->reflectionProbeCount > 0") )
  {
    __debugbreak();
  }
  for ( cellProbeIndex = 0; cellProbeIndex < cell->reflectionProbeCount; ++cellProbeIndex )
  {
    probeIndex = cell->reflectionProbes[cellProbeIndex];
    if ( probeIndex >= world->draw.reflectionProbeCount
      && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
            573,
            0,
            "probeIndex doesn't index world->draw.reflectionProbeCount\n\t%i not in [0, %i)",
            probeIndex,
            world->draw.reflectionProbeCount) )
    {
      __debugbreak();
    }
    testProbeDist = Vec3DistanceSq(world->draw.reflectionProbes[probeIndex].origin, origin);
    if ( bestProbeDist > testProbeDist )
    {
      bestProbeDist = testProbeDist;
      bestProbe = probeIndex;
    }
  }
  return bestProbe;
}

unsigned int __cdecl R_CalcReflectionProbeIndex(const GfxWorld *world, const float *origin)
{
    unsigned int cellIndex; // [esp+0h] [ebp-4h]

    cellIndex = R_CellForPoint(world, origin);
    if ( cellIndex == -1 )
        return R_FindNearestReflectionProbe_0(world, origin);
    if ( cellIndex >= world->dpvsPlanes.cellCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    658,
                    0,
                    "cellIndex doesn't index world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                    cellIndex,
                    world->dpvsPlanes.cellCount) )
    {
        __debugbreak();
    }
    return R_FindNearestReflectionProbeInCell_0(world, &world->cells[cellIndex], origin);
}

int __cdecl R_CellForPoint(const GfxWorld *world, const float *origin)
{
    cplane_s *v2; // edx
    mnode_t *node; // [esp+4h] [ebp-1Ch]
    int cellIndex; // [esp+14h] [ebp-Ch]
    int cellCount; // [esp+18h] [ebp-8h]

    if ( !world
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    626,
                    0,
                    "%s",
                    "world") )
    {
        __debugbreak();
    }
    node = (mnode_t *)world->dpvsPlanes.nodes;
    cellCount = world->dpvsPlanes.cellCount + 1;
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        if ( cellIndex - cellCount < 0 )
            break;
        v2 = &world->dpvsPlanes.planes[cellIndex - cellCount];
        node = (mnode_t *)((char *)node
                                         + 2
                                         * ((float)((float)((float)((float)(*origin * v2->normal[0]) + (float)(origin[1] * v2->normal[1]))
                                                                            + (float)(origin[2] * v2->normal[2]))
                                                            - v2->dist) <= 0.0)
                                         * (node->rightChildOffset - 2)
                                         + 4);
    }
    return cellIndex - 1;
}

void __cdecl R_CreateStaticModel(
                XModel *model,
                const float *origin,
                const float (*axis)[3],
                float scale,
                GfxStaticModelDrawInst *smodelDrawInst,
                GfxStaticModelInst *smodelInst,
                unsigned int staticModelFlags)
{
    smodelDrawInst->model = model;
    smodelDrawInst->placement.origin[0] = *origin;
    smodelDrawInst->placement.origin[1] = origin[1];
    smodelDrawInst->placement.origin[2] = origin[2];
    AxisCopy(axis, smodelDrawInst->placement.axis);
    smodelDrawInst->placement.scale = scale;
    R_GetXModelBounds(model, axis, smodelInst->mins, smodelInst->maxs);
    smodelInst->mins[0] = (float)(scale * smodelInst->mins[0]) + *origin;
    smodelInst->mins[1] = (float)(scale * smodelInst->mins[1]) + origin[1];
    smodelInst->mins[2] = (float)(scale * smodelInst->mins[2]) + origin[2];
    smodelInst->maxs[0] = (float)(scale * smodelInst->maxs[0]) + *origin;
    smodelInst->maxs[1] = (float)(scale * smodelInst->maxs[1]) + origin[1];
    smodelInst->maxs[2] = (float)(scale * smodelInst->maxs[2]) + origin[2];
    if ( scale == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodel_load_obj.cpp",
                    686,
                    0,
                    "%s",
                    "scale") )
    {
        __debugbreak();
    }
    smodelDrawInst->cullDist = XModelGetLodOutDist(model) * scale;
    smodelDrawInst->reflectionProbeIndex = 0;
    smodelDrawInst->flags = staticModelFlags;
}

