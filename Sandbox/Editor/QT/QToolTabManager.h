// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct IViewPaneClass;
class CWnd;

#include "SandboxAPI.h"
#include <QFrame>
#include <QDir>

//Internal class for IPane management.
class QTabPane : public QFrame
{
	Q_OBJECT
public:
	string m_title;
	string m_category;
	string m_class;

	CWnd*  m_MfcWnd;
	bool   m_bViewCreated;

	IPane* m_pane;

	QSize  m_defaultSize;
	QSize  m_minimumSize;

public:
	QTabPane();
	~QTabPane();

protected:
	void          showContextMenu(const QPoint& point);

	void          closeEvent(QCloseEvent* event);
	virtual QSize sizeHint() const        { return m_defaultSize; }
	virtual QSize minimumSizeHint() const { return m_minimumSize; }
};

class SANDBOX_API CTabPaneManager
{
	friend class QTabPane;
public:
	CTabPaneManager(QWidget* const pParent);
	~CTabPaneManager();

	void          SaveLayout();
	bool          LoadUserLayout();
	bool          LoadLayout(tukk filePath);
	bool          LoadDefaultLayout();
	void          SaveLayoutToFile(tukk fullFilename);

	QFileInfoList GetUserLayouts();
	QFileInfoList GetProjectLayouts();
	QFileInfoList GetAppLayouts();

	// nOverrideDockDirection is one of the IViewPanelCass::EDockingDirection
	IPane* CreatePane(tukk paneClassName, tukk title = 0, i32 nOverrideDockDirection = -1);

	template<typename T>
	T* CreatePaneT(tukk paneClassName, tukk title = 0, i32 nOverrideDockDirection = -1)
	{
		static_assert(std::is_base_of<IPane, T>::value, "CreatePaneT can only be called with IPane derived types");
		return static_cast<T*>(CreatePane(paneClassName, title, nOverrideDockDirection));
	}

	void   CloseAllPanes();

	IPane* FindPaneByClass(tukk paneClassName);
	IPane* FindPaneByTitle(tukk title);

	void   Close(IPane* pane);
	void   BringToFront(IPane* pane);
	IPane* OpenOrCreatePane(tukk paneClassName);

	//////////////////////////////////////////////////////////////////////////
	// MFC Specific
	CWnd* OpenMFCPane(tukk paneClassName);
	CWnd* FindMFCPane(tukk paneClassName);
	//////////////////////////////////////////////////////////////////////////

	void                    OnIdle();
	void                    LayoutLoaded();

	static CTabPaneManager* GetInstance();

	void OnTabPaneMoved(QWidget* tabPane, bool visible);

private:
	QTabPane*                 CreateTabPane(tukk paneClassName, tukk title = 0, i32 nOverrideDockDirection = -1, bool bLoadLayoutPersonalization = false);
	QString                   CreateObjectName(tukk title);

	QVariant                  GetState() const;
	void                      SetState(const QVariant& state);

	void                      CreateContentInPanes();
	IPane*                    CreatePaneContents(QTabPane* pTool);

	void                      StoreHistory(QTabPane* viewDesc);

	class QToolWindowManager* GetToolManager() const;

	void                      SetDefaultLayout();

	QTabPane*                 FindTabPane(IPane* pane);

	bool                      CloseTabPane(QTabPane* pane);

	bool                      LoadLayoutFromFile(tukk fullFilename);

	QTabPane*                 FindTabPaneByName(const QString& name = QString());
	QList<QTabPane*>          FindTabPanes(const QString& name = QString());
	QTabPane*                 FindTabPaneByClass(tukk paneClassName);
	QTabPane*                 FindTabPaneByCategory(tukk sPaneCategory);
	QTabPane*                 FindTabPaneByTitle(tukk title);

	void                      PushUserEvent(tukk szEventName, tukk szTitle, ukk pAddress);

	struct SPaneHistory
	{
		QRect rect;    // Last known size of this panel.
		i32   dockDir; // Last known dock style.
	};
	std::map<string, SPaneHistory> m_panesHistory;

	bool                           m_bToolsDirty;
	std::vector<QTabPane*>         m_panes;

	QWidget* const                 m_pParent;
	bool                           m_layoutLoaded;
};

