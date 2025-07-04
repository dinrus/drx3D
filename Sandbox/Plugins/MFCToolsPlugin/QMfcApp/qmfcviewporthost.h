// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "qwinwidget.h"

class CViewport;

class PLUGIN_API QMfcViewportHost
{
public:
	QMfcViewportHost(CWnd* pParent, CViewport* pViewport, const CRect& rect);
	~QMfcViewportHost()
	{
		delete m_pHost;
	}

	void     SetClientRect(const CRect& rect) { m_pHost->setGeometry(rect.left, rect.top, rect.Width(), rect.Height()); }
	QWidget* GetHostWidget()                  { return m_pHost; }

private:
	QWinWidget* m_pHost;
};

class PLUGIN_API QMfcContainer : public CWnd
{
public:
	QMfcContainer() : m_pWidgetHost(nullptr) {}
	virtual ~QMfcContainer() override {}

	void SetWidgetHost(QWidget* pHost);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
private:
	QWidget* m_pWidgetHost;
};

