// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef _SHOTGUN_H_
#define _SHOTGUN_H_

#include <drx3D/Game/Single.h>

class CShotgun :
	public CSingle
{
	struct BeginReloadLoop;
	class PartialReloadAction;
	class ReloadEndAction;
	class ScheduleReload;

public:
	DRX_DECLARE_GTI(CShotgun);

	virtual void GetMemoryUsage(IDrxSizer * s) const override;
	void GetInternalMemoryUsage(IDrxSizer * s) const;
	virtual void Activate(bool activate) override;
	virtual void StartReload(i32 zoomed) override;
	void ReloadShell(i32 zoomed);
	virtual void EndReload(i32 zoomed) override;
	using CSingle::EndReload;
	
	virtual void CancelReload() override;

	virtual bool CanFire(bool considerAmmo) const override;

	virtual bool Shoot(bool resetAnimation, bool autoreload = true , bool isRemote=false ) override;
	virtual void NetShootEx(const Vec3 &pos, const Vec3 &dir, const Vec3 &vel, const Vec3 &hit, float extra, i32 ph) override;

	virtual float GetSpreadForHUD() const override;

	virtual u8 GetShotIncrementAmount() override
	{
		return (u8)m_fireParams->shotgunparams.pellets;
	}

private:

	i32   m_max_shells;
	u8 m_shotIndex;

	bool	m_reload_pump;
	bool	m_load_shell_on_end;				
	bool	m_break_reload;
	bool	m_reload_was_broken;

};

#endif
