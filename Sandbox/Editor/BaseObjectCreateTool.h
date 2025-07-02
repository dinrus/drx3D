// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "EditTool.h"

class QMimeData;

class CBaseObjectCreateTool : public CEditTool
{
	DECLARE_DYNAMIC(CBaseObjectCreateTool)

public:
	CBaseObjectCreateTool();
	virtual ~CBaseObjectCreateTool();

public:
	// Overides from CEditTool
	virtual bool MouseCallback(CViewport* pView, EMouseEvent eventId, CPoint& point, i32 flags) override;
	virtual bool OnDragEvent(CViewport* pView, EDragEvent eventId, QEvent* pEvent, u32 flags) override;
	virtual bool OnKeyDown(CViewport* pView, u32 key, u32 repCnt, u32 flags) override;

private:
	virtual i32  ObjectMouseCallback(CViewport* pView, EMouseEvent eventId, const CPoint& point, u32 flags) = 0;

	virtual bool CanStartCreation() = 0;
	// Has view and click point parameter to calculate a start location for the newly created object
	virtual void StartCreation(CViewport* pView = nullptr, const CPoint& point = CPoint()) = 0;
	virtual void FinishObjectCreation() = 0;
	virtual void CancelCreation(bool clear = true) = 0;
	virtual bool IsValidDragData(const QMimeData* pData, bool acceptValue) = 0;
	virtual bool ObjectWasCreated() const = 0;

	virtual void FinishCreation(bool restart, CViewport* pView = nullptr, const CPoint& point = CPoint());
};

