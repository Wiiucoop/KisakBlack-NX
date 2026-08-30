#pragma once
#include "cscr_main.h"
#include "cscr_variable.h"
#include "cscr_debugger.h"

struct ArchivedCanonicalStringInfo // sizeof=0x8
{
    unsigned __int16 canonicalStr;
    // padding byte
    // padding byte
    const char *value;
};

struct __declspec(align(2)) scrEvaluateGlob_t // sizeof=0x10
{                                       // XREF: .data:scrEvaluateGlob_t * gScrEvaluateGlob/r
    char *archivedCanonicalStringsBuf;  // XREF: Scr_ArchiveCanonicalStrings(scriptInstance_t)+AE/w
                                        // Scr_ArchiveCanonicalStrings(scriptInstance_t)+1BF/r ...
    ArchivedCanonicalStringInfo *archivedCanonicalStrings;
                                        // XREF: Scr_ArchiveCanonicalStrings(scriptInstance_t)+E1/w
                                        // Scr_ArchiveCanonicalStrings(scriptInstance_t)+20F/r ...
    int *canonicalStringLookup;         // XREF: Scr_CompareCanonicalStrings(void const *,void const *)+11/r
                                        // Scr_CompareCanonicalStrings(void const *,void const *)+25/r ...
    bool freezeScope;                   // XREF: Scr_EvalScriptExpression(scriptInstance_t,ScriptExpression_t *,uint,VariableValue *,bool,bool)+38/w
                                        // Scr_EvalExpression+163/r ...
    bool freezeObjects;                 // XREF: Scr_EvalScriptExpression(scriptInstance_t,ScriptExpression_t *,uint,VariableValue *,bool,bool)+47/w
                                        // Scr_EvalExpression+1DF/r ...
    bool objectChanged;                 // XREF: Scr_EvalScriptExpression(scriptInstance_t,ScriptExpression_t *,uint,VariableValue *,bool,bool)+53/w
                                        // Scr_EvalScriptExpression(scriptInstance_t,ScriptExpression_t *,uint,VariableValue *,bool,bool)+7A/r ...
    // padding byte
};

int __cdecl Scr_CompareCanonicalStrings(unsigned int *arg1, unsigned int *arg2);
void __cdecl Scr_ArchiveCanonicalStrings(scriptInstance_t inst);
int __cdecl CompareCanonicalStrings(const char **arg1, const char **arg2);
const char *__cdecl Scr_GetCanonicalString(scriptInstance_t inst, unsigned int fieldName);
void __cdecl Scr_InitEvaluate(scriptInstance_t inst);
void __cdecl Scr_EndLoadEvaluate(scriptInstance_t inst);
void __cdecl Scr_ShutdownEvaluate(scriptInstance_t inst);
unsigned __int16 __cdecl Scr_CompileCanonicalString(scriptInstance_t inst, unsigned int stringValue);
void __cdecl Scr_GetFieldValue(
                scriptInstance_t inst,
                unsigned int objectId,
                const char *fieldName,
                int len,
                char *text);
void __cdecl Scr_GetValueString(scriptInstance_t inst, unsigned int localId, VariableValue *value, int len, char *s);
void __cdecl Scr_EvalArrayVariable(scriptInstance_t inst, unsigned int arrayId, VariableValue *value);
void __cdecl Scr_EvalArrayVariableInternal(scriptInstance_t inst, VariableValue *parentValue, VariableValue *value);
void __cdecl Scr_ClearValue(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_EvalFieldVariableInternal(
                scriptInstance_t inst,
                unsigned int objectId,
                unsigned int fieldName,
                VariableValue *value);
void __cdecl Scr_EvalFieldVariable(
                scriptInstance_t inst,
                unsigned int fieldName,
                VariableValue *value,
                unsigned int objectId);
void __cdecl Scr_CompileExpression(scriptInstance_t inst, sval_u *expr);
void __cdecl Scr_CompilePrimitiveExpression(scriptInstance_t inst, sval_u *expr);
void __cdecl Scr_CompileVariableExpression(scriptInstance_t inst, sval_u *expr);
void __cdecl Scr_CompilePrimitiveExpressionFieldObject(scriptInstance_t inst, sval_u *expr);
void __cdecl Scr_CompilePrimitiveExpressionList(scriptInstance_t inst, sval_u *exprlist);
char __cdecl Scr_CompileCallExpression(scriptInstance_t inst, sval_u *expr);
char __cdecl Scr_CompileFunction(scriptInstance_t inst, sval_u *func_name, sval_u *params);
void __cdecl Scr_CompileCallExpressionList(scriptInstance_t inst, sval_u *exprlist);
char __cdecl Scr_CompileMethod(scriptInstance_t inst, sval_u *expr, sval_u *func_name, sval_u *params);
void __cdecl Scr_CompileText(scriptInstance_t inst, const char *text, ScriptExpression_t *scriptExpr);
void __cdecl Scr_CompileTextInternal(scriptInstance_t inst, const char *text, ScriptExpression_t *scriptExpr);
bool __cdecl Scr_EvalScriptExpression(
                scriptInstance_t inst,
                ScriptExpression_t *expr,
                unsigned int localId,
                VariableValue *value,
                bool freezeScope,
                bool freezeObjects);
void __cdecl Scr_EvalExpression(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalPrimitiveExpression(
                scriptInstance_t inst,
                sval_u expr,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_EvalVariableExpression(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalArrayVariableExpression(
                scriptInstance_t inst,
                sval_u array,
                sval_u index,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_EvalLocalVariable(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value);
unsigned int __cdecl Scr_EvalObject(scriptInstance_t inst, sval_u classnum, sval_u entnum, VariableValue *value);
void __cdecl Scr_EvalSelfValue(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_GetValue(scriptInstance_t inst, unsigned int index, VariableValue *value);
int __cdecl Scr_EvalPrimitiveExpressionFieldObject(scriptInstance_t inst, sval_u expr, unsigned int localId);
void __cdecl Scr_EvalCallExpression(scriptInstance_t inst, sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalFunction(
                scriptInstance_t inst,
                sval_u func_name,
                sval_u params,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_PreEvalBuiltin(scriptInstance_t inst, sval_u params, unsigned int localId);
void __cdecl Scr_PostEvalBuiltin(scriptInstance_t inst, VariableValue *value);
void __cdecl Scr_EvalMethod(
                scriptInstance_t inst,
                sval_u expr,
                sval_u func_name,
                sval_u params,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_EvalBoolOrExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_EvalBoolAndExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_EvalBinaryOperatorExpression(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u opcode,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_EvalVector(
                scriptInstance_t inst,
                sval_u expr1,
                sval_u expr2,
                sval_u expr3,
                unsigned int localId,
                VariableValue *value);
void __cdecl Scr_ClearDebugExprValue(scriptInstance_t inst, sval_u val);
bool __cdecl Scr_RefScriptExpression(scriptInstance_t inst, ScriptExpression_t *expr);
bool __cdecl Scr_RefExpression(scriptInstance_t inst, sval_u expr);
bool __cdecl Scr_RefPrimitiveExpression(scriptInstance_t inst, sval_u expr);
bool __cdecl Scr_RefVariableExpression(scriptInstance_t inst, sval_u expr);
bool __cdecl Scr_RefArrayVariableExpression(scriptInstance_t inst, sval_u array, sval_u index);
bool __cdecl Scr_RefBreakonExpression(scriptInstance_t inst, sval_u expr, sval_u param);
bool __cdecl Scr_RefCallExpression(scriptInstance_t inst, sval_u expr);
bool __cdecl Scr_RefCall(scriptInstance_t inst, sval_u params);
bool __cdecl Scr_RefMethod(scriptInstance_t inst, sval_u expr, sval_u params);
bool __cdecl Scr_RefBinaryOperatorExpression(scriptInstance_t inst, sval_u expr1, sval_u expr2);
bool __cdecl Scr_RefVector(scriptInstance_t inst, sval_u expr1, sval_u expr2, sval_u expr3);
void __cdecl Scr_FreeDebugExprValue(scriptInstance_t inst, sval_u val);


extern int g_script_error_level[2];
#ifdef KISAK_NX
#include <setjmp.h>
extern jmp_buf g_script_error[2][33];
#else
extern int g_script_error[2][33][16];
#endif