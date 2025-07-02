// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IUndoObject.h>

class CAsset;

namespace DrxSchematycEditor {

class CMainWindow;

class CScriptUndoObject : public IUndoObject
{
public:
	CScriptUndoObject(tukk szDesc, CMainWindow& editor);
	~CScriptUndoObject();

	// IUndoObject
	virtual tukk GetDescription() final { return m_desc.c_str(); }

	virtual void        Undo(bool bUndo) override;
	virtual void        Redo() override;
	// ~UndoObject

private:
	CMainWindow* m_pEditor;
	CAsset*      m_pAsset;
	string       m_desc;
	XmlNodeRef   m_before;
	XmlNodeRef   m_after;
};

}

