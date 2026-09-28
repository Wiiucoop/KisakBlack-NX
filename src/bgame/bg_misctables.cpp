#include "bg_misctables.h"
#include <universal/q_shared.h>
#include <qcommon/common.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_vm.h>
#include <client/cl_keys.h>
#include <qcommon/graph.h>
#include <vehicle/nitrous_vehicle.h>
#include <cstddef>

// nx-port: vehicle_info_t matches x86 up to compassIconMaterial (x86 7264);
// that pointer and steerGraph / accelGraph are 8 bytes on LP64. Offsets
// before it are unchanged; the rest are rebased onto the member after each.
#define VEHICLE_X86(o) ((int)((o) < 7320 ? (o) - 7268 + offsetof(vehicle_info_t, gasButton)   \
                            : (o) == 7320 ? offsetof(vehicle_info_t, steerGraph)            \
                            : (o) == 7324 ? offsetof(vehicle_info_t, accelGraph)            \
                            : (o) - 7328 + offsetof(vehicle_info_t, isNitrous)))

const char *s_vehicleCameraModes[6] = { "first", "chase", "view", "strafe", "horse", "oldtank" };
const char *s_vehicleBoostModes[2] = { "burst", "continuous" };
const char *s_tractionTypeNames[3] = { "TRACTION_TYPE_FRONT", "TRACTION_TYPE_BACK", "TRACTION_TYPE_ALL_WD" };
const char *s_vehicleClassNames[7] = { "4 wheel", "motorcycle", "tank", "plane", "boat", "artillery", "helicopter" };

cspField_t s_vehicleFields[345] =
{
  { "type", 64, 15 },
  { "remoteControl", 68, 6 },
  { "steerWheels", 72, 6 },
  { "texureScroll", 76, 6 },
  { "quadBarrel", 80, 6 },
  { "bulletDamage", 84, 6 },
  { "armorPiercingDamage", 88, 6 },
  { "grenadeDamage", 92, 6 },
  { "projectileDamage", 96, 6 },
  { "projectileSplashDamage", 100, 6 },
  { "heavyExplosiveDamage", 104, 6 },
  { "cameraMode", 108, 16 },
  { "autoRecenterOnAccel", 112, 6 },
  { "thirdPersonDriver", 116, 6 },
  { "thirdPersonCameraHeightMin", 132, 7 },
  { "thirdPersonCameraPitchMin", 140, 7 },
  { "thirdPersonCameraRange", 120, 7 },
  { "thirdPersonCameraHeight", 136, 7 },
  { "thirdPersonCameraPitch", 144, 7 },
  { "thirdPersonCameraMinPitchClamp", 124, 7 },
  { "thirdPersonCameraMaxPitchClamp", 128, 7 },
  { "cameraAlwaysAutoCenter", 148, 6 },
  { "cameraAutoCenterLerpRate", 152, 7 },
  { "cameraAutoCenterMaxLerpRate", 156, 7 },
  { "thirdPersonCameraSpringDistance", 160, 7 },
  { "thirdPersonCameraSpringTime", 164, 7 },
  { "thirdPersonCameraHandbrakeTurnRateInc", 168, 7 },
  { "cameraFOV", 172, 7 },
  { "viewInfluence", 392, 7 },
  { "killcamCollision", 176, 6 },
  { "killcamDist", 180, 7 },
  { "killcamZDist", 184, 7 },
  { "killcamMinDist", 188, 7 },
  { "killcamZTargetOffset", 192, 7 },
  { "killcamFOV", 196, 7 },
  { "killcamNearBlur", 200, 7 },
  { "killcamNearBlurStart", 204, 7 },
  { "killcamNearBlurEnd", 208, 7 },
  { "killcamFarBlur", 212, 7 },
  { "killcamFarBlurStart", 216, 7 },
  { "killcamFarBlurEnd", 220, 7 },
  { "isDrivable", 224, 6 },
  { "numberOfSeats", 228, 4 },
  { "numberOfGunners", 232, 4 },
  { "driverControlledGunPos", 280, 4 },
  { "seatSwitchOrder1", 236, 4 },
  { "seatSwitchOrder2", 240, 4 },
  { "seatSwitchOrder3", 244, 4 },
  { "seatSwitchOrder4", 248, 4 },
  { "seatSwitchOrder5", 252, 4 },
  { "seatSwitchOrder6", 256, 4 },
  { "seatSwitchOrder7", 260, 4 },
  { "seatSwitchOrder8", 264, 4 },
  { "seatSwitchOrder9", 268, 4 },
  { "seatSwitchOrder10", 272, 4 },
  { "seatSwitchOrder11", 276, 4 },
  { "texureScrollScale", 304, 7 },
  { "wheelRotRate", 308, 7 },
  { "extraWheelRotScale", 312, 7 },
  { "enterRadiusDriver", 284, 7 },
  { "enterRadiusGunner1", 288, 7 },
  { "enterRadiusGunner2", 292, 7 },
  { "enterRadiusGunner3", 296, 7 },
  { "enterRadiusGunner4", 300, 7 },
  { "maxSpeed", 316, 19 },
  { "accel", 320, 19 },
  { "rotRate", 324, 7 },
  { "rotAccel", 328, 7 },
  { "collisionDamage", 340, 7 },
  { "collisionSpeed", 344, 19 },
  { "suspensionTravel", 348, 7 },
  { "maxBodyPitch", 332, 7 },
  { "maxBodyRoll", 336, 7 },
  { "boostMode", 108, 17 },
  { "boostSpeedIncrease", 356, 7 },
  { "boostAccelMultiplier", 360, 7 },
  { "boostInterval", 364, 7 },
  { "boostDuration", 368, 7 },
  { "boostFrictionMultiplier", 372, 7 },
  { "boostContinuousRegenCooldown", 376, 7 },
  { "boostContinuousRegenAmount", 380, 7 },
  { "heliCollisionScalar", 384, 7 },
  { "viewPitchOffset", 388, 7 },
  { "tiltFromAccelerationPitch", 396, 7 },
  { "tiltFromAccelerationRoll", 400, 7 },
  { "tiltFromDecelerationPitch", 404, 7 },
  { "tiltFromDecelerationRoll", 408, 7 },
  { "tiltFromVelocityPitch", 412, 7 },
  { "tiltFromVelocityRoll", 416, 7 },
  { "tiltSpeedPitch", 420, 7 },
  { "tiltSpeedRoll", 424, 7 },
  { "tracerOffsetForward", 3844, 7 },
  { "tracerOffsetUp", 3848, 7 },
  { "turretWeapon", 428, 0 },
  { "turretHorizSpanLeft", 492, 7 },
  { "turretHorizSpanRight", 496, 7 },
  { "turretVertSpanUp", 500, 7 },
  { "turretVertSpanDown", 504, 7 },
  { "turretRotRate", 508, 7 },
  { "turretClampPlayerView", 512, 6 },
  { "turretTag1", 516, 0 },
  { "turretTag2", 580, 0 },
  { "turretTag3", 644, 0 },
  { "turretTag4", 708, 0 },
  { "gunnerWeapon", 772, 0 },
  { "gunnerWeapon1", 836, 0 },
  { "gunnerWeapon2", 900, 0 },
  { "gunnerWeapon3", 964, 0 },
  { "gunnerRotRate", 1036, 7 },
  { "passenger1HorizSpanLeft", 1072, 7 },
  { "passenger1HorizSpanRight", 1076, 7 },
  { "passenger1VertSpanUp", 1080, 7 },
  { "passenger1VertSpanDown", 1084, 7 },
  { "passenger2HorizSpanLeft", 1088, 7 },
  { "passenger2HorizSpanRight", 1092, 7 },
  { "passenger2VertSpanUp", 1096, 7 },
  { "passenger2VertSpanDown", 1100, 7 },
  { "passenger3HorizSpanLeft", 1104, 7 },
  { "passenger3HorizSpanRight", 1108, 7 },
  { "passenger3VertSpanUp", 1112, 7 },
  { "passenger3VertSpanDown", 1116, 7 },
  { "passenger4HorizSpanLeft", 1120, 7 },
  { "passenger4HorizSpanRight", 1124, 7 },
  { "passenger4VertSpanUp", 1128, 7 },
  { "passenger4VertSpanDown", 1132, 7 },
  { "passenger5HorizSpanLeft", 1136, 7 },
  { "passenger5HorizSpanRight", 1140, 7 },
  { "passenger5VertSpanUp", 1144, 7 },
  { "passenger5VertSpanDown", 1148, 7 },
  { "passenger6HorizSpanLeft", 1152, 7 },
  { "passenger6HorizSpanRight", 1156, 7 },
  { "passenger6VertSpanUp", 1160, 7 },
  { "passenger6VertSpanDown", 1164, 7 },
  { "lowIdleSnd", 1168, 0 },
  { "lowIdleSndPlyr", 1232, 0 },
  { "highIdleSnd", 1296, 0 },
  { "lowEngineSnd", 1360, 0 },
  { "highEngineSnd", 1424, 0 },
  { "turretSpinSnd", 1488, 0 },
  { "turretStopSnd", 1552, 0 },
  { "engineModLoopSnd", 1616, 0 },
  { "engineModLoopSndPlyr", 1680, 0 },
  { "engineModLoopLoadSnd", 1744, 0 },
  { "engineModLoopLoadSndPlyr", 1808, 0 },
  { "tankBodyDmgLeftSnd", 1872, 0 },
  { "tankBodyDmgRightSnd", 1936, 0 },
  { "tankGrindLeftSnd", 2000, 0 },
  { "tankGrindRightSnd", 2064, 0 },
  { "tankGrindLeftDmgSnd", 2128, 0 },
  { "tankGrindRightDmgSnd", 2192, 0 },
  { "tankBoostPerkSnd", 2256, 0 },
  { "tankBoostPerkSndPlyr", 2320, 0 },
  { "wheelRoadNoiseSnd", 2460, 0 },
  { "wheelSlidingSnd", 2524, 0 },
  { "wheelPeelingOutSnd", 2588, 0 },
  { "engineSndSpeed", 2652, 19 },
  { "engineModLoopNaturalRPMs", 2656, 7 },
  { "idleRPMs", 2660, 7 },
  { "idleRPMsMax", 2664, 7 },
  { "gear1MinRPMs", 2668, 7 },
  { "gear1MaxRPMs", 2672, 7 },
  { "gearChangeMPH", 2676, 7 },
  { "gear2MinRPMs", 2680, 7 },
  { "gear2MaxRPMs", 2684, 7 },
  { "trackLengthInches", 2688, 4 },
  { "clacksCurveDamageStart", 2692, 7 },
  { "clacksCurveDamageEnd", 2696, 7 },
  { "clacksCurveMaxOmitted", 2700, 7 },
  { "treadGrindAttenuation", 2704, 7 },
  { "treadDamageLoopHealthRatio", 2708, 7 },
  { "skidSpeedMin", 2712, 7 },
  { "skidSpeedMax", 2716, 7 },
  { "animType", 2720, 4 },
  { "animSet", 2724, 0 },
  { "mantleAngleFront", 2788, 7 },
  { "mantleAngleBack", 2792, 7 },
  { "mantleAngleLeft", 2796, 7 },
  { "mantleAngleRight", 2800, 7 },
  { "extraWheelLeft1", 2804, 21 },
  { "extraWheelRight1", 2808, 21 },
  { "extraWheelLeft2", 2812, 21 },
  { "extraWheelRight2", 2816, 21 },
  { "mod0", 2820, 0 },
  { "tag0", 3076, 0 },
  { "mod1", 2884, 0 },
  { "tag1", 3140, 0 },
  { "mod2", 2948, 0 },
  { "tag2", 3204, 0 },
  { "mod3", 3012, 0 },
  { "tag3", 3268, 0 },
  { "dmod0", 3332, 0 },
  { "dtag0", 3588, 0 },
  { "dmod1", 3396, 0 },
  { "dtag1", 3652, 0 },
  { "dmod2", 3460, 0 },
  { "dtag2", 3716, 0 },
  { "dmod3", 3524, 0 },
  { "dtag3", 3780, 0 },
  { "worldModel", 3852, 0 },
  { "deathModel", 3916, 0 },
  { "modelSwapDelay", 3980, 7 },
  { "exhaustFx", 3984, 0 },
  { "oneExhaust", 4048, 6 },
  { "rotorMainIdleFx", 4052, 0 },
  { "rotorMainStartFx", 4116, 0 },
  { "rotorMainRunningFx", 4180, 0 },
  { "rotorMainStopFx", 4244, 0 },
  { "rotorTailIdleFx", 4308, 0 },
  { "rotorTailStartFx", 4372, 0 },
  { "rotorTailRunningFx", 4436, 0 },
  { "rotorTailStopFx", 4500, 0 },
  { "treadFxAsphalt", 5972, 0 },
  { "treadFxBark", 4628, 0 },
  { "treadFxBrick", 4692, 0 },
  { "treadFxCarpet", 4756, 0 },
  { "treadFxCeramic", 6036, 0 },
  { "treadFxCloth", 4820, 0 },
  { "treadFxConcrete", 4884, 0 },
  { "treadFxCushion", 6228, 0 },
  { "treadFxDefault", 4564, 0 },
  { "treadFxDirt", 4948, 0 },
  { "treadFxFlesh", 5012, 0 },
  { "treadFxFoliage", 5076, 0 },
  { "treadFxFruit", 6292, 0 },
  { "treadFxGlass", 5140, 0 },
  { "treadFxGrass", 5204, 0 },
  { "treadFxGravel", 5268, 0 },
  { "treadFxIce", 5332, 0 },
  { "treadFxMetal", 5396, 0 },
  { "treadFxMud", 5460, 0 },
  { "treadFxPaintedMetal", 6356, 0 },
  { "treadFxPaper", 5524, 0 },
  { "treadFxPlaster", 5588, 0 },
  { "treadFxPlastic", 6100, 0 },
  { "treadFxRock", 5652, 0 },
  { "treadFxRubber", 6164, 0 },
  { "treadFxSand", 5716, 0 },
  { "treadFxSnow", 5780, 0 },
  { "treadFxWater", 5844, 0 },
  { "treadFxWood", 5908, 0 },
  { "deathFxName", 6548, 0 },
  { "deathFxTag", 6612, 0 },
  { "deathFxSound", 6676, 0 },
  { "radiusDamageMin", 6740, 7 },
  { "radiusDamageMax", 6744, 7 },
  { "radiusDamageRadius", 6748, 7 },
  { "shootShock", 6752, 0 },
  { "shootRumble", 6816, 0 },
  { "deathQuakeScale", 6880, 7 },
  { "deathQuakeDuration", 6884, 7 },
  { "deathQuakeRadius", 6888, 7 },
  { "secTurretType", 6892, 0 },
  { "secTurretTag", 6956, 0 },
  { "secTurretModel", 7020, 0 },
  { "secTurretAiControlled", 7084, 6 },
  { "frontArmor", 7088, 7 },
  { "rumbleType", 7092, 0 },
  { "rumbleScale", 7156, 7 },
  { "rumbleDuration", 7160, 7 },
  { "rumbleRadius", 7164, 7 },
  { "rumbleBaseTime", 7168, 7 },
  { "rumbleAdditionalTime", 7172, 7 },
  { "healthDefault", 7176, 4 },
  { "healthMin", 7180, 4 },
  { "healthMax", 7184, 4 },
  { "team", 7188, 22 },
  { "addToCompass", 7192, 6 },
  { "addToCompassEnemy", 7196, 6 },
  { "compassIcon", 7200, 0 },
  { "gasButton", VEHICLE_X86(7268), 23 },
  { "boostButton", VEHICLE_X86(7272), 23 },
  { "reverseBrakeButton", VEHICLE_X86(7276), 23 },
  { "handBrakeButton", VEHICLE_X86(7280), 23 },
  { "attackButton", VEHICLE_X86(7284), 23 },
  { "attackSecondaryButton", VEHICLE_X86(7288), 23 },
  { "moveUpButton", VEHICLE_X86(7292), 23 },
  { "moveDownButton", VEHICLE_X86(7296), 23 },
  { "switchSeatButton", VEHICLE_X86(7300), 23 },
  { "specialAbilityButton", VEHICLE_X86(7304), 23 },
  { "firePickupButton", VEHICLE_X86(7308), 23 },
  { "swapPickupButton", VEHICLE_X86(7312), 23 },
  { "dropDeployableButton", VEHICLE_X86(7316), 23 },
  { "steerGraph", VEHICLE_X86(7320), 24 },
  { "accelGraph", VEHICLE_X86(7324), 24 },
  { "isNitrous", VEHICLE_X86(7328), 6 },
  { "isFourWheelSteering", VEHICLE_X86(7332), 6 },
  { "noDirectionalDamage", VEHICLE_X86(7344), 6 },
  { "max_fric_tilt_angle", VEHICLE_X86(7336), 7 },
  { "max_fric_tilt", VEHICLE_X86(7340), 7 },
  { "maxSpeed", VEHICLE_X86(7348), 19 },
  { "accel", VEHICLE_X86(7352), 19 },
  { "autoHandbrakeMinSpeed", VEHICLE_X86(7448), 19 },
  { "nitrous_steer_angle_max", VEHICLE_X86(7360), 7 },
  { "nitrous_steer_speed", VEHICLE_X86(7364), 7 },
  { "nitrous_body_mass", VEHICLE_X86(7408), 20 },
  { "nitrous_wheel_radius", VEHICLE_X86(7368), 7 },
  { "nitrous_susp_adj", VEHICLE_X86(7380), 7 },
  { "nitrous_susp_spring_k", VEHICLE_X86(7372), 7 },
  { "nitrous_susp_damp_k", VEHICLE_X86(7376), 7 },
  { "nitrous_susp_hard_limit", VEHICLE_X86(7384), 7 },
  { "nitrous_susp_min_height", VEHICLE_X86(7388), 7 },
  { "nitrous_tire_damp_hand", VEHICLE_X86(7444), 7 },
  { "nitrous_tire_damp_brake", VEHICLE_X86(7440), 7 },
  { "nitrous_tire_damp_coast", VEHICLE_X86(7436), 7 },
  { "nitrous_tire_fric_brake", VEHICLE_X86(7400), 7 },
  { "nitrous_tire_fric_hand_brake", VEHICLE_X86(7404), 7 },
  { "nitrous_tire_fric_fwd", VEHICLE_X86(7392), 7 },
  { "nitrous_tire_fric_side", VEHICLE_X86(7396), 7 },
  { "nitrous_roll_stability", VEHICLE_X86(7412), 7 },
  { "nitrous_roll_resistance", VEHICLE_X86(7416), 7 },
  { "nitrous_yaw_resistance", VEHICLE_X86(7420), 7 },
  { "nitrous_upright_strength", VEHICLE_X86(7424), 7 },
  { "nitrous_tilt_fakey", VEHICLE_X86(7428), 7 },
  { "nitrous_traction_type", VEHICLE_X86(7452), 18 },
  { "nitrous_peel_out_max_speed", VEHICLE_X86(7432), 7 },
  { "nitrous_tire_fric_side_max", VEHICLE_X86(7636), 7 },
  { "nitrous_reverse_scale", VEHICLE_X86(7356), 7 },
  { "nitrous_water_speed_max", VEHICLE_X86(7580), 19 },
  { "nitrous_water_accel_max", VEHICLE_X86(7584), 7 },
  { "nitrous_water_turn_accel", VEHICLE_X86(7588), 7 },
  { "nitrous_water_turn_speed_max", VEHICLE_X86(7592), 7 },
  { "nitrous_boat_ebrake_power", VEHICLE_X86(7596), 7 },
  { "nitrous_bbox_min_x", VEHICLE_X86(7520), 7 },
  { "nitrous_bbox_min_y", VEHICLE_X86(7524), 7 },
  { "nitrous_bbox_min_z", VEHICLE_X86(7528), 7 },
  { "nitrous_bbox_max_x", VEHICLE_X86(7532), 7 },
  { "nitrous_bbox_max_y", VEHICLE_X86(7536), 7 },
  { "nitrous_bbox_max_z", VEHICLE_X86(7540), 7 },
  { "nitrous_mass_center_offset_x", VEHICLE_X86(7544), 7 },
  { "nitrous_mass_center_offset_y", VEHICLE_X86(7548), 7 },
  { "nitrous_mass_center_offset_z", VEHICLE_X86(7552), 7 },
  { "nitrous_buoyancybox_min_x", VEHICLE_X86(7556), 7 },
  { "nitrous_buoyancybox_min_y", VEHICLE_X86(7560), 7 },
  { "nitrous_buoyancybox_min_z", VEHICLE_X86(7564), 7 },
  { "nitrous_buoyancybox_max_x", VEHICLE_X86(7568), 7 },
  { "nitrous_buoyancybox_max_y", VEHICLE_X86(7572), 7 },
  { "nitrous_buoyancybox_max_z", VEHICLE_X86(7576), 7 },
  { "nitrous_boat_speed_rise", VEHICLE_X86(7612), 7 },
  { "nitrous_boat_speed_tilt", VEHICLE_X86(7616), 7 },
  { "nitrous_boat_motor_offset_x", VEHICLE_X86(7600), 7 },
  { "nitrous_boat_motor_offset_y", VEHICLE_X86(7604), 7 },
  { "nitrous_boat_motor_offset_z", VEHICLE_X86(7608), 7 },
  { "nitrous_boat_side_fric", VEHICLE_X86(7620), 7 },
  { "nitrous_boat_forward_fric", VEHICLE_X86(7624), 7 },
  { "nitrous_boat_vertical_fric", VEHICLE_X86(7628), 7 }
};



int __cdecl VEH_ParseSpecificField(unsigned __int8 *pStruct, const char *pValue, const int fieldType, const int fieldOffset)
{
    const char *v4; // eax
    const char *v5; // eax
    float v7; // [esp+0h] [ebp-10h]
    float v8; // [esp+4h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-4h]
    int ia; // [esp+Ch] [ebp-4h]
    int ib; // [esp+Ch] [ebp-4h]
    int ic; // [esp+Ch] [ebp-4h]

    switch ( fieldType )
    {
        case 15:
            i = 0;
            break;
        case 16:
            ia = 0;
            while ( 2 )
            {
                if ( ia < 6 )
                {
                    if ( I_stricmp(pValue, s_vehicleCameraModes[ia]) )
                    {
                        ++ia;
                        continue;
                    }
                    *((_WORD *)pStruct + 54) = ia;
                }
                break;
            }
            if ( ia == 6 )
                Com_Error(ERR_DROP, "Unknown vehicle camera mode [%s]", pValue);
            return 1;
        case 17:
            ib = 0;
            while ( 2 )
            {
                if ( ib < 2 )
                {
                    if ( I_stricmp(pValue, s_vehicleBoostModes[ib]) )
                    {
                        ++ib;
                        continue;
                    }
                    *((_WORD *)pStruct + 176) = ib;
                }
                break;
            }
            if ( ib == 2 )
                Com_Error(ERR_DROP, "Unknown vehicle boost mode [%s]", pValue);
            return 1;
        case 18:
            ic = 0;
            while ( 2 )
            {
                if ( ic < 3 )
                {
                    if ( I_stricmp(pValue, s_tractionTypeNames[ic]) )
                    {
                        ++ic;
                        continue;
                    }
                    *((unsigned int *)pStruct + 1863) = ic;
                }
                break;
            }
            if ( ic == 3 )
                Com_Error(ERR_DROP, "Unknown traction type [%s]", pValue);
            return 1;
        case 19:
            v8 = atof(pValue);
            *(float *)&pStruct[fieldOffset] = v8 * 17.6;
            return 1;
        case 20:
            v7 = atof(pValue);
            *(float *)&pStruct[fieldOffset] = v7 * 0.001;
            return 1;
        case 21:
            *(unsigned int *)&pStruct[fieldOffset] = SL_FindLowercaseString(pValue, SCRIPTINSTANCE_SERVER);
            return 1;
        case 22:
            if ( I_stricmp(pValue, "axis") )
            {
                if ( I_stricmp(pValue, "allies") )
                {
                    v4 = va("unknown team '%s', should be axis or allies\n", pValue);
                    Scr_Error(v4, 0);
                }
                else
                {
                    *((unsigned int *)pStruct + 1797) = 2;
                }
            }
            else
            {
                *((unsigned int *)pStruct + 1797) = 1;
            }
            return 1;
        case 23:
            *(unsigned int *)&pStruct[fieldOffset] = Key_StringToKeynum(pValue);
            return 1;
        case 24:
            *(GraphFloat **)&pStruct[fieldOffset] = GraphFloat_Load((char*)pValue);   // nx-port: was stored as unsigned int
            return 1;
        default:
            v5 = va("Bad vehicle field type %i\n", fieldType);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_misctables.cpp", 528, 0, v5) )
                __debugbreak();
            Com_Error(ERR_DROP, "Bad vehicle field type %i", fieldType);
            return 0;
    }
    while ( 1 )
    {
        if ( i >= 7 )
            goto LABEL_7;
        if ( !I_stricmp(pValue, s_vehicleClassNames[i]) )
            break;
        ++i;
    }
    *((_WORD *)pStruct + 32) = i;
LABEL_7:
    if ( i == 7 )
        Com_Error(ERR_DROP, "Unknown vehicle type [%s]", pValue);
    return 1;
}

