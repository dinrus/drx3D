// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"

#include "PluginUtils.h"

#include <IActionMapManager.h> // #SchematycTODO : Remove dependency on DrxAction!
#include <IResourceSelectorHost.h>
#include <ISourceControl.h>
#include <DrxEntitySystem/IEntityClass.h>
#include <DrxEntitySystem/IEntitySystem.h>
#include <Drx3DEngine/I3DEngine.h>
#include <DrxGame/IGameFramework.h>
#include <drx3D/CoreX/String/DrxStringUtils.h>
#include <DrxSchematyc/Utils/Assert.h>

#include <Controls/DictionaryWidget.h>
#include <Controls/QPopupWidget.h>

#include <QtUtil.h>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>

namespace DrxSchematycEditor {

enum EColumn : i32
{
	Column_Name,
	Column_COUNT
};

class CStringListDictionaryEntry : public CAbstractDictionaryEntry
{
	friend class CStringListDictionary;

public:
	CStringListDictionaryEntry() {}
	virtual ~CStringListDictionaryEntry() {}

	// CAbstractDictionaryEntry
	virtual u32   GetType() const override { return CAbstractDictionaryEntry::Type_Entry; }

	virtual QVariant GetColumnValue(i32 columnIndex) const override
	{
		if (columnIndex == Column_Name)
		{
			return m_name;
		}
		return QVariant();
	}
	// ~CAbstractDictionaryEntry

	QString GetName() const { return m_name; }

private:
	QString m_name;
};

class CStringListDictionary : public CAbstractDictionary
{
public:
	CStringListDictionary(const Serialization::StringListStatic& names)
	{
		Load(names);
	}
	CStringListDictionary(const Serialization::StringList& names)
	{
		Load(names);
	}
	virtual ~CStringListDictionary() {}

	// DrxGraphEditor::CAbstractDictionary
	virtual i32                           GetNumEntries() const override       { return m_names.size(); }
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override { return (m_names.size() > index) ? &m_names[index] : nullptr; }

	virtual i32                           GetNumColumns() const override       { return Column_COUNT; };
	virtual QString                         GetColumnName(i32 index) const override
	{
		if (index == Column_Name)
			return QString("Name");
		return QString();
	}

	virtual i32 GetDefaultFilterColumn() const override { return Column_Name; }
	// ~DrxGraphEditor::CAbstractDictionary

	void Load(const Serialization::StringListStatic& names)
	{
		for (string name : names)
		{
			CStringListDictionaryEntry entry;
			entry.m_name = name;
			m_names.emplace_back(entry);
		}
	}

	void Load(const Serialization::StringList& names)
	{
		for (string name : names)
		{
			CStringListDictionaryEntry entry;
			entry.m_name = name;
			m_names.emplace_back(entry);
		}
	}

private:
	std::vector<CStringListDictionaryEntry> m_names;
};

class CActionMapActionQuickSearchOptions : public IActionMapPopulateCallBack
{
public:
	CActionMapActionQuickSearchOptions()
	{
		IActionMapManager* pActionMapManager = gEnv->pGameFramework->GetIActionMapManager();
		if (pActionMapManager && (pActionMapManager->GetActionsCount() > 0))
		{
			pActionMapManager->EnumerateActions(this);
		}
	}

	// IActionMapPopulateCallBack
	virtual void AddActionName(tukk const szName) override
	{
		m_names.push_back(szName);
	}
	// ~IActionMapPopulateCallBack

	Serialization::StringListStatic& GetNames() { return m_names; }

private:
	Serialization::StringListStatic m_names;
};

}

namespace {

dll_string EntityClassNameSelector(const SResourceSelectorContext& context, tukk szPreviousValue, Serialization::StringListValue* pStringListValue)
{
	Serialization::StringListStatic names;

	IEntityClassRegistry& entityClassRegistry = *gEnv->pEntitySystem->GetClassRegistry();
	entityClassRegistry.IteratorMoveFirst();

	while (IEntityClass* pClass = entityClassRegistry.IteratorNext())
	{
		names.push_back(pClass->GetName());
	}

	DrxSchematycEditor::CStringListDictionary dict(names);
	CModalPopupDictionary dictionary("Entity Class", dict);

	const QPoint pos = QCursor::pos();
	dictionary.ExecAt(pos);

	DrxSchematycEditor::CStringListDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CStringListDictionaryEntry*>(dictionary.GetResult());
	if (pEntry)
	{
		return QtUtil::ToString(pEntry->GetName()).c_str();
	}

	return "";
}

dll_string ActionMapNameSelector(const SResourceSelectorContext& context, tukk szPreviousValue, Serialization::StringListValue* pStringListValue)
{
	Serialization::StringListStatic names;

	IActionMapIteratorPtr itActionMap = gEnv->pGameFramework->GetIActionMapManager()->CreateActionMapIterator();
	while (IActionMap* pActionMap = itActionMap->Next())
	{
		tukk szName = pActionMap->GetName();
		if (strcmp(szName, "default"))
		{
			names.push_back(szName);
		}
	}

	DrxSchematycEditor::CStringListDictionary dict(names);
	CModalPopupDictionary dictionary("Action Map", dict);

	const QPoint pos = QCursor::pos();
	dictionary.ExecAt(pos);

	DrxSchematycEditor::CStringListDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CStringListDictionaryEntry*>(dictionary.GetResult());
	if (pEntry)
	{
		return QtUtil::ToString(pEntry->GetName()).c_str();
	}

	return "";
}

dll_string ActionMapActionNameSelector(const SResourceSelectorContext& context, tukk szPreviousValue, Serialization::StringListValue* pStringListValue)
{
	DrxSchematycEditor::CActionMapActionQuickSearchOptions quickSearchOptions;

	DrxSchematycEditor::CStringListDictionary dict(quickSearchOptions.GetNames());
	CModalPopupDictionary dictionary("Action Map Action", dict);

	const QPoint pos = QCursor::pos();
	dictionary.ExecAt(pos);

	DrxSchematycEditor::CStringListDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CStringListDictionaryEntry*>(dictionary.GetResult());
	if (pEntry)
	{
		return QtUtil::ToString(pEntry->GetName()).c_str();
	}

	return "";
}

dll_string SurfaceTypeNameSelector(const SResourceSelectorContext& context, tukk szPreviousValue, Serialization::StringListValue* pStringListValue)
{
	Serialization::StringListStatic names;

	ISurfaceTypeEnumerator* pEnumerator = gEnv->p3DEngine->GetMaterialManager()->GetSurfaceTypeManager()->GetEnumerator();
	for (ISurfaceType* pSurfaceType = pEnumerator->GetFirst(); pSurfaceType != nullptr; pSurfaceType = pEnumerator->GetNext())
	{
		cstr name = pSurfaceType->GetName();
		if (strlen(name) >= 4 && !strncmp(name, "mat_", 4))
			name += 4;
		names.push_back(name);
	}

	DrxSchematycEditor::CStringListDictionary dict(names);
	CModalPopupDictionary dictionary("Surface Type", dict);

	const QPoint pos = QCursor::pos();
	dictionary.ExecAt(pos);

	DrxSchematycEditor::CStringListDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CStringListDictionaryEntry*>(dictionary.GetResult());
	if (pEntry)
	{
		return QtUtil::ToString(pEntry->GetName()).c_str();
	}

	return "";
}

static dll_string MannequinScopeContextName(const SResourceSelectorContext& context, tukk szPreviousValue, Serialization::StringListValue* pStringListValue)
{
	if (std::shared_ptr<Serialization::SMannequinControllerDefResourceParams> pParams = std::static_pointer_cast<Serialization::SMannequinControllerDefResourceParams>(context.pCustomParams))
	{
		if (pParams->pControllerDef != nullptr)
		{
			Serialization::StringListStatic names;

			for (i32 i = 0, n = pParams->pControllerDef->m_scopeContexts.GetNum(); i < n; ++i)
			{
				names.push_back(pParams->pControllerDef->m_scopeContexts.GetTagName(i));
			}

			DrxSchematycEditor::CStringListDictionary dict(names);
			CModalPopupDictionary dictionary("Mannequin Scope Context", dict);

			const QPoint pos = QCursor::pos();
			dictionary.ExecAt(pos);

			DrxSchematycEditor::CStringListDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CStringListDictionaryEntry*>(dictionary.GetResult());
			if (pEntry)
			{
				return QtUtil::ToString(pEntry->GetName()).c_str();
			}
		}
	}

	return "";
}

static dll_string MannequinFragmentName(const SResourceSelectorContext& context, tukk szPreviousValue, Serialization::StringListValue* pStringListValue)
{
	if (std::shared_ptr<Serialization::SMannequinControllerDefResourceParams> pParams = std::static_pointer_cast<Serialization::SMannequinControllerDefResourceParams>(context.pCustomParams))
	{
		if (pParams->pControllerDef != nullptr)
		{
			Serialization::StringListStatic names;

			for (i32 i = 0, n = pParams->pControllerDef->m_fragmentIDs.GetNum(); i < n; ++i)
			{
				names.push_back(pParams->pControllerDef->m_fragmentIDs.GetTagName(i));
			}

			DrxSchematycEditor::CStringListDictionary dict(names);
			CModalPopupDictionary dictionary("Mannequin Fragment", dict);

			const QPoint pos = QCursor::pos();
			dictionary.ExecAt(pos);

			DrxSchematycEditor::CStringListDictionaryEntry* pEntry = static_cast<DrxSchematycEditor::CStringListDictionaryEntry*>(dictionary.GetResult());
			if (pEntry)
			{
				return QtUtil::ToString(pEntry->GetName()).c_str();
			}
		}
	}

	return "";
}

REGISTER_RESOURCE_SELECTOR("EntityClass", EntityClassNameSelector, "")
REGISTER_RESOURCE_SELECTOR("ActionMap", ActionMapNameSelector, "")
REGISTER_RESOURCE_SELECTOR("ActionMapAction", ActionMapActionNameSelector, "")
REGISTER_RESOURCE_SELECTOR("SurfaceTypeName", SurfaceTypeNameSelector, "")
REGISTER_RESOURCE_SELECTOR("MannequinScopeContextName", MannequinScopeContextName, "")
REGISTER_RESOURCE_SELECTOR("MannequinFragmentName", MannequinFragmentName, "")
}

namespace DrxSchematycEditor {
namespace Utils {

void ConstructAbsolutePath(Schematyc::IString& output, tukk szFileName)
{
	char currentDirectory[512] = "";
	GetCurrentDirectory(sizeof(currentDirectory) - 1, currentDirectory);

	Schematyc::CStackString temp = currentDirectory;
	temp.append("\\");
	temp.append(szFileName);
	temp.replace("/", "\\");

	output.assign(temp);
}

bool WriteToClipboard(tukk szText, tukk szPrefix)
{
	SCHEMATYC_EDITOR_ASSERT(szText);
	if (szText)
	{
		QMimeData* pMimeData = new QMimeData();
		if (szPrefix)
			pMimeData->setData(szPrefix, szText);
		else
			pMimeData->setText(szText);

		QClipboard* pClipboard = QApplication::clipboard();
		pClipboard->setMimeData(pMimeData);

		return true;
	}
	return false;
}

bool ReadFromClipboard(string& text, tukk szPrefix)
{
	QClipboard* pClipboard = QApplication::clipboard();
	if (pClipboard)
	{
		const QMimeData* pMimeData = pClipboard->mimeData();
		if (szPrefix)
		{
			QByteArray byteArray = pMimeData->data(szPrefix);
			if (byteArray.length() > 0)
			{
				text.reserve(byteArray.length());
				text.assign(byteArray.data(), byteArray.length());
				return true;
			}
		}
		else
		{
			text = pMimeData->text().toUtf8();
			return text.length() > 0;
		}
	}

	text.clear();
	return false;
}

bool ValidateClipboardContents(tukk szPrefix)
{
	SCHEMATYC_EDITOR_ASSERT(szPrefix);

	QClipboard* pClipboard = QApplication::clipboard();
	if (pClipboard)
	{
		const QMimeData* pMimeData = pClipboard->mimeData();
		QByteArray byteArray = pMimeData->data(szPrefix);
		if (byteArray.length() > 0)
		{
			return true;
		}
	}
	return false;
}

}
}

