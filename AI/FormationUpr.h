// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/AIFormationDescriptor.h>

class CFormation;
struct IAIObject;

class CFormationUpr
{
public:
	typedef std::map<CWeakRef<CAIObject>, CFormation*> FormationMap;
	typedef std::map<string, CFormationDescriptor>     FormationDescriptorMap;

public:

	CFormationUpr();
	~CFormationUpr();

	void Reset();
	void Serialize(TSerialize ser);
	void DebugDraw() const;
	void GetMemoryStatistics(IDrxSizer* pSizer) const;

	void OnAIObjectRemoved(IAIObject* pObject);
	const FormationMap& GetActiveFormations() const { return m_mapActiveFormations; }

	CFormation* CreateFormation(CWeakRef<CAIObject> refOwner, tukk szFormationName, Vec3 vTargetPos = ZERO);
	void ReleaseFormation(CWeakRef<CAIObject> refOwner, bool bDelete);
	CFormation* GetFormation(CFormation::TFormationID id) const;

	bool ChangeFormation(IAIObject* pOwner, tukk szFormationName, float fScale);
	bool ScaleFormation(IAIObject* pOwner, float fScale);
	bool SetFormationUpdate(IAIObject* pOwner, bool bUpdate);
	
	void FreeFormationPoint(CWeakRef<CAIObject> refOwner);	
	void ReleaseFormationPoint(CAIObject* pReserved);
	IAIObject* GetFormationPoint(IAIObject* pObject) const;
	i32 GetFormationPointClass(tukk descriptorName, i32 position) const;

	bool SameFormation(const CPuppet* pHuman, const CAIVehicle* pVehicle) const;

    string GetFormationNameFromCRC32(u32 nCrc32ForFormationName) const;
	void CreateFormationDescriptor(tukk szName);
	void AddFormationPoint(tukk szName, const FormationNode& nodeDescriptor);
	bool IsFormationDescriptorExistent(tukk szName) const;

	void EnumerateFormationNames(u32 maxNames, tukk* outNames, u32* outNameCount) const;

private:
	FormationMap           m_mapActiveFormations;
	FormationDescriptorMap m_mapFormationDescriptors;
};