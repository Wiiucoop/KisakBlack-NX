#pragma once

struct Glyph // sizeof=0x18
{
    unsigned __int16 letter;
    char x0;
    char y0;
    unsigned __int8 dx;
    unsigned __int8 pixelWidth;
    unsigned __int8 pixelHeight;
    // padding byte
    float s0;
    float t0;
    float s1;
    float t1;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(Glyph) == 24);
#endif

struct Font_s // sizeof=0x18
{                                       // XREF: XAssetPoolEntry<Font_s>/r
    const char *fontName;
    int pixelHeight;
    int glyphCount;
    struct Material *material;
    struct Material *glowMaterial;
    Glyph *glyphs;
};
#ifndef KISAK_NX // nx-port: x86 layout assert
static_assert(sizeof(Font_s) == 24);
#endif

const Glyph *__cdecl R_GetCharacterGlyph(Font_s *font, unsigned int letter);
unsigned int __cdecl R_FontGetRandomLetter(Font_s *font, int seed);
unsigned int __cdecl R_FontGetRandomNumberCharacter(Font_s *font, int seed);
Font_s *__cdecl R_RegisterFont(const char *name, int imageTrack);
Font_s *__cdecl R_RegisterFont_LoadObj(const char *fontName, int imageTrack);
Font_s *__cdecl R_RegisterFont_FastFile(const char *fontName);
void __cdecl R_InitFonts();
void __cdecl R_ShutdownFonts();
double __cdecl R_NormalizedTextScale(Font_s *font, float scale);
int __cdecl R_LetterWidth(unsigned int letter, Font_s *font);
int __cdecl R_TextWidth(const char *text, int maxChars, Font_s *font);
int __cdecl R_TextHeight(Font_s *font);
const char *__cdecl R_TextLineWrapPosition(
                const char *text,
                int bufferSize,
                int pixelsAvailable,
                Font_s *font,
                float scale);
int __cdecl R_ConsoleTextWidth(const char *textPool, int poolSize, int firstChar, int charCount, Font_s *font);
