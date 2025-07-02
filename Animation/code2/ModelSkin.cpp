// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/ModelSkin.h>

#include <drx3D/Animation/CharacterUpr.h>

CSkin::CSkin(const string& strFileName, u32 nLoadingFlags)
{
	m_strFilePath = strFileName;
	m_nKeepInMemory = 0;
	m_nRefCounter = 0;
	m_nInstanceCounter = 0;
	m_nLoadingFlags = nLoadingFlags;
	m_needsComputeSkinningBuffers = false;
}

CSkin::~CSkin()
{
	for (i32 i = 0, c = m_arrModelMeshes.size(); i != c; ++i)
		m_arrModelMeshes[i].AbortStream();

	if (m_nInstanceCounter)
		DrxFatalError("The model '%s' still has %d skin-instances. Something went wrong with the ref-counting", m_strFilePath.c_str(), m_nInstanceCounter);
	if (m_nRefCounter)
		DrxFatalError("The model '%s' has the value %d in the m_nRefCounter, while calling the destructor. Something went wrong with the ref-counting", m_strFilePath.c_str(), m_nRefCounter);
	g_pCharacterUpr->UnregisterModelSKIN(this);
}

//////////////////////////////////////////////////////////////////////////
u32 CSkin::GetTextureMemoryUsage2(IDrxSizer* pSizer) const
{
	u32 nSize = 0;
	if (pSizer)
	{
		u32 numModelmeshes = m_arrModelMeshes.size();
		for (u32 i = 0; i < numModelmeshes; i++)
		{
			if (m_arrModelMeshes[i].m_pIRenderMesh)
				nSize += (u32)m_arrModelMeshes[i].m_pIRenderMesh->GetTextureMemoryUsage(m_arrModelMeshes[i].m_pIDefaultMaterial, pSizer);
		}
	}
	else
	{
		if (m_arrModelMeshes[0].m_pIRenderMesh)
			nSize = (u32)m_arrModelMeshes[0].m_pIRenderMesh->GetTextureMemoryUsage(m_arrModelMeshes[0].m_pIDefaultMaterial);
	}
	return nSize;
}

//////////////////////////////////////////////////////////////////////////
u32 CSkin::GetMeshMemoryUsage(IDrxSizer* pSizer) const
{
	u32 nSize = 0;
	u32 numModelMeshes = m_arrModelMeshes.size();
	for (u32 i = 0; i < numModelMeshes; i++)
	{
		if (m_arrModelMeshes[i].m_pIRenderMesh)
		{
			nSize += m_arrModelMeshes[i].m_pIRenderMesh->GetMemoryUsage(0, IRenderMesh::MEM_USAGE_ONLY_STREAMS);
		}
	}
	return nSize;
}

//////////////////////////////////////////////////////////////////////////
i32 CSkin::SelectNearestLoadedLOD(i32 nLod)
{
	i32 nMinLod = 0;
	i32 nMaxLod = (i32)m_arrModelMeshes.size() - 1;

	i32 nLodUp = nLod;
	i32 nLodDown = nLod;

	while (nLodUp >= nMinLod || nLodDown <= nMaxLod)
	{
		if (nLodUp >= nMinLod && m_arrModelMeshes[nLodUp].m_pIRenderMesh)
			return nLodUp;
		if (nLodDown <= nMaxLod && m_arrModelMeshes[nLodDown].m_pIRenderMesh)
			return nLodDown;
		--nLodUp;
		++nLodDown;
	}

	return nLod;
}

//////////////////////////////////////////////////////////////////////////
void CSkin::PrecacheMesh(bool bFullUpdate, i32 nRoundId, i32 nLod)
{
	nLod = max(nLod, 0);
	nLod = min(nLod, (i32)m_arrModelMeshes.size() - 1);

	i32 nZoneIdx = bFullUpdate ? 0 : 1;

	MeshStreamInfo& si = m_arrModelMeshes[nLod].m_stream;
	si.nRoundIds[nZoneIdx] = nRoundId;
}

//////////////////////////////////////////////////////////////////////////
IRenderMesh* CSkin::GetIRenderMesh(u32 nLOD) const
{
	u32 numModelMeshes = m_arrModelMeshes.size();
	if (nLOD >= numModelMeshes)
		return 0;
	return m_arrModelMeshes[nLOD].m_pIRenderMesh;
}

u32 CSkin::SizeOfModelData() const
{
	u32 nSize = sizeof(CSkin);

	u32 numMeshes = m_arrModelMeshes.size();
	for (u32 i = 0; i < numMeshes; ++i)
		nSize += m_arrModelMeshes[i].SizeOfModelMesh();

	nSize += m_arrModelJoints.get_alloc_size();

	u32 numModelJoints = m_arrModelJoints.size();
	for (u32 i = 0; i < numModelJoints; i++)
		nSize += m_arrModelJoints[i].m_NameModelSkin.capacity();

	return nSize;
}

void CSkin::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));

	{
		SIZER_COMPONENT_NAME(pSizer, "CModelMesh");
		pSizer->AddObject(m_arrModelMeshes);
	}

	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "SkinSkeleton");
		//pSizer->AddObject(m_poseData);
		pSizer->AddObject(m_arrModelJoints);

		u32 numModelJoints = m_arrModelJoints.size();
		for (u32 i = 0; i < numModelJoints; i++)
			pSizer->AddObject(m_arrModelJoints[i].m_NameModelSkin);
	}
}
