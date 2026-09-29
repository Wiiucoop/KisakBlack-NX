#include "cg_animtree_mp.h"
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_memorytree.h>
#include <client_mp/cl_cgame_mp.h>
#include <clientscript/cscr_animtree.h>
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include "cg_local_mp.h"
#include <qcommon/dobj_management.h>

#ifdef KISAK_SP
namespace
{
// Retail caches the snapshot animation index alongside model/type. Keep this
// outside cg_s so the reconstructed client globals retain their layout.
unsigned char s_dobjAnimTreeIndex[2][1536] = {};
}

XAnim_s *CG_ResolvePublishedAnims_SP(unsigned int treeIndex, bool preferClientCopy)
{
    if (!treeIndex)
        return NULL;
    const scrAnimPub_t &pub = gScrAnimPub[SCRIPTINSTANCE_SERVER];
    if (treeIndex < MAX_XANIMTREE_NUM)
    {
        // Retail 0x007766E0 resolves the parallel client copy. The host may
        // only have the server copy loaded; its immutable asset can be shared.
        if (preferClientCopy && treeIndex <= pub.xanim_num[0] && pub.xanim_lookup[0][treeIndex].anims)
            return pub.xanim_lookup[0][treeIndex].anims;
        if (treeIndex <= pub.xanim_num[1] && pub.xanim_lookup[1][treeIndex].anims)
            return pub.xanim_lookup[1][treeIndex].anims;
    }
    Com_Error(ERR_DROP, "SP animation tree index %u is not loaded (client %u, server %u)",
        treeIndex, pub.xanim_num[0], pub.xanim_num[1]);
    return NULL;
}

bool CG_CheckDObjAnimTreeMatches_SP(int localClientNum, int entIndex)
{
    return s_dobjAnimTreeIndex[localClientNum][entIndex]
        == CG_GetEntity(localClientNum, entIndex)->nextState.animTreeIndex;
}
#endif

void __cdecl CGScr_LoadAnimTrees()
{
    signed int i; // [esp+14h] [ebp-8h]
    char *string; // [esp+18h] [ebp-4h]

    Scr_BeginLoadAnimTrees(SCRIPTINSTANCE_SERVER, 0);
    // Animation trees own a fixed range; later configstrings may contain HUD text.
    for ( i = CS_ANIMTREES; i <= CS_ANIMTREES_LAST; ++i )
    {
        string = CL_GetConfigString(i);
        if ( *string )
        {
            if ( strcmp("multiplayer", string) )
            {
                Scr_ClientUsingTree(SCRIPTINSTANCE_SERVER, string);
                Scr_PrecacheAnimTrees(SCRIPTINSTANCE_SERVER, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1);
            }
        }
    }
}

unsigned __int8 *__cdecl Hunk_AllocXAnimCreate(unsigned int size)
{
    return Hunk_AllocLow(size, "XAnimCreateAnims", 13);
}

void __cdecl CG_FreeClientDObjInfo(int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
        CG_SafeDObjFree(localClientNum, i);
}

void __cdecl CG_SetDObjInfo(int localClientNum, int iEntNum, int iEntType, XModel *pXModel)
{
    cg_s *cgameGlob; // eax

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->iEntityLastType[iEntNum] = iEntType;
    cgameGlob->pEntityLastXModel[iEntNum] = pXModel;
#ifdef KISAK_SP
    s_dobjAnimTreeIndex[localClientNum][iEntNum] = pXModel
        ? CG_GetEntity(localClientNum, iEntNum)->nextState.animTreeIndex : 0;
#endif
}

bool __cdecl CG_CheckDObjInfoMatches(int localClientNum, int iEntNum, int iEntType, XModel *pXModel)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cgameGlob->iEntityLastType[iEntNum] == iEntType && cgameGlob->pEntityLastXModel[iEntNum] == pXModel
#ifdef KISAK_SP
        && CG_CheckDObjAnimTreeMatches_SP(localClientNum, iEntNum)
#endif
        ;
}

void __cdecl CG_SafeDObjFree(int localClientNum, int entIndex)
{
    centity_s *cent; // [esp+8h] [ebp-4h]

    Com_SafeClientDObjFree(entIndex, localClientNum);
    CG_SetDObjInfo(localClientNum, entIndex, 0, 0);
    cent = CG_GetEntity(localClientNum, entIndex);
    if ( cent->tree )
    {
        if ( entIndex >= 32 )
        {
            XAnimFreeTree(cent->tree, (void (__cdecl *)(void *, int, scriptInstance_t))MT_Free, SCRIPTINSTANCE_SERVER);
            cent->tree = 0;
        }
    }
}

void __cdecl CG_FreeEntityDObjInfo(int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 32; i < 1024; ++i )
        CG_SafeDObjFree(localClientNum, i);
}
