# LP64 sweeps

This directory holds the standing census of **x86-ABI assumptions frozen into the
decompiled source**, the tools that produce it, and the findings that came out of
it.

The source in `src/` is decompiler output from a 32-bit x86 binary. Wherever the
decompiler could not recover a field name, a `sizeof`, or a prototype, it wrote
down the *number it observed on x86* instead. Every one of those numbers is a
silent defect on LP64 (AArch64, 64-bit pointers), and none of them are compile
errors: the code builds clean and then reads or writes the wrong bytes.

Four families, all found the same way:

| family | what the decompiler wrote | findings |
| --- | --- | --- |
| pointer-offset reads | `*((unsigned int *)cent + 201)` for a named field | [findings/pointer-offset-reads.md](findings/pointer-offset-reads.md) |
| `qsort` element sizes | the x86 `sizeof(T)` as a literal | [findings/qsort-element-sizes.md](findings/qsort-element-sizes.md) |
| function-pointer casts | `(int (__cdecl *)(...))` over a pointer-returning function | [findings/function-pointer-casts.md](findings/function-pointer-casts.md) |
| command-buffer reservations | the x86 `sizeof` of a render command | [findings/pointer-offset-reads.md](findings/pointer-offset-reads.md) |
| truncated pointer loads | `unsigned int` where a pointer was meant | [findings/truncated-pointer-loads.md](findings/truncated-pointer-loads.md) |
| fused / dropped parameters | two 32-bit parameters read as one `__int64` | [findings/fused-parameters.md](findings/fused-parameters.md) |

The last of those has **no sweep** — it is the one that has produced the hard
crashes, and both existing sweeps are structurally blind to it. Read that page
before trusting a clean run here.

## The method

The core trick is that **the same headers can be compiled twice**, once for the
real target and once for a 32-bit ABI that stands in for the original x86 one.
Everything else follows from comparing the two.

```
              src/**.{cpp,h}
                    |
            (1) scan.py                 regex for *((T *)base + N)
                    |  hits.tsv
            (2) built.py                drop TUs with no object file
                    |  hits_built.tsv
            (3) typeprobe.py            ask the compiler for each base's type
                    |  types.tsv
            (4) join.py                 attach the type to each hit
                    |  joined.tsv
                    |
   src/**.h --> (5) layouts.sh --> layouts_lp64.o + layouts_ilp32.o  (DWARF)
                    |
            (6) classify.py             same field on both ABIs? OK : BAD
            (6) scalar_groups.py        group what classify cannot judge
```

### 1. Scan — `scan.py`

A raw dword read of a named field looks like `*((unsigned int *)cent + 201)`.
`scan.py` matches that shape across `src/`, skipping vendored trees, and records
the cast type, the base expression, the index `N`, and the x86 byte offset the
index implies (pointer = 4). Output: `hits.tsv`.

This is a text scan, so it over-collects: an index into a genuine `float` array
looks identical to an index into a struct. Stages 3-6 exist to sort that out.

### 2. Filter to what is built — `built.py`

Much of `src/` is dead on this target. Any TU with no object file under
`build-nx/CMakeFiles/KisakBlack.dir/` never reaches the binary, and its hits are
noise. **Run a full build first**, or this stage under-reports. Output:
`built.txt`, `hits_built.tsv`.

### 3. Learn each base's real type — `typeprobe.py`

The decisive question for a hit is *what is `cent`* — and only the compiler
knows. `typeprobe.py` copies each source file, rewrites every hit from

```c
*((unsigned int *)cent + 201)
```

to

```c
*((sizeof(NxProbeT<decltype(cent)>), 0), (unsigned int *)cent + 201)
```

and compiles it with `-fsyntax-only -fmax-errors=0`. `NxProbeT` is declared but
never defined, so the compiler reports the type on that exact line:

```
error: invalid application of 'sizeof' to incomplete type 'NxProbeT<centity_s*>'
```

Line numbers are preserved, so each diagnostic maps back to its hit. It reuses
the real `CXX_FLAGS`/`CXX_INCLUDES`/`CXX_DEFINES` from `flags.make`, minus `-w`
(which would suppress the very diagnostic being read back) and `-g`. Output:
`types.tsv` as `file, line, column, type`.

### 4. Join — `join.py`

`types.tsv` is keyed by column, `hits.tsv` is not, so the two are matched
positionally: the Nth type reported on a line belongs to the Nth hit scanned on
that line. Hits the compiler never reported on get `?`. Output: `joined.tsv`.

### 5. Both layouts at once — `layouts.sh` + `layouts.cpp`

`layouts.cpp` declares one variable per struct under study, so `-g` emits each
one's full field table. It is compiled twice from the *same headers*:

- `-mabi=lp64`  — the real target,
- `-mabi=ilp32` — 32-bit pointers, standing in for the original x86 ABI.

Nothing is linked or run; this is a layout-only build. The one accommodation is
that the ILP32 pass uses a patched copy of `src/nx/compat`: in ILP32 `long` and
`LONG` are the same type, so the `(volatile long *)` `Interlocked*` overloads
collide. `layouts.sh` `#if`s them out in a scratch copy rather than teaching the
shim about a second ABI.

To add a struct to the census, add a line to `layouts.cpp` and rerun.

### 6. Judge — `classify.py`, `scalar_groups.py`

`dwarf.py` reads `readelf --debug-dump=info` from both objects and flattens each
struct to leaf fields, descending through typedefs, nested structs and arrays.
`classify.py` then asks, per `(struct, cast, N)` group: *does the x86 byte offset
land on the same leaf field, at the same sub-offset, as the LP64 byte offset?*

- **OK** — same field. The literal survived, usually because nothing with a
  pointer sits in front of it.
- **BAD** — different field. Every site in the group is reading the wrong bytes.

`classify.py` can only rule on casts whose base is a struct it has layouts for.
Everything else — `char *`, `float *`, `long long int *`, and the hits with no
reported type — goes to `scalar_groups.py`, which groups them by file and by
`(base type, cast, base)` so they can be read by hand: ~1100 scattered lines
become a few hundred decisions.

`query.py` is the interactive version, for checking one struct at a time. It also
validates the ILP32 stand-in: it compares `dwarf.size()` of each ILP32 struct
against the `// sizeof=0x...` comment the decompiler left on the declaration, so
a mismatch means the ILP32 layout is *not* a faithful stand-in for x86, and any
verdict about that struct has to be thrown out.

```
>>> query.check('centity_s')
'centity_s: ilp32=808 x86comment=808 lp64=856 OK'
>>> query.q('centity_s', 201, 4, 4)     # x86 word 201 == byte 804
('BAD', '?{clientFlags|?} @804', 'nextSlideFX @804')
```

`ctx.py` takes `file:line@var` and prints where `var` got its value, for reading a
site without opening the file.

## A second source of truth: the decompiler's own xrefs

The pipeline above compares the source against *itself* under two ABIs. There is
a second, completely independent source in the tree, and it is the one that found
the `UI_RunMenuScript` crash.

Scattered through the headers are the decompiler's cross-reference comments:

```c
    modInfo_t modList[64];              // XREF: UI_FeederItemText_Mods+C/r
                                        // UI_RunMenuScript(int,int,char const * *,char const *)+5FE/r
```

Those carry the **original parameter list**, recorded from the binary's own
symbol data — and they were written independently of whatever the decompiler
managed to recover for the function itself. So where the xref and the declaration
disagree, the declaration lost something:

```c
// what the header declares
void __cdecl UI_RunMenuScript(int localClientNum, int contextIndex, __int64 args);
// what the xref says it was
   UI_RunMenuScript(int, int, char const * *, char const *)
```

Two pointers fused into one `__int64`. On x86 that worked by accident — an
`__int64` argument occupies two 4-byte stack slots, which aliased the two
originals exactly. On LP64 it is one register holding two truncated halves.

`xrefsigs.py` does this comparison across every header: it harvests the xref
signatures, parses the real declarations, and prints each function declared with
fewer parameters than its xref, flagging those whose declaration contains a
64-bit parameter — the fusions, which are the ones that bite.

```sh
python xrefsigs.py
```

Two cautions, both learned from the output:

- **A gap is a lead, not a verdict.** Same-name collisions happen: `TracePoint`
  is declared `(const pointtrace_t *, trace_t *)` and its xref says six floats
  and pointers, because the xref belongs to a different function of that name.
  Confirm the xref actually names the function in front of you.
- **A dropped trailing parameter usually is not a bug.** Caller and callee were
  both decompiled to the same reduced form, so they agree; what is lost is a
  behaviour, not memory safety. The fusions are the dangerous half, because the
  body still reaches for both halves with `LODWORD`/`HIDWORD`.

This technique generalises past parameter lists. The xref comments also record
return types and the names of functions that were inlined away, and none of that
came from the same guesswork that produced the code — which is exactly what makes
it worth cross-checking against.

## Closing the class one subsystem at a time

The sweeps above find defects by *searching*. For the largest class of all —
pointers cast through 32-bit integers — there is no need to search: **the
compiler already diagnoses every one of them**, and `-w` in the build flags has
been hiding them.

Measured across the Switch build, with `-w` removed:

```
TUs analysed:                                    666
files with at least one pointer<->int truncation: 460
sites:                                          3654
```

Worst offenders: `cscr_compiler.cpp` 453, `com_expressions_eval.cpp` 308
(graduated), `cscr_evaluate.cpp` 169, `physics_system_internal.cpp` 79.

So the plan is not another census. It is a ratchet: **a per-file allowlist of
sources that build with warnings on**, extended one subsystem at a time. Each
file that graduates can never silently regress, because the truncation is a hard
error from then on.

### How it works

`cmake/switch.cmake` holds `NX_SANITIZED_SOURCES`. Everything *not* in that list
gets `-w -fpermissive -Wno-narrowing` applied **per source file**; the sanitised
list gets nothing, so the default diagnostics apply.

It has to be done that way round. `-w` cannot be undone by a later flag — adding
`-Werror=…` after it on the same command line changes nothing — so the flag is
attached to the files that still need it, rather than to the target.

What makes a truncation fatal differs by language, and both were verified against
the real toolchain:

- **C++** — `(int)somePointer` is *ill-formed*; it compiles only because of
  `-fpermissive`. Drop that and it is already an error:
  `cast from 'const char*' to 'int' loses precision`. No `-Werror` needed.
- **C** — the relevant options, `-Wpointer-to-int-cast` and
  `-Wint-to-pointer-cast`, are **C-only**; passing them to `g++` just warns that
  they are not valid for C++. They are requested explicitly as errors for
  sanitised `.c` files.

CMake prints the state at configure time:

```
-- LP64 warning policy: 11 sanitised sources build with warnings on, 2005 still build with -w
```

### Graduating a file

1. Add it to `NX_SANITIZED_SOURCES` in `cmake/switch.cmake`.
2. Reconfigure and build. Every truncation in it is now an error.
3. Fix them — a pointer stored in an `int` field needs the field widened, not the
   cast silenced. `(uintptr_t)` is correct only where the value provably is not
   an address: a handle, or small indices packed into a pointer-shaped context.
4. Keep the list additive. **Removing a file from it is a regression**, and the
   configure-time count is there to make that visible.

The list starts at `src/nx/` — the port's own code, which should hold to a higher
standard than the decompiled tree it wraps. Turning it on there immediately
surfaced four real problems, which is the point:

- a genuine LP64 truncation in `snd_driver_xaudio2.h:82`, where two small indices
  packed into a `void *` context were unpacked through `unsigned int`;
- `GlobalAlloc` returning `void *` where C++ wants an explicit conversion;
- `#define rand() nx_rand()` in `nx_prefix.h` colliding with libstdc++'s
  `std::rand()` call inside `std::random_shuffle` — latent, because nothing
  instantiates that template yet.

### Graduated so far

| file | diagnostics | what they were |
| --- | ---: | --- |
| `src/nx/**` | 4 | see above |
| `src/universal/com_expressions_eval.cpp` | 305 | 297 pointer stores through `operandInternalDataUnion::intVal`, plus the reads and guards around them |

`com_expressions_eval.cpp` was the first decompiled file through the ratchet, and
it behaved as advertised: 298 errors and 7 warnings on the first build, 297 of
them the single store shape

```c
result.internals.intVal = (int)CopyTempString(...);   // -> .string = ...
```

which is mechanical. The residue is what makes the exercise worth writing down:

- **The crash was a read, not a store.** `GetSourceString` returned
  `(char *)operand.internals.intVal`, so every string operand came back with its
  top half gone — that is the null `strlen` in `LocalizeString`. The compiler
  reported it as a *warning* (`-Wint-to-pointer-cast`), not an error, because
  widening an `int` to a pointer is legal. **On a graduated file, read the
  warnings too**; only the store direction is fatal on its own.
- **The two guards had to be fixed by hand.** Both tested
  `!data->internals.intVal` while their own assert message names
  `internals.string`. Testing half a pointer is perfectly well-typed, so nothing
  diagnosed it — same blind spot as a wrong struct offset.
- **It reached outside the file.** A sanitised TU compiles its headers with
  warnings on too, so `bdArray<T>`'s `return (int)this;` (x86 thiscall residue,
  never read by any caller) became an error, and
  `Demo_GetSegmentInformation` turned out to be declared `clipSegment *` for a
  function that only ever returns strings — the decompiler's guess, valid on x86
  because `clipSegment::name` is at offset 0. Both were retyped rather than cast
  away.
- **What it left behind.** `cscr_animtree.h:36,48` still warn: the `scr_anim_s`
  and `scr_animtree_t` `int` constructors reconstitute a pointer from
  `Scr_Value::u.intValue`. That is the clientscript VM's truncation, not this
  file's, and it closes when that subsystem graduates.

### Why this beats a census

A sweep tells you how many defects exist today. The allowlist makes each one
*stay* fixed, and it costs nothing to maintain: the build is the check. The
sweeps in this directory remain useful for the classes the compiler cannot see —
a wrong struct offset is perfectly well-typed — but for this class they are the
wrong tool. `com_expressions_eval.cpp` has been through it; the clientscript
group — `cscr_compiler.cpp`, `cscr_evaluate.cpp`, `cscr_vm.cpp` — is the next
subsystem, and `cscr_animtree.h` is already pointing at it.

## Running it

Build first — stages 2 and 3 both read the build tree.

```sh
make -C build-nx -j8

cd docs/lp64-sweeps/tools
python scan.py            # -> hits.tsv
python built.py           # -> built.txt, hits_built.tsv
python typeprobe.py       # -> types.tsv        (slowest stage; compiles ~120 TUs)
python join.py            # -> joined.tsv
sh     layouts.sh         # -> layouts_{lp64,ilp32}.o
python classify.py        # the verdicts
python scalar_groups.py   # -> scalar_groups.txt
python qsort_scan.py      # -> qsort.tsv, and the 72-site table
python qsort_decls.py     # nearest declaration for each qsort site
python xrefsigs.py        # xref signatures vs. real declarations (independent)
```

`xrefsigs.py` reads only headers, so it needs no build and can be run first.

Intermediates go to `build-nx/lp64-work/`, never into the source tree; override
with `$LP64_WORK`. Toolchain location comes from `$DEVKITA64`, default
`/opt/devkitpro/devkitA64/bin`. Run the scripts from `tools/` — they import
`lp64env` and `dwarf` as siblings.

## `data/`

A snapshot of the output, so a later run has something to diff against. It is
regenerated output, not a source of truth — rerun the pipeline before trusting
it.

| file | what |
| --- | --- |
| `hits.tsv` | every `*((T *)base + N)` in `src/` |
| `types.tsv` | compiler-reported base types |
| `joined.tsv` | hits with their base type attached |
| `classify.txt` | the OK/BAD verdicts |
| `scalar_groups.txt` | the non-struct remainder, grouped for manual reading |
| `qsort.tsv` | every `qsort` call and its element size argument |

## What this does not catch

The scan keys on one syntactic shape, `*((T *)base + N)`. It will not find:

- `memcpy`/`memset` with a literal x86 struct size — that family was closed
  separately, in `e71ff0f`, for the 33 asset sizes in `db_assetnames.cpp`;
- a literal offset added to a `char *` before the cast;
- an x86 size baked into an *allocation* rather than an access. The
  `R_GetCommandBuffer` family is exactly this, and was found by reading, not by
  the scan;
- structs the decompiler laid out wrong in the first place;
- a pointer *loaded* through a 32-bit type — `*(unsigned int *)(*(unsigned int *)arg + 12)`.
  The offset is inside the dereference and the base is an expression, so the
  regex does not match it; and even where it does match, the pipeline only ever
  asks *which field* an offset lands on, never whether a 32-bit read is being
  used to hold a pointer. See
  [findings/truncated-pointer-loads.md](findings/truncated-pointer-loads.md).

The `qsort` sweep has its own blind spot, of a different kind: it reads the
element-size argument and stops there. A call that passes `sizeof(T)` is recorded
as correct — which it is, as far as the stride goes — but the **comparator** is a
separate function reached through a function pointer, and nothing in the pipeline
follows it. `nodeCmp` crashed from inside a `qsort` the census had already cleared.

The ILP32 stand-in is also not x86: it matches on pointer width and, for this
codebase's structs, on layout — `query.py`'s `check()` is what confirms that per
struct — but it is a stand-in, not the original ABI. Alignment of `long double`,
and anything where x86 and ILP32 AArch64 differ for reasons other than pointer
size, is outside what these verdicts cover.
