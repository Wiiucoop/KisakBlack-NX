#pragma once
#include "cscr_main.h"
#include "cscr_debugger.h"
#include "cscr_variable.h"

struct VariableCompileValue // sizeof=0xC
{                                       // XREF: scrCompileGlob_t/r
    VariableValue value;                // XREF: Scr_CalcLocalVarsWhileStatement+25/r
    sval_u sourcePos;
};

struct scrCompilePub_t // sizeof=0x21038
{                                       // XREF: .data:_gScrCompilePub/r
    int value_count;                    // XREF: EmitOpcode+54/r
                                        // EmitOpcode+66/r ...
    int far_function_count;             // XREF: ScriptCompile(scriptInstance_t,sval_u,uint,uint,uint,PrecacheEntry *,int)+6C/r
                                        // ScriptCompile(scriptInstance_t,sval_u,uint,uint,uint,PrecacheEntry *,int)+A5/r ...
    unsigned int loadedscripts;         // XREF: EmitFunction+168/r
                                        // Scr_ScriptList::Init(scriptInstance_t)+49/r ...
    unsigned int scriptsPos;            // XREF: AddFilePrecache+D3/r
                                        // Scr_GetFunctionHandle(scriptInstance_t,char const *,char const *)+F/r ...
    unsigned int scriptsCount;          // XREF: AddFilePrecache+105/r
                                        // Scr_BeginLoadScripts(scriptInstance_t,int)+21D/r ...
    unsigned int builtinFunc;           // XREF: EmitCall+69/r
                                        // EmitCall+E9/r ...
    unsigned int builtinMeth;           // XREF: EmitMethod+69/r
                                        // EmitMethod+E9/r ...
    unsigned __int16 canonicalStrings[65536];
                                        // XREF: Scr_ArchiveCanonicalStrings(scriptInstance_t)+35/r
                                        // Scr_ArchiveCanonicalStrings(scriptInstance_t)+155/r ...
    const char *in_ptr;                 // XREF: Scr_CompileTextInternal+A2/w
                                        // Scr_ScanFile(scriptInstance_t,char *,int)+2D/r ...
    const char *parseBuf;               // XREF: Scr_CompileTextInternal+B8/w
                                        // Scr_ScanFile(scriptInstance_t,char *,int)+A7/r ...
    bool script_loading;                // XREF: EmitVariableExpression+D0/r
                                        // EmitObject+F/r ...
    bool allowedBreakpoint;             // XREF: EmitOpcode+FB/w
                                        // AddOpcodePos(scriptInstance_t,uint,int)+8A/r
    // padding byte
    // padding byte
    int developer_statement;            // XREF: Scr_GetBuiltin(scriptInstance_t,sval_u)+4B/r
                                        // ScriptCompile(scriptInstance_t,sval_u,uint,uint,uint,PrecacheEntry *,int)+59/w ...
    unsigned __int8 *opcodePos;         // XREF: EmitPreAssignmentPos+33/r
                                        // EmitOpcode+1E3/r ...
    unsigned int programLen;            // XREF: ScriptCompile(scriptInstance_t,sval_u,uint,uint,uint,PrecacheEntry *,int)+11C/w
                                        // Scr_FindBreakpointInfo+4E/r ...
    int func_table_size;                // XREF: AddFunction+22/r
                                        // AddFunction+58/r ...
    uintptr_t func_table[1024];  // nx-port: function pointers, was int             // XREF: AddFunction+36/r
                                        // AddFunction+C5/w ...
};

struct CaseStatementInfo // sizeof=0x10
{
    unsigned int name;
    const char *codePos;
    unsigned int sourcePos;
    CaseStatementInfo *next;
};

struct BreakStatementInfo // sizeof=0xC
{
    const char *codePos;
    const char *nextCodePos;
    BreakStatementInfo *next;
};

struct ContinueStatementInfo // sizeof=0xC
{
    const char *codePos;
    const char *nextCodePos;
    ContinueStatementInfo *next;
};

struct scrCompileGlob_t
{
    unsigned __int8 *codePos;
    unsigned __int8 *prevOpcodePos;
    unsigned int filePosId;
    unsigned int fileCountId;
    int cumulOffset;
    int maxOffset;
    int maxCallOffset;
    bool bConstRefCount;
    bool in_developer_thread;
    unsigned int developer_thread_sourcePos;
    bool firstThread[2];
    CaseStatementInfo *currentCaseStatement;
    bool bCanBreak;
    BreakStatementInfo *currentBreakStatement;
    bool bCanContinue;
    ContinueStatementInfo *currentContinueStatement;
    scr_block_s **breakChildBlocks;
    int *breakChildCount;
    scr_block_s *breakBlock;
    scr_block_s **continueChildBlocks;
    int *continueChildCount;
    bool forceNotCreate;
    PrecacheEntry *precachescriptList;
    VariableCompileValue value_start[32];
};

enum $BB7F4801BC8A3FB836B39ECFF8B5BEFB : __int32
{
    FUNC_SCOPE_LOCAL = 0x0,
    FUNC_SCOPE_FAR   = 0x1,
};

//enum $4595A41124E79A03522668AB0CFC531F : __int32
enum ParseToken_t : __int32 // LWSS: not a real enum name!
{
    ENUM_NOP                       = 0x0,
    ENUM_program                   = 0x1,
    ENUM_assignment                = 0x2,
    ENUM_unknown_variable          = 0x3,
    ENUM_duplicate_variable        = 0x4,
    ENUM_local_variable            = 0x5,
    ENUM_local_variable_frozen     = 0x6,
    ENUM_duplicate_expression      = 0x7,
    ENUM_primitive_expression      = 0x8,
    ENUM_integer                   = 0x9,
    ENUM_float                     = 0xA,
    ENUM_minus_integer             = 0xB,
    ENUM_minus_float               = 0xC,
    ENUM_string                    = 0xD,
    ENUM_istring                   = 0xE,
    ENUM_array_variable            = 0xF,
    ENUM_unknown_field             = 0x10,
    ENUM_field_variable            = 0x11,
    ENUM_field_variable_frozen     = 0x12,
    ENUM_variable                  = 0x13,
    ENUM_function                  = 0x14,
    ENUM_call_expression           = 0x15,
    ENUM_local_function            = 0x16,
    ENUM_far_function              = 0x17,
    ENUM_function_pointer          = 0x18,
    ENUM_call                      = 0x19,
    ENUM_method                    = 0x1A,
    ENUM_call_expression_statement = 0x1B,
    ENUM_script_call               = 0x1C,
    ENUM_return                    = 0x1D,
    ENUM_return2                   = 0x1E,
    ENUM_wait                      = 0x1F,
    ENUM_script_thread_call        = 0x20,
    ENUM_undefined                 = 0x21,
    ENUM_self                      = 0x22,
    ENUM_self_frozen               = 0x23,
    ENUM_level                     = 0x24,
    ENUM_game                      = 0x25,
    ENUM_anim                      = 0x26,
    ENUM_if                        = 0x27,
    ENUM_if_else                   = 0x28,
    ENUM_while                     = 0x29,
    ENUM_for                       = 0x2A,
    ENUM_inc                       = 0x2B,
    ENUM_dec                       = 0x2C,
    ENUM_binary_equals             = 0x2D,
    ENUM_statement_list            = 0x2E,
    ENUM_developer_statement_list  = 0x2F,
    ENUM_expression_list           = 0x30,
    ENUM_bool_or                   = 0x31,
    ENUM_bool_and                  = 0x32,
    ENUM_binary                    = 0x33,
    ENUM_bool_not                  = 0x34,
    ENUM_bool_complement           = 0x35,
    ENUM_size_field                = 0x36,
    ENUM_self_field                = 0x37,
    ENUM_precachetree              = 0x38,
    ENUM_waittill                  = 0x39,
    ENUM_waittillmatch             = 0x3A,
    ENUM_waittillFrameEnd          = 0x3B,
    ENUM_notify                    = 0x3C,
    ENUM_endon                     = 0x3D,
    ENUM_switch                    = 0x3E,
    ENUM_case                      = 0x3F,
    ENUM_default                   = 0x40,
    ENUM_break                     = 0x41,
    ENUM_continue                  = 0x42,
    ENUM_expression                = 0x43,
    ENUM_empty_array               = 0x44,
    ENUM_animation                 = 0x45,
    ENUM_thread                    = 0x46,
    ENUM_begin_developer_thread    = 0x47,
    ENUM_end_developer_thread      = 0x48,
    ENUM_usingtree                 = 0x49,
    ENUM_false                     = 0x4A,
    ENUM_true                      = 0x4B,
    ENUM_animtree                  = 0x4C,
    ENUM_breakon                   = 0x4D,
    ENUM_breakpoint                = 0x4E,
    ENUM_prof_begin                = 0x4F,
    ENUM_prof_end                  = 0x50,
    ENUM_vector                    = 0x51,
    ENUM_object                    = 0x52,
    ENUM_thread_object             = 0x53,
    ENUM_local                     = 0x54,
    ENUM_statement                 = 0x55,
    ENUM_bad_expression            = 0x56,
    ENUM_bad_statement             = 0x57,
    ENUM_include                   = 0x58,
    ENUM_argument                  = 0x59,
};


void (__cdecl *__cdecl GetFunction(scriptInstance_t inst, const char **pName, int *type))();
void (__cdecl *__cdecl GetMethod(scriptInstance_t inst, const char **pName, int *type))(scr_entref_t);
int __cdecl GetExpressionCount(sval_u exprlist);
unsigned int __cdecl Scr_GetBuiltin(scriptInstance_t inst, sval_u func_name);
int __cdecl CompareCaseInfo(unsigned int *elem1, unsigned int *elem2);
void __cdecl ScriptCompile(
                scriptInstance_t inst,
                sval_u val,
                unsigned int filePosId,
                unsigned int fileCountId,
                unsigned int scriptId,
                PrecacheEntry *entries,
                int entriesCount);
void __cdecl LinkThread(scriptInstance_t inst, unsigned int threadCountId, VariableValue *pos, bool allowFarCall);
void __cdecl LinkFile(scriptInstance_t inst, unsigned int filePosId, unsigned int fileCountId);
void __cdecl CheckThreadPosition(scriptInstance_t inst, unsigned int posId, unsigned int name, unsigned int sourcePos);
void __cdecl EmitThreadList(scriptInstance_t inst, sval_u val);
void __cdecl SpecifyThread(scriptInstance_t inst, sval_u val);
unsigned int __cdecl SpecifyThreadPosition(
                scriptInstance_t inst,
                unsigned int posId,
                unsigned int name,
                unsigned int sourcePos,
                int type);
void __cdecl EmitThread(scriptInstance_t inst, sval_u val);
void __cdecl Scr_CompileRemoveRefToString(scriptInstance_t inst, unsigned int stringValue);
void __cdecl Scr_CalcLocalVarsThread(scriptInstance_t inst, sval_u exprlist, sval_u stmtlist, sval_u *stmttblock);
void __cdecl Scr_CalcLocalVarsStatementList(scriptInstance_t inst, sval_u val, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsStatement(scriptInstance_t inst, sval_u val, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsIncStatement(sval_u expr, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsVariableExpressionRef(sval_u expr, scr_block_s *block);
void __cdecl Scr_RegisterLocalVar(unsigned int name, sval_u sourcePos, scr_block_s *block);
void __cdecl Scr_CheckLocalVarsCount(int localVarsCount);
void __cdecl Scr_CalcLocalVarsArrayVariableRef(sval_u expr, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsArrayPrimitiveExpressionRef(sval_u expr, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsAssignmentStatement(sval_u lhs, sval_u rhs, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsIfStatement(scriptInstance_t inst, sval_u stmt, scr_block_s *block, sval_u *ifStatBlock);
void __cdecl Scr_CopyBlock(scr_block_s *from, scr_block_s **to);
void __cdecl Scr_MergeChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block);
int __cdecl Scr_FindLocalVar(scr_block_s *block, int startIndex, unsigned int name);
void __cdecl Scr_CalcLocalVarsIfElseStatement(
                scriptInstance_t inst,
                sval_u stmt1,
                sval_u stmt2,
                scr_block_s *block,
                sval_u *ifStatBlock,
                sval_u *elseStatBlock);
void __cdecl Scr_AppendChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block);
void __cdecl Scr_AddBreakBlock(scriptInstance_t inst, scr_block_s *block);
void __cdecl Scr_CheckMaxSwitchCases(int count);
void __cdecl Scr_AddContinueBlock(scriptInstance_t inst, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsWhileStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u stmt,
                scr_block_s *block,
                sval_u *whileStatBlock);
char __cdecl EvalExpression(scriptInstance_t inst, sval_u expr, VariableCompileValue *constValue);
char __cdecl EvalPrimitiveExpression(scriptInstance_t inst, sval_u expr, VariableCompileValue *constValue);
void __cdecl EvalUndefined(sval_u sourcePos, VariableCompileValue *constValue);
void __cdecl EvalInteger(int value, sval_u sourcePos, VariableCompileValue *constValue);
void __cdecl EvalFloat(float value, sval_u sourcePos, VariableCompileValue *constValue);
void __cdecl EvalString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue);
void __cdecl EvalIString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue);
char __cdecl EvalPrimitiveExpressionList(
                scriptInstance_t inst,
                sval_u exprlist,
                sval_u sourcePos,
                VariableCompileValue *constValue);
void __cdecl Scr_CreateVector(scriptInstance_t inst, VariableCompileValue *constValue, VariableValue *value);
char __cdecl EvalBinaryOperatorExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u opcode,
                sval_u sourcePos,
                VariableCompileValue *constValue);
void __cdecl Scr_CalcLocalVarsForStatement(
                scriptInstance_t inst,
                sval_u stmt1,
                sval_u expr,
                sval_u stmt2,
                sval_u stmt,
                scr_block_s *block,
                sval_u *forStatBlock,
                sval_u *forStatPostBlock);
void __cdecl Scr_CalcLocalVarsWaittillStatement(sval_u exprlist, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsFormalParameterListInternal(sval_u *node, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsSafeSetVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsSwitchStatement(scriptInstance_t inst, sval_u stmtlist, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsDeveloperStatementList(
                scriptInstance_t inst,
                sval_u val,
                scr_block_s *block,
                sval_u *devStatBlock);
void __cdecl Scr_CalcLocalVarsFormalParameterList(sval_u exprlist, scr_block_s *block);
void __cdecl EmitNormalThread(scriptInstance_t inst, sval_u val, sval_u *stmttblock);
void __cdecl SetThreadPosition(scriptInstance_t inst, unsigned int posId);
void __cdecl EmitThreadInternal(
                scriptInstance_t inst,
                sval_u val,
                sval_u sourcePos,
                sval_u endSourcePos,
                scr_block_s *block);
void __cdecl CompileTransferRefToString(scriptInstance_t inst, unsigned int stringValue, unsigned int user);
void __cdecl EmitEnd(scriptInstance_t inst);
void __cdecl EmitPreAssignmentPos(scriptInstance_t inst);
void __cdecl EmitOpcode(scriptInstance_t inst, unsigned int op, int offset, int callType);
void __cdecl EmitByte(scriptInstance_t inst, unsigned __int8 value);
void __cdecl EmitValue(scriptInstance_t inst, VariableCompileValue *constValue);
void __cdecl EmitGetUndefined(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitGetInteger(scriptInstance_t inst, int value, sval_u sourcePos);
void __cdecl EmitCodepos(scriptInstance_t inst, const char *pos);
void __cdecl EmitCodeInt(scriptInstance_t inst, int value);   // nx-port: see cscr_codepos.h
void __cdecl EmitGetFloat(scriptInstance_t inst, float value, sval_u sourcePos);
void __cdecl EmitFloat(scriptInstance_t inst, float value);
void __cdecl EmitGetString(scriptInstance_t inst, unsigned int value, sval_u sourcePos);
void __cdecl EmitShort(scriptInstance_t inst, __int16 value);
void __cdecl EmitGetIString(scriptInstance_t inst, unsigned int value, sval_u sourcePos);
void __cdecl EmitGetVector(scriptInstance_t inst, const float *value, sval_u sourcePos);
void __cdecl EmitStatementList(
                scriptInstance_t inst,
                sval_u val,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block);
char __cdecl Scr_IsLastStatement(scriptInstance_t inst, sval_u *node);
void __cdecl EmitStatement(
                scriptInstance_t inst,
                sval_u val,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block);
void __cdecl EmitBinaryEqualsOperatorExpression(
                scriptInstance_t inst,
                sval_u lhs,
                sval_u rhs,
                sval_u opcode,
                sval_u sourcePos,
                scr_block_s *block);
void __cdecl EmitSetVariableField(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitAssignmentPos(scriptInstance_t inst);
void __cdecl EmitVariableExpression(scriptInstance_t inst, sval_u expr, scr_block_s *block);
void __cdecl EmitLocalVariable(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitCanonicalString(scriptInstance_t inst, unsigned int stringValue);
int __cdecl Scr_FindLocalVarIndex(
                scriptInstance_t inst,
                unsigned int name,
                sval_u sourcePos,
                bool create,
                scr_block_s *block);
void __cdecl EmitCanonicalStringConst(scriptInstance_t inst, unsigned int stringValue);
void __cdecl EmitFieldVariable(scriptInstance_t inst, sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitObject(scriptInstance_t inst, sval_u expr, sval_u sourcePos);
void __cdecl EmitArrayVariable(
                scriptInstance_t inst,
                sval_u expr,
                sval_u index,
                sval_u sourcePos,
                sval_u indexSourcePos,
                scr_block_s *block);
void __cdecl EmitEvalArray(scriptInstance_t inst, sval_u sourcePos, sval_u indexSourcePos);
void __cdecl EmitPrimitiveExpression(scriptInstance_t inst, sval_u expr, scr_block_s *block);
char __cdecl EmitOrEvalPrimitiveExpression(
                scriptInstance_t inst,
                sval_u expr,
                VariableCompileValue *constValue,
                scr_block_s *block);
void __cdecl EmitAnimTree(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitSize(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitSelf(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitLevel(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitGame(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitAnim(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitEmptyArray(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitAnimation(scriptInstance_t inst, sval_u anim, sval_u sourcePos);
void __cdecl EmitGetFunction(scriptInstance_t inst, sval_u func, sval_u sourcePos);
void __cdecl EmitFunction(scriptInstance_t inst, sval_u func, sval_u sourcePos);
void __cdecl AddFilePrecache(
                scriptInstance_t inst,
                unsigned int filename,
                unsigned int sourcePos,
                bool include,
                unsigned int *filePosId,
                unsigned int *fileCountId);
void __cdecl EmitCallExpression(scriptInstance_t inst, sval_u expr, bool bStatement, scr_block_s *block);
void __cdecl EmitCall(scriptInstance_t inst, sval_u func_name, sval_u params, bool bStatement, scr_block_s *block);
void __cdecl EmitDecTop(scriptInstance_t inst);
int __cdecl EmitExpressionList(scriptInstance_t inst, sval_u exprlist, scr_block_s *block);
void __cdecl AddExpressionListOpcodePos(scriptInstance_t inst, sval_u exprlist);
int __cdecl AddFunction(scriptInstance_t inst, uintptr_t func);
void __cdecl EmitPreFunctionCall(scriptInstance_t inst, sval_u func_name);
void __cdecl EmitPostFunctionCall(
                scriptInstance_t inst,
                sval_u func_name,
                int param_count,
                bool bMethod,
                scr_block_s *block);
void __cdecl EmitPostScriptFunctionCall(
                scriptInstance_t inst,
                sval_u func_name,
                int param_count,
                bool bMethod,
                sval_u nameSourcePos,
                scr_block_s *block);
void __cdecl EmitPostScriptFunction(
                scriptInstance_t inst,
                sval_u func,
                int param_count,
                bool bMethod,
                sval_u nameSourcePos);
void __cdecl EmitPostScriptFunctionPointer(
                scriptInstance_t inst,
                sval_u expr,
                int param_count,
                bool bMethod,
                sval_u nameSourcePos,
                sval_u sourcePos,
                scr_block_s *block);
void __cdecl EmitPostScriptThreadCall(
                scriptInstance_t inst,
                sval_u func_name,
                int param_count,
                bool bMethod,
                sval_u sourcePos,
                sval_u nameSourcePos,
                scr_block_s *block);
void __cdecl EmitPostScriptThread(scriptInstance_t inst, sval_u func, int param_count, bool bMethod, sval_u sourcePos);
void __cdecl EmitPostScriptThreadPointer(
                scriptInstance_t inst,
                sval_u expr,
                int param_count,
                bool bMethod,
                sval_u sourcePos,
                scr_block_s *block);
void __cdecl Scr_BeginDevScript(scriptInstance_t inst, int *type, char **savedPos);
void __cdecl Scr_EndDevScript(scriptInstance_t inst, int type, char **savedPos);
int __cdecl Scr_GetCacheType(int type);
int __cdecl Scr_GetUncacheType(int type);
void __cdecl EmitCallBuiltinOpcode(scriptInstance_t inst, int param_count, sval_u sourcePos);
void __cdecl EmitMethod(
                scriptInstance_t inst,
                sval_u expr,
                sval_u func_name,
                sval_u params,
                sval_u methodSourcePos,
                bool bStatement,
                scr_block_s *block);
void __cdecl EmitCallBuiltinMethodOpcode(scriptInstance_t inst, int param_count, sval_u sourcePos);
void __cdecl EmitBreakOn(scriptInstance_t inst, sval_u expr, sval_u param, sval_u sourcePos);
bool __cdecl EmitOrEvalPrimitiveExpressionList(
                scriptInstance_t inst,
                sval_u exprlist,
                sval_u sourcePos,
                VariableCompileValue *constValue,
                scr_block_s *block);
void __cdecl Scr_PushValue(scriptInstance_t inst, VariableCompileValue *constValue);
char __cdecl EmitOrEvalExpression(
                scriptInstance_t inst,
                sval_u expr,
                VariableCompileValue *constValue,
                scr_block_s *block);
void __cdecl EmitBoolNot(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitBoolComplement(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitBoolOrExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u expr1sourcePos,
                sval_u expr2sourcePos,
                scr_block_s *block);
void __cdecl EmitCastBool(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitBoolAndExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u expr1sourcePos,
                sval_u expr2sourcePos,
                scr_block_s *block);
char __cdecl EmitOrEvalBinaryOperatorExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u opcode,
                sval_u sourcePos,
                VariableCompileValue *constValue,
                scr_block_s *block);
void __cdecl Scr_PopValue(scriptInstance_t inst);
void __cdecl EmitExpression(scriptInstance_t inst, sval_u expr, scr_block_s *block);
void __cdecl EmitVariableExpressionRef(scriptInstance_t inst, sval_u expr, scr_block_s *block);
void __cdecl EmitLocalVariableRef(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitFieldVariableRef(
                scriptInstance_t inst,
                sval_u expr,
                sval_u field,
                sval_u sourcePos,
                scr_block_s *block);
void __cdecl EmitArrayVariableRef(
                scriptInstance_t inst,
                sval_u expr,
                sval_u index,
                sval_u sourcePos,
                sval_u indexSourcePos,
                scr_block_s *block);
void __cdecl EmitEvalArrayRef(scriptInstance_t inst, sval_u sourcePos, sval_u indexSourcePos);
void __cdecl EmitArrayPrimitiveExpressionRef(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitGameRef(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitPrimitiveExpressionFieldObject(
                scriptInstance_t inst,
                sval_u expr,
                sval_u sourcePos,
                scr_block_s *block);
void __cdecl EmitSelfObject(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitLevelObject(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitAnimObject(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitCastFieldObject(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitCallExpressionFieldObject(scriptInstance_t inst, sval_u expr, scr_block_s *block);
void __cdecl EmitExpressionListFieldObject(
                scriptInstance_t inst,
                sval_u exprlist,
                sval_u sourcePos,
                scr_block_s *block);
sval_u *__cdecl GetSingleParameter(sval_u exprlist);
void __cdecl EmitExpressionFieldObject(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitAssignmentStatement(
                scriptInstance_t inst,
                sval_u lhs,
                sval_u rhs,
                sval_u sourcePos,
                sval_u rhsSourcePos,
                scr_block_s *block);
bool __cdecl IsUndefinedExpression(sval_u expr);
bool __cdecl IsUndefinedPrimitiveExpression(sval_u expr);
char __cdecl EmitClearVariableExpression(scriptInstance_t inst, sval_u expr, sval_u rhsSourcePos, scr_block_s *block);
void __cdecl EmitClearFieldVariable(
                scriptInstance_t inst,
                sval_u expr,
                sval_u field,
                sval_u sourcePos,
                sval_u rhsSourcePos,
                scr_block_s *block);
void __cdecl EmitClearArrayVariable(
                scriptInstance_t inst,
                sval_u expr,
                sval_u index,
                sval_u sourcePos,
                sval_u indexSourcePos,
                scr_block_s *block);
void __cdecl EmitClearArray(scriptInstance_t inst, sval_u sourcePos, sval_u indexSourcePos);
void __cdecl EmitCallExpressionStatement(scriptInstance_t inst, sval_u expr, scr_block_s *block);
void __cdecl EmitReturnStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitReturn(scriptInstance_t inst);
void __cdecl EmitEndStatement(scriptInstance_t inst, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitWaitStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u sourcePos,
                sval_u waitSourcePos,
                scr_block_s *block);
void __cdecl EmitWaittillFrameEnd(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitIfStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u stmt,
                sval_u sourcePos,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block,
                sval_u *ifStatBlock);
void __cdecl EmitNOP2(scriptInstance_t inst, bool lastStatement, unsigned int endSourcePos, scr_block_s *block);
void __cdecl EmitRemoveLocalVars(scriptInstance_t inst, scr_block_s *block, scr_block_s *outerBlock);
void __cdecl Scr_TransferBlock(scr_block_s *from, scr_block_s *to);
void __cdecl EmitIfElseStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u stmt1,
                sval_u stmt2,
                sval_u sourcePos,
                sval_u elseSourcePos,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block,
                sval_u *ifStatBlock,
                sval_u *elseStatBlock);
void __cdecl Scr_InitFromChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block);
void __cdecl EmitWhileStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u stmt,
                sval_u sourcePos,
                sval_u whileSourcePos,
                scr_block_s *block,
                sval_u *whileStatBlock);
void __cdecl EmitCreateLocalVars(scriptInstance_t inst, scr_block_s *block);
void __cdecl ConnectBreakStatements(scriptInstance_t inst);
void __cdecl ConnectContinueStatements(scriptInstance_t inst);
void __cdecl EmitForStatement(
                scriptInstance_t inst,
                sval_u stmt1,
                sval_u expr,
                sval_u stmt2,
                sval_u stmt,
                sval_u sourcePos,
                sval_u forSourcePos,
                scr_block_s *block,
                sval_u *forStatBlock,
                sval_u *forStatPostBlock);
void __cdecl EmitIncStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitDecStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitWaittillStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u exprlist,
                sval_u sourcePos,
                sval_u waitSourcePos,
                scr_block_s *block);
void __cdecl EmitFormalWaittillParameterListRefInternal(scriptInstance_t inst, sval_u *node, scr_block_s *block);
void __cdecl EmitSafeSetWaittillVariableField(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitWaittillmatchStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u exprlist,
                sval_u sourcePos,
                sval_u waitSourcePos,
                scr_block_s *block);
void __cdecl EmitNotifyStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u exprlist,
                sval_u sourcePos,
                sval_u notifySourcePos,
                scr_block_s *block);
void __cdecl EmitEndOnStatement(
                scriptInstance_t inst,
                sval_u obj,
                sval_u expr,
                sval_u sourcePos,
                sval_u exprSourcePos,
                scr_block_s *block);
void __cdecl EmitSwitchStatement(
                scriptInstance_t inst,
                sval_u expr,
                sval_u stmtlist,
                sval_u sourcePos,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block);
void __cdecl EmitSwitchStatementList(
                scriptInstance_t inst,
                sval_u val,
                bool lastStatement,
                unsigned int endSourcePos,
                scr_block_s *block);
void __cdecl EmitCaseStatement(scriptInstance_t inst, sval_u expr, sval_u sourcePos);
void __cdecl EmitDefaultStatement(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitCaseStatementInfo(scriptInstance_t inst, unsigned int name, sval_u sourcePos);
void __cdecl EmitBreakStatement(scriptInstance_t inst, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitContinueStatement(scriptInstance_t inst, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitBreakpointStatement(scriptInstance_t inst, sval_u sourcePos);
void __cdecl EmitProfBeginStatement(scriptInstance_t inst, sval_u profileName, sval_u sourcePos);
void __cdecl EmitProfStatement(scriptInstance_t inst, sval_u profileName, sval_u sourcePos, unsigned __int8 op);
void __cdecl EmitProfEndStatement(scriptInstance_t inst, sval_u profileName, sval_u sourcePos);
void __cdecl EmitDeveloperStatementList(
                scriptInstance_t inst,
                sval_u val,
                sval_u sourcePos,
                scr_block_s *block,
                sval_u *devStatBlock);
void __cdecl EmitFormalParameterList(scriptInstance_t inst, sval_u exprlist, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitFormalParameterListInternal(scriptInstance_t inst, sval_u *node, scr_block_s *block);
void __cdecl EmitSafeSetVariableField(scriptInstance_t inst, sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl InitThread(scriptInstance_t inst, int type);
void __cdecl EmitDeveloperThread(scriptInstance_t inst, sval_u val, sval_u *stmttblock);
void __cdecl EmitIncludeList(scriptInstance_t inst, sval_u val);
void __cdecl EmitInclude(scriptInstance_t inst, sval_u val);
void __cdecl Scr_CompileStatement(scriptInstance_t inst, sval_u parseData);


extern scrCompilePub_t gScrCompilePub[2];
