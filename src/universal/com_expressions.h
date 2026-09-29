#pragma once

enum expDataType : __int32
{                                                                             // XREF: Operand/r
                                                                                // operandDataType/r ...
    VAL_INT = 0x0,
    VAL_FLOAT = 0x1,
    VAL_STRING = 0x2,
};

enum expOperationEnum : __int32
{                                       // XREF: entryInternalData/r
                                        // Operator/r ...
    OP_NOOP                  = 0x0,
    OP_RIGHTPAREN            = 0x1,
    OP_MULTIPLY              = 0x2,
    OP_DIVIDE                = 0x3,
    OP_MODULUS               = 0x4,
    OP_ADD                   = 0x5,
    OP_SUBTRACT              = 0x6,
    OP_NEGATE                = 0x7,
    OP_NOT                   = 0x8,
    OP_LESSTHAN              = 0x9,
    OP_LESSTHANEQUALTO       = 0xA,
    OP_GREATERTHAN           = 0xB,
    OP_GREATERTHANEQUALTO    = 0xC,
    OP_EQUALS                = 0xD,
    OP_NOTEQUAL              = 0xE,
    OP_AND                   = 0xF,
    OP_OR                    = 0x10,
    OP_LEFTPAREN             = 0x11,
    OP_COMMA                 = 0x12,
    OP_BITWISEAND            = 0x13,
    OP_BITWISEOR             = 0x14,
    OP_BITWISENOT            = 0x15,
    OP_BITSHIFTLEFT          = 0x16,
    OP_BITSHIFTRIGHT         = 0x17,
    NUM_EXPRESSION_OPERATORS = 0x18,
    MAX_OPERATOR_VALUE       = 0x4000,
};

union operandInternalDataUnion // sizeof=0x4
{                                                                             // XREF: Expression_Parse(char const * *,ExpressionStatement *,void *,int)+19/w
    int intVal;
    float floatVal;
    const char *string;

    // nx-port: string makes the union 8 bytes; clear all of it so an int or
    // float never leaves stale high bits behind.
    operandInternalDataUnion()
    {
        string = nullptr;
    }
    operandInternalDataUnion(int i)
    {
        string = nullptr;
        intVal = i;
    }
    operandInternalDataUnion(float f)
    {
        string = nullptr;
        floatVal = f;
    }
    operandInternalDataUnion(const char *str)
    {
        string = str;
    }

    explicit operator int()
    {
        return intVal;
    }
    explicit operator float()
    {
        return floatVal;
    }
};

struct Operand // sizeof=0x8
{                                                                             // XREF: GetExpressionStringInt(int,itemDef_s *,ExpressionStatement *,char const * *,int *)+40/o
                                                                                // expressionRpnDataUnion/r ...
    expDataType dataType;                             // XREF: Expression_Parse(char const * *,ExpressionStatement *,void *,int)+16/w
    // Expression_Parse(char const * *,ExpressionStatement *,void *,int)+1A0/r ...
    operandInternalDataUnion internals; // XREF: Expression_Parse(char const * *,ExpressionStatement *,void *,int)+19/w
    // Expression_Parse(char const * *,ExpressionStatement *,void *,int):loc_78A62C/r ...

    Operand()
    {
        dataType = VAL_INT;
        internals.intVal = 0;
    }
};

union expressionRpnDataUnion // sizeof=0x8
{                                                                             // XREF: expressionRpn/r
    Operand constant;
    void *cmd;
    int cmdIdx;

    expressionRpnDataUnion() : constant() {};
    ~expressionRpnDataUnion() {}
};

struct expressionRpn // sizeof=0xC
{
    int type;
    expressionRpnDataUnion data;
};

struct ExpressionStatement // sizeof=0x10
{                                                                             // XREF: textExp_s/r
                                                                                // GenericEventScript/r ...
    char *filename;                                         // XREF: PC_CndStackPush(int,ExpressionStatement,bool,bool,int,int,int)+47/r
    // PC_EventScript_Parse+398/r ...
    int line;                                                     // XREF: PC_CndStackPush(int,ExpressionStatement,bool,bool,int,int,int)+4C/r
    // PC_EventScript_Parse+3A0/r ...
    int numRpn;                                                 // XREF: PC_CndStackPush(int,ExpressionStatement,bool,bool,int,int,int)+52/r
    // PC_EventScript_Parse+3A9/r ...
    expressionRpn *rpn;                                 // XREF: PC_CndStackPush(int,ExpressionStatement,bool,bool,int,int,int)+58/r
    // PC_EventScript_Parse+3B2/r ...
};

union entryInternalData // sizeof=0x8
{                                       // XREF: expressionEntry/r
    expOperationEnum op;
    Operand operand;
};

struct expressionEntry // sizeof=0x10
{                                       // XREF: expressionEntryNext/r
    int type;
    entryInternalData data;
    expressionEntry *next;
};

struct ExpressionAllocState // sizeof=0x8
{                                       // XREF: ?Expression_Parse@@YA_NPAPBDPAUExpressionStatement@@PAXH@Z/r
    int size;                           // XREF: Expression_Parse(char const * *,ExpressionStatement *,void *,int)+32/w
    void *buffer;                       // XREF: Expression_Parse(char const * *,ExpressionStatement *,void *,int)+2C/w
};

void __cdecl Expression_Free(ExpressionStatement *statement);
expressionRpn *__cdecl MakeRPN(expressionEntry *entry, int *length);
char __cdecl Expression_Parse(
                const char **text,
                ExpressionStatement *statement,
                void *compileBuffer,
                int compileBufferSize);
const char *__cdecl GetOperandAsString(Operand operand);
expressionEntry *__cdecl Expression_Alloc(ExpressionAllocState *allocState, int size);
bool __cdecl Expression_OpIsFunction(expOperationEnum op);
int __cdecl Expression_ParseOperatorToken(const char *token);
expressionEntry *__cdecl Expression_HashOperand(const char *str, ExpressionAllocState *alloc);
expressionEntry *__cdecl Expression_StringOperand(const char *str, ExpressionAllocState *alloc);
char __cdecl Expression_Read(int handle, ExpressionStatement *statement);
void __cdecl Expression_Init();


extern const char *g_expFunctionNames[457];