#include "r_workercmds.h"
#include <universal/com_workercmds.h>
#include "r_dpvs_entity.h"
#include "r_model_skin.h"
#include "r_rendercmds.h"
#include "r_dpvs_static.h"
#include "r_dpvs_dynmodel.h"
#include "r_dpvs_sceneent.h"
#include "r_water.h"
#include "r_water_sim.h"
#include "r_spotshadow.h"
#include "r_workercmds_common.h"
#include <EffectsCore/fx_update.h>
#include <qcommon/threads.h>
#include <EffectsCore/fx_draw.h>
#include "r_dobj_skin.h"
#include "r_model_pose.h"
#include "r_model_lighting.h"
#include <EffectsCore/fx_marks.h>
#include "r_staticmodelcache.h"

// probably decl'd with a macro here, but ya'know

volatile unsigned int fx_update_spotLimit = 1;
jqModule fx_update_spotModule =
{
  .Name = "fx_update_spot",
  .Type = JQ_WORKER_GENERIC,
  .Code = fx_update_spotCallback,
};
jqWorkerCmd fx_update_spotWorkerCmd = { &fx_update_spotModule, sizeof(FxCmd),   /* nx-port: was an x86 literal */ 0, 0, &fx_update_spotLimit, NULL, 0u };


volatile unsigned int fx_updateLimit = 1;
jqModule fx_updateModule =
{
    .Name = "fx_update",
    .Type = JQ_WORKER_GENERIC,
    .Code = fx_updateCallback,
};
jqWorkerCmd fx_updateWorkerCmd = { &fx_updateModule, sizeof(FxCmd),   /* nx-port: was an x86 literal */ 0, 0, &fx_updateLimit, NULL, 0u };


volatile unsigned int fx_update_remainingLimit = 1;
jqModule fx_update_remainingModule =
{
    .Name = "fx_update_remaining",
    .Type = JQ_WORKER_GENERIC, 
    .Code = fx_update_remainingCallback, 
};
jqWorkerCmd fx_update_remainingWorkerCmd = { &fx_update_remainingModule, sizeof(FxCmd),   /* nx-port: was an x86 literal */ 0, 0, &fx_update_remainingLimit, NULL, 0u };


volatile unsigned int r_dpvs_staticLimit = 1;
jqModule r_dpvs_staticModule =
{
    .Name = "r_dpvs_static",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_dpvs_staticCallback,
};
jqWorkerCmd r_dpvs_staticWorkerCmd = { &r_dpvs_staticModule, sizeof(DpvsStaticCellCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_dpvs_staticLimit, NULL, 0u };


volatile unsigned int r_dpvs_sceneentLimit = 1;
jqModule r_dpvs_sceneentModule =
{
    .Name = "r_dpvs_sceneent",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_dpvs_sceneentCallback,
};
jqWorkerCmd r_dpvs_sceneentWorkerCmd = { &r_dpvs_sceneentModule, sizeof(DpvsDynamicCellCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_dpvs_sceneentLimit, NULL, 0u };


volatile unsigned int r_dpvs_dynmodelLimit = 1;
jqModule r_dpvs_dynmodelModule =
{
    .Name = "r_dpvs_dynmodel",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_dpvs_dynmodelCallback,
};
jqWorkerCmd r_dpvs_dynmodelWorkerCmd = { &r_dpvs_dynmodelModule, sizeof(DpvsDynamicCellCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_dpvs_dynmodelLimit, NULL, 0u };



volatile unsigned int r_dpvs_dynbrushLimit = 1;
jqModule r_dpvs_dynbrushModule =
{
    .Name = "r_dpvs_dynbrush",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_dpvs_dynbrushCallback,
};
jqWorkerCmd r_dpvs_dynbrushWorkerCmd = { &r_dpvs_dynbrushModule, sizeof(DpvsDynamicCellCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_dpvs_dynbrushLimit, NULL, 0u };



volatile unsigned int r_dpvs_entityLimit = 1;
jqModule r_dpvs_entityModule =
{
    .Name = "r_dpvs_entity",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_dpvs_entityCallback,
};
jqWorkerCmd r_dpvs_entityWorkerCmd = { &r_dpvs_entityModule, sizeof(DpvsEntityCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_dpvs_entityLimit, NULL, 0u };


volatile unsigned int r_add_sceneentLimit = 1;
jqModule r_add_sceneentModule =
{
    .Name = "r_add_sceneent",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_add_sceneentCallback,
};
jqWorkerCmd r_add_sceneentWorkerCmd = { &r_add_sceneentModule, sizeof(SceneEntCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_add_sceneentLimit, NULL, 0u };


volatile unsigned int r_spot_shadow_entLimit = 1;
jqModule r_spot_shadow_entModule =
{
    .Name = "r_spot_shadow_ent",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_spot_shadow_entCallback,
};
jqWorkerCmd r_spot_shadow_entWorkerCmd = { &r_spot_shadow_entModule, sizeof(GfxSpotShadowEntCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_spot_shadow_entLimit, NULL, 0u };

volatile unsigned int dobj_skelLimit = 1;
jqModule dobj_skelModule =
{
    .Name = "dobj_skel",
    .Type = JQ_WORKER_GENERIC,
    .Code = dobj_skelCallback,
    //.Group = 0;
};
jqWorkerCmd dobj_skelWorkerCmd = { &dobj_skelModule, sizeof(GfxSceneEntity *),   /* nx-port: was an x86 literal */ 0, 0, &dobj_skelLimit, NULL, 0u };


volatile unsigned int dobj_skinLimit = 1;
jqModule dobj_skinModule =
{
    .Name = "dobj_skin",
    .Type = JQ_WORKER_GENERIC,
    .Code = dobj_skinCallback,
    //.Group = 0,
};
jqWorkerCmd dobj_skinWorkerCmd = { &dobj_skinModule, sizeof(GfxSceneEntity *),   /* nx-port: was an x86 literal */ 0, 0, &dobj_skinLimit, NULL, 0u };

volatile unsigned int fx_drawLimit = 1;
jqModule fx_drawModule =
{
    .Name = "fx_draw",
    .Type = JQ_WORKER_GENERIC,
    .Code = fx_drawCallback
};
jqWorkerCmd fx_drawWorkerCmd = { &fx_drawModule, sizeof(FxGenerateVertsCmd), 0, 0, &fx_drawLimit, NULL, 0u};

volatile unsigned int fx_marks_drawLimit = 1;
jqModule fx_marks_drawModule =
{
    .Name = "fx_marks_draw",
    .Type = JQ_WORKER_GENERIC,
    .Code = fx_marks_drawCallback
};
jqWorkerCmd fx_marks_drawWorkerCmd = { &fx_marks_drawModule, sizeof(FxCmd),   /* nx-port: was an x86 literal */ 0, 0, &fx_marks_drawLimit, NULL, 0u };


volatile unsigned int r_model_skinLimit;
jqModule r_model_skinModule =
{
    .Name = "r_model_skin",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_model_skinCallback,
};
jqWorkerCmd r_model_skinWorkerCmd = { &r_model_skinModule, sizeof(SkinXModelCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_model_skinLimit, NULL, 0u };

volatile unsigned int r_skin_cached_staticmodelLimit = 1;
jqModule r_skin_cached_staticmodelModule =
{
    .Name = "r_skin_cached_staticmodel",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_skin_cached_staticmodelCallback,
};
jqWorkerCmd r_skin_cached_staticmodelWorkerCmd =
{
  &r_skin_cached_staticmodelModule,
  sizeof(SkinCachedStaticModelCmd),   /* nx-port: was an x86 literal */
  0,
  0,
  &r_skin_cached_staticmodelLimit,
  NULL,
  0u
};

volatile unsigned int r_water_simLimit;
jqModule r_water_simModule =
{
    .Name = "r_water_sim",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_water_simCallback,
};
jqWorkerCmd r_water_simWorkerCmd = { &r_water_simModule, sizeof(WaterSimulationCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_water_simLimit, NULL, 0u };


volatile unsigned int fx_update_remaining_ppuLimit = 1;
jqModule fx_update_remaining_ppuModule =
{
    .Name = "fx_update_remaining_ppu",
    .Type = JQ_WORKER_GENERIC,
    .Code = fx_update_remaining_ppuCallback,
};
jqWorkerCmd fx_update_remaining_ppuWorkerCmd =
{
  &fx_update_remaining_ppuModule,
  sizeof(FxCmd),   /* nx-port: was an x86 literal */
  0,
  0,
  &fx_update_remaining_ppuLimit,
  NULL,
  0u
};

volatile unsigned int r_model_lightingLimit = 1;
jqModule r_model_lightingModule =
{
    .Name = "r_model_lighting",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_model_lightingCallback,
};
jqWorkerCmd r_model_lightingWorkerCmd =
{ &r_model_lightingModule, sizeof(CalcLightingCmd),   /* nx-port: was an x86 literal */ 0, 0, &r_model_lightingLimit, NULL, 0u };


int __cdecl r_dpvs_entityCallback(jqBatch *batch)
{
    unsigned __int16 *data; // [esp+10h] [ebp-4h]

    PROF_SCOPED("r_dpvs_entity");
    data = (unsigned __int16 *)jqLockData(batch);
    R_AddEntitySurfacesInFrustumCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_model_skinCallback(jqBatch *batch)
{
    SkinXModelCmd *data = (SkinXModelCmd *)jqLockData(batch);
    if ( R_DynamicFenceBusy() )
        return 1;

    *frontEndDataOut->dynamicBufferCurrentFrame = frontEndDataOut->frameCount;

    PROF_SCOPED("r_model_skin");
    R_SkinXModelCmd(data);
    jqUnlockData(batch);

    return 0;
}

bool __cdecl R_DynamicFenceBusy()
{
    return R_DynamicFencePending();
}

int __cdecl r_dpvs_staticCallback(jqBatch *batch)
{
    PROF_SCOPED("r_dpvs_static");

    DpvsStaticCellCmd *data = (DpvsStaticCellCmd *)jqLockData(batch);
    R_AddCellStaticSurfacesInFrustumCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_dpvs_dynmodelCallback(jqBatch *batch)
{
    PROF_SCOPED("r_dpvs_dynmodel");

    const DpvsPlane **data = (const DpvsPlane **)jqLockData(batch);
    R_AddCellDynModelSurfacesInFrustumCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_dpvs_sceneentCallback(jqBatch *batch)
{
    GfxWorldDpvsPlanes *data; // [esp+10h] [ebp-4h]

    PROF_SCOPED("r_dpvs_sceneent");
    data = (GfxWorldDpvsPlanes *)jqLockData(batch);
    R_AddCellSceneEntSurfacesInFrustumCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_dpvs_dynbrushCallback(jqBatch *batch)
{
    PROF_SCOPED("r_dpvs_dynbrush");
    DpvsPlane **data = (DpvsPlane **)jqLockData(batch);
    R_AddCellDynBrushSurfacesInFrustumCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_waterCallback(jqBatch *batch)
{
    water_t **data = (water_t **)jqLockData(batch);
    if ( R_UploadWaterPending() )
        return 1;

    PROF_SCOPED("r_water");
    R_UploadWaterTextureInternal(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl R_UploadWaterPending()
{
    if ( !_InterlockedExchangeAdd(&g_waterLock, 1u) )
        return 0;
    R_ExecWaterFailed();
    return 1;
}

signed __int32 R_ExecWaterFailed()
{
    return _InterlockedExchangeAdd(&g_waterLock, 0xFFFFFFFF);
}

int __cdecl r_water_simCallback(jqBatch *batch)
{
    const WaterSimulationCmd *data; // [esp+10h] [ebp-4h]

    data = (const WaterSimulationCmd *)jqLockData(batch);
    if ( R_EndFenceBusy() )
        return 1;
    PROF_SCOPED("r_water_sim");
    R_WaterSimulationCmd(data);
    jqUnlockData(batch);
    return 0;
}

bool __cdecl R_EndFenceBusy()
{
    return R_EndFencePending();
}

int __cdecl r_spot_shadow_entCallback(jqBatch *batch)
{
    PROF_SCOPED("r_spot_shadow_ent");
    const GfxSpotShadowEntCmd *data = (const GfxSpotShadowEntCmd *)jqLockData(batch);
    R_AddSpotShadowEntCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl fx_updateCallback(jqBatch *batch)
{
    PROF_SCOPED("fx_update");
    FxCmd *data = (FxCmd *)jqLockData(batch);
    R_ProcessCmd_UpdateFxNonDependent(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl fx_update_remainingCallback(jqBatch *batch)
{
    char *Name; // [esp+8h] [ebp-10h]
    FxCmd *data; // [esp+14h] [ebp-4h]

    data = (FxCmd *)jqLockData(batch);
    if ( R_FXUpdateRemainingWait() )
        return 1;
    Name = va("fx_update_remaining cl=%d", data->localClientNum);

    PROF_SCOPED_RUNTIME_NAME(Name);

    FX_UpdateRemaining(data);
    jqUnlockData(batch);
    return 0;
}

bool __cdecl R_FXUpdateRemainingWait()
{
    return R_FXSpotLightPending();
}

bool __cdecl R_FXSpotLightPending()
{
    return jqPoll(&fx_update_spotModule.Group);
}

int __cdecl fx_update_remaining_ppuCallback(jqBatch *batch)
{
    char *Name; // [esp+8h] [ebp-10h]
    FxCmd *data; // [esp+14h] [ebp-4h]

    data = (FxCmd *)jqLockData(batch);
    if ( R_FXUpdateRemainingPPUWait() )
        return 1;
    Name = va("fx_update_remaining_ppu cl=%d", data->localClientNum);

    PROF_SCOPED_RUNTIME_NAME(Name);
    R_ProcessCmd_UpdateFxRemaining(data);
    jqUnlockData(batch);
    return 0;
}

bool __cdecl R_FXUpdateRemainingPPUWait()
{
    return R_FXDependentPending();
}

bool __cdecl R_FXDependentPending()
{
    return jqPoll(&fx_update_remainingModule.Group);
}

int __cdecl fx_drawCallback(jqBatch *batch)
{
    FxGenerateVertsCmd *data = (FxGenerateVertsCmd *)jqLockData(batch);

    if ( *frontEndDataOut->dynamicBufferCurrentFrame
        && *frontEndDataOut->dynamicBufferCurrentFrame < frontEndDataOut->frameCount )
    {
        return 1;
    }
    *frontEndDataOut->dynamicBufferCurrentFrame = frontEndDataOut->frameCount;
    PROF_SCOPED("fx_draw");
    if (Sys_QueryD3DDeviceOKEvent())
    {
        FX_GenerateVerts(data);
    }
    jqUnlockData(batch);
    return 0;
}

int __cdecl dobj_skelCallback(jqBatch *batch)
{
    PROF_SCOPED("dobj_skel"); 
    GfxSceneEntity **data = (GfxSceneEntity **)jqLockData(batch);
    R_UpdateGfxEntityBoundsCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl dobj_skinCallback(jqBatch *batch)
{
    PROF_SCOPED("dobj_skin");
    GfxSceneEntity **data = (GfxSceneEntity **)jqLockData(batch);
    R_SkinGfxEntityCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_model_lightingCallback(jqBatch *batch)
{
    CalcLightingCmd *data; // [esp+8h] [ebp-4h]

    data = (CalcLightingCmd *)jqLockData(batch);
    PROF_SCOPED("r_model_lighting");
    R_CalcModelLightingCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_add_sceneentCallback(jqBatch *batch)
{
    const GfxViewInfo **data = (const GfxViewInfo **)jqLockData(batch);

    PROF_SCOPED("r_add_sceneent");
    R_AddAllSceneEntSurfacesCamera(*data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl fx_marks_drawCallback(jqBatch *batch)
{
    FxCmd *data; // [esp+10h] [ebp-4h]

    data = (FxCmd *)jqLockData(batch);
    if (*frontEndDataOut->dynamicBufferCurrentFrame
        && *frontEndDataOut->dynamicBufferCurrentFrame < frontEndDataOut->frameCount)
    {
        return 1;
    }
    *frontEndDataOut->dynamicBufferCurrentFrame = frontEndDataOut->frameCount;
    if (Sys_QueryD3DDeviceOKEvent())
    {
        PROF_SCOPED("fx_marks_draw");
        FX_GenerateMarkVertsForWorld(data->localClientNum, data->visibleLights, data->visibleLightCount);
    }
    jqUnlockData(batch);
    return 0;
}

int __cdecl fx_update_spotCallback(jqBatch *batch)
{
    FxCmd *data = (FxCmd*)jqLockData(batch);

    PROF_SCOPED("fx_update_spot");
    R_ProcessCmd_UpdateFxSpotLight(data);

    jqUnlockData(batch);
    return 0;
}

int __cdecl r_skin_cached_staticmodelCallback(jqBatch *batch)
{
    SkinCachedStaticModelCmd *data = (SkinCachedStaticModelCmd*)jqLockData(batch);

    if ( *frontEndDataOut->dynamicBufferCurrentFrame
        && *frontEndDataOut->dynamicBufferCurrentFrame < frontEndDataOut->frameCount )
    {
        return 1;
    }
    *frontEndDataOut->dynamicBufferCurrentFrame = frontEndDataOut->frameCount;
    R_SkinCachedStaticModelCmd(data);
    jqUnlockData(batch);
    return 0;
}

