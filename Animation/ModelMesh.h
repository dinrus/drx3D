// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/CGFContent.h>
#include <drx3D/Animation/VertexData.h>

class CSkin;
class CCharInstance;
class CAttachmentSKIN;
class DrxCHRLoader;

struct ClosestTri
{
	struct AttSkinVertex
	{
		Vec3        m_attTriPos;
		JointIdType m_attJointIDs[4];
		f32         m_attWeights[4];
		u32      m_vertexIdx;
		AttSkinVertex()
		{
			m_attTriPos = Vec3(ZERO);
			m_attJointIDs[0] = 0, m_attJointIDs[1] = 0, m_attJointIDs[2] = 0, m_attJointIDs[3] = 0;
			m_attWeights[0] = 0, m_attWeights[1] = 0, m_attWeights[2] = 0, m_attWeights[3] = 0;
			m_vertexIdx = -1;
		}
	};
	AttSkinVertex v[3];
};

struct RChunk
{
	u32 m_nFirstIndexId;
	u32 m_nNumIndices;
	void   GetMemoryUsage(IDrxSizer* pSizer) const
	{
	}

};

struct MeshStreamInfo
{
	MeshStreamInfo()
	{
		memset(this, 0, sizeof(*this));
	}

	u32        nRoundIds[MAX_STREAM_PREDICTION_ZONES];
	u32        nFrameId;
	u32        nKeepResidentRefs;
	bool          bHasMeshFile;
	bool          bIsUrgent;
	DrxCHRLoader* pStreamer;
};

//////////////////////////////////////////////////////////////////////////
//      This is the class to manage the mesh on the CPU-side:           //
//////////////////////////////////////////////////////////////////////////
class CModelMesh
{
public:
	CModelMesh()
	{
		m_iThreadMeshAccessCounter = 0;
		m_vRenderMeshOffset = Vec3(ZERO);
		m_pIRenderMesh = 0;
		m_faceCount = 0;
		m_geometricMeanFaceArea = 0.0f;
		m_pIDefaultMaterial = g_pI3DEngine->GetMaterialUpr()->GetDefaultMaterial(); // the material physical game id that will be used as default for this character
	}
	void                    AbortStream();

	u32                  InitMesh(CMesh* pMesh, CNodeCGF* pGFXNode, _smart_ptr<IMaterial> pMaterial, CSkinningInfo* pSkinningInfo, tukk szFilePath, u32 nLOD);
	_smart_ptr<IRenderMesh> InitRenderMeshAsync(CMesh* pMesh, tukk szFilePath, i32 nLod, DynArray<RChunk>& arrNewRenderChunksbool, bool useComputeSkinningBuffers = false);
	u32                  InitRenderMeshSync(DynArray<RChunk>& arrNewRenderChunks, _smart_ptr<IRenderMesh> pNewRenderMesh);

	u32                  IsVBufferValid();
	ClosestTri              GetAttachmentTriangle(const Vec3& RMWPosition, const JointIdType* const pRemapTable);

	u32                  InitSWSkinBuffer();
	void                    DrawDebugInfo(CDefaultSkeleton* pCSkel, i32 nLOD, const Matrix34& rRenderMat34, i32 DebugMode, IMaterial* pMaterial, CRenderObject* pObj, const SRendParams& RendParams, bool isGeneralPass, IRenderNode* pRenderNode, const AABB& aabb,const SRenderingPassInfo &passInfo);

#ifdef EDITOR_PCDEBUGCODE
	void ExportModel(IRenderMesh* pIRenderMesh);
	void DrawWireframeStatic(const Matrix34& m34, u32 color);
#endif

	size_t SizeOfModelMesh() const;
	void   GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	void                    PrepareMesh(CMesh* pMesh);
	void                    PrepareRenderChunks(CMesh& mesh, DynArray<RChunk>& renderChunks);
	_smart_ptr<IRenderMesh> CreateRenderMesh(CMesh* pMesh, tukk szFilePath, i32 nLod, bool bCreateDeviceMesh, bool needsComputeSkinningBuffers = false);
	void                    CreateMorphsBuffer(CMesh* pMesh);

public:
	//////////////////////////////////////////////////////////////////////////
	// Member variables.
	//////////////////////////////////////////////////////////////////////////
	_smart_ptr<IRenderMesh> m_pIRenderMesh;
	_smart_ptr<IMaterial>   m_pIDefaultMaterial;  // Default material for this model.

	Vec3                    m_vRenderMeshOffset;
	 i32            m_iThreadMeshAccessCounter;
	DynArray<RChunk>        m_arrRenderChunks; //always initialized in LoaderCHR.cpp

	MeshStreamInfo          m_stream;

	// used for geometric mean lod system
	float         m_geometricMeanFaceArea;
	i32           m_faceCount;

	CSoftwareMesh m_softwareMesh;
};
