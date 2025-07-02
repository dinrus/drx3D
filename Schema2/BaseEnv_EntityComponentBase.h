// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

namespace SchematycBaseEnv
{
	class CEntityComponentBase
	{
	public:

		CEntityComponentBase();

		bool Init(const sxema2::SComponentParams& params);
		sxema2::IObject& GetObject() const;
		IEntity& GetEntity() const;
		EntityId GetEntityId() const;

	private:

		sxema2::IObject* m_pObject;
		IEntity*            m_pEntity;
	};
}