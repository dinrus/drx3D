// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CustomActionsEditorManager_h__
#define __CustomActionsEditorManager_h__

#if _MSC_VER > 1000
	#pragma once
#endif

// Forward declarations.

//////////////////////////////////////////////////////////////////////////
// Custom Actions Editor Manager
//////////////////////////////////////////////////////////////////////////
class CCustomActionsEditorManager
{
public:
	CCustomActionsEditorManager();
	~CCustomActionsEditorManager();
	void Init(ISystem* system);

	void ReloadCustomActionGraphs();
	void SaveCustomActionGraphs();
	void SaveAndReloadCustomActionGraphs();
	bool NewCustomAction(CString& filename);
	void GetCustomActions(std::vector<CString>& values) const;

private:
	void LoadCustomActionGraphs();
	void FreeCustomActionGraphs();
};

#endif // __CustomActionsEditorManager_h__

