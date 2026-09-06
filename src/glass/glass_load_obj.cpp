#include "glass_load_obj.h"
#include "glass.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>

bool glassesInited;
Glasses glasses;

Glasses *__cdecl GetGlasses_LoadObj()
{
    if ( !glassesInited )
    {
        glassesInited = 1;
        memset((unsigned __int8 *)&glasses, 0, sizeof(glasses));
        glasses.glasses = (Glass *)operator new[](0x1E460u);
        glasses.numGlasses = 0;
        glasses.name = 0;
    }
    return &glasses;
}

Glasses *__cdecl GetGlasses_FastFile()
{
    XAssetHeader header; // [esp+0h] [ebp-4h] BYREF

    if ( DB_GetAllXAssetOfType(ASSET_TYPE_GLASSES, &header, 1) <= 0 )
        return 0;
    else
        return (Glasses *)header.xmodelPieces;
}

Glasses *__cdecl GetGlasses()
{
    bool v1; // [esp+4h] [ebp-8h]

    v1 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.string;
    if ( v1 || !useFastFile->current.enabled )
        // x86: cast a un puntatore a funzione con ritorno int. Su LP64
        // troncherebbe il puntatore a 32 bit. Gli argomenti in eccesso
        // erano gia' ignorati dal callee.
        return GetGlasses_LoadObj();
    else
        return GetGlasses_FastFile();
}

