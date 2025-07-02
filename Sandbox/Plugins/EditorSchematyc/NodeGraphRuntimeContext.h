// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/IDrxGraphEditor.h>
#include <Controls/DictionaryWidget.h>

#include <DrxSchematyc/Script/IScriptGraph.h>

#include <ProxyModels/ItemModelAttribute.h>

class QIcon;

namespace Schematyc {

struct IScriptGraph;

}

namespace DrxGraphEditor {

class CNodeGraphViewStyle;

}

namespace DrxSchematycEditor {

class CNodesDictionaryCategoryEntry;

class CNodesDictionaryNodeEntry : public CAbstractDictionaryEntry
{
	friend class CNodesDictionaryCreator;

public:
	CNodesDictionaryNodeEntry(QString name, QString fullName, const Schematyc::IScriptGraphNodeCreationCommandPtr pCommand, CNodesDictionaryCategoryEntry* pParent = nullptr, const QIcon* pIcon = nullptr)
		: CAbstractDictionaryEntry()
		, m_name(name)
		, m_fullName(fullName)
		, m_pCommand(pCommand)
		, m_pParent(pParent)
		, m_pIcon(pIcon)
	{}
	virtual ~CNodesDictionaryNodeEntry() {}

	// CAbstractDictionaryEntry
	virtual u32                          GetType() const override { return Type_Entry; }
	virtual QVariant                        GetColumnValue(i32 columnIndex) const override;
	virtual const QIcon*                    GetColumnIcon(i32 columnIndex) const override;

	virtual const CAbstractDictionaryEntry* GetParentEntry() const override;
	virtual QVariant                        GetIdentifier() const;
	// ~CAbstractDictionaryEntry

	const QString&                                      GetName() const    { return m_name; }
	const Schematyc::IScriptGraphNodeCreationCommandPtr GetCommand() const { return m_pCommand; }

private:
	CNodesDictionaryCategoryEntry*                      m_pParent;
	QString                                             m_name;
	QString                                             m_fullName;
	const QIcon*                                        m_pIcon;
	const Schematyc::IScriptGraphNodeCreationCommandPtr m_pCommand;
};

class CNodesDictionaryCategoryEntry : public CAbstractDictionaryEntry
{
	friend class CNodesDictionaryCreator;

public:
	CNodesDictionaryCategoryEntry(QString name, CNodesDictionaryCategoryEntry* pParent = nullptr)
		: CAbstractDictionaryEntry()
		, m_name(name)
		, m_pParent(pParent)
	{}
	virtual ~CNodesDictionaryCategoryEntry();

	// DrxGraphEditor::CAbstractDictionaryItem
	virtual u32                          GetType() const override { return Type_Folder; }
	virtual QVariant                        GetColumnValue(i32 columnIndex) const override;

	virtual i32                           GetNumChildEntries() const override { return m_categories.size() + m_nodes.size(); }
	virtual const CAbstractDictionaryEntry* GetChildEntry(i32 index) const override;

	virtual const CAbstractDictionaryEntry* GetParentEntry() const override;
	// ~DrxGraphEditor::CAbstractDictionaryItem

	const QString& GetName() const { return m_name; }

private:
	CNodesDictionaryCategoryEntry*              m_pParent;
	QString                                     m_name;
	QString                                     m_fullName;
	std::vector<CNodesDictionaryCategoryEntry*> m_categories;
	std::vector<CNodesDictionaryNodeEntry*>     m_nodes;
};

class CNodesDictionary : public CAbstractDictionary
{
	friend class CNodesDictionaryCreator;

public:
	enum : i32
	{
		eColumn_Name,
		eColumn_Filter,
		eColumn_Identifier,

		eColumn_COUNT
	};

public:
	CNodesDictionary();
	virtual ~CNodesDictionary();

	// CAbstractDictionary
	virtual void                            ClearEntries() override;
	virtual i32                           GetNumEntries() const override { return m_categories.size() + m_nodes.size(); }
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override;

	virtual i32                           GetNumColumns() const                   { return eColumn_COUNT; }

	virtual i32                           GetDefaultFilterColumn() const override { return eColumn_Filter; }
	virtual i32                           GetDefaultSortColumn() const override   { return eColumn_Name; }

	virtual const CItemModelAttribute*      GetColumnAttribute(i32 index) const override;
	// ~CAbstractDictionary

	void LoadLoadsFromScriptGraph(Schematyc::IScriptGraph& scriptGraph);
	void SetStyle(const DrxGraphEditor::CNodeGraphViewStyle* pStyle) { m_pStyle = pStyle; }

private:
	const DrxGraphEditor::CNodeGraphViewStyle*  m_pStyle;
	std::vector<CNodesDictionaryCategoryEntry*> m_categories;
	std::vector<CNodesDictionaryNodeEntry*>     m_nodes;

	static const CItemModelAttribute            s_columnAttributes[eColumn_COUNT];
};

class CNodeGraphRuntimeContext : public DrxGraphEditor::INodeGraphRuntimeContext
{
public:
	CNodeGraphRuntimeContext(Schematyc::IScriptGraph& scriptGraph);
	~CNodeGraphRuntimeContext();

	// DrxGraphEditor::INodeGraphRuntimeContext
	virtual tukk                                GetTypeName() const override { return "Schematyc_Graph"; };
	virtual CAbstractDictionary*                       GetAvailableNodesDictionary() override;

	virtual const DrxGraphEditor::CNodeGraphViewStyle* GetStyle() const override { return m_pStyle; }
	// ~DrxGraphEditor::INodeGraphRuntimeContext

private:
	CNodesDictionary                     m_nodesDictionary;
	DrxGraphEditor::CNodeGraphViewStyle* m_pStyle;
	Schematyc::IScriptGraph&             m_scriptGraph;
};

}

