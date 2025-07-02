// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <vector>
#include <map>

class EDITOR_COMMON_API DependencyManager
{
public:
	typedef std::vector<string> Strings;

	void SetDependencies(tukk asset, Strings& usedAssets);
	void FindUsers(Strings* users, tukk asset) const;
	void FindDepending(Strings* assets, tukk user) const;

private:
	typedef std::map<string, Strings, stl::less_stricmp<string>> UsedAssets;
	UsedAssets m_usedAssets;

	typedef std::map<string, Strings, stl::less_stricmp<string>> AssetUsers;
	AssetUsers m_assetUsers;
};

