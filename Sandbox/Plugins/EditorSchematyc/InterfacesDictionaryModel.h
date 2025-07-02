// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <Controls/DictionaryWidget.h>

#include <QVariant>
#include <QString>
#include <QIcon>

namespace Schematyc {

struct IScriptView;
struct IScriptRegistry;

}

namespace DrxSchematycEditor {

class CInterfaceDictionaryEntry : public CAbstractDictionaryEntry
{
	friend class CInterfacesDictionary;

public:
	CInterfaceDictionaryEntry();
	virtual ~CInterfaceDictionaryEntry();

	// CAbstractDictionaryEntry
	virtual u32   GetType() const override { return CAbstractDictionaryEntry::Type_Entry; }

	virtual QVariant GetColumnValue(i32 columnIndex) const override;
	virtual QString  GetToolTip() const override;
	// ~CAbstractDictionaryEntry

	QString            GetName() const    { return m_name; }
	Schematyc::DrxGUID   GetInterfaceGUID() { return m_identifier; }
	Schematyc::EDomain GetDomain()        { return m_domain; }

private:
	Schematyc::DrxGUID	m_identifier;
	QString            m_name;
	QString            m_fullName;
	QString            m_description;
	Schematyc::EDomain m_domain;
};

class CInterfacesDictionary : public CAbstractDictionary
{
public:
	enum EColumn : i32
	{
		Column_Name,

		Column_COUNT
	};

public:
	CInterfacesDictionary(const Schematyc::IScriptElement* pScriptScope = nullptr);
	virtual ~CInterfacesDictionary();

	// DrxGraphEditor::CAbstractDictionary
	virtual i32                           GetNumEntries() const override { return m_interfaces.size(); }
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override;

	virtual i32                           GetNumColumns() const override { return Column_COUNT; };
	virtual QString                         GetColumnName(i32 index) const override;

	virtual i32                           GetDefaultFilterColumn() const override { return Column_Name; }
	// ~DrxGraphEditor::CAbstractDictionary

private:
	std::vector<CInterfaceDictionaryEntry> m_interfaces;
};

}

