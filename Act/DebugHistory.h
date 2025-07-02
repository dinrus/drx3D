// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DebugHistory_H__
#define __DebugHistory_H__

#pragma once

#include <drx3D/Act/IDebugHistory.h>

//--------------------------------------------------------------------------------

extern void Draw2DLine(float x1, float y1, float x2, float y2, ColorF color, float fThickness = 1.f);

//--------------------------------------------------------------------------------

class CDebugHistory : public IDebugHistory
{
public:

	CDebugHistory(tukk name, i32 size);
	~CDebugHistory();

	virtual void SetName(tukk newName);

	virtual void SetVisibility(bool show);

	virtual void SetupLayoutAbs(float leftx, float topy, float width, float height, float margin);
	virtual void SetupLayoutRel(float leftx, float topy, float width, float height, float margin);
	virtual void SetupScopeExtent(float outermin, float outermax, float innermin, float innermax);
	virtual void SetupScopeExtent(float outermin, float outermax);
	virtual void SetupColors(ColorF curvenormal, ColorF curveclamped, ColorF box, ColorF gridline, ColorF gridnumber, ColorF name);
	virtual void SetGridlineCount(i32 nGridlinesX, i32 nGridlinesY);

	virtual void AddValue(float value);
	virtual void ClearHistory();

	void         GetMemoryStatistics(IDrxSizer* s)       { s->Add(*this); }
	void         GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
	void         Render();
	bool         IsVisible() const                       { return m_show; }

	void         SetDefaultValue(float x)                { m_hasDefaultValue = true; m_defaultValue = x; }

private:

	CDebugHistory() {};

	void UpdateExtent();
	void UpdateGridLines();

	tukk        m_szName;

	bool               m_show;

	Vec2               m_layoutTopLeft;
	Vec2               m_layoutExtent;
	float              m_layoutMargin;

	float              m_scopeOuterMax;
	float              m_scopeOuterMin;
	float              m_scopeInnerMax;
	float              m_scopeInnerMin;
	float              m_scopeCurMax;
	float              m_scopeCurMin;

	ColorF             m_colorCurveNormal;
	ColorF             m_colorCurveClamped;
	ColorF             m_colorBox;
	ColorF             m_colorGridLine;
	ColorF             m_colorGridNumber;
	ColorF             m_colorName;

	i32                m_wantedGridLineCountX;
	i32                m_wantedGridLineCountY;
	static u8k GRIDLINE_MAXCOUNT = 10;
	i32                m_gridLineCount;
	float              m_gridLine[GRIDLINE_MAXCOUNT];

	float*             m_pValues;
	i32                m_maxCount;
	i32                m_head;
	i32                m_count;

	i32                m_scopeRefreshDelay;
	i32                m_gridRefreshDelay;

	bool               m_hasDefaultValue;
	float              m_defaultValue;
	bool               m_gotValue;
};

//--------------------------------------------------------------------------------

class CDebugHistoryUpr : public IDebugHistoryUpr
{

	typedef string                      MapKey;
	typedef CDebugHistory*              MapValue;
	typedef std::map<MapKey, MapValue>  Map;
	typedef std::pair<MapKey, MapValue> MapEntry;
	typedef Map::iterator               MapIterator;

public:

	CDebugHistoryUpr()
	{
		Clear();

		if (!m_allhistory)
			m_allhistory = new std::set<CDebugHistoryUpr*>();
		m_allhistory->insert(this);
	}

	~CDebugHistoryUpr()
	{
		Clear();

		m_allhistory->erase(this);
	}

	virtual IDebugHistory* CreateHistory(tukk id, tukk name = 0);
	virtual void           RemoveHistory(tukk id);

	virtual IDebugHistory* GetHistory(tukk id);
	virtual void           Clear()
	{
		MapIterator it = m_histories.begin();
		while (it != m_histories.end())
		{
			CDebugHistory* history = (*it).second;
			delete history;

			++it;
		}

		m_histories.clear();
	}
	virtual void Release();
	virtual void GetMemoryUsage(IDrxSizer* s) const;

	static void  RenderAll();
	static void  SetupRenderer();

	void         LayoutHelper(tukk id, tukk name, bool visible, float minout, float maxout, float minin, float maxin, float x, float y, float w = 1.0f, float h = 1.0f);

private:
	void Render(bool bSetupRenderer = false);

	Map m_histories;
	static std::set<CDebugHistoryUpr*>* m_allhistory;
};

//--------------------------------------------------------------------------------

#endif // __DebugHistory_H__
