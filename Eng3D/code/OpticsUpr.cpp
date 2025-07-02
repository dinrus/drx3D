// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/OpticsUpr.h>

void COpticsUpr::Reset()
{
	m_OpticsMap.clear();
	m_SearchedOpticsSet.clear();

	m_OpticsList.clear();

	stl::free_container(m_OpticsList);
}

IOpticsElementBase* COpticsUpr::Create(EFlareType type) const
{
	return gEnv->pRenderer->CreateOptics(type);
}

IOpticsElementBase* COpticsUpr::ParseOpticsRecursively(IOpticsElementBase* pParentOptics, XmlNodeRef& node) const
{
	tukk type;
	if (!node->getAttr("Type", &type))
		return NULL;

	if (!gEnv->pRenderer)
		return NULL;

	IOpticsElementBase* pOptics = Create(GetFlareType(type));
	if (pOptics == NULL)
		return NULL;

	bool bEnable(false);
	node->getAttr("Enable", bEnable);
	pOptics->SetEnabled(bEnable);

	tukk name;
	if (node->getAttr("Name", &name))
		pOptics->SetName(name);

	if (pParentOptics)
		pParentOptics->AddElement(pOptics);

	for (i32 i = 0, iChildCount(node->getChildCount()); i < iChildCount; ++i)
	{
		XmlNodeRef pChild = node->getChild(i);
		if (pChild == (IXmlNode*)NULL)
			continue;

		if (!stricmp(pChild->getTag(), "Params"))
		{
			pOptics->Load(pChild);
		}
		else if (!stricmp(pChild->getTag(), "FlareItem"))
		{
			ParseOpticsRecursively(pOptics, pChild);
		}
	}

	return pOptics;
}

bool COpticsUpr::Load(tukk fullFlareName, i32& nOutIndex)
{
	i32 nOpticsIndex = FindOpticsIndex(fullFlareName);
	if (nOpticsIndex >= 0)
	{
		nOutIndex = nOpticsIndex;
		return true;
	}

	string strFullFlareName(fullFlareName);
	i32 nPos = strFullFlareName.find(".");
	if (nPos == -1)
		return false;

	string xmlFileName = strFullFlareName.substr(0, nPos);

	i32 restLength = strFullFlareName.length() - nPos - 1;
	if (restLength <= 0)
		return false;

	if (m_SearchedOpticsSet.find(fullFlareName) != m_SearchedOpticsSet.end())
		return false;

	string fullPath = gEnv->pDrxPak->GetGameFolder() + string("/") + string(FLARE_LIBS_PATH) + xmlFileName + ".xml";
	XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFromFile(fullPath.c_str());

	if (rootNode == (IXmlNode*)NULL)
		return false;

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "%s", fullFlareName);

	string opticsLibName = strFullFlareName.substr(nPos + 1, restLength);
	m_SearchedOpticsSet.insert(fullFlareName);

	for (i32 i = 0, iChildCount(rootNode->getChildCount()); i < iChildCount; ++i)
	{
		XmlNodeRef childNode = rootNode->getChild(i);
		if (childNode == (IXmlNode*)NULL)
			continue;
		tukk name;
		if (!childNode->getAttr("Name", &name))
			continue;
		if (stricmp(name, opticsLibName.c_str()))
			continue;
		IOpticsElementBase* pOptics = ParseOpticsRecursively(NULL, childNode);
		if (pOptics == NULL)
			return false;
		return AddOptics(pOptics, fullFlareName, nOutIndex);
	}

	return false;
}

bool COpticsUpr::Load(XmlNodeRef& rootNode, i32& nOutIndex)
{
	tukk name = NULL;
	if ((rootNode == (IXmlNode*)NULL) || (!rootNode->getAttr("Name", &name)))
		return false;

	tukk libName = NULL;
	if (!rootNode->getAttr("Library", &libName))
		return false;

	IOpticsElementBase* pOptics = ParseOpticsRecursively(NULL, rootNode);
	if (pOptics == NULL)
		return false;

	string fullFlareName = libName + string(".") + name;
	return AddOptics(pOptics, fullFlareName, nOutIndex);
}

EFlareType COpticsUpr::GetFlareType(tukk typeStr) const
{
	if (typeStr == NULL)
		return eFT__Base__;

	const FlareInfoArray::Props array = FlareInfoArray::Get();
	for (size_t i = 0; i < array.size; ++i)
	{
		if (!stricmp(array.p[i].name, typeStr))
			return array.p[i].type;
	}

	return eFT__Base__;
}

i32 COpticsUpr::FindOpticsIndex(tukk fullFlareName) const
{
	std::map<string, i32>::const_iterator iOptics = m_OpticsMap.find(CONST_TEMP_STRING(fullFlareName));
	if (iOptics != m_OpticsMap.end())
		return iOptics->second;
	return -1;
}

IOpticsElementBase* COpticsUpr::GetOptics(i32 nIndex)
{
	if ((size_t)nIndex >= m_OpticsList.size())
		return NULL;
	return m_OpticsList[nIndex];
}

bool COpticsUpr::AddOptics(IOpticsElementBase* pOptics, tukk name, i32& nOutNewIndex)
{
	if (name == NULL)
		return false;
	if (m_OpticsMap.find(name) != m_OpticsMap.end())
		return false;
	nOutNewIndex = (i32)m_OpticsList.size();
	m_OpticsList.push_back(pOptics);
	m_OpticsMap[name] = nOutNewIndex;
	return true;
}

bool COpticsUpr::Rename(tukk fullFlareName, tukk newFullFlareName)
{
	if (m_OpticsMap.find(newFullFlareName) != m_OpticsMap.end())
		return true;

	std::map<string, i32>::iterator iOptics = m_OpticsMap.find(fullFlareName);
	if (iOptics == m_OpticsMap.end())
		return false;

	i32 nOpticsIndex = iOptics->second;
	if (nOpticsIndex < 0 || nOpticsIndex >= (i32)m_OpticsList.size())
		return false;

	m_OpticsMap.erase(iOptics);

	IOpticsElementBasePtr pOptics = m_OpticsList[nOpticsIndex];
	pOptics->SetName(newFullFlareName);
	m_OpticsMap[newFullFlareName] = nOpticsIndex;

	return true;
}

void COpticsUpr::GetMemoryUsage(IDrxSizer* pSizer) const
{
	for (i32 i = 0, iSize(m_OpticsList.size()); i < iSize; ++i)
		m_OpticsList[i]->GetMemoryUsage(pSizer);
}

void COpticsUpr::Invalidate()
{
	for (i32 i = 0, iSize(m_OpticsList.size()); i < iSize; ++i)
		m_OpticsList[i]->Invalidate();
}
