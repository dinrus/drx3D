// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Controls/DictionaryWidget.h>

#include <QString>
#include <QVariant>

namespace DrxParticleEditor {

class CNodesDictionaryCategory;

class CNodesDictionaryNode : public CAbstractDictionaryEntry
{
public:
	CNodesDictionaryNode(QString name, QString filePath, CNodesDictionaryCategory* pParent = nullptr)
		: CAbstractDictionaryEntry()
		, m_name(name)
		, m_filePath(filePath)
		, m_pParent(pParent)
	{}
	virtual ~CNodesDictionaryNode() {}

	// CAbstractDictionaryEntry
	virtual u32                          GetType() const override { return Type_Entry; }
	virtual QVariant                        GetColumnValue(i32 columnIndex) const override;

	virtual const CAbstractDictionaryEntry* GetParentEntry() const override;
	virtual QVariant                        GetIdentifier() const override;
	// ~CAbstractDictionaryEntry

	const QString& GetName() const     { return m_name; }
	const QString& GetFilePath() const { return m_filePath; }

private:
	CNodesDictionaryCategory* m_pParent;
	QString                   m_name;
	QString                   m_filePath;
};

class CNodesDictionaryCategory : public CAbstractDictionaryEntry
{
public:
	CNodesDictionaryCategory(QString name, CNodesDictionaryCategory* pParent = nullptr)
		: CAbstractDictionaryEntry()
		, m_name(name)
		, m_pParent(pParent)
	{}
	virtual ~CNodesDictionaryCategory() {}

	// CAbstractDictionaryEntry
	virtual u32                          GetType() const override { return Type_Folder; }
	virtual QVariant                        GetColumnValue(i32 columnIndex) const override;

	virtual i32                           GetNumChildEntries() const override { return m_categories.size() + m_nodes.size(); }
	virtual const CAbstractDictionaryEntry* GetChildEntry(i32 index) const override;

	virtual const CAbstractDictionaryEntry* GetParentEntry() const override;
	// ~CAbstractDictionaryEntry

	CNodesDictionaryCategory& CreateCategory(QString name);
	CNodesDictionaryNode&     CreateNode(QString name, QString filePath);

private:
	QString                                m_name;
	CNodesDictionaryCategory*              m_pParent;
	std::vector<CNodesDictionaryCategory*> m_categories;
	std::vector<CNodesDictionaryNode*>     m_nodes;
};

class CNodesDictionary : public CAbstractDictionary
{
public:
	enum EColumn : i32
	{
		eColumn_Name,
		eColumn_Identifier,

		eColumn_COUNT
	};

public:
	CNodesDictionary();
	virtual ~CNodesDictionary();

	// DrxGraphEditor::CAbstractDictionary
	virtual i32                           GetNumEntries() const override { return m_root.GetNumChildEntries(); }
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const override;

	virtual i32                           GetDefaultSortColumn() const override { return 0; }
	virtual i32                           GetNumColumns() const { return 1; }
	// ~DrxGraphEditor::CAbstractDictionary

protected:
	void LoadTemplates();

private:
	CNodesDictionaryCategory m_root;
};

}

