// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Impl.h"

#include "Common.h"
#include "BaseConnection.h"
#include "ParameterConnection.h"
#include "ParameterToStateConnection.h"
#include "SoundbankConnection.h"
#include "ProjectLoader.h"
#include "DataPanel.h"

#include <DrxSystem/ISystem.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>

namespace ACE
{
namespace Impl
{
namespace Wwise
{
//////////////////////////////////////////////////////////////////////////
EItemType TagToType(char const* const szTag)
{
	EItemType type = EItemType::None;

	if (_stricmp(szTag, DrxAudio::s_szEventTag) == 0)
	{
		type = EItemType::Event;
	}
	else if (_stricmp(szTag, DrxAudio::Impl::Wwise::s_szFileTag) == 0)
	{
		type = EItemType::SoundBank;
	}
	else if (_stricmp(szTag, DrxAudio::Impl::Wwise::s_szParameterTag) == 0)
	{
		type = EItemType::Parameter;
	}
	else if (_stricmp(szTag, DrxAudio::Impl::Wwise::s_szSwitchGroupTag) == 0)
	{
		type = EItemType::SwitchGroup;
	}
	else if (_stricmp(szTag, DrxAudio::Impl::Wwise::s_szStateGroupTag) == 0)
	{
		type = EItemType::StateGroup;
	}
	else if (_stricmp(szTag, DrxAudio::Impl::Wwise::s_szAuxBusTag) == 0)
	{
		type = EItemType::AuxBus;
	}

	// Backwards compatibility will be removed before March 2019.
#if defined (USE_BACKWARDS_COMPATIBILITY)
	else if (_stricmp(szTag, "WwiseEvent") == 0)
	{
		type = EItemType::Event;
	}
	else if (_stricmp(szTag, "WwiseFile") == 0)
	{
		type = EItemType::SoundBank;
	}
	else if (_stricmp(szTag, "WwiseRtpc") == 0)
	{
		type = EItemType::Parameter;
	}
	else if (_stricmp(szTag, "WwiseSwitch") == 0)
	{
		type = EItemType::SwitchGroup;
	}
	else if (_stricmp(szTag, "WwiseState") == 0)
	{
		type = EItemType::StateGroup;
	}
	else if (_stricmp(szTag, "WwiseAuxBus") == 0)
	{
		type = EItemType::AuxBus;
	}
#endif  // USE_BACKWARDS_COMPATIBILITY

	else
	{
		type = EItemType::None;
	}

	return type;
}

//////////////////////////////////////////////////////////////////////////
char const* TypeToTag(EItemType const type)
{
	char const* szTag = nullptr;

	switch (type)
	{
	case EItemType::Event:
		szTag = DrxAudio::s_szEventTag;
		break;
	case EItemType::Parameter:
		szTag = DrxAudio::Impl::Wwise::s_szParameterTag;
		break;
	case EItemType::Switch:
		szTag = DrxAudio::Impl::Wwise::s_szValueTag;
		break;
	case EItemType::AuxBus:
		szTag = DrxAudio::Impl::Wwise::s_szAuxBusTag;
		break;
	case EItemType::SoundBank:
		szTag = DrxAudio::Impl::Wwise::s_szFileTag;
		break;
	case EItemType::State:
		szTag = DrxAudio::Impl::Wwise::s_szValueTag;
		break;
	case EItemType::SwitchGroup:
		szTag = DrxAudio::Impl::Wwise::s_szSwitchGroupTag;
		break;
	case EItemType::StateGroup:
		szTag = DrxAudio::Impl::Wwise::s_szStateGroupTag;
		break;
	default:
		szTag = nullptr;
		break;
	}

	return szTag;
}

//////////////////////////////////////////////////////////////////////////
CItem* SearchForItem(CItem* const pItem, string const& name, EItemType const type)
{
	CItem* pSearchedItem = nullptr;

	if ((pItem->GetName().compareNoCase(name) == 0) && (pItem->GetType() == type))
	{
		pSearchedItem = pItem;
	}
	else
	{
		size_t const numChildren = pItem->GetNumChildren();

		for (size_t i = 0; i < numChildren; ++i)
		{
			CItem* const pFoundItem = SearchForItem(static_cast<CItem* const>(pItem->GetChildAt(i)), name, type);

			if (pFoundItem != nullptr)
			{
				pSearchedItem = pFoundItem;
				break;
			}
		}
	}

	return pSearchedItem;
}

//////////////////////////////////////////////////////////////////////////
CImpl::CImpl()
	: m_pDataPanel(nullptr)
	, m_projectPath(AUDIO_SYSTEM_DATA_ROOT "/wwise_project")
	, m_assetsPath(AUDIO_SYSTEM_DATA_ROOT "/" + string(DrxAudio::Impl::Wwise::s_szImplFolderName) + "/" + string(DrxAudio::s_szAssetsFolderName))
	, m_szUserSettingsFile("%USER%/audiocontrolseditor_wwise.user")
{
	gEnv->pAudioSystem->GetImplInfo(m_implInfo);
	m_implName = m_implInfo.name.c_str();
	m_implFolderName = DrxAudio::Impl::Wwise::s_szImplFolderName;

	Serialization::LoadJsonFile(*this, m_szUserSettingsFile);
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

	CProjectLoader(m_projectPath, m_assetsPath, m_rootItem, m_itemCache);

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

	if (m_pDataPanel != nullptr)
	{
		m_pDataPanel->Reset();
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
void CImpl::SetProjectPath(char const* const szPath)
{
	m_projectPath = szPath;
	Serialization::SaveJsonFile(m_szUserSettingsFile, *this);
}

//////////////////////////////////////////////////////////////////////////
void CImpl::Serialize(Serialization::IArchive& ar)
{
	ar(m_projectPath, "projectPath", "Project Path");
}

//////////////////////////////////////////////////////////////////////////
bool CImpl::IsTypeCompatible(EAssetType const assetType, IItem const* const pIItem) const
{
	bool isCompatible = false;
	auto const pItem = static_cast<CItem const* const>(pIItem);

	if (pItem != nullptr)
	{
		EItemType const implType = pItem->GetType();

		switch (assetType)
		{
		case EAssetType::Trigger:
			isCompatible = (implType == EItemType::Event);
			break;
		case EAssetType::Parameter:
			isCompatible = (implType == EItemType::Parameter);
			break;
		case EAssetType::State:
			isCompatible = (implType == EItemType::Switch) || (implType == EItemType::State) || (implType == EItemType::Parameter);
			break;
		case EAssetType::Environment:
			isCompatible = (implType == EItemType::AuxBus) || (implType == EItemType::Parameter);
			break;
		case EAssetType::Preload:
			isCompatible = (implType == EItemType::SoundBank);
			break;
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
		case EItemType::Parameter:
			assetType = EAssetType::Parameter;
			break;
		case EItemType::Switch:
		case EItemType::State:
			assetType = EAssetType::State;
			break;
		case EItemType::AuxBus:
			assetType = EAssetType::Environment;
			break;
		case EItemType::SoundBank:
			assetType = EAssetType::Preload;
			break;
		case EItemType::StateGroup:
		case EItemType::SwitchGroup:
			assetType = EAssetType::Switch;
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
	auto const pItem = static_cast<CItem const* const>(pIItem);

	if (pItem != nullptr)
	{
		EItemType itemType = pItem->GetType();

		if (itemType == EItemType::Parameter)
		{
			switch (assetType)
			{
			case EAssetType::Parameter:
			case EAssetType::Environment:
				pConnection = std::make_shared<CParameterConnection>(pItem->GetId());
				break;
			case EAssetType::State:
				pConnection = std::make_shared<CParameterToStateConnection>(pItem->GetId());
				break;
			default:
				pConnection = std::make_shared<CBaseConnection>(pItem->GetId());
				break;
			}
		}
		else if (itemType == EItemType::SoundBank)
		{
			pConnection = std::make_shared<CSoundbankConnection>(pItem->GetId());
		}
		else
		{
			pConnection = std::make_shared<CBaseConnection>(pItem->GetId());
		}

	}

	return pConnection;
}

//////////////////////////////////////////////////////////////////////////
ConnectionPtr CImpl::CreateConnectionFromXMLNode(XmlNodeRef pNode, EAssetType const assetType)
{
	ConnectionPtr pConnectionPtr = nullptr;

	if (pNode != nullptr)
	{
		EItemType const type = TagToType(pNode->getTag());

		if (type != EItemType::None)
		{
			string name = pNode->getAttr(DrxAudio::s_szNameAttribute);
			string localizedAttribute = pNode->getAttr(DrxAudio::Impl::Wwise::s_szLocalizedAttribute);
#if defined (USE_BACKWARDS_COMPATIBILITY)
			if (name.IsEmpty() && pNode->haveAttr("wwise_name"))
			{
				name = pNode->getAttr("wwise_name");
			}

			if (localizedAttribute.IsEmpty() && pNode->haveAttr("wwise_localised"))
			{
				localizedAttribute = pNode->getAttr("wwise_localised");
			}
#endif      // USE_BACKWARDS_COMPATIBILITY
			bool const isLocalized = (localizedAttribute.compareNoCase(DrxAudio::Impl::Wwise::s_szTrueValue) == 0);

			CItem* pItem = SearchForItem(&m_rootItem, name, type);

			// If item not found, create a placeholder.
			// We want to keep that connection even if it's not in the middleware.
			// The user could be using the engine without the wwise project
			if (pItem == nullptr)
			{
				ControlId const id = GenerateID(name, isLocalized, &m_rootItem);
				EItemFlags const flags = isLocalized ? (EItemFlags::IsPlaceHolder | EItemFlags::IsLocalized) : EItemFlags::IsPlaceHolder;

				pItem = new CItem(name, id, type, flags);

				m_itemCache[id] = pItem;
			}

			// If it's a switch we actually connect to one of the states within the switch
			if ((type == EItemType::SwitchGroup) || (type == EItemType::StateGroup))
			{
				if (pNode->getChildCount() == 1)
				{
					pNode = pNode->getChild(0);

					if (pNode != nullptr)
					{
						string childName = pNode->getAttr(DrxAudio::s_szNameAttribute);

#if defined (USE_BACKWARDS_COMPATIBILITY)
						if (childName.IsEmpty() && pNode->haveAttr("wwise_name"))
						{
							childName = pNode->getAttr("wwise_name");
						}
#endif            // USE_BACKWARDS_COMPATIBILITY

						CItem* pStateItem = nullptr;
						size_t const numChildren = pItem->GetNumChildren();

						for (size_t i = 0; i < numChildren; ++i)
						{
							auto const pChild = static_cast<CItem* const>(pItem->GetChildAt(i));

							if ((pChild != nullptr) && (pChild->GetName().compareNoCase(childName) == 0))
							{
								pStateItem = pChild;
							}
						}

						if (pStateItem == nullptr)
						{
							ControlId const id = GenerateID(childName);
							pStateItem = new CItem(childName, id, type == EItemType::SwitchGroup ? EItemType::Switch : EItemType::State, EItemFlags::IsPlaceHolder);
							pItem->AddChild(pStateItem);

							m_itemCache[id] = pStateItem;
						}

						pItem = pStateItem;
					}
				}
				else
				{
					DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "[Audio Controls Editor] [Wwise] Error reading connection to Wwise control %s", name);
				}
			}

			if (pItem != nullptr)
			{
				if (type == EItemType::Parameter)
				{
					switch (assetType)
					{
					case EAssetType::Parameter:
					case EAssetType::Environment:
						{

							float mult = DrxAudio::Impl::Wwise::s_defaultParamMultiplier;
							float shift = DrxAudio::Impl::Wwise::s_defaultParamShift;

							pNode->getAttr(DrxAudio::Impl::Wwise::s_szMutiplierAttribute, mult);
							pNode->getAttr(DrxAudio::Impl::Wwise::s_szShiftAttribute, shift);
#if defined (USE_BACKWARDS_COMPATIBILITY)
							if (pNode->haveAttr("wwise_value_multiplier"))
							{
								pNode->getAttr("wwise_value_multiplier", mult);
							}
							if (pNode->haveAttr("wwise_value_shift"))
							{
								pNode->getAttr("wwise_value_shift", shift);
							}
#endif              // USE_BACKWARDS_COMPATIBILITY

							auto const pConnection = std::make_shared<CParameterConnection>(pItem->GetId(), mult, shift);
							pConnectionPtr = pConnection;
						}
						break;
					case EAssetType::State:
						{
							float value = DrxAudio::Impl::Wwise::s_defaultStateValue;

							pNode->getAttr(DrxAudio::Impl::Wwise::s_szValueAttribute, value);
#if defined (USE_BACKWARDS_COMPATIBILITY)
							if (pNode->haveAttr("wwise_value"))
							{
								pNode->getAttr("wwise_value", value);
							}
#endif              // USE_BACKWARDS_COMPATIBILITY

							auto const pConnection = std::make_shared<CParameterToStateConnection>(pItem->GetId(), value);
							pConnectionPtr = pConnection;
						}
						break;
					default:
						pConnectionPtr = std::make_shared<CBaseConnection>(pItem->GetId());
						break;
					}
				}
				else if (type == EItemType::SoundBank)
				{
					pConnectionPtr = std::make_shared<CSoundbankConnection>(pItem->GetId());
				}
				else
				{
					pConnectionPtr = std::make_shared<CBaseConnection>(pItem->GetId());
				}
			}
		}
	}

	return pConnectionPtr;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CImpl::CreateXMLNodeFromConnection(ConnectionPtr const pConnection, EAssetType const assetType)
{
	XmlNodeRef pNode = nullptr;

	auto const pItem = static_cast<CItem const* const>(GetItem(pConnection->GetID()));

	if (pItem != nullptr)
	{
		auto const itemType = static_cast<EItemType>(pItem->GetType());

		switch (itemType)
		{
		case EItemType::Switch:
		case EItemType::SwitchGroup:
		case EItemType::State:
		case EItemType::StateGroup:
			{
				auto const pParent = static_cast<CItem const* const>(pItem->GetParent());

				if (pParent != nullptr)
				{
					XmlNodeRef const pSwitchNode = GetISystem()->CreateXmlNode(TypeToTag(pParent->GetType()));
					pSwitchNode->setAttr(DrxAudio::s_szNameAttribute, pParent->GetName());

					XmlNodeRef const pStateNode = pSwitchNode->createNode(DrxAudio::Impl::Wwise::s_szValueTag);
					pStateNode->setAttr(DrxAudio::s_szNameAttribute, pItem->GetName());
					pSwitchNode->addChild(pStateNode);

					pNode = pSwitchNode;
				}
			}
			break;
		case EItemType::Parameter:
			{
				XmlNodeRef pConnectionNode;
				pConnectionNode = GetISystem()->CreateXmlNode(TypeToTag(itemType));
				pConnectionNode->setAttr(DrxAudio::s_szNameAttribute, pItem->GetName());

				if ((assetType == EAssetType::Parameter) || (assetType == EAssetType::Environment))
				{
					std::shared_ptr<const CParameterConnection> pParameterConnection = std::static_pointer_cast<const CParameterConnection>(pConnection);

					float const mult = pParameterConnection->GetMultiplier();

					if (mult != DrxAudio::Impl::Wwise::s_defaultParamMultiplier)
					{
						pConnectionNode->setAttr(DrxAudio::Impl::Wwise::s_szMutiplierAttribute, mult);
					}

					float const shift = pParameterConnection->GetShift();

					if (shift != DrxAudio::Impl::Wwise::s_defaultParamShift)
					{
						pConnectionNode->setAttr(DrxAudio::Impl::Wwise::s_szShiftAttribute, shift);
					}

				}
				else if (assetType == EAssetType::State)
				{
					std::shared_ptr<const CParameterToStateConnection> pStateConnection = std::static_pointer_cast<const CParameterToStateConnection>(pConnection);
					pConnectionNode->setAttr(DrxAudio::Impl::Wwise::s_szValueAttribute, pStateConnection->GetValue());
				}

				pNode = pConnectionNode;
			}
			break;
		case EItemType::Event:
			{
				XmlNodeRef pConnectionNode;
				pConnectionNode = GetISystem()->CreateXmlNode(TypeToTag(itemType));
				pConnectionNode->setAttr(DrxAudio::s_szNameAttribute, pItem->GetName());
				pNode = pConnectionNode;
			}
			break;
		case EItemType::AuxBus:
			{
				XmlNodeRef pConnectionNode;
				pConnectionNode = GetISystem()->CreateXmlNode(TypeToTag(itemType));
				pConnectionNode->setAttr(DrxAudio::s_szNameAttribute, pItem->GetName());
				pNode = pConnectionNode;
			}
			break;
		case EItemType::SoundBank:
			{
				XmlNodeRef pConnectionNode = GetISystem()->CreateXmlNode(TypeToTag(itemType));
				pConnectionNode->setAttr(DrxAudio::s_szNameAttribute, pItem->GetName());

				if ((pItem->GetFlags() & EItemFlags::IsLocalized) != 0)
				{
					pConnectionNode->setAttr(DrxAudio::Impl::Wwise::s_szLocalizedAttribute, DrxAudio::Impl::Wwise::s_szTrueValue);
				}

				pNode = pConnectionNode;
			}
			break;
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
ControlId CImpl::GenerateID(string const& fullPathName) const
{
	return DrxAudio::StringToId(fullPathName);
}

//////////////////////////////////////////////////////////////////////////
ControlId CImpl::GenerateID(string const& name, bool isLocalized, CItem* pParent) const
{
	string pathName = (pParent != nullptr && !pParent->GetName().empty()) ? pParent->GetName() + "/" + name : name;

	if (isLocalized)
	{
		pathName = PathUtil::GetLocalizationFolder() + "/" + pathName;
	}

	return GenerateID(pathName);
}
} //endns Wwise
} //endns Impl
} //endns ACE
