// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ProjectLoader.h"

#include "Impl.h"

#include <DrxSystem/File/DrxFile.h>
#include <DrxSystem/ISystem.h>

namespace ACE
{
namespace Impl
{
namespace SDLMixer
{
//////////////////////////////////////////////////////////////////////////
void SetParentPakStatus(CItem* pParent, EPakStatus const pakStatus)
{
	while (pParent != nullptr)
	{
		pParent->SetPakStatus(pParent->GetPakStatus() | pakStatus);
		pParent = static_cast<CItem*>(pParent->GetParent());
	}
}

//////////////////////////////////////////////////////////////////////////
CProjectLoader::CProjectLoader(string const& assetsPath, CItem& rootItem)
	: m_assetsPath(assetsPath)
{
	LoadFolder("", rootItem);
}

//////////////////////////////////////////////////////////////////////////
void CProjectLoader::LoadFolder(string const& folderPath, CItem& parent)
{
	_finddata_t fd;
	IDrxPak* const pDrxPak = gEnv->pDrxPak;
	intptr_t const handle = pDrxPak->FindFirst(m_assetsPath + "/" + folderPath + "/*.*", &fd);

	if (handle != -1)
	{
		do
		{
			string const name = fd.name;

			if ((name != ".") && (name != "..") && !name.empty())
			{
				if (fd.attrib & _A_SUBDIR)
				{
					if (folderPath.empty())
					{
						LoadFolder(name, *CreateItem(name, folderPath, EItemType::Folder, parent));
					}
					else
					{
						LoadFolder(folderPath + "/" + name, *CreateItem(name, folderPath, EItemType::Folder, parent));
					}
				}
				else
				{
					string::size_type const posExtension = name.rfind('.');

					if (posExtension != string::npos)
					{
						if ((_stricmp(name.data() + posExtension, ".mp3") == 0) ||
						    (_stricmp(name.data() + posExtension, ".ogg") == 0) ||
						    (_stricmp(name.data() + posExtension, ".wav") == 0))
						{
							// Create the event with the same name as the file
							CreateItem(name, folderPath, EItemType::Event, parent);
						}
					}
				}
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}
}

//////////////////////////////////////////////////////////////////////////
CItem* CProjectLoader::CreateItem(string const& name, string const& path, EItemType const type, CItem& parent)
{
	ControlId id;
	string filePath = m_assetsPath + "/";

	if (path.empty())
	{
		id = DrxAudio::StringToId(name);
		filePath += name;
	}
	else
	{
		id = DrxAudio::StringToId(path + "/" + name);
		filePath += (path + "/" + name);
	}

	EItemFlags const flags = type == EItemType::Folder ? EItemFlags::IsContainer : EItemFlags::None;
	EPakStatus pakStatus = EPakStatus::None;

	if (gEnv->pDrxPak->IsFileExist(filePath.c_str(), IDrxPak::eFileLocation_InPak))
	{
		pakStatus |= EPakStatus::InPak;
	}

	if (gEnv->pDrxPak->IsFileExist(filePath.c_str(), IDrxPak::eFileLocation_OnDisk))
	{
		pakStatus |= EPakStatus::OnDisk;
	}

	if (type == EItemType::Event)
	{
		SetParentPakStatus(&parent, pakStatus);
	}

	auto const pItem = new CItem(name, id, type, flags, pakStatus, filePath);

	parent.AddChild(pItem);
	return pItem;
}
} //endns SDLMixer
} //endns Impl
} //endns ACE
