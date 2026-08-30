#include "physics_system.h"
#include "physics_system_internal.h"
#include "phys_assert.h"
#include "phys_broad_phase.h"

phys_assert_info pai_create_rbc_contact = { 0, 2, true };


environment_rigid_body *__cdecl phys_sys::get_environment_rigid_body()
{
    return &g_physics_system->m_environment_rigid_body;
}

void __cdecl phys_sys::set_max_delta_t(float max_delta_t)
{
    g_physics_system->m_max_delta_t = max_delta_t;
}

double __cdecl phys_sys::get_max_delta_t()
{
    return g_physics_system->m_max_delta_t;
}

void __cdecl phys_sys::set_v_tol(int max_v_iters)
{
    g_physics_system->m_max_vel_iters = max_v_iters;
}

void __cdecl phys_sys::get_v_tol(int *max_v_iters)
{
    *max_v_iters = g_physics_system->m_max_vel_iters;
}

void __cdecl phys_sys::set_vp_tol(int max_vp_iters)
{
    g_physics_system->m_max_vel_pos_iters = max_vp_iters;
}

void __cdecl phys_sys::get_vp_tol(int *max_vp_iters)
{
    *max_vp_iters = g_physics_system->m_max_vel_pos_iters;
}

void __cdecl phys_sys::set_collision_callback(void (__cdecl *collision_callback)())
{
    g_physics_system->m_collision_callback = collision_callback;
}

void __cdecl phys_sys::phys_frame_advance(float delta_t)
{
    g_physics_system->frame_advance(delta_t);
}

// attributes: thunk
void __cdecl phys_sys::phys_init()
{
    physics_system::initialize();
}

// attributes: thunk
void __cdecl phys_sys::phys_shutdown()
{
    physics_system::shutdown();
}

int __cdecl phys_sys::get_rigid_body_alloc_count()
{
    return g_physics_system->m_list_rigid_body.m_list_count;
}

int __cdecl phys_sys::get_user_rigid_body_alloc_count()
{
    return g_physics_system->m_list_user_rigid_body.m_list_count;
}

int __cdecl phys_sys::get_rbc_point_alloc_count()
{
    return g_physics_system->m_list_rbc_point.m_list_count;
}

int __cdecl phys_sys::get_rbc_hinge_alloc_count()
{
    return g_physics_system->m_list_rbc_hinge.m_list_count;
}

int __cdecl phys_sys::get_rbc_dist_alloc_count()
{
    return g_physics_system->m_list_rbc_dist.m_list_count;
}

int __cdecl phys_sys::get_rbc_ragdoll_alloc_count()
{
    return g_physics_system->m_list_rbc_ragdoll.m_list_count;
}

int __cdecl phys_sys::get_rbc_wheel_alloc_count()
{
    return g_physics_system->m_list_rbc_wheel.m_list_count;
}

int __cdecl phys_sys::get_rbc_angular_actuator_alloc_count()
{
    return g_physics_system->m_list_rbc_angular_actuator.m_list_count;
}

int __cdecl phys_sys::get_rbc_upright_alloc_count()
{
    return g_physics_system->m_list_rbc_upright.m_list_count;
}

int __cdecl phys_sys::get_rbc_custom_orientation_alloc_count()
{
    return g_physics_system->m_list_rbc_custom_orientation.m_list_count;
}

int __cdecl phys_sys::get_rbc_custom_path_alloc_count()
{
    return g_physics_system->m_list_rbc_custom_path.m_list_count;
}

int __cdecl phys_sys::get_rbc_contact_alloc_count()
{
    return g_physics_system->m_list_rbc_contact.m_list_count;
}

user_rigid_body *phys_sys::get_user_rigid_body(const phys_mat44 *const dictactor)
{
    // this seems un-needed
    //if (g_physics_system->m_list_user_rigid_body.empty())
    //{
    //    return NULL;
    //}

    for (user_rigid_body *rb : g_physics_system->m_list_user_rigid_body)
    {
        iassert(rb->is_user_rigid_body());

        if (rb->m_dictator == dictactor)
        {
            return rb;
        }
    }

    return NULL;
}

rigid_body_constraint_contact *__cdecl phys_sys::get_rbc_contact(
                environment_rigid_body *b1,
                environment_rigid_body *b2)
{
    //const rigid_body_pair_key *v2; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);

    rigid_body_pair_key v4(b1, b2); // [esp+8h] [ebp-8h] BYREF

    //v2 = rigid_body_pair_key::rigid_body_pair_key(&v4, b1, b2);
    //return phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::find(&g_physics_system->m_search_tree_rbc_contact, v2);
    return g_physics_system->m_search_tree_rbc_contact.find(v4);
}

void __cdecl phys_sys::fixup_wheel_constraints(rigid_body *const rb)
{
    for (rigid_body_constraint_wheel *wci : g_physics_system->m_list_rbc_wheel)
    {
        if (wci->b2 == rb)
        {
            wci->set_no_collision();
        }
    }
}

void __cdecl phys_sys::update_constraint_infos()
{
    // This function is not used 
    iassert(0);
#if 0
    g_physics_system->m_environment_rigid_body.m_constraint_count = 0;
    g_physics_system->m_environment_rigid_body.m_contact_count = 0;

    for (user_rigid_body *urb : g_physics_system->m_list_user_rigid_body)
    {
        urb->m_constraint_count = 0;
        urb->m_contact_count = 0;
    }

    for (rigid_body *rb : g_physics_system->m_list_rigid_body)
    {
        rb->m_constraint_count = 0;
        rb->m_contact_count = 0;
    }

    for (rigid_body_constraint_point *rbcp : g_physics_system->m_list_rbc_point)
    {
        if (rbcp->b1)
            rbcp->b1->m_constraint_count++;

        if (rbcp->b2)
            rbcp->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_hinge *rbch : g_physics_system->m_list_rbc_hinge)
    {
        if (rbch->b1)
            rbch->b1->m_constraint_count++;

        if (rbch->b2)
            rbch->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_distance *rbcd : g_physics_system->m_list_rbc_dist)
    {
        if (rbcd->b1)
            rbcd->b1->m_constraint_count++;

        if (rbcd->b2)
            rbcd->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_ragdoll *rbcr : g_physics_system->m_list_rbc_ragdoll)
    {
        if (rbcr->b1)
            rbcr->b1->m_constraint_count++;

        if (rbcr->b2)
            rbcr->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_wheel *rbcw : g_physics_system->m_list_rbc_wheel)
    {
        if (rbcw->b1)
            rbcw->b1->m_constraint_count++;

        if (rbcw->b2)
            rbcw->b2->m_constraint_count++;

        if ((rbcw->m_wheel_flags & 1) != 0)
        {
            if (rbcw->b1)
                rbcw->b1->m_contact_count++;

            if (rbcw->b2)
                rbcw->b2->m_contact_count++;
        }
    }

    for (rigid_body_constraint_angular_actuator *rbcaa : g_physics_system->m_list_rbc_angular_actuator)
    {
        if (rbcaa->b1)
            rbcaa->b1->m_constraint_count++;

        if (rbcaa->b2)
            rbcaa->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_upright *rbcu : g_physics_system->m_list_rbc_upright)
    {
        if (rbcu->b1)
            rbcu->b1->m_constraint_count++;

        if (rbcu->b2)
            rbcu->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_custom_orientation *rbcco : g_physics_system->m_list_rbc_custom_orientation)
    {
        if (rbcco->b1)
            rbcco->b1->m_constraint_count++;

        if (rbcco->b2)
            rbcco->b2->m_constraint_count++;
    }

    for (rigid_body_constraint_custom_path *rbccp : g_physics_system->m_list_rbc_custom_path)
    {
        if (rbccp->b1)
            rbccp->b1->m_constraint_count++;

        if (rbccp->b2)
            rbccp->b2->m_constraint_count++;
    }

    phys_free_list<rigid_body_constraint_contact> *v43; // esi
    phys_free_list<rigid_body_constraint_contact> *i; // edi
    rigid_body_constraint_contact *b2; // eax
    int j; // edx
    rigid_body_constraint_contact *v47; // eax
    int k; // ecx
    int v49; // eax
    int v50; // eax
    v43 = (phys_free_list<rigid_body_constraint_contact> *)g_physics_system->m_list_rbc_contact.m_dummy_head.m_next_T_internal;
    for ( i = &g_physics_system->m_list_rbc_contact;
                i != v43;
                v43 = (phys_free_list<rigid_body_constraint_contact> *)v43->m_dummy_head.m_next_T_internal )
    {
        b2 = v43->m_ptr_list[1];
        for ( j = 0; b2; b2 = (rigid_body_constraint_contact *)b2[1].b2 )
            j += b2->m_avl_tree_node.m_balance;
        v47 = v43->m_ptr_list[2];
        for ( k = 0; v47; v47 = (rigid_body_constraint_contact *)v47[1].b2 )
            k += v47->m_avl_tree_node.m_balance;
        if ( k <= j )
            k = j;
        v49 = v43->m_list_count;
        if ( v49 )
            *(unsigned int *)(v49 + 268) += k;
        v50 = v43->m_list_count_high_water;
        if ( v50 )
            *(unsigned int *)(v50 + 268) += k;
    }
#endif
}

rigid_body *__cdecl phys_sys::create_rigid_body(int no_error)
{
    return g_physics_system->m_list_rigid_body.add(no_error, "POOL OUT OF MEMORY, rigid_body, INCREASE phys_mem_info::m_num_rigid_body.");
}

rigid_body_constraint_point *__cdecl phys_sys::create_rbc_point(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_point *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);

    result = g_physics_system->m_list_rbc_point.add(no_error, "OUT_OF_MEMORY, rbc_point, INCREASE phys_mem_info::m_num_rbc_point.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_custom_orientation *__cdecl phys_sys::create_rbc_custom_orientation(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_custom_orientation *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);

    result = g_physics_system->m_list_rbc_custom_orientation.add(no_error, "OUT_OF_MEMORY, rbc_custom_orientation, INCREASE phys_mem_info::m_num_rbc_custom_orientation.");

    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_contact *__cdecl phys_sys::create_rbc_contact(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    return create(
                     &g_physics_system->m_list_rbc_contact,
                     0,
                     b1,
                     b2,
                     no_error,
                     "OUT_OF_MEMORY, rbc_contact, INCREASE phys_mem_info::m_num_rbc_contact.");
}

void __cdecl phys_sys::destroy(rigid_body *const rb)
{
    phys_sys::destroy_all_constraint(rb);

    if (rb)
    {
        using TI = phys_free_list<rigid_body>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
        static_assert(sizeof(TI) == 0x180);
#endif
        TI *ti = (TI *)((char *)rb - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);

        g_physics_system->m_list_rigid_body.remove(ti);
    }
}
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(offsetof(phys_free_list<rigid_body>::T_internal, m_data) == 0x10, "offset mismatch");
#endif
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(phys_free_list<rigid_body>::T_internal) == 0x180, "size mismatch");
#endif

void __cdecl phys_sys::destroy(user_rigid_body *const rb)
{
    phys_sys::destroy_all_constraint(rb);

    if (rb)
    {
        using TI = phys_free_list<user_rigid_body>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
        static_assert(sizeof(TI) == 0x1D0);
#endif
        TI *ti = (TI *)((char *)rb - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);

        g_physics_system->m_list_user_rigid_body.remove(ti);
    }
}
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(offsetof(phys_free_list<user_rigid_body>::T_internal, m_data) == 0x10, "offset mismatch");
#endif
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(phys_free_list<user_rigid_body>::T_internal) == 0x1D0, "size mismatch");
#endif

void __cdecl phys_sys::destroy(rigid_body_constraint_contact *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_contact>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0x38);
#endif

    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 4u);
        g_physics_system->m_list_rbc_contact.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_point *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_point>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0x70);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_point.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_hinge *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_hinge>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0xF0);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_hinge.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_distance *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_distance>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0x80);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_dist.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_ragdoll *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_ragdoll>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0x140);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_ragdoll.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_wheel *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_wheel>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0xF0);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_wheel.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_angular_actuator *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_angular_actuator>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0xE0);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_angular_actuator.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_upright *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_upright>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0xD0);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_upright.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_custom_orientation *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_custom_orientation>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0x3C);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 4u);
        g_physics_system->m_list_rbc_custom_orientation.remove(ti);
    }
}

void __cdecl phys_sys::destroy(rigid_body_constraint_custom_path *const rbc)
{
    using TI = phys_free_list<rigid_body_constraint_custom_path>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0xA0);
#endif
    if (rbc)
    {
        TI *ti = (TI *)((char *)rbc - offsetof(TI, m_data));
        PMM_VALIDATE((char *)ti, sizeof(TI), 0x10u);
        g_physics_system->m_list_rbc_custom_path.remove(ti);
    }
}

void __cdecl phys_sys::destroy_all_rbc_point()
{
    g_physics_system->m_list_rbc_point.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_hinge()
{
    g_physics_system->m_list_rbc_hinge.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_dist()
{
    g_physics_system->m_list_rbc_dist.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_ragdoll()
{
    g_physics_system->m_list_rbc_ragdoll.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_wheel()
{
    g_physics_system->m_list_rbc_wheel.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_angular_actuator()
{
    g_physics_system->m_list_rbc_angular_actuator.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_upright()
{
    g_physics_system->m_list_rbc_upright.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_custom_orientation()
{
    g_physics_system->m_list_rbc_custom_orientation.remove_all();
}

void __cdecl phys_sys::destroy_all_rbc_custom_path()
{
    g_physics_system->m_list_rbc_custom_path.remove_all();
}

void __cdecl phys_sys::destroy_all_unused_user_rigid_body()
{
    // This function is not used
    iassert(0);
#if 0
    // this does not use range-based for because it's removing elements
    for (auto it = g_physics_system->m_list_user_rigid_body.begin(); it != g_physics_system->m_list_user_rigid_body.end();)
    {
        user_rigid_body *urb = *it;
        auto next = it; ++next;
        if (urb->m_constraint_count == 0)
            g_physics_system->m_list_user_rigid_body.remove(urb);
        it = next;
    }
#endif
}

user_rigid_body *__cdecl phys_sys::create_user_rigid_body(int no_error)
{
    return g_physics_system->m_list_user_rigid_body.add(no_error, "POOL OUT OF MEMORY, user_rigid_body, INCREASE phys_mem_info::m_num_user_rigid_body.");
}

rigid_body_constraint_hinge *__cdecl phys_sys::create_rbc_hinge(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_hinge *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);
    result = g_physics_system->m_list_rbc_hinge.add(no_error, "OUT_OF_MEMORY, rbc_hinge, INCREASE phys_mem_info::m_num_rbc_hinge.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_distance *__cdecl phys_sys::create_rbc_dist(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_distance *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);
    result = g_physics_system->m_list_rbc_dist.add(no_error, "OUT_OF_MEMORY, rbc_dist, INCREASE phys_mem_info::m_num_rbc_dist.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_ragdoll *__cdecl phys_sys::create_rbc_ragdoll(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_ragdoll *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);
    result = g_physics_system->m_list_rbc_ragdoll.add(no_error, "OUT_OF_MEMORY, rbc_ragdoll, INCREASE phys_mem_info::m_num_rbc_ragdoll.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_wheel *__cdecl phys_sys::create_rbc_wheel(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_wheel *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);

    result = g_physics_system->m_list_rbc_wheel.add(no_error, "OUT_OF_MEMORY, rbc_wheel, INCREASE phys_mem_info::m_num_rbc_wheel.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_angular_actuator *__cdecl phys_sys::create_rbc_angular_actuator(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_angular_actuator *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);
    result = g_physics_system->m_list_rbc_angular_actuator.add(no_error, "OUT_OF_MEMORY, rbc_angular_actuator, INCREASE phys_mem_info::m_num_rbc_angular_actuator.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_upright *__cdecl phys_sys::create_rbc_upright(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_upright *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);
    result = g_physics_system->m_list_rbc_upright.add(no_error, "OUT_OF_MEMORY, rbc_upright, INCREASE phys_mem_info::m_num_rbc_upright.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

rigid_body_constraint_custom_path *__cdecl phys_sys::create_rbc_custom_path(
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error)
{
    rigid_body_constraint_custom_path *result; // eax

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);
    result = g_physics_system->m_list_rbc_custom_path.add(no_error, "OUT_OF_MEMORY, rbc_custom_path, INCREASE phys_mem_info::m_num_rbc_custom_path.");
    if ( result )
    {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

void __cdecl phys_sys::destroy_all_rigid_body()
{
    g_physics_system->m_list_rbc_point.remove_all();
    g_physics_system->m_list_rbc_hinge.remove_all();
    g_physics_system->m_list_rbc_dist.remove_all();
    g_physics_system->m_list_rbc_ragdoll.remove_all();
    g_physics_system->m_list_rbc_wheel.remove_all();
    g_physics_system->m_list_rbc_angular_actuator.remove_all();
    g_physics_system->m_list_rbc_upright.remove_all();
    g_physics_system->m_list_rbc_custom_orientation.remove_all();
    g_physics_system->m_list_rbc_custom_path.remove_all();
    g_physics_system->m_list_rbc_contact.remove_all();
    g_physics_system->m_list_rigid_body.remove_all();
}

void __cdecl phys_sys::destroy_all_user_rigid_body()
{
    // This function is not used
    iassert(0);
#if 0
    physics_system *v0; // esi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *m_next_T_internal; // eax
    phys_free_list<rigid_body_constraint_point> *p_m_list_rbc_point; // edi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *m_prev_T_internal; // edx
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v4; // edx
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v5; // ebx
    phys_free_list<rigid_body_constraint_point>::T_internal *v6; // esi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v7; // eax
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v8; // ecx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v9; // ecx
    phys_free_list<rigid_body_constraint_wheel> *i; // edi
    rigid_body *b1; // edx
    rigid_body *b2; // edx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v13; // ebx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v14; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v15; // eax
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v16; // ecx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v17; // ecx
    phys_free_list<broad_phase_group> *j; // edi
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v19; // edx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v20; // edx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v21; // ebx
    phys_free_list<broad_phase_group>::T_internal *v22; // esi
    phys_free_list<broad_phase_group>::T_internal_base *v23; // eax
    phys_free_list<broad_phase_group>::T_internal_base *v24; // ecx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v25; // ecx
    phys_free_list<rigid_body_constraint_ragdoll> *k; // edi
    rigid_body *v27; // edx
    rigid_body *v28; // edx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v29; // ebx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v30; // esi
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v31; // eax
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v32; // ecx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v33; // ecx
    phys_free_list<rigid_body_constraint_wheel> *m; // edi
    rigid_body *v35; // edx
    rigid_body *v36; // edx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v37; // ebx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v38; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v39; // eax
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v40; // ecx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v41; // ecx
    phys_free_list<rigid_body_constraint_angular_actuator> *n; // edi
    rigid_body *v43; // edx
    rigid_body *v44; // edx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v45; // ebx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v46; // esi
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v47; // eax
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v48; // ecx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v49; // ecx
    phys_free_list<rigid_body_constraint_upright> *ii; // edi
    rigid_body *v51; // edx
    rigid_body *v52; // edx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v53; // ebx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v54; // esi
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v55; // eax
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v56; // ecx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v57; // ecx
    phys_free_list<rigid_body_constraint_custom_orientation> *jj; // edi
    rigid_body *v59; // edx
    rigid_body *v60; // edx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v61; // ebx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v62; // esi
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v63; // eax
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v64; // ecx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v65; // ecx
    phys_free_list<rigid_body_constraint_custom_path> *kk; // edi
    rigid_body *v67; // edx
    rigid_body *v68; // edx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v69; // ebx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v70; // esi
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v71; // eax
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v72; // ecx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v73; // ecx
    phys_free_list<rigid_body_constraint_contact> *mm; // edi
    rigid_body *v75; // edx
    rigid_body *v76; // edx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v77; // ebx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v78; // esi
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v79; // ecx
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v80; // eax

    v0 = g_physics_system;
    m_next_T_internal = g_physics_system->m_list_rbc_point.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_point = &g_physics_system->m_list_rbc_point;
    if ( &g_physics_system->m_list_rbc_point != (phys_free_list<rigid_body_constraint_point> *)m_next_T_internal )
    {
        do
        {
            m_prev_T_internal = m_next_T_internal[2].m_prev_T_internal;
            if ( m_prev_T_internal && ((int)m_prev_T_internal[31].m_next_T_internal & 0x20) != 0
                || (v4 = m_next_T_internal[2].m_next_T_internal) != 0 && ((int)v4[31].m_next_T_internal & 0x20) != 0 )
            {
                v5 = m_next_T_internal->m_next_T_internal;
                if ( m_next_T_internal != (phys_free_list<rigid_body_constraint_point>::T_internal_base *)-16 )
                {
                    v6 = (phys_free_list<rigid_body_constraint_point>::T_internal *)m_next_T_internal;
                    PMM_VALIDATE((char *)m_next_T_internal, 0x70u, 0x10u);
                    if ( !v6
                        && _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                 477,
                                 "data",
                                 "") )
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_point->m_list_count;
                    //phys_free_list<rigid_body_constraint_point>::debug_remove(p_m_list_rbc_point, v6);
                    p_m_list_rbc_point->debug_remove(v6);
                    v7 = v6->m_next_T_internal;
                    v8 = v6->m_prev_T_internal;
                    v8->m_next_T_internal = v7;
                    v7->m_prev_T_internal = v8;
                    PMM_FREE((unsigned __int8 *)v6, 0x70u, 0x10u);
                    v0 = g_physics_system;
                }
                m_next_T_internal = v5;
            }
            else
            {
                m_next_T_internal = m_next_T_internal->m_next_T_internal;
            }
            p_m_list_rbc_point = &v0->m_list_rbc_point;
        }
        while ( &v0->m_list_rbc_point != (phys_free_list<rigid_body_constraint_point> *)m_next_T_internal );
    }
    v9 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v0->m_list_rbc_hinge.m_dummy_head.m_next_T_internal;
    for ( i = (phys_free_list<rigid_body_constraint_wheel> *)&v0->m_list_rbc_hinge;
                &v0->m_list_rbc_hinge != (phys_free_list<rigid_body_constraint_hinge> *)v9;
                i = (phys_free_list<rigid_body_constraint_wheel> *)&v0->m_list_rbc_hinge )
    {
        b1 = v9->m_data.b1;
        if ( b1 && (b1->m_flags & 0x20) != 0 || (b2 = v9->m_data.b2) != 0 && (b2->m_flags & 0x20) != 0 )
        {
            v13 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v9->m_next_T_internal;
            if ( v9 != (phys_free_list<rigid_body_constraint_wheel>::T_internal *)-16 )
            {
                v14 = v9;
                PMM_VALIDATE((char *)v9, 0xF0u, 0x10u);
                if ( !v14
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --i->m_list_count;
                //phys_free_list<rigid_body_constraint_wheel>::debug_remove(i, v14);
                i->debug_remove(v14);
                v15 = v14->m_next_T_internal;
                v16 = v14->m_prev_T_internal;
                v16->m_next_T_internal = v15;
                v15->m_prev_T_internal = v16;
                PMM_FREE((unsigned __int8 *)v14, 0xF0u, 0x10u);
                v0 = g_physics_system;
            }
            v9 = v13;
        }
        else
        {
            v9 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v9->m_next_T_internal;
        }
    }
    v17 = v0->m_list_rbc_dist.m_dummy_head.m_next_T_internal;
    for ( j = (phys_free_list<broad_phase_group> *)&v0->m_list_rbc_dist;
                &v0->m_list_rbc_dist != (phys_free_list<rigid_body_constraint_distance> *)v17;
                j = (phys_free_list<broad_phase_group> *)&v0->m_list_rbc_dist )
    {
        v19 = v17[2].m_prev_T_internal;
        if ( v19 && ((int)v19[31].m_next_T_internal & 0x20) != 0
            || (v20 = v17[2].m_next_T_internal) != 0 && ((int)v20[31].m_next_T_internal & 0x20) != 0 )
        {
            v21 = v17->m_next_T_internal;
            if ( v17 != (phys_free_list<rigid_body_constraint_distance>::T_internal_base *)-16 )
            {
                v22 = (phys_free_list<broad_phase_group>::T_internal *)v17;
                PMM_VALIDATE((char *)v17, 0x80u, 0x10u);
                if ( !v22
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --j->m_list_count;
                //phys_free_list<broad_phase_group>::debug_remove(j, v22);
                j->debug_remove(v22);
                v23 = v22->m_next_T_internal;
                v24 = v22->m_prev_T_internal;
                v24->m_next_T_internal = v23;
                v23->m_prev_T_internal = v24;
                PMM_FREE((unsigned __int8 *)v22, 0x80u, 0x10u);
                v0 = g_physics_system;
            }
            v17 = v21;
        }
        else
        {
            v17 = v17->m_next_T_internal;
        }
    }
    v25 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v0->m_list_rbc_ragdoll.m_dummy_head.m_next_T_internal;
    for ( k = &v0->m_list_rbc_ragdoll;
                &v0->m_list_rbc_ragdoll != (phys_free_list<rigid_body_constraint_ragdoll> *)v25;
                k = &v0->m_list_rbc_ragdoll )
    {
        v27 = v25->m_data.b1;
        if ( v27 && (v27->m_flags & 0x20) != 0 || (v28 = v25->m_data.b2) != 0 && (v28->m_flags & 0x20) != 0 )
        {
            v29 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v25->m_next_T_internal;
            if ( v25 != (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)-16 )
            {
                v30 = v25;
                PMM_VALIDATE((char *)v25, 0x140u, 0x10u);
                if ( !v30
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --k->m_list_count;
                //phys_free_list<rigid_body_constraint_ragdoll>::debug_remove(k, v30);
                k->debug_remove(v30);
                v31 = v30->m_next_T_internal;
                v32 = v30->m_prev_T_internal;
                v32->m_next_T_internal = v31;
                v31->m_prev_T_internal = v32;
                PMM_FREE((unsigned __int8 *)v30, 0x140u, 0x10u);
                v0 = g_physics_system;
            }
            v25 = v29;
        }
        else
        {
            v25 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v25->m_next_T_internal;
        }
    }
    v33 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v0->m_list_rbc_wheel.m_dummy_head.m_next_T_internal;
    for ( m = &v0->m_list_rbc_wheel;
                &v0->m_list_rbc_wheel != (phys_free_list<rigid_body_constraint_wheel> *)v33;
                m = &v0->m_list_rbc_wheel )
    {
        v35 = v33->m_data.b1;
        if ( v35 && (v35->m_flags & 0x20) != 0 || (v36 = v33->m_data.b2) != 0 && (v36->m_flags & 0x20) != 0 )
        {
            v37 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v33->m_next_T_internal;
            if ( v33 != (phys_free_list<rigid_body_constraint_wheel>::T_internal *)-16 )
            {
                v38 = v33;
                PMM_VALIDATE((char *)v33, 0xF0u, 0x10u);
                if ( !v38
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --m->m_list_count;
                //phys_free_list<rigid_body_constraint_wheel>::debug_remove(m, v38);
                m->debug_remove(v38);
                v39 = v38->m_next_T_internal;
                v40 = v38->m_prev_T_internal;
                v40->m_next_T_internal = v39;
                v39->m_prev_T_internal = v40;
                PMM_FREE((unsigned __int8 *)v38, 0xF0u, 0x10u);
                v0 = g_physics_system;
            }
            v33 = v37;
        }
        else
        {
            v33 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v33->m_next_T_internal;
        }
    }
    v41 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v0->m_list_rbc_angular_actuator.m_dummy_head.m_next_T_internal;
    for ( n = &v0->m_list_rbc_angular_actuator;
                &v0->m_list_rbc_angular_actuator != (phys_free_list<rigid_body_constraint_angular_actuator> *)v41;
                n = &v0->m_list_rbc_angular_actuator )
    {
        v43 = v41->m_data.b1;
        if ( v43 && (v43->m_flags & 0x20) != 0 || (v44 = v41->m_data.b2) != 0 && (v44->m_flags & 0x20) != 0 )
        {
            v45 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v41->m_next_T_internal;
            if ( v41 != (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)-16 )
            {
                v46 = v41;
                PMM_VALIDATE((char *)v41, 0xE0u, 0x10u);
                if ( !v46
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --n->m_list_count;
                //phys_free_list<rigid_body_constraint_angular_actuator>::debug_remove(n, v46);
                n->debug_remove(v46);
                v47 = v46->m_next_T_internal;
                v48 = v46->m_prev_T_internal;
                v48->m_next_T_internal = v47;
                v47->m_prev_T_internal = v48;
                PMM_FREE((unsigned __int8 *)v46, 0xE0u, 0x10u);
                v0 = g_physics_system;
            }
            v41 = v45;
        }
        else
        {
            v41 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v41->m_next_T_internal;
        }
    }
    v49 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v0->m_list_rbc_upright.m_dummy_head.m_next_T_internal;
    for ( ii = &v0->m_list_rbc_upright;
                &v0->m_list_rbc_upright != (phys_free_list<rigid_body_constraint_upright> *)v49;
                ii = &v0->m_list_rbc_upright )
    {
        v51 = v49->m_data.b1;
        if ( v51 && (v51->m_flags & 0x20) != 0 || (v52 = v49->m_data.b2) != 0 && (v52->m_flags & 0x20) != 0 )
        {
            v53 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v49->m_next_T_internal;
            if ( v49 != (phys_free_list<rigid_body_constraint_upright>::T_internal *)-16 )
            {
                v54 = v49;
                PMM_VALIDATE((char *)v49, 0xD0u, 0x10u);
                if ( !v54
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --ii->m_list_count;
                //phys_free_list<rigid_body_constraint_upright>::debug_remove(ii, v54);
                ii->debug_remove(v54);
                v55 = v54->m_next_T_internal;
                v56 = v54->m_prev_T_internal;
                v56->m_next_T_internal = v55;
                v55->m_prev_T_internal = v56;
                PMM_FREE((unsigned __int8 *)v54, 0xD0u, 0x10u);
                v0 = g_physics_system;
            }
            v49 = v53;
        }
        else
        {
            v49 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v49->m_next_T_internal;
        }
    }
    v57 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v0->m_list_rbc_custom_orientation.m_dummy_head.m_next_T_internal;
    for ( jj = &v0->m_list_rbc_custom_orientation;
                &v0->m_list_rbc_custom_orientation != (phys_free_list<rigid_body_constraint_custom_orientation> *)v57;
                jj = &v0->m_list_rbc_custom_orientation )
    {
        v59 = v57->m_data.b1;
        if ( v59 && (v59->m_flags & 0x20) != 0 || (v60 = v57->m_data.b2) != 0 && (v60->m_flags & 0x20) != 0 )
        {
            v61 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v57->m_next_T_internal;
            if ( v57 != (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)-8 )
            {
                v62 = v57;
                PMM_VALIDATE((char *)v57, 0x3Cu, 4u);
                if ( !v62
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --jj->m_list_count;
                //phys_free_list<rigid_body_constraint_custom_orientation>::debug_remove(jj, v62);
                jj->debug_remove(v62);
                v63 = v62->m_next_T_internal;
                v64 = v62->m_prev_T_internal;
                v64->m_next_T_internal = v63;
                v63->m_prev_T_internal = v64;
                PMM_FREE((unsigned __int8 *)v62, 0x3Cu, 4u);
                v0 = g_physics_system;
            }
            v57 = v61;
        }
        else
        {
            v57 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v57->m_next_T_internal;
        }
    }
    v65 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v0->m_list_rbc_custom_path.m_dummy_head.m_next_T_internal;
    for ( kk = &v0->m_list_rbc_custom_path;
                &v0->m_list_rbc_custom_path != (phys_free_list<rigid_body_constraint_custom_path> *)v65;
                kk = &v0->m_list_rbc_custom_path )
    {
        v67 = v65->m_data.b1;
        if ( v67 && (v67->m_flags & 0x20) != 0 || (v68 = v65->m_data.b2) != 0 && (v68->m_flags & 0x20) != 0 )
        {
            v69 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v65->m_next_T_internal;
            if ( v65 != (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)-16 )
            {
                v70 = v65;
                PMM_VALIDATE((char *)v65, 0xA0u, 0x10u);
                if ( !v70
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --kk->m_list_count;
                //phys_free_list<rigid_body_constraint_custom_path>::debug_remove(kk, v70);
                kk->debug_remove(v70);
                v71 = v70->m_next_T_internal;
                v72 = v70->m_prev_T_internal;
                v72->m_next_T_internal = v71;
                v71->m_prev_T_internal = v72;
                PMM_FREE((unsigned __int8 *)v70, 0xA0u, 0x10u);
                v0 = g_physics_system;
            }
            v65 = v69;
        }
        else
        {
            v65 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v65->m_next_T_internal;
        }
    }
    v73 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v0->m_list_rbc_contact.m_dummy_head.m_next_T_internal;
    for ( mm = &v0->m_list_rbc_contact;
                &v0->m_list_rbc_contact != (phys_free_list<rigid_body_constraint_contact> *)v73;
                mm = &v0->m_list_rbc_contact )
    {
        v75 = v73->m_data.b1;
        if ( v75 && (v75->m_flags & 0x20) != 0 || (v76 = v73->m_data.b2) != 0 && (v76->m_flags & 0x20) != 0 )
        {
            v77 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v73->m_next_T_internal;
            if ( v73 != (phys_free_list<rigid_body_constraint_contact>::T_internal *)-8 )
            {
                v78 = v73;
                PMM_VALIDATE((char *)v73, 0x38u, 4u);
                if ( !v78
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --mm->m_list_count;
                //phys_free_list<rigid_body_constraint_contact>::debug_remove(mm, v78);
                mm->debug_remove(v78);
                v79 = v78->m_prev_T_internal;
                v80 = v78->m_next_T_internal;
                v79->m_next_T_internal = v80;
                v80->m_prev_T_internal = v79;
                //rigid_body_constraint_contact::~rigid_body_constraint_contact(&v78->m_data);
                v78->m_data.~rigid_body_constraint_contact();
                PMM_FREE((unsigned __int8 *)v78, 0x38u, 4u);
                v0 = g_physics_system;
            }
            v73 = v77;
        }
        else
        {
            v73 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v73->m_next_T_internal;
        }
    }
    //phys_free_list<user_rigid_body>::remove_all(&v0->m_list_user_rigid_body);
    v0->m_list_user_rigid_body.remove_all();
#endif
}

void __cdecl phys_sys::destroy_all_rbc_contact()
{
    g_physics_system->m_list_rbc_contact.remove_all();
}

void __cdecl phys_sys::destroy_all_constraint(rigid_body *const rb)
{
    rigid_body *v1; // esi
    physics_system *v2; // ebx
    phys_free_list<rigid_body_constraint_point>::T_internal_base *m_next_T_internal; // eax
    phys_free_list<rigid_body_constraint_point> *p_m_list_rbc_point; // edi
    rigid_body *m_prev_T_internal; // edx
    rigid_body *v6; // edx
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v7; // ebx
    phys_free_list<rigid_body_constraint_point>::T_internal *v8; // esi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v9; // eax
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v10; // ecx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v11; // ecx
    phys_free_list<rigid_body_constraint_wheel> *p_m_list_rbc_hinge; // edi
    rigid_body *b1; // edx
    rigid_body *b2; // edx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v15; // ebx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v16; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v17; // eax
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v18; // ecx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v19; // ecx
    phys_free_list<broad_phase_group> *i; // edi
    rigid_body *v21; // edx
    rigid_body *v22; // edx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v23; // ebx
    phys_free_list<broad_phase_group>::T_internal *v24; // esi
    phys_free_list<broad_phase_group>::T_internal_base *v25; // eax
    phys_free_list<broad_phase_group>::T_internal_base *v26; // ecx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v27; // ecx
    phys_free_list<rigid_body_constraint_ragdoll> *p_m_list_rbc_ragdoll; // edi
    rigid_body *v29; // edx
    rigid_body *v30; // edx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v31; // ebx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v32; // esi
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v33; // eax
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v34; // ecx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v35; // ecx
    phys_free_list<rigid_body_constraint_wheel> *j; // edi
    rigid_body *v37; // edx
    rigid_body *v38; // edx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v39; // ebx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v40; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v41; // eax
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v42; // ecx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v43; // ecx
    phys_free_list<rigid_body_constraint_angular_actuator> *p_m_list_rbc_angular_actuator; // edi
    rigid_body *v45; // edx
    rigid_body *v46; // edx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v47; // ebx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v48; // esi
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v49; // eax
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v50; // ecx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v51; // ecx
    phys_free_list<rigid_body_constraint_upright> *k; // edi
    rigid_body *v53; // edx
    rigid_body *v54; // edx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v55; // ebx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v56; // esi
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v57; // eax
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v58; // ecx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v59; // ecx
    phys_free_list<rigid_body_constraint_custom_orientation> *p_m_list_rbc_custom_orientation; // edi
    rigid_body *v61; // edx
    rigid_body *v62; // edx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v63; // ebx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v64; // esi
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v65; // eax
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v66; // ecx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v67; // ecx
    phys_free_list<rigid_body_constraint_custom_path> *m; // edi
    rigid_body *v69; // edx
    rigid_body *v70; // edx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v71; // ebx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v72; // esi
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v73; // eax
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v74; // ecx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v75; // ecx
    phys_free_list<rigid_body_constraint_contact> *p_m_list_rbc_contact; // edi
    rigid_body *v77; // edx
    rigid_body *v78; // edx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v79; // ebx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v80; // esi
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v81; // ecx
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v82; // eax

    v1 = rb;
    phys_sys::fixup_wheel_constraints(rb);
    v2 = g_physics_system;
    m_next_T_internal = g_physics_system->m_list_rbc_point.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_point = &g_physics_system->m_list_rbc_point;
    if ( &g_physics_system->m_list_rbc_point != (phys_free_list<rigid_body_constraint_point> *)m_next_T_internal )
    {
        do
        {
            m_prev_T_internal = (rigid_body *)m_next_T_internal[2].m_prev_T_internal;
            if ( m_prev_T_internal && m_prev_T_internal == v1
                || (v6 = (rigid_body *)m_next_T_internal[2].m_next_T_internal) != 0 && v6 == v1 )
            {
                v7 = m_next_T_internal->m_next_T_internal;
                if ( m_next_T_internal != (phys_free_list<rigid_body_constraint_point>::T_internal_base *)-16 )
                {
                    v8 = (phys_free_list<rigid_body_constraint_point>::T_internal *)m_next_T_internal;
                    PMM_VALIDATE((char *)m_next_T_internal, 0x70u, 0x10u);
                    if ( !v8
                        && _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                 477,
                                 "data",
                                 "") )
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_point->m_list_count;
                    //phys_free_list<rigid_body_constraint_point>::debug_remove(p_m_list_rbc_point, v8);
                    p_m_list_rbc_point->debug_remove(v8);
                    v9 = v8->m_next_T_internal;
                    v10 = v8->m_prev_T_internal;
                    v10->m_next_T_internal = v9;
                    v9->m_prev_T_internal = v10;
                    PMM_FREE((unsigned __int8 *)v8, 0x70u, 0x10u);
                    v1 = rb;
                }
                m_next_T_internal = v7;
                v2 = g_physics_system;
            }
            else
            {
                m_next_T_internal = m_next_T_internal->m_next_T_internal;
            }
            p_m_list_rbc_point = &v2->m_list_rbc_point;
        }
        while ( &v2->m_list_rbc_point != (phys_free_list<rigid_body_constraint_point> *)m_next_T_internal );
    }
    v11 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v2->m_list_rbc_hinge.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_hinge = (phys_free_list<rigid_body_constraint_wheel> *)&v2->m_list_rbc_hinge;
    if ( &v2->m_list_rbc_hinge != (phys_free_list<rigid_body_constraint_hinge> *)v11 )
    {
        do
        {
            b1 = v11->m_data.b1;
            if ( b1 && b1 == rb || (b2 = v11->m_data.b2) != 0 && b2 == rb )
            {
                v15 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v11->m_next_T_internal;
                if ( v11 != (phys_free_list<rigid_body_constraint_wheel>::T_internal *)-16 )
                {
                    v16 = v11;
                    PMM_VALIDATE((char *)v11, 0xF0u, 0x10u);
                    if ( !v16
                        && _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                 477,
                                 "data",
                                 "") )
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_hinge->m_list_count;
                    //phys_free_list<rigid_body_constraint_wheel>::debug_remove(p_m_list_rbc_hinge, v16);
                    p_m_list_rbc_hinge->debug_remove(v16);
                    v17 = v16->m_next_T_internal;
                    v18 = v16->m_prev_T_internal;
                    v18->m_next_T_internal = v17;
                    v17->m_prev_T_internal = v18;
                    PMM_FREE((unsigned __int8 *)v16, 0xF0u, 0x10u);
                }
                v11 = v15;
            }
            else
            {
                v11 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v11->m_next_T_internal;
            }
            p_m_list_rbc_hinge = (phys_free_list<rigid_body_constraint_wheel> *)&g_physics_system->m_list_rbc_hinge;
        }
        while ( &g_physics_system->m_list_rbc_hinge != (phys_free_list<rigid_body_constraint_hinge> *)v11 );
        v2 = g_physics_system;
    }
    v19 = v2->m_list_rbc_dist.m_dummy_head.m_next_T_internal;
    for ( i = (phys_free_list<broad_phase_group> *)&v2->m_list_rbc_dist;
                &v2->m_list_rbc_dist != (phys_free_list<rigid_body_constraint_distance> *)v19;
                i = (phys_free_list<broad_phase_group> *)&v2->m_list_rbc_dist )
    {
        v21 = (rigid_body *)v19[2].m_prev_T_internal;
        if ( v21 && v21 == rb || (v22 = (rigid_body *)v19[2].m_next_T_internal) != 0 && v22 == rb )
        {
            v23 = v19->m_next_T_internal;
            if ( v19 != (phys_free_list<rigid_body_constraint_distance>::T_internal_base *)-16 )
            {
                v24 = (phys_free_list<broad_phase_group>::T_internal *)v19;
                PMM_VALIDATE((char *)v19, 0x80u, 0x10u);
                if ( !v24
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --i->m_list_count;
                //phys_free_list<broad_phase_group>::debug_remove(i, v24);
                i->debug_remove(v24);
                v25 = v24->m_next_T_internal;
                v26 = v24->m_prev_T_internal;
                v26->m_next_T_internal = v25;
                v25->m_prev_T_internal = v26;
                PMM_FREE((unsigned __int8 *)v24, 0x80u, 0x10u);
            }
            v19 = v23;
            v2 = g_physics_system;
        }
        else
        {
            v19 = v19->m_next_T_internal;
        }
    }
    v27 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v2->m_list_rbc_ragdoll.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_ragdoll = &v2->m_list_rbc_ragdoll;
    if ( &v2->m_list_rbc_ragdoll != (phys_free_list<rigid_body_constraint_ragdoll> *)v27 )
    {
        do
        {
            v29 = v27->m_data.b1;
            if ( v29 && v29 == rb || (v30 = v27->m_data.b2) != 0 && v30 == rb )
            {
                v31 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v27->m_next_T_internal;
                if ( v27 != (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)-16 )
                {
                    v32 = v27;
                    PMM_VALIDATE((char *)v27, 0x140u, 0x10u);
                    if ( !v32
                        && _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                 477,
                                 "data",
                                 "") )
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_ragdoll->m_list_count;
                    //phys_free_list<rigid_body_constraint_ragdoll>::debug_remove(p_m_list_rbc_ragdoll, v32);
                    p_m_list_rbc_ragdoll->debug_remove(v32);
                    v33 = v32->m_next_T_internal;
                    v34 = v32->m_prev_T_internal;
                    v34->m_next_T_internal = v33;
                    v33->m_prev_T_internal = v34;
                    PMM_FREE((unsigned __int8 *)v32, 0x140u, 0x10u);
                }
                v27 = v31;
            }
            else
            {
                v27 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v27->m_next_T_internal;
            }
            p_m_list_rbc_ragdoll = &g_physics_system->m_list_rbc_ragdoll;
        }
        while ( &g_physics_system->m_list_rbc_ragdoll != (phys_free_list<rigid_body_constraint_ragdoll> *)v27 );
        v2 = g_physics_system;
    }
    v35 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v2->m_list_rbc_wheel.m_dummy_head.m_next_T_internal;
    for ( j = &v2->m_list_rbc_wheel;
                &v2->m_list_rbc_wheel != (phys_free_list<rigid_body_constraint_wheel> *)v35;
                j = &v2->m_list_rbc_wheel )
    {
        v37 = v35->m_data.b1;
        if ( v37 && v37 == rb || (v38 = v35->m_data.b2) != 0 && v38 == rb )
        {
            v39 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v35->m_next_T_internal;
            if ( v35 != (phys_free_list<rigid_body_constraint_wheel>::T_internal *)-16 )
            {
                v40 = v35;
                PMM_VALIDATE((char *)v35, 0xF0u, 0x10u);
                if ( !v40
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --j->m_list_count;
                //phys_free_list<rigid_body_constraint_wheel>::debug_remove(j, v40);
                j->debug_remove(v40);
                v41 = v40->m_next_T_internal;
                v42 = v40->m_prev_T_internal;
                v42->m_next_T_internal = v41;
                v41->m_prev_T_internal = v42;
                PMM_FREE((unsigned __int8 *)v40, 0xF0u, 0x10u);
            }
            v35 = v39;
            v2 = g_physics_system;
        }
        else
        {
            v35 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v35->m_next_T_internal;
        }
    }
    v43 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v2->m_list_rbc_angular_actuator.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_angular_actuator = &v2->m_list_rbc_angular_actuator;
    if ( &v2->m_list_rbc_angular_actuator != (phys_free_list<rigid_body_constraint_angular_actuator> *)v43 )
    {
        do
        {
            v45 = v43->m_data.b1;
            if ( v45 && v45 == rb || (v46 = v43->m_data.b2) != 0 && v46 == rb )
            {
                v47 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v43->m_next_T_internal;
                if ( v43 != (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)-16 )
                {
                    v48 = v43;
                    PMM_VALIDATE((char *)v43, 0xE0u, 0x10u);
                    if ( !v48
                        && _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                 477,
                                 "data",
                                 "") )
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_angular_actuator->m_list_count;
                    //phys_free_list<rigid_body_constraint_angular_actuator>::debug_remove(p_m_list_rbc_angular_actuator, v48);
                    p_m_list_rbc_angular_actuator->debug_remove(v48);
                    v49 = v48->m_next_T_internal;
                    v50 = v48->m_prev_T_internal;
                    v50->m_next_T_internal = v49;
                    v49->m_prev_T_internal = v50;
                    PMM_FREE((unsigned __int8 *)v48, 0xE0u, 0x10u);
                }
                v43 = v47;
            }
            else
            {
                v43 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v43->m_next_T_internal;
            }
            p_m_list_rbc_angular_actuator = &g_physics_system->m_list_rbc_angular_actuator;
        }
        while ( &g_physics_system->m_list_rbc_angular_actuator != (phys_free_list<rigid_body_constraint_angular_actuator> *)v43 );
        v2 = g_physics_system;
    }
    v51 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v2->m_list_rbc_upright.m_dummy_head.m_next_T_internal;
    for ( k = &v2->m_list_rbc_upright;
                &v2->m_list_rbc_upright != (phys_free_list<rigid_body_constraint_upright> *)v51;
                k = &v2->m_list_rbc_upright )
    {
        v53 = v51->m_data.b1;
        if ( v53 && v53 == rb || (v54 = v51->m_data.b2) != 0 && v54 == rb )
        {
            v55 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v51->m_next_T_internal;
            if ( v51 != (phys_free_list<rigid_body_constraint_upright>::T_internal *)-16 )
            {
                v56 = v51;
                PMM_VALIDATE((char *)v51, 0xD0u, 0x10u);
                if ( !v56
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --k->m_list_count;
                //phys_free_list<rigid_body_constraint_upright>::debug_remove(k, v56);
                k->debug_remove(v56);
                v57 = v56->m_next_T_internal;
                v58 = v56->m_prev_T_internal;
                v58->m_next_T_internal = v57;
                v57->m_prev_T_internal = v58;
                PMM_FREE((unsigned __int8 *)v56, 0xD0u, 0x10u);
            }
            v51 = v55;
            v2 = g_physics_system;
        }
        else
        {
            v51 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v51->m_next_T_internal;
        }
    }
    v59 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v2->m_list_rbc_custom_orientation.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_custom_orientation = &v2->m_list_rbc_custom_orientation;
    if ( &v2->m_list_rbc_custom_orientation != (phys_free_list<rigid_body_constraint_custom_orientation> *)v59 )
    {
        do
        {
            v61 = v59->m_data.b1;
            if ( v61 && v61 == rb || (v62 = v59->m_data.b2) != 0 && v62 == rb )
            {
                v63 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v59->m_next_T_internal;
                if ( v59 != (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)-8 )
                {
                    v64 = v59;
                    PMM_VALIDATE((char *)v59, 0x3Cu, 4u);
                    if ( !v64
                        && _tlAssert(
                                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                 477,
                                 "data",
                                 "") )
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_custom_orientation->m_list_count;
                    //phys_free_list<rigid_body_constraint_custom_orientation>::debug_remove(p_m_list_rbc_custom_orientation, v64);
                    p_m_list_rbc_custom_orientation->debug_remove(v64);
                    v65 = v64->m_next_T_internal;
                    v66 = v64->m_prev_T_internal;
                    v66->m_next_T_internal = v65;
                    v65->m_prev_T_internal = v66;
                    PMM_FREE((unsigned __int8 *)v64, 0x3Cu, 4u);
                }
                v59 = v63;
            }
            else
            {
                v59 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v59->m_next_T_internal;
            }
            p_m_list_rbc_custom_orientation = &g_physics_system->m_list_rbc_custom_orientation;
        }
        while ( &g_physics_system->m_list_rbc_custom_orientation != (phys_free_list<rigid_body_constraint_custom_orientation> *)v59 );
        v2 = g_physics_system;
    }
    v67 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v2->m_list_rbc_custom_path.m_dummy_head.m_next_T_internal;
    for ( m = &v2->m_list_rbc_custom_path;
                &v2->m_list_rbc_custom_path != (phys_free_list<rigid_body_constraint_custom_path> *)v67;
                m = &v2->m_list_rbc_custom_path )
    {
        v69 = v67->m_data.b1;
        if ( v69 && v69 == rb || (v70 = v67->m_data.b2) != 0 && v70 == rb )
        {
            v71 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v67->m_next_T_internal;
            if ( v67 != (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)-16 )
            {
                v72 = v67;
                PMM_VALIDATE((char *)v67, 0xA0u, 0x10u);
                if ( !v72
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
                {
                    __debugbreak();
                }
                --m->m_list_count;
                //phys_free_list<rigid_body_constraint_custom_path>::debug_remove(m, v72);
                m->debug_remove(v72);
                v73 = v72->m_next_T_internal;
                v74 = v72->m_prev_T_internal;
                v74->m_next_T_internal = v73;
                v73->m_prev_T_internal = v74;
                PMM_FREE((unsigned __int8 *)v72, 0xA0u, 0x10u);
            }
            v67 = v71;
            v2 = g_physics_system;
        }
        else
        {
            v67 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v67->m_next_T_internal;
        }
    }
    v75 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v2->m_list_rbc_contact.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_contact = &v2->m_list_rbc_contact;
    if ( &v2->m_list_rbc_contact != (phys_free_list<rigid_body_constraint_contact> *)v75 )
    {
        do
        {
            v77 = v75->m_data.b1;
            if ( v77 && v77 == rb || (v78 = v75->m_data.b2) != 0 && v78 == rb )
            {
                v79 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v75->m_next_T_internal;
                if ( v75 != (phys_free_list<rigid_body_constraint_contact>::T_internal *)-8 )
                {
                    v80 = v75;
                    PMM_VALIDATE((char *)v75, 0x38u, 4u);
                    if ( !v80 )
                    {
                        if ( _tlAssert(
                                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                                     477,
                                     "data",
                                     "") )
                        {
                            __debugbreak();
                        }
                    }
                    --p_m_list_rbc_contact->m_list_count;
                    //phys_free_list<rigid_body_constraint_contact>::debug_remove(p_m_list_rbc_contact, v80);
                    p_m_list_rbc_contact->debug_remove(v80);
                    v81 = v80->m_prev_T_internal;
                    v82 = v80->m_next_T_internal;
                    v81->m_next_T_internal = v82;
                    v82->m_prev_T_internal = v81;
                    //rigid_body_constraint_contact::~rigid_body_constraint_contact(&v80->m_data);
                    v80->m_data.~rigid_body_constraint_contact();
                    PMM_FREE((unsigned __int8 *)v80, 0x38u, 4u);
                }
                v75 = v79;
            }
            else
            {
                v75 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v75->m_next_T_internal;
            }
            p_m_list_rbc_contact = &g_physics_system->m_list_rbc_contact;
        }
        while ( &g_physics_system->m_list_rbc_contact != (phys_free_list<rigid_body_constraint_contact> *)v75 );
    }
}

void __cdecl phys_sys::destroy_all_constraint_with_user_rigid_body(rigid_body *const rb)
{
    // This function is not used
    iassert(0);
#if 0
    physics_system *v1; // ebx
    phys_free_list<rigid_body_constraint_point>::T_internal_base *m_next_T_internal; // eax
    phys_free_list<rigid_body_constraint_point> *p_m_list_rbc_point; // edi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *m_prev_T_internal; // esi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v5; // edx
    phys_free_list<rigid_body_constraint_point>::T_internal *v6; // esi
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v7; // eax
    phys_free_list<rigid_body_constraint_point>::T_internal_base *v8; // ecx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v9; // ecx
    phys_free_list<rigid_body_constraint_wheel> *p_m_list_rbc_hinge; // edi
    rigid_body *b1; // esi
    rigid_body *b2; // edx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v13; // ebx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v14; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v15; // eax
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v16; // ecx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v17; // ecx
    phys_free_list<broad_phase_group> *i; // edi
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v19; // esi
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v20; // edx
    phys_free_list<rigid_body_constraint_distance>::T_internal_base *v21; // ebx
    phys_free_list<broad_phase_group>::T_internal *v22; // esi
    phys_free_list<broad_phase_group>::T_internal_base *v23; // eax
    phys_free_list<broad_phase_group>::T_internal_base *v24; // ecx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v25; // ecx
    phys_free_list<rigid_body_constraint_ragdoll> *j; // edi
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v27; // esi
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v28; // edx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v29; // ebx
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal *v30; // esi
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v31; // eax
    phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *v32; // ecx
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v33; // ecx
    phys_free_list<rigid_body_constraint_wheel> *k; // edi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v35; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v36; // edx
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v37; // ebx
    phys_free_list<rigid_body_constraint_wheel>::T_internal *v38; // esi
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v39; // eax
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *v40; // ecx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v41; // ecx
    phys_free_list<rigid_body_constraint_angular_actuator> *m; // edi
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v43; // esi
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v44; // edx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v45; // ebx
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *v46; // esi
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v47; // eax
    phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *v48; // ecx
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v49; // ecx
    phys_free_list<rigid_body_constraint_upright> *n; // edi
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v51; // esi
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v52; // edx
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v53; // ebx
    phys_free_list<rigid_body_constraint_upright>::T_internal *v54; // esi
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v55; // eax
    phys_free_list<rigid_body_constraint_upright>::T_internal_base *v56; // ecx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v57; // ecx
    phys_free_list<rigid_body_constraint_custom_orientation> *ii; // edi
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v59; // esi
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v60; // edx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v61; // ebx
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *v62; // esi
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v63; // eax
    phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *v64; // ecx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v65; // ecx
    phys_free_list<rigid_body_constraint_custom_path> *jj; // edi
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v67; // esi
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v68; // edx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v69; // ebx
    phys_free_list<rigid_body_constraint_custom_path>::T_internal *v70; // esi
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v71; // eax
    phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *v72; // ecx
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v73; // ecx
    phys_free_list<rigid_body_constraint_contact> *kk; // edi
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v75; // esi
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v76; // edx
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v77; // ebx
    phys_free_list<rigid_body_constraint_contact>::T_internal *v78; // esi
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v79; // ecx
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *v80; // eax
    rigid_body *rba; // [esp+Ch] [ebp-4h]

    phys_sys::fixup_wheel_constraints(rb);
    v1 = g_physics_system;
    m_next_T_internal = g_physics_system->m_list_rbc_point.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_point = &g_physics_system->m_list_rbc_point;
    if (&g_physics_system->m_list_rbc_point != (phys_free_list<rigid_body_constraint_point> *)m_next_T_internal)
    {
        do
        {
            m_prev_T_internal = m_next_T_internal[2].m_prev_T_internal;
            if (m_prev_T_internal
                && (v5 = m_next_T_internal[2].m_next_T_internal) != 0
                && (m_prev_T_internal == (phys_free_list<rigid_body_constraint_point>::T_internal_base *)rb
                    && ((int)v5[31].m_next_T_internal & 0x20) != 0
                    || v5 == (phys_free_list<rigid_body_constraint_point>::T_internal_base *)rb
                    && ((int)m_prev_T_internal[31].m_next_T_internal & 0x20) != 0))
            {
                rba = (rigid_body *)m_next_T_internal->m_next_T_internal;
                if (m_next_T_internal != (phys_free_list<rigid_body_constraint_point>::T_internal_base *) - 16)
                {
                    v6 = (phys_free_list<rigid_body_constraint_point>::T_internal *)m_next_T_internal;
                    PMM_VALIDATE((char *)m_next_T_internal, 0x70u, 0x10u);
                    if (!v6
                        && _tlAssert(
                            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                            477,
                            "data",
                            ""))
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_point->m_list_count;
                    //phys_free_list<rigid_body_constraint_point>::debug_remove(p_m_list_rbc_point, v6);
                    p_m_list_rbc_point->debug_remove(v6);
                    v7 = v6->m_next_T_internal;
                    v8 = v6->m_prev_T_internal;
                    v8->m_next_T_internal = v7;
                    v7->m_prev_T_internal = v8;
                    PMM_FREE((unsigned __int8 *)v6, 0x70u, 0x10u);
                    v1 = g_physics_system;
                }
                m_next_T_internal = (phys_free_list<rigid_body_constraint_point>::T_internal_base *)rba;
            }
            else
            {
                m_next_T_internal = m_next_T_internal->m_next_T_internal;
            }
            p_m_list_rbc_point = &v1->m_list_rbc_point;
        } while (&v1->m_list_rbc_point != (phys_free_list<rigid_body_constraint_point> *)m_next_T_internal);
    }
    v9 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v1->m_list_rbc_hinge.m_dummy_head.m_next_T_internal;
    p_m_list_rbc_hinge = (phys_free_list<rigid_body_constraint_wheel> *) & v1->m_list_rbc_hinge;
    if (&v1->m_list_rbc_hinge != (phys_free_list<rigid_body_constraint_hinge> *)v9)
    {
        do
        {
            b1 = v9->m_data.b1;
            if (b1
                && (b2 = v9->m_data.b2) != 0
                && (b1 == rb && (b2->m_flags & 0x20) != 0 || b2 == rb && (b1->m_flags & 0x20) != 0))
            {
                v13 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v9->m_next_T_internal;
                if (v9 != (phys_free_list<rigid_body_constraint_wheel>::T_internal *) - 16)
                {
                    v14 = v9;
                    PMM_VALIDATE((char *)v9, 0xF0u, 0x10u);
                    if (!v14
                        && _tlAssert(
                            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                            477,
                            "data",
                            ""))
                    {
                        __debugbreak();
                    }
                    --p_m_list_rbc_hinge->m_list_count;
                    //phys_free_list<rigid_body_constraint_wheel>::debug_remove(p_m_list_rbc_hinge, v14);
                    p_m_list_rbc_hinge->debug_remove(v14);
                    v15 = v14->m_next_T_internal;
                    v16 = v14->m_prev_T_internal;
                    v16->m_next_T_internal = v15;
                    v15->m_prev_T_internal = v16;
                    PMM_FREE((unsigned __int8 *)v14, 0xF0u, 0x10u);
                }
                v9 = v13;
            }
            else
            {
                v9 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v9->m_next_T_internal;
            }
            p_m_list_rbc_hinge = (phys_free_list<rigid_body_constraint_wheel> *) & g_physics_system->m_list_rbc_hinge;
        } while (&g_physics_system->m_list_rbc_hinge != (phys_free_list<rigid_body_constraint_hinge> *)v9);
    }
    v17 = g_physics_system->m_list_rbc_dist.m_dummy_head.m_next_T_internal;
    for (i = (phys_free_list<broad_phase_group> *) & g_physics_system->m_list_rbc_dist;
        &g_physics_system->m_list_rbc_dist != (phys_free_list<rigid_body_constraint_distance> *)v17;
        i = (phys_free_list<broad_phase_group> *) & g_physics_system->m_list_rbc_dist)
    {
        v19 = v17[2].m_prev_T_internal;
        if (v19
            && (v20 = v17[2].m_next_T_internal) != 0
            && (v19 == (phys_free_list<rigid_body_constraint_distance>::T_internal_base *)rb
                && ((int)v20[31].m_next_T_internal & 0x20) != 0
                || v20 == (phys_free_list<rigid_body_constraint_distance>::T_internal_base *)rb
                && ((int)v19[31].m_next_T_internal & 0x20) != 0))
        {
            v21 = v17->m_next_T_internal;
            if (v17 != (phys_free_list<rigid_body_constraint_distance>::T_internal_base *) - 16)
            {
                v22 = (phys_free_list<broad_phase_group>::T_internal *)v17;
                PMM_VALIDATE((char *)v17, 0x80u, 0x10u);
                if (!v22
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --i->m_list_count;
                //phys_free_list<broad_phase_group>::debug_remove(i, v22);
                i->debug_remove(v22);
                v23 = v22->m_next_T_internal;
                v24 = v22->m_prev_T_internal;
                v24->m_next_T_internal = v23;
                v23->m_prev_T_internal = v24;
                PMM_FREE((unsigned __int8 *)v22, 0x80u, 0x10u);
            }
            v17 = v21;
        }
        else
        {
            v17 = v17->m_next_T_internal;
        }
    }
    v25 = g_physics_system->m_list_rbc_ragdoll.m_dummy_head.m_next_T_internal;
    for (j = &g_physics_system->m_list_rbc_ragdoll;
        &g_physics_system->m_list_rbc_ragdoll != (phys_free_list<rigid_body_constraint_ragdoll> *)v25;
        j = &g_physics_system->m_list_rbc_ragdoll)
    {
        v27 = v25[2].m_prev_T_internal;
        if (v27
            && (v28 = v25[2].m_next_T_internal) != 0
            && (v27 == (phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *)rb
                && ((int)v28[31].m_next_T_internal & 0x20) != 0
                || v28 == (phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *)rb
                && ((int)v27[31].m_next_T_internal & 0x20) != 0))
        {
            v29 = v25->m_next_T_internal;
            if (v25 != (phys_free_list<rigid_body_constraint_ragdoll>::T_internal_base *) - 16)
            {
                v30 = (phys_free_list<rigid_body_constraint_ragdoll>::T_internal *)v25;
                PMM_VALIDATE((char *)v25, 0x140u, 0x10u);
                if (!v30
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --j->m_list_count;
                //phys_free_list<rigid_body_constraint_ragdoll>::debug_remove(j, v30);
                j->debug_remove(v30);
                v31 = v30->m_next_T_internal;
                v32 = v30->m_prev_T_internal;
                v32->m_next_T_internal = v31;
                v31->m_prev_T_internal = v32;
                PMM_FREE((unsigned __int8 *)v30, 0x140u, 0x10u);
            }
            v25 = v29;
        }
        else
        {
            v25 = v25->m_next_T_internal;
        }
    }
    v33 = g_physics_system->m_list_rbc_wheel.m_dummy_head.m_next_T_internal;
    for (k = &g_physics_system->m_list_rbc_wheel;
        &g_physics_system->m_list_rbc_wheel != (phys_free_list<rigid_body_constraint_wheel> *)v33;
        k = &g_physics_system->m_list_rbc_wheel)
    {
        v35 = v33[2].m_prev_T_internal;
        if (v35
            && (v36 = v33[2].m_next_T_internal) != 0
            && (v35 == (phys_free_list<rigid_body_constraint_wheel>::T_internal_base *)rb
                && ((int)v36[31].m_next_T_internal & 0x20) != 0
                || v36 == (phys_free_list<rigid_body_constraint_wheel>::T_internal_base *)rb
                && ((int)v35[31].m_next_T_internal & 0x20) != 0))
        {
            v37 = v33->m_next_T_internal;
            if (v33 != (phys_free_list<rigid_body_constraint_wheel>::T_internal_base *) - 16)
            {
                v38 = (phys_free_list<rigid_body_constraint_wheel>::T_internal *)v33;
                PMM_VALIDATE((char *)v33, 0xF0u, 0x10u);
                if (!v38
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --k->m_list_count;
                //phys_free_list<rigid_body_constraint_wheel>::debug_remove(k, v38);
                k->debug_remove(v38);
                v39 = v38->m_next_T_internal;
                v40 = v38->m_prev_T_internal;
                v40->m_next_T_internal = v39;
                v39->m_prev_T_internal = v40;
                PMM_FREE((unsigned __int8 *)v38, 0xF0u, 0x10u);
            }
            v33 = v37;
        }
        else
        {
            v33 = v33->m_next_T_internal;
        }
    }
    v41 = g_physics_system->m_list_rbc_angular_actuator.m_dummy_head.m_next_T_internal;
    for (m = &g_physics_system->m_list_rbc_angular_actuator;
        &g_physics_system->m_list_rbc_angular_actuator != (phys_free_list<rigid_body_constraint_angular_actuator> *)v41;
        m = &g_physics_system->m_list_rbc_angular_actuator)
    {
        v43 = v41[2].m_prev_T_internal;
        if (v43
            && (v44 = v41[2].m_next_T_internal) != 0
            && (v43 == (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *)rb
                && ((int)v44[31].m_next_T_internal & 0x20) != 0
                || v44 == (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *)rb
                && ((int)v43[31].m_next_T_internal & 0x20) != 0))
        {
            v45 = v41->m_next_T_internal;
            if (v41 != (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal_base *) - 16)
            {
                v46 = (phys_free_list<rigid_body_constraint_angular_actuator>::T_internal *)v41;
                PMM_VALIDATE((char *)v41, 0xE0u, 0x10u);
                if (!v46
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --m->m_list_count;
                //phys_free_list<rigid_body_constraint_angular_actuator>::debug_remove(m, v46);
                m->debug_remove(v46);
                v47 = v46->m_next_T_internal;
                v48 = v46->m_prev_T_internal;
                v48->m_next_T_internal = v47;
                v47->m_prev_T_internal = v48;
                PMM_FREE((unsigned __int8 *)v46, 0xE0u, 0x10u);
            }
            v41 = v45;
        }
        else
        {
            v41 = v41->m_next_T_internal;
        }
    }
    v49 = g_physics_system->m_list_rbc_upright.m_dummy_head.m_next_T_internal;
    for (n = &g_physics_system->m_list_rbc_upright;
        &g_physics_system->m_list_rbc_upright != (phys_free_list<rigid_body_constraint_upright> *)v49;
        n = &g_physics_system->m_list_rbc_upright)
    {
        v51 = v49[2].m_prev_T_internal;
        if (v51
            && (v52 = v49[2].m_next_T_internal) != 0
            && (v51 == (phys_free_list<rigid_body_constraint_upright>::T_internal_base *)rb
                && ((int)v52[31].m_next_T_internal & 0x20) != 0
                || v52 == (phys_free_list<rigid_body_constraint_upright>::T_internal_base *)rb
                && ((int)v51[31].m_next_T_internal & 0x20) != 0))
        {
            v53 = v49->m_next_T_internal;
            if (v49 != (phys_free_list<rigid_body_constraint_upright>::T_internal_base *) - 16)
            {
                v54 = (phys_free_list<rigid_body_constraint_upright>::T_internal *)v49;
                PMM_VALIDATE((char *)v49, 0xD0u, 0x10u);
                if (!v54
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --n->m_list_count;
                //phys_free_list<rigid_body_constraint_upright>::debug_remove(n, v54);
                n->debug_remove(v54);
                v55 = v54->m_next_T_internal;
                v56 = v54->m_prev_T_internal;
                v56->m_next_T_internal = v55;
                v55->m_prev_T_internal = v56;
                PMM_FREE((unsigned __int8 *)v54, 0xD0u, 0x10u);
            }
            v49 = v53;
        }
        else
        {
            v49 = v49->m_next_T_internal;
        }
    }
    v57 = g_physics_system->m_list_rbc_custom_orientation.m_dummy_head.m_next_T_internal;
    for (ii = &g_physics_system->m_list_rbc_custom_orientation;
        &g_physics_system->m_list_rbc_custom_orientation != (phys_free_list<rigid_body_constraint_custom_orientation> *)v57;
        ii = &g_physics_system->m_list_rbc_custom_orientation)
    {
        v59 = v57[1].m_prev_T_internal;
        if (v59
            && (v60 = v57[1].m_next_T_internal) != 0
            && (v59 == (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *)rb
                && ((int)v60[31].m_next_T_internal & 0x20) != 0
                || v60 == (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *)rb
                && ((int)v59[31].m_next_T_internal & 0x20) != 0))
        {
            v61 = v57->m_next_T_internal;
            if (v57 != (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal_base *) - 8)
            {
                v62 = (phys_free_list<rigid_body_constraint_custom_orientation>::T_internal *)v57;
                PMM_VALIDATE((char *)v57, 0x3Cu, 4u);
                if (!v62
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --ii->m_list_count;
                //phys_free_list<rigid_body_constraint_custom_orientation>::debug_remove(ii, v62);
                ii->debug_remove(v62);
                v63 = v62->m_next_T_internal;
                v64 = v62->m_prev_T_internal;
                v64->m_next_T_internal = v63;
                v63->m_prev_T_internal = v64;
                PMM_FREE((unsigned __int8 *)v62, 0x3Cu, 4u);
            }
            v57 = v61;
        }
        else
        {
            v57 = v57->m_next_T_internal;
        }
    }
    v65 = g_physics_system->m_list_rbc_custom_path.m_dummy_head.m_next_T_internal;
    for (jj = &g_physics_system->m_list_rbc_custom_path;
        &g_physics_system->m_list_rbc_custom_path != (phys_free_list<rigid_body_constraint_custom_path> *)v65;
        jj = &g_physics_system->m_list_rbc_custom_path)
    {
        v67 = v65[2].m_prev_T_internal;
        if (v67
            && (v68 = v65[2].m_next_T_internal) != 0
            && (v67 == (phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *)rb
                && ((int)v68[31].m_next_T_internal & 0x20) != 0
                || v68 == (phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *)rb
                && ((int)v67[31].m_next_T_internal & 0x20) != 0))
        {
            v69 = v65->m_next_T_internal;
            if (v65 != (phys_free_list<rigid_body_constraint_custom_path>::T_internal_base *) - 16)
            {
                v70 = (phys_free_list<rigid_body_constraint_custom_path>::T_internal *)v65;
                PMM_VALIDATE((char *)v65, 0xA0u, 0x10u);
                if (!v70
                    && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", ""))
                {
                    __debugbreak();
                }
                --jj->m_list_count;
                //phys_free_list<rigid_body_constraint_custom_path>::debug_remove(jj, v70);
                jj->debug_remove(v70);
                v71 = v70->m_next_T_internal;
                v72 = v70->m_prev_T_internal;
                v72->m_next_T_internal = v71;
                v71->m_prev_T_internal = v72;
                PMM_FREE((unsigned __int8 *)v70, 0xA0u, 0x10u);
            }
            v65 = v69;
        }
        else
        {
            v65 = v65->m_next_T_internal;
        }
    }
    v73 = g_physics_system->m_list_rbc_contact.m_dummy_head.m_next_T_internal;
    for (kk = &g_physics_system->m_list_rbc_contact;
        &g_physics_system->m_list_rbc_contact != (phys_free_list<rigid_body_constraint_contact> *)v73;
        kk = &g_physics_system->m_list_rbc_contact)
    {
        v75 = v73[1].m_prev_T_internal;
        if (v75
            && (v76 = v73[1].m_next_T_internal) != 0
            && (v75 == (phys_free_list<rigid_body_constraint_contact>::T_internal_base *)rb
                && ((int)v76[31].m_next_T_internal & 0x20) != 0
                || v76 == (phys_free_list<rigid_body_constraint_contact>::T_internal_base *)rb
                && ((int)v75[31].m_next_T_internal & 0x20) != 0))
        {
            v77 = v73->m_next_T_internal;
            if (v73 != (phys_free_list<rigid_body_constraint_contact>::T_internal_base *) - 8)
            {
                v78 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)v73;
                PMM_VALIDATE((char *)v73, 0x38u, 4u);
                if (!v78)
                {
                    if (_tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                        477,
                        "data",
                        ""))
                    {
                        __debugbreak();
                    }
                }
                --kk->m_list_count;
                //phys_free_list<rigid_body_constraint_contact>::debug_remove(kk, v78);
                kk->debug_remove(v78);
                v79 = v78->m_prev_T_internal;
                v80 = v78->m_next_T_internal;
                v79->m_next_T_internal = v80;
                v80->m_prev_T_internal = v79;
                //rigid_body_constraint_contact::~rigid_body_constraint_contact(&v78->m_data);
                v78->m_data.~rigid_body_constraint_contact();
                PMM_FREE((unsigned __int8 *)v78, 0x38u, 4u);
            }
            v73 = v77;
        }
        else
        {
            v73 = v73->m_next_T_internal;
        }
    }
#endif
}

rigid_body_constraint_contact *__cdecl create(
                phys_free_list<rigid_body_constraint_contact> *__formal,
                rigid_body_constraint_contact *a2,
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error,
                const char *error_msg)
{
    physics_system *v6; // esi
    rigid_body_constraint_contact *result; // eax
    phys_free_list<rigid_body_constraint_contact> *p_m_list_rbc_contact; // esi
    phys_free_list<rigid_body_constraint_contact>::T_internal *v9; // eax
    rigid_body_constraint_contact *p_m_data; // edi

    g_physics_system->validate_member(b1);
    g_physics_system->validate_member(b2);

    rigid_body_pair_key key(b1, b2); // [esp+8h] [ebp-8h] BYREF
    //rigid_body_pair_key::rigid_body_pair_key(&key, b1, b2);
    v6 = g_physics_system;
    //result = phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::find(&g_physics_system->m_search_tree_rbc_contact, &key);
    result = g_physics_system->m_search_tree_rbc_contact.find(key);
    if ( result )
        return result;
    p_m_list_rbc_contact = &v6->m_list_rbc_contact;
    v9 = (phys_free_list<rigid_body_constraint_contact>::T_internal *)PMM_ALLOC(0x38u, 4u);
    if ( v9 )
    {
        v9->m_data.m_list_contact_point_info_buffer_1.m_first = 0;
        v9->m_data.m_list_contact_point_info_buffer_2.m_first = 0;
        p_m_data = &v9->m_data;
        v9->m_data.m_solver_priority = 0;
        v9->m_prev_T_internal = &p_m_list_rbc_contact->m_dummy_head;
        v9->m_next_T_internal = p_m_list_rbc_contact->m_dummy_head.m_next_T_internal;
        p_m_list_rbc_contact->m_dummy_head.m_next_T_internal->m_prev_T_internal = v9;
        ++p_m_list_rbc_contact->m_list_count;
        p_m_list_rbc_contact->m_dummy_head.m_next_T_internal = v9;
        //phys_free_list<rigid_body_constraint_contact>::debug_add(p_m_list_rbc_contact, v9);
        p_m_list_rbc_contact->debug_add(v9);
        if ( p_m_data )
        {
            p_m_data->b1 = b1;
            p_m_data->b2 = b2;
            //phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::add(&g_physics_system->m_search_tree_rbc_contact, &key, p_m_data);
            g_physics_system->m_search_tree_rbc_contact.add(key, p_m_data);
            return p_m_data;
        }
    }
    else
    {
        p_m_data = 0;
    }
    if ( no_error )
        return p_m_data;
    if ( (pai_create_rbc_contact.m_hits_total_count < pai_create_rbc_contact.m_max_hits_total
         || !pai_create_rbc_contact.m_max_hits_total)
        && pai_create_rbc_contact.m_hits_frame_count < pai_create_rbc_contact.m_max_hits_per_frame )
    {
        if ( pai_create_rbc_contact.m_use_warnings_only )
        {
            PHYS_WARNING("source/physics_system.cpp", 92, "rbc || no_error", error_msg);
        }
        else if ( _tlAssert("source/physics_system.cpp", 92, "rbc || no_error", error_msg) )
        {
            __debugbreak();
        }
    }
    _InterlockedExchangeAdd(&pai_create_rbc_contact.m_hits_total_count, 1u);
    _InterlockedExchangeAdd(&pai_create_rbc_contact.m_hits_frame_count, 1u);
    return p_m_data;
}