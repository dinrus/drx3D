// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_EntityTransformComponentBase.h>

namespace SchematycBaseEnv
{
	i32k g_emptySlot = -1;

	const sxema2::SGUID g_tranformAttachmentGUID("64020f3e-1cd3-41e9-8bbb-92563378d111");

	CEntityTransformComponentBase::CEntityTransformComponentBase()
		: m_pParent(nullptr)
		, m_slot(g_emptySlot)
	{}

	bool CEntityTransformComponentBase::Init(const sxema2::SComponentParams& params)
	{
		if(!CEntityComponentBase::Init(params))
		{
			return false;
		}

		m_pParent = static_cast<CEntityTransformComponentBase*>(params.pParent);
		return true;
	}

	void CEntityTransformComponentBase::Shutdown()
	{
		FreeSlot();
	}

	void CEntityTransformComponentBase::SetSlot(i32 slot, const STransform& transform)
	{
		m_slot = slot;

		if(m_pParent)
		{
			CEntityComponentBase::GetEntity().SetParentSlot(m_pParent->GetSlot(), slot);
		}

		Matrix34 slotTM = Matrix34::CreateRotationXYZ(transform.rot);
		Matrix33 matScale;
		matScale.SetScale(transform.scale);

		slotTM = slotTM * matScale;
		slotTM.SetTranslation(transform.pos);

		CEntityComponentBase::GetEntity().SetSlotLocalTM(slot, slotTM);
	}

	i32 CEntityTransformComponentBase::GetSlot() const
	{
		return m_slot;
	}

	bool CEntityTransformComponentBase::SlotIsValid() const
	{
		return m_slot != g_emptySlot;
	}

	void CEntityTransformComponentBase::FreeSlot()
	{
		if(m_slot != g_emptySlot)
		{
			CEntityComponentBase::GetEntity().FreeSlot(m_slot);
			m_slot = g_emptySlot;
		}
	}

	Matrix34 CEntityTransformComponentBase::GetWorldTM() const
	{
		return CEntityComponentBase::GetEntity().GetSlotWorldTM(m_slot);
	}
}