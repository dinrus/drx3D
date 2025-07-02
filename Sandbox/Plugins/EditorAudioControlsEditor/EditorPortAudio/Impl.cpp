// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Impl.h"

#include "Common.h"
#include "EventConnection.h"
#include "ProjectLoader.h"
#include "DataPanel.h"

#include <DrxAudioImplPortAudio/GlobalData.h>
#include <DrxSystem/ISystem.h>
#include <drx3D/CoreX/StlUtils.h>

namespace ACE
{
namespace Impl
{
namespace PortAudio
{
//////////////////////////////////////////////////////////////////////////
string GetPath(CItem const* const pItem)
{
	string path;
	IItem const* pParent = pItem->GetParent();

	while (pParent != nullptr)
	{
		string const parentName = pParent->GetName();

		if (!parentName.empty())
		{
			if (path.empty())
			{
				path = parentName;
			}
			else
			{
				path = parentName + "/" + path;
			}
		}

		pParent = pParent->GetParent();
	}

	return path;
}

//////////////////////////////////////////////////////////////////////////
CImpl::CImpl()
	: m_pDataPanel(nullptr)
	, m_assetAndProjectPath(AUDIO_SYSTEM_DATA_ROOT "/" +
	                        string(DrxAudio::Impl::PortAudio::s_szImplFolderName) +
	                        "/" +
	                        string(DrxAudio::s_szAssetsFolderName))
{
	gEnv->pAudioSystem->GetImplInfo(m_implInfo);
	m_implName = m_implInfo.name.c_str();
	m_implFolderName = DrxAudio::Impl::PortAudio::s_szImplFolderName;
}

//////////////////////////////////////////////////////////////////////////
CImpl::~CImpl()
{
	Clear();
	DestroyDataPanel();
}

//////////////////////////////////////////////////////////////////////////
QWidget* CImpl::CreateDataPanel()
{
	m_pDataPanel = new CDataPanel(*this);
	return m_pDataPanel;
}

//////////////////////////////////////////////////////////////////////////
void CImpl::DestroyDataPanel()
{
	if (m_pDataPanel != nullptr)
	{
		delete m_pDataPanel;
		m_pDataPanel = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::Reload(bool const preserveConnectionStatus)
{
	Clear();

	CProjectLoader(m_assetAndProjectPath, m_rootItem);

	CreateItemCache(&m_rootItem);

	if (preserveConnectionStatus)
	{
		for (auto const& connection : m_connectionsByID)
		{
			if (connection.second > 0)
			{
				auto const pItem = static_cast<CItem* const>(GetItem(connection.first));

				if (pItem != nullptr)
				{
					pItem->SetFlags(pItem->GetFlags() | EItemFlags::IsConnected);
				}
			}
		}
	}
	else
	{
		m_connectionsByID.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
IItem* CImpl::GetItem(ControlId const id) const
{
	IItem* pIItem = nullptr;

	if (id >= 0)
	{
		pIItem = stl::find_in_map(m_itemCache, id, nullptr);
	}

	return pIItem;
}

//////////////////////////////////////////////////////////////////////////
DrxIcon const& CImpl::GetItemIcon(IItem const* const pIItem) const
{
	auto const pItem = static_cast<CItem const* const>(pIItem);
	DRX_ASSERT_MESSAGE(pItem != nullptr, "Impl item is null pointer.");
	return GetTypeIcon(pItem->GetType());
}

//////////////////////////////////////////////////////////////////////////
QString const& CImpl::GetItemTypeName(IItem const* const pIItem) const
{
	auto const pItem = static_cast<CItem const* const>(pIItem);
	DRX_ASSERT_MESSAGE(pItem != nullptr, "Impl item is null pointer.");
	return TypeToString(pItem->GetType());
}

//////////////////////////////////////////////////////////////////////////
bool CImpl::IsSystemTypeSupported(EAssetType const assetType) const
{
	bool isSupported = false;

	switch (assetType)
	{
	case EAssetType::Trigger:
	case EAssetType::Folder:
	case EAssetType::Library:
		isSupported = true;
		break;
	default:
		isSupported = false;
		break;
	}

	return isSupported;
}

//////////////////////////////////////////////////////////////////////////
bool CImpl::IsTypeCompatible(EAssetType const assetType, IItem const* const pIItem) const
{
	bool isCompatible = false;
	auto const pItem = static_cast<CItem const* const>(pIItem);

	if (pItem != nullptr)
	{
		if (assetType == EAssetType::Trigger)
		{
			isCompatible = (pItem->GetType() == EItemType::Event);
		}
	}

	return isCompatible;
}

//////////////////////////////////////////////////////////////////////////
EAssetType CImpl::ImplTypeToAssetType(IItem const* const pIItem) const
{
	EAssetType assetType = EAssetType::None;
	auto const pItem = static_cast<CItem const* const>(pIItem);

	if (pItem != nullptr)
	{
		EItemType const implType = pItem->GetType();

		switch (implType)
		{
		case EItemType::Event:
			assetType = EAssetType::Trigger;
			break;
		default:
			assetType = EAssetType::None;
			break;
		}
	}

	return assetType;
}

//////////////////////////////////////////////////////////////////////////
ConnectionPtr CImpl::CreateConnectionToControl(EAssetType const assetType, IItem const* const pIItem)
{
	ConnectionPtr pConnection = nullptr;

	if (pIItem != nullptr)
	{
		pConnection = std::make_shared<CEventConnection>(pIItem->GetId());
	}

	return pConnection;
}

//////////////////////////////////////////////////////////////////////////
ConnectionPtr CImpl::CreateConnectionFromXMLNode(XmlNodeRef pNode, EAssetType const assetType)
{
	ConnectionPtr pConnectionPtr = nullptr;

	if (pNode != nullptr)
	{
		char const* const szTag = pNode->getTag();

		if ((_stricmp(szTag, DrxAudio::s_szEventTag) == 0) ||
		    (_stricmp(szTag, DrxAudio::Impl::PortAudio::s_szFileTag) == 0) ||
		    (_stricmp(szTag, "PortAudioEvent") == 0) || // Backwards compatibility.
		    (_stricmp(szTag, "PortAudioSample") == 0))  // Backwards compatibility.
		{
			string name = pNode->getAttr(DrxAudio::s_szNameAttribute);
			string path = pNode->getAttr(DrxAudio::Impl::PortAudio::s_szPathAttribute);
			// Backwards compatibility will be removed before March 2019.
#if defined (USE_BACKWARDS_COMPATIBILITY)
			if (name.IsEmpty() && pNode->haveAttr("portaudio_name"))
			{
				name = pNode->getAttr("portaudio_name");
			}

			if (path.IsEmpty() && pNode->haveAttr("portaudio_path"))
			{
				path = pNode->getAttr("portaudio_path");
			}
#endif      // USE_BACKWARDS_COMPATIBILITY
			ControlId id;

			if (path.empty())
			{
				id = GetId(name);
			}
			else
			{
				id = GetId(path + "/" + name);
			}

			auto pItem = static_cast<CItem*>(GetItem(id));

			if (pItem == nullptr)
			{
				pItem = new CItem(name, id, EItemType::Event, EItemFlags::IsPlaceHolder);
				m_itemCache[id] = pItem;
			}

			if (pItem != nullptr)
			{
				auto const pConnection = std::make_shared<CEventConnection>(pItem->GetId());
				string actionType = pNode->getAttr(DrxAudio::s_szTypeAttribute);
#if defined (USE_BACKWARDS_COMPATIBILITY)
				if (actionType.IsEmpty() && pNode->haveAttr("event_type"))
				{
					actionType = pNode->getAttr("event_type");
				}
#endif        // USE_BACKWARDS_COMPATIBILITY
				pConnection->m_actionType = actionType.compareNoCase(DrxAudio::Impl::PortAudio::s_szStopValue) == 0 ? CEventConnection::EActionType::Stop : CEventConnection::EActionType::Start;

				i32 loopCount = 0;
				pNode->getAttr(DrxAudio::Impl::PortAudio::s_szLoopCountAttribute, loopCount);
				loopCount = std::max(0, loopCount);
				pConnection->m_loopCount = static_cast<u32>(loopCount);

				if (pConnection->m_loopCount == 0)
				{
					pConnection->m_isInfiniteLoop = true;
				}

				pConnectionPtr = pConnection;
			}
		}
	}

	return pConnectionPtr;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CImpl::CreateXMLNodeFromConnection(ConnectionPtr const pConnection, EAssetType const assetType)
{
	XmlNodeRef pNode = nullptr;

	std::shared_ptr<CEventConnection const> const pImplConnection = std::static_pointer_cast<CEventConnection const>(pConnection);
	auto const pItem = static_cast<CItem const* const>(GetItem(pConnection->GetID()));

	if ((pItem != nullptr) && (pImplConnection != nullptr) && (assetType == EAssetType::Trigger))
	{
		pNode = GetISystem()->CreateXmlNode(DrxAudio::s_szEventTag);
		pNode->setAttr(DrxAudio::s_szNameAttribute, pItem->GetName());

		string const path = GetPath(pItem);
		pNode->setAttr(DrxAudio::Impl::PortAudio::s_szPathAttribute, path);

		if (pImplConnection->m_actionType == CEventConnection::EActionType::Start)
		{
			pNode->setAttr(DrxAudio::s_szTypeAttribute, DrxAudio::Impl::PortAudio::s_szStartValue);

			if (pImplConnection->m_isInfiniteLoop)
			{
				pNode->setAttr(DrxAudio::Impl::PortAudio::s_szLoopCountAttribute, 0);
			}
			else
			{
				pNode->setAttr(DrxAudio::Impl::PortAudio::s_szLoopCountAttribute, pImplConnection->m_loopCount);
			}
		}
		else
		{
			pNode->setAttr(DrxAudio::s_szTypeAttribute, DrxAudio::Impl::PortAudio::s_szStopValue);
		}
	}

	return pNode;
}

//////////////////////////////////////////////////////////////////////////
void CImpl::EnableConnection(ConnectionPtr const pConnection, bool const isLoading)
{
	auto const pItem = static_cast<CItem* const>(GetItem(pConnection->GetID()));

	if (pItem != nullptr)
	{
		++m_connectionsByID[pItem->GetId()];
		pItem->SetFlags(pItem->GetFlags() | EItemFlags::IsConnected);

		if ((m_pDataPanel != nullptr) && !isLoading)
		{
			m_pDataPanel->OnConnectionAdded();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::DisableConnection(ConnectionPtr const pConnection, bool const isLoading)
{
	auto const pItem = static_cast<CItem* const>(GetItem(pConnection->GetID()));

	if (pItem != nullptr)
	{
		i32 connectionCount = m_connectionsByID[pItem->GetId()] - 1;

		if (connectionCount < 1)
		{
			DRX_ASSERT_MESSAGE(connectionCount >= 0, "Connection count is < 0");
			connectionCount = 0;
			pItem->SetFlags(pItem->GetFlags() & ~EItemFlags::IsConnected);
		}

		m_connectionsByID[pItem->GetId()] = connectionCount;

		if ((m_pDataPanel != nullptr) && !isLoading)
		{
			m_pDataPanel->OnConnectionRemoved();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::OnAboutToReload()
{
	if (m_pDataPanel != nullptr)
	{
		m_pDataPanel->OnAboutToReload();
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::OnReloaded()
{
	if (m_pDataPanel != nullptr)
	{
		m_pDataPanel->OnReloaded();
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::OnSelectConnectedItem(ControlId const id) const
{
	if (m_pDataPanel != nullptr)
	{
		m_pDataPanel->OnSelectConnectedItem(id);
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::OnFileImporterOpened()
{
	if (m_pDataPanel != nullptr)
	{
		m_pDataPanel->OnFileImporterOpened();
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::OnFileImporterClosed()
{
	if (m_pDataPanel != nullptr)
	{
		m_pDataPanel->OnFileImporterClosed();
	}
}

//////////////////////////////////////////////////////////////////////////
void CImpl::Clear()
{
	for (auto const& itemPair : m_itemCache)
	{
		delete itemPair.second;
	}

	m_itemCache.clear();
	m_rootItem.Clear();
}

//////////////////////////////////////////////////////////////////////////
void CImpl::CreateItemCache(CItem const* const pParent)
{
	if (pParent != nullptr)
	{
		size_t const numChildren = pParent->GetNumChildren();

		for (size_t i = 0; i < numChildren; ++i)
		{
			auto const pChild = static_cast<CItem* const>(pParent->GetChildAt(i));

			if (pChild != nullptr)
			{
				m_itemCache[pChild->GetId()] = pChild;
				CreateItemCache(pChild);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
ControlId CImpl::GetId(string const& name) const
{
	return DrxAudio::StringToId(name);
}
} //endns PortAudio
} //endns Impl
} //endns ACE
