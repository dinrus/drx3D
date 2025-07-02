// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Control.h"

#include <DrxSystem/ISystem.h>

namespace ACE
{
class CLibrary;

static constexpr tuk s_szLibraryNodeTag = "Library";
static constexpr tuk s_szFoldersNodeTag = "Folders";
static constexpr tuk s_szControlsNodeTag = "Controls";
static constexpr tuk s_szFolderTag = "Folder";
static constexpr tuk s_szPathAttribute = "path";
static constexpr tuk s_szDescriptionAttribute = "description";

struct SLibraryScope final
{
	SLibraryScope()
		: isDirty(false)
	{
		pNodes[0] = GetISystem()->CreateXmlNode(DrxAudio::s_szTriggersNodeTag);
		pNodes[1] = GetISystem()->CreateXmlNode(DrxAudio::s_szParametersNodeTag);
		pNodes[2] = GetISystem()->CreateXmlNode(DrxAudio::s_szSwitchesNodeTag);
		pNodes[3] = GetISystem()->CreateXmlNode(DrxAudio::s_szEnvironmentsNodeTag);
		pNodes[4] = GetISystem()->CreateXmlNode(DrxAudio::s_szPreloadsNodeTag);
	}

	XmlNodeRef GetXmlNode(EAssetType const type) const
	{
		XmlNodeRef pNode;

		switch (type)
		{
		case EAssetType::Trigger:
			pNode = pNodes[0];
			break;
		case EAssetType::Parameter:
			pNode = pNodes[1];
			break;
		case EAssetType::Switch:
			pNode = pNodes[2];
			break;
		case EAssetType::Environment:
			pNode = pNodes[3];
			break;
		case EAssetType::Preload:
			pNode = pNodes[4];
			break;
		default:
			pNode = nullptr;
			break;
		}

		return pNode;
	}

	XmlNodeRef pNodes[5]; // Trigger, Parameter, Switch, Environment, Preload
	bool       isDirty;
};

using LibraryStorage = std::map<Scope, SLibraryScope>;

class CFileWriter final
{
public:

	explicit CFileWriter(FileNames& previousLibraryPaths);

	CFileWriter() = delete;

	void WriteAll();

private:

	void WriteLibrary(CLibrary& library);
	void WriteItem(CAsset* const pAsset, string const& path, LibraryStorage& library);
	void GetScopes(CAsset const* const pAsset, std::unordered_set<Scope>& scopes);
	void WriteControlToXML(XmlNodeRef const pNode, CControl* const pControl, string const& path);
	void WriteConnectionsToXML(XmlNodeRef const pNode, CControl* const pControl, i32 const platformIndex = -1);
	void WriteLibraryEditorData(CAsset const& library, XmlNodeRef const pParentNode) const;
	void WriteFolderEditorData(CAsset const& library, XmlNodeRef const pParentNode) const;
	void WriteControlsEditorData(CAsset const& parentAsset, XmlNodeRef const pParentNode) const;
	void DeleteLibraryFile(string const& filepath);

	FileNames&          m_previousLibraryPaths;
	FileNames           m_foundLibraryPaths;

	static u32 const s_currentFileVersion;
};
} //endns ACE

