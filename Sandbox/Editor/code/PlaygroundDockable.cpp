// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <StdAfx.h>
#include "PlaygroundDockable.h"

#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

#include <QLabel>

//Uncomment this to register and use the Playground Dockable
//REGISTER_VIEWPANE_FACTORY(CPlaygroundDockable, "Playground", "", false);

CPlaygroundDockable::CPlaygroundDockable()
	: CDockableEditor()
{
	//Sample code, replace with your test
	auto someTestWidget = new QLabel("Playground Dockable");
	someTestWidget->setAlignment(Qt::AlignCenter);
	someTestWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	SetContent(someTestWidget);
}

CPlaygroundDockable::~CPlaygroundDockable()
{

}

