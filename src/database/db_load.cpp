#include "db_load.h"
#include "db_stream_load.h"
#include "db_stream.h"
#include <xanim/dobj.h>
#include "db_stringtable_load.h"

#include <xanim/xanim.h>
#include <xanim/xmodel.h>
#include <sound/snd.h>
#include <gfx_d3d/r_image.h>
#include <gfx_d3d/r_water.h>
#include <sound/snd_radverb.h>
#include <sound/snd_bank.h>
#include <physics/phys_colgeom.h>
#include <gfx_d3d/fxprimitives.h>
#include <physics/physconstraints_load_obj.h>
#include <physics/physpreset_load_obj.h>
#include <game/pathnode.h>
#include <DynEntity/DynEntity_client.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <ui/ui_utils.h>
#include <ui/ui_shared.h>
#include <universal/com_expressions.h>
#include <qcommon/com_bsp.h>
#include <physics/rope.h>
#include <glass/glass_shard.h>
#include <glass/glass.h>
#include <gfx_d3d/r_reflection_probe.h>
#include <gfx_d3d/r_extracam.h>
#include <bgame/bg_emblems.h>
#include <stringed/stringed_hooks.h>
#include <cgame/cg_effects_load_obj.h>
#include <qcommon/cm_load_obj.h>
#include <game/g_bsp.h>
#include <qcommon/cm_staticmodel.h>

// This seriously has to be one of the most aids parts of the COD codebases
unsigned __int8 *varbyte;
char *varchar;
void *varint;
void *varunsigned;
void *varDWORD;
void *varuint;
float *varfloat;
void *varraw_uint;
unsigned __int8 *varraw_byte;
void *varraw_uint128;
unsigned __int8 *varraw_byte16;
float (*varvec2_t)[2];
float (*varvec3_t)[3];
float (*varvec4_t)[4];
DObjAnimMat *varDObjAnimMat;
float (*varshared_vec3_t)[3];
__int16 *varshort;
unsigned __int16 *varushort;
__int16 (*varXQuat2)[2];
unsigned __int16 *varUnsignedShort;
unsigned __int16 *varScriptString;
const char *varConstChar;
const char **varTempString;
const char **varXString;
ScriptStringList *varScriptStringList;
complex_s *varcomplex_t;
dmaterial_t *vardmaterial_t;
XAnimParts *varXAnimParts;
XAnimIndices *varXAnimIndices;
XAnimDynamicIndices *varXAnimDynamicIndicesDeltaQuat;
XAnimDeltaPartQuat *varXAnimDeltaPartQuat;
XAnimDeltaPartQuatDataFrames *varXAnimDeltaPartQuatDataFrames;
XAnimDeltaPartQuatData *varXAnimDeltaPartQuatData;
XAnimDeltaPart *varXAnimDeltaPart;
XAnimPartTrans *varXAnimPartTrans;
XAnimDynamicIndices *varXAnimDynamicIndicesTrans;
unsigned __int8 (*varByteVec)[3];
unsigned __int16 (*varUShortVec)[3];
XAnimDynamicFrames *varXAnimDynamicFrames;
XAnimPartTransFrames *varXAnimPartTransFrames;
XAnimPartTransData *varXAnimPartTransData;
XAnimNotifyInfo *varXAnimNotifyInfo;
XAnimParts **varXAnimPartsPtr;
XBoneInfo *varXBoneInfo;
snd_curve *varsnd_curve;
snd_group *varsnd_group;
snd_radverb *varsnd_radverb;
snd_master *varsnd_master;
snd_pan *varsnd_pan;
snd_snapshot_group *varsnd_snapshot_group;
snd_snapshot *varsnd_snapshot;
snd_context *varsnd_context;
SndDriverGlobals *varSndDriverGlobals;
SndDriverGlobals **varSndDriverGlobalsPtr;
PackIndexEntry *varPackIndexEntry;
PackIndex *varPackIndex;
PackIndex **varPackIndexPtr;
char *varsnd_align_char;
snd_asset *varsnd_asset;
LoadedSound *varLoadedSound;
char *varchar_align_2048;
PrimedSound *varPrimedSound;
PrimedSound **varPrimedSoundPtr;
StreamedSound *varStreamedSound;
SoundFile *varSoundFile;
SoundFileRef *varSoundFileRef;
snd_alias_t *varsnd_alias_t;
snd_alias_list_t *varsnd_alias_list_t;
SndIndexEntry *varSndIndexEntry;
SndBank *varSndBank;
SndBank **varSndBankPtr;
SndPatch *varSndPatch;
SndPatch **varSndPatchPtr;
MaterialInfo *varMaterialInfo;
GfxWorldVertex *varGfxWorldVertex0;
GfxPackedVertex *varGfxPackedVertex0;
GfxBrushModel *varGfxBrushModel;
XSurfaceCollisionLeaf *varXSurfaceCollisionLeaf;
XSurfaceCollisionNode *varXSurfaceCollisionNode;
XSurfaceCollisionTree *varXSurfaceCollisionTree;
XRigidVertList *varXRigidVertList;
IDirect3DVertexBuffer9 **varGfxVertexBuffer;
unsigned __int16 *varXBlendInfo;
XSurfaceVertexInfo *varXSurfaceVertexInfo;
unsigned __int16 *varr_index_t;
unsigned __int16 *varr_index16_t;
XSurface *varXSurface;
GfxImageLoadDef **varGfxTextureLoad;
GfxImageLoadDef *varGfxImageLoadDef;
GfxImage *varGfxImage;
GfxImage **varGfxImagePtr;
GfxTexture *varGfxRawTexture;
water_t *varwater_t;
GfxVertexShaderLoadDef *varGfxVertexShaderLoadDef;
GfxPixelShaderLoadDef *varGfxPixelShaderLoadDef;
MaterialVertexShaderProgram *varMaterialVertexShaderProgram;
MaterialVertexShader *varMaterialVertexShader;
MaterialVertexShader **varMaterialVertexShaderPtr;
MaterialPixelShaderProgram *varMaterialPixelShaderProgram;
MaterialPixelShader *varMaterialPixelShader;
MaterialPixelShader **varMaterialPixelShaderPtr;
MaterialVertexDeclaration *varMaterialVertexDeclaration;
MaterialArgumentCodeConst *varMaterialArgumentCodeConst;
MaterialShaderArgument *varMaterialShaderArgument;
MaterialArgumentDef *varMaterialArgumentDef;
GfxStateBits *varGfxStateBits;
MaterialVertexDeclaration **varMaterialPass;
MaterialTechnique *varMaterialTechnique;
MaterialTextureDef *varMaterialTextureDef;
water_t **varMaterialTextureDefInfo;
MaterialConstantDef *varMaterialConstantDef;
MaterialTechnique **varMaterialTechniquePtr;
MaterialTechniqueSet *varMaterialTechniqueSet;
MaterialTechniqueSet **varMaterialTechniqueSetPtr;
Material *varMaterial;
Material **varMaterialHandle;
GfxLightImage *varGfxLightImage;
GfxLightDef *varGfxLightDef;
GfxLightDef **varGfxLightDefPtr;
GfxLight *varGfxLight;
GfxSurface *varGfxSurface;
GfxLightmapArray *varGfxLightmapArray;
PhysPreset *varPhysPreset;
PhysPreset **varPhysPresetPtr;
PhysConstraints *varPhysConstraints;
PhysConstraint *varPhysConstraint;
PhysConstraints **varPhysConstraintsPtr;
cplane_s *varcplane_t;
cbrushside_t *varcbrushside_t;
XModelCollTri_s *varXModelCollTri;
XModelCollSurf_s *varXModelCollSurf;
XModelHighMipBounds *varXModelHighMipBounds;
XModelStreamInfo *varXModelStreamInfo;
XModel *varXModel;
XModel **varXModelPtr;
PhysGeomInfo *varPhysGeomInfo;
BrushWrapper *varBrushWrapper;
PhysGeomList *varPhysGeomList;
Collmap *varCollmap;
XModelPieces *varXModelPieces;
XModelPieces **varXModelPiecesPtr;
XModelPiece *varXModelPiece;
pathlink_s *varpathlink_t;
pathnode_constant_t *varpathnode_constant_t;
pathnode_t *varpathnode_t;
pathbasenode_t *varpathbasenode_t;
pathnode_tree_nodes_t *varpathnode_tree_nodes_t;
pathnode_tree_t **varpathnode_tree_ptr;
pathnode_tree_t *varpathnode_tree_t;
PathData *varPathData;
GameWorldSp *varGameWorldSp;
GameWorldSp **varGameWorldSpPtr;
GameWorldMp *varGameWorldMp;
GameWorldMp **varGameWorldMpPtr;
const FxEffectDef **varFxEffectDefHandle;
FxElemMarkVisuals *varFxElemMarkVisuals;
FxElemVisuals *varFxElemVisuals;
FxElemVisualState *varFxElemVisualState;
FxElemSpawnSound *varFxElemSpawnSound;
FxElemVelStateSample *varFxElemVelStateSample;
FxElemVisStateSample *varFxElemVisStateSample;
FxElemDef *varFxElemDef;
FxTrailVertex *varFxTrailVertex;
FxTrailDef *varFxTrailDef;
FxElemDefVisuals *varFxElemDefVisuals;
FxEffectDefRef *varFxEffectDefRef;
FxEffectDef *varFxEffectDef;
DynEntityDef *varDynEntityDef;
DynEntityPose *varDynEntityPose;
DynEntityColl *varDynEntityColl;
DynEntityClient *varDynEntityClient;
DynEntityServer *varDynEntityServer;
MapEnts *varMapEnts;
MapEnts **varMapEntsPtr;
cStaticModel_s *varcStaticModel_t;
cNode_t *varcNode_t;
cLeaf_s *varcLeaf_t;
cLeafBrushNodeLeaf_t *varcLeafBrushNodeLeaf_t;
unsigned __int16 *varLeafBrush;
cLeafBrushNode_s *varcLeafBrushNode_t;
cLeafBrushNodeChildren_t *varcLeafBrushNodeChildren_t;
cLeafBrushNodeData_t *varcLeafBrushNodeData_t;
CollisionBorder *varCollisionBorder;
CollisionPartition *varCollisionPartition;
CollisionAabbTree *varCollisionAabbTree;
CollisionAabbTreeIndex *varCollisionAabbTreeIndex;
cmodel_t *varcmodel_t;
cbrush_t *varcbrush_t;
rope_t *varrope_t;
clipMap_t *varclipMap_t;
clipMap_t **varclipMap_ptr;
ComPrimaryLight *varComPrimaryLight;
ComWaterCell *varComWaterCell;
ComBurnableSample *varComBurnableSample;
ComBurnableCell *varComBurnableCell;
ComWorld *varComWorld;
ComWorld **varComWorldPtr;
Operand *varOperand;
expDataType *varoperandDataType;
operandInternalDataUnion *varoperandInternalDataUnion;
expressionRpn *varexpressionRpn;
expressionRpnDataUnion *varexpressionRpnDataUnion;
ExpressionStatement *varExpressionStatement;
ScriptCondition *varScriptCondition;
ScriptCondition *varScriptConditionNext;
GenericEventScript *varGenericEventScript;
GenericEventScript *varGenericEventScriptNext;
GenericEventHandler *varGenericEventHandler;
GenericEventHandler *varGenericEventHandlerNext;
ItemKeyHandler *varItemKeyHandler;
ItemKeyHandler *varItemKeyHandlerNext;
editFieldDef_s *vareditFieldDef_t;
editFieldDef_s **vareditFieldDef_ptr;
multiDef_s *varmultiDef_t;
multiDef_s **varmultiDef_ptr;
enumDvarDef_s *varenumDvarDef_t;
enumDvarDef_s **varenumDvarDef_ptr;
MenuCell *varMenuCell;
MenuRow *varMenuRow;
listBoxDef_s *varlistBoxDef_t;
animParamsDef_t *varanimParamsDef_t;
windowDef_t *varwindowDef_t;
windowDef_t *varWindow;
windowDef_t **varwindowDef_ptr;
animParamsDef_t **varanimParamsDef_ptr;
imageDef_s *varimageDef_t;
imageDef_s **varimageDef_ptr;
ownerDrawDef_s *varownerDrawDef_t;
ownerDrawDef_s **varownerDrawDef_ptr;
listBoxDef_s **varlistBoxDef_ptr;
focusDefData_t *varfocusDefData_t;
itemDef_s *varitemDef_t;
focusItemDef_s *varfocusItemDef_t;
focusItemDef_s **varfocusItemDef_ptr;
gameMsgDef_s *vargameMsgDef_t;
gameMsgDef_s **vargameMsgDef_ptr;
textDefData_t *vartextDefData_t;
textExp_s *vartextExp_t;
textDef_s *vartextDef_t;
textDef_s **vartextDef_ptr;
rectData_s *varrectData_t;
itemDefData_t *varitemDefData_t;
UIAnimInfo *varUIAnimInfo;
itemDef_s **varitemDef_ptr;
menuDef_t *varmenuDef_t;
menuDef_t **varmenuDef_ptr;
MenuList **varMenuListPtr;
MenuList *varMenuList;
LocalizeEntry *varLocalizeEntry;
LocalizeEntry **varLocalizeEntryPtr;
FxImpactEntry *varFxImpactEntry;
FxImpactTable *varFxImpactTable;
FxImpactTable **varFxImpactTablePtr;
DestructibleStage *varDestructibleStage;
DestructiblePiece *varDestructiblePiece;
DestructiblePiece **varDestructiblePiecePtr;
DestructibleDef **varDestructibleDefPtr;
DestructibleDef *varDestructibleDef;
flameTable *varflameTable;
flameTable **varflameTablePtr;
WeaponDef *varWeaponDef;
WeaponVariantDef **varWeaponVariantDefPtr;
WeaponVariantDef *varWeaponVariantDef;
EmblemIcon *varEmblemIcon;
EmblemBackground *varEmblemBackground;
EmblemLayer *varEmblemLayer;
EmblemCategory *varEmblemCategory;
EmblemSet *varEmblemSet;
EmblemSet **varEmblemSetPtr;
RawFile *varRawFile;
RawFile **varRawFilePtr;
XGlobals *varXGlobals;
XGlobals **varXGlobalsPtr;
StringTableCell *varStringTableCell;
StringTable *varStringTable;
StringTable **varStringTablePtr;
ddlMemberDef_t *varddlMemberDef_t;
ddlStructDef_t *varddlStructDef_t;
ddlEnumDef_t *varddlEnumDef_t;
ddlDef_t *varddlDef_t;
ddlDef_t *varddlDefNext;
ddlRoot_t *varddlRoot_t;
ddlRoot_t **varddlRoot_ptr;
GfxStaticModelDrawInst *varGfxStaticModelDrawInst;
GfxStaticModelInst *varGfxStaticModelInst;
sunflare_t *varsunflare_t;
GfxReflectionProbeVolumeData *varGfxReflectionProbeVolumeData;
GfxReflectionProbe *varGfxReflectionProbe;
Occluder *varOccluder;
unsigned __int16 *varStaticModelIndex;
GfxAabbTree *varGfxAabbTree;
GfxCell *varGfxCell;
GfxPortal *varGfxPortal;
GfxCullGroup *varGfxCullGroup;
GfxLightGridEntry *varGfxLightGridEntry;
GfxCompressedLightGridColors *varGfxCompressedLightGridColors;
MaterialMemory *varMaterialMemory;
GfxWorldVertexData *varGfxWorldVertexData;
GfxWorldDraw *varGfxWorldDraw;
GfxWorldVertexLayerData *varGfxWorldVertexLayerData;
unsigned __int8 *varaligned_byte_pointer;
GfxLightGrid *varGfxLightGrid;
GfxStreamingAabbTree *varGfxStreamingAabbTree;
GfxWorldStreamInfo *varGfxWorldStreamInfo;
GfxSceneDynModel *varGfxSceneDynModel;
GfxSceneDynBrush *varGfxSceneDynBrush;
GfxDrawSurf *varGfxDrawSurf;
GfxShadowGeometry *varGfxShadowGeometry;
GfxWaterBuffer *varGfxWaterBuffer;
GfxWorldLodChain *varGfxWorldLodChain;
GfxWorldLodInfo *varGfxWorldLodInfo;
GfxLightCorona *varGfxLightCorona;
GfxShadowMapVolume *varGfxShadowMapVolume;
GfxExposureVolume *varGfxExposureVolume;
GfxVolumePlane *varGfxVolumePlane;
GfxLightRegionAxis *varGfxLightRegionAxis;
GfxLightRegionHull *varGfxLightRegionHull;
GfxLightRegion *varGfxLightRegion;
GfxWorldDpvsDynamic *varGfxWorldDpvsDynamic;
GfxWorld *varGfxWorld;
GfxWorldDpvsStatic *varGfxWorldDpvsStatic;
GfxWorldDpvsPlanes *varGfxWorldDpvsPlanes;
GfxOutdoorBounds *varGfxOutdoorBounds;
GfxHeroLight *varGfxHeroLight;
GfxHeroLightTree *varGfxHeroLightTree;
GfxWorld **varGfxWorldPtr;
Glyph *varGlyph;
Font_s *varFont;
Font_s **varFontHandle;
GlassDef *varGlassDef;
Glasses *varGlasses;
Glasses **varGlassesPtr;
Glass *varGlass;
XAsset *varXAsset;
XAssetHeader *varXAssetHeader;
pathnode_tree_info_t *varpathnode_tree_info_t;


void __cdecl Load_byteArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varbyte, count);
}

void __cdecl Load_charArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varchar, count);
}

void __cdecl Load_int(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varint, 4);
}

void __cdecl Load_intArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varint, 4 * count);
}

void __cdecl Load_unsignedArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varunsigned, 4 * count);
}

void __cdecl Load_DWORDArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDWORD, 4 * count);
}

void __cdecl Load_uint(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varuint, 4);
}

void __cdecl Load_uintArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varuint, 4 * count);
}

void __cdecl Load_float(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varfloat, 4);
}

void __cdecl Load_floatArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varfloat, 4 * count);
}

void __cdecl Load_raw_uintArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varraw_uint, 4 * count);
}

unsigned __int8 *__cdecl AllocLoad_raw_uint128()
{
    return DB_AllocStreamPos(127);
}

void __cdecl Load_raw_uint128Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varraw_uint128, 4 * count);
}

void __cdecl Load_raw_byteArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varraw_byte, count);
}

void __cdecl Load_raw_byte16Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varraw_byte16, count);
}

void __cdecl Load_vec2_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec2_t, 8 * count);
}

void __cdecl Load_vec3_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec3_t, 12);
}

void __cdecl Load_vec3_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec3_t, 12 * count);
}

void __cdecl Load_vec4_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec4_t, 16 * count);
}

void __cdecl Load_DObjAnimMatArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDObjAnimMat, 32 * count);
}

void __cdecl Load_shared_vec3_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varshared_vec3_t, 12);
}

void __cdecl Load_shared_vec3_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varshared_vec3_t, 12 * count);
}

void __cdecl Load_shortArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varshort, 2 * count);
}

void __cdecl Load_ushortArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varushort, 2 * count);
}

void __cdecl Load_XQuat2(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXQuat2, 4);
}

void __cdecl Load_XQuat2Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXQuat2, 4 * count);
}

unsigned __int8 *__cdecl AllocLoad_XBlendInfo()
{
    return DB_AllocStreamPos(1);
}

void __cdecl Load_UnsignedShortArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varUnsignedShort, 2 * count);
}

void __cdecl Load_ScriptString(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned char *)varScriptString, 2);
    Load_ScriptStringCustom(varScriptString);
}

void __cdecl Load_ScriptStringArray(bool atStreamStart, int count)
{
    unsigned __int16 *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptString, 2 * count);
    var = varScriptString;
    for ( i = 0; i < count; ++i )
    {
        varScriptString = var;
        Load_ScriptString(0);
        ++var;
    }
}

unsigned char *__cdecl AllocLoad_raw_byte()
{
    return DB_AllocStreamPos(0);
}

void __cdecl Load_ConstCharArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varConstChar, count);
}

void __cdecl Load_TempString(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varTempString, 4);
    if ( *varTempString )
    {
        if ( *varTempString == (const char *)-1 )
        {
            *varTempString = (const char *)AllocLoad_raw_byte();
            varConstChar = *varTempString;
            Load_TempStringCustom(varTempString);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varTempString);
        }
    }
}

void __cdecl Load_TempStringArray(bool atStreamStart, int count)
{
    const char **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varTempString, 4 * count);
    var = varTempString;
    for ( i = 0; i < count; ++i )
    {
        varTempString = var;
        Load_TempString(0);
        ++var;
    }
}

void __cdecl Load_XString(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXString, 4);
    if ( *varXString )
    {
        if ( *varXString == (const char *)-1 )
        {
            *varXString = (const char *)AllocLoad_raw_byte();
            varConstChar = *varXString;
            Load_XStringCustom(varXString);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varXString);
        }
    }
}

void __cdecl Load_XStringArray(bool atStreamStart, int count)
{
    const char **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXString, 4 * count);
    var = varXString;
    for ( i = 0; i < count; ++i )
    {
        varXString = var;
        Load_XString(0);
        ++var;
    }
}

void __cdecl Load_ScriptStringList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptStringList, 8);
    DB_PushStreamPos(4u);
    if ( varScriptStringList->strings )
    {
        varScriptStringList->strings = (const char **)AllocLoad_FxElemVisStateSample();
        varTempString = varScriptStringList->strings;
        Load_TempStringArray(1, varScriptStringList->count);
    }
    DB_PopStreamPos();
}

void __cdecl Load_complex_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcomplex_t, 8 * count);
}

void __cdecl Load_dmaterial_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vardmaterial_t, 72 * count);
}

void __cdecl Mark_ScriptString()
{
    Mark_ScriptStringCustom(varScriptString);
}

void __cdecl Mark_ScriptStringArray(int count)
{
    unsigned __int16 *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varScriptString;
    for ( i = 0; i < count; ++i )
    {
        varScriptString = var;
        Mark_ScriptString();
        ++var;
    }
}

void __cdecl Load_XAnimIndices()
{
    if ( varXAnimParts->numframes >= 0x100u )
    {
        if ( varXAnimIndices->_1 )
        {
            varXAnimIndices->_1 = AllocLoad_XBlendInfo();
            varushort = (unsigned __int16 *)varXAnimIndices->_1;
            Load_ushortArray(1, varXAnimParts->indexCount);
        }
    }
    else if ( varXAnimIndices->_1 )
    {
        varXAnimIndices->_1 = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varXAnimIndices->_1;
        Load_byteArray(1, varXAnimParts->indexCount);
    }
}

void __cdecl Load_XAnimDynamicIndicesDeltaQuat(bool atStreamStart)
{
    if ( varXAnimParts->numframes >= 0x100u )
    {
        if ( !atStreamStart
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1312,
                        0,
                        "%s",
                        "atStreamStart") )
        {
            __debugbreak();
        }
        Load_Stream(1, varXAnimDynamicIndicesDeltaQuat->_1, 0);
        if ( DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesDeltaQuat
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1314,
                        0,
                        "%s",
                        "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesDeltaQuat->_2 )") )
        {
            __debugbreak();
        }
        varUnsignedShort = (unsigned __int16 *)varXAnimDynamicIndicesDeltaQuat;
        Load_UnsignedShortArray(1, varXAnimDeltaPartQuat->size + 1);
    }
    else
    {
        if ( !atStreamStart
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1304,
                        0,
                        "%s",
                        "atStreamStart") )
        {
            __debugbreak();
        }
        Load_Stream(1, varXAnimDynamicIndicesDeltaQuat->_1, 0);
        if ( DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesDeltaQuat
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1306,
                        0,
                        "%s",
                        "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesDeltaQuat->_1 )") )
        {
            __debugbreak();
        }
        varbyte = (unsigned __int8 *)varXAnimDynamicIndicesDeltaQuat;
        Load_byteArray(1, varXAnimDeltaPartQuat->size + 1);
    }
}

void __cdecl Load_XAnimDeltaPartQuatDataFrames(bool atStreamStart)
{
    if ( !atStreamStart
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    1371,
                    0,
                    "%s",
                    "atStreamStart") )
    {
        __debugbreak();
    }
    Load_Stream(1, (unsigned __int8 *)varXAnimDeltaPartQuatDataFrames, 4);
    if ( DB_GetStreamPos() != (unsigned __int8 *)&varXAnimDeltaPartQuatDataFrames->indices
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    1373,
                    0,
                    "%s",
                    "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimDeltaPartQuatDataFrames->indices )") )
    {
        __debugbreak();
    }
    varXAnimDynamicIndicesDeltaQuat = &varXAnimDeltaPartQuatDataFrames->indices;
    Load_XAnimDynamicIndicesDeltaQuat(1);
    if ( varXAnimDeltaPartQuatDataFrames->frames )
    {
        varXAnimDeltaPartQuatDataFrames->frames = (__int16 (*)[2])AllocLoad_FxElemVisStateSample();
        varXQuat2 = varXAnimDeltaPartQuatDataFrames->frames;
        if ( varXAnimDeltaPartQuat->size )
            Load_XQuat2Array(1, varXAnimDeltaPartQuat->size + 1);
        else
            Load_XQuat2Array(1, 0);
    }
}

void __cdecl Load_XAnimDeltaPartQuatData(bool atStreamStart)
{
    if ( varXAnimDeltaPartQuat->size )
    {
        varXAnimDeltaPartQuatDataFrames = &varXAnimDeltaPartQuatData->frames;
        Load_XAnimDeltaPartQuatDataFrames(atStreamStart);
    }
    else if ( atStreamStart )
    {
        varXQuat2 = (__int16 (*)[2])varXAnimDeltaPartQuatData;
        Load_XQuat2(atStreamStart);
    }
}

void __cdecl Load_XAnimDeltaPartQuat(bool atStreamStart)
{
    if ( !atStreamStart
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    1510,
                    0,
                    "%s",
                    "atStreamStart") )
    {
        __debugbreak();
    }
    Load_Stream(1, (unsigned __int8 *)varXAnimDeltaPartQuat, 4);
    if ( DB_GetStreamPos() != (unsigned __int8 *)&varXAnimDeltaPartQuat->u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    1512,
                    0,
                    "%s",
                    "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimDeltaPartQuat->u )") )
    {
        __debugbreak();
    }
    varXAnimDeltaPartQuatData = &varXAnimDeltaPartQuat->u;
    Load_XAnimDeltaPartQuatData(1);
}

void __cdecl Load_XAnimDeltaPart(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimDeltaPart, 8);
    if ( varXAnimDeltaPart->trans )
    {
        varXAnimDeltaPart->trans = (XAnimPartTrans *)AllocLoad_FxElemVisStateSample();
        varXAnimPartTrans = varXAnimDeltaPart->trans;
        Load_XAnimPartTrans(1);
    }
    if ( varXAnimDeltaPart->quat )
    {
        varXAnimDeltaPart->quat = (XAnimDeltaPartQuat *)AllocLoad_FxElemVisStateSample();
        varXAnimDeltaPartQuat = varXAnimDeltaPart->quat;
        Load_XAnimDeltaPartQuat(1);
    }
}

void __cdecl Load_XAnimDynamicIndicesTrans(bool atStreamStart)
{
    if ( varXAnimParts->numframes >= 0x100u )
    {
        if ( !atStreamStart
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1653,
                        0,
                        "%s",
                        "atStreamStart") )
        {
            __debugbreak();
        }
        Load_Stream(1, varXAnimDynamicIndicesTrans->_1, 0);
        if ( DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesTrans
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1655,
                        0,
                        "%s",
                        "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesTrans->_2 )") )
        {
            __debugbreak();
        }
        varUnsignedShort = (unsigned __int16 *)varXAnimDynamicIndicesTrans;
        Load_UnsignedShortArray(1, varXAnimPartTrans->size + 1);
    }
    else
    {
        if ( !atStreamStart
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1645,
                        0,
                        "%s",
                        "atStreamStart") )
        {
            __debugbreak();
        }
        Load_Stream(1, varXAnimDynamicIndicesTrans->_1, 0);
        if ( DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesTrans
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                        1647,
                        0,
                        "%s",
                        "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesTrans->_1 )") )
        {
            __debugbreak();
        }
        varbyte = (unsigned __int8 *)varXAnimDynamicIndicesTrans;
        Load_byteArray(1, varXAnimPartTrans->size + 1);
    }
}

void __cdecl Load_ByteVecArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varByteVec, 3 * count);
}

void __cdecl Load_UShortVecArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varUShortVec, 6 * count);
}

void __cdecl Load_XAnimDynamicFrames()
{
    if ( varXAnimPartTrans->smallTrans )
    {
        if ( varXAnimDynamicFrames->_1 )
        {
            varXAnimDynamicFrames->_1 = (unsigned __int8 (*)[3])AllocLoad_raw_byte();
            varByteVec = varXAnimDynamicFrames->_1;
            if ( varXAnimPartTrans->size )
                Load_ByteVecArray(1, varXAnimPartTrans->size + 1);
            else
                Load_ByteVecArray(1, 0);
        }
    }
    else if ( varXAnimDynamicFrames->_1 )
    {
        varXAnimDynamicFrames->_1 = (unsigned __int8 (*)[3])AllocLoad_FxElemVisStateSample();
        varUShortVec = (unsigned __int16 (*)[3])varXAnimDynamicFrames->_1;
        if ( varXAnimPartTrans->size )
            Load_UShortVecArray(1, varXAnimPartTrans->size + 1);
        else
            Load_UShortVecArray(1, 0);
    }
}

void __cdecl Load_XAnimPartTransFrames(bool atStreamStart)
{
    if ( !atStreamStart
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    1903,
                    0,
                    "%s",
                    "atStreamStart") )
    {
        __debugbreak();
    }
    Load_Stream(1, (unsigned __int8 *)varXAnimPartTransFrames, 28);
    if ( DB_GetStreamPos() != (unsigned __int8 *)&varXAnimPartTransFrames->indices
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    1905,
                    0,
                    "%s",
                    "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimPartTransFrames->indices )") )
    {
        __debugbreak();
    }
    varXAnimDynamicIndicesTrans = &varXAnimPartTransFrames->indices;
    Load_XAnimDynamicIndicesTrans(1);
    varXAnimDynamicFrames = &varXAnimPartTransFrames->frames;
    Load_XAnimDynamicFrames();
}

void __cdecl Load_XAnimPartTransData(bool atStreamStart)
{
    if ( varXAnimPartTrans->size )
    {
        varXAnimPartTransFrames = &varXAnimPartTransData->frames;
        Load_XAnimPartTransFrames(atStreamStart);
    }
    else if ( atStreamStart )
    {
        varvec3_t = (float (*)[3])varXAnimPartTransData;
        Load_vec3_t(atStreamStart);
    }
}

void __cdecl Load_XAnimPartTrans(bool atStreamStart)
{
    if ( !atStreamStart
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    2050,
                    0,
                    "%s",
                    "atStreamStart") )
    {
        __debugbreak();
    }
    Load_Stream(1, (unsigned __int8 *)varXAnimPartTrans, 4);
    if ( DB_GetStreamPos() != (unsigned __int8 *)&varXAnimPartTrans->u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../xanim/xanim_load_db.h",
                    2052,
                    0,
                    "%s",
                    "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimPartTrans->u )") )
    {
        __debugbreak();
    }
    varXAnimPartTransData = &varXAnimPartTrans->u;
    Load_XAnimPartTransData(1);
}

void __cdecl Load_XAnimNotifyInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimNotifyInfo, 8);
    varScriptString = &varXAnimNotifyInfo->name;
    Load_ScriptString(0);
}

void __cdecl Load_XAnimNotifyInfoArray(bool atStreamStart, int count)
{
    XAnimNotifyInfo *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimNotifyInfo, 8 * count);
    var = varXAnimNotifyInfo;
    for ( i = 0; i < count; ++i )
    {
        varXAnimNotifyInfo = var;
        Load_XAnimNotifyInfo(0);
        ++var;
    }
}

void __cdecl Load_XAnimParts(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimParts, 104);
    DB_PushStreamPos(4u);
    varXString = &varXAnimParts->name;
    Load_XString(0);
    if ( varXAnimParts->names )
    {
        varXAnimParts->names = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varScriptString = varXAnimParts->names;
        Load_ScriptStringArray(1, varXAnimParts->boneCount[9]);
    }
    if ( varXAnimParts->notify )
    {
        varXAnimParts->notify = (XAnimNotifyInfo *)AllocLoad_FxElemVisStateSample();
        varXAnimNotifyInfo = varXAnimParts->notify;
        Load_XAnimNotifyInfoArray(1, varXAnimParts->notifyCount);
    }
    if ( varXAnimParts->deltaPart )
    {
        varXAnimParts->deltaPart = (XAnimDeltaPart *)AllocLoad_FxElemVisStateSample();
        varXAnimDeltaPart = varXAnimParts->deltaPart;
        Load_XAnimDeltaPart(1);
    }
    if ( varXAnimParts->dataByte )
    {
        varXAnimParts->dataByte = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varXAnimParts->dataByte;
        Load_byteArray(1, varXAnimParts->dataByteCount);
    }
    if ( varXAnimParts->dataShort )
    {
        varXAnimParts->dataShort = (__int16 *)AllocLoad_XBlendInfo();
        varshort = varXAnimParts->dataShort;
        Load_shortArray(1, varXAnimParts->dataShortCount);
    }
    if ( varXAnimParts->dataInt )
    {
        varXAnimParts->dataInt = (int *)AllocLoad_FxElemVisStateSample();
        varint = varXAnimParts->dataInt;
        Load_intArray(1, varXAnimParts->dataIntCount);
    }
    if ( varXAnimParts->randomDataShort )
    {
        varXAnimParts->randomDataShort = (__int16 *)AllocLoad_XBlendInfo();
        varshort = varXAnimParts->randomDataShort;
        Load_shortArray(1, varXAnimParts->randomDataShortCount);
    }
    if ( varXAnimParts->randomDataByte )
    {
        varXAnimParts->randomDataByte = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varXAnimParts->randomDataByte;
        Load_byteArray(1, varXAnimParts->randomDataByteCount);
    }
    if ( varXAnimParts->randomDataInt )
    {
        varXAnimParts->randomDataInt = (int *)AllocLoad_FxElemVisStateSample();
        varint = varXAnimParts->randomDataInt;
        Load_intArray(1, varXAnimParts->randomDataIntCount);
    }
    varXAnimIndices = &varXAnimParts->indices;
    Load_XAnimIndices();
    DB_PopStreamPos();
}

void __cdecl Load_XAnimPartsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimPartsPtr, 4);
    DB_PushStreamPos(0);
    if ( *varXAnimPartsPtr )
    {
        value = (unsigned int)*varXAnimPartsPtr;
        if ( *varXAnimPartsPtr == (XAnimParts *)-1 || value == -2 )
        {
            *varXAnimPartsPtr = (XAnimParts *)AllocLoad_FxElemVisStateSample();
            varXAnimParts = *varXAnimPartsPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_XAnimParts(1);
            Load_XAnimPartsAsset((XAssetHeader *)varXAnimPartsPtr);
            if ( inserted )
                *inserted = *varXAnimPartsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varXAnimPartsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_XAnimNotifyInfo()
{
    varScriptString = &varXAnimNotifyInfo->name;
    Mark_ScriptString();
}

void __cdecl Mark_XAnimNotifyInfoArray(int count)
{
    XAnimNotifyInfo *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varXAnimNotifyInfo;
    for ( i = 0; i < count; ++i )
    {
        varXAnimNotifyInfo = var;
        Mark_XAnimNotifyInfo();
        ++var;
    }
}

void __cdecl Mark_XAnimParts()
{
    if ( varXAnimParts->names )
    {
        varScriptString = varXAnimParts->names;
        Mark_ScriptStringArray(varXAnimParts->boneCount[9]);
    }
    if ( varXAnimParts->notify )
    {
        varXAnimNotifyInfo = varXAnimParts->notify;
        Mark_XAnimNotifyInfoArray(varXAnimParts->notifyCount);
    }
}

void __cdecl Mark_XAnimPartsPtr()
{
    if ( *varXAnimPartsPtr )
    {
        varXAnimParts = *varXAnimPartsPtr;
        Mark_XAnimPartsAsset(varXAnimParts);
        Mark_XAnimParts();
    }
}

void __cdecl Load_XBoneInfoArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXBoneInfo, 44 * count);
}

void __cdecl Load_snd_curveArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_curve, sizeof(snd_curve) * count);
}

void __cdecl Load_snd_groupArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_group, sizeof(snd_group) * count);
}

void __cdecl Load_snd_radverbArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_radverb, sizeof(snd_radverb) * count);
}

void __cdecl Load_snd_masterArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_master, sizeof(snd_master) * count);
}

void __cdecl Load_snd_panArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_pan, sizeof(snd_pan) * count);
}

void __cdecl Load_snd_snapshot_groupArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_snapshot_group, sizeof(snd_snapshot_group) * count);
}

void __cdecl Load_snd_snapshotArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_snapshot, sizeof(snd_snapshot) * count);
}

void __cdecl Load_snd_contextArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_context, sizeof(snd_context) * count);
}

void __cdecl Load_SndDriverGlobals(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSndDriverGlobals, 52);
    DB_PushStreamPos(4u);
    varXString = &varSndDriverGlobals->name;
    Load_XString(0);
    if ( varSndDriverGlobals->groups )
    {
        varSndDriverGlobals->groups = (snd_group *)AllocLoad_FxElemVisStateSample();
        varsnd_group = varSndDriverGlobals->groups;
        Load_snd_groupArray(1, varSndDriverGlobals->groupCount);
    }
    if ( varSndDriverGlobals->curves )
    {
        varSndDriverGlobals->curves = (snd_curve *)AllocLoad_FxElemVisStateSample();
        varsnd_curve = varSndDriverGlobals->curves;
        Load_snd_curveArray(1, varSndDriverGlobals->curveCount);
    }
    if ( varSndDriverGlobals->pans )
    {
        varSndDriverGlobals->pans = (snd_pan *)AllocLoad_FxElemVisStateSample();
        varsnd_pan = varSndDriverGlobals->pans;
        Load_snd_panArray(1, varSndDriverGlobals->panCount);
    }
    if ( varSndDriverGlobals->snapshotGroups )
    {
        varSndDriverGlobals->snapshotGroups = (snd_snapshot_group *)AllocLoad_FxElemVisStateSample();
        varsnd_snapshot_group = varSndDriverGlobals->snapshotGroups;
        Load_snd_snapshot_groupArray(1, varSndDriverGlobals->snapshotGroupCount);
    }
    if ( varSndDriverGlobals->contexts )
    {
        varSndDriverGlobals->contexts = (snd_context *)AllocLoad_FxElemVisStateSample();
        varsnd_context = varSndDriverGlobals->contexts;
        Load_snd_contextArray(1, varSndDriverGlobals->contextCount);
    }
    if ( varSndDriverGlobals->masters )
    {
        varSndDriverGlobals->masters = (snd_master *)AllocLoad_FxElemVisStateSample();
        varsnd_master = varSndDriverGlobals->masters;
        Load_snd_masterArray(1, varSndDriverGlobals->masterCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_SndDriverGlobalsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varSndDriverGlobalsPtr, 4);
    DB_PushStreamPos(0);
    if ( *varSndDriverGlobalsPtr )
    {
        value = (unsigned int)*varSndDriverGlobalsPtr;
        if ( *varSndDriverGlobalsPtr == (SndDriverGlobals *)-1 || value == -2 )
        {
            *varSndDriverGlobalsPtr = (SndDriverGlobals *)AllocLoad_FxElemVisStateSample();
            varSndDriverGlobals = *varSndDriverGlobalsPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_SndDriverGlobals(1);
            Load_SndDriverGlobalsAsset(varSndDriverGlobalsPtr);
            if ( inserted )
                *inserted = *varSndDriverGlobalsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varSndDriverGlobalsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_PackIndexEntryArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPackIndexEntry, 12 * count);
}

void __cdecl Load_PackIndex(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPackIndex, 28);
    DB_PushStreamPos(4u);
    varXString = &varPackIndex->name;
    Load_XString(0);
    if ( varPackIndex->entries )
    {
        varPackIndex->entries = (PackIndexEntry *)AllocLoad_FxElemVisStateSample();
        varPackIndexEntry = varPackIndex->entries;
        Load_PackIndexEntryArray(1, varPackIndex->header.count);
    }
    DB_PopStreamPos();
}

void __cdecl Load_PackIndexPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPackIndexPtr, 4);
    DB_PushStreamPos(0);
    if ( *varPackIndexPtr )
    {
        value = (unsigned int)*varPackIndexPtr;
        if ( *varPackIndexPtr == (PackIndex *)-1 || value == -2 )
        {
            *varPackIndexPtr = (PackIndex *)AllocLoad_FxElemVisStateSample();
            varPackIndex = *varPackIndexPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_PackIndex(1);
            Load_PackIndexAsset((XAssetHeader *)varPackIndexPtr);
            if ( inserted )
                *inserted = *varPackIndexPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varPackIndexPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_SndDriverGlobalsPtr()
{
    if ( *varSndDriverGlobalsPtr )
    {
        varSndDriverGlobals = *varSndDriverGlobalsPtr;
        Mark_SndDriverGlobalsAsset(varSndDriverGlobals);
    }
}

void __cdecl Mark_PackIndexPtr()
{
    if ( *varPackIndexPtr )
    {
        varPackIndex = *varPackIndexPtr;
        Mark_PackIndexAsset(varPackIndex);
    }
}

char *__cdecl AllocLoad_snd_align_char()
{
    return (char *)DB_AllocStreamPos(2047);
}

void __cdecl Load_snd_align_charArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_align_char, count);
}

void __cdecl Load_snd_asset(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_asset, 56);
    if ( varsnd_asset->seek_table )
    {
        varsnd_asset->seek_table = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varunsigned = varsnd_asset->seek_table;
        Load_unsignedArray(1, varsnd_asset->seek_table_count);
    }
    DB_PushStreamPos(5u);
    DB_PushStreamPos(6u);
    if ( varsnd_asset->data )
    {
        varsnd_asset->data = AllocLoad_snd_align_char();
        varsnd_align_char = varsnd_asset->data;
        Load_snd_align_charArray(1, varsnd_asset->data_size);
    }
    DB_PopStreamPos();
    DB_PopStreamPos();
}

void __cdecl Load_LoadedSound(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varLoadedSound, 60);
    varXString = &varLoadedSound->name;
    Load_XString(0);
    varsnd_asset = &varLoadedSound->sound;
    Load_snd_asset(0);
}

void __cdecl Load_char_align_2048Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varchar_align_2048, count);
}

void __cdecl Load_PrimedSound(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPrimedSound, 12);
    varXString = &varPrimedSound->name;
    Load_XString(0);
    DB_PushStreamPos(5u);
    if ( varPrimedSound->buffer )
    {
        varPrimedSound->buffer = AllocLoad_snd_align_char();
        varchar_align_2048 = varPrimedSound->buffer;
        Load_char_align_2048Array(1, varPrimedSound->size);
    }
    DB_PopStreamPos();
}

void __cdecl Load_PrimedSoundPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPrimedSoundPtr, 4);
    if ( *varPrimedSoundPtr )
    {
        if ( *varPrimedSoundPtr == (PrimedSound *)-1 )
        {
            *varPrimedSoundPtr = (PrimedSound *)AllocLoad_FxElemVisStateSample();
            varPrimedSound = *varPrimedSoundPtr;
            Load_PrimedSound(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varPrimedSoundPtr);
        }
    }
}

void __cdecl Load_StreamedSound(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStreamedSound, 8);
    varXString = (const char **)&varStreamedSound->filename;
    Load_XString(0);
    varPrimedSoundPtr = &varStreamedSound->primeSnd;
    Load_PrimedSoundPtr(0);
}

void __cdecl Load_SoundFileRef()
{
    if ( varSoundFile->type == 1 )
    {
        if ( varSoundFileRef->loadSnd )
        {
            if ( varSoundFileRef->loadSnd == (LoadedSound *)-1 )
            {
                varSoundFileRef->loadSnd = (LoadedSound *)AllocLoad_FxElemVisStateSample();
                varLoadedSound = varSoundFileRef->loadSnd;
                Load_LoadedSound(1);
            }
            else
            {
                DB_ConvertOffsetToPointer((unsigned int *)varSoundFileRef);
            }
        }
    }
    else if ( varSoundFileRef->loadSnd )
    {
        if ( varSoundFileRef->loadSnd == (LoadedSound *)-1 )
        {
            varSoundFileRef->loadSnd = (LoadedSound *)AllocLoad_FxElemVisStateSample();
            varStreamedSound = (StreamedSound *)varSoundFileRef->loadSnd;
            Load_StreamedSound(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varSoundFileRef);
        }
    }
}

void __cdecl Load_SoundFile(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSoundFile, 8);
    varSoundFileRef = &varSoundFile->u;
    Load_SoundFileRef();
}

void __cdecl Load_SoundFileArray(bool atStreamStart, int count)
{
    SoundFile *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varSoundFile, 8 * count);
    var = varSoundFile;
    for ( i = 0; i < count; ++i )
    {
        varSoundFile = var;
        Load_SoundFile(0);
        ++var;
    }
}

void __cdecl Load_snd_alias_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_t, 84);
    varXString = &varsnd_alias_t->name;
    Load_XString(0);
    varXString = &varsnd_alias_t->subtitle;
    Load_XString(0);
    varXString = &varsnd_alias_t->secondaryname;
    Load_XString(0);
    if ( varsnd_alias_t->soundFile )
    {
        if ( varsnd_alias_t->soundFile == (SoundFile *)-1 )
        {
            varsnd_alias_t->soundFile = (SoundFile *)AllocLoad_FxElemVisStateSample();
            varSoundFile = varsnd_alias_t->soundFile;
            Load_SoundFile(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varsnd_alias_t->soundFile);
        }
    }
}

void __cdecl Load_snd_alias_tArray(bool atStreamStart, int count)
{
    snd_alias_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_t, 84 * count);
    var = varsnd_alias_t;
    for ( i = 0; i < count; ++i )
    {
        varsnd_alias_t = var;
        Load_snd_alias_t(0);
        ++var;
    }
}

void __cdecl Load_snd_alias_list_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_list_t, 20);
    varXString = &varsnd_alias_list_t->name;
    Load_XString(0);
    if ( varsnd_alias_list_t->head )
    {
        if ( varsnd_alias_list_t->head == (snd_alias_t *)-1 )
        {
            varsnd_alias_list_t->head = (snd_alias_t *)AllocLoad_FxElemVisStateSample();
            varsnd_alias_t = varsnd_alias_list_t->head;
            Load_snd_alias_tArray(1, varsnd_alias_list_t->count);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varsnd_alias_list_t->head);
        }
    }
}

void __cdecl Load_snd_alias_list_tArray(bool atStreamStart, int count)
{
    snd_alias_list_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_list_t, 20 * count);
    var = varsnd_alias_list_t;
    for ( i = 0; i < count; ++i )
    {
        varsnd_alias_list_t = var;
        Load_snd_alias_list_t(0);
        ++var;
    }
}

void __cdecl Load_SndIndexEntryArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSndIndexEntry, 4 * count);
}

void __cdecl Load_SndBank(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSndBank, 40);
    DB_PushStreamPos(4u);
    varXString = &varSndBank->name;
    Load_XString(0);
    if ( varSndBank->alias )
    {
        varSndBank->alias = (snd_alias_list_t *)AllocLoad_FxElemVisStateSample();
        varsnd_alias_list_t = varSndBank->alias;
        Load_snd_alias_list_tArray(1, varSndBank->aliasCount);
    }
    if ( varSndBank->aliasIndex )
    {
        varSndBank->aliasIndex = (SndIndexEntry *)AllocLoad_FxElemVisStateSample();
        varSndIndexEntry = varSndBank->aliasIndex;
        Load_SndIndexEntryArray(1, varSndBank->aliasCount);
    }
    if ( varSndBank->radverbs )
    {
        varSndBank->radverbs = (snd_radverb *)AllocLoad_FxElemVisStateSample();
        varsnd_radverb = varSndBank->radverbs;
        Load_snd_radverbArray(1, varSndBank->radverbCount);
    }
    if ( varSndBank->snapshots )
    {
        varSndBank->snapshots = (snd_snapshot *)AllocLoad_FxElemVisStateSample();
        varsnd_snapshot = varSndBank->snapshots;
        Load_snd_snapshotArray(1, varSndBank->snapshotCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_SndBankPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varSndBankPtr, 4);
    DB_PushStreamPos(0);
    if ( *varSndBankPtr )
    {
        value = (unsigned int)*varSndBankPtr;
        if ( *varSndBankPtr == (SndBank *)-1 || value == -2 )
        {
            *varSndBankPtr = (SndBank *)AllocLoad_FxElemVisStateSample();
            varSndBank = *varSndBankPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_SndBank(1);
            Load_SndBankAsset((XAssetHeader *)varSndBankPtr);
            if ( inserted )
                *inserted = *varSndBankPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varSndBankPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_SndPatch(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSndPatch, 20);
    DB_PushStreamPos(4u);
    varXString = (const char **)&varSndPatch->name;
    Load_XString(0);
    if ( varSndPatch->elements )
    {
        varSndPatch->elements = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varuint = varSndPatch->elements;
        Load_uintArray(1, varSndPatch->elementCount);
    }
    if ( varSndPatch->files )
    {
        if ( varSndPatch->files == (SoundFile *)-1 )
        {
            varSndPatch->files = (SoundFile *)AllocLoad_FxElemVisStateSample();
            varSoundFile = varSndPatch->files;
            Load_SoundFileArray(1, varSndPatch->fileCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varSndPatch->files);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_SndPatchPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varSndPatchPtr, 4);
    DB_PushStreamPos(0);
    if ( *varSndPatchPtr )
    {
        value = (unsigned int)*varSndPatchPtr;
        if ( *varSndPatchPtr == (SndPatch *)-1 || value == -2 )
        {
            *varSndPatchPtr = (SndPatch *)AllocLoad_FxElemVisStateSample();
            varSndPatch = *varSndPatchPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_SndPatch(1);
            Load_SndPatchAsset((XAssetHeader *)varSndPatchPtr);
            if ( inserted )
                *inserted = *varSndPatchPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varSndPatchPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_SndBankPtr()
{
    if ( *varSndBankPtr )
    {
        varSndBank = *varSndBankPtr;
        Mark_SndBankAsset(varSndBank);
    }
}

void __cdecl Mark_SndPatchPtr()
{
    if ( *varSndPatchPtr )
    {
        varSndPatch = *varSndPatchPtr;
        Mark_SndPatchAsset(varSndPatch);
    }
}

void __cdecl Load_MaterialInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialInfo, 40);
    varXString = &varMaterialInfo->name;
    Load_XString(0);
}

void __cdecl Load_GfxWorldVertex0Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldVertex0, 44 * count);
}

void __cdecl Load_GfxPackedVertex0Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPackedVertex0, 32 * count);
}

void __cdecl Load_GfxBrushModelArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxBrushModel, 60 * count);
}

void __cdecl Load_XSurfaceCollisionLeafArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceCollisionLeaf, 2 * count);
}

void __cdecl Load_XSurfaceCollisionNodeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceCollisionNode, 16 * count);
}

void __cdecl Load_XSurfaceCollisionTree(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceCollisionTree, 40);
    if ( varXSurfaceCollisionTree->nodes )
    {
        varXSurfaceCollisionTree->nodes = (XSurfaceCollisionNode *)AllocLoad_GfxPackedVertex0();
        varXSurfaceCollisionNode = varXSurfaceCollisionTree->nodes;
        Load_XSurfaceCollisionNodeArray(1, varXSurfaceCollisionTree->nodeCount);
    }
    if ( varXSurfaceCollisionTree->leafs )
    {
        varXSurfaceCollisionTree->leafs = (XSurfaceCollisionLeaf *)AllocLoad_XBlendInfo();
        varXSurfaceCollisionLeaf = varXSurfaceCollisionTree->leafs;
        Load_XSurfaceCollisionLeafArray(1, varXSurfaceCollisionTree->leafCount);
    }
}

void __cdecl Load_XRigidVertList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXRigidVertList, 12);
    if ( varXRigidVertList->collisionTree )
    {
        if ( varXRigidVertList->collisionTree == (XSurfaceCollisionTree *)-1 )
        {
            varXRigidVertList->collisionTree = (XSurfaceCollisionTree *)AllocLoad_FxElemVisStateSample();
            varXSurfaceCollisionTree = varXRigidVertList->collisionTree;
            Load_XSurfaceCollisionTree(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varXRigidVertList->collisionTree);
        }
    }
}

void __cdecl Load_XRigidVertListArray(bool atStreamStart, int count)
{
    XRigidVertList *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXRigidVertList, 12 * count);
    var = varXRigidVertList;
    for ( i = 0; i < count; ++i )
    {
        varXRigidVertList = var;
        Load_XRigidVertList(0);
        ++var;
    }
}

void __cdecl Load_GfxVertexBuffer(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxVertexBuffer, 4);
}

void __cdecl Load_XBlendInfoArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXBlendInfo, 2 * count);
}

void __cdecl Load_XSurfaceVertexInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceVertexInfo, 16);
    if ( varXSurfaceVertexInfo->vertsBlend )
    {
        if ( varXSurfaceVertexInfo->vertsBlend == (unsigned __int16 *)-1 )
        {
            varXSurfaceVertexInfo->vertsBlend = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varXBlendInfo = varXSurfaceVertexInfo->vertsBlend;
            Load_XBlendInfoArray(
                1,
                7 * varXSurfaceVertexInfo->vertCount[3]
            + 5 * varXSurfaceVertexInfo->vertCount[2]
            + 3 * varXSurfaceVertexInfo->vertCount[1]
            + varXSurfaceVertexInfo->vertCount[0]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varXSurfaceVertexInfo->vertsBlend);
        }
    }
    if ( varXSurfaceVertexInfo->tensionData )
    {
        if ( varXSurfaceVertexInfo->tensionData == (float *)-1 )
        {
            varXSurfaceVertexInfo->tensionData = (float *)AllocLoad_FxElemVisStateSample();
            varfloat = varXSurfaceVertexInfo->tensionData;
            Load_floatArray(
                1,
                12
            * (varXSurfaceVertexInfo->vertCount[3]
             + varXSurfaceVertexInfo->vertCount[2]
             + varXSurfaceVertexInfo->vertCount[1]
             + varXSurfaceVertexInfo->vertCount[0]));
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varXSurfaceVertexInfo->tensionData);
        }
    }
}

void __cdecl Load_r_index_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varr_index_t, 2 * count);
}

void __cdecl Load_r_index16_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varr_index16_t, 2 * count);
}

void __cdecl Load_XSurface(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varXSurface->tileMode, 68);
    varXSurfaceVertexInfo = &varXSurface->vertInfo;
    Load_XSurfaceVertexInfo(0);
    if ( varXSurface->verts0 )
    {
        if ( varXSurface->verts0 == (GfxPackedVertex *)-1 )
        {
            varXSurface->verts0 = (GfxPackedVertex *)AllocLoad_GfxPackedVertex0();
            varGfxPackedVertex0 = varXSurface->verts0;
            Load_GfxPackedVertex0Array(1, varXSurface->vertCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varXSurface->verts0);
        }
    }
    if ( varXSurface->vertList )
    {
        if ( varXSurface->vertList == (XRigidVertList *)-1 )
        {
            varXSurface->vertList = (XRigidVertList *)AllocLoad_FxElemVisStateSample();
            varXRigidVertList = varXSurface->vertList;
            Load_XRigidVertListArray(1, varXSurface->vertListCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varXSurface->vertList);
        }
    }
    if ( varXSurface->triIndices )
    {
        if ( varXSurface->triIndices == (unsigned __int16 *)-1 )
        {
            varXSurface->triIndices = (unsigned __int16 *)AllocLoad_GfxPackedVertex0();
            varr_index16_t = varXSurface->triIndices;
            Load_r_index16_tArray(1, 3 * varXSurface->triCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varXSurface->triIndices);
        }
    }
}

void __cdecl Load_XSurfaceArray(bool atStreamStart, int count)
{
    XSurface *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varXSurface->tileMode, 68 * count);
    var = varXSurface;
    for ( i = 0; i < count; ++i )
    {
        varXSurface = var;
        Load_XSurface(0);
        ++var;
    }
}

void __cdecl Load_GfxTextureLoad(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxTextureLoad, 4);
    DB_PushStreamPos(0);
    if ( *varGfxTextureLoad )
    {
        value = (unsigned int)*varGfxTextureLoad;
        if ( *varGfxTextureLoad == (GfxImageLoadDef *)-1 || value == -2 )
        {
            *varGfxTextureLoad = (GfxImageLoadDef *)AllocLoad_FxElemVisStateSample();
            varGfxImageLoadDef = *varGfxTextureLoad;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxImageLoadDef(1);
            Load_Texture((GfxTexture *)varGfxTextureLoad, varGfxImage);
            if ( inserted )
                *inserted = *varGfxTextureLoad;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxTextureLoad);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxRawTextureArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxRawTexture, 4 * count);
}

void __cdecl Load_GfxImageLoadDef(bool atStreamStart)
{
    if ( !atStreamStart
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../gfx_d3d/r_image_load_db.h",
                    3791,
                    0,
                    "%s",
                    "atStreamStart") )
    {
        __debugbreak();
    }
    Load_Stream(1, &varGfxImageLoadDef->levelCount, 12);
    if ( DB_GetStreamPos() != varGfxImageLoadDef->data
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../gfx_d3d/r_image_load_db.h",
                    3793,
                    0,
                    "%s",
                    "DB_GetStreamPos() == reinterpret_cast< byte * >( varGfxImageLoadDef->data )") )
    {
        __debugbreak();
    }
    varbyte = varGfxImageLoadDef->data;
    Load_byteArray(1, varGfxImageLoadDef->resourceSize);
}

void __cdecl Load_GfxImage(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxImage, 52);
    DB_PushStreamPos(4u);
    varXString = &varGfxImage->name;
    Load_XString(0);
    varGfxTextureLoad = (GfxImageLoadDef **)varGfxImage;
    Load_GfxTextureLoad(0);
    DB_PopStreamPos();
}

void __cdecl Load_GfxImagePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxImagePtr, 4);
    DB_PushStreamPos(0);
    if ( *varGfxImagePtr )
    {
        value = (unsigned int)*varGfxImagePtr;
        if ( *varGfxImagePtr == (GfxImage *)-1 || value == -2 )
        {
            *varGfxImagePtr = (GfxImage *)AllocLoad_FxElemVisStateSample();
            varGfxImage = *varGfxImagePtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxImage(1);
            Load_GfxImageAsset((XAssetHeader*)varGfxImagePtr);
            if ( inserted )
                *inserted = *varGfxImagePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxImagePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxImagePtrArray(bool atStreamStart, int count)
{
    GfxImage **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxImagePtr, 4 * count);
    var = varGfxImagePtr;
    for ( i = 0; i < count; ++i )
    {
        varGfxImagePtr = var;
        Load_GfxImagePtr(0);
        ++var;
    }
}

void __cdecl Mark_GfxImagePtr()
{
    if ( *varGfxImagePtr )
    {
        varGfxImage = *varGfxImagePtr;
        Mark_GfxImageAsset(varGfxImage);
    }
}

void __cdecl Mark_GfxImagePtrArray(int count)
{
    GfxImage **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxImagePtr;
    for ( i = 0; i < count; ++i )
    {
        varGfxImagePtr = var;
        Mark_GfxImagePtr();
        ++var;
    }
}

void __cdecl Load_water_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varwater_t, 68);
    if ( varwater_t->H0 )
    {
        varwater_t->H0 = (complex_s *)AllocLoad_FxElemVisStateSample();
        varcomplex_t = varwater_t->H0;
        Load_complex_tArray(1, varwater_t->N * varwater_t->M);
    }
    if ( varwater_t->wTerm )
    {
        varwater_t->wTerm = (float *)AllocLoad_FxElemVisStateSample();
        varfloat = varwater_t->wTerm;
        Load_floatArray(1, varwater_t->N * varwater_t->M);
    }
    varGfxImagePtr = &varwater_t->image;
    Load_GfxImagePtr(0);
}

void __cdecl Mark_water_t()
{
    varGfxImagePtr = &varwater_t->image;
    Mark_GfxImagePtr();
}

void __cdecl Load_GfxVertexShaderLoadDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxVertexShaderLoadDef, 8);
    if ( varGfxVertexShaderLoadDef->program )
    {
        varGfxVertexShaderLoadDef->program = (DWORD *)AllocLoad_FxElemVisStateSample();
        varDWORD = varGfxVertexShaderLoadDef->program;
        Load_DWORDArray(1, varGfxVertexShaderLoadDef->programSize);
    }
}

void __cdecl Load_GfxPixelShaderLoadDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPixelShaderLoadDef, 8);
    if ( varGfxPixelShaderLoadDef->program )
    {
        varGfxPixelShaderLoadDef->program = (DWORD *)AllocLoad_FxElemVisStateSample();
        varDWORD = varGfxPixelShaderLoadDef->program;
        Load_DWORDArray(1, varGfxPixelShaderLoadDef->programSize);
    }
}

void __cdecl Load_MaterialVertexShaderProgram(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialVertexShaderProgram, 12);
    varGfxVertexShaderLoadDef = &varMaterialVertexShaderProgram->loadDef;
    Load_GfxVertexShaderLoadDef(0);
    Load_CreateMaterialVertexShader(&varMaterialVertexShaderProgram->loadDef, varMaterialVertexShader);
}

void __cdecl Load_MaterialPixelShaderProgram(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPixelShaderProgram, 12);
    varGfxPixelShaderLoadDef = &varMaterialPixelShaderProgram->loadDef;
    Load_GfxPixelShaderLoadDef(0);
    Load_CreateMaterialPixelShader(&varMaterialPixelShaderProgram->loadDef, varMaterialPixelShader);
}

void __cdecl Load_MaterialVertexShader(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialVertexShader, 16);
    varXString = &varMaterialVertexShader->name;
    Load_XString(0);
    varMaterialVertexShaderProgram = &varMaterialVertexShader->prog;
    Load_MaterialVertexShaderProgram(0);
}

void __cdecl Load_MaterialVertexShaderPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialVertexShaderPtr, 4);
    if ( *varMaterialVertexShaderPtr )
    {
        if ( *varMaterialVertexShaderPtr == (MaterialVertexShader *)-1 )
        {
            *varMaterialVertexShaderPtr = (MaterialVertexShader *)AllocLoad_FxElemVisStateSample();
            varMaterialVertexShader = *varMaterialVertexShaderPtr;
            Load_MaterialVertexShader(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varMaterialVertexShaderPtr);
        }
    }
}

void __cdecl Load_MaterialPixelShader(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPixelShader, 16);
    varXString = &varMaterialPixelShader->name;
    Load_XString(0);
    varMaterialPixelShaderProgram = &varMaterialPixelShader->prog;
    Load_MaterialPixelShaderProgram(0);
}

void __cdecl Load_MaterialPixelShaderPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPixelShaderPtr, 4);
    if ( *varMaterialPixelShaderPtr )
    {
        if ( *varMaterialPixelShaderPtr == (MaterialPixelShader *)-1 )
        {
            *varMaterialPixelShaderPtr = (MaterialPixelShader *)AllocLoad_FxElemVisStateSample();
            varMaterialPixelShader = *varMaterialPixelShaderPtr;
            Load_MaterialPixelShader(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varMaterialPixelShaderPtr);
        }
    }
}

void __cdecl Load_MaterialVertexDeclaration(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varMaterialVertexDeclaration->streamCount, 108);
}

void __cdecl Load_MaterialArgumentCodeConst(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialArgumentCodeConst, 4);
}

void __cdecl Load_MaterialArgumentDef(bool atStreamStart)
{
    switch ( varMaterialShaderArgument->type )
    {
        case 1u:
        case 7u:
            if ( varMaterialArgumentDef->codeSampler )
            {
                if ( varMaterialArgumentDef->codeSampler == -1 )
                {
                    varMaterialArgumentDef->codeSampler = (unsigned int)AllocLoad_FxElemVisStateSample();
                    varfloat = (float *)varMaterialArgumentDef->codeSampler;
                    Load_floatArray(1, 4);
                }
                else
                {
                    DB_ConvertOffsetToPointer((unsigned int *)varMaterialArgumentDef);
                }
            }
            break;
        case 3u:
        case 5u:
            if ( atStreamStart )
            {
                varMaterialArgumentCodeConst = (MaterialArgumentCodeConst *)varMaterialArgumentDef;
                Load_MaterialArgumentCodeConst(atStreamStart);
            }
            break;
        case 4u:
            if ( atStreamStart )
            {
                varuint = varMaterialArgumentDef;
                Load_uint(atStreamStart);
            }
            break;
        default:
            if ( atStreamStart )
            {
                varuint = varMaterialArgumentDef;
                Load_uint(atStreamStart);
            }
            break;
    }
}

void __cdecl Load_MaterialShaderArgument(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialShaderArgument, 8);
    varMaterialArgumentDef = &varMaterialShaderArgument->u;
    Load_MaterialArgumentDef(0);
}

void __cdecl Load_MaterialShaderArgumentArray(bool atStreamStart, int count)
{
    MaterialShaderArgument *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialShaderArgument, 8 * count);
    var = varMaterialShaderArgument;
    for ( i = 0; i < count; ++i )
    {
        varMaterialShaderArgument = var;
        Load_MaterialShaderArgument(0);
        ++var;
    }
}

void __cdecl Load_GfxStateBitsArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStateBits, 8 * count);
}

void __cdecl Load_MaterialPass(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPass, 20);
    if ( *varMaterialPass )
    {
        if ( *varMaterialPass == (MaterialVertexDeclaration *)-1 )
        {
            *varMaterialPass = (MaterialVertexDeclaration *)AllocLoad_FxElemVisStateSample();
            varMaterialVertexDeclaration = *varMaterialPass;
            Load_MaterialVertexDeclaration(1);
            Load_BuildVertexDecl(varMaterialPass);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varMaterialPass);
        }
    }
    varMaterialVertexShaderPtr = (MaterialVertexShader **)(varMaterialPass + 1);
    Load_MaterialVertexShaderPtr(0);
    varMaterialPixelShaderPtr = (MaterialPixelShader **)(varMaterialPass + 2);
    Load_MaterialPixelShaderPtr(0);
    if ( varMaterialPass[4] )
    {
        varMaterialPass[4] = (MaterialVertexDeclaration *)AllocLoad_FxElemVisStateSample();
        varMaterialShaderArgument = (MaterialShaderArgument *)varMaterialPass[4];
        Load_MaterialShaderArgumentArray(
            1,
            *((unsigned __int8 *)varMaterialPass + 14)
        + *((unsigned __int8 *)varMaterialPass + 13)
        + *((unsigned __int8 *)varMaterialPass + 12));
    }
}

void __cdecl Load_MaterialPassArray(bool atStreamStart, int count)
{
    MaterialPass *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPass, 20 * count);
    var = (MaterialPass *)varMaterialPass;
    for ( i = 0; i < count; ++i )
    {
        varMaterialPass = &var->vertexDecl;
        Load_MaterialPass(0);
        ++var;
    }
}

void __cdecl Load_MaterialTechnique(bool atStreamStart)
{
    if ( !atStreamStart
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../gfx_d3d/r_material_load_db.h",
                    5595,
                    0,
                    "%s",
                    "atStreamStart") )
    {
        __debugbreak();
    }
    Load_Stream(1, (unsigned __int8 *)varMaterialTechnique, 8);
    if ( DB_GetStreamPos() != (unsigned __int8 *)varMaterialTechnique->passArray
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\database\\../gfx_d3d/r_material_load_db.h",
                    5597,
                    0,
                    "%s",
                    "DB_GetStreamPos() == reinterpret_cast< byte * >( varMaterialTechnique->passArray )") )
    {
        __debugbreak();
    }
    varMaterialPass = &varMaterialTechnique->passArray[0].vertexDecl;
    Load_MaterialPassArray(1, varMaterialTechnique->passCount);
    varXString = &varMaterialTechnique->name;
    Load_XString(0);
}

void __cdecl Load_MaterialTextureDefInfo(bool atStreamStart)
{
    if ( varMaterialTextureDef->semantic == 11 )
    {
        if ( *varMaterialTextureDefInfo )
        {
            if ( *varMaterialTextureDefInfo == (water_t *)-1 )
            {
                *varMaterialTextureDefInfo = (water_t *)AllocLoad_FxElemVisStateSample();
                varwater_t = *varMaterialTextureDefInfo;
                Load_water_t(1);
                Load_PicmipWater(varMaterialTextureDefInfo);
            }
            else
            {
                DB_ConvertOffsetToPointer((unsigned int *)varMaterialTextureDefInfo);
            }
        }
    }
    else
    {
        varGfxImagePtr = (GfxImage **)varMaterialTextureDefInfo;
        Load_GfxImagePtr(atStreamStart);
    }
}

void __cdecl Load_MaterialTextureDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTextureDef, 16);
    varMaterialTextureDefInfo = (water_t **)&varMaterialTextureDef->u;
    Load_MaterialTextureDefInfo(0);
}

void __cdecl Load_MaterialTextureDefArray(bool atStreamStart, int count)
{
    MaterialTextureDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTextureDef, 16 * count);
    var = varMaterialTextureDef;
    for ( i = 0; i < count; ++i )
    {
        varMaterialTextureDef = var;
        Load_MaterialTextureDef(0);
        ++var;
    }
}

void __cdecl Load_MaterialConstantDefArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialConstantDef, 32 * count);
}

void __cdecl Load_MaterialTechniquePtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniquePtr, 4);
    if ( *varMaterialTechniquePtr )
    {
        if ( *varMaterialTechniquePtr == (MaterialTechnique *)-1 )
        {
            *varMaterialTechniquePtr = (MaterialTechnique *)AllocLoad_FxElemVisStateSample();
            varMaterialTechnique = *varMaterialTechniquePtr;
            Load_MaterialTechnique(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varMaterialTechniquePtr);
        }
    }
}

void __cdecl Load_MaterialTechniquePtrArray(bool atStreamStart, int count)
{
    MaterialTechnique **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniquePtr, 4 * count);
    var = varMaterialTechniquePtr;
    for ( i = 0; i < count; ++i )
    {
        varMaterialTechniquePtr = var;
        Load_MaterialTechniquePtr(0);
        ++var;
    }
}

void __cdecl Load_MaterialTechniqueSet(bool atStreamStart)
{
#ifndef KISAK_NX // nx-port: x86 layout assert
    static_assert(sizeof(MaterialTechniqueSet) == 528);
#endif
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniqueSet, sizeof(MaterialTechniqueSet));
    DB_PushStreamPos(4u);
    varXString = &varMaterialTechniqueSet->name;
    Load_XString(0);
    varMaterialTechniquePtr = varMaterialTechniqueSet->techniques;
    Load_MaterialTechniquePtrArray(0, 130);
    DB_PopStreamPos();
}

void __cdecl Load_MaterialTechniqueSetPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniqueSetPtr, 4);
    DB_PushStreamPos(0);
    if ( *varMaterialTechniqueSetPtr )
    {
        value = (unsigned int)*varMaterialTechniqueSetPtr;
        if ( *varMaterialTechniqueSetPtr == (MaterialTechniqueSet *)-1 || value == -2 )
        {
            *varMaterialTechniqueSetPtr = (MaterialTechniqueSet *)AllocLoad_FxElemVisStateSample();
            varMaterialTechniqueSet = *varMaterialTechniqueSetPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_MaterialTechniqueSet(1);
            Load_MaterialTechniqueSetAsset((XAssetHeader *)varMaterialTechniqueSetPtr);
            if ( inserted )
                *inserted = *varMaterialTechniqueSetPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMaterialTechniqueSetPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_Material(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterial, sizeof(Material));
    DB_PushStreamPos(4u);
    varMaterialInfo = &varMaterial->info;
    Load_MaterialInfo(0);
    varMaterialTechniqueSetPtr = &varMaterial->localTechniqueSet;
    Load_MaterialTechniqueSetPtr(0);
    if ( varMaterial->textureTable )
    {
        if ( varMaterial->textureTable == (MaterialTextureDef *)-1 )
        {
            varMaterial->textureTable = (MaterialTextureDef *)AllocLoad_FxElemVisStateSample();
            varMaterialTextureDef = varMaterial->textureTable;
            Load_MaterialTextureDefArray(1, varMaterial->textureCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varMaterial->textureTable);
        }
    }
    if ( varMaterial->localConstantTable )
    {
        if ( varMaterial->localConstantTable == (MaterialConstantDef *)-1 )
        {
            varMaterial->localConstantTable = (MaterialConstantDef *)AllocLoad_GfxPackedVertex0();
            varMaterialConstantDef = varMaterial->localConstantTable;
            Load_MaterialConstantDefArray(1, varMaterial->constantCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varMaterial->localConstantTable);
        }
    }
    if ( varMaterial->stateBitsTable )
    {
        if ( varMaterial->stateBitsTable == (GfxStateBits *)-1 )
        {
            varMaterial->stateBitsTable = (GfxStateBits *)AllocLoad_FxElemVisStateSample();
            varGfxStateBits = varMaterial->stateBitsTable;
            Load_GfxStateBitsArray(1, varMaterial->stateBitsCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varMaterial->stateBitsTable);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_MaterialHandle(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialHandle, 4);
    DB_PushStreamPos(0);
    if ( *varMaterialHandle )
    {
        value = (unsigned int)*varMaterialHandle;
        if ( *varMaterialHandle == (Material *)-1 || value == -2 )
        {
            *varMaterialHandle = (Material *)AllocLoad_FxElemVisStateSample();
            varMaterial = *varMaterialHandle;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_Material(1);
            Load_MaterialAsset(varMaterialHandle);
            if ( inserted )
                *inserted = *varMaterialHandle;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMaterialHandle);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_MaterialHandleArray(bool atStreamStart, int count)
{
    Material **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialHandle, 4 * count);
    var = varMaterialHandle;
    for ( i = 0; i < count; ++i )
    {
        varMaterialHandle = var;
        Load_MaterialHandle(0);
        ++var;
    }
}

void __cdecl Mark_MaterialTextureDefInfo()
{
    if ( varMaterialTextureDef->semantic == 11 )
    {
        if ( *varMaterialTextureDefInfo )
        {
            varwater_t = *varMaterialTextureDefInfo;
            Mark_water_t();
        }
    }
    else
    {
        varGfxImagePtr = (GfxImage **)varMaterialTextureDefInfo;
        Mark_GfxImagePtr();
    }
}

void __cdecl Mark_MaterialTextureDef()
{
    varMaterialTextureDefInfo = (water_t **)&varMaterialTextureDef->u;
    Mark_MaterialTextureDefInfo();
}

void __cdecl Mark_MaterialTextureDefArray(int count)
{
    MaterialTextureDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varMaterialTextureDef;
    for ( i = 0; i < count; ++i )
    {
        varMaterialTextureDef = var;
        Mark_MaterialTextureDef();
        ++var;
    }
}

void __cdecl Mark_MaterialTechniqueSetPtr()
{
    if ( *varMaterialTechniqueSetPtr )
    {
        varMaterialTechniqueSet = *varMaterialTechniqueSetPtr;
        Mark_MaterialTechniqueSetAsset(varMaterialTechniqueSet);
    }
}

void __cdecl Mark_Material()
{
    varMaterialTechniqueSetPtr = &varMaterial->localTechniqueSet;
    Mark_MaterialTechniqueSetPtr();
    if ( varMaterial->textureTable )
    {
        varMaterialTextureDef = varMaterial->textureTable;
        Mark_MaterialTextureDefArray(varMaterial->textureCount);
    }
}

void __cdecl Mark_MaterialHandle()
{
    if ( *varMaterialHandle )
    {
        varMaterial = *varMaterialHandle;
        Mark_MaterialAsset(varMaterial);
        Mark_Material();
    }
}

void __cdecl Mark_MaterialHandleArray(int count)
{
    Material **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varMaterialHandle;
    for ( i = 0; i < count; ++i )
    {
        varMaterialHandle = var;
        Mark_MaterialHandle();
        ++var;
    }
}

void __cdecl Load_GfxLightImage(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightImage, 8);
    varGfxImagePtr = &varGfxLightImage->image;
    Load_GfxImagePtr(0);
}

void __cdecl Load_GfxLightDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightDef, 16);
    DB_PushStreamPos(4u);
    varXString = &varGfxLightDef->name;
    Load_XString(0);
    varGfxLightImage = &varGfxLightDef->attenuation;
    Load_GfxLightImage(0);
    DB_PopStreamPos();
}

void __cdecl Load_GfxLightDefPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightDefPtr, 4);
    DB_PushStreamPos(0);
    if ( *varGfxLightDefPtr )
    {
        value = (unsigned int)*varGfxLightDefPtr;
        if ( *varGfxLightDefPtr == (GfxLightDef *)-1 || value == -2 )
        {
            *varGfxLightDefPtr = (GfxLightDef *)AllocLoad_FxElemVisStateSample();
            varGfxLightDef = *varGfxLightDefPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxLightDef(1);
            Load_LightDefAsset((XAssetHeader *)varGfxLightDefPtr);
            if ( inserted )
                *inserted = *varGfxLightDefPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxLightDefPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxLight(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varGfxLight->type, 368);
    varGfxLightDefPtr = &varGfxLight->def;
    Load_GfxLightDefPtr(0);
}

void __cdecl Mark_GfxLightImage()
{
    varGfxImagePtr = &varGfxLightImage->image;
    Mark_GfxImagePtr();
}

void __cdecl Mark_GfxLightDef()
{
    varGfxLightImage = &varGfxLightDef->attenuation;
    Mark_GfxLightImage();
}

void __cdecl Mark_GfxLightDefPtr()
{
    if ( *varGfxLightDefPtr )
    {
        varGfxLightDef = *varGfxLightDefPtr;
        Mark_LightDefAsset(varGfxLightDef);
        Mark_GfxLightDef();
    }
}

void __cdecl Mark_GfxLight()
{
    varGfxLightDefPtr = &varGfxLight->def;
    Mark_GfxLightDefPtr();
}

void __cdecl Load_GfxSurface(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSurface, 80);
    varMaterialHandle = &varGfxSurface->material;
    Load_MaterialHandle(0);
}

void __cdecl Load_GfxSurfaceArray(bool atStreamStart, int count)
{
    GfxSurface *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSurface, 80 * count);
    var = varGfxSurface;
    for ( i = 0; i < count; ++i )
    {
        varGfxSurface = var;
        Load_GfxSurface(0);
        ++var;
    }
}

void __cdecl Load_GfxLightmapArray(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightmapArray, 12);
    varGfxImagePtr = &varGfxLightmapArray->primary;
    Load_GfxImagePtr(0);
    varGfxImagePtr = &varGfxLightmapArray->secondary;
    Load_GfxImagePtr(0);
    varGfxImagePtr = &varGfxLightmapArray->secondaryB;
    Load_GfxImagePtr(0);
}

void __cdecl Load_GfxLightmapArrayArray(bool atStreamStart, int count)
{
    GfxLightmapArray *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightmapArray, 12 * count);
    var = varGfxLightmapArray;
    for ( i = 0; i < count; ++i )
    {
        varGfxLightmapArray = var;
        Load_GfxLightmapArray(0);
        ++var;
    }
}

void __cdecl Mark_GfxSurface()
{
    varMaterialHandle = &varGfxSurface->material;
    Mark_MaterialHandle();
}

void __cdecl Mark_GfxSurfaceArray(int count)
{
    GfxSurface *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxSurface;
    for ( i = 0; i < count; ++i )
    {
        varGfxSurface = var;
        Mark_GfxSurface();
        ++var;
    }
}

void __cdecl Mark_GfxLightmapArray()
{
    varGfxImagePtr = &varGfxLightmapArray->primary;
    Mark_GfxImagePtr();
    varGfxImagePtr = &varGfxLightmapArray->secondary;
    Mark_GfxImagePtr();
    varGfxImagePtr = &varGfxLightmapArray->secondaryB;
    Mark_GfxImagePtr();
}

void __cdecl Mark_GfxLightmapArrayArray(int count)
{
    GfxLightmapArray *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxLightmapArray;
    for ( i = 0; i < count; ++i )
    {
        varGfxLightmapArray = var;
        Mark_GfxLightmapArray();
        ++var;
    }
}

void __cdecl Load_PhysPreset(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysPreset, 84);
    DB_PushStreamPos(4u);
    varXString = &varPhysPreset->name;
    Load_XString(0);
    varXString = &varPhysPreset->sndAliasPrefix;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_PhysPresetPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysPresetPtr, 4);
    DB_PushStreamPos(0);
    if ( *varPhysPresetPtr )
    {
        value = (unsigned int)*varPhysPresetPtr;
        if ( *varPhysPresetPtr == (PhysPreset *)-1 || value == -2 )
        {
            *varPhysPresetPtr = (PhysPreset *)AllocLoad_FxElemVisStateSample();
            varPhysPreset = *varPhysPresetPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_PhysPreset(1);
            Load_PhysPresetAsset((XAssetHeader *)varPhysPresetPtr);
            if ( inserted )
                *inserted = *varPhysPresetPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varPhysPresetPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_PhysPresetPtr()
{
    if ( *varPhysPresetPtr )
    {
        varPhysPreset = *varPhysPresetPtr;
        Mark_PhysPresetAsset(varPhysPreset);
    }
}

void __cdecl Load_PhysConstraint(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysConstraint, 168);
    varScriptString = &varPhysConstraint->targetname;
    Load_ScriptString(0);
    varScriptString = &varPhysConstraint->target_ent1;
    Load_ScriptString(0);
    varXString = &varPhysConstraint->target_bone1;
    Load_XString(0);
    varScriptString = &varPhysConstraint->target_ent2;
    Load_ScriptString(0);
    varXString = &varPhysConstraint->target_bone2;
    Load_XString(0);
    varMaterialHandle = &varPhysConstraint->material;
    Load_MaterialHandle(0);
}

void __cdecl Load_PhysConstraintArray(bool atStreamStart, int count)
{
    PhysConstraint *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysConstraint, 168 * count);
    var = varPhysConstraint;
    for ( i = 0; i < count; ++i )
    {
        varPhysConstraint = var;
        Load_PhysConstraint(0);
        ++var;
    }
}

void __cdecl Load_PhysConstraints(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysConstraints, 2696);
    DB_PushStreamPos(4u);
    varXString = &varPhysConstraints->name;
    Load_XString(0);
    varPhysConstraint = varPhysConstraints->data;
    Load_PhysConstraintArray(0, 16);
    DB_PopStreamPos();
}

void __cdecl Load_PhysConstraintsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysConstraintsPtr, 4);
    DB_PushStreamPos(0);
    if ( *varPhysConstraintsPtr )
    {
        value = (unsigned int)*varPhysConstraintsPtr;
        if ( *varPhysConstraintsPtr == (PhysConstraints *)-1 || value == -2 )
        {
            *varPhysConstraintsPtr = (PhysConstraints *)AllocLoad_FxElemVisStateSample();
            varPhysConstraints = *varPhysConstraintsPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_PhysConstraints(1);
            Load_PhysConstraintsAsset((XAssetHeader *)varPhysConstraintsPtr);
            if ( inserted )
                *inserted = *varPhysConstraintsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varPhysConstraintsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_PhysConstraint()
{
    varScriptString = &varPhysConstraint->targetname;
    Mark_ScriptString();
    varScriptString = &varPhysConstraint->target_ent1;
    Mark_ScriptString();
    varScriptString = &varPhysConstraint->target_ent2;
    Mark_ScriptString();
    varMaterialHandle = &varPhysConstraint->material;
    Mark_MaterialHandle();
}

void __cdecl Mark_PhysConstraintArray(int count)
{
    PhysConstraint *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varPhysConstraint;
    for ( i = 0; i < count; ++i )
    {
        varPhysConstraint = var;
        Mark_PhysConstraint();
        ++var;
    }
}

void __cdecl Mark_PhysConstraints()
{
    varPhysConstraint = varPhysConstraints->data;
    Mark_PhysConstraintArray(16);
}

void __cdecl Mark_PhysConstraintsPtr()
{
    if ( *varPhysConstraintsPtr )
    {
        varPhysConstraints = *varPhysConstraintsPtr;
        Mark_PhysConstraintsAsset(varPhysConstraints);
        Mark_PhysConstraints();
    }
}

void __cdecl Load_cplane_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcplane_t, 20);
}

void __cdecl Load_cplane_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcplane_t, 20 * count);
}

void __cdecl Load_cbrushside_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrushside_t, 12);
    if ( varcbrushside_t->plane )
    {
        if ( varcbrushside_t->plane == (cplane_s *)-1 )
        {
            varcbrushside_t->plane = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varcbrushside_t->plane;
            Load_cplane_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varcbrushside_t);
        }
    }
}

void __cdecl Load_cbrushside_tArray(bool atStreamStart, int count)
{
    cbrushside_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrushside_t, 12 * count);
    var = varcbrushside_t;
    for ( i = 0; i < count; ++i )
    {
        varcbrushside_t = var;
        Load_cbrushside_t(0);
        ++var;
    }
}

void __cdecl Load_XModelCollTriArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelCollTri, 48 * count);
}

void __cdecl Load_XModelCollSurf(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelCollSurf, 44);
    if ( varXModelCollSurf->collTris )
    {
        varXModelCollSurf->collTris = (XModelCollTri_s *)AllocLoad_FxElemVisStateSample();
        varXModelCollTri = varXModelCollSurf->collTris;
        Load_XModelCollTriArray(1, varXModelCollSurf->numCollTris);
    }
}

void __cdecl Load_XModelCollSurfArray(bool atStreamStart, int count)
{
    XModelCollSurf_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelCollSurf, 44 * count);
    var = varXModelCollSurf;
    for ( i = 0; i < count; ++i )
    {
        varXModelCollSurf = var;
        Load_XModelCollSurf(0);
        ++var;
    }
}

void __cdecl Load_XModelHighMipBoundsArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelHighMipBounds, 16 * count);
}

void __cdecl Load_XModelStreamInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelStreamInfo, 4);
    if ( varXModelStreamInfo->highMipBounds )
    {
        varXModelStreamInfo->highMipBounds = (XModelHighMipBounds *)AllocLoad_FxElemVisStateSample();
        varXModelHighMipBounds = varXModelStreamInfo->highMipBounds;
        Load_XModelHighMipBoundsArray(1, varXModel->numsurfs);
    }
}

void __cdecl Load_BrushWrapper(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varBrushWrapper, 96);
    if ( varBrushWrapper->sides )
    {
        varBrushWrapper->sides = (cbrushside_t *)AllocLoad_FxElemVisStateSample();
        varcbrushside_t = varBrushWrapper->sides;
        Load_cbrushside_tArray(1, varBrushWrapper->numsides);
    }
    if ( varBrushWrapper->verts )
    {
        if ( varBrushWrapper->verts == (float (*)[3])-1 )
        {
            varBrushWrapper->verts = (float (*)[3])AllocLoad_FxElemVisStateSample();
            varshared_vec3_t = varBrushWrapper->verts;
            Load_shared_vec3_tArray(1, varBrushWrapper->numverts);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varBrushWrapper->verts);
        }
    }
    if ( varBrushWrapper->planes )
    {
        if ( varBrushWrapper->planes == (cplane_s *)-1 )
        {
            varBrushWrapper->planes = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varBrushWrapper->planes;
            Load_cplane_tArray(1, varBrushWrapper->numsides);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varBrushWrapper->planes);
        }
    }
}

void __cdecl Load_PhysGeomInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysGeomInfo, 68);
    if ( varPhysGeomInfo->brush )
    {
        if ( varPhysGeomInfo->brush == (BrushWrapper *)-1 )
        {
            varPhysGeomInfo->brush = (BrushWrapper *)AllocLoad_GfxPackedVertex0();
            varBrushWrapper = varPhysGeomInfo->brush;
            Load_BrushWrapper(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varPhysGeomInfo);
        }
    }
}

void __cdecl Load_PhysGeomInfoArray(bool atStreamStart, int count)
{
    PhysGeomInfo *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysGeomInfo, 68 * count);
    var = varPhysGeomInfo;
    for ( i = 0; i < count; ++i )
    {
        varPhysGeomInfo = var;
        Load_PhysGeomInfo(0);
        ++var;
    }
}

void __cdecl Load_PhysGeomList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysGeomList, 12);
    if ( varPhysGeomList->geoms )
    {
        varPhysGeomList->geoms = (PhysGeomInfo *)AllocLoad_GfxPackedVertex0();
        varPhysGeomInfo = varPhysGeomList->geoms;
        Load_PhysGeomInfoArray(1, varPhysGeomList->count);
    }
}

void __cdecl Load_Collmap(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollmap, 4);
    if ( varCollmap->geomList )
    {
        varCollmap->geomList = (PhysGeomList *)AllocLoad_FxElemVisStateSample();
        varPhysGeomList = varCollmap->geomList;
        Load_PhysGeomList(1);
    }
}

void __cdecl Load_CollmapArray(bool atStreamStart, int count)
{
    Collmap *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varCollmap, 4 * count);
    var = varCollmap;
    for ( i = 0; i < count; ++i )
    {
        varCollmap = var;
        Load_Collmap(0);
        ++var;
    }
}

void __cdecl Load_XModel(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned char *)varXModel, 252);
    DB_PushStreamPos(4u);
    varXString = &varXModel->name;
    Load_XString(0);
    if ( varXModel->localBoneNames )
    {
        if ( varXModel->localBoneNames == (unsigned __int16 *)-1 )
        {
            varXModel->localBoneNames = (unsigned short*)AllocLoad_XBlendInfo();
            varScriptString = varXModel->localBoneNames;
            Load_ScriptStringArray(1, varXModel->numBones);
        }
        else
        {
            //DB_ConvertOffsetToPointer(&varXModel->8);
            DB_ConvertOffsetToPointer((uint32_t *)&varXModel->boneNames);
        }
    }
    if ( varXModel->localParentList )
    {
        if ( varXModel->localParentList == (unsigned __int8 *)-1 )
        {
            varXModel->localParentList = (unsigned __int8 *)AllocLoad_raw_byte();
            varbyte = varXModel->localParentList;
            Load_byteArray(1, varXModel->numBones - varXModel->numRootBones);
        }
        else
        {
            //DB_ConvertOffsetToPointer(&varXModel->12);
            DB_ConvertOffsetToPointer((uint32_t *)&varXModel->parentList);
        }
    }
    if ( varXModel->localQuats )
    {
        if ( varXModel->localQuats == (__int16 *)-1 )
        {
            varXModel->localQuats = (__int16 *)AllocLoad_XBlendInfo();
            varshort = varXModel->localQuats;
            Load_shortArray(1, 4 * (varXModel->numBones - varXModel->numRootBones));
        }
        else
        {
            //DB_ConvertOffsetToPointer(&varXModel->16);
            DB_ConvertOffsetToPointer((uint32_t *)&varXModel->quats);
        }
    }
    if ( varXModel->localTrans )
    {
        if ( varXModel->localTrans == (float *)-1 )
        {
            varXModel->localTrans = (float *)AllocLoad_FxElemVisStateSample();
            varfloat = varXModel->localTrans;
            Load_floatArray(1, 4 * (varXModel->numBones - varXModel->numRootBones));
        }
        else
        {
            //DB_ConvertOffsetToPointer(&varXModel->20);
            DB_ConvertOffsetToPointer((uint32_t *)&varXModel->trans);
        }
    }
    if ( varXModel->partClassification )
    {
        if ( varXModel->partClassification == (unsigned __int8 *)-1 )
        {
            varXModel->partClassification = (unsigned __int8 *)AllocLoad_raw_byte();
            varbyte = varXModel->partClassification;
            Load_byteArray(1, varXModel->numBones);
        }
        else
        {
            //DB_ConvertOffsetToPointer(&varXModel->partClassification);
            DB_ConvertOffsetToPointer((uint32_t *)&varXModel->partClassification);
        }
    }
    if ( varXModel->baseMat )
    {
        if ( varXModel->baseMat == (DObjAnimMat *)-1 )
        {
            varXModel->baseMat = (DObjAnimMat *)AllocLoad_FxElemVisStateSample();
            varDObjAnimMat = varXModel->baseMat;
            Load_DObjAnimMatArray(1, varXModel->numBones);
        }
        else
        {
            //DB_ConvertOffsetToPointer(&varXModel->baseMat);
            DB_ConvertOffsetToPointer((uint32_t *)&varXModel->baseMat);
        }
    }
    if ( varXModel->surfs )
    {
        varXModel->surfs = (XSurface *)AllocLoad_FxElemVisStateSample();
        varXSurface = varXModel->surfs;
        Load_XSurfaceArray(1, varXModel->numsurfs);
    }
    if ( varXModel->materialHandles )
    {
        varXModel->materialHandles = (Material **)AllocLoad_FxElemVisStateSample();
        varMaterialHandle = varXModel->materialHandles;
        Load_MaterialHandleArray(1, varXModel->numsurfs);
    }
    if ( varXModel->collSurfs )
    {
        varXModel->collSurfs = (XModelCollSurf_s *)AllocLoad_FxElemVisStateSample();
        varXModelCollSurf = varXModel->collSurfs;
        Load_XModelCollSurfArray(1, varXModel->numCollSurfs);
    }
    if ( varXModel->localBoneInfo )
    {
        varXModel->localBoneInfo = (XBoneInfo *)AllocLoad_FxElemVisStateSample();
        varXBoneInfo = varXModel->localBoneInfo;
        Load_XBoneInfoArray(1, varXModel->numBones);
    }
    varXModelStreamInfo = &varXModel->streamInfo;
    Load_XModelStreamInfo(0);
    varPhysPresetPtr = &varXModel->physPreset;
    Load_PhysPresetPtr(0);
    if ( varXModel->collmaps )
    {
        varXModel->collmaps = (Collmap *)AllocLoad_FxElemVisStateSample();
        varCollmap = varXModel->collmaps;
        Load_CollmapArray(1, varXModel->numCollmaps);
    }
    varPhysConstraintsPtr = &varXModel->physConstraints;
    Load_PhysConstraintsPtr(0);
    DB_PopStreamPos();
}

void __cdecl Load_XModelPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPtr, 4);
    DB_PushStreamPos(0);
    if ( *varXModelPtr )
    {
        value = (unsigned int)*varXModelPtr;
        if ( *varXModelPtr == (XModel *)-1 || value == -2 )
        {
            *varXModelPtr = (XModel *)AllocLoad_FxElemVisStateSample();
            varXModel = *varXModelPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_XModel(1);
            Load_XModelAsset((XAssetHeader *)varXModelPtr);
            if ( inserted )
                *inserted = *varXModelPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varXModelPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_XModelPtrArray(bool atStreamStart, int count)
{
    XModel **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPtr, 4 * count);
    var = varXModelPtr;
    for ( i = 0; i < count; ++i )
    {
        varXModelPtr = var;
        Load_XModelPtr(0);
        ++var;
    }
}

void __cdecl Load_XModelPiece(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPiece, 16);
    varXModelPtr = &varXModelPiece->model;
    Load_XModelPtr(0);
}

void __cdecl Load_XModelPieceArray(bool atStreamStart, int count)
{
    XModelPiece *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPiece, 16 * count);
    var = varXModelPiece;
    for ( i = 0; i < count; ++i )
    {
        varXModelPiece = var;
        Load_XModelPiece(0);
        ++var;
    }
}

void __cdecl Load_XModelPieces(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPieces, 12);
    varXString = &varXModelPieces->name;
    Load_XString(0);
    if ( varXModelPieces->pieces )
    {
        varXModelPieces->pieces = (XModelPiece *)AllocLoad_FxElemVisStateSample();
        varXModelPiece = varXModelPieces->pieces;
        Load_XModelPieceArray(1, varXModelPieces->numpieces);
    }
}

void __cdecl Load_XModelPiecesPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPiecesPtr, 4);
    if ( *varXModelPiecesPtr )
    {
        if ( *varXModelPiecesPtr == (XModelPieces *)-1 )
        {
            *varXModelPiecesPtr = (XModelPieces *)AllocLoad_FxElemVisStateSample();
            varXModelPieces = *varXModelPiecesPtr;
            Load_XModelPieces(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varXModelPiecesPtr);
        }
    }
}

void __cdecl Mark_XModel()
{
    if ( varXModel->localBoneNames )
    {
        varScriptString = varXModel->localBoneNames;
        Mark_ScriptStringArray(varXModel->numBones);
    }
    if ( varXModel->materialHandles )
    {
        varMaterialHandle = varXModel->materialHandles;
        Mark_MaterialHandleArray(varXModel->numsurfs);
    }
    varPhysPresetPtr = &varXModel->physPreset;
    Mark_PhysPresetPtr();
    varPhysConstraintsPtr = &varXModel->physConstraints;
    Mark_PhysConstraintsPtr();
}

void __cdecl Mark_XModelPtr()
{
    if ( *varXModelPtr )
    {
        varXModel = *varXModelPtr;
        Mark_XModelAsset(varXModel);
        Mark_XModel();
    }
}

void __cdecl Mark_XModelPtrArray(int count)
{
    XModel **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varXModelPtr;
    for ( i = 0; i < count; ++i )
    {
        varXModelPtr = var;
        Mark_XModelPtr();
        ++var;
    }
}

void __cdecl Mark_XModelPiece()
{
    varXModelPtr = &varXModelPiece->model;
    Mark_XModelPtr();
}

void __cdecl Mark_XModelPieceArray(int count)
{
    XModelPiece *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varXModelPiece;
    for ( i = 0; i < count; ++i )
    {
        varXModelPiece = var;
        Mark_XModelPiece();
        ++var;
    }
}

void __cdecl Mark_XModelPieces()
{
    if ( varXModelPieces->pieces )
    {
        varXModelPiece = varXModelPieces->pieces;
        Mark_XModelPieceArray(varXModelPieces->numpieces);
    }
}

void __cdecl Mark_XModelPiecesPtr()
{
    if ( *varXModelPiecesPtr )
    {
        varXModelPieces = *varXModelPiecesPtr;
        Mark_XModelPieces();
    }
}

void __cdecl Load_pathlink_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathlink_t, 12 * count);
}

void __cdecl Load_pathnode_constant_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_constant_t, 68);
    varScriptString = &varpathnode_constant_t->targetname;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->script_linkName;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->script_noteworthy;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->target;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->animscript;
    Load_ScriptString(0);
    if ( varpathnode_constant_t->Links )
    {
        varpathnode_constant_t->Links = (pathlink_s *)AllocLoad_FxElemVisStateSample();
        varpathlink_t = varpathnode_constant_t->Links;
        Load_pathlink_tArray(1, varpathnode_constant_t->totalLinkCount);
    }
}

void __cdecl Load_pathnode_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_t, 128);
    varpathnode_constant_t = &varpathnode_t->constant;
    Load_pathnode_constant_t(0);
}

void __cdecl Load_pathnode_tArray(bool atStreamStart, int count)
{
    pathnode_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_t, count << 7);
    var = varpathnode_t;
    for ( i = 0; i < count; ++i )
    {
        varpathnode_t = var;
        Load_pathnode_t(0);
        ++var;
    }
}

void __cdecl Load_pathbasenode_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathbasenode_t, 16 * count);
}

void __cdecl Load_pathnode_tree_nodes_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_nodes_t, 8);
    if ( varpathnode_tree_nodes_t->nodes )
    {
        varpathnode_tree_nodes_t->nodes = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varpathnode_tree_nodes_t->nodes;
        Load_ushortArray(1, varpathnode_tree_nodes_t->nodeCount);
    }
}

void __cdecl Load_pathnode_tree_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_ptr, 4);
    if ( *varpathnode_tree_ptr )
    {
        if ( *varpathnode_tree_ptr == (pathnode_tree_t *)-1 )
        {
            *varpathnode_tree_ptr = (pathnode_tree_t *)AllocLoad_FxElemVisStateSample();
            varpathnode_tree_t = *varpathnode_tree_ptr;
            Load_pathnode_tree_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*) varpathnode_tree_ptr);
        }
    }
}

void __cdecl Load_pathnode_tree_ptrArray(bool atStreamStart, int count)
{
    pathnode_tree_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_ptr, 4 * count);
    var = varpathnode_tree_ptr;
    for ( i = 0; i < count; ++i )
    {
        varpathnode_tree_ptr = var;
        Load_pathnode_tree_ptr(0);
        ++var;
    }
}

void __cdecl Load_pathnode_tree_info_t(bool atStreamStart)
{
    if ( varpathnode_tree_t->axis < 0 )
    {
        varpathnode_tree_nodes_t = (pathnode_tree_nodes_t *)varpathnode_tree_info_t;
        Load_pathnode_tree_nodes_t(atStreamStart);
    }
    else
    {
        varpathnode_tree_ptr = (pathnode_tree_t **)varpathnode_tree_info_t;
        Load_pathnode_tree_ptrArray(atStreamStart, 2);
    }
}

void __cdecl Load_pathnode_tree_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_t, 16);
    varpathnode_tree_info_t = &varpathnode_tree_t->u;
    Load_pathnode_tree_info_t(0);
}

void __cdecl Load_pathnode_tree_tArray(bool atStreamStart, int count)
{
    pathnode_tree_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_t, 16 * count);
    var = varpathnode_tree_t;
    for ( i = 0; i < count; ++i )
    {
        varpathnode_tree_t = var;
        Load_pathnode_tree_t(0);
        ++var;
    }
}

void __cdecl Mark_pathnode_constant_t()
{
    varScriptString = &varpathnode_constant_t->targetname;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->script_linkName;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->script_noteworthy;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->target;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->animscript;
    Mark_ScriptString();
}

void __cdecl Mark_pathnode_t()
{
    varpathnode_constant_t = &varpathnode_t->constant;
    Mark_pathnode_constant_t();
}

void __cdecl Mark_pathnode_tArray(int count)
{
    pathnode_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varpathnode_t;
    for ( i = 0; i < count; ++i )
    {
        varpathnode_t = var;
        Mark_pathnode_t();
        ++var;
    }
}

void __cdecl Load_PathData(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPathData, 40);
    if ( varPathData->nodes )
    {
        varPathData->nodes = (pathnode_t *)AllocLoad_FxElemVisStateSample();
        varpathnode_t = varPathData->nodes;
        Load_pathnode_tArray(1, varPathData->nodeCount + 128);
    }
    DB_PushStreamPos(1u);
    if ( varPathData->basenodes )
    {
        varPathData->basenodes = (pathbasenode_t *)AllocLoad_GfxPackedVertex0();
        varpathbasenode_t = varPathData->basenodes;
        Load_pathbasenode_tArray(1, varPathData->nodeCount + 128);
    }
    DB_PopStreamPos();
    if ( varPathData->chainNodeForNode )
    {
        varPathData->chainNodeForNode = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varPathData->chainNodeForNode;
        Load_UnsignedShortArray(1, varPathData->nodeCount);
    }
    if ( varPathData->nodeForChainNode )
    {
        varPathData->nodeForChainNode = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varPathData->nodeForChainNode;
        Load_UnsignedShortArray(1, varPathData->nodeCount);
    }
    if ( varPathData->pathVis )
    {
        varPathData->pathVis = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varPathData->pathVis;
        Load_byteArray(1, varPathData->visBytes);
    }
    if ( varPathData->nodeTree )
    {
        varPathData->nodeTree = (pathnode_tree_t *)AllocLoad_FxElemVisStateSample();
        varpathnode_tree_t = varPathData->nodeTree;
        Load_pathnode_tree_tArray(1, varPathData->nodeTreeCount);
    }
}

void __cdecl Load_GameWorldSp(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldSp, 44);
    DB_PushStreamPos(4u);
    varXString = &varGameWorldSp->name;
    Load_XString(0);
    varPathData = &varGameWorldSp->path;
    Load_PathData(0);
    DB_PopStreamPos();
}

void __cdecl Load_GameWorldMp(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldMp, 44);
    DB_PushStreamPos(4u);
    varXString = &varGameWorldMp->name;
    Load_XString(0);
    varPathData = &varGameWorldMp->path;
    Load_PathData(0);
    DB_PopStreamPos();
}

void __cdecl Load_GameWorldSpPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldSpPtr, 4);
    DB_PushStreamPos(0);
    if ( *varGameWorldSpPtr )
    {
        value = (unsigned int)*varGameWorldSpPtr;
        if ( *varGameWorldSpPtr == (GameWorldSp *)-1 || value == -2 )
        {
            *varGameWorldSpPtr = (GameWorldSp *)AllocLoad_FxElemVisStateSample();
            varGameWorldSp = *varGameWorldSpPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GameWorldSp(1);
            Load_GameWorldSpAsset((XAssetHeader *)varGameWorldSpPtr);
            if ( inserted )
                *inserted = *varGameWorldSpPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGameWorldSpPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GameWorldMpPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldMpPtr, 4);
    DB_PushStreamPos(0);
    if ( *varGameWorldMpPtr )
    {
        value = (unsigned int)*varGameWorldMpPtr;
        if ( *varGameWorldMpPtr == (GameWorldMp *)-1 || value == -2 )
        {
            *varGameWorldMpPtr = (GameWorldMp *)AllocLoad_FxElemVisStateSample();
            varGameWorldMp = *varGameWorldMpPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GameWorldMp(1);
            Load_GameWorldMpAsset((XAssetHeader *)varGameWorldMpPtr);
            if ( inserted )
                *inserted = *varGameWorldMpPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGameWorldMpPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_PathData()
{
    if ( varPathData->nodes )
    {
        varpathnode_t = varPathData->nodes;
        Mark_pathnode_tArray(varPathData->nodeCount + 128);
    }
}

void __cdecl Mark_GameWorldSp()
{
    varPathData = &varGameWorldSp->path;
    Mark_PathData();
}

void __cdecl Mark_GameWorldMp()
{
    varPathData = &varGameWorldMp->path;
    Mark_PathData();
}

void __cdecl Mark_GameWorldSpPtr()
{
    if ( *varGameWorldSpPtr )
    {
        varGameWorldSp = *varGameWorldSpPtr;
        Mark_GameWorldSpAsset(varGameWorldSp);
        Mark_GameWorldSp();
    }
}

void __cdecl Mark_GameWorldMpPtr()
{
    if ( *varGameWorldMpPtr )
    {
        varGameWorldMp = *varGameWorldMpPtr;
        Mark_GameWorldMpAsset(varGameWorldMp);
        Mark_GameWorldMp();
    }
}

void __cdecl Load_FxEffectDefHandle(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxEffectDefHandle, 4);
    DB_PushStreamPos(0);
    if ( *varFxEffectDefHandle )
    {
        value = (unsigned int)*varFxEffectDefHandle;
        if ( *varFxEffectDefHandle == (const FxEffectDef *)-1 || value == -2 )
        {
            *varFxEffectDefHandle = (const FxEffectDef *)AllocLoad_FxElemVisStateSample();
            varFxEffectDef = *(FxEffectDef**)varFxEffectDefHandle;  //de-const
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_FxEffectDef(1);
            Load_FxEffectDefAsset((XAssetHeader *)varFxEffectDefHandle);
            if ( inserted )
                *inserted = *varFxEffectDefHandle;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varFxEffectDefHandle);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_FxEffectDefHandleArray(bool atStreamStart, int count)
{
    const FxEffectDef **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxEffectDefHandle, 4 * count);
    var = varFxEffectDefHandle;
    for ( i = 0; i < count; ++i )
    {
        varFxEffectDefHandle = var;
        Load_FxEffectDefHandle(0);
        ++var;
    }
}

void __cdecl Load_FxEffectDefRef(bool atStreamStart)
{
    varXString = (const char **)varFxEffectDefRef;
    Load_XString(atStreamStart);
    Load_FxEffectDefFromName((const char **)varFxEffectDefRef);
}

void __cdecl Load_FxElemMarkVisuals(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemMarkVisuals, 8);
    varMaterialHandle = (Material **)varFxElemMarkVisuals;
    Load_MaterialHandleArray(0, 2);
}

void __cdecl Load_FxElemMarkVisualsArray(bool atStreamStart, int count)
{
    FxElemMarkVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemMarkVisuals, 8 * count);
    var = varFxElemMarkVisuals;
    for ( i = 0; i < count; ++i )
    {
        varFxElemMarkVisuals = var;
        Load_FxElemMarkVisuals(0);
        ++var;
    }
}

void __cdecl Load_FxElemVisuals(bool atStreamStart)
{
    switch ( varFxElemDef->elemType )
    {
        case 7u:
            varXModelPtr = (XModel **)varFxElemVisuals;
            Load_XModelPtr(atStreamStart);
            break;
        case 0xCu:
            varFxEffectDefRef = (FxEffectDefRef *)varFxElemVisuals;
            Load_FxEffectDefRef(atStreamStart);
            break;
        case 0xAu:
            varXString = (const char **)varFxElemVisuals;
            Load_XString(atStreamStart);
            break;
        default:
            if ( varFxElemDef->elemType != 8 && varFxElemDef->elemType != 9 )
            {
                varMaterialHandle = (Material **)varFxElemVisuals;
                Load_MaterialHandle(atStreamStart);
            }
            break;
    }
}

void __cdecl Load_FxElemVisualsArray(bool atStreamStart, int count)
{
    FxElemVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemVisuals, 4 * count);
    var = varFxElemVisuals;
    for ( i = 0; i < count; ++i )
    {
        varFxElemVisuals = var;
        Load_FxElemVisuals(0);
        ++var;
    }
}

void __cdecl Load_FxElemSpawnSound(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemSpawnSound, 4);
    varXString = &varFxElemSpawnSound->spawnSound;
    Load_XString(0);
}

void __cdecl Load_FxElemVisStateSampleArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varFxElemVisStateSample->base.color, 48 * count);
}

void __cdecl Load_FxElemVelStateSampleArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemVelStateSample, 96 * count);
}

void __cdecl Load_FxElemDefVisuals(bool atStreamStart)
{
    if ( varFxElemDef->elemType == 11 )
    {
        if ( varFxElemDefVisuals->markArray )
        {
            varFxElemDefVisuals->markArray = (FxElemMarkVisuals *)AllocLoad_FxElemVisStateSample();
            varFxElemMarkVisuals = varFxElemDefVisuals->markArray;
            Load_FxElemMarkVisualsArray(1, varFxElemDef->visualCount);
        }
    }
    else if ( varFxElemDef->visualCount > 1u )
    {
        if ( varFxElemDefVisuals->markArray )
        {
            varFxElemDefVisuals->markArray = (FxElemMarkVisuals *)AllocLoad_FxElemVisStateSample();
            varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals->markArray;
            Load_FxElemVisualsArray(1, varFxElemDef->visualCount);
        }
    }
    else
    {
        varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals;
        Load_FxElemVisuals(atStreamStart);
    }
}

void __cdecl Load_FxTrailVertexArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxTrailVertex, 20 * count);
}

void __cdecl Load_FxTrailDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxTrailDef, 28);
    if ( varFxTrailDef->vertsEA )
    {
        varFxTrailDef->vertsEA = (FxTrailVertex *)AllocLoad_FxElemVisStateSample();
        varFxTrailVertex = varFxTrailDef->vertsEA;
        Load_FxTrailVertexArray(1, varFxTrailDef->vertCount);
    }
    if ( varFxTrailDef->indsEA )
    {
        varFxTrailDef->indsEA = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varFxTrailDef->indsEA;
        Load_ushortArray(1, varFxTrailDef->indCount);
    }
}

void __cdecl Load_FxElemDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemDef, 292);
    if ( varFxElemDef->velSamples )
    {
        varFxElemDef->velSamples = (const FxElemVelStateSample *)AllocLoad_FxElemVisStateSample();
        varFxElemVelStateSample = (FxElemVelStateSample *)varFxElemDef->velSamples;
        Load_FxElemVelStateSampleArray(1, varFxElemDef->velIntervalCount + 1);
    }
    if ( varFxElemDef->visSamples )
    {
        varFxElemDef->visSamples = (const FxElemVisStateSample *)AllocLoad_FxElemVisStateSample();
        varFxElemVisStateSample = (FxElemVisStateSample *)varFxElemDef->visSamples;
        Load_FxElemVisStateSampleArray(1, varFxElemDef->visStateIntervalCount + 1);
    }
    varFxElemDefVisuals = &varFxElemDef->visuals;
    Load_FxElemDefVisuals(0);
    varFxEffectDefRef = &varFxElemDef->effectOnImpact;
    Load_FxEffectDefRef(0);
    varFxEffectDefRef = &varFxElemDef->effectOnDeath;
    Load_FxEffectDefRef(0);
    varFxEffectDefRef = &varFxElemDef->effectEmitted;
    Load_FxEffectDefRef(0);
    varFxEffectDefRef = &varFxElemDef->effectAttached;
    Load_FxEffectDefRef(0);
    if ( varFxElemDef->localTrailDef )
    {
        varFxElemDef->localTrailDef = (FxTrailDef *)AllocLoad_FxElemVisStateSample();
        varFxTrailDef = varFxElemDef->localTrailDef;
        Load_FxTrailDef(1);
    }
    varFxElemSpawnSound = &varFxElemDef->spawnSound;
    Load_FxElemSpawnSound(0);
}

void __cdecl Load_FxElemDefArray(bool atStreamStart, int count)
{
    FxElemDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemDef, 292 * count);
    var = varFxElemDef;
    for ( i = 0; i < count; ++i )
    {
        varFxElemDef = var;
        Load_FxElemDef(0);
        ++var;
    }
}

void __cdecl Load_FxEffectDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxEffectDef, 60);
    DB_PushStreamPos(4u);
    varXString = &varFxEffectDef->name;
    Load_XString(0);
    if ( varFxEffectDef->elemDefsEA )
    {
        varFxEffectDef->elemDefsEA = (const FxElemDef *)AllocLoad_FxElemVisStateSample();
        varFxElemDef = (FxElemDef*)varFxEffectDef->elemDefsEA;
        Load_FxElemDefArray(
            1,
            varFxEffectDef->elemDefCountEmission + varFxEffectDef->elemDefCountOneShot + varFxEffectDef->elemDefCountLooping);
    }
    DB_PopStreamPos();
}

void __cdecl Mark_FxEffectDefHandle()
{
    if ( *varFxEffectDefHandle )
    {
        varFxEffectDef = *(FxEffectDef**)varFxEffectDefHandle; //de-const
        Mark_FxEffectDefAsset(varFxEffectDef);
        Mark_FxEffectDef();
    }
}

void __cdecl Mark_FxEffectDefHandleArray(int count)
{
    const FxEffectDef **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxEffectDefHandle;
    for ( i = 0; i < count; ++i )
    {
        varFxEffectDefHandle = var;
        Mark_FxEffectDefHandle();
        ++var;
    }
}

void __cdecl Mark_FxElemMarkVisuals()
{
    varMaterialHandle = (Material **)varFxElemMarkVisuals;
    Mark_MaterialHandleArray(2);
}

void __cdecl Mark_FxElemMarkVisualsArray(int count)
{
    FxElemMarkVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxElemMarkVisuals;
    for ( i = 0; i < count; ++i )
    {
        varFxElemMarkVisuals = var;
        Mark_FxElemMarkVisuals();
        ++var;
    }
}

void __cdecl Mark_FxElemVisuals()
{
    if ( varFxElemDef->elemType == 7 )
    {
        varXModelPtr = (XModel **)varFxElemVisuals;
        Mark_XModelPtr();
    }
    else if ( varFxElemDef->elemType != 12
                 && varFxElemDef->elemType != 10
                 && varFxElemDef->elemType != 8
                 && varFxElemDef->elemType != 9 )
    {
        varMaterialHandle = (Material **)varFxElemVisuals;
        Mark_MaterialHandle();
    }
}

void __cdecl Mark_FxElemVisualsArray(int count)
{
    FxElemVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxElemVisuals;
    for ( i = 0; i < count; ++i )
    {
        varFxElemVisuals = var;
        Mark_FxElemVisuals();
        ++var;
    }
}

void __cdecl Mark_FxElemDefVisuals()
{
    if ( varFxElemDef->elemType == 11 )
    {
        if ( varFxElemDefVisuals->markArray )
        {
            varFxElemMarkVisuals = varFxElemDefVisuals->markArray;
            Mark_FxElemMarkVisualsArray(varFxElemDef->visualCount);
        }
    }
    else if ( varFxElemDef->visualCount > 1u )
    {
        if ( varFxElemDefVisuals->markArray )
        {
            varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals->markArray;
            Mark_FxElemVisualsArray(varFxElemDef->visualCount);
        }
    }
    else
    {
        varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals;
        Mark_FxElemVisuals();
    }
}

void __cdecl Mark_FxElemDef()
{
    varFxElemDefVisuals = &varFxElemDef->visuals;
    Mark_FxElemDefVisuals();
}

void __cdecl Mark_FxElemDefArray(int count)
{
    FxElemDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxElemDef;
    for ( i = 0; i < count; ++i )
    {
        varFxElemDef = var;
        Mark_FxElemDef();
        ++var;
    }
}

void __cdecl Mark_FxEffectDef()
{
    if ( varFxEffectDef->elemDefsEA )
    {
        varFxElemDef = (FxElemDef*)varFxEffectDef->elemDefsEA;
        Mark_FxElemDefArray(varFxEffectDef->elemDefCountEmission + varFxEffectDef->elemDefCountOneShot
                                                                                                                         + varFxEffectDef->elemDefCountLooping);
    }
}

void __cdecl Load_DynEntityDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityDef, 84);
    varXModelPtr = &varDynEntityDef->xModel;
    Load_XModelPtr(0);
    varXModelPtr = &varDynEntityDef->destroyedxModel;
    Load_XModelPtr(0);
    varFxEffectDefHandle = &varDynEntityDef->destroyFx;
    Load_FxEffectDefHandle(0);
    varXModelPiecesPtr = &varDynEntityDef->destroyPieces;
    Load_XModelPiecesPtr(0);
    varPhysPresetPtr = &varDynEntityDef->physPreset;
    Load_PhysPresetPtr(0);
    varScriptString = &varDynEntityDef->targetname;
    Load_ScriptString(0);
    varScriptString = &varDynEntityDef->target;
    Load_ScriptString(0);
}

void __cdecl Load_DynEntityDefArray(bool atStreamStart, int count)
{
    DynEntityDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityDef, 84 * count);
    var = varDynEntityDef;
    for ( i = 0; i < count; ++i )
    {
        varDynEntityDef = var;
        Load_DynEntityDef(0);
        ++var;
    }
}

void __cdecl Load_DynEntityPoseArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityPose, 32 * count);
}

void __cdecl Load_DynEntityCollArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityColl, 32 * count);
}

void __cdecl Load_DynEntityClientArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityClient, 20 * count);
}

void __cdecl Load_DynEntityServerArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityServer, 8 * count);
}

void __cdecl Mark_DynEntityDef()
{
    varXModelPtr = &varDynEntityDef->xModel;
    Mark_XModelPtr();
    varXModelPtr = &varDynEntityDef->destroyedxModel;
    Mark_XModelPtr();
    varFxEffectDefHandle = &varDynEntityDef->destroyFx;
    Mark_FxEffectDefHandle();
    varXModelPiecesPtr = &varDynEntityDef->destroyPieces;
    Mark_XModelPiecesPtr();
    varPhysPresetPtr = &varDynEntityDef->physPreset;
    Mark_PhysPresetPtr();
    varScriptString = &varDynEntityDef->targetname;
    Mark_ScriptString();
    varScriptString = &varDynEntityDef->target;
    Mark_ScriptString();
}

void __cdecl Mark_DynEntityDefArray(int count)
{
    DynEntityDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varDynEntityDef;
    for ( i = 0; i < count; ++i )
    {
        varDynEntityDef = var;
        Mark_DynEntityDef();
        ++var;
    }
}

void __cdecl Load_MapEnts(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMapEnts, 12);
    DB_PushStreamPos(4u);
    varXString = &varMapEnts->name;
    Load_XString(0);
    if ( varMapEnts->entityString )
    {
        varMapEnts->entityString = (char *)AllocLoad_raw_byte();
        varchar = varMapEnts->entityString;
        Load_charArray(1, varMapEnts->numEntityChars);
    }
    DB_PopStreamPos();
}

void __cdecl Load_MapEntsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMapEntsPtr, 4);
    DB_PushStreamPos(0);
    if ( *varMapEntsPtr )
    {
        value = (unsigned int)*varMapEntsPtr;
        if ( *varMapEntsPtr == (MapEnts *)-1 || value == -2 )
        {
            *varMapEntsPtr = (MapEnts *)AllocLoad_FxElemVisStateSample();
            varMapEnts = *varMapEntsPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_MapEnts(1);
            Load_MapEntsAsset((XAssetHeader *)varMapEntsPtr);
            if ( inserted )
                *inserted = *varMapEntsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMapEntsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_MapEntsPtr()
{
    if ( *varMapEntsPtr )
    {
        varMapEnts = *varMapEntsPtr;
        Mark_MapEntsAsset(varMapEnts);
    }
}

void __cdecl Load_cStaticModel_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcStaticModel_t, 80);
    varXModelPtr = &varcStaticModel_t->xmodel;
    Load_XModelPtr(0);
}

void __cdecl Load_cStaticModel_tArray(bool atStreamStart, int count)
{
    cStaticModel_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcStaticModel_t, 80 * count);
    var = varcStaticModel_t;
    for ( i = 0; i < count; ++i )
    {
        varcStaticModel_t = var;
        Load_cStaticModel_t(0);
        ++var;
    }
}

void __cdecl Load_cNode_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcNode_t, 8);
    if ( varcNode_t->plane )
    {
        if ( varcNode_t->plane == (cplane_s *)-1 )
        {
            varcNode_t->plane = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varcNode_t->plane;
            Load_cplane_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varcNode_t);
        }
    }
}

void __cdecl Load_cNode_tArray(bool atStreamStart, int count)
{
    cNode_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcNode_t, 8 * count);
    var = varcNode_t;
    for ( i = 0; i < count; ++i )
    {
        varcNode_t = var;
        Load_cNode_t(0);
        ++var;
    }
}

void __cdecl Load_cLeaf_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcLeaf_t, 44 * count);
}

void __cdecl Load_cLeafBrushNodeLeaf_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcLeafBrushNodeLeaf_t, 4);
    if ( varcLeafBrushNodeLeaf_t->brushes )
    {
        if ( varcLeafBrushNodeLeaf_t->brushes == (unsigned __int16 *)-1 )
        {
            varcLeafBrushNodeLeaf_t->brushes = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varLeafBrush = varcLeafBrushNodeLeaf_t->brushes;
            Load_LeafBrushArray(1, varcLeafBrushNode_t->leafBrushCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varcLeafBrushNodeLeaf_t);
        }
    }
}

void __cdecl Load_cLeafBrushNodeChildren_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcLeafBrushNodeChildren_t, 12);
}

void __cdecl Load_cLeafBrushNodeData_t(bool atStreamStart)
{
    if ( varcLeafBrushNode_t->leafBrushCount <= 0 )
    {
        if ( atStreamStart )
        {
            varcLeafBrushNodeChildren_t = (cLeafBrushNodeChildren_t *)varcLeafBrushNodeData_t;
            Load_cLeafBrushNodeChildren_t(atStreamStart);
        }
    }
    else
    {
        varcLeafBrushNodeLeaf_t = &varcLeafBrushNodeData_t->leaf;
        Load_cLeafBrushNodeLeaf_t(atStreamStart);
    }
}

void __cdecl Load_cLeafBrushNode_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varcLeafBrushNode_t->axis, 20);
    varcLeafBrushNodeData_t = &varcLeafBrushNode_t->data;
    Load_cLeafBrushNodeData_t(0);
}

void __cdecl Load_cLeafBrushNode_tArray(bool atStreamStart, int count)
{
    cLeafBrushNode_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varcLeafBrushNode_t->axis, 20 * count);
    var = varcLeafBrushNode_t;
    for ( i = 0; i < count; ++i )
    {
        varcLeafBrushNode_t = var;
        Load_cLeafBrushNode_t(0);
        ++var;
    }
}

void __cdecl Load_CollisionBorder(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollisionBorder, 28);
}

void __cdecl Load_CollisionBorderArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollisionBorder, 28 * count);
}

void __cdecl Load_CollisionPartition(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varCollisionPartition->triCount, 20);
    if ( varCollisionPartition->borders )
    {
        if ( varCollisionPartition->borders == (CollisionBorder *)-1 )
        {
            varCollisionPartition->borders = (CollisionBorder *)AllocLoad_FxElemVisStateSample();
            varCollisionBorder = varCollisionPartition->borders;
            Load_CollisionBorder(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varCollisionPartition->borders);
        }
    }
}

void __cdecl Load_CollisionPartitionArray(bool atStreamStart, int count)
{
    CollisionPartition *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varCollisionPartition->triCount, 20 * count);
    var = varCollisionPartition;
    for ( i = 0; i < count; ++i )
    {
        varCollisionPartition = var;
        Load_CollisionPartition(0);
        ++var;
    }
}

cbrush_t *__cdecl AllocLoad_GfxPackedVertex0()
{
    return (cbrush_t *)DB_AllocStreamPos(15);
}

void __cdecl Load_CollisionAabbTreeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollisionAabbTree, 32 * count);
}

void __cdecl Load_cmodel_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcmodel_t, 72 * count);
}

void __cdecl Load_cbrush_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrush_t, 96);
    if ( varcbrush_t->sides )
    {
        if ( varcbrush_t->sides == (cbrushside_t *)-1 )
        {
            varcbrush_t->sides = (cbrushside_t *)AllocLoad_FxElemVisStateSample();
            varcbrushside_t = varcbrush_t->sides;
            Load_cbrushside_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varcbrush_t->sides);
        }
    }
    if ( varcbrush_t->verts )
    {
        if ( varcbrush_t->verts == (float (*)[3])-1 )
        {
            varcbrush_t->verts = (float (*)[3])AllocLoad_FxElemVisStateSample();
            varshared_vec3_t = varcbrush_t->verts;
            Load_shared_vec3_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varcbrush_t->verts);
        }
    }
}

void __cdecl Load_cbrush_tArray(bool atStreamStart, int count)
{
    cbrush_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrush_t, 96 * count);
    var = varcbrush_t;
    for ( i = 0; i < count; ++i )
    {
        varcbrush_t = var;
        Load_cbrush_t(0);
        ++var;
    }
}

void __cdecl Load_LeafBrushArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varLeafBrush, 2 * count);
}

void __cdecl Load_rope_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varrope_t, 3188 * count);
}

void __cdecl Load_clipMap_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varclipMap_t, 332);
    DB_PushStreamPos(4u);
    varXString = &varclipMap_t->name;
    Load_XString(0);
    if ( varclipMap_t->planes )
    {
        if ( varclipMap_t->planes == (cplane_s *)-1 )
        {
            varclipMap_t->planes = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varclipMap_t->planes;
            Load_cplane_tArray(1, varclipMap_t->planeCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varclipMap_t->planes);
        }
    }
    if ( varclipMap_t->staticModelList )
    {
        varclipMap_t->staticModelList = (cStaticModel_s *)AllocLoad_FxElemVisStateSample();
        varcStaticModel_t = varclipMap_t->staticModelList;
        Load_cStaticModel_tArray(1, varclipMap_t->numStaticModels);
    }
    if ( varclipMap_t->materials )
    {
        varclipMap_t->materials = (dmaterial_t *)AllocLoad_FxElemVisStateSample();
        vardmaterial_t = varclipMap_t->materials;
        Load_dmaterial_tArray(1, varclipMap_t->numMaterials);
    }
    if ( varclipMap_t->brushsides )
    {
        varclipMap_t->brushsides = (cbrushside_t *)AllocLoad_FxElemVisStateSample();
        varcbrushside_t = varclipMap_t->brushsides;
        Load_cbrushside_tArray(1, varclipMap_t->numBrushSides);
    }
    if ( varclipMap_t->nodes )
    {
        varclipMap_t->nodes = (cNode_t *)AllocLoad_FxElemVisStateSample();
        varcNode_t = varclipMap_t->nodes;
        Load_cNode_tArray(1, varclipMap_t->numNodes);
    }
    if ( varclipMap_t->leafs )
    {
        varclipMap_t->leafs = (cLeaf_s *)AllocLoad_FxElemVisStateSample();
        varcLeaf_t = varclipMap_t->leafs;
        Load_cLeaf_tArray(1, varclipMap_t->numLeafs);
    }
    if ( varclipMap_t->leafbrushes )
    {
        varclipMap_t->leafbrushes = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varLeafBrush = varclipMap_t->leafbrushes;
        Load_LeafBrushArray(1, varclipMap_t->numLeafBrushes);
    }
    if ( varclipMap_t->leafbrushNodes )
    {
        varclipMap_t->leafbrushNodes = (cLeafBrushNode_s *)AllocLoad_FxElemVisStateSample();
        varcLeafBrushNode_t = varclipMap_t->leafbrushNodes;
        Load_cLeafBrushNode_tArray(1, varclipMap_t->leafbrushNodesCount);
    }
    if ( varclipMap_t->leafsurfaces )
    {
        varclipMap_t->leafsurfaces = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varuint = varclipMap_t->leafsurfaces;
        Load_uintArray(1, varclipMap_t->numLeafSurfaces);
    }
    if ( varclipMap_t->verts )
    {
        varclipMap_t->verts = (float (*)[3])AllocLoad_FxElemVisStateSample();
        varvec3_t = varclipMap_t->verts;
        Load_vec3_tArray(1, varclipMap_t->vertCount);
    }
    if ( varclipMap_t->brushVerts )
    {
        varclipMap_t->brushVerts = (float (*)[3])AllocLoad_FxElemVisStateSample();
        varshared_vec3_t = varclipMap_t->brushVerts;
        Load_shared_vec3_tArray(1, varclipMap_t->numBrushVerts);
    }
    if ( varclipMap_t->uinds )
    {
        varclipMap_t->uinds = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varclipMap_t->uinds;
        Load_UnsignedShortArray(1, varclipMap_t->nuinds);
    }
    if ( varclipMap_t->triIndices )
    {
        varclipMap_t->triIndices = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varclipMap_t->triIndices;
        Load_UnsignedShortArray(1, 3 * varclipMap_t->triCount);
    }
    if ( varclipMap_t->triEdgeIsWalkable )
    {
        varclipMap_t->triEdgeIsWalkable = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varclipMap_t->triEdgeIsWalkable;
        Load_byteArray(1, 4 * ((3 * varclipMap_t->triCount + 31) >> 5));
    }
    if ( varclipMap_t->borders )
    {
        varclipMap_t->borders = (CollisionBorder *)AllocLoad_FxElemVisStateSample();
        varCollisionBorder = varclipMap_t->borders;
        Load_CollisionBorderArray(1, varclipMap_t->borderCount);
    }
    if ( varclipMap_t->partitions )
    {
        varclipMap_t->partitions = (CollisionPartition *)AllocLoad_FxElemVisStateSample();
        varCollisionPartition = varclipMap_t->partitions;
        Load_CollisionPartitionArray(1, varclipMap_t->partitionCount);
    }
    if ( varclipMap_t->aabbTrees )
    {
        varclipMap_t->aabbTrees = (CollisionAabbTree *)AllocLoad_GfxPackedVertex0();
        varCollisionAabbTree = varclipMap_t->aabbTrees;
        Load_CollisionAabbTreeArray(1, varclipMap_t->aabbTreeCount);
    }
    if ( varclipMap_t->cmodels )
    {
        varclipMap_t->cmodels = (cmodel_t *)AllocLoad_FxElemVisStateSample();
        varcmodel_t = varclipMap_t->cmodels;
        Load_cmodel_tArray(1, varclipMap_t->numSubModels);
    }
    if ( varclipMap_t->brushes )
    {
        varclipMap_t->brushes = AllocLoad_GfxPackedVertex0();
        varcbrush_t = varclipMap_t->brushes;
        Load_cbrush_tArray(1, varclipMap_t->numBrushes);
    }
    if ( varclipMap_t->visibility )
    {
        varclipMap_t->visibility = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varclipMap_t->visibility;
        Load_byteArray(1, varclipMap_t->numClusters * varclipMap_t->clusterBytes);
    }
    varMapEntsPtr = &varclipMap_t->mapEnts;
    Load_MapEntsPtr(0);
    if ( varclipMap_t->box_brush )
    {
        if ( varclipMap_t->box_brush == (cbrush_t *)-1 )
        {
            varclipMap_t->box_brush = AllocLoad_GfxPackedVertex0();
            varcbrush_t = varclipMap_t->box_brush;
            Load_cbrush_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varclipMap_t->box_brush);
        }
    }
    if ( varclipMap_t->dynEntDefList[0] )
    {
        varclipMap_t->dynEntDefList[0] = (DynEntityDef *)AllocLoad_FxElemVisStateSample();
        varDynEntityDef = varclipMap_t->dynEntDefList[0];
        Load_DynEntityDefArray(1, varclipMap_t->dynEntCount[0]);
    }
    if ( varclipMap_t->dynEntDefList[1] )
    {
        varclipMap_t->dynEntDefList[1] = (DynEntityDef *)AllocLoad_FxElemVisStateSample();
        varDynEntityDef = varclipMap_t->dynEntDefList[1];
        Load_DynEntityDefArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntPoseList[0] )
    {
        varclipMap_t->dynEntPoseList[0] = (DynEntityPose *)AllocLoad_FxElemVisStateSample();
        varDynEntityPose = varclipMap_t->dynEntPoseList[0];
        Load_DynEntityPoseArray(1, varclipMap_t->dynEntCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntPoseList[1] )
    {
        varclipMap_t->dynEntPoseList[1] = (DynEntityPose *)AllocLoad_FxElemVisStateSample();
        varDynEntityPose = varclipMap_t->dynEntPoseList[1];
        Load_DynEntityPoseArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntClientList[0] )
    {
        varclipMap_t->dynEntClientList[0] = (DynEntityClient *)AllocLoad_FxElemVisStateSample();
        varDynEntityClient = varclipMap_t->dynEntClientList[0];
        Load_DynEntityClientArray(1, varclipMap_t->dynEntCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntClientList[1] )
    {
        varclipMap_t->dynEntClientList[1] = (DynEntityClient *)AllocLoad_FxElemVisStateSample();
        varDynEntityClient = varclipMap_t->dynEntClientList[1];
        Load_DynEntityClientArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntServerList[0] )
    {
        varclipMap_t->dynEntServerList[0] = (DynEntityServer *)AllocLoad_FxElemVisStateSample();
        varDynEntityServer = varclipMap_t->dynEntServerList[0];
        Load_DynEntityServerArray(1, varclipMap_t->dynEntCount[2]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntServerList[1] )
    {
        varclipMap_t->dynEntServerList[1] = (DynEntityServer *)AllocLoad_FxElemVisStateSample();
        varDynEntityServer = varclipMap_t->dynEntServerList[1];
        Load_DynEntityServerArray(1, varclipMap_t->dynEntCount[3]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntCollList[0] )
    {
        varclipMap_t->dynEntCollList[0] = (DynEntityColl *)AllocLoad_FxElemVisStateSample();
        varDynEntityColl = varclipMap_t->dynEntCollList[0];
        Load_DynEntityCollArray(1, varclipMap_t->dynEntCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntCollList[1] )
    {
        varclipMap_t->dynEntCollList[1] = (DynEntityColl *)AllocLoad_FxElemVisStateSample();
        varDynEntityColl = varclipMap_t->dynEntCollList[1];
        Load_DynEntityCollArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntCollList[2] )
    {
        varclipMap_t->dynEntCollList[2] = (DynEntityColl *)AllocLoad_FxElemVisStateSample();
        varDynEntityColl = varclipMap_t->dynEntCollList[2];
        Load_DynEntityCollArray(1, varclipMap_t->dynEntCount[2]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varclipMap_t->dynEntCollList[3] )
    {
        varclipMap_t->dynEntCollList[3] = (DynEntityColl *)AllocLoad_FxElemVisStateSample();
        varDynEntityColl = varclipMap_t->dynEntCollList[3];
        Load_DynEntityCollArray(1, varclipMap_t->dynEntCount[3]);
    }
    DB_PopStreamPos();
    if ( varclipMap_t->constraints )
    {
        varclipMap_t->constraints = (PhysConstraint *)AllocLoad_FxElemVisStateSample();
        varPhysConstraint = varclipMap_t->constraints;
        Load_PhysConstraintArray(1, varclipMap_t->num_constraints);
    }
    DB_PushStreamPos(1u);
    if ( varclipMap_t->ropes )
    {
        varclipMap_t->ropes = (rope_t *)AllocLoad_FxElemVisStateSample();
        varrope_t = varclipMap_t->ropes;
        Load_rope_tArray(1, varclipMap_t->max_ropes);
    }
    DB_PopStreamPos();
    DB_PopStreamPos();
}

void __cdecl Load_clipMap_ptr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varclipMap_ptr, 4);
    DB_PushStreamPos(0);
    if ( *varclipMap_ptr )
    {
        value = (unsigned int)*varclipMap_ptr;
        if ( *varclipMap_ptr == (clipMap_t *)-1 || value == -2 )
        {
            *varclipMap_ptr = (clipMap_t *)AllocLoad_FxElemVisStateSample();
            varclipMap_t = *varclipMap_ptr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_clipMap_t(1);
            Load_ClipMapAsset((XAssetHeader *)varclipMap_ptr);
            if ( inserted )
                *inserted = *varclipMap_ptr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varclipMap_ptr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_cStaticModel_t()
{
    varXModelPtr = &varcStaticModel_t->xmodel;
    Mark_XModelPtr();
}

void __cdecl Mark_cStaticModel_tArray(int count)
{
    cStaticModel_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varcStaticModel_t;
    for ( i = 0; i < count; ++i )
    {
        varcStaticModel_t = var;
        Mark_cStaticModel_t();
        ++var;
    }
}

void __cdecl Mark_clipMap_t()
{
    if ( varclipMap_t->staticModelList )
    {
        varcStaticModel_t = varclipMap_t->staticModelList;
        Mark_cStaticModel_tArray(varclipMap_t->numStaticModels);
    }
    varMapEntsPtr = &varclipMap_t->mapEnts;
    Mark_MapEntsPtr();
    if ( varclipMap_t->dynEntDefList[0] )
    {
        varDynEntityDef = varclipMap_t->dynEntDefList[0];
        Mark_DynEntityDefArray(varclipMap_t->dynEntCount[0]);
    }
    if ( varclipMap_t->dynEntDefList[1] )
    {
        varDynEntityDef = varclipMap_t->dynEntDefList[1];
        Mark_DynEntityDefArray(varclipMap_t->dynEntCount[1]);
    }
    if ( varclipMap_t->constraints )
    {
        varPhysConstraint = varclipMap_t->constraints;
        Mark_PhysConstraintArray(varclipMap_t->num_constraints);
    }
}

void __cdecl Mark_clipMap_ptr()
{
    if ( *varclipMap_ptr )
    {
        varclipMap_t = *varclipMap_ptr;
        Mark_ClipMapAsset(varclipMap_t);
        Mark_clipMap_t();
    }
}

void __cdecl Load_ComPrimaryLight(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varComPrimaryLight->type, 220);
    varXString = &varComPrimaryLight->defName;
    Load_XString(0);
}

void __cdecl Load_ComPrimaryLightArray(bool atStreamStart, int count)
{
    ComPrimaryLight *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varComPrimaryLight->type, 220 * count);
    var = varComPrimaryLight;
    for ( i = 0; i < count; ++i )
    {
        varComPrimaryLight = var;
        Load_ComPrimaryLight(0);
        ++var;
    }
}

void __cdecl Load_ComWaterCellArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varComWaterCell, 8 * count);
}

void __cdecl Load_ComBurnableSampleArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, &varComBurnableSample->state, count);
}

void __cdecl Load_ComBurnableCell(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varComBurnableCell, 12);
    if ( varComBurnableCell->data )
    {
        varComBurnableCell->data = (ComBurnableSample*)AllocLoad_raw_byte();
        varComBurnableSample = varComBurnableCell->data;
        Load_ComBurnableSampleArray(1, 32);
    }
}

void __cdecl Load_ComBurnableCellArray(bool atStreamStart, int count)
{
    ComBurnableCell *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varComBurnableCell, 12 * count);
    var = varComBurnableCell;
    for ( i = 0; i < count; ++i )
    {
        varComBurnableCell = var;
        Load_ComBurnableCell(0);
        ++var;
    }
}

void __cdecl Load_ComWorld(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varComWorld, 64);
    DB_PushStreamPos(4u);
    varXString = &varComWorld->name;
    Load_XString(0);
    if ( varComWorld->primaryLights )
    {
        varComWorld->primaryLights = (ComPrimaryLight *)AllocLoad_FxElemVisStateSample();
        varComPrimaryLight = varComWorld->primaryLights;
        Load_ComPrimaryLightArray(1, varComWorld->primaryLightCount);
    }
    if ( varComWorld->waterCells )
    {
        varComWorld->waterCells = (ComWaterCell *)AllocLoad_FxElemVisStateSample();
        varComWaterCell = varComWorld->waterCells;
        Load_ComWaterCellArray(1, varComWorld->numWaterCells);
    }
    if ( varComWorld->burnableCells )
    {
        varComWorld->burnableCells = (ComBurnableCell *)AllocLoad_FxElemVisStateSample();
        varComBurnableCell = varComWorld->burnableCells;
        Load_ComBurnableCellArray(1, varComWorld->numBurnableCells);
    }
    DB_PopStreamPos();
}

void __cdecl Load_ComWorldPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varComWorldPtr, 4);
    DB_PushStreamPos(0);
    if ( *varComWorldPtr )
    {
        value = (unsigned int)*varComWorldPtr;
        if ( *varComWorldPtr == (ComWorld *)-1 || value == -2 )
        {
            *varComWorldPtr = (ComWorld *)AllocLoad_FxElemVisStateSample();
            varComWorld = *varComWorldPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_ComWorld(1);
            Load_ComWorldAsset((XAssetHeader *)varComWorldPtr);
            if ( inserted )
                *inserted = *varComWorldPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varComWorldPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_ComWorldPtr()
{
    if ( *varComWorldPtr )
    {
        varComWorld = *varComWorldPtr;
        Mark_ComWorldAsset(varComWorld);
    }
}

void __cdecl Load_operandInternalDataUnion(bool atStreamStart)
{
    if ( varOperand->dataType )
    {
        if ( varOperand->dataType == VAL_FLOAT )
        {
            if ( atStreamStart )
            {
                varfloat = &varoperandInternalDataUnion->floatVal;
                Load_float(atStreamStart);
            }
        }
        else if ( varOperand->dataType == VAL_STRING )
        {
            varXString = (const char **)varoperandInternalDataUnion;
            Load_XString(atStreamStart);
        }
    }
    else if ( atStreamStart )
    {
        varint = varoperandInternalDataUnion;
        Load_int(atStreamStart);
    }
}

void __cdecl Load_Operand(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varOperand, 8);
    varoperandInternalDataUnion = &varOperand->internals;
    Load_operandInternalDataUnion(0);
}

void __cdecl Load_expressionRpnDataUnion(bool atStreamStart)
{
    if ( varexpressionRpn->type )
    {
        if ( varexpressionRpn->type == 1 && atStreamStart )
        {
            varint = varexpressionRpnDataUnion;
            Load_int(atStreamStart);
        }
    }
    else
    {
        varOperand = &varexpressionRpnDataUnion->constant;
        Load_Operand(atStreamStart);
    }
}

void __cdecl Load_expressionRpn(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varexpressionRpn, 12);
    varexpressionRpnDataUnion = &varexpressionRpn->data;
    Load_expressionRpnDataUnion(0);
}

void __cdecl Load_expressionRpnArray(bool atStreamStart, int count)
{
    expressionRpn *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varexpressionRpn, 12 * count);
    var = varexpressionRpn;
    for ( i = 0; i < count; ++i )
    {
        varexpressionRpn = var;
        Load_expressionRpn(0);
        ++var;
    }
}

void __cdecl Load_ExpressionStatement(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varExpressionStatement, 16);
    varXString = (const char **)&varExpressionStatement->filename;
    Load_XString(0);
    if ( varExpressionStatement->rpn )
    {
        if ( varExpressionStatement->rpn == (expressionRpn *)-1 )
        {
            varExpressionStatement->rpn = (expressionRpn *)AllocLoad_FxElemVisStateSample();
            varexpressionRpn = varExpressionStatement->rpn;
            Load_expressionRpnArray(1, varExpressionStatement->numRpn);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varExpressionStatement->rpn);
        }
    }
}

void __cdecl Load_ScriptCondition(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptCondition, 16);
    if ( varScriptCondition->next )
    {
        varScriptCondition->next = (ScriptCondition *)AllocLoad_FxElemVisStateSample();
        varScriptConditionNext = varScriptCondition->next;
        Load_ScriptConditionNext(1);
    }
}

void __cdecl Load_ScriptConditionNext(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptConditionNext, 16);
    varScriptCondition = varScriptConditionNext;
    Load_ScriptCondition(0);
}

void __cdecl Load_GenericEventScript(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGenericEventScript, 44);
    if ( varGenericEventScript->prerequisites )
    {
        varGenericEventScript->prerequisites = (ScriptCondition *)AllocLoad_FxElemVisStateSample();
        varScriptCondition = varGenericEventScript->prerequisites;
        Load_ScriptCondition(1);
    }
    varExpressionStatement = &varGenericEventScript->condition;
    Load_ExpressionStatement(0);
    varXString = &varGenericEventScript->action;
    Load_XString(0);
    if ( varGenericEventScript->next )
    {
        varGenericEventScript->next = (GenericEventScript *)AllocLoad_FxElemVisStateSample();
        varGenericEventScriptNext = varGenericEventScript->next;
        Load_GenericEventScriptNext(1);
    }
}

void __cdecl Load_GenericEventScriptNext(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGenericEventScriptNext, 44);
    varGenericEventScript = varGenericEventScriptNext;
    Load_GenericEventScript(0);
}

void __cdecl Load_GenericEventHandler(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGenericEventHandler, 12);
    varXString = &varGenericEventHandler->name;
    Load_XString(0);
    if ( varGenericEventHandler->eventScript )
    {
        varGenericEventHandler->eventScript = (GenericEventScript *)AllocLoad_FxElemVisStateSample();
        varGenericEventScript = varGenericEventHandler->eventScript;
        Load_GenericEventScript(1);
    }
    if ( varGenericEventHandler->next )
    {
        varGenericEventHandler->next = (GenericEventHandler *)AllocLoad_FxElemVisStateSample();
        varGenericEventHandlerNext = varGenericEventHandler->next;
        Load_GenericEventHandlerNext(1);
    }
}

void __cdecl Load_GenericEventHandlerNext(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGenericEventHandlerNext, 12);
    varGenericEventHandler = varGenericEventHandlerNext;
    Load_GenericEventHandler(0);
}

void __cdecl Load_ItemKeyHandler(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varItemKeyHandler, 12);
    if ( varItemKeyHandler->keyScript )
    {
        varItemKeyHandler->keyScript = (GenericEventScript *)AllocLoad_FxElemVisStateSample();
        varGenericEventScript = varItemKeyHandler->keyScript;
        Load_GenericEventScript(1);
    }
    if ( varItemKeyHandler->next )
    {
        varItemKeyHandler->next = (ItemKeyHandler *)AllocLoad_FxElemVisStateSample();
        varItemKeyHandlerNext = varItemKeyHandler->next;
        Load_ItemKeyHandlerNext(1);
    }
}

void __cdecl Load_ItemKeyHandlerNext(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varItemKeyHandlerNext, 12);
    varItemKeyHandler = varItemKeyHandlerNext;
    Load_ItemKeyHandler(0);
}

void __cdecl Load_editFieldDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vareditFieldDef_t, 36);
}

void __cdecl Load_editFieldDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vareditFieldDef_ptr, 4);
    if ( *vareditFieldDef_ptr )
    {
        *vareditFieldDef_ptr = (editFieldDef_s *)AllocLoad_FxElemVisStateSample();
        vareditFieldDef_t = *vareditFieldDef_ptr;
        Load_editFieldDef_t(1);
    }
}

void __cdecl Load_multiDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varmultiDef_t, 396);
    varXString = (const char **)varmultiDef_t;
    Load_XStringArray(0, 32);
    varXString = varmultiDef_t->dvarStr;
    Load_XStringArray(0, 32);
}

void __cdecl Load_multiDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varmultiDef_ptr, 4);
    if ( *varmultiDef_ptr )
    {
        *varmultiDef_ptr = (multiDef_s *)AllocLoad_FxElemVisStateSample();
        varmultiDef_t = *varmultiDef_ptr;
        Load_multiDef_t(1);
    }
}

void __cdecl Load_enumDvarDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varenumDvarDef_t, 4);
    varXString = &varenumDvarDef_t->enumDvarName;
    Load_XString(0);
}

void __cdecl Load_enumDvarDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varenumDvarDef_ptr, 4);
    if ( *varenumDvarDef_ptr )
    {
        *varenumDvarDef_ptr = (enumDvarDef_s *)AllocLoad_FxElemVisStateSample();
        varenumDvarDef_t = *varenumDvarDef_ptr;
        Load_enumDvarDef_t(1);
    }
}

void __cdecl Load_MenuCell(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuCell, 12);
    if ( varMenuCell->stringValue )
    {
        varMenuCell->stringValue = (char *)AllocLoad_raw_byte();
        varchar = varMenuCell->stringValue;
        Load_charArray(1, varMenuCell->maxChars);
    }
}

void __cdecl Load_MenuCellArray(bool atStreamStart, int count)
{
    MenuCell *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuCell, 12 * count);
    var = varMenuCell;
    for ( i = 0; i < count; ++i )
    {
        varMenuCell = var;
        Load_MenuCell(0);
        ++var;
    }
}

void __cdecl Load_MenuRow(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuRow, 24);
    if ( varMenuRow->cells )
    {
        varMenuRow->cells = (MenuCell *)AllocLoad_FxElemVisStateSample();
        varMenuCell = varMenuRow->cells;
        Load_MenuCellArray(1, varlistBoxDef_t->numColumns);
    }
    if ( varMenuRow->eventName )
    {
        varMenuRow->eventName = (char *)AllocLoad_raw_byte();
        varchar = varMenuRow->eventName;
        Load_charArray(1, 32);
    }
    if ( varMenuRow->onFocusEventName )
    {
        varMenuRow->onFocusEventName = (char *)AllocLoad_raw_byte();
        varchar = varMenuRow->onFocusEventName;
        Load_charArray(1, 32);
    }
}

void __cdecl Load_MenuRowArray(bool atStreamStart, int count)
{
    MenuRow *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuRow, 24 * count);
    var = varMenuRow;
    for ( i = 0; i < count; ++i )
    {
        varMenuRow = var;
        Load_MenuRow(0);
        ++var;
    }
}

void __cdecl Load_animParamsDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varanimParamsDef_t, 108);
    varXString = &varanimParamsDef_t->name;
    Load_XString(0);
    if ( varanimParamsDef_t->onEvent )
    {
        varanimParamsDef_t->onEvent = (GenericEventHandler *)AllocLoad_FxElemVisStateSample();
        varGenericEventHandler = varanimParamsDef_t->onEvent;
        Load_GenericEventHandler(1);
    }
}

void __cdecl Load_windowDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varwindowDef_t, 164);
    varXString = &varwindowDef_t->name;
    Load_XString(0);
    varXString = &varwindowDef_t->group;
    Load_XString(0);
    varMaterialHandle = &varwindowDef_t->background;
    Load_MaterialHandle(0);
}

void __cdecl Load_Window(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varWindow, 164);
    varwindowDef_t = varWindow;
    Load_windowDef_t(0);
}

void __cdecl Load_animParamsDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varanimParamsDef_ptr, 4);
    if ( *varanimParamsDef_ptr )
    {
        *varanimParamsDef_ptr = (animParamsDef_t *)AllocLoad_FxElemVisStateSample();
        varanimParamsDef_t = *varanimParamsDef_ptr;
        Load_animParamsDef_t(1);
    }
}

void __cdecl Load_animParamsDef_ptrArray(bool atStreamStart, int count)
{
    animParamsDef_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varanimParamsDef_ptr, 4 * count);
    var = varanimParamsDef_ptr;
    for ( i = 0; i < count; ++i )
    {
        varanimParamsDef_ptr = var;
        Load_animParamsDef_ptr(0);
        ++var;
    }
}

void __cdecl Load_imageDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varimageDef_t, 16);
    varExpressionStatement = &varimageDef_t->materialExp;
    Load_ExpressionStatement(0);
}

void __cdecl Load_imageDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varimageDef_ptr, 4);
    if ( *varimageDef_ptr )
    {
        *varimageDef_ptr = (imageDef_s *)AllocLoad_FxElemVisStateSample();
        varimageDef_t = *varimageDef_ptr;
        Load_imageDef_t(1);
    }
}

void __cdecl Load_ownerDrawDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varownerDrawDef_t, 16);
    varExpressionStatement = &varownerDrawDef_t->dataExp;
    Load_ExpressionStatement(0);
}

void __cdecl Load_ownerDrawDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varownerDrawDef_ptr, 4);
    if ( *varownerDrawDef_ptr )
    {
        *varownerDrawDef_ptr = (ownerDrawDef_s *)AllocLoad_FxElemVisStateSample();
        varownerDrawDef_t = *varownerDrawDef_ptr;
        Load_ownerDrawDef_t(1);
    }
}

void __cdecl Load_listBoxDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varlistBoxDef_t, 668);
    varMaterialHandle = &varlistBoxDef_t->selectIcon;
    Load_MaterialHandle(0);
    varMaterialHandle = &varlistBoxDef_t->backgroundItemListbox;
    Load_MaterialHandle(0);
    varMaterialHandle = &varlistBoxDef_t->highlightTexture;
    Load_MaterialHandle(0);
    if ( varlistBoxDef_t->rows )
    {
        varlistBoxDef_t->rows = (MenuRow *)AllocLoad_FxElemVisStateSample();
        varMenuRow = varlistBoxDef_t->rows;
        Load_MenuRowArray(1, varlistBoxDef_t->maxRows);
    }
}

void __cdecl Load_listBoxDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varlistBoxDef_ptr, 4);
    if ( *varlistBoxDef_ptr )
    {
        *varlistBoxDef_ptr = (listBoxDef_s *)AllocLoad_FxElemVisStateSample();
        varlistBoxDef_t = *varlistBoxDef_ptr;
        Load_listBoxDef_t(1);
    }
}

void __cdecl Load_focusDefData_t(bool atStreamStart)
{
    switch ( varitemDef_t->type )
    {
        case 4:
            varlistBoxDef_ptr = &varfocusDefData_t->listBox;
            Load_listBoxDef_ptr(atStreamStart);
            break;
        case 0xA:
            varmultiDef_ptr = (multiDef_s **)varfocusDefData_t;
            Load_multiDef_ptr(atStreamStart);
            break;
        case 5:
        case 0xD:
        case 7:
        case 0xE:
        case 0x1E:
        case 9:
        case 0xC:
        case 0x10:
        case 8:
        case 0x16:
            vareditFieldDef_ptr = (editFieldDef_s **)varfocusDefData_t;
            Load_editFieldDef_ptr(atStreamStart);
            break;
        case 0xB:
            varenumDvarDef_ptr = (enumDvarDef_s **)varfocusDefData_t;
            Load_enumDvarDef_ptr(atStreamStart);
            break;
    }
}

void __cdecl Load_focusItemDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varfocusItemDef_t, 24);
    varXString = &varfocusItemDef_t->mouseEnterText;
    Load_XString(0);
    varXString = &varfocusItemDef_t->mouseExitText;
    Load_XString(0);
    varXString = &varfocusItemDef_t->mouseEnter;
    Load_XString(0);
    varXString = &varfocusItemDef_t->mouseExit;
    Load_XString(0);
    if ( varfocusItemDef_t->onKey )
    {
        varfocusItemDef_t->onKey = (ItemKeyHandler *)AllocLoad_FxElemVisStateSample();
        varItemKeyHandler = varfocusItemDef_t->onKey;
        Load_ItemKeyHandler(1);
    }
    varfocusDefData_t = &varfocusItemDef_t->focusTypeData;
    Load_focusDefData_t(0);
}

void __cdecl Load_focusItemDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varfocusItemDef_ptr, 4);
    if ( *varfocusItemDef_ptr )
    {
        *varfocusItemDef_ptr = (focusItemDef_s *)AllocLoad_FxElemVisStateSample();
        varfocusItemDef_t = *varfocusItemDef_ptr;
        Load_focusItemDef_t(1);
    }
}

void __cdecl Load_gameMsgDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vargameMsgDef_t, 8);
}

void __cdecl Load_gameMsgDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vargameMsgDef_ptr, 4);
    if ( *vargameMsgDef_ptr )
    {
        *vargameMsgDef_ptr = (gameMsgDef_s *)AllocLoad_FxElemVisStateSample();
        vargameMsgDef_t = *vargameMsgDef_ptr;
        Load_gameMsgDef_t(1);
    }
}

void __cdecl Load_textDefData_t(bool atStreamStart)
{
    if ( varitemDef_t->type == 3
        || varitemDef_t->type == 4
        || varitemDef_t->type == 21
        || varitemDef_t->type == 20
        || varitemDef_t->type == 10
        || varitemDef_t->type == 5
        || varitemDef_t->type == 13
        || varitemDef_t->type == 7
        || varitemDef_t->type == 14
        || varitemDef_t->type == 30
        || varitemDef_t->type == 9
        || varitemDef_t->type == 12
        || varitemDef_t->type == 16
        || varitemDef_t->type == 8
        || varitemDef_t->type == 11
        || varitemDef_t->type == 22 )
    {
        varfocusItemDef_ptr = &vartextDefData_t->focusItemDef;
        Load_focusItemDef_ptr(atStreamStart);
    }
    else if ( varitemDef_t->type == 15 )
    {
        vargameMsgDef_ptr = (gameMsgDef_s **)vartextDefData_t;
        Load_gameMsgDef_ptr(atStreamStart);
    }
}

void __cdecl Load_textExp_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vartextExp_t, 16);
    varExpressionStatement = &vartextExp_t->textExp;
    Load_ExpressionStatement(0);
}

void __cdecl Load_textDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vartextDef_t, 68);
    varXString = &vartextDef_t->text;
    Load_XString(0);
    if ( vartextDef_t->textExpData )
    {
        vartextDef_t->textExpData = (textExp_s *)AllocLoad_FxElemVisStateSample();
        vartextExp_t = vartextDef_t->textExpData;
        Load_textExp_t(1);
    }
    vartextDefData_t = &vartextDef_t->textTypeData;
    Load_textDefData_t(0);
}

void __cdecl Load_textDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vartextDef_ptr, 4);
    if ( *vartextDef_ptr )
    {
        *vartextDef_ptr = (textDef_s *)AllocLoad_FxElemVisStateSample();
        vartextDef_t = *vartextDef_ptr;
        Load_textDef_t(1);
    }
}

void __cdecl Load_rectData_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varrectData_t, 64);
    varExpressionStatement = &varrectData_t->rectXExp;
    Load_ExpressionStatement(0);
    varExpressionStatement = &varrectData_t->rectYExp;
    Load_ExpressionStatement(0);
    varExpressionStatement = &varrectData_t->rectWExp;
    Load_ExpressionStatement(0);
    varExpressionStatement = &varrectData_t->rectHExp;
    Load_ExpressionStatement(0);
}

void __cdecl Load_itemDefData_t(bool atStreamStart)
{
    switch ( varitemDef_t->type )
    {
        case 1:
        case 3:
        case 0xF:
        case 0x12:
        case 0x14:
        case 4:
        case 0xA:
        case 5:
        case 0xE:
        case 7:
        case 0xD:
        case 9:
        case 0xC:
        case 0x10:
        case 8:
        case 0xB:
        case 0x16:
            vartextDef_ptr = &varitemDefData_t->textDef;
            Load_textDef_ptr(atStreamStart);
            break;
        case 2:
            varimageDef_ptr = (imageDef_s **)varitemDefData_t;
            Load_imageDef_ptr(atStreamStart);
            break;
        case 0x15:
        case 0x13:
            varfocusItemDef_ptr = (focusItemDef_s **)varitemDefData_t;
            Load_focusItemDef_ptr(atStreamStart);
            break;
        case 6:
            varownerDrawDef_ptr = (ownerDrawDef_s **)varitemDefData_t;
            Load_ownerDrawDef_ptr(atStreamStart);
            break;
    }
}

void __cdecl Load_UIAnimInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varUIAnimInfo, 236);
    if ( varUIAnimInfo->animStates )
    {
        varUIAnimInfo->animStates = (animParamsDef_t **)AllocLoad_FxElemVisStateSample();
        varanimParamsDef_ptr = varUIAnimInfo->animStates;
        Load_animParamsDef_ptrArray(1, varUIAnimInfo->animStateCount);
    }
}

void __cdecl Load_itemDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varitemDef_t, 272);
    varWindow = &varitemDef_t->window;
    Load_Window(0);
    varXString = &varitemDef_t->dvar;
    Load_XString(0);
    varXString = &varitemDef_t->dvarTest;
    Load_XString(0);
    varXString = &varitemDef_t->enableDvar;
    Load_XString(0);
    varitemDefData_t = &varitemDef_t->typeData;
    Load_itemDefData_t(0);
    if ( varitemDef_t->rectExpData )
    {
        varitemDef_t->rectExpData = (rectData_s *)AllocLoad_FxElemVisStateSample();
        varrectData_t = varitemDef_t->rectExpData;
        Load_rectData_t(1);
    }
    varExpressionStatement = &varitemDef_t->visibleExp;
    Load_ExpressionStatement(0);
    varExpressionStatement = &varitemDef_t->forecolorAExp;
    Load_ExpressionStatement(0);
    if ( varitemDef_t->onEvent )
    {
        varitemDef_t->onEvent = (GenericEventHandler *)AllocLoad_FxElemVisStateSample();
        varGenericEventHandler = varitemDef_t->onEvent;
        Load_GenericEventHandler(1);
    }
    if ( varitemDef_t->animInfo )
    {
        varitemDef_t->animInfo = (UIAnimInfo *)AllocLoad_FxElemVisStateSample();
        varUIAnimInfo = varitemDef_t->animInfo;
        Load_UIAnimInfo(1);
    }
}

menuDef_t *__cdecl AllocLoad_itemDef_t()
{
    return (menuDef_t *)DB_AllocStreamPos(7);
}

void __cdecl Load_itemDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varitemDef_ptr, 4);
    if ( *varitemDef_ptr )
    {
        *varitemDef_ptr = (itemDef_s *)AllocLoad_itemDef_t();
        varitemDef_t = *varitemDef_ptr;
        Load_itemDef_t(1);
    }
}

void __cdecl Load_itemDef_ptrArray(bool atStreamStart, int count)
{
    itemDef_s **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varitemDef_ptr, 4 * count);
    var = varitemDef_ptr;
    for ( i = 0; i < count; ++i )
    {
        varitemDef_ptr = var;
        Load_itemDef_ptr(0);
        ++var;
    }
}

void __cdecl Load_menuDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varmenuDef_t, 400);
    DB_PushStreamPos(4u);
    varWindow = &varmenuDef_t->window;
    Load_Window(0);
    varXString = &varmenuDef_t->font;
    Load_XString(0);
    if ( varmenuDef_t->onEvent )
    {
        varmenuDef_t->onEvent = (GenericEventHandler *)AllocLoad_FxElemVisStateSample();
        varGenericEventHandler = varmenuDef_t->onEvent;
        Load_GenericEventHandler(1);
    }
    if ( varmenuDef_t->onKey )
    {
        varmenuDef_t->onKey = (ItemKeyHandler *)AllocLoad_FxElemVisStateSample();
        varItemKeyHandler = varmenuDef_t->onKey;
        Load_ItemKeyHandler(1);
    }
    varExpressionStatement = &varmenuDef_t->visibleExp;
    Load_ExpressionStatement(0);
    varXString = &varmenuDef_t->allowedBinding;
    Load_XString(0);
    varXString = &varmenuDef_t->soundName;
    Load_XString(0);
    varExpressionStatement = &varmenuDef_t->rectXExp;
    Load_ExpressionStatement(0);
    varExpressionStatement = &varmenuDef_t->rectYExp;
    Load_ExpressionStatement(0);
    if ( varmenuDef_t->items )
    {
        varmenuDef_t->items = (itemDef_s **)AllocLoad_FxElemVisStateSample();
        varitemDef_ptr = varmenuDef_t->items;
        Load_itemDef_ptrArray(1, varmenuDef_t->itemCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_menuDef_ptr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varmenuDef_ptr, 4);
    DB_PushStreamPos(0);
    if ( *varmenuDef_ptr )
    {
        value = (unsigned int)*varmenuDef_ptr;
        if ( *varmenuDef_ptr == (menuDef_t *)-1 || value == -2 )
        {
            *varmenuDef_ptr = AllocLoad_itemDef_t();
            varmenuDef_t = *varmenuDef_ptr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_menuDef_t(1);
            Load_MenuAsset((XAssetHeader *)varmenuDef_ptr);
            if ( inserted )
                *inserted = *varmenuDef_ptr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varmenuDef_ptr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_menuDef_ptrArray(bool atStreamStart, int count)
{
    menuDef_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varmenuDef_ptr, 4 * count);
    var = varmenuDef_ptr;
    for ( i = 0; i < count; ++i )
    {
        varmenuDef_ptr = var;
        Load_menuDef_ptr(0);
        ++var;
    }
}

void __cdecl Load_MenuList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuList, 12);
    DB_PushStreamPos(4u);
    varXString = &varMenuList->name;
    Load_XString(0);
    if ( varMenuList->menus )
    {
        varMenuList->menus = (menuDef_t **)AllocLoad_FxElemVisStateSample();
        varmenuDef_ptr = varMenuList->menus;
        Load_menuDef_ptrArray(1, varMenuList->menuCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_MenuListPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuListPtr, 4);
    DB_PushStreamPos(0);
    if ( *varMenuListPtr )
    {
        value = (unsigned int)*varMenuListPtr;
        if ( *varMenuListPtr == (MenuList *)-1 || value == -2 )
        {
            *varMenuListPtr = (MenuList *)AllocLoad_FxElemVisStateSample();
            varMenuList = *varMenuListPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_MenuList(1);
            Load_MenuListAsset((XAssetHeader *)varMenuListPtr);
            if ( inserted )
                *inserted = *varMenuListPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMenuListPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_windowDef_t()
{
    varMaterialHandle = &varwindowDef_t->background;
    Mark_MaterialHandle();
}

void __cdecl Mark_Window()
{
    varwindowDef_t = varWindow;
    Mark_windowDef_t();
}

void __cdecl Mark_listBoxDef_t()
{
    varMaterialHandle = &varlistBoxDef_t->selectIcon;
    Mark_MaterialHandle();
    varMaterialHandle = &varlistBoxDef_t->backgroundItemListbox;
    Mark_MaterialHandle();
    varMaterialHandle = &varlistBoxDef_t->highlightTexture;
    Mark_MaterialHandle();
}

void __cdecl Mark_listBoxDef_ptr()
{
    if ( *varlistBoxDef_ptr )
    {
        varlistBoxDef_t = *varlistBoxDef_ptr;
        Mark_listBoxDef_t();
    }
}

void __cdecl Mark_focusDefData_t()
{
    if ( varitemDef_t->type == 4 )
    {
        varlistBoxDef_ptr = &varfocusDefData_t->listBox;
        Mark_listBoxDef_ptr();
    }
}

void __cdecl Mark_focusItemDef_t()
{
    varfocusDefData_t = &varfocusItemDef_t->focusTypeData;
    Mark_focusDefData_t();
}

void __cdecl Mark_focusItemDef_ptr()
{
    if ( *varfocusItemDef_ptr )
    {
        varfocusItemDef_t = *varfocusItemDef_ptr;
        Mark_focusItemDef_t();
    }
}

void __cdecl Mark_textDefData_t()
{
    if ( varitemDef_t->type == 3
        || varitemDef_t->type == 4
        || varitemDef_t->type == 21
        || varitemDef_t->type == 20
        || varitemDef_t->type == 10
        || varitemDef_t->type == 5
        || varitemDef_t->type == 13
        || varitemDef_t->type == 7
        || varitemDef_t->type == 14
        || varitemDef_t->type == 30
        || varitemDef_t->type == 9
        || varitemDef_t->type == 12
        || varitemDef_t->type == 16
        || varitemDef_t->type == 8
        || varitemDef_t->type == 11
        || varitemDef_t->type == 22 )
    {
        varfocusItemDef_ptr = &vartextDefData_t->focusItemDef;
        Mark_focusItemDef_ptr();
    }
}

void __cdecl Mark_textDef_t()
{
    vartextDefData_t = &vartextDef_t->textTypeData;
    Mark_textDefData_t();
}

void __cdecl Mark_textDef_ptr()
{
    if ( *vartextDef_ptr )
    {
        vartextDef_t = *vartextDef_ptr;
        Mark_textDef_t();
    }
}

void __cdecl Mark_itemDefData_t()
{
    if ( varitemDef_t->type == 1
        || varitemDef_t->type == 3
        || varitemDef_t->type == 15
        || varitemDef_t->type == 18
        || varitemDef_t->type == 20
        || varitemDef_t->type == 4
        || varitemDef_t->type == 10
        || varitemDef_t->type == 5
        || varitemDef_t->type == 14
        || varitemDef_t->type == 7
        || varitemDef_t->type == 13
        || varitemDef_t->type == 9
        || varitemDef_t->type == 12
        || varitemDef_t->type == 16
        || varitemDef_t->type == 8
        || varitemDef_t->type == 11
        || varitemDef_t->type == 22 )
    {
        vartextDef_ptr = &varitemDefData_t->textDef;
        Mark_textDef_ptr();
    }
    else if ( varitemDef_t->type != 2 && (varitemDef_t->type == 21 || varitemDef_t->type == 19) )
    {
        varfocusItemDef_ptr = (focusItemDef_s **)varitemDefData_t;
        Mark_focusItemDef_ptr();
    }
}

void __cdecl Mark_itemDef_t()
{
    varWindow = &varitemDef_t->window;
    Mark_Window();
    varitemDefData_t = &varitemDef_t->typeData;
    Mark_itemDefData_t();
}

void __cdecl Mark_itemDef_ptr()
{
    if ( *varitemDef_ptr )
    {
        varitemDef_t = *varitemDef_ptr;
        Mark_itemDef_t();
    }
}

void __cdecl Mark_itemDef_ptrArray(int count)
{
    itemDef_s **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varitemDef_ptr;
    for ( i = 0; i < count; ++i )
    {
        varitemDef_ptr = var;
        Mark_itemDef_ptr();
        ++var;
    }
}

void __cdecl Mark_menuDef_t()
{
    varWindow = &varmenuDef_t->window;
    Mark_Window();
    if ( varmenuDef_t->items )
    {
        varitemDef_ptr = varmenuDef_t->items;
        Mark_itemDef_ptrArray(varmenuDef_t->itemCount);
    }
}

void __cdecl Mark_menuDef_ptr()
{
    if ( *varmenuDef_ptr )
    {
        varmenuDef_t = *varmenuDef_ptr;
        Mark_MenuAsset(varmenuDef_t);
        Mark_menuDef_t();
    }
}

void __cdecl Mark_menuDef_ptrArray(int count)
{
    menuDef_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varmenuDef_ptr;
    for ( i = 0; i < count; ++i )
    {
        varmenuDef_ptr = var;
        Mark_menuDef_ptr();
        ++var;
    }
}

void __cdecl Mark_MenuList()
{
    if ( varMenuList->menus )
    {
        varmenuDef_ptr = varMenuList->menus;
        Mark_menuDef_ptrArray(varMenuList->menuCount);
    }
}

void __cdecl Mark_MenuListPtr()
{
    if ( *varMenuListPtr )
    {
        varMenuList = *varMenuListPtr;
        Mark_MenuListAsset(varMenuList);
        Mark_MenuList();
    }
}

void __cdecl Load_LocalizeEntry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varLocalizeEntry, 8);
    DB_PushStreamPos(4u);
    varXString = &varLocalizeEntry->value;
    Load_XString(0);
    varXString = &varLocalizeEntry->name;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_LocalizeEntryPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varLocalizeEntryPtr, 4);
    DB_PushStreamPos(0);
    if ( *varLocalizeEntryPtr )
    {
        value = (unsigned int)*varLocalizeEntryPtr;
        if ( *varLocalizeEntryPtr == (LocalizeEntry *)-1 || value == -2 )
        {
            *varLocalizeEntryPtr = (LocalizeEntry *)AllocLoad_FxElemVisStateSample();
            varLocalizeEntry = *varLocalizeEntryPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_LocalizeEntry(1);
            Load_LocalizeEntryAsset((XAssetHeader *)varLocalizeEntryPtr);
            if ( inserted )
                *inserted = *varLocalizeEntryPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varLocalizeEntryPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_LocalizeEntryPtr()
{
    if ( *varLocalizeEntryPtr )
    {
        varLocalizeEntry = *varLocalizeEntryPtr;
        Mark_LocalizeEntryAsset(varLocalizeEntry);
    }
}

void __cdecl Load_FxImpactEntry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactEntry, 140);
    varFxEffectDefHandle = (const FxEffectDef **)varFxImpactEntry;
    Load_FxEffectDefHandleArray(0, 31);
    varFxEffectDefHandle = varFxImpactEntry->flesh;
    Load_FxEffectDefHandleArray(0, 4);
}

void __cdecl Load_FxImpactEntryArray(bool atStreamStart, int count)
{
    FxImpactEntry *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactEntry, 140 * count);
    var = varFxImpactEntry;
    for ( i = 0; i < count; ++i )
    {
        varFxImpactEntry = var;
        Load_FxImpactEntry(0);
        ++var;
    }
}

void __cdecl Load_FxImpactTable(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactTable, 8);
    DB_PushStreamPos(4u);
    varXString = &varFxImpactTable->name;
    Load_XString(0);
    if ( varFxImpactTable->table )
    {
        varFxImpactTable->table = (FxImpactEntry *)AllocLoad_FxElemVisStateSample();
        varFxImpactEntry = varFxImpactTable->table;
        Load_FxImpactEntryArray(1, 21);
    }
    DB_PopStreamPos();
}

void __cdecl Load_FxImpactTablePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactTablePtr, 4);
    DB_PushStreamPos(0);
    if ( *varFxImpactTablePtr )
    {
        value = (unsigned int)*varFxImpactTablePtr;
        if ( *varFxImpactTablePtr == (FxImpactTable *)-1 || value == -2 )
        {
            *varFxImpactTablePtr = (FxImpactTable *)AllocLoad_FxElemVisStateSample();
            varFxImpactTable = *varFxImpactTablePtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_FxImpactTable(1);
            Load_FxImpactTableAsset((XAssetHeader *)varFxImpactTablePtr);
            if ( inserted )
                *inserted = *varFxImpactTablePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varFxImpactTablePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_FxImpactEntry()
{
    varFxEffectDefHandle = (const FxEffectDef **)varFxImpactEntry;
    Mark_FxEffectDefHandleArray(31);
    varFxEffectDefHandle = varFxImpactEntry->flesh;
    Mark_FxEffectDefHandleArray(4);
}

void __cdecl Mark_FxImpactEntryArray(int count)
{
    FxImpactEntry *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxImpactEntry;
    for ( i = 0; i < count; ++i )
    {
        varFxImpactEntry = var;
        Mark_FxImpactEntry();
        ++var;
    }
}

void __cdecl Mark_FxImpactTable()
{
    if ( varFxImpactTable->table )
    {
        varFxImpactEntry = varFxImpactTable->table;
        Mark_FxImpactEntryArray(21);
    }
}

void __cdecl Mark_FxImpactTablePtr()
{
    if ( *varFxImpactTablePtr )
    {
        varFxImpactTable = *varFxImpactTablePtr;
        Mark_FxImpactTableAsset(varFxImpactTable);
        Mark_FxImpactTable();
    }
}

void __cdecl Load_DestructibleStage(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDestructibleStage, 48);
    varScriptString = &varDestructibleStage->showBone;
    Load_ScriptString(0);
    varFxEffectDefHandle = &varDestructibleStage->breakEffect;
    Load_FxEffectDefHandle(0);
    varXString = &varDestructibleStage->breakSound;
    Load_XString(0);
    varXString = &varDestructibleStage->breakNotify;
    Load_XString(0);
    varXString = &varDestructibleStage->loopSound;
    Load_XString(0);
    varXModelPtr = varDestructibleStage->spawnModel;
    Load_XModelPtrArray(0, 3);
    varPhysPresetPtr = &varDestructibleStage->physPreset;
    Load_PhysPresetPtr(0);
}

void __cdecl Load_DestructibleStageArray(bool atStreamStart, int count)
{
    DestructibleStage *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varDestructibleStage, 48 * count);
    var = varDestructibleStage;
    for ( i = 0; i < count; ++i )
    {
        varDestructibleStage = var;
        Load_DestructibleStage(0);
        ++var;
    }
}

void __cdecl Load_DestructiblePiece(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDestructiblePiece, 312);
    varDestructibleStage = (DestructibleStage *)varDestructiblePiece;
    Load_DestructibleStageArray(0, 5);
    varPhysConstraintsPtr = &varDestructiblePiece->physConstraints;
    Load_PhysConstraintsPtr(0);
    varXString = &varDestructiblePiece->damageSound;
    Load_XString(0);
    varFxEffectDefHandle = &varDestructiblePiece->burnEffect;
    Load_FxEffectDefHandle(0);
    varXString = &varDestructiblePiece->burnSound;
    Load_XString(0);
    varScriptString = &varDestructiblePiece->enableLabel;
    Load_ScriptString(0);
}

void __cdecl Load_DestructiblePieceArray(bool atStreamStart, int count)
{
    DestructiblePiece *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varDestructiblePiece, 312 * count);
    var = varDestructiblePiece;
    for ( i = 0; i < count; ++i )
    {
        varDestructiblePiece = var;
        Load_DestructiblePiece(0);
        ++var;
    }
}

void __cdecl Load_DestructibleDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDestructibleDef, 24);
    DB_PushStreamPos(4u);
    varXString = &varDestructibleDef->name;
    Load_XString(0);
    varXModelPtr = &varDestructibleDef->model;
    Load_XModelPtr(0);
    varXModelPtr = &varDestructibleDef->pristineModel;
    Load_XModelPtr(0);
    if ( varDestructibleDef->pieces )
    {
        varDestructibleDef->pieces = (DestructiblePiece *)AllocLoad_FxElemVisStateSample();
        varDestructiblePiece = varDestructibleDef->pieces;
        Load_DestructiblePieceArray(1, varDestructibleDef->numPieces);
    }
    DB_PopStreamPos();
}

void __cdecl Load_DestructibleDefPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varDestructibleDefPtr, 4);
    DB_PushStreamPos(0);
    if ( *varDestructibleDefPtr )
    {
        value = (unsigned int)*varDestructibleDefPtr;
        if ( *varDestructibleDefPtr == (DestructibleDef *)-1 || value == -2 )
        {
            *varDestructibleDefPtr = (DestructibleDef *)AllocLoad_FxElemVisStateSample();
            varDestructibleDef = *varDestructibleDefPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_DestructibleDef(1);
            Load_DestructibleDefAsset((XAssetHeader *)varDestructibleDefPtr);
            if ( inserted )
                *inserted = *varDestructibleDefPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varDestructibleDefPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_DestructibleStage()
{
    varScriptString = &varDestructibleStage->showBone;
    Mark_ScriptString();
    varFxEffectDefHandle = &varDestructibleStage->breakEffect;
    Mark_FxEffectDefHandle();
    varXModelPtr = varDestructibleStage->spawnModel;
    Mark_XModelPtrArray(3);
    varPhysPresetPtr = &varDestructibleStage->physPreset;
    Mark_PhysPresetPtr();
}

void __cdecl Mark_DestructibleStageArray(int count)
{
    DestructibleStage *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varDestructibleStage;
    for ( i = 0; i < count; ++i )
    {
        varDestructibleStage = var;
        Mark_DestructibleStage();
        ++var;
    }
}

void __cdecl Mark_DestructiblePiece()
{
    varDestructibleStage = (DestructibleStage *)varDestructiblePiece;
    Mark_DestructibleStageArray(5);
    varPhysConstraintsPtr = &varDestructiblePiece->physConstraints;
    Mark_PhysConstraintsPtr();
    varFxEffectDefHandle = &varDestructiblePiece->burnEffect;
    Mark_FxEffectDefHandle();
    varScriptString = &varDestructiblePiece->enableLabel;
    Mark_ScriptString();
}

void __cdecl Mark_DestructiblePieceArray(int count)
{
    DestructiblePiece *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varDestructiblePiece;
    for ( i = 0; i < count; ++i )
    {
        varDestructiblePiece = var;
        Mark_DestructiblePiece();
        ++var;
    }
}

void __cdecl Mark_DestructibleDef()
{
    varXModelPtr = &varDestructibleDef->model;
    Mark_XModelPtr();
    varXModelPtr = &varDestructibleDef->pristineModel;
    Mark_XModelPtr();
    if ( varDestructibleDef->pieces )
    {
        varDestructiblePiece = varDestructibleDef->pieces;
        Mark_DestructiblePieceArray(varDestructibleDef->numPieces);
    }
}

void __cdecl Mark_DestructibleDefPtr()
{
    if ( *varDestructibleDefPtr )
    {
        varDestructibleDef = *varDestructibleDefPtr;
        Mark_DestructibleDefAsset(varDestructibleDef);
        Mark_DestructibleDef();
    }
}

void __cdecl Load_flameTable(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varflameTable, 476);
    varXString = &varflameTable->name;
    Load_XString(0);
    varMaterialHandle = &varflameTable->fire;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->smoke;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->heat;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->drips;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->streamFuel;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->streamFuel2;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->streamFlame;
    Load_MaterialHandle(0);
    varMaterialHandle = &varflameTable->streamFlame2;
    Load_MaterialHandle(0);
    varXString = &varflameTable->flameOffLoopSound;
    Load_XString(0);
    varXString = &varflameTable->flameIgniteSound;
    Load_XString(0);
    varXString = &varflameTable->flameOnLoopSound;
    Load_XString(0);
    varXString = &varflameTable->flameCooldownSound;
    Load_XString(0);
}

XAsset *__cdecl AllocLoad_FxElemVisStateSample()
{
    return (XAsset *)DB_AllocStreamPos(3);
}

void __cdecl Load_WeaponDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varWeaponDef, 2056);
    varXString = &varWeaponDef->szOverlayName;
    Load_XString(0);
    if ( varWeaponDef->gunXModel )
    {
        if ( varWeaponDef->gunXModel == (XModel **)-1 )
        {
            varWeaponDef->gunXModel = (XModel **)AllocLoad_FxElemVisStateSample();
            varXModelPtr = varWeaponDef->gunXModel;
            Load_XModelPtrArray(1, 16);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->gunXModel);
        }
    }
    varXModelPtr = &varWeaponDef->handXModel;
    Load_XModelPtr(0);
    varXString = &varWeaponDef->szModeName;
    Load_XString(0);
    if ( varWeaponDef->notetrackSoundMapKeys )
    {
        if ( varWeaponDef->notetrackSoundMapKeys == (unsigned __int16 *)-1 )
        {
            varWeaponDef->notetrackSoundMapKeys = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varScriptString = varWeaponDef->notetrackSoundMapKeys;
            Load_ScriptStringArray(1, 20);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->notetrackSoundMapKeys);
        }
    }
    if ( varWeaponDef->notetrackSoundMapValues )
    {
        if ( varWeaponDef->notetrackSoundMapValues == (unsigned __int16 *)-1 )
        {
            varWeaponDef->notetrackSoundMapValues = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varScriptString = varWeaponDef->notetrackSoundMapValues;
            Load_ScriptStringArray(1, 20);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->notetrackSoundMapValues);
        }
    }
    varXString = &varWeaponDef->parentWeaponName;
    Load_XString(0);
    varFxEffectDefHandle = &varWeaponDef->viewFlashEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->worldFlashEffect;
    Load_FxEffectDefHandle(0);
    varXString = &varWeaponDef->pickupSound;
    Load_XString(0);
    varXString = &varWeaponDef->pickupSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->ammoPickupSound;
    Load_XString(0);
    varXString = &varWeaponDef->ammoPickupSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->projectileSound;
    Load_XString(0);
    varXString = &varWeaponDef->pullbackSound;
    Load_XString(0);
    varXString = &varWeaponDef->pullbackSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->fireSound;
    Load_XString(0);
    varXString = &varWeaponDef->fireSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->fireLoopSound;
    Load_XString(0);
    varXString = &varWeaponDef->fireLoopSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->fireLoopEndSound;
    Load_XString(0);
    varXString = &varWeaponDef->fireLoopEndSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->fireStopSound;
    Load_XString(0);
    varXString = &varWeaponDef->fireStopSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->fireLastSound;
    Load_XString(0);
    varXString = &varWeaponDef->fireLastSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->emptyFireSound;
    Load_XString(0);
    varXString = &varWeaponDef->emptyFireSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->crackSound;
    Load_XString(0);
    varXString = &varWeaponDef->whizbySound;
    Load_XString(0);
    varXString = &varWeaponDef->meleeSwipeSound;
    Load_XString(0);
    varXString = &varWeaponDef->meleeSwipeSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->meleeHitSound;
    Load_XString(0);
    varXString = &varWeaponDef->meleeMissSound;
    Load_XString(0);
    varXString = &varWeaponDef->rechamberSound;
    Load_XString(0);
    varXString = &varWeaponDef->rechamberSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->reloadSound;
    Load_XString(0);
    varXString = &varWeaponDef->reloadSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->reloadEmptySound;
    Load_XString(0);
    varXString = &varWeaponDef->reloadEmptySoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->reloadStartSound;
    Load_XString(0);
    varXString = &varWeaponDef->reloadStartSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->reloadEndSound;
    Load_XString(0);
    varXString = &varWeaponDef->reloadEndSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->rotateLoopSound;
    Load_XString(0);
    varXString = &varWeaponDef->rotateLoopSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->deploySound;
    Load_XString(0);
    varXString = &varWeaponDef->deploySoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->finishDeploySound;
    Load_XString(0);
    varXString = &varWeaponDef->finishDeploySoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->breakdownSound;
    Load_XString(0);
    varXString = &varWeaponDef->breakdownSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->finishBreakdownSound;
    Load_XString(0);
    varXString = &varWeaponDef->finishBreakdownSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->detonateSound;
    Load_XString(0);
    varXString = &varWeaponDef->detonateSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->nightVisionWearSound;
    Load_XString(0);
    varXString = &varWeaponDef->nightVisionWearSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->nightVisionRemoveSound;
    Load_XString(0);
    varXString = &varWeaponDef->nightVisionRemoveSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->altSwitchSound;
    Load_XString(0);
    varXString = &varWeaponDef->altSwitchSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->raiseSound;
    Load_XString(0);
    varXString = &varWeaponDef->raiseSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->firstRaiseSound;
    Load_XString(0);
    varXString = &varWeaponDef->firstRaiseSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->putawaySound;
    Load_XString(0);
    varXString = &varWeaponDef->putawaySoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->overheatSound;
    Load_XString(0);
    varXString = &varWeaponDef->overheatSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->adsZoomSound;
    Load_XString(0);
    if ( varWeaponDef->bounceSound )
    {
        if ( varWeaponDef->bounceSound == (const char **)-1 )
        {
            varWeaponDef->bounceSound = (const char **)AllocLoad_FxElemVisStateSample();
            varXString = varWeaponDef->bounceSound;
            Load_XStringArray(1, 31);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->bounceSound);
        }
    }
    varXString = &varWeaponDef->standMountedWeapdef;
    Load_XString(0);
    varXString = &varWeaponDef->crouchMountedWeapdef;
    Load_XString(0);
    varXString = &varWeaponDef->proneMountedWeapdef;
    Load_XString(0);
    varFxEffectDefHandle = &varWeaponDef->viewShellEjectEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->worldShellEjectEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->viewLastShotEjectEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->worldLastShotEjectEffect;
    Load_FxEffectDefHandle(0);
    varMaterialHandle = &varWeaponDef->reticleCenter;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->reticleSide;
    Load_MaterialHandle(0);
    if ( varWeaponDef->worldModel )
    {
        if ( varWeaponDef->worldModel == (XModel **)-1 )
        {
            varWeaponDef->worldModel = (XModel **)AllocLoad_FxElemVisStateSample();
            varXModelPtr = varWeaponDef->worldModel;
            Load_XModelPtrArray(1, 16);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->worldModel);
        }
    }
    varXModelPtr = &varWeaponDef->worldClipModel;
    Load_XModelPtr(0);
    varXModelPtr = &varWeaponDef->rocketModel;
    Load_XModelPtr(0);
    varXModelPtr = &varWeaponDef->mountedModel;
    Load_XModelPtr(0);
    varXModelPtr = &varWeaponDef->additionalMeleeModel;
    Load_XModelPtr(0);
    varMaterialHandle = &varWeaponDef->hudIcon;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->ammoCounterIcon;
    Load_MaterialHandle(0);
    varXString = &varWeaponDef->szSharedAmmoCapName;
    Load_XString(0);
    varScriptString = &varWeaponDef->explosionTag;
    Load_ScriptString(0);
    varXString = &varWeaponDef->spinLoopSound;
    Load_XString(0);
    varXString = &varWeaponDef->spinLoopSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->startSpinSound;
    Load_XString(0);
    varXString = &varWeaponDef->startSpinSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->stopSpinSound;
    Load_XString(0);
    varXString = &varWeaponDef->stopSpinSoundPlayer;
    Load_XString(0);
    varXString = &varWeaponDef->stackSound;
    Load_XString(0);
    varMaterialHandle = &varWeaponDef->killIcon;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->indicatorIcon;
    Load_MaterialHandle(0);
    varXString = &varWeaponDef->szSpawnedGrenadeWeaponName;
    Load_XString(0);
    varXString = &varWeaponDef->szDualWieldWeaponName;
    Load_XString(0);
    varXModelPtr = &varWeaponDef->projectileModel;
    Load_XModelPtr(0);
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect2;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect3;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect4;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect5;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projDudEffect;
    Load_FxEffectDefHandle(0);
    varXString = &varWeaponDef->projExplosionSound;
    Load_XString(0);
    varXString = &varWeaponDef->projDudSound;
    Load_XString(0);
    varXString = &varWeaponDef->mortarShellSound;
    Load_XString(0);
    varXString = &varWeaponDef->tankShellSound;
    Load_XString(0);
    if ( varWeaponDef->parallelBounce )
    {
        if ( varWeaponDef->parallelBounce == (float *)-1 )
        {
            varWeaponDef->parallelBounce = (float *)AllocLoad_FxElemVisStateSample();
            varfloat = varWeaponDef->parallelBounce;
            Load_floatArray(1, 31);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->parallelBounce);
        }
    }
    if ( varWeaponDef->perpendicularBounce )
    {
        if ( varWeaponDef->perpendicularBounce == (float *)-1 )
        {
            varWeaponDef->perpendicularBounce = (float *)AllocLoad_FxElemVisStateSample();
            varfloat = varWeaponDef->perpendicularBounce;
            Load_floatArray(1, 31);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->perpendicularBounce);
        }
    }
    varFxEffectDefHandle = &varWeaponDef->projTrailEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projIgnitionEffect;
    Load_FxEffectDefHandle(0);
    varXString = &varWeaponDef->projIgnitionSound;
    Load_XString(0);
    varXString = varWeaponDef->accuracyGraphName;
    Load_XString(0);
    if ( varWeaponDef->accuracyGraphKnots[0] )
    {
        if ( varWeaponDef->accuracyGraphKnots[0] == (float (*)[2])-1 )
        {
            varWeaponDef->accuracyGraphKnots[0] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->accuracyGraphKnots[0];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[0]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varWeaponDef->accuracyGraphKnots);
        }
    }
    if ( varWeaponDef->originalAccuracyGraphKnots[0] )
    {
        if ( varWeaponDef->originalAccuracyGraphKnots[0] == (float (*)[2])-1 )
        {
            varWeaponDef->originalAccuracyGraphKnots[0] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->originalAccuracyGraphKnots[0];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[0]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varWeaponDef->originalAccuracyGraphKnots);
        }
    }
    varXString = &varWeaponDef->accuracyGraphName[1];
    Load_XString(0);
    if ( varWeaponDef->accuracyGraphKnots[1] )
    {
        if ( varWeaponDef->accuracyGraphKnots[1] == (float (*)[2])-1 )
        {
            varWeaponDef->accuracyGraphKnots[1] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->accuracyGraphKnots[1];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[1]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->accuracyGraphKnots[1]);
        }
    }
    if ( varWeaponDef->originalAccuracyGraphKnots[1] )
    {
        if ( varWeaponDef->originalAccuracyGraphKnots[1] == (float (*)[2])-1 )
        {
            varWeaponDef->originalAccuracyGraphKnots[1] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->originalAccuracyGraphKnots[1];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[1]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->originalAccuracyGraphKnots[1]);
        }
    }
    varXString = &varWeaponDef->szUseHintString;
    Load_XString(0);
    varXString = &varWeaponDef->dropHintString;
    Load_XString(0);
    varXString = &varWeaponDef->szScript;
    Load_XString(0);
    if ( varWeaponDef->locationDamageMultipliers )
    {
        if ( varWeaponDef->locationDamageMultipliers == (float *)-1 )
        {
            varWeaponDef->locationDamageMultipliers = (float *)AllocLoad_FxElemVisStateSample();
            varfloat = varWeaponDef->locationDamageMultipliers;
            Load_floatArray(1, 19);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->locationDamageMultipliers);
        }
    }
    varXString = &varWeaponDef->fireRumble;
    Load_XString(0);
    varXString = &varWeaponDef->meleeImpactRumble;
    Load_XString(0);
    varXString = &varWeaponDef->reloadRumble;
    Load_XString(0);
    varXString = &varWeaponDef->flameTableFirstPerson;
    Load_XString(0);
    varXString = &varWeaponDef->flameTableThirdPerson;
    Load_XString(0);
    if ( varWeaponDef->flameTableFirstPersonPtr )
    {
        if ( varWeaponDef->flameTableFirstPersonPtr == (flameTable *)-1 )
        {
            varWeaponDef->flameTableFirstPersonPtr = (flameTable *)AllocLoad_FxElemVisStateSample();
            varflameTable = varWeaponDef->flameTableFirstPersonPtr;
            Load_flameTable(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->flameTableFirstPersonPtr);
        }
    }
    if ( varWeaponDef->flameTableThirdPersonPtr )
    {
        if ( varWeaponDef->flameTableThirdPersonPtr == (flameTable *)-1 )
        {
            varWeaponDef->flameTableThirdPersonPtr = (flameTable *)AllocLoad_FxElemVisStateSample();
            varflameTable = varWeaponDef->flameTableThirdPersonPtr;
            Load_flameTable(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponDef->flameTableThirdPersonPtr);
        }
    }
    varFxEffectDefHandle = &varWeaponDef->tagFx_preparationEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->tagFlash_preparationEffect;
    Load_FxEffectDefHandle(0);
}

void __cdecl Load_WeaponVariantDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varWeaponVariantDef, 228);
    DB_PushStreamPos(4u);
    varXString = &varWeaponVariantDef->szInternalName;
    Load_XString(0);
    if ( varWeaponVariantDef->weapDef )
    {
        if ( varWeaponVariantDef->weapDef == (WeaponDef *)-1 )
        {
            varWeaponVariantDef->weapDef = (WeaponDef *)AllocLoad_FxElemVisStateSample();
            varWeaponDef = varWeaponVariantDef->weapDef;
            Load_WeaponDef(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponVariantDef->weapDef);
        }
    }
    varXString = &varWeaponVariantDef->szDisplayName;
    Load_XString(0);
    varXString = &varWeaponVariantDef->szAltWeaponName;
    Load_XString(0);
    if ( varWeaponVariantDef->szXAnims )
    {
        if ( varWeaponVariantDef->szXAnims == (const char **)-1 )
        {
            varWeaponVariantDef->szXAnims = (const char **)AllocLoad_FxElemVisStateSample();
            varXString = varWeaponVariantDef->szXAnims;
            Load_XStringArray(1, 66);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponVariantDef->szXAnims);
        }
    }
    if ( varWeaponVariantDef->hideTags )
    {
        if ( varWeaponVariantDef->hideTags == (unsigned __int16 *)-1 )
        {
            varWeaponVariantDef->hideTags = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varScriptString = varWeaponVariantDef->hideTags;
            Load_ScriptStringArray(1, 32);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varWeaponVariantDef->hideTags);
        }
    }
    varXString = &varWeaponVariantDef->szAmmoName;
    Load_XString(0);
    varXString = &varWeaponVariantDef->szClipName;
    Load_XString(0);
    varMaterialHandle = &varWeaponVariantDef->overlayMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponVariantDef->overlayMaterialLowRes;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponVariantDef->dpadIcon;
    Load_MaterialHandle(0);
    DB_PopStreamPos();
}

void __cdecl Load_WeaponVariantDefPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varWeaponVariantDefPtr, 4);
    DB_PushStreamPos(0);
    if ( *varWeaponVariantDefPtr )
    {
        value = (unsigned int)*varWeaponVariantDefPtr;
        if ( *varWeaponVariantDefPtr == (WeaponVariantDef *)-1 || value == -2 )
        {
            *varWeaponVariantDefPtr = (WeaponVariantDef *)AllocLoad_FxElemVisStateSample();
            varWeaponVariantDef = *varWeaponVariantDefPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_WeaponVariantDef(1);
            Load_WeaponVariantDefAsset((XAssetHeader *)varWeaponVariantDefPtr);
            if ( inserted )
                *inserted = *varWeaponVariantDefPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varWeaponVariantDefPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_flameTable()
{
    varMaterialHandle = &varflameTable->fire;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->smoke;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->heat;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->drips;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->streamFuel;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->streamFuel2;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->streamFlame;
    Mark_MaterialHandle();
    varMaterialHandle = &varflameTable->streamFlame2;
    Mark_MaterialHandle();
}

void __cdecl Mark_WeaponDef()
{
    if ( varWeaponDef->gunXModel )
    {
        varXModelPtr = varWeaponDef->gunXModel;
        Mark_XModelPtrArray(16);
    }
    varXModelPtr = &varWeaponDef->handXModel;
    Mark_XModelPtr();
    if ( varWeaponDef->notetrackSoundMapKeys )
    {
        varScriptString = varWeaponDef->notetrackSoundMapKeys;
        Mark_ScriptStringArray(20);
    }
    if ( varWeaponDef->notetrackSoundMapValues )
    {
        varScriptString = varWeaponDef->notetrackSoundMapValues;
        Mark_ScriptStringArray(20);
    }
    varFxEffectDefHandle = &varWeaponDef->viewFlashEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->worldFlashEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->viewShellEjectEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->worldShellEjectEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->viewLastShotEjectEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->worldLastShotEjectEffect;
    Mark_FxEffectDefHandle();
    varMaterialHandle = &varWeaponDef->reticleCenter;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->reticleSide;
    Mark_MaterialHandle();
    if ( varWeaponDef->worldModel )
    {
        varXModelPtr = varWeaponDef->worldModel;
        Mark_XModelPtrArray(16);
    }
    varXModelPtr = &varWeaponDef->worldClipModel;
    Mark_XModelPtr();
    varXModelPtr = &varWeaponDef->rocketModel;
    Mark_XModelPtr();
    varXModelPtr = &varWeaponDef->mountedModel;
    Mark_XModelPtr();
    varXModelPtr = &varWeaponDef->additionalMeleeModel;
    Mark_XModelPtr();
    varMaterialHandle = &varWeaponDef->hudIcon;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->ammoCounterIcon;
    Mark_MaterialHandle();
    varScriptString = &varWeaponDef->explosionTag;
    Mark_ScriptString();
    varMaterialHandle = &varWeaponDef->killIcon;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->indicatorIcon;
    Mark_MaterialHandle();
    varXModelPtr = &varWeaponDef->projectileModel;
    Mark_XModelPtr();
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect2;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect3;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect4;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect5;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projDudEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projTrailEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projIgnitionEffect;
    Mark_FxEffectDefHandle();
    if ( varWeaponDef->flameTableFirstPersonPtr )
    {
        varflameTable = varWeaponDef->flameTableFirstPersonPtr;
        Mark_flameTable();
    }
    if ( varWeaponDef->flameTableThirdPersonPtr )
    {
        varflameTable = varWeaponDef->flameTableThirdPersonPtr;
        Mark_flameTable();
    }
    varFxEffectDefHandle = &varWeaponDef->tagFx_preparationEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->tagFlash_preparationEffect;
    Mark_FxEffectDefHandle();
}

void __cdecl Mark_WeaponVariantDef()
{
    if ( varWeaponVariantDef->weapDef )
    {
        varWeaponDef = varWeaponVariantDef->weapDef;
        Mark_WeaponDef();
    }
    if ( varWeaponVariantDef->hideTags )
    {
        varScriptString = varWeaponVariantDef->hideTags;
        Mark_ScriptStringArray(32);
    }
    varMaterialHandle = &varWeaponVariantDef->overlayMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponVariantDef->overlayMaterialLowRes;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponVariantDef->dpadIcon;
    Mark_MaterialHandle();
}

void __cdecl Mark_WeaponVariantDefPtr()
{
    if ( *varWeaponVariantDefPtr )
    {
        varWeaponVariantDef = *varWeaponVariantDefPtr;
        Mark_WeaponVariantDefAsset(varWeaponVariantDef);
        Mark_WeaponVariantDef();
    }
}

void __cdecl Load_EmblemIcon(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemIcon, 40);
    varGfxImagePtr = &varEmblemIcon->image;
    Load_GfxImagePtr(0);
    varXString = &varEmblemIcon->description;
    Load_XString(0);
}

void __cdecl Load_EmblemIconArray(bool atStreamStart, int count)
{
    EmblemIcon *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemIcon, 40 * count);
    var = varEmblemIcon;
    for ( i = 0; i < count; ++i )
    {
        varEmblemIcon = var;
        Load_EmblemIcon(0);
        ++var;
    }
}

void __cdecl Load_EmblemBackground(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemBackground, 24);
    varMaterialHandle = &varEmblemBackground->material;
    Load_MaterialHandle(0);
    varXString = &varEmblemBackground->description;
    Load_XString(0);
}

void __cdecl Load_EmblemBackgroundArray(bool atStreamStart, int count)
{
    EmblemBackground *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemBackground, 24 * count);
    var = varEmblemBackground;
    for ( i = 0; i < count; ++i )
    {
        varEmblemBackground = var;
        Load_EmblemBackground(0);
        ++var;
    }
}

void __cdecl Load_EmblemLayerArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemLayer, 12 * count);
}

void __cdecl Load_EmblemCategory(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemCategory, 8);
    varXString = &varEmblemCategory->name;
    Load_XString(0);
    varXString = &varEmblemCategory->description;
    Load_XString(0);
}

void __cdecl Load_EmblemCategoryArray(bool atStreamStart, int count)
{
    EmblemCategory *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemCategory, 8 * count);
    var = varEmblemCategory;
    for ( i = 0; i < count; ++i )
    {
        varEmblemCategory = var;
        Load_EmblemCategory(0);
        ++var;
    }
}

void __cdecl Load_EmblemSet(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemSet, 44);
    DB_PushStreamPos(4u);
    if ( varEmblemSet->layers )
    {
        varEmblemSet->layers = (EmblemLayer *)AllocLoad_FxElemVisStateSample();
        varEmblemLayer = varEmblemSet->layers;
        Load_EmblemLayerArray(1, varEmblemSet->layerCount);
    }
    if ( varEmblemSet->categories )
    {
        varEmblemSet->categories = (EmblemCategory *)AllocLoad_FxElemVisStateSample();
        varEmblemCategory = varEmblemSet->categories;
        Load_EmblemCategoryArray(1, varEmblemSet->categoryCount);
    }
    if ( varEmblemSet->icons )
    {
        varEmblemSet->icons = (EmblemIcon *)AllocLoad_FxElemVisStateSample();
        varEmblemIcon = varEmblemSet->icons;
        Load_EmblemIconArray(1, varEmblemSet->iconCount);
    }
    if ( varEmblemSet->backgrounds )
    {
        varEmblemSet->backgrounds = (EmblemBackground *)AllocLoad_FxElemVisStateSample();
        varEmblemBackground = varEmblemSet->backgrounds;
        Load_EmblemBackgroundArray(1, varEmblemSet->backgroundCount);
    }
    if ( varEmblemSet->backgroundLookup )
    {
        varEmblemSet->backgroundLookup = (__int16 *)AllocLoad_XBlendInfo();
        varshort = varEmblemSet->backgroundLookup;
        Load_shortArray(1, varEmblemSet->backgroundLookupCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_EmblemSetPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varEmblemSetPtr, 4);
    DB_PushStreamPos(0);
    if ( *varEmblemSetPtr )
    {
        value = (unsigned int)*varEmblemSetPtr;
        if ( *varEmblemSetPtr == (EmblemSet *)-1 || value == -2 )
        {
            *varEmblemSetPtr = (EmblemSet *)AllocLoad_FxElemVisStateSample();
            varEmblemSet = *varEmblemSetPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_EmblemSet(1);
            Load_EmblemSetAsset((XAssetHeader *)varEmblemSetPtr);
            if ( inserted )
                *inserted = *varEmblemSetPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varEmblemSetPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_EmblemIcon()
{
    varGfxImagePtr = &varEmblemIcon->image;
    Mark_GfxImagePtr();
}

void __cdecl Mark_EmblemIconArray(int count)
{
    EmblemIcon *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varEmblemIcon;
    for ( i = 0; i < count; ++i )
    {
        varEmblemIcon = var;
        Mark_EmblemIcon();
        ++var;
    }
}

void __cdecl Mark_EmblemBackground()
{
    varMaterialHandle = &varEmblemBackground->material;
    Mark_MaterialHandle();
}

void __cdecl Mark_EmblemBackgroundArray(int count)
{
    EmblemBackground *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varEmblemBackground;
    for ( i = 0; i < count; ++i )
    {
        varEmblemBackground = var;
        Mark_EmblemBackground();
        ++var;
    }
}

void __cdecl Mark_EmblemSet()
{
    if ( varEmblemSet->icons )
    {
        varEmblemIcon = varEmblemSet->icons;
        Mark_EmblemIconArray(varEmblemSet->iconCount);
    }
    if ( varEmblemSet->backgrounds )
    {
        varEmblemBackground = varEmblemSet->backgrounds;
        Mark_EmblemBackgroundArray(varEmblemSet->backgroundCount);
    }
}

void __cdecl Mark_EmblemSetPtr()
{
    if ( *varEmblemSetPtr )
    {
        varEmblemSet = *varEmblemSetPtr;
        Mark_EmblemSetAsset(varEmblemSet);
        Mark_EmblemSet();
    }
}

void __cdecl Load_RawFile(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varRawFile, 12);
    DB_PushStreamPos(4u);
    varXString = &varRawFile->name;
    Load_XString(0);
    if ( varRawFile->buffer )
    {
        varRawFile->buffer = (const char *)AllocLoad_raw_byte();
        varConstChar = varRawFile->buffer;
        Load_ConstCharArray(1, varRawFile->len + 1);
    }
    DB_PopStreamPos();
}

void __cdecl Load_RawFilePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varRawFilePtr, 4);
    DB_PushStreamPos(0);
    if ( *varRawFilePtr )
    {
        value = (unsigned int)*varRawFilePtr;
        if ( *varRawFilePtr == (RawFile *)-1 || value == -2 )
        {
            *varRawFilePtr = (RawFile *)AllocLoad_FxElemVisStateSample();
            varRawFile = *varRawFilePtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_RawFile(1);
            Load_RawFileAsset((XAssetHeader *)varRawFilePtr);
            if ( inserted )
                *inserted = *varRawFilePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varRawFilePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_RawFilePtr()
{
    if ( *varRawFilePtr )
    {
        varRawFile = *varRawFilePtr;
        Mark_RawFileAsset(varRawFile);
    }
}

void __cdecl Load_XGlobals(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXGlobals, 40);
    DB_PushStreamPos(4u);
    varXString = &varXGlobals->name;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_XGlobalsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXGlobalsPtr, 4);
    DB_PushStreamPos(0);
    if ( *varXGlobalsPtr )
    {
        value = (unsigned int)*varXGlobalsPtr;
        if ( *varXGlobalsPtr == (XGlobals *)-1 || value == -2 )
        {
            *varXGlobalsPtr = (XGlobals *)AllocLoad_FxElemVisStateSample();
            varXGlobals = *varXGlobalsPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_XGlobals(1);
            Load_XGlobalsAsset((XAssetHeader *)varXGlobalsPtr);
            if ( inserted )
                *inserted = *varXGlobalsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varXGlobalsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_XGlobalsPtr()
{
    if ( *varXGlobalsPtr )
    {
        varXGlobals = *varXGlobalsPtr;
        Mark_XGlobalsAsset(varXGlobals);
    }
}

void __cdecl Load_StringTableCell(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStringTableCell, 8);
    varXString = &varStringTableCell->string;
    Load_XString(0);
}

void __cdecl Load_StringTableCellArray(bool atStreamStart, int count)
{
    StringTableCell *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varStringTableCell, 8 * count);
    var = varStringTableCell;
    for ( i = 0; i < count; ++i )
    {
        varStringTableCell = var;
        Load_StringTableCell(0);
        ++var;
    }
}

void __cdecl Load_StringTable(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStringTable, sizeof(StringTable));
    DB_PushStreamPos(4u);
    varXString = &varStringTable->name;
    Load_XString(0);
    if ( varStringTable->values )
    {
        varStringTable->values = (StringTableCell *)AllocLoad_FxElemVisStateSample();
        varStringTableCell = varStringTable->values;
        Load_StringTableCellArray(1, varStringTable->rowCount * varStringTable->columnCount);
    }
    if ( varStringTable->cellIndex )
    {
        varStringTable->cellIndex = (__int16 *)AllocLoad_XBlendInfo();
        varshort = varStringTable->cellIndex;
        Load_shortArray(1, varStringTable->rowCount * varStringTable->columnCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_StringTablePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varStringTablePtr, 4);
    DB_PushStreamPos(0);
    if ( *varStringTablePtr )
    {
        value = (unsigned int)*varStringTablePtr;
        if ( *varStringTablePtr == (StringTable *)-1 || value == -2 )
        {
            *varStringTablePtr = (StringTable *)AllocLoad_FxElemVisStateSample();
            varStringTable = *varStringTablePtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_StringTable(1);
            Load_StringTableAsset((XAssetHeader *)varStringTablePtr);
            if ( inserted )
                *inserted = *varStringTablePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varStringTablePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_StringTablePtr()
{
    if ( *varStringTablePtr )
    {
        varStringTable = *varStringTablePtr;
        Mark_StringTableAsset(varStringTable);
    }
}

void __cdecl Load_ddlMemberDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varddlMemberDef_t, 48);
    varXString = &varddlMemberDef_t->name;
    Load_XString(0);
}

void __cdecl Load_ddlMemberDef_tArray(bool atStreamStart, int count)
{
    ddlMemberDef_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varddlMemberDef_t, 48 * count);
    var = varddlMemberDef_t;
    for ( i = 0; i < count; ++i )
    {
        varddlMemberDef_t = var;
        Load_ddlMemberDef_t(0);
        ++var;
    }
}

void __cdecl Load_ddlStructDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varddlStructDef_t, 16);
    varXString = &varddlStructDef_t->name;
    Load_XString(0);
    if ( varddlStructDef_t->members )
    {
        varddlStructDef_t->members = (ddlMemberDef_t *)AllocLoad_FxElemVisStateSample();
        varddlMemberDef_t = varddlStructDef_t->members;
        Load_ddlMemberDef_tArray(1, varddlStructDef_t->memberCount);
    }
}

void __cdecl Load_ddlStructDef_tArray(bool atStreamStart, int count)
{
    ddlStructDef_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varddlStructDef_t, 16 * count);
    var = varddlStructDef_t;
    for ( i = 0; i < count; ++i )
    {
        varddlStructDef_t = var;
        Load_ddlStructDef_t(0);
        ++var;
    }
}

void __cdecl Load_ddlEnumDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varddlEnumDef_t, 12);
    varXString = &varddlEnumDef_t->name;
    Load_XString(0);
    if ( varddlEnumDef_t->members )
    {
        varddlEnumDef_t->members = (const char **)AllocLoad_FxElemVisStateSample();
        varXString = varddlEnumDef_t->members;
        Load_XStringArray(1, varddlEnumDef_t->memberCount);
    }
}

void __cdecl Load_ddlEnumDef_tArray(bool atStreamStart, int count)
{
    ddlEnumDef_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varddlEnumDef_t, 12 * count);
    var = varddlEnumDef_t;
    for ( i = 0; i < count; ++i )
    {
        varddlEnumDef_t = var;
        Load_ddlEnumDef_t(0);
        ++var;
    }
}

void __cdecl Load_ddlDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varddlDef_t, 28);
    if ( varddlDef_t->structList )
    {
        varddlDef_t->structList = (ddlStructDef_t *)AllocLoad_FxElemVisStateSample();
        varddlStructDef_t = varddlDef_t->structList;
        Load_ddlStructDef_tArray(1, varddlDef_t->structCount);
    }
    if ( varddlDef_t->enumList )
    {
        varddlDef_t->enumList = (ddlEnumDef_t *)AllocLoad_FxElemVisStateSample();
        varddlEnumDef_t = varddlDef_t->enumList;
        Load_ddlEnumDef_tArray(1, varddlDef_t->enumCount);
    }
    if ( varddlDef_t->next )
    {
        varddlDef_t->next = (ddlDef_t *)AllocLoad_FxElemVisStateSample();
        varddlDefNext = varddlDef_t->next;
        Load_ddlDefNext(1);
    }
}

void __cdecl Load_ddlDefNext(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varddlDefNext, 28);
    varddlDef_t = varddlDefNext;
    Load_ddlDef_t(0);
}

void __cdecl Load_ddlRoot_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varddlRoot_t, 8);
    DB_PushStreamPos(4u);
    varXString = &varddlRoot_t->name;
    Load_XString(0);
    if ( varddlRoot_t->ddlDef )
    {
        varddlRoot_t->ddlDef = (ddlDef_t *)AllocLoad_FxElemVisStateSample();
        varddlDef_t = varddlRoot_t->ddlDef;
        Load_ddlDef_t(1);
    }
    DB_PopStreamPos();
}

void __cdecl Load_ddlRoot_ptr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varddlRoot_ptr, 4);
    DB_PushStreamPos(0);
    if ( *varddlRoot_ptr )
    {
        value = (unsigned int)*varddlRoot_ptr;
        if ( *varddlRoot_ptr == (ddlRoot_t *)-1 || value == -2 )
        {
            *varddlRoot_ptr = (ddlRoot_t *)AllocLoad_FxElemVisStateSample();
            varddlRoot_t = *varddlRoot_ptr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_ddlRoot_t(1);
            Load_DDLAsset((XAssetHeader *)varddlRoot_ptr);
            if ( inserted )
                *inserted = *varddlRoot_ptr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varddlRoot_ptr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_ddlRoot_ptr()
{
    if ( *varddlRoot_ptr )
    {
        varddlRoot_t = *varddlRoot_ptr;
        Mark_DDLAsset(varddlRoot_t);
    }
}

void __cdecl Load_GfxStaticModelDrawInst(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStaticModelDrawInst, 76);
    varXModelPtr = &varGfxStaticModelDrawInst->model;
    Load_XModelPtr(0);
}

void __cdecl Load_GfxStaticModelDrawInstArray(bool atStreamStart, int count)
{
    GfxStaticModelDrawInst *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStaticModelDrawInst, 76 * count);
    var = varGfxStaticModelDrawInst;
    for ( i = 0; i < count; ++i )
    {
        varGfxStaticModelDrawInst = var;
        Load_GfxStaticModelDrawInst(0);
        ++var;
    }
}

void __cdecl Load_GfxStaticModelInstArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStaticModelInst, 40 * count);
}

void __cdecl Mark_GfxStaticModelDrawInst()
{
    varXModelPtr = &varGfxStaticModelDrawInst->model;
    Mark_XModelPtr();
}

void __cdecl Mark_GfxStaticModelDrawInstArray(int count)
{
    GfxStaticModelDrawInst *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxStaticModelDrawInst;
    for ( i = 0; i < count; ++i )
    {
        varGfxStaticModelDrawInst = var;
        Mark_GfxStaticModelDrawInst();
        ++var;
    }
}

void __cdecl Load_sunflare_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsunflare_t, 96);
    varMaterialHandle = &varsunflare_t->spriteMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = &varsunflare_t->flareMaterial;
    Load_MaterialHandle(0);
}

void __cdecl Mark_sunflare_t()
{
    varMaterialHandle = &varsunflare_t->spriteMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = &varsunflare_t->flareMaterial;
    Mark_MaterialHandle();
}

void __cdecl Load_GfxReflectionProbeVolumeDataArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxReflectionProbeVolumeData, 96 * count);
}

void __cdecl Load_GfxReflectionProbe(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxReflectionProbe, 24);
    varGfxImagePtr = &varGfxReflectionProbe->reflectionImage;
    Load_GfxImagePtr(0);
    if ( varGfxReflectionProbe->probeVolumes )
    {
        varGfxReflectionProbe->probeVolumes = (GfxReflectionProbeVolumeData *)AllocLoad_FxElemVisStateSample();
        varGfxReflectionProbeVolumeData = varGfxReflectionProbe->probeVolumes;
        Load_GfxReflectionProbeVolumeDataArray(1, varGfxReflectionProbe->probeVolumeCount);
    }
}

void __cdecl Load_GfxReflectionProbeArray(bool atStreamStart, int count)
{
    GfxReflectionProbe *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxReflectionProbe, 24 * count);
    var = varGfxReflectionProbe;
    for ( i = 0; i < count; ++i )
    {
        varGfxReflectionProbe = var;
        Load_GfxReflectionProbe(0);
        ++var;
    }
}

void __cdecl Mark_GfxReflectionProbe()
{
    varGfxImagePtr = &varGfxReflectionProbe->reflectionImage;
    Mark_GfxImagePtr();
}

void __cdecl Mark_GfxReflectionProbeArray(int count)
{
    GfxReflectionProbe *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxReflectionProbe;
    for ( i = 0; i < count; ++i )
    {
        varGfxReflectionProbe = var;
        Mark_GfxReflectionProbe();
        ++var;
    }
}

void __cdecl Load_OccluderArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varOccluder, 68 * count);
}

void __cdecl Load_StaticModelIndexArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStaticModelIndex, 2 * count);
}

void __cdecl Load_GfxAabbTree(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxAabbTree, 40);
    if ( varGfxAabbTree->smodelIndexes )
    {
        if ( varGfxAabbTree->smodelIndexes == (unsigned __int16 *)-1 )
        {
            varGfxAabbTree->smodelIndexes = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varStaticModelIndex = varGfxAabbTree->smodelIndexes;
            Load_StaticModelIndexArray(1, varGfxAabbTree->smodelIndexCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varGfxAabbTree->smodelIndexes);
        }
    }
}

void __cdecl Load_GfxAabbTreeArray(bool atStreamStart, int count)
{
    GfxAabbTree *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxAabbTree, 40 * count);
    var = varGfxAabbTree;
    for ( i = 0; i < count; ++i )
    {
        varGfxAabbTree = var;
        Load_GfxAabbTree(0);
        ++var;
    }
}

void __cdecl Load_GfxCell(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCell, 56);
    if ( varGfxCell->aabbTree )
    {
        varGfxCell->aabbTree = (GfxAabbTree *)AllocLoad_FxElemVisStateSample();
        varGfxAabbTree = varGfxCell->aabbTree;
        Load_GfxAabbTreeArray(1, varGfxCell->aabbTreeCount);
    }
    if ( varGfxCell->portals )
    {
        varGfxCell->portals = (GfxPortal *)AllocLoad_FxElemVisStateSample();
        varGfxPortal = varGfxCell->portals;
        Load_GfxPortalArray(1, varGfxCell->portalCount);
    }
    if ( varGfxCell->cullGroups )
    {
        varGfxCell->cullGroups = (int *)AllocLoad_FxElemVisStateSample();
        varint = varGfxCell->cullGroups;
        Load_intArray(1, varGfxCell->cullGroupCount);
    }
    if ( varGfxCell->reflectionProbes )
    {
        varGfxCell->reflectionProbes = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varGfxCell->reflectionProbes;
        Load_byteArray(1, varGfxCell->reflectionProbeCount);
    }
}

void __cdecl Load_GfxCellArray(bool atStreamStart, int count)
{
    GfxCell *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCell, 56 * count);
    var = varGfxCell;
    for ( i = 0; i < count; ++i )
    {
        varGfxCell = var;
        Load_GfxCell(0);
        ++var;
    }
}

void __cdecl Load_GfxPortal(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPortal, 68);
    if ( varGfxPortal->cell )
    {
        if ( varGfxPortal->cell == (GfxCell *)-1 )
        {
            varGfxPortal->cell = (GfxCell *)AllocLoad_FxElemVisStateSample();
            varGfxCell = varGfxPortal->cell;
            Load_GfxCell(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varGfxPortal->cell);
        }
    }
    if ( varGfxPortal->vertices )
    {
        varGfxPortal->vertices = (float (*)[3])AllocLoad_FxElemVisStateSample();
        varvec3_t = varGfxPortal->vertices;
        Load_vec3_tArray(1, varGfxPortal->vertexCount);
    }
}

void __cdecl Load_GfxPortalArray(bool atStreamStart, int count)
{
    GfxPortal *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPortal, 68 * count);
    var = varGfxPortal;
    for ( i = 0; i < count; ++i )
    {
        varGfxPortal = var;
        Load_GfxPortal(0);
        ++var;
    }
}

void __cdecl Load_GfxCullGroupArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCullGroup, 32 * count);
}

void __cdecl Load_GfxLightGridEntryArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightGridEntry, 4 * count);
}

void __cdecl Load_GfxCompressedLightGridColorsArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCompressedLightGridColors, 168 * count);
}

void __cdecl Load_MaterialMemory(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialMemory, 8);
    varMaterialHandle = &varMaterialMemory->material;
    Load_MaterialHandle(0);
}

void __cdecl Load_MaterialMemoryArray(bool atStreamStart, int count)
{
    MaterialMemory *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialMemory, 8 * count);
    var = varMaterialMemory;
    for ( i = 0; i < count; ++i )
    {
        varMaterialMemory = var;
        Load_MaterialMemory(0);
        ++var;
    }
}

void __cdecl Load_GfxWorldVertexData(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldVertexData, 8);
    if ( varGfxWorldVertexData->vertices )
    {
        varGfxWorldVertexData->vertices = (GfxWorldVertex *)AllocLoad_FxElemVisStateSample();
        varGfxWorldVertex0 = varGfxWorldVertexData->vertices;
        Load_GfxWorldVertex0Array(1, varGfxWorldDraw->vertexCount);
    }
    varGfxVertexBuffer = &varGfxWorldVertexData->worldVb;
    Load_GfxVertexBuffer(0);
    Load_VertexBuffer(
        &varGfxWorldVertexData->worldVb,
        (unsigned __int8 *)varGfxWorldDraw->vd.vertices,
        44 * varGfxWorldDraw->vertexCount);
}

void __cdecl Load_GfxWorldVertexLayerData(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldVertexLayerData, 8);
    if ( varGfxWorldVertexLayerData->data )
    {
        varGfxWorldVertexLayerData->data = (unsigned __int8 *)AllocLoad_raw_byte();
        varbyte = varGfxWorldVertexLayerData->data;
        Load_byteArray(1, varGfxWorldDraw->vertexLayerDataSize);
    }
    varGfxVertexBuffer = &varGfxWorldVertexLayerData->layerVb;
    Load_GfxVertexBuffer(0);
    Load_VertexBuffer(
        &varGfxWorldVertexLayerData->layerVb,
        varGfxWorldDraw->vld.data,
        varGfxWorldDraw->vertexLayerDataSize);
}

void __cdecl Load_aligned_byte_pointerArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varaligned_byte_pointer, count);
}

void __cdecl Load_GfxLightGrid(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightGrid, 56);
    if ( varGfxLightGrid->rowDataStart )
    {
        varGfxLightGrid->rowDataStart = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxLightGrid->rowDataStart;
        Load_ushortArray(
            1,
            varGfxLightGrid->maxs[varGfxLightGrid->rowAxis] - varGfxLightGrid->mins[varGfxLightGrid->rowAxis] + 1);
    }
    if ( varGfxLightGrid->rawRowData )
    {
        varGfxLightGrid->rawRowData = (unsigned __int8 *)AllocLoad_FxElemVisStateSample();
        varaligned_byte_pointer = varGfxLightGrid->rawRowData;
        Load_aligned_byte_pointerArray(1, varGfxLightGrid->rawRowDataSize);
    }
    if ( varGfxLightGrid->entries )
    {
        varGfxLightGrid->entries = (GfxLightGridEntry *)AllocLoad_FxElemVisStateSample();
        varGfxLightGridEntry = varGfxLightGrid->entries;
        Load_GfxLightGridEntryArray(1, varGfxLightGrid->entryCount);
    }
    if ( varGfxLightGrid->colors )
    {
        varGfxLightGrid->colors = (GfxCompressedLightGridColors *)AllocLoad_FxElemVisStateSample();
        varGfxCompressedLightGridColors = varGfxLightGrid->colors;
        Load_GfxCompressedLightGridColorsArray(1, varGfxLightGrid->colorCount);
    }
}

void __cdecl Load_GfxStreamingAabbTreeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStreamingAabbTree, 32 * count);
}

void __cdecl Load_GfxWorldStreamInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldStreamInfo, 16);
    if ( varGfxWorldStreamInfo->aabbTrees )
    {
        varGfxWorldStreamInfo->aabbTrees = (GfxStreamingAabbTree *)AllocLoad_FxElemVisStateSample();
        varGfxStreamingAabbTree = varGfxWorldStreamInfo->aabbTrees;
        Load_GfxStreamingAabbTreeArray(1, varGfxWorldStreamInfo->aabbTreeCount);
    }
    if ( varGfxWorldStreamInfo->leafRefs )
    {
        varGfxWorldStreamInfo->leafRefs = (int *)AllocLoad_FxElemVisStateSample();
        varint = varGfxWorldStreamInfo->leafRefs;
        Load_intArray(1, varGfxWorldStreamInfo->leafRefCount);
    }
}

void __cdecl Load_GfxSceneDynModelArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSceneDynModel, 6 * count);
}

void __cdecl Load_GfxSceneDynBrushArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSceneDynBrush, 4 * count);
}

void __cdecl Load_GfxDrawSurfArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxDrawSurf, 8 * count);
}

void __cdecl Load_GfxShadowGeometry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxShadowGeometry, 12);
    if ( varGfxShadowGeometry->sortedSurfIndex )
    {
        varGfxShadowGeometry->sortedSurfIndex = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxShadowGeometry->sortedSurfIndex;
        Load_ushortArray(1, varGfxShadowGeometry->surfaceCount);
    }
    if ( varGfxShadowGeometry->smodelIndex )
    {
        varGfxShadowGeometry->smodelIndex = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxShadowGeometry->smodelIndex;
        Load_ushortArray(1, varGfxShadowGeometry->smodelCount);
    }
}

void __cdecl Load_GfxShadowGeometryArray(bool atStreamStart, int count)
{
    GfxShadowGeometry *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxShadowGeometry, 12 * count);
    var = varGfxShadowGeometry;
    for ( i = 0; i < count; ++i )
    {
        varGfxShadowGeometry = var;
        Load_GfxShadowGeometry(0);
        ++var;
    }
}

void __cdecl Load_GfxWaterBuffer(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWaterBuffer, 8);
    if ( varGfxWaterBuffer->buffer )
    {
        varGfxWaterBuffer->buffer = (float (*)[4])AllocLoad_FxElemVisStateSample();
        varvec4_t = varGfxWaterBuffer->buffer;
        Load_vec4_tArray(1, varGfxWaterBuffer->bufferSize >> 4);
    }
}

void __cdecl Load_GfxWaterBufferArray(bool atStreamStart, int count)
{
    GfxWaterBuffer *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWaterBuffer, 8 * count);
    var = varGfxWaterBuffer;
    for ( i = 0; i < count; ++i )
    {
        varGfxWaterBuffer = var;
        Load_GfxWaterBuffer(0);
        ++var;
    }
}

void __cdecl Load_GfxWorldLodChainArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldLodChain, 24 * count);
}

void __cdecl Load_GfxWorldLodInfoArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldLodInfo, 12 * count);
}

void __cdecl Load_GfxLightCoronaArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightCorona, 32 * count);
}

void __cdecl Load_GfxShadowMapVolumeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxShadowMapVolume, 16 * count);
}

void __cdecl Load_GfxExposureVolumeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxExposureVolume, 24 * count);
}

void __cdecl Load_GfxVolumePlaneArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxVolumePlane, 16 * count);
}

void __cdecl Load_GfxLightRegionAxisArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegionAxis, 20 * count);
}

void __cdecl Load_GfxLightRegionHull(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegionHull, 80);
    if ( varGfxLightRegionHull->axis )
    {
        varGfxLightRegionHull->axis = (GfxLightRegionAxis *)AllocLoad_FxElemVisStateSample();
        varGfxLightRegionAxis = varGfxLightRegionHull->axis;
        Load_GfxLightRegionAxisArray(1, varGfxLightRegionHull->axisCount);
    }
}

void __cdecl Load_GfxLightRegionHullArray(bool atStreamStart, int count)
{
    GfxLightRegionHull *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegionHull, 80 * count);
    var = varGfxLightRegionHull;
    for ( i = 0; i < count; ++i )
    {
        varGfxLightRegionHull = var;
        Load_GfxLightRegionHull(0);
        ++var;
    }
}

void __cdecl Load_GfxLightRegion(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegion, 8);
    if ( varGfxLightRegion->hulls )
    {
        varGfxLightRegion->hulls = (GfxLightRegionHull *)AllocLoad_FxElemVisStateSample();
        varGfxLightRegionHull = varGfxLightRegion->hulls;
        Load_GfxLightRegionHullArray(1, varGfxLightRegion->hullCount);
    }
}

void __cdecl Load_GfxLightRegionArray(bool atStreamStart, int count)
{
    GfxLightRegion *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegion, 8 * count);
    var = varGfxLightRegion;
    for ( i = 0; i < count; ++i )
    {
        varGfxLightRegion = var;
        Load_GfxLightRegion(0);
        ++var;
    }
}

void __cdecl Load_GfxWorldDpvsDynamic(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDpvsDynamic, 48);
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntCellBits[0] )
    {
        varGfxWorldDpvsDynamic->dynEntCellBits[0] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorldDpvsDynamic->dynEntCellBits[0];
        Load_raw_uintArray(1, varGfxWorld->dpvsPlanes.cellCount * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntCellBits[1] )
    {
        varGfxWorldDpvsDynamic->dynEntCellBits[1] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorldDpvsDynamic->dynEntCellBits[1];
        Load_raw_uintArray(1, varGfxWorld->dpvsPlanes.cellCount * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntVisData[0][0] )
    {
        varGfxWorldDpvsDynamic->dynEntVisData[0][0] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[0][0];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntVisData[1][0] )
    {
        varGfxWorldDpvsDynamic->dynEntVisData[1][0] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[1][0];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntVisData[0][1] )
    {
        varGfxWorldDpvsDynamic->dynEntVisData[0][1] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[0][1];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntVisData[1][1] )
    {
        varGfxWorldDpvsDynamic->dynEntVisData[1][1] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[1][1];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntVisData[0][2] )
    {
        varGfxWorldDpvsDynamic->dynEntVisData[0][2] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[0][2];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsDynamic->dynEntVisData[1][2] )
    {
        varGfxWorldDpvsDynamic->dynEntVisData[1][2] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[1][2];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorldDpvsStatic(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDpvsStatic, 112);
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->smodelVisData[0] )
    {
        varGfxWorldDpvsStatic->smodelVisData[0] = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisData[0];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->smodelVisData[1] )
    {
        varGfxWorldDpvsStatic->smodelVisData[1] = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisData[1];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->smodelVisData[2] )
    {
        varGfxWorldDpvsStatic->smodelVisData[2] = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisData[2];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->surfaceVisData[0] )
    {
        varGfxWorldDpvsStatic->surfaceVisData[0] = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisData[0];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->surfaceVisData[1] )
    {
        varGfxWorldDpvsStatic->surfaceVisData[1] = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisData[1];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->surfaceVisData[2] )
    {
        varGfxWorldDpvsStatic->surfaceVisData[2] = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisData[2];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->smodelVisDataCameraSaved )
    {
        varGfxWorldDpvsStatic->smodelVisDataCameraSaved = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisDataCameraSaved;
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->surfaceVisDataCameraSaved )
    {
        varGfxWorldDpvsStatic->surfaceVisDataCameraSaved = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisDataCameraSaved;
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->lodData )
    {
        varGfxWorldDpvsStatic->lodData = (unsigned int *)AllocLoad_raw_uint128();
        varraw_uint128 = varGfxWorldDpvsStatic->lodData;
        Load_raw_uint128Array(1, 2 * varGfxWorldDpvsStatic->smodelVisDataCount);
    }
    DB_PopStreamPos();
    if ( varGfxWorldDpvsStatic->sortedSurfIndex )
    {
        varGfxWorldDpvsStatic->sortedSurfIndex = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxWorldDpvsStatic->sortedSurfIndex;
        Load_ushortArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    if ( varGfxWorldDpvsStatic->smodelInsts )
    {
        varGfxWorldDpvsStatic->smodelInsts = (GfxStaticModelInst *)AllocLoad_FxElemVisStateSample();
        varGfxStaticModelInst = varGfxWorldDpvsStatic->smodelInsts;
        Load_GfxStaticModelInstArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    if ( varGfxWorldDpvsStatic->surfaces )
    {
        varGfxWorldDpvsStatic->surfaces = (GfxSurface *)AllocLoad_GfxPackedVertex0();
        varGfxSurface = varGfxWorldDpvsStatic->surfaces;
        Load_GfxSurfaceArray(1, varGfxWorld->surfaceCount);
    }
    if ( varGfxWorldDpvsStatic->cullGroups )
    {
        varGfxWorldDpvsStatic->cullGroups = (GfxCullGroup *)AllocLoad_FxElemVisStateSample();
        varGfxCullGroup = varGfxWorldDpvsStatic->cullGroups;
        Load_GfxCullGroupArray(1, varGfxWorld->cullGroupCount);
    }
    if ( varGfxWorldDpvsStatic->smodelDrawInsts )
    {
        varGfxWorldDpvsStatic->smodelDrawInsts = (GfxStaticModelDrawInst *)AllocLoad_FxElemVisStateSample();
        varGfxStaticModelDrawInst = varGfxWorldDpvsStatic->smodelDrawInsts;
        Load_GfxStaticModelDrawInstArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->surfaceMaterials )
    {
        varGfxWorldDpvsStatic->surfaceMaterials = (GfxDrawSurf *)AllocLoad_FxElemVisStateSample();
        varGfxDrawSurf = varGfxWorldDpvsStatic->surfaceMaterials;
        Load_GfxDrawSurfArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsStatic->surfaceCastsSunShadow )
    {
        varGfxWorldDpvsStatic->surfaceCastsSunShadow = (unsigned int *)AllocLoad_raw_uint128();
        varraw_uint128 = varGfxWorldDpvsStatic->surfaceCastsSunShadow;
        Load_raw_uint128Array(1, varGfxWorldDpvsStatic->surfaceVisDataCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorldDpvsPlanes(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDpvsPlanes, 16);
    if ( varGfxWorldDpvsPlanes->planes )
    {
        if ( varGfxWorldDpvsPlanes->planes == (cplane_s *)-1 )
        {
            varGfxWorldDpvsPlanes->planes = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varGfxWorldDpvsPlanes->planes;
            Load_cplane_tArray(1, varGfxWorld->planeCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varGfxWorldDpvsPlanes->planes);
        }
    }
    if ( varGfxWorldDpvsPlanes->nodes )
    {
        varGfxWorldDpvsPlanes->nodes = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxWorldDpvsPlanes->nodes;
        Load_ushortArray(1, varGfxWorld->nodeCount);
    }
    DB_PushStreamPos(1u);
    if ( varGfxWorldDpvsPlanes->sceneEntCellBits )
    {
        varGfxWorldDpvsPlanes->sceneEntCellBits = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorldDpvsPlanes->sceneEntCellBits;
        Load_raw_uintArray(1, varGfxWorldDpvsPlanes->cellCount << 9);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxOutdoorBoundsArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxOutdoorBounds, 24 * count);
}

void __cdecl Load_GfxHeroLightArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, &varGfxHeroLight->type, 56 * count);
}

void __cdecl Load_GfxHeroLightTreeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxHeroLightTree, 24 * count);
}

void __cdecl Load_GfxWorldDraw(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDraw, 192);
    if ( varGfxWorldDraw->reflectionProbes )
    {
        varGfxWorldDraw->reflectionProbes = (GfxReflectionProbe *)AllocLoad_FxElemVisStateSample();
        varGfxReflectionProbe = varGfxWorldDraw->reflectionProbes;
        Load_GfxReflectionProbeArray(1, varGfxWorldDraw->reflectionProbeCount);
    }
    DB_PushStreamPos(1u);
    if ( varGfxWorldDraw->reflectionProbeTextures )
    {
        varGfxWorldDraw->reflectionProbeTextures = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorldDraw->reflectionProbeTextures;
        Load_GfxRawTextureArray(1, varGfxWorldDraw->reflectionProbeCount);
    }
    DB_PopStreamPos();
    if ( varGfxWorldDraw->lightmaps )
    {
        varGfxWorldDraw->lightmaps = (GfxLightmapArray *)AllocLoad_FxElemVisStateSample();
        varGfxLightmapArray = varGfxWorldDraw->lightmaps;
        Load_GfxLightmapArrayArray(1, varGfxWorldDraw->lightmapCount);
    }
    DB_PushStreamPos(1u);
    if ( varGfxWorldDraw->lightmapPrimaryTextures )
    {
        varGfxWorldDraw->lightmapPrimaryTextures = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorldDraw->lightmapPrimaryTextures;
        Load_GfxRawTextureArray(1, varGfxWorldDraw->lightmapCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDraw->lightmapSecondaryTextures )
    {
        varGfxWorldDraw->lightmapSecondaryTextures = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorldDraw->lightmapSecondaryTextures;
        Load_GfxRawTextureArray(1, varGfxWorldDraw->lightmapCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorldDraw->lightmapSecondaryTexturesB )
    {
        varGfxWorldDraw->lightmapSecondaryTexturesB = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorldDraw->lightmapSecondaryTexturesB;
        Load_GfxRawTextureArray(1, varGfxWorldDraw->lightmapCount);
    }
    DB_PopStreamPos();
    varGfxImagePtr = varGfxWorldDraw->terrainScorchImages;
    Load_GfxImagePtrArray(0, 31);
    varGfxWorldVertexData = &varGfxWorldDraw->vd;
    Load_GfxWorldVertexData(0);
    varGfxWorldVertexLayerData = &varGfxWorldDraw->vld;
    Load_GfxWorldVertexLayerData(0);
    if ( varGfxWorldDraw->indices )
    {
        varGfxWorldDraw->indices = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varr_index_t = varGfxWorldDraw->indices;
        Load_r_index_tArray(1, varGfxWorldDraw->indexCount);
    }
}

void __cdecl Load_GfxWorld(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorld, 1084);
    DB_PushStreamPos(4u);
    varXString = &varGfxWorld->name;
    Load_XString(0);
    varXString = &varGfxWorld->baseName;
    Load_XString(0);
    varGfxWorldStreamInfo = &varGfxWorld->streamInfo;
    Load_GfxWorldStreamInfo(0);
    if ( varGfxWorld->skyStartSurfs )
    {
        varGfxWorld->skyStartSurfs = (int *)AllocLoad_FxElemVisStateSample();
        varint = varGfxWorld->skyStartSurfs;
        Load_intArray(1, varGfxWorld->skySurfCount);
    }
    varGfxImagePtr = &varGfxWorld->skyImage;
    Load_GfxImagePtr(0);
    varXString = &varGfxWorld->skyBoxModel;
    Load_XString(0);
    if ( varGfxWorld->sunLight )
    {
        if ( varGfxWorld->sunLight == (GfxLight *)-1 )
        {
            varGfxWorld->sunLight = (GfxLight *)AllocLoad_GfxPackedVertex0();
            varGfxLight = varGfxWorld->sunLight;
            Load_GfxLight(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varGfxWorld->sunLight);
        }
    }
    if ( varGfxWorld->coronas )
    {
        varGfxWorld->coronas = (GfxLightCorona *)AllocLoad_FxElemVisStateSample();
        varGfxLightCorona = varGfxWorld->coronas;
        Load_GfxLightCoronaArray(1, varGfxWorld->coronaCount);
    }
    if ( varGfxWorld->shadowMapVolumes )
    {
        varGfxWorld->shadowMapVolumes = (GfxShadowMapVolume *)AllocLoad_FxElemVisStateSample();
        varGfxShadowMapVolume = varGfxWorld->shadowMapVolumes;
        Load_GfxShadowMapVolumeArray(1, varGfxWorld->shadowMapVolumeCount);
    }
    if ( varGfxWorld->shadowMapVolumePlanes )
    {
        varGfxWorld->shadowMapVolumePlanes = (GfxVolumePlane *)AllocLoad_FxElemVisStateSample();
        varGfxVolumePlane = varGfxWorld->shadowMapVolumePlanes;
        Load_GfxVolumePlaneArray(1, varGfxWorld->shadowMapVolumePlaneCount);
    }
    if ( varGfxWorld->exposureVolumes )
    {
        varGfxWorld->exposureVolumes = (GfxExposureVolume *)AllocLoad_FxElemVisStateSample();
        varGfxExposureVolume = varGfxWorld->exposureVolumes;
        Load_GfxExposureVolumeArray(1, varGfxWorld->exposureVolumeCount);
    }
    if ( varGfxWorld->exposureVolumePlanes )
    {
        varGfxWorld->exposureVolumePlanes = (GfxVolumePlane *)AllocLoad_FxElemVisStateSample();
        varGfxVolumePlane = varGfxWorld->exposureVolumePlanes;
        Load_GfxVolumePlaneArray(1, varGfxWorld->exposureVolumePlaneCount);
    }
    varGfxWorldDpvsPlanes = &varGfxWorld->dpvsPlanes;
    Load_GfxWorldDpvsPlanes(0);
    if ( varGfxWorld->cells )
    {
        varGfxWorld->cells = (GfxCell *)AllocLoad_FxElemVisStateSample();
        varGfxCell = varGfxWorld->cells;
        Load_GfxCellArray(1, varGfxWorld->dpvsPlanes.cellCount);
    }
    varGfxWorldDraw = &varGfxWorld->draw;
    Load_GfxWorldDraw(0);
    varGfxLightGrid = &varGfxWorld->lightGrid;
    Load_GfxLightGrid(0);
    if ( varGfxWorld->models )
    {
        varGfxWorld->models = (GfxBrushModel *)AllocLoad_FxElemVisStateSample();
        varGfxBrushModel = varGfxWorld->models;
        Load_GfxBrushModelArray(1, varGfxWorld->modelCount);
    }
    if ( varGfxWorld->materialMemory )
    {
        varGfxWorld->materialMemory = (MaterialMemory *)AllocLoad_FxElemVisStateSample();
        varMaterialMemory = varGfxWorld->materialMemory;
        Load_MaterialMemoryArray(1, varGfxWorld->materialMemoryCount);
    }
    varsunflare_t = &varGfxWorld->sun;
    Load_sunflare_t(0);
    varGfxImagePtr = &varGfxWorld->outdoorImage;
    Load_GfxImagePtr(0);
    DB_PushStreamPos(1u);
    if ( varGfxWorld->cellCasterBits )
    {
        varGfxWorld->cellCasterBits = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->cellCasterBits;
        Load_raw_uintArray(1, varGfxWorld->dpvsPlanes.cellCount * ((varGfxWorld->dpvsPlanes.cellCount + 31) >> 5));
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorld->sceneDynModel )
    {
        varGfxWorld->sceneDynModel = (GfxSceneDynModel *)AllocLoad_FxElemVisStateSample();
        varGfxSceneDynModel = varGfxWorld->sceneDynModel;
        Load_GfxSceneDynModelArray(1, varGfxWorld->dpvsDyn.dynEntClientCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorld->sceneDynBrush )
    {
        varGfxWorld->sceneDynBrush = (GfxSceneDynBrush *)AllocLoad_FxElemVisStateSample();
        varGfxSceneDynBrush = varGfxWorld->sceneDynBrush;
        Load_GfxSceneDynBrushArray(1, varGfxWorld->dpvsDyn.dynEntClientCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorld->primaryLightEntityShadowVis )
    {
        varGfxWorld->primaryLightEntityShadowVis = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->primaryLightEntityShadowVis;
        Load_raw_uintArray(1, (varGfxWorld->primaryLightCount - (varGfxWorld->sunPrimaryLightIndex + 1)) << 13);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorld->primaryLightDynEntShadowVis[0] )
    {
        varGfxWorld->primaryLightDynEntShadowVis[0] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->primaryLightDynEntShadowVis[0];
        Load_raw_uintArray(
            1,
            varGfxWorld->dpvsDyn.dynEntClientCount[0]
        * (varGfxWorld->primaryLightCount - (varGfxWorld->sunPrimaryLightIndex + 1)));
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorld->primaryLightDynEntShadowVis[1] )
    {
        varGfxWorld->primaryLightDynEntShadowVis[1] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->primaryLightDynEntShadowVis[1];
        Load_raw_uintArray(
            1,
            varGfxWorld->dpvsDyn.dynEntClientCount[1]
        * (varGfxWorld->primaryLightCount - (varGfxWorld->sunPrimaryLightIndex + 1)));
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if ( varGfxWorld->nonSunPrimaryLightForModelDynEnt )
    {
        varGfxWorld->nonSunPrimaryLightForModelDynEnt = (unsigned __int8 *)AllocLoad_raw_byte();
        varraw_byte = varGfxWorld->nonSunPrimaryLightForModelDynEnt;
        Load_raw_byteArray(1, varGfxWorld->dpvsDyn.dynEntClientCount[0]);
    }
    DB_PopStreamPos();
    if ( varGfxWorld->shadowGeom )
    {
        varGfxWorld->shadowGeom = (GfxShadowGeometry *)AllocLoad_FxElemVisStateSample();
        varGfxShadowGeometry = varGfxWorld->shadowGeom;
        Load_GfxShadowGeometryArray(1, varGfxWorld->primaryLightCount);
    }
    if ( varGfxWorld->lightRegion )
    {
        varGfxWorld->lightRegion = (GfxLightRegion *)AllocLoad_FxElemVisStateSample();
        varGfxLightRegion = varGfxWorld->lightRegion;
        Load_GfxLightRegionArray(1, varGfxWorld->primaryLightCount);
    }
    varGfxWorldDpvsStatic = &varGfxWorld->dpvs;
    Load_GfxWorldDpvsStatic(0);
    varGfxWorldDpvsDynamic = &varGfxWorld->dpvsDyn;
    Load_GfxWorldDpvsDynamic(0);
    if ( varGfxWorld->worldLodChains )
    {
        varGfxWorld->worldLodChains = (GfxWorldLodChain *)AllocLoad_FxElemVisStateSample();
        varGfxWorldLodChain = varGfxWorld->worldLodChains;
        Load_GfxWorldLodChainArray(1, varGfxWorld->worldLodChainCount);
    }
    if ( varGfxWorld->worldLodInfos )
    {
        varGfxWorld->worldLodInfos = (GfxWorldLodInfo *)AllocLoad_FxElemVisStateSample();
        varGfxWorldLodInfo = varGfxWorld->worldLodInfos;
        Load_GfxWorldLodInfoArray(1, varGfxWorld->worldLodInfoCount);
    }
    if ( varGfxWorld->worldLodSurfaces )
    {
        varGfxWorld->worldLodSurfaces = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varuint = varGfxWorld->worldLodSurfaces;
        Load_uintArray(1, varGfxWorld->worldLodSurfaceCount);
    }
    varGfxWaterBuffer = varGfxWorld->waterBuffers;
    Load_GfxWaterBufferArray(0, 2);
    varMaterialHandle = (Material **)&varGfxWorld->waterMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = (Material **)&varGfxWorld->coronaMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = (Material **)&varGfxWorld->ropeMaterial;
    Load_MaterialHandle(0);
    if ( varGfxWorld->occluders )
    {
        varGfxWorld->occluders = (Occluder *)AllocLoad_FxElemVisStateSample();
        varOccluder = varGfxWorld->occluders;
        Load_OccluderArray(1, varGfxWorld->numOccluders);
    }
    if ( varGfxWorld->outdoorBounds )
    {
        varGfxWorld->outdoorBounds = (GfxOutdoorBounds *)AllocLoad_FxElemVisStateSample();
        varGfxOutdoorBounds = varGfxWorld->outdoorBounds;
        Load_GfxOutdoorBoundsArray(1, varGfxWorld->numOutdoorBounds);
    }
    if ( varGfxWorld->heroLights )
    {
        varGfxWorld->heroLights = (GfxHeroLight *)AllocLoad_FxElemVisStateSample();
        varGfxHeroLight = varGfxWorld->heroLights;
        Load_GfxHeroLightArray(1, varGfxWorld->heroLightCount);
    }
    if ( varGfxWorld->heroLightTree )
    {
        varGfxWorld->heroLightTree = (GfxHeroLightTree *)AllocLoad_FxElemVisStateSample();
        varGfxHeroLightTree = varGfxWorld->heroLightTree;
        Load_GfxHeroLightTreeArray(1, varGfxWorld->heroLightTreeCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorldPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldPtr, 4);
    DB_PushStreamPos(0);
    if ( *varGfxWorldPtr )
    {
        value = (unsigned int)*varGfxWorldPtr;
        if ( *varGfxWorldPtr == (GfxWorld *)-1 || value == -2 )
        {
            *varGfxWorldPtr = (GfxWorld *)AllocLoad_FxElemVisStateSample();
            varGfxWorld = *varGfxWorldPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxWorld(1);
            Load_GfxWorldAsset((XAssetHeader *)varGfxWorldPtr);
            if ( inserted )
                *inserted = *varGfxWorldPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxWorldPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_MaterialMemory()
{
    varMaterialHandle = &varMaterialMemory->material;
    Mark_MaterialHandle();
}

void __cdecl Mark_MaterialMemoryArray(int count)
{
    MaterialMemory *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varMaterialMemory;
    for ( i = 0; i < count; ++i )
    {
        varMaterialMemory = var;
        Mark_MaterialMemory();
        ++var;
    }
}

void __cdecl Mark_GfxWorldDpvsStatic()
{
    if ( varGfxWorldDpvsStatic->surfaces )
    {
        varGfxSurface = varGfxWorldDpvsStatic->surfaces;
        Mark_GfxSurfaceArray(varGfxWorld->surfaceCount);
    }
    if ( varGfxWorldDpvsStatic->smodelDrawInsts )
    {
        varGfxStaticModelDrawInst = varGfxWorldDpvsStatic->smodelDrawInsts;
        Mark_GfxStaticModelDrawInstArray(varGfxWorldDpvsStatic->smodelCount);
    }
}

void __cdecl Mark_GfxWorldDraw()
{
    if ( varGfxWorldDraw->reflectionProbes )
    {
        varGfxReflectionProbe = varGfxWorldDraw->reflectionProbes;
        Mark_GfxReflectionProbeArray(varGfxWorldDraw->reflectionProbeCount);
    }
    if ( varGfxWorldDraw->lightmaps )
    {
        varGfxLightmapArray = varGfxWorldDraw->lightmaps;
        Mark_GfxLightmapArrayArray(varGfxWorldDraw->lightmapCount);
    }
    varGfxImagePtr = varGfxWorldDraw->terrainScorchImages;
    Mark_GfxImagePtrArray(31);
}

void __cdecl Mark_GfxWorld()
{
    varGfxImagePtr = &varGfxWorld->skyImage;
    Mark_GfxImagePtr();
    if ( varGfxWorld->sunLight )
    {
        varGfxLight = varGfxWorld->sunLight;
        Mark_GfxLight();
    }
    varGfxWorldDraw = &varGfxWorld->draw;
    Mark_GfxWorldDraw();
    if ( varGfxWorld->materialMemory )
    {
        varMaterialMemory = varGfxWorld->materialMemory;
        Mark_MaterialMemoryArray(varGfxWorld->materialMemoryCount);
    }
    varsunflare_t = &varGfxWorld->sun;
    Mark_sunflare_t();
    varGfxImagePtr = &varGfxWorld->outdoorImage;
    Mark_GfxImagePtr();
    varGfxWorldDpvsStatic = &varGfxWorld->dpvs;
    Mark_GfxWorldDpvsStatic();
    varMaterialHandle = (Material **)&varGfxWorld->waterMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = (Material **)&varGfxWorld->coronaMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = (Material **)&varGfxWorld->ropeMaterial;
    Mark_MaterialHandle();
}

void __cdecl Mark_GfxWorldPtr()
{
    if ( *varGfxWorldPtr )
    {
        varGfxWorld = *varGfxWorldPtr;
        Mark_GfxWorldAsset(varGfxWorld);
        Mark_GfxWorld();
    }
}

void __cdecl Load_GlyphArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGlyph, sizeof(Glyph) * count);
}

void __cdecl Load_Font(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFont, sizeof(Font_s));
    DB_PushStreamPos(4u);
    varXString = &varFont->fontName;
    Load_XString(0);
    varMaterialHandle = &varFont->material;
    Load_MaterialHandle(0);
    varMaterialHandle = &varFont->glowMaterial;
    Load_MaterialHandle(0);
    if ( varFont->glyphs )
    {
        if ( varFont->glyphs == (Glyph *)-1 )
        {
            varFont->glyphs = (Glyph *)AllocLoad_FxElemVisStateSample();
            varGlyph = varFont->glyphs;
            Load_GlyphArray(1, varFont->glyphCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)&varFont->glyphs);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_FontHandle(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFontHandle, 4);
    DB_PushStreamPos(0);
    if ( *varFontHandle )
    {
        value = (unsigned int)*varFontHandle;
        if ( *varFontHandle == (Font_s *)-1 || value == -2 )
        {
            *varFontHandle = (Font_s *)AllocLoad_FxElemVisStateSample();
            varFont = *varFontHandle;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_Font(1);
            Load_FontAsset((XAssetHeader *)varFontHandle);
            if ( inserted )
                *inserted = *varFontHandle;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varFontHandle);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_Font()
{
    varMaterialHandle = &varFont->material;
    Mark_MaterialHandle();
    varMaterialHandle = &varFont->glowMaterial;
    Mark_MaterialHandle();
}

void __cdecl Mark_FontHandle()
{
    if ( *varFontHandle )
    {
        varFont = *varFontHandle;
        Mark_FontAsset(varFont);
        Mark_Font();
    }
}

void __cdecl Load_GlassDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGlassDef, 60);
    varXString = &varGlassDef->name;
    Load_XString(0);
    varMaterialHandle = &varGlassDef->pristineMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = &varGlassDef->crackedMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = &varGlassDef->shardMaterial;
    Load_MaterialHandle(0);
    varXString = &varGlassDef->crackSound;
    Load_XString(0);
    varXString = &varGlassDef->shatterShound;
    Load_XString(0);
    varXString = &varGlassDef->autoShatterShound;
    Load_XString(0);
    varFxEffectDefHandle = &varGlassDef->crackEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varGlassDef->shatterEffect;
    Load_FxEffectDefHandle(0);
}

void __cdecl Load_Glass(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGlass, 124);
    if ( varGlass->glassDef )
    {
        if ( varGlass->glassDef == (GlassDef *)-1 )
        {
            varGlass->glassDef = (GlassDef *)AllocLoad_FxElemVisStateSample();
            varGlassDef = varGlass->glassDef;
            Load_GlassDef(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int *)varGlass);
        }
    }
    if ( varGlass->outline )
    {
        varGlass->outline = (float (*)[2])AllocLoad_FxElemVisStateSample();
        varvec2_t = varGlass->outline;
        Load_vec2_tArray(1, varGlass->numOutlineVerts);
    }
}

void __cdecl Load_GlassArray(bool atStreamStart, int count)
{
    Glass *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGlass, 124 * count);
    var = varGlass;
    for ( i = 0; i < count; ++i )
    {
        varGlass = var;
        Load_Glass(0);
        ++var;
    }
}

void __cdecl Load_Glasses(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGlasses, 56);
    DB_PushStreamPos(4u);
    varXString = &varGlasses->name;
    Load_XString(0);
    if ( varGlasses->glasses )
    {
        varGlasses->glasses = (Glass *)AllocLoad_FxElemVisStateSample();
        varGlass = varGlasses->glasses;
        Load_GlassArray(1, varGlasses->numGlasses);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GlassesPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGlassesPtr, 4);
    DB_PushStreamPos(0);
    if ( *varGlassesPtr )
    {
        value = (unsigned int)*varGlassesPtr;
        if ( *varGlassesPtr == (Glasses *)-1 || value == -2 )
        {
            *varGlassesPtr = (Glasses *)AllocLoad_FxElemVisStateSample();
            varGlasses = *varGlassesPtr;
            if ( value == -2 )
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_Glasses(1);
            Load_GlassesAsset((XAssetHeader *)varGlassesPtr);
            if ( inserted )
                *inserted = *varGlassesPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGlassesPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_GlassDef()
{
    varMaterialHandle = &varGlassDef->pristineMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = &varGlassDef->crackedMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = &varGlassDef->shardMaterial;
    Mark_MaterialHandle();
    varFxEffectDefHandle = &varGlassDef->crackEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varGlassDef->shatterEffect;
    Mark_FxEffectDefHandle();
}

void __cdecl Mark_Glass()
{
    if ( varGlass->glassDef )
    {
        varGlassDef = varGlass->glassDef;
        Mark_GlassDef();
    }
}

void __cdecl Mark_GlassArray(int count)
{
    Glass *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGlass;
    for ( i = 0; i < count; ++i )
    {
        varGlass = var;
        Mark_Glass();
        ++var;
    }
}

void __cdecl Mark_Glasses()
{
    if ( varGlasses->glasses )
    {
        varGlass = varGlasses->glasses;
        Mark_GlassArray(varGlasses->numGlasses);
    }
}

void __cdecl Mark_GlassesPtr()
{
    if ( *varGlassesPtr )
    {
        varGlasses = *varGlassesPtr;
        Mark_GlassesAsset(varGlasses);
        Mark_Glasses();
    }
}

void __cdecl Load_XAssetHeader(bool atStreamStart)
{
    switch ( varXAsset->type )
    {
        case 1:
            varPhysPresetPtr = (PhysPreset **)varXAssetHeader;
            Load_PhysPresetPtr(atStreamStart);
            break;
        case 2:
            varPhysConstraintsPtr = (PhysConstraints **)varXAssetHeader;
            Load_PhysConstraintsPtr(atStreamStart);
            break;
        case 3:
            varDestructibleDefPtr = (DestructibleDef **)varXAssetHeader;
            Load_DestructibleDefPtr(atStreamStart);
            break;
        case 4:
            varXAnimPartsPtr = (XAnimParts **)varXAssetHeader;
            Load_XAnimPartsPtr(atStreamStart);
            break;
        case 5:
            varXModelPtr = (XModel **)varXAssetHeader;
            Load_XModelPtr(atStreamStart);
            break;
        case 6:
            varMaterialHandle = (Material **)varXAssetHeader;
            Load_MaterialHandle(atStreamStart);
            break;
        case 7:
            varMaterialTechniqueSetPtr = (MaterialTechniqueSet **)varXAssetHeader;
            Load_MaterialTechniqueSetPtr(atStreamStart);
            break;
        case 8:
            varGfxImagePtr = (GfxImage **)varXAssetHeader;
            Load_GfxImagePtr(atStreamStart);
            break;
        case 9:
            varSndBankPtr = (SndBank **)varXAssetHeader;
            Load_SndBankPtr(atStreamStart);
            break;
        case 10:
            varSndPatchPtr = (SndPatch **)varXAssetHeader;
            Load_SndPatchPtr(atStreamStart);
            break;
        case 11:
        case 12:
            varclipMap_ptr = (clipMap_t **)varXAssetHeader;
            Load_clipMap_ptr(atStreamStart);
            break;
        case 13:
            varComWorldPtr = (ComWorld **)varXAssetHeader;
            Load_ComWorldPtr(atStreamStart);
            break;
        case 14:
            varGameWorldSpPtr = (GameWorldSp **)varXAssetHeader;
            Load_GameWorldSpPtr(atStreamStart);
            break;
        case 15:
            varGameWorldMpPtr = (GameWorldMp **)varXAssetHeader;
            Load_GameWorldMpPtr(atStreamStart);
            break;
        case 16:
            varMapEntsPtr = (MapEnts **)varXAssetHeader;
            Load_MapEntsPtr(atStreamStart);
            break;
        case 17:
            varGfxWorldPtr = (GfxWorld **)varXAssetHeader;
            Load_GfxWorldPtr(atStreamStart);
            break;
        case 18:
            varGfxLightDefPtr = (GfxLightDef **)varXAssetHeader;
            Load_GfxLightDefPtr(atStreamStart);
            break;
        case 20:
            varFontHandle = (Font_s **)varXAssetHeader;
            Load_FontHandle(atStreamStart);
            break;
        case 21:
            varMenuListPtr = (MenuList **)varXAssetHeader;
            Load_MenuListPtr(atStreamStart);
            break;
        case 22:
            varmenuDef_ptr = (menuDef_t **)varXAssetHeader;
            Load_menuDef_ptr(atStreamStart);
            break;
        case 23:
            varLocalizeEntryPtr = (LocalizeEntry **)varXAssetHeader;
            Load_LocalizeEntryPtr(atStreamStart);
            break;
        case 24:
            varWeaponVariantDefPtr = (WeaponVariantDef **)varXAssetHeader;
            Load_WeaponVariantDefPtr(atStreamStart);
            break;
        case 27:
            varSndDriverGlobalsPtr = (SndDriverGlobals **)varXAssetHeader;
            Load_SndDriverGlobalsPtr(atStreamStart);
            break;
        case 28:
            varFxEffectDefHandle = (const FxEffectDef **)varXAssetHeader;
            Load_FxEffectDefHandle(atStreamStart);
            break;
        case 29:
            varFxImpactTablePtr = (FxImpactTable **)varXAssetHeader;
            Load_FxImpactTablePtr(atStreamStart);
            break;
        case 36:
            varRawFilePtr = (RawFile **)varXAssetHeader;
            Load_RawFilePtr(atStreamStart);
            break;
        case 37:
            varStringTablePtr = (StringTable **)varXAssetHeader;
            Load_StringTablePtr(atStreamStart);
            break;
        case 38:
            varPackIndexPtr = (PackIndex **)varXAssetHeader;
            Load_PackIndexPtr(atStreamStart);
            break;
        case 39:
            varXGlobalsPtr = (XGlobals **)varXAssetHeader;
            Load_XGlobalsPtr(atStreamStart);
            break;
        case 40:
            varddlRoot_ptr = (ddlRoot_t **)varXAssetHeader;
            Load_ddlRoot_ptr(atStreamStart);
            break;
        case 41:
            varGlassesPtr = (Glasses **)varXAssetHeader;
            Load_GlassesPtr(atStreamStart);
            break;
        case 42:
            varEmblemSetPtr = (EmblemSet **)varXAssetHeader;
            Load_EmblemSetPtr(atStreamStart);
            break;
    }
}

void __cdecl Load_XAsset(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAsset, 8);
    varXAssetHeader = &varXAsset->header;
    Load_XAssetHeader(0);
}

void __cdecl Mark_XAssetHeader()
{
    switch ( varXAsset->type )
    {
        case ASSET_TYPE_PHYSPRESET:
            varPhysPresetPtr = (PhysPreset **)varXAssetHeader;
            Mark_PhysPresetPtr();
            break;
        case ASSET_TYPE_PHYSCONSTRAINTS:
            varPhysConstraintsPtr = (PhysConstraints **)varXAssetHeader;
            Mark_PhysConstraintsPtr();
            break;
        case ASSET_TYPE_DESTRUCTIBLEDEF:
            varDestructibleDefPtr = (DestructibleDef **)varXAssetHeader;
            Mark_DestructibleDefPtr();
            break;
        case ASSET_TYPE_XANIMPARTS:
            varXAnimPartsPtr = (XAnimParts **)varXAssetHeader;
            Mark_XAnimPartsPtr();
            break;
        case ASSET_TYPE_XMODEL:
            varXModelPtr = (XModel **)varXAssetHeader;
            Mark_XModelPtr();
            break;
        case ASSET_TYPE_MATERIAL:
            varMaterialHandle = (Material **)varXAssetHeader;
            Mark_MaterialHandle();
            break;
        case ASSET_TYPE_TECHNIQUE_SET:
            varMaterialTechniqueSetPtr = (MaterialTechniqueSet **)varXAssetHeader;
            Mark_MaterialTechniqueSetPtr();
            break;
        case ASSET_TYPE_IMAGE:
            varGfxImagePtr = (GfxImage **)varXAssetHeader;
            Mark_GfxImagePtr();
            break;
        case ASSET_TYPE_SOUND:
            varSndBankPtr = (SndBank **)varXAssetHeader;
            Mark_SndBankPtr();
            break;
        case ASSET_TYPE_SOUND_PATCH:
            varSndPatchPtr = (SndPatch **)varXAssetHeader;
            Mark_SndPatchPtr();
            break;
        case ASSET_TYPE_CLIPMAP:
        case ASSET_TYPE_CLIPMAP_PVS:
            varclipMap_ptr = (clipMap_t **)varXAssetHeader;
            Mark_clipMap_ptr();
            break;
        case ASSET_TYPE_COMWORLD:
            varComWorldPtr = (ComWorld **)varXAssetHeader;
            Mark_ComWorldPtr();
            break;
        case ASSET_TYPE_GAMEWORLD_SP:
            varGameWorldSpPtr = (GameWorldSp **)varXAssetHeader;
            Mark_GameWorldSpPtr();
            break;
        case ASSET_TYPE_GAMEWORLD_MP:
            varGameWorldMpPtr = (GameWorldMp **)varXAssetHeader;
            Mark_GameWorldMpPtr();
            break;
        case ASSET_TYPE_MAP_ENTS:
            varMapEntsPtr = (MapEnts **)varXAssetHeader;
            Mark_MapEntsPtr();
            break;
        case ASSET_TYPE_GFXWORLD:
            varGfxWorldPtr = (GfxWorld **)varXAssetHeader;
            Mark_GfxWorldPtr();
            break;
        case ASSET_TYPE_LIGHT_DEF:
            varGfxLightDefPtr = (GfxLightDef **)varXAssetHeader;
            Mark_GfxLightDefPtr();
            break;
        case ASSET_TYPE_FONT:
            varFontHandle = (Font_s **)varXAssetHeader;
            Mark_FontHandle();
            break;
        case ASSET_TYPE_MENULIST:
            varMenuListPtr = (MenuList **)varXAssetHeader;
            Mark_MenuListPtr();
            break;
        case ASSET_TYPE_MENU:
            varmenuDef_ptr = (menuDef_t **)varXAssetHeader;
            Mark_menuDef_ptr();
            break;
        case ASSET_TYPE_LOCALIZE_ENTRY:
            varLocalizeEntryPtr = (LocalizeEntry **)varXAssetHeader;
            Mark_LocalizeEntryPtr();
            break;
        case ASSET_TYPE_WEAPON:
            varWeaponVariantDefPtr = (WeaponVariantDef **)varXAssetHeader;
            Mark_WeaponVariantDefPtr();
            break;
        case ASSET_TYPE_SNDDRIVER_GLOBALS:
            varSndDriverGlobalsPtr = (SndDriverGlobals **)varXAssetHeader;
            Mark_SndDriverGlobalsPtr();
            break;
        case ASSET_TYPE_FX:
            varFxEffectDefHandle = (const FxEffectDef **)varXAssetHeader;
            Mark_FxEffectDefHandle();
            break;
        case ASSET_TYPE_IMPACT_FX:
            varFxImpactTablePtr = (FxImpactTable **)varXAssetHeader;
            Mark_FxImpactTablePtr();
            break;
        case ASSET_TYPE_RAWFILE:
            varRawFilePtr = (RawFile **)varXAssetHeader;
            Mark_RawFilePtr();
            break;
        case ASSET_TYPE_STRINGTABLE:
            varStringTablePtr = (StringTable **)varXAssetHeader;
            Mark_StringTablePtr();
            break;
        case ASSET_TYPE_PACK_INDEX:
            varPackIndexPtr = (PackIndex **)varXAssetHeader;
            Mark_PackIndexPtr();
            break;
        case ASSET_TYPE_XGLOBALS:
            varXGlobalsPtr = (XGlobals **)varXAssetHeader;
            Mark_XGlobalsPtr();
            break;
        case ASSET_TYPE_DDL:
            varddlRoot_ptr = (ddlRoot_t **)varXAssetHeader;
            Mark_ddlRoot_ptr();
            break;
        case ASSET_TYPE_GLASSES:
            varGlassesPtr = (Glasses **)varXAssetHeader;
            Mark_GlassesPtr();
            break;
        case ASSET_TYPE_EMBLEMSET:
            varEmblemSetPtr = (EmblemSet **)varXAssetHeader;
            Mark_EmblemSetPtr();
            break;
    }
}

void __cdecl Mark_XAsset()
{
    varXAssetHeader = &varXAsset->header;
    Mark_XAssetHeader();
}

