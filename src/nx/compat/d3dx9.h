// d3dx9.h -- D3DX subset shim. Only the dev/raw-material shader-compile path
// and screenshot saving use D3DX; all return failure on Switch (fastfile
// shader loading does not go through D3DX).
#ifndef NX_COMPAT_D3DX9_H
#define NX_COMPAT_D3DX9_H

#include "d3d9.h"

#define D3DXSHADER_DEBUG            (1 << 0)
#define D3DXSHADER_SKIPVALIDATION   (1 << 1)
#define D3DXSHADER_SKIPOPTIMIZATION (1 << 2)
#define D3DXSHADER_NO_PRESHADER     (1 << 16)

class ID3DXBuffer {
public:
    void *GetBufferPointer();
    DWORD GetBufferSize();
    ULONG AddRef();
    ULONG Release();
};
typedef ID3DXBuffer *LPD3DXBUFFER;

class ID3DXConstantTable {
public:
    void *GetBufferPointer();
    DWORD GetBufferSize();
    ULONG AddRef();
    ULONG Release();
};
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;

// Shader binary reflection structures (byte offsets into the shader blob;
// exact 32-bit layout is load-bearing: r_material_load_obj.cpp walks
// ConstantInfo with a hardcoded 20-byte stride).
typedef struct _D3DXSHADER_CONSTANTTABLE {
    DWORD Size;
    DWORD Creator;
    DWORD Version;
    DWORD Constants;
    DWORD ConstantInfo;
    DWORD Flags;
    DWORD Target;
} D3DXSHADER_CONSTANTTABLE;

typedef struct _D3DXSHADER_CONSTANTINFO {
    DWORD Name;
    WORD  RegisterSet;
    WORD  RegisterIndex;
    WORD  RegisterCount;
    WORD  Reserved;
    DWORD TypeInfo;
    DWORD DefaultValue;
} D3DXSHADER_CONSTANTINFO;

typedef struct _D3DXSHADER_TYPEINFO {
    WORD  Class;
    WORD  Type;
    WORD  Rows;
    WORD  Columns;
    WORD  Elements;
    WORD  StructMembers;
    DWORD StructMemberInfo;
} D3DXSHADER_TYPEINFO;

typedef struct _D3DXSEMANTIC {
    UINT Usage;
    UINT UsageIndex;
} D3DXSEMANTIC, *LPD3DXSEMANTIC;

typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP = 0,
    D3DXIFF_JPG = 1,
    D3DXIFF_TGA = 2,
    D3DXIFF_PNG = 3,
    D3DXIFF_DDS = 4,
    D3DXIFF_PPM = 5,
    D3DXIFF_DIB = 6,
    D3DXIFF_HDR = 7,
    D3DXIFF_PFM = 8,
    D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;

typedef struct _D3DXMACRO {
    LPCSTR Name;
    LPCSTR Definition;
} D3DXMACRO;
typedef void *LPD3DXINCLUDE;

HRESULT D3DXCompileShader(LPCSTR srcData, UINT srcDataLen, const D3DXMACRO *defines,
                          LPD3DXINCLUDE include, LPCSTR functionName, LPCSTR profile,
                          DWORD flags, LPD3DXBUFFER *shader, LPD3DXBUFFER *errorMsgs,
                          LPD3DXCONSTANTTABLE *constantTable);
HRESULT D3DXCreateBuffer(DWORD numBytes, LPD3DXBUFFER *buffer);
HRESULT D3DXGetShaderConstantTable(const DWORD *function, LPD3DXCONSTANTTABLE *table);
HRESULT D3DXGetShaderInputSemantics(const DWORD *function, D3DXSEMANTIC *semantics, UINT *count);
HRESULT D3DXGetShaderOutputSemantics(const DWORD *function, D3DXSEMANTIC *semantics, UINT *count);
HRESULT D3DXSaveSurfaceToFileA(LPCSTR destFile, D3DXIMAGE_FILEFORMAT format,
                               IDirect3DSurface9 *surface, const void *palette, const RECT *srcRect);
#define D3DXSaveSurfaceToFile D3DXSaveSurfaceToFileA

#endif // NX_COMPAT_D3DX9_H
