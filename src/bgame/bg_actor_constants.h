#pragma once

// Retail single-player uses 32 actors.  This is independently proven by the
// 0x20 bounds in Actor_Alloc/Actor_FirstActor/Actor_NextActor and by the
// 0x64800-byte Actor_Free/Sentient_Free scans at actor_s stride 0x3240.
// Multiplayer keeps the reconstructed source's original 16-actor contract.
#ifdef KISAK_SP
#define MAX_ACTORS 32
#else
#define MAX_ACTORS 16
#endif
