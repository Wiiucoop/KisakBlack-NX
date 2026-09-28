#pragma once

#include <universal/com_math.h>

struct FxCmd;
struct trace_t;
struct refdef_s;

enum FxUpdateResult : __int32
{                                       // XREF: FX_UpdateEffectPartialForClass/r
                                        // FX_UpdateElement/r ...
    FX_UPDATE_REMOVE = 0x0,
    FX_UPDATE_KEEP   = 0x1,
};

struct FxUpdateElem // sizeof=0x7C
{                                       // XREF: FX_UpdateElement/r
                                        // FX_UpdateTrailElement/r
    struct FxEffect *effect;
    int elemIndex;
    int atRestFraction;                 // XREF: FX_UpdateElement:loc_5EC147/r
                                        // FX_UpdateElement+29B/r
    orientation_t orient;
    int randomSeed;
    int sequence;
    float msecLifeSpan;
    int msecElemBegin;                  // XREF: FX_UpdateElement+19D/r
    int msecElemEnd;
    int msecUpdateBegin;                // XREF: FX_UpdateElement+19A/r
    int msecUpdateEnd;                  // XREF: FX_UpdateElement:loc_5EC1A6/r
    float msecElapsed;
    float normTimeUpdateEnd;
    float *elemOrigin;
    float *elemBaseVel;                 // XREF: FX_UpdateElement+13E/w
                                        // FX_UpdateTrailElement+D7/w
    float posWorld[3];                  // XREF: FX_UpdateElement+1A2/o
    bool onGround;                      // XREF: FX_UpdateElement+14A/w
                                        // FX_UpdateElement+23E/r ...
    // padding byte
    // padding byte
    // padding byte
    intptr_t physObjId;   // nx-port: holds a PhysObjUserData pointer                      // XREF: FX_UpdateElement+147/w
                                        // FX_UpdateTrailElement+DA/w
};

struct FxCameraUpdate // sizeof=0x3C
{                                       // XREF: ?CG_InitView@@YAXH@Z/r
                                        // ?CG_DrawActiveFrame@@YAHHHW4DemoType@@W4CubemapShot@@HH@Z/r ...
    float vieworg[3];
    float viewaxis[3][3];               // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+FFC/o
    float tanHalfFovX;                  // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+1013/w
    float tanHalfFovY;                  // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+1023/w
    float zfar;                         // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+102D/w
};

struct FxElemDef;
struct FxEffectDef;
struct FxEffect;
struct FxEffectContainer;
struct FxSpatialFrame;
struct FxSystem;
struct FxTrail;
struct FxElem;
struct FxElemVelStateSample;
struct FxElemVelStateInFrame;
struct FxTrailElem;
struct orientation_t;
struct FxCamera;

bool __cdecl FX_IsDemoPlaying();
void __cdecl FX_SpawnAllFutureLooping(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin);
void __cdecl FX_SpawnLoopingElems(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefIndex,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin,
                int msecUpdateEnd);
int __cdecl FX_LimitStabilizeTimeForElemDef_Recurse(
                const FxElemDef *elemDef,
                bool needToSpawnSystem,
                int originalUpdateTime);
int __cdecl FX_LimitStabilizeTimeForElemDef_SelfOnly(const FxElemDef *elemDef, bool needToSpawnSystem);
int __cdecl FX_LimitStabilizeTimeForEffectDef_Recurse(const FxEffectDef *remoteEffectDef, int originalUpdateTime);
void __cdecl FX_BeginLooping(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                FxSpatialFrame *frameWhenPlayed,
                const FxSpatialFrame *frameNow,
                int msecWhenPlayed,
                int msecNow);
void __cdecl FX_SpawnTrailLoopingElems(
                FxSystem *system,
                FxEffect *effect,
                FxEffect *remoteEffect,
                FxTrail *trail,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd);
void __cdecl FX_TriggerOneShot(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                FxSpatialFrame *frameWhenPlayed,
                int msecWhenPlayed);
void __cdecl FX_SpawnOneShotElems(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefIndex,
                FxSpatialFrame *frameWhenPlayed,
                int msecWhenPlayed);
void __cdecl FX_StartNewEffect(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect);
void __cdecl FX_PlayElementSpawnSound(int localClientNum, const FxElemDef *elemDef, const float *origin);
bool __cdecl FX_GetBoltTemporalBits(int localClientNum, unsigned int dobjHandle);
char __cdecl FX_GetBoneOrientation(int localClientNum, unsigned int dobjHandle, int boneIndex, orientation_t *orient);
bool __cdecl FX_GetBoneOrientation_IsDObjEntityValid(int localClientNum, int dobjHandle);
void __cdecl FX_UpdateEffectPartial(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd,
                unsigned __int16 *elemHandleStart,
                unsigned __int16 *elemHandleStop,
                volatile unsigned __int16 *trailElemStart,
                volatile unsigned __int16 *trailElemStop);
void __cdecl FX_ProcessLooping(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd);
void __cdecl FX_UpdateEffectPartialForClass(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int msecUpdateBegin,
                int msecUpdateEnd,
                unsigned __int16 elemHandleStart,
                unsigned __int16 elemHandleStop,
                unsigned int elemClass);
FxUpdateResult __cdecl FX_UpdateElement(
                FxSystem *system,
                FxEffect *effect,
                FxElem *elem,
                int msecUpdateBegin,
                int msecUpdateEnd);
const FxElemDef *__cdecl FX_GetUpdateElemDef(const FxUpdateElem *update);
double __cdecl FX_GetAtRestFraction(const FxUpdateElem *update, float msec);
int __cdecl FX_UpdateElementPosition(FxSystem *system, FxUpdateElem *update); // KISAKTODO: retval 'FxUpdateResult'
int __cdecl FX_UpdateElementPosition_Colliding(FxSystem *system, FxUpdateElem *update);
int __cdecl FX_UpdateElementPosition_CollidingStep(
                FxSystem *system,
                FxUpdateElem *update,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float *xyzWorldOld);
void __cdecl FX_NextElementPosition(FxUpdateElem *update, int msecUpdateBegin, int msecUpdateEnd);
void __cdecl FX_NextElementPosition_NoExternalForces(
                FxUpdateElem *update,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float *posLocal,
                float *posWorld);
void __cdecl FX_IntegrateVelocity(const FxUpdateElem *update, float t0, float t1, float *posLocal, float *posWorld);
void __cdecl FX_IntegrateVelocityAcrossSegments(
                int elemDefFlags,
                const orientation_t *orient,
                const FxElemVelStateSample *velState0,
                const FxElemVelStateSample *velState1,
                float t0,
                float t1,
                const float *amplitudeScale,
                float integralScale,
                float *posLocal,
                float *posWorld);
void __cdecl FX_IntegrateVelocityFromZeroInSegment(
                const FxElemVelStateInFrame *statePrev,
                const FxElemVelStateInFrame *stateNext,
                float *weight,
                const float *amplitudeScale,
                float integralScale,
                float *pos);
void __cdecl FX_IntegrateVelocityInSegment(
                int elemDefFlags,
                const orientation_t *orient,
                const FxElemVelStateSample *velState,
                float t0,
                float t1,
                const float *amplitudeScale,
                float integralScale,
                float *posLocal,
                float *posWorld);
void __cdecl FX_IntegrateVelocityInSegmentInFrame(
                const FxElemVelStateInFrame *statePrev,
                const FxElemVelStateInFrame *stateNext,
                const float *weight,
                const float *amplitudeScale,
                float integralScale,
                float *pos);
bool __cdecl FX_TraceHitSomething(const trace_t *trace);
int __cdecl FX_CollisionResponse(
                FxSystem *system,
                FxUpdateElem *update,
                const trace_t *trace,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float *xyzWorldOld);
void __cdecl FX_SpawnImpactEffect(
                FxSystem *system,
                const FxUpdateElem *update,
                const FxEffectDef *impactEffect,
                int msecOnImpact,
                const float *impactNormal);
int __cdecl FX_UpdateElementPosition_NonColliding(FxUpdateElem *update);
int __cdecl FX_UpdateElementPosition_Local(FxUpdateElem *update);
void __cdecl FX_SpawnDeathEffect(FxSystem *system, FxUpdateElem *update);
char __cdecl FX_UpdateElement_SetupUpdate(
                FxEffect *effect,
                int msecUpdateBegin,
                int msecUpdateEnd,
                int elemDefIndex,
                int elemAtRestFraction,
                int elemMsecBegin,
                int elemSequence,
                float *elemOrigin,
                float elemWindInterp,
                FxUpdateElem *update);
void __cdecl FX_UpdateElement_TruncateToElemEnd(FxUpdateElem *update, FxUpdateResult *outUpdateResult);
void __cdecl FX_UpdateElement_HandleEmitting(
                FxSystem *system,
                FxElem *elem,
                const FxUpdateElem *update,
                const float *elemOriginPrev,
                FxUpdateResult *outUpdateResult);
unsigned __int8 __cdecl FX_ProcessEmitting(
                FxSystem *system,
                const FxUpdateElem *update,
                unsigned __int8 emitResidual,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd);
void __cdecl FX_GetQuatForOrientation(
                const FxEffect *effect,
                const FxElemDef *elemDef,
                const FxSpatialFrame *frameNow,
                const orientation_t *orient,
                float *quat);
void __cdecl FX_UpdateElement_HandleAttachedEffect(FxSystem *system, FxElem *elem, const FxUpdateElem *update);
char __cdecl FX_UpdateElement_TruncateToElemBegin(FxUpdateElem *update, FxUpdateResult *outUpdateResult);
void __cdecl FX_UpdateEffectPartialTrail(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                FxTrail *trail,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd,
                unsigned __int16 trailElemHandleStart,
                unsigned __int16 trailElemHandleStop,
                FxSpatialFrame *frameNow);
FxUpdateResult __cdecl FX_UpdateTrailElement(
                FxSystem *system,
                FxEffect *effect,
                FxTrail *trail,
                FxTrailElem *trailElem,
                int msecUpdateBegin,
                int msecUpdateEnd);
void __cdecl FX_UpdateBatch(
                FxSystem *system,
                FxSystem *remoteSystem,
                int localClientNum,
                bool nonBoltedEffectsOnly,
                int firstEffect,
                int lastEffect);
char __cdecl FX_AcquireEffectLock_IsLiveEffect(FxEffect *effect);
void __cdecl FX_UpdateEffect(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect);
void __cdecl FX_UpdateEffectBolt(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect);
char __cdecl FX_CanKillImmediate(const FxEffectDef *def);
bool __cdecl FX_ShouldProcessEffect(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                bool nonBoltedEffectsOnly);
void __cdecl FX_Update(FxSystem *system, FxSystem *remoteSystem, int localClientNum, bool nonBoltedEffectsOnly);
void __cdecl FX_UpdateSpotLight(FxCmd *cmd);
void __cdecl FX_UpdateSpotLightEffect(FxSystem *system, FxEffectContainer *effect, FxEffectContainer *remoteEffect);
void __cdecl FX_UpdateSpotLightEffectPartial(
                FxSystem *system,
                FxEffectContainer *effect,
                int msecUpdateBegin,
                int msecUpdateEnd);
void __cdecl FX_UpdateNonDependent(FxCmd *cmd);
void __cdecl FX_UpdateRemaining(FxCmd *cmd);
void __cdecl FX_BeginUpdate(int localClientNum);
void __cdecl FX_EndUpdate(int localClientNum);
void __cdecl FX_AddNonSpriteDrawSurfs(FxCmd *cmd);
void __cdecl FX_RewindTo_Setup(int localClientNum, int time);
unsigned int __cdecl FX_IndexFromEffectHandle(unsigned __int16 handle);
void __cdecl FX_RewindTo_Finish(int localClientNum);
unsigned int __cdecl FX_EffectHandleFromIndex(unsigned int index);
void __cdecl FX_RewindTo(int localClientNum, int time);
void __cdecl DB_RestartEffects_Begin();
void __cdecl DB_RestartEffects_Finish();
void __cdecl FX_GetCameraUpdateFromRefdefAndZFar(FxCameraUpdate *cameraUpdate, const refdef_s *refdef, float zfar);
void __cdecl FX_SetExtraCamera(int localClientNum, const FxCameraUpdate *cameraUpdate);
void __cdecl FX_SetupCamera(
                FxCamera *camera,
                const float *vieworg,
                const float (*viewaxis)[3],
                float tanHalfFovX,
                float tanHalfFovY,
                float zfar);
void __cdecl FX_SetNextUpdateCamera(int localClientNum, const FxCameraUpdate *cameraUpdate);
void __cdecl FX_SetNextUpdateTime(int localClientNum, int time);
void __cdecl FX_FillUpdateCmd(int localClientNum, FxCmd *cmd);
