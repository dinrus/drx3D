// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Asset.h"

namespace ACE
{
class CFolder final : public CAsset
{
public:

	explicit CFolder(string const& name);

	CFolder() = delete;
};
} //endns ACE
