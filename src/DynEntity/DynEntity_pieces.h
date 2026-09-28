#pragma once

#include <universal/dvar.h>

struct XModel;
struct XModelPieces;
struct PhysPreset;
struct PhysObjUserData;

struct __declspec(align(2)) BreakablePiece // sizeof=0xC
{                                       // XREF: .data:g_breakablePieces/r
    const XModel *model;                // XREF: DynEntPieces_AddDrawSurfs(int)+8D/r
                                        // DynEntPieces_SpawnPhysicsModel(int,XModel const *,float const * const,float const * const,float const (* const)[3],float const * const,float const * const,PhysPreset *)+268/w
    intptr_t physObjId;   // nx-port: holds a PhysObjUserData pointer                      // XREF: DynEntPieces_AddDrawSurfs(int)+4C/r
                                        // DynEntPieces_SpawnPhysicsModel(int,XModel const *,float const * const,float const * const,float const (* const)[3],float const * const,float const * const,PhysPreset *)+256/w
    unsigned __int16 lightingHandle;    // XREF: DynEntPieces_AddDrawSurfs(int)+7A/o
                                        // DynEntPieces_SpawnPhysicsModel(int,XModel const *,float const * const,float const * const,float const (* const)[3],float const * const,float const * const,PhysPreset *)+278/w
    bool active;                        // XREF: DynEntPieces_AddDrawSurfs(int)+29/r
                                        // DynEntPieces_SpawnPhysicsModel(int,XModel const *,float const * const,float const * const,float const (* const)[3],float const * const,float const * const,PhysPreset *)+288/w
    // padding byte
};

void __cdecl DynEntPieces_RegisterDvars();
void __cdecl DynEntPieces_AddDrawSurfs();
char __cdecl DynEntPieces_SpawnPhysicsModel(
                int localClientNum,
                const XModel *model,
                const float *offset,
                const float *origin,
                const float (*axis)[3],
                const float *hitPos,
                const float *hitDir,
                PhysPreset *physPreset);
PhysObjUserData *__cdecl DynEntPieces_SpawnPhysObj(
                const char *modelName,
                int stype,
                const float *mins,
                const float *maxs,
                const float *position,
                const float *quat,
                const float *velocity,
                float *angularVelocity,
                const PhysPreset *physPreset);
void __cdecl DynEntPieces_CalcForceDir(const float *hitDir, float spreadFraction, float *forceDir);
void __cdecl DynEntPieces_SpawnPieces(
                int localClientNum,
                const XModelPieces *pieces,
                const float *origin,
                const float (*axis)[3],
                const float *hitPos,
                const float *hitDir);

extern const dvar_s *dynEntPieces_velocity;
extern const dvar_s *dynEntPieces_angularVelocity;
extern const dvar_s *dynEntPieces_impactForce;