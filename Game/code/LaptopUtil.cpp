// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/CoreX/Game/IGameFramework.h>

#ifdef USE_LAPTOPUTIL
#include <drx3D/Game/LaptopUtil.h>



#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT

#include #include <drx3D/Game/../SDKs/IntelLaptop/Include/IntelGamingTDKAPI.h>

/*
class CGamingTDKObserver : public Notifiable
{
public:
	CGamingTDKObserver() : Notifiable(), m_pLaptopUtil(0){}
	
	void Init(CLaptopUtil * pLaptopUtil)
	{
		m_pLaptopUtil = pLaptopUtil;
	}
	
	virtual void notify(i32 event, long value = 0);
private:
	CLaptopUtil * m_pLaptopUtil;
};

void CGamingTDKObserver::notify(i32 event, long value)
{
  CRITICAL_SECTION cs;
  InitializeCriticalSection(&cs);
  EnterCriticalSection(&cs);
	if(m_pLaptopUtil)
		m_pLaptopUtil->Update();
  LeaveCriticalSection(&cs);
  DeleteCriticalSection(&cs);
}

namespace
{
	CGamingTDKObserver observer;
}
*/

#endif



i32 CLaptopUtil::g_show_laptop_status_test=0;
i32 CLaptopUtil::g_show_laptop_status=0;

CLaptopUtil::CLaptopUtil()
{
	REGISTER_CVAR2(	"g_show_laptop_status", &CLaptopUtil::g_show_laptop_status, 0, 0,
											"Show laptop status");

	REGISTER_CVAR2(	"g_show_laptop_status_test", &CLaptopUtil::g_show_laptop_status_test, 0, 0,
											"Show fake laptop status for testing");

	m_isLaptop = false;
	m_percentBatLife = 0;
	m_secBatLife = 0;
	m_isWLan = false;
	m_isBattery = false;
	m_signalStrength = false;

	Init();

	if(m_isLaptop)
	{
		Update();
		//gEnv->pGame->GetIGameFramework()->RegisterListener(this, "laptoputil", FRAMEWORKLISTENERPRIORITY_MENU);
	}
}


CLaptopUtil::~CLaptopUtil()
{
	if(m_isLaptop)
	{
		//gEnv->pGame->GetIGameFramework()->UnregisterListener(this);

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
		/*
		IntelLaptopGamingTDKInterface *IGT = IntelLaptopGamingTDKInterface::GetTDKInterface();
		if(IGT)
		{
			IGT->UnregisterEventObserver(LOW_BATTERY_REACHED);
			IGT->UnregisterEventObserver(BATTERY_LIFE_PRCNT_CHANGED);
			IGT->UnregisterEventObserver(POWER_SRC_CHANGED);
			IGT->UnregisterEventObserver(WIRELESS_SIGNAL_STRENGTH);
		}
		*/
#endif 
	}
}


void CLaptopUtil::Update()
{
#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
	IntelLaptopGamingTDKInterface *IGT = IntelLaptopGamingTDKInterface::GetTDKInterface();
	if(!IGT)
		return;
	m_isBattery =(IGT->GetPowerSrc()==Battery_Power);
	m_percentBatLife = IGT->GetPercentBatteryLife();
	m_secBatLife = IGT->GetSecBatteryLifeTimeRemaining();
	m_isWLan = IGT->IsWirelessAdapterConnected() &  IGT->IsWirelessAdapterEnabled();
	m_signalStrength = IGT->Get80211SignalStrength();

	//gEnv->pLog->Log("Battery: %d, %d", m_percentBatLife, m_isBattery);
	//gEnv->pLog->Log("WLan: %d, %d", m_signalStrength, m_isWLan);
#endif
}


void CLaptopUtil::Init()
{
#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
	IntelLaptopGamingTDKInterface *IGT = IntelLaptopGamingTDKInterface::GetTDKInterface();
	if(!IGT)
		return;
	m_isLaptop = IGT->IsLaptop();
	/*
	if(m_isLaptop)
	{
		observer.Init(this);
		IGT->RegisterEventObserver(LOW_BATTERY_REACHED, &observer);
		IGT->RegisterEventObserver(BATTERY_LIFE_PRCNT_CHANGED, &observer);
		IGT->RegisterEventObserver(POWER_SRC_CHANGED, &observer);
		IGT->RegisterEventObserver(WIRELESS_SIGNAL_STRENGTH, &observer);
		gEnv->pLog->Log("It is Laptop.");
	}
	else
		gEnv->pLog->Log("It is not Laptop.");
	/* */
#endif
}

/*
void CLaptopUtil::OnPostUpdate(float fDeltaTime)
{
	if(!m_isLaptop)
		return;
	Update();
}
*/


/*
	if (ms_sys_flash_info)
		gEnv->pRenderer->SF_Flush();

	{
    i32* isLost((i32*) m_pRenderer->GetXRender()->EF_Query(EFQ_DeviceLost, 0));
		if (m_pMovieView && !*isLost)
		{
			UpdateRenderFlags();
			m_pMovieView->Display();
		}

		if (ms_sys_flash_info)
			gEnv->pRenderer->SF_Flush();
	}
*/
#endif