// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   RangeSignaling.cpp
   $Id$
   $DateTime$
   Описание: Signal entities based on ranges from other entities
   ---------------------------------------------------------------------
   История:
   - 09:04:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/RangeSignaling.h>
#include <drx3D/Act/PersonalRangeSignaling.h>

CRangeSignaling* CRangeSignaling::m_pInstance = NULL;

// Описание:
//   Constructor
// Arguments:
//
// Return:
//
CRangeSignaling::CRangeSignaling()
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
CRangeSignaling::~CRangeSignaling()
{
	Reset();
}

// Описание:
//   Ref
// Arguments:
//
// Return:
//
CRangeSignaling& CRangeSignaling::ref()
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
bool CRangeSignaling::Create()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new CRangeSignaling();
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
void CRangeSignaling::Init()
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
void CRangeSignaling::Reset()
{
	for (MapPersonals::iterator iter = m_Personals.begin(); iter != m_Personals.end(); ++iter)
	{
		CPersonalRangeSignaling* pPersonal = iter->second;
		SAFE_DELETE(pPersonal);
	}

	m_Personals.clear();
}

// Описание:
//
// Arguments:
//
// Return:
//
void CRangeSignaling::OnEditorSetGameMode(bool bGameMode)
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
void CRangeSignaling::OnProxyReset(EntityId IdEntity)
{
	DRX_ASSERT(IdEntity > 0);

	MapPersonals::iterator iter = m_Personals.find(IdEntity);

	if (iter != m_Personals.end())
	{
		iter->second->OnProxyReset();
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CRangeSignaling::Update(float fElapsedTime)
{
	bool bRet = true;

	for (MapPersonals::iterator iter = m_Personals.begin(); iter != m_Personals.end(); ++iter)
	{
		CPersonalRangeSignaling* pPersonal = iter->second;
		bRet |= pPersonal->Update(fElapsedTime);
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CRangeSignaling::Shutdown()
{
	SAFE_DELETE(m_pInstance);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CRangeSignaling::SetDebug(bool bDebug)
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
bool CRangeSignaling::GetDebug() const
{
	return(m_bDebug);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CRangeSignaling::AddRangeSignal(EntityId IdEntity, float fRadius, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*=NULL*/)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);
	DRX_ASSERT(fRadius > 0.5f);
	DRX_ASSERT(fBoundary >= 0.0f);

	bool bRet = false;
	CPersonalRangeSignaling* pPersonal = GetPersonalRangeSignaling(IdEntity);

	if (pPersonal == NULL)
	{
		pPersonal = CreatePersonalRangeSignaling(IdEntity);
	}

	if (pPersonal != NULL)
	{
		bRet = pPersonal->AddRangeSignal(fRadius, fBoundary, sSignal, pData);
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CRangeSignaling::AddTargetRangeSignal(EntityId IdEntity, EntityId IdTarget, float fRadius, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*= NULL*/)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(IdTarget > 0);
	DRX_ASSERT(sSignal != NULL);
	DRX_ASSERT(fRadius > 0.5f);
	DRX_ASSERT(fBoundary >= 0.0f);

	bool bRet = false;
	CPersonalRangeSignaling* pPersonal = GetPersonalRangeSignaling(IdEntity);

	if (pPersonal == NULL)
	{
		pPersonal = CreatePersonalRangeSignaling(IdEntity);
	}

	if (pPersonal != NULL)
	{
		bRet = pPersonal->AddTargetRangeSignal(IdTarget, fRadius, fBoundary, sSignal, pData);
	}

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CRangeSignaling::AddAngleSignal(EntityId IdEntity, float fAngle, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*=NULL*/)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);
	DRX_ASSERT(sSignal != NULL);
	DRX_ASSERT(fAngle > 0.5f);
	DRX_ASSERT(fBoundary >= 0.0f);

	bool bRet = true;
	CPersonalRangeSignaling* pPersonal = GetPersonalRangeSignaling(IdEntity);

	if (pPersonal == NULL)
	{
		pPersonal = CreatePersonalRangeSignaling(IdEntity);
	}

	bRet = pPersonal->AddAngleSignal(fAngle, fBoundary, sSignal, pData);

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CRangeSignaling::DestroyPersonalRangeSignaling(EntityId IdEntity)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	return(stl::member_find_and_erase(m_Personals, IdEntity));
}

// Описание:
//
// Arguments:
//
// Return:
//
void CRangeSignaling::ResetPersonalRangeSignaling(EntityId IdEntity)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	MapPersonals::iterator iter = m_Personals.find(IdEntity);

	if (iter != m_Personals.end())
	{
		iter->second->Reset();
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CRangeSignaling::EnablePersonalRangeSignaling(EntityId IdEntity, bool bEnable)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	MapPersonals::iterator iter = m_Personals.find(IdEntity);

	if (iter != m_Personals.end())
	{
		iter->second->SetEnabled(bEnable);
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
CPersonalRangeSignaling* CRangeSignaling::GetPersonalRangeSignaling(EntityId IdEntity) const
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	CPersonalRangeSignaling* pPersonal = NULL;
	MapPersonals::const_iterator iter = m_Personals.find(IdEntity);

	if (iter != m_Personals.end())
	{
		pPersonal = iter->second;
	}

	return(pPersonal);
}

// Описание:
//
// Arguments:
//
// Return:
//
CPersonalRangeSignaling* CRangeSignaling::CreatePersonalRangeSignaling(EntityId IdEntity)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(IdEntity > 0);

	CPersonalRangeSignaling* pPersonal = new CPersonalRangeSignaling(this);

	if (pPersonal->Init(IdEntity) == true)
	{
		m_Personals.insert(std::pair<EntityId, CPersonalRangeSignaling*>(IdEntity, pPersonal));
	}
	else
	{
		SAFE_DELETE(pPersonal);
	}

	return(pPersonal);
}
