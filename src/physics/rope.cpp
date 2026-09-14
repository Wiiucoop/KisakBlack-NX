#include "rope.h"
#include <qcommon/cm_load.h>
#include "physconstraints_load_obj.h"
#include <qcommon/common.h>
#include <gfx_d3d/r_rope_render.h>
#include <cgame_mp/cg_main_mp.h>
#include <bgame/bg_wind.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_ents_mp.h>
#include <client/splitscreen.h>
#include <universal/curve.h>
#include <gfx_d3d/r_dpvs.h>
#include <bgame/bg_misc.h>
#include <game/g_debug.h>
#include <cgame/cg_drawtools.h>
#include <game/g_mover.h>
#include <cgame/cg_world.h>
#include <cgame_mp/cg_predict_mp.h>

static const float sc = 20.0f;


int g_ropesRendered;

cCurve g_ropeCurve;

int g_max_ropes;
rope_t *g_ropes;
int g_rope_sys_time;
int g_ropesWithEntsAnchorsCount;
int g_ropeCount;
int g_totalRopeCount;
int g_mapRopeCount;
int g_ropesWithEntsAnchors[10];
int g_num_contacts;
int g_update_count;
contact_t g_contacts[600];

bool __cdecl Rope_IsValid(int rope_index)
{
    return rope_index >= 0 && rope_index < g_max_ropes;
}

void __cdecl Rope_SetFlag(int rope_index, int flags, int onoff)
{
    rope_t *rope; // [esp+0h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    138,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( onoff )
        rope->m_flags |= flags;
    else
        rope->m_flags &= ~flags;
}

void __cdecl Rope_Reset()
{
    int ri; // [esp+4h] [ebp-8h]
    int rope_index; // [esp+8h] [ebp-4h]

    g_rope_sys_time = 0;
    g_ropes = cm.ropes;
    g_max_ropes = cm.max_ropes;
    rope_index = 0;
    for ( ri = 0; ri < cm.num_constraints; ++ri )
    {
        if ( cm.constraints[ri].type == CONSTRAINT_ROPE )
        {
            cm.constraints[ri].rope_index = -1;
            Rope_Reset(rope_index++);
        }
    }
    g_ropesWithEntsAnchorsCount = 0;
    g_ropeCount = 0;
}

void __cdecl Rope_Reset(int rope_index)
{
    int i; // [esp+0h] [ebp-8h]
    rope_t *rope; // [esp+4h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    155,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    rope->m_num_entity_anchors = 0;
    rope->m_num_constraints = 0;
    rope->m_num_particles = 0;
    rope->m_num_draw_verts = 0;
    rope->m_in_use = 0;
    for ( i = 0; i < g_ropesWithEntsAnchorsCount; ++i )
    {
        if ( g_ropesWithEntsAnchors[i] == rope_index )
        {
            g_ropesWithEntsAnchors[i] = g_ropesWithEntsAnchors[--g_ropesWithEntsAnchorsCount];
            return;
        }
    }
}

void __cdecl Rope_SetMaterial(int rope_index, Material *const material)
{
    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    205,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    g_ropes[rope_index].m_material = material;
}

int __cdecl Rope_GetNumParticles(int rope_index)
{
    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    219,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    return g_ropes[rope_index].m_num_particles;
}

void __cdecl Rope_AddAnchor(
                int rope_index,
                char particle_index,
                const float *p,
                int entity_index,
                int bone_name_hash,
                rope_constraint_e type)
{
    unsigned int v6[7]; // [esp+8h] [ebp-20h] BYREF
    rope_t *rope; // [esp+24h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    297,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( type != ROPE_CENTITY_CONSTRAINT )
        goto LABEL_18;
    if ( rope->m_num_entity_anchors >= 3 )
    {
        Com_PrintWarning(16, "WARNING: Max number of entity nachors exeeded. (%d)\n", 3);
        return;
    }
    if ( g_ropesWithEntsAnchorsCount >= 10 )
    {
        Com_PrintWarning(16, "WARNING: Max number of ropes with ent anchors exceeded. (%d)\n", 10);
    }
    else
    {
LABEL_18:
        if ( rope->m_num_constraints >= 30
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                        314,
                        0,
                        "%s",
                        "rope->m_num_constraints < MAX_ROPE_CONSTRAINTS") )
        {
            __debugbreak();
        }
        LOBYTE(v6[6]) = particle_index;
        BYTE1(v6[6]) = particle_index;
        *(float *)v6 = *p;
        *(float *)&v6[1] = p[1];
        *(float *)&v6[2] = p[2];
        v6[3] = 0;
        memcpy(&rope->m_constraints[rope->m_num_constraints], v6, sizeof(rope->m_constraints[rope->m_num_constraints]));
        rope->m_constraints[rope->m_num_constraints].type = type;
        rope->m_constraints[rope->m_num_constraints].entity_index = entity_index;
        rope->m_constraints[rope->m_num_constraints].bone_name_hash = bone_name_hash;
        if ( type == ROPE_CENTITY_CONSTRAINT )
        {
            rope->m_entity_anchors[rope->m_num_entity_anchors++] = rope->m_num_constraints;
            if ( rope->m_num_entity_anchors == 1 )
                g_ropesWithEntsAnchors[g_ropesWithEntsAnchorsCount++] = rope_index;
        }
        ++rope->m_num_constraints;
        Rope_Activate(rope_index);
    }
}

void __cdecl Rope_Activate(int rope_index)
{
    rope_t *rope; // [esp+0h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    129,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    rope->m_flags &= ~2u;
    rope->m_flags |= 8u;
    rope->m_stable_count = 0;
}

void __cdecl Rope_AddDynEntAnchor(int rope_index, char particle_index, const float *p, int entity_index)
{
    Rope_AddAnchor(rope_index, particle_index, p, entity_index, 0, ROPE_DENTITY_CONSTRAINT);
}

void __cdecl Rope_AddEntAnchor(
                int rope_index,
                char particle_index,
                const float *p,
                int entity_index,
                int bone_name_hash)
{
    Rope_AddAnchor(rope_index, particle_index, p, entity_index, bone_name_hash, ROPE_CENTITY_CONSTRAINT);
}

void __cdecl Rope_RemoveAnchor(int rope_index, int particle_index)
{
    int i; // [esp+8h] [ebp-20h]
    int ea1; // [esp+Ch] [ebp-1Ch]
    int v4; // [esp+10h] [ebp-18h]
    int ea; // [esp+14h] [ebp-14h]
    int num_entitty_anchors; // [esp+18h] [ebp-10h]
    int ci; // [esp+20h] [ebp-8h]
    rope_t *rope; // [esp+24h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    391,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    for ( ci = 0; ci < rope->m_num_constraints; ++ci )
    {
        if ( rope->m_constraints[ci].pi1 == rope->m_constraints[ci].pi2
            && rope->m_constraints[ci].pi1 == particle_index
            && rope->m_constraints[ci].type != ROPE_CENTITY_CONSTRAINT )
        {
            memcpy(
                &rope->m_constraints[ci--],
                &rope->m_constraints[--rope->m_num_constraints],
                sizeof(rope->m_constraints[ci--]));
        }
    }
    num_entitty_anchors = rope->m_num_entity_anchors;
    for ( ea = 0; ea < rope->m_num_entity_anchors; ++ea )
    {
        v4 = rope->m_entity_anchors[ea];
        if ( rope->m_constraints[v4].pi1 == particle_index )
        {
            memcpy(
                &rope->m_constraints[v4],
                &rope->m_constraints[--rope->m_num_constraints],
                sizeof(rope->m_constraints[v4]));
            rope->m_entity_anchors[ea--] = rope->m_entity_anchors[--rope->m_num_entity_anchors];
            for ( ea1 = 0; ea1 < rope->m_num_entity_anchors; ++ea1 )
            {
                if ( rope->m_entity_anchors[ea1] == rope->m_num_constraints )
                    rope->m_entity_anchors[ea1] = v4;
            }
        }
    }
    if ( num_entitty_anchors > 0 && !rope->m_num_entity_anchors )
    {
        for ( i = 0; i < g_ropesWithEntsAnchorsCount; ++i )
        {
            if ( g_ropesWithEntsAnchors[i] == rope_index )
                g_ropesWithEntsAnchors[i--] = g_ropesWithEntsAnchors[--g_ropesWithEntsAnchorsCount];
        }
    }
    Rope_Activate(rope_index);
}

static const float denom = 25.0f;
void __cdecl Rope_Create(
                int rope_index,
                const float *start,
                const float *end,
                float length,
                float scale,
                float width,
                float force_scale,
                int static_rope)
{
    int nsegments; // [esp+14h] [ebp-4h]

    if ( rope_index >= g_ropeCount )
        g_ropeCount = rope_index + 1;
    if ( g_ropeCount <= g_max_ropes )
    {
        Rope_Create(rope_index);
        nsegments = (int)(float)(length / denom);
        if ( nsegments < 4 )
            nsegments = 4;
        if ( nsegments > 25 )
            nsegments = 25;
        Rope_Init(rope_index, nsegments, start, end, length, width, scale, force_scale, static_rope);
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Ran out of ropes. Max is %d\n", g_max_ropes);
    }
}

void __cdecl Rope_Create(int rope_index)
{
    g_ropes[rope_index].m_client_verts.frame_index = 0;
}

void __cdecl Rope_Init(
                int rope_index,
                int npars,
                const float *p0,
                const float *p1,
                float rope_len,
                float rope_width,
                float scale,
                float force_scale,
                int static_rope)
{
    Material *MaterialFromGfxWorld; // [esp+8h] [ebp-68h]
    float *m_end; // [esp+14h] [ebp-5Ch]
    float *m_start; // [esp+18h] [ebp-58h]
    unsigned int v12[7]; // [esp+1Ch] [ebp-54h] BYREF
    int j; // [esp+38h] [ebp-38h]
    int i; // [esp+3Ch] [ebp-34h]
    float vlen; // [esp+40h] [ebp-30h]
    rope_t *rope; // [esp+44h] [ebp-2Ch]
    float dv[3]; // [esp+48h] [ebp-28h]
    float seg_len; // [esp+54h] [ebp-1Ch]
    float v[3]; // [esp+58h] [ebp-18h] BYREF
    float p[3]; // [esp+64h] [ebp-Ch] BYREF
    float rope_lena; // [esp+88h] [ebp+18h]

    Rope_Reset(rope_index);
    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    241,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    m_start = rope->m_start;
    rope->m_start[0] = *p0;
    m_start[1] = p0[1];
    m_start[2] = p0[2];
    m_end = rope->m_end;
    rope->m_end[0] = *p1;
    m_end[1] = p1[1];
    m_end[2] = p1[2];
    if ( R_Rope_GetMaterialFromGfxWorld() )
        MaterialFromGfxWorld = (Material *)R_Rope_GetMaterialFromGfxWorld();
    else
        MaterialFromGfxWorld = cgMedia.ropeMaterial;
    rope->m_material = MaterialFromGfxWorld;
    rope->m_in_use = 1;
    rope->m_num_entity_anchors = 0;
    rope->m_length = rope_len;
    rope->m_width = rope_width;
    rope->m_force_scale = force_scale;
    rope->m_scale = scale;
    rope->m_frame = 0;
    rope->m_visible = 0;
    rope->m_flags = 0;
    rope->m_stable_count = 0;
    rope->m_static_rope = static_rope;
    rope_lena = rope_len * scale;
    v[0] = *p1 - *p0;
    v[1] = p1[1] - p0[1];
    v[2] = p1[2] - p0[2];
    vlen = Vec3Length(v);
    if ( vlen < 0.001 )
    {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = -1.0f;
        vlen = 1.0f;
    }
    v[0] = (float)(1.0 / vlen) * v[0];
    v[1] = (float)(1.0 / vlen) * v[1];
    v[2] = (float)(1.0 / vlen) * v[2];
    seg_len = rope_lena / (float)(npars - 1);
    dv[0] = seg_len * v[0];
    dv[1] = seg_len * v[1];
    dv[2] = seg_len * v[2];
    p[0] = *p0;
    p[1] = p0[1];
    p[2] = p0[2];
    for ( i = 0; i < npars; ++i )
    {
        Rope_AddParticle(rope_index, p);
        p[0] = p[0] + dv[0];
        p[1] = p[1] + dv[1];
        p[2] = p[2] + dv[2];
    }
    rope->m_seglen = seg_len;
    rope->m_health = 0;
    rope->m_dist_constraint = 0;
    for ( j = 0; j < npars - 1; ++j )
    {
        LOBYTE(v12[6]) = j;
        BYTE1(v12[6]) = j + 1;
        *(float *)v12 = p[0];
        *(float *)&v12[1] = p[1];
        *(float *)&v12[2] = p[2];
        v12[3] = 0;
        memcpy(
            &rope->m_constraints[rope->m_num_constraints++],
            v12,
            sizeof(rope->m_constraints[rope->m_num_constraints++]));
    }
    rope->m_lightingHandle = 0;
}

void __cdecl Rope_AddParticle(int rope_index, const float *pos)
{
    rope_t *rope; // [esp+8h] [ebp-8h]
    par_t *par; // [esp+Ch] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    226,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( rope->m_num_particles >= 25
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    229,
                    0,
                    "%s",
                    "rope->m_num_particles < MAX_ROPE_PARTICLES") )
    {
        __debugbreak();
    }
    par = &rope->m_particles[rope->m_num_particles++];
    par->p[0] = *pos;
    par->p[1] = pos[1];
    par->p[2] = pos[2];
    par->p0[0] = *pos;
    par->p0[1] = pos[1];
    par->p0[2] = pos[2];
    par->p_prev[0] = *pos;
    par->p_prev[1] = pos[1];
    par->p_prev[2] = pos[2];
    par->flags = 0;
}

void __cdecl Rope_UpdateInternal(const RopeUpdateCmdData *cmd)
{
    int ri; // [esp+10h] [ebp-Ch]

    PROF_SCOPED("Rope_UpdateInternal");

    for ( ri = 0; ri < cmd->ropeCount; ++ri )
    {
        if ( cmd->ropes[ri].m_in_use )
            Rope_Tick(cmd, ri, 17.0 * 0.001, 0);
    }
}

void __cdecl Rope_Tick(const RopeUpdateCmdData *cmd, int rope_index, float dt, bool force_update)
{
    int v4; // eax
    bool v5; // [esp+4h] [ebp-40h]
    int j; // [esp+14h] [ebp-30h]
    float diff_4; // [esp+1Ch] [ebp-28h]
    float diff_8; // [esp+20h] [ebp-24h]
    par_t *par; // [esp+2Ch] [ebp-18h]
    int i; // [esp+30h] [ebp-14h]
    char forward; // [esp+3Ah] [ebp-Ah]
    bool stable; // [esp+3Bh] [ebp-9h]
    rope_t *rope; // [esp+40h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    757,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( rope->m_num_constraints )
    {
        rope->m_visible = 1;
        v5 = force_update || (rope->m_flags & 0x40) != 0;
        if ( v5 || (rope->m_flags & 8) != 0 || (Rope_UpdateVisible(rope_index), rope->m_visible) )
        {
            if ( (rope->m_flags & 2) == 0 )
            {
                if ( !v5 && (rope->m_flags & 8) != 0 )
                {
                    v4 = RETURN_ZERO32();
                    if ( !CG_GetLocalClientGlobals(v4)->nextSnap )
                        return;
                    Rope_Settle(rope_index);
                }
                rope->m_flags &= ~8u;
                g_num_contacts = 0;
                Rope_ApplyPhysics(rope_index, dt);
                rope->m_min[0] = FLT_MAX;
                rope->m_min[1] = FLT_MAX;
                rope->m_min[2] = FLT_MAX;
                rope->m_max[0] = -FLT_MAX;
                rope->m_max[1] = -FLT_MAX;
                rope->m_max[2] = -FLT_MAX;
                stable = 1;
                for ( i = 0; i < rope->m_num_particles; ++i )
                {
                    par = &rope->m_particles[i];
                    Vec3Min(par->p, rope->m_min, rope->m_min);
                    Vec3Min(par->p_prev, rope->m_min, rope->m_min);
                    Vec3Max(par->p, rope->m_max, rope->m_max);
                    Vec3Max(par->p_prev, rope->m_max, rope->m_max);
                    diff_4 = par->p[1] - par->p0[1];
                    diff_8 = par->p[2] - par->p0[2];
                    if ( (float)((float)((float)((float)(par->p[0] - par->p0[0]) * (float)(par->p[0] - par->p0[0]))
                                                         + (float)(diff_4 * diff_4))
                                         + (float)(diff_8 * diff_8)) > (float)(0.25 * 0.25) )
                        stable = 0;
                }
                if ( stable )
                {
                    if ( ++rope->m_stable_count > 30 && (rope->m_flags & 4) != 0 )
                        rope->m_flags |= 2u;
                }
                else
                {
                    rope->m_stable_count = 0;
                }
                if ( !v5 && (rope->m_flags & 1) != 0 )
                {
                    Rope_CollideWorld(rope_index);
                    Rope_CollideEntities(rope_index);
                }
                forward = 1;
                for ( j = 0; j < 6; ++j )
                {
                    Rope_ApplyConstraints(rope_index, forward);
                    forward = forward == 0;
                }
                ++rope->m_frame;
            }
            if ( cmd )
                Rope_BuildCurve(cmd, rope_index);
        }
    }
}

static const float base_fric = 0.01f;
static const float col_fric = 0.9f;
static const float wall_col_fric = 0.2f;
void __cdecl Rope_ApplyPhysics(int rope_index, float dt)
{
    int v2; // eax
    par_t *par; // [esp+1Ch] [ebp-54h]
    float fric; // [esp+2Ch] [ebp-44h]
    int i; // [esp+30h] [ebp-40h]
    float p1; // [esp+34h] [ebp-3Ch]
    float p1_4; // [esp+38h] [ebp-38h]
    float p1_8; // [esp+3Ch] [ebp-34h]
    rope_t *rope; // [esp+40h] [ebp-30h]
    float p0; // [esp+44h] [ebp-2Ch]
    float p0_4; // [esp+48h] [ebp-28h]
    float p0_8; // [esp+4Ch] [ebp-24h]
    float wind_vector[3]; // [esp+50h] [ebp-20h] BYREF
    float gravity; // [esp+5Ch] [ebp-14h]
    float a[3]; // [esp+60h] [ebp-10h]
    float windscale; // [esp+6Ch] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    458,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    ++g_update_count;
    gravity = -800.0f;
    BG_GetVariantWindVector(rope_index % 16, wind_vector);
    wind_vector[0] = (float)((float)(rand() % 101) / 100.0) * wind_vector[0];
    wind_vector[1] = (float)((float)(rand() % 101) / 100.0) * wind_vector[1];
    v2 = RETURN_ZERO32();
    windscale = CG_GetLocalWindSourceScale(v2, (float *)rope) * 10.0 + 1.0;
    wind_vector[0] = wind_vector[0] * windscale;
    wind_vector[1] = wind_vector[1] * windscale;
    wind_vector[0] = wind_vector[0] * rope->m_force_scale;
    wind_vector[1] = wind_vector[1] * rope->m_force_scale;
    if ( (rope->m_flags & 0x80) != 0 )
        memset(wind_vector, 0, sizeof(wind_vector));
    for ( i = 0; i < rope->m_num_particles; ++i )
    {
        par = &rope->m_particles[i];
        p1 = par->p[0];
        p1_4 = par->p[1];
        p1_8 = par->p[2];
        p0 = par->p0[0];
        p0_4 = par->p0[1];
        p0_8 = par->p0[2];
        a[0] = wind_vector[0];
        a[1] = wind_vector[1];
        a[2] = gravity;
        fric = base_fric;
        if ( (par->flags & 1) != 0 )
        {
            fric = col_fric;
        }
        else if ( (par->flags & 2) != 0 )
        {
            fric = wall_col_fric;
        }
        par->p[0] = (float)((float)(2.0 - fric) * p1) + (float)((float)(dt * dt) * a[0]);
        par->p[1] = (float)((float)(2.0 - fric) * p1_4) + (float)((float)(dt * dt) * a[1]);
        par->p[2] = (float)((float)(2.0 - fric) * p1_8) + (float)((float)(dt * dt) * a[2]);
        par->p[0] = par->p[0] - (float)((float)(1.0 - fric) * p0);
        par->p[1] = par->p[1] - (float)((float)(1.0 - fric) * p0_4);
        par->p[2] = par->p[2] - (float)((float)(1.0 - fric) * p0_8);
        par->p0[0] = p1;
        par->p0[1] = p1_4;
        par->p0[2] = p1_8;
    }
}

void __cdecl Rope_ApplyConstraints(int rope_index, bool forward)
{
    int i; // [esp+0h] [ebp-14h]
    int constraint_index; // [esp+4h] [ebp-10h]
    int contact_index; // [esp+8h] [ebp-Ch]
    int ci; // [esp+Ch] [ebp-8h]
    rope_t *rope; // [esp+10h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    613,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( forward )
    {
        for ( ci = rope->m_num_constraints - 1; ci >= 0; --ci )
            Rope_ApplyConstraint(rope_index, ci);
        for ( contact_index = g_num_contacts - 1; contact_index >= 0; --contact_index )
            Rope_ApplyContactConstraint(rope_index, contact_index);
    }
    else
    {
        for ( constraint_index = 0; constraint_index < rope->m_num_constraints; ++constraint_index )
            Rope_ApplyConstraint(rope_index, constraint_index);
        for ( i = 0; i < g_num_contacts; ++i )
            Rope_ApplyContactConstraint(rope_index, i);
    }
}

void __cdecl Rope_ApplyContactConstraint(int rope_index, int contact_index)
{
    int particle_index; // ecx
    float *p0; // [esp+0h] [ebp-48h]
    float *p; // [esp+18h] [ebp-30h]
    float d; // [esp+30h] [ebp-18h]
    float da; // [esp+30h] [ebp-18h]
    rope_t *rope; // [esp+34h] [ebp-14h]
    contact_t *contact; // [esp+44h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    516,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    contact = &g_contacts[contact_index];
    d = (float)((float)((float)(rope->m_particles[contact->particle_index].p[0] - contact->point[0]) * contact->normal[0])
                        + (float)((float)(rope->m_particles[contact->particle_index].p[1] - contact->point[1]) * contact->normal[1]))
        + (float)((float)(rope->m_particles[contact->particle_index].p[2] - contact->point[2]) * contact->normal[2]);
    if ( d <= 0.0 )
    {
        p = rope->m_particles[contact->particle_index].p;
        particle_index = contact->particle_index;
        *p = (float)((-(d)) * contact->normal[0]) + rope->m_particles[particle_index].p[0];
        p[1] = (float)((-(d)) * contact->normal[1]) + rope->m_particles[particle_index].p[1];
        p[2] = (float)((-(d)) * contact->normal[2]) + rope->m_particles[particle_index].p[2];
    }
    da = (float)((float)((float)(rope->m_particles[contact->particle_index].p0[0] - contact->point[0]) * contact->normal[0])
                         + (float)((float)(rope->m_particles[contact->particle_index].p0[1] - contact->point[1]) * contact->normal[1]))
         + (float)((float)(rope->m_particles[contact->particle_index].p0[2] - contact->point[2]) * contact->normal[2]);
    if ( da <= 0.0 )
    {
        p0 = rope->m_particles[contact->particle_index].p0;
        p0[0] = (float)((-(da)) * contact->normal[0]) + *p0;
        p0[1] = (float)((-(da)) * contact->normal[1]) + p0[1];
        p0[2] = (float)((-(da)) * contact->normal[2]) + p0[2];
    }
}

void __cdecl Rope_ApplyConstraint(int rope_index, int constraint_index)
{
    float v2; // [esp+0h] [ebp-E8h]
    float *v3; // [esp+Ch] [ebp-DCh]
    float *v4; // [esp+10h] [ebp-D8h]
    float *v5; // [esp+24h] [ebp-C4h]
    float *v6; // [esp+28h] [ebp-C0h]
    float *p0; // [esp+34h] [ebp-B4h]
    float *p; // [esp+38h] [ebp-B0h]
    float vlen; // [esp+3Ch] [ebp-ACh]
    float *p1; // [esp+40h] [ebp-A8h]
    float temp; // [esp+44h] [ebp-A4h]
    float temp_4; // [esp+48h] [ebp-A0h]
    float temp_8; // [esp+4Ch] [ebp-9Ch]
    float *p2; // [esp+50h] [ebp-98h]
    float targetd; // [esp+54h] [ebp-94h]
    float v[3]; // [esp+58h] [ebp-90h] BYREF
    float l1; // [esp+64h] [ebp-84h]
    DObj *obj; // [esp+68h] [ebp-80h]
    float axis[9]; // [esp+6Ch] [ebp-7Ch] BYREF
    float origin[3]; // [esp+90h] [ebp-58h] BYREF
    centity_s *cent; // [esp+9Ch] [ebp-4Ch]
    DynEntityPose *dynEntPose; // [esp+A0h] [ebp-48h]
    DynEntityDrawType drawType; // [esp+A4h] [ebp-44h] BYREF
    float xform[4][3]; // [esp+A8h] [ebp-40h] BYREF
    unsigned __int16 dynEntId; // [esp+D8h] [ebp-10h] BYREF
    int ind; // [esp+DCh] [ebp-Ch]
    rope_t *rope; // [esp+E0h] [ebp-8h]
    constraint_t *constraint; // [esp+E4h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    538,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    constraint = &rope->m_constraints[constraint_index];
    if ( rope->m_constraints[constraint_index].pi1 == rope->m_constraints[constraint_index].pi2 )
    {
        ind = constraint->pi1;
        switch ( constraint->type )
        {
            case ROPE_WORLD_CONSTRAINT:
                p = rope->m_particles[ind].p;
                *p = constraint->p[0];
                p[1] = constraint->p[1];
                p[2] = constraint->p[2];
                p0 = rope->m_particles[ind].p0;
                *p0 = constraint->p[0];
                p0[1] = constraint->p[1];
                p0[2] = constraint->p[2];
                return;
            case ROPE_DENTITY_CONSTRAINT:
                DynEnt_GetClientIdDrawType(constraint->entity_index, &dynEntId, &drawType);
                dynEntPose = DynEnt_GetClientPose(dynEntId, drawType);
                UnitQuatToAxis(dynEntPose->pose.quat, xform);
                xform[3][0] = dynEntPose->pose.origin[0];
                xform[3][1] = dynEntPose->pose.origin[1];
                xform[3][2] = dynEntPose->pose.origin[2];
                MatrixTransformVector43(constraint->p, xform, rope->m_particles[ind].p);
                v5 = rope->m_particles[ind].p0;
                v6 = rope->m_particles[ind].p;
                *v5 = *v6;
                v5[1] = v6[1];
                v5[2] = v6[2];
                rope->m_stable_count = 0;
                return;
            case ROPE_CENTITY_CONSTRAINT:
                cent = CG_GetEntity(0, constraint->entity_index);
                if ( ((cent->clientFlags >> 1) & 1) != 0 )
                {
                    if ( !constraint->bone_name_hash )
                    {
                        AnglesToAxis(cent->pose.angles, (float (*)[3])axis);
                        origin[0] = cent->pose.origin[0];
                        origin[1] = cent->pose.origin[1];
                        origin[2] = cent->pose.origin[2];
LABEL_16:
                        MatrixTransformVector43(constraint->p, (const float (*)[3])axis, rope->m_particles[ind].p);
                        v3 = rope->m_particles[ind].p0;
                        v4 = rope->m_particles[ind].p;
                        *v3 = *v4;
                        v3[1] = v4[1];
                        v3[2] = v4[2];
                        rope->m_stable_count = 0;
                        return;
                    }
                    obj = Com_GetClientDObj(constraint->entity_index, 0);
                    if ( obj && CG_DObjGetWorldTagMatrix(&cent->pose, obj, constraint->bone_name_hash, (float (*)[3])axis, origin) )
                        goto LABEL_16;
                }
                else
                {
                    rope->m_flags |= 8u;
                }
                break;
        }
    }
    else
    {
        p1 = rope->m_particles[constraint->pi1].p;
        p2 = rope->m_particles[constraint->pi2].p;
        v[0] = *p2 - *p1;
        v[1] = p2[1] - p1[1];
        v[2] = p2[2] - p1[2];
        vlen = Vec3Length(v);
        targetd = rope->m_seglen;
        v[0] = (float)(1.0 / vlen) * v[0];
        v[1] = (float)(1.0 / vlen) * v[1];
        v[2] = (float)(1.0 / vlen) * v[2];
        l1 = (float)(targetd + vlen) * 0.5;
        temp = *p2;
        temp_4 = p2[1];
        temp_8 = p2[2];
        *p2 = (float)(l1 * v[0]) + *p1;
        p2[1] = (float)(l1 * v[1]) + p1[1];
        p2[2] = (float)(l1 * v[2]) + p1[2];
        v2 = -l1;
        p1[0] = (float)((-(l1)) * v[0]) + temp;
        p1[1] = (float)(v2 * v[1]) + temp_4;
        p1[2] = (float)(v2 * v[2]) + temp_8;
    }
}

void __cdecl Rope_UpdateVisible(int rope_index)
{
    float v1; // [esp+8h] [ebp-60h]
    float v2; // [esp+Ch] [ebp-5Ch]
    float v3; // [esp+10h] [ebp-58h]
    float v4; // [esp+14h] [ebp-54h]
    float v5; // [esp+24h] [ebp-44h]
    par_t *v6; // [esp+34h] [ebp-34h]
    float dist2; // [esp+38h] [ebp-30h]
    cg_s *cgameGlob; // [esp+3Ch] [ebp-2Ch]
    int ci; // [esp+44h] [ebp-24h]
    float p1[3]; // [esp+48h] [ebp-20h] BYREF
    rope_t *rope; // [esp+54h] [ebp-14h]
    int skip; // [esp+58h] [ebp-10h]
    float p0[3]; // [esp+5Ch] [ebp-Ch] BYREF

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    712,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    skip = 1;
    p0[0] = rope->m_particles[0].p[0];
    p0[1] = rope->m_particles[0].p[1];
    p0[2] = rope->m_particles[0].p[2];
    v6 = &rope->m_particles[rope->m_num_particles - 1];
    p1[0] = v6->p[0];
    p1[1] = v6->p[1];
    p1[2] = v6->p[2];
    for ( ci = 0; ci < 1; ++ci )
    {
        if ( CL_LocalClient_IsActive(ci) )
        {
            cgameGlob = CG_GetLocalClientGlobals(ci);
            if ( cg_pmove[ci].ps )
            {
                v2 = cgameGlob->refdef.vieworg[0] - p0[0];
                v3 = cgameGlob->refdef.vieworg[1] - p0[1];
                v4 = cgameGlob->refdef.vieworg[2] - p0[2];
                v1 = (float)((float)(v2 * (float)(p1[0] - p0[0])) + (float)(v3 * (float)(p1[1] - p0[1])))
                     + (float)(v4 * (float)(p1[2] - p0[2]));
                if ( v1 > 0.0 )
                {
                    v5 = (float)((float)((float)(p1[0] - p0[0]) * (float)(p1[0] - p0[0]))
                                         + (float)((float)(p1[1] - p0[1]) * (float)(p1[1] - p0[1])))
                         + (float)((float)(p1[2] - p0[2]) * (float)(p1[2] - p0[2]));
                    dist2 = v1 < v5
                                ? (float)((float)((float)(v2 * v2) + (float)(v3 * v3)) + (float)(v4 * v4))
                                - (float)((float)(v1 * v1) / v5)
                                : (float)((float)((float)(cgameGlob->refdef.vieworg[0] - p1[0])
                                                                * (float)(cgameGlob->refdef.vieworg[0] - p1[0]))
                                                + (float)((float)(cgameGlob->refdef.vieworg[1] - p1[1])
                                                                * (float)(cgameGlob->refdef.vieworg[1] - p1[1])))
                                + (float)((float)(cgameGlob->refdef.vieworg[2] - p1[2]) * (float)(cgameGlob->refdef.vieworg[2] - p1[2]));
                }
                else
                {
                    dist2 = (float)((float)(v2 * v2) + (float)(v3 * v3)) + (float)(v4 * v4);
                }
                if ( dist_thresh2 > dist2 && !R_CullLine(ci, p0, p1, cull_thresh) )
                {
                    skip = 0;
                    break;
                }
            }
        }
    }
    if ( skip )
        rope->m_visible = 0;
}

void __cdecl Rope_BuildCurve(const RopeUpdateCmdData *cmd, int rope_index)
{
    int v2; // [esp+8h] [ebp-7Ch]
    int v3; // [esp+14h] [ebp-70h]
    int v4; // [esp+1Ch] [ebp-68h]
    int j; // [esp+20h] [ebp-64h]
    int i; // [esp+3Ch] [ebp-48h]
    float segLength; // [esp+40h] [ebp-44h]
    rope_frame_verts_t *verts; // [esp+44h] [ebp-40h]
    int segCount; // [esp+48h] [ebp-3Ch]
    float segDist; // [esp+50h] [ebp-34h]
    float dist; // [esp+58h] [ebp-2Ch]
    float p1[3]; // [esp+64h] [ebp-20h] BYREF
    float p2[3]; // [esp+70h] [ebp-14h] BYREF
    int numSegments; // [esp+7Ch] [ebp-8h]
    rope_t *rope; // [esp+80h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    915,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( rope->m_num_particles )
    {
        numSegments = 3 * rope->m_num_particles;
        if ( numSegments >= 50 )
            numSegments = 49;
        if ( (rope->m_flags & 0x100) == 0 )
        {
            MatrixVecMultiplyProject(cmd->screenMtx, (const float *)rope, p1);
            MatrixVecMultiplyProject(cmd->screenMtx, rope->m_particles[rope->m_num_particles - 1].p, p2);
            if ( p1[2] > 1.0 == p2[2] > 1.0 )
            {
                dist = sqrtf(
                                 (float)((float)(p1[0] - p2[0]) * (float)(p1[0] - p2[0]))
                             + (float)((float)(p1[1] - p2[1]) * (float)(p1[1] - p2[1])));
                if ( numSegments < 6 )
                    v3 = numSegments;
                else
                    v3 = 6;
                if ( (int)(float)((float)numSegments * (float)(dist * 2.0)) < numSegments )
                    v4 = (int)(float)((float)numSegments * (float)(dist * 2.0));
                else
                    v4 = numSegments;
                if ( v3 < v4 )
                    v2 = v4;
                else
                    v2 = v3;
                numSegments = v2;
            }
        }
        if ( (rope->m_flags & 2) == 0 || rope->m_num_draw_verts != numSegments )
        {
            rope->m_num_draw_verts = numSegments;
            rope->m_client_verts.frame_index = 1 - rope->m_client_verts.frame_index;
            verts = &rope->m_client_verts.frame_verts[rope->m_client_verts.frame_index];
            verts->num_verts = numSegments;
            g_ropeCurve.Reinit();

            for (i = 0; i < rope->m_num_particles; ++i)
            {
                //cCurve::AddNode(&g_ropeCurve, rope->m_particles[i].p);
                g_ropeCurve.AddNode(rope->m_particles[i].p);
            }

            if ( g_ropeCurve.mCurveType != cCurve::eCurveType::CURVE_BSPLINE )
                g_ropeCurve.mCurveType = cCurve::eCurveType::CURVE_BSPLINE;

            //cCurve::Build(&g_ropeCurve);
            g_ropeCurve.Build();
            segLength = 1.0 / (float)numSegments;
            segDist = 0.0f;
            for ( segCount = 0; segCount < numSegments; ++segCount )
            {
                //cCurve::GetPos(&g_ropeCurve, segDist, verts->v[segCount]);
                g_ropeCurve.GetPos(segDist, verts->v[segCount]);
                segDist = segDist + segLength;
            }
            //cCurve::GetPos(&g_ropeCurve, 1.0, verts->v[segCount]);
            g_ropeCurve.GetPos(1.0, verts->v[segCount]);
            if ( !rope->m_client_verts.frame_index )
                rope->m_flags |= 4u;
            if ( debug_rope->current.enabled )
            {
                for ( j = 0; j < numSegments; ++j )
                    G_DebugStar(verts->v[j], colorRed, 0);
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
//void    Rope_Update(int localClientNum, int curtime)
//{
//    _BYTE v3[208]; // [esp+20h] [ebp-17Ch] OVERLAPPED BYREF
//    __int128 viewAxis_24; // [esp+108h] [ebp-94h] OVERLAPPED BYREF
//    const cg_s *cgameGlob; // [esp+118h] [ebp-84h]
//    RopeUpdateCmdData cmd; // [esp+11Ch] [ebp-80h] BYREF
//    int v7; // [esp+188h] [ebp-14h]
//    const char *v8; // [esp+18Ch] [ebp-10h]
//    //int v9; // [esp+190h] [ebp-Ch]
//    //int delta_t; // [esp+194h] [ebp-8h]
//    //int retaddr; // [esp+19Ch] [ebp+0h]
//
//    //v9 = a1;
//    //delta_t = retaddr;
//    if ( !Com_IsMenuLevel(0) )
//    {
//        v8 = va("Rope_Update %i/%i", g_update_count, g_ropeCount);
//        //PIXBeginNamedEvent(-1, v8);
//        g_update_count = 0;
//        v7 = curtime - g_rope_sys_time;
//        if ( curtime - g_rope_sys_time > 0 )
//        {
//            if ( RETURN_ZERO32() == localClientNum )
//            {
//                LODWORD(cmd.viewOrg[0]) = g_ropeCount;
//                LODWORD(cmd.screenMtx[3][1]) = g_ropes;
//                DWORD1(viewAxis_24) = CG_GetLocalClientGlobals(localClientNum);
//                LODWORD(viewAxis_24) = DWORD1(viewAxis_24) + 274140;
//                *((_QWORD *)&viewAxis_24 + 1) = *(_QWORD *)(DWORD1(viewAxis_24) + 274140);
//                cgameGlob = *(const cg_s **)(DWORD1(viewAxis_24) + 274148);
//                LODWORD(cmd.viewOrg[0]) = g_ropeCount;
//                LODWORD(cmd.screenMtx[3][1]) = g_ropes;
//                AnglesToAxis((const float *)(DWORD1(viewAxis_24) + 373712), (float (*)[3])&v3[196]);
//                MatrixForViewer(
//                    (const float *)(DWORD1(viewAxis_24) + 274140),
//                    (const float (*)[3])&v3[196],
//                    (float (*)[4])&v3[128]);
//                InfinitePerspectiveMatrix(
//                    *(float *)(DWORD1(viewAxis_24) + 274128),
//                    *(float *)(DWORD1(viewAxis_24) + 274132),
//                    1.0,
//                    (float (*)[4])&v3[64]);
//                MatrixMultiply44((const float (*)[4])&v3[128], (const float (*)[4])&v3[64], (float (*)[4])v3);
//                MatrixTranspose44((const float *)v3, &cmd.viewOrg[1]);
//                Rope_UpdateInternal((const RopeUpdateCmdData *)((char *)&viewAxis_24 + 8));
//                g_rope_sys_time = curtime;
//                //if ( GetCurrentThreadId() == g_DXDeviceThread )
//                    //D3DPERF_EndEvent();
//            }
//            else //if ( GetCurrentThreadId() == g_DXDeviceThread )
//            {
//                goto LABEL_4;
//            }
//        }
//        else //if ( GetCurrentThreadId() == g_DXDeviceThread )
//        {
//LABEL_4:
//            //D3DPERF_EndEvent();
//        }
//    }
//}

// aislop
void Rope_Update(int localClientNum, int curtime)
{
    int delta_t;
    cg_s *cg;
    RopeUpdateCmdData cmd;

    float viewAxis[3][3];
    float viewMtx[4][4];
    float projMtx[4][4];
    float screenMtx[4][4];

    if (Com_IsMenuLevel(0))
        return;

    PROF_SCOPED_RUNTIME_NAME(va("Rope_Update %i/%i", g_update_count, g_ropeCount));

    g_update_count = 0;

    delta_t = curtime - g_rope_sys_time;
    if (delta_t <= 0)
        return;

    if (localClientNum != 0)
        return;

    cmd.ropeCount = g_ropeCount;
    cmd.ropes = g_ropes;

    cg = CG_GetLocalClientGlobals(localClientNum);

    cmd.viewOrg[0] = cg->refdef.vieworg[0];
    cmd.viewOrg[1] = cg->refdef.vieworg[1];
    cmd.viewOrg[2] = cg->refdef.vieworg[2];

    AnglesToAxis(cg->refdefViewAngles, viewAxis);

    MatrixForViewer(cg->refdef.vieworg, viewAxis, viewMtx);

    InfinitePerspectiveMatrix(
        cg->refdef.tanHalfFovX,
        cg->refdef.tanHalfFovY,
        1.0f,
        projMtx
    );

    MatrixMultiply44(viewMtx, projMtx, screenMtx);
    MatrixTranspose44(screenMtx, cmd.screenMtx);

    Rope_UpdateInternal(&cmd);

    g_rope_sys_time = curtime;
}



void __cdecl Rope_Render(unsigned int localClientNum)
{
    int ri; // [esp+8h] [ebp-8h]

    PROF_SCOPED("Rope_Render");

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    1195,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    for ( ri = 0; ri < g_ropeCount; ++ri )
    {
        if ( g_ropes[ri].m_in_use )
        {
            Rope_Draw(ri, localClientNum);
            if ( debug_rope->current.enabled )
                Rope_DebugDraw(ri);
        }
    }
}

void __cdecl Rope_DebugDraw(int rope_index)
{
    float p1[3]; // [esp+8h] [ebp-3Ch] BYREF
    const contact_t *contact; // [esp+14h] [ebp-30h]
    int k; // [esp+18h] [ebp-2Ch]
    const par_t *par; // [esp+1Ch] [ebp-28h]
    int j; // [esp+20h] [ebp-24h]
    int i; // [esp+24h] [ebp-20h]
    float mins[3]; // [esp+28h] [ebp-1Ch]
    rope_t *rope; // [esp+34h] [ebp-10h]
    float maxs[3]; // [esp+38h] [ebp-Ch]

    mins[0] = -2.5f;
    mins[1] = -2.5f;
    mins[2] = -2.5f;
    maxs[0] = 2.5f;
    maxs[1] = 2.5f;
    maxs[2] = 2.5f;
    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    868,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( rope->m_num_particles )
    {
        for ( i = 0; i < rope->m_num_particles - 1; ++i )
            CG_DebugLine(rope->m_particles[i].p, rope->m_particles[i + 1].p, colorBlack, 0, 1);
        for ( j = 0; j < rope->m_num_particles; ++j )
        {
            par = &rope->m_particles[j];
            G_DebugStar(par->p_prev, colorGreen, 0);
        }
        for ( k = 0; k < g_num_contacts; ++k )
        {
            contact = &g_contacts[k];
            p1[0] = (float)(sc * contact->normal[0]) + contact->point[0];
            p1[1] = (float)(sc * contact->normal[1]) + contact->point[1];
            p1[2] = (float)(sc * contact->normal[2]) + contact->point[2];
            CG_DebugLine(contact->point, p1, colorYellow, 0, 1);
        }
    }
}

void __cdecl Rope_Draw(int rope_index, unsigned int localClientNum)
{
    rope_t *rope; // [esp+0h] [ebp-2Ch]
    VisualRope vrope; // [esp+4h] [ebp-28h] BYREF
    rope_frame_verts_t *verts; // [esp+24h] [ebp-8h]
    unsigned int frameIdx; // [esp+28h] [ebp-4h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    1002,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( rope->m_num_particles )
    {
        if ( rope->m_visible )
        {
            vrope.visibilityFlags = R_Rope_CalcVisibility(rope->m_min, localClientNum);
            if ( vrope.visibilityFlags )
            {
                ++g_ropesRendered;
                frameIdx = rope->m_client_verts.frame_index;
                verts = &rope->m_client_verts.frame_verts[frameIdx];
                vrope.beginRadius = rope->m_width * 0.5;
                vrope.endRadius = rope->m_width * 0.5;
                vrope.material = rope->m_material;
                vrope.segmentVerts = rope->m_client_verts.frame_verts[frameIdx].v[0];
                vrope.segmentCount = verts->num_verts;
                vrope.lightingHandle = &rope->m_lightingHandle;
                if ( !rope->m_material
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                                1049,
                                0,
                                "%s",
                                "rope->m_material") )
                {
                    __debugbreak();
                }
                R_Rope_Add(&vrope);
            }
        }
    }
}

bool doSettle = true;

void __cdecl Rope_Settle(int rope_index)
{
    int i; // [esp+8h] [ebp-4h]

    if ( doSettle )
    {
        for ( i = 0; i < 100; ++i )
            Rope_Tick(0, rope_index, 0.017000001, 1);
    }
}

void __cdecl Rope_ExplosionEvent(
                const float *origin,
                float innerRadius,
                float outerRadius,
                int innerDamage,
                int outerDamage)
{
    int ri; // [esp+10h] [ebp-4h]

    for ( ri = 0; ri < g_ropeCount; ++ri )
    {
        if ( g_ropes[ri].m_in_use )
            Rope_ExplosionEvent(ri, origin, innerRadius, outerRadius, innerDamage, outerDamage);
    }
}

void __cdecl Rope_ExplosionEvent(
                int rope_index,
                const float *origin,
                float innerRadius,
                float outerRadius,
                int innerDamage,
                int outerDamage)
{
    float *p; // [esp+18h] [ebp-74h]
    float dist; // [esp+44h] [ebp-48h]
    float dist2; // [esp+48h] [ebp-44h]
    int i; // [esp+4Ch] [ebp-40h]
    float distSqr; // [esp+50h] [ebp-3Ch]
    float force; // [esp+58h] [ebp-34h]
    float p1; // [esp+5Ch] [ebp-30h]
    float p1_4; // [esp+60h] [ebp-2Ch]
    float p1_8; // [esp+64h] [ebp-28h]
    rope_t *rope; // [esp+68h] [ebp-24h]
    float invRange; // [esp+6Ch] [ebp-20h]
    float f; // [esp+70h] [ebp-1Ch]
    float f_4; // [esp+74h] [ebp-18h]
    float f_8; // [esp+78h] [ebp-14h]
    float innerRadiusSqr; // [esp+7Ch] [ebp-10h]
    float scale; // [esp+80h] [ebp-Ch]
    float outerRadiusSqr; // [esp+84h] [ebp-8h]

    if ( !Rope_IsValid(rope_index)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                    650,
                    0,
                    "%s",
                    "Rope_IsValid( rope_index )") )
    {
        __debugbreak();
    }
    rope = &g_ropes[rope_index];
    if ( rope->m_visible )
    {
        outerRadiusSqr = outerRadius * outerRadius;
        innerRadiusSqr = innerRadius * innerRadius;
        distSqr = PointToBoxDistSq(origin, rope->m_min, rope->m_max);
        if ( distSqr <= (float)(outerRadius * outerRadius) )
        {
            rope->m_flags &= ~2u;
            rope->m_stable_count = 0;
            invRange = 0.0f;
            if ( outerRadiusSqr > innerRadiusSqr )
                invRange = 1.0 / (float)(innerRadius - outerRadius);
            for ( i = 0; i < rope->m_num_particles; ++i )
            {
                p1 = rope->m_particles[i].p[0];
                p1_4 = rope->m_particles[i].p[1];
                p1_8 = rope->m_particles[i].p[2];
                f = p1 - *origin;
                f_4 = p1_4 - origin[1];
                f_8 = p1_8 - origin[2];
                dist2 = (float)((float)(f * f) + (float)(f_4 * f_4)) + (float)(f_8 * f_8);
                if ( dist2 < outerRadiusSqr )
                {
                    scale = 1.0f;
                    dist = sqrtf(dist2);
                    if ( dist2 > innerRadiusSqr )
                    {
                        if ( outerRadiusSqr <= innerRadiusSqr
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp",
                                        692,
                                        1,
                                        "%s",
                                        "outerRadiusSqr > innerRadiusSqr") )
                        {
                            __debugbreak();
                        }
                        scale = (float)((float)(dist - outerRadius) * invRange) * 1.0;
                    }
                    if ( scale < 0.0
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\rope.cpp", 695, 1, "%s", "scale >= 0") )
                    {
                        __debugbreak();
                    }
                    force = (float)((float)((float)((float)((float)innerDamage * 0.1) - (float)((float)outerDamage * 0.1)) * scale)
                                                + (float)((float)outerDamage * 0.1))
                                * rope->m_force_scale;
                    p = rope->m_particles[i].p;
                    *p = (float)(2.0 * p1) + (float)(-1.0 * p[3]);
                    p[1] = (float)(2.0 * p1_4) + (float)(-1.0 * p[4]);
                    p[2] = (float)(2.0 * p1_8) + (float)(-1.0 * p[5]);
                    *p = *p + (float)(force * (float)((float)(1.0 / dist) * f));
                    p[1] = p[1] + (float)(force * (float)((float)(1.0 / dist) * f_4));
                    p[2] = p[2] + (float)(force * (float)((float)(1.0 / dist) * f_8));
                    p[3] = p1;
                    p[4] = p1_4;
                    p[5] = p1_8;
                }
            }
        }
    }
}

double __cdecl PointToBoxDistSq(const float *pt, const float *mins, const float *maxs)
{
    float v4; // [esp+0h] [ebp-30h]
    float v5; // [esp+4h] [ebp-2Ch]
    float v6; // [esp+8h] [ebp-28h]
    float v7; // [esp+Ch] [ebp-24h]
    float v8; // [esp+10h] [ebp-20h]
    float v9; // [esp+14h] [ebp-1Ch]
    float delta0_mins_pt; // [esp+18h] [ebp-18h]
    float delta1_mins_pt; // [esp+1Ch] [ebp-14h]
    float delta0_pt_maxs; // [esp+20h] [ebp-10h]
    float delta2_pt_maxs; // [esp+24h] [ebp-Ch]
    float delta1_pt_maxs; // [esp+28h] [ebp-8h]
    float delta2_mins_pt; // [esp+2Ch] [ebp-4h]

    delta0_mins_pt = *mins - *pt;
    delta1_mins_pt = mins[1] - pt[1];
    delta2_mins_pt = mins[2] - pt[2];
    delta0_pt_maxs = *pt - *maxs;
    delta1_pt_maxs = pt[1] - maxs[1];
    delta2_pt_maxs = pt[2] - maxs[2];
    if ( delta0_mins_pt < 0.0 )
        v9 = 0.0f;
    else
        v9 = delta0_mins_pt * delta0_mins_pt;
    if ( delta1_mins_pt < 0.0 )
        v8 = 0.0f;
    else
        v8 = delta1_mins_pt * delta1_mins_pt;
    if ( delta2_mins_pt < 0.0 )
        v7 = 0.0f;
    else
        v7 = delta2_mins_pt * delta2_mins_pt;
    if ( delta0_pt_maxs < 0.0 )
        v6 = 0.0f;
    else
        v6 = delta0_pt_maxs * delta0_pt_maxs;
    if ( delta1_pt_maxs < 0.0 )
        v5 = 0.0f;
    else
        v5 = delta1_pt_maxs * delta1_pt_maxs;
    if ( delta2_pt_maxs < 0.0 )
        v4 = 0.0f;
    else
        v4 = delta2_pt_maxs * delta2_pt_maxs;
    return v9 + v8 + v7 + v6 + v5 + v4;
}

int __cdecl trace_point_through_sphere(
                const float *p,
                const float *ud,
                const float *ctr,
                float r,
                float *t,
                float *hitp)
{
    float v7; // [esp+0h] [ebp-1Ch]
    float m; // [esp+4h] [ebp-18h]
    float m_4; // [esp+8h] [ebp-14h]
    float m_8; // [esp+Ch] [ebp-10h]
    float c; // [esp+10h] [ebp-Ch]
    float b; // [esp+14h] [ebp-8h]
    float discr; // [esp+18h] [ebp-4h]

    m = *p - *ctr;
    m_4 = p[1] - ctr[1];
    m_8 = p[2] - ctr[2];
    b = (float)((float)(m * *ud) + (float)(m_4 * ud[1])) + (float)(m_8 * ud[2]);
    c = (float)((float)((float)(m * m) + (float)(m_4 * m_4)) + (float)(m_8 * m_8)) - (float)(r * r);
    if ( c > 0.0 && b > 0.0 )
        return 0;
    discr = (float)(b * b) - c;
    if ( discr < 0.0 )
        return 0;
    *t = (-(b)) - sqrtf(discr);
    if ( *t < 0.0 )
        *t = 0.0f;
    v7 = *t;
    *hitp = (float)(*t * *ud) + *p;
    hitp[1] = (float)(v7 * ud[1]) + p[1];
    hitp[2] = (float)(v7 * ud[2]) + p[2];
    return 1;
}

static const float radius_0 = 5.0f;
static const float local_scale = 20.0f;



void __cdecl Rope_Trace(const float *p0, const float *p1)
{
    float *p; // [esp+28h] [ebp-8Ch]
    float force; // [esp+48h] [ebp-6Ch]
    float v5; // [esp+4Ch] [ebp-68h]
    float v6; // [esp+50h] [ebp-64h]
    float v7; // [esp+54h] [ebp-60h]
    float f; // [esp+5Ch] [ebp-58h]
    float f_4; // [esp+60h] [ebp-54h]
    float f_8; // [esp+64h] [ebp-50h]
    int vi; // [esp+68h] [ebp-4Ch]
    int nverts; // [esp+6Ch] [ebp-48h]
    rope_t *rope; // [esp+70h] [ebp-44h]
    int ri; // [esp+74h] [ebp-40h]
    float t; // [esp+78h] [ebp-3Ch] BYREF
    float vlen; // [esp+7Ch] [ebp-38h]
    float ud[3]; // [esp+84h] [ebp-30h] BYREF
    float hitp[3]; // [esp+90h] [ebp-24h] BYREF
    float mins[3]; // [esp+9Ch] [ebp-18h] BYREF
    float maxs[3]; // [esp+A8h] [ebp-Ch] BYREF

    PROF_SCOPED("Rope_Trace");

    ud[0] = *p1 - *p0;
    ud[1] = p1[1] - p0[1];
    ud[2] = p1[2] - p0[2];
    vlen = Vec3Length(ud);
    if ( vlen >= 0.0099999998 )
    {
        ud[0] = (float)(1.0 / vlen) * ud[0];
        ud[1] = (float)(1.0 / vlen) * ud[1];
        ud[2] = (float)(1.0 / vlen) * ud[2];
        Vec3Min(p0, p1, mins);
        Vec3Max(p0, p1, maxs);
        for ( ri = 0; ri < g_ropeCount; ++ri )
        {
            rope = &g_ropes[ri];
            if ( rope->m_in_use )
            {
                if ( maxs[0] >= rope->m_min[0]
                    && maxs[1] >= rope->m_min[1]
                    && maxs[2] >= rope->m_min[2]
                    && rope->m_max[0] >= mins[0]
                    && rope->m_max[1] >= mins[1]
                    && rope->m_max[2] >= mins[2] )
                {
                    nverts = 3 * rope->m_num_particles;
                    for ( vi = 0; vi < nverts; ++vi )
                    {
                        if ( trace_point_through_sphere(p0, ud, rope->m_client_verts.frame_verts[0].v[vi], radius_0, &t, hitp) )
                        {
                            if ( vlen > t )
                            {
                                rope->m_flags &= ~2u;
                                rope->m_stable_count = 0;
                                force = local_scale * rope->m_force_scale;
                                v5 = rope->m_particles[vi / 3].p[0];
                                v6 = rope->m_particles[vi / 3].p[1];
                                v7 = rope->m_particles[vi / 3].p[2];
                                f = force * ud[0];
                                f_4 = force * ud[1];
                                f_8 = force * ud[2];
                                p = rope->m_particles[vi / 3].p;
                                *p = (float)(2.0 * v5) + (float)(-1.0 * p[3]);
                                p[1] = (float)(2.0 * v6) + (float)(-1.0 * p[4]);
                                p[2] = (float)(2.0 * v7) + (float)(-1.0 * p[5]);
                                *p = *p + f;
                                p[1] = p[1] + f_4;
                                p[2] = p[2] + f_8;
                                p[3] = v5;
                                p[4] = v6;
                                p[5] = v7;
                                if ( rope->m_health > 0 && !--rope->m_health )
                                {
                                    DeleteConstraint(ri);
                                    if ( LODWORD(rope->m_particles[24].p_prev[7 * rope->m_num_particles]) == 1 )
                                    {
                                        Rope_RemoveAnchor(ri, 0);
                                    }
                                    else if ( rope->m_constraints[rope->m_num_particles].type == ROPE_WORLD_CONSTRAINT )
                                    {
                                        Rope_RemoveAnchor(ri, rope->m_num_particles - 1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static const float ex = 10.0f;
static const float fudge = 0.01f;



void __cdecl Rope_CollideWorld(int rope_index)
{
    float v1; // xmm2_4
    int v2; // edx
    float *v3; // [esp+8h] [ebp-858h]
    float *p_prev; // [esp+Ch] [ebp-854h]
    float v5; // [esp+10h] [ebp-850h]
    float *m_min; // [esp+60h] [ebp-800h]
    float *v7; // [esp+64h] [ebp-7FCh]
    float *m_max; // [esp+68h] [ebp-7F8h]
    float *v9; // [esp+6Ch] [ebp-7F4h]
    int index_hint; // [esp+84h] [ebp-7DCh] BYREF
    float d; // [esp+88h] [ebp-7D8h]
    contact_t *contact; // [esp+8Ch] [ebp-7D4h]
    int hint; // [esp+90h] [ebp-7D0h] BYREF
    float len; // [esp+94h] [ebp-7CCh]
    float dir[3]; // [esp+98h] [ebp-7C8h] BYREF
    float normal[3]; // [esp+A4h] [ebp-7BCh]
    float p1[3]; // [esp+B0h] [ebp-7B0h] BYREF
    par_t *par; // [esp+BCh] [ebp-7A4h]
    float pi[3]; // [esp+C0h] [ebp-7A0h] BYREF
    float p2[3]; // [esp+CCh] [ebp-794h] BYREF
    float p0[3]; // [esp+D8h] [ebp-788h] BYREF
    int i; // [esp+E4h] [ebp-77Ch]
    float fudge_vec[3]; // [esp+E8h] [ebp-778h]
    float expand_vec[3]; // [esp+F4h] [ebp-76Ch] BYREF
    colgeom_visitor_inlined_t<200> visitor; // [esp+100h] [ebp-760h] BYREF
    col_context_t context; // [esp+7BCh] [ebp-A4h] BYREF
    rope_t *rope; // [esp+7E4h] [ebp-7Ch]
    trace_t trace; // [esp+7E8h] [ebp-78h] BYREF
    int mask; // [esp+820h] [ebp-40h]
    trace_t trace1; // [esp+824h] [ebp-3Ch] BYREF

    rope = &g_ropes[rope_index];
    //colgeom_visitor_t::colgeom_visitor_t(&visitor);
    //visitor.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
    //colgeom_visitor_inlined_t<500>::reset(&visitor);
    mask = 0x280EC93;
    //col_context_t::col_context_t(&context);
    if ( rope->m_in_use )
    {
        //colgeom_visitor_inlined_t<500>::reset(&visitor);
        visitor.reset();
        fudge_vec[0] = ex;
        fudge_vec[1] = ex;
        fudge_vec[2] = ex;
        m_max = rope->m_max;
        v9 = rope->m_max;
        rope->m_max[0] = rope->m_max[0] + ex;
        m_max[1] = v9[1] + fudge_vec[1];
        m_max[2] = v9[2] + fudge_vec[2];
        m_min = rope->m_min;
        v7 = rope->m_min;
        rope->m_min[0] = rope->m_min[0] - fudge_vec[0];
        m_min[1] = v7[1] - fudge_vec[1];
        m_min[2] = v7[2] - fudge_vec[2];
        expand_vec[0] = 1.0f;
        expand_vec[1] = 1.0f;
        expand_vec[2] = 1.0f;
        //colgeom_visitor_inlined_t<200>::update(&visitor, rope->m_min, rope->m_max, mask, expand_vec);
        visitor.update(rope->m_min, rope->m_max, mask, expand_vec);
        context.prims = visitor.prims;
        context.nprims = visitor.nprims;
        for ( i = 0; i < rope->m_num_particles; ++i )
        {
            par = &rope->m_particles[i];
            par->flags &= 0xFFFFFFFC;
            if ( g_num_contacts < 600 )
            {
                p0[0] = par->p_prev[0];
                p0[1] = par->p_prev[1];
                p0[2] = par->p_prev[2];
                p1[0] = par->p[0];
                p1[1] = par->p[1];
                p1[2] = par->p[2];
                dir[0] = p1[0] - p0[0];
                dir[1] = p1[1] - p0[1];
                dir[2] = p1[2] - p0[2];
                len = Vec3Length(dir);
                if ( len >= 0.001 )
                {
                    hint = -1;
                    trace_point_vs_env(&trace, p0, p1, &context, &hint);
                    if ( trace.fraction >= 1.0 )
                    {
                        Vec3Copy(par->p, par->p_prev);
                    }
                    else
                    {
                        contact = &g_contacts[g_num_contacts++];
                        Vec3Lerp(p0, p1, trace.fraction * 0.99000001, pi);
                        v1 = (float)((float)((float)(p1[0] - pi[0]) * trace.normal.vec.v[0])
                                             + (float)((float)(p1[1] - pi[1]) * trace.normal.vec.v[1]))
                             + (float)((float)(p1[2] - pi[2]) * trace.normal.vec.v[2]);
                        d = v1 - (float)(fabs(v1) * 0.0099999998);
                        dir[0] = (float)((-(d)) * trace.normal.vec.v[0]) + (float)(p1[0] - pi[0]);
                        dir[1] = (float)((-(d)) * trace.normal.vec.v[1]) + (float)(p1[1] - pi[1]);
                        dir[2] = (float)((-(d)) * trace.normal.vec.v[2]) + (float)(p1[2] - pi[2]);
                        p2[0] = pi[0] + dir[0];
                        p2[1] = pi[1] + dir[1];
                        p2[2] = pi[2] + dir[2];
                        index_hint = -1;
                        trace_point_vs_env(&trace1, pi, p2, &context, &index_hint);
                        if ( trace1.fraction >= 1.0 )
                        {
                            *(_QWORD *)normal = *(_QWORD *)trace.normal.vec.v;
                            normal[2] = trace.normal.vec.v[2];
                        }
                        else
                        {
                            normal[0] = 1.0 * (float)(trace.normal.vec.v[0] + trace1.normal.vec.v[0]);
                            normal[1] = 1.0 * (float)(trace.normal.vec.v[1] + trace1.normal.vec.v[1]);
                            normal[2] = 1.0 * (float)(trace.normal.vec.v[2] + trace1.normal.vec.v[2]);
                            Vec3Lerp(pi, p2, trace1.fraction, pi);
                        }
                        v5 = fudge;
                        contact->point[0] = (float)(fudge * normal[0]) + pi[0];
                        contact->point[1] = (float)(v5 * normal[1]) + pi[1];
                        contact->point[2] = (float)(v5 * normal[2]) + pi[2];
                        p_prev = par->p_prev;
                        par->p_prev[0] = (float)(1.0 * normal[0]) + pi[0];
                        p_prev[1] = (float)(1.0 * normal[1]) + pi[1];
                        p_prev[2] = (float)(1.0 * normal[2]) + pi[2];
                        contact->particle_index = i;
                        v3 = contact->normal;
                        contact->normal[0] = normal[0];
                        v3[1] = normal[1];
                        v3[2] = normal[2];
                        if ( normal[2] <= 0.80000001 )
                            v2 = par->flags | 2;
                        else
                            v2 = par->flags | 1;
                        par->flags = v2;
                    }
                }
            }
        }
    }
}

static const float radius_1 = 20.0f;
static const float force_scale = 5.0f;


void __cdecl Rope_CollideEntitiesHelper(int rope_index, float *origin)
{
    float dist; // [esp+10h] [ebp-38h]
    float dist2; // [esp+14h] [ebp-34h]
    par_t *par; // [esp+18h] [ebp-30h]
    int pi; // [esp+1Ch] [ebp-2Ch]
    rope_t *rope; // [esp+20h] [ebp-28h]
    float f; // [esp+24h] [ebp-24h]
    float f_4; // [esp+28h] [ebp-20h]
    float f_8; // [esp+2Ch] [ebp-1Ch]
    float p; // [esp+30h] [ebp-18h]
    float p_4; // [esp+34h] [ebp-14h]
    float p_8; // [esp+38h] [ebp-10h]
    float center; // [esp+3Ch] [ebp-Ch]
    float center_4; // [esp+40h] [ebp-8h]

    rope = &g_ropes[rope_index];
    center = *origin;
    center_4 = origin[1];
    for ( pi = 0; pi < rope->m_num_particles; ++pi )
    {
        par = &rope->m_particles[pi];
        p = par->p[0];
        p_4 = par->p[1];
        p_8 = par->p[2];
        if ( p_8 > (float)(origin[2] + 10.0) && (float)(origin[2] + 60.0) > p_8 )
        {
            dist2 = (float)((float)((float)(p - center) * (float)(p - center))
                                        + (float)((float)(p_4 - center_4) * (float)(p_4 - center_4)))
                        + (float)((float)(p_8 - p_8) * (float)(p_8 - p_8));
            if ( (float)(radius_1 * radius_1) > dist2 )
            {
                dist = sqrtf(dist2);
                f = (float)((float)(1.0 - (float)(dist / radius_1)) * force_scale)
                    * (float)((float)(1.0 / dist) * (float)(p - center));
                f_4 = (float)((float)(1.0 - (float)(dist / radius_1)) * force_scale)
                        * (float)((float)(1.0 / dist) * (float)(p_4 - center_4));
                f_8 = (float)((float)(1.0 - (float)(dist / radius_1)) * force_scale)
                        * (float)((float)(1.0 / dist) * (float)(p_8 - p_8));
                par->p[0] = (float)(2.0 * p) + (float)(-1.0 * par->p0[0]);
                par->p[1] = (float)(2.0 * p_4) + (float)(-1.0 * par->p0[1]);
                par->p[2] = (float)(2.0 * p_8) + (float)(-1.0 * par->p0[2]);
                par->p[0] = par->p[0] + f;
                par->p[1] = par->p[1] + f_4;
                par->p[2] = par->p[2] + f_8;
                par->p0[0] = p;
                par->p0[1] = p_4;
                par->p0[2] = p_8;
            }
        }
    }
}

void __cdecl Rope_CollideEntities(int rope_index)
{
    centity_s *cent; // [esp+8h] [ebp-58h]
    int i; // [esp+Ch] [ebp-54h]
    int numEnts; // [esp+14h] [ebp-4Ch]
    int entitylist[17]; // [esp+18h] [ebp-48h] BYREF
    rope_t *rope; // [esp+5Ch] [ebp-4h]

    rope = &g_ropes[rope_index];
    if ( rope->m_in_use )
    {
        if ( CL_LocalClient_IsActive(0) && cg_pmove[0].ps )
            Rope_CollideEntitiesHelper(rope_index, cg_pmove[0].ps->origin);
        numEnts = CG_AreaEntities(rope->m_min, rope->m_max, entitylist, 16, 0x2008000);
        for ( i = 0; i < numEnts; ++i )
        {
            cent = CG_GetEntity(0, entitylist[i]);
            Rope_CollideEntitiesHelper(rope_index, cent->pose.origin);
        }
    }
}

