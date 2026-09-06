#include "r_font.h"
#include <universal/assertive.h>
#include <universal/com_math.h>
#include <universal/q_shared.h>
#include "r_font_load_obj.h"
#include <qcommon/common.h>
#include <database/db_registry.h>
#include <stringed/stringed_hooks.h>
#include <ctype.h>
#include "r_init.h"

const char MYRANDOMNUMCHARS[10] =
{ '0', '2', '3', '4', '5', '6', '7', '8', '9', '\0' };

const char MYRANDOMCHARS[63] =
{
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z',
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '\0'
};


int registeredFontCount;
Font_s *registeredFont[16];


const Glyph *__cdecl R_GetCharacterGlyph(Font_s *font, unsigned int letter)
{
    Glyph *glyph; // [esp+0h] [ebp-10h]
    int top; // [esp+4h] [ebp-Ch]
    int bottom; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    if ( letter < 0x20 || letter > 0x7F )
    {
        top = font->glyphCount - 1;
        bottom = 96;
        while ( bottom <= top )
        {
            mid = (bottom + top) / 2;
            if ( font->glyphs[mid].letter == letter )
                return &font->glyphs[mid];
            if ( font->glyphs[mid].letter >= letter )
                top = mid - 1;
            else
                bottom = mid + 1;
        }
        return font->glyphs + 14;
    }
    else
    {
        glyph = &font->glyphs[letter - 32];
        if ( glyph->letter != letter
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp",
                        48,
                        0,
                        "%s",
                        "glyph->letter == letter") )
        {
            __debugbreak();
        }
        return glyph;
    }
}

unsigned int __cdecl R_FontGetRandomLetter(Font_s *font, int seed)
{
    return MYRANDOMCHARS[RandWithSeed(&seed) % 0x3E];
}

unsigned int __cdecl R_FontGetRandomNumberCharacter(Font_s *font, int seed)
{
    return MYRANDOMNUMCHARS[RandWithSeed(&seed) % 9];
}

Font_s *__cdecl R_RegisterFont(const char *name, int imageTrack)
{
    if ( useFastFile->current.enabled )
        // x86: cast a un puntatore a funzione con ritorno int. Su LP64
        // troncherebbe il puntatore a 32 bit. Gli argomenti in eccesso
        // erano gia' ignorati dal callee.
        return R_RegisterFont_FastFile(name);
    else
        return R_RegisterFont_LoadObj(name, imageTrack);
}

Font_s *__cdecl R_RegisterFont_LoadObj(const char *fontName, int imageTrack)
{
    Font_s *font; // [esp+0h] [ebp-8h]
    int fontIndex; // [esp+4h] [ebp-4h]

    if ( !rg.registered
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp", 112, 0, "%s", "rg.registered") )
    {
        __debugbreak();
    }
    for ( fontIndex = 0; fontIndex < registeredFontCount; ++fontIndex )
    {
        if ( !I_stricmp(fontName, registeredFont[fontIndex]->fontName) )
            return registeredFont[fontIndex];
    }
    if ( registeredFontCount < 16 )
    {
        font = R_LoadFont(fontName, imageTrack);
        if ( !font )
            Com_Error(ERR_DROP, "R_RegisterFont: Error while reading font '%s'", fontName);
        registeredFont[registeredFontCount++] = font;
        return font;
    }
    else
    {
        Com_Error(ERR_DROP, "R_RegisterFont: Too many fonts registered already.\n");
        return 0;
    }
}

Font_s *__cdecl R_RegisterFont_FastFile(const char *fontName)
{
    return DB_FindXAssetHeader(ASSET_TYPE_FONT, (char*)fontName, 1, -1).font;
}

void __cdecl R_InitFonts()
{
    if ( registeredFontCount )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp",
                        156,
                        0,
                        "%s",
                        "registeredFontCount == 0") )
            __debugbreak();
    }
}

void __cdecl R_ShutdownFonts()
{
    registeredFontCount = 0;
}

double __cdecl R_NormalizedTextScale(Font_s *font, float scale)
{
    if ( !font && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp", 168, 0, "%s", "font") )
        __debugbreak();
    if ( font->pixelHeight <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp",
                    169,
                    0,
                    "%s",
                    "font->pixelHeight > 0") )
    {
        __debugbreak();
    }
    return scale * 48.0 / (double)font->pixelHeight;
}

int __cdecl R_LetterWidth(unsigned int letter, Font_s *font)
{
    return R_GetCharacterGlyph(font, letter)->dx;
}

int __cdecl R_TextWidth(const char *text, int maxChars, Font_s *font)
{
    unsigned int letter; // [esp+0h] [ebp-10h]
    int lineWidth; // [esp+4h] [ebp-Ch]
    int maxWidth; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp", 185, 0, "%s", "text") )
        __debugbreak();
    if ( !font && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp", 186, 0, "%s", "font") )
        __debugbreak();
    lineWidth = 0;
    maxWidth = 0;
    if ( maxChars <= 0 )
        maxChars = 0x7FFFFFFF;
    count = 0;
    while ( *text && count < maxChars )
    {
        letter = SEH_ReadCharFromString(&text, 0);
        if ( letter == 13 || letter == 10 )
        {
            lineWidth = 0;
        }
        else if ( letter == 94 && (text && *text != 94 && *text >= 48 && *text <= 64 || text && *text == 70) )
        {
            ++text;
        }
        else if ( letter != 94 || *text != 67 || text == (const char *)-1 || text[1] == 94 || text[1] < 48 || text[1] > 64 )
        {
            if ( letter == 94 && *text == 76 )
            {
                ++text;
            }
            else
            {
                lineWidth += R_GetCharacterGlyph(font, letter)->dx;
                if ( maxWidth < lineWidth )
                    maxWidth = lineWidth;
                ++count;
            }
        }
        else
        {
            text += 2;
        }
    }
    return maxWidth;
}

int __cdecl R_TextHeight(Font_s *font)
{
    if ( !font && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp", 231, 0, "%s", "font") )
        __debugbreak();
    return font->pixelHeight;
}

const char *__cdecl R_TextLineWrapPosition(
                const char *text,
                int bufferSize,
                int pixelsAvailable,
                Font_s *font,
                float scale)
{
    int pixelsUsed; // [esp+Ch] [ebp-14h]
    const char *preLetterPos; // [esp+10h] [ebp-10h]
    const char *wrapPos; // [esp+14h] [ebp-Ch]
    const char *parsePos; // [esp+18h] [ebp-8h] BYREF
    unsigned int letter; // [esp+1Ch] [ebp-4h]

    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp", 323, 0, "%s", "text") )
        __debugbreak();
    pixelsUsed = 0;
    if ( bufferSize <= 0 )
        bufferSize = 0x7FFFFFFF;
    wrapPos = 0;
    preLetterPos = 0;
    parsePos = text;
    while ( *parsePos )
    {
        preLetterPos = parsePos;
        letter = SEH_ReadCharFromString(&parsePos, 0);
        if ( letter == 13 )
        {
            pixelsUsed = 0;
        }
        else
        {
            if ( letter == 10 )
                return parsePos;
            if ( letter == 94
                && (parsePos && *parsePos != 94 && *parsePos >= 48 && *parsePos <= 64 || parsePos && *parsePos == 70) )
            {
                ++parsePos;
            }
            else if ( letter == 94 && (*parsePos == 1 || *parsePos == 2) )
            {
                if ( font )
                    pixelsUsed += (font->pixelHeight * (parsePos[1] - 16) + 16) / 32;
                if ( preLetterPos != text )
                    wrapPos = preLetterPos;
                parsePos += 7;
            }
            else
            {
                if ( font )
                    pixelsUsed += R_LetterWidth(letter, font);
                if ( preLetterPos != text && letter < 0x100 && isspace(letter) )
                    wrapPos = parsePos;
            }
            if ( wrapPos && (float)((float)pixelsUsed * scale) > (float)pixelsAvailable )
                return wrapPos;
            if ( parsePos - text > bufferSize )
            {
                if ( wrapPos )
                    return wrapPos;
                else
                    return preLetterPos;
            }
            if ( (float)((float)pixelsUsed * scale) > (float)pixelsAvailable )
                return preLetterPos;
        }
    }
    if ( parsePos - text != bufferSize )
        return parsePos;
    if ( wrapPos )
        return wrapPos;
    else
        return preLetterPos;
}

int __cdecl R_ConsoleTextWidth(const char *textPool, int poolSize, int firstChar, int charCount, Font_s *font)
{
    int indexMask; // [esp+0h] [ebp-18h]
    int parsePos; // [esp+4h] [ebp-14h]
    int width; // [esp+8h] [ebp-10h]
    int letter; // [esp+Ch] [ebp-Ch]
    int usedCharCount; // [esp+10h] [ebp-8h] BYREF
    int stopPos; // [esp+14h] [ebp-4h]

    if ( (poolSize & (poolSize - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_font.cpp",
                    402,
                    0,
                    "%s",
                    "IsPowerOf2( poolSize )") )
    {
        __debugbreak();
    }
    indexMask = poolSize - 1;
    stopPos = (poolSize - 1) & (charCount + firstChar);
    parsePos = firstChar;
    width = 0;
    while ( parsePos != stopPos )
    {
        letter = SEH_DecodeLetter(textPool[parsePos], textPool[indexMask & (parsePos + 1)], &usedCharCount, 0);
        parsePos = indexMask & (usedCharCount + parsePos);
        if ( letter == 94
            && (&textPool[parsePos] && textPool[parsePos] != 94 && textPool[parsePos] >= 48 && textPool[parsePos] <= 64
             || &textPool[parsePos] && textPool[parsePos] == 70) )
        {
            parsePos = indexMask & (parsePos + 1);
        }
        else if ( letter == 94 && (textPool[parsePos] == 1 || textPool[parsePos] == 2) )
        {
            width += (font->pixelHeight * (textPool[indexMask & (parsePos + 1)] - 16) + 16) / 32;
            parsePos = indexMask & (parsePos + 7);
        }
        else
        {
            width += R_LetterWidth(letter, font);
        }
    }
    return width;
}

