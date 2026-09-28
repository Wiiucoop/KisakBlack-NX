#pragma once
#include <clientscript/cscr_stringlist.h>

struct ClientNotifyData // sizeof=0x10
{                                       // XREF: XAnimAddClientNotify/r
    unsigned int notetrackName;         // XREF: XAnimAddClientNotify+52/w
    unsigned int notifyType;            // XREF: XAnimAddClientNotify+69/w
    unsigned int notifyName;            // XREF: XAnimAddClientNotify+5F/w
    float timeFrac;                     // XREF: XAnimAddClientNotify+5A/w
};

struct XAnimClientNotifyAdder // sizeof=0x0
{                                       // XREF: ?CG_UpdateViewWeaponAnim@@YAXHH@Z/r
};

struct XAnimClientNotify // sizeof=0x14
{
    const char *name;
    unsigned int svNotetrackName;
    unsigned int clNotetrackName;
    unsigned int notifyType;
    unsigned int clNotifyName;
    float timeFrac;

    XAnimClientNotify(const ClientNotifyData *notifyData);
    ~XAnimClientNotify();

    bool IsClientAnimNotify();
    void swap(XAnimClientNotify *otherNotify);
    const char *GetNotifyStringName();
    unsigned int GetNotifyName();
    unsigned int GetNotifyType();
    unsigned int GetNotetrackCLName();
};

struct XAnimClientNotifyList // sizeof=0x604
{                                       // XREF: ?CG_UpdateViewWeaponAnim@@YAXHH@Z/r
    // nx-port: was [1536], 64 x86 notifies of 24 bytes; XAnimClientNotify holds
    // a pointer, so it is sized from the struct. Index it with Notify(i).
    alignas(XAnimClientNotify) unsigned __int8 m_clientNotifyMemory[64 * sizeof(XAnimClientNotify)];
    int m_numNotifies;

    XAnimClientNotifyList();
    ~XAnimClientNotifyList();

    XAnimClientNotifyList *GetNotifyList();
    XAnimClientNotify *Notify(int i) { return (XAnimClientNotify *)m_clientNotifyMemory + i; }
    void AddNotify(const ClientNotifyData *notifyData);
};


