// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Intel Laptop Gaming TDK support. 
             Battery and wireless signal status.

-------------------------------------------------------------------------
История:
- 29:06:2007: Created by Sergiy Shaykin

*************************************************************************/

#ifndef __LAPTOPUTIL_H__
#define __LAPTOPUTIL_H__

#pragma once

class CLaptopUtil //: public IGameFrameworkListener
{
public:
	CLaptopUtil();
	~CLaptopUtil();

	// Call before getting params. You can call not in each frames
	void Update();

	bool IsLaptop() {return m_isLaptop;}

	// Is Battery like power source
	bool IsBattteryPowerSrc() {return m_isBattery;}
	// Get Battery Life in percent
	i32 GetBatteryLife() {return m_percentBatLife;}
	// Get Battery Life in second
	u64 GetBattteryLifeTime() {return m_secBatLife;}

	bool IsWLan() {return m_isWLan;}
	// Get WLan Signal Strength in percent
	i32 GetWLanSignalStrength() {return m_signalStrength;}

	/* 
	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime);
	virtual void OnSaveGame(ISaveGame* pSaveGame) {};
	virtual void OnLoadGame(ILoadGame* pLoadGame) {};
	virtual void OnLevelEnd(tukk nextLevel) {};
	virtual void OnActionEvent(const SActionEvent& event) {};
	/* */

	static i32 g_show_laptop_status_test;
	static i32 g_show_laptop_status;

private:
	void Init();

private:
	bool m_isLaptop;
	u64 m_secBatLife;
	i32 m_percentBatLife;
	bool m_isWLan;
	i32 m_signalStrength;
	bool m_isBattery;
};



#endif // __LAPTOPUTIL_H__