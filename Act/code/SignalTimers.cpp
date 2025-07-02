// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

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
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/SignalTimers.h>
#include <drx3D/Act/PersonalSignalTimer.h>

CSignalTimer* CSignalTimer::m_pInstance = NULL;

// Описание:
//   Constructor
// Arguments:
//
// Return:
//
CSignalTimer::CSignalTimer()
{
	m_bInit = false;
	m_bDebug = false;
}

// Описание:
//   Destructor
// Arguments:
//
// Return:
//
CSignalTimer::~CSignalTimer()
{
	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		SAFE_DELETE(m_vecPersonalSignalTimers[uIndex]);
	}

	m_vecPersonalSignalTimers.clear();
}

// Описание:
//   Create
// Arguments:
//
// Return:
//
CSignalTimer& CSignalTimer::ref()
{
	DRX_ASSERT(NULL != m_pInstance);
	return(*m_pInstance);
}

// Описание:
//   Create
// Arguments:
//
// Return:
//
bool CSignalTimer::Create()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new CSignalTimer();
	}
	else
	{
		DRX_ASSERT("Trying to Create() the singleton more than once");
	}

	return(m_pInstance != NULL);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CSignalTimer::Init()
{
	if (m_bInit == false)
	{
		m_bInit = true;
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CSignalTimer::Reset()
{
	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		m_vecPersonalSignalTimers[uIndex]->ForceReset(false);
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CSignalTimer::OnEditorSetGameMode(bool bGameMode)
{
	DRX_ASSERT(m_bInit == true);

	if (bGameMode == true)
	{
		Reset();
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CSignalTimer::OnProxyReset(EntityId IdEntity)
{
	DRX_ASSERT(IdEntity > 0);

	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		if (m_vecPersonalSignalTimers[uIndex]->GetEntityId() == IdEntity)
		{
			m_vecPersonalSignalTimers[uIndex]->OnProxyReset();
		}
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::Update(float fElapsedTime)
{
	bool bRet = true;
	u32 uOrder = 0;
	std::vector<CPersonalSignalTimer*>::iterator iter = m_vecPersonalSignalTimers.begin();

	while (iter != m_vecPersonalSignalTimers.end())
	{
		CPersonalSignalTimer* pPersonal = *iter;

		if (pPersonal && pPersonal->Update(fElapsedTime, ((m_bDebug == true) ? ++uOrder : 0)) == false)
		{
			SAFE_DELETE(pPersonal);
			iter = m_vecPersonalSignalTimers.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CSignalTimer::Shutdown()
{
	SAFE_DELETE(m_pInstance);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::EnablePersonalUpr(EntityId IdEntity, tukk sSignal)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);

	bool bRet = true;
	CPersonalSignalTimer* pPersonal = GetPersonalSignalTimer(IdEntity, sSignal);

	if (pPersonal == NULL)
	{
		pPersonal = CreatePersonalSignalTimer(IdEntity, sSignal);
		bRet = false;
	}

	pPersonal->SetEnabled(true);

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::DisablePersonalSignalTimer(EntityId IdEntity, tukk sSignal)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);

	bool bRet = false;
	CPersonalSignalTimer* pPersonal = GetPersonalSignalTimer(IdEntity, sSignal);

	if (pPersonal != NULL)
	{
		pPersonal->SetEnabled(false);
		bRet = true;
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::ResetPersonalTimer(EntityId IdEntity, tukk sSignal)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);

	bool bRet = false;
	CPersonalSignalTimer* pPersonal = GetPersonalSignalTimer(IdEntity, sSignal);

	if (pPersonal != NULL)
	{
		pPersonal->ForceReset();
		bRet = true;
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::EnableAllPersonalUprs(EntityId IdEntity)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	bool bRet = false;

	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		if (m_vecPersonalSignalTimers[uIndex]->GetEntityId() == IdEntity)
		{
			m_vecPersonalSignalTimers[uIndex]->SetEnabled(true);
			bRet = true;
		}
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::DisablePersonalSignalTimers(EntityId IdEntity)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	bool bRet = false;

	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		if (m_vecPersonalSignalTimers[uIndex]->GetEntityId() == IdEntity)
		{
			m_vecPersonalSignalTimers[uIndex]->SetEnabled(false);
			bRet = true;
		}
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::ResetPersonalTimers(EntityId IdEntity)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	bool bRet = false;

	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		if (m_vecPersonalSignalTimers[uIndex]->GetEntityId() == IdEntity)
		{
			m_vecPersonalSignalTimers[uIndex]->ForceReset();
			bRet = true;
		}
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::SetTurnRate(EntityId IdEntity, tukk sSignal, float fTime, float fTimeMax)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(fTime > 0.0f);
	DRX_ASSERT(sSignal != NULL);

	bool bRet = false;
	CPersonalSignalTimer* pPersonal = GetPersonalSignalTimer(IdEntity, sSignal);

	if (pPersonal != NULL)
	{
		pPersonal->SetRate(fTime, max(fTimeMax, fTime));
		pPersonal->ForceReset();
		bRet = true;
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
CPersonalSignalTimer* CSignalTimer::GetPersonalSignalTimer(EntityId IdEntity, tukk sSignal) const
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);

	CPersonalSignalTimer* pPersonal = NULL;

	for (u32 uIndex = 0; uIndex < m_vecPersonalSignalTimers.size(); ++uIndex)
	{
		if (m_vecPersonalSignalTimers[uIndex]->GetEntityId() == IdEntity &&
		    m_vecPersonalSignalTimers[uIndex]->GetSignalString().compareNoCase(sSignal) == 0)
		{
			pPersonal = m_vecPersonalSignalTimers[uIndex];
			break;
		}
	}

	return(pPersonal);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CSignalTimer::SetDebug(bool bDebug)
{
	DRX_ASSERT(m_bInit == true);

	m_bDebug = bDebug;
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CSignalTimer::GetDebug() const
{
	return(m_bDebug);
}

// Описание:
//
// Arguments:
//
// Return:
//
CPersonalSignalTimer* CSignalTimer::CreatePersonalSignalTimer(EntityId IdEntity, tukk sSignal)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);

	CPersonalSignalTimer* pPersonal = new CPersonalSignalTimer(this);

	if (pPersonal->Init(IdEntity, sSignal) == true)
	{
		m_vecPersonalSignalTimers.push_back(pPersonal);
	}
	else
	{
		SAFE_DELETE(pPersonal);
	}

	return(pPersonal);
}
