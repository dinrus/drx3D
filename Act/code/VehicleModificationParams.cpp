// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/VehicleModificationParams.h>

//////////////////////////////////////////////////////////////////////////
struct CVehicleModificationParams::Implementation
{
	typedef std::pair<string, string>              TModificationKey;
	typedef std::map<TModificationKey, XmlNodeRef> TModificationMap;

	TModificationMap m_modifications;
};

//////////////////////////////////////////////////////////////////////////
CVehicleModificationParams::CVehicleModificationParams()
	: m_pImpl(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
CVehicleModificationParams::CVehicleModificationParams(XmlNodeRef xmlVehicleData, tukk modificationName)
	: m_pImpl(NULL)
{
	assert(modificationName != NULL);
	if (modificationName[0] == 0)
	{
		return;
	}

	string mods(modificationName);

	i32 start = 0;
	string modification = mods.Tokenize(",", start);

	while (!modification.empty())
	{
		XmlNodeRef xmlModificationsGroup = xmlVehicleData->findChild("Modifications");
		if (!xmlModificationsGroup)
		{
			GameWarning("Failed to set Modification '%s' because the vehicle doesn't have any modifications", modification.c_str());
			return;
		}

		XmlNodeRef xmlModification = FindModificationNodeByName(modification.c_str(), xmlModificationsGroup);
		if (!xmlModification)
		{
			GameWarning("Failed to set Modification '%s' because the vehicle doesn't have that modification", modification.c_str());
			return;
		}

		if (m_pImpl == NULL)
		{
			m_pImpl = new Implementation();
		}

		InitModification(xmlModification);

		modification = mods.Tokenize(",", start);
	}
}

//////////////////////////////////////////////////////////////////////////
CVehicleModificationParams::~CVehicleModificationParams()
{
	delete m_pImpl;
}

//////////////////////////////////////////////////////////////////////////
void CVehicleModificationParams::InitModification(XmlNodeRef xmlModificationData)
{
	assert(xmlModificationData);

	bool hasParentModification = xmlModificationData->haveAttr("parent");
	if (hasParentModification)
	{
		XmlNodeRef xmlModificationsGroup = xmlModificationData->getParent();

		tukk parentModificationName = xmlModificationData->getAttr("parent");
		XmlNodeRef xmlParentModificationData = FindModificationNodeByName(parentModificationName, xmlModificationsGroup);
		if (xmlParentModificationData && (xmlParentModificationData != xmlModificationData))
		{
			InitModification(xmlParentModificationData);
		}
	}

	XmlNodeRef xmlElemsGroup = xmlModificationData->findChild("Elems");
	if (!xmlElemsGroup)
	{
		return;
	}

	for (i32 i = 0; i < xmlElemsGroup->getChildCount(); ++i)
	{
		XmlNodeRef xmlElem = xmlElemsGroup->getChild(i);

		InitModificationElem(xmlElem);
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CVehicleModificationParams::FindModificationNodeByName(tukk name, XmlNodeRef xmlModificationsGroup)
{
	assert(name != NULL);
	assert(xmlModificationsGroup);

	i32 numNodes = xmlModificationsGroup->getChildCount();
	for (i32 i = 0; i < numNodes; i++)
	{
		XmlNodeRef xmlModification = xmlModificationsGroup->getChild(i);
		tukk modificationName = xmlModification->getAttr("name");
		if (modificationName != 0 && (strcmpi(name, modificationName) == 0))
		{
			return xmlModification;
		}
	}

	return XmlNodeRef();
}

//////////////////////////////////////////////////////////////////////////
void CVehicleModificationParams::InitModificationElem(XmlNodeRef xmlElem)
{
	assert(m_pImpl != NULL);
	assert(xmlElem != (IXmlNode*)NULL);

	bool valid = true;
	valid &= xmlElem->haveAttr("idRef");
	valid &= xmlElem->haveAttr("name");
	valid &= xmlElem->haveAttr("value");

	if (!valid)
	{
		DrxLog("Vehicle modification element at line %i invalid, skipping.", xmlElem->getLine());
		return;
	}

	tukk id = xmlElem->getAttr("idRef");
	tukk attrName = xmlElem->getAttr("name");

	Implementation::TModificationKey key(id, attrName);
	m_pImpl->m_modifications[key] = xmlElem;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CVehicleModificationParams::GetModificationNode(tukk nodeId, tukk attrName) const
{
	assert(nodeId != NULL);
	assert(attrName != NULL);

	if (m_pImpl == NULL)
	{
		return XmlNodeRef();
	}

	Implementation::TModificationKey key(nodeId, attrName);
	Implementation::TModificationMap::const_iterator cit = m_pImpl->m_modifications.find(key);
	bool modificationFound = (cit != m_pImpl->m_modifications.end());
	if (!modificationFound)
	{
		return XmlNodeRef();
	}

	return cit->second;
}
