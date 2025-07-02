// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CGFSaver_h__
#define __CGFSaver_h__

#if defined(RESOURCE_COMPILER) || defined(INCLUDE_SAVECGF)

	#include <drx3D/Eng3D/CGFContent.h>

	#if defined(RESOURCE_COMPILER)
class CInternalSkinningInfo;
	#endif

class CChunkFile;

//////////////////////////////////////////////////////////////////////////
class CSaverCGF
{
public:
	CSaverCGF(CChunkFile& chunkFile);

	void SaveContent(CContentCGF* pCGF, bool bSwapEndian, bool bStorePositionsAsF16, bool bUseQtangents, bool bStoreIndicesAsU16);

	void SetContent(CContentCGF* pCGF);

	// Enable/Disable saving of the node mesh.
	void SetMeshDataSaving(bool bEnable);

	// Enable/Disable saving of non-mesh related data.
	void SetNonMeshDataSaving(bool bEnable);

	// Enable/disable saving of physics meshes.
	void SetSavePhysicsMeshes(bool bEnable);

	// Enable compaction of vertex streams (for optimised streaming)
	void SetVertexStreamCompacting(bool bEnable);

	// Enable computation of subset texel density
	void SetSubsetTexelDensityComputing(bool bEnable);

	// Store nodes in chunk file.
	#if defined(RESOURCE_COMPILER)
	void SaveNodes(bool bSwapEndian, bool bStorePositionsAsF16, bool bUseQtangents, bool bStoreIndicesAsU16, CInternalSkinningInfo* pSkinningInfo);
	i32  SaveNode(CNodeCGF* pNode, bool bSwapEndian, bool bStorePositionsAsF16, bool bUseQtangents, bool bStoreIndicesAsU16, CInternalSkinningInfo* pSkinningInfo);
	#else
	void SaveNodes(bool bSwapEndian, bool bStorePositionsAsF16, bool bUseQtangents, bool bStoreIndicesAsU16);
	i32  SaveNode(CNodeCGF* pNode, bool bSwapEndian, bool bStorePositionsAsF16, bool bUseQtangents, bool bStoreIndicesAsU16);
	#endif
	void SaveMaterials(bool bSwapEndian);
	i32  SaveMaterial(CMaterialCGF* pMtl, bool bNeedSwap);
	i32  SaveExportFlags(bool bSwapEndian);

	// Compiled chunks for characters
	i32 SaveCompiledBones(bool bSwapEndian, uk pData, i32 nSize, i32 version);
	i32 SaveCompiledPhysicalBones(bool bSwapEndian, uk pData, i32 nSize, i32 version);
	i32 SaveCompiledPhysicalProxis(bool bSwapEndian, uk pData, i32 nSize, u32 numIntMorphTargets, i32 version);
	i32 SaveCompiledMorphTargets(bool bSwapEndian, uk pData, i32 nSize, u32 numIntMorphTargets, i32 version);
	i32 SaveCompiledIntFaces(bool bSwapEndian, uk pData, i32 nSize, i32 version);
	i32 SaveCompiledIntSkinVertices(bool bSwapEndian, uk pData, i32 nSize, i32 version);
	i32 SaveCompiledExt2IntMap(bool bSwapEndian, uk pData, i32 nSize, i32 version);
	i32 SaveCompiledBoneBox(bool bSwapEndian, uk pData, i32 nSize, i32 version);

	// Chunks for characters (for Collada->cgf export)
	i32  SaveBones(bool bSwapEndian, uk pData, i32 numBones, i32 nSize);
	i32  SaveBoneNames(bool bSwapEndian, tuk boneList, i32 numBones, i32 listSize);
	i32  SaveBoneInitialMatrices(bool bSwapEndian, SBoneInitPosMatrix* matrices, i32 numBones, i32 nSize);
	i32  SaveBoneMesh(bool bSwapEndian, const PhysicalProxy& proxy);
	#if defined(RESOURCE_COMPILER)
	void SaveUncompiledNodes();
	i32  SaveUncompiledNode(CNodeCGF* pNode);
	void SaveUncompiledMorphTargets();
	i32  SaveUncompiledNodeMesh(CNodeCGF* pNode);
	i32  SaveUncompiledHelperChunk(CNodeCGF* pNode);
	#endif

	i32 SaveBreakablePhysics(bool bNeedEndianSwap);

	i32 SaveController829(bool bSwapEndian, const CONTROLLER_CHUNK_DESC_0829& ctrlChunk, uk pData, i32 nSize);
	i32 SaveController832(bool bSwapEndian, const CONTROLLER_CHUNK_DESC_0832& ctrlChunk, uk pData, i32 nSize);
	i32 SaveControllerDB905(bool bSwapEndian, const CONTROLLER_CHUNK_DESC_0905& ctrlChunk, uk pData, i32 nSize);

	i32 SaveFoliage();

	i32 SaveVCloth(bool bSwapEndian);

	#if defined(RESOURCE_COMPILER)
	i32 SaveTiming(CInternalSkinningInfo* pSkinningInfo);
	#endif

private:
	// Return mesh chunk id
	i32 SaveNodeMesh(CNodeCGF* pNode, bool bSwapEndian, bool bStorePositionsAsF16, bool bUseQTangents, bool bStoreIndicesAsU16);
	i32 SaveHelperChunk(CNodeCGF* pNode, bool bSwapEndian);
	i32 SaveMeshSubsetsChunk(CMesh& mesh, bool bSwapEndian);
	i32 SaveStreamDataChunk(ukk pStreamData, i32 nStreamType, i32 nCount, i32 nElemSize, bool bSwapEndian);
	i32 SavePhysicalDataChunk(ukk pData, i32 nSize, bool bSwapEndian);

	#if defined(RESOURCE_COMPILER)
	i32 SaveTCB3Track(CInternalSkinningInfo* pSkinningInfo, i32 trackIndex);
	i32 SaveTCBQTrack(CInternalSkinningInfo* pSkinningInfo, i32 trackIndex);
	#endif

private:
	CChunkFile*             m_pChunkFile;
	CContentCGF*            m_pCGF;
	std::set<CNodeCGF*>     m_savedNodes;
	std::set<CMaterialCGF*> m_savedMaterials;
	std::map<CMesh*, i32>   m_mapMeshToChunk;
	bool                    m_bDoNotSaveMeshData;
	bool                    m_bDoNotSaveNonMeshData;
	bool                    m_bSavePhysicsMeshes;
	bool                    m_bCompactVertexStreams;
	bool                    m_bComputeSubsetTexelDensity;
};

#endif
#endif
