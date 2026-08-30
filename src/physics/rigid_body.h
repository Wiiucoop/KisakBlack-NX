#pragma once

#include "phys_local.h"
#include "phys_collision.h"
#include "phys_transient_allocator.h"

struct pulse_sum_cache // sizeof=0x4
{                                       // XREF: rigid_body_constraint_point/r
    float m_pulse_sum;
};

struct rigid_body_constraint // sizeof=0xC
{                                                                             // XREF: rigid_body_constraint_point/r
        struct rigid_body *b1;
        struct rigid_body *b2;
        struct rigid_body_constraint *m_next;
};

struct    rigid_body_constraint_point : rigid_body_constraint // sizeof=0x50
{                                                                             // XREF: phys_free_list<rigid_body_constraint_point>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_vec3 m_b1_r_loc;
        phys_vec3 m_b2_r_loc;
        pulse_sum_cache m_ps_cache_list[3];
        float m_stress;
        float m_spring_k;
        float m_damp_k;
        bool m_spring_enabled;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        void set(const phys_vec3 *b1_r_loc, const phys_vec3 *b2_r_loc);
        void epilog_vel_constaint(float __formal);
        void setup_constraint(struct pulse_sum_constraint_solver *phys, float delta_t);
};

struct    __declspec(align(16)) rigid_body_constraint_hinge : rigid_body_constraint // sizeof=0xD0
{                                                                             // XREF: phys_free_list<rigid_body_constraint_hinge>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_vec3 m_b1_r_loc;
        phys_vec3 m_b2_r_loc;
        phys_vec3 m_b1_axis_loc;
        phys_vec3 m_b2_axis_loc;
        phys_vec3 m_b1_a1_loc;
        phys_vec3 m_b1_a2_loc;
        phys_vec3 m_b1_ref_loc;
        phys_vec3 m_b2_ref_min_loc;
        phys_vec3 m_b2_ref_max_loc;
        float m_damp_k;
        unsigned int m_flags;
        pulse_sum_cache m_ps_cache[8];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        void set(
            const phys_vec3 *b1_r_loc,
            const phys_vec3 *b2_r_loc,
            const phys_vec3 *b1_axis_loc,
            const phys_vec3 *b2_axis_loc,
            const phys_vec3 *b1_ref_loc,
            const phys_vec3 *b2_ref_loc,
            float theta_min,
            float theta_max,
            float damp_k);

        void setup_constraint(
            pulse_sum_constraint_solver *psys,
            float delta_t);
};

// aislop used to gen methods
struct __declspec(align(16)) rigid_body_constraint_distance : rigid_body_constraint
{
    phys_vec3 m_b1_r_loc;
    phys_vec3 m_b2_r_loc;

    float m_min_distance;
    float m_max_distance;
    float m_next_max_distance;
    float m_max_distance_vel;
    float m_damp_coef;

    unsigned int m_flags;

    pulse_sum_cache m_ps_cache_list[3];

public:
    void set(
        const phys_vec3 *b1_r_loc,
        const phys_vec3 *b2_r_loc,
        float min_distance,
        float max_distance);

    /* ============================================================
       Distance Update Phases
    ============================================================ */

    void outer_prolog_update(float delta_t);
    void inner_update(float delta_t);
    void outer_epilog_update(float delta_t);

    /* ============================================================
       Constraint Setup (cleaned, but logically identical)
    ============================================================ */

    void setup_constraint(struct pulse_sum_constraint_solver *psys, float delta_t);
};


struct    __declspec(align(8)) ragdoll_joint_limit_info // sizeof=0x20
{                                                                             // XREF: rigid_body_constraint_ragdoll/r
        phys_vec3 m_b1_ud_loc;
        float m_b1_ud_limit_co_;
        float m_b1_ud_limit_si_;
        float m_b1_ud_active_limit_co_;
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        void set_b1_ud_loc(const phys_vec3 *b1_ud_loc);
        void set(const phys_vec3 *b1_ud_loc, float theta_limit);
        void set_theta_limit(float theta_limit);
};

struct    __declspec(align(16)) rigid_body_constraint_ragdoll : rigid_body_constraint // sizeof=0x120
{                                                                             // XREF: phys_free_list<rigid_body_constraint_ragdoll>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_vec3 m_b1_r_loc;
        phys_vec3 m_b2_r_loc;
        unsigned int m_flags;
        pulse_sum_cache m_ps_cache_list[10];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_vec3 m_b1_axis_loc;
        phys_vec3 m_b2_axis_loc;
        phys_vec3 m_b1_a1_loc;
        phys_vec3 m_b1_a2_loc;
        phys_vec3 m_b1_ref_loc;
        phys_vec3 m_b2_ref_min_loc;
        phys_vec3 m_b2_ref_max_loc;
        ragdoll_joint_limit_info m_joint_limits[2];
        int m_joint_limits_count;
        float m_damp_k;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        void set(const phys_vec3 *b1_r_loc, const phys_vec3 *b2_r_loc);
        void set_snider_style(const phys_vec3 *b1_axis_loc, const phys_vec3 *b1_ref_loc);
        void set_damp_k(float damp_k);
        void set_theta_min_max(const phys_vec3 *b2_ref_loc, float theta_min, float theta_max);
        void set_hinge(const phys_vec3 *b1_axis_loc, const phys_vec3 *b2_axis_loc, const phys_vec3 *b1_ref_loc, const phys_vec3 *b2_ref_loc, float theta_min, float theta_max);
        void set_swivel(const phys_vec3 *b1_axis_loc, const phys_vec3 *b2_axis_loc, const phys_vec3 *b1_ref_loc, const phys_vec3 *b2_ref_loc, float theta_min, float theta_max);
        void add_joint_limit(const phys_vec3 *b1_ud_loc, float theta_limit);
        float pull_together();
        void setup_hinge(struct pulse_sum_constraint_solver *psys,
            const phys_vec3 *b1_ref,
            const phys_vec3 *b2_axis,
            float delta_t);
        void setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
};

struct    rigid_body_constraint_wheel : rigid_body_constraint // sizeof=0xD0
{                                                                             // XREF: phys_free_list<rigid_body_constraint_wheel>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_vec3 m_b2_hitp_loc;
        phys_vec3 m_b2_hitn_loc;
        phys_vec3 m_b1_wheel_center_loc;
        phys_vec3 m_b1_suspension_dir_loc;
        phys_vec3 m_b1_wheel_axis_loc;
        float m_wheel_radius;
        float m_fwd_fric_k;
        float m_side_fric_k;
        float m_side_fric_max;
        float m_suspension_stiffness_k;
        float m_suspension_damp_k;
        float m_hard_limit_dist;
        float m_roll_stability_factor;
        float m_pitch_stability_factor;
        float m_turning_radius_ratio_max_speed;
        float m_turning_radius_ratio_accel;
        float m_desired_speed_k;
        float m_acceleration_factor_k;
        float m_braking_factor_k;
        float m_wheel_vel;
        float m_wheel_fwd;
        float m_wheel_pos;
        float m_wheel_displaced_center_dist;
        float m_wheel_normal_force;
        unsigned int m_wheel_state;
        unsigned int m_wheel_flags;
        pulse_sum_cache m_ps_cache_list[4];
        struct pulse_sum_normal *m_ps_suspension;
        struct pulse_sum_normal *m_ps_side_fric;
        struct pulse_sum_normal *m_ps_fwd_fric;

        void set_wheel_state_accelerating(float desired_speed_k, float acceleration_factor_k);
        void get_wheel_state_accelerating(float *desired_speed_k, float *acceleration_factor_k);
        void set_wheel_state_braking(float braking_factor_k);
        void set_no_collision();
        void set_collision(
            rigid_body *const rb,
            const phys_vec3 *hitp_loc,
            const phys_vec3 *hitn_loc);
        void set(
            const phys_vec3 *wheel_center_loc,
            const phys_vec3 *suspension_dir_loc,
            const phys_vec3 *wheel_axis_loc,
            float wheel_radius,
            float fwd_fric_k,
            float side_fric_k,
            float suspension_stiffness_k,
            float suspension_damp_k,
            float hard_limit_dist,
            float roll_stability_factor,
            float pitch_stability_factor,
            float side_fric_max);
        void get_wheel_collide_segment(
            const phys_mat44 *b1_mat,
            phys_vec3 *const p0,
            phys_vec3 *const p1);
        void epilog_vel_constraint(float delta_t);
        void setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
};

struct    __declspec(align(16)) rigid_body_constraint_angular_actuator : rigid_body_constraint // sizeof=0xC0
{                                                                             // XREF: phys_free_list<rigid_body_constraint_angular_actuator>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_mat44 m_target_mat;
        phys_mat44 m_next_target_mat;
        phys_vec3 m_a_vel;
        float m_power;
        float m_power_scale;
        bool m_enabled;
        // padding byte
        // padding byte
        // padding byte
        pulse_sum_cache m_ps_cache_list[3];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        void setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
        void set(float power, const phys_mat44 *target_mat);
        void outer_prolog_update(float delta_t);
        void outer_epilog_update(const float __formal);
        void inner_update(float delta_t);

};

struct    __declspec(align(16)) rigid_body_constraint_upright : rigid_body_constraint // sizeof=0xB0
{                                                                             // XREF: phys_free_list<rigid_body_constraint_upright>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_vec3 m_b1_forward_axis_loc;
        phys_vec3 m_b1_right_axis_loc;
        phys_vec3 m_b1_up_axis_loc;
        phys_vec3 m_b1_lean_axis_loc;
        phys_vec3 m_b2_up_axis_loc;
        phys_vec3 m_last_t_vel;
        phys_vec3 m_last_a_vel;
        float m_avg_side_force;
        float m_avg_normal_force;
        float m_lean_angle_calc_delta_t;
        float m_lean_angle;
        float m_lean_angle_multiplier;
        float m_max_lean_angle;
        float m_moving_average_total_time;
        bool m_enabled;
        // padding byte
        // padding byte
        // padding byte
        pulse_sum_cache m_ps_cache_list[1];
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

        const phys_vec3 *__thiscall calc_b1_lean_axis_loc(phys_vec3 *result, float lean_angle);
        double calc_current_lean_angle();
        void epilog_vel_constraint(float delta_t);
        void setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
        void update_lean_axis(const phys_vec3 *b1_lean_center, const phys_vec3 *b1_lean_axis_loc);
};

struct    rigid_body_constraint_custom_orientation : rigid_body_constraint // sizeof=0x30
{                                                                             // XREF: phys_free_list<rigid_body_constraint_custom_orientation>::T_internal/r
        pulse_sum_cache m_ps_cache_list[5];
        bool m_active;
        bool m_no_orientation_correction;
        // padding byte
        // padding byte
        float m_torque_resistance_pitch_roll;
        float m_torque_resistance_yaw;
        float m_upright_strength;

        void setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
};

struct rb_inplace_partition_node // sizeof=0x38
{                                                                             // XREF: rigid_body/r
    struct rigid_body_constraint_point *m_rbc_point_first;
    struct rigid_body_constraint_hinge *m_rbc_hinge_first;
    struct rigid_body_constraint_distance *m_rbc_dist_first;
    struct rigid_body_constraint_ragdoll *m_rbc_ragdoll_first;
    struct rigid_body_constraint_wheel *m_rbc_wheel_first;
    struct rigid_body_constraint_angular_actuator *m_rbc_angular_actuator_first;
    struct rigid_body_constraint_upright *m_rbc_upright_first;
    struct rigid_body_constraint_custom_orientation *m_rbc_custom_orientation_first;
    struct rigid_body_constraint_custom_path *m_rbc_custom_path_first;
    struct rigid_body_constraint_contact *m_rbc_contact_first;
    struct rigid_body *m_partition_head;
    struct rigid_body *m_partition_tail;
    struct rigid_body *m_next_node;
    int m_partition_size;
};

class rigid_body
{
public:
    void add_force(const phys_vec3 *force);
    void add_force(
        const phys_vec3 *force,
        const phys_vec3 *point,
        float torque_mult);
    void add_torque(const phys_vec3 *torque);

    void set_inertia(const phys_vec3 *inertia);
    void set_mass(float mass);

    void set(
        float mass,
        const phys_vec3 *inertia,
        const phys_mat44 *mat,
        const phys_vec3 *t_vel,
        const phys_vec3 *a_vel,
        int stable_min_contact_count);

    unsigned int get_flag(unsigned int f);
    void set_flag(unsigned int f, int b);
    unsigned int is_group_stable();

    unsigned int is_user_rigid_body();

    unsigned int is_dangerous();
    unsigned int is_stable();

    void dangerous_set_a_vel(const phys_vec3 *a_vel);
    void dangerous_set_t_vel(const phys_vec3 *t_vel);

    rigid_body &operator=(const rigid_body *__that);

    void swap_last_position();
    void update_last_position();

    void adjust_col_moved_vec(float lambda);

    inline unsigned int is_environment_rigid_body()
    {
        return this->m_flags & 0x10;
    }

    phys_vec3 m_last_position;
    phys_vec3 m_moved_vec;
    float m_smallest_lambda;
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
    phys_mat44 m_mat;
    phys_vec3 m_inv_inertia;
    phys_vec3 m_gravity_acc_vec;
    phys_vec3 m_t_vel;
    phys_vec3 m_a_vel;
    phys_vec3 m_last_t_vel;
    phys_vec3 m_last_a_vel;
    phys_vec3 m_force_sum;
    phys_vec3 m_torque_sum;
    float m_inv_mass;
    float m_max_avel;
    float m_max_delta_t;
    unsigned int m_flags;
    unsigned int m_tick;
    struct pulse_sum_node *m_node;
    int m_constraint_count;
    int m_contact_count;
    int m_stable_min_contact_count;
    float m_stable_energy_time;
    float m_largest_vel_sq;
    float m_t_drag_coef;
    float m_a_drag_coef;
    void *m_userdata;
    rb_inplace_partition_node m_partition_node;
};


struct    user_rigid_body : rigid_body // sizeof=0x1B0
{                                                                             // XREF: phys_free_list<user_rigid_body>::T_internal/r
                                                                                // pulse_sum_constraint_solver::temp_user_rigid_body/r
        const phys_mat44 *m_dictator;
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
        phys_mat44 m_dictator_mat;

        user_rigid_body &operator=(const user_rigid_body *__that);

        void set(const phys_mat44 *const dictator);
        void setPosition(const phys_mat44 *const dictator);
};

struct    __declspec(align(16)) rigid_body_constraint_custom_path : rigid_body_constraint // sizeof=0x80
{                                                                             // XREF: phys_free_list<rigid_body_constraint_custom_path>::T_internal/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        phys_mat44 m_path_mat;
        phys_vec3 b1_r_loc;
        user_rigid_body *m_urb;
        pulse_sum_cache m_list_psc[4];
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

        void setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
};

struct rigid_body_pair_key // sizeof=0x8
{                                                                             // XREF: rigid_body_constraint_contact/r
    rigid_body_pair_key(rigid_body *const b1, rigid_body *const b2)
    {
        iassert(b1 != b2);
        iassert(b1);
        iassert(b2);

        if (b1 >= b2)
        {
            this->m_b2 = b1;
            this->m_b1 = b2;
        }
        this->m_b1 = b1;
        this->m_b2 = b2;
    }

    bool operator<(const rigid_body_pair_key &other) const
    {
        if (m_b1 < other.m_b1) return true;
        if (other.m_b1 < m_b1) return false;
        return m_b2 < other.m_b2;
    }

    bool operator==(const rigid_body_pair_key &other) const
    {
        return m_b1 == other.m_b1 &&
            m_b2 == other.m_b2;
    }

    struct rigid_body *m_b1;
    struct rigid_body *m_b2;
};

struct broad_phase_info;
struct phys_collision_pair : phys_link_list_base<phys_collision_pair> // sizeof=0x14
{                                                                             // XREF: ?do_initial_tunnel_test@@YAXPAVbroad_phase_group@@ABVbroad_phase_environement_query_results@@@Z/r
    broad_phase_info *m_bpi1;
    broad_phase_info *m_bpi2;
    float m_hit_time;
    struct phys_gjk_cache_info *m_gjk_ci;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(phys_collision_pair) == 20);
#endif

class __declspec(align(16)) contact_point_info // sizeof=0x50
{
    struct pulse_sum_cache_info // sizeof=0xC
    {
        pulse_sum_cache m_ps_cache_list[3];
    };
public:
    phys_vec3 m_normal;
    float m_fric_coef;
    float m_bounce_coef;
    float m_max_restitution_vel;
    int m_flags;
    int m_point_pair_count;
    phys_vec3 *m_list_b1_r_loc;
    phys_vec3 *m_list_b2_r_loc;
    contact_point_info::pulse_sum_cache_info *m_list_pulse_sum_cache_info;
    contact_point_info *m_next_link;
    const void *m_rb2_entity;
    float m_translation_lambda;
    phys_collision_pair *m_pcp;
    struct rigid_body_constraint_contact *m_rbc_contact;
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

    void __thiscall set_solver_priority(unsigned int priority);
    void __thiscall set_rb2_entity(const void *rb2_entity);
    static void __cdecl set_closest_cached_psc(contact_point_info *cached_cpi, const phys_vec3 *normal, const phys_vec3 *b1_r_loc, const phys_vec3 *b2_r_loc, contact_point_info::pulse_sum_cache_info *psc);
    void __thiscall set_closest_cached_psc(contact_point_info *cached_cpi);
    static phys_transient_allocator *__cdecl get_cpi_allocator();
    void __thiscall get_closest_psc(const phys_vec3 *normal, const phys_vec3 *b1_r_loc, const phys_vec3 *b2_r_loc, float *closest_error, const contact_point_info::pulse_sum_cache_info **closest_psc);
    static contact_point_info *__cdecl create_cpi(int point_pair_count, bool no_error, phys_transient_allocator *allocator);
    void __thiscall check_surface_properties();
};

struct environment_rigid_body;
struct rigid_body_constraint_contact : rigid_body_constraint // sizeof=0x2C
{                                                                             // XREF: phys_free_list<rigid_body_constraint_contact>::T_internal/r
struct avl_tree_accessor
{
    using key_type  = rigid_body_pair_key;
    using node_type = rigid_body_constraint_contact;

    static bool less(const key_type& key, const node_type* node)
    {
        return key < node->m_avl_key;
    }

    static bool less(const node_type* node, const key_type& key)
    {
        return node->m_avl_key < key;
    }

    static bool equals(const node_type* node, const key_type& key)
    {
        return node->m_avl_key == key;
    }

    // hacky aislop shit
    static void set_key(node_type *node, const key_type &key)
    {
        node->m_avl_key = key;
    }
};

    phys_simple_link_list<contact_point_info> m_list_contact_point_info_buffer_1;
    phys_simple_link_list<contact_point_info> m_list_contact_point_info_buffer_2;

    unsigned int m_solver_priority;
    phys_inplace_avl_tree_node<rigid_body_constraint_contact> m_avl_tree_node;
    rigid_body_pair_key m_avl_key;

    ~rigid_body_constraint_contact();

    void add_cpi_simple(
        contact_point_info *cpi,
        environment_rigid_body *b1_,
        environment_rigid_body *b2_);

    void update_smallest_lambda();
    void setup_constraint(struct pulse_sum_constraint_solver *phys, float delta_t);
    void verify_constraint(environment_rigid_body *b1_, environment_rigid_body *b2_);
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(rigid_body_constraint_contact) == 0x2C);
#endif

struct environment_rigid_body : rigid_body // sizeof=0x160
{                                       // XREF: physics_system/r
    void set();
};

//void __thiscall rigid_body::add_force(rigid_body *this, const phys_vec3 *force);

//void __thiscall rigid_body::add_force(
//    rigid_body *this,
//    const phys_vec3 *force,
//    const phys_vec3 *point,
//    float torque_mult);

//void __thiscall rigid_body::add_force(rigid_body *this, const phys_vec3 *force);
//void __thiscall rigid_body::set_inertia(rigid_body *this, const phys_vec3 *inertia);
//void __thiscall rigid_body::set_mass(rigid_body *this, float mass);
// 
//void __thiscall rigid_body::set(
//                rigid_body *this,
//                float mass,
//                const phys_vec3 *inertia,
//                const phys_mat44 *mat,
//                const phys_vec3 *t_vel,
//                const phys_vec3 *a_vel,
//                int stable_min_contact_count);
//void __thiscall rigid_body::add_force(
//                rigid_body *this,
//                const phys_vec3 *force,
//                const phys_vec3 *point,
//                float torque_mult);
// 
//void __thiscall user_rigid_body::set(user_rigid_body *this, const phys_mat44 *const dictator);
//void __thiscall user_rigid_body::setPosition(user_rigid_body *this, const phys_mat44 *const dictator);
// 
//void __thiscall environment_rigid_body::set(environment_rigid_body *this);
