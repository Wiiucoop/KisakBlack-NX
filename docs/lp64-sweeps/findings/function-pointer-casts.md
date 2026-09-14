# Function-pointer casts that truncated returned pointers

**Status: closed.** Fixed in `1f14435`, "Fix 15 function-pointer casts that
truncated returned pointers on LP64".

## The shape

Black Ops selects between a fast-file and a load-obj implementation at runtime.
On x86 the decompiler could not see through the indirection, so it rendered each
dispatch as a cast of the target function to a pointer-to-function type it
inferred from the *call site registers* — which, on x86, meant `int` return and
however many arguments were pushed:

```c
Font_s *__cdecl R_RegisterFont(const char *name, int imageTrack)
{
    if ( useFastFile->current.enabled )
        return (Font_s *)((int (__cdecl *)(const char *, int))R_RegisterFont_FastFile)(name, imageTrack);
    else
        return R_RegisterFont_LoadObj(name, imageTrack);
}
```

On x86 this is harmless: `int` and `Font_s *` are both 32 bits, so the value
round-trips through the `int` return and the outer cast puts it back.

On LP64 it is not. The call is made through a prototype that returns `int`, so
the callee's 64-bit pointer return is **truncated to 32 bits** before the outer
cast ever sees it. The upper half is gone. Every one of these returned a pointer
into the asset pools, so the result was a plausible-looking but wrong address —
and it failed silently, at whatever later point something dereferenced it.

Two further details made these easy to miss:

- The casts also declared *more arguments than the callee takes*
  (`R_RegisterFont_FastFile` takes only `name`). On x86 `__cdecl` the caller
  cleans the stack, so the extras were simply ignored. The fix drops them.
- Some pass the target function to itself as an argument
  (`GetGlasses_LoadObj(GetGlasses_LoadObj)`), which is pure decompiler noise.

## The fix

Call the function directly and let the real prototype apply:

```c
    if ( useFastFile->current.enabled )
        return R_RegisterFont_FastFile(name);
```

No cast, no truncation, and the compiler now checks the arguments.

## Sites

| file | enclosing function | target |
| --- | --- | --- |
| `src/bgame/bg_weapons_load_obj.cpp` | `BG_FlameTableUpdateField` | `BG_LoadDefaultWeaponVariantDef_FastFile` / `_LoadObj` |
| `src/gfx_d3d/r_font.cpp` | `R_RegisterFont` | `R_RegisterFont_FastFile` |
| `src/gfx_d3d/r_image.cpp` | `Image_Register` | `Image_Register_FastFile` |
| `src/gfx_d3d/r_material.cpp` | `Material_GetHashIndex` | `Material_Register_FastFile` |
| `src/glass/glass_load_obj.cpp` | `GetGlasses` | `GetGlasses_LoadObj` / `_FastFile` |
| `src/ui/ui_shared_obj.cpp` | `Item_SetupKeywordHash` | `UI_LoadMenus_FastFile` / `UI_LoadMenu_LoadObj` |
| `src/ui/ui_shared_obj.cpp` | `UI_LoadMenus` | `UI_LoadMenus_FastFile` / `_LoadObj` |
| `src/xanim/xanim.cpp` | `XAnimPrecache` | `XAnimFindData_FastFile` (×2) |
| `src/xanim/xanim.cpp` | `XAnimCreate` | `XAnimFindData_FastFile` |
| `src/xanim/xanim.cpp` | `XAnimSetupSyncNodes_r` | `XAnimFindData_FastFile` |
| `src/xanim/xmodel.cpp` | `XModelSurfsSetData` | `XModelPrecache_FastFile` |

Two adjacent LP64 defects shipped in the same commit, found while reading these
functions rather than by any sweep:

- `R_ShutdownImages` parked `GfxImage *` values in an `unsigned int v4[2049]`
  scratch array and read them back in a second loop — a 64-bit pointer through a
  32-bit slot. The array is now `GfxImage *v4[2049]`.
- `Image_IsProg` compared against `&g_imageProgs[30]` on an array of 29, one
  element past the end. It now uses `ARRAY_COUNT(g_imageProgs)`.

## Finding more of them

This family has no sweep of its own; it was found by reading. A regex for a
cast to `(int (__cdecl *)(` immediately wrapped in a pointer cast gets most of
the shape:

```sh
grep -rnE '\)\(\(int ?\(__cdecl \*\)\(' src/ --include=*.cpp
```

This returns nothing today — that is the family being closed, not the pattern
being wrong. It matches all 15 sites when run against `1f14435^`.

The same truncation happens anywhere a pointer-returning function is called
through an `int`-returning prototype, so it is worth re-running after importing
any new decompiler output.
