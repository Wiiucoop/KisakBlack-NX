#include "cg_vehicles_mp.h"
#include <client_mp/cl_main_mp.h>
#include "cg_actors_mp.h"
#include "cg_ents_mp.h"
#include "cg_local_mp.h"
#include <cgame/cg_main.h>
#include <bgame/bg_vehicles_mp.h>

DObj *__cdecl GetVehicleEntDObj(int localClientNum, centity_s *centVeh)
{
    const cgs_t *cgs; // [esp+4h] [ebp-8h]

    if ( ((centVeh->clientFlags >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    60,
                    0,
                    "%s",
                    "centVeh->nextValid") )
    {
        __debugbreak();
    }
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    return CG_PreProcess_GetDObj(
                     localClientNum,
                     centVeh->nextState.number,
                     centVeh->nextState.eType,
                     cgs->gameModels[centVeh->nextState.un3.item],
                     0);
}

bool __cdecl CG_VehEntityUsingVehicle(int localClientNum, int entNum)
{
    centity_s *veh; // [esp+10h] [ebp-Ch]
    clientInfo_t *ci; // [esp+18h] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    if ( !ci )
        return 0;
    if ( ci->attachedVehEntNum == 1023 || ci->attachedVehEntNum <= 44 )
        return 0;
    if ( (CG_GetEntity(localClientNum, entNum)->nextState.lerp.eFlags & 0x4000) == 0 )
        return 0;
    veh = CG_GetEntity(localClientNum, ci->attachedVehEntNum);
    return veh && veh->nextState.eType == 14;
}

clientInfo_t *__cdecl ClientInfoForEntity(int localClientNum, int entNum)
{
    centity_s *cent; // [esp+8h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entNum);
    if ( cent->nextState.eType != 1 )
        return 0;
    if ( cent->nextState.eType >= 21 )
        return 0;
    if ( cent->nextState.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    50,
                    0,
                    "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cent->nextState.clientNum,
                    32) )
    {
        __debugbreak();
    }
    return &bgs->clientinfo[cent->nextState.clientNum];
}

bool __cdecl CG_VehEntityUsingRemoteControlVehicle(int localClientNum, int entNum)
{
    centity_s *veh; // [esp+10h] [ebp-Ch]
    clientInfo_t *ci; // [esp+18h] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    if ( !ci )
        return 0;
    if ( ci->attachedVehEntNum == 1023 || ci->attachedVehEntNum <= 44 )
        return 0;
    if ( (CG_GetEntity(localClientNum, entNum)->nextState.lerp.eFlags & 0x4000) == 0 )
        return 0;
    veh = CG_GetEntity(localClientNum, ci->attachedVehEntNum);
    return veh
            && (veh->nextState.eType == 14 || veh->nextState.eType == 12 || veh->nextState.eType == 13)
            && CG_IsVehicleRemoteControl(veh->nextState.vehicleState.vehicleInfoIndex);
}

const vehicle_info_t *__cdecl CG_GetVehicleInfo(int index)
{
    return BG_GetVehicleInfo(index);
}

int __cdecl CG_VehPlayerVehicleSeat(int localClientNum, int entNum)
{
    clientInfo_t *ci; // [esp+0h] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    if ( !ci && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp", 168, 0, "%s", "ci") )
        __debugbreak();
    if ( ci->attachedVehEntNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    169,
                    0,
                    "%s",
                    "ci->attachedVehEntNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    return ci->attachedVehSeat;
}

void __cdecl CG_VehSeatTransformForPlayer(int localClientNum, int entNum, float *resultOrigin, float *resultAngles)
{
    clientInfo_t *ci; // [esp+8h] [ebp-8h]
    centity_s *centPlayer; // [esp+Ch] [ebp-4h]

    ci = ClientInfoForEntity(localClientNum, entNum);
    centPlayer = CG_GetEntity(localClientNum, entNum);
    if ( centPlayer->nextState.eType != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    246,
                    0,
                    "%s",
                    "centPlayer->nextState.eType == ET_PLAYER") )
    {
        __debugbreak();
    }
    if ( centPlayer->nextState.eType >= 21
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    247,
                    0,
                    "%s",
                    "centPlayer->nextState.eType < ET_EVENTS") )
    {
        __debugbreak();
    }
    if ( !ci->attachedVehEntNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    248,
                    0,
                    "%s",
                    "ci->attachedVehEntNum != 0") )
    {
        __debugbreak();
    }
    SeatTransformForClientInfo(localClientNum, ci, resultOrigin, resultAngles);
}

void __cdecl SeatTransformForClientInfo(int localClientNum, clientInfo_t *ci, float *resultOrigin, float *resultAngles)
{
    if ( !ci && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp", 232, 0, "%s", "ci") )
        __debugbreak();
    if ( ci->attachedVehEntNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    233,
                    0,
                    "%s",
                    "ci->attachedVehEntNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    SeatTransformForSlot(localClientNum, ci->attachedVehEntNum, ci->attachedVehSeat, resultOrigin, resultAngles);
}

void __cdecl SeatTransformForSlot(
                int localClientNum,
                int vehEntNum,
                int vehSlotIdx,
                float *resultOrigin,
                float *resultAngles)
{
    unsigned __int16 tagName; // [esp+0h] [ebp-34h]
    float tagOrigin[3]; // [esp+4h] [ebp-30h] BYREF
    float tagMtx[3][3]; // [esp+10h] [ebp-24h] BYREF

    tagName = BG_VehiclesGetSlotTagName(vehSlotIdx);
    GetTagMatrix(localClientNum, vehEntNum, tagName, tagMtx, tagOrigin);
    if ( resultAngles )
        AxisToAngles(tagMtx, resultAngles);
    if ( resultOrigin )
    {
        *resultOrigin = tagOrigin[0];
        resultOrigin[1] = tagOrigin[1];
        resultOrigin[2] = tagOrigin[2];
    }
}

void __cdecl GetTagMatrix(
                int localClientNum,
                int vehEntNum,
                unsigned __int16 tagName,
                float (*resultTagMat)[3],
                float *resultOrigin)
{
    centity_s *centVeh; // [esp+8h] [ebp-8h]
    DObj *objVeh; // [esp+Ch] [ebp-4h]

    centVeh = CG_GetEntity(localClientNum, vehEntNum);
    if ( centVeh->nextState.eType != 14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    180,
                    0,
                    "%s",
                    "centVeh->nextState.eType == ET_VEHICLE") )
    {
        __debugbreak();
    }
    if ( centVeh->nextState.eType == 14 )
    {
        if ( ((centVeh->clientFlags >> 1) & 1) != 0 )
        {
            objVeh = GetVehicleEntDObj(localClientNum, centVeh);
            if ( objVeh )
            {
                CG_DObjGetWorldTagMatrix(&centVeh->pose, objVeh, tagName, resultTagMat, resultOrigin);
            }
            else
            {
                Com_PrintWarning(14, "GetTagMatrix() unable to get vehicle DObj.\n");
                MatrixIdentity33(resultTagMat);
                *resultOrigin = 0.0f;
                resultOrigin[1] = 0.0f;
                resultOrigin[2] = 0.0f;
            }
        }
        else
        {
            Com_PrintWarning(14, "Vehicle being used by a player was not included in latest snapshot.\n");
            MatrixIdentity33(resultTagMat);
            *resultOrigin = 0.0f;
            resultOrigin[1] = 0.0f;
            resultOrigin[2] = 0.0f;
        }
    }
    else
    {
        Com_PrintWarning(14, "Calling GetTagMarix on an entity that is not a vehicle.\n");
        MatrixIdentity33(resultTagMat);
        *resultOrigin = 0.0f;
        resultOrigin[1] = 0.0f;
        resultOrigin[2] = 0.0f;
    }
}

double __cdecl CG_VehGetHealthPercentageEntity(const centity_s *cent)
{
    return (double)(cent->nextState.time2 & 0x3F) / 63.0;
}

double __cdecl CG_VehGetHealthPercentageLeftTread(const centity_s *cent)
{
    return (double)((cent->nextState.time2 & 0xFC0) >> 6) / 63.0;
}

double __cdecl CG_VehGetHealthPercentageRightTread(const centity_s *cent)
{
    return (double)((cent->nextState.time2 & 0x3F000) >> 12) / 63.0;
}

int __cdecl CG_VehGetSeatOccupancyFlags(const centity_s *cent)
{
    return (cent->nextState.time2 & 0xC0000) >> 18;
}

const int yawturn_masks[2] = { 1048576, 4194304 };
const int pitchturn_masks[2] = { 2097152, 8388608 };
const int overheating_shifts[2] = { 24, 25 };
const int overheating_masks[2] = { 16777216, 33554432 };

bool __cdecl CG_VehGetSeatGunTurningYaw(const centity_s *cent, unsigned int seatIndex)
{
    if ( seatIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    316,
                    0,
                    "%s",
                    "seatIndex >= 0 && seatIndex < ( sizeof( _yawturn_masks ) / sizeof ( _yawturn_masks[ 0 ] ) )") )
    {
        __debugbreak();
    }
    return (yawturn_masks[seatIndex] & cent->nextState.time2) >> yawturn_masks[seatIndex] != 0;
}

bool __cdecl CG_VehGetSeatGunTurningPitch(const centity_s *cent, unsigned int seatIndex)
{
    if ( seatIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    322,
                    0,
                    "%s",
                    "seatIndex >= 0 && seatIndex < ( sizeof( _pitchturn_masks ) / sizeof ( _pitchturn_masks[ 0 ] ) )") )
    {
        __debugbreak();
    }
    return (pitchturn_masks[seatIndex] & cent->nextState.time2) >> pitchturn_masks[seatIndex] != 0;
}

bool __cdecl CG_VehGetSeatGunOverheating(const centity_s *cent, unsigned int seatIndex)
{
    if ( seatIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_vehicles_mp.cpp",
                    328,
                    0,
                    "%s",
                    "seatIndex >= 0 && seatIndex < ( sizeof( _overheating_masks ) / sizeof ( _overheating_masks[ 0 ] ) )") )
    {
        __debugbreak();
    }
    return (overheating_masks[seatIndex] & cent->nextState.time2) >> overheating_shifts[seatIndex] != 0;
}

