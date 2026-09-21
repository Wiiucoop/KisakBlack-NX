#include "r_image_load_obj.h"
#include <qcommon/common.h>
#include "r_image.h"
#include "r_image_wavelet.h"
#include "r_image_load_common.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/threads.h>
#include "rb_resource.h"
#include "r_pixelcost_load_obj.h"
#include "r_outdoor.h"

const BuiltinImageConstructorTable constructorTable[12] =
{
  { "$white", &Image_LoadWhite },
  { "$black", &Image_LoadBlack },
  { "$blank", &Image_LoadBlackTransparent },
  { "$g16r16", &Image_LoadG16R16 },
  { "$r5g6b5", &Image_LoadR5G6B5 },
  { "$gray", &Image_LoadGray },
  { "$identitynormalmap", &Image_LoadIdentityNormalMap },
  { "$whitetransparent", &Image_LoadWhiteTransparent },
  { "$blacktransparent", &Image_LoadBlackTransparent },
  { "$outdoor", &R_GenerateOutdoorImage },
  { "$pixelcostcolorcode", &Image_LoadPixelCostColorCode },
  { "$white_r32f", &Image_LoadWhiteR32F }
};

FastCriticalSection s_imageLoadLock;

char __cdecl Image_ValidateHeader(GfxImageFileHeader *imageFile, const char *filepath)
{
    if ( imageFile->tag[0] == 73 && imageFile->tag[1] == 87 && imageFile->tag[2] == 105 )
    {
        if ( imageFile->version == 13 )
        {
            return 1;
        }
        else
        {
            Com_PrintError(8, "ERROR: image '%s' is version %i but should be version %i\n", filepath, imageFile->version, 13);
            return 0;
        }
    }
    else
    {
        Com_PrintError(8, "ERROR: image '%s' is not an IW image\n", filepath);
        return 0;
    }
}

void __cdecl Image_LoadFromData(GfxImage *image, GfxImageFileHeader *fileHeader, unsigned __int8 *srcData)
{
    const char *v3; // eax

    image->loadedSize = fileHeader->fileSizeForPicmip[image->skippedMipLevels] - 48;
    image->baseSize = fileHeader->fileSizeForPicmip[0] - 48;
    Image_TrackBytes += image->loadedSize;
    image->texture.basemap = 0;
    switch ( fileHeader->format )
    {
        case 1u:
            Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8R8G8B8, 4);
            break;
        case 2u:
            Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_X8R8G8B8, 3);
            break;
        case 3u:
            Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8L8, 2);
            break;
        case 4u:
            Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_L8, 1);
            break;
        case 5u:
            Image_LoadBitmap(image, fileHeader, srcData, D3DFMT_A8, 1);
            break;
        case 6u:
            Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8R8G8B8, 4);
            break;
        case 7u:
            Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_X8R8G8B8, 3);
            break;
        case 8u:
            Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8L8, 2);
            break;
        case 9u:
            Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_L8, 1);
            break;
        case 0xAu:
            Image_LoadWavelet(image, fileHeader, srcData, D3DFMT_A8, 1);
            break;
        case 0xBu:
            Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT1, 8);
            break;
        case 0xCu:
            Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT3, 16);
            break;
        case 0xDu:
            Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_DXT5, 16);
            break;
        case 0x13u:
            Image_LoadDxtc(image, fileHeader, srcData, D3DFMT_A16B16G16R16F, 128);
            break;
        default:
            v3 = va("unhandled case: %d", fileHeader->format);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 620, 1, v3) )
                __debugbreak();
            break;
    }
}

void __cdecl Image_LoadWavelet(
                GfxImage *image,
                const GfxImageFileHeader *fileHeader,
                const unsigned __int8 *data,
                _D3DFORMAT format,
                int bytesPerPixel)
{
    unsigned __int8 *v5; // eax
    unsigned int v6; // [esp+0h] [ebp-9Ch]
    unsigned int v7; // [esp+4h] [ebp-98h]
    unsigned __int8 *from[6]; // [esp+14h] [ebp-88h]
    unsigned __int8 *pixels[6]; // [esp+2Ch] [ebp-70h]
    int sizeForLevel; // [esp+44h] [ebp-58h]
    int width; // [esp+48h] [ebp-54h]
    int height; // [esp+4Ch] [ebp-50h]
    _D3DCUBEMAP_FACES face; // [esp+50h] [ebp-4Ch]
    unsigned int faceCount; // [esp+54h] [ebp-48h]
    unsigned int faceIndex; // [esp+58h] [ebp-44h]
    WaveletDecode decode; // [esp+5Ch] [ebp-40h] BYREF
    unsigned __int8 *to[6]; // [esp+7Ch] [ebp-20h]
    int picmip; // [esp+94h] [ebp-8h]
    int totalSize; // [esp+98h] [ebp-4h]

    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 307, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !fileHeader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 308, 0, "%s", "fileHeader") )
    {
        __debugbreak();
    }
    if ( !data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 309, 0, "%s", "data") )
    {
        __debugbreak();
    }
    Image_SetupFromFile(image, fileHeader, format);
    decode.value = 0;
    decode.bit = 0;
    decode.width = fileHeader->dimensions[0];
    decode.height = fileHeader->dimensions[1];
    decode.mipLevel = Image_CountMipmapsForFile(fileHeader) - 1;
    decode.channels = bytesPerPixel;
    decode.bpp = bytesPerPixel;
    decode.dataInitialized = 0;
    if ( bytesPerPixel == 3 )
        decode.bpp = 4;
    if ( image->mapType == 5 )
        faceCount = 6;
    else
        faceCount = 1;
    totalSize = decode.bpp * fileHeader->dimensions[1] * fileHeader->dimensions[0];
    for ( faceIndex = 0; faceIndex < faceCount; ++faceIndex )
    {
        v5 = Image_AllocTempMemory(totalSize);
        pixels[faceIndex] = v5;
        to[faceIndex] = 0;
    }
    picmip = image->picmip.platform[!useFastFile->current.enabled];
    decode.data = data;
    while ( decode.mipLevel >= picmip )
    {
        if ( (int)((unsigned int)decode.width >> SLOBYTE(decode.mipLevel)) > 1 )
            v7 = (unsigned int)decode.width >> SLOBYTE(decode.mipLevel);
        else
            v7 = 1;
        width = v7;
        if ( (int)((unsigned int)decode.height >> SLOBYTE(decode.mipLevel)) > 1 )
            v6 = (unsigned int)decode.height >> SLOBYTE(decode.mipLevel);
        else
            v6 = 1;
        height = v6;
        sizeForLevel = decode.bpp * v6 * width;
        for ( faceIndex = 0; faceIndex < faceCount; ++faceIndex )
        {
            from[faceIndex] = to[faceIndex];
            to[faceIndex] = &pixels[faceIndex][totalSize - sizeForLevel];
            Wavelet_DecompressLevel(from[faceIndex], to[faceIndex], &decode);
            face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
            Image_UploadData(image, format, face, decode.mipLevel - picmip, to[faceIndex]);
        }
        --decode.mipLevel;
    }
    faceIndex = faceCount;
    do
        Image_FreeTempMemory(pixels[--faceIndex], totalSize);
    while ( faceIndex );
}

void __cdecl Image_SetupFromFile(GfxImage *image, const GfxImageFileHeader *fileHeader, _D3DFORMAT imageFormat)
{
    unsigned int v3; // [esp+0h] [ebp-2Ch]
    unsigned int v4; // [esp+4h] [ebp-28h]
    unsigned int v5; // [esp+8h] [ebp-24h]
    unsigned __int8 picmip; // [esp+28h] [ebp-4h]

    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 194, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !fileHeader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 195, 0, "%s", "fileHeader") )
    {
        __debugbreak();
    }
    picmip = image->picmip.platform[!useFastFile->current.enabled];
    if ( (int)((unsigned int)fileHeader->dimensions[0] >> picmip) > 1 )
        v5 = (unsigned int)fileHeader->dimensions[0] >> picmip;
    else
        v5 = 1;
    if ( (int)((unsigned int)fileHeader->dimensions[1] >> picmip) > 1 )
        v4 = (unsigned int)fileHeader->dimensions[1] >> picmip;
    else
        v4 = 1;
    if ( (int)((unsigned int)fileHeader->dimensions[2] >> picmip) > 1 )
        v3 = (unsigned int)fileHeader->dimensions[2] >> picmip;
    else
        v3 = 1;
    Image_Setup(image, v5, v4, v3, fileHeader->flags, imageFormat);
    if ( image->cardMemory.platform[0] <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    205,
                    0,
                    "%s",
                    "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0") )
    {
        __debugbreak();
    }
}

unsigned int __cdecl Image_CountMipmapsForFile(const GfxImageFileHeader *fileHeader)
{
    return Image_CountMipmaps(
                     fileHeader->flags,
                     fileHeader->dimensions[0],
                     fileHeader->dimensions[1],
                     fileHeader->dimensions[2]);
}

unsigned int s_imageLoadBytesUsed;
unsigned __int8 *s_imageLoadBuf;
unsigned __int8 *__cdecl Image_AllocTempMemory(int bytes)
{
    unsigned __int8 *mem; // [esp+10h] [ebp-4h]
    unsigned int bytesa; // [esp+1Ch] [ebp+8h]

    bytesa = (bytes + 3) & 0xFFFFFFFC;
    if (bytesa + s_imageLoadBytesUsed > 0x1000000)
        Com_Error(
            ERR_DROP,
            "Needed to allocate at least %.1f MB to load images",
            (double)(bytesa + s_imageLoadBytesUsed) * 0.00000095367432);
    if (!s_imageLoadBuf)
    {
        s_imageLoadBuf = (unsigned __int8 *)Z_VirtualAlloc(0x1000000, "Image_AllocTempMemory", 19);
        if (!s_imageLoadBuf
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                267,
                0,
                "%s",
                "s_imageLoadBuf"))
        {
            __debugbreak();
        }
    }
    mem = &s_imageLoadBuf[s_imageLoadBytesUsed];
    s_imageLoadBytesUsed += bytesa;
    return mem;
}

void __cdecl Image_FreeTempMemory(unsigned __int8 *mem, int bytes)
{
    unsigned int bytesa; // [esp+Ch] [ebp+Ch]

    bytesa = (bytes + 3) & 0xFFFFFFFC;
    if ( &mem[bytesa] != &s_imageLoadBuf[s_imageLoadBytesUsed]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    283,
                    0,
                    "%s",
                    "mem + bytes == s_imageLoadBuf + s_imageLoadBytesUsed") )
    {
        __debugbreak();
    }
    s_imageLoadBytesUsed -= bytesa;
}

void __cdecl Image_LoadBitmap(
                GfxImage *image,
                const GfxImageFileHeader *fileHeader,
                unsigned __int8 *data,
                _D3DFORMAT format,
                int bytesPerPixel)
{
    unsigned int v5; // [esp+0h] [ebp-48h]
    unsigned int v6; // [esp+4h] [ebp-44h]
    unsigned int v7; // [esp+8h] [ebp-40h]
    unsigned int mipCount; // [esp+20h] [ebp-28h]
    _D3DCUBEMAP_FACES face; // [esp+2Ch] [ebp-1Ch]
    unsigned int faceCount; // [esp+30h] [ebp-18h]
    unsigned int faceIndex; // [esp+34h] [ebp-14h]
    unsigned __int8 *expandedData; // [esp+38h] [ebp-10h]
    int mipLevel; // [esp+3Ch] [ebp-Ch]
    int picmip; // [esp+40h] [ebp-8h]
    unsigned int expandedSize; // [esp+44h] [ebp-4h]

    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 378, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !fileHeader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 379, 0, "%s", "fileHeader") )
    {
        __debugbreak();
    }
    if ( !data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 380, 0, "%s", "data") )
    {
        __debugbreak();
    }
    Image_SetupFromFile(image, fileHeader, format);
    if ( image->mapType == 5 )
        faceCount = 6;
    else
        faceCount = 1;
    expandedData = 0;
    expandedSize = 4 * image->height * image->width * image->depth;
    if ( format == D3DFMT_X8R8G8B8 )
        expandedData = Image_AllocTempMemory(expandedSize);
    mipCount = Image_CountMipmapsForFile(fileHeader);
    picmip = image->picmip.platform[!useFastFile->current.enabled];
    for ( mipLevel = mipCount - 1; mipLevel >= picmip; --mipLevel )
    {
        if ( (int)((unsigned int)fileHeader->dimensions[0] >> mipLevel) > 1 )
            v7 = (unsigned int)fileHeader->dimensions[0] >> mipLevel;
        else
            v7 = 1;
        if ( (int)((unsigned int)fileHeader->dimensions[1] >> mipLevel) > 1 )
            v6 = (unsigned int)fileHeader->dimensions[1] >> mipLevel;
        else
            v6 = 1;
        if ( (int)((unsigned int)fileHeader->dimensions[2] >> mipLevel) > 1 )
            v5 = (unsigned int)fileHeader->dimensions[2] >> mipLevel;
        else
            v5 = 1;
        for ( faceIndex = 0; faceIndex < faceCount; ++faceIndex )
        {
            face = (D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
            if ( format == D3DFMT_X8R8G8B8 )
            {
                Image_ExpandBgr(data, v5 * v6 * v7, expandedData);
                Image_UploadData(image, D3DFMT_X8R8G8B8, face, mipLevel - picmip, expandedData);
            }
            else
            {
                if (format == D3DFMT_A8R8G8B8)
                {
                    //BLOPS_NULLSUB();
                }
                Image_UploadData(image, format, face, mipLevel - picmip, data);
            }
            data += bytesPerPixel * v6 * v7 * v5;
        }
    }
    if ( expandedData )
        Image_FreeTempMemory(expandedData, expandedSize);
}

void __cdecl Image_ExpandBgr(const unsigned __int8 *src, unsigned int count, unsigned __int8 *dst)
{
    if ( !src
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 222, 0, "%s", "src") )
    {
        __debugbreak();
    }
    if ( !dst
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 223, 0, "%s", "dst") )
    {
        __debugbreak();
    }
    if ( !count
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    224,
                    0,
                    "%s\n\t(count) = %i",
                    "(count > 0)",
                    0) )
    {
        __debugbreak();
    }
    do
    {
        *dst = *src;
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = -1;
        dst += 4;
        src += 3;
        --count;
    }
    while ( count );
}

void __cdecl Image_LoadDxtc(
                GfxImage *image,
                const GfxImageFileHeader *fileHeader,
                unsigned __int8 *data,
                _D3DFORMAT format,
                int bytesPerBlock)
{
    unsigned int v5; // [esp+0h] [ebp-34h]
    unsigned int v6; // [esp+4h] [ebp-30h]
    _D3DCUBEMAP_FACES face; // [esp+14h] [ebp-20h]
    unsigned int faceIndex; // [esp+18h] [ebp-1Ch]
    int mipLevel; // [esp+1Ch] [ebp-18h]
    unsigned int mipCount; // [esp+20h] [ebp-14h]
    unsigned int faceCount; // [esp+2Ch] [ebp-8h]
    int picmip; // [esp+30h] [ebp-4h]

    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 436, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !fileHeader
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 437, 0, "%s", "fileHeader") )
    {
        __debugbreak();
    }
    if ( !data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 438, 0, "%s", "data") )
    {
        __debugbreak();
    }
    if ( format != D3DFMT_A16B16G16R16F
        && format != D3DFMT_DXT1
        && format != D3DFMT_DXT3
        && format != D3DFMT_DXT5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    442,
                    0,
                    "%s",
                    "format == GFX_PF_A16B16G16R16F || format == GFX_PF_DXT1 || format == GFX_PF_DXT3 || format == GFX_PF_DXT5") )
    {
        __debugbreak();
    }
    Image_SetupFromFile(image, fileHeader, format);
    if ( image->mapType == 5 )
        faceCount = 6;
    else
        faceCount = 1;
    mipCount = Image_CountMipmapsForFile(fileHeader);
    picmip = image->picmip.platform[!useFastFile->current.enabled];
    for ( mipLevel = mipCount - 1; mipLevel >= picmip; --mipLevel )
    {
        if ( (int)((unsigned int)fileHeader->dimensions[0] >> mipLevel) > 1 )
            v6 = (unsigned int)fileHeader->dimensions[0] >> mipLevel;
        else
            v6 = 1;
        if ( (int)((unsigned int)fileHeader->dimensions[1] >> mipLevel) > 1 )
            v5 = (unsigned int)fileHeader->dimensions[1] >> mipLevel;
        else
            v5 = 1;
        for ( faceIndex = 0; faceIndex < faceCount; ++faceIndex )
        {
            face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
            Image_UploadData(image, format, face, mipLevel - picmip, data);
            data += bytesPerBlock * ((int)(v5 + 3) >> 2) * ((int)(v6 + 3) >> 2);
        }
    }
}

char __cdecl Image_LoadToBuffer(GfxImage *image, bool loadHighmip, unsigned __int8 **imageBuffer, int *bufferSize)
{
    unsigned int v5; // eax
    int v6; // [esp+8h] [ebp-98h]
    GfxImageFileHeader fileHeader; // [esp+18h] [ebp-88h] BYREF
    int fileSize; // [esp+48h] [ebp-58h]
    int fileHandle; // [esp+4Ch] [ebp-54h] BYREF
    char filepath[64]; // [esp+50h] [ebp-50h] BYREF
    unsigned __int8 streamedMipLevels; // [esp+97h] [ebp-9h]
    int picmip; // [esp+98h] [ebp-8h]
    int readSize; // [esp+9Ch] [ebp-4h]

    streamedMipLevels = 0;
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 1315, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( image->category != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1316,
                    0,
                    "%s",
                    "image->category == IMG_CATEGORY_LOAD_FROM_FILE") )
    {
        __debugbreak();
    }
    *imageBuffer = 0;
    *bufferSize = 0;
    if ( Com_sprintf(filepath, 0x40u, "%s%s%s", "images/", image->name, ".iwi") < 0 )
    {
        Com_PrintError(8, "ERROR: filename '%s' too long\n", filepath);
        return 0;
    }
    fileSize = FS_FOpenFileRead(filepath, &fileHandle);
    if ( fileSize < 0 )
    {
        Com_PrintError(8, "ERROR: image '%s' is missing\n", filepath);
        return 0;
    }
    if ( (unsigned int)fileSize < 0x30
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1334,
                    0,
                    "%s\n\t(filepath) = %s",
                    "(fileSize >= sizeof( fileHeader ))",
                    filepath) )
    {
        __debugbreak();
    }
    if ( FS_Read((unsigned __int8 *)&fileHeader, 0x30u, fileHandle) != 48 )
        goto LABEL_15;
    if ( Image_ValidateHeader(&fileHeader, filepath) )
    {
        if ( (fileHeader.flags & 3) != 0
            || (fileHeader.dimensions[1] < fileHeader.dimensions[0]
                ? (v6 = fileHeader.dimensions[1])
                : (v6 = fileHeader.dimensions[0]),
                    v6 < 32) )
        {
            image->noPicmip = 1;
        }
        Image_GetPicmip(image, &image->picmip);
        if ( fileHeader.fileSizeForPicmip[0] == fileSize )
        {
            picmip = image->picmip.platform[!useFastFile->current.enabled];
            streamedMipLevels = picmip != 0;
            if ( (unsigned __int8)(picmip != 0) >= 8u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                            1373,
                            0,
                            "streamedMipLevels doesn't index PICMIP_SIZES_STORED\n\t%i not in [0, %i)",
                            streamedMipLevels,
                            8) )
            {
                __debugbreak();
            }
            readSize = fileHeader.fileSizeForPicmip[picmip > 0] - 48;
            *bufferSize = readSize + 48;
            *imageBuffer = (unsigned __int8 *)Z_TryVirtualAlloc(*bufferSize, "Image_AllocTempMemory", 20);
            if ( !*imageBuffer )
            {
LABEL_15:
                Image_PrintTruncatedFileError(filepath);
                FS_FCloseFile(fileHandle);
                return 0;
            }
            memcpy(*imageBuffer, &fileHeader, 0x30u);
            v5 = FS_Read(*imageBuffer + 48, readSize, fileHandle);
            if ( v5 == readSize )
            {
                FS_FCloseFile(fileHandle);
                streamedMipLevels = Image_GetPcStreamedMips(&fileHeader);
                image->streaming = streamedMipLevels != 0;
                if ( !image->streaming || loadHighmip )
                {
                    image->skippedMipLevels = 0;
                    streamedMipLevels = 0;
                }
                else
                {
                    fileHeader.dimensions[0] >>= streamedMipLevels;
                    fileHeader.dimensions[1] >>= streamedMipLevels;
                    image->skippedMipLevels = streamedMipLevels;
                }
                return 1;
            }
            else
            {
                Image_PrintTruncatedFileError(filepath);
                Z_VirtualFree(*imageBuffer, 20);
                FS_FCloseFile(fileHandle);
                return 0;
            }
        }
        else
        {
            Com_PrintError(
                8,
                "ERROR: image '%s' is invalid. fileHeader.fileSizeForPicmip[0] != fileSize (%d != %d)\n",
                filepath,
                fileHeader.fileSizeForPicmip[0],
                fileSize);
            FS_FCloseFile(fileHandle);
            return 0;
        }
    }
    else
    {
        FS_FCloseFile(fileHandle);
        return 0;
    }
}

unsigned __int8 __cdecl Image_GetPcStreamedMips(GfxImageFileHeader *fileHeader)
{
    int v2; // [esp+0h] [ebp-14h]
    int minDimension; // [esp+Ch] [ebp-8h]
    unsigned __int8 streamedMipLevels; // [esp+13h] [ebp-1h]

    streamedMipLevels = 0;
    if ( (fileHeader->flags & 0x10) == 0 )
        return 0;
    if ( (fileHeader->flags & 0xC) != 0 )
        return 0;
    if ( fileHeader->dimensions[1] < fileHeader->dimensions[0] )
        v2 = fileHeader->dimensions[1];
    else
        v2 = fileHeader->dimensions[0];
    for ( minDimension = v2; minDimension > 128; minDimension >>= 1 )
        ++streamedMipLevels;
    return streamedMipLevels;
}

void __cdecl Image_PrintTruncatedFileError(const char *filepath)
{
    Com_PrintError(8, "ERROR: image '%s' is truncated.    Delete the file and run converter to fix.\n", filepath);
}

struct __declspec(align(4))// $3429FE2B485ED1F097C487ED1F67B3DD // sizeof=0x10
{                                       // XREF: .data:callbackParams/r
    GfxImage *image;                    // XREF: readCallback(void)+11/r
                                        // Image_LoadFromFileWithReader+12/w
    bool loadHighmip;                   // XREF: readCallback(void)+9/r
                                        // Image_LoadFromFileWithReader+1B/w
    // padding byte
    // padding byte
    // padding byte
    int (__cdecl *OpenFileRead)(const char *, int *);
                                        // XREF: readCallback(void)+3/r
                                        // Image_LoadFromFileWithReader+24/w
    bool ret;                           // XREF: readCallback(void)+20/w
                                        // Image_LoadFromFileWithReader+3B/r
    // padding byte
    // padding byte
    // padding byte
} callbackParams;
void __cdecl readCallback()
{
    callbackParams.ret = _Image_LoadFromFileWithReader(
                                                 callbackParams.image,
                                                 callbackParams.loadHighmip,
                                                 callbackParams.OpenFileRead);
}

char __cdecl _Image_LoadFromFileWithReader(
                GfxImage *image,
                bool loadHighmip,
                int (__cdecl *OpenFileRead)(const char *, int *))
{
    unsigned int v4; // eax
    int v5; // [esp+0h] [ebp-98h]
    unsigned __int8 *imageData; // [esp+Ch] [ebp-8Ch]
    GfxImageFileHeader fileHeader; // [esp+10h] [ebp-88h] BYREF
    int fileSize; // [esp+40h] [ebp-58h]
    int fileHandle; // [esp+44h] [ebp-54h] BYREF
    char filepath[64]; // [esp+48h] [ebp-50h] BYREF
    unsigned __int8 streamedMipLevels; // [esp+8Fh] [ebp-9h]
    int picmip; // [esp+90h] [ebp-8h]
    int readSize; // [esp+94h] [ebp-4h]

    streamedMipLevels = 0;
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 1485, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( image->category != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1487,
                    0,
                    "%s",
                    "image->category == IMG_CATEGORY_LOAD_FROM_FILE") )
    {
        __debugbreak();
    }
    if ( image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1489,
                    0,
                    "%s",
                    "!image->texture.basemap") )
    {
        __debugbreak();
    }
    if ( Com_sprintf(filepath, 0x40u, "%s%s%s", "images/", image->name, ".iwi") >= 0 )
    {
        fileSize = OpenFileRead(filepath, &fileHandle);
        if ( fileSize >= 0 )
        {
            if ( (unsigned int)fileSize < 0x30
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                            1505,
                            0,
                            "%s\n\t(filepath) = %s",
                            "(fileSize >= sizeof( fileHeader ))",
                            filepath) )
            {
                __debugbreak();
            }
            if ( FS_Read((unsigned __int8 *)&fileHeader, 0x30u, fileHandle) == 48 )
            {
                if ( Image_ValidateHeader(&fileHeader, filepath) )
                {
                    if ( (fileHeader.flags & 3) != 0
                        || (fileHeader.dimensions[1] < fileHeader.dimensions[0]
                            ? (v5 = fileHeader.dimensions[1])
                            : (v5 = fileHeader.dimensions[0]),
                                v5 < 32) )
                    {
                        image->noPicmip = 1;
                    }
                    Image_GetPicmip(image, &image->picmip);
                    if ( fileHeader.fileSizeForPicmip[0] == fileSize )
                    {
                        picmip = image->picmip.platform[!useFastFile->current.enabled];
                        streamedMipLevels = picmip != 0;
                        if ( (unsigned __int8)(picmip != 0) >= 8u
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                                        1670,
                                        0,
                                        "streamedMipLevels doesn't index PICMIP_SIZES_STORED\n\t%i not in [0, %i)",
                                        streamedMipLevels,
                                        8) )
                        {
                            __debugbreak();
                        }
                        readSize = fileHeader.fileSizeForPicmip[picmip > 0] - 48;
                        imageData = Image_AllocTempMemory(readSize);
                        v4 = FS_Read(imageData, readSize, fileHandle);
                        if ( v4 == readSize )
                        {
                            FS_FCloseFile(fileHandle);
                            streamedMipLevels = Image_GetPcStreamedMips(&fileHeader);
                            image->streaming = streamedMipLevels != 0;
                            if ( !image->streaming || loadHighmip )
                            {
                                image->skippedMipLevels = 0;
                                streamedMipLevels = 0;
                            }
                            else
                            {
                                fileHeader.dimensions[0] >>= streamedMipLevels;
                                fileHeader.dimensions[1] >>= streamedMipLevels;
                                image->skippedMipLevels = streamedMipLevels;
                            }
                            image->loadedSize = fileHeader.fileSizeForPicmip[streamedMipLevels] - 48;
                            image->baseSize = fileHeader.fileSizeForPicmip[0] - 48;
                            Image_LoadFromData(image, &fileHeader, imageData);
                            Image_FreeTempMemory(imageData, readSize);
                            return 1;
                        }
                        else
                        {
                            Image_PrintTruncatedFileError(filepath);
                            Image_FreeTempMemory(imageData, readSize);
                            FS_FCloseFile(fileHandle);
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(
                            8,
                            "ERROR: image '%s' is invalid. fileHeader.fileSizeForPicmip[0] != fileSize (%d != %d)\n",
                            filepath,
                            fileHeader.fileSizeForPicmip[0],
                            fileSize);
                        FS_FCloseFile(fileHandle);
                        return 0;
                    }
                }
                else
                {
                    FS_FCloseFile(fileHandle);
                    return 0;
                }
            }
            else
            {
                Image_PrintTruncatedFileError(filepath);
                FS_FCloseFile(fileHandle);
                return 0;
            }
        }
        else
        {
            Com_PrintError(8, "ERROR: image '%s' is missing\n", filepath);
            return 0;
        }
    }
    else
    {
        Com_PrintError(8, "ERROR: filename '%s' too long\n", filepath);
        return 0;
    }
}

bool __cdecl Image_LoadFromFile(GfxImage *image, bool loadHighmip)
{
    bool ret; // [esp+3h] [ebp-1h]

    Sys_LockWrite(&s_imageLoadLock);
    ret = Image_LoadFromFileWithReader(image, loadHighmip, FS_FOpenFileRead);
    Sys_UnlockWrite(&s_imageLoadLock);
    return ret;
}

char __cdecl Image_LoadFromFileWithReader(
                GfxImage *image,
                bool loadHighmip,
                int (__cdecl *OpenFileRead)(const char *, int *))
{
    // nx-port: come sopra. RB_Resource_Flush qui sotto blocca finche' il
    // thread renderer non esegue la callback, e durante una zona non lo fa.
    if ( Sys_CanCreateDeviceResourcesInline() )
        return _Image_LoadFromFileWithReader(image, loadHighmip, OpenFileRead);

    iassert(image->category == 3/*IMG_CATEGORY_LOAD_FROM_FILE*/);
    callbackParams.image = image;
    callbackParams.loadHighmip = loadHighmip;
    callbackParams.OpenFileRead = OpenFileRead;
    RB_Resource_Callback(readCallback);
    RB_Resource_Flush();
    return callbackParams.ret;
}

void __cdecl Image_Generate2D(GfxImage *image, unsigned __int8 *pixels, int width, int height, _D3DFORMAT imageFormat)
{
    if ( !pixels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 1898, 0, "%s", "pixels") )
    {
        __debugbreak();
    }
    if ( (width <= 0 || (width & (width - 1)) != 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1899,
                    0,
                    "%s\n\t(width) = %i",
                    "(width > 0 && (((width) & ((width) - 1)) == 0))",
                    width) )
    {
        __debugbreak();
    }
    if ( (height <= 0 || (height & (height - 1)) != 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1900,
                    0,
                    "%s\n\t(height) = %i",
                    "(height > 0 && (((height) & ((height) - 1)) == 0))",
                    height) )
    {
        __debugbreak();
    }
    Image_Setup(image, width, height, 1, 3, imageFormat);
    if ( image->cardMemory.platform[0] <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1905,
                    0,
                    "%s",
                    "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0") )
    {
        __debugbreak();
    }
    Image_UploadData(image, imageFormat, D3DCUBEMAP_FACE_POSITIVE_X, 0, pixels);
}

void __cdecl Image_GenerateCube(
                GfxImage *image,
                const unsigned __int8 *(*pixels)[15],
                int edgeLen,
                _D3DFORMAT imageFormat,
                unsigned int mipCount)
{
    _D3DCUBEMAP_FACES face; // [esp+0h] [ebp-10h]
    unsigned int mipIndex; // [esp+4h] [ebp-Ch]
    unsigned int faceIndex; // [esp+8h] [ebp-8h]
    unsigned __int8 imageFlags; // [esp+Fh] [ebp-1h]

    if ( !pixels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 1953, 0, "%s", "pixels") )
    {
        __debugbreak();
    }
    if ( edgeLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1954,
                    0,
                    "%s",
                    "edgeLen > 0") )
    {
        __debugbreak();
    }
    if ( (edgeLen & (edgeLen - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1955,
                    0,
                    "%s",
                    "IsPowerOf2( edgeLen )") )
    {
        __debugbreak();
    }
    if ( mipCount > 0xF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1956,
                    0,
                    "%s",
                    "mipCount <= 15") )
    {
        __debugbreak();
    }
    imageFlags = 5;
    if ( mipCount == 1 )
        imageFlags = 7;
    Image_Setup(image, edgeLen, edgeLen, 1, imageFlags, imageFormat);
    if ( image->cardMemory.platform[0] <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    1965,
                    0,
                    "%s",
                    "image->cardMemory.platform[PICMIP_PLATFORM_USED] > 0") )
    {
        __debugbreak();
    }
    for ( faceIndex = 0; faceIndex < 6; ++faceIndex )
    {
        face = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
        for ( mipIndex = 0; mipIndex < mipCount; ++mipIndex )
            Image_UploadData(image, imageFormat, face, mipIndex, (unsigned __int8 *)(&(*pixels)[15 * faceIndex])[mipIndex]);
    }
}

void __cdecl Image_LoadWhite(GfxImage *image)
{
    Image_LoadSolid(image, 0xFFu, 0xFFu, 0xFFu, 0xFFu);
}

void __cdecl Image_LoadSolid(
                GfxImage *image,
                unsigned __int8 r,
                unsigned __int8 g,
                unsigned __int8 b,
                unsigned __int8 a)
{
    unsigned __int8 pic[4]; // [esp+8h] [ebp-4h] BYREF

    *(unsigned int *)pic = (a << 24) | b | (g << 8) | (r << 16);
    Image_Generate2D(image, pic, 1, 1, D3DFMT_A8R8G8B8);
}

void __cdecl Image_LoadWhiteR32F(GfxImage *image)
{
    float pic; // [esp+0h] [ebp-4h] BYREF

    pic = FLT_MAX;
    Image_Generate2D(image, (unsigned __int8 *)&pic, 1, 1, D3DFMT_R32F);
}

void __cdecl Image_LoadG16R16(GfxImage *image)
{
    float pic; // [esp+0h] [ebp-4h] BYREF

    pic = 1.0f;
    Image_Generate2D(image, (unsigned __int8 *)&pic, 1, 1, D3DFMT_G16R16);
}

void __cdecl Image_LoadR5G6B5(GfxImage *image)
{
    unsigned __int16 pic; // [esp+0h] [ebp-8h] BYREF
    unsigned __int16 col; // [esp+4h] [ebp-4h]

    col = 8;
    pic = 16392;
    Image_Generate2D(image, (unsigned __int8 *)&pic, 1, 1, D3DFMT_R5G6B5);
}

void __cdecl Image_LoadBlack(GfxImage *image)
{
    Image_LoadSolid(image, 0, 0, 0, 0xFFu);
}

void __cdecl Image_LoadBlackTransparent(GfxImage *image)
{
    Image_LoadSolid(image, 0, 0, 0, 0);
}

void __cdecl Image_LoadGray(GfxImage *image)
{
    Image_LoadSolid(image, 0x80u, 0x80u, 0x80u, 0x80u);
}

void __cdecl Image_LoadIdentityNormalMap(GfxImage *image)
{
    Image_LoadSolid(image, 0x80u, 0x80u, 0xFFu, 0x80u);
}

void __cdecl Image_LoadWhiteTransparent(GfxImage *image)
{
    Image_LoadSolid(image, 0xFFu, 0xFFu, 0xFFu, 0);
}

void __cdecl Image_LoadPixelCostColorCode(GfxImage *image)
{
    unsigned __int8 pic[257][4]; // [esp+0h] [ebp-408h] BYREF

    RB_PixelCost_BuildColorCodeMap(pic, 256);
    Image_Generate2D(image, pic[0], 256, 1, D3DFMT_X8R8G8B8);
}

GfxImage *__cdecl Image_Load(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+4h] [ebp-4h]

    if ( *name == 36 )
        return Image_LoadBuiltin(name, semantic, imageTrack);
    image = Image_Alloc(name, 3u, semantic, imageTrack);
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 2200, 1, "%s", "image") )
    {
        __debugbreak();
    }
    if ( image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp",
                    2202,
                    1,
                    "%s",
                    "image->texture.basemap == NULL") )
    {
        __debugbreak();
    }
    if ( Image_LoadFromFile(image, 1) )
        return image;
    else
        return 0;
}

GfxImage *__cdecl Image_LoadBuiltin(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+18h] [ebp-8h]
    unsigned int tableIndex; // [esp+1Ch] [ebp-4h]

    for ( tableIndex = 0; ; ++tableIndex )
    {
        if ( tableIndex >= 0xC )
        {
            Com_PrintError(8, "ERROR: Unknown built-in image '%s'", name);
            return 0;
        }
        if ( !strcmp(constructorTable[tableIndex].name, name) )
            break;
    }
    image = Image_Alloc(name, 1u, semantic, imageTrack);
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 2181, 1, "%s", "image") )
    {
        __debugbreak();
    }
    constructorTable[tableIndex].LoadCallback(image);
    return image;
}

void __cdecl Image_BuildWaterMap(GfxImage *image)
{
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 2212, 0, "%s", "image") )
    {
        __debugbreak();
    }
    Image_SetupAndLoad(image, image->width, image->height, 1, 65537, D3DFMT_L8);
}

GfxImage *__cdecl R_CreateWaterMap(char *name, unsigned __int16 imageWidth, unsigned __int16 imageHeight)
{
    GfxImage *image; // [esp+4h] [ebp-4h]

    image = Image_Alloc(name, 5u, 0xBu, 9u);
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_obj.cpp", 2222, 1, "%s", "image") )
    {
        __debugbreak();
    }
    image->width = imageWidth;
    image->height = imageHeight;
    Image_BuildWaterMap(image);
    return image;
}

