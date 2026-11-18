#include "common.h"
#include "Enhanced.h"
#include "AnimBlendClumpData.h"
#include "Camera.h"
#include "ColPoint.h"
#include "CutsceneMgr.h"
#include "Ped.h"
#include "PedIK.h"
#include "PlayerPed.h"
#include "Replay.h"
#include "RwHelper.h"
#include "Timer.h"
#include "World.h"

namespace Enhanced {
namespace {
// A bounded, render-only articulated solver. The native ped remains the
// collision/mission entity, so this cannot leave a mission ped in a new state.
// This first version supports original non-skinned PC models; skinned models
// keep their original animation rather than receiving a broken skeleton pose.
const int kNodes = 10;
const int bone[kNodes] = {PED_MID,PED_HEAD,PED_UPPERARML,PED_HANDL,
    PED_UPPERARMR,PED_HANDR,PED_UPPERLEGL,PED_FOOTL,PED_UPPERLEGR,PED_FOOTR};
const int child[kNodes] = {1,-1,3,-1,5,-1,7,-1,9,-1};
const int edges[][2] = {{0,1},{0,2},{2,3},{0,4},{4,5},{0,6},{6,7},
    {0,8},{8,9},{2,4},{6,8},{2,8},{4,6},{2,6},{4,8}};
struct Body {
    CPed *ped;
    CVector position[kNodes], previous[kNodes];
    float length[ARRAY_SIZE(edges)];
    CVector anchor;
    uint32 time;
    RwMatrix saved[kNodes];
    bool applied;
};
Body bodies[32];

Body *Find(CPed *ped)
{
    for (unsigned i=0; i<ARRAY_SIZE(bodies); ++i)
        if (bodies[i].ped == ped) return &bodies[i];
    return nil;
}

bool Eligible(CPed *ped)
{
    const PedState state = ped->GetPedState();
    return Ragdoll && ped->m_rwObject && !ped->InVehicle() &&
        !CReplay::IsPlayingBack() && !CCutsceneMgr::IsRunning() &&
        (state == PED_DIE || state == PED_DEAD || state == PED_FALL) &&
        (ped->GetPosition()-TheCamera.GetPosition()).MagnitudeSqr() < 3600.0f;
}

void Simulate(Body &b, CPed *ped)
{
    const uint32 now = CTimer::GetTimeInMilliseconds();
    if (now == b.time || CTimer::GetIsPaused()) return;
    float dt = Min((now-b.time)*0.001f, 0.05f);
    b.time = now;
    const CVector anchor = ped->GetNodePosition(PED_MID);
    CVector shift = anchor-b.anchor;
    if (shift.MagnitudeSqr() > 9.0f) { b.ped = nil; return; }
    for (int i=0;i<kNodes;++i) {
        b.position[i] += shift;
        b.previous[i] += shift;
    }
    b.anchor = anchor;
    for (int step=0;step<2;++step) {
        const float h = dt*0.5f;
        for (int i=1;i<kNodes;++i) {
            const CVector old = b.position[i];
            CVector velocity = (b.position[i]-b.previous[i])*0.97f;
            // Bound impulses from sudden animation/state changes.
            if (velocity.MagnitudeSqr()>0.16f) { velocity.Normalise(); velocity *= 0.4f; }
            b.position[i] += velocity+CVector(0.0f,0.0f,-17.0f*h*h);
            b.previous[i] = old;
        }
        for (int iteration=0;iteration<6;++iteration) {
            b.position[0] = anchor;
            for (unsigned e=0;e<ARRAY_SIZE(edges);++e) {
                int a=edges[e][0], c=edges[e][1];
                CVector delta=b.position[c]-b.position[a];
                float distance=delta.Magnitude();
                if (distance<0.0001f) continue;
                CVector correction=delta*((distance-b.length[e])/distance);
                if (a==0) b.position[c]-=correction;
                else { b.position[a]+=correction*0.5f; b.position[c]-=correction*0.5f; }
            }
        }
        for (int i=1;i<kNodes;++i) {
            CColPoint hit;
            CEntity *entity=nil;
            CVector from=b.previous[i]+CVector(0.0f,0.0f,0.2f);
            CVector to=b.position[i]-CVector(0.0f,0.0f,0.08f);
            if (CWorld::ProcessLineOfSight(from,to,hit,entity,true,false,false,true,true,false)) {
                b.position[i]=hit.point+hit.normal*0.08f;
                b.previous[i]=b.position[i];
            }
        }
    }
}
}

void ForgetPed(CPed *ped)
{
    if (Body *body=Find(ped)) { body->ped=nil; body->applied=false; }
}

void BeginPedRender(CPed *ped)
{
    if (!Eligible(ped)) { ForgetPed(ped); return; }
#ifdef PED_SKIN
    if (IsClumpSkinned(ped->GetClump())) return;
#endif
    for (int i=0;i<kNodes;++i)
        if (!ped->m_pFrames[bone[i]] || !ped->m_pFrames[bone[i]]->frame) return;
    Body *b=Find(ped);
    if (!b) {
        for (unsigned i=0;i<ARRAY_SIZE(bodies);++i)
            if (!bodies[i].ped || CTimer::GetTimeInMilliseconds()-bodies[i].time > 5000) { b=&bodies[i]; break; }
        if (!b) return;
        b->ped=ped;
        b->applied=false;
        b->time=CTimer::GetTimeInMilliseconds();
        b->anchor=ped->GetNodePosition(PED_MID);
        for (int i=0;i<kNodes;++i) {
            b->position[i]=ped->GetNodePosition(bone[i]);
            b->previous[i]=b->position[i]-ped->m_vecMoveSpeed*0.25f;
        }
        for (unsigned e=0;e<ARRAY_SIZE(edges);++e)
            b->length[e]=(b->position[edges[e][0]]-b->position[edges[e][1]]).Magnitude();
    }
    Simulate(*b,ped);
    if (!b->ped) return;
    RwMatrix original[kNodes];
    for (int i=0;i<kNodes;++i) {
        RwFrame *frame=ped->m_pFrames[bone[i]]->frame;
        b->saved[i]=*RwFrameGetMatrix(frame);
        CPedIK::GetWorldMatrix(frame,&original[i]);
    }
    for (int i=0;i<kNodes;++i) {
        RwFrame *frame=ped->m_pFrames[bone[i]]->frame;
        RwMatrix desired=original[i];
        if (child[i]>=0) {
            CVector from=CVector(original[child[i]].pos)-CVector(original[i].pos);
            CVector to=b->position[child[i]]-b->position[i];
            if (from.MagnitudeSqr()>0.0001f && to.MagnitudeSqr()>0.0001f) {
                from.Normalise(); to.Normalise();
                CVector axis=CrossProduct(from,to);
                float sine=axis.Magnitude();
                float cosine=Clamp(DotProduct(from,to),-1.0f,1.0f);
                if (sine>0.0001f) {
                    axis*=1.0f/sine;
                    RwMatrixRotate(&desired,&axis,RADTODEG(Atan2(sine,cosine)),rwCOMBINEPOSTCONCAT);
                }
            }
        }
        desired.pos=b->position[i];
        RwMatrix parent,inverse;
        CPedIK::GetWorldMatrix(RwFrameGetParent(frame),&parent);
        RwMatrixInvert(&inverse,&parent);
        RwMatrixMultiply(RwFrameGetMatrix(frame),&desired,&inverse);
        RwMatrixUpdate(RwFrameGetMatrix(frame));
        RwFrameUpdateObjects(frame);
    }
    b->applied=true;
}

void EndPedRender(CPed *ped)
{
    Body *b=Find(ped);
    if (!b || !b->applied) return;
    // Never let visual physics leak into animation, IK, save data or collisions.
    for (int i=0;i<kNodes;++i) {
        RwFrame *frame=ped->m_pFrames[bone[i]]->frame;
        *RwFrameGetMatrix(frame)=b->saved[i];
        RwFrameUpdateObjects(frame);
    }
    b->applied=false;
}
}
