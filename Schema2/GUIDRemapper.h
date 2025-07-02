// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Separate interface and implementation?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/GUID.h>

namespace sxema2
{
	struct IGUIDRemapper
	{
		virtual ~IGUIDRemapper() {}

		virtual bool Empty() const = 0;
		virtual void Bind(const SGUID& from, const SGUID& to) = 0;
		virtual SGUID Remap(const SGUID& from) const = 0;
	};

	class CGUIDRemapper : public IGUIDRemapper
	{
	public:

		virtual bool Empty() const override
		{
			return m_guids.empty();
		}

		virtual void Bind(const SGUID& from, const SGUID& to) override
		{
			m_guids.insert(GUIDMap::value_type(from, to));
		}

		virtual SGUID Remap(const SGUID& from) const override
		{
			GUIDMap::const_iterator itGUID = m_guids.find(from);
			return itGUID != m_guids.end() ? itGUID->second : from;
		}

	private:

		typedef std::unordered_map<SGUID, SGUID> GUIDMap;

		GUIDMap m_guids;
	};
}
