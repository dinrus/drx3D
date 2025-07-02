// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  Debug history interface
   -------------------------------------------------------------------------
   История:
   - 12/15/2006   : Stas Spivakov, Created

*************************************************************************/

#pragma once

struct IDebugHistory
{
	// <interfuscator:shuffle>
	virtual ~IDebugHistory(){}
	virtual void SetVisibility(bool show) = 0;

	virtual void SetName(tukk newName) = 0;

	virtual void SetupLayoutAbs(float leftx, float topy, float width, float height, float margin) = 0;
	virtual void SetupLayoutRel(float leftx, float topy, float width, float height, float margin) = 0;
	virtual void SetupScopeExtent(float outermin, float outermax, float innermin, float innermax) = 0;
	virtual void SetupScopeExtent(float outermin, float outermax) = 0;
	//    virtual void SetupGrid(i32 x, i32 y) = 0;
	virtual void SetupColors(ColorF curvenormal, ColorF curveclamped, ColorF box, ColorF gridline, ColorF gridnumber, ColorF name) = 0;
	virtual void SetGridlineCount(i32 nGridlinesX, i32 nGridlinesY) = 0;

	virtual void AddValue(float value) = 0;
	virtual void ClearHistory() = 0;
	// if i don't get a value in a frame, then i'll automatically add this value
	virtual void SetDefaultValue(float x) = 0;
	// </interfuscator:shuffle>
};

struct IDebugHistoryUpr
{
	// <interfuscator:shuffle>
	virtual ~IDebugHistoryUpr(){}
	virtual IDebugHistory* CreateHistory(tukk id, tukk name = 0) = 0;
	virtual void           RemoveHistory(tukk name) = 0;
	virtual IDebugHistory* GetHistory(tukk name) = 0;
	virtual void           Clear() = 0;
	virtual void           GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	virtual void           Release() = 0;

	virtual void           LayoutHelper(tukk id, tukk name, bool visible, float minout, float maxout, float minin, float maxin, float x, float y, float w = 1.0f, float h = 1.0f) = 0;
	// </interfuscator:shuffle>
};
