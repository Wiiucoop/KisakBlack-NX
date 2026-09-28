#pragma once
#include "cscr_main.h"
#include "cscr_variable.h"

struct scr_localVar_t // sizeof=0x8
{                                       // XREF: scr_block_s/r
                                        // Scr_MergeChildBlocks/r ...
    unsigned int name;                  // XREF: Scr_MergeChildBlocks+AE/w
                                        // Scr_MergeChildBlocks+B4/r ...
    unsigned int sourcePos;             // XREF: Scr_MergeChildBlocks+B1/w
                                        // Scr_MergeChildBlocks+134/r ...
};

struct scr_block_s // sizeof=0x218
{                                       // XREF: scr_block_t/r
    int abortLevel;
    int localVarsCreateCount;
    int localVarsPublicCount;
    int localVarsCount;
    unsigned __int8 localVarsInitBits[8];
    scr_localVar_t localVars[64];
};

enum ParseToken_t : __int32;
union sval_u // sizeof=0x4
{                                       // XREF: GetExpressionCount(sval_u)+D/r
                                        // Scr_GetBuiltin(scriptInstance_t,sval_u)+3/r ...
    // nx-port: the union is pointer-sized on LP64; the integer forms zero all
    // of it first, so a node read through .node never sees stale high bytes.
    sval_u()
    {
        node = 0;
    }
    sval_u(int val)
    {
        node = 0;
        intValue = val;
    }
    sval_u(unsigned int val)
    {
        node = 0;
        sourcePosValue = val;
    }
    //unsigned __int8 type;
    ParseToken_t type; // this is changed to int32, but shouldn't matter cuz of the union I believe
    unsigned int stringValue;
    unsigned int idValue;
    float floatValue;
    int intValue;
    sval_u *node;
    unsigned int sourcePosValue;
    const char *codePosValue;
    const char *debugString;
    scr_block_s *block;
};

struct debugger_sval_s // sizeof=0x4
{                                       // XREF: debugger_sval_t/r
    debugger_sval_s *next;
};

struct ScriptExpression_t // sizeof=0xC
{                                       // XREF: Scr_WatchElement_s/r
                                        // ?AddElement@Scr_ScriptWatch@@QAEXW4scriptInstance_t@@PAUScr_WatchElement_s@@PBD@Z/r
    sval_u parseData;                   // XREF: Scr_ScriptWatch::AddElement(scriptInstance_t,Scr_WatchElement_s *,char const *)+5E/r
                                        // Scr_ScriptWatch::AddElement(scriptInstance_t,Scr_WatchElement_s *,char const *)+D0/r ...
    int breakonExpr;                    // XREF: Scr_ScriptWatch::AddElement(scriptInstance_t,Scr_WatchElement_s *,char const *)+1CD/r
    debugger_sval_s *exprHead;          // XREF: Scr_ScriptWatch::AddElement(scriptInstance_t,Scr_WatchElement_s *,char const *):loc_8AD863/w
                                        // Scr_ScriptWatch::AddElement(scriptInstance_t,Scr_WatchElement_s *,char const *)+1D3/r
};

struct Scr_WatchElement_s // sizeof=0x64
{                                       // XREF: Scr_WatchElement_t/r
    ScriptExpression_t expr;
    const char *valueText;
    const char *refText;
    bool directObject;
    // padding byte
    // padding byte
    // padding byte
    unsigned int objectId;
    unsigned __int8 objectType;
    unsigned __int8 oldObjectType;
    bool expand;
    unsigned __int8 breakpointType;
    bool hitBreakpoint;
    bool changed;
    bool valueDefined;
    bool threadList;
    bool endonList;
    // padding byte
    // padding byte
    // padding byte
    VariableValue value;
    unsigned int fieldName;
    unsigned int childCount;
    unsigned int hardcodedCount;
    int id;
    struct Scr_Breakpoint *breakpoint;
    const char *deadCodePos;
    unsigned int bufferIndex;
    unsigned int sourcePos;
    int changedTime;
    Scr_WatchElement_s *parent;
    Scr_WatchElement_s *childArrayHead;
    Scr_WatchElement_s *childHead;
    Scr_WatchElement_s *next;
};

struct Scr_Breakpoint // sizeof=0x1C
{                                       // XREF: .data:g_breakpoints/r
    int line;
    unsigned int bufferIndex;
    char *codePos;
    Scr_WatchElement_s *element;
    int builtinIndex;
    Scr_Breakpoint *next;               // XREF: Scr_InitBreakpoints+31/w
                                        // Scr_InitBreakpoints:loc_8AF469/r
    Scr_Breakpoint **prev;
};

struct Scr_SourcePos_t // sizeof=0xC
{                                       // XREF: scrDebuggerGlob_t/r
                                        // ?Scr_GetCodePos@@YAXW4scriptInstance_t@@PBDIPADH@Z/r ...
    unsigned int bufferIndex;           // XREF: Scr_ResumeBreakpoints+1D/w
                                        // Scr_ScriptWindow::EnterCallInternal(scriptInstance_t)+179/r ...
    int lineNum;                        // XREF: Scr_SetTempBreakpoint+86/r
                                        // Scr_ScriptWindow::EnterCallInternal(scriptInstance_t)+185/r ...
    unsigned int sourcePos;             // XREF: Scr_GetCodePos(scriptInstance_t,char const *,uint,char *,int)+96/r
};

struct Scr_OpcodeList_s // sizeof=0x8
{                                       // XREF: Scr_OpcodeList_t/r
    char *codePos;
    Scr_OpcodeList_s *next;
};

struct Scr_WatchElementNode_s // sizeof=0x8
{                                       // XREF: Scr_WatchElementNode_t/r
    Scr_WatchElement_s *element;
    Scr_WatchElementNode_s *next;
};

struct Scr_WatchElementDoubleNode_t // sizeof=0x8
{
    Scr_WatchElementNode_s *list;
    Scr_WatchElementNode_s *removedList;
};

class UI_Component
{
};


struct UI_VerticalDivider : UI_Component // sizeof=0x0
{                                       // XREF: scrDebuggerGlob_t/r
};

struct UI_ScrollPane : UI_Component // sizeof=0x0
{                                       // XREF: scrDebuggerGlob_t/r
};

class UI_LinesComponent : public UI_Component // sizeof=0x8
{                                       // XREF: Scr_ScriptWindow/r
public:
                                        // Scr_AbstractScriptList/r ...
    int selectedLine;
    int numLines;                       // XREF: Scr_DisplayDebuggerRemote+E9/r
                                        // Scr_DisplayDebuggerRemote+116/r
};

class Scr_ScriptWindow : public UI_LinesComponent // sizeof=0x20
{
public:
    void *operator new(size_t size)
    {
        return Hunk_UserAlloc(g_DebugHunkUser, size, 4, "Scr_ScriptWindow");
    }
    void operator delete(void *ptr)
    {
        Hunk_UserFree(g_DebugHunkUser, ptr);
    }

    unsigned int bufferIndex;
    int currentTopLine;
    const char *currentBufPos;
    Scr_Breakpoint *breakpointHead;
    Scr_Breakpoint *builtinHead;
    int numCols;

public:
    void SetScriptFile(scriptInstance_t inst, const char *name);
    void AddBreakpoint(
        scriptInstance_t inst,
        Scr_Breakpoint **pBreakpoint,
        char *codePos,
        int builtinIndex,
        Scr_WatchElement_s *element,
        unsigned __int8 type);
    void GetSourcePos(
        scriptInstance_t inst,
        unsigned int *start,
        unsigned int *end);
    char AddBreakpointAtSourcePos(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        unsigned __int8 breakpointType,
        bool user,
        Scr_Breakpoint **pBreakpoint,
        unsigned int startSourcePos,
        unsigned int endSourcePos);
    void ToggleBreakpointInternal(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        bool force,
        bool overwrite,
        unsigned __int8 breakpointType,
        bool user);
    char * GetBreakpointCodePos(scriptInstance_t inst);
    void RunToCursor(scriptInstance_t inst);
    void EnterCallInternal(scriptInstance_t inst);
    void Init(scriptInstance_t inst);
};

struct Scr_SourcePos2_t // sizeof=0x8
{                                       // XREF: Scr_ScriptCallStack/r
    unsigned int bufferIndex;           // XREF: Scr_DisplayDebuggerRemote+12A/r
    unsigned int sourcePos;             // XREF: Scr_DisplayDebuggerRemote+146/r
};

class Scr_ScriptCallStack : public UI_LinesComponent
{
public:
    Scr_SourcePos2_t stack[33];

public:
    void UpdateStack(scriptInstance_t inst);
};

class Scr_AbstractScriptList : public UI_LinesComponent // sizeof=0xC
{
public:
    Scr_ScriptWindow **scriptWindows;   // XREF: Scr_ToggleBreakpointRemote+17/r
                                        // Scr_RunToCursorRemote+17/r ...
public:
    void Init(scriptInstance_t inst);
    void Shutdown(scriptInstance_t inst);
};

struct Scr_OpenScriptList : Scr_AbstractScriptList // sizeof=0xC
{                                       // XREF: scrDebuggerGlob_t/r
};

struct Scr_AddFileInfo // sizeof=0xC
{                                       // XREF: ?Init@Scr_ScriptList@@QAEXW4scriptInstance_t@@@Z/r
    int maxNumCols;                     // XREF: Scr_ScriptList::Init(scriptInstance_t)+183/w
    int to;                             // XREF: Scr_ScriptList::Init(scriptInstance_t)+197/w
    int from;                           // XREF: Scr_ScriptList::Init(scriptInstance_t)+18D/w
};

class Scr_ScriptList : public Scr_AbstractScriptList // sizeof=0xC
{                                       // XREF: scrDebuggerGlob_t/r
public:
    void AddFile(
        scriptInstance_t inst,
        const char *filename,
        Scr_AddFileInfo *info);
    void Init(scriptInstance_t inst);
    void Shutdown(scriptInstance_t inst);
};

class Scr_ScriptWatch: UI_LinesComponent // sizeof=0x18
{
public:
    Scr_WatchElement_s *elementHead;    // XREF: Scr_ResumeBreakpoints+30/r
                                        // Scr_CheckBreakonNotify(scriptInstance_t,uint,uint,VariableValue *,char const *,uint)+FF/r ...
    int elementId;
    unsigned int localId;               // XREF: Scr_CheckBreakonNotify(scriptInstance_t,uint,uint,VariableValue *,char const *,uint)+59/w
                                        // Scr_CheckBreakonNotify(scriptInstance_t,uint,uint,VariableValue *,char const *,uint)+E1/w ...
    int dirty;                          // XREF: Scr_UpdateDebugSocket(scriptInstance_t)+F/r
                                        // Scr_UpdateDebugSocket(scriptInstance_t)+1B7/r
public:
    Scr_WatchElement_s * GetElementWithId_r(Scr_WatchElement_s *element, int id);
    Scr_WatchElement_s * GetElementWithId(int id);
    Scr_WatchElement_s ** GetElementRef(Scr_WatchElement_s *element);
    Scr_WatchElement_s * GetElementPrev(Scr_WatchElement_s *element);
    void ToggleExpandElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    void ExpandElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        bool expand);
    Scr_WatchElement_s *CreateBreakpointElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        unsigned int bufferIndex,
        unsigned int sourcePos,
        bool user);
    Scr_WatchElement_s *PasteNonBreakpointElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        char *text,
        bool user);
    Scr_WatchElement_s *DeleteElementInternal(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    Scr_WatchElement_s *BackspaceElementInternal(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    void Init(scriptInstance_t inst);
    void Shutdown(scriptInstance_t inst);
    void FreeWatchElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    void EvaluateWatchChildren(
        scriptInstance_t inst,
        Scr_WatchElement_s *parentElement);
    char PostEvaluateWatchElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        VariableValue *value);
    char EvaluateWatchChildElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        unsigned int fieldName,
        Scr_WatchElement_s *childElement,
        bool hardcodedField);
    void EvaluateWatchElementExpression(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        VariableValue *value);
    void EvaluateWatchElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    Scr_WatchElement_s *CreateWatchElement(
        scriptInstance_t inst,
        char *text,
        Scr_WatchElement_s **prevElem,
        const char *name);
    void AddElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        char *text);
    Scr_WatchElement_s *CloneElement(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    void Evaluate(scriptInstance_t inst);
    void DisplayThreadPos(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    void UpdateBreakpoints(scriptInstance_t inst, bool add);
    Scr_WatchElement_s *AddBreakpoint(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        unsigned __int8 type);
    Scr_WatchElement_s *RemoveBreakpoint(
        scriptInstance_t inst,
        Scr_WatchElement_s *element);
    void ToggleWatchElementBreakpoint(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        unsigned __int8 type);
    void ToggleBreakpointInternal(
        scriptInstance_t inst,
        Scr_WatchElement_s *element,
        unsigned __int8 type);
    void SortHitBreakpointsTop(scriptInstance_t inst);

};

struct scrDebuggerGlob_t // sizeof=0x1C4
{                                       // XREF: .data:scrDebuggerGlob_t * gScrDebuggerGlob/r
    int prevMouseTime;
    float prevMousePos[2];
    UI_ScrollPane scriptScrollPane;
    UI_ScrollPane miscScrollPane;
    // padding byte
    // padding byte
    Scr_ScriptList scriptList;          // XREF: Scr_InitDebugger(scriptInstance_t)+D8/o
    // Scr_ShutdownDebugger(scriptInstance_t)+67/o ...
    Scr_OpenScriptList openScriptList;  // XREF: Scr_InitDebugger(scriptInstance_t)+F0/o
    // Scr_ShutdownDebugger(scriptInstance_t)+4F/o
    Scr_ScriptWatch scriptWatch;        // XREF: Scr_ReadElement(scriptInstance_t)+21/o
    // Scr_ScriptWindow::AddBreakpointAtSourcePos(scriptInstance_t,Scr_WatchElement_s *,uchar,bool,Scr_Breakpoint * *,uint,uint)+1FB/o ...
    Scr_ScriptCallStack scriptCallStack; // XREF: Scr_DisplayDebugger+24/o
    // Scr_DisplayDebuggerRemote+E9/r ...
    UI_VerticalDivider mainWindow;
    // padding byte
    // padding byte
    // padding byte
    char *breakpoints;                  // XREF: Scr_FindBreakpointInfo+85/r
    // Scr_InitDebugger(scriptInstance_t)+9B/w ...
    int breakpointOpcode;               // XREF: Scr_SpecialBreakpoint+113/w
    // Scr_Step+BF/r ...
    const char *breakpointCodePos;      // XREF: Scr_ScriptCallStack::UpdateStack(scriptInstance_t)+7B/r
    // Scr_SpecialBreakpoint+101/w ...
    Scr_SourcePos_t breakpointPos;      // XREF: Scr_ResumeBreakpoints+1D/w
    // Scr_SetTempBreakpoint+86/r ...
    bool atBreakpoint;                  // XREF: Scr_ScriptWindow::RunToCursor(scriptInstance_t)+1B/r
    // Scr_ResumeBreakpoints+D/w ...
    volatile bool abort;                // XREF: Scr_MonitorCommand(char const *,scriptInstance_t)+67/w
    // Scr_ResetAbortDebugger(scriptInstance_t)+C/w ...
// padding byte
// padding byte
    char *nextBreakpointCodePos;        // XREF: Scr_AddManualBreakpoint(scriptInstance_t,uchar *)+57/r
    // Scr_RemoveManualBreakpoint+C/r ...
    unsigned int nextBreakpointThreadId;
    // XREF: Scr_SetTempBreakpoint+9E/w
    // Scr_HitBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+131/r ...
    bool nextBreakpointCodePosMasked;   // XREF: Scr_AddManualBreakpoint(scriptInstance_t,uchar *)+91/r
    // Scr_AddManualBreakpoint(scriptInstance_t,uchar *)+CB/w ...
// padding byte
// padding byte
// padding byte
    char *killThreadCodePos;            // XREF: Scr_SetTempBreakpoint+17/r
    // Scr_InitDebuggerSystem(scriptInstance_t)+74/w ...
    bool kill_thread;                   // XREF: Scr_HitBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+161/r
    // Scr_HitBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+17F/w ...
// padding byte
// padding byte
// padding byte
    VariableValue *breakpointTop;       // XREF: Scr_SpecialBreakpoint+EF/w
    // Scr_Step+13B/r ...
    bool run_debugger;                  // XREF: Scr_InitDebuggerSystem(scriptInstance_t)+111/w
    // Scr_DebugKillThread(scriptInstance_t,uint,char const *)+178/w ...
// padding byte
// padding byte
// padding byte
    int step_mode;                      // XREF: Scr_DisplayDebuggerRemoteInternal+FB/w
    // Scr_Step+38/r ...
    Scr_OpcodeList_s *assignHead;       // XREF: Scr_UnbreakAllAssignmentPos+33/r
    // Scr_BreakOnAllAssignmentPos+52/r ...
    char *assignHeadCodePos;            // XREF: Scr_InitDebuggerMain(scriptInstance_t)+DA/w
    // Scr_AddAssignmentPos(scriptInstance_t,char *)+58/r ...
    bool assignBreakpointSet;           // XREF: Scr_UnbreakAllAssignmentPos+D/r
    // Scr_UnbreakAllAssignmentPos+23/w ...
    bool add;                           // XREF: Scr_RefToVariable(scriptInstance_t,uint,int)+113/r
    // Scr_RefToVariable(scriptInstance_t,uint,int)+1CE/r ...
// padding byte
// padding byte
    Scr_WatchElement_s *currentElement; // XREF: Scr_RefToVariable(scriptInstance_t,uint,int)+81/r
    // Scr_RefToVariable(scriptInstance_t,uint,int)+1B9/r ...
    unsigned int removeId;              // XREF: Scr_RefToVariable(scriptInstance_t,uint,int)+53/r
    // Scr_RefToVariable(scriptInstance_t,uint,int)+65/r ...
    Scr_WatchElementDoubleNode_t **variableBreakpoints;
    // XREF: Scr_RefToVariable(scriptInstance_t,uint,int)+B9/r
    // Scr_RefToVariable(scriptInstance_t,uint,int)+F1/r ...
    bool debugger_inited_main;          // XREF: Scr_InitDebuggerMain(scriptInstance_t)+22/r
    // Scr_InitDebuggerMain(scriptInstance_t)+115/w ...
    bool debugger_inited;               // XREF: Scr_InitDebugger(scriptInstance_t)+3B/r
    // Scr_InitDebugger(scriptInstance_t)+104/w ...
    bool debugger_inited_system;        // XREF: Scr_SortElementChildren+F/r
    // Scr_InitDebuggerSystem(scriptInstance_t)+22/r ...
// padding byte
    unsigned int objectId;              // XREF: Scr_BreakOnAllAssignmentPos+43/w
    // Scr_HitAssignmentBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+E0/r ...
    char *colBuf;                       // XREF: Scr_ScriptList::Init(scriptInstance_t)+297/w
    // Scr_ScriptList::Shutdown(scriptInstance_t)+12/r ...
    int prevBreakpointLineNum;          // XREF: Scr_SetTempBreakpoint+8C/w
    // Scr_HitBreakpoint(scriptInstance_t,VariableValue *,char const *,uint,int)+460/r ...
    bool disableBreakpoints;            // XREF: Scr_AllowBreakpoint+C/r
    // Scr_InitDebuggerMain(scriptInstance_t)+F3/w ...
    bool showConsole;
    // padding byte
    // padding byte
    volatile int disableDebuggerRemote; // XREF: Scr_DisplayDebuggerRemote+1F1/w
    // Scr_DisplayDebuggerRemote+204/r ...
    int breakpointCount;                // XREF: Scr_AddBreakpoint+8C/r
    // Scr_AddBreakpoint+9E/w ...
};

#pragma region SCRIPT_WATCH

#pragma endregion

void __cdecl Scr_AddManualBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos);
void __cdecl Scr_AddBreakpoint(scriptInstance_t inst, const unsigned __int8 *codePos);
char *__cdecl Scr_FindBreakpointInfo(scriptInstance_t inst, const char *codePos);
Scr_Breakpoint *__cdecl Scr_AllocBreakpoint();
void __cdecl Scr_FreeBreakpoint(Scr_Breakpoint *breakpoint);
Scr_WatchElement_s *__cdecl Scr_ReadElement(scriptInstance_t inst);
Scr_WatchElement_s *__cdecl Scr_GetElementRoot(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementChildrenStrict(scriptInstance_t inst, Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementChildren(scriptInstance_t inst, Scr_WatchElement_s *element);
void __cdecl Scr_RemoveValue(scriptInstance_t inst, Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementText(scriptInstance_t inst, Scr_WatchElement_s *element);
VariableValue __cdecl Scr_GetArrayIndexValue(scriptInstance_t inst, unsigned int name);
bool __cdecl Scr_IsSortWatchElement(Scr_WatchElement_s *element);
int __cdecl CompareArrayIndices(unsigned int *arg1, unsigned int *arg2);
int __cdecl CompareArrayIndicesClient(unsigned int *arg1, unsigned int *arg2);
void __cdecl Scr_DeltaElementValueText(Scr_WatchElement_s *element, const char *oldValueText);
void __cdecl Scr_SetNonFieldElementRefText(scriptInstance_t inst, Scr_WatchElement_s *element);
void __cdecl Scr_PostSetText(scriptInstance_t inst, Scr_WatchElement_s *element);
char *__cdecl Scr_GetElementThreadPos(scriptInstance_t inst, Scr_WatchElement_s *element);
void __cdecl Scr_SetElementRefText(scriptInstance_t inst, Scr_WatchElement_s *element, char *fieldText);
void __cdecl Scr_DeltaElementRefText(Scr_WatchElement_s *element, const char *oldRefText, char *fieldText);
void __cdecl Scr_ConnectElementChildren(Scr_WatchElement_s *parentElement);
void __cdecl Scr_SortElementChildren(scriptInstance_t inst, Scr_WatchElement_s *parentElement);
int __cdecl CompareThreadElements(int *arg1, int *arg2);
Scr_WatchElement_s *__cdecl Scr_CreateWatchElement(
    scriptInstance_t inst,
    char *text,
    Scr_WatchElement_s **prevElem,
    const char *name);
void __cdecl Scr_Evaluate(scriptInstance_t inst);
void __cdecl Scr_CheckBreakonNotify(
    scriptInstance_t inst,
    unsigned int notifyListOwnerId,
    unsigned int stringValue,
    VariableValue *top,
    const char *pos,
    unsigned int localId);
void __cdecl Scr_SpecialBreakpoint(
    scriptInstance_t inst,
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int opcode,
    int type);
bool __cdecl Scr_AllowBreakpoint(scriptInstance_t inst);
void __cdecl Scr_UnbreakAllAssignmentPos(scriptInstance_t inst);
void __cdecl Scr_RemoveAssignmentBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos);
bool __cdecl Scr_RefToVariable(scriptInstance_t inst, unsigned int id, int isObject);
void __cdecl Scr_BreakOnAllAssignmentPos(scriptInstance_t inst);
void __cdecl Scr_AddAssignmentBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos);

void __cdecl Scr_WriteElement(Scr_WatchElement_s *element);
void __cdecl Scr_FreeLineBreakpoint(scriptInstance_t inst, Scr_Breakpoint *breakpoint, bool deleteElement);
void __cdecl Scr_RemoveManualBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos);
void __cdecl Scr_RemoveBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos);
void __cdecl Scr_MonitorCommand(const char *text, scriptInstance_t inst);
void __cdecl Scr_ResumeBreakpoints(scriptInstance_t inst);
void __cdecl Scr_SetTempBreakpoint(scriptInstance_t inst, char *codePos, unsigned int threadId);
void __cdecl Scr_FreeDebugMem(scriptInstance_t inst, char *ptr);
unsigned int *__cdecl Scr_AllocDebugMem(scriptInstance_t inst, int size, const char *name);

void __cdecl Scr_InitDebuggerMain(scriptInstance_t inst);
void __cdecl Scr_ShutdownDebuggerMain(scriptInstance_t inst);
void __cdecl Scr_InitDebugger(scriptInstance_t inst);
void __cdecl Scr_ShutdownDebugger(scriptInstance_t inst);
void __cdecl Scr_InitDebuggerSystem(scriptInstance_t inst);
void Scr_InitBreakpoints();
void __cdecl Scr_ShutdownDebuggerSystem(scriptInstance_t inst);
void __cdecl Scr_ConnectRemote(scriptInstance_t inst);
void __cdecl Scr_AddAssignmentPos(scriptInstance_t inst, char *codePos);
void __cdecl Scr_ResetAbortDebugger(scriptInstance_t inst);
void __cdecl Scr_RunDebugger(scriptInstance_t inst);
void __cdecl Scr_DisplayDebugger(scriptInstance_t inst);
void __cdecl Scr_DisplayDebuggerRemote(scriptInstance_t inst);
void __cdecl Scr_DisplayDebuggerRemoteInternal(scriptInstance_t inst);
void __cdecl Scr_Step(scriptInstance_t inst);
void __cdecl Scr_WatchElementHitBreakpoint(Scr_WatchElement_s *element, bool enabled);
void __cdecl Scr_HitBreakpointInternal(scriptInstance_t inst);
void __cdecl Scr_DisplayHitBreakpointRemote(scriptInstance_t inst);
int __cdecl Scr_HitBreakpoint(
                scriptInstance_t inst,
                VariableValue *top,
                const char *pos,
                unsigned int localId,
                int hitBreakpoint);
bool __cdecl Scr_ConditionalExpression(scriptInstance_t inst, Scr_WatchElement_s *element, unsigned int localId);
void __cdecl Scr_HitBuiltinBreakpoint(
                scriptInstance_t inst,
                VariableValue *top,
                const char *pos,
                unsigned int localId,
                int opcode,
                int builtinIndex,
                unsigned int outparamcount);
void __cdecl Scr_DebugKillThread(scriptInstance_t inst, unsigned int threadId, const char *codePos);
void __cdecl Scr_DebugTerminateThread(scriptInstance_t inst, int topThread);
void __cdecl Scr_ProcessDebugMessages(scriptInstance_t inst);
void __cdecl Scr_ToggleBreakpointRemote(scriptInstance_t inst);
void __cdecl Scr_StepRemote(scriptInstance_t inst);
void __cdecl Scr_RunToCursorRemote(scriptInstance_t inst);
void __cdecl Scr_EnterCallRemote(scriptInstance_t inst);
void __cdecl Scr_ExpandElementRemote(scriptInstance_t inst);
void __cdecl Scr_PasteElementRemote(scriptInstance_t inst);
void __cdecl Scr_InsertElementRemote(scriptInstance_t inst);
void __cdecl Scr_DeleteElementRemote(scriptInstance_t inst);
void __cdecl Scr_BackspaceElementRemote(scriptInstance_t inst);
void __cdecl Scr_AddTextRemote(scriptInstance_t inst);
void __cdecl Scr_CloneElementRemote(scriptInstance_t inst);
void __cdecl Scr_DisplayThreadPosRemote(scriptInstance_t inst);
void __cdecl Scr_ToggleWatchElementBreakpointRemote(scriptInstance_t inst);
void __cdecl Scr_KeepAliveRemote(scriptInstance_t inst);
int __cdecl Scr_UpdateDebugSocket(scriptInstance_t inst);
void __cdecl Sys_ConsolePrintRemote(scriptInstance_t inst, int localClientNum);
void __cdecl Scr_UpdateRemoteDebugger(scriptInstance_t inst);
void __cdecl Scr_UpdateDebugger(scriptInstance_t inst);
char __cdecl Scr_WatchElementHasSameValue(scriptInstance_t inst, Scr_WatchElement_s *element, VariableValue *newValue);
int __cdecl Scr_HitAssignmentBreakpoint(
                scriptInstance_t inst,
                VariableValue *top,
                const char *pos,
                unsigned int localId,
                int forceBreak);
bool __cdecl Scr_IgnoreErrors(scriptInstance_t inst);
int __cdecl CompareThreadIndices(unsigned int *arg1, unsigned int *arg2);


extern scrDebuggerGlob_t gScrDebuggerGlob[2];
extern scriptInstance_t gDebuggerInstance;

extern thread_local int g_breakonExpr;
extern thread_local unsigned int g_breakonString;
extern thread_local int g_breakonHit;
extern thread_local unsigned int g_breakonObject;