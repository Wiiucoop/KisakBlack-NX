# Fused and dropped parameters

The decompiler could not always recover a function's parameter list. Two
failures, with very different consequences on LP64.

**Fusion** — two adjacent 32-bit parameters read as one `__int64`. On x86 an
`__int64` argument occupies two 4-byte stack slots, so the fused parameter
aliased the two original ones exactly and the code worked by accident. On LP64 it
is one 64-bit register holding two truncated halves, and if either half was a
pointer, it is gone.

**Dropping** — a trailing parameter simply absent from the recovered signature.

## Closed

### `UI_RunMenuScript` — fused, fatal

Crashed in `Com_ParseExt` (`q_parse.cpp:466`, `data = *data_p`) reached through
`Item_RunScript` → `UI_RunMenuScript` → `String_Parse`.

The recovered form was:

```c
void __cdecl UI_RunMenuScript(int localClientNum, int contextIndex, __int64 args);
```

and its only caller packed two pointers into that `__int64` (`ui_shared.cpp:3011`):

```c
HIDWORD(v4) = (DWORD)s;      // the script text
LODWORD(v4) = (DWORD)&p;     // the parse cursor
UI_RunMenuScript(localClientNum, dc->contextIndex, v4);
```

Both `(DWORD)` casts truncate a 64-bit pointer to 32 bits. The callee then did
`String_Parse((const char **)args, …)`, so the pointer it parsed through was
`((uint32)s << 32) | (uint32)&p` — neither of the two.

The original signature came from **three independent sources that agree**:

1. the decompiler's own xref comments, in `ui_shared.h:513` and `:517` —
   `UI_RunMenuScript(int,int,char const * *,char const *)`;
2. the call site, which packs exactly a `const char **` and a `const char *`;
3. the body itself, which at `ui_main.cpp:1204` forwards the two halves as two
   *separate* arguments to `UI_Project_RunMenuScript` — a function whose
   signature survived decompilation intact:
   `(int, int, const char *name, const char **args, const char *actualScript)`.

Restored to:

```c
void __cdecl UI_RunMenuScript(int localClientNum, int contextIndex,
                              const char **args, const char *actualScript);
```

with the 11 `(const char **)args` casts in the body becoming plain `args`, the
single `(const char *)HIDWORD(args)` becoming `actualScript`, and the caller
becoming `UI_RunMenuScript(localClientNum, dc->contextIndex, &p, s)`.

## Known waste

### `UI_ServersSort` — fused, harmless

```c
void __cdecl UI_ServersSort(__int64 column);      // ui_server.h:17
```

Every caller passes `column | 0x100000000LL` — low half the column, high half a
constant 1 (`ui_main.cpp:1365`, `ui_server.cpp:102`, `:223`). The original was
almost certainly `(int column, int force)` or similar.

No pointer is involved, so nothing is truncated and the code is self-consistent
on both ABIs. The visible effect is in the body:

```c
if (column != sharedUiInfo.serverStatus.sortKey)
```

which compares `0x1_0000_000B` against the `sortKey` of `0xB`, so the guard is
always true and the server list re-sorts on every call. Wasted work in the menu,
not a defect. Left as is deliberately; fixing it means recovering what the second
parameter meant, which needs the original more than the port does.

## Open — dropped trailing parameters, 14 functions

Found by the xref cross-check described in [../README.md](../README.md). Each is
declared with fewer parameters than the decompiler's own xref signature for it:

| function | declared | original | header |
| --- | ---: | ---: | --- |
| `create_gjk_geom` | 3 | 5 | `physics/phys_colgeom.h:923` |
| `XModelReadSurface` | 4 | 6 | `gfx_d3d/r_xsurface_load_obj.h:12` |
| `Playlist_CategoryIsLocked` | 4 | 6 | `ui/ui_playlists.h:182` |
| `Phys_FindAndRenderBulletMesh` | 2 | 4 | `physics/phys_main.h:270` |
| `DeployWeapon` | 1 | 3 | `game/g_weapon.h:50` |
| `SV_MasterHeartbeat` | 1 | 2 | `server_mp/sv_main_pc_mp.h:28` |
| `SD_UnpauseVoice` | 1 | 2 | `sound/snd_driver_xaudio2.h:174` |
| `R_DrawBspDrawSurfs` | 2 | 3 | `gfx_d3d/r_draw_bsp.h:55` |
| `RB_GaussianFilterImage` | 3 | 4 | `gfx_d3d/rb_imagefilter.h:32` |
| `GlassSv_Touch` | 2 | 3 | `glass/glass_server.h:70` |
| `G_InitGame` | 4 | 5 | `game_mp/g_main_mp.h:373` |
| `Flame_Phys_Update_Items_PerStream` | 3 | 4 | `flame/flame_physics.h:71` |
| `CL_MapLoading` | 1 | 2 | `client_mp/cl_main_mp.h:862` |
| `TracePoint` | 2 | 6 | `glass/glass_client.h:91` |

None of these currently faults, and that is worth stating precisely: caller and
callee were *both* decompiled to the same reduced form, so they agree with each
other. What is lost is a behaviour — whatever the dropped argument controlled —
not memory safety.

The reason to track them anyway is the ABI. On x86 `__cdecl` every argument is on
the stack, so a caller passing one fewer argument than the callee reads leaves
the callee reading the adjacent stack slot: wrong, but deterministic, and often
the previous frame's data. **On ARM64 the first eight arguments are in `x0`-`x7`,
so the same disagreement reads a register nobody set for this call** — whatever
the last unrelated code left there. That is not deterministic and will not
reproduce the x86 misbehaviour, so any of these that gets one caller "fixed"
without the callee, or vice versa, fails in a way that looks unrelated to the
change.

`TracePoint` is almost certainly a false positive: the declared signature
(`const pointtrace_t *`, `trace_t *`) and the xref one (five floats and pointers)
have nothing in common, so the xref most likely belongs to a different function
of the same name.

## Related: pointers parked in `float` slots

Not parameter recovery, but the same root cause — on x86 a `float` local and a
pointer are both 4 bytes, so the decompiler reused a float slot to hold a
pointer. On LP64 the store truncates it to 32 bits and the later dereference goes
to a wild address. Three sites, all in the vehicle/wheel path:

| site | shape |
| --- | --- |
| `physics/phys_main.cpp:3436` | `LODWORD(v14.z) = (DWORD)phys_inv_multiply(...)`, then three `*(float *)LODWORD(v14.z)` reads — live and broken |
| `physics/phys_broad_phase.cpp:2792` | `LODWORD(v38) = (unsigned int)v2->m_rbvm->m_wheels.m_buffer`, then `*(_DWORD *)(LODWORD(v38) + 20)` — live |
| `physics/phys_main.cpp:3404` | `LODWORD(p0[3]) = (DWORD)&v30.z`, but `p0` is overwritten by `Phys_NitrousVecToVec3` immediately after — looks like a decompiler artefact of a spilled register, needs reading before it is touched |

**Both existing sweeps miss this family, for the same reason they missed
`nodeCmp`:**

- `scan.py` matches `*((T *)base + N)` with `base` an identifier chain. Here the
  base is `LODWORD(v14.z)` — a macro expanding to `*((_DWORD *)&(v14.z) + 0)`, an
  expression, not an identifier. No match.
- the `qsort` census never looks at function bodies at all.
- and even where `scan.py` does match, it only asks *which field* an offset lands
  on — never whether a 32-bit slot is being used to hold a pointer. That is the
  same blind spot documented in
  [truncated-pointer-loads.md](truncated-pointer-loads.md), of which this is a
  sub-family: there the pointer was *loaded* through a 32-bit type, here it is
  *stored* into one.

The grep that finds the storing form:

```sh
grep -rnE '(LODWORD|HIDWORD)\([^)]*\) *= *\((DWORD|unsigned int|_DWORD)\) *&?[A-Za-z_]' src/ --include=*.cpp
```

It returns five sites: the three above plus the two `UI_RunMenuScript` packing
lines, now fixed.
