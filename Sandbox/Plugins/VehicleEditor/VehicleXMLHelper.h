// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLE_XML_HELPER__H__
#define __VEHICLE_XML_HELPER__H__

struct IVariable;

namespace VehicleXml
{

bool        IsUseNode(const XmlNodeRef& node);
bool        IsArrayNode(const XmlNodeRef& node);
bool        IsArrayElementNode(const XmlNodeRef& node, tukk name);
bool        IsArrayParentNode(const XmlNodeRef& node, tukk name);
bool        IsPropertyNode(const XmlNodeRef& node);
bool        IsOptionalNode(const XmlNodeRef& node);
bool        IsDeprecatedNode(const XmlNodeRef& node);

bool        HasElementNameEqualTo(const XmlNodeRef& node, tukk name);
bool        HasNodeIdEqualTo(const XmlNodeRef& node, tukk id);
bool        HasNodeNameEqualTo(const XmlNodeRef& node, tukk name);

tukk GetNodeId(const XmlNodeRef& node);
tukk GetNodeElementName(const XmlNodeRef& node);
tukk GetNodeName(const XmlNodeRef& node);

XmlNodeRef  GetXmlNodeDefinitionById(const XmlNodeRef& definitionRoot, tukk id);
XmlNodeRef  GetXmlNodeDefinitionByVariable(const XmlNodeRef& definitionRoot, IVariable* pRootVar, IVariable* pVar);

bool        GetVariableHierarchy(IVariable* pRootVar, IVariable* pVar, std::vector<IVariable*>& varHierarchyOut);

void        GetXmlNodeFromVariable(IVariable* pVar, XmlNodeRef& xmlNode);
void        GetXmlNodeChildDefinitions(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition, std::vector<XmlNodeRef>& childDefinitionsOut);
void        GetXmlNodeChildDefinitionsByVariable(const XmlNodeRef& definitionRoot, IVariable* pRootVar, IVariable* pVar, std::vector<XmlNodeRef>& childDefinitionsOut);
XmlNodeRef  GetXmlNodeChildDefinitionsByName(const XmlNodeRef& definitionRoot, tukk childName, IVariable* pRootVar, IVariable* pVar);

IVariable*  CreateDefaultVar(const XmlNodeRef& definitionRoot, const XmlNodeRef& definition, tukk varName);

void        SetExtendedVarProperties(IVariable* pVar, const XmlNodeRef& node);

template<class T>
IVariable* CreateVar(T value, const XmlNodeRef& definition)
{
	return new CVariable<T>;
}

IVariable* CreateVar(tukk value, const XmlNodeRef& definition);

IVariable* CreateSimpleVar(tukk attributeName, tukk attributeValue, const XmlNodeRef& definition);

void       CleanUp(IVariable* pVar);
}

/*
   =========================================================
   DefinitionTable
   =========================================================
 */

struct DefinitionTable
{
	// Build a list of definitions against their name for easy searching
	typedef std::map<string, XmlNodeRef> TXmlNodeMap;

	static TXmlNodeMap g_useDefinitionMap;

	TXmlNodeMap        m_definitionList;

	static void ReloadUseReferenceTables(XmlNodeRef definition);
	void        Create(XmlNodeRef definitionNode);
	XmlNodeRef  GetDefinition(tukk definitionName);
	XmlNodeRef  GetPropertyDefinition(tukk attributeName);
	bool        IsArray(XmlNodeRef def);
	bool        IsTable(XmlNodeRef def);
	void        Dump();

private:
	static void GetUseReferenceTables(XmlNodeRef definition);
};

#endif

