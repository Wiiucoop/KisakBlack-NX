# Stub assets: the leading comma

A `.ff` fastfile does not only contain assets. It also contains **stubs**: an
asset the zone *references* but does not carry, left behind so the loader can
wire the reference up to whatever zone really holds it. A stub is marked by a
single byte — its name begins with `,` (44, `0x2C`).

So in `ui_mp.ff` this is not damage:

```
0x000033CD:  00 00 00 | 2C 77 68 69 74 65 00 |     ,white
```

It is a reference to the material `white`, which lives in `code_post_gfx_mp`.

## Where the engine knows this

One place, and only one: `DB_LinkXAssetEntry`, `src/database/db_registry.cpp:2434`.

```c
v4 = *name;
isStubAsset = v4 == 44;
if ( v4 == 44 )
    ++name;
type = newEntry->asset.type;
hash = DB_HashForName(name, type);
```

The comma is stripped before the name is hashed, so the lookup, the dedup and
the override check all run against the real name. Everything downstream of the
database sees `white`; nothing else in the engine strips a comma, and nothing
else needs to.

The consequence is a rule about ownership: **a stub's name is the database's
business alone**. Read `info.name` off a stub Material and hand it to a
by-name registration function and the comma goes with it, because that function
has no reason to expect one.

## What a loader owes a stub

A stub is inert data until something registers it. `Load_MaterialHandle`
(`src/database/db_load.cpp:2504`) reads the inline Material and then calls
`Load_MaterialAsset`, which is two steps in one line (`db_registry.cpp:685`):

```c
*material = DB_AddXAsset(ASSET_TYPE_MATERIAL, asset).material;
```

`DB_AddXAsset` resolves the stub — comma stripped — to the entry that really
holds the asset, and the assignment writes that entry back through **the slot
the stub was loaded from**. Every later reference in the zone is a
`DB_ConvertOffsetToAlias` that reads the same slot, so after the write-back
nothing in the zone points at the stub any more.

This is not special to materials. Every one of the 32 `Load_<T>Ptr` functions in
`db_load.cpp` ends the same way, so the rule is: **an inline asset is registered,
and the reference that carried it is repointed at what the database returned.**
A loader that copies the struct but skips either half leaves the zone wired to
an empty stub.

That is exactly what the `.kbz` path did. `tools/ffconv` allocated the inline
Material but never put it in the asset table, so `src/nx/nx_kbz.cpp` never
registered it and never had a header to redirect to. In `ui_mp` that left 1945
relocations pointing at a 208-byte Material that was zero in every field but its
name, while the real `white` — registered from `code_post_gfx_mp` — sat in the
pool with nothing referencing it. The two populations were disjoint: of the 367
materials in that zone's asset table, not one was the target of a relocation.

The fix is the same two halves. ffconv calls `z.addAsset` at each of the ten
inline sites that correspond to a `Load_<T>Asset` on PC, and `nx_kbz` rewrites
every relocation slot that pointed at an asset's block copy to the header
`DB_AddXAsset` returned. The redirect has to happen immediately after each
asset's own registration: `DB_AddXAsset` clones the struct into the pool, so a
pointer field inside an asset is frozen when that asset is registered, and a
sweep deferred to the end of the zone would fix block copies nobody reads.

## How this was found, and what it cost

Two sessions, chasing a byte that was never wrong.

A material name reached the renderer as `,white`, which read as `$white` with
one bit flipped (`0x24` → `0x2C`). That is a convincing amount of evidence for
memory corruption, and it was all wrong:

- `$white` is a real asset name — it is the **image** registered by
  `Image_Register("$white", ...)` at `src/gfx_d3d/r_image.cpp:1322` — so the
  wrong-bit reading had a plausible victim.
- `code_post_gfx_mp.kbz` contains `$white` three times and `,white` zero times.
  Grepping that one zone "confirmed" the file was clean and pointed the
  investigation at the on-device loader.
- The zones that actually carry the stub are `patch_mp`, `patch_ui_mp`, `ui_mp`
  and `common_mp`, one occurrence each. `common_mp` carries **both** — 26 clean
  `$white` and one `,white` — which also rules out any blanket transformation.
- Inflating `ui_mp.ff` settles it: `'$white' = 0`, `',white' = 1`. The byte is
  Treyarch's. `tools/ffconv` copied it faithfully and `src/nx/nx_kbz.cpp` copied
  it faithfully.

The structure confirms it independently. In `ui_mp.kbz` the name sits at block 4
`+0x3840`, `0xD0` = 208 bytes (`SZ_MATERIAL`) past a Material at `+0x3770` whose
every other field is zero, and several hundred relocations point at that
Material. A name-only struct that half the zone references is a stub, not a
corrupted asset.

If you are about to write a canary on asset names: a leading `,` is legal, and
rejecting it will fire on every stub in the game.
