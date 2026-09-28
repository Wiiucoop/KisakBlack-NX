#include "xanim_clientnotify.h"

#include <new>
#include <universal/assertive.h>

XAnimClientNotifyList::XAnimClientNotifyList()
{
    this->m_numNotifies = 0;
    //return this;
}

XAnimClientNotifyList::~XAnimClientNotifyList()
{
    int iLoop; // [esp+4h] [ebp-8h]
    XAnimClientNotify *pNotify; // [esp+8h] [ebp-4h]

    pNotify = (XAnimClientNotify *)this;
    for (iLoop = 0; iLoop < this->m_numNotifies; ++iLoop)
    {
        //XAnimClientNotify::~XAnimClientNotify(pNotify++);
        pNotify->~XAnimClientNotify();
        pNotify++;
    }
}

XAnimClientNotifyList *XAnimClientNotifyList::GetNotifyList()
{
    return this;
}

void XAnimClientNotifyList::AddNotify(const ClientNotifyData *notifyData)
{
    XAnimClientNotify *v3; // [esp+Ch] [ebp-10h]
    int iInsertLoc; // [esp+18h] [ebp-4h]

    if ( this->m_numNotifies < 64 )
    {
        for ( iInsertLoc = this->m_numNotifies - 1;
                    iInsertLoc >= 0 && notifyData->timeFrac < Notify(iInsertLoc)->timeFrac;   // nx-port: was [24 * i + 20]
                    --iInsertLoc )
        {
            Notify(iInsertLoc)->swap(Notify(iInsertLoc + 1));
        }
        v3 = Notify(iInsertLoc + 1);
        if (v3)
        {
            //XAnimClientNotify::XAnimClientNotify(v3, notifyData);
            new (v3) XAnimClientNotify(notifyData);
        }
        ++this->m_numNotifies;
    }
}

XAnimClientNotify::XAnimClientNotify(const ClientNotifyData *notifyData)
{
    this->name = 0;
    this->svNotetrackName = 0;
    this->clNotetrackName = 0;
    this->notifyType = 0;
    this->clNotifyName = 0;
    this->timeFrac = 0.0f;
    this->svNotetrackName = notifyData->notetrackName;
    this->notifyType = notifyData->notifyType;
    this->clNotifyName = notifyData->notifyName;
    this->timeFrac = notifyData->timeFrac;
    if ( this->clNotifyName )
        SL_AddRefToString(this->clNotifyName, SCRIPTINSTANCE_CLIENT);
    //return this;
}

XAnimClientNotify::~XAnimClientNotify()
{
    if ( this->clNotetrackName )
    {
        SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, this->clNotetrackName);
        this->clNotetrackName = 0;
    }
    if ( this->clNotifyName )
    {
        SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, this->clNotifyName);
        this->clNotifyName = 0;
    }
}

void __thiscall XAnimClientNotify::swap(XAnimClientNotify *otherNotify)
{
    float *p_timeFrac; // [esp+4h] [ebp-40h]
    float *v3; // [esp+8h] [ebp-3Ch]
    float v4; // [esp+Ch] [ebp-38h]
    unsigned int *p_clNotifyName; // [esp+10h] [ebp-34h]
    unsigned int *v6; // [esp+14h] [ebp-30h]
    unsigned int v7; // [esp+18h] [ebp-2Ch]
    unsigned int *p_notifyType; // [esp+1Ch] [ebp-28h]
    unsigned int *v9; // [esp+20h] [ebp-24h]
    unsigned int v10; // [esp+24h] [ebp-20h]
    unsigned int *p_clNotetrackName; // [esp+28h] [ebp-1Ch]
    unsigned int *v12; // [esp+2Ch] [ebp-18h]
    unsigned int v13; // [esp+30h] [ebp-14h]
    unsigned int *p_svNotetrackName; // [esp+34h] [ebp-10h]
    unsigned int *v15; // [esp+38h] [ebp-Ch]
    unsigned int v16; // [esp+3Ch] [ebp-8h]
    const char *name; // [esp+40h] [ebp-4h]

    if ( this != otherNotify )
    {
        name = this->name;
        this->name = otherNotify->name;
        otherNotify->name = name;
    }
    p_svNotetrackName = &otherNotify->svNotetrackName;
    v15 = &this->svNotetrackName;
    if ( &this->svNotetrackName != &otherNotify->svNotetrackName )
    {
        v16 = *v15;
        *v15 = *p_svNotetrackName;
        *p_svNotetrackName = v16;
    }
    p_clNotetrackName = &otherNotify->clNotetrackName;
    v12 = &this->clNotetrackName;
    if ( &this->clNotetrackName != &otherNotify->clNotetrackName )
    {
        v13 = *v12;
        *v12 = *p_clNotetrackName;
        *p_clNotetrackName = v13;
    }
    p_notifyType = &otherNotify->notifyType;
    v9 = &this->notifyType;
    if ( &this->notifyType != &otherNotify->notifyType )
    {
        v10 = *v9;
        *v9 = *p_notifyType;
        *p_notifyType = v10;
    }
    p_clNotifyName = &otherNotify->clNotifyName;
    v6 = &this->clNotifyName;
    if ( &this->clNotifyName != &otherNotify->clNotifyName )
    {
        v7 = *v6;
        *v6 = *p_clNotifyName;
        *p_clNotifyName = v7;
    }
    p_timeFrac = &otherNotify->timeFrac;
    v3 = &this->timeFrac;
    if ( &this->timeFrac != &otherNotify->timeFrac )
    {
        v4 = *v3;
        *v3 = *p_timeFrac;
        *p_timeFrac = v4;
    }
}

bool XAnimClientNotify::IsClientAnimNotify()
{
    return this->clNotifyName != 0;
}

const char *XAnimClientNotify::GetNotifyStringName()
{
    if ( !this->name )
        this->name = SL_ConvertToString(this->svNotetrackName, SCRIPTINSTANCE_SERVER);
    return this->name;
}

unsigned int __thiscall XAnimClientNotify::GetNotetrackCLName()
{
    char *v1; // eax

    if ( !this->clNotetrackName )
    {
        v1 = SL_ConvertToString(this->svNotetrackName, SCRIPTINSTANCE_SERVER);
        this->clNotetrackName = SL_GetString(v1, 0, SCRIPTINSTANCE_CLIENT);
    }
    return this->clNotetrackName;
}

unsigned int __thiscall XAnimClientNotify::GetNotifyType()
{
    if ( this->clNotifyName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_clientnotify.cpp",
                    128,
                    0,
                    "%s",
                    "clNotifyName == 0") )
    {
        __debugbreak();
    }
    return this->notifyType;
}

unsigned int __thiscall XAnimClientNotify::GetNotifyName()
{
    if ( !this->clNotifyName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_clientnotify.cpp",
                    134,
                    0,
                    "%s",
                    "clNotifyName != 0") )
    {
        __debugbreak();
    }
    return this->clNotifyName;
}

