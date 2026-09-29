#pragma once
#include "net_chan_mp.h"
#include "msg_mp.h"
#include <client_mp/cl_main_mp.h>
#include <server/server.h>

enum PacketEntityType : __int32
{                                       // XREF: SnapshotInfo_s/r
                                        // ?MSG_PacketAnalyze_SetPacketEntityType@@YAXPAUSnapshotInfo_s@@W4PacketEntityType@@_N@Z/r
    ANALYZE_DATATYPE_ENTITYTYPE_GENERALENTITY = 0x0,
    ANALYZE_DATATYPE_ENTITYTYPE_PLAYERENTITY = 0x1,
    ANALYZE_DATATYPE_ENTITYTYPE_PLAYERCORPSEENTITY = 0x2,
    ANALYZE_DATATYPE_ENTITYTYPE_ITEMENTITY = 0x3,
    ANALYZE_DATATYPE_ENTITYTYPE_MISSILEENTITY = 0x4,
    ANALYZE_DATATYPE_ENTITYTYPE_INVISIBLEENTITY = 0x5,
    ANALYZE_DATATYPE_ENTITYTYPE_SCRIPTMOVERENTITY = 0x6,
    ANALYZE_DATATYPE_ENTITYTYPE_SOUNDBLENDENTITY = 0x7,
    ANALYZE_DATATYPE_ENTITYTYPE_FXENTITY = 0x8,
    ANALYZE_DATATYPE_ENTITYTYPE_LOOPFXENTITY = 0x9,
    ANALYZE_DATATYPE_ENTITYTYPE_PRIMARYLIGHTENTITY = 0xA,
    ANALYZE_DATATYPE_ENTITYTYPE_MG42ENTITY = 0xB,
    ANALYZE_DATATYPE_ENTITYTYPE_HELICOPTER = 0xC,
    ANALYZE_DATATYPE_ENTITYTYPE_PLANE = 0xD,
    ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE = 0xE,
    ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_COLLMAP = 0xF,
    ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_CORPSE = 0x10,
    ANALYZE_DATATYPE_ENTITYTYPE_ACTOR = 0x11,
    ANALYZE_DATATYPE_ENTITYTYPE_ACTOR_SPAWNER = 0x12,
    ANALYZE_DATATYPE_ENTITYTYPE_ACTOR_CORPSE = 0x13,
    ANALYZE_DATATYPE_ENTITYTYPE_STREAMER_HINT = 0x14,
    ANALYZE_DATATYPE_ENTITYTYPE_TEMPENTITY = 0x15,
    ANALYZE_DATATYPE_ENTITYTYPE_ARCHIVEDENTITY = 0x16,
    ANALYZE_DATATYPE_ENTITYTYPE_MATCHSTATE = 0x17,
    ANALYZE_DATATYPE_ENTITYTYPE_CLIENTSTATE = 0x18,
    ANALYZE_DATATYPE_ENTITYTYPE_PLAYERSTATE = 0x19,
    ANALYZE_DATATYPE_ENTITYTYPE_HUDELEM = 0x1A,
    ANALYZE_DATATYPE_ENTITYTYPE_BASELINE = 0x1B,
    ANALYZE_DATATYPE_ENTITYTYPE_COUNT = 0x1C,
};

enum DeltaFlags : __int32
{                                       // XREF: SV_EmitPacketEntities/r
                                        // ?MSG_WriteEntityDelta@@YAHPBUSnapshotInfo_s@@PAUmsg_t@@HPBUentityState_s@@2W4DeltaFlags@@HHPBUNetField@@@Z/r ...
    DELTA_FLAGS_NONE  = 0x1,
    DELTA_FLAGS_FORCE = 0x0,
};

struct __declspec(align(4)) SnapshotInfo_s // sizeof=0x18
{                                       // XREF: .data:SnapshotInfo_s g_snapInfo/r
                                        // SnapshotInfo/r ...
    int clientNum;                      // XREF: CL_WriteUncompressedDemoInfo(int)+78C/w
                                        // SV_ArchiveSnapshot(msg_t *)+25/w ...
    const clientHeader_t *client;       // XREF: CL_WriteUncompressedDemoInfo(int)+78F/w
                                        // SV_ArchiveSnapshot(msg_t *)+2B/w ...
    int snapshotDeltaTime;              // XREF: CL_WriteUncompressedDemoInfo(int)+792/w
                                        // SV_ArchiveSnapshot(msg_t *)+31/w ...
    bool fromBaseline;                  // XREF: CL_WriteUncompressedDemoInfo(int)+795/w
                                        // SV_ArchiveSnapshot(msg_t *)+37/w ...
    bool archived;                      // XREF: SV_ArchiveSnapshot(msg_t *)+49/w
                                        // SV_WriteSnapshotToClient(client_t *,msg_t *)+D1/w ...
    bool entJustUnlinked;               // XREF: SV_ArchiveSnapshot(msg_t *)+50/w
    bool demoSnapshot;                  // XREF: Demo_Init(void)+B2/w
    PacketEntityType packetEntityType;  // XREF: CL_WriteUncompressedDemoInfo(int)+798/w
                                        // SV_ArchiveSnapshot(msg_t *)+3D/w ...
    bool packetEntityTypeArchived;      // XREF: CL_WriteUncompressedDemoInfo(int)+79B/w
                                        // SV_ArchiveSnapshot(msg_t *)+43/w ...
    // padding byte
    // padding byte
    // padding byte
};

bool __cdecl MSG_EntityIsLinked(const entityState_s *ent);
void __cdecl MSG_WriteGroundEntityNum(int clientNum, msg_t *msg, int groundEntityNum);
void __cdecl MSG_WriteDeltaField_Default(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                const NetField *field,
                const int *fromF,
                const int *toF);
int __cdecl MSG_WriteDeltaStruct(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                unsigned __int8 *from,
                unsigned __int8 *to,
                int force,
                int numFields,
                int indexBits,
                const NetField *stateFields,
                int bChangeBit,
                int skippedFieldBits);
int __cdecl MSG_WriteAppendedDeltaStruct(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                unsigned __int8 *from,
                unsigned __int8 *to,
                int force,
                int numFields,
                int indexBits,
                const NetField *stateFields,
                int bChangeBit,
                int skippedFieldBits);
int __cdecl MSG_WriteEntityDelta(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const entityState_s *fromES,
                const entityState_s *toES,
                DeltaFlags flags,
                int numFields,
                int indexBits,
                const NetField *stateFields);
void __cdecl MSG_WriteEntityDeltaFields(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const entityState_s *fromES,
                const entityState_s *toES,
                int force,
                int lastChanged,
                int numFields,
                const NetField *stateFields,
                int skippedFieldBits,
                int *fieldsChanged);
bool __cdecl MSG_WriteDeltaArchivedEntity(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const archivedEntity_s *from,
                const archivedEntity_s *to,
                DeltaFlags flags);
void __cdecl MSG_WriteDeltaHudElems(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const hudelem_s *from,
                const hudelem_s *to,
                int count);



static const int maxStateFieldCount = 74 + SP_ENTITY_ANIM_FIELDS;
static const int numEntityStateFields = 69 + SP_ENTITY_ANIM_FIELDS;


extern int g_entsClientOnce;
extern int g_entsDeltaCompared;
extern int g_entsTransmitted;
