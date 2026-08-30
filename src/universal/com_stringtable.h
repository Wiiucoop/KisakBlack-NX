#pragma once

struct StringTableCell // sizeof=0x8
{
    const char *string;
    int hash;
};

struct StringTable // sizeof=0x14
{                                       // XREF: XAssetPoolEntry<StringTable>/r
    const char *name;
    int columnCount;
    int rowCount;
    StringTableCell *values;
    __int16 *cellIndex;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(StringTable) == 20);
#endif

union XAssetHeader;

int __cdecl StringTable_LookupRowNumForValue(const StringTable *table, int comparisonColumn, const char *value);
int __cdecl StringTable_FirstHashIndex(const StringTable *table, int hash);
int __cdecl StringTable_LookupColumnNumForValue(const StringTable *table, int comparisonRow, const char *value);
int __cdecl StringTable_LookupRowNumForValue(
                const StringTable *table,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2,
                int comparisonColumn3,
                const char *value3);
const char *__cdecl StringTable_GetColumnValueForRow(const StringTable *table, int row, int column);
const char *__cdecl StringTable_Lookup(
                const StringTable *table,
                int comparisonColumn,
                const char *value,
                int valueColumn);
const char *__cdecl StringTable_Lookup(
                const StringTable *table,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2,
                int valueColumn);
int __cdecl StringTable_Find(
                const StringTable *table,
                int *results,
                int maxResults,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2);
int __cdecl StringTable_RowCount(const StringTable *table);
int __cdecl StringTable_ColumnCount(const StringTable *table);
int __cdecl StringTable_Checksum(const StringTable *table, unsigned int initialCrc);
void __cdecl StringTable_GetAsset_FastFile(const char *filename, XAssetHeader *tablePtr);
void __cdecl StringTable_GetAsset(const char *filename, XAssetHeader *tablePtr);
