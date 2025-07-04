// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PickObjectTool_h__
#define __PickObjectTool_h__

#if _MSC_VER > 1000
	#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
class CPickObjectTool : public CEditTool
{
public:
	DECLARE_DYNAMIC(CPickObjectTool)

	CPickObjectTool(IPickObjectCallback* callback, CRuntimeClass* targetClass = NULL);

	//! If set to true, pick tool will not stop picking after first pick.
	void SetMultiplePicks(bool bEnable) { m_bMultiPick = bEnable; };

	// Overrides from CEditTool
	virtual string GetDisplayName() const override { return "Pick Object"; }
	bool           MouseCallback(CViewport* view, EMouseEvent event, CPoint& point, i32 flags);

	virtual void   Display(DisplayContext& dc)                                           {};
	virtual bool   OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags);
	virtual bool   OnKeyUp(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) { return false; };

protected:
	virtual ~CPickObjectTool();
	// Delete itself.
	void DeleteThis() { delete this; };

private:
	bool IsRelevant(CBaseObject* obj);

	//! Object that requested pick.
	IPickObjectCallback* m_callback;

	//! If target class specified, will pick only objects that belongs to that runtime class.
	CRuntimeClass* m_targetClass;

	bool           m_bMultiPick;
};

#endif // __PickObjectTool_h__

