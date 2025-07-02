// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MiniTable.cpp
//  Created:     01/12/2009 by AndyM.
//  Описание: Table implementation in the MiniGUI
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _MINITABLE_H_
#define _MINITABLE_H_

#include <drx3D/Sys/MiniGUI.h>

MINIGUI_BEGIN

class CMiniTable : public CMiniCtrl, public IMiniTable, public IInputEventListener
{
public:
	CMiniTable();

	//////////////////////////////////////////////////////////////////////////
	// CMiniCtrl interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EMiniCtrlType GetType() const { return eCtrlType_Table; }
	virtual void          OnPaint(CDrawContext& dc);
	virtual void          OnEvent(float x, float y, EMiniCtrlEvent event);
	virtual void          Reset();
	virtual void          SaveState();
	virtual void          RestoreState();
	virtual void          AutoResize();
	virtual void          SetVisible(bool state);

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IMiniTable interface implementation.
	//////////////////////////////////////////////////////////////////////////

	//Add a new column to the table, return column index
	virtual i32 AddColumn(tukk name);

	//Remove all columns an associated data
	virtual void RemoveColumns();

	//Add data to specified column (add onto the end), return row index
	virtual i32 AddData(i32 columnIndex, ColorB col, tukk format, ...);

	//Clear all data from table
	virtual void ClearTable();

	virtual bool IsHidden() { return CheckFlag(eCtrl_Hidden); }

	virtual void Hide(bool stat);

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& rInputEvent);

	static i32k MAX_TEXT_LENGTH = 64;

protected:

	struct SCell
	{
		char   text[MAX_TEXT_LENGTH];
		ColorB col;
	};

	struct SColumn
	{
		char               name[MAX_TEXT_LENGTH];
		float              width;
		std::vector<SCell> cells;
	};

	std::vector<SColumn> m_columns;
	i32                  m_pageSize;
	i32                  m_pageNum;
};

MINIGUI_END

#endif// _MINITABLE_H_
