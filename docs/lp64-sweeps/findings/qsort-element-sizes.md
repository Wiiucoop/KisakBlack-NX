# qsort element sizes

`qsort` takes the element size as a runtime argument, so the decompiler had a
number to write down rather than a `sizeof` to recover. It wrote the number it
saw on x86.

That literal is correct only while the element type contains no pointer. When it
does, `qsort` on LP64 strides by the x86 size across an array laid out at the
LP64 size: it reads fields from the middle of neighbouring elements, and — because
`qsort` swaps — writes them back shuffled. The array is left corrupted, not merely
mis-ordered.

**100 `qsort` calls in `src/`; 72 pass a literal instead of `sizeof`.** Those 72
are the table below. The other 28 already say `sizeof(T)` and are correct by
construction.

## How to read a row

The verdict column is filled in only where the element type could be recovered
from the nearest declaration of the base *and* its LP64 `sizeof` probed:

- **ok** — the element type holds no pointer, so the x86 literal is still right.
  Leave it alone, or convert to `sizeof(T)` for clarity.
- **`n -> m`** — confirmed mismatch. The call strides `n` bytes over `m`-byte
  elements.
- **unresolved** — `qsort_decls.py` could not recover a named element type,
  usually because the base is a decompiler temporary (`v17`, `v86`, `pos3`) or a
  bare scalar array. Most of these are fine — a `2` over `unsigned __int16`, a `8`
  over `float[2]` — but each needs its own look. The recipe is: find the base's
  real type (`ctx.py file:line@base` shows where it is assigned), then compare
  `sizeof` of that type against the literal.

The four confirmed mismatches, all live:

| site | literal | element | LP64 | effect |
| --- | ---: | --- | ---: | --- |
| `src/clientscript/cscr_variable.cpp:116` | 140 | `ThreadDebugInfo` | 272 | strides at half the element size |
| `src/EffectsCore/fx_profile.cpp:38` | 28 | `FxProfileEntry` | 32 | |
| `src/gfx_d3d/r_material_load_obj.cpp:1481` | 8 | `MaterialShaderArgument` | 16 | material load path |
| `src/gfx_d3d/r_material_load_obj.cpp:4616` | 8 | `MaterialShaderArgument` | 16 | material load path |

`MaterialShaderArgument` is the one to fix first: it is on the material load
path, it runs for every material in every zone, and it is off by a clean factor
of two.

## All 72 sites

| site | literal | base | element type | LP64 sizeof | verdict | comparator |
| --- | ---: | --- | --- | ---: | --- | --- |
| `src/bgame/bg_emblems.cpp:230` | 2 | `best->ids` | — | ? | unresolved | `ResultSort` |
| `src/bgame/bg_weapons_def.cpp:217` | 8 | `bg_weaponVariantNameHashTable` | `WeaponVariantDefHash` | 8 | ok | `BG_WeaponVariantNameHashCompare` |
| `src/cgame/cg_draw_debug.cpp:879` | 240 | `info` | — | ? | unresolved | `CG_SoundCompares[snd_drawSort->cur` |
| `src/cgame/cg_draw_names.cpp:670` | 16 | `drawNameEntities` | — | ? | unresolved | `compareEntityDist` |
| `src/cgame/cg_world.cpp:1778` | 8 | `intervals` | — | ? | unresolved | `cmpr` |
| `src/clientscript/cscr_compiler.cpp:5022` | 8 | `pos3` | — | ? | unresolved | `CompareCaseInfo` |
| `src/clientscript/cscr_evaluate.cpp:110` | 8 | `gScrEvaluateGlob[inst].archivedCanon` | — | ? | unresolved | `CompareCanonicalStrings` |
| `src/clientscript/cscr_variable.cpp:116` | 140 | `infoArray` | `ThreadDebugInfo` | 272 | **140 -> 272** | `ThreadInfoCompare` |
| `src/clientscript/cscr_variable.cpp:274` | 16 | `infoArray` | — | ? | unresolved | `VariableInfoFileNameCompare` |
| `src/clientscript/cscr_variable.cpp:279` | 16 | `infoArray` | — | ? | unresolved | `VariableInfoFunctionCompare` |
| `src/clientscript/cscr_variable.cpp:284` | 16 | `infoArray` | — | ? | unresolved | `CompareThreadIndices` |
| `src/clientscript/cscr_variable.cpp:297` | 16 | `infoArray` | — | ? | unresolved | `VariableInfoFileLineCompare` |
| `src/clientscript/cscr_variable.cpp:299` | 16 | `infoArray` | — | ? | unresolved | `VariableInfoCountCompare` |
| `src/client_mp/cl_main_pc_mp.cpp:2230` | 376 | `s_quickmatchCandidates` | — | ? | unresolved | `CL_QuickMatch_CompareServers` |
| `src/DynEntity/DynEntity_load_obj.cpp:481` | 84 | `base` | — | ? | unresolved | `DynEnt_CompareEntities` |
| `src/EffectsCore/fx_profile.cpp:38` | 28 | `entryPool` | `FxProfileEntry` | 32 | **28 -> 32** | `FX_CompareProfileEntries` |
| `src/EffectsCore/fx_profile.cpp:276` | 8 | `base` | — | ? | unresolved | `FX_ComparePriorityDebugEntries` |
| `src/game/actor_senses.cpp:466` | 8 | `check` | — | ? | unresolved | `compare_sentient_sort` |
| `src/game/pathnode_load_obj.cpp:411` | 2 | `gameWorldCurrent->path.nodeForChainN` | — | ? | unresolved | `compare_pathnodes` |
| `src/game_mp/g_scr_main_mp.cpp:15055` | 12 | `scored_spawn_points` | — | ? | unresolved | `sort_scored_spawn_point_vectors_as` |
| `src/gfx_d3d/r_bsp_load_obj.cpp:2231` | 56 | `&s_world.heroLights[firstLight]` | — | ? | unresolved | `R_HeroLightSorter` |
| `src/gfx_d3d/r_init.cpp:1151` | 16 | `dx.displayModes` | — | ? | unresolved | `R_CompareDisplayModes` |
| `src/gfx_d3d/r_material_load_obj.cpp:1481` | 8 | `args` | `MaterialShaderArgument` | 16 | **8 -> 16** | `Material_CompareShaderArgumentsFor` |
| `src/gfx_d3d/r_material_load_obj.cpp:4616` | 8 | `localArgs` | `MaterialShaderArgument` | 16 | **8 -> 16** | `Material_CompareShaderArgumentsFor` |
| `src/gfx_d3d/r_material_load_obj.cpp:6032` | 16 | `newMtl->textureTable` | — | ? | unresolved | `CompareHashedMaterialTextures` |
| `src/gfx_d3d/r_material_load_obj.cpp:6037` | 32 | `newMtl->localConstantTable` | — | ? | unresolved | `CompareHashedMaterialTextures` |
| `src/gfx_d3d/r_material_load_obj.cpp:7296` | 12 | `textureTable` | `MaterialTextureDefRaw` | 12 | ok | `CompareRawMaterialTextures` |
| `src/gfx_d3d/r_material_load_obj.cpp:7309` | 20 | `constantTable` | `MaterialConstantDefRaw` | 20 | ok | `CompareRawMaterialTextures` |
| `src/gfx_d3d/r_staticmodel_load_obj.cpp:51` | 2 | `tree->smodelIndexes` | — | ? | unresolved | `CompareStaticModels` |
| `src/glass/glass_client.cpp:420` | 24 | `outlineEdges` | `OutlineEdge` | 24 | ok | `compareOutlineEdges` |
| `src/live/live_combatrecord.cpp:546` | 20 | `sortedItemList` | `sortedItemsData_t` | 20 | ok | `LiveCombatRecord_CompareItemsBySta` |
| `src/live/live_combatrecord.cpp:619` | 20 | `s_sortedItemList` | — | ? | unresolved | `LiveCombatRecord_CompareItemsBySta` |
| `src/live/live_combatrecord.cpp:624` | 20 | `s_otherPlayerSortedItemList` | — | ? | unresolved | `LiveCombatRecord_CompareItemsBySta` |
| `src/live/live_combatrecord.cpp:651` | 20 | `sortedItemList` | `sortedItemsData_t` | 20 | ok | `LiveCombatRecord_CompareMatchCount` |
| `src/live/live_combatrecord.cpp:677` | 20 | `sortedItemList` | `sortedItemsData_t` | 20 | ok | `LiveCombatRecord_CompareMatchCount` |
| `src/live/live_leaderboard.cpp:1230` | 152 | `lb->userStats.m_leaderBoardEntries` | — | ? | unresolved | `LB_CompareXUserStatsRowRanks` |
| `src/live/live_meetplayer.cpp:119` | 48 | `metPlayersXuidinfo[localControllerIn` | — | ? | unresolved | `LiveMeetPlayer_MetPlayerCompare` |
| `src/live/live_stats.cpp:2445` | 20 | `s_personalBests[controllerIndex]` | — | ? | unresolved | `LiveStats_ComparePersonalBests` |
| `src/live/live_stats.cpp:2517` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeComparePercenta` |
| `src/live/live_stats.cpp:2524` | 36 | `s_sortedAttachmentsList` | — | ? | unresolved | `LiveStats_ChallengeComparePercenta` |
| `src/live/live_stats.cpp:2531` | 36 | `s_sortedWeaponGroupList` | — | ? | unresolved | `LiveStats_ChallengeComparePercenta` |
| `src/live/live_stats.cpp:2541` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeComparePercenta` |
| `src/live/live_stats.cpp:2549` | 36 | `s_sortedGlobalChallengeList` | — | ? | unresolved | `LiveStats_ChallengeComparePercenta` |
| `src/live/live_stats.cpp:2597` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareNumToTar` |
| `src/live/live_stats.cpp:2604` | 36 | `s_sortedAttachmentsList` | — | ? | unresolved | `LiveStats_ChallengeCompareNumToTar` |
| `src/live/live_stats.cpp:2611` | 36 | `s_sortedWeaponGroupList` | — | ? | unresolved | `LiveStats_ChallengeCompareNumToTar` |
| `src/live/live_stats.cpp:2621` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareNumToTar` |
| `src/live/live_stats.cpp:2629` | 36 | `s_sortedGlobalChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareNumToTar` |
| `src/live/live_stats.cpp:2650` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareReward` |
| `src/live/live_stats.cpp:2657` | 36 | `s_sortedAttachmentsList` | — | ? | unresolved | `LiveStats_ChallengeCompareReward` |
| `src/live/live_stats.cpp:2664` | 36 | `s_sortedWeaponGroupList` | — | ? | unresolved | `LiveStats_ChallengeCompareReward` |
| `src/live/live_stats.cpp:2674` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareReward` |
| `src/live/live_stats.cpp:2682` | 36 | `s_sortedGlobalChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareReward` |
| `src/live/live_stats.cpp:2706` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareDefault` |
| `src/live/live_stats.cpp:2713` | 36 | `s_sortedAttachmentsList` | — | ? | unresolved | `LiveStats_ChallengeCompareDefault` |
| `src/live/live_stats.cpp:2720` | 36 | `s_sortedWeaponGroupList` | — | ? | unresolved | `LiveStats_ChallengeCompareDefault` |
| `src/live/live_stats.cpp:2730` | 36 | `s_sortedChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareDefault` |
| `src/live/live_stats.cpp:2738` | 36 | `s_sortedGlobalChallengeList` | — | ? | unresolved | `LiveStats_ChallengeCompareDefault` |
| `src/live/live_storage.cpp:2392` | 2104 | `fsTask->descriptors` | — | ? | unresolved | `LiveStorage_FileShareSortComparato` |
| `src/live/live_storage.cpp:2398` | 2104 | `fsTask->descriptors` | — | ? | unresolved | `LiveStorage_FileShareSortComparato` |
| `src/live/live_storage.cpp:3784` | 200 | `ratingTask->outStatsInfo` | — | ? | unresolved | `LiveStorage_FileShare_VoteRankComp` |
| `src/live/live_win.cpp:1729` | 16 | `s_recentServers` | — | ? | unresolved | `compareRecentServers` |
| `src/physics/phys_main.cpp:2460` | 8 | `v17` | — | ? | unresolved | `SortSModelsByDist` |
| `src/physics/phys_main.cpp:2984` | 8 | `smodel_debug_infos` | — | ? | unresolved | `SortSModelsByDist` |
| `src/qcommon/com_bsp_load_obj.cpp:783` | 12 | `comWorld.burnableCells` | — | ? | unresolved | `Com_BurnableCellSort` |
| `src/qcommon/mem_track.cpp:1139` | 152 | `sorted_mem_track` | — | ? | unresolved | `mem_track_compare` |
| `src/server/sv_autoplaylist.cpp:210` | 8 | `playlists` | — | ? | unresolved | `comparePlaylists` |
| `src/ui/ui_main_pc.cpp:647` | 32 | `info->lines[1]` | — | ? | unresolved | `ScoreBar_CompareScores` |
| `src/universal/com_stringtable_obj.cpp:200` | 2 | `table->cellIndex` | — | ? | unresolved | `CellCompare` |
| `src/universal/curve.cpp:349` | 36 | `this` | — | ? | unresolved | `cCurve::CurveSortCompare` |
| `src/xanim/xanim_load_obj.cpp:610` | 2 | `base` | — | ? | unresolved | `XAnimCompareQuatParts` |
| `src/xanim/xanim_load_obj.cpp:655` | 2 | `v86` | — | ? | unresolved | `XAnimCompareTransParts` |

Regenerate with `python qsort_scan.py` (the census) and `python qsort_decls.py`
(the declaration for each base); raw output lands in `build-nx/lp64-work/qsort.tsv`
and is snapshotted at [`../data/qsort.tsv`](../data/qsort.tsv).
