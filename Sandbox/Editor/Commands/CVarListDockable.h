// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ISystem.h>
#include "EditorFramework/Editor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>
#include "Util/Variable.h"
#include "ProxyModels/FavoritesHelper.h"
#include <QAbstractItemModel>
#include "Controls/EditorDialog.h"
#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <memory>
#include <tuple>
#include <vector>

struct ICVar;
class CFavoriteFilterProxy;
class QAdvancedTreeView;

class CCVarModel : public QAbstractItemModel, IConsoleVarSink
{
public:
	enum Roles
	{
		DataTypeRole = Qt::UserRole,
		SortRole,
	};

	enum DataTypes
	{
		Int,
		Float,
		String
	};

	CCVarModel();
	~CCVarModel();

	//QAbstractItemModel implementation begin
	virtual i32           columnCount(const QModelIndex& parent = QModelIndex()) const override { return DRX_ARRAY_COUNT(s_szColumnNames); }
	virtual QVariant      data(const QModelIndex& index, i32 role = Qt::DisplayRole) const override;
	virtual bool          hasChildren(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant      headerData(i32 section, Qt::Orientation orientation, i32 role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual QModelIndex   index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex   parent(const QModelIndex& index) const override;
	virtual bool          setData(const QModelIndex& index, const QVariant& value, i32 role = Qt::EditRole) override;
	virtual i32           rowCount(const QModelIndex& parent = QModelIndex()) const override;
	//QAbstractItemModel implementation end

	//IConsoleVarSink implementation begin
	virtual bool OnBeforeVarChange(ICVar* pVar, tukk sNewValue) override;
	virtual void OnAfterVarChange(ICVar* pVar) override;
	virtual void OnVarUnregister(ICVar* pVar) override {}
	//IConsoleVarSink implementation end

	ICVar* GetCVar(const QModelIndex& idx) const;

protected:
	static tukk s_szColumnNames[5];

	void OnCVarChanged(ICVar* const pCVar);

	std::vector<string> m_cvars;
	FavoritesHelper m_favHelper;

	enum TupleIndices
	{
		CVarIndex,
		ListenerIndex,
	};
};

class CCVarBrowser : public QWidget
{
	class CCVarListDelegate;
public:
	CCVarBrowser(QWidget* pParent = nullptr);

	QStringList GetSelectedCVars() const;

public:
	CDrxSignal<void(tukk)> signalOnClick;
	CDrxSignal<void(tukk)> signalOnDoubleClick;

private:
	std::unique_ptr<CCVarModel>           m_pModel;
	std::unique_ptr<CFavoriteFilterProxy> m_pFilterProxy;
	QAdvancedTreeView*                    m_pTreeView;
};

class CCVarBrowserDialog : public CEditorDialog
{
public:
	CCVarBrowserDialog();

	void               CVarsSelected(const QStringList& cvars);
	const QStringList& GetSelectedCVars() const { return m_selectedCVars; }

protected:
	virtual QSize sizeHint() const override { return QSize(800, 500); }

private:
	CCVarBrowser* m_pCVarBrowserWidget;
	QStringList   m_selectedCVars;
};

class CCVarListDockable : public CDockableEditor
{
public:
	CCVarListDockable(QWidget* const pParent = nullptr);
	~CCVarListDockable();

	//////////////////////////////////////////////////////////
	// CDockableWidget implementation
	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_FLOAT; }
	virtual QRect                             GetPaneRect() override               { return QRect(0, 0, 900, 500); }
	//////////////////////////////////////////////////////////

	virtual tukk GetEditorName() const override { return "Console Variables"; }
	static QVariant     GetState();
	static void         SetState(const QVariant& state);

	static void         LoadCVarListFromFile(tukk szFilename);
	static void         SaveCVarListToFile(tukk szFilename);

protected:
	virtual bool OnCopy() override;

protected:
	CCVarBrowser* m_pCVarBrowser;
};

