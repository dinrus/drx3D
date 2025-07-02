// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Item.h"

#include <DrxSystem/XML/IXml.h>

namespace ACE
{
namespace Impl
{
namespace PortAudio
{
class CProjectLoader final
{
public:

	explicit CProjectLoader(string const& sAssetsPath, CItem& rootItem);

	CProjectLoader() = delete;

private:

	CItem* CreateItem(string const& name, string const& path, EItemType const type, CItem& rootItem);
	void   LoadFolder(string const& folderPath, CItem& parent);

	string const m_assetsPath;
};
} //endns PortAudio
} //endns Impl
} //endns ACE

