// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>

namespace sxema
{

struct IQuickSearchOptions
{
	virtual ~IQuickSearchOptions() {}

	virtual u32      GetCount() const = 0;
	virtual tukk GetLabel(u32 optionIdx) const = 0;
	virtual tukk GetDescription(u32 optionIdx) const = 0;
	virtual tukk GetHeader() const = 0;
	virtual tukk GetDelimiter() const = 0;
};

} // sxema
