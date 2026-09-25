// nx_d3d9_shader.h -- D3D9 shader-model 3 bytecode to GLSL ES 3.00.
//
// The engine's shaders reach the device as the compiled D3D9 token streams the
// fastfiles carry. NX_TranslateD3D9Shader turns one into GLSL that the GL 4.3
// core context compiles as-is (#version 300 es is core there, through
// ARB_ES3_compatibility). Pure CPU work, any thread; nx_d3d9_null.cpp compiles
// the result on the thread that owns the context.
//
// The translator is adapted from riicchhaarrd/KisakBlack's web port
// (src/gfx_gl/gl_shader.cpp, GPL-3.0, as this tree is), where it drives the
// same engine's shaders through WebGL2 into maps. See nx_d3d9_shader.cpp for
// what was changed for this backend.
#pragma once

#include <d3d9.h>

// What the device needs to know about a translated shader besides its text.
struct NxShaderInfo {
    bool isPixel;
    unsigned version;           // e.g. 0x0300 for vs_3_0 / ps_3_0
    unsigned samplerMask;       // bit N: the shader samples sN
    unsigned char samplerDim[16]; // per sampler: 2 = 2D, 3 = cube, 4 = volume (D3DSTT_*)
    unsigned unknownOps;        // opcodes the translator skipped; nonzero = output is suspect
    unsigned firstUnknownOp;    // the first of them, for the report
};

// Returns a malloc'd, NUL-terminated GLSL translation unit (free() it), or NULL
// when the stream is not one the translator can read at all.
//
// Names the output uses, which the device binds by:
//   vertex inputs   aPos, aNormal, aColor0, aTexCoord3, ... -- NX_ShaderAttribLocation
//   constants       vsc[] (vertex), psc[] (pixel), each sized to the highest
//                   register the shader reads
//   samplers        sN in a pixel shader (texture unit N), svN in a vertex shader
//                   (unit NX_SHADER_VS_SAMPLER_UNIT + N)
//   uniforms        nxHalfPixel (vertex: D3D9's half-pixel offset, (-1/W, 1/H)),
//                   uAlphaTestFunc and uAlphaRef (pixel: the D3D9 alpha test,
//                   D3DCMP_* or 0 for off, and the reference in 0..1)
char *NX_TranslateD3D9Shader(const DWORD *tokens, NxShaderInfo *info);

enum { NX_SHADER_VS_SAMPLER_UNIT = 16, NX_SHADER_VS_SAMPLERS = 4 };

// One generic attribute location per (usage, usage index), the same for the
// translator, the program link and the vertex setup. -1 for a usage with no
// location (it then reads as a constant).
int NX_ShaderAttribLocation(int usage, int usageIndex);
// The GLSL name bound to that location, into buf.
void NX_ShaderAttribName(int usage, int usageIndex, char *buf, unsigned size);
// Every (usage, index) pair that has a location, for binding them all before a
// link: fills up to `max` pairs and returns how many.
int NX_ShaderAttribAll(int (*pairs)[2], int max);
