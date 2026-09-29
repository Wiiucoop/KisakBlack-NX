#pragma once

struct centity_s;
struct actorInfo_t;
struct cg_s;
struct cgs_t;

void __cdecl CG_ActorProcessSnapshot(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateActorDObj(int localClientNum, centity_s *cent, actorInfo_t *ai);
cgs_t *__cdecl CG_GetLocalClientStaticGlobals(int localClientNum);
void __cdecl CG_ResetActorEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_Actor_PreControllers(int localClientNum, centity_s *cent);
void __cdecl CG_Actor(int localClientNum, centity_s *cent);
bool __cdecl CG_EntityNeedsScriptThread(int localClientNum, centity_s *cent);
void __cdecl CG_ActorCorpse(int localClientNum, centity_s *cent);

#ifdef KISAK_SP
// Integrated-SP counterpart of retail's separate per-actor attachment block.
void __cdecl CG_PublishActorAttachments_SP(
    int entNum,
    const unsigned __int16 *modelIndices,
    const unsigned __int16 *tagNames,
    unsigned int ignoreCollisionBits);
#endif
