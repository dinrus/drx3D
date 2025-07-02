// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace sxema
{
// Forward declare structures.


struct IGUIDRemapper
{
	virtual ~IGUIDRemapper() {}

	virtual bool  Empty() const = 0;
	virtual void  Bind(const DrxGUID& from, const DrxGUID& to) = 0;
	virtual DrxGUID Remap(const DrxGUID& from) const = 0;
};
} // sxema
