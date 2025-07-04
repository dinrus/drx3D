// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <Controls/DictionaryWidget.h>

#include <QVariant>
#include <QString>
#include <QIcon>

#include <DrxSchematyc/FundamentalTypes.h>

namespace Schematyc {

struct IScriptView;
struct IScriptRegistry;

}

namespace DrxSchematycEditor {

class CTypeDictionaryEntry : public CAbstractDictionaryEntry
{
	friend class CTypesDictionary;

public:
	CTypeDictionaryEntry();
	virtual ~CTypeDictionaryEntry();

	// CAbstractDictionaryEntry
	virtual u32   GetType() const override { return CAbstractDictionaryEntry::Type_Entry; }

	virtual QVariant GetColumnValue(i32 columnIndex) const override;
	// ~CAbstractDictionaryEntry

	QString                      GetName() const { return m_name; }
	const Schematyc::SElementId& GetTypeId()     { return m_elementId; }

private:
	QString               m_name;
	Schematyc::SElementId m_elementId;
};

class CTypesDictionary : public CAbstractDictionary
{
	struct SType
	{
		QString m_name;
		QIcon   m_icon;
	};

public:
	enum EColumn : i32
	{
		Column_Name,

		Column_COUNT
	};

public:
	CTypesDictionary(const Schematyc::IScriptElement* pScriptScope);
	virtual ~CTypesDictionary();

	// CAbstractDictionary
	virtual i32                           GetNumEntries() const override { return m_types.size(); }
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override;

	virtual i32                           GetNumColumns() const override { return Column_COUNT; };
	virtual QString                         GetColumnName(i32 index) const override;

	virtual i32                           GetDefaultFilterColumn() const override { return Column_Name; }
	// ~CAbstractDictionary

	void Load(const Schematyc::IScriptElement* pScriptScope);

private:
	std::vector<CTypeDictionaryEntry> m_types;
};

}

