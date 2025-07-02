// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _TINYCOOLEDIT_H_
#define _TINYCOOLEDIT_H_

#ifndef _TINY_WINDOW_H_
	#error "_TinyCoolEdit require <_TinyWindow.h>"
#endif

class _TinyCoolEdit : public _TinyRichEdit
{
	virtual BOOL Create(ULONG nID = 0, DWORD dwStyle = WS_VISIBLE, DWORD dwExStyle = 0, const RECT* pRect = NULL, _TinyWindow* pParentWnd = NULL)
	{
		BOOL bRes = _TinyEdit::Create(nID, dwStyle, dwExStyle, pRect, pParentWnd);
		if (!bRes) return FALSE;
		return TRUE;
	}
	bool Reshape(i32 w, i32 h)
	{
	}
};

#endif //_TINYCOOLEDIT_H_
