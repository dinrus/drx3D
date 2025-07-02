// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "EditorAutoLevelLoadTest.h"

CEditorAutoLevelLoadTest& CEditorAutoLevelLoadTest::Instance()
{
	static CEditorAutoLevelLoadTest levelLoadTest;
	return levelLoadTest;
}

CEditorAutoLevelLoadTest::CEditorAutoLevelLoadTest()
{
	GetIEditorImpl()->RegisterNotifyListener(this);
}

CEditorAutoLevelLoadTest::~CEditorAutoLevelLoadTest()
{
	GetIEditorImpl()->UnregisterNotifyListener(this);
}

void CEditorAutoLevelLoadTest::OnEditorNotifyEvent(EEditorNotifyEvent event)
{
	switch (event)
	{
	case eNotify_OnEndSceneOpen:
		CLogFile::WriteLine("[LevelLoadFinished]");
		exit(0);
		break;
	}
}

