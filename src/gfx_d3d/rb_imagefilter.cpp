#include "rb_imagefilter.h"
#include "rb_backend.h"
#include "rb_shade.h"
#include "r_state_utils.h"
#include "r_foliage.h"
#include "rb_draw3d.h"
#include "r_state.h"

void __cdecl RB_GaussianFilterImage(float radius, GfxRenderTargetId srcRenderTargetId, GfxRenderTargetId dstRenderTargetId)
{
    float radiusY; // [esp+30h] [ebp-13ACh] BYREF
    int srcWidth; // [esp+34h] [ebp-13A8h]
    float radiusX; // [esp+38h] [ebp-13A4h] BYREF
    int dstWidth; // [esp+3Ch] [ebp-13A0h]
    int srcHeight; // [esp+44h] [ebp-1398h]
    int dstHeight; // [esp+48h] [ebp-1394h]
    GfxImageFilter filter; // [esp+4Ch] [ebp-1390h] BYREF

    PROF_SCOPED("RB_GaussianFilterImage");

    RB_VirtualToSceneRadius(radius, &radiusX, &radiusY);
    srcWidth = gfxRenderTargets[srcRenderTargetId].width;
    srcHeight = gfxRenderTargets[srcRenderTargetId].height;
    dstWidth = gfxRenderTargets[dstRenderTargetId].width;
    dstHeight = gfxRenderTargets[dstRenderTargetId].height;
    filter.sourceImage = gfxRenderTargets[srcRenderTargetId].image;
    filter.finalTarget = dstRenderTargetId;
    filter.passCount = RB_GenerateGaussianFilterChain(
        radiusX,
        radiusY,
        srcWidth,
        srcHeight,
        dstWidth,
        dstHeight,
        32,
        filter.passes);
    if (!filter.passCount)
    {
        return;
    }
    RB_FilterImage(&filter);
}

void __cdecl RB_VirtualToSceneRadius(float radius, float *radiusX, float *radiusY)
{
    if ( !radiusX
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp", 64, 0, "%s", "radiusX") )
    {
        __debugbreak();
    }
    if ( !radiusY
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp", 65, 0, "%s", "radiusY") )
    {
        __debugbreak();
    }
    *radiusY = (double)vidConfig.sceneHeight * radius / 480.0;
    *radiusX = *radiusY * vidConfig.aspectRatioScenePixel;
}

int __cdecl RB_GenerateGaussianFilterChain(
                float radiusX,
                float radiusY,
                int srcWidth,
                int srcHeight,
                int dstWidth,
                int dstHeight,
                int passLimit,
                GfxImageFilterPass *filterPass)
{
    float v9; // [esp+18h] [ebp-24h]
    int dstRes[2]; // [esp+24h] [ebp-18h] BYREF
    float passRadius; // [esp+2Ch] [ebp-10h]
    int passRes; // [esp+30h] [ebp-Ch]
    int passAxis; // [esp+34h] [ebp-8h]
    int passCount; // [esp+38h] [ebp-4h]

    dstRes[0] = dstWidth;
    dstRes[1] = dstHeight;
    passCount = 0;
    if ( srcWidth != dstWidth || srcHeight != dstHeight )
    {
        if ( (float)(radiusY - radiusX) < 0.0 )
            v9 = radiusY;
        else
            v9 = radiusX;
        passRadius = v9;
        if ( v9 > 1.3895605 )
            passRadius = 1.3895605f;
        radiusX = (float)(sqrtf(
                                                fabs((float)(radiusX * radiusX) - (float)(passRadius * passRadius)))
                                        * (float)dstWidth)
                        / (float)srcWidth;
        radiusY = (float)(sqrtf(
                                                fabs((float)(radiusY * radiusY) - (float)(passRadius * passRadius)))
                                        * (float)dstHeight)
                        / (float)srcHeight;
        if ( passCount >= passLimit
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
                        241,
                        0,
                        "%s",
                        "passCount < passLimit") )
        {
            __debugbreak();
        }
        RB_GenerateGaussianFilter2D(passRadius, srcWidth, srcHeight, dstWidth, dstHeight, &filterPass[passCount++]);
    }
    while ( passCount < 32 && (radiusX >= 0.32950512 || radiusY >= 0.32950512) )
    {
        if ( fabs(radiusX - radiusY) < 0.32950512 )
        {
            passRadius = (float)(radiusX + radiusY) * 0.5;
            if ( passRadius <= 1.3895605 )
            {
                if ( passCount >= passLimit
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
                                253,
                                0,
                                "%s",
                                "passCount < passLimit") )
                {
                    __debugbreak();
                }
                RB_GenerateGaussianFilter2D(passRadius, dstWidth, dstHeight, dstWidth, dstHeight, &filterPass[passCount++]);
                break;
            }
        }
        if ( radiusX <= radiusY )
        {
            if ( radiusY > 6.4977503 )
            {
                passRadius = 6.4977503f;
                radiusY = sqrtf((float)(radiusY * radiusY) - 42.22076);
            }
            else
            {
                passRadius = radiusY;
                radiusY = 0.0f;
            }
            passAxis = 1;
            passRes = dstHeight;
        }
        else
        {
            if ( radiusX >= 6.4977503 )
            {
                passRadius = 6.4977503f;
                radiusX = sqrtf((float)(radiusX * radiusX) - 42.22076);
            }
            else
            {
                passRadius = radiusX;
                radiusX = 0.0f;
            }
            passAxis = 0;
            passRes = dstWidth;
        }
        if ( passCount >= passLimit
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
                        291,
                        0,
                        "%s",
                        "passCount < passLimit") )
        {
            __debugbreak();
        }
        RB_GenerateGaussianFilter1D(passRadius, dstRes, passAxis, &filterPass[passCount++]);
    }
    if ( passCount > passLimit
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
                    296,
                    1,
                    "%s",
                    "passCount <= passLimit") )
    {
        __debugbreak();
    }
    return passCount;
}

void __cdecl RB_GenerateGaussianFilter1D(float radius, int *res, int axis, GfxImageFilterPass *filterPass)
{
    float tapWeights[8]; // [esp+18h] [ebp-48h] BYREF
    int tapHalfCount; // [esp+38h] [ebp-28h]
    int tapIndex; // [esp+3Ch] [ebp-24h]
    float tapOffsets[8]; // [esp+40h] [ebp-20h] BYREF

    tapHalfCount = RB_GaussianFilterPoints1D(radius, res[axis], res[axis], 8, tapOffsets, tapWeights);
    filterPass->tapHalfCount = RB_PickSymmetricFilterMaterial(tapHalfCount, &filterPass->material);
    for ( tapIndex = 0; tapIndex < 8; ++tapIndex )
    {
        filterPass->tapOffsetsAndWeights[tapIndex][axis] = tapOffsets[tapIndex];
        filterPass->tapOffsetsAndWeights[tapIndex][1 - axis] = 0.0f;
        filterPass->tapOffsetsAndWeights[tapIndex][2] = 0.0f;
        filterPass->tapOffsetsAndWeights[tapIndex][3] = tapWeights[tapIndex];
    }
    filterPass->srcWidth = 1.0f;
    filterPass->srcHeight = 1.0f;
    filterPass->dstWidth = *res;
    filterPass->dstHeight = res[1];
}

int __cdecl RB_PickSymmetricFilterMaterial(int halfTapCount, const Material **material)
{
    if ( (halfTapCount <= 0 || halfTapCount > 8)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
                    82,
                    0,
                    "%s\n\t(halfTapCount) = %i",
                    "(halfTapCount > 0 && halfTapCount <= 8)",
                    halfTapCount) )
    {
        __debugbreak();
    }
    // nx-port: the decompile read this as the 32-bit word `halfTapCount` slots
    // past &rgp.poisonFXMaterial. symmetricFilterMaterial[8] follows that
    // field, so on x86 it was symmetricFilterMaterial[halfTapCount - 1]; on
    // LP64 the 4-byte stride lands inside a pointer and the truncated value
    // became the blur material -- the UI3D blur behind the settings menu then
    // died in Material_GetTechniqueSet. Index the array it always meant.
    *material = rgp.symmetricFilterMaterial[halfTapCount - 1];
    return halfTapCount;
}

int __cdecl RB_GaussianFilterPoints1D(
    float pixels,
    int srcRes,
    int dstRes,
    int tapLimit,
    float *tapOffsets,
    float *tapWeights)
{
    const char *v6; // eax
    double v7; // st7
    float v9; // [esp+8h] [ebp-58h]
    float v10; // [esp+Ch] [ebp-54h]
    float v11; // [esp+10h] [ebp-50h]
    float v12; // [esp+14h] [ebp-4Ch]
    float v13; // [esp+20h] [ebp-40h]
    float v14; // [esp+24h] [ebp-3Ch]
    int tapHalfCount; // [esp+34h] [ebp-2Ch]
    int tapIndex; // [esp+38h] [ebp-28h]
    int tapIndexa; // [esp+38h] [ebp-28h]
    int resolutionRatio; // [esp+3Ch] [ebp-24h]
    float totalWeight; // [esp+44h] [ebp-1Ch]
    float gaussianExponent; // [esp+48h] [ebp-18h]
    float weight; // [esp+4Ch] [ebp-14h]
    float weightScale; // [esp+54h] [ebp-Ch]
    float sample; // [esp+58h] [ebp-8h]
    float sample_4; // [esp+5Ch] [ebp-4h]

    iassert((pixels > 0));
    iassert((dstRes > 0));
    iassert(srcRes >= dstRes);

    resolutionRatio = (int)((float)srcRes / (float)dstRes);

    iassert(abs(srcRes - resolutionRatio * dstRes) < resolutionRatio);

    if ((resolutionRatio & 1) != 0)
        v13 = 0.0f;
    else
        v13 = 0.5f;
    gaussianExponent = -0.5f / (pixels * pixels);
    totalWeight = 0.0f;
    for (tapIndex = 0; tapIndex < tapLimit; ++tapIndex)
    {
        sample = (float)(2 * tapIndex) + v13;
        sample_4 = (float)(2 * tapIndex + 1) + v13;
        v12 = sample * (sample * gaussianExponent);
        v11 = exp(v12);
        weight = v11;
        v10 = sample_4 * (sample_4 * gaussianExponent);
        v9 = exp(v10);
        if (!tapIndex && v13 == 0.0f)
            weight = v11 * 0.5f;
        tapWeights[tapIndex] = weight + v9;
        if (tapWeights[tapIndex] == 0.0f)
            v7 = (sample + sample_4) * 0.5f / (float)srcRes;
        else
            v7 = (sample * weight + sample_4 * v9) / ((float)srcRes * tapWeights[tapIndex]);
        tapOffsets[tapIndex] = v7;
        totalWeight = totalWeight + tapWeights[tapIndex];
    }
    if (totalWeight > 0.001f)
    {
        iassert(totalWeight > 0);
        tapHalfCount = tapLimit;
        for (tapIndexa = tapLimit - 1; tapIndexa >= 0; --tapIndexa)
        {
            weightScale = 0.5f / totalWeight;
            tapWeights[tapIndexa] = tapWeights[tapIndexa] * weightScale;
            if (tapWeights[tapIndexa] < 0.01f)
                tapHalfCount = tapIndexa + 1;
        }
    }
    else
    {
        *tapWeights = 0.5f;
        return 1;
    }
    return tapHalfCount;
}

void __cdecl RB_GenerateGaussianFilter2D(
                float radius,
                int srcWidth,
                int srcHeight,
                int dstWidth,
                int dstHeight,
                GfxImageFilterPass *filterPass)
{
    float tapOffsetsY[2]; // [esp+18h] [ebp-2Ch] BYREF
    float tapWeightsX[2]; // [esp+20h] [ebp-24h] BYREF
    int tapIndex; // [esp+28h] [ebp-1Ch]
    float tapOffsetsX[2]; // [esp+2Ch] [ebp-18h] BYREF
    int x; // [esp+34h] [ebp-10h]
    int y; // [esp+38h] [ebp-Ch]
    float tapWeightsY[2]; // [esp+3Ch] [ebp-8h] BYREF

    RB_GaussianFilterPoints1D(radius, srcWidth, dstWidth, 2, tapOffsetsX, tapWeightsX);
    RB_GaussianFilterPoints1D(radius, srcHeight, dstHeight, 2, tapOffsetsY, tapWeightsY);
    tapIndex = 0;
    for ( y = 0; y < 2; ++y )
    {
        for ( x = 0; x < 2; ++x )
        {
            filterPass->tapOffsetsAndWeights[2 * tapIndex][0] = -tapOffsetsX[x];
            filterPass->tapOffsetsAndWeights[2 * tapIndex][1] = tapOffsetsY[y];
            filterPass->tapOffsetsAndWeights[2 * tapIndex][2] = 0.0f;
            filterPass->tapOffsetsAndWeights[2 * tapIndex][3] = tapWeightsX[x] * tapWeightsY[y];
            filterPass->tapOffsetsAndWeights[2 * tapIndex + 1][0] = tapOffsetsX[x];
            filterPass->tapOffsetsAndWeights[2 * tapIndex + 1][1] = tapOffsetsY[y];
            filterPass->tapOffsetsAndWeights[2 * tapIndex + 1][2] = 0.0f;
            filterPass->tapOffsetsAndWeights[2 * tapIndex++ + 1][3] = tapWeightsX[x] * tapWeightsY[y];
        }
    }
    if ( 2 * tapIndex != 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
                    206,
                    1,
                    "%s\n\t(tapIndex) = %i",
                    "(tapIndex * 2 == 8)",
                    tapIndex) )
    {
        __debugbreak();
    }
    filterPass->tapHalfCount = RB_PickSymmetricFilterMaterial(2 * tapIndex, &filterPass->material);
    filterPass->srcWidth = 1.0f;
    filterPass->srcHeight = 1.0f;
    filterPass->dstWidth = dstWidth;
    filterPass->dstHeight = dstHeight;
}

void __cdecl RB_FilterImage(GfxImageFilter *filter)
{
    int passIndex; // [esp+2Ch] [ebp-Ch]

    if (!filter
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp", 394, 0, "%s", "filter"))
    {
        __debugbreak();
    }
    if (filter->passCount <= 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
            395,
            0,
            "%s\n\t(filter->passCount) = %i",
            "(filter->passCount > 0)",
            filter->passCount))
    {
        __debugbreak();
    }
    if (!filter->sourceImage
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
            396,
            0,
            "%s",
            "filter->sourceImage"))
    {
        __debugbreak();
    }
    if (tess.indexCount)
        RB_EndTessSurface();
    for (passIndex = 0; passIndex < filter->passCount; ++passIndex)
    {
        RB_FilterPingPong(filter, passIndex);
        RB_SetupFilterPass(&filter->passes[passIndex]);
        R_Set2D(&gfxCmdBufSourceState);
        RB_DrawStretchPic(
            filter->passes[passIndex].material,
            0.0,
            0.0,
            (float)gfxCmdBufSourceState.renderTargetWidth,
            (float)gfxCmdBufSourceState.renderTargetHeight,
            0.0,
            0.0,
            filter->passes[passIndex].srcWidth,
            filter->passes[passIndex].srcHeight,
            0xFFFFFFFF,
            GFX_PRIM_STATS_CODE);
        RB_EndTessSurface();
    }
    //BG_EvalVehicleName((jpeg_decompress_struct *)filter);
}

void __cdecl RB_SetupFilterPass(const GfxImageFilterPass *filterPass)
{
  int constIndex; // [esp+4h] [ebp-4h]

  if ( filterPass->tapHalfCount >= 9u
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagefilter.cpp",
          339,
          0,
          "filterPass->tapHalfCount doesn't index ARRAY_COUNT( filterPass->tapOffsetsAndWeights ) + 1\n"
          "\t%i not in [0, %i)",
          filterPass->tapHalfCount,
          9) )
  {
    __debugbreak();
  }
  for ( constIndex = 0; constIndex < filterPass->tapHalfCount; ++constIndex )
    R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, (CodeConstant)(constIndex + 36), (float*)filterPass->tapOffsetsAndWeights[constIndex]);
}

void __cdecl RB_FilterPingPong(const GfxImageFilter *filter, int passIndex)
{
  GfxRenderTargetId finalTarget; // [esp+3h] [ebp-11h]
  GfxImage *image; // [esp+4h] [ebp-10h]
  unsigned int pingpong; // [esp+8h] [ebp-Ch]

  pingpong = passIndex & 1;
  if ( passIndex )
    image = gfxRenderTargets[9 - pingpong].image;
  else
    image = filter->sourceImage;
  R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, image);
  if ( passIndex == filter->passCount - 1 )
    finalTarget = filter->finalTarget;
  else
    finalTarget = (GfxRenderTargetId)(pingpong + 8);
  R_SetRenderTargetSize(&gfxCmdBufSourceState, finalTarget);
  R_SetRenderTarget(gfxCmdBufContext, finalTarget);
  R_ClearRenderTargetForMultiGpu(gfxCmdBufContext, finalTarget);
}

