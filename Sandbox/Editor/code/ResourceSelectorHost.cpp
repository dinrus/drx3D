// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "ResourceSelectorHost.h"

#include "Alembic/AlembicCompiler.h"
#include "Controls/QuestionDialog.h"
#include "Util/FileUtil.h"
#include "FileSystem/FileSystem_Snapshot.h"

namespace Private_ResourceSelectorHost
{
class CResourceSelectorHost : public IResourceSelectorHost
{
public:
	SStaticResourceSelectorEntry defaultEntry;

	CResourceSelectorHost()
		: defaultEntry("", nullptr, "")
	{
	}

	~CResourceSelectorHost()
	{
	}

	const SStaticResourceSelectorEntry* GetSelector(tukk typeName) const override
	{
		const auto it = m_typeMap.find(typeName);
		if (it == m_typeMap.end())
		{
			//Changed from warning to assert as this is a programmer error which should never happen 
			//as it could allow invalid data to be inputted and saved
			DRX_ASSERT_MESSAGE(0, "No Resource Selector is registered for resource type \"%s\"", typeName);
			return &defaultEntry;
		}

		return it->second;
	}

	dll_string  SelectResource(tukk typeName, tukk previousValue, QWidget* parentWidget = nullptr, uk contextObject = nullptr) const override
	{
		auto selector = GetSelector(typeName);
		SResourceSelectorContext context;
		context.resourceSelectorEntry = selector;
		context.parentWidget = parentWidget;
		context.contextObjectType = selector->GetContextType();
		context.contextObject = contextObject;
		return selector->SelectResource(context, previousValue);
	}

	void RegisterResourceSelector(const SStaticResourceSelectorEntry* entry) override
	{
		DRX_ASSERT_MESSAGE(m_typeMap.find(entry->GetTypeName()) == m_typeMap.end(), "Resource selector already registered with name \"%s\"", entry->GetTypeName());
		m_typeMap[entry->GetTypeName()] = entry;
	}

	void SetGlobalSelection(tukk resourceType, tukk value) override
	{
		if (!resourceType)
			return;
		if (!value)
			return;
		m_globallySelectedResources[resourceType] = value;
	}

	tukk GetGlobalSelection(tukk resourceType) const override
	{
		if (!resourceType)
			return "";
		auto it = m_globallySelectedResources.find(resourceType);
		if (it != m_globallySelectedResources.end())
			return it->second.c_str();
		return "";
	}

private:
	typedef std::map<string, const SStaticResourceSelectorEntry*, stl::less_stricmp<string>> TTypeMap;
	TTypeMap                 m_typeMap;

	std::map<string, string> m_globallySelectedResources;
};

// ---------------------------------------------------------------------------

dll_string AnyFileSelector(const SResourceSelectorContext& x, tukk previousValue)
{
	string relativeFilename = previousValue;
	string startPath = PathUtil::GetPathWithoutFilename(previousValue);
	if (CFileUtil::SelectSingleFile(x.parentWidget, EFILE_TYPE_ANY, relativeFilename, "", startPath))
		return relativeFilename.GetBuffer();
	else
		return previousValue;
}

REGISTER_RESOURCE_SELECTOR("AnyFile", AnyFileSelector, "")
}

// ---------------------------------------------------------------------------

IResourceSelectorHost* CreateResourceSelectorHost()
{
	return new Private_ResourceSelectorHost::CResourceSelectorHost();
}

