// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "EnvironmentTool.h"
#include "EnvironmentPanel.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CEnvironmentTool, CEditTool)

//////////////////////////////////////////////////////////////////////////
CEnvironmentTool::CEnvironmentTool()
{
	SetStatusText(_T("Click Apply to accept changes"));
	m_panelId = 0;
	m_panel = 0;
}

//////////////////////////////////////////////////////////////////////////
CEnvironmentTool::~CEnvironmentTool()
{
}

//////////////////////////////////////////////////////////////////////////
void CEnvironmentTool::BeginEditParams(IEditor* ie, i32 flags)
{
	m_ie = ie;
	if (!m_panelId)
	{
		m_panel = new CEnvironmentPanel(AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage(ROLLUP_TERRAIN, "Environment", m_panel);
		AfxGetMainWnd()->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEnvironmentTool::EndEditParams()
{
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN, m_panelId);
		m_panel = 0;
	}
}

