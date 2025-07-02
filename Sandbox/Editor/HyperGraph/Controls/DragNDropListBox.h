// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CDragNDropListBox : public CListBox
{
#define TID_SCROLLDOWN  100
#define TID_SCROLLUP    101

	DECLARE_DYNAMIC(CDragNDropListBox)

public:
	typedef Functor2<i32, i32> DragNDropCallback;

	CDragNDropListBox(const DragNDropCallback &cb); // cb can be nullptr
	virtual ~CDragNDropListBox() {}

	void OnDrop(i32 curIdx, i32 newIdx);

protected:
	void DrawLine(i32 idx, bool isActiveLine);
	void UpdateSelection(i32 hoverIdx);

	void Scroll(CPoint Point, CRect ClientRect);
	void StopScrolling();
	void OnTimer(UINT nIDEvent);

	void DropDraggedItem();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd* pWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

protected:
	bool m_isDragging;
	i32 m_itemPrevIdx;
	i32 m_itemNextIdx;
	i32 m_Interval;

	DragNDropCallback m_dropCb;
};

