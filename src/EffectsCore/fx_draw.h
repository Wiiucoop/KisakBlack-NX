#pragma once

#include <gfx_d3d/fxprimitives.h>

struct __declspec(align(4)) FxGenerateVertsCmd // sizeof=0x50
{                                       // XREF: ?R_DrawEffects@@YAXPAUFxCmd@@@Z/r
    FxSystem *system;
    FxBeamInfo *beamInfo;
    FxPostLightInfo *postLightInfo;
    FxSpriteInfo *spriteInfo;
    int localClientNum;
    float vieworg[3];
    float viewaxis[3][3];
    bool useReflection;                 // XREF: R_DrawEffects(FxCmd *)+74/w
    // padding byte
    // padding byte
    // padding byte
    float reflectionHeight;             // XREF: R_DrawEffects(FxCmd *)+7F/w
    unsigned __int8 genVertsCameraType; // XREF: R_DrawEffects(FxCmd *)+8A/w
    // padding byte
    // padding byte
    // padding byte
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(FxGenerateVertsCmd) == 80);
#endif

struct FxReflectParams // sizeof=0xC
{                                       // XREF: FxDrawState/r
                                        // ?FX_AddNonSpriteDrawSurfs@@YAXPAUFxCmd@@@Z/r ...
    bool enabled;                       // XREF: FX_DrawNonSpriteEffect+80/w
                                        // FX_DrawTrailsForEffect+58/w ...
    // padding byte
    // padding byte
    // padding byte
    float zAdd;                         // XREF: FX_DrawNonSpriteEffect+89/w
                                        // FX_DrawTrailsForEffect+61/w ...
    float zScale;                       // XREF: FX_DrawNonSpriteEffect+92/w
                                        // FX_DrawTrailsForEffect+6A/w ...
};

struct FxElemPreVisualState // sizeof=0x1C
{                                       // XREF: FxDrawState/r
                                        // FX_CreateImpactMarkInternal/r
    float sampleLerp;                   // XREF: FX_CreateImpactMarkInternal+8E/r
    float sampleLerpInv;                // XREF: FX_CreateImpactMarkInternal+98/r
    const FxElemDef *elemDef;
    const FxEffect *effect;
    const FxElemVisStateSample *refState;
                                        // XREF: FX_CreateImpactMarkInternal+47/r
                                        // FX_CreateImpactMarkInternal+55/r ...
    int randomSeed;
    unsigned int distanceFade;
};

struct FxDrawState // sizeof=0xDC
{                                       // XREF: FX_DrawNonSpriteEffect/r
                                        // FX_DrawSpotLightEffect/r ...
    const FxSystem *system;             // XREF: FX_DrawNonSpriteEffect+63/w
                                        // FX_DrawSpotLightEffect+9F/w ...
    const FxEffect *effect;             // XREF: FX_DrawNonSpriteEffect+2C/w
                                        // FX_DrawNonSpriteEffect+3B/r ...
    const FxElem *elem;                 // XREF: FX_DrawTrailsForEffect+70/w
    const FxElemDef *elemDef;
    unsigned int region;                // XREF: FX_DrawNonSpriteEffect+75/w
                                        // FX_DrawSpotLightEffect+AB/w ...
    orientation_t orient;
    const FxCamera *camera;
    FxReflectParams reflect;            // XREF: FX_DrawNonSpriteEffect+80/w
                                        // FX_DrawNonSpriteEffect+89/w ...
    int randomSeed;
    float msecLifeSpan;
    float msecElapsed;
    float normTimeUpdateEnd;
    float posWorld[3];
    float velDirWorld[3];
    FxElemVisualState visState;
    FxElemPreVisualState preVisState;
    float physicsLerpFrac;
    int msecDraw;                       // XREF: FX_DrawNonSpriteEffect+35/w
                                        // FX_DrawSpotLightEffect+A8/w ...
    float rotationAxis[3][3];
};

struct FxTrailSegmentDrawState // sizeof=0x3C
{                                       // XREF: FX_DrawTrail/r
                                        // FX_DrawTrail/r ...
    FxTrailDef *trailDef;
    float posWorld[3];                  // XREF: FX_DrawTrail+3B3/o
                                        // FX_DrawTrail+3C4/o ...
    float basis[2][3];                  // XREF: FX_DrawTrail+3DA/o
                                        // FX_DrawTrail+3EB/o ...
    float rotation;
    float size[2];
    float uCoord;                       // XREF: FX_DrawTrail+38E/r
                                        // FX_DrawTrail+393/r ...
    unsigned __int8 color[4];           // XREF: FX_DrawTrail+457/w
                                        // FX_DrawTrail+468/w
};

struct GfxPackedVertex;

int __cdecl FX_RegionForReflect(const FxReflectParams *reflect);
void __cdecl FX_GetNullReflection(FxReflectParams *params);
void __cdecl FX_GetReflection(bool on, float height, FxReflectParams *params);
void __cdecl FX_EvaluateVisAlpha(FxElemPreVisualState *preVisState, FxElemVisualState *visState);
unsigned __int8 __cdecl FX_InterpolateColor(
                const FxElemVisStateSample *refState,
                float valueLerp,
                float valueLerpInv,
                float sampleLerp,
                float sampleLerpInv,
                int channel);
void __cdecl FX_SetupVisualState(
                const FxElemDef *elemDef,
                const FxEffect *effect,
                int randomSeed,
                float normTimeUpdateEnd,
                FxElemPreVisualState *preVisState);
void __cdecl FX_EvaluateSize(FxElemPreVisualState *preVisState, FxElemVisualState *visState);
void __cdecl FX_EvaluateVisualState(
                int localClientNum,
                FxElemPreVisualState *preVisState,
                float msecLifeSpan,
                FxElemVisualState *visState);
double __cdecl FX_IntegrateRotationFromZero(
                const FxElemVisStateSample *refState,
                int randomSeed,
                FxRandKey randomKey,
                float sampleLerp,
                float msecLifeSpan);
double __cdecl FX_LerpFloat(float grid, float tint, float fraction);
void __cdecl FX_DrawElem_BillboardSprite(FxDrawState *draw);
bool __cdecl FX_CullElementForDraw_Sprite(const FxDrawState *draw);
unsigned int __cdecl FX_CullElementForDraw_FrustumPlaneCount(const FxDrawState *draw);
void __cdecl FX_DrawElem_BillboardSprite_NoCull(FxDrawState *draw);
void __cdecl FX_GenSpriteVerts(FxDrawState *draw, const float *tangent, const float *binormal, const float *normal);
void __cdecl FX_GetSpriteTexCoords(const FxDrawState *draw, float *s0, float *ds, float *t0, float *dt);
void __cdecl FX_DrawElem_OrientedSprite(FxDrawState *draw);
void __cdecl FX_DrawElem_OrientedSprite_NoCull(FxDrawState *draw);
void __cdecl FX_DrawElem_Tail(FxDrawState *draw);
bool __cdecl FX_CullElementForDraw_Tail(const FxDrawState *draw);
char __cdecl FX_CullCylinder(
                const FxCamera *camera,
                unsigned int frustumPlaneCount,
                const float *posWorld0,
                const float *posWorld1,
                float radius);
void __cdecl FX_DrawElem_Tail_Main(FxDrawState *draw);
void __cdecl FX_DrawElem_Line(FxDrawState *draw);
bool __cdecl FX_CullElement_Line(const FxDrawState *draw);
void __cdecl FX_DrawElem_Line_Main(FxDrawState *draw);
void __cdecl FX_DrawElem_RotatedSprite(FxDrawState *draw);
void __cdecl FX_DrawElem_RotatedSprite_NoCull(FxDrawState *draw);
void __cdecl FX_DrawElem_Cloud(FxDrawState *draw);
void __cdecl FX_DrawElem_Cloud_Main(FxDrawState *draw);
void __cdecl FX_SetPlacement(const FxDrawState *draw, GfxScaledPlacement *placement);
double __cdecl FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int atRestFraction);
bool __cdecl FX_CullElementForDraw_Cloud(const FxDrawState *draw);
void __cdecl FX_DrawElem_Model(FxDrawState *draw);
void __cdecl FX_SetPlacementFromPhysics(const FxDrawState *draw, GfxPlacement *placement);
void __cdecl FX_DrawElem_Light(FxDrawState *draw);
bool __cdecl FX_CullElementForDraw_Light(const FxDrawState *draw);
void __cdecl FX_DrawElem_Light_NoCull(FxDrawState *draw);
void __cdecl FX_DrawElem_SpotLight(FxDrawState *draw);
void __cdecl FX_DrawElem_SpotLight_NoCull(FxDrawState *draw);
void __cdecl FX_DrawNonSpriteElems(FxSystem *system, const FxReflectParams *reflect, char cameraType);
void __cdecl FX_BeginIteratingOverEffects_Cooperative(FxSystem *system);
void __cdecl FX_DrawNonSpriteEffect(
                FxSystem *system,
                FxEffect *effect,
                unsigned int elemClass,
                int drawTime,
                const FxReflectParams *reflect,
                char cameraType);
void __cdecl FX_DrawElement(FxSystem *system, const FxElemDef *elemDef, FxElem *elem, FxDrawState *draw);
double __cdecl Vec4Length(const float *v);
bool __cdecl FX_ElemSpawnedVisibleToCamera(unsigned int elemSpawnVisBits, char cameraType);
void __cdecl FX_DrawSpotLight(FxSystem *system);
void __cdecl FX_DrawSpotLightEffect(FxSystem *system, FxEffect *effect, int drawTime);
void __cdecl FX_DrawSpriteElems(FxSystem *system, int drawTime, const FxReflectParams *reflect, char cameraType);
void __cdecl FX_DrawTrailsForEffect(FxSystem *system, FxEffect *effect, int drawTime, const FxReflectParams *reflect);
FxPool<FxTrail,FxTrail> *__cdecl FX_TrailFromHandle(FxSystem *system, unsigned __int16 handle);
void __cdecl FX_DrawTrail(FxSystem *system, FxDrawState *draw, FxTrail *trail);
void __cdecl FX_TrailElem_UncompressBasis(const char (*inBasis)[3], float (*basis)[3]);
FxPool<FxTrailElem,FxTrailElem> *__cdecl FX_TrailElemFromHandle(FxSystem *system, unsigned __int16 handle);
void __cdecl FX_GenTrail_IndsForSegment(
                FxDrawState *draw,
                unsigned __int16 reservedBaseVertex,
                r_double_index_t *outIndices);
void __cdecl Fx_GenTrail_PopulateSegmentDrawState(
                FxDrawState *draw,
                float spawnDist,
                float uCoordOffset,
                const float (*basis)[3],
                FxTrailSegmentDrawState *outState);
void __cdecl FX_GenTrail_VertsForSegment(const FxTrailSegmentDrawState *segmentDrawState, GfxPackedVertex *remoteVerts);
void __cdecl FX_DrawSpriteEffect(FxSystem *system, FxEffect *effect, int drawTime, const FxReflectParams *reflect);
void __cdecl FX_GenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl FX_FillGenerateVertsCmd(int localClientNum, FxGenerateVertsCmd *cmd);

//FxPool<FxTrailElem,FxTrailElem> *__cdecl FX_PoolFromHandle_Generic<FxTrailElem,FxTrailElem,2048>(
//                FxPool<FxTrailElem,FxTrailElem> *poolArray,
//                unsigned __int16 handle);
//FxPool<FxTrail,FxTrail> *__cdecl FX_PoolFromHandle_Generic<FxTrail,FxTrail,128>(
//                FxPool<FxTrail,FxTrail> *poolArray,
//                unsigned __int16 handle);

void __cdecl FX_EvaluateDistanceFade(FxDrawState *draw);
