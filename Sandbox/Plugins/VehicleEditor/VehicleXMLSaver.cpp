// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "VehicleXMLSaver.h"

#include <stack>
#include <DrxSystem/XML/IReadWriteXMLSink.h>
#include "VehicleXMLHelper.h"

#include "VehicleData.h"

/*
 * Implementation of IWriteXMLSource, writes IVehicleData structure
 */
class CVehicleDataSaver : public IWriteXMLSource
{
public:
	CVehicleDataSaver();
	CVehicleDataSaver(IVariablePtr pNode);

	// IWriteXMLSource
	virtual void               AddRef();
	virtual void               Release();

	virtual IWriteXMLSourcePtr BeginTable(tukk name);
	virtual IWriteXMLSourcePtr BeginTableAt(i32 elem);
	virtual bool               HaveValue(tukk name);
	virtual bool               GetValue(tukk name, TValue& value, const XmlNodeRef& definition);
	virtual bool               EndTableAt(i32 elem);
	virtual bool               EndTable(tukk name);

	virtual IWriteXMLSourcePtr BeginArray(tukk name, size_t* numElems, const XmlNodeRef& definition);
	virtual bool               HaveElemAt(i32 elem);
	virtual bool               GetAt(i32 elem, TValue& value, const XmlNodeRef& definition);
	virtual bool               EndArray(tukk name);

	virtual bool               Complete();
	// ~IWriteXMLSource

private:
	i32                      m_nRefs;
	std::stack<IVariablePtr> m_nodeStack;

	IVariablePtr CurNode() { assert(!m_nodeStack.empty()); return m_nodeStack.top(); }

};

TYPEDEF_AUTOPTR(CVehicleDataSaver);
typedef CVehicleDataSaver_AutoPtr CVehicleDataSaverPtr;

void CVehicleDataSaver::AddRef()
{
	++m_nRefs;
}

void CVehicleDataSaver::Release()
{
	if (0 == --m_nRefs)
		delete this;
}

IWriteXMLSourcePtr CVehicleDataSaver::BeginTable(tukk name)
{
	IVariablePtr childNode = GetChildVar(CurNode(), name);
	if (!childNode)
		return NULL;
	m_nodeStack.push(childNode);
	return this;
}

IWriteXMLSourcePtr CVehicleDataSaver::BeginTableAt(i32 elem)
{
	IVariablePtr childNode = CurNode()->GetVariable(elem - 1);
	if (!childNode)
		return NULL;
	m_nodeStack.push(childNode);
	return this;
}

IWriteXMLSourcePtr CVehicleDataSaver::BeginArray(tukk name, size_t* numElems, const XmlNodeRef& definition)
{
	IVariablePtr childNode = GetChildVar(CurNode(), name);
	if (!childNode)
		return NULL;
	*numElems = childNode->GetNumVariables();
	m_nodeStack.push(childNode);
	return this;
}

bool CVehicleDataSaver::EndTable(tukk name)
{
	m_nodeStack.pop();
	return true;
}

bool CVehicleDataSaver::EndTableAt(i32 elem)
{
	return EndTable(NULL);
}

bool CVehicleDataSaver::EndArray(tukk name)
{
	return EndTable(name);
}

namespace
{
#define VAL_MIN       0.0002f
#define VAL_PRECISION 4.0f

template<class T> void ClampValue(T& val)
{
}

//! rounding
template<> void ClampValue(float& val)
{
	const static float coeff = pow(10.0f, VAL_PRECISION);
	val = ((float)((long)(val * coeff + sgn(val) * 0.5f))) / coeff;

	if (abs(val) <= VAL_MIN)
		val = 0;
}
template<> void ClampValue(Vec3& val)
{
	for (i32 i = 0; i < 3; ++i)
		ClampValue<float>(val[i]);
}

struct CGetValueVisitor
{
public:
	CGetValueVisitor(IVariablePtr pNode, tukk name) : m_pNode(pNode), m_name(name), m_ok(false) {}

	void Visit(tukk & value)
	{
		static string sVal;
		if (IVariable* child = GetChildVar(m_pNode, m_name))
		{
			child->Get(sVal);
			value = sVal;
			m_ok = true;
		}
	}
	template<class T> void Visit(T& value)
	{
		if (IVariable* child = GetChildVar(m_pNode, m_name))
		{
			child->Get(value);
			ClampValue(value);
			m_ok = true;
		}
	}
	template<class T>
	void operator()(T& type)
	{
		Visit(type);
	}
	void operator()(SReadWriteXMLCommon::TValue& var)
	{
		VisitVariant(var);
	}
	bool Ok()
	{
		return m_ok;
	}
private:
	template<size_t I = 0>
	void VisitVariant(SReadWriteXMLCommon::TValue& var)
	{
		if (var.index() == I)
		{
			Visit(stl::get<I>(var));
		}
		else
		{
			VisitVariant<I + 1>(var);
		}
	}

	bool         m_ok;
	IVariablePtr m_pNode;
	tukk  m_name;
};
template<>
void CGetValueVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

struct CGetAtVisitor
{
public:
	CGetAtVisitor(IVariablePtr pNode, i32 elem) : m_pNode(pNode), m_elem(elem), m_ok(false) {}

	template<class T> void Visit(T& value)
	{
		if (IVariable* child = m_pNode->GetVariable(m_elem - 1))
		{
			child->Get(value);
			ClampValue(value);
			m_ok = true;
		}
	}
	void Visit(tukk & value)
	{
		static string sVal;
		if (IVariable* child = m_pNode->GetVariable(m_elem - 1))
		{
			child->Get(sVal);
			value = sVal;
			m_ok = true;
		}
	}
	template<class T>
	void operator()(T& type)
	{
		Visit(type);
	}
	void operator()(SReadWriteXMLCommon::TValue& var)
	{
		VisitVariant(var);
	}
	bool Ok()
	{
		return m_ok;
	}
private:
	template<size_t I = 0>
	void VisitVariant(SReadWriteXMLCommon::TValue& var)
	{
		if (var.index() == I)
		{
			Visit(stl::get<I>(var));
		}
		else
		{
			VisitVariant<I + 1>(var);
		}
	}

	bool         m_ok;
	IVariablePtr m_pNode;
	i32          m_elem;
};
template<>
void CGetAtVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

}

bool CVehicleDataSaver::GetValue(tukk name, TValue& value, const XmlNodeRef& definition)
{
	CGetValueVisitor visitor(CurNode(), name);
	stl::visit(visitor, value);
	return visitor.Ok();
}

bool CVehicleDataSaver::GetAt(i32 elem, TValue& value, const XmlNodeRef& definition)
{
	CGetAtVisitor visitor(CurNode(), elem);
	stl::visit(visitor, value);
	return visitor.Ok();
}

bool CVehicleDataSaver::HaveElemAt(i32 elem)
{
	if (elem <= CurNode()->GetNumVariables())
		return true;
	return false;
}

bool CVehicleDataSaver::HaveValue(tukk name)
{
	if (GetChildVar(CurNode(), name))
		return true;
	return false;
}

bool CVehicleDataSaver::Complete()
{
	return true;
}

CVehicleDataSaver::CVehicleDataSaver(IVariablePtr pNode) : m_nRefs(0)
{
	m_nodeStack.push(pNode);
}

XmlNodeRef VehicleDataSave(tukk definitionFile, IVehicleData* pData)
{
	CVehicleDataSaverPtr pSaver(new CVehicleDataSaver(pData->GetRoot()));
	return GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->CreateXMLFromSource(definitionFile, &*pSaver);
}

bool VehicleDataSave(tukk definitionFile, tukk dataFile, IVehicleData* pData)
{
	CVehicleDataSaverPtr pSaver(new CVehicleDataSaver(pData->GetRoot()));
	return GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->WriteXML(definitionFile, dataFile, &*pSaver);
}

////////////////////////////////////////////////////////////////////////////////////////////
static i32 GetRequiredPrecision(float f)
{
	i32 width = 4;
	const float precision = 10000.f;

	// Quantise, round, and get the fraction
	f = (float)((i32)((fabsf(f) * precision) + 0.499999f)) / precision;
	i32 decimal = (i32)f;
	i32 ifrac = (i32)(((f - (float)decimal) * precision) + 0.499999f);

	if (ifrac == 0)
		return 0;

	while (ifrac >= 10 && (ifrac % 10) == 0)
	{
		width--;
		ifrac /= 10;
	}
	return width;
}

// Display Name formatted sensibly to minimise xml merge diffs
string GetDisplayValue(IVariablePtr var)
{
#define FFMT(x) GetRequiredPrecision(x), x
	string output;
	switch (var->GetType())
	{
	case IVariable::FLOAT:
		{
			float val;
			var->Get(val);
			output.Format("%.*f", FFMT(val));
			break;
		}
	case IVariable::VECTOR2:
		{
			Vec2 val;
			var->Get(val);
			output.Format("%.*f,%.*f", FFMT(val.x), FFMT(val.y));
			break;
		}
	case IVariable::VECTOR:
		{
			Vec3 val;
			var->Get(val);
			output.Format("%.*f,%.*f,%.*f", FFMT(val.x), FFMT(val.y), FFMT(val.z));
			break;
		}
	case IVariable::VECTOR4:
		{
			Vec4 val;
			var->Get(val);
			output.Format("%.*f,%.*f,%.*f,%.*f", FFMT(val.x), FFMT(val.y), FFMT(val.z), FFMT(val.w));
			break;
		}
	case IVariable::QUAT:
		{
			Quat val;
			var->Get(val);
			output.Format("%.*f,%.*f,%.*f,%.*f", FFMT(val.w), FFMT(val.v.x), FFMT(val.v.y), FFMT(val.v.z));
			break;
		}
	default:
		output = var->GetDisplayValue();
	}
	return output;
#undef FFMT
}

XmlNodeRef findChild(tukk name, XmlNodeRef source, i32& index)
{
	for (; index < source->getChildCount(); )
	{
		XmlNodeRef child = source->getChild(index++);
		if (stricmp(name, child->getTag()) == 0)
			return child;
	}
	return 0;
}

void VehicleDataMerge_ProcessArray(XmlNodeRef source, XmlNodeRef definition, IVariable* array)
{
	if (array->GetType() != IVariable::ARRAY)
		return;

	DefinitionTable definitionList;
	definitionList.Create(definition);
	//definitionList.Dump();

	// We need to keep a search index for each param name type
	// since xmlnode->findChild() would just return the first,
	// which is incorrect
	typedef std::map<string, i32> TMap;
	TMap sourceNodeIndex;

	// Loop over the definition list and initialise the start index
	DefinitionTable::TXmlNodeMap::iterator it = definitionList.m_definitionList.begin();
	for (; it != definitionList.m_definitionList.end(); ++it)
		sourceNodeIndex[it->first] = 0;

	// loop through the vehicle data
	for (i32 i = 0; i < array->GetNumVariables(); i++)
	{
		IVariable* var = array->GetVariable(i);
		tukk dataName = var->GetName();

		XmlNodeRef propertyDef = definitionList.GetDefinition(dataName);
		if (!propertyDef)
			continue;

		if (stricmp(propertyDef->getTag(), "Property") == 0)
		{
			// Has the var changed from the src var?
			if (IVariablePtr srcVar = VehicleXml::CreateSimpleVar(dataName, source->getAttr(dataName), propertyDef))
				if (srcVar->GetDisplayValue() == var->GetDisplayValue())
					continue;

			// Patch/add the data to the source xml
			string dataDisplayName = GetDisplayValue(var);
			source->setAttr(dataName, dataDisplayName);
		}

		// We expect the sub-variable to be an array
		if (var->GetType() != IVariable::ARRAY)
			continue;

		// Load Table
		i32& index = sourceNodeIndex[dataName];
		XmlNodeRef xmlTable = findChild(dataName, source, index);
		if (!xmlTable)
			continue;

		if (definitionList.IsTable(propertyDef))
		{
			VehicleDataMerge_ProcessArray(xmlTable, propertyDef, var);
		}
		else if (definitionList.IsArray(propertyDef))
		{
			tukk elementName = propertyDef->getAttr("elementName");
			IVariable* arrayRoot = var;

			// Delete the block of elements from source and re-export it
			if (arrayRoot->GetNumVariables() != xmlTable->getChildCount())
			{
				xmlTable->removeAllChilds();
				VehicleXml::GetXmlNodeFromVariable(arrayRoot, xmlTable);
			}
			else
			{
				if (propertyDef->haveAttr("type"))
				{
					for (i32 j = 0; j < arrayRoot->GetNumVariables(); j++)
					{
						XmlNodeRef xmlChildElement = xmlTable->getChild(j);
						string dataDisplayName = GetDisplayValue(arrayRoot->GetVariable(j));
						xmlChildElement->setAttr("value", dataDisplayName);
					}
				}
				else
				{
					for (i32 j = 0; j < arrayRoot->GetNumVariables(); j++)
					{
						XmlNodeRef xmlChildElement = xmlTable->getChild(j);
						VehicleDataMerge_ProcessArray(xmlChildElement, propertyDef, arrayRoot->GetVariable(j));
					}
				}
			}
		}
		else
		{
			// Leave the node alone, no need to change the src xml
		}
	}
}

XmlNodeRef VehicleDataMergeAndSave(tukk originalXml, XmlNodeRef definition, IVehicleData* pData)
{
	XmlNodeRef root = GetISystem()->LoadXmlFromFile(originalXml);

	if (!root)
	{
		DrxLog("Vehicle Editor: can't create save-data, as source file %s couldnt be opened", originalXml);
		return 0;
	}

	IVariablePtr pDataRoot = pData->GetRoot();
	if (pDataRoot->GetType() != IVariable::ARRAY)
	{
		DrxLog("Vehicle Editor: can't create save-data");
		return 0;
	}

	// Recursively walk the vehicle data, and patch the source xml
	VehicleDataMerge_ProcessArray(root, definition, pDataRoot);

	return root;
}

