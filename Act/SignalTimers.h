// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   SignalTimer.h
   $Id$
   $DateTime$
   Описание: Signal entities based on configurable timers
   ---------------------------------------------------------------------
   История:
   - 16:04:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#ifndef _SIGNAL_TIMER_H_
#define _SIGNAL_TIMER_H_

class CPersonalSignalTimer;

class CSignalTimer
{

public:

	/*$1- Singleton Stuff ----------------------------------------------------*/
	static CSignalTimer& ref();
	static bool          Create();
	static void          Shutdown();
	void                 Reset();

	/*$1- IEditorSetGameModeListener -----------------------------------------*/
	void OnEditorSetGameMode(bool bGameMode);
	void OnProxyReset(EntityId IdEntity);

	/*$1- Basics -------------------------------------------------------------*/
	void Init();
	bool Update(float fElapsedTime);

	/*$1- Utils --------------------------------------------------------------*/
	bool EnablePersonalUpr(EntityId IdEntity, tukk sSignal);
	bool DisablePersonalSignalTimer(EntityId IdEntity, tukk sSignal);
	bool ResetPersonalTimer(EntityId IdEntity, tukk sSignal);
	bool EnableAllPersonalUprs(EntityId IdEntity);
	bool DisablePersonalSignalTimers(EntityId IdEntity);
	bool ResetPersonalTimers(EntityId IdEntity);
	bool SetTurnRate(EntityId IdEntity, tukk sSignal, float fTime, float fTimeMax = -1.0f);
	void SetDebug(bool bDebug);
	bool GetDebug() const;

protected:

	/*$1- Creation and destruction via singleton -----------------------------*/
	CSignalTimer();
	virtual ~CSignalTimer();

	/*$1- Utils --------------------------------------------------------------*/
	CPersonalSignalTimer* GetPersonalSignalTimer(EntityId IdEntity, tukk sSignal) const;
	CPersonalSignalTimer* CreatePersonalSignalTimer(EntityId IdEntity, tukk sSignal);

private:

	/*$1- Members ------------------------------------------------------------*/
	bool                               m_bInit;
	bool                               m_bDebug;
	static CSignalTimer*               m_pInstance;
	std::vector<CPersonalSignalTimer*> m_vecPersonalSignalTimers;
};
#endif // _SIGNAL_TIMER_H_
