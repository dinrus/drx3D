// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   Range.h
   $Id$
   $DateTime$
   Описание: Upr per-actor signals to other actors by range
   ---------------------------------------------------------------------
   История:
   - 09:04:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/PersonalRangeSignaling.h>
#include <drx3D/Act/RangeSignaling.h>
#include <drx3D/Act/Range.h>
#include <drx3D/Act/AngleAlert.h>
#include <drx3D/Act/IUIDraw.h>

#include <functional>
#include <drx3D/AI/IAIObject.h>

// Описание:
//   Constructor
// Arguments:
//
// Return:
//
CPersonalRangeSignaling::CPersonalRangeSignaling(CRangeSignaling* pParent) : m_bInit(false), m_bEnabled(false), m_pParent(pParent), m_EntityId(0)
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
CPersonalRangeSignaling::~CPersonalRangeSignaling()
{
	Reset();

	for (i32 i = 0; i < m_vecRanges.size(); ++i)
	{
		SAFE_DELETE(m_vecRanges[i]);
	}

	m_vecRanges.clear();

	MapTargetRanges::iterator itTargetRange = m_mapTargetRanges.begin();
	MapTargetRanges::iterator itTargetRangeEnd = m_mapTargetRanges.end();
	for (; itTargetRange != itTargetRangeEnd; ++itTargetRange)
	{
		for (i32 i = 0; i < itTargetRange->second.size(); ++i)
		{
			SAFE_DELETE(itTargetRange->second[i]);
		}
	}

	m_mapTargetRanges.clear();

	for (i32 i = 0; i < m_vecAngles.size(); ++i)
	{
		SAFE_DELETE(m_vecAngles[i]);
	}

	m_vecAngles.clear();

	SetListener(false);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalRangeSignaling::Init(EntityId Id)
{
	DRX_ASSERT(m_bInit == false);

	m_EntityId = Id;
	m_bInit = true;

	// Enabled state starts off matching proxy's state
	IAIActorProxy* pAIProxy = GetEntityAIProxy(m_EntityId);
	SetEnabled(pAIProxy && pAIProxy->IsEnabled());

	if (GetActor() == NULL)
	{
		m_bInit = false;
	}
	else
	{
		SetListener(true);
	}

	return(m_bInit);
}

// Описание: std::less functor template specialization, so to make comparison of two CRange pointers
//							work properly
//
// Arguments:		two pointers to CRange instances
//
// Return:			comparison result
//
namespace std
{

template<>
struct less<CRange*> : public binary_function<CRange*, CRange*, bool>
{
	bool operator()(const CRange* pRange1, const CRange* pRange2) const
	{
		DRX_ASSERT(pRange1 != NULL);
		DRX_ASSERT(pRange2 != NULL);

		return((*pRange1) < (*pRange2));
	}
};

}

bool CompareRange(CRange* pRange1, CRange* pRange2)
{
	DRX_ASSERT(pRange1 != NULL);
	DRX_ASSERT(pRange2 != NULL);

	return((*pRange1) < (*pRange2));
}

bool CompareAngle(CAngleAlert* pAngle1, CAngleAlert* pAngle2)
{
	DRX_ASSERT(pAngle1 != NULL);
	DRX_ASSERT(pAngle2 != NULL);

	return((*pAngle1) < (*pAngle2));
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalRangeSignaling::AddRangeSignal(float fRadius, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*=NULL*/)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(sSignal != NULL);
	DRX_ASSERT(fRadius > 0.5f);
	DRX_ASSERT(fBoundary >= 0.0f);

	bool bRet = true;
	CRange* pRange = SearchRange(sSignal, m_vecRanges);

	if (pRange == NULL)
	{
		pRange = new CRange(this);
		m_vecRanges.push_back(pRange);
	}

	pRange->Init(fRadius, fBoundary, sSignal, pData);
	std::sort(m_vecRanges.begin(), m_vecRanges.end(), &CompareRange);   // std::less is properly overridden

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalRangeSignaling::AddTargetRangeSignal(EntityId IdTarget, float fRadius, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*= NULL*/)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(GetActor());
	DRX_ASSERT(IdTarget > 0);
	DRX_ASSERT(sSignal != NULL);
	DRX_ASSERT(fRadius > 0.5f);
	DRX_ASSERT(fBoundary >= 0.0f);

	bool bRet = true;
	CRange* pRange = SearchRange(sSignal, m_mapTargetRanges[IdTarget]);

	if (pRange == NULL)
	{
		pRange = new CRange(this);
		m_mapTargetRanges[IdTarget].push_back(pRange);
	}

	pRange->Init(fRadius, fBoundary, sSignal, pData);
	std::sort(m_mapTargetRanges[IdTarget].begin(), m_mapTargetRanges[IdTarget].end(), &CompareRange);   // std::less is properly overridden

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalRangeSignaling::AddAngleSignal(float fAngle, float fBoundary, tukk sSignal, IAISignalExtraData* pData /*=NULL*/)
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(sSignal != NULL);
	DRX_ASSERT(fAngle > 0.5f);
	DRX_ASSERT(fBoundary >= 0.0f);

	bool bRet = true;
	CAngleAlert* pAngle = SearchAngle(sSignal);

	if (pAngle == NULL)
	{
		pAngle = new CAngleAlert(this);
		m_vecAngles.push_back(pAngle);
	}

	pAngle->Init(fAngle, fBoundary, sSignal, pData);
	std::sort(m_vecAngles.begin(), m_vecAngles.end(), &CompareAngle);

	return(bRet);
}

// Описание:
//
// Arguments:
//
// Return:
//
bool CPersonalRangeSignaling::Update(float fElapsedTime, u32 uDebugOrder)
{
	DRX_ASSERT(m_bInit == true);

	bool bRet = false;

	if (m_bEnabled)
	{
		if (uDebugOrder > 0)
		{
			DebugDraw(uDebugOrder);
		}

		// Iterate all actors and check range to them
		IActorIteratorPtr pActorIt = CDrxAction::GetDrxAction()->GetIActorSystem()->CreateActorIterator();
		while (IActor* pActor = pActorIt->Next())
		{
			EntityId entityId = pActor->GetEntityId();
			if (entityId != GetEntityId())
			{
				// Skip over if not enabled
				IAIObject const* pAIObject = GetEntityAI(entityId);
				if (!pAIObject || pAIObject->IsEnabled())
				{
					CheckActorRanges(pActor);
					CheckActorAngles(pActor);

					// Do check on target ranges
					MapTargetRanges::iterator itTargetRanges = m_mapTargetRanges.find(entityId);
					if (itTargetRanges != m_mapTargetRanges.end())
					{
						CheckActorTargetRanges(pActor);
					}
				}
			}
		}

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
void CPersonalRangeSignaling::CheckActorRanges(IActor* pActor)
{
	DRX_ASSERT(pActor != NULL);

	MapRangeSignals::iterator it = m_mapRangeSignalsSent.find(pActor->GetEntityId());
	CRange const* pRange = GetRangeTo(pActor->GetEntity()->GetPos(), m_vecRanges);
	CRange const* pOldRange = ((it != m_mapRangeSignalsSent.end()) ? it->second : NULL);

	if (pRange != pOldRange)
	{
		if (pRange != NULL)
		{
			// Check boundary
			if (GetRangeTo(pActor->GetEntity()->GetPos(), m_vecRanges, true) != pOldRange)
			{
				if (pOldRange == NULL)
				{
					m_mapRangeSignalsSent.insert(std::pair<EntityId, CRange const*>(pActor->GetEntityId(), pRange));
				}
				else
				{
					it->second = pRange;
				}

				SendSignal(pActor, pRange->GetSignal(), pRange->GetSignalData());
			}
		}
		else
		{
			m_mapRangeSignalsSent.erase(it);
		}
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::CheckActorTargetRanges(IActor* pActor)
{
	DRX_ASSERT(pActor != NULL);

	MapTargetRanges::iterator itTargetRanges = m_mapTargetRanges.find(pActor->GetEntityId());
	DRX_ASSERT(itTargetRanges != m_mapTargetRanges.end());

	MapRangeSignals::iterator it = m_mapTargetRangeSignalsSent.find(pActor->GetEntityId());
	CRange const* pRange = GetRangeTo(pActor->GetEntity()->GetPos(), itTargetRanges->second);
	CRange const* pOldRange = ((it != m_mapTargetRangeSignalsSent.end()) ? it->second : NULL);

	if (pRange != pOldRange)
	{
		if (pRange != NULL)
		{
			// Check boundary
			if (GetRangeTo(pActor->GetEntity()->GetPos(), itTargetRanges->second, true) != pOldRange)
			{
				if (pOldRange == NULL)
				{
					m_mapTargetRangeSignalsSent.insert(std::pair<EntityId, CRange const*>(pActor->GetEntityId(), pRange));
				}
				else
				{
					it->second = pRange;
				}

				IActor* pMe = GetActor();
				DRX_ASSERT_MESSAGE(pMe, "An actor has been removed without cleaning up its range signals.");
				if (pMe)
				{
					SendSignal(pMe, pRange->GetSignal(), pRange->GetSignalData());
				}
			}
		}
		else
		{
			m_mapTargetRangeSignalsSent.erase(it);
		}
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::CheckActorAngles(IActor* pActor)
{
	DRX_ASSERT(pActor != NULL);

	MapAngleSignals::iterator it = m_mapAngleSignalsSent.find(pActor->GetEntityId());
	CAngleAlert const* pAngle = GetAngleTo(pActor->GetEntity()->GetPos());
	CAngleAlert const* pOldAngle = ((it != m_mapAngleSignalsSent.end()) ? it->second : NULL);

	if (pAngle != pOldAngle)
	{
		if (pAngle != NULL)
		{
			// Check boundary
			if (GetAngleTo(pActor->GetEntity()->GetPos(), true) != pOldAngle)
			{
				if (pOldAngle == NULL)
				{
					m_mapAngleSignalsSent.insert(std::pair<EntityId, CAngleAlert const*>(pActor->GetEntityId(), pAngle));
				}
				else
				{
					it->second = pAngle;
				}

				SendSignal(pActor, pAngle->GetSignal(), pAngle->GetSignalData());
			}
		}
		else
		{
			m_mapAngleSignalsSent.erase(it);
		}
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::Reset()
{
	m_mapRangeSignalsSent.clear();
	m_mapTargetRangeSignalsSent.clear();
	m_mapAngleSignalsSent.clear();
	m_mapTargetRanges.clear();
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::OnProxyReset()
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
void CPersonalRangeSignaling::SetEnabled(bool bEnable)
{
	DRX_ASSERT(m_bInit == true);

	m_bEnabled = bEnable;
}

// Описание:
//
// Arguments:
//
// Return:
//
EntityId CPersonalRangeSignaling::GetEntityId() const
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
IEntity* CPersonalRangeSignaling::GetEntity()
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
IEntity const* CPersonalRangeSignaling::GetEntity() const
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
IActor* CPersonalRangeSignaling::GetActor()
{
	DRX_ASSERT(m_bInit == true);

	return(CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(m_EntityId));
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::SendSignal(IActor* pActor, const string& sSignal, IAISignalExtraData* pData /*= NULL*/) const
{
	DRX_ASSERT(m_bInit == true);
	DRX_ASSERT(pActor != NULL);
	DRX_ASSERT(sSignal.length() > 0);

	IAIObject* pAI = pActor->GetEntity()->GetAI();
	//DRX_ASSERT(pAI);  Not every actor has an ai, and nor do they need it
	if (pAI && gEnv->pAISystem)
	{
		gEnv->pAISystem->SendSignal(SIGNALFILTER_SENDER, 1, sSignal, pAI, PrepareSignalData(pData));
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
IAISignalExtraData* CPersonalRangeSignaling::PrepareSignalData(IAISignalExtraData* pRequestedData) const
{
	IAISignalExtraData* pSendData = gEnv->pAISystem->CreateSignalExtraData();
	if (pRequestedData)
	{
		// Clone requested data and send it over
		*pSendData = *pRequestedData;
	}
	else
	{
		// Send useful info
		pSendData->nID = GetEntityId();
		pSendData->point = (GetEntity() ? GetEntity()->GetWorldPos() : Vec3Constants<float>::fVec3_Zero);
	}

	return (pSendData);
}

// Описание:
//
// Arguments:
//
// Return:
//
CRange const* CPersonalRangeSignaling::GetRangeTo(const Vec3& vPos, const VecRanges& vecRangeList, bool bUseBoundary) const
{
	IEntity const* pEntity = GetEntity();
	if (!pEntity)
		return(NULL);

	CRange const* pRange = NULL;
	Vec3 vOrigin = pEntity->GetPos();
	float fSquaredDist = (vPos - vOrigin).GetLengthSquared();

	for (i32 i = 0; i < vecRangeList.size(); ++i)
	{
		bool bInRange = false;

		if (bUseBoundary == true)
		{
			bInRange = (vecRangeList[i]->IsInRangePlusBoundary(fSquaredDist) == true);
		}
		else
		{
			bInRange = (vecRangeList[i]->IsInRange(fSquaredDist) == true);
		}

		if (bInRange == true)
		{
			pRange = vecRangeList[i];
			break;
		}
	}

	return(pRange);
}

// Описание:
//
// Arguments:
//
// Return:
//
CAngleAlert const* CPersonalRangeSignaling::GetAngleTo(const Vec3& vPos, bool bUseBoundary) const
{
	CAngleAlert const* pAngle = NULL;
	float fAngleDiff = -1.0f;

	for (i32 i = 0; i < m_vecAngles.size(); ++i)
	{
		bool bInAngle = false;

		if (fAngleDiff < 0.0f)
		{
			fAngleDiff = m_vecAngles[i]->GetAngleTo(vPos);
		}

		if (bUseBoundary == true)
		{
			bInAngle = (m_vecAngles[i]->CheckPlusBoundary(fAngleDiff) == true);
		}
		else
		{
			bInAngle = (m_vecAngles[i]->Check(fAngleDiff) == true);
		}

		if (bInAngle == true)
		{
			pAngle = m_vecAngles[i];
			break;
		}
	}

	return(pAngle);
}

// Описание:
//
// Arguments:
//
// Return:
//
CRange* CPersonalRangeSignaling::SearchRange(tukk sSignal, const VecRanges& vecRangeList) const
{
	CRange* pRange = NULL;

	for (i32 i = 0; i < vecRangeList.size(); ++i)
	{
		if (vecRangeList[i]->GetSignal().compareNoCase(sSignal) == 0)
		{
			pRange = vecRangeList[i];
			break;
		}
	}

	return(pRange);
}

// Описание:
//
// Arguments:
//
// Return:
//
CAngleAlert* CPersonalRangeSignaling::SearchAngle(tukk sSignal) const
{
	CAngleAlert* pAngle = NULL;

	for (i32 i = 0; i < m_vecAngles.size(); ++i)
	{
		if (m_vecAngles[i]->GetSignal().compareNoCase(sSignal) == 0)
		{
			pAngle = m_vecAngles[i];
			break;
		}
	}

	return(pAngle);
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::DebugDraw(u32 uOrder) const
{
	DRX_ASSERT(m_bInit == true);

	float x = 120.0f;
	float y = 100.0f + (float(uOrder) * 10.0f);
	float r = 0.0f;
	float g = 8.0f;
	float b = 0.0f;

	char txt[512] = "\0";

	//drx_sprintf( txt, "%s > %s: %0.1f / %0.1f", GetActor()->GetEntity()->GetName(), m_sSignal.c_str(), m_fTimer, m_fRateMax );
	//IRenderAuxText::Draw2dLabel( x,y, 13.0f, ColorF(r,g,b), false, "%s", txt);
}

// Описание:
//
// Arguments:
//
// Return:
//
IAIObject const* CPersonalRangeSignaling::GetEntityAI(EntityId entityId) const
{
	IAIObject const* pAIObject = NULL;

	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (pEntity)
	{
		pAIObject = pEntity->GetAI();
	}

	return pAIObject;
}

// Описание:
//
// Arguments:
//
// Return:
//
IAIActorProxy* CPersonalRangeSignaling::GetEntityAIProxy(EntityId entityId) const
{
	IAIActorProxy* pAIProxy = NULL;

	IAIObject const* pAIObject = GetEntityAI(m_EntityId);
	if (pAIObject)
	{
		pAIProxy = pAIObject->GetProxy();
	}

	return pAIProxy;
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::SetListener(bool bAdd)
{
	CAIProxy* pAIProxy = static_cast<CAIProxy*>(GetEntityAIProxy(m_EntityId));
	if (pAIProxy)
	{
		if (bAdd)
			pAIProxy->AddListener(this);
		else
			pAIProxy->RemoveListener(this);
	}
}

// Описание:
//
// Arguments:
//
// Return:
//
void CPersonalRangeSignaling::OnAIProxyEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		Reset();
		SetEnabled(true);
	}
	else
	{
		SetEnabled(false);
	}
}
