#include "r_warn.h"
#include <cstdarg>
#include <universal/assertive.h>
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <win32/win_shared.h>
#include "r_init.h"

const dvar_s *r_warningRepeatDelay;
unsigned int s_warnCount[49];

const char *s_warnFormat[49] =
{
  "entity buffer exceeded - not drawing model\n",
  "too many existing models (more than %i)\n",
  "too many existing special models (more than %i)\n",
  "too many texture overrides (more than %i)\n",
  "model light cache alloc failed - not drawing model\n",
  "too many scene entities (more than %i)\n",
  "TEMP_SKIN_BUF_SIZE exceeded - not skinning surface\n",
  "R_MAX_SKINNED_CACHE_ASSERT(((1024 * 256) - (1024 * 5))) exceeded - not drawing surface\n",
  "MAX_SCENE_SURFS_SIZE(131072*2) exceeded - not drawing surface\n",
  "Portal plane buffer full - flushing\n",
  "GFX_PARTICLE_CLOUD_LIMIT(256) exceeded - not drawing particle cloud\n",
  "MAX_ADDED_DLIGHTS(32) exceeded.\n",
  "Too many visible static models - not drawing static model\n",
  "MAX_DRAWSURFS(32768) exceeded - not drawing geometry\n",
  "GFX_CODE_MESH_LIMIT(2048) exceeded - not drawing code mesh\n",
  "GFX_MARK_MESH_LIMIT(1536) exceeded - not drawing mark mesh\n",
  "Max scene drawsurfs exceed from %s - not drawing surface\n",
  "Max fx drawsurfs %i exceed for region %i - not drawing surface\n",
  "non effect material '%s' used for effect (or code geom) '%s'\n",
  "non impact mark material '%s' used for impact mark\n",
  "PRIM_DRAW_SURF_BUFFER_SIZE((128 * 512)) exceeded - not drawing surface\n",
  "command buffer overflow - not drawing surface\n",
  "Missing decl '%s' techset '%s' tech '%s' shader '%s' (ignore for debug settings).\n",
  "Max dyn ent refs exceeded\n",
  "Max scene dobj refs (%i) exceeded\n",
  "Max scene model refs (%i) exceeded\n",
  "Max scene brush refs (%i) exceeded\n",
  "GFX_CODE_MESH_INDEX_LIMIT((2048 * 6 * 5)) exceeded\n",
  "GFX_CODE_MESH_VERT_LIMIT((2048 * 4 * 2)) exceeded\n",
  "GFX_CODE_MESH_ARGS_LIMIT(256) exceeded\n",
  "GFX_MARK_MESH_INDEX_LIMIT((1536 * 6)) exceeded\n",
  "GFX_MARK_MESH_VERT_LIMIT((1536 * 4)) exceeded\n",
  "Out of debug memory for (%s)\n",
  "FX_SPOT_LIGHT_LIMIT(1) exceeded - not spawning spot light effect\n",
  "FX_ELEM_LIMIT(2048) exceeded - not spawning fx elem\n",
  "Dynamic vertex buffer overflow\n",
  "worker command size exceeded for type %i\n",
  "Unknown static model shader (%s)\n",
  "Unknown xmodel shader\n",
  "DYNAMIC_INDEX_BUFFER_SIZE exceeded - speed warning\n",
  "GFX_MICROCODE_BUFFER_SIZE exceeded - things won't look right\n",
  "Too many light grid points\n",
  "too many static models for foliage system (more than %i)\n",
  "too many transfer requests (more than %i)\n",
  "max %i shader constant sets exceeded\n",
  "max %i dynsmodel drawstates exceeded\n",
  "%s is missing %s(%d), using %s(%d)\n",
  "Marks probe mismatch %d,%d mark %s surf %s\n",
  "Marks lmap mismatch %d,%d mark %s surf %s\n"
};



void R_WarnOncePerFrame(GfxWarningType warnType, ...)
{
    char message[1028]; // [esp+0h] [ebp-410h] BYREF
    float frameRate; // [esp+408h] [ebp-8h]
#ifdef KISAK_NX
    va_list vargs;
#else
    char *vargs; // [esp+40Ch] [ebp-4h]
#endif
    va_list va; // [esp+41Ch] [ebp+Ch] BYREF

    va_start(va, warnType);
    if ( !r_warningRepeatDelay
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_warn.cpp", 163, 0, "%s", "r_warningRepeatDelay") )
    {
        __debugbreak();
    }
    frameRate = R_UpdateFrameRate();
    if ( s_warnCount[warnType] < rg.frontEndFrameCount )
    {
        s_warnCount[warnType] = rg.frontEndFrameCount + (int)(float)(frameRate * r_warningRepeatDelay->current.value);
        va_copy(vargs, va);
        _vsnprintf(message, 0x400u, s_warnFormat[warnType], va);
#ifdef KISAK_NX
        va_end(vargs);
#else
        vargs = 0;
#endif
        Com_PrintWarning(8, "%s", message);
    }
}

unsigned int frameCount;
int previous;
float frameRate;
double __cdecl R_UpdateFrameRate()
{
    int frameTime; // [esp+0h] [ebp-8h]
    int current; // [esp+4h] [ebp-4h]

    if (frameCount != rg.frontEndFrameCount)
    {
        if (frameCount)
        {
            if (frameCount + 1 == rg.frontEndFrameCount)
            {
                current = Sys_Milliseconds();
                frameTime = current - previous;
                previous = current;
                if (!frameTime)
                    frameTime = 1;
                if (frameTime >= 0)
                    frameRate = 1000.0 / (float)frameTime;
                else
                    frameRate = 0.0f;
            }
            else
            {
                frameRate = 0.0f;
            }
        }
        else
        {
            previous = Sys_Milliseconds();
        }
        frameCount = rg.frontEndFrameCount;
    }
    return frameRate;
}

void __cdecl R_WarnInitDvars()
{
    r_warningRepeatDelay = _Dvar_RegisterFloat(
                                                     "r_warningRepeatDelay",
                                                     5.0,
                                                     0.0,
                                                     30.0,
                                                     0,
                                                     "Number of seconds after displaying a \"per-frame\" warning before it will display again");
}

