// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ProceduralClipConversion.h>

static tukk const ProceduralClipConversionFilename = "Scripts/Mannequin/ProcClipConversion.xml";

//////////////////////////////////////////////////////////////////////////
namespace
{
void InitParamList(SProcClipConversionEntry::ParamConversionStringList& paramList, const string& s)
{
	paramList.clear();

	i32 start = 0;
	string token = s.Tokenize(".", start);
	while (!token.empty())
	{
		paramList.push_back(token);
		token = s.Tokenize(".", start);
	}
}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef SProcClipConversionEntry::Convert(const XmlNodeRef& pOldNode) const
{
	DRX_ASSERT(pOldNode);

	const string typeName = pOldNode->getAttr("type");

	XmlNodeRef pNewNode = gEnv->pSystem->CreateXmlNode("ProceduralParams");
	if (!pNewNode)
	{
		DrxFatalError("SProcClipConversionEntry::Convert: Unable to create a ProceduralParams XmlNode while trying to convert old node of type '%s'.", typeName.c_str());
	}

	pNewNode->setAttr("type", typeName.c_str());

	ConvertAttribute(pOldNode, pNewNode, "anim", animRef);
	ConvertAttribute(pOldNode, pNewNode, "string", dataString);
	ConvertAttribute(pOldNode, pNewNode, "crcString", crcString);

	XmlNodeRef pOldParamsNode = pOldNode->findChild("Params");
	if (pOldParamsNode)
	{
		i32k oldParamsCount = pOldParamsNode->getChildCount();
		i32k expectedParamsCount = static_cast<i32>(parameters.size());
		if (expectedParamsCount < oldParamsCount)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "SProcClipConversionEntry::Convert: Mismatch in xml Proc clip node of type '%s' at line %d between expected parameters (%d, defined in '%s') and number of parameters in the xml node (%d).", typeName.c_str(), pOldNode->getLine(), expectedParamsCount, ProceduralClipConversionFilename, oldParamsCount);
		}

		i32k safeParamsCount = min(oldParamsCount, expectedParamsCount);
		for (i32 i = 0; i < safeParamsCount; ++i)
		{
			XmlNodeRef pOldParamsChildNode = pOldParamsNode->getChild(i);
			const ParamConversionStringList& paramList = parameters[i];
			ConvertAttribute(pOldParamsChildNode, pNewNode, "value", paramList);
		}

		// Convert remaining parameters as if they had a value of 0 because the old procedural clip format would not save trailing parameters when their value was 0.
		for (i32 i = safeParamsCount; i < expectedParamsCount; ++i)
		{
			const ParamConversionStringList& paramList = parameters[i];
			CreateAttribute(pNewNode, paramList, "0");
		}
	}

	return pNewNode;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef SProcClipConversionEntry::CreateNodeStructure(XmlNodeRef pNewNode, const ParamConversionStringList& paramList) const
{
	DRX_ASSERT(pNewNode);
	if (paramList.empty())
	{
		return XmlNodeRef();
	}

	const size_t paramListSize = paramList.size();
	XmlNodeRef pCurrentNode = pNewNode;
	for (size_t i = 0; i < paramListSize; ++i)
	{
		const string& paramName = paramList[i];
		XmlNodeRef pChildNode = pCurrentNode->findChild(paramName.c_str());
		if (!pChildNode)
		{
			pChildNode = pCurrentNode->createNode(paramName.c_str());
			pCurrentNode->addChild(pChildNode);
		}
		pCurrentNode = pChildNode;
	}
	return pCurrentNode;
}

//////////////////////////////////////////////////////////////////////////
void SProcClipConversionEntry::ConvertAttribute(const XmlNodeRef& pOldNode, XmlNodeRef pNewNode, tukk const oldAttributeName, const ParamConversionStringList& newAttributeName) const
{
	DRX_ASSERT(pOldNode);
	DRX_ASSERT(pNewNode);

	if (pOldNode->haveAttr(oldAttributeName))
	{
		if (newAttributeName.empty())
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Procedural clip conversion for attribute '%s' failed: Mapping to a new attribute name is missing.", oldAttributeName);
			return;
		}

		XmlNodeRef pAttributeNode = CreateNodeStructure(pNewNode, newAttributeName);
		if (!pAttributeNode)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Procedural clip conversion for attribute '%s' failed: Unable to create structure for new node.", oldAttributeName);
			return;
		}

		pAttributeNode->setAttr("value", pOldNode->getAttr(oldAttributeName));
	}
}

//////////////////////////////////////////////////////////////////////////
void SProcClipConversionEntry::CreateAttribute(XmlNodeRef pNewNode, const ParamConversionStringList& newAttributeName, tukk const newAttributeValue) const
{
	DRX_ASSERT(pNewNode);

	if (newAttributeName.empty())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Procedural clip conversion failed: Failed to create new attribute since no name is defined.");
		return;
	}

	XmlNodeRef pAttributeNode = CreateNodeStructure(pNewNode, newAttributeName);
	if (!pAttributeNode)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Procedural clip conversion failed: Unable to create node structure for new attribute.");
		return;
	}

	pAttributeNode->setAttr("value", newAttributeValue);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CProcClipConversionHelper::CProcClipConversionHelper()
{
	XmlNodeRef pXmlNode = gEnv->pSystem->LoadXmlFromFile(ProceduralClipConversionFilename);
	if (!pXmlNode)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Unable to load '%s'. Old procedural clips will not be converted.", ProceduralClipConversionFilename);
		return;
	}

	i32k childCount = pXmlNode->getChildCount();
	for (i32 i = 0; i < childCount; ++i)
	{
		XmlNodeRef pXmlEntryNode = pXmlNode->getChild(i);
		LoadEntry(pXmlEntryNode);
	}
}

//////////////////////////////////////////////////////////////////////////
void CProcClipConversionHelper::LoadEntry(const XmlNodeRef& pXmlEntryNode)
{
	DRX_ASSERT(pXmlEntryNode);

	const string name = pXmlEntryNode->getTag();

	SProcClipConversionEntry entry;

	InitParamList(entry.animRef, pXmlEntryNode->getAttr("anim"));
	InitParamList(entry.crcString, pXmlEntryNode->getAttr("crcString"));
	InitParamList(entry.dataString, pXmlEntryNode->getAttr("string"));

	i32k childCount = pXmlEntryNode->getChildCount();
	for (i32 i = 0; i < childCount; ++i)
	{
		XmlNodeRef pXmlParamsNode = pXmlEntryNode->getChild(i);
		SProcClipConversionEntry::ParamConversionStringList paramEntry;
		InitParamList(paramEntry, pXmlParamsNode->getAttr("value"));
		entry.parameters.push_back(paramEntry);
	}

	conversionMap[name] = entry;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CProcClipConversionHelper::Convert(const XmlNodeRef& pOldXmlNode)
{
	const string typeName = pOldXmlNode->getAttr("type");

	ConversionEntryMap::const_iterator cit = conversionMap.find(typeName);
	if (cit == conversionMap.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Found no conversion information for old procedural clip of type '%s'. Please add it to '%s'.", typeName.c_str(), ProceduralClipConversionFilename);
		return XmlNodeRef();
	}

	const SProcClipConversionEntry& entry = cit->second;
	return entry.Convert(pOldXmlNode);
}
