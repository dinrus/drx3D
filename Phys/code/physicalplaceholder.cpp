// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Phys/StdAfx.h>

#include <drx3D/Phys/bvtree.h>
#include <drx3D/Phys/geometry.h>
#include <drx3D/Phys/geoman.h>
#include <drx3D/Phys/rigidbody.h>
#include <drx3D/Phys/physicalplaceholder.h>
#include <drx3D/Phys/physicalentity.h>
#include <drx3D/Phys/physicalworld.h>


IPhysicalWorld *CPhysicalPlaceholder::GetWorld() const
{
	CPhysicalWorld *pWorld = NULL;
	if (g_nPhysWorlds==1)
		pWorld = g_pPhysWorlds[0];
	else
		for(i32 i=0;i<g_nPhysWorlds && !(pWorld=g_pPhysWorlds[i])->IsPlaceholder(this);i++);
	return pWorld;
}


CPhysicalEntity *CPhysicalPlaceholder::GetEntity()
{
	CPhysicalEntity *pEntBuddy;
	if (!m_pEntBuddy) {
		CPhysicalWorld *pWorld = NULL;
		if (g_nPhysWorlds==1)
			pWorld = g_pPhysWorlds[0];
		else 
			for(i32 i=0;i<g_nPhysWorlds && !(pWorld=g_pPhysWorlds[i])->IsPlaceholder(this);i++);
		PREFAST_ASSUME(pWorld);
		if (pWorld->m_pPhysicsStreamer) {
			pWorld->m_pPhysicsStreamer->CreatePhysicalEntity(m_pForeignData,m_iForeignData,m_iForeignFlags);
			pEntBuddy = m_pEntBuddy ? (CPhysicalEntity*)m_pEntBuddy : &g_StaticPhysicalEntity;
		} else
			return 0;
	}	else
		pEntBuddy = (CPhysicalEntity*)m_pEntBuddy;
	pEntBuddy->m_timeIdle = 0;
	return pEntBuddy;
}


pe_type CPhysicalPlaceholder::GetType() const
{
	switch (m_iSimClass) {
		case 0: return PE_STATIC;
		case 1:case 2: return PE_RIGID;
		case 3: return PE_LIVING;
		case 4: return PE_PARTICLE;
		default: return PE_NONE;
	}
}

inline i32 IsAreaTrigger(const CPhysicalPlaceholder *ppc) { return iszero(ppc->m_iSimClass-6) & -(ppc->m_iForeignFlags & ent_areas)>>31; }

i32 CPhysicalPlaceholder::SetParams(pe_params *_params, i32 bThreadSafe)
{
	if (_params->type==pe_params_bbox::type_id) {
		pe_params_bbox *params = (pe_params_bbox*)_params;
		if (!is_unused(params->BBox[0])) m_BBox[0] = params->BBox[0];
		if (!is_unused(params->BBox[1])) m_BBox[1] = params->BBox[1];
		if (m_pEntBuddy) {
			CPhysicalEntity *pent = (CPhysicalEntity*)m_pEntBuddy;
			if (pent->m_flags & (pef_monitor_state_changes | pef_log_state_changes)) {
				EventPhysStateChange event;
				event.pEntity=pent; event.pForeignData=pent->m_pForeignData; event.iForeignData=pent->m_iForeignData;
				event.BBoxNew[0]=params->BBox[0]; event.BBoxNew[1]=params->BBox[1];
				event.BBoxOld[0]=pent->m_BBox[0]; event.BBoxOld[1]=pent->m_BBox[1];
				event.iSimClass[0] = pent->m_iSimClass; event.iSimClass[1] = pent->m_iSimClass;
				event.timeIdle = pent->m_timeIdle;

				pent->m_pWorld->OnEvent(pent->m_flags, &event);
			}

			if (m_pEntBuddy->m_pEntBuddy==this) {
				m_pEntBuddy->m_BBox[0] = m_BBox[0];
				m_pEntBuddy->m_BBox[1] = m_BBox[1];
			}
		}

		CPhysicalWorld *pWorld = (CPhysicalWorld*)GetWorld();
		pWorld->RepositionEntity(this,1|8);
		return 1;
	}

	if (_params->type==pe_params_pos::type_id) {
		pe_params_pos *params = (pe_params_pos*)_params;
		if (!is_unused(params->pos) | !is_unused(params->q) | !is_unused(params->scale) | 
				(intptr_t)params->pMtx3x3 | (intptr_t)params->pMtx3x4)
			return GetEntity()->SetParams(params);
		if (!is_unused(params->iSimClass)) {
			i32 wasAreaTrigger = IsAreaTrigger(this);
			m_iSimClass = params->iSimClass;
			if (IsAreaTrigger(this) ^ wasAreaTrigger)
				((CPhysicalWorld*)GetWorld())->m_numAreaTriggers += 1-wasAreaTrigger*2;
		}
		return 1;
	}

	if (_params->type==pe_params_foreign_data::type_id) {
		pe_params_foreign_data *params = (pe_params_foreign_data*)_params;
		i32 wasAreaTrigger = IsAreaTrigger(this);
		if (!is_unused(params->pForeignData)) m_pForeignData = 0;
		if (!is_unused(params->iForeignData)) m_iForeignData = params->iForeignData;
		if (!is_unused(params->pForeignData)) m_pForeignData = params->pForeignData;
		if (!is_unused(params->iForeignFlags)) m_iForeignFlags = params->iForeignFlags;
		m_iForeignFlags = (m_iForeignFlags & params->iForeignFlagsAND) | params->iForeignFlagsOR;
		if (IsAreaTrigger(this) ^ wasAreaTrigger)
			((CPhysicalWorld*)GetWorld())->m_numAreaTriggers += 1-wasAreaTrigger*2;
		if (m_pEntBuddy) {
			m_pEntBuddy->m_pForeignData = m_pForeignData;
			m_pEntBuddy->m_iForeignData = m_iForeignData;
			m_pEntBuddy->m_iForeignFlags = m_iForeignFlags;
		}
		return 1;
	}

	if (m_pEntBuddy)
		return m_pEntBuddy->SetParams(_params);
	return 0;//GetEntity()->SetParams(_params);
}

i32 CPhysicalPlaceholder::GetParams(pe_params *_params) const
{
	if (_params->type==pe_params_bbox::type_id) {
		pe_params_bbox *params = (pe_params_bbox*)_params;
		params->BBox[0] = m_BBox[0];
		params->BBox[1] = m_BBox[1];
		return 1;
	}

	if (_params->type==pe_params_foreign_data::type_id) {
		pe_params_foreign_data *params = (pe_params_foreign_data*)_params;
		params->iForeignData = m_iForeignData;
		params->pForeignData = m_pForeignData;
		params->iForeignFlags = m_iForeignFlags;
		return 1;
	}

	return ((CPhysicalEntity*)this)->GetEntity()->GetParams(_params);
}

i32 CPhysicalPlaceholder::GetStatus(pe_status* _status) const
{ 
	if (_status->type==pe_status_placeholder::type_id) {
		((pe_status_placeholder*)_status)->pFullEntity = m_pEntBuddy;
		return 1;
	}

	if (_status->type==pe_status_awake::type_id)
		return 0;

	return ((CPhysicalEntity*)this)->GetEntity()->GetStatus(_status); 
}
i32 CPhysicalPlaceholder::Action(pe_action* action, i32 bThreadSafe) { 
	if (action->type==pe_action_awake::type_id && ((pe_action_awake*)action)->bAwake==0 && !m_pEntBuddy) {
		if (m_iSimClass==2)
			m_iSimClass = 1;
		return 1;
	}
	if (action->type==pe_action_remove_all_parts::type_id && !m_pEntBuddy)
		return 1;
	if (action->type==pe_action_reset::type_id) {
		if (!m_pEntBuddy)
			return 1;
		((CPhysicalEntity*)m_pEntBuddy)->m_timeIdle = ((CPhysicalEntity*)m_pEntBuddy)->m_maxTimeIdle+1.0f;
		return m_pEntBuddy->Action(action);
	}
	return GetEntity()->Action(action); 
}

i32 CPhysicalPlaceholder::AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id, i32 bThreadSafe) { 
	return GetEntity()->AddGeometry(pgeom,params,id,bThreadSafe); 
}
void CPhysicalPlaceholder::RemoveGeometry(i32 id, i32 bThreadSafe) { 
	return GetEntity()->RemoveGeometry(id,bThreadSafe); 
}
i32 CPhysicalPlaceholder::GetStateSnapshot(class CStream &stm, float time_back, i32 flags) { 
	return GetEntity()->GetStateSnapshot(stm,time_back,flags); 
}
i32 CPhysicalPlaceholder::GetStateSnapshot(TSerialize ser, float time_back, i32 flags) {
	return GetEntity()->GetStateSnapshot(ser,time_back,flags);
}
i32 CPhysicalPlaceholder::SetStateFromSnapshot(class CStream &stm, i32 flags) { 
	return GetEntity()->SetStateFromSnapshot(stm,flags); 
}
i32 CPhysicalPlaceholder::SetStateFromSnapshot(TSerialize ser, i32 flags) {
	return GetEntity()->SetStateFromSnapshot(ser,flags);
}
i32 CPhysicalPlaceholder::SetStateFromTypedSnapshot(TSerialize ser, i32 type, i32 flags) {
	return GetEntity()->SetStateFromTypedSnapshot(ser,type,flags);
}
i32 CPhysicalPlaceholder::PostSetStateFromSnapshot() { 
	return GetEntity()->PostSetStateFromSnapshot(); 
}
i32 CPhysicalPlaceholder::GetStateSnapshotTxt(char *txtbuf,i32 szbuf, float time_back) {
	return GetEntity()->GetStateSnapshotTxt(txtbuf,szbuf,time_back);
}
void CPhysicalPlaceholder::SetStateFromSnapshotTxt(tukk txtbuf,i32 szbuf) {
	GetEntity()->SetStateFromSnapshotTxt(txtbuf,szbuf);
}
u32 CPhysicalPlaceholder::GetStateChecksum() {
	return GetEntity()->GetStateChecksum();
}
void CPhysicalPlaceholder::SetNetworkAuthority(i32 authoritive, i32 paused) {
	return GetEntity()->SetNetworkAuthority(authoritive, paused);
}

i32 CPhysicalPlaceholder::Step(float time_interval) { 
	return GetEntity()->Step(time_interval); 
}
void CPhysicalPlaceholder::StartStep(float time_interval) { 
	GetEntity()->StartStep(time_interval); 
}
void CPhysicalPlaceholder::StepBack(float time_interval) { 
	return GetEntity()->StepBack(time_interval); 
}
