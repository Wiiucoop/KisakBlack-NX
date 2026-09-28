#pragma once
// nx-port: code positions inside script bytecode.
//
// The x86 compiler wrote every code position -- a call's target, a thread's
// entry point, the chain of placeholders the linker patches later, a switch
// case's target -- into the byte stream as a 4-byte word, and the VM read it
// back the same way. On LP64 a code position is a pointer, so those operands
// are pointer-sized, written and read here and nowhere else. Integer operands
// that merely travelled through EmitCodepos on x86 (jump offsets, parameter
// counts, case names, integer constants) stay 4 bytes: see EmitCodeInt.
//
// Bytecode is packed with no alignment, so every access is a memcpy.
#include <string.h>

enum { SCR_CODEPOS_SIZE = sizeof(const char *) };

static inline const char *Scr_ReadCodePosAt(const char *slot)
{
    const char *pos;
    memcpy(&pos, slot, sizeof(pos));
    return pos;
}

static inline void Scr_WriteCodePosAt(char *slot, const char *pos)
{
    memcpy(slot, &pos, sizeof(pos));
}

// A switch table entry: the case's name (a string or integer, 4 bytes) and
// the code position it jumps to.
enum { SCR_CASE_ENTRY_SIZE = 4 + SCR_CODEPOS_SIZE };

static inline unsigned int Scr_CaseEntryName(const char *entry)
{
    unsigned int name;
    memcpy(&name, entry, sizeof(name));
    return name;
}
