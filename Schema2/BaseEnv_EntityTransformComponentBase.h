// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_BasicTypes.h>
#include <drx3D/Schema2/BaseEnv_EntityComponentBase.h>

namespace SchematycBaseEnv
{
	extern i32k g_emptySlot;

	extern const sxema2::SGUID g_tranformAttachmentGUID;

	class CEntityTransformComponentBase : public sxema2::IComponent, public CEntityComponentBase
	{
	public:

		CEntityTransformComponentBase();

		// sxema2::IComponent
		virtual bool Init(const sxema2::SComponentParams& params) override;
		virtual void Shutdown() override;
		// ~sxema2::IComponent

		void SetSlot(i32 slot, const STransform& transform);
		i32 GetSlot() const;
		bool SlotIsValid() const;
		void FreeSlot();
		Matrix34 GetWorldTM() const;

	private:

		CEntityTransformComponentBase* m_pParent;
		i32                            m_slot;
	};
}