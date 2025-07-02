// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   PersonalSignalTimer.h
   $Id$
   $DateTime$
   Описание: Upr per-actor signal timers
   ---------------------------------------------------------------------
   История:
   - 07:05:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#ifndef _PERSONAL_SIGNAL_TIMER_H_
#define _PERSONAL_SIGNAL_TIMER_H_

#include "AIProxy.h"

class CSignalTimer;

class CPersonalSignalTimer : public IAIProxyListener
{

public:
	// Base ----------------------------------------------------------
	CPersonalSignalTimer(CSignalTimer* pParent);
	virtual ~CPersonalSignalTimer();
	bool Init(EntityId Id, tukk sSignal);
	bool Update(float fElapsedTime, u32 uDebugOrder = 0);
	void ForceReset(bool bAlsoEnable = true);
	void OnProxyReset();

	// Utils ---------------------------------------------------------
	void          SetEnabled(bool bEnabled);
	void          SetRate(float fNewRateMin, float fNewRateMax);
	EntityId      GetEntityId() const;
	const string& GetSignalString() const;

private:
	void           Reset(bool bAlsoEnable = true);
	void           SendSignal();
	IEntity*       GetEntity();
	IEntity const* GetEntity() const;
	void           DebugDraw(u32 uOrder) const;

	// IAIProxyListener
	void         SetListener(bool bAdd);
	virtual void OnAIProxyEnabled(bool bEnabled);
	// ~IAIProxyListener

private:

	bool          m_bInit;
	CSignalTimer* m_pParent;
	EntityId      m_EntityId;
	string        m_sSignal;
	float         m_fRateMin;
	float         m_fRateMax;
	float         m_fTimer;
	float         m_fTimerSinceLastReset;
	i32           m_iSignalsSinceLastReset;
	bool          m_bEnabled;
	IFFont*       m_pDefaultFont;
};
#endif // _PERSONAL_SIGNAL_TIMER_H_
