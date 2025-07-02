// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "VehicleXMLHelper.h"
#include <Util/Variable.h>

#include <algorithm>

namespace VehicleXml
{
bool       IsNodeTypeEqual(const XmlNodeRef& node, tukk nodeType);

XmlNodeRef GetXmlNodeDefinitionByIdRec(const XmlNodeRef& node, tukk id);
bool       GetVariableHierarchyRec(IVariable* pCurrentVar, IVariable* pSearchedVar, std::vector<IVariable*>& varHierarchyOut);

XmlNodeRef FindChildDefinitionByName(const XmlNodeRef& node, tukk name);

void       CopyUseNodeImportantProperties(const XmlNodeRef& useNode, XmlNodeRef useNodeDefintionOut);
void       CleanUpExecute(IVariable* pVar, std::vector<uk>& deletedPointers);

XmlNodeRef GetRootXmlNode(const XmlNodeRef& node);

bool       PropertyValueEquals(tukk a, tukk b);

IVariable* CreateSimpleVar(const XmlNodeRef& definition);
IVariable* CreateSimpleVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition);
IVariable* CreateSimpleArrayVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition);
IVariable* CreateTableVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition);
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::PropertyValueEquals(tukk a, tukk b)
{
	if (a == NULL || b == NULL)
	{
		return false;
	}

	return (strcmpi(a, b) == 0);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef VehicleXml::GetRootXmlNode(const XmlNodeRef& node)
{
	assert(node);
	XmlNodeRef currentNode = node;
	do
	{
		if (!currentNode->getParent())
		{
			return currentNode;
		}

		currentNode = currentNode->getParent();

	}
	while (true);
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsNodeTypeEqual(const XmlNodeRef& node, tukk nodeType)
{
	assert(node);
	if (!node)
	{
		return false;
	}

	assert(nodeType != NULL);
	if (nodeType == NULL)
	{
		return false;
	}

	bool isNodeTypeEqual = (stricmp(node->getTag(), nodeType) == 0);
	return isNodeTypeEqual;
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsUseNode(const XmlNodeRef& node)
{
	return IsNodeTypeEqual(node, "Use");
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsArrayNode(const XmlNodeRef& node)
{
	return IsNodeTypeEqual(node, "Array");
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsArrayElementNode(const XmlNodeRef& node, tukk name)
{
	assert(name != NULL);
	if (name == NULL)
	{
		return false;
	}

	return (IsArrayNode(node) && HasElementNameEqualTo(node, name));
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsArrayParentNode(const XmlNodeRef& node, tukk name)
{
	assert(name != NULL);
	if (name == NULL)
	{
		return false;
	}

	return (IsArrayNode(node) && HasNodeNameEqualTo(node, name));
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsPropertyNode(const XmlNodeRef& node)
{
	return IsNodeTypeEqual(node, "Property");
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsOptionalNode(const XmlNodeRef& node)
{
	assert(node);
	if (!node)
	{
		return NULL;
	}

	bool isOptional = false;
	if (node->getAttr("optional", isOptional))
	{
		return isOptional;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::IsDeprecatedNode(const XmlNodeRef& node)
{
	assert(node);
	if (!node)
	{
		return NULL;
	}

	bool isDeprecated = false;
	if (node->getAttr("deprecated", isDeprecated))
	{
		return isDeprecated;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
tukk VehicleXml::GetNodeElementName(const XmlNodeRef& node)
{
	assert(node);
	if (!node)
	{
		return NULL;
	}

	return node->getAttr("elementName");
}

//////////////////////////////////////////////////////////////////////////
tukk VehicleXml::GetNodeId(const XmlNodeRef& node)
{
	assert(node);
	if (!node)
	{
		return NULL;
	}

	return node->getAttr("id");
}

//////////////////////////////////////////////////////////////////////////
tukk VehicleXml::GetNodeName(const XmlNodeRef& node)
{
	assert(node);
	if (!node)
	{
		return NULL;
	}

	return node->getAttr("name");
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::HasNodeIdEqualTo(const XmlNodeRef& node, tukk searchedNodeId)
{
	return PropertyValueEquals(GetNodeId(node), searchedNodeId);
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::HasElementNameEqualTo(const XmlNodeRef& node, tukk searchedName)
{
	return PropertyValueEquals(GetNodeElementName(node), searchedName);
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::HasNodeNameEqualTo(const XmlNodeRef& node, tukk searchedName)
{
	return PropertyValueEquals(GetNodeName(node), searchedName);
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::CopyUseNodeImportantProperties(const XmlNodeRef& useNode, XmlNodeRef useNodeDefintionOut)
{
	assert(useNode);
	assert(useNodeDefintionOut);

	bool isUseNodeOptional;
	if (useNode->getAttr("optional", isUseNodeOptional))
	{
		useNodeDefintionOut->setAttr("optional", isUseNodeOptional);
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef VehicleXml::GetXmlNodeDefinitionByIdRec(const XmlNodeRef& node, tukk id)
{
	assert(id != NULL);
	assert(node);

	if (IsUseNode(node))
	{
		// 'Use' nodes are just references to another node, so they don't contain the definition we're interested in.
		return XmlNodeRef();
	}

	if (HasNodeIdEqualTo(node, id))
	{
		return node;
	}

	for (i32 i = 0; i < node->getChildCount(); ++i)
	{
		XmlNodeRef childNode = node->getChild(i);
		XmlNodeRef nodeFound = GetXmlNodeDefinitionByIdRec(childNode, id);
		if (nodeFound)
		{
			return nodeFound;
		}
	}

	return XmlNodeRef();
}

XmlNodeRef VehicleXml::GetXmlNodeDefinitionById(const XmlNodeRef& definitionRoot, tukk id)
{
	if (!definitionRoot)
	{
		return XmlNodeRef();
	}

	if (id == NULL)
	{
		return XmlNodeRef();
	}

	return GetXmlNodeDefinitionByIdRec(definitionRoot, id);
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::GetVariableHierarchyRec(IVariable* pCurrentVar, IVariable* pSearchedVar, std::vector<IVariable*>& varHierarchyOut)
{
	assert(pCurrentVar != NULL);
	assert(pSearchedVar != NULL);

	if (pSearchedVar == pCurrentVar)
	{
		varHierarchyOut.push_back(pCurrentVar);
		return true;
	}

	for (i32 i = 0; i < pCurrentVar->GetNumVariables(); ++i)
	{
		IVariable* pChildVar = pCurrentVar->GetVariable(i);
		bool found = GetVariableHierarchyRec(pChildVar, pSearchedVar, varHierarchyOut);
		if (found)
		{
			varHierarchyOut.push_back(pCurrentVar);
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool VehicleXml::GetVariableHierarchy(IVariable* pRootVar, IVariable* pVar, std::vector<IVariable*>& varHierarchyOut)
{
	if (pRootVar == NULL)
	{
		return false;
	}

	if (pVar == NULL)
	{
		return false;
	}

	bool found = GetVariableHierarchyRec(pRootVar, pVar, varHierarchyOut);
	std::reverse(varHierarchyOut.begin(), varHierarchyOut.end());

	return found;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef VehicleXml::FindChildDefinitionByName(const XmlNodeRef& node, tukk name)
{
	if (IsArrayNode(node))
	{
		if (HasElementNameEqualTo(node, name))
		{
			// We're searching for an element of the array.
			// Return the array definition as the child definition is defined there.
			return node;
		}
	}

	for (i32 i = 0; i < node->getChildCount(); ++i)
	{
		XmlNodeRef childNode = node->getChild(i);
		if (HasNodeNameEqualTo(childNode, name))
		{
			return childNode;
		}

		if (IsUseNode(childNode))
		{
			tukk id = GetNodeId(childNode);
			XmlNodeRef rootNode = GetRootXmlNode(node);
			XmlNodeRef useNodeDefintion = GetXmlNodeDefinitionById(rootNode, id);
			if (useNodeDefintion)
			{
				if (HasNodeNameEqualTo(useNodeDefintion, name))
				{
					XmlNodeRef useNodeDefintionCopy = useNodeDefintion->clone();
					CopyUseNodeImportantProperties(childNode, useNodeDefintionCopy);
					return useNodeDefintionCopy;
				}
			}
		}
	}

	return XmlNodeRef();
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::GetXmlNodeChildDefinitions(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition, std::vector<XmlNodeRef>& childDefinitionsOut)
{
	if (!definitionRoot)
	{
		return;
	}

	if (!definition)
	{
		return;
	}

	for (i32 i = 0; i < definition->getChildCount(); ++i)
	{
		XmlNodeRef childNode = definition->getChild(i);

		if (IsUseNode(childNode))
		{
			// Use nodes will only reference a definition by id, so we must obtain the real definition.
			tukk childNodeId = GetNodeId(childNode);
			childNode = GetXmlNodeDefinitionById(definitionRoot, childNodeId);
		}

		if (childNode)
		{
			childDefinitionsOut.push_back(childNode);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef VehicleXml::GetXmlNodeDefinitionByVariable(const XmlNodeRef& definitionRoot, IVariable* pRootVar, IVariable* pVar)
{
	if (!definitionRoot)
	{
		return XmlNodeRef();
	}

	if (pRootVar == NULL)
	{
		return XmlNodeRef();
	}

	if (pVar == NULL)
	{
		return XmlNodeRef();
	}

	std::vector<IVariable*> varHirerarchy;
	bool varFoundInHirerarchy = GetVariableHierarchy(pRootVar, pVar, varHirerarchy);
	if (!varFoundInHirerarchy)
	{
		return XmlNodeRef();
	}

	XmlNodeRef currentNode = definitionRoot;
	for (i32 i = 1; i < varHirerarchy.size(); ++i)
	{
		IVariable* pCurrentVar = varHirerarchy[i];
		tukk nameToSearch = pCurrentVar->GetName();

		currentNode = FindChildDefinitionByName(currentNode, nameToSearch);
		if (!currentNode)
		{
			return XmlNodeRef();
		}
	}

	return currentNode;
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::GetXmlNodeChildDefinitionsByVariable(const XmlNodeRef& definitionRoot, IVariable* pRootVar, IVariable* pVar, std::vector<XmlNodeRef>& childDefinitionsOut)
{
	assert(definitionRoot);
	if (!definitionRoot)
	{
		return;
	}

	assert(pRootVar != NULL);
	if (pRootVar == NULL)
	{
		return;
	}

	assert(pVar != NULL);
	if (pVar == NULL)
	{
		return;
	}

	XmlNodeRef varDefinitionNode = GetXmlNodeDefinitionByVariable(definitionRoot, pRootVar, pVar);
	if (!varDefinitionNode)
	{
		return;
	}

	if (IsArrayNode(varDefinitionNode))
	{
		// Array parent properties cannot have child property definitions but they store the definitions of the
		// elements they can have so we have to check if we're looking at the elementNode or the parent...
		bool isArrayParent = HasNodeNameEqualTo(varDefinitionNode, pVar->GetName());
		if (isArrayParent)
		{
			return;
		}
	}

	if (IsPropertyNode(varDefinitionNode))
	{
		// Property nodes shouldn't have child property definitions.
		return;
	}

	for (i32 i = 0; i < varDefinitionNode->getChildCount(); ++i)
	{
		XmlNodeRef childNode = varDefinitionNode->getChild(i);

		if (IsUseNode(childNode))
		{
			// Use nodes will only reference a definition by id, so we must obtain the real definition.
			tukk childNodeId = GetNodeId(childNode);
			childNode = GetXmlNodeDefinitionById(definitionRoot, childNodeId);
		}

		if (childNode)
		{
			childDefinitionsOut.push_back(childNode);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef VehicleXml::GetXmlNodeChildDefinitionsByName(const XmlNodeRef& definitionRoot, tukk childName, IVariable* pRootVar, IVariable* pVar)
{
	assert(childName != NULL);
	if (childName == NULL)
	{
		return XmlNodeRef();
	}

	std::vector<XmlNodeRef> childDefinitions;
	GetXmlNodeChildDefinitionsByVariable(definitionRoot, pRootVar, pVar, childDefinitions);

	for (i32 i = 0; i < childDefinitions.size(); ++i)
	{
		XmlNodeRef childNode = childDefinitions[i];
		if (HasNodeNameEqualTo(childNode, childName))
		{
			return childNode;
		}
	}

	return XmlNodeRef();
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::GetXmlNodeFromVariable(IVariable* pVar, XmlNodeRef& xmlNode)
{
	if (pVar->GetType() == IVariable::ARRAY)
	{
		for (i32 i = 0; i < pVar->GetNumVariables(); ++i)
		{
			IVariable* pTempVar = pVar->GetVariable(i);
			XmlNodeRef pTempNode = GetISystem()->CreateXmlNode(pTempVar->GetName());

			if (pTempVar->GetType() == IVariable::ARRAY)
			{
				for (i32 j = 0; j < pTempVar->GetNumVariables(); ++j)
				{
					GetXmlNodeFromVariable(pTempVar->GetVariable(j), pTempNode);
				}
			}
			else
			{
				pTempNode->setAttr(pTempVar->GetName(), pTempVar->GetDisplayValue());
			}
			xmlNode->addChild(pTempNode);
		}
	}
	else
	{
		xmlNode->setAttr(pVar->GetName(), pVar->GetDisplayValue());
	}
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateVar(tukk value, const XmlNodeRef& definition)
{
	XmlNodeRef enumNode = definition->findChild("Enum");

	if (!enumNode && !definition->haveAttr("list"))
	{
		return new CVariable<string>();
	}

	CVariableEnum<string>* pVarEnum = new CVariableEnum<string>();
	// for non strict mode, add empty value and the current value in case it is not exposed on the vehicle definition
	// test that the current value is not duplicated with empty or one of the enum values
	if (strcmpi("", value) != 0)
	{
		pVarEnum->AddEnumItem("", "");
	}
	pVarEnum->AddEnumItem(value, value);

	if (enumNode)
	{
		for (i32 i = 0; i < enumNode->getChildCount(); ++i)
		{
			tukk itemName = enumNode->getChild(i)->getContent();
			if (strcmpi(itemName, value) != 0)
			{
				pVarEnum->AddEnumItem(itemName, itemName);
			}
		}
	}
	else
	{
		tukk listType = definition->getAttr("list");
		if (strcmpi(listType, "Helper") == 0)
		{
			pVarEnum->SetDataType(IVariable::DT_VEEDHELPER);
		}
		else if (strcmpi(listType, "Part") == 0)
		{
			pVarEnum->SetDataType(IVariable::DT_VEEDPART);
		}
		else if (strcmpi(listType, "Component") == 0)
		{
			pVarEnum->SetDataType(IVariable::DT_VEEDCOMP);
		}
	}

	return pVarEnum;
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateSimpleVar(const XmlNodeRef& definition)
{
	tukk type = "";
	if (type = definition->getAttr("type"))
	{
		if (strcmpi(type, "float") == 0)
		{
			return CreateVar<float>(0.f, definition);
		}
		else if (strcmpi(type, 'bool') == 0)
		{
			return CreateVar<bool>(true, definition);
		}
		else if (strcmpi(type, "Vec3") == 0)
		{
			return CreateVar<Vec3>(Vec3(0, 0, 0), definition);
		}
		else if (strcmpi(type, "i32") == 0)
		{
			return CreateVar<i32>(0, definition);
		}
		else
		{
			return CreateVar("", definition);
		}
	}
	else
	{
		return CreateVar("", definition);
	}
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateSimpleVar(tukk attributeName, tukk attributeValue, const XmlNodeRef& definition)
{
	IVariable* pVar = CreateSimpleVar(definition);
	if (pVar)
	{
		pVar->SetName(attributeName);
		pVar->Set(attributeValue);
		SetExtendedVarProperties(pVar, definition);
	}
	return pVar;
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateSimpleVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition)
{
	return CreateSimpleVar(definition);
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateSimpleArrayVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition)
{
	return new CVariableArray();
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateTableVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition)
{
	IVariable* pVar = new CVariableArray();

	std::vector<XmlNodeRef> childDefinitions;
	GetXmlNodeChildDefinitions(definitionRoot, definition, childDefinitions);
	for (i32 i = 0; i < childDefinitions.size(); ++i)
	{
		XmlNodeRef childDefinition = childDefinitions[i];

		bool create = false;
		create |= !IsOptionalNode(childDefinition);

		if (create)
		{
			IVariablePtr pChildVar = CreateDefaultVar(definitionRoot, childDefinition, GetNodeName(childDefinition));
			pVar->AddVariable(pChildVar);
		}
	}
	return pVar;
}

//////////////////////////////////////////////////////////////////////////
IVariable* VehicleXml::CreateDefaultVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition, tukk varName)
{
	assert(definitionRoot);
	if (!definitionRoot)
	{
		return NULL;
	}

	assert(definition);
	if (!definition)
	{
		return NULL;
	}

	assert(varName != NULL);
	if (varName == NULL)
	{
		return NULL;
	}

	IVariable* pVar = NULL;
	if (IsPropertyNode(definition))
	{
		pVar = CreateSimpleVar(definitionRoot, definition);
	}
	else if (IsArrayNode(definition))
	{
		if (IsArrayParentNode(definition, varName))
		{
			pVar = CreateSimpleArrayVar(definitionRoot, definition);
		}
		else if (IsArrayElementNode(definition, varName))
		{
			bool isSimpleArrayType = (definition->getChildCount() == 0);
			if (isSimpleArrayType)
			{
				pVar = CreateSimpleVar(definitionRoot, definition);
			}
			else
			{
				pVar = CreateTableVar(definitionRoot, definition);
			}
		}
	}
	else
	{
		pVar = CreateTableVar(definitionRoot, definition);
	}

	if (pVar)
	{
		pVar->SetName(varName);
		SetExtendedVarProperties(pVar, definition);
		return pVar;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::SetExtendedVarProperties(IVariable* pVar, const XmlNodeRef& node)
{
	assert(pVar != NULL);
	if (pVar == NULL)
	{
		return;
	}

	assert(node);
	if (!node)
	{
		return;
	}

	pVar->SetUserData(NULL);

	{
		float min;
		float max;
		pVar->GetLimits(min, max);

		node->getAttr("min", min);
		node->getAttr("max", max);
		pVar->SetLimits(min, max);
	}

	{
		string description;
		if (node->getAttr("desc", description))
		{
			pVar->SetDescription(description);
		}
	}

	{
		i32 deprecated = 0;
		node->getAttr("deprecated", deprecated);
		if (deprecated == 1)
		{
			pVar->SetFlags(IVariable::UI_DISABLED);
		}
	}

	tukk varName = pVar->GetName();
	if (IsArrayParentNode(node, varName))
	{
		i32 extendable = 0;
		node->getAttr("extendable", extendable);

		if (extendable == 1)
		{
			pVar->SetDataType(IVariable::DT_EXTARRAY);

			XmlNodeRef rootNode = GetRootXmlNode(node);
			tukk elementName = GetNodeElementName(node);
			IVariable* pChild = VehicleXml::CreateDefaultVar(rootNode, node, elementName);
			pChild->AddRef(); // For this reason we must clean up later...
			pVar->SetUserData(pChild);
		}
	}
	else if (strcmpi(varName, "filename") == 0 || strcmpi(varName, "filenameDestroyed") == 0)
	{
		pVar->SetDataType(IVariable::DT_FILE);
	}
	else if (strcmpi(varName, "sound") == 0)
	{
		pVar->SetDataType(IVariable::DT_AUDIO_TRIGGER);
	}
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::CleanUp(IVariable* pVar)
{
	std::vector<uk> deletedPointers;
	VehicleXml::CleanUpExecute(pVar, deletedPointers);
}

//////////////////////////////////////////////////////////////////////////
void VehicleXml::CleanUpExecute(IVariable* pVar, std::vector<uk>& deletedPointers)
{
	if (pVar == NULL)
	{
		return;
	}

	for (i32 i = 0; i < pVar->GetNumVariables(); ++i)
	{
		CleanUpExecute(pVar->GetVariable(i), deletedPointers);
	}

	if (pVar->GetDataType() == IVariable::DT_EXTARRAY)
	{
		uk pUserData = pVar->GetUserData();
		if (std::find(deletedPointers.begin(), deletedPointers.end(), pUserData) == deletedPointers.end())
		{
			deletedPointers.push_back(pUserData);
			IVariable* pChild = static_cast<IVariable*>(pUserData);
			pChild->Release();
		}
		pVar->SetUserData(NULL);
	}
}

/*
   =========================================================
   DefinitionTable: Implementation
   =========================================================
 */

DefinitionTable::TXmlNodeMap DefinitionTable::g_useDefinitionMap;

void DefinitionTable::ReloadUseReferenceTables(XmlNodeRef definition)
{
	g_useDefinitionMap.clear();
	GetUseReferenceTables(definition);
}

void DefinitionTable::GetUseReferenceTables(XmlNodeRef definition)
{
	for (i32 i = 0; i < definition->getChildCount(); i++)
	{
		XmlNodeRef propertyDef = definition->getChild(i);
		tukk propertyType = propertyDef->getTag();
		bool isUseReference = stricmp(propertyType, "Use") == 0;

		if (propertyDef->haveAttr("id") && !isUseReference)
		{
			// Insert into the use reference library
			tukk id = propertyDef->getAttr("id");
			if (g_useDefinitionMap.find(id) != g_useDefinitionMap.end())
			{
				DrxWarning(EValidatorModule::VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Duplicate ID found: '%s'", id);
			}
			else
			{
				g_useDefinitionMap[id] = propertyDef;
			}
		}

		GetUseReferenceTables(propertyDef);
	}
}

void DefinitionTable::Create(XmlNodeRef definition)
{
	if (!definition)
		return;

	for (i32 i = 0; i < definition->getChildCount(); i++)
	{
		XmlNodeRef propertyDef = definition->getChild(i);
		tukk propertyType = propertyDef->getTag();
		bool isUseReference = stricmp(propertyType, "Use") == 0;
		if (isUseReference)
		{
			TXmlNodeMap::iterator iter = g_useDefinitionMap.find(propertyDef->getAttr("id"));
			if (iter == g_useDefinitionMap.end())
			{
				DrxLog("Veed Warning: No definition with id '%s' found", propertyDef->getAttr("id"));
				continue;
			}
			propertyDef = iter->second;
		}

		if (propertyDef->haveAttr("name"))
		{
			tukk propertyName = propertyDef->getAttr("name");
			m_definitionList.insert(TXmlNodeMap::value_type(string(propertyName), propertyDef));
		}
	}
}

XmlNodeRef DefinitionTable::GetDefinition(tukk definitionName)
{
	TXmlNodeMap::iterator it = m_definitionList.find(definitionName);
	if (it != m_definitionList.end())
		return it->second;
	return 0;
}

XmlNodeRef DefinitionTable::GetPropertyDefinition(tukk attributeName)
{
	if (XmlNodeRef def = GetDefinition(attributeName))
		if (stricmp(def->getTag(), "Property") == 0)
			return def;
	return 0;
}

bool DefinitionTable::IsArray(XmlNodeRef def)
{
	return (stricmp(def->getTag(), "Array") == 0) && def->haveAttr("elementName");
}

bool DefinitionTable::IsTable(XmlNodeRef def)
{
	return (stricmp(def->getTag(), "Table") == 0);
}

void DefinitionTable::Dump()
{
	for (TXmlNodeMap::iterator it = m_definitionList.begin(); it != m_definitionList.end(); ++it)
	{
		XmlNodeRef def = it->second;
		tukk name = def->getAttr("name");
		tukk type = def->getTag();
		DrxLog("## %s is a %s", (tukk)it->first, name, type);
	}
}

