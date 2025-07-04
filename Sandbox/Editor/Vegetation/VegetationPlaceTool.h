// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "BaseObjectCreateTool.h"

class QMimeData;
class CVegetationObject;
struct CVegetationInstance;

class CVegetationPlaceTool : public CBaseObjectCreateTool
{
	DECLARE_DYNCREATE(CVegetationPlaceTool)

public:
	CVegetationPlaceTool();
	virtual ~CVegetationPlaceTool();

	virtual string GetDisplayName() const override { return "Place Vegetation"; }
	//Selects object type to create (and optional file to use)
	void           SelectObjectToCreate(CVegetationObject* pSelectedObject);

protected:
	// Delete itself.
	virtual void DeleteThis() override { delete this; }

private:
	virtual i32  ObjectMouseCallback(CViewport* pView, EMouseEvent eventId, const CPoint& point, u32 flags) override;
	virtual bool CanStartCreation() override;
	virtual void StartCreation(CViewport* pView = nullptr, const CPoint& point = CPoint()) override;
	virtual void FinishObjectCreation() override;
	virtual void CancelCreation(bool clear = true) override;
	virtual bool IsValidDragData(const QMimeData* pData, bool acceptValue) override;
	virtual bool ObjectWasCreated() const override;

private:
	CVegetationObject*   m_pSelectedObject;
	CVegetationInstance* m_pCreatedInstance;
};

