#include "phys_collision.h"
#include "phys_collision_multithreaded.h"
#include "rigid_body.h"
#include "phys_transient_allocator.h"
#include "phys_assert.h"
#include <DynEntity/DynEntity_client.h>
#include <bgame/bg_slidemove.h>
#include <cgame/cg_drawtools.h>
#include "phys_render.h"
#include "physics_system.h"
#include "phys_contact_manifold.h"
#include <tl/physics/rbc_def_generic.h>
#include <universal/com_math_anglevectors.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <new>
#include <qcommon/cm_load.h>
#include <cgame/cg_world.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include "phys_auto_rigid_body.h"
#include <cgame_mp/cg_vehicles_mp.h>
#include <glass/glass_client.h>
#include <algorithm>

phys_assert_info pai_create_cpi = { 0, 2, true };
phys_simple_allocator<generic_avl_map_node_t> g_generic_avl_map_node_allocator;
standard_query g_standard_query;

//rigid_body_constraint_contact *__cdecl avl_tree_find<rigid_body_pair_key, rigid_body_constraint_contact, rigid_body_constraint_contact::avl_tree_accessor>(
rigid_body_constraint_contact *__cdecl avl_tree_find(rigid_body_constraint_contact *tree_root, const rigid_body_pair_key *key)
{
    rigid_body_constraint_contact *result; // eax
    rigid_body *m_b1; // edx
    rigid_body *v4; // ecx
    bool v5; // cf

    result = tree_root;
    if (tree_root)
    {
        m_b1 = key->m_b1;
        do
        {
            if (m_b1 == result->m_avl_key.m_b1 && key->m_b2 == result->m_avl_key.m_b2)
                break;
            v4 = result->m_avl_key.m_b1;
            v5 = m_b1 < v4;
            if (m_b1 == v4)
                v5 = key->m_b2 < result->m_avl_key.m_b2;
            result = v5 ? result->m_avl_tree_node.m_left : result->m_avl_tree_node.m_right;
        } while (result);
    }
    return result;
}

void __thiscall contact_point_info::get_closest_psc(
                const phys_vec3 *normal,
                const phys_vec3 *b1_r_loc,
                const phys_vec3 *b2_r_loc,
                float *closest_error,
                const contact_point_info::pulse_sum_cache_info **closest_psc)
{
    phys_vec3 *m_list_b1_r_loc; // edx
    phys_vec3 *m_list_b2_r_loc; // esi
    const phys_vec3 *v8; // edi
    double v9; // st7
    double v10; // st6
    float v11; // [esp-30h] [ebp-3Ch]
    float v12; // [esp-2Ch] [ebp-38h]
    float v13; // [esp-28h] [ebp-34h]
    float v14; // [esp-20h] [ebp-2Ch]
    float v15; // [esp-20h] [ebp-2Ch]
    float v16; // [esp-1Ch] [ebp-28h]
    float v17; // [esp-1Ch] [ebp-28h]
    float v18; // [esp-18h] [ebp-24h]
    float v19; // [esp-18h] [ebp-24h]
    float v20; // [esp-Ch] [ebp-18h]
    float v21; // [esp-Ch] [ebp-18h]
    float v22; // [esp-Ch] [ebp-18h]
    float v23; // [esp-Ch] [ebp-18h]
    int v24; // [esp-8h] [ebp-14h]
    int v25; // [esp-4h] [ebp-10h]

    m_list_b1_r_loc = this->m_list_b1_r_loc;
    m_list_b2_r_loc = this->m_list_b2_r_loc;
    v24 = 0;
    if ( this->m_point_pair_count > 0 )
    {
        v8 = b1_r_loc;
        v14 = normal->x - this->m_normal.x;
        v16 = normal->y - this->m_normal.y;
        v18 = normal->z - this->m_normal.z;
        v20 = v14 * v14 + v16 * v16 + v18 * v18;
        v9 = v20;
        v25 = 0;
        do
        {
            v15 = m_list_b1_r_loc->x - v8->x;
            v17 = m_list_b1_r_loc->y - v8->y;
            v19 = m_list_b1_r_loc->z - v8->z;
            v11 = m_list_b2_r_loc->x - b2_r_loc->x;
            v12 = m_list_b2_r_loc->y - b2_r_loc->y;
            v13 = m_list_b2_r_loc->z - b2_r_loc->z;
            v21 = v19 * v19 + v15 * v15 + v17 * v17;
            v10 = v21 + v9;
            v22 = v13 * v13 + v12 * v12 + v11 * v11;
            v23 = v10 + v22;
            if ( *closest_error > (double)v23 )
            {
                *closest_error = v23;
                *closest_psc = &this->m_list_pulse_sum_cache_info[v25];
                v8 = b1_r_loc;
            }
            ++v24;
            ++v25;
            ++m_list_b1_r_loc;
            ++m_list_b2_r_loc;
        }
        while ( v24 < this->m_point_pair_count );
    }
}

void phys_contact_manifold::xform_and_translate_mesh_points(
                const phys_mat44 *xform,
                const phys_vec3 *translation)
{
    phys_contact_manifold *v4; // esi
    const phys_vec3 **m_list_sorted_mesh_point; // eax
    const phys_vec3 **v6; // ecx
    phys_vec3 *v7; // esi
    const phys_vec3 *v8; // eax
    double v9; // st7
    const phys_vec3 **v10; // eax
    const phys_vec3 *v11; // eax
    phys_vec3 v12; // [esp-40h] [ebp-4Ch] BYREF
    float v13; // [esp-30h] [ebp-3Ch]
    float v14; // [esp-2Ch] [ebp-38h]
    float v15; // [esp-28h] [ebp-34h]
    float v16; // [esp-20h] [ebp-2Ch]
    float v17; // [esp-1Ch] [ebp-28h]
    float v18; // [esp-18h] [ebp-24h]
    phys_contact_manifold *v19; // [esp-Ch] [ebp-18h]
    const phys_vec3 **v20; // [esp-8h] [ebp-14h]
    const phys_vec3 **v21; // [esp-4h] [ebp-10h]
    int v22; // [esp+0h] [ebp-Ch]
    //contact_manifold_mesh_point **last_mp_i; // [esp+4h] [ebp-8h]
    //contact_manifold_mesh_point **retaddr; // [esp+Ch] [ebp+0h]

    //v22 = a2;
    //last_mp_i = retaddr;
    v4 = this;
    m_list_sorted_mesh_point = (const phys_vec3 **)this->m_list_sorted_mesh_point;
    v6 = &m_list_sorted_mesh_point[this->m_list_mesh_point_count];
    v19 = v4;
    v20 = v6;
    v21 = m_list_sorted_mesh_point;
    if ( m_list_sorted_mesh_point != v6 )
    {
        while ( 1 )
        {
            v7 = (phys_vec3 *)*m_list_sorted_mesh_point;
            v8 = phys_multiply(&v12, xform, *m_list_sorted_mesh_point);
            v16 = v8->x + xform->w.x;
            v17 = v8->y + xform->w.y;
            v18 = v8->z + xform->w.z;
            v13 = translation->x + v16;
            v14 = translation->y + v17;
            v9 = translation->z + v18;
            v10 = ++v21;
            v15 = v9;
            v7->x = v13;
            v7->y = v14;
            v7->z = v15;
            if ( v10 == v20 )
                break;
            m_list_sorted_mesh_point = v21;
        }
        v4 = v19;
    }
    v11 = phys_multiply(&v12, xform, &v4->m_feature_normal);
    v4->m_feature_normal.x = v11->x;
    v4->m_feature_normal.y = v11->y;
    v4->m_feature_normal.z = v11->z;
}



void __thiscall phys_contact_manifold_process::comp_contact_mat(const phys_vec3 *contact_normal)
{
    double v3; // st6
    double v4; // st7
    float v5; // [esp-1Ch] [ebp-3Ch]
    float v6; // [esp-1Ch] [ebp-3Ch]
    float v7; // [esp-1Ch] [ebp-3Ch]
    float v8; // [esp-18h] [ebp-38h]
    float v9; // [esp-18h] [ebp-38h]
    float v10; // [esp-18h] [ebp-38h]
    float v11; // [esp-14h] [ebp-34h]
    float v12; // [esp-14h] [ebp-34h]
    float v13; // [esp-14h] [ebp-34h]
    float v14; // [esp-Ch] [ebp-2Ch]
    float v15; // [esp-Ch] [ebp-2Ch]
    float v16; // [esp-8h] [ebp-28h]
    float v17; // [esp-8h] [ebp-28h]
    float v18; // [esp-4h] [ebp-24h]
    float v19; // [esp+10h] [ebp-10h]
    float y; // [esp+10h] [ebp-10h]
    float v21; // [esp+10h] [ebp-10h]
    float v22; // [esp+10h] [ebp-10h]
    float v23; // [esp+10h] [ebp-10h]

    v19 = fabs(contact_normal->x);
    if ( v19 >= 0.800000011920929 )
    {
        y = contact_normal->y;
        v15 = contact_normal->x * y;
        v17 = y * y;
        v18 = y * contact_normal->z;
        v4 = 0.0;
        v5 = 0.0 - v15;
        v3 = 1.0 - v17;
    }
    else
    {
        v14 = contact_normal->x * contact_normal->x;
        v16 = contact_normal->y * contact_normal->x;
        v18 = contact_normal->x * contact_normal->z;
        v5 = 1.0 - v14;
        v3 = 0.0 - v16;
        v4 = 0.0;
    }
    v8 = v3;
    v11 = v4 - v18;
    v21 = v8 * v8 + v5 * v5 + v11 * v11;
    v22 = sqrt(v21);
    if ( v22 <= 0.1000000014901161
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_contact_manifold.h",
                 397,
                 "nyrow > 0.1f",
                 "") )
    {
        __debugbreak();
    }
    v23 = 1.0 / v22;
    v6 = v23 * v5;
    v9 = v8 * v23;
    v12 = v23 * v11;
    this->contact_mat.y.x = v6;
    this->contact_mat.y.y = v9;
    this->contact_mat.y.z = v12;
    v7 = contact_normal->z * this->contact_mat.y.y - contact_normal->y * this->contact_mat.y.z;
    v10 = contact_normal->x * this->contact_mat.y.z - contact_normal->z * this->contact_mat.y.x;
    v13 = this->contact_mat.y.x * contact_normal->y - contact_normal->x * this->contact_mat.y.y;
    this->contact_mat.x.x = v7;
    this->contact_mat.x.y = v10;
    this->contact_mat.x.z = v13;
    this->contact_mat.z.x = contact_normal->x;
    this->contact_mat.z.y = contact_normal->y;
    this->contact_mat.z.z = contact_normal->z;
}

#if 0 // KISAKTODO 
rigid_body_constraint_contact *__cdecl avl_tree_find<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>(
                rigid_body_constraint_contact *tree_root,
                const rigid_body_pair_key *key)
{
    rigid_body_constraint_contact *result; // eax
    rigid_body *m_b1; // edx
    rigid_body *v4; // ecx
    bool v5; // cf

    result = tree_root;
    if ( tree_root )
    {
        m_b1 = key->m_b1;
        do
        {
            if ( m_b1 == result->m_avl_key.m_b1 && key->m_b2 == result->m_avl_key.m_b2 )
                break;
            v4 = result->m_avl_key.m_b1;
            v5 = m_b1 < v4;
            if ( m_b1 == v4 )
                v5 = key->m_b2 < result->m_avl_key.m_b2;
            result = v5 ? result->m_avl_tree_node.m_left : result->m_avl_tree_node.m_right;
        }
        while ( result );
    }
    return result;
}
#endif
contact_point_info *__cdecl contact_point_info::create_cpi(
                int point_pair_count,
                bool no_error,
                phys_transient_allocator *allocator)
{
    unsigned int v3; // edi
    contact_point_info *result; // eax

    if ( point_pair_count <= 0
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                 159,
                 "point_pair_count > 0",
                 "") )
    {
        __debugbreak();
    }
    v3 = (32 * point_pair_count + 95) & 0xFFFFFFF0;
    //result = (contact_point_info *)phys_transient_allocator::mt_allocate(
    result = (contact_point_info *)allocator->mt_allocate(v3 + ((12 * point_pair_count + 15) & 0xFFFFFFF0), 16, 1, "contact_point_info buffer overflow");
    if ( result )
    {
        result->m_fric_coef = -1.0;
        result->m_bounce_coef = -1.0;
        result->m_max_restitution_vel = -1.0;
        result->m_list_b1_r_loc = &result[1].m_normal;
        result->m_list_b2_r_loc = &result[1].m_normal + point_pair_count;
        result->m_list_pulse_sum_cache_info = (contact_point_info::pulse_sum_cache_info *)((char *)result + v3);
        result->m_point_pair_count = point_pair_count;
        result->m_flags = 0;
        result->m_rb2_entity = (const void *)-1;
    }
    else if ( !no_error )
    {
        if ( (pai_create_cpi.m_hits_total_count < pai_create_cpi.m_max_hits_total || !pai_create_cpi.m_max_hits_total)
            && pai_create_cpi.m_hits_frame_count < pai_create_cpi.m_max_hits_per_frame )
        {
            if ( pai_create_cpi.m_use_warnings_only )
            {
                PHYS_WARNING(
                    "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                    173,
                    "cpi || no_error",
                    "contact_point_info buffer overflow");
            }
            else if ( _tlAssert(
                                    "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                                    173,
                                    "cpi || no_error",
                                    "contact_point_info buffer overflow") )
            {
                __debugbreak();
            }
        }
        _InterlockedExchangeAdd(&pai_create_cpi.m_hits_total_count, 1u);
        _InterlockedExchangeAdd(&pai_create_cpi.m_hits_frame_count, 1u);
        return 0;
    }
    return result;
}

void __thiscall contact_point_info::set_solver_priority(unsigned int priority)
{
    if ((priority & 7) != priority
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
            78,
            "(priority & FLAG_SOLVER_PRIORITY_MASK) == priority",
            ""))
    {
        __debugbreak();
    }
    this->m_flags &= 0xFFFFFFF8;
    this->m_flags |= priority;
}

void __thiscall contact_point_info::set_rb2_entity(const void *rb2_entity)
{
    this->m_flags |= 0x10u;
    this->m_rb2_entity = rb2_entity;
}

void __thiscall contact_point_info::check_surface_properties()
{
    iassert(m_fric_coef >= 0.0f);
    iassert(m_bounce_coef <= 1.0f && m_bounce_coef >= 0.0f);
    iassert(m_max_restitution_vel >= 0.0f);
}

extern const char *g_contact_manifold_error_msg;
void __thiscall phys_contact_manifold::set_get_feature_params(
                const phys_vec3 *hitp,
                const phys_vec3 *hitn,
                float feature_distance_eps,
                float sin_feautre_angular_eps_sq)
{
    phys_memory_heap *m_allocator; // edi
    const char *v7; // ecx
    uintptr_t v8; // eax

    this->m_feature_hitp.x = hitp->x;
    this->m_feature_hitp.y = hitp->y;
    this->m_feature_hitp.z = hitp->z;
    this->m_feature_hitn.x = hitn->x;
    this->m_feature_hitn.y = hitn->y;
    this->m_feature_hitn.z = hitn->z;
    m_allocator = this->m_allocator;
    this->m_feature_distance_eps = feature_distance_eps;
    this->m_sin_feautre_angular_eps_sq = sin_feautre_angular_eps_sq;
    v7 = g_contact_manifold_error_msg;
    v8 = ((uintptr_t)(m_allocator->m_buffer_cur + 15) & ~(uintptr_t)15);
    m_allocator->m_buffer_cur = (char *)v8;
    if ( (char *)v8 >= m_allocator->m_buffer_end
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                 114,
                 "m_buffer_cur < m_buffer_end",
                 v7) )
    {
        __debugbreak();
    }
    this->m_list_mesh_point = (contact_manifold_mesh_point *)m_allocator->m_buffer_cur;
    this->m_list_mesh_point_count = 0;
    this->m_close_mesh_point_count = 0;
    this->m_list_sorted_mesh_point = 0;
    this->m_list_contact_point = 0;
}

void __cdecl contact_point_info::set_closest_cached_psc(
                contact_point_info *cached_cpi,
                const phys_vec3 *normal,
                const phys_vec3 *b1_r_loc,
                const phys_vec3 *b2_r_loc,
                contact_point_info::pulse_sum_cache_info *psc)
{
    contact_point_info *v5; // esi
    const contact_point_info::pulse_sum_cache_info *v6; // eax
    float closest_error; // [esp+4h] [ebp-8h] BYREF
    const contact_point_info::pulse_sum_cache_info *closest_psc; // [esp+8h] [ebp-4h] BYREF

    v5 = cached_cpi;
    closest_error = 10000000.0;
    closest_psc = 0;
    if ( !cached_cpi )
        goto LABEL_5;
    do
    {
        //contact_point_info::get_closest_psc(v5, normal, b1_r_loc, b2_r_loc, &closest_error, &closest_psc);
        v5->get_closest_psc(normal, b1_r_loc, b2_r_loc, &closest_error, &closest_psc);
        v5 = v5->m_next_link;
    }
    while ( v5 );
    v6 = closest_psc;
    if ( closest_psc )
    {
        psc->m_ps_cache_list[0].m_pulse_sum = closest_psc->m_ps_cache_list[0].m_pulse_sum;
        psc->m_ps_cache_list[1].m_pulse_sum = v6->m_ps_cache_list[1].m_pulse_sum;
        psc->m_ps_cache_list[2].m_pulse_sum = v6->m_ps_cache_list[2].m_pulse_sum;
    }
    else
    {
LABEL_5:
        if ( psc )
        {
            psc->m_ps_cache_list[0].m_pulse_sum = 0.0;
            psc->m_ps_cache_list[1].m_pulse_sum = 0.0;
            psc->m_ps_cache_list[2].m_pulse_sum = 0.0;
        }
    }
}

void __thiscall contact_point_info::set_closest_cached_psc(contact_point_info *cached_cpi)
{
    contact_point_info::pulse_sum_cache_info *m_list_pulse_sum_cache_info; // esi
    phys_vec3 *m_list_b1_r_loc; // ebx
    contact_point_info::pulse_sum_cache_info *last_psci_i; // [esp+Ch] [ebp-8h]
    const phys_vec3 *b2_r_loc; // [esp+10h] [ebp-4h]

    m_list_pulse_sum_cache_info = this->m_list_pulse_sum_cache_info;
    m_list_b1_r_loc = this->m_list_b1_r_loc;
    b2_r_loc = this->m_list_b2_r_loc;
    for ( last_psci_i = &m_list_pulse_sum_cache_info[this->m_point_pair_count];
                m_list_pulse_sum_cache_info != last_psci_i;
                ++m_list_b1_r_loc )
    {
        contact_point_info::set_closest_cached_psc(
            cached_cpi,
            &this->m_normal,
            m_list_b1_r_loc,
            b2_r_loc++,
            m_list_pulse_sum_cache_info++);
    }
}

const phys_surface_type_info *__cdecl surface_type_info_database_get(int surface_type_1, int surface_type_2)
{
    if (!G_BPM->g_surface_type_info_database
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
            357,
            "G_BPM->g_surface_type_info_database",
            ""))
    {
        __debugbreak();
    }
    return &G_BPM->g_surface_type_info_database[surface_type_info_database_get_index(surface_type_1, surface_type_2)];
}

void __cdecl set_cpi_params(contact_point_info *cpi, phys_collision_pair *pcp)
{
    const phys_surface_type_info *psti; // [esp+1Ch] [ebp-20h]
    const PhysObjUserData *userData2; // [esp+20h] [ebp-1Ch]
    broad_phase_info *bpi1; // [esp+24h] [ebp-18h]
    float bounce; // [esp+28h] [ebp-14h]
    unsigned int priority; // [esp+2Ch] [ebp-10h]
    broad_phase_info *bpi2; // [esp+30h] [ebp-Ch]
    const PhysObjUserData *userData1; // [esp+34h] [ebp-8h]
    float friction; // [esp+38h] [ebp-4h]

    bpi1 = pcp->m_bpi1;
    bpi2 = pcp->m_bpi2;
    userData1 = (const PhysObjUserData *)bpi1->m_rb->m_userdata;
    userData2 = (const PhysObjUserData *)bpi2->m_rb->m_userdata;
    if (!userData1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 59, 0, "%s", "userData1"))
    {
        __debugbreak();
    }
    priority = 0;
    if (userData1->vehicle)
    {
        priority = 1;
        friction = phys_vehicleFriction->current.value;
        bounce = userData1->bounce;
        if (userData1->vehicle->m_vehicle_info->type == 4 && cpi->m_normal.z < -0.69999999)
            friction = userData1->friction;
        if (bpi1->m_rb->m_mat.z.z < -0.1)
            friction = 1.0f;
    }
    else if (userData2)
    {
        friction = (float)(userData1->friction + userData2->friction) * 0.5;
        bounce = (float)(userData1->bounce + userData2->bounce) * 0.5;
    }
    else
    {
        psti = surface_type_info_database_get(bpi1->m_surface_type, bpi2->m_surface_type);
        friction = userData1->friction * psti->m_friction_coef;
        bounce = userData1->bounce * psti->m_bounce_coef;
    }
    cpi->m_fric_coef = friction;
    cpi->m_bounce_coef = bounce;
    cpi->m_max_restitution_vel = 3400.0f;
    //contact_point_info::check_surface_properties(cpi);
    cpi->check_surface_properties();
    //contact_point_info::set_solver_priority(cpi, priority);
    cpi->set_solver_priority(priority);
    cpi->m_flags &= ~8u;
    //contact_point_info::set_rb2_entity(cpi, pcp->m_bpi2);
    cpi->set_rb2_entity(pcp->m_bpi2);
}

const centity_s *get_entity(gjk_physics_collision_visitor *collision_visitor)
{
    if (collision_visitor->cent)
        return collision_visitor->cent;
    if (collision_visitor->glass)
        return (const centity_s *)collision_visitor->glass;

    iassert(collision_visitor->dynEntDef);
    //if (!collision_visitor->dynEntDef
    //    && !Assert_MyHandler(
    //        a1,
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
    //        405,
    //        0,
    //        "%s",
    //        "collision_visitor->dynEntDef"))
    //{
    //    __debugbreak();
    //}
    return (const centity_s *)collision_visitor->dynEntDef;
}

// aislop
void phys_contact_manifold_process::process(
                phys_collision_pair *pcp,
                phys_gjk_info *gjk_info)
{
// -----------------------------------------------------------------------
    // Init manifold lists and reset the bump allocator
    // -----------------------------------------------------------------------
    this->cman1.m_list_mesh_point        = nullptr;
    this->cman1.m_list_sorted_mesh_point = nullptr;
    this->cman1.m_list_contact_point     = nullptr;
    this->cman2.m_list_mesh_point        = nullptr;
    this->cman2.m_list_sorted_mesh_point = nullptr;
    this->cman2.m_list_contact_point     = nullptr;
    this->m_list_isect_point             = nullptr;
    this->m_cpi                          = nullptr;
    this->m_allocator.m_buffer_cur       = this->m_allocator.m_buffer_start;
 
    // -----------------------------------------------------------------------
    // Cache commonly-used pointers
    // -----------------------------------------------------------------------
    broad_phase_info    *bpi1             = pcp->m_bpi1;
    const phys_mat44    *cg1_to_rb1_xform = bpi1->m_cg_to_rb_xform;
    //rigid_body          *rb1              = (rigid_body *)bpi1->m_cg_to_world_xform;
    rigid_body *rb1                       = bpi1->m_rb;
    const phys_mat44    *rb2_to_world     = pcp->m_bpi2->m_rb_to_world_xform;
 
    // cg1->rb2 = inv(rb2_to_world) * cg1_to_world
    phys_full_inv_multiply_mat(
        &this->cg1_to_rb2_xform,
        rb2_to_world,
        bpi1->m_cg_to_world_xform);   // rb1 is used here as cg1_to_world
 
    // -----------------------------------------------------------------------
    // Compute penetration_t � how far along the translation we are
    //
    //   displacement = p1_orig - p2_orig   (before the 0.34 nudge)
    //   dist_p1_p2_n = dot(n, displacement)
    //   penetration_t = clamp(-dist_p1_p2_n / 10.2, 0, 1)
    // -----------------------------------------------------------------------
    phys_vec3 *contact_n = &gjk_info->cg1_cinfo_loc.m_n;
 
    // Save original p1/p2 before nudging them apart along the normal
    phys_vec3 p1_orig = gjk_info->cg1_cinfo_loc.m_p1;
    phys_vec3 p2_orig = gjk_info->cg1_cinfo_loc.m_p2;
 
    // Nudge p1 outward and p2 inward by 0.34 * n
    const float NUDGE = 0.34f;
    gjk_info->cg1_cinfo_loc.m_p1.x += contact_n->x * NUDGE;
    gjk_info->cg1_cinfo_loc.m_p1.y += contact_n->y * NUDGE;
    gjk_info->cg1_cinfo_loc.m_p1.z += contact_n->z * NUDGE;
 
    gjk_info->cg1_cinfo_loc.m_p2.x -= contact_n->x * NUDGE;
    gjk_info->cg1_cinfo_loc.m_p2.y -= contact_n->y * NUDGE;
    gjk_info->cg1_cinfo_loc.m_p2.z -= contact_n->z * NUDGE;
 
    // displacement = original p1 - original p2
    phys_vec3 p1_minus_p2;
    p1_minus_p2.x = p1_orig.x - p2_orig.x;
    p1_minus_p2.y = p1_orig.y - p2_orig.y;
    p1_minus_p2.z = p1_orig.z - p2_orig.z;
 
    float dist_p1_p2_n = contact_n->dot(p1_minus_p2);
 
    const float PENETRATION_SCALE = -10.2f;
    float penetration_t = dist_p1_p2_n / PENETRATION_SCALE;
    penetration_t = (penetration_t < 0.0f) ? 0.0f
                  : (penetration_t > 1.0f) ? 1.0f
                  : penetration_t;
 
    // -----------------------------------------------------------------------
    // Get feature params scaled by penetration depth
    // -----------------------------------------------------------------------
    float feature_dist_eps    = phys_contact_manifold::get_STD_GET_FEATURE_DISTANCE_EPS(penetration_t);
    float feature_angular_eps = phys_contact_manifold::get_STD_GET_FEATURE_SIN_ANGULAR_EPS_SQ(penetration_t);
 
    // --- Body 1 feature ---
    this->cman1.set_get_feature_params(
        &gjk_info->cg1_cinfo_loc.m_p1, contact_n,
        feature_dist_eps, feature_angular_eps);
 
    pcp->m_bpi1->m_gjk_geom->get_feature(&this->cman1);
 
    // --- Body 2 feature: transform normal and contact point into cg2 space ---
    phys_vec3 neg_n;
    neg_n.x = -contact_n->x;
    neg_n.y = -contact_n->y;
    neg_n.z = -contact_n->z;
 
    // Rotate neg_n by the cg2->cg1 rotation (transpose multiply rows)
    phys_vec3 n_in_cg2;
    n_in_cg2.x = gjk_info->cg2_to_cg1_xform.x.x * neg_n.x
               + gjk_info->cg2_to_cg1_xform.x.y * neg_n.y
               + gjk_info->cg2_to_cg1_xform.x.z * neg_n.z;
    n_in_cg2.y = gjk_info->cg2_to_cg1_xform.y.x * neg_n.x
               + gjk_info->cg2_to_cg1_xform.y.y * neg_n.y
               + gjk_info->cg2_to_cg1_xform.y.z * neg_n.z;
    n_in_cg2.z = gjk_info->cg2_to_cg1_xform.z.x * neg_n.x
               + gjk_info->cg2_to_cg1_xform.z.y * neg_n.y
               + gjk_info->cg2_to_cg1_xform.z.z * neg_n.z;
 
    // Transform p2 into cg2 space
    phys_vec3 tmp;
    const phys_vec3 *p2_in_cg2 = phys_full_inv_multiply(
        &tmp, &gjk_info->cg2_to_cg1_xform, &gjk_info->cg1_cinfo_loc.m_p2);
 
    this->cman2.set_get_feature_params(
        p2_in_cg2, &n_in_cg2,
        feature_dist_eps, feature_angular_eps);
 
    pcp->m_bpi2->m_gjk_geom->get_feature(&this->cman2);
 
    // -----------------------------------------------------------------------
    // Continuous-collision translation offset (for swept contacts)
    // -----------------------------------------------------------------------
    float lambda = gjk_info->m_continuous_collision_lambda;
    phys_vec3 cg1_rel_trans;
    cg1_rel_trans.x = lambda * gjk_info->m_cg1_relative_translation_loc.x;
    cg1_rel_trans.y = lambda * gjk_info->m_cg1_relative_translation_loc.y;
    cg1_rel_trans.z = lambda * gjk_info->m_cg1_relative_translation_loc.z;
 
    // -----------------------------------------------------------------------
    // Try to build a full polygon-polygon contact manifold.
    // Fall back to a single-point contact if any step fails.
    // -----------------------------------------------------------------------
    bool full_manifold = false;
 
    if (this->cman1.m_list_mesh_point_count >= 2
        && this->cman2.m_list_mesh_point_count >= 2
        && !(this->cman1.m_list_mesh_point_count == 2 && this->cman2.m_list_mesh_point_count == 2))
    {
        // Re-compute feature normals with tighter tolerances
        float comp_dist_eps    = phys_contact_manifold::get_STD_COMP_FEATURE_NORMAL_DISTANCE_EPS(penetration_t);
        float comp_angular_eps = phys_contact_manifold::get_STD_COMP_FEATURE_NORMAL_SIN_ANGULAR_EPS_SQ(penetration_t);
 
        this->cman1.m_feature_distance_eps      = comp_dist_eps;
        this->cman1.m_sin_feautre_angular_eps_sq = comp_angular_eps;
        this->cman1.comp_feature_normal();
 
        this->cman2.m_feature_distance_eps      = comp_dist_eps;
        this->cman2.m_sin_feautre_angular_eps_sq = comp_angular_eps;
        this->cman2.comp_feature_normal();
 
        if (this->cman1.m_list_mesh_point_count >= 2
            && this->cman2.m_list_mesh_point_count >= 2
            && !(this->cman1.m_list_mesh_point_count == 2 && this->cman2.m_list_mesh_point_count == 2))
        {
            this->comp_contact_mat(contact_n);
            this->cman1.generate_convex_poly(&this->contact_mat);
 
            // Move cman2 mesh points into cg1 space, offset by -cg1_rel_trans
            phys_vec3 neg_rel_trans;
            neg_rel_trans.x = -cg1_rel_trans.x;
            neg_rel_trans.y = -cg1_rel_trans.y;
            neg_rel_trans.z = -cg1_rel_trans.z;
            this->cman2.xform_and_translate_mesh_points(&gjk_info->cg2_to_cg1_xform, &neg_rel_trans);
            this->cman2.generate_convex_poly(&this->contact_mat);
 
            if (this->cman1.m_list_contact_point_count >= 2
                && this->cman2.m_list_contact_point_count >= 2
                && !(this->cman1.m_list_contact_point_count == 2 && this->cman2.m_list_contact_point_count == 2))
            {
                this->intersect_poly_poly();
                full_manifold = (this->m_contact_point_count > 0);
            }
        }
    }
 
    // -----------------------------------------------------------------------
    // Single-point fallback
    // -----------------------------------------------------------------------
    if (!full_manifold)
    {
        this->m_cpi = (contact_point_info *)contact_point_info::create_cpi(
            1, false, this->m_cpi_allocator);
 
        if (!this->m_cpi)
            return;
 
        // Contact point on body 1 (in rb1 local space)
        phys_vec3 scratch;
        phys_vec3 *p1_rb1 = this->m_cpi->m_list_b1_r_loc;
        const phys_vec3 *r1 = phys_full_multiply(&scratch, cg1_to_rb1_xform, &p1_orig);
        p1_rb1->x = r1->x;
        p1_rb1->y = r1->y;
        p1_rb1->z = r1->z;
 
        // Contact point on body 2 (in rb2 local space)
        phys_vec3 *p2_rb2 = this->m_cpi->m_list_b2_r_loc;
        const phys_vec3 *r2 = phys_full_multiply(&scratch, &this->cg1_to_rb2_xform, &p2_orig);
        p2_rb2->x = r2->x;
        p2_rb2->y = r2->y;
        p2_rb2->z = r2->z;
 
        this->m_contact_point_count = 1;
        goto FINALIZE;
    }
 
    // -----------------------------------------------------------------------
    // Sanity checks
    // -----------------------------------------------------------------------
    if (this->m_contact_point_count <= 0
        && _tlAssert("source/phys_collision.cpp", 83, "m_contact_point_count > 0", ""))
    {
        __debugbreak();
    }
    if (!this->m_list_isect_point
        && _tlAssert("source/phys_collision.cpp", 84, "m_list_isect_point", ""))
    {
        __debugbreak();
    }
 
    // -----------------------------------------------------------------------
    // Project intersection points onto both feature planes and compute
    // plane offsets (n1, n2) used later per-point.
    // -----------------------------------------------------------------------
    {
        float d1 = contact_n->dot(this->cman1.m_feature_normal);
        float d2 = contact_n->dot(this->cman2.m_feature_normal);
 
        if (fabsf(d1) <= 1e-7f)
        {
            if (_tlAssert("source/phys_collision.cpp", 88, "fabsf(d1) > 0.0000001f", ""))
                __debugbreak();
        }
        if (fabsf(d2) <= 1e-7f)
        {
            if (_tlAssert("source/phys_collision.cpp", 89, "fabsf(d2) > 0.0000001f", ""))
                __debugbreak();
        }
 
        // Plane offsets: n1 = dot(cman1.normal, p1), n2 = dot(cman2.normal, p2)
        float n1 = this->cman1.m_feature_normal.dot(p1_orig);
        float n2 = this->cman2.m_feature_normal.dot(p2_orig);
 
        // -----------------------------------------------------------------------
        // If more than 5 intersection points, reduce to 5 keeping the most
        // "spread-out" set.  The algorithm picks a "closest to centroid" anchor
        // point and iteratively discards the point that contributes the smallest
        // triangle area to adjacent edges.
        // -----------------------------------------------------------------------
        if (this->m_contact_point_count > 5)
        {
            int count = this->m_contact_point_count;
            contact_manifold_mesh_point **isect = this->m_list_isect_point;
            contact_manifold_mesh_point **isect_end = isect + count;
 
            // Weighted centroid direction vectors:
            //   dir1 = cman1.normal / d1,   dir2 = cman2.normal / d2
            // Then delta = dir2 - dir1  projected into contact_mat axes
            phys_vec3 dir1, dir2;
            dir1.x = this->cman1.m_feature_normal.x / d1;
            dir1.y = this->cman1.m_feature_normal.y / d1;
            dir1.z = this->cman1.m_feature_normal.z / d1;
 
            dir2.x = this->cman2.m_feature_normal.x / d2;
            dir2.y = this->cman2.m_feature_normal.y / d2;
            dir2.z = this->cman2.m_feature_normal.z / d2;
 
            phys_vec3 delta;
            delta.x = dir2.x - dir1.x;
            delta.y = dir2.y - dir1.y;
            delta.z = dir2.z - dir1.z;
 
            // Project delta into contact matrix axes to get a 2D bias
            float bias_u = this->contact_mat.x.dot(delta);
            float bias_v = this->contact_mat.y.dot(delta);
 
            // Find the intersection point whose contact_p is closest to the
            // biased origin (smallest weighted 2D distance)
            const float BIG = 1e7f;
            float min_score = BIG;
            contact_manifold_mesh_point *anchor = nullptr;
 
            contact_manifold_mesh_point **prev_i = isect_end - 2;
            contact_manifold_mesh_point **cur_i  = isect_end - 1;
 
            for (contact_manifold_mesh_point **i = isect; i != isect_end; ++i)
            {
                contact_manifold_mesh_point *mp   = *cur_i;
                contact_manifold_mesh_point *prev  = *prev_i;
                contact_manifold_mesh_point *self  = *i;
 
                // Triangle area of (prev, self, mp) gives a "spread" measure
                float ex = self->m_contact_p.x - prev->m_contact_p.x;
                float ey = self->m_contact_p.y - prev->m_contact_p.y;
                float fx = mp->m_contact_p.x   - prev->m_contact_p.x;
                float fy = mp->m_contact_p.y   - prev->m_contact_p.y;
                float area = fabsf(ey * fx - fy * ex);
                mp->m_p.x = area;   // stash area in m_p.x for the reduction loop
 
                // Biased distance to origin
                float score = mp->m_contact_p.y * bias_v + bias_u * mp->m_contact_p.x;
                if (min_score > score)
                {
                    min_score = score;
                    anchor    = mp;
                }
                prev_i = cur_i;
                cur_i  = i;
            }
 
            if (!anchor)
            {
                if (_tlAssert("source/phys_collision.cpp", 115, "closest_mp", ""))
                    __debugbreak();
            }
 
            // Reduce until at most 5 points remain, always keeping the anchor
            contact_manifold_mesh_point **end = isect_end;
            while ((end - isect) > 5)
            {
                // Find the point with the smallest area (excluding anchor)
                contact_manifold_mesh_point **smallest_i = isect;
 
                // Walk neighbours: prev->cur->next to compute triangle areas,
                // then remove the one that adds the least area.
                contact_manifold_mesh_point **last = end - 1;
                contact_manifold_mesh_point **second_last = last - 1;
 
                // Recompute areas for the two neighbours of the candidates
                {
                    contact_manifold_mesh_point *A = *second_last;
                    contact_manifold_mesh_point *B = *last;
                    // neighbour wrapping
                    contact_manifold_mesh_point **nn = (smallest_i < last) ? smallest_i + 1 : isect;
                    contact_manifold_mesh_point *C = *nn;
 
                    float ex2 = C->m_contact_p.x - A->m_contact_p.x;
                    float ey2 = C->m_contact_p.y - A->m_contact_p.y;
                    float fx2 = B->m_contact_p.x - A->m_contact_p.x;
                    float fy2 = B->m_contact_p.y - A->m_contact_p.y;
                    float area2 = fabsf(ey2 * fx2 - fy2 * ex2);
                    B->m_p.x = area2;
 
                    // Also update next-next neighbour
                    contact_manifold_mesh_point **nn2 = (nn < last) ? nn + 1 : isect;
                    contact_manifold_mesh_point *D = *nn2;
                    float ex3 = D->m_contact_p.x - B->m_contact_p.x;
                    float ey3 = D->m_contact_p.y - B->m_contact_p.y;
                    float fx3 = C->m_contact_p.x - B->m_contact_p.x;
                    float fy3 = C->m_contact_p.y - B->m_contact_p.y;
                    float area3 = fabsf(ey3 * fx3 - fy3 * ex3);
                    C->m_p.x = area3;
                }
 
                end = last;
 
                // Compact: remove smallest_i by shifting the rest left
                if (smallest_i < last)
                {
                    int shift_count = (int)((char *)last - (char *)smallest_i - 1) / 4 + 1;
                    memmove(smallest_i, smallest_i + 1, shift_count * sizeof(void *));
                }
            }
 
            this->m_contact_point_count = 5;
        }
 
        // -----------------------------------------------------------------------
        // Allocate contact_point_info and project each intersection point onto
        // both feature planes to get body-local contact positions.
        // -----------------------------------------------------------------------
        contact_point_info *cpi = contact_point_info::create_cpi(
            this->m_contact_point_count, false, this->m_cpi_allocator);
        this->m_cpi = cpi;
 
        if (cpi)
        {
            phys_vec3 *out_b1 = cpi->m_list_b1_r_loc;
            phys_vec3 *out_b2 = cpi->m_list_b2_r_loc;
 
            contact_manifold_mesh_point **mp_i    = this->m_list_isect_point;
            contact_manifold_mesh_point **mp_end  = mp_i + this->m_contact_point_count;
 
            for (; mp_i != mp_end; ++mp_i, ++out_b1, ++out_b2)
            {
                // Convert 2D contact point back to 3D in cg1 space
                phys_vec3 ip_3d;
                phys_v2_to_v3_multiply(&ip_3d, &this->contact_mat,
                    (const phys_vec2 *)((char *)(*mp_i) + 16)); // m_contact_p offset
 
                // --- Project onto cman1 feature plane -> body1 local ---
                float dot_ip_n1 = this->cman1.m_feature_normal.dot(ip_3d);
                float t1        = (n1 - dot_ip_n1) / d1;
 
                phys_vec3 ip_on_plane1;
                ip_on_plane1.x = ip_3d.x + t1 * contact_n->x;
                ip_on_plane1.y = ip_3d.y + t1 * contact_n->y;
                ip_on_plane1.z = ip_3d.z + t1 * contact_n->z;
 
                phys_vec3 scratch2;
                const phys_vec3 *r1 = phys_multiply(&scratch2, cg1_to_rb1_xform, &ip_on_plane1);
                out_b1->x = r1->x + cg1_to_rb1_xform->w.x;
                out_b1->y = r1->y + cg1_to_rb1_xform->w.y;
                out_b1->z = r1->z + cg1_to_rb1_xform->w.z;
 
                // --- Project onto cman2 feature plane -> body2 local ---
                // ip_3d shifted by the CCD translation
                phys_vec3 ip_3d_shifted;
                ip_3d_shifted.x = ip_3d.x + cg1_rel_trans.x;
                ip_3d_shifted.y = ip_3d.y + cg1_rel_trans.y;
                ip_3d_shifted.z = ip_3d.z + cg1_rel_trans.z;
 
                float dot_ip_n2 = this->cman2.m_feature_normal.dot(ip_3d_shifted);
                float t2        = (n2 - dot_ip_n2) / d2;
 
                phys_vec3 ip_on_plane2;
                ip_on_plane2.x = ip_3d_shifted.x + t2 * contact_n->x;
                ip_on_plane2.y = ip_3d_shifted.y + t2 * contact_n->y;
                ip_on_plane2.z = ip_3d_shifted.z + t2 * contact_n->z;
 
                phys_vec3 scratch3;
                const phys_vec3 *r2 = phys_multiply(&scratch3, &this->cg1_to_rb2_xform, &ip_on_plane2);
                out_b2->x = r2->x + this->cg1_to_rb2_xform.w.x;
                out_b2->y = r2->y + this->cg1_to_rb2_xform.w.y;
                out_b2->z = r2->z + this->cg1_to_rb2_xform.w.z;
            }
        }
    }
 
FINALIZE:
    if (!this->m_cpi
        && _tlAssert("source/phys_collision.cpp", 155, "m_cpi", ""))
    {
        __debugbreak();
    }
 
    // -----------------------------------------------------------------------
    // Store the contact normal (negated n, rotated into rb1 space)
    // -----------------------------------------------------------------------
    {
        phys_vec3 neg_contact_n = -(*contact_n);
 
        phys_vec3 scratch4;
        const phys_vec3 *n_rb1 = phys_multiply(&scratch4, bpi1->m_cg_to_world_xform, &neg_contact_n);
 
        this->m_cpi->m_normal.x = n_rb1->x;     // stored as m_feature_normal in the raw struct
        this->m_cpi->m_normal.y = n_rb1->y;
        this->m_cpi->m_normal.z = n_rb1->z;
 
        PHYS_ASSERT_UNIT(&this->m_cpi->m_normal);
    }
 
    // -----------------------------------------------------------------------
    // Compute translation_lambda for continuous collision
    // -----------------------------------------------------------------------
    {
        float vel_along_n = contact_n->dot(gjk_info->m_cg1_relative_translation_loc);
 
        if (vel_along_n >= -0.17f)
        {
            this->m_cpi->m_translation_lambda = 1.0f;
        }
        else
        {
            float t = -dist_p1_p2_n / vel_along_n;
            t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
            this->m_cpi->m_translation_lambda = t;
        }
    }
 
    // -----------------------------------------------------------------------
    // Set surface properties and link into the CPI list
    // -----------------------------------------------------------------------
    set_cpi_params(this->m_cpi, pcp);
 
    if (this->m_list_cpi.m_last_next_ptr == nullptr
        && _tlAssert("C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                     230, "m_last_next_ptr", ""))
    {
        __debugbreak();
    }
 
    contact_point_info *cpi = this->m_cpi;
    cpi->m_next_link = NULL;
    ++this->m_list_cpi.m_alloc_count;
    *this->m_list_cpi.m_last_next_ptr   = cpi;
    this->m_list_cpi.m_last_next_ptr = &cpi->m_next_link;
 
    // -----------------------------------------------------------------------
    // Look up any cached constraint for this body pair
    // -----------------------------------------------------------------------
    this->m_cpi->m_pcp = pcp;
 
    rigid_body *rb2 = pcp->m_bpi2->m_rb;
    rigid_body_pair_key pair_key(rb1, rb2);
 
    rigid_body_constraint_contact *rbc = avl_tree_find(
        this->m_rbc_contact_search_tree_root, &pair_key);
 
    this->m_cpi->m_rbc_contact = rbc;
 
    if (rbc)
    {
        // If the bodies are stored in reverse order in the constraint, flip
        // the b1/b2 pointers and negate the normal.
        if (rbc->b1 != rb1)
        {
            phys_vec3 *tmp_b1      = this->m_cpi->m_list_b1_r_loc;
            this->m_cpi->m_list_b1_r_loc = this->m_cpi->m_list_b2_r_loc;
            this->m_cpi->m_list_b2_r_loc = tmp_b1;
 
            this->m_cpi->m_normal.x = -this->m_cpi->m_normal.x;
            this->m_cpi->m_normal.y = -this->m_cpi->m_normal.y;
            this->m_cpi->m_normal.z = -this->m_cpi->m_normal.z;
        }
 
        this->m_cpi->set_closest_cached_psc(
            (contact_point_info *)rbc->m_list_contact_point_info_buffer_2.m_first);
    }
    else
    {
        this->m_cpi->set_closest_cached_psc(nullptr);
    }
}

const bpei_database_id *__cdecl get_database_id(bpei_database_id *result, gjk_physics_collision_visitor *collision_visitor)
{
    if (collision_visitor->cent)
    {
        result->m_id1 = (unsigned int)collision_visitor->cent;
        result->m_id2 = 0;
        return result;
    }
    else if (collision_visitor->glass)
    {
        result->m_id1 = (unsigned int)collision_visitor->glass;
        result->m_id2 = 0;
        return result;
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                418,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        result->m_id1 = (unsigned int)collision_visitor->dynEntDef;
        result->m_id2 = 0;
        return result;
    }
}

phys_mat44 *__cdecl create_ent_mat(gjk_physics_collision_visitor *collision_visitor)
{
    phys_mat44 *v3; // [esp+0h] [ebp-A8h]
    float v4[9]; // [esp+8h] [ebp-A0h] BYREF
    DynEntityDrawType v5; // [esp+2Ch] [ebp-7Ch]
    const float *origin; // [esp+30h] [ebp-78h]
    float v7[9]; // [esp+38h] [ebp-70h] BYREF
    const float *inVector; // [esp+5Ch] [ebp-4Ch]
    float axis[9]; // [esp+64h] [ebp-44h] BYREF
    int v11; // [esp+8Ch] [ebp-1Ch]
    int v12; // [esp+90h] [ebp-18h]
    phys_mat44 *v13; // [esp+94h] [ebp-14h]
    DynEntityPose *dynEntPose; // [esp+98h] [ebp-10h]
    DynEntityDrawType drawType; // [esp+9Ch] [ebp-Ch]
    unsigned __int16 dynEntId; // [esp+A0h] [ebp-8h]
    phys_mat44 *ent_mat; // [esp+A4h] [ebp-4h]

    v12 = 4;
    v11 = 16;
    v13 = (phys_mat44 *)collision_visitor->allocate(sizeof(phys_mat44), 16, 0);
    if (v13)
        v3 = v13;
    else
        v3 = 0;
    ent_mat = v3;
    if (collision_visitor->cent)
    {
        inVector = collision_visitor->cent->pose.origin;
        AnglesToAxis(collision_visitor->cent->pose.angles, (float (*)[3])axis);
        Phys_AxisToNitrousMat((float (*)[3])axis, ent_mat);
        Phys_Vec3ToNitrousVec(inVector, &ent_mat->w);
    }
    else if (collision_visitor->glass)
    {
        origin = collision_visitor->glass->origin;
        AnglesToAxis(collision_visitor->glass->angles, (float (*)[3])v7);
        Phys_AxisToNitrousMat((float (*)[3])v7, ent_mat);
        Phys_Vec3ToNitrousVec(origin, &ent_mat->w);
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                361,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        v5 = (DynEntityDrawType)(collision_visitor->dynEntDef->xModel == 0);
        drawType = v5;
        dynEntId = DynEnt_GetId(collision_visitor->dynEntDef, v5);
        dynEntPose = DynEnt_GetClientPose(dynEntId, v5);
        UnitQuatToAxis(dynEntPose->pose.quat, (float (*)[3])v4);
        Phys_AxisToNitrousMat((float (*)[3])v4, ent_mat);
        Phys_Vec3ToNitrousVec(dynEntPose->pose.origin, &ent_mat->w);
    }
    return ent_mat;
}

phys_auto_activate_callback *__cdecl create_ent_aac(gjk_physics_collision_visitor *collision_visitor)
{
    DynEntityDef *dynEntDef; // [esp+10h] [ebp-30h]
    //phys_auto_activate_callback_vtbl *cent; // [esp+18h] [ebp-28h]
    dynamic_ent_aa *v6; // [esp+28h] [ebp-18h]
    destructible_ent_aa *v7; // [esp+34h] [ebp-Ch]

    if (collision_visitor->cent)
    {
        if (collision_visitor->cent->destructible && (collision_visitor->bpeqi->env_collision_flags & 8) != 0)
        {
            v7 = (destructible_ent_aa *)collision_visitor->allocate(sizeof(destructible_ent_aa), 4, 0);
            if (!v7)
                return 0;
            //cent = (phys_auto_activate_callback_vtbl *)collision_visitor->cent;
            //v7->__vftable = (destructible_ent_aa_vtbl *)&phys_auto_activate_callback::`vftable';
            //v7->__vftable = (destructible_ent_aa_vtbl *)&destructible_ent_aa::`vftable';
            new ((void *)v7) destructible_ent_aa();
            
            v7->m_has_auto_activated = 0;
            v7->m_cent = (centity_s*)collision_visitor->cent; // de-const cast
            return (dynamic_ent_aa *)v7;
        }
        else
        {
            return 0;
        }
    }
    else if (collision_visitor->glass)
    {
        return 0;
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                391,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        v6 = (dynamic_ent_aa *)collision_visitor->allocate(sizeof(dynamic_ent_aa), 4, 0);
        if (!v6)
            return 0;
        dynEntDef = (DynEntityDef*)collision_visitor->dynEntDef;

        
        //v6->__vftable = (dynamic_ent_aa_vtbl *)&phys_auto_activate_callback::`vftable';
        //v6->__vftable = (dynamic_ent_aa_vtbl *)&dynamic_ent_aa::`vftable';
        new ((void *)v6) dynamic_ent_aa();
        v6->m_has_auto_activated = 0;
        v6->m_dynEntDef = dynEntDef;
        return v6;
    }
}

void create_entity_bpi(gjk_physics_collision_visitor *collision_visitor, int mask)
{
    const centity_s *entity; // eax
    bpei_database_id database_id; // [esp+34h] [ebp-10h] BYREF
    broad_phase_environment_info *bpei; // [esp+3Ch] [ebp-8h]
    entity_bpi_header *ebpih = NULL; // [esp+40h] [ebp-4h]
    //int savedregs; // [esp+44h] [ebp+0h] BYREF

    get_database_id(&database_id, collision_visitor);
    bpei = G_BPM->g_bpei_database.get_bpei_mt(database_id);
    if (!bpei->m_data)
    {
        //minspec_mutex::Lock(&bpei->m_mutex);
        bpei->m_mutex.Lock();
        if (!bpei->m_data)
        {
            ebpih = (entity_bpi_header *)collision_visitor->allocate(sizeof(entity_bpi_header), 4, 0);
            if (collision_visitor->rb->is_environment_rigid_body())
                ebpih->m_mat = create_ent_mat(collision_visitor);
            else
                ebpih->m_mat = 0;
            ebpih->m_aac = create_ent_aac(collision_visitor);

            iassert(ebpih);
            bpei->m_data = ebpih;
        }
        //minspec_mutex::Unlock(&bpei->m_mutex);
        bpei->m_mutex.Unlock();
    }

    iassert(bpei->m_data);
    ebpih = (entity_bpi_header *)bpei->m_data;

    //if (rigid_body::is_environment_rigid_body(collision_visitor->rb))
    if (collision_visitor->rb->is_environment_rigid_body())
    {
        if (collision_visitor->cg_to_world_xform
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                463,
                0,
                "%s",
                "collision_visitor->cg_to_world_xform == NULL"))
        {
            __debugbreak();
        }
        if (collision_visitor->cg_to_rb_xform
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                464,
                0,
                "%s",
                "collision_visitor->cg_to_rb_xform == NULL"))
        {
            __debugbreak();
        }
        if (!ebpih->m_mat
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                465,
                0,
                "%s",
                "ebpih->m_mat"))
        {
            __debugbreak();
        }
        collision_visitor->cg_to_world_xform = ebpih->m_mat;
        collision_visitor->cg_to_rb_xform = ebpih->m_mat;
    }
    collision_visitor->auto_activate_callback = ebpih->m_aac;
    if (!collision_visitor->rb_to_world_xform
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            472,
            0,
            "%s",
            "collision_visitor->rb_to_world_xform"))
    {
        __debugbreak();
    }
    if (!collision_visitor->cg_to_world_xform
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            473,
            0,
            "%s",
            "collision_visitor->cg_to_world_xform"))
    {
        __debugbreak();
    }
    if (!collision_visitor->cg_to_rb_xform
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            474,
            0,
            "%s",
            "collision_visitor->cg_to_rb_xform"))
    {
        __debugbreak();
    }
    entity = get_entity(collision_visitor);
    //gjk_physics_collision_visitor::set_local_query_info(collision_visitor, (int)&savedregs, entity);
    collision_visitor->set_local_query_info(entity);
    if (collision_visitor->cent)
    {
        create_gjk_geom(0, collision_visitor->cent, collision_visitor, 0, mask, 0, 1);
    }
    else if (collision_visitor->glass)
    {
        create_gjk_geom(collision_visitor->glass, collision_visitor, mask);
    }
    else
    {
        if (!collision_visitor->dynEntDef
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
                487,
                0,
                "%s",
                "collision_visitor->dynEntDef"))
        {
            __debugbreak();
        }
        create_gjk_geom(collision_visitor->dynEntDef, collision_visitor, mask);
    }
}

void __cdecl phys_aabb_add_hace(phys_vec3 *aabb_min, phys_vec3 *aabb_max)
{
    aabb_min->x = aabb_min->x - 0.50999999;
    aabb_min->y = aabb_min->y - 0.50999999;
    aabb_min->z = aabb_min->z - 0.50999999;
    aabb_max->x = aabb_max->x + 0.50999999;
    aabb_max->y = aabb_max->y + 0.50999999;
    aabb_max->z = aabb_max->z + 0.50999999;
}

void __cdecl set_bp_standard_query()
{
    G_BPM->g_broad_phase_terrain_query_callback = &g_standard_query;
}

void __cdecl debug_callback(void*)
{
    if (phys_debugCallback->current.enabled)
    {
        if (_tlAssert("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 796, "0", ""))
            __debugbreak();
    }
}

void __cdecl set_debug_callback()
{
    phys_set_debug_callback(debug_callback);
}

void broad_phase_info::set(
    rigid_body *rb,
    const phys_mat44 *rb_to_world_xform,
    const phys_mat44 *cg_to_world_xform,
    const phys_mat44 *cg_to_rb_xform,
    const phys_gjk_geom *gjk_geom,
    unsigned int gjk_geom_id,
    bool calc_cg_to_world_xform,
    int surface_type,
    void *user_data,
    unsigned int env_collision_flags)
{
    this->m_flags = 0;
    this->m_list_bpb_next = 0;
    this->m_sap_node = 0;
    this->m_flags |= 1u;
    this->m_rb = rb;
    this->m_rb_to_world_xform = rb_to_world_xform;
    this->m_cg_to_world_xform = cg_to_world_xform;
    this->m_cg_to_rb_xform = cg_to_rb_xform;
    this->m_gjk_geom = gjk_geom;
    this->m_gjk_geom_id = gjk_geom_id;
    if (calc_cg_to_world_xform)
        this->m_flags |= 0x200u;
    else
        this->m_flags &= ~0x200u;
    this->m_surface_type = surface_type;
    this->m_user_data = user_data;
    this->m_env_collision_flags = env_collision_flags;
    this->m_my_collision_type_flags = 0;
}

void broad_phase_info::collision_prolog()
{
    float y; // [esp+18h] [ebp-2Ch]
    float z; // [esp+1Ch] [ebp-28h]
    rigid_body *m_rb; // [esp+30h] [ebp-14h]

    if ((this->m_flags & 0x200) != 0)
        phys_full_multiply_mat((phys_mat44 *)this->m_cg_to_world_xform, this->m_rb_to_world_xform, this->m_cg_to_rb_xform);

    ((phys_gjk_geom *)this->m_gjk_geom)->calc_aabb(this->m_cg_to_world_xform, &this->m_trace_aabb_min_whace, &this->m_trace_aabb_max_whace);

    phys_aabb_add_hace(&this->m_trace_aabb_min_whace, &this->m_trace_aabb_max_whace);
    m_rb = this->m_rb;
    y = m_rb->m_moved_vec.y;
    z = m_rb->m_moved_vec.z;
    this->m_trace_translation.x = m_rb->m_moved_vec.x;
    this->m_trace_translation.y = y;
    this->m_trace_translation.z = z;

    if ((this->m_rb->m_flags & 0x30) == 0)
        calc_largest_vel_sq(this);
}


void broad_phase_info::set_bpi_env(phys_auto_activate_callback *auto_activate_callback)
{
    iassert(is_bpi()); // "call broad_phase_info::set first."
    iassert(m_sap_node == NULL);
    this->m_flags = 0;
    this->m_sap_node = auto_activate_callback;
    this->m_flags |= 4u;
    if (auto_activate_callback)
        this->m_flags |= 0x80u;
    else
        this->m_flags &= ~0x80u;
}

generic_avl_map_node_t *__cdecl generic_avl_map_add(
    phys_inplace_avl_tree<uintptr_t, generic_avl_map_node_t, generic_avl_map_node_t> *gam,
    void *data,
    uintptr_t avl_key)
{
    //generic_avl_map_node_t *m_tree_root; // [esp+15Ch] [ebp-8h]
    generic_avl_map_node_t *gamn; // [esp+160h] [ebp-4h]

    //m_tree_root = gam->m_tree_root;
    //while (m_tree_root && avl_key != m_tree_root->m_avl_key)
    //{
    //    if (avl_key >= m_tree_root->m_avl_key)
    //        m_tree_root = m_tree_root->m_avl_tree_node.m_right;
    //    else
    //        m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    //}
    //if (m_tree_root
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
    //        1243,
    //        0,
    //        "%s",
    //        "gam->find(avl_key) == NULL"))
    //{
    //    __debugbreak();
    //}
    // 

    iassert(gam->find(avl_key) == NULL);

    //gamn = phys_simple_allocator<generic_avl_map_node_t>::allocate(&g_generic_avl_map_node_allocator);
    gamn = g_generic_avl_map_node_allocator.allocate();

    iassert(gamn);

    gamn->m_data = data;
    //phys_inplace_avl_tree<uintptr_t, generic_avl_map_node_t, generic_avl_map_node_t>::add(gam, &avl_key, gamn);
    gam->add(avl_key, gamn);

    iassert(gam->find(avl_key) != NULL); // lwss add

    return gamn;
}

void *__cdecl generic_avl_map_destroy(
    phys_inplace_avl_tree<uintptr_t, generic_avl_map_node_t, generic_avl_map_node_t> *gam,
    uintptr_t avl_key)
{
    generic_avl_map_node_t *m_tree_root; // [esp+144h] [ebp-Ch]
    void *data; // [esp+148h] [ebp-8h]

    m_tree_root = gam->m_tree_root;
    while (m_tree_root && avl_key != m_tree_root->m_avl_key)
    {
        if (avl_key >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (!m_tree_root)
        return 0;
    data = m_tree_root->m_data;
    //phys_inplace_avl_tree<uintptr_t, generic_avl_map_node_t, generic_avl_map_node_t>::remove(gam, &avl_key);
    gam->remove(avl_key);
    //phys_simple_allocator<generic_avl_map_node_t>::free(&g_generic_avl_map_node_allocator, m_tree_root);
    g_generic_avl_map_node_allocator.free(m_tree_root);
    return data;
}

// LWSS: yes, convert the int directly to a pointer to a struct. 
PhysObjUserData *__cdecl Phys_GetUserData(intptr_t id)
{
    iassert(id);

    return (PhysObjUserData *)id;
}

int __cdecl get_physics_contents_mask(char phys_env_collision_flags)
{
    int mask; // [esp+0h] [ebp-4h]

    mask = 0x280EC93;
    if ((phys_env_collision_flags & 0x20) != 0 && (phys_env_collision_flags & 0x10) == 0)
        return 0x281EE93;
    if ((phys_env_collision_flags & 8) != 0 || (phys_env_collision_flags & 0x10) != 0)
        return 0x211;
    return mask;
}

broad_phase_info *__cdecl allocate_bpi_env()
{
    broad_phase_info *bpi; // [esp+0h] [ebp-4Ch]
    broad_phase_info *first; // [esp+44h] [ebp-8h]

    // nx-port: was mt_allocate(112) and a push through x86 offset 56
    // (m_list_bpb_cluster_next) with a 32-bit compare-exchange.
    _InterlockedExchangeAdd(&G_BPM->m_bpi_env_count, 1u);
    bpi = (broad_phase_info *)G_BPM->g_collision_memory_buffer.mt_allocate(
        sizeof(broad_phase_info),
        16,
        0,
        "broad phase collision out of memory.");
    do
    {
        first = G_BPM->m_list_bpi_env;
        bpi->m_list_bpb_cluster_next = first;
    } while (__sync_val_compare_and_swap(&G_BPM->m_list_bpi_env, first, bpi) != first);
    return bpi;
}

char are_intersecting(
    const broad_phase_environment_query_input *bpeqi,
    const phys_vec3 *aabb_min,
    const phys_vec3 *aabb_max,
    const phys_vec3 *aabb_trans)
{
    float v6; // [esp-Ch] [ebp-30h] BYREF
    phys_vec3 v7; // [esp-8h] [ebp-2Ch] OVERLAPPED BYREF
    float trans_4; // [esp+8h] [ebp-1Ch]
    float trans_8; // [esp+Ch] [ebp-18h]
    float trans_12; // [esp+10h] [ebp-14h]
    const phys_vec3 *p_trace_translation; // [esp+14h] [ebp-10h]
    //int v12; // [esp+18h] [ebp-Ch]
    //void *v13; // [esp+1Ch] [ebp-8h]
    //void *retaddr; // [esp+24h] [ebp+0h]
    //
    //v12 = a1;
    //v13 = retaddr;
    p_trace_translation = &bpeqi->trace_translation;
    trans_12 = bpeqi->trace_translation.x - aabb_trans->x;
    trans_8 = bpeqi->trace_translation.y - aabb_trans->y;
    trans_4 = bpeqi->trace_translation.z - aabb_trans->z;
    v7.x = trans_12;
    v7.y = trans_8;
    v7.z = trans_4;
    return phys_are_potentially_colliding(
        &bpeqi->trace_aabb_min_wace,
        &bpeqi->trace_aabb_max_wace,
        &v7,
        aabb_min,
        aabb_max,
        &v6);
}


// aislop supervised
void standard_query::query(
    const broad_phase_environment_query_input *bpeqi,
    broad_phase_environement_query_results *bpeqr)
{
    // Compute swept AABB: union of start and end positions
    phys_vec3 aabb_end_min = bpeqi->trace_aabb_min_wace + bpeqi->trace_translation;
    phys_vec3 swept_min;
    phys_min(&swept_min, &bpeqi->trace_aabb_min_wace, &aabb_end_min);

    phys_vec3 aabb_end_max = bpeqi->trace_aabb_max_wace + bpeqi->trace_translation;
    phys_vec3 swept_max;
    phys_max(&swept_max, &bpeqi->trace_aabb_max_wace, &aabb_end_max);

    float mins[3], maxs[3];
    Phys_NitrousVecToVec3(&swept_min, mins);
    Phys_NitrousVecToVec3(&swept_max, maxs);

    iassert(!isnan(mins[0]) && !isnan(mins[1]) && !isnan(mins[2]));
    iassert(!isnan(maxs[0]) && !isnan(maxs[1]) && !isnan(maxs[2]));

    // Debug visualization for large queries
    if (phys_debugBigQueries->current.enabled &&
        Abs(bpeqi->trace_translation) > 500.0f)
    {
        g_render_mutex.Lock();
        render_box(&bpeqi->trace_aabb_min_wace, &bpeqi->trace_aabb_max_wace, colorBlue, 1000);

        float lineStart[3];
        Phys_NitrousVecToVec3(&bpeqi->trace_aabb_min_wace, lineStart);

        phys_vec3 aabb_end = bpeqi->trace_aabb_min_wace + bpeqi->trace_translation;
        float lineEnd[3];
        Phys_NitrousVecToVec3(&aabb_end, lineEnd);
        CG_DebugLine(lineStart, lineEnd, colorRed, 1, 1000);

        lineEnd[0] = lineStart[0];
        lineEnd[1] = lineStart[1];
        lineEnd[2] = lineStart[2] + 10000.0f;
        CG_DebugLine(lineStart, lineEnd, colorWhite, 1, 1000);

        g_render_mutex.Unlock();
    }

    // Initialize the collision visitor and query results
    gjk_physics_collision_visitor v83;
    v83.bpeqi = bpeqi;
    v83.bpeqr = bpeqr;

    bpeqr->m_list_bpi_env.m_list_cur = &bpeqr->m_list_bpi_env.m_list;
    bpeqr->m_list_bpi_env_count = 0;
    bpeqr->m_env_collision_flags = 0;

    environment_rigid_body *env_rb = phys_sys::get_environment_rigid_body();
    int physics_contents_mask = get_physics_contents_mask(bpeqi->env_collision_flags);

    // --- Static world geometry (brushes and partitions) ---
    if (bpeqi->env_collision_flags & 1)
    {
        static_colgeom_visitor_t v76;
        v76.intersect_box(mins, maxs, physics_contents_mask);

        // Process brushes
        for (int i = 0; i < v76.nbrushes; ++i)
        {
            const cbrush_t *brush = v76.brushes[i];

            phys_vec3 brush_min, brush_max;
            Phys_Vec3ToNitrousVec(brush->mins, &brush_min);
            Phys_Vec3ToNitrousVec(brush->maxs, &brush_max);

            phys_vec3 zero_trans = PHYS_ZERO_VEC;
            if (!are_intersecting(bpeqi, &brush_min, &brush_max, &zero_trans))
                continue;

            broad_phase_environment_info *bpei_mt =
                G_BPM->g_bpei_database.get_bpei_mt(bpei_database_id((unsigned int)brush));

            if (!bpei_mt->m_data)
            {
                bpei_mt->m_mutex.Lock();
                if (!bpei_mt->m_data)
                {
                    int stype = (brush->axial_sflags[0][0] & 0x3F00000) >> 20;
                    gjk_brush_t *geom = gjk_brush_t::create(brush, stype, &v83);
                    geom->set_geom_id_new(bpei_mt->m_gjk_geom_id);

                    broad_phase_info *bpi_env = allocate_bpi_env();
                    bpi_env->set(
                        env_rb,
                        &PHYS_IDENTITY_MATRIX,
                        &PHYS_IDENTITY_MATRIX,
                        nullptr,
                        geom,
                        geom->get_geom_id(),
                        false,
                        geom->stype,
                        nullptr,
                        1u);
                    bpi_env->set_bpi_env(nullptr);

                    if (bpi_env)
                    {
                        iassert(bpi_env->is_bpi_env());
                        bpi_env->get_bpi_env()->collision_prolog();
                        bpei_mt->m_data = bpi_env;
                    }
                }
                bpei_mt->m_mutex.Unlock();
            }

            if (bpei_mt->m_data)
                bpeqr->add((broad_phase_base *)bpei_mt->m_data);
        }

        // Process terrain partitions (CollisionAabbTrees)
        for (int j = 0; j < v76.ntrees; ++j)
        {
            const CollisionAabbTree *tree = v76.trees[j];
            const CollisionPartition *partition = &cm.partitions[tree->u.firstChildIndex];

            phys_vec3 tree_half, tree_origin;
            Phys_Vec3ToNitrousVec(tree->halfSize, &tree_half);
            Phys_Vec3ToNitrousVec(tree->origin, &tree_origin);

            phys_vec3 tree_min = tree_origin - tree_half;
            phys_vec3 tree_max = tree_origin + tree_half;

            // Translation relative to static geometry is zero
            phys_vec3 rel_trans = bpeqi->trace_translation - PHYS_ZERO_VEC;

            float hit_time;
            if (!phys_are_potentially_colliding(
                &bpeqi->trace_aabb_min_wace,
                &bpeqi->trace_aabb_max_wace,
                &rel_trans,
                &tree_min,
                &tree_max,
                &hit_time))
                continue;

            broad_phase_environment_info *bpei_mt =
                G_BPM->g_bpei_database.get_bpei_mt(bpei_database_id((unsigned int)partition));

            if (!bpei_mt->m_data)
            {
                bpei_mt->m_mutex.Lock();
                if (!bpei_mt->m_data)
                {
                    gjk_partition_t *geom = gjk_partition_t::create(tree, &v83);
                    geom->set_geom_id_new(bpei_mt->m_gjk_geom_id);

                    broad_phase_info *bpi_env = allocate_bpi_env();
                    bpi_env->set(
                        env_rb,
                        &PHYS_IDENTITY_MATRIX,
                        &PHYS_IDENTITY_MATRIX,
                        nullptr,
                        geom,
                        geom->get_geom_id(),
                        false,
                        geom->stype,
                        nullptr,
                        1u);
                    bpi_env->set_bpi_env(nullptr);

                    if (bpi_env)
                    {
                        iassert(bpi_env->is_bpi_env());
                        bpi_env->get_bpi_env()->collision_prolog();
                        bpei_mt->m_data = bpi_env;
                    }
                }
                bpei_mt->m_mutex.Unlock();
            }

            if (bpei_mt->m_data)
                bpeqr->add((broad_phase_base *)bpei_mt->m_data);
        }
    }

    // --- Dynamic entity collision ---
    if (phys_entityCollision->current.enabled)
    {
        // DynEntities (client-side dynamic entities)
        if (!(bpeqi->env_collision_flags & 0x10) && (bpeqi->env_collision_flags & 4))
        {
            DynEntityAreaParms areaParms;
            areaParms.mins = mins;
            areaParms.maxs = maxs;
            areaParms.contentMask = -1;
            areaParms.list = (uint16_t *)alloca(4096 * sizeof(uint16_t));
            areaParms.maxCount = 4096;

            for (DynEntityDrawType k = DYNENT_DRAW_MODEL; k < DYNENT_DRAW_COUNT; ++k)
            {
                areaParms.count = 0;
                DynEntCl_AreaEntities_r((DynEntityCollType)k, 1u, &areaParms);

                for (int m = 0; m < areaParms.count; ++m)
                {
                    const DynEntityDef *entDef = DynEnt_GetEntityDef(areaParms.list[m], k);

                    if (!(bpeqi->env_collision_flags & 8) && !(entDef->flags & 2))
                        continue;

                    unsigned short id = DynEnt_GetId(entDef, k);
                    DynEntityClient *clientEnt = DynEnt_GetClientEntity(id, k);

                    if (clientEnt->physObjId)
                        continue;

                    v83.cent = nullptr;
                    v83.dynEntDef = entDef;
                    v83.glass = nullptr;
                    v83.rb = env_rb;
                    v83.rb_to_world_xform = &PHYS_IDENTITY_MATRIX;
                    v83.cg_to_world_xform = nullptr;
                    v83.cg_to_rb_xform = nullptr;
                    v83.env_collision_flags = 4;
                    create_entity_bpi(&v83, physics_contents_mask);
                }
            }
        }

        // World entities (players, vehicles, etc.)
        if (bpeqi->env_collision_flags & 2)
        {
            int entityList[16];
            int entCount = CG_AreaEntities(mins, maxs, entityList, 16, physics_contents_mask);

            for (int n = 0; n < entCount; ++n)
            {
                const centity_s *cent = CG_GetEntity(0, entityList[n]);
                const DObj *dobj = Com_GetClientDObj(entityList[n], 0);

                if (dobj)
                {
                    const XModel *model = DObjGetModel(dobj, 0);
                    if (model)
                    {
                        int contents = XModelGetContents(model);
                        if (!(physics_contents_mask & contents) && cent->nextState.eType != 17)
                            continue;
                    }
                }

                if (cent->nextState.eType == 6)
                {
                    auto_rigid_body::add(cent, &v83, physics_contents_mask);
                }
                else if (cent->nextState.eType == 14 || cent->nextState.eType == 16)
                {
                    short vehicleInfoIndex = cent->nextState.vehicleState.vehicleInfoIndex;
                    const vehicle_info_t *vehicleInfo = CG_GetVehicleInfo(vehicleInfoIndex);
                    if (!vehicleInfo->name[0])
                        vehicleInfo = BG_GetVehicleInfo(vehicleInfoIndex);

                    iassert(vehicleInfo->name[0] > 0);

                    if (!vehicleInfo->isNitrous)
                        auto_rigid_body::add(cent, &v83, physics_contents_mask);
                }
                else if (cent->nextState.eType == 17 && dynEnt_sentientAutoActivate->current.enabled)
                {
                    auto_rigid_body::add(cent, &v83, physics_contents_mask);
                }
            }
        }

        // Glass entities
        if (bpeqi->env_collision_flags & 0x40)
        {
            const Glass *glasses[128];
            unsigned int glassCount = GlassCl_AreaGlasses(mins, maxs, glasses, 128u);

            for (unsigned int ii = 0; ii < glassCount; ++ii)
            {
                v83.cent = nullptr;
                v83.dynEntDef = nullptr;
                v83.glass = glasses[ii];
                v83.rb = phys_sys::get_environment_rigid_body();
                v83.rb_to_world_xform = &PHYS_IDENTITY_MATRIX;
                v83.cg_to_world_xform = nullptr;
                v83.cg_to_rb_xform = nullptr;
                v83.env_collision_flags = 64;
                create_entity_bpi(&v83, physics_contents_mask);
            }
        }

        // User rigid bodies (physics objects owned by the game)
        if ((bpeqi->env_collision_flags & 0x10) &&
            !(bpeqi->env_collision_flags & 8) &&
            (bpeqi->env_collision_flags & 0x100))
        {
            for (int jj = 0; jj < 16; ++jj)
            {
                PhysObjUserData *userRb = physGlob.userRigidBodies[jj];
                if (!userRb)
                    continue;

                iassert(userRb->m_gjk_geom_list.m_geom_count >= 0);

                gjk_base_t *first_geom = userRb->m_gjk_geom_list.m_first_geom;

                broad_phase_environment_info *bpei_mt =
                    G_BPM->g_bpei_database.get_bpei_mt(bpei_database_id((unsigned int)userRb));

                if (!bpei_mt->m_data)
                {
                    bpei_mt->m_mutex.Lock();
                    if (!bpei_mt->m_data)
                    {
                        broad_phase_info *bpi_env = allocate_bpi_env();
                        bpi_env->set(
                            userRb->body,
                            &userRb->body->m_mat,
                            &userRb->cg2w,
                            &userRb->cg2rb,
                            first_geom,
                            first_geom->get_geom_id(),
                            false,
                            first_geom->stype,
                            nullptr,
                            2u);
                        bpi_env->set_bpi_env(nullptr);

                        if (bpi_env)
                        {
                            iassert(bpi_env->is_bpi_env());
                            bpi_env->get_bpi_env()->collision_prolog();
                            bpei_mt->m_data = bpi_env;
                        }
                    }
                    bpei_mt->m_mutex.Unlock();
                }

                if (bpei_mt->m_data)
                    bpeqr->add((broad_phase_base *)bpei_mt->m_data);
            }
        }
    }
}

void gjk_physics_collision_visitor::set_local_query_info(
    const void *entity)
{
    const phys_vec3 *v3; // eax
    phys_vec3 *p_m_local_query_trace_translation; // [esp-58h] [ebp-64h]
    phys_vec3 v5; // [esp-50h] [ebp-5Ch] BYREF
    phys_vec3 v6; // [esp-40h] [ebp-4Ch] BYREF
    float v7; // [esp-30h] [ebp-3Ch]
    float v8; // [esp-2Ch] [ebp-38h]
    float v9; // [esp-28h] [ebp-34h]
    const float *p_x; // [esp-24h] [ebp-30h]
    phys_vec3 m_moved_vec; // [esp-20h] [ebp-2Ch]
    rigid_body *rb; // [esp-8h] [ebp-14h]
    gjk_physics_collision_visitor *v13; // [esp-4h] [ebp-10h]
    //int v14; // [esp+0h] [ebp-Ch]
    //void *v15; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v14 = a2;
    //v15 = retaddr;
    v13 = this;
    phys_calc_local_aabb(
        &this->bpeqi->trace_aabb_min_wace,
        &this->bpeqi->trace_aabb_max_wace,
        this->cg_to_world_xform,
        &this->m_local_query_trace_aabb_min,
        &this->m_local_query_trace_aabb_max);
    rb = v13->rb;
    m_moved_vec = rb->m_moved_vec;
    p_x = &v13->bpeqi->trace_translation.x;
    v9 = *p_x - m_moved_vec.x;
    v8 = p_x[1] - m_moved_vec.y;
    v7 = p_x[2] - m_moved_vec.z;
    v6.x = v9;
    v6.y = v8;
    v6.z = v7;
    v3 = phys_inv_multiply(&v5, v13->cg_to_world_xform, &v6);
    p_m_local_query_trace_translation = &v13->m_local_query_trace_translation;
    v13->m_local_query_trace_translation.x = v3->x;
    p_m_local_query_trace_translation->y = v3->y;
    p_m_local_query_trace_translation->z = v3->z;
    v13->m_local_entity = entity;
}

void gjk_physics_collision_visitor::get_local_query_aabb(
    float *local_query_aabb_min,
    float *local_query_aabb_max)
{
    phys_vec3 v4; // [esp-Ch] [ebp-8Ch] BYREF
    phys_vec3 m_local_query_aabb_max_4; // [esp+4h] [ebp-7Ch] BYREF
    float v6; // [esp+20h] [ebp-60h]
    float v7; // [esp+24h] [ebp-5Ch]
    float v8; // [esp+28h] [ebp-58h]
    phys_vec3 *p_m_local_query_trace_aabb_max; // [esp+2Ch] [ebp-54h]
    phys_vec3 *v10; // [esp+30h] [ebp-50h]
    phys_vec3 m_local_query_aabb_min; // [esp+34h] [ebp-4Ch] BYREF
    phys_vec3 v12; // [esp+44h] [ebp-3Ch] BYREF
    float v13; // [esp+5Ch] [ebp-24h]
    float v14; // [esp+60h] [ebp-20h]
    float v15; // [esp+64h] [ebp-1Ch]
    phys_vec3 *p_m_local_query_trace_aabb_min; // [esp+68h] [ebp-18h]
    phys_vec3 *p_m_local_query_trace_translation; // [esp+6Ch] [ebp-14h]
    gjk_physics_collision_visitor *thisa; // [esp+70h] [ebp-10h]
    //_UNKNOWN *v19; // [esp+74h] [ebp-Ch]
    //float *local_query_aabb_mina; // [esp+78h] [ebp-8h]
    //int vars0; // [esp+80h] [ebp+0h]
    //
    //v19 = a2;
    //local_query_aabb_mina = (float *)vars0;
    thisa = this;
    p_m_local_query_trace_translation = &this->m_local_query_trace_translation;
    p_m_local_query_trace_aabb_min = &this->m_local_query_trace_aabb_min;
    v15 = this->m_local_query_trace_aabb_min.x + this->m_local_query_trace_translation.x;
    v14 = this->m_local_query_trace_aabb_min.y + this->m_local_query_trace_translation.y;
    v13 = this->m_local_query_trace_aabb_min.z + this->m_local_query_trace_translation.z;
    v12.x = v15;
    v12.y = v14;
    v12.z = v13;
    phys_min(&m_local_query_aabb_min, &this->m_local_query_trace_aabb_min, &v12);
    v10 = &thisa->m_local_query_trace_translation;
    p_m_local_query_trace_aabb_max = &thisa->m_local_query_trace_aabb_max;
    v8 = thisa->m_local_query_trace_aabb_max.x + thisa->m_local_query_trace_translation.x;
    v7 = thisa->m_local_query_trace_aabb_max.y + thisa->m_local_query_trace_translation.y;
    v6 = thisa->m_local_query_trace_aabb_max.z + thisa->m_local_query_trace_translation.z;
    m_local_query_aabb_max_4.x = v8;
    m_local_query_aabb_max_4.y = v7;
    m_local_query_aabb_max_4.z = v6;
    phys_max(&v4, &thisa->m_local_query_trace_aabb_max, &m_local_query_aabb_max_4);
    Phys_NitrousVecToVec3(&m_local_query_aabb_min, local_query_aabb_min);
    Phys_NitrousVecToVec3(&v4, local_query_aabb_max);
}

bool gjk_physics_collision_visitor::query_create_prolog(const void *geom)
{
    if (!geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 290, 0, "%s", "geom"))
    {
        __debugbreak();
    }
    this->m_local_database_id.m_id1 = (unsigned int)this->m_local_entity;
    this->m_local_database_id.m_id2 = (unsigned int)geom;
    //this->m_local_bpei = bpei_database_t::get_bpei_mt(&G_BPM->g_bpei_database, this->m_local_database_id);
    this->m_local_bpei = G_BPM->g_bpei_database.get_bpei_mt(this->m_local_database_id);

    if (!this->m_local_bpei->m_data)
    {
        //minspec_mutex::Lock(&this->m_local_bpei->m_mutex);
        this->m_local_bpei->m_mutex.Lock();
        if (!this->m_local_bpei->m_data)
            return 1;
        //minspec_mutex::Unlock(&this->m_local_bpei->m_mutex);
        this->m_local_bpei->m_mutex.Unlock();
    }

    if (!this->m_local_bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            300,
            0,
            "%s",
            "m_local_bpei->m_data"))
    {
        __debugbreak();
    }

    //broad_phase_environement_query_results::add(this->bpeqr, (broad_phase_base *)this->m_local_bpei->m_data);
    this->bpeqr->add((broad_phase_base *)this->m_local_bpei->m_data);
    return 0;
}

bool gjk_physics_collision_visitor::query_create_prolog_1(
    const float *local_aabb_min,
    const float *local_aabb_max,
    const void *geom)
{
    float hit_time; // [esp-Ch] [ebp-40h] BYREF
    phys_vec3 pv_local_aabb_max; // [esp-8h] [ebp-3Ch] BYREF
    phys_vec3 pv_local_aabb_min; // [esp+8h] [ebp-2Ch] BYREF

    Phys_Vec3ToNitrousVec(local_aabb_min, &pv_local_aabb_min);
    Phys_Vec3ToNitrousVec(local_aabb_max, &pv_local_aabb_max);

    return phys_are_potentially_colliding(
        &this->m_local_query_trace_aabb_min,
        &this->m_local_query_trace_aabb_max,
        &this->m_local_query_trace_translation,
        &pv_local_aabb_min,
        &pv_local_aabb_max,
        &hit_time)
        && this->query_create_prolog(geom);
}

void gjk_physics_collision_visitor::query_create_epilog(gjk_base_t *gjk_geom)
{
    unsigned int geom_id; // eax
    int stype; // [esp-Ch] [ebp-90h]
    unsigned int env_collision_flags; // [esp-4h] [ebp-88h]
    BOOL v6; // [esp+0h] [ebp-84h]
    broad_phase_info *bpi_env; // [esp+80h] [ebp-4h]

    if (!gjk_geom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 307, 0, "%s", "gjk_geom"))
    {
        __debugbreak();
    }
    if (gjk_geom->stype > 0x22u
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            308,
            0,
            "%s",
            "gjk_geom->stype >= 0 && gjk_geom->stype < MAX_NUM_SURFACE_TYPES"))
    {
        __debugbreak();
    }
    if (!this->m_local_bpei
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp", 309, 0, "%s", "m_local_bpei"))
    {
        __debugbreak();
    }
    if (this->m_local_bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            310,
            0,
            "%s",
            "m_local_bpei->m_data == NULL"))
    {
        __debugbreak();
    }
    v6 = this->m_local_bpei->m_database_id.m_id1 == this->m_local_database_id.m_id1
        && this->m_local_bpei->m_database_id.m_id2 == this->m_local_database_id.m_id2;
    if (!v6
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            311,
            0,
            "%s",
            "m_local_bpei->m_database_id == m_local_database_id"))
    {
        __debugbreak();
    }
    //gjk_base_t::set_geom_id_new(gjk_geom, this->m_local_bpei->m_gjk_geom_id);
    gjk_geom->set_geom_id_new(this->m_local_bpei->m_gjk_geom_id);
    bpi_env = allocate_bpi_env();
    env_collision_flags = this->env_collision_flags;
    stype = gjk_geom->stype;
    //geom_id = gjk_base_t::get_geom_id(gjk_geom);
    geom_id = gjk_geom->get_geom_id();
    //broad_phase_info::set(
        bpi_env->set(
        this->rb,
        this->rb_to_world_xform,
        this->cg_to_world_xform,
        this->cg_to_rb_xform,
        gjk_geom,
        geom_id,
        0,
        stype,
        0,
        env_collision_flags);
    //broad_phase_info::set_bpi_env(bpi_env, this->auto_activate_callback);
    bpi_env->set_bpi_env(this->auto_activate_callback);
    //broad_phase_info::collision_prolog(bpi_env);
    bpi_env->collision_prolog();
    this->m_local_bpei->m_data = bpi_env;
    //minspec_mutex::Unlock(&this->m_local_bpei->m_mutex);
    this->m_local_bpei->m_mutex.Unlock();
    if (!this->m_local_bpei->m_data
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            321,
            0,
            "%s",
            "m_local_bpei->m_data"))
    {
        __debugbreak();
    }
    //broad_phase_environement_query_results::add(this->bpeqr, (broad_phase_base *)this->m_local_bpei->m_data);
    this->bpeqr->add((broad_phase_base *)this->m_local_bpei->m_data);
}

void __thiscall gjk_physics_collision_visitor::query_create_epilog_1(
    gjk_base_t *gjk_geom)
{
    this->query_create_epilog(gjk_geom);
}