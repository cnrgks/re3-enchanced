#include "common.h"
#include "Enhanced.h"
#include "EnhancedMath.h"
#include "Automobile.h"
#include "Collision.h"
#include "DMAudio.h"
#include "Glass.h"
#include "ModelIndices.h"
#include "ModelInfo.h"
#include "PedModelInfo.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "RwHelper.h"
#include "NodeName.h"
#include "World.h"

namespace Enhanced {
namespace {
struct GlassTrace {
    const CVector *source, *target;
    float best;
    RpAtomic *atomic;
    bool broken;
};
RwObject *TraceAtomic(RwObject *object, void *data)
{
    GlassTrace &trace=*(GlassTrace*)data;
    if (RwObjectGetType(object)!=rpATOMIC) return object;
    RpAtomic *atomic=(RpAtomic*)object;
    if (!trace.broken && !(RpAtomicGetFlags(atomic)&rpATOMICRENDER)) return object;
    RpGeometry *geometry=RpAtomicGetGeometry(atomic);
    if (!geometry) return object;
    RwV3d *vertices=RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry,0));
    RpTriangle *triangles=RpGeometryGetTriangles(geometry);
    if (!vertices || !triangles) return object;
    RwMatrix inverse;
    RwMatrixInvert(&inverse,RwFrameGetLTM(RpAtomicGetFrame(atomic)));
    CVector start,end;
    RwV3dTransformPoints(&start,trace.source,1,&inverse);
    RwV3dTransformPoints(&end,trace.target,1,&inverse);
    const int vertexCount=RpGeometryGetNumVertices(geometry);
    for (int i=0;i<RpGeometryGetNumTriangles(geometry);++i) {
#ifdef LIBRW
        const uint16 *indices=triangles[i].v;
#else
        const uint16 *indices=triangles[i].vertIndex;
#endif
        if (indices[0]>=vertexCount || indices[1]>=vertexCount || indices[2]>=vertexCount) continue;
        if (EnhancedMath::SegmentTriangle(start,end-start,vertices[indices[0]],
            vertices[indices[1]],vertices[indices[2]],trace.best)) trace.atomic=atomic;
    }
    return object;
}
RwFrame *TraceFrame(RwFrame *frame, void *data)
{
    RwFrameForAllObjects(frame,TraceAtomic,data);
    RwFrameForAllChildren(frame,TraceFrame,data);
    return frame;
}
RwFrame *FindNamedGlass(RwFrame *frame, void *data)
{
    const char *name=GetFrameNodeName(frame);
    if (name && strncmp(name,"windscreen",10)==0) TraceFrame(frame,data);
    else RwFrameForAllChildren(frame,FindNamedGlass,data);
    return frame;
}
void BreakGlass(CAutomobile *car, RpAtomic *glass, const CVector &impact)
{
    if (car->Damage.GetPanelStatus(VEHPANEL_WINDSCREEN)==PANEL_STATUS_MISSING) return;
    RpGeometry *geometry=RpAtomicGetGeometry(glass);
    RwV3d *vertices=RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry,0));
    const int count=RpGeometryGetNumVertices(geometry);
    if (count) {
        CVector lo=vertices[0],hi=vertices[0];
        for (int i=1;i<count;++i) {
            lo.x=Min(lo.x,vertices[i].x); lo.y=Min(lo.y,vertices[i].y); lo.z=Min(lo.z,vertices[i].z);
            hi.x=Max(hi.x,vertices[i].x); hi.y=Max(hi.y,vertices[i].y); hi.z=Max(hi.z,vertices[i].z);
        }
        CMatrix world(RwFrameGetLTM(RpAtomicGetFrame(glass)));
        const CVector right=Multiply3x3(world,CVector(hi.x-lo.x,0,0));
        const CVector up=Multiply3x3(world,CVector(0,hi.y-lo.y,hi.z-lo.z));
        if (right.Magnitude()>0.1f && up.Magnitude()>0.1f)
            CGlass::GeneratePanesForWindow(0,world*lo,up,right,car->m_vecMoveSpeed,impact,0.05f,false,false);
    }
    car->Damage.SetPanelStatus(VEHPANEL_WINDSCREEN,PANEL_STATUS_MISSING);
    if (car->m_aCarNodes[CAR_WINDSCREEN]) car->SetPanelDamage(CAR_WINDSCREEN,VEHPANEL_WINDSCREEN,true);
    else RpAtomicSetFlags(glass,0);
    DMAudio.PlayOneShot(car->m_audioEntityId,SOUND_CAR_WINDSHIELD_CRACK,0.0f);
}
}

bool ProcessWindscreenShot(const CVector &source, const CVector &target,
    CEntity *shooter, bool hit, CColPoint &point, CEntity *&entity, bool hitOccupants)
{
    if (!BreakableWindscreen) return hit;
    const CVector ray=target-source;
    const float lengthSq=ray.MagnitudeSqr();
    if (lengthSq<0.001f) return hit;
    const float oldHit=hit ? Clamp(DotProduct(point.point-source,ray)/lengthSq,0.0f,1.0f) : 1.0f;
    CAutomobile *nearest=nil;
    RpAtomic *glass=nil;
    float best=1.0f;
    for (int i=0;i<CPools::GetVehiclePool()->GetSize();++i) {
        CVehicle *vehicle=CPools::GetVehiclePool()->GetSlot(i);
        if (!vehicle || !vehicle->IsCar() || vehicle==shooter || !vehicle->m_rwObject ||
            vehicle->bBulletProof || !vehicle->bCanBeDamaged || vehicle->GetModelIndex()==MI_RHINO) continue;
        if (shooter && shooter->IsPed() && ((CPed*)shooter)->InVehicle() && ((CPed*)shooter)->m_pMyVehicle==vehicle) continue;
        const float t=Clamp(DotProduct(vehicle->GetPosition()-source,ray)/lengthSq,0.0f,1.0f);
        if ((vehicle->GetPosition()-(source+ray*t)).MagnitudeSqr()>sq(vehicle->GetBoundRadius()+1.0f)) continue;
        CAutomobile *car=(CAutomobile*)vehicle;
        GlassTrace trace={&source,&target,best,nil,car->Damage.GetPanelStatus(VEHPANEL_WINDSCREEN)==PANEL_STATUS_MISSING};
        if (car->m_aCarNodes[CAR_WINDSCREEN]) TraceFrame(car->m_aCarNodes[CAR_WINDSCREEN],&trace);
        else FindNamedGlass(RpClumpGetFrame(car->GetClump()),&trace);
        if (!trace.atomic) continue;
        // The same car's coarse proxy may lie ahead of the actual pane.
        // An intervening wall/other car, or a shot through the trunk, may not.
        if (hit && oldHit+0.0001f<trace.best && entity!=car) continue;
        if (hit && entity==car && oldHit<trace.best && DotProduct(ray,car->GetForward())>=0) continue;
        const CVector impact=source+ray*trace.best;
        if (!CWorld::GetIsLineOfSightClear(source,impact,true,false,false,true,true,true)) continue;
        best=trace.best; nearest=car; glass=trace.atomic;
    }
    if (!nearest) return hit;
    BreakGlass(nearest,glass,source+ray*best);
    if (!hitOccupants) return hit;
    // Seated peds have bUsesCollision=false and are skipped by normal world
    // traces. Return their animated hit model to native weapon damage handling.
    float closest=1.0f;
    CColPoint blocked;
    CEntity *blocking=nil;
    if (CWorld::ProcessLineOfSight(source,target,blocked,blocking,true,false,false,true,true,true))
        closest=Clamp(DotProduct(blocked.point-source,ray)/lengthSq,0.0f,1.0f);
    CEntity *occupant=nil;
    CColPoint occupantPoint;
    for (int seat=-1;seat<8;++seat) {
        CPed *ped=seat<0 ? nearest->pDriver : nearest->pPassengers[seat];
        if (!ped || ped==shooter || !ped->m_rwObject || ped->DyingOrDead()) continue;
        CPedModelInfo *info=(CPedModelInfo*)CModelInfo::GetModelInfo(ped->GetModelIndex());
        CColModel *model;
#ifdef PED_SKIN
        if (IsClumpSkinned(ped->GetClump())) model=info->AnimatePedColModelSkinned(ped->GetClump());
        else
#endif
#ifdef ANIMATE_PED_COL_MODEL
            model=CPedModelInfo::AnimatePedColModel(info->GetHitColModel(),RpClumpGetFrame(ped->GetClump()));
#else
            model=info->GetHitColModel();
#endif
        float distance=closest;
        CColPoint candidate;
        if (model && CCollision::ProcessLineOfSight(CColLine(source,target),ped->GetMatrix(),*model,candidate,distance,true) && distance>=best-0.0001f) {
            closest=distance; occupant=ped; occupantPoint=candidate;
        }
    }
    if (occupant) { entity=occupant; point=occupantPoint; return true; }
    return hit;
}
}
