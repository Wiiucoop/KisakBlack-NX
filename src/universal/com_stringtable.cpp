#include "com_stringtable.h"
#include "com_stringtable_obj.h"
#include "q_shared.h"
#include <qcommon/common.h>
#include <string.h>
#include <database/db_registry.h>
#include <xanim/xanim.h>
#include <xanim/xmodel.h>
#include <qcommon/md4.h>

int __cdecl StringTable_LookupRowNumForValue(const StringTable *table, int comparisonColumn, const char *value)
{
    int cell; // [esp+10h] [ebp-10h]
    int hash; // [esp+14h] [ebp-Ch]
    int idx; // [esp+18h] [ebp-8h]
    int valueCount; // [esp+1Ch] [ebp-4h]

    if ( !table->columnCount )
        return -1;
    if ( comparisonColumn < table->columnCount )
    {
        hash = StringTable_HashString(value);
        valueCount = table->columnCount * table->rowCount;
        idx = StringTable_FirstHashIndex(table, hash);
        if ( idx >= 0 )
        {
            while ( idx < valueCount )
            {
                cell = table->cellIndex[idx];
                if ( table->values[cell].hash != hash )
                    return -1;
                if ( cell % table->columnCount == comparisonColumn
                    && table->values[cell].string
                    && !I_stricmp(value, table->values[cell].string) )
                {
                    return cell / table->columnCount;
                }
                ++idx;
            }
            return -1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        Com_PrintError(
            13,
            "Unable to compare against column number %i - there are only %i columns",
            comparisonColumn,
            table->columnCount);
        return -1;
    }
}

int __cdecl StringTable_FirstHashIndex(const StringTable *table, int hash)
{
    int end; // [esp+0h] [ebp-10h]
    int begin; // [esp+8h] [ebp-8h]

    begin = 0;
    end = table->columnCount * table->rowCount;
    while ( begin < end )
    {
        if ( table->values[table->cellIndex[(end + begin) / 2]].hash >= hash )
            end = (end + begin) / 2;
        else
            begin = (end + begin) / 2 + 1;
    }
    return begin;
}

int __cdecl StringTable_LookupColumnNumForValue(const StringTable *table, int comparisonRow, const char *value)
{
    int hash; // [esp+0h] [ebp-Ch]
    int slot; // [esp+4h] [ebp-8h]
    int col; // [esp+8h] [ebp-4h]

    hash = StringTable_HashString(value);
    if ( !table->columnCount )
        return -1;
    if ( comparisonRow < table->rowCount )
    {
        for ( col = table->columnCount - 1; col >= 0; --col )
        {
            slot = col + table->columnCount * comparisonRow;
            if ( table->values[slot].hash == hash
                && table->values[slot].string
                && !I_stricmp(value, table->values[slot].string) )
            {
                return col;
            }
        }
        return -1;
    }
    else
    {
        Com_PrintError(
            13,
            "Unable to compare against row number %i - there are only %i row",
            comparisonRow,
            table->rowCount);
        return -1;
    }
}

int __cdecl StringTable_LookupRowNumForValue(
                const StringTable *table,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2,
                int comparisonColumn3,
                const char *value3)
{
    int v8; // [esp+14h] [ebp-28h]
    int slot2; // [esp+18h] [ebp-24h]
    int row; // [esp+1Ch] [ebp-20h]
    int slot3; // [esp+20h] [ebp-1Ch]
    int slot1; // [esp+24h] [ebp-18h]
    int hash3; // [esp+28h] [ebp-14h]
    int idx; // [esp+2Ch] [ebp-10h]
    int hash1; // [esp+30h] [ebp-Ch]
    int hash2; // [esp+34h] [ebp-8h]
    int valueCount; // [esp+38h] [ebp-4h]

    if ( !table->columnCount )
        return -1;
    if ( comparisonColumn1 < table->columnCount
        && comparisonColumn2 < table->columnCount
        && comparisonColumn3 < table->columnCount )
    {
        hash1 = StringTable_HashString(value1);
        hash2 = 0;
        hash3 = 0;
        if ( value2 )
            hash2 = StringTable_HashString(value2);
        if ( value3 )
            hash3 = StringTable_HashString(value3);
        valueCount = table->columnCount * table->rowCount;
        idx = StringTable_FirstHashIndex(table, hash1);
        if ( idx >= 0 )
        {
            while ( idx < valueCount )
            {
                slot1 = table->cellIndex[idx];
                if ( table->values[slot1].hash != hash1 )
                    return -1;
                if ( slot1 % table->columnCount == comparisonColumn1 )
                {
                    row = slot1 / table->columnCount;
                    slot2 = comparisonColumn2 + table->columnCount * row;
                    slot3 = comparisonColumn3 + table->columnCount * row;
                    if ( table->values[slot1].string )
                    {
                        if ( !I_stricmp(value1, table->values[slot1].string)
                            && (!value2
                             || table->values[slot2].hash == hash2
                             && table->values[slot2].string
                             && !I_stricmp(value2, table->values[slot2].string))
                            && (!value3
                             || table->values[slot3].hash == hash3
                             && table->values[slot3].string
                             && !I_stricmp(value3, table->values[slot3].string)) )
                        {
                            return row;
                        }
                    }
                }
                ++idx;
            }
            return -1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if ( comparisonColumn1 < comparisonColumn2 )
            v8 = comparisonColumn2;
        else
            v8 = comparisonColumn1;
        if ( v8 < comparisonColumn3 )
            Com_PrintError(
                13,
                "Unable to compare against column number %i - there are only %i columns",
                comparisonColumn3,
                table->columnCount);
        else
            Com_PrintError(
                13,
                "Unable to compare against column number %i - there are only %i columns",
                v8,
                table->columnCount);
        return -1;
    }
}

const char *__cdecl StringTable_GetColumnValueForRow(const StringTable *table, int row, int column)
{
    if ( column < table->columnCount
        && row < table->rowCount
        && row >= 0
        && column >= 0
        && table->values[column + table->columnCount * row].string )
    {
        return table->values[column + table->columnCount * row].string;
    }
    else
    {
        return "";
    }
}

const char *__cdecl StringTable_Lookup(
                const StringTable *table,
                int comparisonColumn,
                const char *value,
                int valueColumn)
{
    int v5; // eax

    if ( table )
    {
        v5 = StringTable_LookupRowNumForValue(table, comparisonColumn, value);
        return StringTable_GetColumnValueForRow(table, v5, valueColumn);
    }
    else
    {
        Com_PrintError(13, "Unable to find the lookup table in the fastfile, aborting lookup\n");
        return "";
    }
}

const char *__cdecl StringTable_Lookup(
                const StringTable *table,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2,
                int valueColumn)
{
    int v7; // eax

    if ( table )
    {
        v7 = StringTable_LookupRowNumForValue(table, comparisonColumn1, value1, comparisonColumn2, value2, -1, 0);
        return StringTable_GetColumnValueForRow(table, v7, valueColumn);
    }
    else
    {
        Com_PrintError(13, "Unable to find the lookup table in the fastfile, aborting lookup\n");
        return "";
    }
}

int __cdecl StringTable_Find(
                const StringTable *table,
                int *results,
                int maxResults,
                int comparisonColumn1,
                const char *value1,
                int comparisonColumn2,
                const char *value2)
{
    int slot1; // [esp+4h] [ebp-18h]
    int slot2; // [esp+8h] [ebp-14h]
    int row; // [esp+Ch] [ebp-10h]
    int hash1; // [esp+10h] [ebp-Ch]
    int hash2; // [esp+14h] [ebp-8h]
    int nFound; // [esp+18h] [ebp-4h]

    if ( !table->columnCount )
        return 0;
    if ( comparisonColumn1 < table->columnCount && comparisonColumn2 < table->columnCount )
    {
        hash1 = StringTable_HashString(value1);
        hash2 = 0;
        if ( value2 )
            hash2 = StringTable_HashString(value2);
        nFound = 0;
        for ( row = 0; row < table->rowCount; ++row )
        {
            slot1 = comparisonColumn1 + table->columnCount * row;
            slot2 = comparisonColumn2 + table->columnCount * row;
            if ( table->values[slot1].hash == hash1
                && table->values[slot1].string
                && !I_stricmp(value1, table->values[slot1].string)
                && (!value2
                 || table->values[slot2].hash == hash2
                 && table->values[slot2].string
                 && !I_stricmp(value2, table->values[slot2].string)) )
            {
                if ( nFound == maxResults )
                {
                    Com_PrintError(13, "Too many matches found. Increase maxResults: %i", maxResults);
                    return maxResults;
                }
                results[nFound++] = row;
            }
        }
        return nFound;
    }
    else
    {
        if ( comparisonColumn1 < comparisonColumn2 )
            Com_PrintError(
                13,
                "Unable to compare against column number %i - there are only %i columns",
                comparisonColumn2,
                table->columnCount);
        else
            Com_PrintError(
                13,
                "Unable to compare against column number %i - there are only %i columns",
                comparisonColumn1,
                table->columnCount);
        return 0;
    }
}

int __cdecl StringTable_RowCount(const StringTable *table)
{
    if ( !table
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_stringtable.cpp", 241, 0, "%s", "table") )
    {
        __debugbreak();
    }
    return table->rowCount;
}

int __cdecl StringTable_ColumnCount(const StringTable *table)
{
    if ( !table
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_stringtable.cpp", 248, 0, "%s", "table") )
    {
        __debugbreak();
    }
    return table->columnCount;
}

int __cdecl StringTable_Checksum(const StringTable *table, unsigned int initialCrc)
{
    const char *val; // [esp+20h] [ebp-10h]
    int row; // [esp+24h] [ebp-Ch]
    int crc; // [esp+28h] [ebp-8h]
    int col; // [esp+2Ch] [ebp-4h]

    crc = Com_BlockChecksumKey32((const unsigned __int8 *)table->name, strlen(table->name), initialCrc);
    for ( col = 0; col < table->columnCount; ++col )
    {
        for ( row = 0; row < table->rowCount; ++row )
        {
            val = table->values[col + table->columnCount * row].string;
            crc = Com_BlockChecksumKey32((const unsigned __int8 *)val, strlen(val), crc);
        }
    }
    return crc;
}

void __cdecl StringTable_GetAsset_FastFile(const char *filename, XAssetHeader *tablePtr)
{
#ifdef KISAK_NX
    // errorIfMissing=0: return null on a missing table instead of loading the
    // default and fatal-erroring. Lets boot proceed past optional stringtables
    // (e.g. the dev-console restricted-access lists) while the fastfile asset
    // format is still being ported to 64-bit.
    tablePtr->xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, (char*)filename, 0, -1).xmodelPieces;
#else
    tablePtr->xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, (char*)filename, 1, -1).xmodelPieces;
#endif
}

void __cdecl StringTable_GetAsset(const char *filename, XAssetHeader *tablePtr)
{
    if ( useFastFile->current.enabled )
        StringTable_GetAsset_FastFile(filename, tablePtr);
    else
        StringTable_GetAsset_LoadObj(filename, (StringTable **)tablePtr);
}

