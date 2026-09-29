#include "demo_version.h"
#include "demo_common.h"
#include <qcommon/common.h>

const NetField demoPlayerStateFields[176] =
{
  { "viewangles[1]", 388, 4, -105, 0u, "MSG_FIELD_DEMO_VIEWANGLES", "0" },
  { "viewangles[0]", 384, 4, -105, 0u, "MSG_FIELD_DEMO_VIEWANGLES", "0" },
  { "viewangles[2]", 392, 4, -105, 0u, "MSG_FIELD_DEMO_VIEWANGLES", "0" },
  { "origin[0]", 36, 4, -103, 0u, "MSG_FIELD_DEMO_ORIGINX", "0" },
  { "origin[1]", 40, 4, -102, 0u, "MSG_FIELD_DEMO_ORIGINY", "0" },
  { "bobCycle", 8, 4, 8, 0u, "8", "0" },
  { "velocity[1]", 52, 4, -104, 0u, "MSG_FIELD_DEMO_VELOCITY", "0" },
  { "velocity[0]", 48, 4, -104, 0u, "MSG_FIELD_DEMO_VELOCITY", "0" },
  { "vehicleType", 1096, 4, 3, 0u, "VEHICLE_TYPES_BITS", "0" },
  { "movementDir", 220, 4, -8, 0u, "-8", "0" },
  {
    "predictableEventSequence",
    232,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  {
    "predictableEventSequenceOld",
    234,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  { "legsAnim", 188, 2, 11, 0u, "ANIM_BITS", "0" },
  { "origin[2]", 44, 4, -101, 0u, "MSG_FIELD_DEMO_ORIGINZ", "0" },
  { "weaponTime", 60, 4, -16, 0u, "-16", "0" },
  { "weaponTimeLeft", 68, 4, -16, 0u, "-16", "0" },
  { "weaponIdleTime", 76, 4, -71, 0u, "MSG_FIELD_WEAPON_IDLE_TIME", "0" },
  { "aimSpreadScale", 1308, 4, -106, 0u, "MSG_FIELD_AIM_SPREAD_SCALE", "0" },
  { "torsoTimer", 184, 4, 16, 0u, "16", "0" },
  // DELIBERATELY STILL 25, in BOTH configs. msg_mp.cpp's live playerStateFields
  // row widens to 32 under KISAK_SP (retail 0x00a5c6d8) so pm_flags bit 26 can
  // reach the client; this is the DEMO archive table, a frozen file format that
  // is written and read by the same build, so widening it would only invalidate
  // existing .dm_ files for no gain. Same rule dc5eeb7 applied to the demo
  // clientState list pinned at 50u.
  { "pm_flags", 12, 4, 25, 0u, "PMF_BIT_COUNT", "0" },
  { "weapAnim", 1300, 4, 11, 0u, "ANIM_BITS", "0" },
  { "weapAnimLeft", 1304, 4, 11, 0u, "ANIM_BITS", "0" },
  { "weaponstate", 344, 4, 7, 0u, "WEAPON_STATE_BITS", "0" },
  { "weaponstateLeft", 348, 4, 7, 0u, "WEAPON_STATE_BITS", "0" },
  { "velocity[2]", 56, 4, -104, 0u, "MSG_FIELD_DEMO_VELOCITY", "0" },
  { "viewAngleClampBase[1]", 420, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "viewAngleClampBase[0]", 416, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "predictableEventParms[0]", 252, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "predictableEventParms[1]", 256, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "predictableEvents[3]", 248, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEvents[2]", 244, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEvents[1]", 240, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEvents[0]", 236, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "predictableEventParms[2]", 260, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "predictableEventParms[3]", 264, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  {
    "unpredictableEventSequence",
    268,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  {
    "unpredictableEventSequenceOld",
    270,
    2,
    -59,
    0u,
    "MSG_FIELD_EVENTSEQUENCE",
    "0"
  },
  { "unpredictableEventParms[0]", 288, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "unpredictableEventParms[1]", 292, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "unpredictableEvents[3]", 284, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEvents[2]", 280, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEvents[1]", 276, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEvents[0]", 272, 4, -94, 0u, "MSG_FIELD_EVENT", "0" },
  { "unpredictableEventParms[2]", 296, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "unpredictableEventParms[3]", 300, 4, -93, 0u, "MSG_FIELD_EVENTPARAM", "0" },
  { "torsoAnim", 190, 2, 11, 0u, "ANIM_BITS", "0" },
  {
    "holdBreathScale",
    1192,
    4,
    -88,
    0u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "0"
  },
  { "eFlags", 224, 4, -98, 0u, "MSG_FIELD_EFLAGS", "0" },
  { "viewHeightCurrent", 400, 4, -107, 0u, "MSG_FIELD_VIEW_HEIGHT", "0" },
  {
    "fWeaponPosFrac",
    360,
    4,
    -88,
    0u,
    "MSG_FIELD_FLOAT_RARELYZERO_NONINT",
    "0"
  },
  { "moveType", 176, 4, 5, 0u, "ANIM_MOVETYPE_BITS", "0" },
  { "legsTimer", 180, 4, 16, 0u, "16", "0" },
  { "viewHeightTarget", 396, 4, -8, 0u, "-8", "0" },
  { "sprintState.lastSprintStart", 1160, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "sprintState.lastSprintEnd", 1164, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "lastDtpEnd", 1180, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "weapon", 324, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "stowedWeapon", 332, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "stowedWeaponCamo", 336, 1, 6, 0u, "RENDER_OPTIONS_CAMO_BITS", "0" },
  { "lastWeaponAltModeSwitch", 328, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "meleeWeapon", 340, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "renderOptions", 320, 4, 32, 0u, "MAX_RENDER_OPTIONS_BITS", "0" },
  { "weaponDelay", 64, 4, -16, 0u, "-16", "0" },
  { "sprintState.sprintStartMaxLength", 1168, 4, 14, 0u, "14", "0" },
  { "sprintState.sprintDuration", 1172, 4, 14, 0u, "14", "0" },
  { "sprintState.sprintCooldown", 1176, 4, 14, 0u, "14", "0" },
  { "weapFlags", 16, 4, 24, 0u, "PWF_BIT_COUNT", "0" },
  { "weapLockFlags", 1248, 4, 6, 0u, "WEAPONLOCKFLAG_BITS", "0" },
  { "groundEntityNum", 144, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "damageTimer", 200, 4, 10, 0u, "10", "0" },
  { "delta_angles[1]", 136, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "offHandIndex", 308, 4, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "pm_time", 24, 4, -16, 0u, "-16", "0" },
  { "otherFlags", 20, 4, 6, 0u, "POF_BIT_COUNT", "0" },
  { "moveSpeedScaleMultiplier", 1200, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "perks[0]", 1256, 4, 32, 0u, "PERK0_BITS", "0" },
  { "perks[1]", 1260, 4, 32, 0u, "PERK1_BITS", "0" },
  { "visionSetLerpRatio", 1384, 4, -80, 0u, "MSG_FIELD_0TO1_P3", "0" },
  { "poisoned", 1388, 4, 1, 0u, "1", "0" },
  { "throwBackGrenadeOwner", 84, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "actionSlotType[2]", 1272, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "delta_angles[0]", 132, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "speed", 128, 4, 16, 0u, "16", "0" },
  { "viewlocked_entNum", 1088, 2, 16, 0u, "16", "0" },
  { "gravity", 120, 4, 16, 0u, "16", "0" },
  { "actionSlotType[0]", 1264, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "dofNearBlur", 1340, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofFarBlur", 1344, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "eFlags2", 228, 4, 32, 0u, "32", "0" },
  { "clientNum", 304, 1, 8, 0u, "8", "0" },
  { "corpseIndex", 216, 4, 3, 0u, "CLIENT_CORPSE_INDEX_BITS", "0" },
  { "damageEvent", 436, 4, 8, 0u, "8", "0" },
  { "viewHeightLerpTarget", 408, 4, -8, 0u, "-8", "0" },
  { "damageYaw", 440, 4, 8, 0u, "8", "0" },
  { "viewmodelIndex", 380, 4, 9, 0u, "MODEL_BITS", "0" },
  { "damageDuration", 204, 4, 16, 0u, "16", "0" },
  { "damagePitch", 444, 4, 8, 0u, "8", "0" },
  { "dmgDirection", 208, 4, 3, 0u, "3", "0" },
  { "dmgType", 212, 4, 4, 0u, "4", "0" },
  { "weaponShotCount", 352, 4, 3, 0u, "PLAYER_WEAPONSHOTCOUNT_BITS", "0" },
  { "weaponShotCountLeft", 356, 4, 3, 0u, "PLAYER_WEAPONSHOTCOUNT_BITS", "0" },
  { "stackFireCount", 432, 4, 3, 0u, "PLAYER_WEAPONSHOTCOUNT_BITS", "0" },
  { "viewHeightLerpDown", 412, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "cursorHint", 1120, 4, 8, 0u, "8", "0" },
  { "cursorHintString", 1124, 4, -8, 0u, "-8", "0" },
  { "cursorHintEntIndex", 1128, 4, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "viewHeightLerpTime", 404, 4, 32, 0u, "32", "0" },
  { "offhandSecondary", 312, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "offhandPrimary", 316, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "spyplaneTypeEnabled", 1136, 4, 2, 0u, "2", "0" },
  { "satelliteTypeEnabled", 1140, 4, 2, 0u, "2", "0" },
  { "pm_type", 4, 4, 8, 0u, "8", "0" },
  { "fTorsoPitch", 1184, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "holdBreathTimer", 1196, 4, 16, 0u, "16", "0" },
  { "actionSlotParam[2]", 1288, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "jumpTime", 164, 4, 32, 0u, "32", "0" },
  { "slideTime", 172, 4, 32, 0u, "32", "0" },
  { "mantleState.flags", 1216, 4, 12, 0u, "MANTLE_FLAG_BITS", "0" },
  { "fWaistPitch", 1188, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "grenadeTimeLeft", 80, 4, -16, 0u, "-16", "0" },
  { "proneDirection", 1072, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "mantleState.timer", 1208, 4, 32, 0u, "32", "0" },
  { "damageCount", 448, 4, 7, 0u, "7", "0" },
  { "shellshockTime", 1316, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "shellshockDuration", 1320, 4, 16, 2u, "16", "ALWAYS_CHANGES" },
  {
    "sprintState.sprintButtonUpRequired",
    1152,
    4,
    1,
    2u,
    "1",
    "ALWAYS_CHANGES"
  },
  { "shellshockIndex", 1312, 4, 4, 0u, "4", "0" },
  { "proneTorsoPitch", 1080, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "sprintState.sprintDelay", 1156, 4, 1, 2u, "1", "ALWAYS_CHANGES" },
  { "actionSlotParam[3]", 1292, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "actionSlotType[3]", 1276, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "proneDirectionPitch", 1076, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "jumpOriginZ", 168, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "mantleState.yaw", 1204, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "mantleState.transIndex", 1212, 4, 4, 0u, "MANTLE_TRANS_BITS", "0" },
  { "throwBackGrenadeTimeLeft", 88, 4, -16, 0u, "-16", "0" },
  { "foliageSoundTime", 116, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "vLadderVec[0]", 152, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "viewlocked", 1084, 4, 2, 0u, "MAX_BITS_PLAYERVIEWLOCK", "0" },
  { "deltaTime", 2936, 4, 32, 0u, "32", "0" },
  { "linkAngles[1]", 1112, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "viewAngleClampRange[1]", 428, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "viewAngleClampRange[0]", 424, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "vLadderVec[1]", 156, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  {
    "locationSelectionInfo",
    1148,
    4,
    8,
    0u,
    "LOC_SEL_MTL_BITS + LOC_SEL_RADIUS_BITS",
    "0"
  },
  { "locationSelectionType", 1144, 4, 3, 0u, "3", "0" },
  { "meleeChargeTime", 1244, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "meleeChargeYaw", 1236, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "meleeChargeDist", 1240, 4, 8, 0u, "8", "0" },
  { "iCompassPlayerInfo", 1132, 4, 32, 0u, "32", "0" },
  { "actionSlotType[1]", 1268, 4, 2, 0u, "MAX_BITS_ACTIONSLOTTYPE", "0" },
  { "vLadderVec[2]", 160, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "weaponRestrictKickTime", 92, 4, -16, 0u, "-16", "0" },
  { "delta_angles[2]", 140, 4, -100, 0u, "MSG_FIELD_ANGLE2SHORT", "0" },
  { "spreadOverride", 368, 4, 6, 0u, "PLAYER_SPREAD_OVERRIDE_BITS", "0" },
  {
    "spreadOverrideState",
    372,
    4,
    2,
    0u,
    "PLAYER_SPREAD_OVERRIDE_STATE_BITS",
    "0"
  },
  { "actionSlotParam[0]", 1280, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "actionSlotParam[1]", 1284, 4, 11, 0u, "MAX_BITS_ACTIONSLOTPARAM", "0" },
  { "dofNearStart", 1324, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofNearEnd", 1328, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofFarStart", 1332, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofFarEnd", 1336, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofViewmodelStart", 1348, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "dofViewmodelEnd", 1352, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "scriptedAnim", 1228, 4, 32, 0u, "32", "0" },
  { "adsZoomSelect", 1368, 4, 32, 0u, "32", "0" },
  { "adsZoomLatchState", 1376, 1, 1, 0u, "1", "0" },
  { "leanf", 124, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "vehiclePos", 1092, 4, 4, 0u, "MAX_VEHICLE_SEAT_BITS", "0" },
  {
    "artilleryInboundIconLocation",
    1380,
    4,
    19,
    0u,
    "ARTILLERY_LOCATION_MESSAGE_BITS",
    "0"
  },
  { "weaponSpinLerp", 376, 4, 0, 0u, "MSG_FIELD_FLOAT", "0" },
  { "lastStandPrevWeapon", 326, 2, 11, 0u, "MAX_WEAPONS_BITS", "0" },
  { "adsDelayTime", 364, 4, 32, 1u, "32", "NEVER_CHANGES" },
  { "vehicleAnimBoneIndex", 1100, 4, 8, 1u, "MAX_BONE_BITS", "NEVER_CHANGES" },
  {
    "vehicleAnimStage",
    1220,
    4,
    4,
    1u,
    "VEHICLE_ANIM_STAGE_BITS",
    "NEVER_CHANGES"
  },
  {
    "vehicleEntryPoint",
    1224,
    4,
    3,
    1u,
    "VEHICLE_ENTRY_POINT_BITS",
    "NEVER_CHANGES"
  }
};



const NetFieldList s_demoOtherNetFieldList_Base[6] =
{
  { archivedEntityFields, 8u, "archivedEntityFields" },
  { clientStateFields, 50u, "clientStateFields" },
  { demoPlayerStateFields, 176u, "demoPlayerStateFields" },
  { objectiveFields, 9u, "objectiveFields" },
  { hudElemFields, 43u, "hudElemFields" },
  { matchStateFields, 15u, "matchStateFields" }
};

const NetFieldList s_demoEntityNetFieldList_Base[22] =
{
  { entityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "entityStateFields" },
  { playerEntityStateFields, 74u + SP_ENTITY_ANIM_FIELDS, "playerEntityStateFields" },
  { corpseEntityStateFields, 68u + SP_ENTITY_ANIM_FIELDS, "corpseEntityStateFields" },
  { itemEntityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "itemEntityStateFields" },
  { missileEntityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "missileEntityStateFields" },
  { entityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "entityStateFields" },
  { scriptMoverStateFields, 72u + SP_ENTITY_ANIM_FIELDS, "scriptMoverStateFields" },
  { soundBlendEntityStateFields, 68u + SP_ENTITY_ANIM_FIELDS, "soundBlendEntityStateFields" },
  { fxStateFields, 68u + SP_ENTITY_ANIM_FIELDS, "fxStateFields" },
  { loopFxEntityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "loopFxEntityStateFields" },
  { entityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "entityStateFields" },
  { turretEntityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "turretEntityStateFields" },
  { helicopterEntityStateFields, 71u + SP_ENTITY_ANIM_FIELDS, "helicopterEntityStateFields" },
  { planeStateFields, 66u + SP_ENTITY_ANIM_FIELDS, "planeStateFields" },
  { vehicleEntityStateFields, 71u + SP_ENTITY_ANIM_FIELDS, "vehicleEntityStateFields" },
  { entityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "entityStateFields" },
  { entityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "entityStateFields" },
  { actorStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "actorStateFields" },
  { actorStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "actorStateFields" },
  { actorStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "actorStateFields" },
  { entityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "entityStateFields" },
  { eventEntityStateFields, 69u + SP_ENTITY_ANIM_FIELDS, "eventEntityStateFields" }
};

const NetFieldList *s_demoOtherNetFieldLists[] = { s_demoOtherNetFieldList_Base };
const NetFieldList *s_demoEntityNetFieldLists[] = { s_demoEntityNetFieldList_Base };


int __cdecl Demo_GetVersion()
{
    if ( Demo_IsIdle() )
        Com_Error(ERR_DROP, "Trying to get version of a demo when the demo system is idle.");
    return demo.header.version;
}

const NetFieldList *__cdecl Demo_GetNetFieldListForType(netFieldTypes_t fieldType)
{
    int netFieldsVersionIndex; // [esp+0h] [ebp-8h]

    netFieldsVersionIndex = OPENBLOPS_DEMO_VERSION - Demo_GetVersion();
    if ( netFieldsVersionIndex != 0 )
        Com_Error(ERR_DROP, "Invalid Version Handling. Grab Bat !!!");
    return &s_demoOtherNetFieldLists[netFieldsVersionIndex][fieldType];
}

const NetFieldList *__cdecl Demo_GetStateFieldListForEntityType(int eType)
{
    int v2; // [esp+0h] [ebp-Ch]
    int netFieldsVersionIndex; // [esp+4h] [ebp-8h]

    netFieldsVersionIndex = OPENBLOPS_DEMO_VERSION - Demo_GetVersion();
    if ( netFieldsVersionIndex != 0 )
        Com_Error(ERR_DROP, "Invalid Version Handling. Grab Bat !!!");
    if ( eType > 21 )
        v2 = 21;
    else
        v2 = eType;
    return &s_demoEntityNetFieldLists[netFieldsVersionIndex][v2];
}
