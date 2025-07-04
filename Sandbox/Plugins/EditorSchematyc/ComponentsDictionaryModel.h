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

class CComponentDictionaryEntry : public CAbstractDictionaryEntry
{
	friend class CComponentsDictionary;

public:
	CComponentDictionaryEntry();
	virtual ~CComponentDictionaryEntry();

	// CAbstractDictionaryEntry
	virtual u32   GetType() const override { return CAbstractDictionaryEntry::Type_Entry; }

	virtual QVariant GetColumnValue(i32 columnIndex) const override;
	virtual QString  GetToolTip() const override;
	// ~CAbstractDictionaryEntry

	QString          GetName() const { return m_name; }
	DrxGUID GetTypeGUID()   { return m_identifier; }

private:
	DrxGUID m_identifier;
	QString          m_name;
	QString          m_fullName;
	QString          m_description;
	QIcon            m_icon;
};

class CComponentsDictionary : public CAbstractDictionary
{
public:
	enum EColumn : i32
	{
		Column_Name,

		Column_COUNT
	};

public:
	CComponentsDictionary(const Schematyc::IScriptElement* pScriptScope = nullptr);
	virtual ~CComponentsDictionary();

	// DrxGraphEditor::CAbstractDictionary
	virtual i32                           GetNumEntries() const override { return m_components.size(); }
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override;

	virtual i32                           GetNumColumns() const override { return Column_COUNT; };
	virtual QString                         GetColumnName(i32 index) const override;

	virtual i32                           GetDefaultFilterColumn() const override { return Column_Name; }
	virtual i32                           GetDefaultSortColumn() const override { return Column_Name; }
	// ~DrxGraphEditor::CAbstractDictionary

	void Load(const Schematyc::IScriptElement* pScriptScope);

private:
	std::vector<CComponentDictionaryEntry> m_components;
};

}

