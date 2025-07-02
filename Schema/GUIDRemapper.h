// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/IGUIDRemapper.h>

namespace sxema
{
class CGUIDRemapper : public IGUIDRemapper
{
private:

	typedef std::unordered_map<DrxGUID, DrxGUID> GUIDs;

public:

	// IGUIDRemapper
	virtual bool  Empty() const override;
	virtual void  Bind(const DrxGUID& from, const DrxGUID& to) override;
	virtual DrxGUID Remap(const DrxGUID& from) const override;
	// ~IGUIDRemapper

private:

	GUIDs m_guids;
};
} // sxema
