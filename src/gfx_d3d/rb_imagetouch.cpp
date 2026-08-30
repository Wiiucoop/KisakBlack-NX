#include "rb_imagetouch.h"
#include "r_init.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "rb_shade.h"
#include "r_image.h"
#include "rb_draw3d.h"
#include "r_state_utils.h"

void __cdecl RB_TouchAllImages()
{
  const char *v0; // eax
  const char *v1; // eax
  int v3; // [esp+4h] [ebp-4224h]
  int hr; // [esp+Ch] [ebp-421Ch]
  unsigned int i; // [esp+14h] [ebp-4214h]
  int v9; // [esp+1Ch] [ebp-420Ch]
  ImageList imageList; // [esp+20h] [ebp-4208h] BYREF

  if ( !dx.inScene )
  {
    if ( r_logFile && r_logFile->current.integer )
      RB_LogPrint("dx.device->BeginScene()\n");
    hr = dx.device->BeginScene();
    if ( hr < 0 )
    {
      ++g_disableRendering;
      v0 = R_ErrorDescription(hr);
      Com_Error(
        ERR_FATAL,
        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagetouch.cpp (%i) dx.device->BeginScene() failed: %s\n",
        62,
        v0);
    }
  }
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_GetImageList(&imageList);
  qsort(imageList.image, imageList.count, sizeof((imageList.image)[0]), (_CoreCrtNonSecureSearchSortCompareFunction)RB_CompareTouchImages);
  v9 = 0;
  for ( i = 0; i < imageList.count && imageList.image[i]->semantic; ++i )
  {
    RB_TouchImage(imageList.image[i]);
    v9 += imageList.image[i]->cardMemory.platform[0];
  }
  R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, 0);
  if ( !dx.inScene )
  {
    if ( r_logFile && r_logFile->current.integer )
      RB_LogPrint("dx.device->EndScene()\n");
    //v3 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *))dx.device->EndScene)(dx.device, dx.device);
    v3 = dx.device->EndScene();
    if ( v3 < 0 )
    {
      ++g_disableRendering;
      v1 = R_ErrorDescription(v3);
      Com_Error(
        ERR_FATAL,
        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_imagetouch.cpp (%i) dx.device->EndScene() failed: %s\n",
        79,
        v1);
    }
  }
}

void __cdecl RB_TouchImage(GfxImage *image)
{
  if ( image->mapType == 3 )
  {
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, image);
    R_Set2D(&gfxCmdBufSourceState);
    RB_DrawStretchPic(
      rgp.feedbackReplaceMaterial,
      0.0,
      0.0,
      8.0,
      8.0,
      0.0,
      0.0,
      1.0,
      1.0,
      0xFFFFFFFF,
      GFX_PRIM_STATS_CODE);
    RB_EndTessSurface();
  }
}

int __cdecl RB_CompareTouchImages(int *e0, int *e1)
{
    int image; // [esp+0h] [ebp-Ch]
    int image_4; // [esp+4h] [ebp-8h]

    image = *e0;
    image_4 = *e1;
    if ( !*(_BYTE *)(*e1 + 5) )
        return -1;
    if ( !*(_BYTE *)(image + 5) )
        return 1;
    if ( *(unsigned int *)(image_4 + 12) != *(unsigned int *)(image + 12) )
        return *(unsigned int *)(image_4 + 12) - *(unsigned int *)(image + 12);
    if ( *(unsigned __int8 *)(image + 5) == *(unsigned __int8 *)(image_4 + 5) )
        return 0;
    return *(unsigned __int8 *)(image + 5) - *(unsigned __int8 *)(image_4 + 5);
}

