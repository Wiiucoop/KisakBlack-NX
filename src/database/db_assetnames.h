#pragma once

union XAssetHeader;
struct XModelPiece;
struct XAsset;

const char *__cdecl DB_ImageGetName(const XAssetHeader *header);
void __cdecl DB_ImageSetName(XAssetHeader *header, const char *name);
const char *__cdecl DB_LocalizeEntryGetName(const XAssetHeader *header);
void __cdecl DB_LocalizeEntrySetName(XAssetHeader *header, const char *name);
void __cdecl DB_DDLSetname(XAssetHeader *header, const char *name);
const char *__cdecl DB_DDLGetName(const XAssetHeader *header);
const char *__cdecl DB_GetEmblemSetName(const XAssetHeader *header);
const char *__cdecl DB_GetXAssetHeaderName(int type, const XAssetHeader *header);
const char *__cdecl DB_GetXAssetName(const XAsset *asset);
void __cdecl DB_SetXAssetName(XAsset *asset, const char *name);
int __cdecl DB_GetXAssetTypeSize(int type);
const char *__cdecl DB_GetXAssetTypeName(unsigned int type);

extern const char *g_assetNames[43];
#ifdef KISAK_NX
// nx-port probe helper: like DB_GetXAssetName but returns NULL instead of
// asserting. Diagnostics only.
const char *__cdecl DB_GetXAssetNameNoAssert(const struct XAsset *asset);
#endif