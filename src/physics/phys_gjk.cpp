#include "phys_gjk.h"
#include <bgame/bg_slidemove.h>
#include "phys_assert.h"
#include <tl/tl_system.h>
#include "phys_gjk_sep_dir.h"

// LWSS: For this file, x87 should be turned on. 
// Why? Well it has more bits of precision in the floats
// (Honestly it should probably be turned on for the entire project)
// `mp_array`, I was having endless (endless, and not sometimes!) gjk warnings about max iterations. This seems to have fixed it.
// Could also be fixed with more iterations, but that's a hack.
#if (!defined(_M_IX86_FP) || _M_IX86_FP != 0) && !defined(KISAK_NX)
// to turn on x87: Edit the vcxproj 
// <ClCompile Include="src\physics\phys_gjk.cpp">
// <EnableEnhancedInstructionSet Condition = "'$(Platform)'=='Win32'" > NoExtensions< / EnableEnhancedInstructionSet>
// < / ClCompile>
#error "phys_gjk.cpp expects x87 floats"
#endif

const float SEP_CONV_THRESH = 0.001f;
const float CONV_THRESH = 0.001f;
int MAX_CC_RESET_ITERS = 5;

phys_assert_info pai_gjk_initial_support_dir = { 0, 128, true };

// aislop
int    backup1(phys_gjk_info *gjk_info, int new_index, bool seed_simplex)
{
    // Pointer to the new support vertex (w_verts[new_index])
    const phys_vec3 *new_vert = &gjk_info->m_w_verts[new_index];

    float new_vert_sq = new_vert->x * new_vert->x
        + new_vert->y * new_vert->y
        + new_vert->z * new_vert->z;

    int new_bit = 1 << new_index;
    int best_set = new_bit;
    float best_dist_sq = new_vert_sq;

    // Initialize lambda for the singleton set containing new_index
    gjk_info->m_set_list[new_bit].m_lamda[new_index] = 1.0f;

    // Remaining verts in the current w_set (excluding new_index)
    int remaining = ~new_bit & gjk_info->m_w_set;

    // -----------------------------------------------------------------------
    // Phase 1: Test edges from each existing simplex vertex to new_index
    // -----------------------------------------------------------------------
    int w_count = 0;

    // Per-vertex data accumulated during phase 1, used in phase 2
    int   w_inds[4] = {};   // index of each existing vert
    float ndifs_sq[4] = {};   // squared length of edge vector (w_i - new_vert)
    float dotps[4] = {};   // dot(edge, new_vert) where edge = w_i - new_vert (used for barycentric projection)

    // Parallel arrays of (w_i - new_vert) direction vectors, one per existing vert
    phys_vec3 difs[4] = {};

    int bit = 1;
    for (int i = 0; i < 4; ++i, bit <<= 1)
    {
        if (!(remaining & bit))
            continue;

        const phys_vec3 *wi = &gjk_info->m_w_verts[i];

        // Edge vector from new_vert to w_i
        phys_vec3 edge;
        edge.x = wi->x - new_vert->x;
        edge.y = wi->y - new_vert->y;
        edge.z = wi->z - new_vert->z;

        float edge_sq = edge.x * edge.x + edge.y * edge.y + edge.z * edge.z;
        // dot(edge, new_vert) - used for barycentric projection of origin onto the edge
        float dot_edge_new_vert = edge.x * new_vert->x + edge.y * new_vert->y + edge.z * new_vert->z;

        // Store for phase 2
        w_inds[w_count] = i;
        ndifs_sq[w_count] = edge_sq;
        dotps[w_count] = dot_edge_new_vert;
        difs[w_count] = edge;

        // Test the line segment [new_vert, w_i]:
        // closest point P(t) = new_vert + t*edge, perpendicular to edge gives
        //   t = -dot(edge, new_vert) / |edge|^2
        if (edge_sq > 1e-10f)
        {
            float t = -dot_edge_new_vert / edge_sq;
            float one_minus_t = 1.0f - t;

            if (t > 0.0f && one_minus_t > 0.0f)
            {
                // Closest point on segment = new_vert + t * edge
                phys_vec3 closest;
                closest.x = new_vert->x + edge.x * t;
                closest.y = new_vert->y + edge.y * t;
                closest.z = new_vert->z + edge.z * t;

                float dist_sq = closest.x * closest.x
                    + closest.y * closest.y
                    + closest.z * closest.z;

                if (best_dist_sq > dist_sq)
                {
                    best_dist_sq = dist_sq;
                    best_set = new_bit | bit;

                    phys_gjk_info::phys_gjk_set_info *s = &gjk_info->m_set_list[best_set];
                    s->m_lamda[i] = t;
                    s->m_lamda[new_index] = one_minus_t;
                }
            }
        }

        // seed_simplex mode: also test whether w_i alone is closer than current best
        if (seed_simplex)
        {
            float wi_sq = wi->x * wi->x + wi->y * wi->y + wi->z * wi->z;
            if (best_dist_sq > wi_sq)
            {
                best_dist_sq = wi_sq;
                best_set = bit;
                gjk_info->m_set_list[bit].m_lamda[i] = 1.0f;
            }
        }

        ++w_count;
    }

    // -----------------------------------------------------------------------
    // seed_simplex special case: when exactly 2 existing verts were found,
    // also test the edge between w_verts[1] and w_verts[2] (indices 1 and 2)
    // -----------------------------------------------------------------------
    if (seed_simplex && w_count == 2)
    {
        phys_vec3 edge;
        edge.x = gjk_info->m_w_verts[2].x - gjk_info->m_w_verts[1].x;
        edge.y = gjk_info->m_w_verts[2].y - gjk_info->m_w_verts[1].y;
        edge.z = gjk_info->m_w_verts[2].z - gjk_info->m_w_verts[1].z;

        float edge_sq = edge.x * edge.x + edge.y * edge.y + edge.z * edge.z;
        if (edge_sq > 1e-10f)
        {
            // Project w_verts[1] onto the edge
            float dot = gjk_info->m_w_verts[1].x * edge.x
                + gjk_info->m_w_verts[1].y * edge.y
                + gjk_info->m_w_verts[1].z * edge.z;
            float t = -dot / edge_sq;
            float one_minus_t = 1.0f - t;

            if (t > 0.0f && one_minus_t > 0.0f)
            {
                phys_vec3 closest;
                closest.x = gjk_info->m_w_verts[1].x + edge.x * t;
                closest.y = gjk_info->m_w_verts[1].y + edge.y * t;
                closest.z = gjk_info->m_w_verts[1].z + edge.z * t;

                float dist_sq = closest.x * closest.x
                    + closest.y * closest.y
                    + closest.z * closest.z;

                if (best_dist_sq > dist_sq)
                {
                    best_dist_sq = dist_sq;
                    best_set = 6; // bits 1|2 => indices 1 and 2

                    gjk_info->m_set_list[6].m_lamda[1] = one_minus_t;
                    gjk_info->m_set_list[6].m_lamda[2] = t;
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Phase 2: Test triangles formed by pairs of existing verts + new_index
    // -----------------------------------------------------------------------

    if (w_count - 1 > 0)
    {
        // Iterate over all ordered pairs (a, b) where a < b, both from w_inds[0..w_count-1]
        // Outer loop: 'a' iterates w_inds[0 .. w_count-2]
        // Inner loop: 'b' iterates w_inds[a+1 .. w_count-1]
        for (int a = 0; a < w_count - 1; ++a)
        {
            int   idx_a = w_inds[a];
            float a_sq = ndifs_sq[a];   // |dif_a|^2
            float neg_dp_a = -dotps[a];     // -dot(dif_a, new_vert)

            for (int b = a + 1; b < w_count; ++b)
            {
                int   idx_b = w_inds[b];
                const phys_vec3 *dif_b = &difs[b];

                // dot(dif_a, dif_b)
                float dot_ab = difs[a].x * dif_b->x
                    + difs[a].y * dif_b->y
                    + difs[a].z * dif_b->z;

                float b_sq = ndifs_sq[b];
                float neg_dp_b = -dotps[b];

                // Gram determinant: |dif_a|^2 * |dif_b|^2 - dot(dif_a,dif_b)^2
                float gram_det = a_sq * b_sq - dot_ab * dot_ab;

                if (gram_det > 1e-5f)
                {
                    // Barycentric coords for the triangle [new_vert, w_a, w_b]
                    float lambda_a = (b_sq * neg_dp_a - neg_dp_b * dot_ab) / gram_det;
                    float lambda_b = (a_sq * neg_dp_b - dot_ab * neg_dp_a) / gram_det;
                    float lambda_new = 1.0f - lambda_a - lambda_b;

                    if (lambda_a > 0.0f && lambda_b > 0.0f && lambda_new > 0.0f)
                    {
                        // Closest point = new_vert*lambda_new + w_a*lambda_a + w_b*lambda_b
                        phys_vec3 contrib_a, contrib_b, closest;
                        contrib_a.x = difs[a].x * lambda_a;
                        contrib_a.y = difs[a].y * lambda_a;
                        contrib_a.z = difs[a].z * lambda_a;

                        contrib_b.x = dif_b->x * lambda_b;
                        contrib_b.y = dif_b->y * lambda_b;
                        contrib_b.z = dif_b->z * lambda_b;

                        // new_vert scaled by lambda_new
                        phys_vec3 new_contrib;
                        new_contrib.x = new_vert->x + contrib_a.x;
                        new_contrib.y = new_vert->y + contrib_a.y;
                        new_contrib.z = new_vert->z + contrib_a.z;

                        closest.x = new_contrib.x + contrib_b.x;
                        closest.y = new_contrib.y + contrib_b.y;
                        closest.z = new_contrib.z + contrib_b.z;

                        float dist_sq = closest.x * closest.x
                            + closest.y * closest.y
                            + closest.z * closest.z;

                        if (best_dist_sq > dist_sq)
                        {
                            best_dist_sq = dist_sq;
                            best_set = new_bit | (1 << idx_a) | (1 << idx_b);

                            phys_gjk_info::phys_gjk_set_info *s = &gjk_info->m_set_list[best_set];
                            s->m_lamda[idx_a] = lambda_a;
                            s->m_lamda[idx_b] = lambda_b;
                            s->m_lamda[new_index] = lambda_new;
                        }
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Phase 3: Tetrahedron-contains-origin test (binary @ 0xa0c9a6..0xa0cb9c).
    //
    // An earlier pass deleted this as "dead code with sign-convention
    // landmines" - that analysis was wrong. Hex-rays displays every local of
    // this function shifted by -12 bytes (frame re-alignment prologue:
    // `and esp, 0FFFFFFF0h` + pushes after `mov ebp, esp`), so the
    // pseudocode's difs[]/w_inds[] names point one slot off while the math
    // itself is fine. Per the raw disasm the real slots are
    //   difs[0] @ ebp-E0h, difs[1] @ ebp-D0h, difs[2] @ ebp-C0h
    // and the test below is plain Cramer's rule on
    //   -new_vert = t1*difs[0] + t0*difs[1] + t2*difs[2]
    // If all three coefficients and the new_vert weight (1 - t1 - t0 - t2)
    // are non-negative, the origin lies inside the tetrahedron spanned by
    // new_vert and the three existing verts -> return 15 (full simplex,
    // penetrating). This is the intended termination path for deep contacts;
    // without it gjk()/gjk_ray_cast() can only exit penetrating cases via
    // the pen-thresh check or the 30-iteration cap (= warning spam).
    //
    // Only runs when no positive separation lower bound exists yet: the
    // binary compares a literal fldz against m_lower_dist_sq (hex-rays'
    // `v22 >= m_lower_dist_sq` with v22 == 0.0).
    // -----------------------------------------------------------------------
    if (w_count == 3 && gjk_info->m_lower_dist_sq <= 0.0f)
    {
        // cross = difs[0] x difs[1]   (0xa0c9c1..0xa0ca1d)
        phys_vec3 cross;
        cross.x = difs[0].y * difs[1].z - difs[1].y * difs[0].z;
        cross.y = difs[0].z * difs[1].x - difs[0].x * difs[1].z;
        cross.z = difs[0].x * difs[1].y - difs[0].y * difs[1].x;

        // det = dot(cross, difs[2]) - signed volume spanned by the edges
        float det = cross.x * difs[2].x + cross.y * difs[2].y + cross.z * difs[2].z;
        if (fabs(det) <= 1e-5f)
            return best_set;                    // degenerate tetrahedron

        // t2 = coefficient of difs[2]   (0xa0ca74..0xa0caa7)
        float t2 = -(cross.x * new_vert->x + cross.y * new_vert->y + cross.z * new_vert->z) / det;
        if (t2 < 0.0f)
            return best_set;

        // wxd2 = new_vert x difs[2]   (0xa0caad..0xa0cae9)
        phys_vec3 wxd2;
        wxd2.x = new_vert->y * difs[2].z - difs[2].y * new_vert->z;
        wxd2.y = difs[2].x * new_vert->z - new_vert->x * difs[2].z;
        wxd2.z = difs[2].y * new_vert->x - new_vert->y * difs[2].x;

        // t1 = coefficient of difs[0]
        float t1 = (difs[1].x * wxd2.x + difs[1].y * wxd2.y + difs[1].z * wxd2.z) / det;
        if (t1 < 0.0f)
            return best_set;

        // t0 = coefficient of difs[1]
        float t0 = -(difs[0].x * wxd2.x + difs[0].y * wxd2.y + difs[0].z * wxd2.z) / det;
        if (t0 < 0.0f)
            return best_set;

        // weight of new_vert itself; >= 0 (binary accepts equality) -> inside
        if (1.0f - t1 - t0 - t2 >= 0.0f)
            return 15;
    }

    return best_set;
}

// local variable allocation has failed, the output may be wrong!
void    phys_full_inv_multiply_mat(
                phys_mat44 *dest,
                const phys_mat44 *left,
                const phys_mat44 *right)
{
    phys_vec3 *v5; // eax
    phys_mat44 v6; // [esp-Ch] [ebp-5Ch] BYREF
    phys_vec3 temp; // [esp+34h] [ebp-1Ch] BYREF

    assert_mat44_initialized(*left);
    assert_mat44_initialized(*right);

    if (dest == left)
    {
        memcpy(&v6, left, sizeof(v6));
        phys_full_inv_multiply_mat(dest, &v6, right);
    }
    else
    {
        temp.x = left->x.x * right->x.x + left->x.y * right->x.y + left->x.z * right->x.z;
        temp.y = left->y.y * right->x.y + left->y.x * right->x.x + left->y.z * right->x.z;
        temp.z = left->z.y * right->x.y + left->z.x * right->x.x + left->z.z * right->x.z;
        dest->x.x = temp.x;
        dest->x.y = temp.y;
        dest->x.z = temp.z;
        temp.x = left->x.x * right->y.x + right->y.y * left->x.y + right->y.z * left->x.z;
        temp.y = left->y.y * right->y.y + left->y.x * right->y.x + left->y.z * right->y.z;
        temp.z = left->z.y * right->y.y + left->z.x * right->y.x + left->z.z * right->y.z;
        dest->y.x = temp.x;
        dest->y.y = temp.y;
        dest->y.z = temp.z;
        temp.x = right->z.x * left->x.x + left->x.y * right->z.y + right->z.z * left->x.z;
        temp.y = left->y.y * right->z.y + left->y.x * right->z.x + left->y.z * right->z.z;
        temp.z = left->z.y * right->z.y + right->z.x * left->z.x + left->z.z * right->z.z;
        dest->z.x = temp.x;
        dest->z.y = temp.y;
        dest->z.z = temp.z;
        v5 = phys_full_inv_multiply(&temp, left, &right->w);
        dest->w.x = v5->x;
        dest->w.y = v5->y;
        dest->w.z = v5->z;
    }
}

void __cdecl get_simplex(
                const phys_gjk_geom *cg1,
                const phys_gjk_geom *cg2,
                phys_gjk_cache_info *gjk_ci,
                phys_vec3 *a_verts,
                phys_vec3 *a_inds,
                phys_vec3 *b_verts,
                phys_vec3 *b_inds,
                int *vert_count)
{
    iassert(gjk_ci->is_simplex_valid());

    cg1->get_simplex(&gjk_ci->m_support_a, gjk_ci->m_support_count, a_verts, a_inds);
    cg2->get_simplex(&gjk_ci->m_support_b, gjk_ci->m_support_count, b_verts, b_inds);
    *vert_count = gjk_ci->m_support_count;
}

static const int BIT_COUNT[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
void __cdecl set_simplex(
                const phys_gjk_geom *cg1,
                const phys_gjk_geom *cg2,
                phys_gjk_cache_info *gjk_ci,
                const phys_vec3 *a_normal,
                const phys_vec3 *b_normal,
                const phys_vec3 *a_inds,
                const phys_vec3 *b_inds,
                int w_set)
{
    iassert(w_set > 0 && w_set <= 15);

    gjk_ci->m_flags |= 8u;
    cg1->set_simplex(a_inds, w_set, a_normal, &gjk_ci->m_support_a);
    cg2->set_simplex(b_inds, w_set, b_normal, &gjk_ci->m_support_b);

    gjk_ci->m_support_count = BIT_COUNT[w_set];
}

int phys_gjk_info::gjk_subalgorithm(int w_set, int new_index)
{
    return backup1(this, new_index, 0);
}

int phys_gjk_info::seed_simplex(int cached_vert_count)
{
    iassert(cached_vert_count > 0);
    iassert(cached_vert_count < 4);

    this->m_w_set = 0;
    int bit = 1;
    int i = 0;

#if 0 // LWSS: this (shitty!) code is not even intended to go beyond 3 (see assert^^)
    if (cached_vert_count >= 4)
    {
        int iterations = ((unsigned int)(cached_vert_count - 4) >> 2) + 1;
        i = iterations * 4;

        for (int k = 0; k < iterations; ++k)
        {
            // w_verts[k*4+0] = (a_verts[k*4+0] - b_verts[k*4+0]) - origin
            this->m_w_verts[k * 4 + 0].x = (this->m_a_verts[k * 4 + 0].x - this->m_b_verts[k * 4 + 0].x) - this->m_gjk_origin.x;
            this->m_w_verts[k * 4 + 0].y = (this->m_a_verts[k * 4 + 0].y - this->m_b_verts[k * 4 + 0].y) - this->m_gjk_origin.y;
            this->m_w_verts[k * 4 + 0].z = (this->m_a_verts[k * 4 + 0].z - this->m_b_verts[k * 4 + 0].z) - this->m_gjk_origin.z;
            this->m_w_set |= bit;
            bit = __ROL4__(bit, 1);

            this->m_w_verts[k * 4 + 1].x = (this->m_a_verts[k * 4 + 1].x - this->m_b_verts[k * 4 + 1].x) - this->m_gjk_origin.x;
            this->m_w_verts[k * 4 + 1].y = (this->m_a_verts[k * 4 + 1].y - this->m_b_verts[k * 4 + 1].y) - this->m_gjk_origin.y;
            this->m_w_verts[k * 4 + 1].z = (this->m_a_verts[k * 4 + 1].z - this->m_b_verts[k * 4 + 1].z) - this->m_gjk_origin.z;
            this->m_w_set |= bit;
            bit = __ROL4__(bit, 1);

            this->m_w_verts[k * 4 + 2].x = (this->m_a_verts[k * 4 + 2].x - this->m_b_verts[k * 4 + 2].x) - this->m_gjk_origin.x;
            this->m_w_verts[k * 4 + 2].y = (this->m_a_verts[k * 4 + 2].y - this->m_b_verts[k * 4 + 2].y) - this->m_gjk_origin.y;
            this->m_w_verts[k * 4 + 2].z = (this->m_a_verts[k * 4 + 2].z - this->m_b_verts[k * 4 + 2].z) - this->m_gjk_origin.z;
            this->m_w_set |= bit;
            bit = __ROL4__(bit, 1);

            this->m_w_verts[k * 4 + 3].x = (this->m_a_verts[k * 4 + 3].x - this->m_b_verts[k * 4 + 3].x) - this->m_gjk_origin.x;
            this->m_w_verts[k * 4 + 3].y = (this->m_a_verts[k * 4 + 3].y - this->m_b_verts[k * 4 + 3].y) - this->m_gjk_origin.y;
            this->m_w_verts[k * 4 + 3].z = (this->m_a_verts[k * 4 + 3].z - this->m_b_verts[k * 4 + 3].z) - this->m_gjk_origin.z;
            this->m_w_set |= bit;
            bit = __ROL4__(bit, 1);
        }
    }
#endif

    // -----------------------------------------------------------------------
    // Scalar tail: handles remaining verts (or all verts when count < 4)
    // In practice with the assert, this always handles all 1-3 verts.
    // -----------------------------------------------------------------------
    for (; i < cached_vert_count; ++i)
    {
        this->m_w_verts[i].x = (this->m_a_verts[i].x - this->m_b_verts[i].x) - this->m_gjk_origin.x;
        this->m_w_verts[i].y = (this->m_a_verts[i].y - this->m_b_verts[i].y) - this->m_gjk_origin.y;
        this->m_w_verts[i].z = (this->m_a_verts[i].z - this->m_b_verts[i].z) - this->m_gjk_origin.z;

        this->m_w_set |= bit;
        bit = __ROL4__(bit, 1);
    }

    return backup1(this, 0, true);
}

// supervised aislop
phys_gjk_info::gjk_retval_e phys_gjk_info::gjk(
                const phys_gjk_input *d,
                const phys_vec3 *initial_support_dir,
                bool in_separation_loop)
{
    // Bounds on the squared distance from origin to the Minkowski difference
    // (A - B):
    //   m_upper_dist_sq = |v|^2          (current closest-point candidate)
    //   m_lower_dist_sq = max over iters of (v . w)^2 / |v|^2
    //                    (Johnson lower bound, monotonically -> |v|^2)
    //
    // The +/- 34.0 are sentinels - just "larger than any plausible squared
    // distance"; not algorithmically meaningful.
    m_lower_dist_sq = -34.0f;
    m_upper_dist_sq = 34.0f;
    m_gjk_iter = init_gjk(d, initial_support_dir, in_separation_loop);

    for (;;)
    {
        // -------- 1. Update upper bound from current search direction v ------
        m_upper_dist_sq = m_support_dir.x * m_support_dir.x
            + m_support_dir.y * m_support_dir.y
            + m_support_dir.z * m_support_dir.z;

        // After the first iteration, if |v|^2 has dropped below the
        // penetration threshold, the simplex effectively wraps the origin.
        if (m_gjk_iter != 0 && m_gjk_pen_thresh_sq > m_upper_dist_sq)
            return GJK_PENETRATING;

        // -------- 2. Pick the lowest free slot in the 4-vertex simplex -------
        //   Bit i of m_w_set is set iff slot i currently holds a vertex.
        //   Original codegen does the last two cases branchlessly via
        //     ((m_w_set & 4) | 8) >> 2  ->  2 or 3
        //   Expanded here for clarity; semantics are identical.
        int new_index;
        if ((m_w_set & 1) == 0) new_index = 0;
        else if ((m_w_set & 2) == 0) new_index = 1;
        else if ((m_w_set & 4) == 0) new_index = 2;
        else                         new_index = 3;

        // -------- 3. Support point on shape A in direction -v ----------------
        d->gjk_cg1->support(-m_support_dir, &m_a_verts[new_index], &m_a_inds[new_index]);

        // -------- 4. Support point on shape B in direction v -----------------
        // v lives in cg1's frame; rotate into cg2's frame.
        // cg2_to_cg1_xform stores basis vectors as rows, so multiplying v
        // against the rows gives the inverse rotation (R^T * v).
        phys_vec3 v_in_cg2;
        v_in_cg2.x = m_support_dir.x * cg2_to_cg1_xform.x.x
                   + m_support_dir.y * cg2_to_cg1_xform.x.y
                   + m_support_dir.z * cg2_to_cg1_xform.x.z;
        v_in_cg2.y = m_support_dir.x * cg2_to_cg1_xform.y.x
                   + m_support_dir.y * cg2_to_cg1_xform.y.y
                   + m_support_dir.z * cg2_to_cg1_xform.y.z;
        v_in_cg2.z = m_support_dir.x * cg2_to_cg1_xform.z.x
                   + m_support_dir.y * cg2_to_cg1_xform.z.y
                   + m_support_dir.z * cg2_to_cg1_xform.z.z;

        d->gjk_cg2->support(v_in_cg2, &m_b_verts[new_index], &m_b_inds[new_index]);

        // Bring B's support point back into A's frame so we can subtract:
        //   b_cg1 = R * b_cg2 + t   where (R | t) = cg2_to_cg1_xform
        phys_vec3 b_rotated;
        phys_multiply(&b_rotated, &cg2_to_cg1_xform, &m_b_verts[new_index]);
        m_b_verts[new_index].x = b_rotated.x + cg2_to_cg1_xform.w.x;
        m_b_verts[new_index].y = b_rotated.y + cg2_to_cg1_xform.w.y;
        m_b_verts[new_index].z = b_rotated.z + cg2_to_cg1_xform.w.z;

        // -------- 5. Minkowski-difference vertex w = a - b - origin ----------
        const phys_vec3 w = {
            (m_a_verts[new_index].x - m_b_verts[new_index].x) - m_gjk_origin.x,
            (m_a_verts[new_index].y - m_b_verts[new_index].y) - m_gjk_origin.y,
            (m_a_verts[new_index].z - m_b_verts[new_index].z) - m_gjk_origin.z
        };


        // -------- 6. Separation / lower-bound update -------------------------
        // v . w is the signed support along the current search direction.
        // If positive, we have new evidence the shapes are separated and can
        // tighten the lower bound on the squared distance.
        const float support_dot_w = m_support_dir.x * w.x
            + m_support_dir.y * w.y
            + m_support_dir.z * w.z;

        if (support_dot_w > 0.0f && m_upper_dist_sq > 0.0f)
        {
            const float candidate = (support_dot_w * support_dot_w) / m_upper_dist_sq;

            if (candidate > m_lower_dist_sq)
            {
                m_lower_dist_sq = candidate;

                // Optional early-out: caller asked us to stop as soon as
                // separation beyond m_gjk_sep_thresh can be proven.
                if ((m_flags & 1) != 0)
                {
                    const float thresh_sq = m_gjk_sep_thresh * m_gjk_sep_thresh;
                    if (thresh_sq < candidate)
                        return GJK_SEPARATED;
                }
            }
        }

        // -------- 7. Convergence check (only after first iteration) ----------
        // Original code's "v24 < m_lower_dist_sq" with v24 forced to 0.0
        // collapses to "have we ever produced a positive lower bound?".
        if (m_gjk_iter != 0 && m_lower_dist_sq > 0.0f)
        {
            // Tightness test: is m_lower_dist_sq / m_upper_dist_sq close
            // enough to 1?  Equivalent to |v_low| / |v_up| >= (1 - eps).
            const float ratio = 1.0f - CONV_THRESH;
            const float ratio_sq = ratio * ratio;
            if (m_lower_dist_sq > ratio_sq * m_upper_dist_sq)
                return GJK_VALID;

            // Duplicate-vertex test: if the new w almost equals any vertex
            // already in the simplex, no further progress is possible.
            // Iterates over m_last_w_set (the pre-reduction 4-vertex mask)
            // so vertices culled by the most recent backup1() are still
            // checked - this is what catches cycling.
            for (int i = 0, mask = 1; i < 4; ++i, mask <<= 1)
            {
                if ((m_last_w_set & mask) == 0)
                    continue;

                const float dx = w.x - m_w_verts[i].x;
                const float dy = w.y - m_w_verts[i].y;
                const float dz = w.z - m_w_verts[i].z;
                if (dx * dx + dy * dy + dz * dz < 1.0e-6f)
                    return GJK_VALID;
            }
        }

        // -------- 8. Add w to the simplex and reduce ------------------------
        m_w_verts[new_index] = w;
        m_w_set |= (1u << new_index);
        m_last_w_set = m_w_set;                 // snapshot pre-reduce simplex

        // Johnson sub-distance: returns the reduced simplex bitmask, or 0xF
        // if the tetrahedron actually contains the origin.
        m_w_set = backup1(this, new_index, /*seed_simplex=*/false);
        if (m_w_set == 15)
            return GJK_PENETRATING;

        // Recompute v as the closest point of the (possibly reduced) simplex
        // to the origin.
        comp_v(m_w_set, &m_support_dir);

        if (++m_gjk_iter >= 30)                 // hard iteration cap
        {
            tlWarning("gjk reached the maximum number of iterations. cg1=%s cg2=%s "
                "lower=%.6f upper=%.6f lambda=%.6f flags=0x%X",
                GjkTypeToString(d->gjk_cg1->get_type()),
                GjkTypeToString(d->gjk_cg2->get_type()),
                this->m_lower_dist_sq,
                this->m_upper_dist_sq,
                this->m_continuous_collision_lambda,
                this->m_flags);
            //tlWarning("gjk reached the maximum number of iterations.");
            return GJK_VALID;
        }
    }
}

// aislop used here, manual was about the same
phys_gjk_info::gjk_retval_e phys_gjk_info::gjk_ray_cast(
                const phys_gjk_input *d,
                const phys_vec3 *initial_support_dir,
                bool in_separation_loop)
{
    float nsupport_dir_len;
    float move;
    float ray_end_dist_numer;
    int new_index;

    // Initialize continuous collision time and shift GJK origin accordingly
    float lambda = d->m_start_time;
    this->m_continuous_collision_lambda = lambda;

    float neg_lambda = -lambda;
    this->m_gjk_origin.x = this->m_cg1_relative_translation_loc.x * neg_lambda;
    this->m_gjk_origin.y = this->m_cg1_relative_translation_loc.y * neg_lambda;
    this->m_gjk_origin.z = this->m_cg1_relative_translation_loc.z * neg_lambda;

    this->m_lower_dist_sq = -34.0f;
    this->m_upper_dist_sq = 34.0f;

    // init_gjk returns nonzero if initialized successfully; stores gjk_iter
    this->m_gjk_iter = init_gjk(d, initial_support_dir, in_separation_loop);

    // LWSS ADD
    iassert(d->m_start_time >= 0.0f && d->m_start_time <= 1.0f);
    iassert(d->m_end_time >= d->m_start_time && d->m_end_time <= 1.0f);
    iassert(this->m_gjk_sep_thresh > 0.0f);
    iassert(this->m_geom_radii_sum >= 0.0f);
    // LWSS END

    // Clear flags: bits 0 (sep), 2 (converged/done?), 5 (positive-witness)
    static_assert(0xFFFFFFDA == ~(0x1u | 0x4u | 0x20u));
    this->m_flags &= ~(0x1u | 0x4u | 0x20u);

    // Set/clear bit 4 (cached-support-dir) based on gjk_ci flags
    if (d->gjk_ci && (d->gjk_ci->m_flags & 4))
        this->m_flags |= 0x10u;
    else
        this->m_flags &= ~0x10u;

    this->m_cc_reset_iter = 0;

    float support_dir_moveback = (this->m_geom_radii_sum >= 0.051f) ? this->m_geom_radii_sum : 0.051f;

    iassert(support_dir_moveback < this->m_gjk_sep_thresh);

    // =========================================================
    // Outer loop: conservative advancement — advances lambda
    // =========================================================
    while (true)
    {
        // =====================================================
        // Inner GJK loop
        // =====================================================
        while (true)
        {
            // Compute ||support_dir||^2
            this->m_upper_dist_sq = AbsSquared(this->m_support_dir);

            if (this->m_gjk_iter != 0)
            {
                // Cached-support-dir mode: if ||v||^2 < sep_thresh^2, set sep flag
                if (this->m_flags & 0x10)
                {
                    float sep_thresh_sq = this->m_gjk_sep_thresh * this->m_gjk_sep_thresh;
                    if (sep_thresh_sq > this->m_upper_dist_sq)
                        this->m_flags |= 1u;
                }

                // Penetration threshold check
                if (this->m_gjk_pen_thresh_sq > this->m_upper_dist_sq)
                {
                    // Penetrating — emit warning if in CC mode with nonzero lambda
                    if ((this->m_flags & 8) &&
                        this->m_continuous_collision_lambda != 0.0f &&
                        !(this->m_flags & 2))
                    {
                        tlWarning("m_continuous_collision_lambda problem");
                    }
                    return GJK_PENETRATING;
                }
            }

            // Use bit flags to select new simplex slot from w_set
            // There are 4 slots 1 per vertex of the simplex
            int m_w_set = this->m_w_set;
            if (m_w_set & 1)
            {
                if (m_w_set & 2)
                    new_index = ((m_w_set & 4) | 8) >> 2;
                else
                    new_index = 1;
            }
            else
            {
                new_index = 0;
            }

            // Grab a m_a_vert[] Support Vertex from CG_1 - in the negative support direction
            d->gjk_cg1->support(-m_support_dir, &this->m_a_verts[new_index], &this->m_a_inds[new_index]);

            iassert(m_a_verts[new_index].GetX() == m_a_verts[new_index].GetX()
                && m_a_verts[new_index].GetY() == m_a_verts[new_index].GetY()
                && m_a_verts[new_index].GetZ() == m_a_verts[new_index].GetZ());

            // Support function for B (cg2): rotate support_dir into cg2 frame
            phys_vec3 dir_b;
            dir_b.x = this->cg2_to_cg1_xform.x.y * this->m_support_dir.y
                    + this->cg2_to_cg1_xform.x.x * this->m_support_dir.x
                    + this->cg2_to_cg1_xform.x.z * this->m_support_dir.z;
            dir_b.y = this->cg2_to_cg1_xform.y.y * this->m_support_dir.y
                    + this->cg2_to_cg1_xform.y.x * this->m_support_dir.x
                    + this->cg2_to_cg1_xform.y.z * this->m_support_dir.z;
            dir_b.z = this->cg2_to_cg1_xform.z.y * this->m_support_dir.y
                    + this->cg2_to_cg1_xform.z.x * this->m_support_dir.x
                    + this->cg2_to_cg1_xform.z.z * this->m_support_dir.z;

            // Grab a m_b_vert[] Support Vertex from CG_2 - in the support direction (local to cg2)
            d->gjk_cg2->support(dir_b, &this->m_b_verts[new_index], &this->m_b_inds[new_index]);

            // Transform the m_b_vert[] Support vertex we just got into cg1's local space. Reassign m_b_verts[] with this converted value.
            phys_vec3 b_world;
            phys_multiply(&b_world, &this->cg2_to_cg1_xform, &this->m_b_verts[new_index]);
            this->m_b_verts[new_index].x = b_world.x + this->cg2_to_cg1_xform.w.x;
            this->m_b_verts[new_index].y = b_world.y + this->cg2_to_cg1_xform.w.y;
            this->m_b_verts[new_index].z = b_world.z + this->cg2_to_cg1_xform.w.z;

            iassert(m_b_verts[new_index].GetX() == m_b_verts[new_index].GetX()
                && m_b_verts[new_index].GetY() == m_b_verts[new_index].GetY()
                && m_b_verts[new_index].GetZ() == m_b_verts[new_index].GetZ());

            // Calculate 1 vertex of the Minkowski difference (w = a - b - origin)
            const phys_vec3 w = {
                (m_a_verts[new_index].x - m_b_verts[new_index].x) - m_gjk_origin.x,
                (m_a_verts[new_index].y - m_b_verts[new_index].y) - m_gjk_origin.y,
                (m_a_verts[new_index].z - m_b_verts[new_index].z) - m_gjk_origin.z
            };

            // v_dot_w = dot(support_dir, w)
            float support_dot_w = this->m_support_dir.x * w.x
                                + this->m_support_dir.y * w.y
                                + this->m_support_dir.z * w.z;

            // Lower-bound update and separation check
            float candidate = 0.0f;
            if (support_dot_w > 0.0f)
            {
                if (this->m_upper_dist_sq > 0.0f)
                {
                    this->m_flags |= 0x20u;
                    float raw_candidate = (support_dot_w * support_dot_w) / this->m_upper_dist_sq;
                    candidate = 0.0f;  // candidate used for convergence check stays 0
                    if (this->m_lower_dist_sq < raw_candidate)
                    {
                        this->m_lower_dist_sq = raw_candidate;
                        if (this->m_flags & 1)
                        {
                            float thresh_sq = this->m_gjk_sep_thresh * this->m_gjk_sep_thresh;
                            if (thresh_sq < raw_candidate)
                                return GJK_SEPARATED;
                        }
                    }
                }
            }

            // Convergence check — only after first iteration and if lower bound didn't grow
            bool has_converged = false;
            //if (this->m_gjk_iter != 0 && candidate <= this->m_lower_dist_sq)
            //if (this->m_gjk_iter != 0 && v_dot_w > 0.0f && this->m_upper_dist_sq > 0.0f && candidate < this->m_lower_dist_sq)
            if (this->m_gjk_iter != 0 && candidate < this->m_lower_dist_sq)
            {
                // has_converged = (lower > (1-CONV_THRESH)^2 * upper)
                float thresh = 1.0f - CONV_THRESH;
                thresh *= thresh;
                if (this->m_lower_dist_sq > thresh * this->m_upper_dist_sq)
                    has_converged = true;

                // Degeneracy check: is w already in the simplex?
                if (!has_converged)
                {
                    const float epsilon_sq = 1e-6f;
                    for (int i = 0, mask = 1; i < 4; ++i, mask <<= 1)
                    {
                        if (this->m_last_w_set & mask)
                        {
                            float dx = w.x - this->m_w_verts[i].x;
                            float dy = w.y - this->m_w_verts[i].y;
                            float dz = w.z - this->m_w_verts[i].z;
                            if (dx * dx + dy * dy + dz * dz < epsilon_sq)
                            {
                                has_converged = true;
                                break;
                            }
                        }
                    }
                }
            }

            // Conservative advancement — only if flag 4 not set and v_dot_w > 0
            if (!(this->m_flags & 4) && support_dot_w > 0.0f)
            {
                float geom_block = support_dir_moveback * support_dir_moveback;
                if (geom_block * this->m_upper_dist_sq <= support_dot_w * support_dot_w)
                {
                    nsupport_dir_len = sqrtf(this->m_upper_dist_sq);
                    iassert(nsupport_dir_len > 0.0f); // "nsupport_dir > 0.0f"

                    move = -(this->m_cg1_relative_translation_loc.x * this->m_support_dir.x
                        + this->m_cg1_relative_translation_loc.y * this->m_support_dir.y
                        + this->m_cg1_relative_translation_loc.z * this->m_support_dir.z);

                    // Adjust v_dot_w by subtracting the geom radii contribution
                    ray_end_dist_numer = support_dot_w - nsupport_dir_len * support_dir_moveback;

                    if (move <= 0.0f)
                    {
                        // Objects not approaching each other
                        if (ray_end_dist_numer > 0.0f)
                        {
                            float thresh_sq = this->m_gjk_sep_thresh * this->m_gjk_sep_thresh;
                            if (thresh_sq < this->m_lower_dist_sq)
                                return GJK_SEPARATED;
                            this->m_flags |= 5u;
                        }
                        // Fall through to add w to simplex
                    }
                    else if (ray_end_dist_numer >= 0.0f)
                    {
                        // move > 0: objects approaching, compute new lambda candidate.
                        // IDA gates the advance on ray_end_dist_numer >= 0 as well -
                        // once the support point is inside the moveback radius the
                        // binary stops advancing lambda and just refines the simplex.
                        float dot_origin = this->m_support_dir.x * this->m_gjk_origin.x
                            + this->m_support_dir.y * this->m_gjk_origin.y
                            + this->m_support_dir.z * this->m_gjk_origin.z;

                        float new_lambda = (dot_origin + ray_end_dist_numer) / move;

                        if (this->m_continuous_collision_lambda + 0.0001f < new_lambda)
                        {
                            // Lambda advanced enough — check against end_time before breaking
                            if (d->m_end_time < new_lambda)
                            {
                                //float end_numerator = ray_end_dist_numer - move * d->m_end_time;
                                float end_numerator = ray_end_dist_numer + dot_origin - move * d->m_end_time;

                                // Check: sep_thresh * nsupport_dir_len >= end_numerator
                                float sep_limit = this->m_gjk_sep_thresh * nsupport_dir_len;

                                if (sep_limit >= end_numerator)
                                {
                                    // Clamp: use end_time as lambda, set converged+sep flags
                                    lambda = d->m_end_time;
                                    this->m_flags |= 5u;
                                    // Jump to outer-loop update (break inner)
                                    break;
                                }
                                else
                                {
                                    // Ray exits before contact — separated
                                    iassert(end_numerator > 0.0f); // "ray_end_dist_numer > 0.0f"
                                    return GJK_SEPARATED;
                                }
                            }
                            else
                            {
                                // new_lambda <= end_time: valid advancement, break to outer loop
                                lambda = new_lambda;
                                break; // break inner GJK loop
                            }
                        }
                        // else: lambda didn't advance enough, fall through to add w to simplex
                    }
                }
            }

            if (has_converged)
                return GJK_VALID;

            // Add w to simplex
            this->m_w_verts[new_index].x = w.x;
            this->m_w_verts[new_index].y = w.y;
            this->m_w_verts[new_index].z = w.z;

            m_w_set |= (1 << new_index);
            this->m_last_w_set = m_w_set;
            this->m_w_set = m_w_set;

            m_w_set = backup1(this, new_index, false);
            this->m_w_set = m_w_set;

            if (m_w_set == 0xF)
            {
                // Full simplex — origin enclosed, penetrating
                if ((this->m_flags & 8) && this->m_continuous_collision_lambda != 0.0f)
                    tlWarning("m_continuous_collision_lambda problem");
                return GJK_PENETRATING;
            }

            comp_v(m_w_set, &this->m_support_dir);

            ++this->m_gjk_iter;
            if (this->m_gjk_iter >= 30)
            {
                //tlWarning("gjk reached the maximum number of iterations.");
                //tlWarning("gjk reached the maximum number of iterations. cg1(type:%s) - cg2(type:%s)", 
                //    GjkTypeToString(d->gjk_cg1->get_type()), GjkTypeToString(d->gjk_cg2->get_type()));
                tlWarning("gjk_ray_cast reached the maximum number of iterations. cg1=%s cg2=%s "
                    "lower=%.6f upper=%.6f lambda=%.6f flags=0x%X",
                    GjkTypeToString(d->gjk_cg1->get_type()),
                    GjkTypeToString(d->gjk_cg2->get_type()),
                    this->m_lower_dist_sq,
                    this->m_upper_dist_sq,
                    this->m_continuous_collision_lambda,
                    this->m_flags);
                return (this->m_lower_dist_sq > 0.0f) ? GJK_VALID : GJK_PENETRATING;
            }

        } // end inner GJK loop

        // -----------------------------------------------------------------
        // Lambda advanced into outer loop — update state and re-init GJK
        // -----------------------------------------------------------------
        // This checks: new_lambda - old_lambda <= 0 (didn't really advance)
        if (lambda - this->m_continuous_collision_lambda <= 0.0001f)
            this->m_flags |= 5u;

        // Store new lambda, update origin
        this->m_continuous_collision_lambda = lambda;

        float neg_lam = -lambda;
        this->m_gjk_origin.x = this->m_cg1_relative_translation_loc.x * neg_lam;
        this->m_gjk_origin.y = this->m_cg1_relative_translation_loc.y * neg_lam;
        this->m_gjk_origin.z = this->m_cg1_relative_translation_loc.z * neg_lam;

        ++this->m_cc_reset_iter;

        // Reset bounds
        this->m_lower_dist_sq = -34.0f;
        this->m_upper_dist_sq = 34.0f;

        // After first reset, enable cached-support-dir mode
        if (this->m_cc_reset_iter > 1)
            this->m_flags |= 0x10u;

        // If we've reset too many times, force convergence
        if (this->m_cc_reset_iter >= MAX_CC_RESET_ITERS)
            this->m_flags |= 5u;

        // If w_set is empty, seed with current new_index
        if (!this->m_w_set)
            this->m_w_set = (1 << new_index);

        // Re-initialize GJK from current support dir in separation-loop mode
        // ASM: push 1; push edx (=m_support_dir pointer); push edi (d); mov ecx, esi; call init_gjk
        // Note: edx = lea [esi+1E0h] set earlier as the support_dir address
        int did_init = init_gjk(d, &this->m_support_dir, true);
        iassert(did_init);                                       // "did_init"
        iassert(this->m_w_set != 0);                            // "m_w_set != 0"
        iassert(this->m_last_w_set == this->m_w_set);           // "m_last_w_set == m_w_set"

        ++this->m_gjk_iter;

    } // end outer conservative advancement loop
}

phys_gjk_info::gjk_retval_e phys_gjk_info::collide(const phys_gjk_input *d)
{
    phys_vec3 support_dir; // [esp-3Ch] [ebp-6Ch] BYREF

    this->m_gjk_origin = PHYS_ZERO_VEC;
    this->m_flags |= 1u;

    phys_vec3 initial_support_dir; // [esp-Ch] [ebp-3Ch] BYREF
    phys_gjk_info::get_initial_support_dir(&initial_support_dir, d);

    // seems to be some kind of valid number check
    iassert((initial_support_dir.GetX() == initial_support_dir.GetX() 
        && initial_support_dir.GetY() == initial_support_dir.GetY() 
        && initial_support_dir.GetZ() == initial_support_dir.GetZ()));

    phys_gjk_info::gjk_retval_e retv; // eax

    if ((this->m_flags & 8) != 0)
        retv = phys_gjk_info::gjk_ray_cast(d, &initial_support_dir, false);
    else
        retv = phys_gjk_info::gjk(d, &initial_support_dir, false);

    if (retv == GJK_INVALID)
    {
        iassert(retv != GJK_INVALID);
        iassert(retv == GJK_SEPARATED || retv == GJK_VALID);

        return retv;
    }

    if (retv != GJK_PENETRATING)
    {
        if ((unsigned int)retv < GJK_PENETRATING)
            return retv;

        iassert(retv == GJK_SEPARATED || retv == GJK_VALID);
        return retv;
    }

    if ((this->m_flags & 2) != 0)
        return GJK_PENETRATING;

    if ((this->m_flags & 8) && this->m_continuous_collision_lambda != 0.0f)
        tlWarning("m_continuous_collision_lambda problem");

    this->m_continuous_collision_lambda = 0.0;

    if (d->gjk_ci && (d->gjk_ci->m_flags & 4) != 0)
    {
        support_dir = initial_support_dir;
    }
    else
    {
        gjk_sep_dir::comp_sep_dir(&support_dir, d, this);
    }

    float support_dir_sq = AbsSquared(support_dir);

    if (support_dir_sq < 0.0000000099999991f)
    {
        float init_sq = initial_support_dir.x * initial_support_dir.x
            + initial_support_dir.y * initial_support_dir.y
            + initial_support_dir.z * initial_support_dir.z;

        if (init_sq < 0.0000000099999991f)
        {
            // Both degenerate — fatal
            iassert(0); // "initial support dir invalid." line 0x820
            return GJK_INVALID;
        }

        support_dir = initial_support_dir;
        support_dir_sq = init_sq;
    }


    this->m_flags &= ~1u;
    int iters = 0;

    while (true)
    {
        iters++;

        phys_vec3 a_vert;
        phys_vec3 __;
        d->gjk_cg1->support(-support_dir, &a_vert, &__);
        phys_vec3 b_vert;
        d->gjk_cg2->support_only(&b_vert, &this->cg2_to_cg1_xform, &support_dir);

        phys_vec3 w;
        w.x = a_vert.x - b_vert.x;
        w.y = a_vert.y - b_vert.y;
        w.z = a_vert.z - b_vert.z;

        iassert((w.GetX() == w.GetX() && w.GetY() == w.GetY() && w.GetZ() == w.GetZ()));

        float support_dir_len = sqrt(support_dir_sq);
        float inflate = 17.0f / support_dir_len;

        float dot_w_sep = w.x * support_dir.x + w.y * support_dir.y + w.z * support_dir.z;
        float scale = -(inflate - dot_w_sep / support_dir_sq);

        this->m_gjk_origin.x = support_dir.x * scale;
        this->m_gjk_origin.y = support_dir.y * scale;
        this->m_gjk_origin.z = support_dir.z * scale;

        if (phys_gjk_info::gjk(d, &support_dir, iters > 1) == GJK_PENETRATING)
        {
            tlWarning("gjk has possibly failed.");

            if (iters != 1)
                break;

            if (!d->gjk_ci || !d->gjk_ci->is_simplex_valid())
                break;

            d->gjk_ci->m_flags &= ~8u;

            if (phys_gjk_info::gjk(d, &support_dir, 0) == GJK_PENETRATING)
                break;
        }

        phys_gjk_info::comp_v(this->m_w_set, &support_dir);

        support_dir_sq = AbsSquared(support_dir);

        iassert(AbsSquared(support_dir) >= m_gjk_pen_thresh_sq);

        float conv = 1.0f - SEP_CONV_THRESH;
        conv *= conv;
        if (conv * this->m_lower_dist_sq < 289.0f || iters >= 10)
            return GJK_PENETRATING;
    }

    iassert(0); // "gjk has failed."
    return GJK_INVALID;
}

char phys_gjk_info::phys_collide_do_gjk_collide(const phys_gjk_input *d)
{
    double v5; // st6
    double v6; // st7
    phys_gjk_info::gjk_retval_e retv; // eax
    float v15; // [esp+14h] [ebp-2Ch]
    float v16; // [esp+14h] [ebp-2Ch]
    float v17; // [esp+14h] [ebp-2Ch]
    float v18; // [esp+14h] [ebp-2Ch]
    float v19; // [esp+18h] [ebp-28h]
    float v20; // [esp+18h] [ebp-28h]
    float v21; // [esp+18h] [ebp-28h]
    float v22; // [esp+18h] [ebp-28h]
    float v23; // [esp+1Ch] [ebp-24h]
    float v24; // [esp+1Ch] [ebp-24h]
    float v25; // [esp+1Ch] [ebp-24h]
    float v26; // [esp+1Ch] [ebp-24h]


    iassert((d->m_start_time >= 0.0f) && (d->m_start_time <= d->m_end_time) && (d->m_end_time <= 1.0f));

    this->m_flags = 0;
    phys_full_inv_multiply_mat(&this->cg2_to_cg1_xform, d->cg1_to_world_xform, d->cg2_to_world_xform);
    if ( d->m_continuous_collision )
    {
        v15 = d->m_cg1_translation.x - d->m_cg2_translation.x;
        v19 = d->m_cg1_translation.y - d->m_cg2_translation.y;
        v23 = d->m_cg1_translation.z - d->m_cg2_translation.z;
        v5 = v19;
        v6 = v15;
        v16 = d->cg1_to_world_xform->x.z * v23 + d->cg1_to_world_xform->x.x * v15 + d->cg1_to_world_xform->x.y * v19;
        v20 = d->cg1_to_world_xform->y.y * v19 + d->cg1_to_world_xform->y.x * v6 + d->cg1_to_world_xform->y.z * v23;
        v24 = v6 * d->cg1_to_world_xform->z.x + v5 * d->cg1_to_world_xform->z.y + v23 * d->cg1_to_world_xform->z.z;
        this->m_cg1_relative_translation_loc.x = v16;
        this->m_cg1_relative_translation_loc.y = v20;
        this->m_cg1_relative_translation_loc.z = v24;
        this->m_flags |= 8u;
    }

    this->m_geom_radii_sum = d->cg2_radius + d->cg1_radius;
    this->m_gjk_sep_thresh = m_geom_radii_sum + d->m_sep_thresh;

    if ( d->m_intersection_test_only )
    {
        this->m_flags |= 2u;
        this->m_gjk_pen_thresh_sq = this->m_gjk_sep_thresh * this->m_gjk_sep_thresh;
    }
    else
    {
        this->m_gjk_pen_thresh_sq = 0.0011560001;
    }

    retv = phys_gjk_info::collide(d);

    if ( retv == GJK_INVALID )
    {
        iassert(0); // "phys_gjk failed"
        if (d->gjk_ci)
        {
            d->gjk_ci->m_flags |= 1u;
            d->gjk_ci->m_flags &= ~4u;
            d->gjk_ci->m_flags &= ~8u;
        }
        return 0;
    }

    if (retv == GJK_SEPARATED)
    {
        phys_gjk_info::gjk_cache_update_test_only_valid(d);
        return 0;
    }

    if ( (this->m_flags & 2) != 0 )
    {
        iassert(retv == GJK_VALID || retv == GJK_PENETRATING);

        if (this->m_w_set == 15 )
        {
            phys_gjk_info::gjk_cache_update_invalid(d);
            return 1;
        }
        else if ( retv == GJK_VALID )
        {
            phys_gjk_info::comp_v(this->m_w_set, &this->m_support_dir);
            phys_gjk_info::gjk_cache_update_test_only_valid(d);
            return 1;
        }
        else
        {
            iassert(retv == GJK_PENETRATING);
            phys_gjk_info::gjk_cache_update_test_only_penetrating(d);
            return 1;
        }
    }
    else
    {
        iassert(retv == GJK_VALID || retv == GJK_PENETRATING);

        if (m_w_set == 15)
        {
            iassert(0); // "phys_gjk failed"

            phys_gjk_info::gjk_cache_update_invalid(d);
            return 0;
        }
        else
        {
            iassert(m_w_set < 15 && m_w_set > 0);

            phys_gjk_info::comp_closest_points(this->m_w_set, &this->cg1_cinfo_loc.m_p1, &this->cg1_cinfo_loc.m_p2);
            phys_gjk_info::comp_v(this->m_w_set, &this->cg1_cinfo_loc.m_n);

            //phys_vec3::operator/=(&this->cg1_cinfo_loc.m_n, v14);
            this->cg1_cinfo_loc.m_n /= Vec3Length(&this->cg1_cinfo_loc.m_n.x);

            this->cg1_cinfo_loc.m_p1.x -= (this->cg1_cinfo_loc.m_n.x * d->cg1_radius);
            this->cg1_cinfo_loc.m_p1.y -= (this->cg1_cinfo_loc.m_n.y * d->cg1_radius);
            this->cg1_cinfo_loc.m_p1.z -= (this->cg1_cinfo_loc.m_n.z * d->cg1_radius);

            this->cg1_cinfo_loc.m_p2.x += (this->cg1_cinfo_loc.m_n.x * d->cg2_radius);
            this->cg1_cinfo_loc.m_p2.y += (this->cg1_cinfo_loc.m_n.y * d->cg2_radius);
            this->cg1_cinfo_loc.m_p2.z += (this->cg1_cinfo_loc.m_n.z * d->cg2_radius);

            phys_gjk_info::gjk_cache_update_colliding(d);
            return 1;
        }
    }
}

int phys_gjk_info::compress_verts(char w_set)
{
    int result = 0;

    if ( (w_set & 1) != 0 )
    {
        // useless logic
        //this->m_a_verts[0].x = this->m_a_verts[0].x;
        //this->m_a_verts[0].y = this->m_a_verts[0].y;
        //this->m_a_verts[0].z = this->m_a_verts[0].z;
        //
        //this->m_b_verts[0].x = this->m_b_verts[0].x;
        //this->m_b_verts[0].y = this->m_b_verts[0].y;
        //this->m_b_verts[0].z = this->m_b_verts[0].z;
        //
        //this->m_a_inds[0].x = this->m_a_inds[0].x;
        //this->m_a_inds[0].y = this->m_a_inds[0].y;
        //this->m_a_inds[0].z = this->m_a_inds[0].z;
        //
        //this->m_b_inds[0].x = this->m_b_inds[0].x;
        //this->m_b_inds[0].y = this->m_b_inds[0].y;
        //this->m_b_inds[0].z = this->m_b_inds[0].z;

        result++;
    }

    if (w_set & 2)
    {
        this->m_a_verts[result] = this->m_a_verts[1];
        this->m_b_verts[result] = this->m_b_verts[1];
        this->m_a_inds[result] = this->m_a_inds[1];
        this->m_b_inds[result] = this->m_b_inds[1];
        result++;
    }

    if (w_set & 4)
    {
        this->m_a_verts[result] = this->m_a_verts[2];
        this->m_b_verts[result] = this->m_b_verts[2];
        this->m_a_inds[result] = this->m_a_inds[2];
        this->m_b_inds[result] = this->m_b_inds[2];
        result++;
    }

    if (w_set & 8)
    {
        this->m_a_verts[result] = this->m_a_verts[3];
        this->m_b_verts[result] = this->m_b_verts[3];
        this->m_a_inds[result] = this->m_a_inds[3];
        this->m_b_inds[result] = this->m_b_inds[3];
        result++;
    }

    return result;
}

void phys_gjk_info::comp_closest_points(int w_set, phys_vec3 *a, phys_vec3 *b)
{
    phys_gjk_info::phys_gjk_set_info *set_info; // eax
    float inv_lambda_sum; // [esp-8h] [ebp-14h]
    float lambda_sum; // [esp-4h] [ebp-10h]

    a->x = 0.0;
    a->y = 0.0;
    a->z = 0.0;

    b->x = 0.0;
    b->y = 0.0;
    b->z = 0.0;

    lambda_sum = 0.0;

    set_info = &this->m_set_list[w_set];

    if ((w_set & 1) != 0)
    {
        a->x += this->m_a_verts[0].x * set_info->m_lamda[0];
        a->y += this->m_a_verts[0].y * set_info->m_lamda[0];
        a->z += this->m_a_verts[0].z * set_info->m_lamda[0];

        b->x += this->m_b_verts[0].x * set_info->m_lamda[0];
        b->y += this->m_b_verts[0].y * set_info->m_lamda[0];
        b->z += this->m_b_verts[0].z * set_info->m_lamda[0];

        lambda_sum += set_info->m_lamda[0];
    }
    if ((w_set & 2) != 0)
    {
        a->x += this->m_a_verts[1].x * set_info->m_lamda[1];
        a->y += this->m_a_verts[1].y * set_info->m_lamda[1];
        a->z += this->m_a_verts[1].z * set_info->m_lamda[1];

        b->x += this->m_b_verts[1].x * set_info->m_lamda[1];
        b->y += this->m_b_verts[1].y * set_info->m_lamda[1];
        b->z += this->m_b_verts[1].z * set_info->m_lamda[1];

        lambda_sum += set_info->m_lamda[1];
    }

    if ((w_set & 4) != 0)
    {
        a->x += this->m_a_verts[2].x * set_info->m_lamda[2];
        a->y += this->m_a_verts[2].y * set_info->m_lamda[2];
        a->z += this->m_a_verts[2].z * set_info->m_lamda[2];

        b->x += this->m_b_verts[2].x * set_info->m_lamda[2];
        b->y += this->m_b_verts[2].y * set_info->m_lamda[2];
        b->z += this->m_b_verts[2].z * set_info->m_lamda[2];

        lambda_sum += set_info->m_lamda[2];
    }
    if ((w_set & 8) != 0)
    {
        a->x += this->m_a_verts[3].x * set_info->m_lamda[3];
        a->y += this->m_a_verts[3].y * set_info->m_lamda[3];
        a->z += this->m_a_verts[3].z * set_info->m_lamda[3];

        b->x += this->m_b_verts[3].x * set_info->m_lamda[3];
        b->y += this->m_b_verts[3].y * set_info->m_lamda[3];
        b->z += this->m_b_verts[3].z * set_info->m_lamda[3];

        lambda_sum += set_info->m_lamda[3];
    }

    iassert(lambda_sum >= 0.0f);
    inv_lambda_sum = 1.0 / lambda_sum;

    a->x = a->x * inv_lambda_sum;
    a->y = a->y * inv_lambda_sum;
    a->z = a->z * inv_lambda_sum;

    b->x = b->x * inv_lambda_sum;
    b->y = b->y * inv_lambda_sum;
    b->z = b->z * inv_lambda_sum;
}

const phys_vec3 *phys_gjk_info::get_initial_support_dir(
                phys_vec3 *result,
                const phys_gjk_input *d)
{
    phys_vec3 cg1_center_local; // [esp-3Ch] [ebp-5Ch] BYREF
    phys_vec3 cg2_center_local; // [esp-2Ch] [ebp-4Ch] BYREF
    phys_vec3 cg2_center_world; // [esp-1Ch] [ebp-3Ch] BYREF

    if (d->gjk_ci && d->gjk_ci->get_flag(phys_gjk_cache_info::phys_gjk_cache_info_e::FLAG_IS_SUPPORT_DIR_VALID))
    {
        phys_inv_multiply(result, d->cg1_to_world_xform, &d->gjk_ci->m_support_dir);
        return result;
    }

    d->gjk_cg2->get_center(&cg2_center_local);
    phys_full_multiply(&cg2_center_world, &this->cg2_to_cg1_xform, &cg2_center_local);
    d->gjk_cg1->get_center(&cg1_center_local);

    float dx = cg1_center_local.x - cg2_center_world.x;
    float dy = cg1_center_local.y - cg2_center_world.y;
    float dz = cg1_center_local.z - cg2_center_world.z;

    float dist_sq = dx * dx + dy * dy + dz * dz;

    if (dist_sq < 0.0000000099999991)
    {
        if ((pai_gjk_initial_support_dir.m_hits_total_count < pai_gjk_initial_support_dir.m_max_hits_total
            || !pai_gjk_initial_support_dir.m_max_hits_total)
            && pai_gjk_initial_support_dir.m_hits_frame_count < pai_gjk_initial_support_dir.m_max_hits_per_frame)
        {
            if (pai_gjk_initial_support_dir.m_use_warnings_only)
            {
                PHYS_WARNING("source/phys_gjk.cpp", 2039, "0", "degenerate gjk initial support dir.");
            }
            else if (_tlAssert("source/phys_gjk.cpp", 2039, "0", "degenerate gjk initial support dir."))
            {
                __debugbreak();
            }
        }
        _InterlockedExchangeAdd(&pai_gjk_initial_support_dir.m_hits_total_count, 1u);
        _InterlockedExchangeAdd(&pai_gjk_initial_support_dir.m_hits_frame_count, 1u);
        *result = PHYS_X_VEC;
        return result;
    }

    result->x = dx;
    result->y = dy;
    result->z = dz;
    result->w = 0.0f;
    return result;
}

void phys_gjk_info::gjk_cache_update_invalid(const phys_gjk_input *d)
{
    if (d->gjk_ci)
    {
        d->gjk_ci->m_flags |= 1u;
        d->gjk_ci->m_flags &= ~4u;
        d->gjk_ci->m_flags &= ~8u;
    }
}

void phys_gjk_info::gjk_cache_update_test_only_valid(const phys_gjk_input *d)
{
    if (d->gjk_ci)
    {
        d->gjk_ci->m_flags |= 1u;

        phys_vec3 world_support_dir; // [esp-10h] [ebp-1Ch] BYREF
        phys_multiply(&world_support_dir, d->cg1_to_world_xform, &this->m_support_dir);

        d->gjk_ci->m_flags |= 4u;
        d->gjk_ci->m_support_dir = world_support_dir;
        d->gjk_ci->m_flags &= ~8u;
    }
}

void phys_gjk_info::gjk_cache_update_colliding(const phys_gjk_input *d)
{
    if (d->gjk_ci)
    {
        d->gjk_ci->m_flags |= 1u;
        
        phys_vec3 world_support_dir;
        phys_multiply(&world_support_dir, d->cg1_to_world_xform, &this->cg1_cinfo_loc.m_n);

        d->gjk_ci->m_flags |= 4u;
        d->gjk_ci->m_support_dir = world_support_dir;

        phys_vec3 b_normal;
        phys_multiply(&b_normal, &this->cg2_to_cg1_xform, &this->cg1_cinfo_loc.m_n);

        phys_vec3 a_normal; 
        a_normal.x = -this->cg1_cinfo_loc.m_n.x;
        a_normal.y = -this->cg1_cinfo_loc.m_n.y;
        a_normal.z = -this->cg1_cinfo_loc.m_n.z;

        set_simplex(d->gjk_cg1, d->gjk_cg2, d->gjk_ci, &a_normal, &b_normal, this->m_a_inds, this->m_b_inds, this->m_w_set);
    }
}

void phys_gjk_info::gjk_cache_update_test_only_penetrating(const phys_gjk_input *d)
{
    if (d->gjk_ci)
    {
        d->gjk_ci->m_flags |= 1u;
        d->gjk_ci->m_flags &= ~8u;
    }
}

const phys_vec3 *phys_gjk_geom::support_only(
                const phys_vec3 *result,
                const phys_mat44 *xform,
                const phys_vec3 *v) const
{
    phys_vec3 local_dir;
    local_dir.x = xform->x.x * v->x + xform->x.y * v->y + xform->x.z * v->z;
    local_dir.y = xform->y.x * v->x + xform->y.y * v->y + xform->y.z * v->z;
    local_dir.z = xform->z.x * v->x + xform->z.y * v->y + xform->z.z * v->z;

    phys_vec3 support_vert;
    phys_vec3 __;
    this->support(local_dir, &support_vert, &__);

    phys_full_multiply((phys_vec3*)result, xform, &support_vert);
    return result;
}

void phys_gjk_info::comp_v(int w_set, phys_vec3 *v) const
{
    iassert(w_set > 0 && w_set < 15);

    phys_vec3 list_w_vert[3];
    int list_w_vert_count = 0;

    if (w_set & 1) list_w_vert[list_w_vert_count++] = this->m_w_verts[0];
    if (w_set & 2) list_w_vert[list_w_vert_count++] = this->m_w_verts[1];
    if (w_set & 4) list_w_vert[list_w_vert_count++] = this->m_w_verts[2];
    if (w_set & 8) list_w_vert[list_w_vert_count++] = this->m_w_verts[3];

    iassert(list_w_vert_count > 0 && list_w_vert_count < 4);

    if (list_w_vert_count == 1)
    {
        *v = list_w_vert[0];
        return;
    }

    phys_vec3 e01;
    e01.x = list_w_vert[1].x - list_w_vert[0].x;
    e01.y = list_w_vert[1].y - list_w_vert[0].y;
    e01.z = list_w_vert[1].z - list_w_vert[0].z;

    if (list_w_vert_count != 2)
    {
        iassert(list_w_vert_count == 3);

        float e02x = list_w_vert[2].x - list_w_vert[0].x;
        float e02y = list_w_vert[2].y - list_w_vert[0].y;
        float e02z = list_w_vert[2].z - list_w_vert[0].z;

        phys_vec3 normal;
        normal.x = e02z * e01.y - e02y * e01.z;
        normal.y = e01.z * e02x - e02z * e01.x;
        normal.z = e02y * e01.x - e01.y * e02x;

        float normal_sq = normal.y * normal.y + normal.x * normal.x + normal.z * normal.z;
        if (normal_sq > 0.01f)
        {
            float t = (normal.x * list_w_vert[0].x
                     + normal.y * list_w_vert[0].y
                     + normal.z * list_w_vert[0].z) / normal_sq;

            v->x = (normal.x * t);
            v->y = (normal.y * t);
            v->z = (normal.z * t);
            return;
        }

        phys_vec3 sides[3];
        sides[0] = e01;
        sides[1].x = list_w_vert[2].x - list_w_vert[1].x;
        sides[1].y = list_w_vert[2].y - list_w_vert[1].y;
        sides[1].z = list_w_vert[2].z - list_w_vert[1].z;
        sides[2].x = -e02x;
        sides[2].y = -e02y;
        sides[2].z = -e02z;

        float nside_sq[3];
        nside_sq[0] = sides[0].x * sides[0].x + sides[0].y * sides[0].y + sides[0].z * sides[0].z;
        nside_sq[1] = sides[1].x * sides[1].x + sides[1].y * sides[1].y + sides[1].z * sides[1].z;
        nside_sq[2] = sides[2].x * sides[2].x + sides[2].y * sides[2].y + sides[2].z * sides[2].z;


        int side_i;
        if (nside_sq[1] > nside_sq[0])
        {
            side_i = 1;
            if (nside_sq[2] <= nside_sq[1])
                goto project_edge;
        }
        else if (nside_sq[2] <= nside_sq[0])
        {
            side_i = 0;
            goto project_edge;
        }
        side_i = 2;

    project_edge:
        iassert(nside_sq[side_i] > 0.0f);

        float t = (list_w_vert[side_i].x * sides[side_i].x
            + list_w_vert[side_i].y * sides[side_i].y
            + list_w_vert[side_i].z * sides[side_i].z) / nside_sq[side_i];
        v->x = list_w_vert[side_i].x - sides[side_i].x * t;
        v->y = list_w_vert[side_i].y - sides[side_i].y * t;
        v->z = list_w_vert[side_i].z - sides[side_i].z * t;
        return;
    }

    // 2-vertex case: project origin onto line segment w[0]->w[1]
    float ne1_sq = e01.x * e01.x + e01.y * e01.y + e01.z * e01.z;
    iassert(ne1_sq > 0.0f);

    // t = dot(w[0], e01) / |e01|^2, closest = w[0] - t * e01
    float t = (e01.x * list_w_vert[0].x
        + e01.y * list_w_vert[0].y
        + e01.z * list_w_vert[0].z) / ne1_sq;
    v->x = list_w_vert[0].x - e01.x * t;
    v->y = list_w_vert[0].y - e01.y * t;
    v->z = list_w_vert[0].z - e01.z * t;
}

int phys_gjk_info::init_gjk(
                const phys_gjk_input *d,
                const phys_vec3 *initial_support_dir,
                bool in_separation_loop)
{
    int vert_count = 0;

    if (in_separation_loop)
    {
        vert_count = phys_gjk_info::compress_verts(this->m_w_set);
    }
    else
    {
        if (d->gjk_ci && d->gjk_ci->is_simplex_valid())
        {
            get_simplex(d->gjk_cg1, d->gjk_cg2, d->gjk_ci, this->m_a_verts, this->m_a_inds, this->m_b_verts, this->m_b_inds, &vert_count);

            for (int i = 0; i < vert_count; ++i)
            {
                phys_vec3 transformed;
                phys_full_multiply(&transformed, &this->cg2_to_cg1_xform, &this->m_b_verts[i]);
                this->m_b_verts[i] = transformed;
            }

        }
    }

    if (vert_count)
    {
        this->m_w_set = phys_gjk_info::seed_simplex(vert_count);
        phys_gjk_info::comp_v(this->m_w_set, &this->m_support_dir);

        this->m_last_w_set = this->m_w_set;
        return 1;
    }

    // No cached simplex
    this->m_support_dir = *initial_support_dir;

    this->m_last_w_set = 0;
    this->m_w_set = 0;
    return 0;
}

