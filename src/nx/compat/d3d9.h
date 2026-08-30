// d3d9.h -- Direct3D 9 compatibility header for the Switch build.
//
// Declares the exact D3D9 API surface KisakBlack uses (interfaces as plain
// C++ classes -- no COM), with numerically-exact enums/constants/HRESULTs
// because the decompiled code casts raw ints to these types and compares
// raw HRESULT values. Backed by a null-device implementation in
// src/nx/nx_d3d9_null.cpp: resources have real backing memory (Lock/Unlock
// work), queries complete immediately, draws are discarded.
#ifndef NX_COMPAT_D3D9_H
#define NX_COMPAT_D3D9_H

#include "windows.h"

#define D3D_SDK_VERSION 32
#define D3DADAPTER_DEFAULT 0

typedef DWORD D3DCOLOR;
#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0,c1,c2,c3) \
    ((DWORD)(BYTE)(c0) | ((DWORD)(BYTE)(c1) << 8) | ((DWORD)(BYTE)(c2) << 16) | ((DWORD)(BYTE)(c3) << 24))
#endif

// ---------------------------------------------------------------------------
// HRESULTs (raw values compared in code)
// ---------------------------------------------------------------------------
#define D3D_OK                  ((HRESULT)0)
#define D3DERR_DEVICELOST       ((HRESULT)0x88760868)
#define D3DERR_DEVICENOTRESET   ((HRESULT)0x88760869)
#define D3DERR_NOTAVAILABLE     ((HRESULT)0x8876086A)
#define D3DERR_OUTOFVIDEOMEMORY ((HRESULT)0x8876017C)
#define D3DERR_INVALIDCALL      ((HRESULT)0x8876086C)
#define D3DERR_DRIVERINTERNALERROR ((HRESULT)0x88760827)

// ---------------------------------------------------------------------------
// Enums (numeric values load-bearing)
// ---------------------------------------------------------------------------
typedef enum _D3DFORMAT {
    D3DFMT_UNKNOWN       = 0,
    D3DFMT_R8G8B8        = 20,
    D3DFMT_A8R8G8B8      = 21,
    D3DFMT_X8R8G8B8      = 22,
    D3DFMT_R5G6B5        = 23,
    D3DFMT_X1R5G5B5      = 24,
    D3DFMT_A1R5G5B5      = 25,
    D3DFMT_A4R4G4B4      = 26,
    D3DFMT_R3G3B2        = 27,
    D3DFMT_A8            = 28,
    D3DFMT_A8R3G3B2      = 29,
    D3DFMT_X4R4G4B4      = 30,
    D3DFMT_A2B10G10R10   = 31,
    D3DFMT_A8B8G8R8      = 32,
    D3DFMT_X8B8G8R8      = 33,
    D3DFMT_G16R16        = 34,
    D3DFMT_A2R10G10B10   = 35,
    D3DFMT_A16B16G16R16  = 36,
    D3DFMT_A8P8          = 40,
    D3DFMT_P8            = 41,
    D3DFMT_L8            = 50,
    D3DFMT_A8L8          = 51,
    D3DFMT_A4L4          = 52,
    D3DFMT_V8U8          = 60,
    D3DFMT_L6V5U5        = 61,
    D3DFMT_X8L8V8U8      = 62,
    D3DFMT_Q8W8V8U8      = 63,
    D3DFMT_V16U16        = 64,
    D3DFMT_A2W10V10U10   = 67,
    D3DFMT_UYVY          = MAKEFOURCC('U','Y','V','Y'),
    D3DFMT_R8G8_B8G8     = MAKEFOURCC('R','G','B','G'),
    D3DFMT_YUY2          = MAKEFOURCC('Y','U','Y','2'),
    D3DFMT_G8R8_G8B8     = MAKEFOURCC('G','R','G','B'),
    D3DFMT_DXT1          = MAKEFOURCC('D','X','T','1'),
    D3DFMT_DXT2          = MAKEFOURCC('D','X','T','2'),
    D3DFMT_DXT3          = MAKEFOURCC('D','X','T','3'),
    D3DFMT_DXT4          = MAKEFOURCC('D','X','T','4'),
    D3DFMT_DXT5          = MAKEFOURCC('D','X','T','5'),
    D3DFMT_D16_LOCKABLE  = 70,
    D3DFMT_D32           = 71,
    D3DFMT_D15S1         = 73,
    D3DFMT_D24S8         = 75,
    D3DFMT_D24X8         = 77,
    D3DFMT_D24X4S4       = 79,
    D3DFMT_D16           = 80,
    D3DFMT_L16           = 81,
    D3DFMT_D32F_LOCKABLE = 82,
    D3DFMT_D24FS8        = 83,
    D3DFMT_VERTEXDATA    = 100,
    D3DFMT_INDEX16       = 101,
    D3DFMT_INDEX32       = 102,
    D3DFMT_Q16W16V16U16  = 110,
    D3DFMT_R16F          = 111,
    D3DFMT_G16R16F       = 112,
    D3DFMT_A16B16G16R16F = 113,
    D3DFMT_R32F          = 114,
    D3DFMT_G32R32F       = 115,
    D3DFMT_A32B32G32R32F = 116,
    D3DFMT_CxV8U8        = 117,
    D3DFMT_FORCE_DWORD   = 0x7fffffff
} D3DFORMAT;

typedef enum _D3DPOOL {
    D3DPOOL_DEFAULT   = 0,
    D3DPOOL_MANAGED   = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH   = 3,
    D3DPOOL_FORCE_DWORD = 0x7fffffff
} D3DPOOL;

#define D3DUSAGE_RENDERTARGET       0x00000001u
#define D3DUSAGE_DEPTHSTENCIL       0x00000002u
#define D3DUSAGE_WRITEONLY          0x00000008u
#define D3DUSAGE_SOFTWAREPROCESSING 0x00000010u
#define D3DUSAGE_DONOTCLIP          0x00000020u
#define D3DUSAGE_POINTS             0x00000040u
#define D3DUSAGE_RTPATCHES          0x00000080u
#define D3DUSAGE_NPATCHES           0x00000100u
#define D3DUSAGE_DYNAMIC            0x00000200u
#define D3DUSAGE_AUTOGENMIPMAP      0x00000400u
#define D3DUSAGE_QUERY_VERTEXTEXTURE 0x00100000u

typedef enum _D3DDEVTYPE {
    D3DDEVTYPE_HAL = 1,
    D3DDEVTYPE_REF = 2,
    D3DDEVTYPE_SW  = 3,
    D3DDEVTYPE_NULLREF = 4,
    D3DDEVTYPE_FORCE_DWORD = 0x7fffffff
} D3DDEVTYPE;

#define D3DCREATE_FPU_PRESERVE              0x02
#define D3DCREATE_MULTITHREADED             0x04
#define D3DCREATE_PUREDEVICE                0x10
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING 0x20
#define D3DCREATE_HARDWARE_VERTEXPROCESSING 0x40
#define D3DCREATE_MIXED_VERTEXPROCESSING    0x80

typedef enum _D3DMULTISAMPLE_TYPE {
    D3DMULTISAMPLE_NONE        = 0,
    D3DMULTISAMPLE_NONMASKABLE = 1,
    D3DMULTISAMPLE_2_SAMPLES   = 2,
    D3DMULTISAMPLE_4_SAMPLES   = 4,
    D3DMULTISAMPLE_8_SAMPLES   = 8,
    D3DMULTISAMPLE_16_SAMPLES  = 16,
    D3DMULTISAMPLE_FORCE_DWORD = 0x7fffffff
} D3DMULTISAMPLE_TYPE;

typedef enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD = 1,
    D3DSWAPEFFECT_FLIP    = 2,
    D3DSWAPEFFECT_COPY    = 3,
    D3DSWAPEFFECT_FORCE_DWORD = 0x7fffffff
} D3DSWAPEFFECT;

#define D3DPRESENT_INTERVAL_DEFAULT   0x00000000u
#define D3DPRESENT_INTERVAL_ONE       0x00000001u
#define D3DPRESENT_INTERVAL_IMMEDIATE 0x80000000u
#define D3DPRESENT_RATE_DEFAULT       0

typedef enum _D3DRESOURCETYPE {
    D3DRTYPE_SURFACE       = 1,
    D3DRTYPE_VOLUME        = 2,
    D3DRTYPE_TEXTURE       = 3,
    D3DRTYPE_VOLUMETEXTURE = 4,
    D3DRTYPE_CUBETEXTURE   = 5,
    D3DRTYPE_VERTEXBUFFER  = 6,
    D3DRTYPE_INDEXBUFFER   = 7,
    D3DRTYPE_FORCE_DWORD   = 0x7fffffff
} D3DRESOURCETYPE;

typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST     = 1,
    D3DPT_LINELIST      = 2,
    D3DPT_LINESTRIP     = 3,
    D3DPT_TRIANGLELIST  = 4,
    D3DPT_TRIANGLESTRIP = 5,
    D3DPT_TRIANGLEFAN   = 6,
    D3DPT_FORCE_DWORD   = 0x7fffffff
} D3DPRIMITIVETYPE;

typedef enum _D3DRENDERSTATETYPE {
    D3DRS_ZENABLE          = 7,
    D3DRS_FILLMODE         = 8,
    D3DRS_SHADEMODE        = 9,
    D3DRS_ZWRITEENABLE     = 14,
    D3DRS_ALPHATESTENABLE  = 15,
    D3DRS_LASTPIXEL        = 16,
    D3DRS_SRCBLEND         = 19,
    D3DRS_DESTBLEND        = 20,
    D3DRS_CULLMODE         = 22,
    D3DRS_ZFUNC            = 23,
    D3DRS_ALPHAREF         = 24,
    D3DRS_ALPHAFUNC        = 25,
    D3DRS_DITHERENABLE     = 26,
    D3DRS_ALPHABLENDENABLE = 27,
    D3DRS_FOGENABLE        = 28,
    D3DRS_SPECULARENABLE   = 29,
    D3DRS_FOGCOLOR         = 34,
    D3DRS_FOGTABLEMODE     = 35,
    D3DRS_FOGSTART         = 36,
    D3DRS_FOGEND           = 37,
    D3DRS_FOGDENSITY       = 38,
    D3DRS_RANGEFOGENABLE   = 48,
    D3DRS_STENCILENABLE    = 52,
    D3DRS_STENCILFAIL      = 53,
    D3DRS_STENCILZFAIL     = 54,
    D3DRS_STENCILPASS      = 55,
    D3DRS_STENCILFUNC      = 56,
    D3DRS_STENCILREF       = 57,
    D3DRS_STENCILMASK      = 58,
    D3DRS_STENCILWRITEMASK = 59,
    D3DRS_TEXTUREFACTOR    = 60,
    D3DRS_WRAP0            = 128,
    D3DRS_WRAP1            = 129,
    D3DRS_WRAP2            = 130,
    D3DRS_WRAP3            = 131,
    D3DRS_WRAP4            = 132,
    D3DRS_WRAP5            = 133,
    D3DRS_WRAP6            = 134,
    D3DRS_WRAP7            = 135,
    D3DRS_CLIPPING         = 136,
    D3DRS_LIGHTING         = 137,
    D3DRS_AMBIENT          = 139,
    D3DRS_FOGVERTEXMODE    = 140,
    D3DRS_COLORVERTEX      = 141,
    D3DRS_LOCALVIEWER      = 142,
    D3DRS_NORMALIZENORMALS = 143,
    D3DRS_DIFFUSEMATERIALSOURCE  = 145,
    D3DRS_SPECULARMATERIALSOURCE = 146,
    D3DRS_AMBIENTMATERIALSOURCE  = 147,
    D3DRS_EMISSIVEMATERIALSOURCE = 148,
    D3DRS_VERTEXBLEND      = 151,
    D3DRS_CLIPPLANEENABLE  = 152,
    D3DRS_POINTSIZE        = 154,
    D3DRS_POINTSIZE_MIN    = 155,
    D3DRS_POINTSPRITEENABLE = 156,
    D3DRS_POINTSCALEENABLE = 157,
    D3DRS_POINTSCALE_A     = 158,
    D3DRS_POINTSCALE_B     = 159,
    D3DRS_POINTSCALE_C     = 160,
    D3DRS_MULTISAMPLEANTIALIAS = 161,
    D3DRS_MULTISAMPLEMASK  = 162,
    D3DRS_PATCHEDGESTYLE   = 163,
    D3DRS_DEBUGMONITORTOKEN = 165,
    D3DRS_POINTSIZE_MAX    = 166,
    D3DRS_INDEXEDVERTEXBLENDENABLE = 167,
    D3DRS_COLORWRITEENABLE = 168,
    D3DRS_TWEENFACTOR      = 170,
    D3DRS_BLENDOP          = 171,
    D3DRS_POSITIONDEGREE   = 172,
    D3DRS_NORMALDEGREE     = 173,
    D3DRS_SCISSORTESTENABLE = 174,
    D3DRS_SLOPESCALEDEPTHBIAS = 175,
    D3DRS_ANTIALIASEDLINEENABLE = 176,
    D3DRS_MINTESSELLATIONLEVEL = 178,
    D3DRS_MAXTESSELLATIONLEVEL = 179,
    D3DRS_ADAPTIVETESS_X   = 180,
    D3DRS_ADAPTIVETESS_Y   = 181,
    D3DRS_ADAPTIVETESS_Z   = 182,
    D3DRS_ADAPTIVETESS_W   = 183,
    D3DRS_ENABLEADAPTIVETESSELLATION = 184,
    D3DRS_TWOSIDEDSTENCILMODE = 185,
    D3DRS_CCW_STENCILFAIL  = 186,
    D3DRS_CCW_STENCILZFAIL = 187,
    D3DRS_CCW_STENCILPASS  = 188,
    D3DRS_CCW_STENCILFUNC  = 189,
    D3DRS_COLORWRITEENABLE1 = 190,
    D3DRS_COLORWRITEENABLE2 = 191,
    D3DRS_COLORWRITEENABLE3 = 192,
    D3DRS_BLENDFACTOR      = 193,
    D3DRS_SRGBWRITEENABLE  = 194,
    D3DRS_DEPTHBIAS        = 195,
    D3DRS_SEPARATEALPHABLENDENABLE = 206,
    D3DRS_SRCBLENDALPHA    = 207,
    D3DRS_DESTBLENDALPHA   = 208,
    D3DRS_BLENDOPALPHA     = 209,
    D3DRS_FORCE_DWORD      = 0x7fffffff
} D3DRENDERSTATETYPE;

typedef enum _D3DSAMPLERSTATETYPE {
    D3DSAMP_ADDRESSU      = 1,
    D3DSAMP_ADDRESSV      = 2,
    D3DSAMP_ADDRESSW      = 3,
    D3DSAMP_BORDERCOLOR   = 4,
    D3DSAMP_MAGFILTER     = 5,
    D3DSAMP_MINFILTER     = 6,
    D3DSAMP_MIPFILTER     = 7,
    D3DSAMP_MIPMAPLODBIAS = 8,
    D3DSAMP_MAXMIPLEVEL   = 9,
    D3DSAMP_MAXANISOTROPY = 10,
    D3DSAMP_SRGBTEXTURE   = 11,
    D3DSAMP_ELEMENTINDEX  = 12,
    D3DSAMP_DMAPOFFSET    = 13,
    D3DSAMP_FORCE_DWORD   = 0x7fffffff
} D3DSAMPLERSTATETYPE;

typedef enum _D3DTEXTURESTAGESTATETYPE {
    D3DTSS_COLOROP   = 1,
    D3DTSS_COLORARG1 = 2,
    D3DTSS_COLORARG2 = 3,
    D3DTSS_ALPHAOP   = 4,
    D3DTSS_ALPHAARG1 = 5,
    D3DTSS_ALPHAARG2 = 6,
    D3DTSS_TEXCOORDINDEX = 11,
    D3DTSS_TEXTURETRANSFORMFLAGS = 24,
    D3DTSS_FORCE_DWORD = 0x7fffffff
} D3DTEXTURESTAGESTATETYPE;

typedef enum _D3DTEXTUREOP {
    D3DTOP_DISABLE    = 1,
    D3DTOP_SELECTARG1 = 2,
    D3DTOP_SELECTARG2 = 3,
    D3DTOP_MODULATE   = 4,
    D3DTOP_MODULATE2X = 5,
    D3DTOP_MODULATE4X = 6,
    D3DTOP_ADD        = 7,
    D3DTOP_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREOP;

#define D3DTA_DIFFUSE  0x00000000
#define D3DTA_CURRENT  0x00000001
#define D3DTA_TEXTURE  0x00000002
#define D3DTA_TFACTOR  0x00000003
#define D3DTA_SPECULAR 0x00000004

typedef enum _D3DTEXTUREFILTERTYPE {
    D3DTEXF_NONE   = 0,
    D3DTEXF_POINT  = 1,
    D3DTEXF_LINEAR = 2,
    D3DTEXF_ANISOTROPIC = 3,
    D3DTEXF_PYRAMIDALQUAD = 6,
    D3DTEXF_GAUSSIANQUAD  = 7,
    D3DTEXF_FORCE_DWORD   = 0x7fffffff
} D3DTEXTUREFILTERTYPE;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP   = 1,
    D3DTADDRESS_MIRROR = 2,
    D3DTADDRESS_CLAMP  = 3,
    D3DTADDRESS_BORDER = 4,
    D3DTADDRESS_MIRRORONCE = 5,
    D3DTADDRESS_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREADDRESS;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO         = 1,
    D3DBLEND_ONE          = 2,
    D3DBLEND_SRCCOLOR     = 3,
    D3DBLEND_INVSRCCOLOR  = 4,
    D3DBLEND_SRCALPHA     = 5,
    D3DBLEND_INVSRCALPHA  = 6,
    D3DBLEND_DESTALPHA    = 7,
    D3DBLEND_INVDESTALPHA = 8,
    D3DBLEND_DESTCOLOR    = 9,
    D3DBLEND_INVDESTCOLOR = 10,
    D3DBLEND_SRCALPHASAT  = 11,
    D3DBLEND_BLENDFACTOR  = 14,
    D3DBLEND_INVBLENDFACTOR = 15,
    D3DBLEND_FORCE_DWORD  = 0x7fffffff
} D3DBLEND;

typedef enum _D3DBLENDOP {
    D3DBLENDOP_ADD         = 1,
    D3DBLENDOP_SUBTRACT    = 2,
    D3DBLENDOP_REVSUBTRACT = 3,
    D3DBLENDOP_MIN         = 4,
    D3DBLENDOP_MAX         = 5,
    D3DBLENDOP_FORCE_DWORD = 0x7fffffff
} D3DBLENDOP;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER        = 1,
    D3DCMP_LESS         = 2,
    D3DCMP_EQUAL        = 3,
    D3DCMP_LESSEQUAL    = 4,
    D3DCMP_GREATER      = 5,
    D3DCMP_NOTEQUAL     = 6,
    D3DCMP_GREATEREQUAL = 7,
    D3DCMP_ALWAYS       = 8,
    D3DCMP_FORCE_DWORD  = 0x7fffffff
} D3DCMPFUNC;

typedef enum _D3DSTENCILOP {
    D3DSTENCILOP_KEEP    = 1,
    D3DSTENCILOP_ZERO    = 2,
    D3DSTENCILOP_REPLACE = 3,
    D3DSTENCILOP_INCRSAT = 4,
    D3DSTENCILOP_DECRSAT = 5,
    D3DSTENCILOP_INVERT  = 6,
    D3DSTENCILOP_INCR    = 7,
    D3DSTENCILOP_DECR    = 8,
    D3DSTENCILOP_FORCE_DWORD = 0x7fffffff
} D3DSTENCILOP;

typedef enum _D3DCULL {
    D3DCULL_NONE = 1,
    D3DCULL_CW   = 2,
    D3DCULL_CCW  = 3,
    D3DCULL_FORCE_DWORD = 0x7fffffff
} D3DCULL;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT     = 1,
    D3DFILL_WIREFRAME = 2,
    D3DFILL_SOLID     = 3,
    D3DFILL_FORCE_DWORD = 0x7fffffff
} D3DFILLMODE;

typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE = 0,
    D3DZB_TRUE  = 1,
    D3DZB_USEW  = 2,
    D3DZB_FORCE_DWORD = 0x7fffffff
} D3DZBUFFERTYPE;

#define D3DCLEAR_TARGET  0x1u
#define D3DCLEAR_ZBUFFER 0x2u
#define D3DCLEAR_STENCIL 0x4u

#define D3DLOCK_READONLY    0x00000010u
#define D3DLOCK_DISCARD     0x00002000u
#define D3DLOCK_NOOVERWRITE 0x00001000u
#define D3DLOCK_NOSYSLOCK   0x00000800u
#define D3DLOCK_DONOTWAIT   0x00004000u

typedef enum _D3DQUERYTYPE {
    D3DQUERYTYPE_VCACHE = 4,
    D3DQUERYTYPE_RESOURCEMANAGER = 5,
    D3DQUERYTYPE_VERTEXSTATS = 6,
    D3DQUERYTYPE_EVENT = 8,
    D3DQUERYTYPE_OCCLUSION = 9,
    D3DQUERYTYPE_TIMESTAMP = 10,
    D3DQUERYTYPE_FORCE_DWORD = 0x7fffffff
} D3DQUERYTYPE;

#define D3DISSUE_END   0x1u
#define D3DISSUE_BEGIN 0x2u
#define D3DGETDATA_FLUSH 0x1u

typedef enum _D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5,
    D3DCUBEMAP_FACE_FORCE_DWORD = 0x7fffffff
} D3DCUBEMAP_FACES;

typedef enum _D3DBACKBUFFER_TYPE {
    D3DBACKBUFFER_TYPE_MONO = 0,
    D3DBACKBUFFER_TYPE_LEFT = 1,
    D3DBACKBUFFER_TYPE_RIGHT = 2,
    D3DBACKBUFFER_TYPE_FORCE_DWORD = 0x7fffffff
} D3DBACKBUFFER_TYPE;

typedef enum _D3DDECLTYPE {
    D3DDECLTYPE_FLOAT1    = 0,
    D3DDECLTYPE_FLOAT2    = 1,
    D3DDECLTYPE_FLOAT3    = 2,
    D3DDECLTYPE_FLOAT4    = 3,
    D3DDECLTYPE_D3DCOLOR  = 4,
    D3DDECLTYPE_UBYTE4    = 5,
    D3DDECLTYPE_SHORT2    = 6,
    D3DDECLTYPE_SHORT4    = 7,
    D3DDECLTYPE_UBYTE4N   = 8,
    D3DDECLTYPE_SHORT2N   = 9,
    D3DDECLTYPE_SHORT4N   = 10,
    D3DDECLTYPE_USHORT2N  = 11,
    D3DDECLTYPE_USHORT4N  = 12,
    D3DDECLTYPE_UDEC3     = 13,
    D3DDECLTYPE_DEC3N     = 14,
    D3DDECLTYPE_FLOAT16_2 = 15,
    D3DDECLTYPE_FLOAT16_4 = 16,
    D3DDECLTYPE_UNUSED    = 17
} D3DDECLTYPE;

typedef enum _D3DDECLMETHOD {
    D3DDECLMETHOD_DEFAULT = 0,
    D3DDECLMETHOD_PARTIALU = 1,
    D3DDECLMETHOD_PARTIALV = 2,
    D3DDECLMETHOD_CROSSUV = 3,
    D3DDECLMETHOD_UV = 4,
    D3DDECLMETHOD_LOOKUP = 5,
    D3DDECLMETHOD_LOOKUPPRESAMPLED = 6
} D3DDECLMETHOD;

typedef enum _D3DDECLUSAGE {
    D3DDECLUSAGE_POSITION = 0,
    D3DDECLUSAGE_BLENDWEIGHT = 1,
    D3DDECLUSAGE_BLENDINDICES = 2,
    D3DDECLUSAGE_NORMAL = 3,
    D3DDECLUSAGE_PSIZE = 4,
    D3DDECLUSAGE_TEXCOORD = 5,
    D3DDECLUSAGE_TANGENT = 6,
    D3DDECLUSAGE_BINORMAL = 7,
    D3DDECLUSAGE_TESSFACTOR = 8,
    D3DDECLUSAGE_POSITIONT = 9,
    D3DDECLUSAGE_COLOR = 10,
    D3DDECLUSAGE_FOG = 11,
    D3DDECLUSAGE_DEPTH = 12,
    D3DDECLUSAGE_SAMPLE = 13
} D3DDECLUSAGE;

#define D3DDECL_END() { 0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0 }

#define D3DFVF_XYZ      0x002u
#define D3DFVF_XYZRHW   0x004u
#define D3DFVF_NORMAL   0x010u
#define D3DFVF_DIFFUSE  0x040u
#define D3DFVF_SPECULAR 0x080u
#define D3DFVF_TEX1     0x100u

// caps bits actually consulted
#define D3DCAPS2_FULLSCREENGAMMA     0x00020000u
#define D3DPTEXTURECAPS_POW2                 0x00000002u
#define D3DPTEXTURECAPS_NONPOW2CONDITIONAL   0x00000100u
#define D3DPTEXTURECAPS_MIPCUBEMAP           0x00010000u
#define D3DPTFILTERCAPS_MINFANISOTROPIC      0x00000400u
#define D3DPTFILTERCAPS_MAGFANISOTROPIC      0x04000000u
#define D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS   0x02000000u
#define D3DPRASTERCAPS_DEPTHBIAS             0x04000000u
#define D3DVS_VERSION(major, minor) (0xFFFE0000u | ((major) << 8) | (minor))
#define D3DPS_VERSION(major, minor) (0xFFFF0000u | ((major) << 8) | (minor))

// ---------------------------------------------------------------------------
// Structs (exact Windows field order; several are offset-poked)
// ---------------------------------------------------------------------------
typedef struct _D3DDISPLAYMODE {
    UINT Width;
    UINT Height;
    UINT RefreshRate;
    D3DFORMAT Format;
} D3DDISPLAYMODE;

typedef struct _D3DPRESENT_PARAMETERS_ {
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    D3DFORMAT           BackBufferFormat;
    UINT                BackBufferCount;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD               MultiSampleQuality;
    D3DSWAPEFFECT       SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    D3DFORMAT           AutoDepthStencilFormat;
    DWORD               Flags;
    UINT                FullScreen_RefreshRateInHz;
    UINT                PresentationInterval;
} D3DPRESENT_PARAMETERS;

typedef struct _D3DVSHADERCAPS2_0 {
    DWORD Caps;
    INT   DynamicFlowControlDepth;
    INT   NumTemps;
    INT   StaticFlowControlDepth;
} D3DVSHADERCAPS2_0;

typedef struct _D3DPSHADERCAPS2_0 {
    DWORD Caps;
    INT   DynamicFlowControlDepth;
    INT   NumTemps;
    INT   StaticFlowControlDepth;
    INT   NumInstructionSlots;
} D3DPSHADERCAPS2_0;

typedef struct _D3DCAPS9 {
    D3DDEVTYPE DeviceType;
    UINT   AdapterOrdinal;
    DWORD  Caps;
    DWORD  Caps2;
    DWORD  Caps3;
    DWORD  PresentationIntervals;
    DWORD  CursorCaps;
    DWORD  DevCaps;
    DWORD  PrimitiveMiscCaps;
    DWORD  RasterCaps;
    DWORD  ZCmpCaps;
    DWORD  SrcBlendCaps;
    DWORD  DestBlendCaps;
    DWORD  AlphaCmpCaps;
    DWORD  ShadeCaps;
    DWORD  TextureCaps;
    DWORD  TextureFilterCaps;
    DWORD  CubeTextureFilterCaps;
    DWORD  VolumeTextureFilterCaps;
    DWORD  TextureAddressCaps;
    DWORD  VolumeTextureAddressCaps;
    DWORD  LineCaps;
    DWORD  MaxTextureWidth;
    DWORD  MaxTextureHeight;
    DWORD  MaxVolumeExtent;
    DWORD  MaxTextureRepeat;
    DWORD  MaxTextureAspectRatio;
    DWORD  MaxAnisotropy;
    float  MaxVertexW;
    float  GuardBandLeft;
    float  GuardBandTop;
    float  GuardBandRight;
    float  GuardBandBottom;
    float  ExtentsAdjust;
    DWORD  StencilCaps;
    DWORD  FVFCaps;
    DWORD  TextureOpCaps;
    DWORD  MaxTextureBlendStages;
    DWORD  MaxSimultaneousTextures;
    DWORD  VertexProcessingCaps;
    DWORD  MaxActiveLights;
    DWORD  MaxUserClipPlanes;
    DWORD  MaxVertexBlendMatrices;
    DWORD  MaxVertexBlendMatrixIndex;
    float  MaxPointSize;
    DWORD  MaxPrimitiveCount;
    DWORD  MaxVertexIndex;
    DWORD  MaxStreams;
    DWORD  MaxStreamStride;
    DWORD  VertexShaderVersion;
    DWORD  MaxVertexShaderConst;
    DWORD  PixelShaderVersion;
    float  PixelShader1xMaxValue;
    DWORD  DevCaps2;
    float  MaxNpatchTessellationLevel;
    DWORD  Reserved5;
    UINT   MasterAdapterOrdinal;
    UINT   AdapterOrdinalInGroup;
    UINT   NumberOfAdaptersInGroup;
    DWORD  DeclTypes;
    DWORD  NumSimultaneousRTs;
    DWORD  StretchRectFilterCaps;
    D3DVSHADERCAPS2_0 VS20Caps;
    D3DPSHADERCAPS2_0 PS20Caps;
    DWORD  VertexTextureFilterCaps;
    DWORD  MaxVShaderInstructionsExecuted;
    DWORD  MaxPShaderInstructionsExecuted;
    DWORD  MaxVertexShader30InstructionSlots;
    DWORD  MaxPixelShader30InstructionSlots;
} D3DCAPS9;

#define MAX_DEVICE_IDENTIFIER_STRING 512
typedef struct _D3DADAPTER_IDENTIFIER9 {
    char Driver[MAX_DEVICE_IDENTIFIER_STRING];
    char Description[MAX_DEVICE_IDENTIFIER_STRING];
    char DeviceName[32];
    LARGE_INTEGER DriverVersion;
    DWORD VendorId;
    DWORD DeviceId;
    DWORD SubSysId;
    DWORD Revision;
    GUID  DeviceIdentifier;
    DWORD WHQLLevel;
} D3DADAPTER_IDENTIFIER9;

typedef struct _D3DLOCKED_RECT {
    INT   Pitch;
    void *pBits;
} D3DLOCKED_RECT;

typedef struct _D3DBOX {
    UINT Left, Top, Right, Bottom, Front, Back;
} D3DBOX;

typedef struct _D3DLOCKED_BOX {
    INT   RowPitch;
    INT   SlicePitch;
    void *pBits;
} D3DLOCKED_BOX;

typedef struct _D3DSURFACE_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD               MultiSampleQuality;
    UINT                Width;
    UINT                Height;
} D3DSURFACE_DESC;

typedef struct _D3DVOLUME_DESC {
    D3DFORMAT       Format;
    D3DRESOURCETYPE Type;
    DWORD           Usage;
    D3DPOOL         Pool;
    UINT            Width;
    UINT            Height;
    UINT            Depth;
} D3DVOLUME_DESC;

typedef struct _D3DVERTEXELEMENT9 {
    WORD Stream;
    WORD Offset;
    BYTE Type;
    BYTE Method;
    BYTE Usage;
    BYTE UsageIndex;
} D3DVERTEXELEMENT9, *LPD3DVERTEXELEMENT9;

typedef struct _D3DVIEWPORT9 {
    DWORD X;
    DWORD Y;
    DWORD Width;
    DWORD Height;
    float MinZ;
    float MaxZ;
} D3DVIEWPORT9;

typedef struct _D3DRECT {
    LONG x1, y1, x2, y2;
} D3DRECT;

typedef struct _D3DGAMMARAMP {
    WORD red[256];
    WORD green[256];
    WORD blue[256];
} D3DGAMMARAMP;

typedef struct _D3DVERTEXBUFFER_DESC {
    D3DFORMAT       Format;
    D3DRESOURCETYPE Type;
    DWORD           Usage;
    D3DPOOL         Pool;
    UINT            Size;
    DWORD           FVF;
} D3DVERTEXBUFFER_DESC;

typedef struct _D3DINDEXBUFFER_DESC {
    D3DFORMAT       Format;
    D3DRESOURCETYPE Type;
    DWORD           Usage;
    D3DPOOL         Pool;
    UINT            Size;
} D3DINDEXBUFFER_DESC;

typedef struct _D3DDEVICE_CREATION_PARAMETERS {
    UINT       AdapterOrdinal;
    D3DDEVTYPE DeviceType;
    HWND       hFocusWindow;
    DWORD      BehaviorFlags;
} D3DDEVICE_CREATION_PARAMETERS;

// ---------------------------------------------------------------------------
// Interfaces -- plain classes bound to the null implementation
// (src/nx/nx_d3d9_null.cpp). No COM, no vtables; objects are internal
// structs and `this` is cast inside the implementation.
// ---------------------------------------------------------------------------
class IDirect3D9;
class IDirect3DDevice9;
class IDirect3DSwapChain9;
class IDirect3DSurface9;
class IDirect3DBaseTexture9;
class IDirect3DTexture9;
class IDirect3DCubeTexture9;
class IDirect3DVolumeTexture9;
class IDirect3DVertexBuffer9;
class IDirect3DIndexBuffer9;
class IDirect3DVertexDeclaration9;
class IDirect3DVertexShader9;
class IDirect3DPixelShader9;
class IDirect3DQuery9;
class IDirect3DResource9;

class IUnknown9Like {
public:
    ULONG AddRef();
    ULONG Release();
};

class IDirect3DResource9 : public IUnknown9Like {
public:
    DWORD SetPriority(DWORD priority);
    void  PreLoad();
};

class IDirect3D9 : public IUnknown9Like {
public:
    UINT    GetAdapterCount();
    HRESULT GetAdapterIdentifier(UINT adapter, DWORD flags, D3DADAPTER_IDENTIFIER9 *id);
    UINT    GetAdapterModeCount(UINT adapter, D3DFORMAT format);
    HRESULT EnumAdapterModes(UINT adapter, D3DFORMAT format, UINT mode, D3DDISPLAYMODE *displayMode);
    HRESULT GetAdapterDisplayMode(UINT adapter, D3DDISPLAYMODE *mode);
    HRESULT CheckDeviceFormat(UINT adapter, D3DDEVTYPE devType, D3DFORMAT adapterFormat,
                              DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFormat);
    HRESULT CheckDepthStencilMatch(UINT adapter, D3DDEVTYPE devType, D3DFORMAT adapterFormat,
                                   D3DFORMAT renderTargetFormat, D3DFORMAT depthStencilFormat);
    HRESULT CheckDeviceMultiSampleType(UINT adapter, D3DDEVTYPE devType, D3DFORMAT surfaceFormat,
                                       BOOL windowed, D3DMULTISAMPLE_TYPE multiSampleType, DWORD *qualityLevels);
    HMONITOR GetAdapterMonitor(UINT adapter);
    HRESULT GetDeviceCaps(UINT adapter, D3DDEVTYPE devType, D3DCAPS9 *caps);
    HRESULT CreateDevice(UINT adapter, D3DDEVTYPE devType, HWND focusWindow, DWORD behaviorFlags,
                         D3DPRESENT_PARAMETERS *pp, IDirect3DDevice9 **device);
};

class IDirect3DSwapChain9 : public IUnknown9Like {
public:
    HRESULT Present(const RECT *srcRect, const RECT *dstRect, HWND destWindowOverride,
                    const void *dirtyRegion, DWORD flags);
    HRESULT GetBackBuffer(UINT backBuffer, D3DBACKBUFFER_TYPE type, IDirect3DSurface9 **surface);
};

class IDirect3DSurface9 : public IDirect3DResource9 {
public:
    HRESULT LockRect(D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD flags);
    HRESULT UnlockRect();
    HRESULT GetDesc(D3DSURFACE_DESC *desc);
};

class IDirect3DBaseTexture9 : public IDirect3DResource9 {
public:
    DWORD GetLevelCount();
};

class IDirect3DTexture9 : public IDirect3DBaseTexture9 {
public:
    HRESULT LockRect(UINT level, D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD flags);
    HRESULT UnlockRect(UINT level);
    HRESULT GetSurfaceLevel(UINT level, IDirect3DSurface9 **surface);
    HRESULT GetLevelDesc(UINT level, D3DSURFACE_DESC *desc);
    HRESULT AddDirtyRect(const RECT *dirtyRect);
};

class IDirect3DCubeTexture9 : public IDirect3DBaseTexture9 {
public:
    HRESULT LockRect(D3DCUBEMAP_FACES face, UINT level, D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD flags);
    HRESULT UnlockRect(D3DCUBEMAP_FACES face, UINT level);
    HRESULT GetLevelDesc(UINT level, D3DSURFACE_DESC *desc);
    HRESULT GetCubeMapSurface(D3DCUBEMAP_FACES face, UINT level, IDirect3DSurface9 **surface);
};

class IDirect3DVolumeTexture9 : public IDirect3DBaseTexture9 {
public:
    HRESULT LockBox(UINT level, D3DLOCKED_BOX *lockedBox, const D3DBOX *box, DWORD flags);
    HRESULT UnlockBox(UINT level);
    HRESULT GetLevelDesc(UINT level, D3DVOLUME_DESC *desc);
};

class IDirect3DVertexBuffer9 : public IDirect3DResource9 {
public:
    HRESULT Lock(UINT offset, UINT size, void **data, DWORD flags);
    HRESULT Unlock();
    HRESULT GetDesc(D3DVERTEXBUFFER_DESC *desc);
};

class IDirect3DIndexBuffer9 : public IDirect3DResource9 {
public:
    HRESULT Lock(UINT offset, UINT size, void **data, DWORD flags);
    HRESULT Unlock();
    HRESULT GetDesc(D3DINDEXBUFFER_DESC *desc);
};

class IDirect3DVertexDeclaration9 : public IUnknown9Like {
public:
    HRESULT GetDeclaration(D3DVERTEXELEMENT9 *decl, UINT *numElements);
};

class IDirect3DVertexShader9 : public IUnknown9Like {};
class IDirect3DPixelShader9 : public IUnknown9Like {};

class IDirect3DQuery9 : public IUnknown9Like {
public:
    HRESULT Issue(DWORD issueFlags);
    HRESULT GetData(void *data, DWORD size, DWORD getDataFlags);
    DWORD   GetDataSize();
};

class IDirect3DDevice9 : public IUnknown9Like {
public:
    HRESULT TestCooperativeLevel();
    UINT    GetAvailableTextureMem();
    HRESULT GetDeviceCaps(D3DCAPS9 *caps);
    HRESULT GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *params);
    HRESULT Reset(D3DPRESENT_PARAMETERS *pp);
    HRESULT Present(const RECT *srcRect, const RECT *dstRect, HWND destWindowOverride, const void *dirtyRegion);
    HRESULT GetSwapChain(UINT swapChainIndex, IDirect3DSwapChain9 **swapChain);
    HRESULT GetBackBuffer(UINT swapChainIndex, UINT backBuffer, D3DBACKBUFFER_TYPE type, IDirect3DSurface9 **surface);
    void    SetGammaRamp(UINT swapChainIndex, DWORD flags, const D3DGAMMARAMP *ramp);

    HRESULT CreateTexture(UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format,
                          D3DPOOL pool, IDirect3DTexture9 **texture, HANDLE *sharedHandle);
    HRESULT CreateVolumeTexture(UINT width, UINT height, UINT depth, UINT levels, DWORD usage,
                                D3DFORMAT format, D3DPOOL pool, IDirect3DVolumeTexture9 **texture, HANDLE *sharedHandle);
    HRESULT CreateCubeTexture(UINT edgeLength, UINT levels, DWORD usage, D3DFORMAT format,
                              D3DPOOL pool, IDirect3DCubeTexture9 **texture, HANDLE *sharedHandle);
    HRESULT CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool,
                               IDirect3DVertexBuffer9 **vb, HANDLE *sharedHandle);
    HRESULT CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool,
                              IDirect3DIndexBuffer9 **ib, HANDLE *sharedHandle);
    HRESULT CreateRenderTarget(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE multiSample,
                               DWORD multiSampleQuality, BOOL lockable, IDirect3DSurface9 **surface, HANDLE *sharedHandle);
    HRESULT CreateDepthStencilSurface(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE multiSample,
                                      DWORD multiSampleQuality, BOOL discard, IDirect3DSurface9 **surface, HANDLE *sharedHandle);
    HRESULT CreateOffscreenPlainSurface(UINT width, UINT height, D3DFORMAT format, D3DPOOL pool,
                                        IDirect3DSurface9 **surface, HANDLE *sharedHandle);
    HRESULT CreateVertexDeclaration(const D3DVERTEXELEMENT9 *elements, IDirect3DVertexDeclaration9 **decl);
    HRESULT CreateVertexShader(const DWORD *function, IDirect3DVertexShader9 **shader);
    HRESULT CreatePixelShader(const DWORD *function, IDirect3DPixelShader9 **shader);
    HRESULT CreateQuery(D3DQUERYTYPE type, IDirect3DQuery9 **query);

    HRESULT UpdateSurface(IDirect3DSurface9 *src, const RECT *srcRect, IDirect3DSurface9 *dst, const POINT *dstPoint);
    HRESULT UpdateTexture(IDirect3DBaseTexture9 *src, IDirect3DBaseTexture9 *dst);
    HRESULT GetRenderTargetData(IDirect3DSurface9 *renderTarget, IDirect3DSurface9 *destSurface);
    HRESULT StretchRect(IDirect3DSurface9 *src, const RECT *srcRect, IDirect3DSurface9 *dst,
                        const RECT *dstRect, D3DTEXTUREFILTERTYPE filter);
    HRESULT ColorFill(IDirect3DSurface9 *surface, const RECT *rect, D3DCOLOR color);

    HRESULT SetRenderTarget(DWORD renderTargetIndex, IDirect3DSurface9 *renderTarget);
    HRESULT GetRenderTarget(DWORD renderTargetIndex, IDirect3DSurface9 **renderTarget);
    HRESULT SetDepthStencilSurface(IDirect3DSurface9 *depthStencil);
    HRESULT GetDepthStencilSurface(IDirect3DSurface9 **depthStencil);
    HRESULT BeginScene();
    HRESULT EndScene();
    HRESULT Clear(DWORD count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);
    HRESULT SetViewport(const D3DVIEWPORT9 *viewport);
    HRESULT GetViewport(D3DVIEWPORT9 *viewport);
    HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
    HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD *value);
    HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture9 *texture);
    HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
    HRESULT SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
    HRESULT SetScissorRect(const RECT *rect);
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount);
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex,
                                 UINT numVertices, UINT startIndex, UINT primCount);
    HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT primitiveCount, const void *vertexData, UINT vertexStreamStride);
    HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9 *decl);
    HRESULT SetFVF(DWORD fvf);
    HRESULT SetVertexShader(IDirect3DVertexShader9 *shader);
    HRESULT SetVertexShaderConstantF(UINT startRegister, const float *data, UINT vector4fCount);
    HRESULT SetPixelShader(IDirect3DPixelShader9 *shader);
    HRESULT SetPixelShaderConstantF(UINT startRegister, const float *data, UINT vector4fCount);
    HRESULT SetStreamSource(UINT streamNumber, IDirect3DVertexBuffer9 *vb, UINT offsetInBytes, UINT stride);
    HRESULT SetIndices(IDirect3DIndexBuffer9 *ib);
    HRESULT EvictManagedResources();
};

typedef IDirect3D9        *LPDIRECT3D9;
typedef IDirect3DDevice9  *LPDIRECT3DDEVICE9;
typedef IDirect3DSurface9 *LPDIRECT3DSURFACE9;
typedef IDirect3DTexture9 *LPDIRECT3DTEXTURE9;
typedef IDirect3DVertexBuffer9 *LPDIRECT3DVERTEXBUFFER9;
typedef IDirect3DIndexBuffer9  *LPDIRECT3DINDEXBUFFER9;

IDirect3D9 *Direct3DCreate9(UINT sdkVersion);

int D3DPERF_BeginEvent(D3DCOLOR color, const wchar_t *name);
int D3DPERF_EndEvent(void);
void D3DPERF_SetMarker(D3DCOLOR color, const wchar_t *name);

#endif // NX_COMPAT_D3D9_H
