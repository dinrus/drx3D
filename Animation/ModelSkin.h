// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/ModelMesh.h> //embedded
#include <drx3D/Animation/AttachmentVClothPreProcess.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////
// This class contain data which can be shared between different several SKIN-models of same type.
// It loads and contains all geometry and materials.
//////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkin : public ISkin
{
public:
	explicit CSkin(const string& strFileName, u32 nLoadingFlags);
	virtual ~CSkin();
	void AddRef()
	{
		++m_nRefCounter;
	}
	void Release()
	{
		--m_nRefCounter;
		if (m_nKeepInMemory)
			return;
		if (m_nRefCounter == 0)
			delete this;
	}
	void DeleteIfNotReferenced()
	{
		if (m_nRefCounter <= 0)
			delete this;
	}
	i32 GetRefCounter() const
	{
		return m_nRefCounter;
	}

	virtual void         SetKeepInMemory(bool nKiM) { m_nKeepInMemory = nKiM; }
	u32               GetKeepInMemory() const    { return m_nKeepInMemory; }

	virtual void         PrecacheMesh(bool bFullUpdate, i32 nRoundId, i32 nLod);

	virtual tukk  GetModelFilePath() const { return m_strFilePath.c_str(); }
	virtual u32       GetNumLODs() const       { return m_arrModelMeshes.size(); }
	virtual IRenderMesh* GetIRenderMesh(u32 nLOD) const;
	virtual u32       GetTextureMemoryUsage2(IDrxSizer* pSizer = 0) const;
	virtual u32       GetMeshMemoryUsage(IDrxSizer* pSizer = 0) const;
	virtual Vec3         GetRenderMeshOffset(u32 nLOD) const
	{
		u32 numMeshes = m_arrModelMeshes.size();
		if (nLOD >= numMeshes)
			return Vec3(ZERO);
		return m_arrModelMeshes[nLOD].m_vRenderMeshOffset;
	};

	i32         SelectNearestLoadedLOD(i32 nLod);

	CModelMesh* GetModelMesh(u32 nLOD)
	{
		return (nLOD < m_arrModelMeshes.size()) ? &m_arrModelMeshes[nLOD] : nullptr;
	}

	const CModelMesh* GetModelMesh(u32 nLOD) const
	{
		return (nLOD < m_arrModelMeshes.size()) ? &m_arrModelMeshes[nLOD] : nullptr;
	}

	// this is the array that's returned from the RenderMesh
	TRenderChunkArray* GetRenderMeshMaterials()
	{
		if (m_arrModelMeshes[0].m_pIRenderMesh)
			return &m_arrModelMeshes[0].m_pIRenderMesh->GetChunks();
		else
			return NULL;
	}
	void       GetMemoryUsage(IDrxSizer* pSizer) const;
	u32     SizeOfModelData() const;

	IMaterial* GetIMaterial(u32 nLOD) const
	{
		u32 numModelMeshes = m_arrModelMeshes.size();
		if (nLOD >= numModelMeshes)
			return 0;
		return m_arrModelMeshes[nLOD].m_pIDefaultMaterial;
	}

	bool                         LoadNewSKIN(tukk szFilePath, u32 nLoadingFlags);

	virtual const IVertexFrames* GetVertexFrames() const { return &m_arrModelMeshes[0].m_softwareMesh.GetVertexFrames(); }

	SAttachmentVClothPreProcessData const& GetVClothData() const { return m_VClothData; }
	bool HasVCloth() const { return (m_VClothData.m_listBendTrianglePairs.size()>0) || (m_VClothData.m_listBendTriangles.size()>0) || (m_VClothData.m_nndc.size()>0) || (m_VClothData.m_nndcNotAttachedOrderedIdx.size()>0); }
	void SetNeedsComputeSkinningBuffers() { m_needsComputeSkinningBuffers = true;  }
	bool NeedsComputeSkinningBuffers() const { return m_needsComputeSkinningBuffers; }
public:
	// this struct contains the minimal information to attach a SKIN to a base-SKEL
	struct SJointInfo
	{
		i32  m_idxParent;         //index of parent-joint. if the idx==-1 then this joint is the root. Usually this values are > 0
		u32 m_nJointCRC32Lower;  //hash value for searching joint-names
		i32  m_nExtraJointID;     //offset for extra joints added programmatically
		QuatT  m_DefaultAbsolute;
		string m_NameModelSkin;     // the name of joint
		void   GetMemoryUsage(IDrxSizer* pSizer) const {};
	};

	DynArray<SJointInfo> m_arrModelJoints;
	DynArray<CModelMesh> m_arrModelMeshes;
	u32               m_nLoadingFlags;

private:
	i32    m_nKeepInMemory;
	i32    m_nRefCounter, m_nInstanceCounter;
	string m_strFilePath;

	bool m_needsComputeSkinningBuffers;
	SAttachmentVClothPreProcessData m_VClothData;
};
