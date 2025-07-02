// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare interfaces.
struct IObject;

struct SActionParams
{
	inline SActionParams(const DrxGUID& _guid, IObject& _object)
		: guid(_guid)
		, object(_object)
	{}

	const DrxGUID& guid;
	IObject&     object;
	IEntityComponent*  pComponent = nullptr;
};

class CAction
{
public:

	virtual bool Init()
	{
		return true;
	}

	virtual void Start()    {}
	virtual void Stop()     {}
	virtual void Shutdown() {}

	inline void  PreInit(const SActionParams& params)
	{
		m_guid = params.guid;
		m_pObject = &params.object;
		m_pComponent = params.pComponent;
	}

	template<typename SIGNAL> inline void OutputSignal(const SIGNAL& signal)
	{
		GetObject().ProcessSignal(signal, m_guid);
	}

	inline const DrxGUID& GetGUID() const
	{
		return m_guid;
	}

	inline IObject& GetObject() const
	{
		SXEMA_CORE_ASSERT(m_pObject);
		return *m_pObject;
	}

	inline IEntityComponent* GetComponent() const
	{
		return m_pComponent;
	}

private:

	DrxGUID       m_guid;
	IObject*    m_pObject = nullptr;
	IEntityComponent* m_pComponent = nullptr;
};

DECLARE_SHARED_POINTERS(CAction)

} // sxema
