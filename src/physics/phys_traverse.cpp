#include "phys_gjk.h"
#include "phys_traverse.h"

#include <qcommon/cm_test.h>
#include <qcommon/common.h>
#include <qcommon/cm_load.h>

#include <gfx_d3d/r_material.h>


colgeom_visitor_t::colgeom_visitor_t()
{
    //this->__vftable = (colgeom_visitor_t_vtbl *)&visitor_base_t::`vftable';
    //this->__vftable = (colgeom_visitor_t_vtbl *)&colgeom_visitor_t::`vftable';
    this->m_mn.vec.u[0] = 0;
    this->m_mn.vec.u[1] = 0;
    this->m_mn.vec.u[2] = 0;
    this->m_mn.vec.u[3] = 0;
    this->m_mx.vec.u[0] = 0;
    this->m_mx.vec.u[1] = 0;
    this->m_mx.vec.u[2] = 0;
    this->m_mx.vec.u[3] = 0;
    this->m_p0.vec.u[0] = 0;
    this->m_p0.vec.u[1] = 0;
    this->m_p0.vec.u[2] = 0;
    this->m_p0.vec.u[3] = 0;
    this->m_p1.vec.u[0] = 0;
    this->m_p1.vec.u[1] = 0;
    this->m_p1.vec.u[2] = 0;
    this->m_p1.vec.u[3] = 0;
    this->m_delta.vec.u[0] = 0;
    this->m_delta.vec.u[1] = 0;
    this->m_delta.vec.u[2] = 0;
    this->m_delta.vec.u[3] = 0;
    this->m_rvec.vec.u[0] = 0;
    this->m_rvec.vec.u[1] = 0;
    this->m_rvec.vec.u[2] = 0;
    this->m_rvec.vec.u[3] = 0;
}

colgeom_visitor_t::colgeom_visitor_t(const colgeom_visitor_t *__that)
{
    //this->__vftable = (colgeom_visitor_t_vtbl *)&visitor_base_t::`vftable';
    //    this->__vftable = (colgeom_visitor_t_vtbl *)&colgeom_visitor_t::`vftable';
    this->m_mn = __that->m_mn;
    this->m_mx = __that->m_mx;
    this->m_p0 = __that->m_p0;
    this->m_p1 = __that->m_p1;
    this->m_delta = __that->m_delta;
    this->m_rvec = __that->m_rvec;
    this->m_radius = __that->m_radius;
    this->m_mask = __that->m_mask;
    this->m_threadInfo = __that->m_threadInfo;
}


void static_colgeom_visitor_t::visit(const CollisionAabbTree *tree)
{
    if ( this->ntrees < 512 )
        this->trees[this->ntrees++] = tree;
}

void static_colgeom_visitor_t::visit(const cbrush_t *brush)
{
    int i; // [esp+4h] [ebp-4h]

    if ( this->nbrushes < 512 )
    {
        for ( i = 0; i < this->nbrushes; ++i )
        {
            if ( brush == this->brushes[i] )
                return;
        }
        this->brushes[this->nbrushes++] = brush;
    }
}

void static_colgeom_visitor_t::update(
                const float *_mn,
                const float *_mx,
                int mask,
                const float *expand_vec)
{
    float result[3]; // [esp+10h] [ebp-40h] BYREF
    float b[3]; // [esp+1Ch] [ebp-34h] BYREF
    float a[3]; // [esp+28h] [ebp-28h] BYREF
    bool inside; // [esp+37h] [ebp-19h]
    float mx[3]; // [esp+38h] [ebp-18h] BYREF
    float mn[3]; // [esp+44h] [ebp-Ch] BYREF

    a[0] = this->m_mn.vec.v[0] - _mn[0];
    a[1] = this->m_mn.vec.v[1] - _mn[1];
    a[2] = this->m_mn.vec.v[2] - _mn[2];

    b[0] = _mx[0] - this->m_mx.vec.v[0];
    b[1] = _mx[1] - this->m_mx.vec.v[1];
    b[2] = _mx[2] - this->m_mx.vec.v[2];

    Vec3Max(a, b, result);
    inside = result[0] < 0.0 && result[1] < 0.0 && result[2] < 0.0;

    if ( this->m_mask != mask || !inside )
    {
        mn[0] = _mn[0] - expand_vec[0];
        mn[1] = _mn[1] - expand_vec[1];
        mn[2] = _mn[2] - expand_vec[2];

        mx[0] = _mx[0] + expand_vec[0];
        mx[1] = _mx[1] + expand_vec[1];
        mx[2] = _mx[2] + expand_vec[2];

        this->prolog();

        colgeom_visitor_t::intersect_box(mn, mx, mask);
    }
}


void colgeom_visitor_t::intersect_box(float *mn, float *mx, int mask)
{

    this->m_mn.vec.v[0] = mn[0];
    this->m_mn.vec.v[1] = mn[1];
    this->m_mn.vec.v[2] = mn[2];

    this->m_mx.vec.v[0] = mx[0];

    this->m_mx.vec.v[1] = mx[1];
    this->m_mx.vec.v[2] = mx[2];
    this->m_mask = mask;

    leafList_s ll; // [esp+20h] [ebp-1048h] BYREF
    _WORD leaflist[2048]; // [esp+60h] [ebp-1008h] BYREF

    ll.bounds[0][0] = this->m_mn.vec.v[0];
    ll.bounds[0][1] = this->m_mn.vec.v[1];
    ll.bounds[0][2] = this->m_mn.vec.v[2];
    ll.bounds[1][0] = this->m_mx.vec.v[0];
    ll.bounds[1][1] = this->m_mx.vec.v[1];
    ll.bounds[1][2] = this->m_mx.vec.v[2];

    ll.count = 0;
    ll.maxcount = 2048;
    ll.list = leaflist;
    ll.lastLeaf = 0;
    ll.overflowed = 0;
    CM_BoxLeafnums_r(&ll, 0);

    if (!ll.count)
    {
        return;
    }

    if (ll.overflowed)
    {
#ifdef KISAK_SP
        // The retail walk bounds-checks the 2048-entry array, so an overflow
        // here is truncation rather than a write past leaflist. Preserve the
        // warning and capture the exact query that preceded the SP crash.
        Com_Printf(
            0,
            "colgeom_visitor_t::intersect_box: leafList overflow (max %d) count=%d mask=0x%08X mn=(%.3f %.3f %.3f) mx=(%.3f %.3f %.3f)\n",
            ll.maxcount,
            ll.count,
            mask,
            mn[0],
            mn[1],
            mn[2],
            mx[0],
            mx[1],
            mx[2]);
#else
        Com_Printf(0, "colgeom_visitor_t::intersect_box: leafList overflow (max %d)\n", 2048);
#endif
    }

    TraceThreadInfo threadInfo;
    this->m_threadInfo = &threadInfo;
    CM_GetTraceThreadInfo(this->m_threadInfo);

    if (!this->m_threadInfo->checkcount.partitions && cm.partitionCount)
    {
        return;
    }

    for (int i = 0; i < ll.count; ++i)
    {
        cLeaf_s *leaf = &cm.leafs[(unsigned __int16)leaflist[i]];

        if ((this->m_mask & leaf->brushContents) != 0)
            colgeom_visitor_t::intersect_box_brushes(leaf);

        if ((this->m_mask & leaf->terrainContents) != 0)
            colgeom_visitor_t::intersect_box_partitions(leaf);
    }
}


void colgeom_visitor_t::intersect_box_brushes(cLeaf_s *leaf)
{
    if (leaf->leafBrushNode)
    {
        if (this->m_mx.vec.v[0] >= leaf->mins[0]
            && this->m_mx.vec.v[1] >= leaf->mins[1]
            && this->m_mx.vec.v[2] >= leaf->mins[2]
            && leaf->maxs[0] >= this->m_mn.vec.v[0]
            && leaf->maxs[1] >= this->m_mn.vec.v[1]
            && leaf->maxs[2] >= this->m_mn.vec.v[2])
        {
            colgeom_visitor_t::intersect_box_brushnode(&cm.leafbrushNodes[leaf->leafBrushNode]);
        }
    }
}

void colgeom_visitor_t::intersect_box_partitions(cLeaf_s *leaf)
{
    CollisionAabbTree *aabbTree; // [esp+8h] [ebp-8h]
    int k; // [esp+Ch] [ebp-4h]

    for (k = 0; k < leaf->collAabbCount; ++k)
    {
        aabbTree = &cm.aabbTrees[k + leaf->firstCollAabbIndex];
        if ((cm.materials[aabbTree->materialIndex].contentFlags & this->m_mask) != 0)
            colgeom_visitor_t::intersect_box_partitions_r(aabbTree);
    }
}

void colgeom_visitor_t::intersect_box_brushnode(cLeafBrushNode_s *node)
{
    cbrush_t *b; // [esp+14h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-4h]

    while ((this->m_mask & node->contents) != 0)
    {
        if (node->leafBrushCount)
        {
            if (node->leafBrushCount > 0) // KISAKTODO: redundant if()?
            {
                for (i = 0; i < node->leafBrushCount; ++i)
                {
                    b = &cm.brushes[node->data.leaf.brushes[i]];
                    if ((this->m_mask & b->contents) != 0
                        && this->m_mx.vec.v[0] >= b->mins[0]
                        && this->m_mx.vec.v[1] >= b->mins[1]
                        && this->m_mx.vec.v[2] >= b->mins[2]
                        && b->maxs[0] >= this->m_mn.vec.v[0]
                        && b->maxs[1] >= this->m_mn.vec.v[1]
                        && b->maxs[2] >= this->m_mn.vec.v[2])
                    {
                        this->visit(b);
                    }
                }
                return;
            }
            colgeom_visitor_t::intersect_box_brushnode(node + 1);
        }
        if (this->m_mn.vec.v[node->axis] <= node->data.children.dist)
        {
            if (this->m_mx.vec.v[node->axis] >= node->data.children.dist)
                colgeom_visitor_t::intersect_box_brushnode(&node[node->data.children.childOffset[0]]);
            node += node->data.children.childOffset[1];
        }
        else
        {
            node += node->data.children.childOffset[0];
        }
    }
}

void colgeom_visitor_t::intersect_box_partitions_r(CollisionAabbTree *aabbTree)
{
    bool v3; // [esp+7h] [ebp-25h]
    int childIndex; // [esp+18h] [ebp-14h]
    int partitionIndex; // [esp+1Ch] [ebp-10h]
    CollisionAabbTree *child; // [esp+20h] [ebp-Ch]
    int checkStamp; // [esp+24h] [ebp-8h]

    if (this->m_mn.vec.v[0] <= (float)(aabbTree->origin[0] + aabbTree->halfSize[0]))
    {
        if (this->m_mn.vec.v[1] <= (float)(aabbTree->origin[1] + aabbTree->halfSize[1]))
        {
            if (this->m_mn.vec.v[2] <= (float)(aabbTree->origin[2] + aabbTree->halfSize[2]))
                v3 = (float)(aabbTree->origin[0] - aabbTree->halfSize[0]) <= this->m_mx.vec.v[0]
                && (float)(aabbTree->origin[1] - aabbTree->halfSize[1]) <= this->m_mx.vec.v[1]
                && (float)(aabbTree->origin[2] - aabbTree->halfSize[2]) <= this->m_mx.vec.v[2];
            else
                v3 = 0;
        }
        else
        {
            v3 = 0;
        }
    }
    else
    {
        v3 = 0;
    }
    if (v3)
    {
        if (aabbTree->childCount)
        {
            childIndex = 0;
            child = &cm.aabbTrees[aabbTree->u.firstChildIndex];
            while (childIndex < aabbTree->childCount)
            {
                colgeom_visitor_t::intersect_box_partitions_r(child);
                ++childIndex;
                ++child;
            }
        }
        else
        {
            partitionIndex = aabbTree->u.firstChildIndex;
            checkStamp = this->m_threadInfo->checkcount.global;
            if (this->m_threadInfo->checkcount.partitions[partitionIndex] != checkStamp)
            {
                this->m_threadInfo->checkcount.partitions[partitionIndex] = checkStamp;
                this->visit(aabbTree);
            }
        }
    }
}