// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "EditorFramework/Editor.h"

class CModel;
class CGraphViewModel;
class CAsset;

class CMainWindow : public CDockableEditor
{
	Q_OBJECT;
public:
	CMainWindow(QWidget* pParent = nullptr);
	virtual tukk GetEditorName() const override;
	static void         CreateNewWindow(CAsset* asset);
protected:
	virtual void CreateDefaultLayout(CDockableContainer* pSender) override;
private:
	void                InitMenu();
	void                RegisterDockingWidgets();
	bool                OnOpen() final override;
	bool                OnOpenFile(const QString& path) final override;
	bool                OnClose() final override;
	bool                Open(CAsset* pAsset);
private:
	std::unique_ptr<CModel>          m_pModel;
	std::unique_ptr<CGraphViewModel> m_pGraphViewModel;
};

