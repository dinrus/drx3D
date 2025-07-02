// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Iron Sight

-------------------------------------------------------------------------
История:
- 28:10:2005   16:00 : Created by M�rcio Martins

*************************************************************************/
#pragma once

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include <drx3D/Game/IronSight.h>

class CScope : public CIronSight
{
	public:
		DRX_DECLARE_GTI(CScope);

	CScope();

	// IZoomMode
	virtual void Update(float frameTime, u32 frameId) override;

	virtual void GetMemoryUsage(IDrxSizer * s) const override;

	virtual void UpdateFPView(float frameTime) override {}
	// ~IZoomMode

	// CIronSight
	virtual void OnEnterZoom() override;
	virtual void OnLeaveZoom() override;
	virtual void OnZoomStep(bool zoomingIn, float t) override;
	virtual void OnZoomedOut() override;
	virtual void Activate(bool activate) override;
	virtual bool IsScope() const override { return true; }
	// ~CIronSight

protected:
	float m_showTimer;
	float m_hideTimer;
	
private:
	void InformActorOfScope(bool active);
	void ToggleScopeVisionMode(bool enabled, bool toggleOffscreen);
};

#endif // __SCOPE_H__