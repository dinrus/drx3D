// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined __EDITOR_AUTOLEVELLOAD_TEST_H__
#define __EDITOR_AUTOLEVELLOAD_TEST_H__

#pragma once

class CEditorAutoLevelLoadTest : public IEditorNotifyListener
{
public:
	static CEditorAutoLevelLoadTest& Instance();
private:
	CEditorAutoLevelLoadTest();
	virtual ~CEditorAutoLevelLoadTest();

	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event);
};

#endif //__EDITOR_AUTOLEVELLOAD_TEST_H__

