// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MiniInfoBox.h
//  Created:     26/08/2009 by Timur.
//  Описание: Button implementation in the MiniGUI
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MiniInfoBox_h__
#define __MiniInfoBox_h__

#include <drx3D/Sys/MiniGUI.h>

MINIGUI_BEGIN

//////////////////////////////////////////////////////////////////////////
// Root window all other controls derive from
class CMiniInfoBox : public CMiniCtrl, public IMiniInfoBox
{
public:
	CMiniInfoBox();

	//////////////////////////////////////////////////////////////////////////
	// CMiniCtrl interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EMiniCtrlType GetType() const { return eCtrlType_InfoBox; }
	virtual void          OnPaint(CDrawContext& dc);
	virtual void          OnEvent(float x, float y, EMiniCtrlEvent event);
	virtual void          Reset();
	virtual void          SaveState();
	virtual void          RestoreState();
	virtual void          AutoResize();
	//////////////////////////////////////////////////////////////////////////

	virtual void SetTextIndent(float x) { m_fTextIndent = x; }
	virtual void SetTextSize(float sz)  { m_fTextSize = sz; }
	virtual void ClearEntries();
	virtual void AddEntry(tukk str, ColorB col, float textSize);

	//////////////////////////////////////////////////////////////////////////
	// IMiniTable interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual bool IsHidden()      { return CheckFlag(eCtrl_Hidden); }
	virtual void Hide(bool stat) { SetVisible(!stat); }

public:

	//////////////////////////////////////////////////////////////////////////

	static i32k MAX_TEXT_LENGTH = 64;

	struct SInfoEntry
	{
		char   text[MAX_TEXT_LENGTH];
		ColorB color;
		float  textSize;
	};
	//////////////////////////////////////////////////////////////////////////

protected:

	std::vector<SInfoEntry> m_entries;
	float                   m_fTextIndent;

};

MINIGUI_END

#endif // __MiniInfoBox_h__
