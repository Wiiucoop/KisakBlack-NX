#pragma once

struct XModel;

void __cdecl CGScr_LoadAnimTrees();
unsigned __int8 *__cdecl Hunk_AllocXAnimCreate(unsigned int size);
void __cdecl CG_FreeClientDObjInfo(int localClientNum);
void __cdecl CG_SetDObjInfo(int localClientNum, int iEntNum, int iEntType, XModel *pXModel);
bool __cdecl CG_CheckDObjInfoMatches(int localClientNum, int iEntNum, int iEntType, XModel *pXModel);
void __cdecl CG_SafeDObjFree(int localClientNum, int entIndex);
void __cdecl CG_FreeEntityDObjInfo(int localClientNum);

#ifdef KISAK_SP
struct XAnim_s;
XAnim_s *CG_ResolvePublishedAnims_SP(unsigned int treeIndex, bool preferClientCopy);
bool CG_CheckDObjAnimTreeMatches_SP(int localClientNum, int entIndex);
#endif
