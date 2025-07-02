// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/FormationUpr.h>
#include <drx3D/AI/Puppet.h>
#include <drx3D/AI/AIVehicle.h>

//-----------------------------------------------------------------------------------------------------------
CFormationUpr::CFormationUpr()
{
	GetAISystem()->Callbacks().ObjectRemoved().Add(functor(*this, &CFormationUpr::OnAIObjectRemoved));
}

//-----------------------------------------------------------------------------------------------------------
CFormationUpr::~CFormationUpr()
{
	Reset();
	
	GetAISystem()->Callbacks().ObjectRemoved().Remove(functor(*this, &CFormationUpr::OnAIObjectRemoved));
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::Reset()
{
	for (auto it = m_mapActiveFormations.begin(); it != m_mapActiveFormations.end(); ++it)
	{
		delete it->second;
	}
	m_mapActiveFormations.clear();
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::OnAIObjectRemoved(IAIObject* pObject)
{
	for (auto it = m_mapActiveFormations.begin(); it != m_mapActiveFormations.end(); ++it)
	{
		it->second->OnObjectRemoved(static_cast<CAIObject*>(pObject));
	}
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::EnumerateFormationNames(u32 maxNames, tukk* outNames, u32* outNameCount) const
{
	DRX_ASSERT(outNames);
	DRX_ASSERT(outNameCount);

	*outNameCount = 0;

	FormationDescriptorMap::const_iterator it = m_mapFormationDescriptors.begin();
	FormationDescriptorMap::const_iterator end = m_mapFormationDescriptors.end();

	for (; it != end; ++it)
	{
		if (*outNameCount == maxNames)
		{
			gEnv->pLog->LogError("CAISystem::EnumerateFormationNames: Can't fit more formation names. Number of names = %" PRISIZE_T ", maximum number of names = %u.", m_mapFormationDescriptors.size(), maxNames);
			return;
		}

		outNames[(*outNameCount)++] = it->first.c_str();
	}
}

// Creates a formation and associates it with a group of agents
//-----------------------------------------------------------------------------------------------------------
CFormation* CFormationUpr::CreateFormation(CWeakRef<CAIObject> refOwner, tukk szFormationName, Vec3 vTargetPos)
{
	FormationMap::iterator fi;
	fi = m_mapActiveFormations.find(refOwner);
	if (fi != m_mapActiveFormations.end())
	{
		return (fi->second);
	}
	else
	{
		FormationDescriptorMap::iterator di;
		di = m_mapFormationDescriptors.find(szFormationName);
		if (di != m_mapFormationDescriptors.end())
		{
			CCCPOINT(CAISystem_CreateFormation);

			CFormation* pFormation = new CFormation();
			pFormation->Create(di->second, refOwner, vTargetPos);
			m_mapActiveFormations.insert(FormationMap::iterator::value_type(refOwner, pFormation));
			return pFormation;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------------------------------------
string CFormationUpr::GetFormationNameFromCRC32(u32 nCrc32ForFormationName) const
{
	for (auto currentFormation = m_mapFormationDescriptors.begin(); currentFormation != m_mapFormationDescriptors.end(); ++currentFormation)
	{
		if (currentFormation->second.m_nNameCRC32 == nCrc32ForFormationName)
			return currentFormation->first;
	}
	return "";
}

// return a formation by id (used for serialization)
//-----------------------------------------------------------------------------------------------------------
CFormation* CFormationUpr::GetFormation(CFormation::TFormationID id) const
{
	for (auto it = m_mapActiveFormations.begin(), end = m_mapActiveFormations.end(); it != end; ++it)
	{
		if (it->second->GetId() == id)
			return it->second;
	}
	return nullptr;
}

// change the current formation for the given group
//-----------------------------------------------------------------------------------------------------------
bool CFormationUpr::ChangeFormation(IAIObject* pOwner, tukk szFormationName, float fScale)
{
	if (pOwner)
	{
		CFormation* pFormation = ((CAIObject*)pOwner)->m_pFormation;
		if (pFormation)
		{
			FormationDescriptorMap::iterator di;
			di = m_mapFormationDescriptors.find(szFormationName);
			if (di != m_mapFormationDescriptors.end())
			{
				CCCPOINT(CAISystem_ChangeFormation);

				pFormation->Change(di->second, fScale);
				return true;
			}
		}
	}
	return false;
}

// scale the current formation for the given group
//-----------------------------------------------------------------------------------------------------------
bool CFormationUpr::ScaleFormation(IAIObject* pOwner, float fScale)
{
	CWeakRef<CAIObject> refOwner = GetWeakRef(static_cast<CAIObject*>(pOwner));
	FormationMap::iterator fi;
	fi = m_mapActiveFormations.find(refOwner);
	if (fi != m_mapActiveFormations.end())
	{
		CFormation* pFormation = fi->second;
		if (pFormation)
		{
			CCCPOINT(CAISystem_ScaleFormation);

			pFormation->SetScale(fScale);
			return true;
		}
	}
	return false;
}

// scale the current formation for the given group
//-----------------------------------------------------------------------------------------------------------
bool CFormationUpr::SetFormationUpdate(IAIObject* pOwner, bool bUpdate)
{
	CWeakRef<CAIObject> refOwner = GetWeakRef(static_cast<CAIObject*>(pOwner));
	FormationMap::iterator fi;
	fi = m_mapActiveFormations.find(refOwner);
	if (fi != m_mapActiveFormations.end())
	{
		CFormation* pFormation = fi->second;
		if (pFormation)
		{
			pFormation->SetUpdate(bUpdate);
			return true;
		}
	}
	return false;
}

// check if puppet and vehicle are in the same formation
//-----------------------------------------------------------------------------------------------------------
bool CFormationUpr::SameFormation(const CPuppet* pHuman, const CAIVehicle* pVehicle) const
{
	if (pHuman->IsHostile(pVehicle))
		return false;

	for (auto it = m_mapActiveFormations.begin(); it != m_mapActiveFormations.end(); ++it)
	{
		const CFormation* pFormation = it->second;
		if (pFormation->GetFormationPoint(GetWeakRef(pHuman)))
		{
			CAIObject* pFormationOwner(pFormation->GetPointOwner(0));
			if (!pFormationOwner)
				return false;
			if (pFormationOwner == pVehicle->GetAttentionTarget())
			{
				CCCPOINT(CAISystem_SameFormation);
				return true;
			}
			return false;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::ReleaseFormation(CWeakRef<CAIObject> refOwner, bool bDelete)
{
	CCCPOINT(CAISystem_ReleaseFormation);

	auto it = m_mapActiveFormations.find(refOwner);
	if (it != m_mapActiveFormations.end())
	{
		CFormation* pFormation = it->second;
		if (bDelete && pFormation)
		{
			delete pFormation;
		}
		m_mapActiveFormations.erase(it);
	}
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::FreeFormationPoint(CWeakRef<CAIObject> refOwner)
{
	auto it = m_mapActiveFormations.find(refOwner);
	if (it != m_mapActiveFormations.end())
	{
		(it->second)->FreeFormationPoint(refOwner);
	}
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::ReleaseFormationPoint(CAIObject* pReserved)
{
	if (m_mapActiveFormations.empty())
		return;

	CCCPOINT(CAISystem_ReleaseFormationPoint);

	CWeakRef<CAIObject> refReserved = GetWeakRef(pReserved);

	CAIActor* pActor = pReserved->CastToCAIActor();
	CLeader* pLeader = GetAISystem()->GetLeader(pActor);
	if (pLeader)
	{
		pLeader->FreeFormationPoint(refReserved);
	}
	else
	{
		for (auto it = m_mapActiveFormations.begin(); it != m_mapActiveFormations.end(); ++it)
		{
			(it->second)->FreeFormationPoint(refReserved);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::CreateFormationDescriptor(tukk name)
{
	auto it = m_mapFormationDescriptors.find(name);
	if (it != m_mapFormationDescriptors.end())
	{
		m_mapFormationDescriptors.erase(it);
	}

	CFormationDescriptor fdesc;
	fdesc.m_sName = name;
	fdesc.m_nNameCRC32 = CCrc32::Compute(name);
	m_mapFormationDescriptors.insert(FormationDescriptorMap::iterator::value_type(fdesc.m_sName, fdesc));
}

//-----------------------------------------------------------------------------------------------------------
bool CFormationUpr::IsFormationDescriptorExistent(tukk name) const
{
	return m_mapFormationDescriptors.find(name) != m_mapFormationDescriptors.end();
}

//-----------------------------------------------------------------------------------------------------------
void CFormationUpr::AddFormationPoint(tukk name, const FormationNode& nodeDescriptor)
{
	auto it = m_mapFormationDescriptors.find(name);
	if (it != m_mapFormationDescriptors.end())
	{
		it->second.AddNode(nodeDescriptor);
	}
}

//-----------------------------------------------------------------------------------------------------------
IAIObject* CFormationUpr::GetFormationPoint(IAIObject* pObject) const
{
	const CAIActor* pActor = pObject->CastToCAIActor();
	const CLeader* pLeader = GetAISystem()->GetLeader(pActor);
	if (pLeader)
	{
		CCCPOINT(CAISystem_GetFormationPoint);
		return pLeader->GetFormationPoint(GetWeakRef((CAIObject*)pObject)).GetAIObject();
	}
	return nullptr;
}

//-----------------------------------------------------------------------------------------------------------
i32 CFormationUpr::GetFormationPointClass(tukk descriptorName, i32 position) const
{
	auto it = m_mapFormationDescriptors.find(descriptorName);
	if (it != m_mapFormationDescriptors.end())
	{
		return it->second.GetNodeClass(position);
	}
	return -1;
}

void CFormationUpr::Serialize(TSerialize ser)
{
	// Active formations
	ser.BeginGroup("ActiveFormations");
	{
		i32 count = m_mapActiveFormations.size();
		ser.Value("numObjects", count);

		if (ser.IsReading())
		{
			FormationMap::iterator iFormation = m_mapActiveFormations.begin(), iEnd = m_mapActiveFormations.end();
			for (; iFormation != iEnd; ++iFormation)
			{
				delete iFormation->second;
			}

			m_mapActiveFormations.clear();
		}

		char formationName[32];
		while (--count >= 0)
		{
			CFormation* pFormationObject(0);
			CWeakRef<CAIObject> refOwner;

			if (ser.IsWriting())
			{
				FormationMap::iterator iFormation = m_mapActiveFormations.begin();
				std::advance(iFormation, count);
				refOwner = iFormation->first;
				pFormationObject = iFormation->second;
			}
			else
			{
				pFormationObject = new CFormation();
			}

			drx_sprintf(formationName, "Formation_%d", count);
			ser.BeginGroup(formationName);
			{
				refOwner.Serialize(ser, "formationOwner");
				pFormationObject->Serialize(ser);
			}
			ser.EndGroup();
			if (ser.IsReading())
			{
				m_mapActiveFormations.insert(FormationMap::iterator::value_type(refOwner, pFormationObject));
			}
		}
	}
	ser.EndGroup(); // "ActiveFormations"
}

void CFormationUpr::DebugDraw() const
{
	for (auto it = m_mapActiveFormations.begin(); it != m_mapActiveFormations.end(); ++it)
	{
		if (it->second)
			(it->second)->Draw();
	}
}

void CFormationUpr::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	SIZER_SUBCOMPONENT_NAME(pSizer, "Formations");

	size_t size = 0;
	for (auto it = m_mapFormationDescriptors.begin(); it != m_mapFormationDescriptors.end(); ++it)
	{
		size += (it->first).capacity();
		size += sizeof((it->second));
		size += (it->second).m_sName.capacity();
		size += (it->second).m_Nodes.size() * sizeof(FormationNode);
	}
	pSizer->AddObject(this, size);
}