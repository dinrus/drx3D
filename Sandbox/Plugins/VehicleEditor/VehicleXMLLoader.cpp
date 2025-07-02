// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "VehicleXMLLoader.h"

#include <stack>
#include <DrxSystem/XML/IReadWriteXMLSink.h>

#include "VehicleData.h"
#include "VehicleXMLHelper.h"

/*
 * Implementation of IReadXMLSink, creates IVehicleData structure
 */
class CVehicleDataLoader : public IReadXMLSink
{
public:
	CVehicleDataLoader();

	IVehicleData* GetVehicleData() { return m_pData; }

	// IReadXMLSink
	virtual void            AddRef();
	virtual void            Release();

	virtual IReadXMLSinkPtr BeginTable(tukk name, const XmlNodeRef& definition);
	virtual IReadXMLSinkPtr BeginTableAt(i32 elem, const XmlNodeRef& definition);
	virtual bool            SetValue(tukk name, const TValue& value, const XmlNodeRef& definition);
	virtual bool            EndTableAt(i32 elem);
	virtual bool            EndTable(tukk name);

	virtual IReadXMLSinkPtr BeginArray(tukk name, const XmlNodeRef& definition);
	virtual bool            SetAt(i32 elem, const TValue& value, const XmlNodeRef& definition);
	virtual bool            EndArray(tukk name);

	virtual bool            Complete();

	virtual bool            IsCreationMode()                       { return m_creationNode != 0; }
	virtual XmlNodeRef      GetCreationNode()                      { return m_creationNode; }
	virtual void            SetCreationNode(XmlNodeRef definition) { m_creationNode = definition; }
	// ~IReadXMLSink

private:
	i32                      m_nRefs;
	IVehicleData*            m_pData;
	std::stack<IVariablePtr> m_nodeStack;
	XmlNodeRef               m_creationNode;

	IVariablePtr CurNode() { assert(!m_nodeStack.empty()); return m_nodeStack.top(); }

	class SSetValueVisitor
	{
	public:
		SSetValueVisitor(IVariablePtr parent, tukk name, const XmlNodeRef& definition, IReadXMLSink* sink)
			: m_parent(parent)
			, m_name(name)
			, m_def(definition)
			, m_sink(sink)
		{}

		template<class T> void Visit(const T& value)
		{
			IVariablePtr node = VehicleXml::CreateVar(value, m_def);
			DoVisit(value, node);
		}

		template<class T>
		void operator()(const T& type)
		{
			Visit(type);
		}

		void operator()(const SReadWriteXMLCommon::TValue& var)
		{
			VisitVariant(var);
		}

	private:
		template<class T> void DoVisit(const T& value, IVariablePtr node)
		{
			node->SetName(m_name);
			node->Set(value);

			VehicleXml::SetExtendedVarProperties(node, m_def);

			m_parent->AddVariable(node);
		}

		template<size_t I = 0>
		void VisitVariant(const SReadWriteXMLCommon::TValue& var)
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

		IVariablePtr      m_parent;
		tukk       m_name;
		const XmlNodeRef& m_def;
		IReadXMLSink*     m_sink;
	};

	class SSetValueAtVisitor
	{
	public:
		SSetValueAtVisitor(IVariablePtr parent, i32 elem, const XmlNodeRef& definition, IReadXMLSink* sink)
			: m_parent(parent)
			, m_elem(elem)
			, m_def(definition)
			, m_sink(sink)
		{}

		template<class T> void Visit(const T& value)
		{
			IVariablePtr node = VehicleXml::CreateVar(value, m_def);
			DoVisit(value, node);
		}

		template<class T>
		void operator()(const T& type)
		{
			Visit(type);
		}
		
		void operator()(const SReadWriteXMLCommon::TValue& var)
		{
			VisitVariant(var);
		}

	private:
		template<class T> void DoVisit(const T& value, IVariablePtr node)
		{
			string s;
			//s.Format(_T("%d"), m_elem);
			s = m_def->getAttr("elementName");

			node->SetName(s);
			node->Set(value);

			VehicleXml::SetExtendedVarProperties(node, m_def);

			m_parent->AddVariable(node);
		}

		template<size_t I = 0>
		void VisitVariant(const SReadWriteXMLCommon::TValue& var)
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

		IVariablePtr      m_parent;
		i32               m_elem;
		const XmlNodeRef& m_def;
		IReadXMLSink*     m_sink;
	};
};
template<>
void CVehicleDataLoader::SSetValueVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(const SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}
template<>
void CVehicleDataLoader::SSetValueAtVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(const SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

TYPEDEF_AUTOPTR(CVehicleDataLoader);
typedef CVehicleDataLoader_AutoPtr CVehicleDataLoaderPtr;

CVehicleDataLoader::CVehicleDataLoader()
	: m_nRefs(0)
	, m_creationNode(0)
{
	m_pData = new CVehicleData;
	m_nodeStack.push(m_pData->GetRoot());
}

void CVehicleDataLoader::AddRef()
{
	++m_nRefs;
}

void CVehicleDataLoader::Release()
{
	if (0 == --m_nRefs)
		delete this;
}

IReadXMLSinkPtr CVehicleDataLoader::BeginTable(tukk name, const XmlNodeRef& definition)
{
	CVariableArray* arr = new CVariableArray;
	arr->SetName(name);
	m_nodeStack.push(arr);

	return this;
}

IReadXMLSinkPtr CVehicleDataLoader::BeginTableAt(i32 elem, const XmlNodeRef& definition)
{
	CVariableArray* arr = new CVariableArray;

	string s;
	//s.Format(_T("%d"), elem);
	s = definition->getAttr("elementName");

	arr->SetName(s);
	m_nodeStack.push(arr);

	return this;
}

bool CVehicleDataLoader::SetValue(tukk name, const TValue& value, const XmlNodeRef& definition)
{
	SSetValueVisitor visitor(CurNode(), name, definition, this);
	stl::visit(visitor, value);
	return true;
}

bool CVehicleDataLoader::EndTable(tukk name)
{
	IVariablePtr newNode = CurNode();
	m_nodeStack.pop();
	CurNode()->AddVariable(newNode);   // add child
	return true;
}

bool CVehicleDataLoader::EndTableAt(i32 elem)
{
	return EndTable(0);
}

IReadXMLSinkPtr CVehicleDataLoader::BeginArray(tukk name, const XmlNodeRef& definition)
{
	IVariablePtr arr = new CVariableArray;
	arr->SetName(name);

	VehicleXml::SetExtendedVarProperties(arr, definition);

	m_nodeStack.push(arr);
	return this;
}

bool CVehicleDataLoader::SetAt(i32 elem, const TValue& value, const XmlNodeRef& definition)
{
	SSetValueAtVisitor visitor(CurNode(), elem, definition, this);
	stl::visit(visitor, value);
	return true;
}

bool CVehicleDataLoader::EndArray(tukk name)
{
	return EndTable(name);
}

bool CVehicleDataLoader::Complete()
{
	return true;
}

IVehicleData* VehicleDataLoad(tukk definitionFile, tukk dataFile, bool bFillDefaults)
{
	XmlNodeRef definition = GetISystem()->LoadXmlFromFile(definitionFile);
	if (!definition)
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "VehicleDataLoad: unable to load definition file %s", definitionFile);
		return 0;
	}
	return VehicleDataLoad(definition, dataFile, bFillDefaults);
}

IVehicleData* VehicleDataLoad(const XmlNodeRef& definition, tukk dataFile, bool bFillDefaults)
{
	CVehicleDataLoaderPtr pLoader(new CVehicleDataLoader);
	if (GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->ReadXML(definition, dataFile, &*pLoader))
	{
		return pLoader->GetVehicleData();
	}
	else
	{
		return NULL;
	}
}

IVehicleData* VehicleDataLoad(const XmlNodeRef& definition, const XmlNodeRef& data, bool bFillDefaults)
{
	CVehicleDataLoaderPtr pLoader(new CVehicleDataLoader);
	if (GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->ReadXML(definition, data, &*pLoader))
	{
		return pLoader->GetVehicleData();
	}
	else
	{
		return NULL;
	}
}

//
// end of CVehicleDataLoader implementation

