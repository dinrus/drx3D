// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"

#include <IResourceSelectorHost.h>
#include <QtUtil.h>
#include <Controls/DictionaryWidget.h>
#include <Controls/QPopupWidget.h>
#include <DrxSchematyc/Utils/Assert.h>
#include <DrxSchematyc/SerializationUtils/SerializationQuickSearch.h>

#include "PluginUtils.h"
#include "TypesDictionary.h"

enum EColumn : i32
{
	Column_Name,
	Column_COUNT
};

class CStringListDictionaryEntry;

typedef std::unique_ptr<CStringListDictionaryEntry> StringListDictionaryEntryPtr;
typedef std::vector<StringListDictionaryEntryPtr>   StringListDictionaryEntries;

class CStringListDictionaryEntry : public CAbstractDictionaryEntry
{
public:

	inline CStringListDictionaryEntry(u32 type, const QString& name, CStringListDictionaryEntry* pParent = nullptr)
		: m_type(type)
		, m_name(name)
		, m_pParent(pParent)
	{}

	virtual ~CStringListDictionaryEntry() {}

	// CAbstractDictionaryEntry

	virtual u32 GetType() const override
	{
		return m_type;
	}

	virtual QVariant GetColumnValue(i32 columnIdx) const override
	{
		if (columnIdx == Column_Name)
		{
			return m_name;
		}
		return QVariant();
	}

	virtual i32 GetNumChildEntries() const override
	{
		return m_children.size();
	}

	virtual const CAbstractDictionaryEntry* GetChildEntry(i32 idx) const override
	{
		return idx < m_children.size() ? m_children[idx].get() : nullptr;
	}

	virtual const CAbstractDictionaryEntry* GetParentEntry() const override
	{
		return m_pParent;
	}

	// ~CAbstractDictionaryEntry

	inline const QString& GetName() const
	{
		return m_name;
	}

	inline CStringListDictionaryEntry* AddChild(u32 type, const QString& name)
	{
		m_children.emplace_back(new CStringListDictionaryEntry(type, name, this));
		return m_children.back().get();
	}

	inline StringListDictionaryEntries& GetChildren()
	{
		return m_children;
	}

private:

	u32                      m_type;
	QString                     m_name;
	CStringListDictionaryEntry* m_pParent;
	StringListDictionaryEntries m_children;
};

inline CStringListDictionaryEntry* FindStringListDictionaryEntry(StringListDictionaryEntries& entries, const QString& name)
{
	for (StringListDictionaryEntryPtr& pEntry : entries)
	{
		if (pEntry->GetName() == name)
		{
			return pEntry.get();
		}
	}
	return nullptr;
}

class CStringListDictionary : public CAbstractDictionary
{
public:

	inline CStringListDictionary() {}

	virtual ~CStringListDictionary() {}

	// DrxGraphEditor::CAbstractDictionary

	virtual i32 GetNumEntries() const override
	{
		return m_rootEntries.size();
	}

	virtual const CAbstractDictionaryEntry* GetEntry(i32 idx) const override
	{
		return idx < m_rootEntries.size() ? m_rootEntries[idx].get() : nullptr;
	}

	virtual i32 GetNumColumns() const override
	{
		return Column_COUNT;
	}

	virtual QString GetColumnName(i32 idx) const override
	{
		if (idx == Column_Name)
		{
			return QString("Name");
		}
		return QString();
	}

	virtual i32 GetDefaultFilterColumn() const override
	{
		return Column_Name;
	}

	// ~DrxGraphEditor::CAbstractDictionary

	void Load(const Schematyc::IQuickSearchOptions& quickSearchOptions)
	{
		m_rootEntries.clear();

		u32k optionCount = quickSearchOptions.GetCount();
		m_rootEntries.reserve(optionCount);

		for (u32 optionIdx = 0; optionIdx < optionCount; ++optionIdx)
		{
			QString fullName = quickSearchOptions.GetLabel(optionIdx);
			QStringList names = fullName.split(quickSearchOptions.GetDelimiter());

			CStringListDictionaryEntry* pParentEntry = nullptr;
			for (u32 nameIdx = 0, nameCount = names.size(); nameIdx < nameCount; ++nameIdx)
			{
				const QString& name = names[nameIdx];
				CStringListDictionaryEntry* pEntry = FindStringListDictionaryEntry(pParentEntry ? pParentEntry->GetChildren() : m_rootEntries, name);
				if (pEntry)
				{
					pParentEntry = pEntry;
				}
				else
				{
					u32k type = nameIdx < (nameCount - 1) ? CAbstractDictionaryEntry::Type_Folder : CAbstractDictionaryEntry::Type_Entry;
					if (pParentEntry)
					{
						pParentEntry = pParentEntry->AddChild(type, name);
					}
					else
					{
						m_rootEntries.emplace_back(new CStringListDictionaryEntry(type, name));
						pParentEntry = m_rootEntries.back().get();
					}
				}
			}
		}
	}

private:

	static u32k         s_invalidIdx = 0xffffffff;

	StringListDictionaryEntries m_rootEntries;
};

namespace Schematyc
{
namespace SerializationUtils
{
dll_string StringListStaticQuickSearchSelector(const SResourceSelectorContext& context, tukk )
{
	const Private::CStringListStaticQuickSearchOptions* pOptions = static_cast<const Private::CStringListStaticQuickSearchOptions*>(context.pCustomParams.get());
	SCHEMATYC_EDITOR_ASSERT(pOptions);
	if (pOptions)
	{
		static CStringListDictionary dictionary;
		dictionary.Load(*pOptions);

		CModalPopupDictionary popup(pOptions->GetHeader(), dictionary);

		const QPoint pos = QCursor::pos();
		popup.ExecAt(pos, QPopupWidget::TopRight);

		CStringListDictionaryEntry* pEntry = static_cast<CStringListDictionaryEntry*>(popup.GetResult());
		if (pEntry)
		{
			return QtUtil::ToString(pEntry->GetName()).c_str();
		}
	}
	return "";
}

dll_string StringListQuickSearchSelector(const SResourceSelectorContext& context, tukk )
{
	const Private::CStringListQuickSearchOptions* pOptions = static_cast<const Private::CStringListQuickSearchOptions*>(context.pCustomParams.get());
	SCHEMATYC_EDITOR_ASSERT(pOptions);
	if (pOptions)
	{
		static CStringListDictionary dictionary;
		dictionary.Load(*pOptions);

		CModalPopupDictionary popup(pOptions->GetHeader(), dictionary);

		const QPoint pos = QCursor::pos();
		popup.ExecAt(pos, QPopupWidget::TopRight);

		CStringListDictionaryEntry* pEntry = static_cast<CStringListDictionaryEntry*>(popup.GetResult());
		if (pEntry)
		{
			return QtUtil::ToString(pEntry->GetName()).c_str();
		}
	}
	return "";
}

REGISTER_RESOURCE_SELECTOR("StringListStaticSearch", StringListStaticQuickSearchSelector, "icons:General/Search.ico")
REGISTER_RESOURCE_SELECTOR("StringListSearch", StringListQuickSearchSelector, "icons:General/Search.ico")
} // SerializationUtils
} // Schematyc

