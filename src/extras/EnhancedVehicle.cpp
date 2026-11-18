#include "common.h"
#include "Enhanced.h"
#include "EnhancedMath.h"
#include "Automobile.h"
#include "DMAudio.h"
#include "Glass.h"
#include "ModelIndices.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "RwHelper.h"

namespace Enhanced {
namespace {
RpAtomic *FindVisibleGlass;
RwObject *FindGlassAtomic(RwObject *object, void *)
{
    if (RwObjectGetType(object)==rpATOMIC &&
        (RpAtomicGetFlags((RpAtomic*)object)&rpATOMICRENDER)) FindVisibleGlass=(RpAtomic*)object;
    return object;
}

}

void ProcessWindscreenShot(const CVector &source, const CVector &target, CEntity *shooter)
{
    if (!BreakableWindscreen) return;
    const CVector ray=target-source;
    const float rayLengthSq=ray.MagnitudeSqr();
    if (rayLengthSq<0.001f) return;
    CAutomobile *nearest=nil;
    RpAtomic *glass=nil;
    float best=1.0f;
    // Only actual weapon traces call this. Existing world hits limit the
    // segment, so panes behind walls/other impacts cannot break.
    for (int i=0;i<CPools::GetVehiclePool()->GetSize();++i) {
        CVehicle *vehicle=CPools::GetVehiclePool()->GetSlot(i);
        if (!vehicle || !vehicle->IsCar() || vehicle==shooter ||
            vehicle->bBulletProof || !vehicle->bCanBeDamaged ||
            vehicle->GetModelIndex()==MI_RHINO) continue;
        if (shooter && shooter->IsPed() && ((CPed*)shooter)->m_pMyVehicle==vehicle && ((CPed*)shooter)->InVehicle()) continue;
        CAutomobile *car=(CAutomobile*)vehicle;
        if (!car->m_aCarNodes[CAR_WINDSCREEN] || !car->m_rwObject ||
            car->Damage.GetPanelStatus(VEHPANEL_WINDSCREEN)==PANEL_STATUS_MISSING) continue;
        const float t=Clamp(DotProduct(car->GetPosition()-source,ray)/rayLengthSq,0.0f,1.0f);
        if ((car->GetPosition()-(source+ray*t)).MagnitudeSqr()>sq(car->GetBoundRadius()+1.0f)) continue;
        FindVisibleGlass=nil;
        RwFrameForAllObjects(car->m_aCarNodes[CAR_WINDSCREEN],FindGlassAtomic,nil);
        if (!FindVisibleGlass) continue;
        RpGeometry *geometry=RpAtomicGetGeometry(FindVisibleGlass);
        RpMorphTarget *morph=RpGeometryGetMorphTarget(geometry,0);
        RwV3d *vertices=RpMorphTargetGetVertices(morph);
        RpTriangle *triangles=RpGeometryGetTriangles(geometry);
        if (!vertices || !triangles) continue;
        RwMatrix inverse;
        RwMatrixInvert(&inverse,RwFrameGetLTM(car->m_aCarNodes[CAR_WINDSCREEN]));
        CVector start,end;
        RwV3dTransformPoints(&start,&source,1,&inverse);
        RwV3dTransformPoints(&end,&target,1,&inverse);
        for (int tri=0;tri<RpGeometryGetNumTriangles(geometry);++tri) {
            const RpTriangle &triangle=triangles[tri];
#ifdef LIBRW
            const uint16 *indices=triangle.v;
#else
            const uint16 *indices=triangle.vertIndex;
#endif
            const int vertexCount=RpGeometryGetNumVertices(geometry);
            if (indices[0]>=vertexCount || indices[1]>=vertexCount || indices[2]>=vertexCount) continue;
            if (EnhancedMath::SegmentTriangle(start,end-start,vertices[indices[0]],
                vertices[indices[1]],vertices[indices[2]],best)) {
                nearest=car;
                glass=FindVisibleGlass;
            }
        }
    }
    if (!nearest || !glass) return;
    RpGeometry *geometry=RpAtomicGetGeometry(glass);
    RwV3d *vertices=RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry,0));
    int count=RpGeometryGetNumVertices(geometry);
    if (count>0) {
        CVector lo=vertices[0],hi=vertices[0];
        for (int i=1;i<count;++i) {
            lo.x=Min(lo.x,vertices[i].x); lo.y=Min(lo.y,vertices[i].y); lo.z=Min(lo.z,vertices[i].z);
            hi.x=Max(hi.x,vertices[i].x); hi.y=Max(hi.y,vertices[i].y); hi.z=Max(hi.z,vertices[i].z);
        }
        CMatrix world(RwFrameGetLTM(nearest->m_aCarNodes[CAR_WINDSCREEN]));
        // Typical GTA III windscreen geometry spans x horizontally and z/y
        // along the inclined pane. Native glass handles particle budgets.
        CVector right=Multiply3x3(world,CVector(hi.x-lo.x,0.0f,0.0f));
        CVector up=Multiply3x3(world,CVector(0.0f,hi.y-lo.y,hi.z-lo.z));
        if (right.Magnitude()>0.1f && up.Magnitude()>0.1f)
            CGlass::GeneratePanesForWindow(0,world*lo,up,right,nearest->m_vecMoveSpeed,
                source+ray*best,0.05f,false,false);
    }
    nearest->Damage.SetPanelStatus(VEHPANEL_WINDSCREEN,PANEL_STATUS_MISSING);
    nearest->SetPanelDamage(CAR_WINDSCREEN,VEHPANEL_WINDSCREEN,true);
    DMAudio.PlayOneShot(nearest->m_audioEntityId,SOUND_CAR_WINDSHIELD_CRACK,0.0f);
}
}
