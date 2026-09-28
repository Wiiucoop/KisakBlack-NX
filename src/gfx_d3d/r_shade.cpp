#include "r_shade.h"
#include "r_state.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "rb_shade.h"
#include "r_water.h"
#include <universal/profile.h>

bool Use_R_SetVertexShaderConstantFromCode_New = true;


void __cdecl R_SetupPassPerPrimArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if ( pass->perPrimArgCount )
        R_SetPassShaderPrimArguments(context, pass->perPrimArgCount, pass->localArgs);
}

void __cdecl R_SetPassShaderPrimArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg)
{
    while ( arg->type == 3 )
    {
        if ( Use_R_SetVertexShaderConstantFromCode_New )
            R_SetVertexShaderConstantFromCode_New(context, arg);
        else
            R_SetVertexShaderConstantFromCode_Old(context, arg);
        ++arg;
        if ( !--argCount )
            return;
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 276, 0, "unreachable") )
        __debugbreak();
}

void __cdecl R_SetVertexShaderConstantFromCode_Old(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    const char *v2; // eax
    const char *v3; // eax
    unsigned int rowCount; // [esp+30h] [ebp-28h]
    const float *v5; // [esp+34h] [ebp-24h]
    unsigned int dest; // [esp+38h] [ebp-20h]
    IDirect3DDevice9 *device; // [esp+3Ch] [ebp-1Ch]
    int hr; // [esp+44h] [ebp-14h]
    GfxCmdBufContext source; // [esp+4Ch] [ebp-Ch] BYREF
    const float *data; // [esp+54h] [ebp-4h]

    if ( !R_IsVertexShaderConstantUpToDate(context, routingData) )
    {
        source = context;
        if ( routingData->u.codeConst.index < 0xC5u )
            data = (const float *)R_GetCodeConstant(source, routingData->u.codeConst.index);
        else
            data = (const float *)R_GetCodeMatrix(source.source, routingData->u.codeConst.index, routingData->u.codeConst.firstRow);
        rowCount = routingData->u.codeConst.rowCount;
        v5 = data;
        dest = routingData->dest;
        device = context.state->prim.device;
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetVertexShaderConstantF( dest, data, rowCount )\n");
        hr = device->SetVertexShaderConstantF(dest, v5, rowCount);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v2 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShaderConstantF( dest, data, "
                "rowCount ) failed: %s\n",
                123,
                v2);
        }
        if ( r_logFile->current.integer )
        {
            while ( rowCount )
            {
                v3 = va("vertex const %i: %g %g %g %g\n", dest, *v5, v5[1], v5[2], v5[3]);
                RB_LogPrint(v3);
                ++dest;
                v5 += 4;
                --rowCount;
            }
        }
    }
}

float *__cdecl R_GetCodeConstant(const GfxCmdBufContext context, unsigned int constant)
{
    iassert(context.state);
    iassert(context.source);
    bcassert(constant, CONST_SRC_CODE_COUNT_FLOAT4);

    return context.source->input.consts[constant];
}

char __cdecl R_IsVertexShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    if ( routingData->dest >= 0x100u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                    98,
                    0,
                    "routingData->dest doesn't index ARRAY_COUNT( context.state->vertexShaderConstState )\n\t%i not in [0, %i)",
                    routingData->dest,
                    256) )
    {
        __debugbreak();
    }
    if ( routingData->u.codeConst.index < 0xC5u )
        return R_IsShaderConstantUpToDate(
                         context.source,
                         (GfxShaderConstantState *)&context.state->vertexShaderConstState[routingData->dest],
                         routingData);
    else
        return R_IsShaderMatrixUpToDate(
                         context.source,
                         (GfxShaderConstantState *)&context.state->vertexShaderConstState[routingData->dest],
                         routingData);
}

bool __cdecl R_IsShaderMatrixUpToDate(
                const GfxCmdBufSourceState *source,
                GfxShaderConstantState *constant,
                const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]
    unsigned int rowCount; // [esp+Ch] [ebp-8h]

    newState.fields.codeConst = routingData->u.codeConst;
    newState.fields.version = source->matrixVersions[MATRIX_VERSIONS_INDEX(routingData->u.codeConst.index)];

    if (constant->packed == newState.packed)
        return true;

    *constant = newState;

    rowCount = routingData->u.codeConst.rowCount;
    iassert(rowCount);

    for (rowCount = rowCount - 1; rowCount; --rowCount)
    {
        ++constant;
        constant->packed = (unsigned __int64)-1;
    }

    return false;
}

bool __cdecl R_IsShaderConstantUpToDate(
                const GfxCmdBufSourceState *source,
                GfxShaderConstantState *constant,
                const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]

    iassert(source);

    newState.fields.codeConst = routingData->u.codeConst;
    newState.fields.version = source->constVersions[routingData->u.codeConst.index];

    if (constant->packed == newState.packed)
        return true;

    *constant = newState;

    iassert(routingData->u.codeConst.rowCount == 1);
    return false;
}

void __cdecl R_SetVertexShaderConstantFromCode_New(
    const GfxCmdBufContext context,
    const MaterialShaderArgument *routingData)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    //$26AC422158757CD6FC73CEC8E4188A45 *CodeConstant; // [esp+40h] [ebp-6Ch]
    unsigned int dest; // [esp+44h] [ebp-68h]
    IDirect3DDevice9 *device; // [esp+48h] [ebp-64h]
    int v11; // [esp+50h] [ebp-5Ch]
    int hr; // [esp+68h] [ebp-44h]
    unsigned int rowLoop; // [esp+70h] [ebp-3Ch]
    unsigned __int64 packed; // [esp+74h] [ebp-38h]
    unsigned __int64 *constant; // [esp+80h] [ebp-2Ch]
    unsigned __int64 version; // [esp+84h] [ebp-28h]
    unsigned int rowCount; // [esp+90h] [ebp-1Ch]
    unsigned __int64 codeConstBits; // [esp+94h] [ebp-18h]
    unsigned __int16 index; // [esp+A0h] [ebp-Ch]
    const float *data;

    bcassert(routingData->dest, ARRAY_COUNT(context.state->vertexShaderConstState));

    constant = &context.state->vertexShaderConstState[routingData->dest];
    index = routingData->u.codeConst.index;
    codeConstBits = (unsigned __int64)routingData->u.codeSampler << 32;
    
    if (index < CONST_SRC_FIRST_CODE_MATRIX)
    {
        iassert(routingData->u.codeConst.rowCount == 1);

        //packed = context.source->constVersions[index + 24] | codeConstBits;
        packed = context.source->constVersions[index] | codeConstBits;
        if (*constant != packed)
        {
            *constant = packed;
            rowCount = 1;
            data = R_GetCodeConstant(context, index);
            dest = routingData->dest;
            device = context.state->prim.device;


            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetVertexShaderConstantF( dest, data, rowCount )\n");

            v11 = device->SetVertexShaderConstantF(dest, data, rowCount);
            if (v11 < 0)
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(v11);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShaderConstantF( dest, data"
                    ", rowCount ) failed: %s\n",
                    123,
                    v4);
            }

            if (r_logFile->current.integer)
            {
                while (rowCount)
                {
                    RB_LogPrint(va("vertex const %i: %g %g %g %g\n", dest, data[0], data[1], data[2], data[3]));
                    dest++;
                    data += 4;
                    rowCount--;
                }
            }
        }
    }
    else
    {
        //version = *((unsigned __int16 *)&context.source->viewParms3D + ((index - 197) >> 2) + 1);
        version = context.source->matrixVersions[MATRIX_VERSIONS_INDEX(index)];
        packed = version | codeConstBits;
        if (*constant != packed)
        {
            *constant = packed;
            rowCount = routingData->u.codeConst.rowCount;
            iassert(rowCount);

            //for (rowLoop = rowCount - 1; rowLoop; --rowLoop)
            //{
            //    *(_DWORD *)++constant = -1;
            //    *((_DWORD *)constant + 1) = -1;
            //}
            for (rowLoop = rowCount - 1; rowLoop; --rowLoop)
            {
                ++constant;
                *constant = (unsigned __int64)-1;
            }

            data = R_GetCodeMatrix(context.source, index, routingData->u.codeConst.firstRow);
            dest = routingData->dest;

            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("device->SetVertexShaderConstantF( dest, data, rowCount )\n");
            hr = context.state->prim.device->SetVertexShaderConstantF(dest, data, rowCount);
            if (hr < 0)
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShaderConstantF( dest, data"
                    ", rowCount ) failed: %s\n",
                    123,
                    v2);
            }

            if (r_logFile->current.integer)
            {
                while (rowCount)
                {
                    RB_LogPrint(va("vertex const %i: %g %g %g %g\n", dest, data[0], data[1], data[2], data[3]));
                    ++dest;
                    data += 4;
                    --rowCount;
                }
            }
        }
    }
}

void __cdecl R_SetupPassPerObjectArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if ( pass->perObjArgCount )
        R_SetPassShaderObjectArguments(context, pass->perObjArgCount, &pass->localArgs[pass->perPrimArgCount]);
}

void __cdecl R_SetPassShaderObjectArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg)
{
    const GfxImage *image; // [esp+94h] [ebp-8h]
    unsigned __int8 samplerState; // [esp+9Bh] [ebp-1h] BYREF

    while ( arg->type == 3 )
    {
        if ( Use_R_SetVertexShaderConstantFromCode_New )
            R_SetVertexShaderConstantFromCode_New(context, arg);
        else
            R_SetVertexShaderConstantFromCode_Old(context, arg);
        ++arg;
        if ( !--argCount )
            return;
    }

    while ( arg->type == 4 )
    {
        if (context.source->input.codeImageRenderTargetControl[arg->u.codeSampler].fields.enable)
        {
            R_SetTextureSamplerCodeImageRenderTarget(
                context,
                arg->dest,
                context.source->input.codeImageRenderTargetControl[arg->u.codeSampler]);
            ++arg;
            if ( !--argCount )
                return;
        }
        else
        {
            image = R_GetTextureFromCode(context.source, arg->u.codeSampler, &samplerState);
            if ( !image )
                R_TextureFromCodeError(context, arg->u.codeSampler);
            R_SetSampler(context, arg->dest, samplerState, image);
            ++arg;
            if ( !--argCount )
                return;
        }
    }

    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 333, 0, "unreachable") )
        __debugbreak();
}

void __cdecl R_SetPassPixelShaderStableArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg)
{
    const char *v3; // eax
    const Material *material; // [esp+14h] [ebp-8h]
    const MaterialConstantDef *constDef; // [esp+18h] [ebp-4h]

    material = context.state->material;
    while ( arg->type < 5u )
    {
        ++arg;
        if ( !--argCount )
            return;
    }
    while ( arg->type == 5 )
    {
        if ( !R_IsPixelShaderConstantUpToDate(context, arg) )
            R_SetPixelShaderConstantFromCode(context, arg);
        ++arg;
        if ( !--argCount )
            return;
    }
    constDef = material->localConstantTable;
    while ( arg->type == 6 )
    {
        while ( constDef->nameHash != arg->u.codeSampler )
        {
            if ( ++constDef == &material->localConstantTable[material->constantCount] )
            {
                v3 = va("material '%s' is missing a required named constant", material->info.name);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                                415,
                                0,
                                "%s\n\t%s",
                                "constDef != &material->constantTable[material->constantCount]",
                                v3) )
                    __debugbreak();
            }
        }
        R_SetPixelShaderConstantFromLiteral(context.state, arg->dest, constDef->literal);
        ++arg;
        if ( !--argCount )
            return;
    }
    while ( arg->type == 7 )
    {
        R_SetPixelShaderConstantFromLiteral(context.state, arg->dest, arg->u.literalConst);
        ++arg;
        if ( !--argCount )
            return;
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 428, 0, "unreachable") )
        __debugbreak();
}

void __cdecl R_SetPixelShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal)
{
    if ( dest >= 0x100
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                    1888,
                    0,
                    "dest doesn't index ARRAY_COUNT( state->pixelShaderConstState )\n\t%i not in [0, %i)",
                    dest,
                    256) )
    {
        __debugbreak();
    }
    LODWORD(state->pixelShaderConstState[dest]) = -1;
    HIDWORD(state->pixelShaderConstState[dest]) = -1;
    R_HW_SetPixelShaderConstant(state->prim.device, dest, literal, 1u);
}

void __cdecl R_HW_SetPixelShaderConstant(
                IDirect3DDevice9 *device,
                unsigned int dest,
                const float *data,
                unsigned int rowCount)
{
    const char *v4; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetPixelShaderConstantF( dest, data, rowCount )\n");

    hr = device->SetPixelShaderConstantF(dest, data, rowCount);
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, const float *, unsigned int))device->SetPixelShaderConstantF)(
    //             device,
    //             device,
    //             dest,
    //             data,
    //             rowCount);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetPixelShaderConstantF( dest, data, row"
            "Count ) failed: %s\n",
            158,
            v4);
    }
}

int __cdecl R_IsPixelShaderConstantUpToDate(const GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    GfxShaderConstantState newState; // [esp+4h] [ebp-10h]

    iassert(routingData->u.codeConst.rowCount == 1);
    iassert(context.source);

    newState.fields.codeConst = routingData->u.codeConst;
    newState.fields.version = context.source->constVersions[routingData->u.codeConst.index];

    iassert(newState.fields.version);

    if (context.state->pixelShaderConstState[routingData->dest] == newState.packed)
        return 1;

    context.state->pixelShaderConstState[routingData->dest] = newState.packed;

    return 0;
}

void __cdecl R_SetPixelShaderConstantFromCode(const GfxCmdBufContext context, const MaterialShaderArgument *routingData)
{
    iassert(context.source->constVersions[routingData->u.codeConst.index]);
    //bcassert(routingData->u.codeConst.index, CONST_SRC_FIRST_CODE_MATRIX);

    R_HW_SetPixelShaderConstant(
        context.state->prim.device,
        routingData->dest,
        R_GetCodeConstant(context, routingData->u.codeConst.index),
        routingData->u.codeConst.rowCount);
}

void __cdecl R_SetupPassCriticalPixelShaderArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if ( pass->stableArgCount )
        R_SetPassPixelShaderStableArguments(
            context,
            pass->stableArgCount,
            &pass->localArgs[pass->perPrimArgCount + pass->perObjArgCount]);
}

void __cdecl R_SetupPassVertexShaderArgs(GfxCmdBufContext context)
{
    const MaterialPass *pass; // [esp+0h] [ebp-4h]

    pass = context.state->pass;
    if ( pass->stableArgCount )
        R_SetPassVertexShaderStableArguments(
            context,
            pass->stableArgCount,
            &pass->localArgs[pass->perPrimArgCount + pass->perObjArgCount]);
}

void __cdecl R_SetPassVertexShaderStableArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg)
{
    while ( arg->type < 3u )
    {
        ++arg;
        if ( !--argCount )
            return;
    }
    do
    {
        if ( arg->type != 3 )
            break;
        if ( Use_R_SetVertexShaderConstantFromCode_New )
            R_SetVertexShaderConstantFromCode_New(context, arg);
        else
            R_SetVertexShaderConstantFromCode_Old(context, arg);
        ++arg;
        --argCount;
    }
    while ( argCount );
}

const MaterialTextureDef *__cdecl R_SetPixelSamplerFromMaterial(
                const GfxCmdBufContext context,
                const MaterialShaderArgument *arg,
                const MaterialTextureDef *texDef)
{
    const char *v3; // eax
    const GfxImage *image; // [esp+8h] [ebp-Ch] BYREF
    const Material *material; // [esp+Ch] [ebp-8h]

    material = context.state->material;

    while ( texDef->nameHash != arg->u.codeSampler )
    {
        if ( ++texDef == &material->textureTable[material->textureCount] )
        {
            v3 = va("material '%s' is missing a required named texture", material->info.name);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                            749,
                            1,
                            "%s\n\t%s",
                            "texDef != &material->textureTable[material->textureCount]",
                            v3) )
                __debugbreak();
        }
    }
    if ( texDef->semantic == 11 )
    {
        //image = (const GfxImage *)texDef->u.image[1].cardMemory.platform[0];
        image = texDef->u.water->image;
        R_UploadWaterTexture(texDef->u.water, context.source->sceneDef.floatTime);
    }
    else
    {
        image = texDef->u.image;
    }

    if ( rg.hasAnyImageOverrides )
        R_OverrideImage((GfxImage **)&image, texDef);

    /// lwss: removed mature content hack
    //if ( texDef->isMatureContent && context.source->input.data->hideMatureContent && (unsigned __int8)((unsigned __int8)HIBYTE(material->info.layeredSurfaceTypes) >> 5) > 1u )
    //{
    //    image = rgp.blankImage;
    //}

    R_SetSampler(context, arg->dest, texDef->samplerState, image);
    return texDef;
}

void __cdecl R_OverrideImage(GfxImage **image, const MaterialTextureDef *texdef)
{
    const char *v2; // eax
    unsigned __int8 semantic; // [esp+7h] [ebp-1h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 672, 0, "%s", "image") )
        __debugbreak();
    if ( !texdef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 673, 0, "%s", "texdef") )
        __debugbreak();
    if ( (*image)->mapType == 3 )
    {
        semantic = texdef->semantic;
        switch ( semantic )
        {
            case 0u:
            case 1u:
            case 0xBu:
            case 0x1Au:
            case 0x1Bu:
                return;
            case 2u:
            case 0xCu:
            case 0xDu:
            case 0xEu:
            case 0xFu:
            case 0x10u:
            case 0x11u:
            case 0x12u:
            case 0x13u:
            case 0x14u:
            case 0x15u:
            case 0x16u:
            case 0x17u:
            case 0x18u:
            case 0x19u:
            case 0x1Cu:
                if ( r_colorMap->current.integer != 1 )
                    *image = (GfxImage *)R_OverrideGrayscaleImage(r_colorMap);
                break;
            case 5u:
                if ( r_normalMap->current.integer )
                {
                    if ( r_normalMap->current.integer != 1
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                                    722,
                                    0,
                                    "%s\n\t(r_normalMap->current.integer) = %i",
                                    "(r_normalMap->current.integer == R_NORMAL_OVERRIDE_NONE)",
                                    r_normalMap->current.integer) )
                    {
                        __debugbreak();
                    }
                }
                else
                {
                    *image = rgp.identityNormalMapImage;
                }
                break;
            case 8u:
                if ( r_specularRoughnessMap->current.integer )
                {
                    switch ( r_specularRoughnessMap->current.integer )
                    {
                        case 1:
                            *image = rgp.blackTransparentImage;
                            break;
                        case 2:
                            *image = rgp.blackImage;
                            break;
                        case 3:
                            *image = rgp.whiteTransparentImage;
                            break;
                        case 4:
                            *image = rgp.whiteImage;
                            break;
                    }
                }
                break;
            default:
                v2 = va("unhandled case %i", semantic);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 733, 1, v2) )
                    __debugbreak();
                break;
        }
    }
}

void __cdecl R_SetPassShaderStableArguments(
                const GfxCmdBufContext context,
                unsigned int argCount,
                const MaterialShaderArgument *arg)
{
    const char *v3; // eax
    const GfxImage *image; // [esp+C4h] [ebp-14h]
    const Material *material; // [esp+C8h] [ebp-10h]
    const MaterialTextureDef *texDef; // [esp+CCh] [ebp-Ch]
    unsigned __int8 samplerState; // [esp+D3h] [ebp-5h] BYREF
    const MaterialConstantDef *constDef; // [esp+D4h] [ebp-4h]

    material = context.state->material;
    constDef = material->localConstantTable;
    while ( !arg->type )
    {
        while ( constDef->nameHash != arg->u.codeSampler )
        {
            if ( ++constDef == &material->localConstantTable[material->constantCount] )
            {
                v3 = va("material '%s' is missing a required named constant", material->info.name);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                                826,
                                0,
                                "%s\n\t%s",
                                "constDef != &material->constantTable[material->constantCount]",
                                v3) )
                    __debugbreak();
            }
        }
        R_SetVertexShaderConstantFromLiteral(context.state, arg->dest, constDef->literal);
        ++arg;
        if ( !--argCount )
            return;
    }
    while ( arg->type == 1 )
    {
        R_SetVertexShaderConstantFromLiteral(context.state, arg->dest, arg->u.literalConst);
        ++arg;
        if ( !--argCount )
            return;
    }
    texDef = material->textureTable;

    while ( arg->type == 2 )
    {
        texDef = R_SetPixelSamplerFromMaterial(context, arg++, texDef);
        if ( !--argCount )
            return;
    }

    while ( arg->type == 3 )
    {
        if ( Use_R_SetVertexShaderConstantFromCode_New )
            R_SetVertexShaderConstantFromCode_New(context, arg);
        else
            R_SetVertexShaderConstantFromCode_Old(context, arg);
        ++arg;
        if ( !--argCount )
            return;
    }

    while ( arg->type == 4 )
    {
        if ( ((*(_QWORD *)&context.source->input.codeImageRenderTargetControl[arg->u.codeSampler].fields >> 8) & 1) != 0 )
        {
            R_SetTextureSamplerCodeImageRenderTarget(
                context,
                arg->dest,
                context.source->input.codeImageRenderTargetControl[arg->u.codeSampler]);
            ++arg;
            if ( !--argCount )
                return;
        }
        else
        {
            image = R_GetTextureFromCode(context.source, arg->u.codeSampler, &samplerState);
            if ( !image )
                R_TextureFromCodeError(context, arg->u.codeSampler);
            R_SetSampler(context, arg->dest, samplerState, image);
            ++arg;
            if ( !--argCount )
                return;
        }
    }
}

void __cdecl R_SetVertexShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal)
{
    const char *v3; // eax
    const char *v4; // eax
    int v5; // [esp+30h] [ebp-18h]
    const float *v6; // [esp+34h] [ebp-14h]
    unsigned int v7; // [esp+38h] [ebp-10h]
    IDirect3DDevice9 *device; // [esp+3Ch] [ebp-Ch]
    int hr; // [esp+44h] [ebp-4h]

    if ( dest >= 0x100
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                    1878,
                    0,
                    "dest doesn't index ARRAY_COUNT( state->vertexShaderConstState )\n\t%i not in [0, %i)",
                    dest,
                    256) )
    {
        __debugbreak();
    }
    LODWORD(state->vertexShaderConstState[dest]) = -1;
    HIDWORD(state->vertexShaderConstState[dest]) = -1;
    v5 = 1;
    v6 = literal;
    v7 = dest;
    device = state->prim.device;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetVertexShaderConstantF( dest, data, rowCount )\n");
    hr = device->SetVertexShaderConstantF(dest, literal, 1u);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShaderConstantF( dest, data, ro"
            "wCount ) failed: %s\n",
            123,
            v3);
    }
    if ( r_logFile->current.integer )
    {
        while ( v5 )
        {
            v4 = va("vertex const %i: %g %g %g %g\n", v7, *v6, v6[1], v6[2], v6[3]);
            RB_LogPrint(v4);
            ++v7;
            v6 += 4;
            --v5;
        }
    }
}

int __cdecl R_ReserveIndexData(GfxCmdBufPrimState *state, int triCount)
{
    unsigned int v2; // ecx
    int indexCount; // [esp+8h] [ebp-4h]

    indexCount = 3 * triCount;
    if ( 3 * triCount > gfxBuf.dynamicIndexBuffer->total
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                    937,
                    0,
                    "%s\n\t(indexCount) = %i",
                    "(indexCount <= gfxBuf.dynamicIndexBuffer->total)",
                    indexCount) )
    {
        __debugbreak();
    }
    if ( indexCount + gfxBuf.dynamicIndexBuffer->used > gfxBuf.dynamicIndexBuffer->total )
        gfxBuf.dynamicIndexBuffer->used = 0;
    if ( !gfxBuf.dynamicIndexBuffer->used )
    {
        v2 = (gfxBuf.dynamicIndexBuffer - gfxBuf.dynamicIndexBufferPool + 1) & 0x80000000;
        if ( gfxBuf.dynamicIndexBuffer - gfxBuf.dynamicIndexBufferPool + 1 < 0 )
            v2 = 0;
        gfxBuf.dynamicIndexBuffer = &gfxBuf.dynamicIndexBufferPool[v2];
    }
    return gfxBuf.dynamicIndexBuffer->used;
}

int __cdecl R_SetIndexData(GfxCmdBufPrimState *state, unsigned __int8 *indices, int triCount)
{
    unsigned __int8 *v3; // eax
    int baseIndex; // [esp+0h] [ebp-18h]
    unsigned int indexDataSize; // [esp+4h] [ebp-14h]
    IDirect3DIndexBuffer9 *ib; // [esp+10h] [ebp-8h]
    unsigned __int16 *bufferData; // [esp+14h] [ebp-4h]

    baseIndex = R_ReserveIndexData(state, triCount);
    indexDataSize = 6 * triCount;
    ib = gfxBuf.dynamicIndexBuffer->buffer;
    if ( !ib && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 981, 0, "%s", "ib") )
        __debugbreak();
    if ( gfxBuf.dynamicIndexBuffer->used )
        v3 = (unsigned __int8 *)R_LockIndexBuffer(ib, 2 * gfxBuf.dynamicIndexBuffer->used, indexDataSize, 0x1000u);
    else
        v3 = (unsigned __int8 *)R_LockIndexBuffer(ib, 2 * gfxBuf.dynamicIndexBuffer->used, indexDataSize, 0x2000u);
    bufferData = (unsigned __int16 *)v3;
    if ( v3 )
        memcpy(v3, indices, indexDataSize);
    R_UnlockIndexBuffer(ib);
    if ( state->indexBuffer != ib )
        R_ChangeIndices(state, ib);
    if ( bufferData )
        gfxBuf.dynamicIndexBuffer->used += 3 * triCount;
    return baseIndex;
}

#ifdef KISAK_NX
// nx-port: shaders from a KBZ zone are created by nx_kbz.cpp's build step, one
// technique set at a time. A pass it never reached (the first map load hit
// one: pimp_shader_vertcolorsimple, drawing a 2D pic) would assert below and
// trap, so a shader that still holds its load def is created at first use
// instead, and named so the gap can be found.
static void R_NxCreateShaderLate(const GfxCmdBufState *state, const char *kind, const char *name)
{
    static unsigned s_count;
    if (++s_count <= 32)
        Com_PrintWarning(8, "R_NxCreateShaderLate: %s shader '%s' created at first use (material '%s')\n",
                         kind, name ? name : "?",
                         state->material && state->material->info.name ? state->material->info.name : "?");
}
#endif

void __cdecl R_SetPixelShader(GfxCmdBufState *state, const MaterialPixelShader *pixelShader)
{
    if ( !pixelShader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 1039, 0, "%s", "pixelShader") )
    {
        __debugbreak();
    }
#ifdef KISAK_NX
    if ( !pixelShader->prog.ps && pixelShader->prog.loadDef.program )
    {
        MaterialPixelShader *mutableShader = (MaterialPixelShader *)pixelShader;
        Load_CreateMaterialPixelShader(&mutableShader->prog.loadDef, mutableShader);
        R_NxCreateShaderLate(state, "pixel", pixelShader->name);
    }
#endif
    if ( !pixelShader->prog.ps
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                    1041,
                    0,
                    "%s\n\t(pixelShader->name) = %s",
                    "(pixelShader->prog.ps)",
                    pixelShader->name) )
    {
        __debugbreak();
    }
    if ( state->pixelShader != pixelShader )
    {
        R_HW_SetPixelShader(state->prim.device, pixelShader);
        state->pixelShader = pixelShader;
    }
}

void __cdecl R_UpdateVertexDecl(GfxCmdBufState *state)
{
    const MaterialPass *pass; // [esp+14h] [ebp-8h]
    const MaterialVertexShader *vertexShader; // [esp+18h] [ebp-4h]

    pass = state->pass;
    if ( !pass->vertexDecl
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 1299, 0, "%s", "pass->vertexDecl") )
    {
        __debugbreak();
    }
    vertexShader = pass->vertexShader;
    if ( !vertexShader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 1331, 0, "%s", "vertexShader") )
    {
        __debugbreak();
    }
#ifdef KISAK_NX
    if ( pass->vertexDecl && !pass->vertexDecl->isLoaded )   // see R_NxCreateShaderLate
    {
        MaterialPass *mutablePass = (MaterialPass *)pass;
        Load_BuildVertexDecl(&mutablePass->vertexDecl);
        R_NxCreateShaderLate(state, "vertex decl for", vertexShader ? vertexShader->name : 0);
    }
#endif
    R_SetVertexDecl(&state->prim, pass->vertexDecl);
    if ( !pass->pixelShader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 1337, 0, "%s", "pass->pixelShader") )
    {
        __debugbreak();
    }
    if ( pass->vertexDecl == (MaterialVertexDeclaration *)-36
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                    1340,
                    0,
                    "%s",
                    "pass->vertexDecl->routing.decl") )
    {
        __debugbreak();
    }
    if ( !pass->vertexDecl->routing.decl[state->prim.vertDeclType] )
        Com_Error(
            ERR_FATAL,
            "Vertex type %i doesn't have the information used by shader %s in material %s\n",
            state->prim.vertDeclType,
            pass->vertexShader->name,
            state->material->info.name);
    R_SetVertexShader(state, vertexShader);
}

void __cdecl R_SetVertexShader(GfxCmdBufState *state, const MaterialVertexShader *vertexShader)
{
    if ( state->vertexShader != vertexShader )
    {
        if ( !vertexShader
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp", 1178, 0, "%s", "vertexShader") )
        {
            __debugbreak();
        }
#ifdef KISAK_NX
        if ( !vertexShader->prog.vs && vertexShader->prog.loadDef.program )
        {
            MaterialVertexShader *mutableShader = (MaterialVertexShader *)vertexShader;
            Load_CreateMaterialVertexShader(&mutableShader->prog.loadDef, mutableShader);
            R_NxCreateShaderLate(state, "vertex", vertexShader->name);
        }
#endif
        if ( !vertexShader->prog.vs
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                        1180,
                        0,
                        "%s\n\t(vertexShader->name) = %s",
                        "(vertexShader->prog.vs)",
                        vertexShader->name) )
        {
            __debugbreak();
        }
        R_HW_SetVertexShader(state->prim.device, vertexShader);
        state->vertexShader = vertexShader;
    }
}

void __cdecl R_SetupPass(GfxCmdBufContext context, unsigned int passIndex)
{
    PROF_SCOPED("R_SetupPass");

    const MaterialPass *pass; // [esp+0h] [ebp-14h]
    const Material *material; // [esp+4h] [ebp-10h]
    const GfxStateBits *refStateBits; // [esp+8h] [ebp-Ch]
    unsigned int stateBits[2]; // [esp+Ch] [ebp-8h] BYREF

    pass = &context.state->technique->passArray[passIndex];
    context.state->pass = pass;
    context.state->passIndex = passIndex;
    material = context.state->material;
    if ( material->stateBitsEntry[context.state->techType] >= (unsigned int)material->stateBitsCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.cpp",
                    1376,
                    0,
                    "material->stateBitsEntry[context.local.state->techType] doesn't index material->stateBitsCount\n"
                    "\t%i not in [0, %i)",
                    material->stateBitsEntry[context.state->techType],
                    material->stateBitsCount) )
    {
        __debugbreak();
    }
    refStateBits = &material->stateBitsTable[passIndex + material->stateBitsEntry[context.state->techType]];
    stateBits[0] = refStateBits->loadBits[0];
    stateBits[1] = refStateBits->loadBits[1];
    R_SetState(context.state, stateBits);
    R_SetPixelShader(context.state, pass->pixelShader);
    if ( pass->stableArgCount )
        R_SetPassShaderStableArguments(context, pass->stableArgCount, &pass->localArgs[pass->perPrimArgCount + pass->perObjArgCount]);
}

void __cdecl R_SetState(GfxCmdBufState *state, unsigned int *stateBits)
{
    if ( *stateBits != state->refStateBits[0] )
    {
        R_ChangeState_0(state, *stateBits);
        state->refStateBits[0] = *stateBits;
    }
    if ( stateBits[1] != state->refStateBits[1] )
    {
        R_ChangeState_1(state, stateBits[1]);
        state->refStateBits[1] = stateBits[1];
    }
}

int __cdecl R_SetVertexData(GfxCmdBufState *state, const void *data, int vertexCount, int stride)
{
    IDirect3DVertexBuffer9 *vb; // [esp+0h] [ebp-14h]
    volatile int vertexOffset; // [esp+4h] [ebp-10h]
    void *bufferData; // [esp+Ch] [ebp-8h]
    int totalSize; // [esp+10h] [ebp-4h]

    iassert(vertexCount > 0);
    totalSize = stride * vertexCount;

    iassert(totalSize <= gfxBuf.dynamicVertexBuffer->total);

    iassert(gfxBuf.dynamicVertexBuffer->used + totalSize <= gfxBuf.dynamicVertexBuffer->total);
    
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    iassert(vb);

    bufferData = R_LockVertexBuffer(
                                 vb,
                                 gfxBuf.dynamicVertexBuffer->used,
                                 totalSize,
                                 gfxBuf.dynamicVertexBuffer->used != 0 ? 4096 : 0x2000);

    iassert(bufferData);
    
    if ( bufferData )
        Com_Memcpy(bufferData, data, totalSize);
    R_UnlockVertexBuffer(vb);
    vertexOffset = gfxBuf.dynamicVertexBuffer->used;
    gfxBuf.dynamicVertexBuffer->used += totalSize;
    return vertexOffset;
}

