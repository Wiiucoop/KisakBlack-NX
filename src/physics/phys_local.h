#pragma once
#include <universal/assertive.h>
#include <cstring>

#include <Windows.h> // interlockedxchg
#include <cmath>

struct phys_vec2 // sizeof=0x8
{                                                                             // XREF: contact_manifold_mesh_point/r
                                                                                // phys_contact_manifold_process::bridge/r ...
    float x;                                                        // XREF: phys_contact_manifold::generate_convex_poly_internal(void)+59/w
    // phys_contact_manifold::generate_convex_poly_internal(void)+70/w ...
    float y;                                                        // XREF: phys_contact_manifold::generate_convex_poly_internal(void)+68/w
    // phys_contact_manifold::generate_convex_poly_internal(void)+7C/w ...
};

struct alignas(16) phys_vec3 // sizeof=0x10
{                                                                             // XREF: .data:PHYS_X_VEC/r
    float x;                                                        // XREF: gjkcc_info::update_cg(float const * const,float const * const,bool)+1F2/r
    float y;                                                        // XREF: gjkcc_info::update_cg(float const * const,float const * const,bool)+209/r
    float z;                                                        // XREF: gjkcc_info::update_cg(float const * const,float const * const,bool)+221/r
    float w;                                                        // XREF: standard_query::query(broad_phase_environment_query_input const &,broad_phase_environement_query_results *)+440/r
public:

    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }

    void SetX(const float v) { x = v; }
    void SetY(const float v) { y = v; }
    void SetZ(const float v) { z = v; }

    phys_vec3() { }
    phys_vec3(float setall)
    {
        x = setall;
        y = setall;
        z = setall;
        w = setall;
    }
    phys_vec3(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
        w = 0.0f;
    }
    phys_vec3(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    // Dot product with another vector (ignores w)
    float dot(const phys_vec3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    inline float max_component() const // trash for the ai
    {
        float m = x > y ? x : y;
        return m > z ? m : z;
    }

    inline phys_vec3 &operator*=(float d)
    {
        x *= d;
        y *= d;
        z *= d;
        return *this;
    }

    inline phys_vec3 &operator+=(const phys_vec3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    inline phys_vec3 &operator-=(const phys_vec3 &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }


    inline phys_vec3 &operator-=(const phys_vec3 *v)
    {
        x -= v->x;
        y -= v->y;
        z -= v->z;
        return *this;
    }

    inline phys_vec3 &operator/=(float d)
    {
        float d_inv = 1.0f / d;
        x *= d_inv;
        y *= d_inv;
        z *= d_inv;
        return *this;
    }

    inline phys_vec3 &operator=(const phys_vec3 &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    inline float &operator[](unsigned int i)
    {
        iassert(i < 3);
        return reinterpret_cast<float *>(this)[i];
    }

    inline const float &operator[](unsigned int i) const
    {
        iassert(i < 3);
        return reinterpret_cast<const float *>(this)[i];
    }

    inline bool operator<(const phys_vec3 &v) const
    {
        return x < v.x && y < v.y && z < v.z;
    }

    inline bool operator>(const phys_vec3 &v) const
    {
        return x > v.x && y > v.y && z > v.z;
    }

    inline bool operator<=(const phys_vec3 &v) const
    {
        return x <= v.x && y <= v.y && z <= v.z;
    }

    inline bool operator>=(const phys_vec3 &v) const
    {
        return x >= v.x && y >= v.y && z >= v.z;
    }

    inline phys_vec3 operator*(const float mul)
    {
        phys_vec3 r;
        r.x = x * mul;
        r.y = y * mul;
        r.z = z * mul;
        return r;
    }

    inline phys_vec3 operator+(const phys_vec3 &other)
    {
        phys_vec3 r;
        r.x = x + other.x;
        r.y = y + other.y;
        r.z = z + other.z;
        r.w = 0.0f;
        return r;
    }

    // negate
    inline phys_vec3 operator-() const
    {
        phys_vec3 r;
        r.x = -x;
        r.y = -y;
        r.z = -z;
        r.w = w;
        return r;
    }
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(alignof(phys_vec3) == 16, "Alignment broken"); // The physics system expects in a dozen+ places that the alignment is 16 (sizeof phys_vec3)
#endif


// KISAKTODO: operator cleanup
inline phys_vec3 operator+(const phys_vec3 &a, const phys_vec3 &b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z, 0.0f };
}

inline phys_vec3 operator-(const phys_vec3 &a, const phys_vec3 &b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z, 0.0f };
}

inline phys_vec3 operator*(const phys_vec3 &a, float d)
{
    return { a.x * d, a.y * d, a.z * d, 0.0f };
}

inline phys_vec3 operator/(const phys_vec3 &a, float d)
{
    float inv = 1.0f / d;
    return { a.x * inv, a.y * inv, a.z * inv, 0.0f };
}

struct bpei_database_id // sizeof=0x8
{                                       // XREF: broad_phase_environment_info/r
    unsigned int m_id1;                 // XREF: gjk_physics_collision_visitor::query_create_prolog(void const *)+5F/w
    unsigned int m_id2;                 // XREF: gjk_physics_collision_visitor::query_create_prolog(void const *)+65/w

    bpei_database_id()
    {
    }

    bpei_database_id(unsigned int id) // this is psycho, but it looks like what's going on
    {
        m_id1 = id;
        m_id2 = 0;
    }
};

struct __declspec(align(16)) plane_lt // sizeof=0x20
{                                       // XREF: ?calc_winding@@YAXABV?$phys_static_array@Uplane_lt@@$0CAA@@@HAAV?$phys_static_array@Vphys_vec3@@$0CAA@@@@Z/r
    phys_vec3 n;
    float d;
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
};

struct chull_t // sizeof=0x20
{
    unsigned int key;
    int nverts;
    phys_vec3 *verts;
    int ninds;
    unsigned __int16 *inds;
    chull_t *next;
    int touched;
    chull_t *next_list;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(chull_t) == 0x20);
#endif

struct minspec_mutex // sizeof=0x4
{                                       // XREF: .data:minspec_mutex g_render_mutex/r
    volatile unsigned int m_token;      // XREF: _dynamic_initializer_for__g_render_mutex__+3/w

    // aislopped function def's for correctness

    inline void Lock()
    {
        while (_InterlockedCompareExchange((volatile LONG *)&m_token, 1, 0) != 0)
            ;
        // acquire barrier � no reads/writes below can move above this
        _ReadWriteBarrier();
        MemoryBarrier();
    }

    inline void Unlock()
    {
        iassert(m_token == 1);
        // release barrier � no reads/writes above can move below this
        _ReadWriteBarrier();
        MemoryBarrier();
        LONG prev = _InterlockedCompareExchange((volatile LONG *)&m_token, 0, 1);
        iassert(prev == 1);
    }
};

struct minspec_read_write_mutex // sizeof=0x4
{                                       // XREF: phys_transient_allocator/r
    volatile unsigned int m_count;      // XREF: physics_system::time_step(float,bool)+158/w

    minspec_read_write_mutex()
    {
        m_count = 1;
    }

    // aislopped function def's for correctness
    inline void ReadLock()
    {
        unsigned int count;
        do
        {
            do
                count = m_count;
            while (count == 0); // spin while writer holds
        } while (_InterlockedCompareExchange((volatile LONG *)&m_count, count + 1, count) != (LONG)count);
        // full barrier
        _ReadWriteBarrier();
        MemoryBarrier();
    }

    void ReadUnlock()
    {
        unsigned int count;
        do
        {
            count = m_count;
            iassert(count > 1);
        } while (_InterlockedCompareExchange((volatile LONG *)&m_count, count - 1, count) != (LONG)count);
    }

    inline void WriteLock()
    {
        // must wait until exactly idle (count == 1), then CAS to 0
        while (_InterlockedCompareExchange((volatile LONG *)&m_count, 0, 1) != 1)
            ;
        // full barrier after acquisition
        _ReadWriteBarrier();
        MemoryBarrier();
    }

    void WriteUnlock()
    {
        iassert(m_count == 0);
        // full barrier before release
        _ReadWriteBarrier();
        MemoryBarrier();
        // restore to idle
        LONG prev = _InterlockedCompareExchange((volatile LONG *)&m_count, 1, 0);
        iassert(prev == 0);
    }
};

struct phys_mat44 // sizeof=0x40
{                                                                             // XREF: .data:PHYS_IDENTITY_MATRIX/r
    phys_mat44(
        const phys_vec3 *x_,
        const phys_vec3 *y_,
        const phys_vec3 *z_,
        const phys_vec3 *w_)
    {
        this->x = *x_;
        this->y = *y_;
        this->z = *z_;
        this->w = *w_;
    }

    phys_mat44()
    {

    }

    void fix_w_column()
    {
        this->x.w = 0.0f;
        this->y.w = 0.0f;
        this->z.w = 0.0f;
        this->w.w = 1.0f;
    }

    phys_mat44& operator=(const phys_mat44 *that)
    {
            this->x.x = that->x.x;
            this->x.y = that->x.y;
            this->x.z = that->x.z;
            this->y.x = that->y.x;
            this->y.y = that->y.y;
            this->y.z = that->y.z;
            this->z.x = that->z.x;
            this->z.y = that->z.y;
            this->z.z = that->z.z;
            this->w.x = that->w.x;
            this->w.y = that->w.y;
            this->w.z = that->w.z;
            return *this;
    }

    phys_mat44 &operator=(const phys_mat44 &that)
    {
        this->x.x = that.x.x;
        this->x.y = that.x.y;
        this->x.z = that.x.z;
        this->y.x = that.y.x;
        this->y.y = that.y.y;
        this->y.z = that.y.z;
        this->z.x = that.z.x;
        this->z.y = that.z.y;
        this->z.z = that.z.z;
        this->w.x = that.w.x;
        this->w.y = that.w.y;
        this->w.z = that.w.z;
        return *this;
    }
                                                                            // .data:PHYS_IDENTITY_MATRIX_0/r ...
    phys_vec3 x;                                                // XREF: gjk_cylinder_t::get_feature(phys_contact_manifold *)+63/r
    // gjk_cylinder_t::get_feature(phys_contact_manifold *)+70/r ...
    phys_vec3 y;                                                // XREF: gjk_cylinder_t::get_feature(phys_contact_manifold *)+95/o
    // SetIdentity(phys_mat44 &)+20/r ...
    phys_vec3 z;                                                // XREF: gjk_cylinder_t::get_feature(phys_contact_manifold *):loc_834A44/o
    // SetIdentity(phys_mat44 &)+3B/r ...
    phys_vec3 w; // position                                                // XREF: phys_calc_world_aabb(phys_vec3 const &,phys_vec3 const &,phys_mat44 const &,phys_vec3 *,phys_vec3 *)+10/w
    // create_obb_gjk_geom(float const (* const)[3],float const * const,float const * const,int,gjk_collision_visitor *)+10/w ...
};

inline void assert_mat44_initialized(const phys_mat44 &m)
{
    const unsigned int UNINIT = 0xCCCCCCCC;
    const unsigned int *u = reinterpret_cast<const unsigned int *>(&m);
    iassert(u[0] != UNINIT); //x.x
    iassert(u[1] != UNINIT); //x.y
    iassert(u[2] != UNINIT); //x.z

    iassert(u[4] != UNINIT); //y.x
    iassert(u[5] != UNINIT); //y.y
    iassert(u[6] != UNINIT); //y.z

    iassert(u[8] != UNINIT); //z.x
    iassert(u[9] != UNINIT); //z.y
    iassert(u[10] != UNINIT);//z.z

    iassert(u[12] != UNINIT);//w.x
    iassert(u[13] != UNINIT);//w.y
    iassert(u[14] != UNINIT);//w.z
}

template <typename T>
struct phys_link_list_base // sizeof=0x4
{                                                                             // XREF: pulse_sum_node/r
    T *m_next_link;
};

template <typename T>
struct phys_link_list // sizeof=0xC
{ 
    T *m_first;
    T **m_last_next_ptr;
    volatile unsigned int m_alloc_count;

    phys_link_list() // INLINED
    {
        clear();
    }

    void clear()
    {
        m_first = NULL;
        m_last_next_ptr = &this->m_first;
        m_alloc_count = 0;
    }

    void add(T *p)
    {
        //if (!this->m_last_next_ptr
        //    && _tlAssert(
        //        "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
        //        230,
        //        "m_last_next_ptr",
        //        ""))
        //{
        //    __debugbreak();
        //}
        iassert(m_last_next_ptr);

        p->m_next_link = NULL;
        this->m_alloc_count++;
        *this->m_last_next_ptr = p;
        this->m_last_next_ptr = &p->m_next_link;
    }

    void add_mt(T *p)
    {
        p->m_next_link = nullptr;

        InterlockedIncrement(reinterpret_cast<volatile LONG *>(&m_alloc_count));

        T **prev = (T **)InterlockedExchangePointer(
            (PVOID *)&m_last_next_ptr,
            &p->m_next_link
        );

        *prev = p;
    }
};

template <typename T>
struct phys_link_list1 //<PhysObjUserData> // sizeof=0xC
{                                       // XREF: PhysGlob/r
    //PhysObjUserData *m_first;
    //PhysObjUserData *m_last;
    T *m_first;
    T *m_last;
    int m_alloc_count;                  // XREF: Phys_CollisionCallback(void)+1C/r

    void add(T *p)
    {
        T *i; // [esp+Ch] [ebp-4h]

        for (i = this->m_first; i; i = i->m_next_link)
        {
            iassert(i != p);
            //if (i == p)
            //{
            //    if (_tlAssert("c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_local.h", 135, "i != p", ""))
            //        __debugbreak();
            //}
        }
        ++this->m_alloc_count;
        if (this->m_last)
            this->m_last->m_next_link = p;
        else
            this->m_first = p;
        this->m_last = p;
        this->m_last->m_next_link = 0;
    }

    void remove(T *p)
    {
        T *i; // [esp+10h] [ebp-8h]
        T *last_i; // [esp+14h] [ebp-4h]

        i = this->m_first;
        last_i = 0;
        while (i)
        {
            if (p == i)
            {
                --this->m_alloc_count;
                if (last_i)
                    last_i->m_next_link = i->m_next_link;
                else
                    this->m_first = i->m_next_link;
                if (i == this->m_last)
                {
                    this->m_last = last_i;
                    //iassert(!last_i || last_i->get_next_link() == NULL);
                    //if (last_i)
                    //{
                    //    if (last_i->m_next_link)
                    //    {
                    //        if (_tlAssert(
                    //            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_local.h",
                    //            160,
                    //            "!last_i || last_i->get_next_link() == NULL",
                    //            ""))
                    //        {
                    //            __debugbreak();
                    //        }
                    //    }
                    //}
                }
                return;
            }
            last_i = i;
            i = i->m_next_link;
        }
        iassert(0);
        //if (_tlAssert("c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_local.h", 165, "0", ""))
        //    __debugbreak();
    }
};

template <typename T>
struct phys_simple_link_list//<contact_point_info> // sizeof=0x4
{                                                                             // XREF: rigid_body_constraint_contact/r
                                                                                // rigid_body_constraint_contact/r
        //contact_point_info *m_first;
        T *m_first;

        struct iterator//phys_simple_link_list<contact_point_info>::iterator // sizeof=0x4
        {                                       // XREF: ?render_contact@@YAXPAVrigid_body_constraint_contact@@@Z/r
            //contact_point_info *m_ptr;
            T *m_ptr;
        };
};

template <typename T, int TABLE_SIZE>
struct minspec_hash_table//<phys_slot_pool,64> // sizeof=0x10C
{                                       // XREF: phys_memory_manager/r
    //phys_slot_pool *m_hash_table[64];
    T *m_hash_table[TABLE_SIZE];
    unsigned int m_mod;
    unsigned int m_highest_collision;
    unsigned int m_total_collisions;

    // this got totally inlined
    inline T *find(unsigned int key)
    {
        T *entry = this->m_hash_table[key % this->m_mod];

        if (entry)
        {
            while (entry->m_map_key != key)
            {
                entry = entry->m_hash_next;
                if (!entry)
                    return nullptr;
            }

            return entry;
        }

        return nullptr;
    }

    inline void add(unsigned int key, T *entry_to_add)
    {
        T *entry; // edx
        unsigned int v5; // ebx
        unsigned int i; // ecx
        T *j; // eax
        unsigned int v8; // edx
        unsigned int v9; // eax
        unsigned int v10; // edi
        unsigned int v11; // ecx
        unsigned int v12; // edx
        unsigned int v13; // eax
        unsigned int m_mod; // eax
        unsigned int k; // edi
        T *v16; // ecx
        unsigned int v17; // edx
        unsigned int collision_counts[64]; // [esp+Ch] [ebp-200h] BYREF
        T *entry_list[64]; // [esp+10Ch] [ebp-100h]
        unsigned int total_collisions; // [esp+214h] [ebp+8h]
        unsigned int mod_i; // [esp+218h] [ebp+Ch]

        //entry = this->m_hash_table[key % this->m_mod];
        //
        //if (entry)
        //{
        //    while (entry->m_map_key != key)
        //    {
        //        entry = entry->m_hash_next;
        //        if (!entry)
        //            goto LABEL_7;
        //    }
        //    //if (_tlAssert(
        //    //    "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_hash_table.h",
        //    //    38,
        //    //    "find(key) == NULL",
        //    //    ""))
        //    //{
        //    //    __debugbreak();
        //    //}
        //}

        iassert(find(key) == NULL);

    LABEL_7:
        entry_list[0] = entry_to_add;
        v5 = 1;
        for (i = 0; i < TABLE_SIZE; ++i)
        {
            for (j = this->m_hash_table[i]; j; ++v5)
            {
                entry_list[v5] = j;
                j = j->m_hash_next;
            }
            this->m_hash_table[i] = 0;
        }
        this->m_mod = v5;
        this->m_highest_collision = 100000;
        this->m_total_collisions = 100000;
        v8 = v5;
        for (mod_i = v5; v8 < TABLE_SIZE; mod_i = v8)
        {
            if (v8)
                memset(collision_counts, 0, 4 * v8);
            v9 = 0;
            v10 = 0;
            v11 = 0;
            total_collisions = 0;
            if (v5)
            {
                do
                {
                    v12 = entry_list[v11]->m_map_key % mod_i;
                    v13 = ++collision_counts[v12];
                    if (v13 > v10)
                        v10 = collision_counts[v12];
                    total_collisions += v13;
                    ++v11;
                } while (v11 < v5);
                v8 = mod_i;
                v9 = total_collisions;
            }
            if (v9 < this->m_total_collisions)
            {
                this->m_mod = v8;
                this->m_highest_collision = v10;
                this->m_total_collisions = v9;
            }
            ++v8;
        }
        m_mod = this->m_mod;
        //if ((!m_mod || m_mod >= 0x40)
        //    && _tlAssert(
        //        "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_hash_table.h",
        //        83,
        //        "m_mod > 0 && m_mod < TABLE_SIZE",
        //        ""))
        //{
        //    __debugbreak();
        //}
        iassert(m_mod > 0 && m_mod < TABLE_SIZE);

        for (k = 0; k < v5; this->m_hash_table[v17] = v16)
        {
            v16 = entry_list[k];
            v17 = v16->m_map_key % this->m_mod;
            ++k;
            v16->m_hash_next = this->m_hash_table[v17];
        }
    }
};

char *__cdecl PMM_ALLOC(unsigned int size, unsigned int alignment);
void __cdecl PMM_VALIDATE(char *ptr, unsigned int size, unsigned int alignment);
void __cdecl PMM_FREE(unsigned __int8 *ptr, unsigned int size, unsigned int alignment);

template <typename T>
struct phys_simple_allocator//<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal> // sizeof=0x4
{                                                                             // XREF: phys_heap_gjk_cache_system_avl_tree/r
        int m_count;

        phys_simple_allocator()
        {
            m_count = 0;
        }

        T *allocate()
        {
            char *slot; // [esp+18h] [ebp-4h]

            slot = PMM_ALLOC(sizeof(T), sizeof(T) % 16 == 0 ? 16 : 4);
            if (!slot)
            {
                // lwss add
                iassert(0);
                return 0;
            }
            ++this->m_count;
            memset(slot, 0, sizeof(T));
            new ((void *)slot) T();
            return (T*)slot;
        }

        void free(T *slot)
        {
            if (slot)
            {
                PMM_VALIDATE((char *)slot, sizeof(T), sizeof(T) % 16 == 0 ? 16 : 4);
                --this->m_count;
                slot->~T(); // disgusting
                PMM_FREE((unsigned __int8 *)slot, sizeof(T), sizeof(T) % 16 == 0 ? 16 : 4);
            }
        }

        const int get_count() const
        {
            return m_count;
        }
};

struct phys_memory_heap // sizeof=0x10
{                                                                             // XREF: phys_contact_manifold_process/r
        char *m_buffer_start;
        char *m_buffer_end;
        char *m_buffer_cur;
        char *m_user_start;

        //char *phys_memory_heap::fast_allocate(phys_memory_heap *this, int size, const char *error_msg);
        char *fast_allocate(int size, const char *error_msg);
        char *fast_align_start(int alignment, const char *error_msg);
        void set_buffer(char *start, int size, unsigned int alignment);
};

template <typename T, int SIZE>
struct __declspec(align(16)) phys_static_array
{
    char m_buffer[sizeof(T) * SIZE];
    T * m_slot_array;
    int m_alloc_count;

    struct iterator//phys_static_array<phys_convex_hull::ch_edge,128>::iterator // sizeof=0x4
    {                                       // XREF: ?compute_convex_hull@phys_convex_hull@@QAEXHM@Z/r
        //phys_convex_hull::ch_edge *m_ptr;   // XREF: phys_convex_hull::add_intermediate_edge(phys_vec3 *,phys_vec3 *)+4D/w
        T *m_ptr;
    };

private:
    bool is_member(T *data)
    {
        return (data >= this->m_slot_array && data < &this->m_slot_array[this->m_alloc_count]);
    }
public:

    phys_static_array()
    {
        m_slot_array = (T*)&m_buffer;
        m_alloc_count = 0;
        memset(m_buffer, 0, sizeof(m_buffer)); // lwss add
    }

    ~phys_static_array()
    {
        clear();
    }

    void clear()
    {
        for (int i = 0; i < this->m_alloc_count; i++)
        {
            this->operator[](i)->~T();
        }
        this->m_alloc_count = 0;
    }

    int get_count()
    {
        return m_alloc_count;
    }

    T *operator[](int i)
    {
        iassert(i >= 0 && i < m_alloc_count);
        return &this->m_slot_array[i];
    }

    const T *operator[](int i) const
    {
        iassert(i >= 0 && i < m_alloc_count);
        return &this->m_slot_array[i];
    }

    T *add(int no_error, const char *error_msg)
    {
        if (m_alloc_count < 512)
        {
            return &this->m_slot_array[m_alloc_count++];
        }
        else
        {
            if (!no_error)
            {
                iassert(0);
                //tlFatal(error_msg);
            }
            return NULL;
        }
    }

    T *back()
    {
        iassert(m_alloc_count > 0);

        return &this->m_slot_array[m_alloc_count - 1];
    }

    T *get_list_head()
    {
        iassert(m_alloc_count > 0);

        return this->m_slot_array;
    }

    void remove_slow(T *data)
    {
        iassert(is_member(data));

        //*data = &this->m_slot_array[--this->m_alloc_count];

        int idx = data - m_slot_array;
        int last = --m_alloc_count;

        if (idx != last)
        {
            m_slot_array[idx] = m_slot_array[last];
        }
    }

    void remove(T *data)
    {
        iassert(is_member(data));

        //*data = *phys_static_array<phys_vec3 *, 6144>::back(this);
        *data = *this->back();
        --this->m_alloc_count;
    }
};


#define PTR_LIST_SIZE 256

// whoever wrote this class is a crackhead
template <typename T>
struct phys_free_list
{
public:
    struct T_internal_base
    {
        T_internal_base *m_prev_T_internal;
        T_internal_base *m_next_T_internal;
    };
    //static_assert(sizeof(T_internal_base) == 8);

    //struct __declspec(align(16)) T_internal : T_internal_base
    struct T_internal : T_internal_base // This struct itself is not align(16), it's whatever `m_data` is
    {
        T m_data;
        int m_ptr_list_index;
    };

    struct iterator//phys_free_list<broad_phase_collision_pair>::iterator // sizeof=0x4
    {
        //phys_free_list<broad_phase_collision_pair>::T_internal_base *m_ptr;
        T_internal_base *m_ptr;

        iterator()                             : m_ptr(NULL) {}
        iterator(T_internal_base *p)           : m_ptr(p)    {}

        T   *operator*()  const { return &((T_internal *)m_ptr)->m_data; }
        T   *operator->() const { return &((T_internal *)m_ptr)->m_data; }
        iterator &operator++() { m_ptr = m_ptr->m_next_T_internal; return *this; }
        bool operator==(const iterator &rhs) const { return m_ptr == rhs.m_ptr; }
        bool operator!=(const iterator &rhs) const { return m_ptr != rhs.m_ptr; }
    };

    // lwss: add iterator (idk how accurate this is)
    iterator begin() { return iterator(m_dummy_head.m_next_T_internal); }
    iterator end()   { return iterator(&m_dummy_head); }

    T_internal_base m_dummy_head;

    int m_list_count;
    int m_list_count_high_water;

    T *m_ptr_list[PTR_LIST_SIZE];
    int m_ptr_list_count;

public:

    phys_free_list()
    {
        m_list_count = 0;
        m_list_count_high_water = 0;
        m_ptr_list_count = 0;
        m_dummy_head.m_next_T_internal = &m_dummy_head;
        m_dummy_head.m_prev_T_internal = &m_dummy_head;
    }

    // lwss add
    bool empty() const
    {
        return m_dummy_head.m_next_T_internal == &m_dummy_head;
    }
    // lwss end

    void debug_add(T_internal *T_i)
    {
        int m_list_count; // [esp+0h] [ebp-10h]

        if (this->m_list_count_high_water <= this->m_list_count)
            m_list_count = this->m_list_count;
        else
            m_list_count = this->m_list_count_high_water;
        this->m_list_count_high_water = m_list_count;
        if (this->m_ptr_list_count >= PTR_LIST_SIZE)
        {
            T_i->m_ptr_list_index = -1;
        }
        else
        {
            T_i->m_ptr_list_index = this->m_ptr_list_count;
            this->m_ptr_list[this->m_ptr_list_count++] = &T_i->m_data;
        }
    }

    T *add(int no_error, const char *error_msg)
    {
        T_internal *ptr = (T_internal *)PMM_ALLOC(sizeof(T_internal), sizeof(T_internal) % 16 == 0 ? 16 : 4);

        if (ptr)
        {
            new ((void *)&ptr->m_data) T();
            ptr->m_ptr_list_index = -1;
            ptr->m_prev_T_internal = &this->m_dummy_head;
            ptr->m_next_T_internal = this->m_dummy_head.m_next_T_internal;
            this->m_dummy_head.m_next_T_internal->m_prev_T_internal = ptr;
            this->m_dummy_head.m_next_T_internal = ptr;
            m_list_count++;
            debug_add(ptr);
            return &ptr->m_data;
        }
        else
        {
            iassert(no_error);
            return NULL;
        }
    }

    void debug_remove(T_internal *T_i)
    {
        if (T_i->m_ptr_list_index != -1)
        {
            iassert(T_i->m_ptr_list_index >= 0 && T_i->m_ptr_list_index < PTR_LIST_SIZE);
            iassert(m_ptr_list[T_i->m_ptr_list_index] == &T_i->m_data);

            --this->m_ptr_list_count;

            // update the last element's index to point to its new position
            // (it's being swapped into T_i's old slot)
            T_internal *last_ti = (T_internal *)((char *)this->m_ptr_list[this->m_ptr_list_count] - offsetof(T_internal, m_data));
            last_ti->m_ptr_list_index = T_i->m_ptr_list_index;

            // swap last element into removed element's slot
            this->m_ptr_list[T_i->m_ptr_list_index] = this->m_ptr_list[this->m_ptr_list_count];
        }
    }

    void remove(T_internal *data)
    {
        iassert(data);
        --this->m_list_count;
        debug_remove(data);
        T_internal_base *next = data->m_next_T_internal;
        T_internal_base *prev = data->m_prev_T_internal;
        prev->m_next_T_internal = next;
        next->m_prev_T_internal = prev;
        //data->m_data.~T();
        PMM_FREE((unsigned __int8 *)data, sizeof(T_internal), sizeof(T_internal) % 16 == 0 ? 16 : 4);
    }

    void remove(T *data_)
    {
        if (data_)
        {
            T_internal *ti = (T_internal *)((char *)data_ - offsetof(T_internal, m_data));
            PMM_VALIDATE((char *)ti, sizeof(T_internal), sizeof(T_internal) % 16 == 0 ? 16 : 4);
            remove(ti);
        }
    }

    void remove_all()
    {
        while ((phys_free_list<T> *)this->m_dummy_head.m_next_T_internal != this)
        {
            T_internal *data = (T_internal *)this->m_dummy_head.m_next_T_internal;
            iassert(data);
            --this->m_list_count;
            debug_remove(data);
            T_internal_base *prev = data->m_prev_T_internal;
            T_internal_base *next = data->m_next_T_internal;
            prev->m_next_T_internal = next;
            next->m_prev_T_internal = prev;
            //data->m_data.~T();
            PMM_FREE((unsigned __int8 *)data, sizeof(T_internal), sizeof(T_internal) % 16 == 0 ? 16 : 4);
        }
    }

    void ptr_array_read(T **ptr_array, int ptr_array_size)
    {
        T_internal_base *i; // eax
        iassert(ptr_array_size == m_list_count);

        for (i = this->m_dummy_head.m_next_T_internal;
            i != (T_internal_base *)this;
            ++ptr_array)
        {
            *ptr_array = (T*)&i[1];
            i = i->m_next_T_internal;
        }
    }

    void ptr_array_write(T **ptr_array, int ptr_array_size)
    {
        T **v4; // edx
        T_internal_base *p_m_avl_key; // eax
        T**v6; // esi
        T_internal_base *v7; // ecx

        iassert(ptr_array_size == m_list_count);

        if (ptr_array_size > 0)
        {
            v4 = ptr_array;
            p_m_avl_key = (T_internal_base *) & (*ptr_array)[-1].m_avl_key;
            v6 = &ptr_array[ptr_array_size - 1];
            p_m_avl_key->m_prev_T_internal = &this->m_dummy_head;
            this->m_dummy_head.m_next_T_internal = p_m_avl_key;
            if (ptr_array < v6)
            {
                do
                {
                    v7 = (T_internal_base *)v4[1];
                    ++v4;
                    p_m_avl_key->m_next_T_internal = --v7;
                    v7->m_prev_T_internal = p_m_avl_key;
                    p_m_avl_key = v7;
                } while (v4 < v6);
            }
            p_m_avl_key->m_next_T_internal = &this->m_dummy_head;
            this->m_dummy_head.m_prev_T_internal = p_m_avl_key;
        }
    }
};

inline phys_vec3 *__cdecl phys_AbsValue(phys_vec3 *result, const phys_vec3 *a)
{
    result->x = fabsf(a->x);
    result->y = fabsf(a->y);
    result->z = fabsf(a->z);

    return result;
}

template <typename T>
struct phys_inplace_avl_tree_node // sizeof=0xC
{                                       // XREF: auto_rigid_body/r
    T *m_left;
    T *m_right;
    int m_balance;
};

// aislop for avl nodes
// -----------------------------------------------------------------------
// Default accessor helper � requires T2 to have m_avl_tree_node
// Specialize for types that use a different field name (e.g. generic_avl_map_node_t)
// -----------------------------------------------------------------------
template<typename T2>
struct phys_avl_node_accessor
{
    static phys_inplace_avl_tree_node<T2> *get(T2 *n) { return &n->m_avl_tree_node; }
    static const phys_inplace_avl_tree_node<T2> *get(const T2 *n) { return &n->m_avl_tree_node; }
};

template<typename T1, typename T2, typename Accessor>
struct phys_inplace_avl_tree
{
    struct stack_item
    {
        T2 **m_node;
        int  m_child;
    };

    T2 *m_tree_root;

    phys_inplace_avl_tree() { m_tree_root = nullptr; }

    // Shorthand � routes all node pointer access through the specializable accessor
    static phys_inplace_avl_tree_node<T2> *ni(T2 *n) { return phys_avl_node_accessor<T2>::get(n); }

    /* ------------------------------------------------------------ */
    /* rotations                                                     */
    /* ------------------------------------------------------------ */

    inline void rotate_left(T2 **root)
    {
        T2 *save_right = ni(*root)->m_right;

        ni(*root)->m_right = ni(save_right)->m_left;
        ni(save_right)->m_left = *root;

        int bal = ni(save_right)->m_balance;
        ni(*root)->m_balance -= (bal <= 0 ? 0 : bal) + 1;

        int adj = ni(*root)->m_balance >= 0 ? 0 : -ni(*root)->m_balance;
        ni(save_right)->m_balance -= adj + 1;

        *root = save_right;
    }

    inline void rotate_right(T2 **root)
    {
        T2 *save_left = ni(*root)->m_left;

        ni(*root)->m_left = ni(save_left)->m_right;
        ni(save_left)->m_right = *root;

        int bal = ni(save_left)->m_balance;
        ni(*root)->m_balance += (bal >= 0 ? 0 : -bal) + 1;

        int adj = ni(*root)->m_balance <= 0 ? 0 : ni(*root)->m_balance;
        ni(save_left)->m_balance += adj + 1;

        *root = save_left;
    }

    /* ------------------------------------------------------------ */
    /* find                                                          */
    /* ------------------------------------------------------------ */

    inline T2 *find(const T1 &key) const
    {
        T2 *cur = m_tree_root;
        while (cur)
        {
            if (Accessor::equals(cur, key))
                break;
            if (Accessor::less(key, cur))
                cur = ni(cur)->m_left;
            else
                cur = ni(cur)->m_right;
        }
        return cur;
    }

    /* ------------------------------------------------------------ */
    /* add                                                           */
    /* ------------------------------------------------------------ */

    inline void add(const T1 &key, T2 *data)
    {
        stack_item the_stack[32];
        stack_item *cur_item = the_stack;

        cur_item->m_node = &m_tree_root;

        while (*cur_item->m_node)
        {
            T2 *root = *cur_item->m_node;

            iassert(cur_item + 1 - the_stack < 32,
                "phys_avl_tree.h", 103, "cur_item + 1 - the_stack < 32");

            stack_item *next_item = cur_item + 1;

            if (Accessor::less(key, root))
            {
                cur_item->m_child = -1;
                next_item->m_node = &ni(root)->m_left;
            }
            else
            {
                iassert(Accessor::less(root, key),
                    "phys_avl_tree.h", 112, "key > accessor::get_avl_key(root)");

                cur_item->m_child = +1;
                next_item->m_node = &ni(root)->m_right;
            }

            cur_item = next_item;
        }

        // Link the new node
        *cur_item->m_node = data;
        ni(data)->m_left = nullptr;
        ni(data)->m_right = nullptr;
        ni(data)->m_balance = 0;
        Accessor::set_key(data, key);

        // Rebalance upward � continue while balance != 0 (subtree grew)
        do
        {
            if (cur_item <= the_stack)
                break;

            --cur_item;
            T2 **m_node = cur_item->m_node;
            ni(*m_node)->m_balance += cur_item->m_child;

            if (ni(*m_node)->m_balance == -2)
            {
                iassert(ni(ni(*m_node)->m_left)->m_balance == -1
                    || ni(ni(*m_node)->m_left)->m_balance == +1,
                    "phys_avl_tree.h", 130,
                    "accessor::get_avl_node(accessor::get_avl_node(root)->m_left)->m_balance == -1 || ...== 1");

                if (ni(ni(*m_node)->m_left)->m_balance == 1)
                    rotate_left(&ni(*m_node)->m_left);
                rotate_right(m_node);

                iassert(ni(*m_node)->m_balance == 0,
                    "phys_avl_tree.h", 134, "accessor::get_avl_node(root)->m_balance == 0");
            }
            else if (ni(*m_node)->m_balance == 2)
            {
                iassert(ni(ni(*m_node)->m_right)->m_balance == -1
                    || ni(ni(*m_node)->m_right)->m_balance == +1,
                    "phys_avl_tree.h", 138,
                    "accessor::get_avl_node(accessor::get_avl_node(root)->m_right)->m_balance == -1 || ...== 1");

                if (ni(ni(*m_node)->m_right)->m_balance == -1)
                    rotate_right(&ni(*m_node)->m_right);
                rotate_left(m_node);

                iassert(ni(*m_node)->m_balance == 0,
                    "phys_avl_tree.h", 142, "accessor::get_avl_node(root)->m_balance == 0");
            }
        } while (ni(*cur_item->m_node)->m_balance != 0);
    }

    /* ------------------------------------------------------------ */
    /* remove                                                        */
    /* ------------------------------------------------------------ */

    inline void remove(const T1 &key)
    {
        stack_item the_stack[32];
        stack_item *cur_item = the_stack;

        cur_item->m_node = &m_tree_root;

        // Find the node
        while (true)
        {
            T2 *root = *cur_item->m_node;

            iassert(root != nullptr,
                "phys_avl_tree.h", 161, "root");
            iassert(cur_item + 1 - the_stack < 32,
                "phys_avl_tree.h", 162, "cur_item + 1 - the_stack < 32");

            stack_item *next_item = cur_item + 1;

            if (Accessor::less(key, root))
            {
                cur_item->m_child = -1;
                next_item->m_node = &ni(root)->m_left;
                cur_item = next_item;
            }
            else if (Accessor::less(root, key))
            {
                cur_item->m_child = +1;
                next_item->m_node = &ni(root)->m_right;
                cur_item = next_item;
            }
            else
            {
                iassert(Accessor::equals(root, key),
                    "phys_avl_tree.h", 176, "key == accessor::get_avl_key(root)");
                break;
            }
        }

        T2 **node_to_be_removed = cur_item->m_node;

        if (ni(*node_to_be_removed)->m_right)
        {
            // Replace with in-order successor
            cur_item->m_child = +1;
            ++cur_item;
            cur_item->m_node = &ni(*node_to_be_removed)->m_right;

            stack_item *right_item = cur_item;

            while (ni(*cur_item->m_node)->m_left)
            {
                iassert(cur_item + 1 - the_stack < 32,
                    "phys_avl_tree.h", 191, "cur_item + 1 - the_stack < 32");

                stack_item *next_item = cur_item + 1;
                cur_item->m_child = -1;
                next_item->m_node = &ni(*cur_item->m_node)->m_left;
                cur_item = next_item;
            }

            T2 *replace_node = *cur_item->m_node;

            // Unlink successor � splice out replace_node, promoting its right child
            *cur_item->m_node = ni(replace_node)->m_right;

            // Copy victim's linkage to replace_node
            ni(replace_node)->m_left = ni(*node_to_be_removed)->m_left;
            ni(replace_node)->m_right = ni(*node_to_be_removed)->m_right;
            ni(replace_node)->m_balance = ni(*node_to_be_removed)->m_balance;

            // Fix right_item so rebalance walks through replace_node's right subtree
            right_item->m_node = &ni(replace_node)->m_right;

            // Publish replace_node into the tree
            *node_to_be_removed = replace_node;
        }
        else
        {
            // No right child � promote left child directly
            *node_to_be_removed = ni(*node_to_be_removed)->m_left;
        }

        // Rebalance upward � continue while balance == 0 (subtree shrank)
        do
        {
            if (cur_item <= the_stack)
                break;

            --cur_item;
            T2 **m_node = cur_item->m_node;
            ni(*m_node)->m_balance -= cur_item->m_child;

            if (ni(*m_node)->m_balance == -2)
            {
                if (ni(ni(*m_node)->m_left)->m_balance == 1)
                    rotate_left(&ni(*m_node)->m_left);
                rotate_right(m_node);

                iassert((unsigned int)ni(*m_node)->m_balance < 2u,
                    "phys_avl_tree.h", 218,
                    "accessor::get_avl_node(root)->m_balance == 0 || accessor::get_avl_node(root)->m_balance == +1");

                if (ni(*m_node)->m_balance != 0)
                    break;
            }
            else if (ni(*m_node)->m_balance == 2)
            {
                if (ni(ni(*m_node)->m_right)->m_balance == -1)
                    rotate_right(&ni(*m_node)->m_right);
                rotate_left(m_node);

                iassert(ni(*m_node)->m_balance == 0 || ni(*m_node)->m_balance == -1,
                    "phys_avl_tree.h", 225,
                    "accessor::get_avl_node(root)->m_balance == 0 || accessor::get_avl_node(root)->m_balance == -1");

                if (ni(*m_node)->m_balance != 0)
                    break;
            }
            else if (ni(*m_node)->m_balance != 0)
            {
                break; // �1, height unchanged
            }
            // 0 � subtree shrank, continue up
        } while (ni(*cur_item->m_node)->m_balance == 0);
    }
};

struct __declspec(align(16)) cached_query_info_t // sizeof=0x30
{                                       // XREF: gjk_query_output/r
    phys_vec3 m_query_aabb_min;
    phys_vec3 m_query_aabb_max;
    int m_query_contents;
    unsigned int m_query_flags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    inline bool is_empty()
    {
        if (this->m_query_contents)
            return false;

        if (this->m_query_flags)
        {
            if (!Assert_MyHandler(
                "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../physics/phys_gjk_collision_detection.h",
                212,
                0,
                "%s",
                "m_query_flags == 0"))
                __debugbreak();
        }

        return true;
    }

    bool aabb_is_valid();
    void init_query(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max,
        const phys_vec3 *extra,
        int query_contents,
        unsigned int query_flags);

    void add_query(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max,
        const phys_vec3 *extra,
        int query_contents,
        unsigned int query_flags);
    bool is_subset(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max,
        int query_contents,
        unsigned int query_flags);
    bool is_subset_aabb(
        const phys_vec3 *query_aabb_min,
        const phys_vec3 *query_aabb_max);
};

struct __declspec(align(2)) geom_plane // sizeof=0x30
{
    phys_vec3 m_normal;
    phys_vec3 m_arm;
    float m_d;
    float m_lambda;
    float m_right_side;
    bool m_active;
    bool m_walkable;
    bool m_no_push_out;
    // padding byte
};

struct Phys_UnitQuaternion // sizeof=0x10
{                                       // XREF: ?calc_velocities@nuge@@SAXABVphys_mat44@@0MPAVphys_vec3@@1@Z/r
    float x;
    float y;
    float z;
    float w;
};

inline const phys_vec3 *__cdecl phys_min(phys_vec3 *result, const phys_vec3 *v1, const phys_vec3 *v2)
{
    float x; // [esp+0h] [ebp-24h]
    float y; // [esp+4h] [ebp-20h]
    float z; // [esp+8h] [ebp-1Ch]

    if (v2->z < v1->z)
        z = v2->z;
    else
        z = v1->z;
    if (v2->y < v1->y)
        y = v2->y;
    else
        y = v1->y;
    if (v2->x < v1->x)
        x = v2->x;
    else
        x = v1->x;
    result->x = x;
    result->y = y;
    result->z = z;
    return result;
}

inline const phys_vec3 *__cdecl phys_max(phys_vec3 *result, const phys_vec3 *v1, const phys_vec3 *v2)
{
    float x; // [esp+0h] [ebp-24h]
    float y; // [esp+4h] [ebp-20h]
    float z; // [esp+8h] [ebp-1Ch]

    if (v1->z < v2->z)
        z = v2->z;
    else
        z = v1->z;
    if (v1->y < v2->y)
        y = v2->y;
    else
        y = v1->y;
    if (v1->x < v2->x)
        x = v2->x;
    else
        x = v1->x;
    result->x = x;
    result->y = y;
    result->z = z;
    return result;
}

inline const phys_vec3 *__cdecl phys_multiply(phys_vec3 *result, const phys_mat44 *mat, const phys_vec3 *v)
{
    // x*x.x + y*y.x + z*z.x
    result->x = ((v->x * mat->x.x) + (v->y * mat->y.x)) + (v->z * mat->z.x);
    // x*x.y + y*y.y + z*z.y
    result->y = ((v->x * mat->x.y) + (v->y * mat->y.y)) + (v->z * mat->z.y);
    // x*x.z + y*y.z + z*z.z
    result->z = ((v->x * mat->x.z) + (v->y * mat->y.z)) + (v->z * mat->z.z);

    return result;
}

inline const phys_vec3 *phys_full_multiply(
    phys_vec3 *result,
    const phys_mat44 *mat,
    const phys_vec3 *v)
{
    const phys_vec3 *v7; // [esp-24h] [ebp-30h]
    phys_vec3 v8; // [esp-20h] [ebp-2Ch] BYREF

    assert_mat44_initialized(*mat);

    v7 = phys_multiply(&v8, mat, v);
    result->x = v7->x + mat->w.x;
    result->y = v7->y + mat->w.y;
    result->z = v7->z + mat->w.z;
    return result;
}

inline float phys_dot(const phys_vec3 *a, const phys_vec3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

inline void __cdecl phys_transpose(phys_mat44 *dest, const phys_mat44 *source);
inline void phys_calc_world_aabb(
    const phys_vec3 *local_center,
    const phys_vec3 *local_half_aabb_dims,
    const phys_mat44 *local_to_world_xform,
    phys_vec3 *aabb_min,
    phys_vec3 *aabb_max)
{
    phys_vec3 transformedCenter; // [esp-Ch] [ebp-CCh] BYREF
    float center[3]; // [esp+4h] [ebp-BCh]
    phys_vec3 transformedX; // [esp+24h] [ebp-9Ch] BYREF
    phys_vec3 transformedY; // [esp+44h] [ebp-7Ch] BYREF
    phys_vec3 transformedZ; // [esp+64h] [ebp-5Ch] BYREF
    phys_mat44 xform; // [esp+74h] [ebp-4Ch] OVERLAPPED BYREF

    phys_transpose(&xform, local_to_world_xform);

    phys_AbsValue(&transformedX, &xform.x);
    phys_AbsValue(&transformedZ, &xform.z);
    phys_AbsValue(&transformedY, &xform.y);

    center[0] = phys_dot(&transformedX, local_half_aabb_dims);
    center[1] = phys_dot(&transformedY, local_half_aabb_dims);
    center[2] = phys_dot(&transformedZ, local_half_aabb_dims);

    phys_full_multiply(&transformedCenter, local_to_world_xform, local_center);

    aabb_min->x = transformedCenter.x - center[0];
    aabb_min->y = transformedCenter.y - center[1];
    aabb_min->z = transformedCenter.z - center[2];

    aabb_max->x = transformedCenter.x + center[0];
    aabb_max->y = transformedCenter.y + center[1];
    aabb_max->z = transformedCenter.z + center[2];
}

inline void __cdecl phys_transpose(phys_mat44 *dest, const phys_mat44 *source)
{
    assert_mat44_initialized(*source);

    if (dest == source)
    {
        float tmp;

        tmp = dest->x.y;
        dest->x.y = dest->y.x;
        dest->y.x = tmp;

        tmp = dest->x.z;
        dest->x.z = dest->z.x;
        dest->z.x = tmp;

        tmp = dest->y.z;
        dest->y.z = dest->z.y;
        dest->z.y = tmp;
    }
    else
    {
        dest->x.x = source->x.x;
        dest->x.y = source->y.x;
        dest->x.z = source->z.x;

        dest->y.x = source->x.y;
        dest->y.y = source->y.y;
        dest->y.z = source->z.y;

        dest->z.x = source->x.z;
        dest->z.y = source->y.z;
        dest->z.z = source->z.z;
    }
}

inline const phys_vec3 *__cdecl phys_inv_multiply(phys_vec3 *result, const phys_mat44 *mat, const phys_vec3 *v)
{
    float v4; // [esp+4h] [ebp-10h]
    float v5; // [esp+Ch] [ebp-8h]

    assert_mat44_initialized(*mat);

    v5 = (float)((float)(v->x * mat->z.x) + (float)(v->y * mat->z.y)) + (float)(v->z * mat->z.z);
    v4 = (float)((float)(v->x * mat->y.x) + (float)(v->y * mat->y.y)) + (float)(v->z * mat->y.z);
    result->x = (float)((float)(v->x * mat->x.x) + (float)(v->y * mat->x.y)) + (float)(v->z * mat->x.z);
    result->y = v4;
    result->z = v5;
    return result;
}

inline phys_vec3 *phys_full_inv_multiply(
    phys_vec3 *result,
    const phys_mat44 *mat,
    const phys_vec3 *v)
{
    phys_vec3 v5; // [esp-20h] [ebp-2Ch] BYREF
    float v6; // [esp-10h] [ebp-1Ch]
    float v7; // [esp-Ch] [ebp-18h]
    float v8; // [esp-8h] [ebp-14h]
    const phys_vec3 *p_w; // [esp-4h] [ebp-10h]
    //int v10; // [esp+0h] [ebp-Ch]
    //void *v11; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v10 = a1;
    //v11 = retaddr;

    assert_mat44_initialized(*mat);

    p_w = &mat->w;
    v8 = v->x - mat->w.x;
    v7 = v->y - mat->w.y;
    v6 = v->z - mat->w.z;
    v5.x = v8;
    v5.y = v7;
    v5.z = v6;
    phys_inv_multiply(result, mat, &v5);
    return result;
}

inline void __cdecl phys_calc_local_aabb(
    const phys_vec3 *aabb_min,
    const phys_vec3 *aabb_max,
    const phys_mat44 *local_to_world_xform,
    phys_vec3 *local_aabb_min,
    phys_vec3 *local_aabb_max)
{
    const phys_vec3 *v5; // eax
    const phys_vec3 *v6; // eax
    const phys_vec3 *v7; // eax
    float v8; // [esp+1F4h] [ebp-13Ch]
    float v9; // [esp+1F8h] [ebp-138h]
    float v10; // [esp+214h] [ebp-11Ch]
    float v11; // [esp+218h] [ebp-118h]
    phys_vec3 v12; // [esp+230h] [ebp-100h] BYREF
    phys_vec3 v; // [esp+240h] [ebp-F0h] BYREF
    float v14; // [esp+254h] [ebp-DCh]
    float v15; // [esp+258h] [ebp-D8h]
    float v16; // [esp+25Ch] [ebp-D4h]
    float v17; // [esp+260h] [ebp-D0h]
    float v18; // [esp+264h] [ebp-CCh]
    float v19; // [esp+268h] [ebp-C8h]
    float v20; // [esp+274h] [ebp-BCh]
    float v21; // [esp+278h] [ebp-B8h]
    float v22; // [esp+27Ch] [ebp-B4h]
    float v23; // [esp+280h] [ebp-B0h]
    float v24; // [esp+284h] [ebp-ACh]
    float v25; // [esp+288h] [ebp-A8h]
    float v26; // [esp+29Ch] [ebp-94h]
    phys_vec3 v27; // [esp+2A0h] [ebp-90h] BYREF
    float v28; // [esp+2BCh] [ebp-74h]
    phys_vec3 v29; // [esp+2C0h] [ebp-70h] BYREF
    float v30; // [esp+2DCh] [ebp-54h]
    phys_vec3 result; // [esp+2E0h] [ebp-50h] BYREF
    phys_vec3 b; // [esp+2F0h] [ebp-40h] BYREF
    float v33; // [esp+304h] [ebp-2Ch]
    float v34; // [esp+308h] [ebp-28h]
    float v35; // [esp+30Ch] [ebp-24h]
    float v36; // [esp+310h] [ebp-20h]
    float v37; // [esp+314h] [ebp-1Ch]
    float v38; // [esp+318h] [ebp-18h]
    float v39; // [esp+324h] [ebp-Ch]
    float v40; // [esp+328h] [ebp-8h]
    float v41; // [esp+32Ch] [ebp-4h]
    int savedregs; // [esp+330h] [ebp+0h] BYREF

    assert_mat44_initialized(*local_to_world_xform);

    v41 = aabb_max->x - aabb_min->x;
    v40 = aabb_max->y - aabb_min->y;
    v39 = aabb_max->z - aabb_min->z;
    v36 = v41;
    v37 = v40;
    v38 = v39;
    v35 = 0.5 * v41;
    v34 = 0.5 * v40;
    v33 = 0.5 * v39;
    b.x = 0.5 * v41;
    b.y = 0.5 * v40;
    b.z = 0.5 * v39;
    v5 = phys_AbsValue(&result, &local_to_world_xform->z);
    v30 = phys_dot(v5, &b);
    v6 = phys_AbsValue(&v29, &local_to_world_xform->y);
    v28 = phys_dot(v6, &b);
    v7 = phys_AbsValue(&v27, &local_to_world_xform->x);
    v26 = phys_dot(v7, &b);
    v23 = v26;
    v24 = v28;
    v25 = v30;
    v22 = aabb_min->x + aabb_max->x;
    v21 = aabb_min->y + aabb_max->y;
    v20 = aabb_min->z + aabb_max->z;
    v17 = v22;
    v18 = v21;
    v19 = v20;
    v16 = 0.5 * v22;
    v15 = 0.5 * v21;
    v14 = 0.5 * v20;
    v.x = 0.5 * v22;
    v.y = 0.5 * v21;
    v.z = 0.5 * v20;
    phys_full_inv_multiply(&v12, local_to_world_xform, &v);
    v10 = v12.y - v24;
    v11 = v12.z - v25;
    local_aabb_min->x = v12.x - v23;
    local_aabb_min->y = v10;
    local_aabb_min->z = v11;
    v8 = v12.y + v24;
    v9 = v12.z + v25;
    local_aabb_max->x = v12.x + v23;
    local_aabb_max->y = v8;
    local_aabb_max->z = v9;
}

void phys_multiply_mat(phys_mat44 *dest, const phys_mat44 *left, const phys_mat44 *right);
phys_vec3 *phys_cross(phys_vec3 *result, const phys_vec3 *a, const phys_vec3 *b);
void phys_full_multiply_mat(phys_mat44 *dest, const phys_mat44 *left, const phys_mat44 *right);
void Phys_Mat4ToNitrousMat(float (*inMat)[3], phys_mat44 *outMat);
void Phys_NitrousMat44ToVec33(const phys_mat44 *inMat, float (*outAxis)[3]);

void make_rotate(
    phys_mat44 *mat,
    const phys_vec3 *v,
    float theta_factor,
    float max_rotation_radians);
//void    make_rotate(
//    phys_mat44 *mat,
//    const phys_vec3 *v,
//    float theta_factor,
//    float max_rotation_radians);

void __cdecl make_rotate(phys_mat44 &m, const phys_vec3 *u, float ca, float sa);
//void __cdecl make_rotate(phys_mat44 *m, const phys_vec3 *u, float ca, float sa);

void make_rotate(phys_mat44 *mat, const phys_vec3 *v1, const phys_vec3 *v2);
//void make_rotate(phys_mat44 *mat, const phys_vec3 *v1, const phys_vec3 *v2);


// dumb
inline void Phys_NitrousVecToVec3(const phys_vec3 *inVector, float *outVector)
{
    outVector[0] = inVector->x;
    outVector[1] = inVector->y;
    outVector[2] = inVector->z;
}
inline void Phys_Vec3ToNitrousVec(const float * const inVector, phys_vec3 *outVector)
{
    outVector->x = inVector[0];
    outVector->y = inVector[1];
    outVector->z = inVector[2];
}

inline void __cdecl Phys_AxisToNitrousMat(float (*axis)[3], phys_mat44 *outMat)
{
    Phys_Vec3ToNitrousVec((float *)axis, &outMat->x);
    Phys_Vec3ToNitrousVec(&(*axis)[3], &outMat->y);
    Phys_Vec3ToNitrousVec(&(*axis)[6], &outMat->z);
}

inline float Abs(const phys_vec3 &vec)
{
    return sqrt( (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z) );
}

inline float AbsSquared(const phys_vec3 &vec)
{
    return ((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

inline const phys_vec3 &get_mat_wrow(const phys_mat44 &mat)
{
    return mat.w;
}

inline bool CrazyFloat(float f) // lwss add
{
    return !(f < 50000.0f && f > -50000.0f);
}

// oh fuck yes, in the compiler this is slurped into every file and duplicated 68 times
static const phys_vec3 PHYS_X_VEC = { 1.0f, 0.0f, 0.0f, 0.0f };
static const phys_vec3 PHYS_Y_VEC = { 0.0f, 1.0f, 0.0f, 0.0f };
static const phys_vec3 PHYS_Z_VEC = { 0.0f, 0.0f, 1.0f, 0.0f };

static const phys_vec3 PHYS_ZERO_VEC = { 0.0f, 0.0f, 0.0f, 0.0f };

static const phys_mat44 PHYS_IDENTITY_MATRIX(&PHYS_X_VEC, &PHYS_Y_VEC, &PHYS_Z_VEC, &PHYS_ZERO_VEC);

static const float PHYS_PI = 3.1415927f;
static const float PHYS_PI_TIMES_2 = 3.1415927 * 2.0;// 6.2831855f;
static const float PHYS_PI_OVER_2 = 3.1415927 / 2.0;// 1.5707964f;

#define PHYS_ALIGNOF(type) alignof(type)
