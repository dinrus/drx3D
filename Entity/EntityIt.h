// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/EntitySystem.h>
#include <drx3D/Entity/Entity.h>

class CEntity;
struct IEntity;

class CEntityItMap final : public IEntityIt
{
public:
	CEntityItMap()
	{
		m_nRefCount = 0;
		MoveFirst();
	}
	virtual ~CEntityItMap() {}

	virtual bool IsEnd() override
	{
		u32 dwMaxUsed = (u32)g_pIEntitySystem->m_EntitySaltBuffer.GetMaxUsed();

		// jump over gaps
		while (m_id <= (i32)dwMaxUsed)
		{
			if (g_pIEntitySystem->m_EntityArray[m_id] != 0)
				return false;

			++m_id;
		}

		return m_id > (i32)dwMaxUsed; // we passed the last element
	}

	virtual IEntity* This() override      { return !IsEnd() ? g_pIEntitySystem->m_EntityArray[m_id] : nullptr; }
	virtual IEntity* Next() override      { return !IsEnd() ? g_pIEntitySystem->m_EntityArray[m_id++] : nullptr; }
	virtual void     MoveFirst() override { m_id = 0; };
	virtual void     AddRef() override    { m_nRefCount++; }
	virtual void     Release() override   { --m_nRefCount; if (m_nRefCount <= 0) { delete this; } }

protected: // ---------------------------------------------------

	i32 m_nRefCount;                          //
	i32 m_id;                                 //
};
