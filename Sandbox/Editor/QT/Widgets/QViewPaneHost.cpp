// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DrxEngine Source File.
//  Copyright (C), DinrusPro 3D, 2014.
// -------------------------------------------------------------------------
//  File name:   QViewPaneHost.cpp
//  Version:     v1.00
//  Created:     29/10/2014 by timur.
//  Description: Host widget for view panes
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "QViewPaneHost.h"

#include <QLayout>

//////////////////////////////////////////////////////////////////////////
QViewPaneHost::QViewPaneHost()
{
	setLayout(new QVBoxLayout);
	setFrameStyle(StyledPanel | Plain);
	layout()->setContentsMargins(0, 0, 0, 0);
}

