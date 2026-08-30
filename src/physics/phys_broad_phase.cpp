#include "phys_broad_phase.h"
#include "phys_main.h"
#include <bgame/bg_slidemove.h>
#include "phys_assert.h"
#include "phys_transient_allocator.h"
#include "phys_task_manager.h"

#include <algorithm>
#include <tl/tl_system.h>
#include "phys_collision_multithreaded.h"



broad_phase_memory *G_BPM;
phys_assert_info pai_check_terrain_query_params = { 0, 2, true };
phys_assert_info pai_max_num_sap_active_pair = { 0, 2, true };
phys_assert_info pai_create_sap_active_pair = { 0, 1, true };



broad_phase_base **g_bpb_ptr_list;
broad_phase_base *g_bpb_list_cur;
volatile unsigned int g_bpb_list_index;
volatile unsigned int g_bpb_list_max_index;

tlAtomicMutex g_prolog_task_mutex;

int g_bpb_cluster_sort_axis = -1;
unsigned volatile int g_thread_id;

// these dont seem to have the workerCmd aspect
jqModule bp_env_jq_module1Module =
{
  "bp_env_jq_module1",
  JQ_WORKER_GENERIC,
  &bp_env_jq_batch_function1
};

jqModule bp_env_jq_module2Module =
{
  "bp_env_jq_module2",
  JQ_WORKER_GENERIC,
  &bp_env_jq_batch_function2,
};



void __thiscall broad_phase_base_list::add(broad_phase_base *bpb)
{
    broad_phase_base_list::node *cur; // [esp+58h] [ebp-4h]

    cur = *this->m_list_cur;
    if (!cur)
    {
        //cur = (broad_phase_base_list::node *)phys_transient_allocator::mt_allocate(
        cur = (broad_phase_base_list::node *)G_BPM->g_collision_memory_buffer.mt_allocate(
            8,
            4,
            0,
            "broad phase collision out of memory.");
        *this->m_list_cur = cur;
        cur->m_next = 0;
    }
    cur->m_bpb = bpb;
    this->m_list_cur = &cur->m_next;
}


jqBatchGroup::jqBatchGroup()
{
    this->BatchCount = 0;
}

void    comp_trace_volume(
                const phys_vec3 *aabb1_min,
                const phys_vec3 *aabb1_max,
                const phys_vec3 *aabb2_min,
                const phys_vec3 *aabb2_max,
                phys_vec3 *p1,
                phys_vec3 *p2,
                phys_vec3 *half_dims)
{
    const phys_vec3 *v8; // eax
    const phys_vec3 *v9; // eax
    const phys_vec3 *v10; // eax
    const phys_vec3 *v11; // eax
    const phys_vec3 *v12; // eax
    phys_vec3 v13; // [esp-Ch] [ebp-4Ch] BYREF
    phys_vec3 v14; // [esp+4h] [ebp-3Ch] BYREF
    phys_vec3 v15; // [esp+14h] [ebp-2Ch] BYREF
    float v16; // [esp+24h] [ebp-1Ch]
    float v17; // [esp+28h] [ebp-18h]
    float v18; // [esp+2Ch] [ebp-14h]
    //int v19; // [esp+34h] [ebp-Ch]
    //void *v20; // [esp+38h] [ebp-8h]
    //void *retaddr; // [esp+40h] [ebp+0h]
    //
    //v19 = a1;
    //v20 = retaddr;
    v14.x = aabb1_max->x - aabb1_min->x;
    v14.y = aabb1_max->y - aabb1_min->y;
    v14.z = aabb1_max->z - aabb1_min->z;
    v13.x = aabb2_max->x - aabb2_min->x;
    v13.y = aabb2_max->y - aabb2_min->y;
    v13.z = aabb2_max->z - aabb2_min->z;
    v8 = phys_max(&v15, &v14, &v13);
    v16 = v8->x * 0.5;
    v17 = v8->y * 0.5;
    v18 = 0.5 * v8->z;
    half_dims->x = v16;
    half_dims->y = v17;
    half_dims->z = v18;
    phys_min(&v14, aabb1_min, aabb2_min);
    phys_max(&v15, aabb1_max, aabb2_max);
    v13.x = v14.x + v16;
    v13.y = v14.y + v17;
    v13.z = v14.z + v18;
    v14.x = v15.x - v16;
    v14.y = v15.y - v17;
    v14.z = v15.z - v18;
    v16 = aabb1_min->x + aabb1_max->x;
    v17 = aabb1_min->y + aabb1_max->y;
    v18 = aabb1_min->z + aabb1_max->z;
    v15.x = v16 * 0.5;
    v15.y = v17 * 0.5;
    v15.z = 0.5 * v18;
    p1->x = v15.x;
    p1->y = v15.y;
    p1->z = v15.z;
    v9 = phys_min(&v15, p1, &v14);
    p1->x = v9->x;
    p1->y = v9->y;
    p1->z = v9->z;
    v10 = phys_max(&v15, p1, &v13);
    p1->x = v10->x;
    p1->y = v10->y;
    p1->z = v10->z;
    v15.x = aabb2_min->x + aabb2_max->x;
    v15.y = aabb2_min->y + aabb2_max->y;
    v15.z = aabb2_min->z + aabb2_max->z;
    v16 = v15.x * 0.5;
    v17 = v15.y * 0.5;
    v18 = 0.5 * v15.z;
    p2->x = v16;
    p2->y = v17;
    p2->z = v18;
    v11 = phys_min(&v15, p2, &v14);
    p2->x = v11->x;
    p2->y = v11->y;
    p2->z = v11->z;
    v12 = phys_max(&v14, p2, &v13);
    p2->x = v12->x;
    p2->y = v12->y;
    p2->z = v12->z;
}

void broad_phase_base::get_aabb(phys_vec3 *aabb)
{
    const phys_vec3 *v4; // eax
    const phys_vec3 *v5; // eax
    phys_vec3 v6; // [esp-20h] [ebp-2Ch] BYREF
    phys_vec3 v7; // [esp-10h] [ebp-1Ch] BYREF
    //int v8; // [esp+0h] [ebp-Ch]
    //void *v9; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v8 = a2;
    //v9 = retaddr;
    v7.x = this->m_trace_translation.x + this->m_trace_aabb_min_whace.x;
    v7.y = this->m_trace_translation.y + this->m_trace_aabb_min_whace.y;
    v7.z = this->m_trace_translation.z + this->m_trace_aabb_min_whace.z;
    v4 = phys_min(&v6, &this->m_trace_aabb_min_whace, &v7);
    aabb->x = v4->x;
    aabb->y = v4->y;
    aabb->z = v4->z;
    v7.x = this->m_trace_aabb_max_whace.x + this->m_trace_translation.x;
    v7.y = this->m_trace_aabb_max_whace.y + this->m_trace_translation.y;
    v7.z = this->m_trace_aabb_max_whace.z + this->m_trace_translation.z;
    v5 = phys_max(&v6, &this->m_trace_aabb_max_whace, &v7);
    aabb[1].x = v5->x;
    aabb[1].y = v5->y;
    aabb[1].z = v5->z;
}

void __cdecl setup_gjk_input_from_pcp(phys_gjk_input *pgi, phys_collision_pair *pcp)
{
    iassert(pcp->m_hit_time >= 0.0f && pcp->m_hit_time <= 1.0f);

    broad_phase_info *bpi1 = pcp->m_bpi1;
    broad_phase_info *bpi2 = pcp->m_bpi2;
    float hit_time = pcp->m_hit_time;

    pgi->m_cg1_translation = bpi1->m_trace_translation;
    pgi->m_cg2_translation = bpi2->m_trace_translation;

    pgi->m_start_time = hit_time;
    pgi->m_end_time = 1.0f;

    const phys_gjk_geom *geom1 = bpi1->m_gjk_geom;
    const phys_gjk_geom *geom2 = bpi2->m_gjk_geom;

    float radius1 = geom1->get_geom_radius() + 0.34f;
    float radius2 = geom2->get_geom_radius() + 0.34f;

    pgi->gjk_cg1 = (const gjk_base_t*)geom1; // LWSS: bit hacky downcast here (wouldnt be necessary without my change)
    pgi->cg1_to_world_xform = bpi1->m_cg_to_world_xform;
    pgi->cg1_radius = radius1;

    pgi->gjk_cg2 = (const gjk_base_t*)geom2; // ^^^^^^^^^^^^
    pgi->cg2_to_world_xform = bpi2->m_cg_to_world_xform;
    pgi->cg2_radius = radius2;

    pgi->gjk_ci = pcp->m_gjk_ci;
}

//void __userpurge phys_wheel_collide_info::collision_process(
//                phys_wheel_collide_info *this@<ecx>,
//                int a2@<ebp>,
//                int a3@<edi>,
//                int a4@<esi>,
//                broad_phase_info *bpi)
//{
//    const phys_mat44 *m_cg_to_world_xform; // edi
//    const phys_gjk_geom *m_gjk_geom; // ecx
//    _BYTE v10[12]; // [esp+1Ch] [ebp-4Ch] BYREF
//    phys_vec3 ray_pos_loc; // [esp+28h] [ebp-40h] BYREF
//    phys_vec3 hitn; // [esp+38h] [ebp-30h] BYREF
//    phys_vec3 ray_dir_loc; // [esp+48h] [ebp-20h]
//    float v14; // [esp+58h] [ebp-10h] BYREF
//    unsigned int v15[2]; // [esp+5Ch] [ebp-Ch] BYREF
//    _UNKNOWN *retaddr; // [esp+68h] [ebp+0h]
//
//    v15[0] = a2;
//    v15[1] = retaddr;
//    m_cg_to_world_xform = bpi->m_cg_to_world_xform;
//    phys_full_inv_multiply((int)v15, (phys_vec3 *)v10, m_cg_to_world_xform, &this->m_ray_pos);
//    v14 = m_cg_to_world_xform->y.y * this->m_ray_dir.y
//            + m_cg_to_world_xform->y.x * this->m_ray_dir.x
//            + m_cg_to_world_xform->y.z * this->m_ray_dir.z;
//    ray_dir_loc.w = m_cg_to_world_xform->z.y * this->m_ray_dir.y
//                                + m_cg_to_world_xform->z.x * this->m_ray_dir.x
//                                + m_cg_to_world_xform->z.z * this->m_ray_dir.z;
//    m_gjk_geom = bpi->m_gjk_geom;
//    hitn.y = m_cg_to_world_xform->x.y * this->m_ray_dir.y
//                 + this->m_ray_dir.x * m_cg_to_world_xform->x.x
//                 + m_cg_to_world_xform->x.z * this->m_ray_dir.z;
//    hitn.z = v14;
//    hitn.w = ray_dir_loc.w;
//    if ( ((unsigned __int8 (__thiscall *)(const phys_gjk_geom *, _BYTE *, float *, float, float *, float *, int, int))m_gjk_geom->ray_cast)(
//                 m_gjk_geom,
//                 v10,
//                 &hitn.y,
//                 this->m_hit_t,
//                 &v14,
//                 &ray_pos_loc.y,
//                 a3,
//                 a4) )
//    {
//        if ( this->m_hit_t < (double)v14 )
//        {
//            if ( _tlAssert("source/phys_broad_phase.cpp", 41, "hit_t <= m_hit_t", "") )
//                __debugbreak();
//        }
//        this->m_hit_t = v14;
//        this->m_hitn.x = ray_pos_loc.y;
//        this->m_hitn.y = ray_pos_loc.z;
//        this->m_hitn.z = ray_pos_loc.w;
//        this->m_hit_bpi = bpi;
//    }
//}

void phys_wheel_collide_info::collision_process(broad_phase_info *bpi)
{
    const phys_mat44 *cg_to_world = bpi->m_cg_to_world_xform;

    // Ray origin into bpi cg-local space (full inverse, includes translation).
    phys_vec3 ray_pos_loc;
    phys_full_inv_multiply(&ray_pos_loc, cg_to_world, &this->m_ray_pos);

    // Ray direction into bpi cg-local space (rotation only).
    phys_vec3 ray_dir_loc;
    ray_dir_loc.x = cg_to_world->x.x * this->m_ray_dir.x
                  + cg_to_world->x.y * this->m_ray_dir.y
                  + cg_to_world->x.z * this->m_ray_dir.z;
    ray_dir_loc.y = cg_to_world->y.x * this->m_ray_dir.x
                  + cg_to_world->y.y * this->m_ray_dir.y
                  + cg_to_world->y.z * this->m_ray_dir.z;
    ray_dir_loc.z = cg_to_world->z.x * this->m_ray_dir.x
                  + cg_to_world->z.y * this->m_ray_dir.y
                  + cg_to_world->z.z * this->m_ray_dir.z;

    phys_vec3 hit_normal;
    float hit_t = 0.0f;

    if (bpi->m_gjk_geom->ray_cast(&ray_pos_loc, &ray_dir_loc, this->m_hit_t, &hit_t, &hit_normal))
    {
        iassert(hit_t <= this->m_hit_t);

        this->m_hit_t = hit_t;
        this->m_hitn = hit_normal;
        this->m_hit_bpi = bpi;
    }
}

#if 0
void __userpurge phys_wheel_collide_info::collision_epilog(
                phys_wheel_collide_info *this@<ecx>,
                int a2@<ebp>,
                rigid_body_constraint_wheel *rbc_wheel)
{
    phys_vec3 *p_m_hitn; // esi
    double v5; // st7
    const phys_vec3 *v6; // eax
    double z; // st7
    broad_phase_info *m_hit_bpi; // edi
    const phys_mat44 *m_rb_to_world_xform; // [esp-30h] [ebp-70h]
    _BYTE v10[12]; // [esp-Ch] [ebp-4Ch] BYREF
    phys_vec3 b2_hitn_loc; // [esp+0h] [ebp-40h] BYREF
    float v12; // [esp+14h] [ebp-2Ch] BYREF
    float v13; // [esp+18h] [ebp-28h]
    float v14; // [esp+1Ch] [ebp-24h]
    float m_hit_t; // [esp+30h] [ebp-10h]
    unsigned int v16[2]; // [esp+34h] [ebp-Ch] BYREF
    _UNKNOWN *retaddr; // [esp+40h] [ebp+0h]

    v16[0] = a2;
    v16[1] = retaddr;
    rigid_body_constraint_wheel::set_no_collision(rbc_wheel);
    if ( this->m_hit_bpi )
    {
        p_m_hitn = &this->m_hitn;
        m_hit_t = this->m_hitn.y * this->m_hitn.y + this->m_hitn.x * this->m_hitn.x + this->m_hitn.z * this->m_hitn.z;
        m_hit_t = sqrt(m_hit_t);
        if ( m_hit_t > 0.000001 )
        {
            m_hit_t = 1.0 / m_hit_t;
            v5 = m_hit_t;
            p_m_hitn->x = p_m_hitn->x * m_hit_t;
            this->m_hitn.y = this->m_hitn.y * v5;
            this->m_hitn.z = v5 * this->m_hitn.z;
            v6 = phys_multiply((phys_vec3 *)&b2_hitn_loc.y, this->m_hit_bpi->m_cg_to_world_xform, &this->m_hitn);
            p_m_hitn->x = v6->x;
            this->m_hitn.y = v6->y;
            this->m_hitn.z = v6->z;
            m_hit_t = this->m_hit_t;
            v12 = m_hit_t * this->m_ray_dir.x;
            v13 = this->m_ray_dir.y * m_hit_t;
            v14 = m_hit_t * this->m_ray_dir.z;
            b2_hitn_loc.y = this->m_ray_pos.x + v12;
            b2_hitn_loc.z = this->m_ray_pos.y + v13;
            z = this->m_ray_pos.z;
            m_hit_bpi = this->m_hit_bpi;
            m_rb_to_world_xform = m_hit_bpi->m_rb_to_world_xform;
            b2_hitn_loc.w = z + v14;
            phys_full_inv_multiply((int)v16, (phys_vec3 *)&v12, m_rb_to_world_xform, (phys_vec3 *)&b2_hitn_loc.y);
            phys_inv_multiply((const phys_vec3 *)v10, m_hit_bpi->m_rb_to_world_xform, p_m_hitn);
            rigid_body_constraint_wheel::set_collision(
                rbc_wheel,
                m_hit_bpi->m_rb,
                (const phys_vec3 *)&v12,
                (const phys_vec3 *)v10);
        }
    }
}
#endif

void phys_wheel_collide_info::collision_epilog(rigid_body_constraint_wheel *rbc_wheel)
{
    rbc_wheel->set_no_collision();

    if (!this->m_hit_bpi)
        return;

    // Threshold is on length (post-sqrt), not squared length, per binary.
    float len = sqrtf(this->m_hitn.x * this->m_hitn.x
                    + this->m_hitn.y * this->m_hitn.y
                    + this->m_hitn.z * this->m_hitn.z);
    if (len <= 1.0e-6f)
        return;

    float invLen = 1.0f / len;
    this->m_hitn.x *= invLen;
    this->m_hitn.y *= invLen;
    this->m_hitn.z *= invLen;

    // Rotate normal into world space, then store back into m_hitn.
    phys_vec3 worldNormal;
    phys_multiply(&worldNormal, this->m_hit_bpi->m_cg_to_world_xform, &this->m_hitn);
    this->m_hitn.x = worldNormal.x;
    this->m_hitn.y = worldNormal.y;
    this->m_hitn.z = worldNormal.z;

    // World-space hit point: m_ray_pos + m_hit_t * m_ray_dir.
    float t = this->m_hit_t;
    phys_vec3 hitPointWorld;
    hitPointWorld.x = this->m_ray_pos.x + t * this->m_ray_dir.x;
    hitPointWorld.y = this->m_ray_pos.y + t * this->m_ray_dir.y;
    hitPointWorld.z = this->m_ray_pos.z + t * this->m_ray_dir.z;

    // Transform hit point + normal into rigid-body local space.
    const phys_mat44 *rb_to_world = this->m_hit_bpi->m_rb_to_world_xform;
    phys_vec3 hitPointRBLocal;
    phys_vec3 hitNormalRBLocal;
    phys_full_inv_multiply(&hitPointRBLocal, rb_to_world, &hitPointWorld);
    phys_inv_multiply(&hitNormalRBLocal, rb_to_world, &this->m_hitn);

    rbc_wheel->set_collision(this->m_hit_bpi->m_rb, &hitPointRBLocal, &hitNormalRBLocal);
}


void __thiscall axis_aligned_sweep_and_prune::sap_node::init(
                broad_phase_base *bpb,
                axis_aligned_sweep_and_prune::axis_element **xlist,
                axis_aligned_sweep_and_prune::axis_element **ylist,
                axis_aligned_sweep_and_prune::axis_element **zlist)
{
    this->m_bpb = bpb;
    this->m_updated = 0;
    *(_QWORD *)&this->m_ae1[0][0].m_node = (unsigned int)this;
    this->m_ae1[0][1].m_node = this;
    this->m_ae1[0][1].m_min_max = 1;
    this->m_ae1[0][0].m_next = &this->m_ae1[0][1];
    this->m_ae1[0][1].m_next = *xlist;
    *xlist = this->m_ae1[0];
    *(_QWORD *)&this->m_ae1[1][0].m_node = (unsigned int)this;
    this->m_ae1[1][1].m_node = this;
    this->m_ae1[1][1].m_min_max = 1;
    this->m_ae1[1][0].m_next = &this->m_ae1[1][1];
    this->m_ae1[1][1].m_next = *ylist;
    *ylist = this->m_ae1[1];
    *(_QWORD *)&this->m_ae1[2][0].m_node = (unsigned int)this;
    this->m_ae1[2][1].m_node = this;
    this->m_ae1[2][1].m_min_max = 1;
    this->m_ae1[2][0].m_next = &this->m_ae1[2][1];
    this->m_ae1[2][1].m_next = *zlist;
    *zlist = this->m_ae1[2];
}

bool __thiscall axis_aligned_sweep_and_prune::are_overlapping(
                axis_aligned_sweep_and_prune::sap_node *n1,
                axis_aligned_sweep_and_prune::sap_node *n2)
{
    return n1->m_ae1[0][0].m_ae_list_index < n2->m_ae1[0][1].m_ae_list_index
            && n2->m_ae1[0][0].m_ae_list_index < n1->m_ae1[0][1].m_ae_list_index
            && n1->m_ae1[1][0].m_ae_list_index < n2->m_ae1[1][1].m_ae_list_index
            && n2->m_ae1[1][0].m_ae_list_index < n1->m_ae1[1][1].m_ae_list_index
            && n1->m_ae1[2][0].m_ae_list_index < n2->m_ae1[2][1].m_ae_list_index
            && n2->m_ae1[2][0].m_ae_list_index < n1->m_ae1[2][1].m_ae_list_index;
}

void __thiscall axis_aligned_sweep_and_prune::remove(
                axis_aligned_sweep_and_prune::axis_element **axis_list,
                axis_aligned_sweep_and_prune::sap_node *node)
{
    axis_aligned_sweep_and_prune::axis_element **p_m_next; // ecx
    axis_aligned_sweep_and_prune::axis_element *v4; // eax
    int v5; // edx

    p_m_next = axis_list;
    v4 = *axis_list;
    v5 = 0;
    if ( !*axis_list )
        goto LABEL_12;
    do
    {
        if ( v5 >= 2 )
            break;
        if ( v4->m_node == node )
        {
            *p_m_next = v4->m_next;
            ++v5;
        }
        else
        {
            p_m_next = &v4->m_next;
        }
        v4 = *p_m_next;
    }
    while ( *p_m_next );
    if ( v5 != 2 )
    {
LABEL_12:
        if ( _tlAssert("source/phys_broad_phase.cpp", 515, "count == 2", "") )
            __debugbreak();
    }
}

void __cdecl init_bpeqi(broad_phase_environment_query_input *bpeqi, broad_phase_base *bpb)
{
    nanassertvec3(bpb->m_trace_aabb_min_whace); // LWSS ADD
    nanassertvec3(bpb->m_trace_aabb_max_whace); // LWSS ADD

    bpeqi->trace_aabb_min_wace.x = bpb->m_trace_aabb_min_whace.x - 0.5099999904632568;
    bpeqi->trace_aabb_min_wace.y = bpb->m_trace_aabb_min_whace.y - 0.5099999904632568;
    bpeqi->trace_aabb_min_wace.z = bpb->m_trace_aabb_min_whace.z - 0.5099999904632568;

    bpeqi->trace_aabb_max_wace.x = bpb->m_trace_aabb_max_whace.x + 0.5099999904632568;
    bpeqi->trace_aabb_max_wace.y = bpb->m_trace_aabb_max_whace.y + 0.5099999904632568;
    bpeqi->trace_aabb_max_wace.z = bpb->m_trace_aabb_max_whace.z + 0.5099999904632568;

    bpeqi->trace_translation.x = bpb->m_trace_translation.x;
    bpeqi->trace_translation.y = bpb->m_trace_translation.y;
    bpeqi->trace_translation.z = bpb->m_trace_translation.z;

    bpeqi->env_collision_flags = bpb->m_env_collision_flags;
}

void __cdecl check_terrain_query_params(broad_phase_base *bpb)
{
    float ntranslation_sq; // [esp+8h] [ebp+8h]

    ntranslation_sq = bpb->m_trace_translation.y * bpb->m_trace_translation.y
                                    + bpb->m_trace_translation.x * bpb->m_trace_translation.x
                                    + bpb->m_trace_translation.z * bpb->m_trace_translation.z;
    if ( ntranslation_sq > 260100.0 )
    {
        if ( (pai_check_terrain_query_params.m_hits_total_count < pai_check_terrain_query_params.m_max_hits_total
             || !pai_check_terrain_query_params.m_max_hits_total)
            && pai_check_terrain_query_params.m_hits_frame_count < pai_check_terrain_query_params.m_max_hits_per_frame )
        {
            if ( pai_check_terrain_query_params.m_use_warnings_only )
            {
                PHYS_WARNING(
                    "source/phys_broad_phase.cpp",
                    636,
                    "ntranslation_sq <= MAX_TRANSLATION_SQ",
                    "terrain query translation too large.");
            }
            else if ( ntranslation_sq > 260100.0 )
            {
                if ( _tlAssert(
                             "source/phys_broad_phase.cpp",
                             636,
                             "ntranslation_sq <= MAX_TRANSLATION_SQ",
                             "terrain query translation too large.") )
                {
                    __debugbreak();
                }
            }
        }
        _InterlockedExchangeAdd(&pai_check_terrain_query_params.m_hits_total_count, 1u);
        _InterlockedExchangeAdd(&pai_check_terrain_query_params.m_hits_frame_count, 1u);
    }
}

#if 0
void    calc_largest_vel_sq(broad_phase_info *bpi)
{
    rigid_body *m_rb; // edi
    float *p_x; // esi
    double v4; // st7
    double v5; // st7
    double v6; // st6
    phys_vec3 *v7; // eax
    phys_vec3 *v8; // eax
    const phys_vec3 *v9; // eax
    volatile unsigned __int32 *p_m_largest_vel_sq; // edx
    double v11; // st7
    signed __int32 v12; // ecx
    phys_vec3 v13; // [esp-4Ch] [ebp-BCh] BYREF
    phys_vec3 v14; // [esp-3Ch] [ebp-ACh] BYREF
    phys_vec3 v15; // [esp-2Ch] [ebp-9Ch] BYREF
    phys_vec3 v16; // [esp-1Ch] [ebp-8Ch] BYREF
    phys_vec3 v17; // [esp-Ch] [ebp-7Ch] BYREF
    phys_vec3 v18; // [esp+4h] [ebp-6Ch] BYREF
    float v19; // [esp+20h] [ebp-50h]
    float y; // [esp+24h] [ebp-4Ch] BYREF
    float z; // [esp+28h] [ebp-48h]
    float t_vel_sq; // [esp+2Ch] [ebp-44h]
    phys_vec3 dir; // [esp+30h] [ebp-40h] BYREF
    float v24; // [esp+44h] [ebp-2Ch] BYREF
    float v25; // [esp+48h] [ebp-28h]
    float v26; // [esp+4Ch] [ebp-24h]
    signed __int32 p_w; // [esp+5Ch] [ebp-14h]
    float v28; // [esp+60h] [ebp-10h]
    int v29; // [esp+64h] [ebp-Ch] BYREF
    float lvs; // [esp+68h] [ebp-8h]
    float retaddr; // [esp+70h] [ebp+0h]

    v29 = a1;
    lvs = retaddr;
    if ((bpi->m_flags & 4) != 0 && _tlAssert("source/phys_broad_phase.cpp", 692, "!bpi->is_bpi_env()", ""))
        __debugbreak();
    m_rb = bpi->m_rb;
    if ((m_rb->m_flags & 0x30) != 0
        && _tlAssert(
            "source/phys_broad_phase.cpp",
            694,
            "!rb->get_flag(rigid_body::FLAG_ENVIRONMENT_RIGID_BODY|rigid_body::FLAG_USER_RIGID_BODY)",
            ""))
    {
        __debugbreak();
    }
    p_x = &m_rb->m_a_vel.x;
    *(float *)&p_w = m_rb->m_a_vel.y * m_rb->m_a_vel.y
        + m_rb->m_a_vel.x * m_rb->m_a_vel.x
        + m_rb->m_a_vel.z * m_rb->m_a_vel.z;
    v19 = m_rb->m_t_vel.y * m_rb->m_t_vel.y + m_rb->m_t_vel.x * m_rb->m_t_vel.x + m_rb->m_t_vel.z * m_rb->m_t_vel.z;
    v4 = 0.0000010000001;
    if (*(float *)&p_w >= 0.0000010000001)
    {
        y = m_rb->m_t_vel.y * m_rb->m_a_vel.z - m_rb->m_t_vel.z * m_rb->m_a_vel.y;
        z = m_rb->m_a_vel.x * m_rb->m_t_vel.z - m_rb->m_t_vel.x * m_rb->m_a_vel.z;
        t_vel_sq = m_rb->m_t_vel.x * m_rb->m_a_vel.y - m_rb->m_a_vel.x * m_rb->m_t_vel.y;
        v28 = z * z + y * y + t_vel_sq * t_vel_sq;
        if (v28 < 0.0000010000001)
        {
            dir.y = m_rb->m_a_vel.y * 0.0 - m_rb->m_a_vel.z * 0.0;
            v6 = 0.0 * *p_x;
            dir.z = m_rb->m_a_vel.z - v6;
            dir.w = v6 - m_rb->m_a_vel.y;
            v24 = m_rb->m_t_vel.x + dir.y;
            v25 = m_rb->m_t_vel.y + dir.z;
            v26 = m_rb->m_t_vel.z + dir.w;
            dir.y = m_rb->m_a_vel.z * v25 - v26 * m_rb->m_a_vel.y;
            dir.z = v26 * *p_x - m_rb->m_a_vel.z * v24;
            dir.w = v24 * m_rb->m_a_vel.y - v25 * m_rb->m_a_vel.x;
            y = dir.y;
            z = dir.z;
            t_vel_sq = dir.w;
            v28 = dir.w * dir.w + dir.z * dir.z + dir.y * dir.y;
            if (v28 < 0.0000010000001)
            {
                v24 = 0.0;
                v25 = 2.0;
                v26 = 0.0;
                v7 = phys_cross(&v14, &m_rb->m_a_vel, (const phys_vec3 *)&v24);
                dir.y = m_rb->m_t_vel.x + v7->x;
                dir.z = m_rb->m_t_vel.y + v7->y;
                dir.w = m_rb->m_t_vel.z + v7->z;
                v8 = phys_cross(&v16, (phys_vec3 *)&dir.y, &m_rb->m_a_vel);
                y = v8->x;
                z = v8->y;
                t_vel_sq = v8->z;
                v28 = z * z + y * y + t_vel_sq * t_vel_sq;
                v4 = 0.0000010000001;
            }
        }
        if (v4 > v28)
        {
            v5 = v19 + *(float *)&p_w;
        }
        else
        {
            phys_inv_multiply(&v17, bpi->m_cg_to_world_xform, (const phys_vec3 *)&y);
            bpi->m_gjk_geom->support(&v17, &v18, &v15);
            p_w = (signed __int32)&bpi->m_rb_to_world_xform->w;
            v9 = phys_full_multiply(&v13, bpi->m_cg_to_world_xform, &v18);
            v24 = v9->x - *(float *)p_w;
            v25 = v9->y - *(float *)(p_w + 4);
            v26 = v9->z - *(float *)(p_w + 8);
            dir.y = m_rb->m_a_vel.y * v26 - v25 * m_rb->m_a_vel.z;
            dir.z = v24 * m_rb->m_a_vel.z - v26 * m_rb->m_a_vel.x;
            dir.w = v25 * m_rb->m_a_vel.x - v24 * m_rb->m_a_vel.y;
            v24 = dir.y + m_rb->m_t_vel.x;
            v25 = m_rb->m_t_vel.y + dir.z;
            v26 = m_rb->m_t_vel.z + dir.w;
            v5 = v25 * v25 + v24 * v24 + v26 * v26;
        }
    }
    else
    {
        v5 = v19;
    }
    v28 = v5;
    p_m_largest_vel_sq = (volatile unsigned __int32 *)&m_rb->m_largest_vel_sq;
    v11 = v28;
    do
    {
        p_w = *(signed __int32 *)p_m_largest_vel_sq;
        if (*(float *)&p_w > v11)
            break;
        v12 = p_w;
    } while (_InterlockedCompareExchange(p_m_largest_vel_sq, SLODWORD(v28), p_w) != v12);
}
#else // aislop
// Calculates the maximum possible velocity squared for a rigid body's broadphase sweep,
// accounting for both translational and rotational velocity at the furthest support point.
// Result is atomically stored in m_rb->m_largest_vel_sq if it exceeds the current value.
void calc_largest_vel_sq(broad_phase_info *bpi)
{
    static constexpr float EPSILON_SQ = 1e-6f;

    //ASSERT(!bpi->is_bpi_env(), "source/phys_broad_phase.cpp", 692, "!bpi->is_bpi_env()");

    rigid_body *rb = bpi->m_rb;

    //ASSERT(!rb->get_flag(rigid_body::FLAG_ENVIRONMENT_RIGID_BODY | rigid_body::FLAG_USER_RIGID_BODY),
    //    "source/phys_broad_phase.cpp", 694,
    //    "!rb->get_flag(rigid_body::FLAG_ENVIRONMENT_RIGID_BODY|rigid_body::FLAG_USER_RIGID_BODY)");

    const phys_vec3 &a_vel = rb->m_a_vel; // angular velocity
    const phys_vec3 &t_vel = rb->m_t_vel; // translational velocity

    float a_vel_sq = a_vel.x * a_vel.x + a_vel.y * a_vel.y + a_vel.z * a_vel.z;
    float t_vel_sq = t_vel.x * t_vel.x + t_vel.y * t_vel.y + t_vel.z * t_vel.z;

    float result_vel_sq;

    if (a_vel_sq < EPSILON_SQ)
    {
        // No meaningful angular velocity — translational speed alone is the bound.
        result_vel_sq = t_vel_sq;
    }
    else
    {
        // Find a direction perpendicular to a_vel pointing toward the worst-case
        // support point (the geometry point with the highest tangential speed).

        // Attempt 1: perp = t_vel x a_vel
        phys_vec3 perp;
        phys_cross(&perp, &t_vel, &a_vel);
        float perp_sq = perp.x * perp.x + perp.y * perp.y + perp.z * perp.z;

        if (perp_sq < EPSILON_SQ)
        {
            // t_vel is parallel to a_vel — perturb t_vel slightly off-axis and retry.
            // Construct an offset orthogonal to a_vel, add to t_vel, then cross with a_vel.
            phys_vec3 offset = {
                a_vel.y * 0.0f - a_vel.z * 0.0f,  // = 0; degenerate row intentional (see asm)
                a_vel.z * 1.0f - a_vel.x * 0.0f,  // = a_vel.z
                a_vel.x * 0.0f - a_vel.y * 1.0f   // = -a_vel.y
            };
            phys_vec3 perturbed_t = {
                t_vel.x + offset.x,
                t_vel.y + offset.y,
                t_vel.z + offset.z
            };
            phys_cross(&perp, &perturbed_t, &a_vel);
            perp_sq = perp.x * perp.x + perp.y * perp.y + perp.z * perp.z;

            if (perp_sq < EPSILON_SQ)
            {
                // Still degenerate — cross a_vel with world Y (0,2,0), add to t_vel, retry.
                phys_vec3 world_y = { 0.0f, 2.0f, 0.0f };
                phys_vec3 a_cross_y;
                phys_cross(&a_cross_y, &a_vel, &world_y);
                phys_vec3 perturbed_t2 = {
                    t_vel.x + a_cross_y.x,
                    t_vel.y + a_cross_y.y,
                    t_vel.z + a_cross_y.z
                };
                phys_cross(&perp, &perturbed_t2, &a_vel);
                perp_sq = perp.x * perp.x + perp.y * perp.y + perp.z * perp.z;
            }
        }

        if (perp_sq < EPSILON_SQ)
        {
            // Completely degenerate — fall back to simple sum.
            result_vel_sq = t_vel_sq + a_vel_sq;
        }
        else
        {
            // perp points toward the support point in the plane perpendicular to a_vel.
            // Transform into geometry local space, query the support point, transform back.
            phys_vec3 perp_local;
            phys_inv_multiply(&perp_local, bpi->m_cg_to_world_xform, &perp);

            phys_vec3 support_local, support_unused;
            bpi->m_gjk_geom->support(perp_local, &support_local, &support_unused);

            phys_vec3 support_world;
            phys_full_multiply(&support_world, bpi->m_cg_to_world_xform, &support_local);

            // Lever arm: from the rotation origin (rb_to_world translation) to the support point.
            const phys_vec3 &origin = bpi->m_rb_to_world_xform->w;
            phys_vec3 lever = {
                support_world.x - origin.x,
                support_world.y - origin.y,
                support_world.z - origin.z
            };

            // Velocity at support point = t_vel + (a_vel x lever)
            phys_vec3 rot_vel;
            phys_cross(&rot_vel, &a_vel, &lever);

            phys_vec3 total_vel = {
                t_vel.x + rot_vel.x,
                t_vel.y + rot_vel.y,
                t_vel.z + rot_vel.z
            };

            result_vel_sq = total_vel.x * total_vel.x
                + total_vel.y * total_vel.y
                + total_vel.z * total_vel.z;
        }
    }

    // Atomically update m_largest_vel_sq if our result is larger (lock-free CAS loop).
    volatile unsigned __int32 *p_largest = (volatile unsigned __int32 *)&rb->m_largest_vel_sq;
    unsigned __int32 expected;
    do {
        expected = *p_largest;
        if (*(float *)&expected > result_vel_sq)
            break; // existing value is already larger, nothing to do
    } while (_InterlockedCompareExchange(p_largest, *(unsigned __int32 *)&result_vel_sq, expected) != expected);
}
#endif
void __cdecl broad_phase_reset_buffer()
{
    //phys_transient_allocator::reset(&G_BPM->g_collision_memory_buffer);
    G_BPM->g_collision_memory_buffer.reset();
}

int    bp_env_jq_batch_function1(jqBatch *pBatch)
{
    signed __int32 i; // eax
    float **Input; // esi
    float *v4; // ecx
    double z; // st7
    double y; // st7
    double x; // st7
    float *v8; // ecx
    double v9; // st7
    bool v10; // c0
    bool v11; // c3
    double v12; // st7
    double v13; // st7
    bool v14; // c0
    bool v15; // c3
    double v16; // st7
    double v17; // st7
    bool v18; // c0
    bool v19; // c3
    double v20; // st7
    broad_phase_base bpb; // [esp-Ch] [ebp-6Ch] BYREF
    float bpb_72; // [esp+48h] [ebp-18h] BYREF
    float bpb_76; // [esp+4Ch] [ebp-14h]
    float v25; // [esp+50h] [ebp-10h]
    //_DWORD v26[3]; // [esp+54h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+60h] [ebp+0h]
    //
    //v26[0] = a1;
    //v26[1] = retaddr;
    bpb.m_trace_aabb_min_whace.x = 1.0e30;
    bpb.m_trace_aabb_min_whace.y = 1.0e30;
    bpb.m_trace_aabb_min_whace.z = 1.0e30;
    bpb.m_trace_aabb_max_whace.x = -1.0e30;
    bpb.m_trace_aabb_max_whace.y = -1.0e30;
    bpb.m_trace_aabb_max_whace.z = -1.0e30;

    for (i = _InterlockedExchangeAdd(&g_bpb_list_index, 1u);
        i < g_bpb_list_max_index;
        i = _InterlockedExchangeAdd(&g_bpb_list_index, 1u))
    {
        process_cluster_environment_collision_prolog((broad_phase_info *)g_bpb_ptr_list[i], &bpb);
    }

    Input = (float **)pBatch->Input;
    //tlAtomicMutex::Lock(&g_prolog_task_mutex);
    g_prolog_task_mutex.Lock();

    float *minOut = Input[0];
    minOut[0] = (bpb.m_trace_aabb_min_whace.x < minOut[0]) ? bpb.m_trace_aabb_min_whace.x : minOut[0];
    minOut[1] = (bpb.m_trace_aabb_min_whace.y < minOut[1]) ? bpb.m_trace_aabb_min_whace.y : minOut[1];
    minOut[2] = (bpb.m_trace_aabb_min_whace.z < minOut[2]) ? bpb.m_trace_aabb_min_whace.z : minOut[2];

    float *maxOut = Input[1];
    maxOut[0] = (bpb.m_trace_aabb_max_whace.x > maxOut[0]) ? bpb.m_trace_aabb_max_whace.x : maxOut[0];
    maxOut[1] = (bpb.m_trace_aabb_max_whace.y > maxOut[1]) ? bpb.m_trace_aabb_max_whace.y : maxOut[1];
    maxOut[2] = (bpb.m_trace_aabb_max_whace.z > maxOut[2]) ? bpb.m_trace_aabb_max_whace.z : maxOut[2];

    g_prolog_task_mutex.Unlock();
    return 0;
}

void __cdecl merge_sort_bpb(broad_phase_base **list, int list_count)
{
    broad_phase_base *v2; // eax
    broad_phase_base **v3; // ebx
    broad_phase_base **i; // esi
    broad_phase_base *v5; // eax
    broad_phase_base **j; // edi
    broad_phase_base *v7; // eax
    broad_phase_base **last; // [esp+8h] [ebp-4h]

    if ( list_count >= 3 )
    {
        merge_sort_bpb(list, list_count / 2);
        merge_sort_bpb(&list[list_count / 2], list_count - list_count / 2);
        v3 = &list[list_count / 2];
        last = &list[list_count];
        for ( i = v3 - 1; v3 < last; ++v3 )
        {
            if ( !compare_bpb(*v3, *i) )
                break;
            v5 = *i;
            *i = *v3;
            *v3 = v5;
            for ( j = i - 1; i > list; --j )
            {
                if ( !compare_bpb(*i, *j) )
                    break;
                v7 = *j;
                *j = *i;
                *i = v7;
                i = j;
            }
            i = v3;
        }
    }
    else if ( list_count == 2 )
    {
        if ( compare_bpb(list[1], *list) )
        {
            v2 = *list;
            *list = list[1];
            list[1] = v2;
        }
    }
}

void broad_phase_process_object_environment_collision(bpi_environment_collision_info *eci)
{
    broad_phase_base *m_bpb_i_start; // edi
    broad_phase_base **v8; // edi
    int v9; // ecx
    broad_phase_base **v10; // edx
    broad_phase_base **i; // eax
    broad_phase_base *bpb_i; // edi
    int v13; // eax
    float x; // eax
    float y; // ecx
    float z; // edx
    int v17; // esi
    double v18; // st7
    double v19; // st7
    float w; // eax
    float v21; // ecx
    float v22; // edx
    double v23; // st7
    float v24; // eax
    float v25; // ecx
    unsigned int m_env_collision_flags; // edx
    double v27; // st7
    double v28; // st7
    double v29; // st7
    broad_phase_base *m_list_bpb_next; // esi
    double v31; // st7
    double v32; // st7
    double v33; // st7
    double v34; // st7
    double v35; // st7
    double v36; // st7
    double v37; // st7
    double v38; // st6
    double v39; // st5
    double v40; // st4
    double v41; // st3
    double v42; // st2
    broad_phase_base *v43; // eax
    unsigned int v44; // edx
    double v45; // st1
    double v46; // st1
    char *m_cur; // edx
    unsigned int v48; // eax
    unsigned int v49; // ecx
    unsigned int v50; // eax
    float v51; // esi
    double v52; // st7
    double v53; // st6
    double v54; // st5
    double v55; // st4
    double v56; // st3
    double v57; // st2
    unsigned int v58; // ecx
    phys_vec3 v60; // [esp-1Ch] [ebp-15Ch] BYREF
    phys_vec3 v61; // [esp-Ch] [ebp-14Ch] BYREF
    phys_vec3 p2; // [esp+4h] [ebp-13Ch] BYREF
    phys_vec3 aabb2_min_new; // [esp+14h] [ebp-12Ch] BYREF
    phys_vec3 half_dims; // [esp+24h] [ebp-11Ch] BYREF
    phys_vec3 v65; // [esp+34h] [ebp-10Ch] BYREF
    phys_vec3 p1; // [esp+44h] [ebp-FCh] BYREF
    phys_vec3 bp_aabb_min; // [esp+64h] [ebp-DCh] BYREF
    phys_vec3 bp_aabb_max; // [esp+74h] [ebp-CCh] BYREF
    phys_vec3 aabb2_max; // [esp+84h] [ebp-BCh] BYREF
    phys_vec3 aabb2_min; // [esp+94h] [ebp-ACh] BYREF
    float bpb_i_aabb_min; // [esp+A8h] [ebp-98h]
    //phys_transient_allocator transient_buffer; // [esp+ACh] [ebp-94h] BYREF
    phys_vec3 aabb1_max; // [esp+C4h] [ebp-7Ch] BYREF
    phys_vec3 aabb1_min; // [esp+D4h] [ebp-6Ch] BYREF
    int bpb_cluster_list_count; // [esp+F0h] [ebp-50h]
    float v77; // [esp+F4h] [ebp-4Ch]
    float v78; // [esp+F8h] [ebp-48h]
    broad_phase_base *bpb_cluster_list; // [esp+FCh] [ebp-44h]
    broad_phase_base **bpb_ptr_list; // [esp+100h] [ebp-40h]
    broad_phase_base **bpb_ptr_cur; // [esp+104h] [ebp-3Ch]
    int v82; // [esp+108h] [ebp-38h]
    broad_phase_base *bpb_cluster_head; // [esp+10Ch] [ebp-34h]
    unsigned int env_collision_flags; // [esp+110h] [ebp-30h]
    float v85; // [esp+114h] [ebp-2Ch]
    float v86; // [esp+118h] [ebp-28h]
    float v87; // [esp+11Ch] [ebp-24h]
    float v88; // [esp+120h] [ebp-20h]
    int *v89; // [esp+124h] [ebp-1Ch]

    iassert(eci->m_bpb_count > 0);

    phys_transient_allocator transient_buffer;
    m_bpb_i_start = eci->m_bpb_i_start;
    bpb_ptr_list = (broad_phase_base **)transient_buffer.allocate(sizeof(broad_phase_base *) * eci->m_bpb_count, 4, 0, "phys_transient_allocator out of memory.");
    bpb_ptr_cur = bpb_ptr_list;

    for (broad_phase_base *bpb = eci->m_bpb_i_start; bpb != eci->m_bpb_i_end; bpb = bpb->m_list_bpb_next)
    {
        iassert(bpb_ptr_cur - bpb_ptr_list < eci->m_bpb_count);
        *bpb_ptr_cur++ = bpb;
    }

    iassert(bpb_ptr_cur - bpb_ptr_list == eci->m_bpb_count);



    g_bpb_ptr_list = bpb_ptr_list;
    g_bpb_list_index = 0;
    g_bpb_list_max_index = eci->m_bpb_count;

    bp_aabb_min.x = 1.0e30;
    bp_aabb_min.y = 1.0e30;
    bp_aabb_min.z = 1.0e30;

    bp_aabb_max.x = -1.0e30;
    bp_aabb_max.y = -1.0e30;
    bp_aabb_max.z = -1.0e30;

    broad_phase_prolog_task_input bppti;
    bppti.m_aabb_min = &bp_aabb_min;
    bppti.m_aabb_max = &bp_aabb_max;

    phys_task_manager_process(&bp_env_jq_module1Module, &bppti, eci->m_bpb_count);
    phys_task_manager_flush();

    //*(float *)&bpb_cluster_list_count = bp_aabb_max.x - bp_aabb_min.x;
    //*(float *)&env_collision_flags = bp_aabb_max.y - bp_aabb_min.y;
    //*(float *)&bpb_cluster_list = bp_aabb_max.z - bp_aabb_min.z;
    //if (*(float *)&env_collision_flags > (double)*(float *)&bpb_cluster_list_count)
    //{
    //    g_bpb_cluster_sort_axis = 1;
    //    if (*(float *)&bpb_cluster_list <= (double)*(float *)&env_collision_flags)
    //        goto LABEL_20;
    //}
    //else if (*(float *)&bpb_cluster_list <= (double)*(float *)&bpb_cluster_list_count)
    //{
    //    g_bpb_cluster_sort_axis = 0;
    //    goto LABEL_20;
    //}
    //
    //g_bpb_cluster_sort_axis = 2;
//LABEL_20:

    float extentX = bp_aabb_max.x - bp_aabb_min.x;
    float extentY = bp_aabb_max.y - bp_aabb_min.y;
    float extentZ = bp_aabb_max.z - bp_aabb_min.z;

    if (extentY > extentX)
        g_bpb_cluster_sort_axis = (extentZ > extentY) ? 2 : 1;
    else
        g_bpb_cluster_sort_axis = (extentZ > extentX) ? 2 : 0;

    merge_sort_bpb(bpb_ptr_list, eci->m_bpb_count);

    //v8 = bpb_ptr_list;
    //G_BPM->g_list_bpb = *v8;
    //v9 = eci->m_bpb_count;
    //eci->m_bpb_i_start = *v8;
    //v10 = &v8[v9];
    //for (i = v8; i < v10; ++i)
    //    (*i)->m_list_bpb_next = i[1];
    //(*(i - 1))->m_list_bpb_next = eci->m_bpb_i_end;

    G_BPM->g_list_bpb = bpb_ptr_list[0];
    eci->m_bpb_i_start = bpb_ptr_list[0];
    for (int i = 0; i < eci->m_bpb_count - 1; ++i)
        bpb_ptr_list[i]->m_list_bpb_next = bpb_ptr_list[i + 1];
    bpb_ptr_list[eci->m_bpb_count - 1]->m_list_bpb_next = eci->m_bpb_i_end;

    bpb_cluster_list = NULL;
    bpb_cluster_list_count = 0;

#if 0
    bpb_i = eci->m_bpb_i_start;
    v13 = 0;

    if (bpb_i != eci->m_bpb_i_end)
    {
        while ((bpb_i->m_flags & 0x10) != 0)
        {
        LABEL_73:
            bpb_i = bpb_i->m_list_bpb_next;
            if (bpb_i == eci->m_bpb_i_end)
            {
                v13 = bpb_cluster_list_count;
                goto LABEL_75;
            }
        }

        iassert(bpb_i->get_bpb_cluster_next() == NULL);

        bpb_i->m_flags |= 0x10u;

        x = bpb_i->m_trace_aabb_min_whace.x;
        y = bpb_i->m_trace_aabb_min_whace.y;
        z = bpb_i->m_trace_aabb_min_whace.z;
        *(float *)&env_collision_flags = bpb_i->m_trace_aabb_min_whace.x + bpb_i->m_trace_translation.x;
        v17 = g_bpb_cluster_sort_axis;
        v18 = bpb_i->m_trace_translation.y;
        aabb1_min.x = x;
        v19 = v18 + bpb_i->m_trace_aabb_min_whace.y;
        w = bpb_i->m_trace_aabb_min_whace.w;
        aabb1_min.y = y;
        v21 = bpb_i->m_trace_aabb_max_whace.x;
        *(float *)&bpb_cluster_head = v19;
        aabb1_min.z = z;
        v22 = bpb_i->m_trace_aabb_max_whace.y;
        v23 = bpb_i->m_trace_translation.z + bpb_i->m_trace_aabb_min_whace.z;
        aabb1_min.w = w;
        v24 = bpb_i->m_trace_aabb_max_whace.z;
        aabb1_max.x = v21;
        v88 = v23;
        v25 = bpb_i->m_trace_aabb_max_whace.w;
        aabb1_max.y = v22;
        aabb2_min.x = *(float *)&env_collision_flags;
        m_env_collision_flags = bpb_i->m_env_collision_flags;
        aabb1_max.z = v24;
        aabb2_min.y = *(float *)&bpb_cluster_head;
        aabb1_max.w = v25;
        aabb2_min.z = v88;
        *(float *)&env_collision_flags = bpb_i->m_trace_aabb_max_whace.x + bpb_i->m_trace_translation.x;
        *(float *)&bpb_cluster_head = bpb_i->m_trace_aabb_max_whace.y + bpb_i->m_trace_translation.y;
        v88 = bpb_i->m_trace_aabb_max_whace.z + bpb_i->m_trace_translation.z;

        v27 = *(float *)&env_collision_flags;
        env_collision_flags = m_env_collision_flags;
        aabb2_max.x = v27;
        v28 = *(float *)&bpb_cluster_head;
        bpb_cluster_head = bpb_i;
        aabb2_max.y = v28;
        aabb2_max.z = v88;
        if ((unsigned int)g_bpb_cluster_sort_axis > 2
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                32,
                "i >= 0 && i < 3",
                ""))
        {
            __debugbreak();
        }
        v29 = *(&bpb_i->m_trace_aabb_min_whace.x + v17);
        m_list_bpb_next = bpb_i->m_list_bpb_next;
        for (bpb_i_aabb_min = v29; m_list_bpb_next != eci->m_bpb_i_end; m_list_bpb_next = m_list_bpb_next->m_list_bpb_next)
        {
            v88 = *(float *)&g_bpb_cluster_sort_axis;
            if ((unsigned int)g_bpb_cluster_sort_axis > 2
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                    32,
                    "i >= 0 && i < 3",
                    ""))
            {
                __debugbreak();
            }
            v88 = *(&m_list_bpb_next->m_trace_aabb_min_whace.x + LODWORD(v88));
            if (v88 - bpb_i_aabb_min > 136.0)
                break;
            if ((m_list_bpb_next->m_flags & 0x10) == 0)
            {
                if (m_list_bpb_next->m_list_bpb_cluster_next
                    && _tlAssert("source/phys_broad_phase.cpp", 1108, "bpb_j->get_bpb_cluster_next() == NULL", ""))
                {
                    __debugbreak();
                }
                v88 = m_list_bpb_next->m_trace_aabb_min_whace.z;
                v31 = aabb1_min.z;
                if (v88 < (double)aabb1_min.z)
                    v31 = m_list_bpb_next->m_trace_aabb_min_whace.z;
                *(float *)&bpb_ptr_cur = v31;
                v88 = m_list_bpb_next->m_trace_aabb_min_whace.y;
                v32 = aabb1_min.y;
                if (v88 < (double)aabb1_min.y)
                    v32 = m_list_bpb_next->m_trace_aabb_min_whace.y;
                *(float *)&v82 = v32;
                v88 = m_list_bpb_next->m_trace_aabb_min_whace.x;
                v33 = aabb1_min.x;
                if (v88 < (double)aabb1_min.x)
                    v33 = m_list_bpb_next->m_trace_aabb_min_whace.x;
                *(float *)&bpb_ptr_list = v33;
                v88 = m_list_bpb_next->m_trace_aabb_max_whace.z;
                v34 = aabb1_max.z;
                if (v88 > (double)aabb1_max.z)
                    v34 = m_list_bpb_next->m_trace_aabb_max_whace.z;
                v88 = v34;
                v77 = m_list_bpb_next->m_trace_aabb_max_whace.y;
                v35 = aabb1_max.y;
                if (v77 > (double)aabb1_max.y)
                    v35 = m_list_bpb_next->m_trace_aabb_max_whace.y;
                v77 = v35;
                v78 = m_list_bpb_next->m_trace_aabb_max_whace.x;
                v36 = aabb1_max.x;
                if (v78 > (double)aabb1_max.x)
                    v36 = m_list_bpb_next->m_trace_aabb_max_whace.x;
                v78 = v36;
                v87 = v78 - *(float *)&bpb_ptr_list;
                v85 = v77 - *(float *)&v82;
                v86 = v88 - *(float *)&bpb_ptr_cur;
                if (v87 <= 136.0 && v85 <= 136.0 && v86 <= 136.0)
                {
                    v86 = m_list_bpb_next->m_trace_aabb_min_whace.x + m_list_bpb_next->m_trace_translation.x;
                    v85 = m_list_bpb_next->m_trace_translation.y + m_list_bpb_next->m_trace_aabb_min_whace.y;
                    v87 = m_list_bpb_next->m_trace_translation.z + m_list_bpb_next->m_trace_aabb_min_whace.z;
                    v60.x = v86;
                    v60.y = v85;
                    v60.z = v87;
                    phys_min(&aabb2_min_new, &aabb2_min, &v60);
                    v86 = m_list_bpb_next->m_trace_aabb_max_whace.x + m_list_bpb_next->m_trace_translation.x;
                    v85 = m_list_bpb_next->m_trace_aabb_max_whace.y + m_list_bpb_next->m_trace_translation.y;
                    v87 = m_list_bpb_next->m_trace_aabb_max_whace.z + m_list_bpb_next->m_trace_translation.z;
                    v65.x = v86;
                    v65.y = v85;
                    v65.z = v87;
                    phys_max(&v61, &aabb2_max, &v65);
                    v37 = v61.x;
                    v38 = aabb2_min_new.x;
                    v86 = v61.x - aabb2_min_new.x;
                    v39 = v61.y;
                    v40 = aabb2_min_new.y;
                    v85 = v61.y - aabb2_min_new.y;
                    v41 = v61.z;
                    v42 = aabb2_min_new.z;
                    v87 = v61.z - aabb2_min_new.z;
                    if (v86 <= 136.0 && v85 <= 136.0 && v87 <= 136.0)
                    {
                        v43 = bpb_cluster_head;
                        aabb1_min.x = *(float *)&bpb_ptr_list;
                        v44 = m_list_bpb_next->m_env_collision_flags;
                        v45 = *(float *)&v82;
                        m_list_bpb_next->m_flags |= 0x10u;
                        aabb1_min.y = v45;
                        env_collision_flags |= v44;
                        v46 = *(float *)&bpb_ptr_cur;
                        m_list_bpb_next->m_list_bpb_cluster_next = v43;
                        aabb1_min.z = v46;
                        bpb_cluster_head = m_list_bpb_next;
                        aabb1_max.x = v78;
                        aabb1_max.y = v77;
                        aabb1_max.z = v88;
                        aabb2_min.x = v38;
                        aabb2_min.y = v40;
                        aabb2_min.z = v42;
                        aabb2_max.x = v37;
                        aabb2_max.y = v39;
                        aabb2_max.z = v41;
                    }
                }
            }
        }
        transient_allocator_update_largest_size();
        m_cur = transient_buffer.m_cur;
        v48 = (int)(transient_buffer.m_cur + 15) & 0xFFFFFFF0;
        if ((char *)(v48 + 80) <= transient_buffer.m_end)
        {
            transient_buffer.m_cur = (char *)(v48 + 80);
            v49 = (unsigned int)(m_cur + 15) & 0xFFFFFFF0;
            if (v48)
                goto LABEL_70;
        }
        //phys_transient_allocator::resize(&transient_buffer);
        transient_buffer.resize();
        v50 = (int)(transient_buffer.m_cur + 15) & 0xFFFFFFF0;
        if ((char *)(v50 + 80) <= transient_buffer.m_end)
        {
            transient_buffer.m_cur = (char *)(v50 + 80);
            v88 = *(float *)&v50;
            if (*(float *)&v50 != 0.0)
            {
            LABEL_69:
                v49 = LODWORD(v88);
            LABEL_70:
                v51 = *(float *)&v49;
            LABEL_72:
                ++bpb_cluster_list_count;
                *(float *)(LODWORD(v51) + 52) = *(float *)&bpb_cluster_list;
                *(float *)&bpb_cluster_list = v51;
                comp_trace_volume(&aabb1_min, &aabb1_max, &aabb2_min, &aabb2_max, &p1, &p2, &half_dims);
                v52 = p1.x;
                v53 = half_dims.x;
                v86 = p1.x - half_dims.x;
                v54 = p1.y;
                v55 = half_dims.y;
                v85 = p1.y - half_dims.y;
                v56 = p1.z;
                v57 = half_dims.z;
                v87 = p1.z - half_dims.z;
                *(float *)LODWORD(v51) = v86;
                *(float *)(LODWORD(v51) + 4) = v85;
                *(float *)(LODWORD(v51) + 8) = v87;
                v86 = v53 + v52;
                v85 = v55 + v54;
                v87 = v57 + v56;
                *(float *)(LODWORD(v51) + 16) = v86;
                *(float *)(LODWORD(v51) + 20) = v85;
                *(float *)(LODWORD(v51) + 24) = v87;
                v86 = p2.x - v52;
                v85 = p2.y - v54;
                v87 = p2.z - v56;
                *(float *)(LODWORD(v51) + 32) = v86;
                *(float *)(LODWORD(v51) + 36) = v85;
                *(float *)(LODWORD(v51) + 40) = v87;
                v58 = env_collision_flags;
                *(float *)(LODWORD(v51) + 56) = *(float *)&bpb_cluster_head;
                *(_DWORD *)(LODWORD(v51) + 68) = v58;
                goto LABEL_73;
            }
        }
        else
        {
            v88 = 0.0;
        }
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
            79,
            "ptr",
            "transient allocation too large, increase block_size."))
        {
            __debugbreak();
        }
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
            81,
            "ptr || no_error",
            "phys_transient_allocator out of memory."))
        {
            __debugbreak();
        }
        if (v88 == 0.0)
        {
            v51 = 0.0;
            goto LABEL_72;
        }
        goto LABEL_69;
    }

LABEL_75:

    g_bpb_list_cur = bpb_cluster_list;
    g_bpb_list_index = 0;
    g_bpb_list_max_index = v13;
    g_thread_id = 0;
    phys_task_manager_process(&bp_env_jq_module2Module, 0, v13);
    phys_task_manager_flush();

    transient_buffer.reset();
#else // aislop hybrid
 for (broad_phase_base *bpb_i = eci->m_bpb_i_start; bpb_i != eci->m_bpb_i_end; bpb_i = bpb_i->m_list_bpb_next)
    {
        // Skip BPBs that have already been clustered
        if (bpb_i->is_in_cluster())
            continue;

        iassert(bpb_i->get_bpb_cluster_next() == NULL);

        // Start a new cluster with bpb_i as the head
        bpb_i->m_flags |= broad_phase_base::FLAG_IS_IN_CLUSTER;

        phys_vec3 aabb1_min = bpb_i->m_trace_aabb_min_whace;
        phys_vec3 aabb1_max = bpb_i->m_trace_aabb_max_whace;

        phys_vec3 aabb2_min = {
            bpb_i->m_trace_aabb_min_whace.x + bpb_i->m_trace_translation.x,
            bpb_i->m_trace_aabb_min_whace.y + bpb_i->m_trace_translation.y,
            bpb_i->m_trace_aabb_min_whace.z + bpb_i->m_trace_translation.z
        };
        phys_vec3 aabb2_max = {
            bpb_i->m_trace_aabb_max_whace.x + bpb_i->m_trace_translation.x,
            bpb_i->m_trace_aabb_max_whace.y + bpb_i->m_trace_translation.y,
            bpb_i->m_trace_aabb_max_whace.z + bpb_i->m_trace_translation.z
        };

        unsigned int cluster_env_flags = bpb_i->m_env_collision_flags;
        broad_phase_base *cluster_head = bpb_i;

        iassert(g_bpb_cluster_sort_axis >= 0 && g_bpb_cluster_sort_axis < 3);
        float bpb_i_sort_val = *(&bpb_i->m_trace_aabb_min_whace.x + g_bpb_cluster_sort_axis);

        // Try to merge subsequent BPBs into this cluster
        for (broad_phase_base *bpb_j = bpb_i->m_list_bpb_next;
             bpb_j != eci->m_bpb_i_end;
             bpb_j = bpb_j->m_list_bpb_next)
        {
            iassert(g_bpb_cluster_sort_axis >= 0 && g_bpb_cluster_sort_axis < 3);

            float bpb_j_sort_val = *(&bpb_j->m_trace_aabb_min_whace.x + g_bpb_cluster_sort_axis);

            // Stop searching if BPBs are too far apart along the sort axis
            if (bpb_j_sort_val - bpb_i_sort_val > 136.0f)
                break;

            if (bpb_j->m_flags & 0x10)
                continue;

            iassert(bpb_j->get_bpb_cluster_next() == NULL);

            // Check if merged static AABB (whace) is within the cluster size limit
            phys_vec3 merged1_min = {
                fminf(aabb1_min.x, bpb_j->m_trace_aabb_min_whace.x),
                fminf(aabb1_min.y, bpb_j->m_trace_aabb_min_whace.y),
                fminf(aabb1_min.z, bpb_j->m_trace_aabb_min_whace.z)
            };
            phys_vec3 merged1_max = {
                fmaxf(aabb1_max.x, bpb_j->m_trace_aabb_max_whace.x),
                fmaxf(aabb1_max.y, bpb_j->m_trace_aabb_max_whace.y),
                fmaxf(aabb1_max.z, bpb_j->m_trace_aabb_max_whace.z)
            };

            static constexpr float MAX_CLUSTER_EXTENT = 136.0f;

            if (merged1_max.x - merged1_min.x > MAX_CLUSTER_EXTENT) continue;
            if (merged1_max.y - merged1_min.y > MAX_CLUSTER_EXTENT) continue;
            if (merged1_max.z - merged1_min.z > MAX_CLUSTER_EXTENT) continue;

            // Also check if merged trace AABB (including translation) is within limit
            phys_vec3 bpb_j_aabb2_min = {
                bpb_j->m_trace_aabb_min_whace.x + bpb_j->m_trace_translation.x,
                bpb_j->m_trace_aabb_min_whace.y + bpb_j->m_trace_translation.y,
                bpb_j->m_trace_aabb_min_whace.z + bpb_j->m_trace_translation.z
            };
            phys_vec3 bpb_j_aabb2_max = {
                bpb_j->m_trace_aabb_max_whace.x + bpb_j->m_trace_translation.x,
                bpb_j->m_trace_aabb_max_whace.y + bpb_j->m_trace_translation.y,
                bpb_j->m_trace_aabb_max_whace.z + bpb_j->m_trace_translation.z
            };

            phys_vec3 merged2_min, merged2_max;
            phys_min(&merged2_min, &aabb2_min, &bpb_j_aabb2_min);
            phys_max(&merged2_max, &aabb2_max, &bpb_j_aabb2_max);

            if (merged2_max.x - merged2_min.x > MAX_CLUSTER_EXTENT) continue;
            if (merged2_max.y - merged2_min.y > MAX_CLUSTER_EXTENT) continue;
            if (merged2_max.z - merged2_min.z > MAX_CLUSTER_EXTENT) continue;

            // Accept bpb_j into the cluster
            bpb_j->m_flags |= 0x10;
            bpb_j->m_list_bpb_cluster_next = cluster_head;
            cluster_head = bpb_j;
            cluster_env_flags |= bpb_j->m_env_collision_flags;

            aabb1_min = merged1_min;
            aabb1_max = merged1_max;
            aabb2_min = merged2_min;
            aabb2_max = merged2_max;
        }

        // Allocate a cluster entry from the transient buffer (80 bytes, 16-byte aligned)
        transient_allocator_update_largest_size();

        broad_phase_base *entry = (broad_phase_base *)transient_buffer.allocate(sizeof(broad_phase_base), 16, 0, "phys_transient_allocator out of memory.");

        // Link into cluster list
        entry->m_list_bpb_next = bpb_cluster_list;
        bpb_cluster_list = entry;
        ++bpb_cluster_list_count;

        // Compute the combined trace volume for this cluster
        phys_vec3 p1, p2, half_dims;
        comp_trace_volume(&aabb1_min, &aabb1_max, &aabb2_min, &aabb2_max, &p1, &p2, &half_dims);

        entry->m_trace_aabb_min_whace = { p1.x - half_dims.x, p1.y - half_dims.y, p1.z - half_dims.z };
        entry->m_trace_aabb_max_whace = { p1.x + half_dims.x, p1.y + half_dims.y, p1.z + half_dims.z };
        entry->m_trace_translation = { p2.x - p1.x,         p2.y - p1.y,         p2.z - p1.z };
        entry->m_env_collision_flags = cluster_env_flags;
        entry->m_list_bpb_cluster_next = cluster_head;
    }

    // Phase 3: process all clusters in parallel
    g_bpb_list_cur       = bpb_cluster_list;
    g_bpb_list_index     = 0;
    g_bpb_list_max_index = bpb_cluster_list_count;
    g_thread_id          = 0;

    phys_task_manager_process(&bp_env_jq_module2Module, nullptr, bpb_cluster_list_count);
    phys_task_manager_flush();

    transient_buffer.reset();
#endif
}

void __cdecl broad_phase_process_object_environment_collision()
{
    int g_list_bpb_count; // eax
    int v1; // eax
    bpi_environment_collision_info eci; // [esp+0h] [ebp-10h] BYREF
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    g_list_bpb_count = G_BPM->g_list_bpb_count;
    eci.m_bpb_i_start = G_BPM->g_list_bpb;
    eci.m_bpb_i_end = 0;
    eci.m_bpb_count = g_list_bpb_count;
    for ( eci.m_bpb_last_count = g_list_bpb_count; eci.m_bpb_count; eci.m_bpb_last_count = v1 )
    {
        broad_phase_process_object_environment_collision(&eci);
        eci.m_bpb_i_end = eci.m_bpb_i_start;
        v1 = G_BPM->g_list_bpb_count;
        eci.m_bpb_i_start = G_BPM->g_list_bpb;
        eci.m_bpb_count = v1 - eci.m_bpb_last_count;
    }
}

void __thiscall axis_aligned_sweep_and_prune::swap(
                axis_aligned_sweep_and_prune::axis_element **a1_ptr,
                axis_aligned_sweep_and_prune::axis_element **a2_ptr)
{
    axis_aligned_sweep_and_prune::axis_element *v3; // esi
    axis_aligned_sweep_and_prune::axis_element *v4; // edi
    axis_aligned_sweep_and_prune::sap_node *m_node; // ebx
    bool (__cdecl *m_should_collide_callback)(const broad_phase_base *, const broad_phase_base *); // eax

    v3 = *a1_ptr;
    v4 = *a2_ptr;
    if ( (*a1_ptr)->m_node == (*a2_ptr)->m_node
        && _tlAssert("source/phys_broad_phase.cpp", 570, "a1->m_node != a2->m_node", "") )
    {
        __debugbreak();
    }
    if ( v3->m_ae_list_index + 1 != v4->m_ae_list_index
        && _tlAssert("source/phys_broad_phase.cpp", 571, "a1->m_ae_list_index + 1 == a2->m_ae_list_index", "") )
    {
        __debugbreak();
    }
    ++v3->m_ae_list_index;
    --v4->m_ae_list_index;
    if ( v3->m_min_max == 1 && !v4->m_min_max )
    {
        m_node = v4->m_node;
        if ( axis_aligned_sweep_and_prune::are_overlapping(v3->m_node, v4->m_node) )
        {
            m_should_collide_callback = this->m_should_collide_callback;
            if ( !m_should_collide_callback || m_should_collide_callback(v3->m_node->m_bpb, m_node->m_bpb) )
                axis_aligned_sweep_and_prune::add_active_pair(v3->m_node, v4->m_node);
        }
    }
    *a1_ptr = v4;
    *a2_ptr = v3;
}

void __thiscall axis_aligned_sweep_and_prune::merge_sort(
                axis_aligned_sweep_and_prune::axis_element **list,
                int list_count)
{
    axis_aligned_sweep_and_prune *v3; // esi
    axis_aligned_sweep_and_prune::axis_element **v4; // ebx
    axis_aligned_sweep_and_prune::axis_element **v5; // edi
    axis_aligned_sweep_and_prune::axis_element **v6; // ecx
    axis_aligned_sweep_and_prune::axis_element **i; // esi
    axis_aligned_sweep_and_prune::axis_element **last; // [esp+8h] [ebp-8h]

    v3 = this;
    if ( list_count >= 3 )
    {
        axis_aligned_sweep_and_prune::merge_sort(list, list_count / 2);
        axis_aligned_sweep_and_prune::merge_sort(&list[list_count / 2], list_count - list_count / 2);
        v4 = &list[list_count / 2];
        last = &list[list_count];
        v5 = v4 - 1;
        if ( v4 < last )
        {
            while ( (*v5)->m_val > (double)(*v4)->m_val )
            {
                axis_aligned_sweep_and_prune::swap(v5, v4);
                v6 = v5;
                for ( i = v5 - 1; v6 > list; --i )
                {
                    if ( (*i)->m_val <= (double)(*v6)->m_val )
                        break;
                    axis_aligned_sweep_and_prune::swap(i, v6);
                    v6 = i;
                }
                v5 = v4++;
                if ( v4 >= last )
                    break;
                v3 = this;
            }
        }
    }
    else if ( list_count == 2 && (*list)->m_val > (double)list[1]->m_val )
    {
        axis_aligned_sweep_and_prune::swap(list, list + 1);
    }
}

void __cdecl aasap_list_add(broad_phase_base *bpb)
{
    //axis_aligned_sweep_and_prune::create_sap_node(g_axis_aligned_sweep_and_prune, bpb);
    g_axis_aligned_sweep_and_prune->create_sap_node(bpb);
}

void __cdecl aasap_list_remove(broad_phase_base *bpb)
{
    //axis_aligned_sweep_and_prune::destroy_sap_node(g_axis_aligned_sweep_and_prune, bpb);
    g_axis_aligned_sweep_and_prune->destroy_sap_node(bpb);
}

bool phys_are_potentially_colliding(
    const phys_vec3 *aabb_min0,
    const phys_vec3 *aabb_max0,
    const phys_vec3 *aabb0_translation,
    const phys_vec3 *aabb_min1,
    const phys_vec3 *aabb_max1,
    float *hit_time)
{
    float v7; // [esp+4h] [ebp-38h]
    float v8; // [esp+8h] [ebp-34h]
    float v9; // [esp+Ch] [ebp-30h]
    float v10; // [esp+10h] [ebp-2Ch]
    float dif_max1_min0; // [esp+24h] [ebp-18h]
    float dif_min1_max0; // [esp+28h] [ebp-14h]
    int i; // [esp+2Ch] [ebp-10h]
    float tmax; // [esp+34h] [ebp-8h]
    float tmin; // [esp+38h] [ebp-4h]

    tmin = 0.0f;
    tmax = 1.0f;
    for (i = 0; i < 3; ++i)
    {
        if (i < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                32,
                "i >= 0 && i < 3",
                ""))
        {
            __debugbreak();
        }
        if (i < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                32,
                "i >= 0 && i < 3",
                ""))
        {
            __debugbreak();
        }
        dif_min1_max0 = *(&aabb_min1->x + i) - *(&aabb_max0->x + i);
        if (i < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                32,
                "i >= 0 && i < 3",
                ""))
        {
            __debugbreak();
        }
        if (i < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                32,
                "i >= 0 && i < 3",
                ""))
        {
            __debugbreak();
        }
        dif_max1_min0 = *(&aabb_max1->x + i) - *(&aabb_min0->x + i);
        if (i < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                32,
                "i >= 0 && i < 3",
                ""))
        {
            __debugbreak();
        }
        if (*(&aabb0_translation->x + i) <= 0.0000099999997)
        {
            if (i < 0
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                    32,
                    "i >= 0 && i < 3",
                    ""))
            {
                __debugbreak();
            }
            if ((-(0.0000099999997)) <= *(&aabb0_translation->x + i))
            {
                if (dif_min1_max0 > 0.0 || dif_max1_min0 < 0.0)
                    return 0;
            }
            else
            {
                if (i < 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                        32,
                        "i >= 0 && i < 3",
                        ""))
                {
                    __debugbreak();
                }
                if ((float)(dif_min1_max0 / *(&aabb0_translation->x + i)) <= tmax)
                    v8 = dif_min1_max0 / *(&aabb0_translation->x + i);
                else
                    v8 = tmax;
                tmax = v8;
                if (i < 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                        32,
                        "i >= 0 && i < 3",
                        ""))
                {
                    __debugbreak();
                }
                if (tmin <= (float)(dif_max1_min0 / *(&aabb0_translation->x + i)))
                    v7 = dif_max1_min0 / *(&aabb0_translation->x + i);
                else
                    v7 = tmin;
                tmin = v7;
            }
        }
        else
        {
            if (i < 0
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                    32,
                    "i >= 0 && i < 3",
                    ""))
            {
                __debugbreak();
            }
            if (tmin <= (float)(dif_min1_max0 / *(&aabb0_translation->x + i)))
                v10 = dif_min1_max0 / *(&aabb0_translation->x + i);
            else
                v10 = tmin;
            tmin = v10;
            if (i < 0
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                    32,
                    "i >= 0 && i < 3",
                    ""))
            {
                __debugbreak();
            }
            if ((float)(dif_max1_min0 / *(&aabb0_translation->x + i)) <= tmax)
                v9 = dif_max1_min0 / *(&aabb0_translation->x + i);
            else
                v9 = tmax;
            tmax = v9;
        }
        if (tmin > tmax)
            return 0;
    }
    if ((tmin < 0.0 || tmin > 1.0)
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_bounding_volume.h",
            172,
            "tmin >= 0.0f && tmin <= 1.0f",
            ""))
    {
        __debugbreak();
    }
    *hit_time = tmin;
    return 1;
}

void    do_initial_tunnel_test(
                broad_phase_group *bpg,
                const broad_phase_environement_query_results *bpeqr)
{
    broad_phase_info *m_list_bpi_head; // esi
    const phys_gjk_geom *m_gjk_geom; // ecx
    //phys_gjk_geom_vtbl *v5; // eax
    double v6; // st7
    broad_phase_base_list::node *v7; // eax
    broad_phase_info *m_bpb; // edi
    phys_gjk_info gjk_info; // [esp+14h] [ebp-43Ch] BYREF
    phys_gjk_input pgi; // [esp+3B4h] [ebp-9Ch] BYREF
    phys_vec3 v11; // [esp+404h] [ebp-4Ch] BYREF
    phys_collision_pair pcp; // [esp+418h] [ebp-38h] BYREF
    float reduced_radius; // [esp+42Ch] [ebp-24h]
    broad_phase_base_list::node *last_bpi_env_iter; // [esp+430h] [ebp-20h]
    float v15; // [esp+434h] [ebp-1Ch]
    float hit_time; // [esp+438h] [ebp-18h] BYREF
    broad_phase_base_list::node *bpi_env_iter; // [esp+43Ch] [ebp-14h]
    float smallest_lambda; // [esp+440h] [ebp-10h]
    //_UNKNOWN *v19[2]; // [esp+444h] [ebp-Ch] BYREF
    //int arg4; // [esp+450h] [ebp+0h]
    //
    //v19[0] = a1;
    //v19[1] = (_UNKNOWN *)arg4;
    if ((bpg->m_flags & 0x200) == 0
        && _tlAssert(
            "source/phys_broad_phase.cpp",
            753,
            "bpg->get_flag(broad_phase_group::FLAG_DO_INITIAL_TUNNEL_TEST)",
            ""))
    {
        __debugbreak();
    }
    bpg->m_flags &= ~0x200u;
    m_list_bpi_head = bpg->m_list_bpi_head;
    pcp.m_hit_time = -1.0;
    for (hit_time = 0.0; m_list_bpi_head; m_list_bpi_head = (broad_phase_info *)m_list_bpi_head->m_list_bpb_next)
    {
        m_gjk_geom = m_list_bpi_head->m_gjk_geom;
        //v5 = m_gjk_geom->__vftable;
        smallest_lambda = 1.0;
        //v6 = ((double(__thiscall *)(const phys_gjk_geom *))v5->get_geom_radius)(m_gjk_geom);
        v6 = m_gjk_geom->get_geom_radius();
        v7 = *bpeqr->m_list_bpi_env.m_list_cur;
        bpi_env_iter = bpeqr->m_list_bpi_env.m_list;
        reduced_radius = v6 * 0.8999999761581421 + 0.3400000035762787;
        for (last_bpi_env_iter = v7; bpi_env_iter != last_bpi_env_iter; bpi_env_iter = bpi_env_iter->m_next)
        {
            m_bpb = (broad_phase_info *)bpi_env_iter->m_bpb;
            if ((bpi_env_iter->m_bpb->m_env_collision_flags & m_list_bpi_head->m_env_collision_flags) != 0)
            {
                v11.x = m_list_bpi_head->m_trace_translation.x - m_bpb->m_trace_translation.x;
                v11.y = m_list_bpi_head->m_trace_translation.y - m_bpb->m_trace_translation.y;
                v11.z = m_list_bpi_head->m_trace_translation.z - m_bpb->m_trace_translation.z;
                if (phys_are_potentially_colliding(
                    &m_list_bpi_head->m_trace_aabb_min_whace,
                    &m_list_bpi_head->m_trace_aabb_max_whace,
                    &v11,
                    &m_bpb->m_trace_aabb_min_whace,
                    &m_bpb->m_trace_aabb_max_whace,
                    &hit_time))
                {
                    pcp.m_hit_time = hit_time;
                    pcp.m_bpi1 = m_list_bpi_head;
                    pcp.m_bpi2 = m_bpb;
                    pcp.m_gjk_ci = 0;
                    setup_gjk_input_from_pcp(&pgi, &pcp);
                    pgi.cg1_radius = reduced_radius;
                    pgi.m_sep_thresh = 1.02;
                    pgi.m_intersection_test_only = 0;
                    pgi.m_continuous_collision = 1;
                    //if (phys_gjk_info::phys_collide_do_gjk_collide(&gjk_info, (int)v19, &pgi))
                    if (gjk_info.phys_collide_do_gjk_collide(&pgi))
                    {
                        v15 = gjk_info.cg1_cinfo_loc.m_n.y * gjk_info.m_cg1_relative_translation_loc.y
                            + gjk_info.cg1_cinfo_loc.m_n.x * gjk_info.m_cg1_relative_translation_loc.x
                            + gjk_info.cg1_cinfo_loc.m_n.z * gjk_info.m_cg1_relative_translation_loc.z;
                        if (v15 < 0.0 && smallest_lambda >(double)gjk_info.m_continuous_collision_lambda)
                            smallest_lambda = gjk_info.m_continuous_collision_lambda;
                    }
                }
            }
        }
        //rigid_body::adjust_col_moved_vec(m_list_bpi_head->m_rb, smallest_lambda);
        m_list_bpi_head->m_rb->adjust_col_moved_vec(smallest_lambda);
        //broad_phase_info::collision_prolog(m_list_bpi_head);
        m_list_bpi_head->collision_prolog();
    }
}

void __cdecl destroy_broad_phase_info(broad_phase_info *bpi)
{
    environment_collision_list_remove(bpi);
    g_axis_aligned_sweep_and_prune->destroy_sap_node(bpi);

    if (bpi)
    {
        using TI = phys_free_list<broad_phase_info>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
        static_assert(sizeof(TI) == 0x90, "size mismatch");
#endif
        TI *ti = (TI *)((char *)bpi - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 16);
        G_BPM->g_list_broad_phase_info.remove(ti);
    }

    //if ( bpi )
    //{
    //    PMM_VALIDATE((char *)&bpi[-1].m_gjk_geom, 0x90u, 0x10u);
    //    //phys_free_list<broad_phase_info>::remove(p_g_list_broad_phase_info, (phys_free_list<broad_phase_info>::T_internal *)&bpi[-1].m_gjk_geom);
    //    G_BPM->g_list_broad_phase_info.remove((phys_free_list<broad_phase_info>::T_internal *) &bpi[-1].m_gjk_geom);
    //}
}

void __cdecl destroy_broad_phase_info_list(broad_phase_info *list_bpi)
{
    broad_phase_info *v1; // esi
    broad_phase_info *m_list_bpb_next; // ebx
    char *p_m_gjk_geom; // esi
    phys_free_list<broad_phase_info> *p_g_list_broad_phase_info; // edi
    unsigned int *v5; // eax
    int v6; // ecx

    v1 = list_bpi;
    if ( list_bpi )
    {
        do
        {
            if ( (v1->m_flags & 0x20) != 0
                && _tlAssert(
                         "source/phys_broad_phase.cpp",
                         130,
                         "!bpi->get_flag(broad_phase_base::FLAG_ON_ENV_LIST)",
                         "") )
            {
                __debugbreak();
            }
            if ( v1->m_sap_node && _tlAssert("source/phys_broad_phase.cpp", 131, "bpi->m_sap_node == NULL", "") )
                __debugbreak();
            m_list_bpb_next = (broad_phase_info *)v1->m_list_bpb_next;
            p_m_gjk_geom = (char *)&v1[-1].m_gjk_geom;
            p_g_list_broad_phase_info = &G_BPM->g_list_broad_phase_info;
            PMM_VALIDATE(p_m_gjk_geom, 0x90u, 0x10u);
            if ( !p_m_gjk_geom )
            {
                if ( _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                    __debugbreak();
            }
            --p_g_list_broad_phase_info->m_list_count;
            //phys_free_list<broad_phase_info>::debug_remove(p_g_list_broad_phase_info, (phys_free_list<broad_phase_info>::T_internal *)p_m_gjk_geom);
            p_g_list_broad_phase_info->debug_remove((phys_free_list<broad_phase_info>::T_internal *)p_m_gjk_geom);
            v5 = (unsigned int *)*((unsigned int *)p_m_gjk_geom + 1);
            v6 = *(unsigned int *)p_m_gjk_geom;
            *(unsigned int *)(v6 + 4) = (unsigned int)v5;
            *v5 = v6;
            PMM_FREE((unsigned __int8 *)p_m_gjk_geom, 0x90u, 0x10u);
            v1 = m_list_bpb_next;
        }
        while ( m_list_bpb_next );
    }
}

broad_phase_collision_pair *__cdecl create_broad_phase_collision_pair()
{
    phys_free_list<broad_phase_collision_pair> *p_g_list_broad_phase_collision_pair; // edi
    phys_free_list<broad_phase_collision_pair>::T_internal *v1; // eax
    phys_free_list<broad_phase_collision_pair>::T_internal *v2; // esi

    p_g_list_broad_phase_collision_pair = &G_BPM->g_list_broad_phase_collision_pair;
    v1 = (phys_free_list<broad_phase_collision_pair>::T_internal *)PMM_ALLOC(0x18u, 4u);
    v2 = v1;
    if ( v1 )
    {
        v1->m_prev_T_internal = &p_g_list_broad_phase_collision_pair->m_dummy_head;
        v1->m_next_T_internal = p_g_list_broad_phase_collision_pair->m_dummy_head.m_next_T_internal;
        p_g_list_broad_phase_collision_pair->m_dummy_head.m_next_T_internal->m_prev_T_internal = v1;
        ++p_g_list_broad_phase_collision_pair->m_list_count;
        p_g_list_broad_phase_collision_pair->m_dummy_head.m_next_T_internal = v1;
        //phys_free_list<broad_phase_collision_pair>::debug_add(p_g_list_broad_phase_collision_pair, v1);
        p_g_list_broad_phase_collision_pair->debug_add(v1);
        return &v2->m_data;
    }
    else
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                     470,
                     "no_error",
                     "phys_free_list error: out of memory.") )
        {
            __debugbreak();
        }
        return 0;
    }
}

void __cdecl destroy_broad_phase_collision_pair(broad_phase_collision_pair *bpcp)
{
    phys_free_list<broad_phase_collision_pair> *p_g_list_broad_phase_collision_pair; // edi

    p_g_list_broad_phase_collision_pair = &G_BPM->g_list_broad_phase_collision_pair;
    if ( bpcp )
    {
        PMM_VALIDATE((char *)&bpcp[-1].m_bpi2, 0x18u, 4u);
        //phys_free_list<broad_phase_collision_pair>::remove(p_g_list_broad_phase_collision_pair, (phys_free_list<broad_phase_collision_pair>::T_internal *)&bpcp[-1].m_bpi2);
        p_g_list_broad_phase_collision_pair->remove((phys_free_list<broad_phase_collision_pair>::T_internal *) &bpcp[-1].m_bpi2);
    }
}

void __cdecl destroy_broad_phase_collision_pair_list(broad_phase_collision_pair *list_bpcp)
{
    broad_phase_collision_pair *v1; // eax
    broad_phase_collision_pair *m_next_bpcp; // ebx
    phys_free_list<broad_phase_collision_pair>::T_internal *p_m_bpi2; // esi
    phys_free_list<broad_phase_collision_pair> *p_g_list_broad_phase_collision_pair; // edi
    phys_free_list<broad_phase_collision_pair>::T_internal_base *m_next_T_internal; // eax
    phys_free_list<broad_phase_collision_pair>::T_internal_base *m_prev_T_internal; // ecx

    v1 = list_bpcp;
    if ( list_bpcp )
    {
        do
        {
            m_next_bpcp = v1->m_next_bpcp;
            p_m_bpi2 = (phys_free_list<broad_phase_collision_pair>::T_internal *)&v1[-1].m_bpi2;
            p_g_list_broad_phase_collision_pair = &G_BPM->g_list_broad_phase_collision_pair;
            PMM_VALIDATE((char *)&v1[-1].m_bpi2, 0x18u, 4u);
            if ( !p_m_bpi2 )
            {
                if ( _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                    __debugbreak();
            }
            --p_g_list_broad_phase_collision_pair->m_list_count;
            //phys_free_list<broad_phase_collision_pair>::debug_remove(p_g_list_broad_phase_collision_pair, p_m_bpi2);
            p_g_list_broad_phase_collision_pair->debug_remove(p_m_bpi2);
            m_next_T_internal = p_m_bpi2->m_next_T_internal;
            m_prev_T_internal = p_m_bpi2->m_prev_T_internal;
            m_prev_T_internal->m_next_T_internal = m_next_T_internal;
            m_next_T_internal->m_prev_T_internal = m_prev_T_internal;
            PMM_FREE((unsigned __int8 *)p_m_bpi2, 0x18u, 4u);
            v1 = m_next_bpcp;
        }
        while ( m_next_bpcp );
    }
}

broad_phase_group *__cdecl create_broad_phase_group()
{
    phys_free_list<broad_phase_group> *p_g_list_broad_phase_group; // edi
    phys_free_list<broad_phase_group>::T_internal *v1; // eax
    phys_free_list<broad_phase_group>::T_internal *v2; // esi

    p_g_list_broad_phase_group = &G_BPM->g_list_broad_phase_group;
    v1 = (phys_free_list<broad_phase_group>::T_internal *)PMM_ALLOC(0x80u, 0x10u);
    v2 = v1;
    if ( v1 )
    {
        v1->m_prev_T_internal = &p_g_list_broad_phase_group->m_dummy_head;
        v1->m_next_T_internal = p_g_list_broad_phase_group->m_dummy_head.m_next_T_internal;
        p_g_list_broad_phase_group->m_dummy_head.m_next_T_internal->m_prev_T_internal = v1;
        ++p_g_list_broad_phase_group->m_list_count;
        p_g_list_broad_phase_group->m_dummy_head.m_next_T_internal = v1;
        //phys_free_list<broad_phase_group>::debug_add(p_g_list_broad_phase_group, v1);
        p_g_list_broad_phase_group->debug_add(v1);
        return &v2->m_data;
    }
    else
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                     470,
                     "no_error",
                     "phys_free_list error: out of memory.") )
        {
            __debugbreak();
        }
        return 0;
    }
}

void __cdecl destroy_broad_phase_group(broad_phase_group *bpg)
{
    phys_free_list<broad_phase_group> *p_g_list_broad_phase_group; // edi

    environment_collision_list_remove(bpg);
    //axis_aligned_sweep_and_prune::destroy_sap_node(g_axis_aligned_sweep_and_prune, bpg);
    g_axis_aligned_sweep_and_prune->destroy_sap_node(bpg);
    p_g_list_broad_phase_group = &G_BPM->g_list_broad_phase_group;
    if ( bpg )
    {
        PMM_VALIDATE((char *)&bpg[-1].m_list_bpi_head, 0x80u, 0x10u);
        //phys_free_list<broad_phase_group>::remove(p_g_list_broad_phase_group, (phys_free_list<broad_phase_group>::T_internal *)&bpg[-1].m_list_bpi_head);
        p_g_list_broad_phase_group->remove((phys_free_list<broad_phase_group>::T_internal *) &bpg[-1].m_list_bpi_head);
    }
}

char    bpi_do_gjk_intersect(
                broad_phase_info *p1,
                broad_phase_info *p2,
                float hit_time)
{
    phys_gjk_input pgi; // [esp+394h] [ebp-7Ch] BYREF
    phys_collision_pair pcp; // [esp+3E8h] [ebp-28h] BYREF
    unsigned int id1; // [esp+3FCh] [ebp-14h]
    unsigned int id2; // [esp+400h] [ebp-10h]

    iassert(hit_time >= 0.0f && hit_time <= 1.0f);

    id1 = p1->m_gjk_geom_id;
    id2 = p2->m_gjk_geom_id;

    iassert(p1 && p2 && p1 != p2);
    iassert(id1 != id2);

    pcp.m_hit_time = hit_time;
    pcp.m_bpi1 = p1;
    pcp.m_bpi2 = p2;
    pcp.m_gjk_ci = G_BPM->g_phys_gjk_cache_system.get_gjk_cache_info_mutex(id1, id2, &G_BPM->g_bp_gjk_cache_mutex, 1);
    setup_gjk_input_from_pcp(&pgi, &pcp);
    pgi.m_sep_thresh = 1.02;
    pgi.m_intersection_test_only = 1;
    pgi.m_continuous_collision = 1;

    phys_gjk_info gjk;
    return gjk.phys_collide_do_gjk_collide(&pgi);
}

// char __usercall phys_are_potentially_colliding_whace<broad_phase_info,broad_phase_info>@<al>
// omitting template stuff ^^ - only used in 1 distinct typing
bool phys_are_potentially_colliding_whace(broad_phase_info *p1, broad_phase_info *p2, float *hit_time)
{
    phys_vec3 relative_translation = {
        p1->m_trace_translation.x - p2->m_trace_translation.x,
        p1->m_trace_translation.y - p2->m_trace_translation.y,
        p1->m_trace_translation.z - p2->m_trace_translation.z
    };

    return phys_are_potentially_colliding(
        &p1->m_trace_aabb_min_whace,
        &p1->m_trace_aabb_max_whace,
        &relative_translation,
        &p2->m_trace_aabb_min_whace,
        &p2->m_trace_aabb_max_whace,
        hit_time);
}

void collide_bpi_environment(broad_phase_info *bpi, const broad_phase_environement_query_results &bpeqr)
{
    if (!(bpi->m_env_collision_flags & bpeqr.m_env_collision_flags))
        return;

    broad_phase_base_list::node *iter = bpeqr.m_list_bpi_env.m_list;
    broad_phase_base_list::node *end  = *bpeqr.m_list_bpi_env.m_list_cur;

    for (; iter != end; iter = iter->m_next)
    {
        broad_phase_base *bpi_env = iter->m_bpb;
        float hit_time = 0.0f;

        if (!(bpi_env->m_env_collision_flags & bpi->m_env_collision_flags))
            continue;

        if (!phys_are_potentially_colliding_whace(bpi, (broad_phase_info *)bpi_env, &hit_time))
            continue;

        if (bpi_env->m_flags & 0x80)
        {
            // bpi_env has an auto-activate controller. Try to wake it via GJK; binary
            // never adds a collision pair on this branch.
            phys_auto_activate_callback *aac = (phys_auto_activate_callback *)bpi_env->m_sap_node;

            if (aac->has_auto_activated())
                continue;

            if (!bpi_do_gjk_intersect(bpi, (broad_phase_info *)bpi_env, hit_time))
                continue;

            G_BPM->g_bp_auto_activate_mutex.Lock();
            if (!aac->has_auto_activated())
                aac->auto_activate(bpi);
            G_BPM->g_bp_auto_activate_mutex.Unlock();
        }
        else
        {
            add_collision_pair_mutex(bpi, (broad_phase_info *)bpi_env, hit_time, nullptr);
        }
    }
}

#if 0
void    collide_bpg_environment(
                broad_phase_group *bpg,
                const broad_phase_environement_query_results *bpeqr)
{
    broad_phase_info *v3; // edi
    broad_phase_base_list::node *v4; // ecx
    broad_phase_base *m_bpb; // esi
    broad_phase_info *m_rb; // edi
    phys_wheel_collide_info *m_sap_node; // ecx
    broad_phase_memory *v8; // eax
    phys_wheel_collide_info *v9; // eax
    phys_wheel_collide_info *m_cg_to_rb_xform; // edi
    phys_wheel_collide_info *p_z; // eax
    phys_vec3 v12; // [esp+18h] [ebp-4Ch] BYREF
    phys_vec3 v13; // [esp+28h] [ebp-3Ch] BYREF
    broad_phase_base_list::node *i; // [esp+40h] [ebp-24h]
    phys_wheel_collide_info *v15; // [esp+44h] [ebp-20h]
    LONG v16; // [esp+48h] [ebp-1Ch] BYREF
    broad_phase_base_list::node *last_bpi_env_iter; // [esp+4Ch] [ebp-18h]
    phys_wheel_collide_info *last_wci; // [esp+50h] [ebp-14h]
    float v19; // [esp+54h] [ebp-10h] BYREF
    //broad_phase_base_list::node *bpi_env_iter; // [esp+58h] [ebp-Ch] BYREF
    //phys_auto_activate_callback *aac; // [esp+5Ch] [ebp-8h]
    //phys_auto_activate_callback *retaddr; // [esp+64h] [ebp+0h]
    //
    //bpi_env_iter = a1;
    //aac = retaddr;
    v19 = 0.0;
    v3 = (broad_phase_info *)bpg;
    if ((bpeqr->m_env_collision_flags & bpg->m_env_collision_flags) != 0)
    {
        if ((bpg->m_flags & 0x200) != 0)
            do_initial_tunnel_test(bpg,bpeqr);
        v4 = *bpeqr->m_list_bpi_env.m_list_cur;
        last_bpi_env_iter = bpeqr->m_list_bpi_env.m_list;
        for (i = v4; last_bpi_env_iter != i; last_bpi_env_iter = last_bpi_env_iter->m_next)
        {
            m_bpb = last_bpi_env_iter->m_bpb;
            if ((last_bpi_env_iter->m_bpb->m_env_collision_flags & v3->m_env_collision_flags) != 0
                && phys_are_potentially_colliding_whace(
                    v3,
                    (broad_phase_info *)m_bpb,
                    &v19))
            {
                m_rb = (broad_phase_info *)v3->m_rb;
                if (SLOBYTE(m_bpb->m_flags) >= 0)
                {
                    for (; m_rb; m_rb = (broad_phase_info *)m_rb->m_list_bpb_next)
                    {
                        if ((m_rb->m_env_collision_flags & m_bpb->m_env_collision_flags) != 0)
                        {
                            v13.x = m_rb->m_trace_translation.x - m_bpb->m_trace_translation.x;
                            v13.y = m_rb->m_trace_translation.y - m_bpb->m_trace_translation.y;
                            v13.z = m_rb->m_trace_translation.z - m_bpb->m_trace_translation.z;
                            if (phys_are_potentially_colliding(
                                &m_rb->m_trace_aabb_min_whace,
                                &m_rb->m_trace_aabb_max_whace,
                                &v13,
                                &m_bpb->m_trace_aabb_min_whace,
                                &m_bpb->m_trace_aabb_max_whace,
                                &v19))
                            {
                                add_collision_pair_mutex(m_rb, (broad_phase_info *)m_bpb, v19, 0);
                            }
                        }
                    }
                    if (bpg->m_rbvm)
                    {
                        if (!bpg->m_list_wci && _tlAssert("source/phys_broad_phase.cpp", 835, "bpg->m_list_wci", ""))
                            __debugbreak();
                        m_cg_to_rb_xform = bpg->m_list_wci;
                        v15 = &m_cg_to_rb_xform[bpg->m_rbvm->m_wheels.m_alloc_count];
                        if (m_cg_to_rb_xform != v15)
                        {
                            p_z = (phys_wheel_collide_info *)&m_cg_to_rb_xform->m_ray_dir.z;
                            for (last_wci = (phys_wheel_collide_info *)&m_cg_to_rb_xform->m_ray_dir.z; ; p_z = last_wci)
                            {
                                v12.x = *((float *)&p_z[-1].m_hit_bpi + 1) - m_bpb->m_trace_translation.x;
                                v12.y = *((float *)&p_z[-1].m_hit_bpi + 2) - m_bpb->m_trace_translation.y;
                                v12.z = p_z->m_ray_pos.x - m_bpb->m_trace_translation.z;
                                if (phys_are_potentially_colliding(
                                    &m_cg_to_rb_xform->m_ray_pos,
                                    &m_cg_to_rb_xform->m_ray_pos,
                                    &v12,
                                    &m_bpb->m_trace_aabb_min_whace,
                                    &m_bpb->m_trace_aabb_max_whace,
                                    &v19))
                                {
                                    //phys_wheel_collide_info::collision_process(
                                        m_cg_to_rb_xform->collision_process((broad_phase_info *)m_bpb);
                                }
                                ++last_wci;
                                if (++m_cg_to_rb_xform == v15)
                                    break;
                            }
                        }
                    }
                }
                else
                {
                    m_sap_node = (phys_wheel_collide_info *)m_bpb->m_sap_node;
                    last_wci = m_sap_node;
                    if (m_rb)
                    {
                        while (!(*(unsigned __int8(__thiscall **)(phys_wheel_collide_info *))LODWORD(m_sap_node->m_ray_pos.x))(m_sap_node))
                        {
                            if ((m_rb->m_env_collision_flags & m_bpb->m_env_collision_flags) != 0
                                && phys_are_potentially_colliding_whace(
                                    m_rb,
                                    (broad_phase_info *)m_bpb,
                                    &v19)
                                && bpi_do_gjk_intersect(
                                    m_rb,
                                    (broad_phase_info *)m_bpb,
                                    0.0f))
                            {
                                //tlAtomicMutex::Lock(&G_BPM->g_bp_auto_activate_mutex);
                                G_BPM->g_bp_auto_activate_mutex.Lock();
                                if (!(*(unsigned __int8(__thiscall **)(phys_wheel_collide_info *))LODWORD(last_wci->m_ray_pos.x))(last_wci))
                                    (*(void(__thiscall **)(phys_wheel_collide_info *, broad_phase_info *))(LODWORD(last_wci->m_ray_pos.x)
                                        + 4))(
                                            last_wci,
                                            m_rb);

                                G_BPM->g_bp_auto_activate_mutex.Unlock();
                                //v8 = G_BPM;
                                //--G_BPM->g_bp_auto_activate_mutex.LockCount;
                                //v15 = (phys_wheel_collide_info *)v8;
                                //if (!v8->g_bp_auto_activate_mutex.LockCount)
                                //{
                                //    v16 = 0;
                                //    InterlockedExchange(&v16, 0);
                                //    v9 = v15;
                                //    v15->m_ray_pos.x = 0.0;
                                //    v9->m_ray_pos.y = 0.0;
                                //}

                            }
                            m_rb = (broad_phase_info *)m_rb->m_list_bpb_next;
                            if (!m_rb)
                                break;
                            m_sap_node = last_wci;
                        }
                    }
                }
                v3 = (broad_phase_info *)bpg;
            }
        }
    }
    //broad_phase_group::collision_epilog((broad_phase_group *)v3);
    v3->get_bpg()->collision_epilog();
}
#else
// aisloppd
void collide_bpg_environment(broad_phase_group *bpg, const broad_phase_environement_query_results &bpeqr)
{
    // Early out if no collision flag overlap
    if (!(bpg->m_env_collision_flags & bpeqr.m_env_collision_flags))
    {
        bpg->collision_epilog();
        return;
    }

    // Optional tunnel test for vehicle groups
    if (bpg->m_flags & 0x200)
        do_initial_tunnel_test(bpg, &bpeqr);

    broad_phase_base_list::node *end = *bpeqr.m_list_bpi_env.m_list_cur;
    broad_phase_base_list::node *iter = bpeqr.m_list_bpi_env.m_list;

    if (iter == end)
    {
        bpg->collision_epilog();
        return;
    }

    for (; iter != end; iter = iter->m_next)
    {
        broad_phase_base *bpi_env = iter->m_bpb;

        if (!(bpi_env->m_env_collision_flags & bpg->m_env_collision_flags))
            continue;

        float hit_time = 0.0f;
        if (!phys_are_potentially_colliding_whace((broad_phase_info *)bpg, (broad_phase_info *)bpi_env, &hit_time))
            continue;

        if (bpi_env->m_flags & 0x80) // bpi_env has auto-activate controller
        {
            phys_auto_activate_callback *aac = (phys_auto_activate_callback *)bpi_env->m_sap_node;

            // Iterate BPG's broad_phase_info list via m_list_bpb_next
            for (broad_phase_info *bpi = bpg->m_list_bpi_head;
                bpi != nullptr;
                bpi = (broad_phase_info *)bpi->m_list_bpb_next)
            {
                if (aac->has_auto_activated())
                    break;

                if (!(bpi->m_env_collision_flags & bpi_env->m_env_collision_flags))
                    continue;

                if (!phys_are_potentially_colliding_whace(bpi, (broad_phase_info *)bpi_env, &hit_time))
                    continue;

                if (!bpi_do_gjk_intersect(bpi, (broad_phase_info *)bpi_env, hit_time))
                    continue;

                G_BPM->g_bp_auto_activate_mutex.Lock();

                if (!aac->has_auto_activated())
                    aac->auto_activate(bpi);

                G_BPM->g_bp_auto_activate_mutex.Unlock();
            }
        }
        else
        {
            // Normal path: iterate BPG's BPI list and add collision pairs
            for (broad_phase_info *bpi = bpg->m_list_bpi_head;
                bpi != nullptr;
                bpi = (broad_phase_info *)bpi->m_list_bpb_next)
            {
                if (!(bpi->m_env_collision_flags & bpi_env->m_env_collision_flags))
                    continue;

                phys_vec3 relative_translation = {
                    bpi->m_trace_translation.x - bpi_env->m_trace_translation.x,
                    bpi->m_trace_translation.y - bpi_env->m_trace_translation.y,
                    bpi->m_trace_translation.z - bpi_env->m_trace_translation.z
                };

                if (!phys_are_potentially_colliding(
                    &bpi->m_trace_aabb_min_whace,
                    &bpi->m_trace_aabb_max_whace,
                    &relative_translation,
                    &bpi_env->m_trace_aabb_min_whace,
                    &bpi_env->m_trace_aabb_max_whace,
                    &hit_time))
                    continue;

                add_collision_pair_mutex(bpi, (broad_phase_info *)bpi_env, hit_time, nullptr);
            }

            // Wheel collision path
            if (bpg->m_rbvm)
            {
                iassert(bpg->m_list_wci);

                phys_wheel_collide_info *wci = bpg->m_list_wci;
                phys_wheel_collide_info *wci_end = wci + bpg->m_rbvm->m_wheels.m_alloc_count;

                for (; wci != wci_end; ++wci)
                {
                    // Wheel uses m_ray_pos as a point AABB (min == max)
                    // relative translation is wci->m_ray_dir - bpi_env->m_trace_translation
                    phys_vec3 wheel_relative_translation = {
                        wci->m_ray_dir.x - bpi_env->m_trace_translation.x,
                        wci->m_ray_dir.y - bpi_env->m_trace_translation.y,
                        wci->m_ray_dir.z - bpi_env->m_trace_translation.z
                    };

                    if (!phys_are_potentially_colliding(
                        &wci->m_ray_pos,  // point AABB — min == max
                        &wci->m_ray_pos,
                        &wheel_relative_translation,
                        &bpi_env->m_trace_aabb_min_whace,
                        &bpi_env->m_trace_aabb_max_whace,
                        &hit_time))
                        continue;

                    wci->collision_process((broad_phase_info *)bpi_env);
                }
            }
        }
    }

    bpg->collision_epilog();
}
#endif

int    bp_env_jq_batch_function2(jqBatch *pBatch)
{
    int v1; // eax
    broad_phase_base *bpb; // esi
    broad_phase_environment_query_input bpeqi; // [esp-Ch] [ebp-6Ch] BYREF
    broad_phase_environement_query_results bpeqr; // [esp+40h] [ebp-20h] BYREF
    
    v1 = _InterlockedExchangeAdd(&g_thread_id, 1u);
    bpb = g_bpb_list_cur;
    bpeqr.m_list_bpi_env.m_list = 0;
    bpeqr.m_list_bpi_env.m_list_cur = (broad_phase_base_list::node **)&bpeqr;
    for (bpeqr.m_thread_id = v1; g_bpb_list_cur; bpb = g_bpb_list_cur)
    {
        if ((broad_phase_base *)_InterlockedCompareExchange(
            (volatile unsigned __int32 *)&g_bpb_list_cur,
            (signed __int32)bpb->m_list_bpb_next,
            (signed __int32)bpb) == bpb)
        {
            check_terrain_query_params(bpb);
            bpeqr.m_list_bpi_env.m_list_cur = (broad_phase_base_list::node **)&bpeqr;
            bpeqr.m_list_bpi_env_count = 0;
            bpeqr.m_env_collision_flags = 0;
            init_bpeqi(&bpeqi, bpb);
            G_BPM->g_broad_phase_terrain_query_callback->query(&bpeqi, &bpeqr);
            process_cluster_environment_collision(bpb, bpeqr);
        }
    }
    return 0;
}

void    broad_phase_process()
{
    broad_phase_base *i; // esi
    axis_aligned_sweep_and_prune::sap_node *m_sap_node; // ecx

    G_BPM->m_list_bpi_env = 0;
    G_BPM->m_bpi_env_count = 0;
    G_BPM->m_bpg_env_count = 0;
    G_BPM->m_bpg_env_bpi_count = 0;
    G_BPM->m_bpi_env_no_database_count = 0;

    iassert(!tlScratchpadLocked); // "Scratchpad is already locked!"
    tlScratchpadLocked = 1;

    iassert(G_BPM->g_collision_memory_buffer.is_empty());

    G_BPM->g_list_phys_collide_data.clear();

    broad_phase_process_object_environment_collision();
    broad_phase_process_collision_pairs();

    for ( i = G_BPM->g_list_bpb; i; i = i->m_list_bpb_next )
    {
        m_sap_node = (axis_aligned_sweep_and_prune::sap_node *)i->m_sap_node;
        if (m_sap_node)
        {
            //axis_aligned_sweep_and_prune::sap_node::update_ae_val(m_sap_node, a1);
            m_sap_node->update_ae_val();
        }
    }

    //axis_aligned_sweep_and_prune::process(g_axis_aligned_sweep_and_prune);
    g_axis_aligned_sweep_and_prune->process();

    for (phys_collision_pair *pair = G_BPM->g_list_phys_collide_data.m_first; pair; pair = pair->m_next_link )
    {
        if ( !pair->m_gjk_ci )
        {
            pair->m_gjk_ci = G_BPM->g_phys_gjk_cache_system.get_gjk_cache_info(pair->m_bpi1->m_gjk_geom_id, pair->m_bpi2->m_gjk_geom_id, 1);
        }
    }

    if (G_BPM->g_collision_memory_buffer.m_total_memory_allocated > G_BPM->m_memory_high_water)
    {
        G_BPM->m_memory_high_water = G_BPM->g_collision_memory_buffer.m_total_memory_allocated;
    }

    process_list_do_gjk_collide_and_contact_manifold(&G_BPM->g_list_phys_collide_data);

    G_BPM->g_list_phys_collide_data.clear();

    iassert(tlScratchpadLocked); // "Scratchpad is already unlocked!"
    tlScratchpadLocked = 0;

    //phys_heap_gjk_cache_system_avl_tree::update_cache(&G_BPM->g_phys_gjk_cache_system);
    G_BPM->g_phys_gjk_cache_system.update_cache();
    //bpei_database_t::update_database(&G_BPM->g_bpei_database);
    G_BPM->g_bpei_database.update_database();
}

void __cdecl broad_phase_system_init(
                const broad_phase_memory_info *bpmi,
                bool (__cdecl *should_collide_callback)(const broad_phase_base *, const broad_phase_base *))
{
    //break_here_39 = 1;

    G_BPM = broad_phase_memory::allocate_buffer(bpmi);

    g_axis_aligned_sweep_and_prune = (axis_aligned_sweep_and_prune *)PMM_PERM_ALLOCATE(sizeof(axis_aligned_sweep_and_prune), 4);
    g_axis_aligned_sweep_and_prune->m_sap_node_allocator.m_count = 0;
    g_axis_aligned_sweep_and_prune->m_active_pair_allocator.m_count = 0;

    G_BPM->g_broad_phase_terrain_query_callback = 0;
    G_BPM->g_list_bpb = 0;
    G_BPM->g_list_bpb_count = 0;
    G_BPM->g_list_broad_phase_info.remove_all();
    G_BPM->g_list_broad_phase_group.remove_all();
    G_BPM->g_list_broad_phase_collision_pair.remove_all();
    G_BPM->g_bpei_database.purge_database();
    G_BPM->m_memory_high_water = 0;

    g_axis_aligned_sweep_and_prune->init_system(bpmi->m_max_num_sap_active_pairs);
    g_axis_aligned_sweep_and_prune->m_should_collide_callback = should_collide_callback;
}

void __cdecl broad_phase_system_shutdown()
{
    //broad_phase_memory::~broad_phase_memory(G_BPM);
    G_BPM->~broad_phase_memory();
    G_BPM = 0;
    g_axis_aligned_sweep_and_prune = 0;
}

void    broad_phase_group::collision_prolog()
{
    broad_phase_group *v2; // edi
    bool v3; // zf
    broad_phase_info *m_list_bpi_head; // eax
    double v5; // st7
    float y; // edx
    float z; // ecx
    float x; // ecx
    double v9; // st7
    float w; // edx
    float v11; // ecx
    double v12; // st7
    double v13; // st7
    float v14; // edx
    unsigned int m_env_collision_flags; // ecx
    double v16; // st7
    double v17; // st7
    broad_phase_info *m_list_bpb_next; // esi
    double v19; // st7
    double v20; // st7
    double v21; // st7
    double v22; // st7
    double v23; // st7
    double v24; // st7
    double v25; // st7
    double v26; // st7
    double v27; // st7
    double v28; // st7
    double v29; // st7
    double v30; // st7
    unsigned int v31; // edx
    double v32; // st7
    rb_vehicle_model *m_rbvm; // eax
    phys_wheel_collide_info *v34; // eax
    int m_buffer; // esi
    int v36; // eax
    phys_vec3 *v37; // esi
    float v38; // edi
    float *p_x; // edi
    double v40; // st7
    double v41; // st7
    double v42; // st7
    double v43; // st7
    double v44; // st7
    double v45; // st7
    double v46; // st7
    double v47; // st7
    double v48; // st7
    double v49; // st7
    double v50; // st7
    double v51; // st7
    bool v52; // cc
    double v53; // st7
    double v54; // st6
    double v55; // st5
    double v56; // st4
    double v57; // st3
    double v58; // st2
    double v59; // st1
    double v60; // st6
    double v61; // st1
    double v62; // st4
    double v63; // st1
    double v64; // st2
    phys_vec3 v65; // [esp-Ch] [ebp-BCh] BYREF
    phys_vec3 v66; // [esp+4h] [ebp-ACh] BYREF
    const phys_mat44 *half_dims_12; // [esp+18h] [ebp-98h]
    const phys_mat44 *rb_mat; // [esp+1Ch] [ebp-94h]
    int wheel_count; // [esp+20h] [ebp-90h]
    phys_vec3 aabb2_min; // [esp+24h] [ebp-8Ch] BYREF
    phys_vec3 aabb2_max; // [esp+34h] [ebp-7Ch] BYREF
    phys_vec3 p1; // [esp+44h] [ebp-6Ch] BYREF
    phys_vec3 aabb1_max; // [esp+54h] [ebp-5Ch] BYREF
    phys_vec3 aabb1_min; // [esp+64h] [ebp-4Ch] BYREF
    float v75; // [esp+74h] [ebp-3Ch]
    float v76; // [esp+78h] [ebp-38h]
    float v77; // [esp+7Ch] [ebp-34h]
    float v78; // [esp+90h] [ebp-20h]
    float v79; // [esp+94h] [ebp-1Ch]
    float v80; // [esp+98h] [ebp-18h]
    int i; // [esp+9Ch] [ebp-14h]
    float v82; // [esp+A0h] [ebp-10h]
    //_UNKNOWN *v83[2]; // [esp+A4h] [ebp-Ch] BYREF
    //int vars0; // [esp+B0h] [ebp+0h]
    //
    //v83[0] = a2;
    //v83[1] = (_UNKNOWN *)vars0;
    v2 = this;
    v3 = this->m_list_bpi_head == 0;
    wheel_count = (int)this;
    if (v3
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
            184,
            "m_list_bpi_head",
            ""))
    {
        __debugbreak();
    }
    //broad_phase_info::collision_prolog(v2->m_list_bpi_head);
    v2->m_list_bpi_head->collision_prolog();
    m_list_bpi_head = v2->m_list_bpi_head;
    v5 = m_list_bpi_head->m_trace_translation.x + m_list_bpi_head->m_trace_aabb_min_whace.x;
    y = m_list_bpi_head->m_trace_aabb_min_whace.y;
    aabb1_min.x = m_list_bpi_head->m_trace_aabb_min_whace.x;
    z = m_list_bpi_head->m_trace_aabb_min_whace.z;
    aabb2_min.x = v5;
    aabb1_min.z = z;
    x = m_list_bpi_head->m_trace_aabb_max_whace.x;
    v9 = m_list_bpi_head->m_trace_translation.y + m_list_bpi_head->m_trace_aabb_min_whace.y;
    aabb1_min.y = y;
    w = m_list_bpi_head->m_trace_aabb_min_whace.w;
    aabb1_max.x = x;
    v11 = m_list_bpi_head->m_trace_aabb_max_whace.z;
    aabb2_min.y = v9;
    v12 = m_list_bpi_head->m_trace_translation.z;
    aabb1_min.w = w;
    v13 = v12 + m_list_bpi_head->m_trace_aabb_min_whace.z;
    v14 = m_list_bpi_head->m_trace_aabb_max_whace.y;
    aabb1_max.z = v11;
    m_env_collision_flags = m_list_bpi_head->m_env_collision_flags;
    aabb2_min.z = v13;
    aabb1_max.y = v14;
    v16 = m_list_bpi_head->m_trace_aabb_max_whace.x + m_list_bpi_head->m_trace_translation.x;
    aabb1_max.w = m_list_bpi_head->m_trace_aabb_max_whace.w;
    aabb2_max.x = v16;
    aabb2_max.y = m_list_bpi_head->m_trace_aabb_max_whace.y + m_list_bpi_head->m_trace_translation.y;
    v17 = m_list_bpi_head->m_trace_aabb_max_whace.z + m_list_bpi_head->m_trace_translation.z;
    v2->m_env_collision_flags = m_env_collision_flags;
    m_list_bpb_next = (broad_phase_info *)m_list_bpi_head->m_list_bpb_next;
    for (aabb2_max.z = v17; m_list_bpb_next; aabb2_max.z = *(float *)&i)
    {
        //broad_phase_info::collision_prolog(m_list_bpb_next);
        m_list_bpb_next->collision_prolog();
        v19 = aabb1_min.z;
        if (m_list_bpb_next->m_trace_aabb_min_whace.z < (double)aabb1_min.z)
            v19 = m_list_bpb_next->m_trace_aabb_min_whace.z;
        v82 = v19;
        v20 = aabb1_min.y;
        if (m_list_bpb_next->m_trace_aabb_min_whace.y < (double)aabb1_min.y)
            v20 = m_list_bpb_next->m_trace_aabb_min_whace.y;
        *(float *)&i = v20;
        v21 = aabb1_min.x;
        if (m_list_bpb_next->m_trace_aabb_min_whace.x < (double)aabb1_min.x)
            v21 = m_list_bpb_next->m_trace_aabb_min_whace.x;
        v80 = v21;
        aabb1_min.x = v80;
        aabb1_min.y = *(float *)&i;
        aabb1_min.z = v82;
        v22 = aabb1_max.z;
        if (m_list_bpb_next->m_trace_aabb_max_whace.z > (double)aabb1_max.z)
            v22 = m_list_bpb_next->m_trace_aabb_max_whace.z;
        *(float *)&i = v22;
        v23 = aabb1_max.y;
        if (m_list_bpb_next->m_trace_aabb_max_whace.y > (double)aabb1_max.y)
            v23 = m_list_bpb_next->m_trace_aabb_max_whace.y;
        v80 = v23;
        v24 = aabb1_max.x;
        if (m_list_bpb_next->m_trace_aabb_max_whace.x > (double)aabb1_max.x)
            v24 = m_list_bpb_next->m_trace_aabb_max_whace.x;
        v82 = v24;
        aabb1_max.x = v82;
        aabb1_max.y = v80;
        aabb1_max.z = *(float *)&i;
        v75 = m_list_bpb_next->m_trace_aabb_max_whace.x + m_list_bpb_next->m_trace_translation.x;
        v76 = m_list_bpb_next->m_trace_aabb_max_whace.y + m_list_bpb_next->m_trace_translation.y;
        v77 = m_list_bpb_next->m_trace_aabb_max_whace.z + m_list_bpb_next->m_trace_translation.z;
        p1.x = m_list_bpb_next->m_trace_aabb_min_whace.x + m_list_bpb_next->m_trace_translation.x;
        p1.y = m_list_bpb_next->m_trace_translation.y + m_list_bpb_next->m_trace_aabb_min_whace.y;
        p1.z = m_list_bpb_next->m_trace_translation.z + m_list_bpb_next->m_trace_aabb_min_whace.z;
        v25 = aabb2_min.z;
        if (p1.z < (double)aabb2_min.z)
            v25 = p1.z;
        *(float *)&i = v25;
        v26 = aabb2_min.y;
        if (p1.y < (double)aabb2_min.y)
            v26 = p1.y;
        v80 = v26;
        v27 = aabb2_min.x;
        if (p1.x < (double)aabb2_min.x)
            v27 = p1.x;
        v82 = v27;
        aabb2_min.x = v82;
        aabb2_min.y = v80;
        aabb2_min.z = *(float *)&i;
        v28 = aabb2_max.z;
        if (v77 > (double)aabb2_max.z)
            v28 = v77;
        *(float *)&i = v28;
        v29 = aabb2_max.y;
        if (v76 > (double)aabb2_max.y)
            v29 = v76;
        v80 = v29;
        v30 = aabb2_max.x;
        if (v75 > (double)aabb2_max.x)
            v30 = v75;
        v31 = m_list_bpb_next->m_env_collision_flags;
        v82 = v30;
        v32 = v82;
        v2->m_env_collision_flags |= v31;
        m_list_bpb_next = (broad_phase_info *)m_list_bpb_next->m_list_bpb_next;
        aabb2_max.x = v32;
        aabb2_max.y = v80;
    }
    m_rbvm = v2->m_rbvm;
    if (m_rbvm)
    {
        rb_mat = (const phys_mat44 *)m_rbvm->m_wheels.m_alloc_count;
        //v34 = (phys_wheel_collide_info *)phys_transient_allocator::mt_allocate(&G_BPM->g_collision_memory_buffer, m_rbvm->m_wheels.m_alloc_count << 6, 16, 0, "broad phase collision out of memory.");
        v34 = (phys_wheel_collide_info *)G_BPM->g_collision_memory_buffer.mt_allocate(m_rbvm->m_wheels.m_alloc_count << 6, 16, 0, "broad phase collision out of memory.");
        m_buffer = (int)v2->m_rbvm->m_wheels.m_buffer;
        v2->m_list_wci = v34;
        if (*(int *)(m_buffer + 20) <= 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_array_base.inc",
                118,
                "i >= 0 && i < m_alloc_count",
                ""))
        {
            __debugbreak();
        }
        half_dims_12 = (const phys_mat44 *)(***(_DWORD ***)(m_buffer + 16) + 48);
        v36 = 0;
        *(float *)&i = 0.0;
        if ((int)rb_mat > 0)
        {
            v80 = 0.0;
            while (1)
            {
                v37 = (phys_vec3 *)((char *)&v2->m_list_wci->m_ray_pos + LODWORD(v80));
                LODWORD(v38) = (unsigned int)v2->m_rbvm->m_wheels.m_buffer;
                v79 = v38;
                if ((v36 < 0 || v36 >= *(_DWORD *)(LODWORD(v38) + 20))
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_array_base.inc",
                        118,
                        "i >= 0 && i < m_alloc_count",
                        ""))
                {
                    __debugbreak();
                }
                p_x = &v37[1].x;
                //rigid_body_constraint_wheel::get_wheel_collide_segment*(rigid_body_constraint_wheel **)(*(_DWORD *)(LODWORD(v79) + 16) + 4 * i), (int)v83, half_dims_12, v37, v37 + 1);
                (*(rigid_body_constraint_wheel **)(*(_DWORD *)(LODWORD(v79) + 16) + 4 * i))->get_wheel_collide_segment(half_dims_12, v37, v37 + 1);
                v37[1].x = v37[1].x - v37->x;
                v37[1].y = v37[1].y - v37->y;
                v37[1].z = v37[1].z - v37->z;
                v37[3].y = 0.0;
                v37[3].x = 1.0;
                v40 = aabb1_min.z;
                if (v37->z < (double)aabb1_min.z)
                    v40 = v37->z;
                v79 = v40;
                v41 = aabb1_min.y;
                if (v37->y < (double)aabb1_min.y)
                    v41 = v37->y;
                v78 = v41;
                v42 = aabb1_min.x;
                if (v37->x < (double)aabb1_min.x)
                    v42 = v37->x;
                v82 = v42;
                aabb1_min.x = v82;
                aabb1_min.y = v78;
                aabb1_min.z = v79;
                v43 = aabb1_max.z;
                if (v37->z > (double)aabb1_max.z)
                    v43 = v37->z;
                v82 = v43;
                v44 = aabb1_max.y;
                if (v37->y > (double)aabb1_max.y)
                    v44 = v37->y;
                v78 = v44;
                v45 = aabb1_max.x;
                if (v37->x > (double)aabb1_max.x)
                    v45 = v37->x;
                v79 = v45;
                aabb1_max.x = v79;
                aabb1_max.y = v78;
                aabb1_max.z = v82;
                p1.x = v37->x + *p_x;
                p1.y = v37[1].y + v37->y;
                p1.z = v37[1].z + v37->z;
                v75 = v37->x + *p_x;
                v76 = v37[1].y + v37->y;
                v77 = v37[1].z + v37->z;
                v46 = aabb2_min.z;
                if (v77 < (double)aabb2_min.z)
                    v46 = v77;
                v82 = v46;
                v47 = aabb2_min.y;
                if (v76 < (double)aabb2_min.y)
                    v47 = v76;
                v78 = v47;
                v48 = aabb2_min.x;
                if (v75 < (double)aabb2_min.x)
                    v48 = v75;
                v79 = v48;
                aabb2_min.x = v79;
                aabb2_min.y = v78;
                aabb2_min.z = v82;
                v49 = aabb2_max.z;
                if (p1.z > (double)aabb2_max.z)
                    v49 = p1.z;
                v82 = v49;
                v50 = aabb2_max.y;
                if (p1.y > (double)aabb2_max.y)
                    v50 = p1.y;
                v78 = v50;
                v51 = aabb2_max.x;
                if (p1.x > (double)aabb2_max.x)
                    v51 = p1.x;
                v79 = v51;
                LODWORD(v80) += 64;
                v2 = (broad_phase_group *)wheel_count;
                aabb2_max.x = v79;
                v52 = i + 1 < (int)rb_mat;
                aabb2_max.y = v78;
                ++i;
                aabb2_max.z = v82;
                if (!v52)
                    break;
                v36 = i;
            }
        }
    }
    comp_trace_volume(&aabb1_min, &aabb1_max, &aabb2_min, &aabb2_max, &p1, &v65, &v66);
    v53 = p1.x;
    v54 = v66.x;
    v75 = p1.x - v66.x;
    v55 = p1.y;
    v56 = v66.y;
    v76 = p1.y - v66.y;
    v57 = p1.z;
    v58 = v66.z;
    v77 = p1.z - v66.z;
    v2->m_trace_aabb_min_whace.x = v75;
    v2->m_trace_aabb_min_whace.y = v76;
    v2->m_trace_aabb_min_whace.z = v77;
    v59 = v54 + v65.x;
    v60 = v65.x;
    v75 = v59;
    v61 = v56 + v65.y;
    v62 = v65.y;
    v76 = v61;
    v63 = v58 + v65.z;
    v64 = v65.z;
    v77 = v63;
    v2->m_trace_aabb_max_whace.x = v75;
    v2->m_trace_aabb_max_whace.y = v76;
    v2->m_trace_aabb_max_whace.z = v77;
    v75 = v60 - v53;
    v76 = v62 - v55;
    v77 = v64 - v57;
    v2->m_trace_translation.x = v75;
    v2->m_trace_translation.y = v76;
    v2->m_trace_translation.z = v77;
}

void __thiscall broad_phase_group::collision_epilog()
{
    broad_phase_group *v1; // edi
    int v2; // esi
    phys_wheel_collide_info *v3; // ebx
    int m_buffer; // edi
    int wheel_count; // [esp+8h] [ebp-Ch]
    int v7; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    v1 = this;
    v2 = 0;
    if ( this->m_rbvm )
    {
        if ( !this->m_list_wci && _tlAssert("source/phys_broad_phase.cpp", 69, "m_list_wci", "") )
            __debugbreak();
        wheel_count = v1->m_rbvm->m_wheels.m_alloc_count;
        if ( wheel_count > 0 )
        {
            v7 = 0;
            while ( 1 )
            {
                v3 = &v1->m_list_wci[v7];
                m_buffer = (int)v1->m_rbvm->m_wheels.m_buffer;
                if ( v2 < 0 || v2 >= *(unsigned int *)(m_buffer + 20) )
                {
                    if ( _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_array_base.inc",
                                 118,
                                 "i >= 0 && i < m_alloc_count",
                                 "") )
                    {
                        __debugbreak();
                    }
                }
                //phys_wheel_collide_info::collision_epilog(v3,(int)&savedregs, *(rigid_body_constraint_wheel **)(*(unsigned int *)(m_buffer + 16) + 4 * v2));
                v3->collision_epilog(*(rigid_body_constraint_wheel **)(*(unsigned int *)(m_buffer + 16) + 4 * v2));
                ++v7;
                if ( ++v2 >= wheel_count )
                    break;
                v1 = this;
            }
        }
    }
}

void    axis_aligned_sweep_and_prune::sap_node::update_ae_val()
{
    phys_vec3 minmaxs[2]; // [esp-Ch] [ebp-2Ch] OVERLAPPED BYREF

    iassert(m_updated == 0);

    this->m_updated = 1;

    //broad_phase_base::get_aabb(m_bpb, (int)v5, v4);
    this->m_bpb->get_aabb(minmaxs);

    this->m_ae1[0][0].m_val = minmaxs[0].x;
    this->m_ae1[0][1].m_val = minmaxs[1].x;
    this->m_ae1[1][0].m_val = minmaxs[0].y;
    this->m_ae1[1][1].m_val = minmaxs[1].y;
    this->m_ae1[2][0].m_val = minmaxs[0].z;
    this->m_ae1[2][1].m_val = minmaxs[1].z;
}

void __thiscall axis_aligned_sweep_and_prune::init_system(int max_num_active_pairs)
{
    iassert(m_active_pair_allocator.get_count() == 0);
    iassert(m_sap_node_allocator.get_count() == 0);

    this->m_x_list = 0;
    this->m_y_list = 0;
    this->m_z_list = 0;
    this->m_list_bpi_bpi = 0;
    this->m_list_bpi_bpg = 0;
    this->m_list_bpg_bpg = 0;
    this->m_max_num_active_pairs = max_num_active_pairs;
}

void    process_cluster_environment_collision_prolog(broad_phase_info *info, broad_phase_base *bpb)
{
    if ( info->is_bpi() )
    {
        //broad_phase_info::collision_prolog(bpb);
        info->collision_prolog();
    }
    else
    {
        iassert(info->is_bpg());
        //broad_phase_group::collision_prolog((broad_phase_group *)bpb, (int)v7);
        info->get_bpg()->collision_prolog();
    }

    info->m_flags &= ~(broad_phase_base::FLAG_IS_IN_CLUSTER);
    info->m_list_bpb_cluster_next = 0;

    phys_vec3 tmp; // [esp-10h] [ebp-1Ch] BYREF

    phys_min(&tmp, &bpb->m_trace_aabb_min_whace, &info->m_trace_aabb_min_whace);
    bpb->m_trace_aabb_min_whace.x = tmp.x;
    bpb->m_trace_aabb_min_whace.y = tmp.y;
    bpb->m_trace_aabb_min_whace.z = tmp.z;

    phys_max(&tmp, &bpb->m_trace_aabb_max_whace, &info->m_trace_aabb_max_whace);
    bpb->m_trace_aabb_max_whace.x = tmp.x;
    bpb->m_trace_aabb_max_whace.y = tmp.y;
    bpb->m_trace_aabb_max_whace.z = tmp.z;

    nanassertvec3(info->m_trace_aabb_min_whace); // LWSS ADD
    nanassertvec3(info->m_trace_aabb_max_whace); // LWSS ADD
    nanassertvec3(bpb->m_trace_aabb_min_whace); // LWSS ADD
    nanassertvec3(bpb->m_trace_aabb_max_whace); // LWSS ADD
}

bool compare_bpb(broad_phase_base *bpb1, broad_phase_base *bpb2)
{
    return bpb2->m_trace_aabb_min_whace[g_bpb_cluster_sort_axis] > bpb1->m_trace_aabb_min_whace[g_bpb_cluster_sort_axis];
}

bool phys_are_potentially_colliding_whace_broad_phase_info_broad_phase_info_(
                broad_phase_info *p1,
                broad_phase_info *p2,
                float *hit_time)
{
    phys_vec3 translation; // [esp-Ch] [ebp-1Ch] BYREF

    translation.x = p1->m_trace_translation.x - p2->m_trace_translation.x;
    translation.y = p1->m_trace_translation.y - p2->m_trace_translation.y;
    translation.z = p1->m_trace_translation.z - p2->m_trace_translation.z;

    return phys_are_potentially_colliding(
        &p1->m_trace_aabb_min_whace,
        &p1->m_trace_aabb_max_whace,
        &translation,
        &p2->m_trace_aabb_min_whace,
        &p2->m_trace_aabb_max_whace,
        hit_time);
}

void __cdecl add_collision_pair(
                broad_phase_info *bpi1,
                broad_phase_info *bpi2,
                float hit_time,
                phys_gjk_cache_info *gjk_ci)
{
    phys_collision_pair *pair; // esi

    iassert(hit_time >= 0.0f && hit_time <= 1.0f);

    pair = (phys_collision_pair *)G_BPM->g_collision_memory_buffer.allocate(sizeof(phys_collision_pair), 4, 0, "phys_transient_allocator out of memory.");

    pair->m_hit_time = hit_time;
    pair->m_bpi1 = bpi1;
    pair->m_bpi2 = bpi2;
    pair->m_gjk_ci = gjk_ci;

    G_BPM->g_list_phys_collide_data.add(pair);
}

void __cdecl add_collision_pair_mutex(
                broad_phase_info *bpi1,
                broad_phase_info *bpi2,
                float hit_time,
                phys_gjk_cache_info *gjk_ci)
{
    phys_collision_pair *pair; // eax

    iassert(hit_time >= 0.0f && hit_time <= 1.0f);

    pair = (phys_collision_pair *)G_BPM->g_collision_memory_buffer.mt_allocate(sizeof(phys_collision_pair), 4, 0, "broad phase collision out of memory.");

    iassert(pair); // LWSS ADD

    //if ( pair )
    //    pair->m_hit_time = -1.0;
    //else
    //    pair = 0;

    pair->m_hit_time = hit_time;
    pair->m_bpi1 = bpi1;
    pair->m_gjk_ci = gjk_ci;
    pair->m_bpi2 = bpi2;

    G_BPM->g_list_phys_collide_data.add_mt(pair);
}

void __thiscall axis_aligned_sweep_and_prune::add_active_pair(
                axis_aligned_sweep_and_prune::sap_node *p1,
                axis_aligned_sweep_and_prune::sap_node *p2)
{
    axis_aligned_sweep_and_prune::active_pair *v4; // eax

    if ( this->m_active_pair_allocator.m_count >= this->m_max_num_active_pairs )
    {
        if ( (pai_max_num_sap_active_pair.m_hits_total_count < pai_max_num_sap_active_pair.m_max_hits_total
             || !pai_max_num_sap_active_pair.m_max_hits_total)
            && pai_max_num_sap_active_pair.m_hits_frame_count < pai_max_num_sap_active_pair.m_max_hits_per_frame )
        {
            if ( pai_max_num_sap_active_pair.m_use_warnings_only )
            {
                PHYS_WARNING(
                    "source/phys_broad_phase.cpp",
                    309,
                    "m_active_pair_allocator.get_count() < m_max_num_active_pairs",
                    "max num sap active pairs reached.");
            }
            else if ( _tlAssert(
                                    "source/phys_broad_phase.cpp",
                                    309,
                                    "m_active_pair_allocator.get_count() < m_max_num_active_pairs",
                                    "max num sap active pairs reached.") )
            {
                __debugbreak();
            }
        }
        _InterlockedExchangeAdd(&pai_max_num_sap_active_pair.m_hits_total_count, 1u);
        _InterlockedExchangeAdd(&pai_max_num_sap_active_pair.m_hits_frame_count, 1u);
    }
    if ( this->m_active_pair_allocator.m_count < this->m_max_num_active_pairs )
    {
        v4 = (axis_aligned_sweep_and_prune::active_pair *)PMM_ALLOC(0x10u, 4u);
        if ( v4 )
        {
            ++this->m_active_pair_allocator.m_count;
            v4->m_p1 = p1;
            v4->m_p2 = p2;
            v4->m_gjk_ci = 0;
            if ( p1->m_bpb->is_bpi() )
            {
                if ( p2->m_bpb->is_bpi() )
                {
                    v4->m_next = this->m_list_bpi_bpi;
                    this->m_list_bpi_bpi = v4;
                }
                else
                {
                    v4->m_next = this->m_list_bpi_bpg;
                    this->m_list_bpi_bpg = v4;
                }
            }
            else if ( p2->m_bpb->is_bpi() )
            {
                v4->m_p1 = p2;
                v4->m_p2 = p1;
                v4->m_gjk_ci = 0;
                v4->m_next = this->m_list_bpi_bpg;
                this->m_list_bpi_bpg = v4;
            }
            else
            {
                v4->m_next = this->m_list_bpg_bpg;
                this->m_list_bpg_bpg = v4;
            }
        }
        else
        {
            if ( (pai_create_sap_active_pair.m_hits_total_count < pai_create_sap_active_pair.m_max_hits_total
                 || !pai_create_sap_active_pair.m_max_hits_total)
                && pai_create_sap_active_pair.m_hits_frame_count < pai_create_sap_active_pair.m_max_hits_per_frame )
            {
                if ( pai_create_sap_active_pair.m_use_warnings_only )
                {
                    PHYS_WARNING("source/phys_broad_phase.cpp", 313, "ap", "list sap active pairs out of memory.");
                }
                else if ( _tlAssert("source/phys_broad_phase.cpp", 313, "ap", "list sap active pairs out of memory.") )
                {
                    __debugbreak();
                }
            }
            _InterlockedExchangeAdd(&pai_create_sap_active_pair.m_hits_total_count, 1u);
            _InterlockedExchangeAdd(&pai_create_sap_active_pair.m_hits_frame_count, 1u);
        }
    }
}

void __thiscall axis_aligned_sweep_and_prune::create_sap_node(
                broad_phase_base *bpb)
{
    char *v3; // eax
    axis_aligned_sweep_and_prune::sap_node *v4; // edi

    if ( bpb->m_sap_node && _tlAssert("source/phys_broad_phase.cpp", 496, "bpb->m_sap_node == NULL", "") )
        __debugbreak();
    v3 = PMM_ALLOC(0x80u, 4u);
    if ( v3 )
    {
        ++this->m_sap_node_allocator.m_count;
        v4 = (axis_aligned_sweep_and_prune::sap_node *)v3;
    }
    else
    {
        v4 = 0;
    }
    //axis_aligned_sweep_and_prune::sap_node::init(v4, bpb, &this->m_x_list, &this->m_y_list, &this->m_z_list);
    v4->init(bpb, &this->m_x_list, &this->m_y_list, &this->m_z_list);
    bpb->m_sap_node = v4;
}

void __thiscall axis_aligned_sweep_and_prune::remove(
                axis_aligned_sweep_and_prune::active_pair **list_ap,
                axis_aligned_sweep_and_prune::sap_node *node)
{
    int i; // esi
    char *v4; // edi
    phys_heap_gjk_cache_system_avl_tree *p_g_phys_gjk_cache_system; // ebx

    for ( i = (int)*list_ap; *list_ap; i = (int)*list_ap )
    {
        if ( *(axis_aligned_sweep_and_prune::sap_node **)i == node
            || *(axis_aligned_sweep_and_prune::sap_node **)(i + 4) == node )
        {
            *list_ap = *(axis_aligned_sweep_and_prune::active_pair **)(i + 8);
            v4 = *(char **)(i + 12);
            p_g_phys_gjk_cache_system = &G_BPM->g_phys_gjk_cache_system;
            if ( v4 )
            {
                PMM_VALIDATE(v4, 0x90u, 0x10u);
                --p_g_phys_gjk_cache_system->m_list_phys_gjk_cache_info_internal.m_count;
                PMM_FREE((unsigned __int8 *)v4, 0x90u, 0x10u);
            }
            PMM_VALIDATE((char *)i, 0x10u, 4u);
            --this->m_active_pair_allocator.m_count;
            PMM_FREE((unsigned __int8 *)i, 0x10u, 4u);
        }
        else
        {
            list_ap = (axis_aligned_sweep_and_prune::active_pair **)(i + 8);
        }
    }
}

void __thiscall axis_aligned_sweep_and_prune::destroy_sap_node(broad_phase_base *bpb)
{
    axis_aligned_sweep_and_prune::sap_node *m_sap_node; // edi

    m_sap_node = (axis_aligned_sweep_and_prune::sap_node *)bpb->m_sap_node;
    if ( m_sap_node )
    {
        axis_aligned_sweep_and_prune::remove(&this->m_list_bpi_bpi, (axis_aligned_sweep_and_prune::sap_node *)bpb->m_sap_node);
        axis_aligned_sweep_and_prune::remove(&this->m_list_bpi_bpg, m_sap_node);
        axis_aligned_sweep_and_prune::remove(&this->m_list_bpg_bpg, m_sap_node);
        axis_aligned_sweep_and_prune::remove(&this->m_x_list, m_sap_node);
        axis_aligned_sweep_and_prune::remove(&this->m_y_list, m_sap_node);
        axis_aligned_sweep_and_prune::remove(&this->m_z_list, m_sap_node);
        PMM_VALIDATE((char *)m_sap_node, 0x80u, 4u);
        --this->m_sap_node_allocator.m_count;
        PMM_FREE((unsigned __int8 *)m_sap_node, 0x80u, 4u);
        bpb->m_sap_node = 0;
    }
}

void    broad_phase_process_collision_pairs()
{
    phys_free_list<broad_phase_collision_pair>::T_internal_base *m_next_T_internal; // esi
    phys_free_list<broad_phase_collision_pair> *i; // edi
    const phys_vec3 *m_prev_T_internal; // ecx
    const phys_vec3 *v4; // eax
    phys_vec3 v5; // [esp+18h] [ebp-2Ch] BYREF
    float hit_time; // [esp+34h] [ebp-10h] BYREF
    //_UNKNOWN *v7; // [esp+38h] [ebp-Ch]
    //int v8; // [esp+3Ch] [ebp-8h]
    //int vars0; // [esp+44h] [ebp+0h]
    //
    //v7 = a1;
    //v8 = vars0;
    hit_time = 0.0;
    m_next_T_internal = G_BPM->g_list_broad_phase_collision_pair.m_dummy_head.m_next_T_internal;
    for (i = &G_BPM->g_list_broad_phase_collision_pair;
        i != (phys_free_list<broad_phase_collision_pair> *)m_next_T_internal;
        m_next_T_internal = m_next_T_internal->m_next_T_internal)
    {
        m_prev_T_internal = (const phys_vec3 *)m_next_T_internal[1].m_prev_T_internal;
        v4 = (const phys_vec3 *)m_next_T_internal[1].m_next_T_internal;
        v5.x = m_prev_T_internal[2].x - v4[2].x;
        v5.y = m_prev_T_internal[2].y - v4[2].y;
        v5.z = m_prev_T_internal[2].z - v4[2].z;
        if (phys_are_potentially_colliding(m_prev_T_internal, m_prev_T_internal + 1, &v5, v4, v4 + 1, &hit_time))
            add_collision_pair(
                (broad_phase_info *)m_next_T_internal[1].m_prev_T_internal,
                (broad_phase_info *)m_next_T_internal[1].m_next_T_internal,
                hit_time,
                0);
    }
}

extern phys_assert_info pai_gjk_cache_system_max_num_gjk_ci;
extern phys_assert_info pai_gjk_cache_system_create_gjk_ci;
phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *__thiscall phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info_mutex(
                unsigned int id1,
                unsigned int id2,
                tlAtomicReadWriteMutex *query_mutex,
                bool __formal)
{
    unsigned int v5; // esi
    unsigned int v6; // edi
    bool v8; // cc
    phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *v10; // eax
    phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *v11; // esi
    phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *v13; // eax
    unsigned int m_id2; // ecx
    phys_gjk_geom_id_pair_key key; // [esp+Ch] [ebp-8h] BYREF
    bool swapped; // [esp+1Ch] [ebp+8h]

    v5 = id1;
    v6 = id2;
    v8 = id1 <= id2;
    if ( id1 == id2 )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                     347,
                     "id1 != id2",
                     "") )
        {
            __debugbreak();
        }
        v8 = id1 <= id2;
    }
    if ( v8 )
    {
        swapped = 0;
    }
    else
    {
        v5 = id2;
        swapped = 1;
        v6 = id1;
    }
    if ( v5 >= v6
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                 24,
                 "id1 < id2",
                 "") )
    {
        __debugbreak();
    }
    key.m_id1 = v5;
    key.m_id2 = v6;
    //tlAtomicReadWriteMutex::ReadLock(query_mutex);
    query_mutex->ReadLock();
    //v10 = phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::
    // phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor>::find(
    v10 = this->m_search_tree.find(key);
    v11 = v10;
    _InterlockedExchangeAdd(&query_mutex->ThisPtr->ReadLockCount, 0xFFFFFFFF);
    if ( v10 )
    {
        //phys_gjk_cache_info::update_swapped(v10, (int)&savedregs, swapped);
        v10->update_swapped(swapped);
        return v11;
    }
    if ( this->m_list_phys_gjk_cache_info_internal.m_count >= this->m_max_num_gjk_ci )
    {
        if ( (pai_gjk_cache_system_max_num_gjk_ci.m_hits_total_count < pai_gjk_cache_system_max_num_gjk_ci.m_max_hits_total
             || !pai_gjk_cache_system_max_num_gjk_ci.m_max_hits_total)
            && pai_gjk_cache_system_max_num_gjk_ci.m_hits_frame_count < pai_gjk_cache_system_max_num_gjk_ci.m_max_hits_per_frame )
        {
            if ( pai_gjk_cache_system_max_num_gjk_ci.m_use_warnings_only )
            {
                PHYS_WARNING(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                    368,
                    "m_list_phys_gjk_cache_info_internal.get_count() < m_max_num_gjk_ci",
                    "max num gjk_ci reached.");
            }
            else if ( _tlAssert(
                                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                                    368,
                                    "m_list_phys_gjk_cache_info_internal.get_count() < m_max_num_gjk_ci",
                                    "max num gjk_ci reached.") )
            {
                __debugbreak();
            }
        }
        _InterlockedExchangeAdd(&pai_gjk_cache_system_max_num_gjk_ci.m_hits_total_count, 1u);
        _InterlockedExchangeAdd(&pai_gjk_cache_system_max_num_gjk_ci.m_hits_frame_count, 1u);
    }
    if ( this->m_list_phys_gjk_cache_info_internal.m_count >= this->m_max_num_gjk_ci )
        return v11;
    //tlAtomicReadWriteMutex::WriteLock(query_mutex);
    query_mutex->WriteLock();
    //if ( phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor>::find(
    //             &this->m_search_tree,
    //             &key)
    //    && _tlAssert(
    //             "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
    //             372,
    //             "m_search_tree.find(key) == NULL",
    //             "") )
    //{
    //    __debugbreak();
    //}
    iassert(m_search_tree.find(key) == NULL);
    v13 = (phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *)PMM_ALLOC(0x90u, 0x10u);
    v11 = v13;
    if ( v13 )
    {
        ++this->m_list_phys_gjk_cache_info_internal.m_count;
        m_id2 = key.m_id2;
        v13->m_key.m_id1 = key.m_id1;
        v13->m_key.m_id2 = m_id2;
        v13->m_flags = 0;
        v13->m_flags = swapped ? 2 : 0;
        //phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree
        //    ::phys_gjk_cache_info_internal::avl_tree_accessor>::add(
        //    &this->m_search_tree,
        //    &key,
        //    v13);
        this->m_search_tree.add(key, v13);
        v11->m_next_gjk_ci = this->m_list_head;
        this->m_list_head = v11;
        //tlAtomicReadWriteMutex::WriteUnlock(query_mutex);
        query_mutex->WriteUnlock();
        return v11;
    }
    if ( (pai_gjk_cache_system_create_gjk_ci.m_hits_total_count < pai_gjk_cache_system_create_gjk_ci.m_max_hits_total
         || !pai_gjk_cache_system_create_gjk_ci.m_max_hits_total)
        && pai_gjk_cache_system_create_gjk_ci.m_hits_frame_count < pai_gjk_cache_system_create_gjk_ci.m_max_hits_per_frame )
    {
        if ( pai_gjk_cache_system_create_gjk_ci.m_use_warnings_only )
        {
            PHYS_WARNING(
                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                374,
                "gjk_ci",
                "gjk_ci memory allocation failed.");
        }
        else if ( _tlAssert(
                                "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_gjk_cache_system.h",
                                374,
                                "gjk_ci",
                                "gjk_ci memory allocation failed.") )
        {
            __debugbreak();
        }
    }
    _InterlockedExchangeAdd(&pai_gjk_cache_system_create_gjk_ci.m_hits_total_count, 1u);
    _InterlockedExchangeAdd(&pai_gjk_cache_system_create_gjk_ci.m_hits_frame_count, 1u);
    //tlAtomicReadWriteMutex::WriteUnlock(query_mutex);
    query_mutex->WriteUnlock();
    return 0;
}

void    axis_aligned_sweep_and_prune::process_active_pair_list()
{
    axis_aligned_sweep_and_prune::active_pair **p_m_list_bpi_bpi; // edx
    axis_aligned_sweep_and_prune::active_pair *m_list_bpi_bpi; // edi
    axis_aligned_sweep_and_prune::sap_node *m_p1; // eax
    axis_aligned_sweep_and_prune::sap_node *m_p2; // ecx
    broad_phase_base *m_bpb; // eax
    bool v7; // zf
    broad_phase_info **v8; // eax
    broad_phase_info *v9; // esi
    phys_heap_gjk_cache_system_avl_tree *v10; // esi
    phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *v11; // eax
    char *m_gjk_ci; // esi
    axis_aligned_sweep_and_prune::active_pair *m_list_bpi_bpg; // esi
    axis_aligned_sweep_and_prune::active_pair **p_m_list_bpi_bpg; // edx
    axis_aligned_sweep_and_prune::sap_node *v15; // eax
    axis_aligned_sweep_and_prune::sap_node *v16; // ecx
    broad_phase_info *v17; // edi
    axis_aligned_sweep_and_prune::sap_node *v18; // eax
    broad_phase_base *v19; // esi
    broad_phase_info *i; // esi
    char *v21; // edi
    axis_aligned_sweep_and_prune::active_pair *m_list_bpg_bpg; // edi
    axis_aligned_sweep_and_prune::active_pair **p_m_list_bpg_bpg; // edx
    axis_aligned_sweep_and_prune::sap_node *v24; // eax
    axis_aligned_sweep_and_prune::sap_node *v25; // ecx
    broad_phase_base *v26; // esi
    broad_phase_info **v27; // eax
    broad_phase_info *v28; // edi
    broad_phase_info *j; // esi
    broad_phase_info *k; // edi
    char *v31; // esi
    phys_vec3 v32; // [esp+18h] [ebp-4Ch] BYREF
    phys_vec3 v33; // [esp+28h] [ebp-3Ch] BYREF
    phys_heap_gjk_cache_system_avl_tree *p_g_phys_gjk_cache_system; // [esp+44h] [ebp-20h]
    broad_phase_info *v35; // [esp+48h] [ebp-1Ch]
    axis_aligned_sweep_and_prune::active_pair **ap_i; // [esp+4Ch] [ebp-18h]
    axis_aligned_sweep_and_prune *v37; // [esp+50h] [ebp-14h]
    float hit_time; // [esp+54h] [ebp-10h] BYREF
    //_UNKNOWN *v39; // [esp+58h] [ebp-Ch]
    //int v40; // [esp+5Ch] [ebp-8h]
    //int vars0; // [esp+64h] [ebp+0h]
    //
    //v39 = a2;
    //v40 = vars0;
    axis_aligned_sweep_and_prune *thisptr = this; // :)

    p_m_list_bpi_bpi = &thisptr->m_list_bpi_bpi;
    m_list_bpi_bpi = thisptr->m_list_bpi_bpi;
    v37 = thisptr;
    ap_i = &thisptr->m_list_bpi_bpi;
    if (m_list_bpi_bpi)
    {
        while (1)
        {
            m_p1 = m_list_bpi_bpi->m_p1;
            m_p2 = m_list_bpi_bpi->m_p2;
            if (m_list_bpi_bpi->m_p1->m_ae1[0][0].m_ae_list_index >= m_p2->m_ae1[0][1].m_ae_list_index
                || m_p2->m_ae1[0][0].m_ae_list_index >= m_p1->m_ae1[0][1].m_ae_list_index
                || m_p1->m_ae1[1][0].m_ae_list_index >= m_p2->m_ae1[1][1].m_ae_list_index
                || m_p2->m_ae1[1][0].m_ae_list_index >= m_p1->m_ae1[1][1].m_ae_list_index
                || m_p1->m_ae1[2][0].m_ae_list_index >= m_p2->m_ae1[2][1].m_ae_list_index
                || m_p2->m_ae1[2][0].m_ae_list_index >= m_p1->m_ae1[2][1].m_ae_list_index)
            {
                *p_m_list_bpi_bpi = m_list_bpi_bpi->m_next;
                m_gjk_ci = (char *)m_list_bpi_bpi->m_gjk_ci;
                p_g_phys_gjk_cache_system = &G_BPM->g_phys_gjk_cache_system;
                if (m_gjk_ci)
                {
                    PMM_VALIDATE(m_gjk_ci, 0x90u, 0x10u);
                    --p_g_phys_gjk_cache_system->m_list_phys_gjk_cache_info_internal.m_count;
                    PMM_FREE((unsigned __int8 *)m_gjk_ci, 0x90u, 0x10u);
                }
                PMM_VALIDATE((char *)m_list_bpi_bpi, 0x10u, 4u);
                --v37->m_active_pair_allocator.m_count;
                PMM_FREE((unsigned __int8 *)m_list_bpi_bpi, 0x10u, 4u);
            }
            else
            {
                m_bpb = m_p1->m_bpb;
                v7 = (m_bpb->m_flags & 1) == 0;
                ap_i = &m_list_bpi_bpi->m_next;
                v35 = (broad_phase_info *)m_bpb;
                if (v7
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_base.h",
                        99,
                        "is_bpi()",
                        ""))
                {
                    __debugbreak();
                }
                v8 = (broad_phase_info **)m_list_bpi_bpi->m_p2;
                v9 = *v8;
                v7 = ((*v8)->m_flags & 1) == 0;
                p_g_phys_gjk_cache_system = (phys_heap_gjk_cache_system_avl_tree *)*v8;
                if (v7
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_base.h",
                        99,
                        "is_bpi()",
                        ""))
                {
                    __debugbreak();
                }
                v33.x = v35->m_trace_translation.x - v9->m_trace_translation.x;
                v33.y = v35->m_trace_translation.y - v9->m_trace_translation.y;
                v33.z = v35->m_trace_translation.z - v9->m_trace_translation.z;
                if (phys_are_potentially_colliding(
                    &v35->m_trace_aabb_min_whace,
                    &v35->m_trace_aabb_max_whace,
                    &v33,
                    &v9->m_trace_aabb_min_whace,
                    &v9->m_trace_aabb_max_whace,
                    &hit_time))
                {
                    if (!m_list_bpi_bpi->m_gjk_ci)
                    {
                        v10 = &G_BPM->g_phys_gjk_cache_system;
                        v11 = (phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal*)PMM_ALLOC(sizeof(phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal), 16);
                        if (v11)
                        {
                            ++v10->m_list_phys_gjk_cache_info_internal.m_count;
                            v11->m_key.m_id1 = 1;
                            v11->m_key.m_id2 = 2;
                            v11->m_flags = 0;
                        }
                        else
                        {
                            v11 = 0;
                        }
                        v9 = (broad_phase_info *)p_g_phys_gjk_cache_system;
                        m_list_bpi_bpi->m_gjk_ci = v11;
                    }
                    add_collision_pair(v35, v9, hit_time, m_list_bpi_bpi->m_gjk_ci);
                }
            }
            m_list_bpi_bpi = *ap_i;
            if (!*ap_i)
                break;
            p_m_list_bpi_bpi = ap_i;
        }
        thisptr = v37;
    }
    m_list_bpi_bpg = thisptr->m_list_bpi_bpg;
    p_m_list_bpi_bpg = &thisptr->m_list_bpi_bpg;
    ap_i = &thisptr->m_list_bpi_bpg;
    if (m_list_bpi_bpg)
    {
        while (1)
        {
            v15 = m_list_bpi_bpg->m_p1;
            v16 = m_list_bpi_bpg->m_p2;
            if (m_list_bpi_bpg->m_p1->m_ae1[0][0].m_ae_list_index >= v16->m_ae1[0][1].m_ae_list_index
                || v16->m_ae1[0][0].m_ae_list_index >= v15->m_ae1[0][1].m_ae_list_index
                || v15->m_ae1[1][0].m_ae_list_index >= v16->m_ae1[1][1].m_ae_list_index
                || v16->m_ae1[1][0].m_ae_list_index >= v15->m_ae1[1][1].m_ae_list_index
                || v15->m_ae1[2][0].m_ae_list_index >= v16->m_ae1[2][1].m_ae_list_index
                || v16->m_ae1[2][0].m_ae_list_index >= v15->m_ae1[2][1].m_ae_list_index)
            {
                *p_m_list_bpi_bpg = m_list_bpi_bpg->m_next;
                v21 = (char *)m_list_bpi_bpg->m_gjk_ci;
                p_g_phys_gjk_cache_system = &G_BPM->g_phys_gjk_cache_system;
                if (v21)
                {
                    PMM_VALIDATE(v21, 0x90u, 0x10u);
                    --p_g_phys_gjk_cache_system->m_list_phys_gjk_cache_info_internal.m_count;
                    PMM_FREE((unsigned __int8 *)v21, 0x90u, 0x10u);
                }
                PMM_VALIDATE((char *)m_list_bpi_bpg, 0x10u, 4u);
                --v37->m_active_pair_allocator.m_count;
                PMM_FREE((unsigned __int8 *)m_list_bpi_bpg, 0x10u, 4u);
            }
            else
            {
                v17 = (broad_phase_info *)v15->m_bpb;
                v7 = (v15->m_bpb->m_flags & 1) == 0;
                ap_i = &m_list_bpi_bpg->m_next;
                if (v7
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_base.h",
                        99,
                        "is_bpi()",
                        ""))
                {
                    __debugbreak();
                }
                v18 = m_list_bpi_bpg->m_p2;
                v19 = v18->m_bpb;
                if ((v18->m_bpb->m_flags & 2) == 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_base.h",
                        100,
                        "is_bpg()",
                        ""))
                {
                    __debugbreak();
                }
                v33.x = v17->m_trace_translation.x - v19->m_trace_translation.x;
                v33.y = v17->m_trace_translation.y - v19->m_trace_translation.y;
                v33.z = v17->m_trace_translation.z - v19->m_trace_translation.z;
                if (phys_are_potentially_colliding(
                    &v17->m_trace_aabb_min_whace,
                    &v17->m_trace_aabb_max_whace,
                    &v33,
                    &v19->m_trace_aabb_min_whace,
                    &v19->m_trace_aabb_max_whace,
                    &hit_time))
                {
                    for (i = (broad_phase_info *)LODWORD(v19[1].m_trace_aabb_min_whace.x);
                        i;
                        i = (broad_phase_info *)i->m_list_bpb_next)
                    {
                        v32.x = v17->m_trace_translation.x - i->m_trace_translation.x;
                        v32.y = v17->m_trace_translation.y - i->m_trace_translation.y;
                        v32.z = v17->m_trace_translation.z - i->m_trace_translation.z;
                        if (phys_are_potentially_colliding(
                            &v17->m_trace_aabb_min_whace,
                            &v17->m_trace_aabb_max_whace,
                            &v32,
                            &i->m_trace_aabb_min_whace,
                            &i->m_trace_aabb_max_whace,
                            &hit_time))
                        {
                            add_collision_pair(v17, i, hit_time, 0);
                        }
                    }
                }
            }
            m_list_bpi_bpg = *ap_i;
            if (!*ap_i)
                break;
            p_m_list_bpi_bpg = ap_i;
        }
        thisptr = v37;
    }
    m_list_bpg_bpg = thisptr->m_list_bpg_bpg;
    p_m_list_bpg_bpg = &thisptr->m_list_bpg_bpg;
    ap_i = &thisptr->m_list_bpg_bpg;
    if (m_list_bpg_bpg)
    {
        while (1)
        {
            v24 = m_list_bpg_bpg->m_p1;
            v25 = m_list_bpg_bpg->m_p2;
            if (m_list_bpg_bpg->m_p1->m_ae1[0][0].m_ae_list_index >= v25->m_ae1[0][1].m_ae_list_index
                || v25->m_ae1[0][0].m_ae_list_index >= v24->m_ae1[0][1].m_ae_list_index
                || v24->m_ae1[1][0].m_ae_list_index >= v25->m_ae1[1][1].m_ae_list_index
                || v25->m_ae1[1][0].m_ae_list_index >= v24->m_ae1[1][1].m_ae_list_index
                || v24->m_ae1[2][0].m_ae_list_index >= v25->m_ae1[2][1].m_ae_list_index
                || v25->m_ae1[2][0].m_ae_list_index >= v24->m_ae1[2][1].m_ae_list_index)
            {
                *p_m_list_bpg_bpg = m_list_bpg_bpg->m_next;
                v31 = (char *)m_list_bpg_bpg->m_gjk_ci;
                p_g_phys_gjk_cache_system = &G_BPM->g_phys_gjk_cache_system;
                if (v31)
                {
                    PMM_VALIDATE(v31, 0x90u, 0x10u);
                    --p_g_phys_gjk_cache_system->m_list_phys_gjk_cache_info_internal.m_count;
                    PMM_FREE((unsigned __int8 *)v31, 0x90u, 0x10u);
                }
                PMM_VALIDATE((char *)m_list_bpg_bpg, 0x10u, 4u);
                --v37->m_active_pair_allocator.m_count;
                PMM_FREE((unsigned __int8 *)m_list_bpg_bpg, 0x10u, 4u);
            }
            else
            {
                v26 = v24->m_bpb;
                v7 = (v24->m_bpb->m_flags & 2) == 0;
                ap_i = &m_list_bpg_bpg->m_next;
                if (v7
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_base.h",
                        100,
                        "is_bpg()",
                        ""))
                {
                    __debugbreak();
                }
                v27 = (broad_phase_info **)m_list_bpg_bpg->m_p2;
                v28 = *v27;
                v7 = ((*v27)->m_flags & 2) == 0;
                v35 = *v27;
                if (v7
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_base.h",
                        100,
                        "is_bpg()",
                        ""))
                {
                    __debugbreak();
                }
                for (j = (broad_phase_info *)LODWORD(v26[1].m_trace_aabb_min_whace.x);
                    j;
                    j = (broad_phase_info *)j->m_list_bpb_next)
                {
                    v32.x = j->m_trace_translation.x - v28->m_trace_translation.x;
                    v32.y = j->m_trace_translation.y - v28->m_trace_translation.y;
                    v32.z = j->m_trace_translation.z - v28->m_trace_translation.z;
                    if (phys_are_potentially_colliding(
                        &j->m_trace_aabb_min_whace,
                        &j->m_trace_aabb_max_whace,
                        &v32,
                        &v28->m_trace_aabb_min_whace,
                        &v28->m_trace_aabb_max_whace,
                        &hit_time))
                    {
                        for (k = (broad_phase_info *)v28->m_rb; k; k = (broad_phase_info *)k->m_list_bpb_next)
                        {
                            v33.x = j->m_trace_translation.x - k->m_trace_translation.x;
                            v33.y = j->m_trace_translation.y - k->m_trace_translation.y;
                            v33.z = j->m_trace_translation.z - k->m_trace_translation.z;
                            if (phys_are_potentially_colliding(
                                &j->m_trace_aabb_min_whace,
                                &j->m_trace_aabb_max_whace,
                                &v33,
                                &k->m_trace_aabb_min_whace,
                                &k->m_trace_aabb_max_whace,
                                &hit_time))
                            {
                                add_collision_pair(j, k, hit_time, 0);
                            }
                        }
                        v28 = v35;
                    }
                }
            }
            m_list_bpg_bpg = *ap_i;
            if (!*ap_i)
                break;
            p_m_list_bpg_bpg = ap_i;
        }
    }
}

void __thiscall axis_aligned_sweep_and_prune::process()
{
    bool v1; // sf
    int v2; // esi
    axis_aligned_sweep_and_prune::axis_element **v3; // ebx
    axis_aligned_sweep_and_prune::axis_element **v4; // eax
    axis_aligned_sweep_and_prune::axis_element *m_y_list; // edx
    axis_aligned_sweep_and_prune::axis_element *m_x_list; // edi
    axis_aligned_sweep_and_prune::axis_element *m_z_list; // ecx
    axis_aligned_sweep_and_prune::axis_element **v8; // eax
    int v9; // esi
    axis_aligned_sweep_and_prune::axis_element **v10; // eax
    axis_aligned_sweep_and_prune::axis_element *v11; // ecx
    axis_aligned_sweep_and_prune::axis_element **v12; // edx
    axis_aligned_sweep_and_prune::axis_element *m_next; // edx
    axis_aligned_sweep_and_prune::axis_element *v14; // ecx
    axis_aligned_sweep_and_prune::axis_element **v15; // edx
    axis_aligned_sweep_and_prune::axis_element *v16; // ecx
    int v17; // esi
    axis_aligned_sweep_and_prune::axis_element **v18; // edi
    axis_aligned_sweep_and_prune *v19; // esi
    axis_aligned_sweep_and_prune::axis_element **v20; // ecx
    axis_aligned_sweep_and_prune::axis_element **v21; // edx
    axis_aligned_sweep_and_prune::axis_element **i; // eax
    axis_aligned_sweep_and_prune::axis_element *v23; // edi
    axis_aligned_sweep_and_prune::axis_element *v24; // ebx
    int v25; // [esp-4h] [ebp-60h]
//    phys_transient_allocator transient_buffer; // [esp+10h] [ebp-4Ch] BYREF
    axis_aligned_sweep_and_prune::axis_element **zlist; // [esp+28h] [ebp-34h]
    int axis_element_count; // [esp+2Ch] [ebp-30h]
    axis_aligned_sweep_and_prune::axis_element **ylist; // [esp+30h] [ebp-2Ch]
    axis_aligned_sweep_and_prune::axis_element **zl_i; // [esp+34h] [ebp-28h]
    axis_aligned_sweep_and_prune::axis_element **yl_i; // [esp+38h] [ebp-24h]
    axis_aligned_sweep_and_prune::axis_element **xl_i; // [esp+3Ch] [ebp-20h]
    axis_aligned_sweep_and_prune::axis_element **xl_i_last; // [esp+40h] [ebp-1Ch]
    axis_aligned_sweep_and_prune::axis_element *zcur; // [esp+44h] [ebp-18h]
    axis_aligned_sweep_and_prune::axis_element *ycur; // [esp+48h] [ebp-14h]
    axis_aligned_sweep_and_prune *v36; // [esp+4Ch] [ebp-10h]
    int v37; // [esp+58h] [ebp-4h]
    axis_aligned_sweep_and_prune::active_pair *savedregs; // [esp+5Ch] [ebp+0h] BYREF

    v36 = this;
    v1 = (this->m_sap_node_allocator.m_count & 0x40000000) != 0;
    axis_element_count = 2 * this->m_sap_node_allocator.m_count;
    if ( !v1 && axis_element_count != 0 )
    {
        phys_transient_allocator transient_buffer;

        v2 = 4 * axis_element_count;
        v37 = 0;
        v3 = (axis_aligned_sweep_and_prune::axis_element **)   transient_buffer.allocate(4 * axis_element_count, 4, 0, "phys_transient_allocator out of memory.");
        ylist = (axis_aligned_sweep_and_prune::axis_element **)transient_buffer.allocate(v2, 4, 0, "phys_transient_allocator out of memory.");
        v4 = (axis_aligned_sweep_and_prune::axis_element **)   transient_buffer.allocate(v2, 4, 0, "phys_transient_allocator out of memory.");
        m_y_list = v36->m_y_list;
        m_x_list = v36->m_x_list;
        m_z_list = v36->m_z_list;
        zlist = v4;
        zl_i = v4;
        v8 = &v3[v2 / 4u];
        ycur = m_y_list;
        v9 = 0;
        zcur = m_z_list;
        xl_i = v3;
        yl_i = ylist;
        xl_i_last = v8;
        if ( v3 != v8 )
        {
            do
            {
                if ( m_x_list->m_node->m_updated != 1
                    && _tlAssert("source/phys_broad_phase.cpp", 451, "xcur->m_node->m_updated == 1", "") )
                {
                    __debugbreak();
                }
                if ( ycur->m_node->m_updated != 1
                    && _tlAssert("source/phys_broad_phase.cpp", 452, "ycur->m_node->m_updated == 1", "") )
                {
                    __debugbreak();
                }
                if ( zcur->m_node->m_updated != 1
                    && _tlAssert("source/phys_broad_phase.cpp", 453, "zcur->m_node->m_updated == 1", "") )
                {
                    __debugbreak();
                }
                v10 = xl_i;
                v11 = ycur;
                v12 = yl_i;
                *xl_i = m_x_list;
                m_x_list->m_ae_list_index = v9;
                m_x_list = m_x_list->m_next;
                *v12 = v11;
                yl_i = v12 + 1;
                m_next = v11->m_next;
                v11->m_ae_list_index = v9;
                v14 = zcur;
                ycur = m_next;
                v15 = zl_i;
                *zl_i = zcur;
                v14->m_ae_list_index = v9;
                v16 = v14->m_next;
                ++v9;
                xl_i = v10 + 1;
                zl_i = v15 + 1;
                zcur = v16;
            }
            while ( v10 + 1 != xl_i_last );
        }
        if ( (m_x_list || ycur || zcur)
            && _tlAssert("source/phys_broad_phase.cpp", 459, "xcur == NULL && ycur == NULL && zcur == NULL", "") )
        {
            __debugbreak();
        }
        v17 = axis_element_count;
        v36->merge_sort(v3, axis_element_count);
        v18 = ylist;
        v25 = v17;
        v19 = v36;
        v36->merge_sort(ylist, v25);
        v19->merge_sort(zlist, axis_element_count);
        v19->m_x_list = *v3;
        v20 = zlist;
        v19->m_y_list = *v18;
        v19->m_z_list = *v20;
        v21 = v3;
        for ( i = v18; v21 != xl_i_last; v23->m_next = v24 )
        {
            (*v21)->m_node->m_updated = 0;
            (*i)->m_node->m_updated = 0;
            (*v20)->m_node->m_updated = 0;
            (*v21)->m_next = v21[1];
            (*i)->m_next = i[1];
            v23 = *v20;
            v24 = v20[1];
            ++i;
            ++v20;
            ++v21;
        }
        (*(v21 - 1))->m_next = 0;
        (*(i - 1))->m_next = 0;
        (*(v20 - 1))->m_next = 0;
        //phys_transient_allocator::reset(&transient_buffer);
        transient_buffer.reset();
        v37 = -1;
        if ( transient_buffer.m_first_block
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
                     69,
                     "m_first_block == NULL",
                     "") )
        {
            __debugbreak();
        }
    }
    //axis_aligned_sweep_and_prune::process_active_pair_list(v36, &savedregs);
    v36->process_active_pair_list();
}

void __cdecl process_cluster_environment_collision(broad_phase_base *bpb, const broad_phase_environement_query_results &bpeqr)
{
    broad_phase_base *i; // esi

    iassert(bpb && bpb->get_bpb_cluster_next());

    for ( i = bpb->m_list_bpb_cluster_next; i; i = i->m_list_bpb_cluster_next )
    {
        iassert(i->get_flag(broad_phase_base::FLAG_IS_IN_CLUSTER));

        if (i->is_bpi())
        {
            collide_bpi_environment(i->get_bpi(), bpeqr);
        }
        else
        {
            iassert(i->is_bpg());
            
            collide_bpg_environment(i->get_bpg(), bpeqr);
        }
    }
}

broad_phase_memory *__cdecl broad_phase_memory::allocate_buffer(const broad_phase_memory_info *bpmi)
{
    broad_phase_memory *v1; // eax
    broad_phase_memory *v2; // esi
    int v3; // ecx
    int m_max_num_surface_types; // ecx
    int v5; // eax
    phys_surface_type_info *v6; // eax
    int v7; // ecx
    bool v8; // cc
    int v9; // edx
    phys_surface_type_info *g_surface_type_info_database; // eax
    phys_surface_type_info *v11; // eax

    v1 = (broad_phase_memory *)PMM_PERM_ALLOCATE(sizeof(broad_phase_memory), 8);
    if (v1)
    {
        //v2 = broad_phase_memory::broad_phase_memory(v1);
        v2 = new(v1) broad_phase_memory();
    }
    else
        v2 = 0;
    v3 = bpmi->m_max_num_gjk_ci + bpmi->m_max_num_sap_active_pairs;
    v2->g_phys_gjk_cache_system.m_list_head = 0;
    v2->g_phys_gjk_cache_system.m_max_num_gjk_ci = v3;
    m_max_num_surface_types = bpmi->m_max_num_surface_types;
    v5 = m_max_num_surface_types * (m_max_num_surface_types + 1) / 2;
    v2->m_max_num_surface_types = m_max_num_surface_types;
    v2->m_max_num_surface_type_infos = v5;
    if ( v5 <= 0 )
    {
        v2->g_surface_type_info_database = 0;
        return v2;
    }
    v6 = (phys_surface_type_info *)PMM_PERM_ALLOCATE(sizeof(phys_surface_type_info) * v5, 4u);
    v7 = 0;
    v8 = v2->m_max_num_surface_type_infos <= 0;
    v2->g_surface_type_info_database = v6;
    if ( v8 )
        return v2;
    v9 = 0;
    do
    {
        g_surface_type_info_database = v2->g_surface_type_info_database;
        g_surface_type_info_database[v9].m_friction_coef = 0.0;
        v11 = &g_surface_type_info_database[v9];
        ++v7;
        v11->m_bounce_coef = 0.0;
        v11->m_solver_priority = 0;
        v11->m_no_overflow_error = 0;
        ++v9;
    }
    while ( v7 < v2->m_max_num_surface_type_infos );
    return v2;
}