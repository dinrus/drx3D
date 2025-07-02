// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   Range.h
   $Id$
   $DateTime$
   Описание: Single Range donut
   ---------------------------------------------------------------------
   История:
   - 24:08:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/Range.h>
#include <drx3D/Act/PersonalRangeSignaling.h>

// Описание:
//   Constructor
// Arguments:
//
// Return:
//
CRange::CRange(CPersonalRangeSignaling* pPersonal) : m_pPersonal(pPersonal), m_fRadius(-1.0f), m_fBoundary(-1.0f), m_pSignalData(NULL)
{
	DRX_ASSERT(pPersonal != NULL);
}

// Описание:
//   Destructor
// Arguments:
//
// Return:
//
CRange::~CRange()
{
	if (gEnv->pAISystem)
		gEnv->pAISystem->FreeSignalExtraData(m_pSignalData);
}

// Описание:
//   Constructor
// Arguments:
//
// Return:
//
void CRange::Init(float fRadius, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*= NULL*/)
{
	DRX_ASSERT(fRadius >= 1.0f);
	DRX_ASSERT(fBoundary >= 0.0f);
	DRX_ASSERT(sSignal != NULL);

	m_sSignal = sSignal;
	m_fRadius = fRadius * fRadius;
	m_fBoundary = fBoundary + fRadius;
	m_fBoundary *= m_fBoundary;

	// Clone extra data
	if (pData && gEnv->pAISystem)
	{
		gEnv->pAISystem->FreeSignalExtraData(m_pSignalData);
		m_pSignalData = gEnv->pAISystem->CreateSignalExtraData();
		DRX_ASSERT(m_pSignalData);
		*m_pSignalData = *pData;
	}
}

// Описание:
//   Destructor
// Arguments:
//
// Return:
//
bool CRange::IsInRange(const Vec3& vPos) const
{
	bool bResult = false;
	IEntity* pEntity = m_pPersonal->GetEntity();
	DRX_ASSERT(pEntity);
	if (pEntity)
	{
		Vec3 vOrigin = pEntity->GetPos();
		bResult = IsInRange((vPos - vOrigin).GetLengthSquared());
	}
	return bResult;
}

// Описание:
//   Destructor
// Arguments:
//
// Return:
//
bool CRange::IsInRangePlusBoundary(const Vec3& vPos) const
{
	bool bResult = false;
	IEntity* pEntity = m_pPersonal->GetEntity();
	DRX_ASSERT(pEntity);
	if (pEntity)
	{
		Vec3 vOrigin = pEntity->GetPos();
		bResult = IsInRangePlusBoundary((vPos - vOrigin).GetLengthSquared());
	}
	return bResult;
}
