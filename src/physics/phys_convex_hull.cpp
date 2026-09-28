#include "phys_convex_hull.h"
#include <tl/tl_system.h>
#include "phys_gjk_collision_detection.h"

chull_t *chull_list;
phys_convex_hull g_hull;
phys_inplace_avl_tree<uintptr_t, generic_avl_map_node_t, generic_avl_map_node_t> entities_map;

void phys_convex_hull::compute_convex_hull(
    int max_verts,
    float min_expansion_volume_percent)
{
    phys_vec3 *v4; // eax
    phys_convex_hull::ch_triangle *v5; // eax
    const phys_vec3 *v6; // eax
    phys_convex_hull::ch_triangle *v7; // eax
    phys_convex_hull::ch_triangle *v8; // [esp-14h] [ebp-B0h]
    phys_convex_hull::ch_triangle *list_head; // [esp-Ch] [ebp-A8h]
    phys_convex_hull::ch_edge *edge; // [esp+0h] [ebp-9Ch]
    phys_convex_hull::ch_edge *edge_i; // [esp+Ch] [ebp-90h]
    float v12; // [esp+18h] [ebp-84h]
    float v13; // [esp+1Ch] [ebp-80h]
    phys_vec3 increase_percent_; // [esp+20h] [ebp-7Ch] BYREF
    const phys_vec3 *v15; // [esp+30h] [ebp-6Ch]
    phys_convex_hull::ch_triangle *m_ptr; // [esp+34h] [ebp-68h]
    float volume; // [esp+38h] [ebp-64h]
    float support_vert_dist; // [esp+3Ch] [ebp-60h]
    //phys_vec3 v19; // [esp+40h] [ebp-5Ch] BYREF
    const phys_vec3 *v20; // [esp+50h] [ebp-4Ch]
    phys_vec3 **support_vert; // [esp+54h] [ebp-48h]
    phys_convex_hull::ch_triangle *tri; // [esp+58h] [ebp-44h]
    phys_convex_hull::ch_triangle *v23; // [esp+5Ch] [ebp-40h]
    phys_convex_hull::ch_triangle *v24; // [esp+60h] [ebp-3Ch]
    phys_static_array<phys_convex_hull::ch_triangle, 256> *p_m_intermediate_triangle_list; // [esp+64h] [ebp-38h]
    phys_static_array<phys_convex_hull::ch_triangle, 256>::iterator tri_i; // [esp+68h] [ebp-34h]
    phys_convex_hull::ch_triangle *m_slot_array; // [esp+6Ch] [ebp-30h]
    float best_volume; // [esp+70h] [ebp-2Ch]
    phys_vec3 **best_vert; // [esp+74h] [ebp-28h]
    phys_convex_hull::ch_triangle *best_tri; // [esp+78h] [ebp-24h]
    int v31; // [esp+7Ch] [ebp-20h]
    int v32; // [esp+80h] [ebp-1Ch]
    int m_alloc_count; // [esp+84h] [ebp-18h]
    float total_volume; // [esp+88h] [ebp-14h]
    //phys_convex_hull *thisa; // [esp+8Ch] [ebp-10h]
    //_UNKNOWN *v36[2]; // [esp+90h] [ebp-Ch] BYREF
    //int vars0; // [esp+9Ch] [ebp+0h]
    //
    //v36[0] = a2;
    //v36[1] = (_UNKNOWN *)vars0;
    //thisa = this;
    if (max_verts < 3
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
            331,
            "max_verts >= 3",
            ""))
    {
        __debugbreak();
    }
    phys_convex_hull::init_convex_hull();
    total_volume = 0.0f;
    while (1)
    {
        m_alloc_count = this->m_convex_hull_vert_list.m_alloc_count;
        if (m_alloc_count >= max_verts)
            break;
        v32 = this->m_intermediate_triangle_list.m_alloc_count;
        if (v32 <= 0)
            break;
        v31 = this->m_intermediate_vertex_list.m_alloc_count;
        if (v31 <= 0)
            break;
        best_tri = 0;
        best_vert = 0;
        best_volume = 0.0f;
        m_slot_array = this->m_intermediate_triangle_list.m_slot_array;
        tri_i.m_ptr = m_slot_array;
        while (1)
        {
            p_m_intermediate_triangle_list = &this->m_intermediate_triangle_list;
            v24 = &this->m_intermediate_triangle_list.m_slot_array[this->m_intermediate_triangle_list.m_alloc_count];
            v23 = v24;
            if (v24 == tri_i.m_ptr)
                break;
            tri = tri_i.m_ptr;
            support_vert = phys_convex_hull::support_intermediate_verts(&tri_i.m_ptr->m_normal);
            v20 = *support_vert;
            //v4 = operator-(&v19, v20, tri->m_verts[0]);
            //support_vert_dist = phys_dot(&tri->m_normal, v4);
            phys_vec3 aids = *support_vert[0] - *tri->m_verts[0]; // having fun with operators
            support_vert_dist = phys_dot(&tri->m_normal, &aids);
            if (support_vert_dist <= 0.034000002)
            {
                v8 = tri;
                //v5 = phys_static_array<phys_convex_hull::ch_triangle, 64>::add(&this->m_convex_hull_triangle_list, 0, "phys array add overflow.");
                v5 = this->m_convex_hull_triangle_list.add(0, "phys array add overflow.");
                //phys_convex_hull::ch_triangle::operator=(v5, v8);
                *v5 = *v8;
                m_ptr = tri_i.m_ptr;
                //phys_static_array<phys_convex_hull::ch_triangle, 256>::remove_slow(&this->m_intermediate_triangle_list, tri);
                this->m_intermediate_triangle_list.remove_slow(tri);
            }
            else
            {
                volume = phys_convex_hull::calc_expansion_volume(*support_vert);
                if (volume > best_volume)
                {
                    best_tri = tri;
                    best_vert = support_vert;
                    best_volume = volume;
                }
                ++tri_i.m_ptr;
            }
        }
        if (!best_tri)
            break;
        if (!best_vert
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
                373,
                "best_vert",
                ""))
        {
            __debugbreak();
        }
        if (best_volume <= 0.0
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
                374,
                "best_volume > 0.0f",
                ""))
        {
            __debugbreak();
        }
        v15 = *best_vert;
        //v6 = operator-(&increase_percent_, v15, best_tri->m_verts[0]);
        phys_vec3 crap = *v15 - *best_tri->m_verts[0];
        //v13 = phys_dot(&best_tri->m_normal, v6);
        v13 = phys_dot(&best_tri->m_normal, &crap);
        if (v13 <= 0.034000002
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
                375,
                "best_tri->get_dist(**best_vert) > CONVEX_HULL_VERT_ADD_DIST_THRESH",
                ""))
        {
            __debugbreak();
        }
        v12 = total_volume <= 0.000099999997 ? 100000.0 : best_volume / total_volume;
        if (min_expansion_volume_percent > v12)
            break;
        total_volume = total_volume + best_volume;
        phys_convex_hull::create_edge_list(*best_vert);
        edge_i = this->m_intermediate_edge_list.m_slot_array;
        edge = &edge_i[this->m_intermediate_edge_list.m_alloc_count];
        while (edge != edge_i)
        {
            phys_convex_hull::create_intermediate_triangle(
                *best_vert,
                edge_i->m_verts[0],
                edge_i->m_verts[1]);
            ++edge_i;
        }
        phys_convex_hull::add_convex_hull_vert(best_vert);
        phys_convex_hull::remove_inside_verts();
    }
    while (this->m_intermediate_triangle_list.m_alloc_count > 0)
    {
        //list_head = phys_static_array<phys_convex_hull::ch_triangle, 256>::get_list_head(&this->m_intermediate_triangle_list);
        list_head = this->m_intermediate_triangle_list.get_list_head();
        //v7 = phys_static_array<phys_convex_hull::ch_triangle, 64>::add(&this->m_convex_hull_triangle_list, 0, "phys array add overflow.");
        v7 = this->m_convex_hull_triangle_list.add(0, "phys array add overflow.");
        //phys_convex_hull::ch_triangle::operator=(v7, list_head);
        *v7 = *list_head;
        //phys_static_array<phys_convex_hull::ch_triangle, 256>::remove_slow(&this->m_intermediate_triangle_list, list_head);
        this->m_intermediate_triangle_list.remove_slow(list_head);
    }
}

void __thiscall phys_convex_hull::add_convex_hull_vert(phys_vec3 **vert)
{
    phys_vec3 **v3; // [esp+8h] [ebp-10h]

    iassert(vert);
    //if ( !vert
    //    && _tlAssert(
    //             "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
    //             52,
    //             "vert",
    //             "") )
    //{
    //    __debugbreak();
    //}

    v3 = this->m_convex_hull_vert_list.add(0, "phys array add overflow.");
    *v3 = *vert;
    //phys_static_array<phys_vec3 *,6144>::remove(&this->m_intermediate_vertex_list, vert);

    this->m_intermediate_vertex_list.remove(vert);
}

void phys_convex_hull::create_intermediate_triangle(
    phys_vec3 *v0,
    phys_vec3 *v1,
    phys_vec3 *v2)
{
    float v5; // [esp-34h] [ebp-98h]
    float v6; // [esp-30h] [ebp-94h]
    float nnormal; // [esp-Ch] [ebp-70h]
    phys_vec3 v8; // [esp-8h] [ebp-6Ch] BYREF
    phys_vec3 v9; // [esp+8h] [ebp-5Ch] BYREF
    float v10; // [esp+1Ch] [ebp-48h]
    float v11; // [esp+20h] [ebp-44h]
    float v12; // [esp+24h] [ebp-40h]
    phys_vec3 v13; // [esp+28h] [ebp-3Ch] BYREF
    float v14; // [esp+44h] [ebp-20h]
    float v15; // [esp+48h] [ebp-1Ch]
    float v16; // [esp+4Ch] [ebp-18h]
    phys_convex_hull::ch_triangle *v17; // [esp+50h] [ebp-14h]
    phys_convex_hull *v18; // [esp+54h] [ebp-10h]
    phys_convex_hull::ch_triangle *tri; // [esp+58h] [ebp-Ch]
    //phys_convex_hull *thisa; // [esp+5Ch] [ebp-8h]
    //phys_vec3 *v0a; // [esp+64h] [ebp+0h]
    //
    //tri = a2;
    //thisa = (phys_convex_hull *)v0a;
    v18 = this;
    //v17 = phys_static_array<phys_convex_hull::ch_triangle, 256>::add(&this->m_intermediate_triangle_list, 0, "phys array add overflow.");
    v17 = this->m_intermediate_triangle_list.add(0, "phys array add overflow.");
    v17->m_verts[0] = v0;
    v17->m_verts[1] = v1;
    v17->m_verts[2] = v2;
    v16 = v2->x - v0->x;
    v15 = v2->y - v0->y;
    v14 = v2->z - v0->z;
    v13.x = v16;
    v13.y = v15;
    v13.z = v14;
    v12 = v1->x - v0->x;
    v11 = v1->y - v0->y;
    v10 = v1->z - v0->z;
    v9.x = v12;
    v9.y = v11;
    v9.z = v10;
    phys_cross(&v8, &v9, &v13);
    //nnormal = Abs(&v8.x);
    nnormal = Abs(v8);
    if (nnormal <= 0.0000099999997
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
            95,
            "nnormal > 0.00001f",
            ""))
    {
        __debugbreak();
    }
    v5 = (float)(1.0 / nnormal) * v8.y;
    v6 = (float)(1.0 / nnormal) * v8.z;
    v17->m_normal.x = (float)(1.0 / nnormal) * v8.x;
    v17->m_normal.y = v5;
    v17->m_normal.z = v6;
}

phys_vec3 **__thiscall phys_convex_hull::support_intermediate_verts(const phys_vec3 *dir)
{
    phys_vec3 **vert_i; // [esp+1Ch] [ebp-10h]
    phys_vec3 **best_vert; // [esp+24h] [ebp-8h]
    float best_dotp; // [esp+28h] [ebp-4h]

    best_vert = 0;
    best_dotp = -1000000.0;
    for (vert_i = this->m_intermediate_vertex_list.m_slot_array;
        &this->m_intermediate_vertex_list.m_slot_array[this->m_intermediate_vertex_list.m_alloc_count] != vert_i;
        ++vert_i)
    {
        if ((float)((float)((float)(dir->x * (*vert_i)->x) + (float)(dir->y * (*vert_i)->y))
            + (float)(dir->z * (*vert_i)->z)) > best_dotp)
        {
            best_vert = vert_i;
            best_dotp = (float)((float)(dir->x * (*vert_i)->x) + (float)(dir->y * (*vert_i)->y))
                + (float)(dir->z * (*vert_i)->z);
        }
    }
    if (!best_vert
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
            118,
            "best_vert",
            ""))
    {
        __debugbreak();
    }
    return best_vert;
}

void __thiscall phys_convex_hull::init_convex_hull()
{
    phys_vec3 **v1; // eax
    phys_vec3 **v2; // eax
    phys_vec3 *v3; // [esp-8h] [ebp-160h]
    phys_vec3 *v4; // [esp-8h] [ebp-160h]
    phys_vec3 *v5; // [esp-4h] [ebp-15Ch]
    phys_vec3 *v6; // [esp-4h] [ebp-15Ch]
    int m; // [esp+118h] [ebp-40h]
    int k; // [esp+120h] [ebp-38h]
    int j; // [esp+128h] [ebp-30h]
    phys_vec3 **v11; // [esp+12Ch] [ebp-2Ch]
    int i; // [esp+14Ch] [ebp-Ch]
    phys_vec3 *vert_i; // [esp+150h] [ebp-8h]
    phys_static_array<phys_vec3, 6144>::iterator vert_i_end; // [esp+154h] [ebp-4h]

    this->m_intermediate_vertex_list.clear();

    vert_i = this->m_vertex_buffer.m_slot_array;
    vert_i_end.m_ptr = &vert_i[this->m_vertex_buffer.m_alloc_count];
    while (vert_i_end.m_ptr != vert_i)
    {
        v11 = m_intermediate_vertex_list.add(0, "phys array add overflow.");
        *v11 = vert_i++;
    }

    this->m_convex_hull_vert_list.clear();
    this->m_intermediate_triangle_list.clear();
    this->m_convex_hull_triangle_list.clear();

    phys_convex_hull::calculate_initial_triangle_vertices();

    iassert(m_convex_hull_vert_list.get_count() == 3);

    //v5 = *phys_static_array<phys_vec3 *,64>::operator[](&this->m_convex_hull_vert_list, 2);
    v5 = *this->m_convex_hull_vert_list.operator[](2);
    //v3 = *phys_static_array<phys_vec3 *,64>::operator[](&this->m_convex_hull_vert_list, 1);
    v3 = *this->m_convex_hull_vert_list.operator[](1);
    //v1 = phys_static_array<phys_vec3 *,64>::operator[](&this->m_convex_hull_vert_list, 0);
    v1 = this->m_convex_hull_vert_list.operator[](0);
    phys_convex_hull::create_intermediate_triangle(*v1, v3, v5);
    //v6 = *phys_static_array<phys_vec3 *,64>::operator[](&this->m_convex_hull_vert_list, 2);
    v6 = *this->m_convex_hull_vert_list.operator[](2);
    //v4 = *phys_static_array<phys_vec3 *,64>::operator[](&this->m_convex_hull_vert_list, 0);
    v4 = *this->m_convex_hull_vert_list.operator[](0);
    //v2 = phys_static_array<phys_vec3 *,64>::operator[](&this->m_convex_hull_vert_list, 1);
    v2 = this->m_convex_hull_vert_list.operator[](1);
    phys_convex_hull::create_intermediate_triangle(*v2, v4, v6);
}

void    phys_convex_hull::calculate_initial_triangle_vertices()
{
    phys_vec3 *eax15; // eax
    phys_vec3 **v3; // [esp-Ch] [ebp-17Ch]
    int k; // [esp-8h] [ebp-178h]
    char v5; // [esp-1h] [ebp-171h]
    phys_vec3 **i; // [esp+4h] [ebp-16Ch]
    phys_vec3 **v7; // [esp+8h] [ebp-168h]
    int m_alloc_count; // [esp+10h] [ebp-160h]
    phys_vec3 v9; // [esp+14h] [ebp-15Ch] BYREF
    float v10; // [esp+30h] [ebp-140h]
    float v11; // [esp+34h] [ebp-13Ch]
    float v12; // [esp+38h] [ebp-138h]
    float *v13; // [esp+3Ch] [ebp-134h]
    phys_vec3 *v14; // [esp+40h] [ebp-130h]
    phys_vec3 v15; // [esp+44h] [ebp-12Ch] BYREF
    phys_vec3 nn; // [esp+54h] [ebp-11Ch] BYREF
    phys_vec3 v17; // [esp+64h] [ebp-10Ch] BYREF
    float v18; // [esp+80h] [ebp-F0h]
    float v19; // [esp+84h] [ebp-ECh]
    float v20; // [esp+88h] [ebp-E8h]
    float *v21; // [esp+8Ch] [ebp-E4h]
    float *v22; // [esp+90h] [ebp-E0h]
    phys_vec3 v23; // [esp+94h] [ebp-DCh] BYREF
    float v24; // [esp+ACh] [ebp-C4h]
    float v25; // [esp+B0h] [ebp-C0h]
    float v26; // [esp+B4h] [ebp-BCh]
    float *v27; // [esp+B8h] [ebp-B8h]
    float *v28; // [esp+BCh] [ebp-B4h]
    phys_vec3 **best_verts[3]; // [esp+C0h] [ebp-B0h]
    float twice_area_sq; // [esp+CCh] [ebp-A4h]
    phys_vec3 *v31; // [esp+D0h] [ebp-A0h]
    phys_vec3 v32; // [esp+D4h] [ebp-9Ch] BYREF
    phys_vec3 v33; // [esp+E4h] [ebp-8Ch] BYREF
    float v34; // [esp+100h] [ebp-70h]
    float v35; // [esp+104h] [ebp-6Ch]
    float v36; // [esp+108h] [ebp-68h]
    phys_vec3 *v37; // [esp+10Ch] [ebp-64h]
    phys_vec3 *v38; // [esp+110h] [ebp-60h]
    phys_vec3 v39; // [esp+114h] [ebp-5Ch] BYREF
    float v40; // [esp+12Ch] [ebp-44h]
    float v41; // [esp+130h] [ebp-40h]
    float v42; // [esp+134h] [ebp-3Ch]
    phys_vec3 *v43; // [esp+138h] [ebp-38h]
    phys_vec3 *v44; // [esp+13Ch] [ebp-34h]
    phys_vec3 **z2; // [esp+140h] [ebp-30h]
    int v2; // [esp+144h] [ebp-2Ch]
    phys_vec3 **z1; // [esp+148h] [ebp-28h]
    int v1; // [esp+14Ch] [ebp-24h]
    phys_vec3 **z0; // [esp+150h] [ebp-20h]
    int j; // [esp+154h] [ebp-1Ch]
    int NUM_SPHERE_VERTS; // [esp+158h] [ebp-18h]
    float largest_twice_area_sq; // [esp+15Ch] [ebp-14h]
    phys_convex_hull *thisa; // [esp+160h] [ebp-10h]

    thisa = this;
    largest_twice_area_sq = 0.0f;
    NUM_SPHERE_VERTS = this->m_intermediate_vertex_list.m_alloc_count;
    for (j = 0; j < NUM_SPHERE_VERTS - 2; ++j)
    {
        //z0 = phys_static_array<phys_vec3 *, 6144>::operator[](&thisa->m_intermediate_vertex_list, j);
        z0 = m_intermediate_vertex_list[j];
        for (v1 = j + 1; v1 < NUM_SPHERE_VERTS - 1; ++v1)
        {
            //z1 = phys_static_array<phys_vec3 *, 6144>::operator[](&thisa->m_intermediate_vertex_list, v1);
            z1 = m_intermediate_vertex_list[v1];
            for (v2 = v1 + 1; v2 < NUM_SPHERE_VERTS; ++v2)
            {
                //z2 = phys_static_array<phys_vec3 *, 6144>::operator[](&thisa->m_intermediate_vertex_list, v2);
                z2 = m_intermediate_vertex_list[v2];
                v44 = *z0;
                v43 = *z2;
                v42 = v43->x - v44->x;
                v41 = v43->y - v44->y;
                v40 = v43->z - v44->z;
                v39.x = v42;
                v39.y = v41;
                v39.z = v40;
                v38 = *z0;
                v37 = *z1;
                v36 = v37->x - v38->x;
                v35 = v37->y - v38->y;
                v34 = v37->z - v38->z;
                v33.x = v36;
                v33.y = v35;
                v33.z = v34;
                v31 = phys_cross(&v32, &v33, &v39);
                twice_area_sq = (float)((float)(v31->x * v31->x) + (float)(v31->y * v31->y)) + (float)(v31->z * v31->z);
                if (twice_area_sq > largest_twice_area_sq)
                {
                    largest_twice_area_sq = twice_area_sq;
                    best_verts[0] = z0;
                    best_verts[1] = z1;
                    best_verts[2] = z2;
                }
            }
        }
    }
    if (largest_twice_area_sq <= 0.0
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
            210,
            "largest_twice_area_sq>0.0f",
            ""))
    {
        __debugbreak();
    }
    v28 = (float *)*best_verts[0];
    v27 = (float *)*best_verts[2];
    v26 = *v27 - *v28;
    v25 = v27[1] - v28[1];
    v24 = v27[2] - v28[2];
    v23.x = v26;
    v23.y = v25;
    v23.z = v24;
    v22 = (float *)*best_verts[0];
    v21 = (float *)*best_verts[1];
    v20 = *v21 - *v22;
    v19 = v21[1] - v22[1];
    v18 = v21[2] - v22[2];
    v17.x = v20;
    v17.y = v19;
    v17.z = v18;
    phys_cross(&nn, &v17, &v23);
    v14 = phys_Unitize(&v15, &nn);
    nn.x = v14->x;
    nn.y = v14->y;
    nn.z = v14->z;
    v13 = (float *)*best_verts[0];
    v12 = *v13 + nn.x;
    v11 = v13[1] + nn.y;
    v10 = v13[2] + nn.z;
    v9.x = v12;
    v9.y = v11;
    v9.z = v10;
    //eax15 = phys_static_array<phys_vec3, 6144>::add(&thisa->m_vertex_buffer, 0, "phys array add overflow.");
    eax15 = m_vertex_buffer.add(0, "phys array add overflow.");
    //phys_vec3::operator=(eax15, &v9);
    *eax15 = v9;
    m_alloc_count = thisa->m_vertex_buffer.m_alloc_count;
    v7 = this->m_intermediate_vertex_list.add(0, "phys array add overflow.");
    //*v7 = phys_static_array<phys_vec3, 6144>::operator[](&thisa->m_vertex_buffer, m_alloc_count - 1);
    *v7 = m_vertex_buffer[m_alloc_count - 1];
    v5 = 1;
    while (v5)
    {
        v5 = 0;
        for (k = 0; k < 2; ++k)
        {
            if (best_verts[k] < best_verts[k + 1])
            {
                v3 = best_verts[k];
                best_verts[k] = best_verts[k + 1];
                best_verts[k + 1] = v3;
                v5 = 1;
            }
        }
    }

    this->add_convex_hull_vert(best_verts[0]);
    this->add_convex_hull_vert(best_verts[1]);
    this->add_convex_hull_vert(best_verts[2]);
}

double __thiscall phys_convex_hull::calc_expansion_volume(const phys_vec3 *vert)
{
    float vert_dist; // [esp+B0h] [ebp-10h]
    phys_convex_hull::ch_triangle *tri_i; // [esp+B4h] [ebp-Ch]
    phys_static_array<phys_convex_hull::ch_triangle, 256>::iterator tri_i_end; // [esp+B8h] [ebp-8h]
    float volume; // [esp+BCh] [ebp-4h]
    int savedregs; // [esp+C0h] [ebp+0h] BYREF

    volume = 0.0f;
    tri_i = this->m_intermediate_triangle_list.m_slot_array;
    tri_i_end.m_ptr = &tri_i[this->m_intermediate_triangle_list.m_alloc_count];
    while (tri_i_end.m_ptr != tri_i)
    {
        //vert_dist = phys_convex_hull::ch_triangle::get_dist(tri_i, vert);
        vert_dist = tri_i->get_dist(vert);
        if (vert_dist > 0.034000002)
            volume = phys_convex_hull::tetrahedron_volume(
                vert,
                tri_i->m_verts[0],
                tri_i->m_verts[1],
                tri_i->m_verts[2])
            + volume;
        ++tri_i;
    }
    if (volume <= 0.0
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
            277,
            "volume > 0.0f",
            ""))
    {
        __debugbreak();
    }
    return volume;
}

double __thiscall phys_convex_hull::ch_triangle::get_dist(const phys_vec3 *vert)
{
    return this->m_normal.z * (float)(vert->z - this->m_verts[0]->z)
        + this->m_normal.y * (float)(vert->y - this->m_verts[0]->y)
        + this->m_normal.x * (float)(vert->x - this->m_verts[0]->x);
}

double phys_convex_hull::tetrahedron_volume(
    const phys_vec3 *a,
    const phys_vec3 *b,
    const phys_vec3 *c,
    const phys_vec3 *d)
{
    phys_vec3 *v6; // edx
    phys_vec3 vol; // [esp+0h] [ebp-7Ch] BYREF
    float v9; // [esp+10h] [ebp-6Ch]
    float v10; // [esp+14h] [ebp-68h]
    float v11; // [esp+18h] [ebp-64h]
    float v12; // [esp+24h] [ebp-58h]
    float v13; // [esp+28h] [ebp-54h]
    float v14; // [esp+2Ch] [ebp-50h]
    phys_vec3 v15; // [esp+30h] [ebp-4Ch] BYREF
    float v16; // [esp+44h] [ebp-38h]
    float v17; // [esp+48h] [ebp-34h]
    float v18; // [esp+4Ch] [ebp-30h]
    phys_vec3 v19; // [esp+50h] [ebp-2Ch] BYREF
    float v20; // [esp+60h] [ebp-1Ch]
    float v21; // [esp+64h] [ebp-18h]
    float v22; // [esp+68h] [ebp-14h]
    phys_convex_hull *v23; // [esp+6Ch] [ebp-10h]
    //int v24; // [esp+70h] [ebp-Ch]
    //void *v25; // [esp+74h] [ebp-8h]
    //void *retaddr; // [esp+7Ch] [ebp+0h]
    //
    //v24 = a2;
    //v25 = retaddr;
    v23 = this;
    v22 = c->x - d->x;
    v21 = c->y - d->y;
    v20 = c->z - d->z;
    v19.x = v22;
    v19.y = v21;
    v19.z = v20;
    v18 = b->x - d->x;
    v17 = b->y - d->y;
    v16 = b->z - d->z;
    v15.x = v18;
    v15.y = v17;
    v15.z = v16;
    v14 = a->x - d->x;
    v13 = a->y - d->y;
    v12 = a->z - d->z;
    v9 = v14;
    v10 = v13;
    v11 = v12;
    v6 = phys_cross(&vol, &v15, &v19);
    return (float)(fabs((float)((float)(v9 * v6->x) + (float)(v10 * v6->y)) + (float)(v11 * v6->z))
        / 6.0);
}

void __thiscall phys_convex_hull::create_edge_list(const phys_vec3 *vert)
{
    phys_convex_hull::ch_triangle *m_slot_array; // [esp+34h] [ebp-20h]
    int i; // [esp+3Ch] [ebp-18h]

    this->m_intermediate_edge_list.clear();

    m_slot_array = this->m_intermediate_triangle_list.m_slot_array;
    while (&this->m_intermediate_triangle_list.m_slot_array[this->m_intermediate_triangle_list.m_alloc_count] != m_slot_array)
    {
        if ((float)((float)((float)(m_slot_array->m_normal.x * (float)(vert->x - m_slot_array->m_verts[0]->x))
            + (float)(m_slot_array->m_normal.y * (float)(vert->y - m_slot_array->m_verts[0]->y)))
            + (float)(m_slot_array->m_normal.z * (float)(vert->z - m_slot_array->m_verts[0]->z))) <= 0.034000002)
        {
            ++m_slot_array;
        }
        else
        {
            phys_convex_hull::add_triangle_edges(m_slot_array);
            //phys_static_array<phys_convex_hull::ch_triangle,256>::remove_slow(&this->m_intermediate_triangle_list, m_slot_array);
            this->m_intermediate_triangle_list.remove_slow(m_slot_array);
        }
    }
    if (this->m_intermediate_edge_list.m_alloc_count < 3
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
            301,
            "m_intermediate_edge_list.get_count() >= 3",
            ""))
    {
        __debugbreak();
    }
}

void __thiscall phys_convex_hull::add_triangle_edges(phys_convex_hull::ch_triangle *tri)
{
    phys_convex_hull::add_intermediate_edge(tri->m_verts[0], tri->m_verts[1]);
    phys_convex_hull::add_intermediate_edge(tri->m_verts[1], tri->m_verts[2]);
    phys_convex_hull::add_intermediate_edge(tri->m_verts[2], tri->m_verts[0]);
}

void __thiscall phys_convex_hull::add_intermediate_edge(phys_vec3 *v0, phys_vec3 *v1)
{
    phys_convex_hull::ch_edge *v4; // [esp+Ch] [ebp-24h]
    phys_convex_hull::ch_edge *edge_i; // [esp+28h] [ebp-8h]
    phys_convex_hull::ch_edge *edge; // [esp+2Ch] [ebp-4h]

    iassert(v0 && v1 && v0 != v1);
    //if ( (!v0 || !v1 || v0 == v1)
    //    && _tlAssert(
    //             "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_convex_hull.h",
    //             59,
    //             "v0 && v1 && v0 != v1",
    //             "") )
    //{
    //    __debugbreak();
    //}
    for (edge_i = this->m_intermediate_edge_list.m_slot_array;
        &this->m_intermediate_edge_list.m_slot_array[this->m_intermediate_edge_list.m_alloc_count] != edge_i;
        ++edge_i)
    {
        if (edge_i->m_verts[0] == v0 && edge_i->m_verts[1] == v1 || edge_i->m_verts[0] == v1 && edge_i->m_verts[1] == v0)
        {
            //phys_static_array<phys_convex_hull::ch_edge,128>::remove_slow(&this->m_intermediate_edge_list, edge_i);
            this->m_intermediate_edge_list.remove_slow(edge_i);
            return;
        }
    }

    edge = m_intermediate_edge_list.add(0, "phys array add overflow.");

    edge->m_verts[0] = v0;
    edge->m_verts[1] = v1;
}

void __thiscall phys_convex_hull::remove_inside_verts()
{
    float vert_dist; // [esp+50h] [ebp-18h]
    phys_convex_hull::ch_triangle *tri_i; // [esp+54h] [ebp-14h]
    bool inside; // [esp+5Bh] [ebp-Dh]
    phys_static_array<phys_convex_hull::ch_triangle, 256>::iterator tri_i_end; // [esp+5Ch] [ebp-Ch]
    phys_vec3 **vert_i; // [esp+64h] [ebp-4h]

    vert_i = this->m_intermediate_vertex_list.m_slot_array;
    while (&this->m_intermediate_vertex_list.m_slot_array[this->m_intermediate_vertex_list.m_alloc_count] != vert_i)
    {
        inside = 1;
        tri_i = this->m_intermediate_triangle_list.m_slot_array;
        tri_i_end.m_ptr = &tri_i[this->m_intermediate_triangle_list.m_alloc_count];
        while (tri_i_end.m_ptr != tri_i && inside)
        {
            //vert_dist = phys_convex_hull::ch_triangle::get_dist(tri_i, *vert_i);
            vert_dist = tri_i->get_dist(*vert_i);
            inside = vert_dist <= 0.034000002;
            ++tri_i;
        }
        if (inside)
        {
            //phys_static_array<phys_vec3 *, 6144>::remove(&this->m_intermediate_vertex_list, vert_i);
            this->m_intermediate_vertex_list.remove(vert_i);
        }
        else
            ++vert_i;
    }
}


chull_t *__cdecl create_chull(phys_convex_hull *pch)
{
    chull_t *hull = (chull_t *)PMM_ALLOC(sizeof(chull_t), 4u);
    hull->nverts = pch->m_convex_hull_vert_list.m_alloc_count;
    hull->verts = (phys_vec3 *)PMM_ALLOC(sizeof(phys_vec3) * hull->nverts, 16);
    hull->ninds = 3 * pch->m_convex_hull_triangle_list.m_alloc_count;
    hull->inds = (unsigned __int16 *)PMM_ALLOC((2 * hull->ninds + 3) & 0xFFFFFFFC, 4u);
    hull->next = 0;

    // copy vertex positions
    int vert_i = 0;
    phys_vec3 **vi = pch->m_convex_hull_vert_list.m_slot_array;
    phys_vec3 **vi_end = vi + pch->m_convex_hull_vert_list.m_alloc_count;

    while (vi_end != vi)
    {
        phys_vec3 *src = *vi;
        phys_vec3 *dst = &hull->verts[vert_i];

        dst->x = src->x;
        dst->y = src->y;
        dst->z = src->z;

        vi++;
        vert_i++;
    }

    int ind_i = 0;
    phys_convex_hull::ch_triangle *tri = pch->m_convex_hull_triangle_list.m_slot_array;
    phys_convex_hull::ch_triangle *tri_end = tri + pch->m_convex_hull_triangle_list.m_alloc_count;

    while (tri_end != tri)
    {
        unsigned __int16 slot_i = 0;
        phys_vec3 **slot = pch->m_convex_hull_vert_list.m_slot_array;
        phys_vec3 **slot_end = slot + pch->m_convex_hull_vert_list.m_alloc_count;

        iassert((slot_end - slot) < 100);

        while (slot_end != slot)
        {
            if (tri->m_verts[0] == *slot)
            {
                hull->inds[ind_i] = slot_i;
            }
            if (tri->m_verts[1] == *slot)
            {
                hull->inds[ind_i + 1] = slot_i;
            }
            if (tri->m_verts[2] == *slot)
            {
                hull->inds[ind_i + 2] = slot_i;
            }
            slot_i++;
            slot++;

            iassert(slot_i < 100);
        }

        ind_i += 3;
        tri++;
    }

    return hull;
}

void __cdecl free_chull(chull_t *first)
{
    chull_t *next; // [esp+Ch] [ebp-4h]

    while (first)
    {
        next = first->next;
        PMM_FREE((unsigned __int8 *)first->verts, 16 * first->nverts, 0x10u);
        PMM_FREE((unsigned __int8 *)first->inds, (2 * first->ninds + 3) & 0xFFFFFFFC, 4u);
        PMM_FREE((unsigned __int8 *)first, 0x20u, 4u);
        first = next;
    }
}

void __cdecl chull_list_add(chull_t *chull, unsigned int key)
{
    generic_avl_map_add(&entities_map, chull, key);
    chull->key = key;
    chull->next_list = chull_list;
    chull_list = chull;
}

void __cdecl free_chull_lists()
{
    chull_t *next_list; // [esp+0h] [ebp-Ch]
    chull_t *first_list; // [esp+4h] [ebp-8h]
    chull_t **prev_next; // [esp+8h] [ebp-4h]

    first_list = chull_list;
    prev_next = &chull_list;
    while (first_list)
    {
        next_list = first_list->next_list;
        if (first_list->touched == 1)
        {
            first_list->touched = 0;
            prev_next = &first_list->next_list;
        }
        else
        {
            *prev_next = next_list;
            generic_avl_map_destroy(&entities_map, first_list->key);
            free_chull(first_list);
        }
        first_list = next_list;
    }
}

chull_t *generate_brush_chull(const cbrush_t *brush)
{
    phys_vec3 *v3; // eax
    phys_vec3 v4; // [esp+14h] [ebp-2Ch] BYREF
    unsigned int vi; // [esp+2Ch] [ebp-14h]
    int i; // [esp+30h] [ebp-10h]
    //int v7; // [esp+34h] [ebp-Ch] BYREF
    //unsigned int vi; // [esp+38h] [ebp-8h]
    //unsigned int retaddr; // [esp+40h] [ebp+0h]
    //
    //v7 = a1;
    //vi = retaddr;
    if (!brush)
        return 0;

    g_hull.m_vertex_buffer.clear();

    for (vi = 0; vi < brush->numverts; ++vi)
    {
        Phys_Vec3ToNitrousVec(brush->verts[vi], &v4);
        //v3 = phys_static_array<phys_vec3, 6144>::add(&g_hull.m_vertex_buffer, 0, "phys array add overflow.");
        v3 = g_hull.m_vertex_buffer.add(0, "phys array add overflow.");
        v3->x = v4.x;
        v3->y = v4.y;
        v3->z = v4.z;
    }
    //phys_convex_hull::compute_convex_hull(&g_hull, (int)&v7, 128, 0.0);
    g_hull.compute_convex_hull(128, 0.0);
    return create_chull(&g_hull);
}

chull_t *generate_partition_chull(const CollisionAabbTree *tree)
{
    phys_vec3 *v3; // eax
    phys_vec3 v; // [esp+14h] [ebp-3Ch] BYREF
    int ind_i; // [esp+2Ch] [ebp-24h]
    int ninds; // [esp+30h] [ebp-20h]
    unsigned __int16 *inds; // [esp+34h] [ebp-1Ch]
    const float (*verts)[3]; // [esp+38h] [ebp-18h]
    const CollisionPartition *partition; // [esp+3Ch] [ebp-14h]
    int i; // [esp+40h] [ebp-10h]

    if (!tree)
        return 0;

    g_hull.m_vertex_buffer.clear();

    partition = &cm.partitions[tree->u.firstChildIndex];
    verts = cm.verts;
    inds = &cm.uinds[partition->fuind];
    ninds = partition->nuinds;
    for (ind_i = 0; ind_i < ninds; ++ind_i)
    {
        Phys_Vec3ToNitrousVec((float *)verts[inds[ind_i]], &v);
        //v3 = phys_static_array<phys_vec3, 6144>::add(&g_hull.m_vertex_buffer, 0, "phys array add overflow.");
        v3 = g_hull.m_vertex_buffer.add(0, "phys array add overflow.");
        v3->x = v.x;
        v3->y = v.y;
        v3->z = v.z;
    }
    //phys_convex_hull::compute_convex_hull(&g_hull, (int)v11, 128, 0.0);
    g_hull.compute_convex_hull(128, 0.0f);
    return create_chull(&g_hull);
}

void __cdecl generate_brushmodel_chull_r(cLeafBrushNode_s *node, chull_t **hull)
{
    chull_t *brush_chull; // eax
    int i; // [esp+Ch] [ebp-4h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if (node->leafBrushCount)
    {
        if (node->leafBrushCount > 0)
        {
            for (i = 0; i < node->leafBrushCount; ++i)
            {
                brush_chull = generate_brush_chull(&cm.brushes[node->data.leaf.brushes[i]]);
                brush_chull->next = *hull;
                *hull = brush_chull;
            }
            return;
        }
        generate_brushmodel_chull_r(node + 1, hull);
    }
    if (node->data.children.childOffset[0] || node->data.children.childOffset[1])
    {
        generate_brushmodel_chull_r(&node[node->data.children.childOffset[0]], hull);
        generate_brushmodel_chull_r(&node[node->data.children.childOffset[1]], hull);
    }
}

chull_t *__cdecl generate_brushmodel_chull(unsigned int brushmodel)
{
    const cmodel_t *model; // [esp+4h] [ebp-8h]
    chull_t *hull; // [esp+8h] [ebp-4h] BYREF

    hull = 0;
    model = CM_ClipHandleToModel(brushmodel);
    generate_brushmodel_chull_r(&cm.leafbrushNodes[model->leaf.leafBrushNode], &hull);
    return hull;
}

chull_t *__cdecl generate_collmap_chull(PhysGeomList *geomList)
{
    chull_t *brush_chull; // eax
    PhysGeomInfo *geom; // [esp+8h] [ebp-Ch]
    chull_t *first; // [esp+Ch] [ebp-8h]
    unsigned int gi; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    first = 0;
    gi = 0;
    geom = geomList->geoms;
    while (gi < geomList->count)
    {
        if (geom->brush)
        {
            brush_chull = generate_brush_chull((const cbrush_t *)geom->brush);
            brush_chull->next = first;
            first = brush_chull;
        }
        ++geom;
        ++gi;
    }
    return first;
}

chull_t *__cdecl get_collmap_chull(PhysGeomList *geomList, unsigned int key)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-Ch]
    chull_t *chull; // [esp+8h] [ebp-4h]

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && key != m_tree_root->m_avl_key)
    {
        if (key >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_collmap_chull(geomList);
        if (!chull)
            return 0;
        chull_list_add(chull, key);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1789, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}

chull_t *__cdecl get_brushmodel_chull(unsigned int brushmodel, unsigned int key)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-Ch]
    chull_t *chull; // [esp+8h] [ebp-4h]

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && key != m_tree_root->m_avl_key)
    {
        if (key >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_brushmodel_chull(brushmodel);
        if (!chull)
            return 0;
        chull_list_add(chull, key);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1810, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}

chull_t *__cdecl get_brush_chull(const cbrush_t *brush)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-10h]
    chull_t *chull; // [esp+8h] [ebp-8h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && brush != (const cbrush_t *)m_tree_root->m_avl_key)
    {
        if ((unsigned int)brush >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_brush_chull(brush);
        if (!chull)
            return 0;
        chull_list_add(chull, (unsigned int)brush);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1832, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}

chull_t *__cdecl get_partition_chull(const CollisionAabbTree *tree)
{
    generic_avl_map_node_t *m_tree_root; // [esp+0h] [ebp-10h]
    chull_t *chull; // [esp+8h] [ebp-8h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    m_tree_root = entities_map.m_tree_root;
    while (m_tree_root && tree != (const CollisionAabbTree *)m_tree_root->m_avl_key)
    {
        if ((unsigned int)tree >= m_tree_root->m_avl_key)
            m_tree_root = m_tree_root->m_avl_tree_node.m_right;
        else
            m_tree_root = m_tree_root->m_avl_tree_node.m_left;
    }
    if (m_tree_root)
    {
        chull = (chull_t *)m_tree_root->m_data;
    }
    else
    {
        chull = generate_partition_chull(tree);
        if (!chull)
            return 0;
        chull_list_add(chull, (unsigned int)tree);
    }
    if (!chull && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1854, 0, "%s", "chull"))
        __debugbreak();
    chull->touched = 1;
    return chull;
}