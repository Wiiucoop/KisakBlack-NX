#include "cg_animscripted_mp.h"
#include "cg_local_mp.h"
#include "cg_ents_mp.h"
#include <qcommon/dobj_management.h>
#include <universal/com_math_anglevectors.h>
#include <clientscript/cscr_memorytree.h>
#include <client_mp/cl_cgame_mp.h>
#include <clientscript/cscr_stringlist.h>

void __cdecl CG_GetTagMatrix(int localClientNum, int linkEntNum, unsigned __int16 tagName, float (*resultTagMat)[3])
{
    centity_s *centLink; // [esp+14h] [ebp-8h]
    DObj *objLink; // [esp+18h] [ebp-4h]

    centLink = CG_GetEntity(localClientNum, linkEntNum);
    if ( ((centLink->clientFlags >> 1) & 1) != 0 )
    {
        objLink = GetLinkEntDObj(localClientNum, centLink);
        if ( !objLink || !CG_DObjGetWorldTagMatrix(&centLink->pose, objLink, tagName, resultTagMat, &(*resultTagMat)[9]) )
        {
            MatrixIdentity33(resultTagMat);
            (*resultTagMat)[9] = 0.0f;
            (*resultTagMat)[10] = 0.0f;
            (*resultTagMat)[11] = 0.0f;
        }
    }
    else
    {
        Com_PrintWarning(14, "An actor is linking to an entity that was not included in latest snapshot.\n");
        MatrixIdentity33(resultTagMat);
        (*resultTagMat)[9] = 0.0f;
        (*resultTagMat)[10] = 0.0f;
        (*resultTagMat)[11] = 0.0f;
    }
}

DObj *__cdecl GetLinkEntDObj(int localClientNum, centity_s *centLink)
{
    if ( ((centLink->clientFlags >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    12,
                    0,
                    "%s",
                    "centLink->nextValid") )
    {
        __debugbreak();
    }
    return Com_GetClientDObj(centLink->nextState.number, localClientNum);
}

void __cdecl CG_CalcTagParentAxis(int localClientNum, centity_s *cent, float (*parentAxis)[3])
{
    centity_s *centLink; // [esp+40h] [ebp-8h]
    cLinkInfo_s *linkInfo; // [esp+44h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 54, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    linkInfo = cent->linkInfo;
    if ( !linkInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 57, 0, "%s", "linkInfo") )
    {
        __debugbreak();
    }
    if ( linkInfo->linkTag <= 0 )
    {
        centLink = CG_GetEntity(localClientNum, linkInfo->linkEnt);
        AnglesToAxis(centLink->pose.angles, parentAxis);
        (*parentAxis)[9] = centLink->pose.origin[0];
        (*parentAxis)[10] = centLink->pose.origin[1];
        (*parentAxis)[11] = centLink->pose.origin[2];
    }
    else
    {
        CG_GetTagMatrix(localClientNum, linkInfo->linkEnt, linkInfo->linkTag, parentAxis);
    }
    if ( ((LODWORD((*parentAxis)[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    71,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[0])[0]) && !IS_NAN((parentAxis[0])[1]) && !IS_NAN((parentAxis[0])[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD((*parentAxis)[3]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[4]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[5]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    72,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[1])[0]) && !IS_NAN((parentAxis[1])[1]) && !IS_NAN((parentAxis[1])[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD((*parentAxis)[6]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[7]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[8]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    73,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[2])[0]) && !IS_NAN((parentAxis[2])[1]) && !IS_NAN((parentAxis[2])[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD((*parentAxis)[9]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[10]) & 0x7F800000) == 0x7F800000
         || (LODWORD((*parentAxis)[11]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    74,
                    0,
                    "%s",
                    "!IS_NAN((parentAxis[3])[0]) && !IS_NAN((parentAxis[3])[1]) && !IS_NAN((parentAxis[3])[2])") )
    {
        __debugbreak();
    }
}

void __cdecl CG_LinkTransformForEntity(int localClientNum, centity_s *cent, float *resultOrigin, float *resultAngles)
{
    float *v4; // [esp+4h] [ebp-80h]
    float matrix[4][3]; // [esp+Ch] [ebp-78h] BYREF
    float origin[3]; // [esp+3Ch] [ebp-48h]
    float angles[3]; // [esp+48h] [ebp-3Ch] BYREF
    float parentAxis[4][3]; // [esp+54h] [ebp-30h] BYREF

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 89, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    CG_CalcTagParentAxis(localClientNum, cent, parentAxis);
    if ( cent->nextState.clientLinkInfo.parentEnt )
    {
        AnglesToAxis(cent->nextState.lerp.apos.trDelta, cent->linkInfo->axis);
        v4 = cent->linkInfo->axis[3];
        *v4 = cent->nextState.lerp.pos.trDelta[0];
        v4[1] = cent->nextState.lerp.pos.trDelta[1];
        v4[2] = cent->nextState.lerp.pos.trDelta[2];
    }
    MatrixMultiply43(cent->linkInfo->axis, parentAxis, matrix);
    AxisToAngles(matrix, angles);
    *(_QWORD *)origin = *(_QWORD *)&matrix[3][0];
    origin[2] = matrix[3][2];
    if ( resultOrigin )
    {
        *resultOrigin = origin[0];
        resultOrigin[1] = origin[1];
        resultOrigin[2] = origin[2];
    }
    if ( resultAngles )
    {
        *resultAngles = angles[0];
        resultAngles[1] = angles[1];
        resultAngles[2] = angles[2];
    }
}

void __cdecl CG_GenerateLinkInfo(int localClientNum, centity_s *cent, int attachedEntNum, int attachedTagIndex)
{
    float *v4; // [esp+0h] [ebp-A4h]
    float *relative_angles; // [esp+10h] [ebp-94h]
    float invParentAxis[4][3]; // [esp+14h] [ebp-90h] BYREF
    float parentAxis[4][3]; // [esp+44h] [ebp-60h] BYREF
    float axis[4][3]; // [esp+74h] [ebp-30h] BYREF

    if ( !cent->linkInfo )
    {
        cent->linkInfo = (cLinkInfo_s *)MT_Alloc(72, 18, SCRIPTINSTANCE_SERVER);
        cent->linkInfo->linkEnt = 1023;
        relative_angles = cent->linkInfo->relative_angles;
        *relative_angles = 0.0f;
        relative_angles[1] = 0.0f;
        relative_angles[2] = 0.0f;
        cent->linkInfo->angles_set = 0;
    }
    if ( cent->linkInfo->linkEnt != attachedEntNum )
    {
        cent->linkInfo->linkEnt = attachedEntNum;
        if ( attachedTagIndex )
            cent->linkInfo->linkTag = attachedTagIndex;
        else
            cent->linkInfo->linkTag = -1;
        if ( cent->linkInfo->linkTag <= 0 )
        {
            v4 = cent->linkInfo->axis[3];
            *v4 = cent->nextState.lerp.pos.trBase[0];
            v4[1] = cent->nextState.lerp.pos.trBase[1];
            v4[2] = cent->nextState.lerp.pos.trBase[2];
            AnglesToAxis(cent->nextState.lerp.apos.trBase, cent->linkInfo->axis);
        }
        else
        {
            AnglesToAxis(cent->nextState.lerp.apos.trBase, axis);
            axis[3][0] = cent->nextState.lerp.pos.trBase[0];
            axis[3][1] = cent->nextState.lerp.pos.trBase[1];
            axis[3][2] = cent->nextState.lerp.pos.trBase[2];
            CG_GetTagMatrix(localClientNum, cent->linkInfo->linkEnt, cent->linkInfo->linkTag, parentAxis);
            MatrixInverseOrthogonal43(parentAxis, invParentAxis);
            MatrixMultiply43(axis, invParentAxis, cent->linkInfo->axis);
        }
    }
}

void __cdecl CG_UpdateEntityLink(int localClientNum, centity_s *cent)
{
    char *tagName; // [esp+0h] [ebp-Ch]
    __int16 parentEnt; // [esp+4h] [ebp-8h]
    __int16 parentEnta; // [esp+4h] [ebp-8h]
    unsigned int tagIndex; // [esp+8h] [ebp-4h]

    if ( cent->nextState.number < 1024 )
    {
        parentEnt = cent->nextState.clientLinkInfo.parentEnt;
        if ( parentEnt )
        {
            parentEnta = parentEnt - 1;
            if ( cent->nextState.clientLinkInfo.tagIndex >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                            172,
                            0,
                            "%s",
                            "cent->nextState.clientLinkInfo.tagIndex < MAX_TAGS") )
            {
                __debugbreak();
            }
            tagIndex = 0;
            if ( cent->nextState.clientLinkInfo.tagIndex )
            {
                tagName = CL_GetConfigString(cent->nextState.clientLinkInfo.tagIndex + 3115);
                tagIndex = SL_FindString(tagName, SCRIPTINSTANCE_SERVER);
            }
            if ( !cent->linkInfo || cent->linkInfo->linkEnt != parentEnta || cent->linkInfo->linkTag != tagIndex )
                CG_GenerateLinkInfo(localClientNum, cent, parentEnta, tagIndex);
        }
        else if ( cent->linkInfo )
        {
            cent->linkInfo->linkEnt = 1023;
        }
    }
}

void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent, int parentNum, int tagIndex)
{
    cLinkInfo_s *linkInfo; // [esp+0h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 188, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.number >= 0x600u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    189,
                    0,
                    "cent->nextState.number doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    cent->nextState.number,
                    1536) )
    {
        __debugbreak();
    }
    linkInfo = cent->linkInfo;
    if ( parentNum == 1023 )
    {
        if ( linkInfo )
        {
            MT_Free((unsigned char*)linkInfo, 72, SCRIPTINSTANCE_SERVER);
            cent->linkInfo = 0;
        }
    }
    else
    {
        CG_GenerateLinkInfo(localClientNum, cent, parentNum, tagIndex);
    }
}

void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent)
{
    cLinkInfo_s *linkInfo; // [esp+Ch] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp", 207, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.number >= 0x600u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    208,
                    0,
                    "cent->nextState.number doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    cent->nextState.number,
                    1536) )
    {
        __debugbreak();
    }
    linkInfo = cent->linkInfo;
    if ( linkInfo
        && linkInfo->linkEnt != 1023
        && ((*((unsigned int *)CG_GetEntity(localClientNum, cent->linkInfo->linkEnt) + 201) >> 1) & 1) == 0 )
    {
        MT_Free((unsigned char*)linkInfo, 72, SCRIPTINSTANCE_SERVER);
        cent->linkInfo = 0;
    }
}

bool __cdecl CG_EntityLinked(int localClientNum, centity_s *cent)
{
    return CG_EntGetLinkToParent(localClientNum, cent) != 0;
}

centity_s *__cdecl CG_EntGetLinkToParent(int localClientNum, centity_s *cent)
{
    centity_s *centParent; // [esp+8h] [ebp-4h]

    if ( !cent->linkInfo || cent->linkInfo->linkEnt == 1023 )
        return 0;
    if ( cent->linkInfo->linkEnt >= 0x600u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    234,
                    0,
                    "%s",
                    "cent->linkInfo->linkEnt >= 0 && cent->linkInfo->linkEnt < MAX_LOCAL_CENTITIES") )
    {
        __debugbreak();
    }
    centParent = CG_GetEntity(localClientNum, cent->linkInfo->linkEnt);
    if ( !centParent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_animscripted_mp.cpp",
                    236,
                    0,
                    "%s",
                    "centParent") )
    {
        __debugbreak();
    }
    if ( ((centParent->clientFlags >> 1) & 1) != 0 )
        return centParent;
    Com_PrintWarning(
        14,
        "Entity #%i of type %i: parent ent #%i not in snapshot.\n",
        cent->nextState.number,
        cent->nextState.eType,
        cent->linkInfo->linkEnt);
    return 0;
}

