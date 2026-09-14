# Pointers loaded through a 32-bit type

A pointer read back through `unsigned int` instead of a pointer type. On x86 the
two are the same width and the code works; on LP64 the load keeps the low 32 bits
and throws the rest away, so the next dereference goes to a wild address.

This family is **not covered by either sweep**, and it is the one that has
produced the hard crashes so far.

## The shape

```c
*(unsigned int *)(*(unsigned int *)arg + 12)
 ^                ^                    ^
 |                |                    x86 byte offset of the field
 |                truncates the element pointer to 32 bits
 reads the field at that (now bogus) address
```

Two independent x86 assumptions in one expression. The inner one is fatal on its
own — the outer offset never gets a chance to be wrong.

## Sites found so far

### `nodeCmp` — `src/qcommon/huffman.cpp:94` (fixed)

Crashed in `Huff_BuildFromData` → `Huff_initNode` → `qsort` → `nodeCmp`, the
first time the huffman tree is built.

`Huff_BuildFromData` sorts `nodetype *heap[256]`, so each comparator argument is
a `nodetype **`. The decompiled comparator read it as `unsigned int`:

```c
return *(unsigned int *)(*(unsigned int *)left  + 12)
     - *(unsigned int *)(*(unsigned int *)right + 12);
```

`nodetype` is `{ nodetype *left, *right, *parent; int weight; int symbol; }` —
three pointers before the scalar tail:

| | x86 | LP64 |
| --- | ---: | ---: |
| `sizeof(nodetype)` | 20 | 32 |
| `offsetof(weight)` | 12 | 24 |

So `+ 12` now lands in the upper half of `right`. But the crash came earlier: the
`nodetype *` was already truncated to 32 bits before the offset was added.

Fixed by reading the argument as what it is:

```c
    const nodetype *lnode = *(const nodetype *const *)left;
    const nodetype *rnode = *(const nodetype *const *)right;

    return lnode->weight - rnode->weight;
```

### `BG_UnlockablesCompareItemsBySortKey` (fixed in `c12c1b9`)

Identical shape over `itemInfo_t *`, with the sort key at hardcoded x86 offset
264 (`sizeof` 296 → 320, `sortKey` 264 → 288, so 264 landed inside
`defaultClass[19]`). Fixed the same way.

### Unswept remainder

The strict shape — a 32-bit load whose result is immediately used as a base
address — appears **68 times across 8 files**:

| file | sites |
| --- | ---: |
| `src/gfx_d3d/rb_postfx.cpp` | 30 |
| `src/xanim/xanim_load_obj.cpp` | 21 |
| `src/physics/physics_system_internal.cpp` | 4 |
| `src/gfx_d3d/r_water_sim.cpp` | 3 |
| `src/physics/phys_main.cpp` | 2 |
| `src/qcommon/threads.cpp` | 1 |
| `src/clientscript/cscr_compiler.cpp` | 1 |
| `src/DynEntity/DynEntity_client.cpp` | 1 |

Spot-checked, the two large groups are genuine: both park a pointer in a `_BYTE`
array (`_BYTE iPass[76]`, `dest[8 * animPartIndex + 128]`) and reload it through
`unsigned int`. Not every one of the 68 is necessarily a live defect — some may
be on dead paths, and a few are commented out — but each needs the same check.

```sh
grep -rnE '\*\((unsigned int|_DWORD|int) \*\)\(\*\((unsigned int|_DWORD|int) \*\)' src/ --include=*.cpp
```

That grep only finds the *double-dereference* spelling. The class is wider than
its spelling: any `unsigned int` variable, struct field, or array slot that holds
a pointer belongs to it, and those look like ordinary integer code.

## Why both sweeps missed it

Worth understanding, because the same gap will swallow the next one.

**The qsort sweep looked at the wrong half of the call.** `huffman.cpp` *was* in
the census — `data/qsort.tsv` lines 87-88 — but in the 28 calls that pass
`sizeof(...)`, not the 72 that pass a literal:

```c
qsort(heap, 0x100u, sizeof((heap)[0]), nodeCmp);
```

That classification was correct on its own terms. The stride genuinely is right:
`sizeof(heap[0])` is 8 on LP64. The sweep asked "does `qsort` step the right
number of bytes?" and the answer was yes. It never asked whether the
**comparator** could read the element it was handed — and the comparator is a
different function, reached through a function pointer, which no stage of the
pipeline follows.

A correct element size is not evidence of a correct sort. Those are two
independent questions about the same call, and only one of them was being asked.

**The pointer-offset scan missed it on spelling.** `scan.py` matches

```
*( (T *) base + N )
```

with `base` an identifier chain. `nodeCmp` is written

```
*( (T *) ( *(T *)left + 12 ) )
```

— the offset is *inside* the dereference, and the base is an expression, not an
identifier. Verified: the regex returns no match on that line, and `huffman.cpp`
has **0 hits** in `data/hits.tsv`.

**And the deeper gap:** even where the scan does match, it treats
`*(unsigned int *)p` as "reads a `uint`" and only ever asks *which field* that
offset lands on. It never asks whether a 32-bit read is being used to load a
**pointer** — which is this whole class, and which stays invisible no matter how
many structs get added to `layouts.cpp`.

### What would have caught it

Two cheap additions, both worth having before the next subsystem:

1. **Sweep comparators, not just calls.** `qsort.tsv` already records the
   comparator name for all 100 calls. Resolve each name to its definition and
   check the body for a 32-bit load of an argument. That is a bounded list and it
   covers exactly the functions where this defect is fatal.
2. **Widen `scan.py` to the double-dereference spelling** so
   `*(T *)(*(T *)x + N)` lands in `hits.tsv` alongside `*((T *)x + N)`. The type
   probe already knows how to type `x`; a base typed `T **` read through
   `unsigned int` is mechanically detectable.
