// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "Gizmo.h"
#include "Grid.h"

//////////////////////////////////////////////////////////////////////////
// CGizmo implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CGizmo::CGizmo()
	: m_flags(0)
{
	// Set selectable flag by default. Unselectable gizmos should opt-out
	// Also set state to invalid so any updates can run the first time
	m_flags = EGIZMO_SELECTABLE | EGIZMO_INVALID;
}

//////////////////////////////////////////////////////////////////////////
CGizmo::~CGizmo()
{
}

void CGizmo::SetHighlighted(bool highlighted)
{
	if (highlighted)
	{
		SetFlag(EGIZMO_HIGHLIGHTED);
	}
	else
	{
		// only highlighted gizmos can be interacted with
		UnsetFlag(EGIZMO_INTERACTING);
		UnsetFlag(EGIZMO_HIGHLIGHTED);
	}
}

