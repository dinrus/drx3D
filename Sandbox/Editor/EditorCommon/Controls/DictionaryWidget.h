// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QDialog>
#include <QVariant>
#include <QString>
#include <QWidget>
#include <QObject>

#include "QPopupWidget.h"
#include "DrxSandbox/DrxSignal.h"

class QModelIndex;
class QShowEvent;
class QHideEvent;
class QFilteringPanel;

class CAbstractDictionary;
class QSearchBox;
class QAdvancedTreeView;
class CDictionaryFilterProxyModel;
class CDictionaryModel;
class CMergingProxyModel;
class QAbstractItemModel;
class CItemModelAttribute;

class EDITOR_COMMON_API CAbstractDictionaryEntry
{
public:
	enum EType : u32
	{
		Type_Undefined = 0,
		Type_Folder,
		Type_Entry
	};

public:
	CAbstractDictionaryEntry() {}
	virtual ~CAbstractDictionaryEntry() {}

	virtual u32                          GetType() const                         { return Type_Undefined; }	
	virtual QVariant                        GetColumnValue(i32 columnIndex) const { return QVariant(); }
	virtual const QIcon*                    GetColumnIcon(i32 columnIndex) const  { return nullptr; }

	virtual QString                         GetToolTip() const                      { return QString(); }

	virtual i32                           GetNumChildEntries() const              { return 0; }
	virtual const CAbstractDictionaryEntry* GetChildEntry(i32 index) const        { return nullptr; }

	virtual const CAbstractDictionaryEntry* GetParentEntry() const                  { return nullptr; }

	virtual QVariant                        GetIdentifier() const                   { return QVariant(); }
	virtual bool                            IsEnabled() const                       { return true; }
};

class EDITOR_COMMON_API CAbstractDictionary
{
public:
	CDrxSignal<void()>                      signalDictionaryClear;

public:
	CAbstractDictionary() {}
	virtual ~CAbstractDictionary() {}

public:
	void                                    Clear();

public:
	virtual tukk                     GetName() const                  { return "<Unknown>"; }
	virtual i32                           GetNumEntries() const = 0;
	virtual const CAbstractDictionaryEntry* GetEntry(i32 index) const = 0;

	virtual void                            ClearEntries()                   { return; }

	virtual i32                           GetNumColumns() const            { return 0; };
	virtual QString                         GetColumnName(i32 index) const { return QString(); }

	virtual i32                           GetDefaultFilterColumn() const   { return -1; }
	virtual i32                           GetDefaultSortColumn() const     { return -1; }

	virtual const CItemModelAttribute*      GetFilterAttribute() const;
	virtual const CItemModelAttribute*      GetColumnAttribute(i32 index) const;
};

class EDITOR_COMMON_API CDictionaryWidget : public QWidget
{
	Q_OBJECT

public:
	CDictionaryWidget(QWidget* pParent = nullptr, QFilteringPanel* pFilteringPanel = nullptr);
	virtual ~CDictionaryWidget();

	CAbstractDictionary* GetDictionary(const QString& name) const;
	void                 AddDictionary(CAbstractDictionary& dictionary);
	void                 RemoveDictionary(const QString& name);
	void                 RemoveAllDictionaries();

	void                 ShowHeader(bool flag);
	void                 SetFilterText(const QString& filterText);
	

Q_SIGNALS:
	void         OnEntryClicked(CAbstractDictionaryEntry& entry);
	void         OnEntryDoubleClicked(CAbstractDictionaryEntry& entry);

	void         OnHide();

private:
	void         OnClicked(const QModelIndex& index);
	void         OnDoubleClicked(const QModelIndex& index);

	void         OnFiltered();

	void         GatherItemModelAttributes(std::vector<CItemModelAttribute*>& columns);
	QVariant     GeneralHeaderDataCallback(std::vector<CItemModelAttribute*>& columns, i32 section, Qt::Orientation orientation, i32 role);

	virtual void showEvent(QShowEvent* pEvent) override;
	virtual void hideEvent(QHideEvent* pEvent) override;

private:	
	QSearchBox*                         m_pFilter;
	QFilteringPanel*                    m_pFilteringPanel;
	QAdvancedTreeView*                  m_pTreeView;	
	CMergingProxyModel*                 m_pMergingModel;
	CDictionaryFilterProxyModel*        m_pFilterProxy;
	

	QMap<QString, QAbstractItemModel*>  m_modelMap;	
};

class EDITOR_COMMON_API CModalPopupDictionary : public QObject
{
public:
	CModalPopupDictionary(QString name, CAbstractDictionary& dictionary);
	~CModalPopupDictionary();

	void                      ExecAt(const QPoint& globalPos, QPopupWidget::Origin origin = QPopupWidget::Unknown);

	CAbstractDictionaryEntry* GetResult() const { return m_pResult; }

protected:
	void OnEntryClicked(CAbstractDictionaryEntry& entry);
	void OnAborted();

private:
	QString                   m_dictName;
	QPopupWidget*             m_pPopupWidget;
	CAbstractDictionaryEntry* m_pResult;
	QEventLoop*               m_pEventLoop;
	CDictionaryWidget*        m_pDictionaryWidget;
};

