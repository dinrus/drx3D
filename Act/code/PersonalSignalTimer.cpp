// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   PersonalSignalTimer.cpp
   $Id$
   $DateTime$
   Описание: Upr per-actor signal timers
   ---------------------------------------------------------------------
   История:
   - 07:05:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/PersonalSignalTimer.h>
#include <drx3D/Act/SignalTimers.h>
#include <drx3D/Act/IUIDraw.h>
#include <drx3D/AI/IAIObject.h>
// Описание:
//   Constructor
// Arguments:
//
// Return:
//
CPersonalSignalTimer::CPersonalSignalTimer(CSignalTimer* pParent) :
	m_bInit(false),
	m_pParent(pParent),
	m_EntityId(0),
	m_fRateMin(4.0f),
	m_fRateMax(6.0f),
	m_fTimer(0.0f),
	m_bEnabled(false),
	m_fTimerSinceLastReset(0.0f),
	m_iSignalsSinceLastReset(0)
{
	DRX_ASSERT(pParent != NULL);

	m_pDefaultFont = gEnv->pDrxFont->GetFont("default");
	DRX_ASSERT(m_pDefaultFont);
}

// Описание:
//   Destructor
// Arguments:
//
// Return:
//
CPersonalSignalTimer::~CPersonalSignalTimer()
{
	SetListener(false);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalSignalTimer::Init(EntityId Id, tukk sSignal)
{
	DRX_ASSERT(m_bInit == false);
	DRX_ASSERT(sSignal != NULL);

	m_EntityId = Id;
	m_sSignal = sSignal;
	m_bInit = true;
	SetListener(true);

	return(m_bInit);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalSignalTimer::Update(float fElapsedTime, u32 uDebugOrder)
{
	DRX_ASSERT(m_bInit == true);

	bool bRet = true;
	if (m_bEnabled == true)
	{
		m_fTimer -= fElapsedTime;
		m_fTimerSinceLastReset += fElapsedTime;

		if (m_fTimer < 0.0f)
		{
			SendSignal();
			Reset();
		}
	}

	if (uDebugOrder > 0)
	{
		DebugDraw(uDebugOrder);
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::ForceReset(bool bAlsoEnable)
{
	DRX_ASSERT(m_bInit == true);

	m_fTimerSinceLastReset = 0.0f;
	m_iSignalsSinceLastReset = 0;
	Reset(bAlsoEnable);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::OnProxyReset()
{
	// Reset listener
	SetListener(true);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::Reset(bool bAlsoEnable)
{
	DRX_ASSERT(m_bInit == true);

	if (m_fRateMin < m_fRateMax)
	{
		m_fTimer = drx_random(m_fRateMin, m_fRateMax);
	}
	else
	{
		m_fTimer = m_fRateMin;
	}

	SetEnabled(bAlsoEnable);
}

// Описание:
//
// Arguments:
//
// Return:
//
EntityId CPersonalSignalTimer::GetEntityId() const
{
	DRX_ASSERT(m_bInit == true);

	return(m_EntityId);
}

// Описание:
//
// Arguments:
//
// Return:
//
const string& CPersonalSignalTimer::GetSignalString() const
{
	DRX_ASSERT(m_bInit == true);

	return(m_sSignal);
}

// Описание:
//
// Arguments:
//
// Return:
//
IEntity* CPersonalSignalTimer::GetEntity()
{
	DRX_ASSERT(m_bInit == true);

	return(gEnv->pEntitySystem->GetEntity(m_EntityId));
}

// Описание:
//
// Arguments:
//
// Return:
//
IEntity const* CPersonalSignalTimer::GetEntity() const
{
	DRX_ASSERT(m_bInit == true);

	return(gEnv->pEntitySystem->GetEntity(m_EntityId));
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::SetRate(float fNewRateMin, float fNewRateMax)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(fNewRateMin > 0.0f);
	DRX_ASSERT(fNewRateMax > 0.0f);

	m_fRateMin = fNewRateMin;
	m_fRateMax = max(fNewRateMin, fNewRateMax);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::SendSignal()
{
	DRX_ASSERT(m_bInit == true);

	IEntity* pEntity = GetEntity();
	if (pEntity && gEnv->pAISystem)
	{
		IAISignalExtraData* pData = gEnv->pAISystem->CreateSignalExtraData();
		pData->iValue = ++m_iSignalsSinceLastReset;
		pData->fValue = m_fTimerSinceLastReset;

		gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 1, m_sSignal, pEntity->GetAI(), pData);
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::SetEnabled(bool bEnabled)
{
	DRX_ASSERT(m_bInit == true);

	if (bEnabled != m_bEnabled)
	{
		m_fTimerSinceLastReset = 0.0f;
		m_iSignalsSinceLastReset = 0;
		m_bEnabled = bEnabled;
		if (m_pParent->GetDebug() == true)
		{
			gEnv->pLog->Log(
			  "PersonalSignalTimer [%d]: Signal [%s] is %s",
			  m_EntityId,
			  m_sSignal.c_str(),
			  (bEnabled) ? "enabled" : "disabled");
		}
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::DebugDraw(u32 uOrder) const
{
	DRX_ASSERT(m_bInit == true);

	IUIDraw* pUI = CDrxAction::GetDrxAction()->GetIUIDraw();
	float x = 120.0f;
	float y = 100.0f + (float(uOrder) * 10.0f);
	float r = 0.0f;
	float g = 8.0f;
	float b = 0.0f;

	char txt[512] = "\0";
	if (GetEntity())
		drx_sprintf(txt, "%s > %s: %0.1f / %0.1f", GetEntity()->GetName(), m_sSignal.c_str(), m_fTimer, m_fRateMax);

	if (m_bEnabled == false)
	{
		r = 8.0f;
		g = b = 0.0f;
	}
	else if (m_fTimer < 0.5f)
	{
		r = g = 8.0f;
		b = 0.0f;
	}

	IRenderAuxText::Draw2dLabel( x,y, 13.0f, ColorF(r,g,b), false, "%s", txt);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::SetListener(bool bAdd)
{
	IEntity* pEntity = GetEntity();
	;
	if (pEntity)
	{
		IAIObject* pAIObject = pEntity->GetAI();
		if (pAIObject)
		{
			CAIProxy* pAIProxy = (CAIProxy*)pAIObject->GetProxy();
			if (pAIProxy)
			{
				if (bAdd)
					pAIProxy->AddListener(this);
				else
					pAIProxy->RemoveListener(this);
			}
		}
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalSignalTimer::OnAIProxyEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		ForceReset();
		SetEnabled(true);
	}
	else
	{
		SetEnabled(false);
	}
}
