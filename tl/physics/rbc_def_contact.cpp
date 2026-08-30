#include "rbc_def_contact.h"
#include <physics/physics_system_internal.h>
#include <physics/phys_assert.h>
#include <physics/phys_constraint_solver_multithreaded.h>

phys_assert_info pai_create_pulse_sum_contact = { 0, 2, true };



phys_transient_allocator *__cdecl contact_point_info::get_cpi_allocator()
{
  return &g_physics_system->m_contact_point_buffer_1;
}

void __thiscall rigid_body_constraint_contact::verify_constraint(
        environment_rigid_body *b1_,
        environment_rigid_body *b2_)
{
    iassert((b1 == b1_ && b2 == b2_) || (b1 == b2_ && b2 == b1_));
    
    g_physics_system->validate_member(b1_);
    g_physics_system->validate_member(b2_);

    using TI = phys_free_list<rigid_body_constraint_contact>::T_internal;
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(TI) == 0x38, "size mismatch");
#endif
    TI *ti = (TI *)((char *)this - offsetof(TI, m_data));
    PMM_VALIDATE((char *)ti, sizeof(TI), 4u);
}

void __thiscall rigid_body_constraint_contact::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t)
{
  contact_point_info *m_first; // esi

  m_first = this->m_list_contact_point_info_buffer_1.m_first;
  for ( this->m_list_contact_point_info_buffer_2.m_first = 0; m_first; m_first = m_first->m_next_link )
  {
    //contact_point_info::check_surface_properties(m_first);
    m_first->check_surface_properties();
    //pulse_sum_constraint_solver::create_pulse_sum_contact(psys, this->b1, this->b2, m_first, delta_t);
    psys->create_pulse_sum_contact(this->b1, this->b2, m_first, delta_t);
  }
}

rigid_body_constraint_contact::~rigid_body_constraint_contact()
{
  //phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::remove(&g_physics_system->m_search_tree_rbc_contact, (phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::stack_item *)&this->m_avl_key);
  g_physics_system->m_search_tree_rbc_contact.remove(this->m_avl_key);
}

//void __thiscall phys_link_list<pulse_sum_contact>::add(phys_link_list<pulse_sum_contact> *this, pulse_sum_contact *p)
//{
//  pulse_sum_contact **m_last_next_ptr; // ecx
//
//  if ( !this->m_last_next_ptr
//    && _tlAssert(
//         "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
//         230,
//         "m_last_next_ptr",
//         "") )
//  {
//    __debugbreak();
//  }
//  p->m_next_link = 0;
//  m_last_next_ptr = this->m_last_next_ptr;
//  ++this->m_alloc_count;
//  *m_last_next_ptr = p;
//  this->m_last_next_ptr = &p->m_next_link;
//}

pulse_sum_contact *__thiscall pulse_sum_constraint_solver::create_pulse_sum_contact(
        rigid_body *b1,
        rigid_body *b2,
        contact_point_info *cpi,
        float delta_t)
{
  pulse_sum_contact *result; // eax
  pulse_sum_contact *v7; // esi
  int savedregs; // [esp+10h] [ebp+0h] BYREF

  //result = (pulse_sum_contact *)phys_transient_allocator::allocate(&this->m_solver_memory_allocator, 208 * cpi->m_point_pair_count + 96, 16, 1, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  result = (pulse_sum_contact *)this->m_solver_memory_allocator.allocate(208 * cpi->m_point_pair_count + 96, 16, 1, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  v7 = result;
  if ( result )
  {
    //phys_link_list<pulse_sum_contact>::add(&this->m_list_pulse_sum_contact, result);
    this->m_list_pulse_sum_contact.add(result);
    v7->m_list_pscp = (pulse_sum_contact_point *)&v7[1];
    v7->m_list_pscp_count = cpi->m_point_pair_count;
    v7->set(b1, b2, cpi, delta_t);
    return v7;
  }
  else if ( (cpi->m_flags & 8) == 0 )
  {
    if ( (pai_create_pulse_sum_contact.m_hits_total_count < pai_create_pulse_sum_contact.m_max_hits_total
       || !pai_create_pulse_sum_contact.m_max_hits_total)
      && pai_create_pulse_sum_contact.m_hits_frame_count < pai_create_pulse_sum_contact.m_max_hits_per_frame )
    {
      if ( pai_create_pulse_sum_contact.m_use_warnings_only )
      {
        PHYS_WARNING(
          "source/rbc_def_contact.cpp",
          44,
          "psc || cpi->get_flag(contact_point_info::FLAG_NO_OVERFLOW_ERROR)",
          SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
      }
      else if ( _tlAssert(
                  "source/rbc_def_contact.cpp",
                  44,
                  "psc || cpi->get_flag(contact_point_info::FLAG_NO_OVERFLOW_ERROR)",
                  SOLVER_MEMORY_ALLOCATOR_ERROR_MSG) )
      {
        __debugbreak();
      }
    }
    _InterlockedExchangeAdd(&pai_create_pulse_sum_contact.m_hits_total_count, 1u);
    _InterlockedExchangeAdd(&pai_create_pulse_sum_contact.m_hits_frame_count, 1u);
    return 0;
  }
  return result;
}

//void __thiscall phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::remove(
//        phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor> *this,
//        phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::stack_item *key)
//{
//  phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::stack_item *v2; // ebx
//  rigid_body_constraint_contact *v3; // esi
//  rigid_body *m_node; // ecx
//  rigid_body *m_b1; // eax
//  bool v6; // cf
//  rigid_body *v7; // eax
//  bool v8; // cf
//  rigid_body_constraint_contact **v9; // esi
//  rigid_body_constraint_contact *v10; // ecx
//  rigid_body_constraint_contact **p_m_right; // eax
//  rigid_body_constraint_contact *v12; // edx
//  bool v13; // zf
//  int p_m_avl_tree_node; // ecx
//  rigid_body_constraint_contact *m_left; // eax
//  rigid_body_constraint_contact **v16; // esi
//  rigid_body_constraint_contact *v17; // eax
//  int m_balance; // ecx
//  rigid_body_constraint_contact *v19; // ecx
//  rigid_body_constraint_contact *m_right; // ecx
//  int v21; // edi
//  rigid_body_constraint_contact *v22; // eax
//  int v23; // edi
//  char v24; // al
//  rigid_body_constraint_contact *v25; // ecx
//  rigid_body_constraint_contact *v26; // ecx
//  rigid_body_constraint_contact *v27; // eax
//  int v28; // edi
//  int v29; // eax
//  phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::stack_item the_stack[32]; // [esp+Ch] [ebp-100h] BYREF
//  phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor>::stack_item *right_item; // [esp+114h] [ebp+8h]
//
//  v2 = the_stack;
//  the_stack[0].m_node = &this->m_tree_root;
//  while ( 1 )
//  {
//    while ( 1 )
//    {
//      v3 = *v2->m_node;
//      if ( !v3
//        && _tlAssert(
//             "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_avl_tree.h",
//             161,
//             "root",
//             "") )
//      {
//        __debugbreak();
//      }
//      if ( (int)(((char *)v2 - (char *)the_stack + 8) & 0xFFFFFFF8) >= 256
//        && _tlAssert(
//             "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_avl_tree.h",
//             162,
//             "cur_item + 1 - the_stack < 32",
//             "") )
//      {
//        __debugbreak();
//      }
//      m_node = (rigid_body *)key->m_node;
//      m_b1 = v3->m_avl_key.m_b1;
//      v6 = key->m_node < (rigid_body_constraint_contact **)m_b1;
//      if ( (rigid_body *)key->m_node == m_b1 )
//        v6 = (rigid_body *)key->m_child < v3->m_avl_key.m_b2;
//      if ( !v6 )
//        break;
//      v2->m_child = -1;
//      v2[1].m_node = &v3->m_avl_tree_node.m_left;
//      ++v2;
//    }
//    v7 = v3->m_avl_key.m_b1;
//    v8 = v7 < m_node;
//    if ( v7 == m_node )
//      v8 = v3->m_avl_key.m_b2 < (rigid_body *)key->m_child;
//    if ( !v8 )
//      break;
//    v2->m_child = 1;
//    v2[1].m_node = &v3->m_avl_tree_node.m_right;
//    ++v2;
//  }
//  if ( (key->m_node != (rigid_body_constraint_contact **)v3->m_avl_key.m_b1
//     || (rigid_body *)key->m_child != v3->m_avl_key.m_b2)
//    && _tlAssert(
//         "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_avl_tree.h",
//         176,
//         "key == accessor::get_avl_key(root)",
//         "") )
//  {
//    __debugbreak();
//  }
//  v9 = v2->m_node;
//  v10 = *v2->m_node;
//  p_m_right = &v10->m_avl_tree_node.m_right;
//  if ( v10->m_avl_tree_node.m_right )
//  {
//    v12 = *p_m_right;
//    v2->m_child = 1;
//    ++v2;
//    v13 = v12->m_avl_tree_node.m_left == 0;
//    v2->m_node = p_m_right;
//    right_item = v2;
//    if ( !v13 )
//    {
//      do
//      {
//        if ( (int)(((char *)v2 - (char *)the_stack + 8) & 0xFFFFFFF8) >= 256
//          && _tlAssert(
//               "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_avl_tree.h",
//               191,
//               "cur_item + 1 - the_stack < 32",
//               "") )
//        {
//          __debugbreak();
//        }
//        p_m_avl_tree_node = (int)&(*v2->m_node)->m_avl_tree_node;
//        v2->m_child = -1;
//        v2[1].m_node = (rigid_body_constraint_contact **)p_m_avl_tree_node;
//        ++v2;
//      }
//      while ( *(unsigned int *)(*(unsigned int *)p_m_avl_tree_node + 24) );
//    }
//    m_left = *v2->m_node;
//    *v2->m_node = m_left->m_avl_tree_node.m_right;
//    m_left->m_avl_tree_node.m_left = (*v9)->m_avl_tree_node.m_left;
//    m_left->m_avl_tree_node.m_right = (*v9)->m_avl_tree_node.m_right;
//    m_left->m_avl_tree_node.m_balance = (*v9)->m_avl_tree_node.m_balance;
//    right_item->m_node = &m_left->m_avl_tree_node.m_right;
//  }
//  else
//  {
//    m_left = v10->m_avl_tree_node.m_left;
//  }
//  *v9 = m_left;
//  if ( v2 > the_stack )
//  {
//    do
//    {
//      v16 = v2[-1].m_node;
//      (*v16)->m_avl_tree_node.m_balance -= v2[-1].m_child;
//      v17 = *v16;
//      m_balance = (*v16)->m_avl_tree_node.m_balance;
//      --v2;
//      if ( m_balance == -2 )
//      {
//        v19 = v17->m_avl_tree_node.m_left;
//        if ( v19->m_avl_tree_node.m_balance == 1 )
//        {
//          m_right = v19->m_avl_tree_node.m_right;
//          v17->m_avl_tree_node.m_left->m_avl_tree_node.m_right = m_right->m_avl_tree_node.m_left;
//          m_right->m_avl_tree_node.m_left = v17->m_avl_tree_node.m_left;
//          v17->m_avl_tree_node.m_left->m_avl_tree_node.m_balance += m_right->m_avl_tree_node.m_balance <= 0
//                                                                  ? 1
//                                                                  : m_right->m_avl_tree_node.m_balance - -1;
//          v21 = -v17->m_avl_tree_node.m_left->m_avl_tree_node.m_balance;
//          m_right->m_avl_tree_node.m_balance += v21 < 0 || v17->m_avl_tree_node.m_left->m_avl_tree_node.m_balance == 0
//                                              ? 1
//                                              : v21 - -1;
//          v17->m_avl_tree_node.m_left = m_right;
//        }
//        v22 = (*v16)->m_avl_tree_node.m_left;
//        (*v16)->m_avl_tree_node.m_left = v22->m_avl_tree_node.m_right;
//        v23 = v22->m_avl_tree_node.m_balance;
//        v22->m_avl_tree_node.m_right = *v16;
//        (*v16)->m_avl_tree_node.m_balance += (-v23 & ((v23 >= 0) - 1)) + 1;
//        v22->m_avl_tree_node.m_balance += (*v16)->m_avl_tree_node.m_balance <= 0
//                                        ? 1
//                                        : (*v16)->m_avl_tree_node.m_balance + 1;
//        *v16 = v22;
//        if ( v22->m_avl_tree_node.m_balance < 2u )
//          continue;
//        v24 = _tlAssert(
//                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_avl_tree.h",
//                218,
//                "accessor::get_avl_node(root)->m_balance == 0 || accessor::get_avl_node(root)->m_balance == +1",
//                "");
//      }
//      else
//      {
//        if ( m_balance != 2 )
//          continue;
//        v25 = v17->m_avl_tree_node.m_right;
//        if ( v25->m_avl_tree_node.m_balance == -1 )
//        {
//          v26 = v25->m_avl_tree_node.m_left;
//          v17->m_avl_tree_node.m_right->m_avl_tree_node.m_left = v26->m_avl_tree_node.m_right;
//          v26->m_avl_tree_node.m_right = v17->m_avl_tree_node.m_right;
//          v17->m_avl_tree_node.m_right->m_avl_tree_node.m_balance += -v26->m_avl_tree_node.m_balance < 0
//                                                                  || v26->m_avl_tree_node.m_balance == 0
//                                                                   ? 1
//                                                                   : -v26->m_avl_tree_node.m_balance + 1;
//          v26->m_avl_tree_node.m_balance += v17->m_avl_tree_node.m_right->m_avl_tree_node.m_balance <= 0
//                                          ? 1
//                                          : v17->m_avl_tree_node.m_right->m_avl_tree_node.m_balance + 1;
//          v17->m_avl_tree_node.m_right = v26;
//        }
//        v27 = (*v16)->m_avl_tree_node.m_right;
//        (*v16)->m_avl_tree_node.m_right = v27->m_avl_tree_node.m_left;
//        v28 = v27->m_avl_tree_node.m_balance;
//        v27->m_avl_tree_node.m_left = *v16;
//        (*v16)->m_avl_tree_node.m_balance += v28 <= 0 ? 1 : v28 - -1;
//        v27->m_avl_tree_node.m_balance += -(*v16)->m_avl_tree_node.m_balance < 0
//                                       || (*v16)->m_avl_tree_node.m_balance == 0
//                                        ? 1
//                                        : -(*v16)->m_avl_tree_node.m_balance - -1;
//        *v16 = v27;
//        v29 = v27->m_avl_tree_node.m_balance;
//        if ( !v29 || v29 == -1 )
//          continue;
//        v24 = _tlAssert(
//                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_avl_tree.h",
//                225,
//                "accessor::get_avl_node(root)->m_balance == 0 || accessor::get_avl_node(root)->m_balance == -1",
//                "");
//      }
//      if ( v24 )
//        __debugbreak();
//    }
//    while ( !(*v16)->m_avl_tree_node.m_balance && v2 > the_stack );
//  }
//}

