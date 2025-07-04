// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SliderCtrlEx_h__
#define __SliderCtrlEx_h__
#pragma once

#include <drx3D/CoreX/functor.h>

//////////////////////////////////////////////////////////////////////////
class PLUGIN_API CSliderCtrlEx : public CSliderCtrl
{
public:
	typedef Functor1<CSliderCtrlEx*> UpdateCallback;

	DECLARE_DYNAMIC(CSliderCtrlEx)
	CSliderCtrlEx();

	//////////////////////////////////////////////////////////////////////////
	virtual void    EnableUndo(const CString& undoText);
	virtual void    SetUpdateCallback(const UpdateCallback& cb) { m_updateCallback = cb; };

	virtual void    SetRangeFloat(float min, float max, float step = 0.f);
	virtual void    SetValue(float val);
	virtual float   GetValue() const;
	virtual CString GetValueAsString() const;

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

	bool SetThumb(const CPoint& pt);
	void PostMessageToParent(i32k nTBCode);

protected:
	bool           m_bDragging;
	bool           m_bDragChanged;

	float          m_min, m_max;
	mutable float  m_value;
	float          m_lastUpdateValue;
	CPoint         m_mousePos;

	bool           m_noNotify;
	bool           m_integer;

	bool           m_bUndoEnabled;
	bool           m_bUndoStarted;
	bool           m_bDragged;
	CString        m_undoText;
	bool           m_bLocked;
	bool           m_bInNotifyCallback;

	UpdateCallback m_updateCallback;
};

//////////////////////////////////////////////////////////////////////////
class PLUGIN_API CSliderCtrlCustomDraw : public CSliderCtrlEx
{
public:
	DECLARE_DYNAMIC(CSliderCtrlCustomDraw)

	CSliderCtrlCustomDraw() { m_tickFreq = 1; m_selMin = m_selMax = 0; }
	void SetTicFreq(i32 nFreq) { m_tickFreq = nFreq; };
	void SetSelection(i32 nMin, i32 nMax);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

	void         DrawTicks(CDC* pDC);
	void         DrawTick(CDC* pDC, i32 x, bool bMajor = false);
	i32          ValueToPos(i32 n);

private:
	i32   m_selMin, m_selMax;
	i32   m_tickFreq;
	CRect m_channelRc;
};

#endif // __SliderCtrlEx_h__

