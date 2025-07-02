// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XMLScriptLoader.h>
#include <drx3D/Sys/IReadWriteXMLSink.h>
#include <stack>

/*
 * Load an XML file to a script table
 */

class CXmlScriptLoad : public IReadXMLSink
{
public:
	CXmlScriptLoad();

	SmartScriptTable GetTable() { DRX_ASSERT(m_tableStack.size() == 1); return m_tableStack.top(); }

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

	virtual bool            IsCreationMode()                       { return false; }
	virtual void            SetCreationNode(XmlNodeRef definition) {}
	virtual XmlNodeRef      GetCreationNode()                      { return XmlNodeRef(0); }

	virtual bool            Complete();
	// ~IReadXMLSink

private:
	i32                          m_nRefs;
	IScriptSystem*               m_pSS;
	std::stack<SmartScriptTable> m_tableStack;

	IScriptTable* CurTable() { DRX_ASSERT(!m_tableStack.empty()); return m_tableStack.top().GetPtr(); }

	class SSetValueVisitor
	{
	public:
		SSetValueVisitor(IScriptTable* pTable, tukk name) : m_pTable(pTable), m_name(name) {}

		template<class T>
		void Visit(const T& value)
		{
			m_pTable->SetValue(m_name, value);
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

		IScriptTable* m_pTable;
		tukk   m_name;
	};
	class SSetValueAtVisitor
	{
	public:
		SSetValueAtVisitor(IScriptTable* pTable, i32 elem) : m_pTable(pTable), m_elem(elem) {}

		template<class T>
		void Visit(const T& value)
		{
			m_pTable->SetAt(m_elem, value);
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

		IScriptTable* m_pTable;
		i32           m_elem;
	};
};
template<>
void CXmlScriptLoad::SSetValueVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(const SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}
template<>
void CXmlScriptLoad::SSetValueAtVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(const SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

TYPEDEF_AUTOPTR(CXmlScriptLoad);
typedef CXmlScriptLoad_AutoPtr CXmlScriptLoadPtr;

CXmlScriptLoad::CXmlScriptLoad() : m_nRefs(0), m_pSS(gEnv->pScriptSystem)
{
	m_tableStack.push(SmartScriptTable(m_pSS));
}

void CXmlScriptLoad::AddRef()
{
	++m_nRefs;
}

void CXmlScriptLoad::Release()
{
	if (0 == --m_nRefs)
		delete this;
}

IReadXMLSinkPtr CXmlScriptLoad::BeginTable(tukk name, const XmlNodeRef& definition)
{
	m_tableStack.push(SmartScriptTable(m_pSS));
	return this;
}

IReadXMLSinkPtr CXmlScriptLoad::BeginTableAt(i32 elem, const XmlNodeRef& definition)
{
	return BeginTable(NULL, definition);
}

bool CXmlScriptLoad::SetValue(tukk name, const TValue& value, const XmlNodeRef& definition)
{
	SSetValueVisitor visitor(CurTable(), name);
	stl::visit(visitor, value);
	return true;
}

bool CXmlScriptLoad::EndTable(tukk name)
{
	SmartScriptTable newTable = CurTable();
	m_tableStack.pop();
	CurTable()->SetValue(name, newTable);
	return true;
}

bool CXmlScriptLoad::EndTableAt(i32 elem)
{
	SmartScriptTable newTable = CurTable();
	m_tableStack.pop();
	CurTable()->SetAt(elem, newTable);
	return true;
}

IReadXMLSinkPtr CXmlScriptLoad::BeginArray(tukk name, const XmlNodeRef& definition)
{
	m_tableStack.push(SmartScriptTable(m_pSS));
	return this;
}

bool CXmlScriptLoad::SetAt(i32 elem, const TValue& value, const XmlNodeRef& definition)
{
	SSetValueAtVisitor visitor(CurTable(), elem);
	stl::visit(visitor, value);
	return true;
}

bool CXmlScriptLoad::EndArray(tukk name)
{
	return EndTable(name);
}

bool CXmlScriptLoad::Complete()
{
	return m_tableStack.size() == 1;
}

SmartScriptTable XmlScriptLoad(tukk definitionFile, XmlNodeRef data)
{
	CXmlScriptLoadPtr pLoader(new CXmlScriptLoad);
	if (GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->ReadXML(definitionFile, data, &*pLoader))
		return pLoader->GetTable();
	else
		return NULL;
}

SmartScriptTable XmlScriptLoad(tukk definitionFile, tukk dataFile)
{
	CXmlScriptLoadPtr pLoader(new CXmlScriptLoad);
	if (GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->ReadXML(definitionFile, dataFile, &*pLoader))
		return pLoader->GetTable();
	else
		return NULL;
}

/*
 * Save an XML file from a script table
 */

class CXmlScriptSaver : public IWriteXMLSource
{
public:
	CXmlScriptSaver(SmartScriptTable pTable);

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
	i32                          m_nRefs;
	std::stack<SmartScriptTable> m_tables;

	IScriptTable* CurTable() { return m_tables.top().GetPtr(); }
};

TYPEDEF_AUTOPTR(CXmlScriptSaver);
typedef CXmlScriptSaver_AutoPtr CXmlScriptSaverPtr;

void CXmlScriptSaver::AddRef()
{
	++m_nRefs;
}

void CXmlScriptSaver::Release()
{
	if (0 == --m_nRefs)
		delete this;
}

IWriteXMLSourcePtr CXmlScriptSaver::BeginTable(tukk name)
{
	SmartScriptTable childTable;
	if (!CurTable()->GetValue(name, childTable))
		return NULL;
	m_tables.push(childTable);
	return this;
}

IWriteXMLSourcePtr CXmlScriptSaver::BeginTableAt(i32 elem)
{
	SmartScriptTable childTable;
	if (!CurTable()->GetAt(elem, childTable))
		return NULL;
	m_tables.push(childTable);
	return this;
}

IWriteXMLSourcePtr CXmlScriptSaver::BeginArray(tukk name, size_t* numElems, const XmlNodeRef& definition)
{
	SmartScriptTable childTable;
	if (!CurTable()->GetValue(name, childTable))
		return NULL;
	*numElems = childTable->Count();
	m_tables.push(childTable);
	return this;
}

bool CXmlScriptSaver::EndTable(tukk name)
{
	m_tables.pop();
	return true;
}

bool CXmlScriptSaver::EndTableAt(i32 elem)
{
	return EndTable(NULL);
}

bool CXmlScriptSaver::EndArray(tukk name)
{
	return EndTable(name);
}

namespace
{

struct CGetValueVisitor
{
public:
	CGetValueVisitor(IScriptTable* pTable, tukk name) : m_ok(false), m_pTable(pTable), m_name(name) {}

	template<class T>
	void Visit(T& value)
	{
		m_ok = m_pTable->GetValue(m_name, value);
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

	bool          m_ok;
	IScriptTable* m_pTable;
	tukk   m_name;
};
template<>
void CGetValueVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

struct CGetAtVisitor
{
public:
	CGetAtVisitor(IScriptTable* pTable, i32 elem) : m_ok(false), m_pTable(pTable), m_elem(elem) {}

	template<class T>
	void Visit(T& value)
	{
		m_ok = m_pTable->GetAt(m_elem, value);
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

	bool          m_ok;
	IScriptTable* m_pTable;
	i32           m_elem;
};
template<>
void CGetAtVisitor::VisitVariant<stl::variant_size<SReadWriteXMLCommon::TValue>::value>(SReadWriteXMLCommon::TValue& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}
}

bool CXmlScriptSaver::GetValue(tukk name, TValue& value, const XmlNodeRef& definition)
{
	CGetValueVisitor visitor(CurTable(), name);
	stl::visit(visitor, value);
	return visitor.Ok();
}

bool CXmlScriptSaver::GetAt(i32 elem, TValue& value, const XmlNodeRef& definition)
{
	CGetAtVisitor visitor(CurTable(), elem);
	stl::visit(visitor, value);
	return visitor.Ok();
}

bool CXmlScriptSaver::HaveElemAt(i32 elem)
{
	ScriptAnyValue value;
	if (CurTable()->GetAtAny(elem, value))
		if (value.GetVarType() != svtNull)
			return true;
	return false;
}

bool CXmlScriptSaver::HaveValue(tukk name)
{
	ScriptAnyValue value;
	if (CurTable()->GetValueAny(name, value))
		if (value.GetVarType() != svtNull)
			return true;
	return false;
}

bool CXmlScriptSaver::Complete()
{
	return true;
}

CXmlScriptSaver::CXmlScriptSaver(SmartScriptTable pTable) : m_nRefs(0)
{
	m_tables.push(pTable);
}

XmlNodeRef XmlScriptSave(tukk definitionFile, SmartScriptTable scriptTable)
{
	CXmlScriptSaverPtr pSaver(new CXmlScriptSaver(scriptTable));
	return GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->CreateXMLFromSource(definitionFile, &*pSaver);
}

bool XmlScriptSave(tukk definitionFile, tukk dataFile, SmartScriptTable scriptTable)
{
	CXmlScriptSaverPtr pSaver(new CXmlScriptSaver(scriptTable));
	return GetISystem()->GetXmlUtils()->GetIReadWriteXMLSink()->WriteXML(definitionFile, dataFile, &*pSaver);
}
