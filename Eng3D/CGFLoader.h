// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CGFLoader.h
//  Version:     v1.00
//  Created:     6/11/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CGFLoader_h__
#define __CGFLoader_h__
#pragma once

#include <drx3D/Eng3D/MeshCompiler.h>
#include <drx3D/Eng3D/ChunkFile.h>
#include <drx3D/Eng3D/CGFContent.h>

#if defined(RESOURCE_COMPILER)
	#include "../../../Tools/DrxCommonTools/Export/MeshUtils.h"
#endif

//////////////////////////////////////////////////////////////////////////
class ILoaderCGFListener
{
public:
	virtual ~ILoaderCGFListener(){}
	virtual void Warning(tukk format) = 0;
	virtual void Error(tukk format) = 0;
	virtual bool IsValidationEnabled() { return true; }
};

class CLoaderCGF
{
public:
	typedef uk (* AllocFncPtr)(size_t);
	typedef void (*  DestructFncPtr)(uk );

	CLoaderCGF(AllocFncPtr pAlloc = operator new, DestructFncPtr pDestruct = operator delete, bool bAllowStreamSharing = true);
	~CLoaderCGF();

	CContentCGF* LoadCGF(tukk filename, IChunkFile& chunkFile, ILoaderCGFListener* pListener, u64 nLoadingFlags = 0);
	bool         LoadCGF(CContentCGF* pContentCGF, tukk filename, IChunkFile& chunkFile, ILoaderCGFListener* pListener, u64 nLoadingFlags = 0);
	bool         LoadCGFFromMem(CContentCGF* pContentCGF, ukk pData, size_t nDataLen, IChunkFile& chunkFile, ILoaderCGFListener* pListener, u64 nLoadingFlags = 0);

	tukk  GetLastError()                                  { return m_LastError; }
	CContentCGF* GetCContentCGF()                                { return m_pCompiledCGF; }

	void         SetMaxWeightsPerVertex(i32 maxWeightsPerVertex) { m_maxWeightsPerVertex = maxWeightsPerVertex; }

private:
	bool          LoadCGF_Int(CContentCGF* pContentCGF, tukk filename, IChunkFile& chunkFile, ILoaderCGFListener* pListener, u64 nLoadingFlags);

	bool          LoadChunks(bool bJustGeometry);
	bool          LoadExportFlagsChunk(IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadNodeChunk(IChunkFile::ChunkDesc* pChunkDesc, bool bJustGeometry);

	bool          LoadHelperChunk(CNodeCGF* pNode, IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadGeomChunk(CNodeCGF* pNode, IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadCompiledMeshChunk(CNodeCGF* pNode, IChunkFile::ChunkDesc* pChunkDesc);
	bool          LoadMeshSubsetsChunk(CMesh& mesh, IChunkFile::ChunkDesc* pChunkDesc, std::vector<std::vector<u16>>& globalBonesPerSubset);
	bool          LoadStreamDataChunk(i32 nChunkId, uk & pStreamData, i32& nStreamType, i32& nCount, i32& nElemSize, bool& bSwapEndianness);
	template<class T>
	bool          LoadStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk, ECgfStreamType Type, CMesh::EStream MStream);
	template<class TA, class TB>
	bool          LoadStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk, ECgfStreamType Type, CMesh::EStream MStreamA, CMesh::EStream MStreamB);
	bool          LoadBoneMappingStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk, const std::vector<std::vector<u16>>& globalBonesPerSubset);
	bool          LoadIndexStreamChunk(CMesh& mesh, const MESH_CHUNK_DESC_0801& chunk);

	bool          LoadPhysicsDataChunk(CNodeCGF* pNode, i32 nPhysGeomType, i32 nChunkId);

	bool          LoadFoliageInfoChunk(IChunkFile::ChunkDesc* pChunkDesc);

	bool LoadVClothChunk(IChunkFile::ChunkDesc* pChunkDesc);

	CMaterialCGF* LoadMaterialFromChunk(i32 nChunkId);

	CMaterialCGF* LoadMaterialNameChunk(IChunkFile::ChunkDesc* pChunkDesc);

	void          SetupMeshSubsets(CMesh& mesh, CMaterialCGF* pMaterialCGF);

	//////////////////////////////////////////////////////////////////////////
	// loading of skinned meshes
	//////////////////////////////////////////////////////////////////////////
	bool         ProcessSkinning();
	CContentCGF* MakeCompiledSkinCGF(CContentCGF* pCGF, std::vector<i32>* pVertexRemapping, std::vector<i32>* pIndexRemapping);

	//old chunks
	bool   ReadBoneNameList(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadMorphTargets(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadBoneInitialPos(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadBoneHierarchy(IChunkFile::ChunkDesc* pChunkDesc);
	u32 RecursiveBoneLoader(i32 nBoneParentIndex, i32 nBoneIndex);
	bool   ReadBoneMesh(IChunkFile::ChunkDesc* pChunkDesc);

	//new chunks
	bool ReadCompiledBones(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledPhysicalBones(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledPhysicalProxies(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledMorphTargets(IChunkFile::ChunkDesc* pChunkDesc);

	bool ReadCompiledIntFaces(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledIntSkinVertice(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledExt2IntMap(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadCompiledBonesBoxes(IChunkFile::ChunkDesc* pChunkDesc);

	bool ReadCompiledBreakablePhysics(IChunkFile::ChunkDesc* pChunkDesc);

	void Warning(tukk szFormat, ...) PRINTF_PARAMS(2, 3);

private:
	u32                              m_IsCHR;
	u32                              m_CompiledBones;
	u32                              m_CompiledBonesBoxes;
	u32                              m_CompiledMesh;

	u32                              m_numBonenameList;
	u32                              m_numBoneInitialPos;
	u32                              m_numMorphTargets;
	u32                              m_numBoneHierarchy;

	std::vector<u32>                 m_arrIndexToId;     // the mapping BoneIndex -> BoneID
	std::vector<u32>                 m_arrIdToIndex;     // the mapping BoneID -> BineIndex
	std::vector<string>                 m_arrBoneNameTable; // names of bones
	std::vector<Matrix34>               m_arrInitPose34;
#if defined(RESOURCE_COMPILER)
	std::vector<MeshUtils::VertexLinks> m_arrLinksTmp;
	std::vector<i32>                    m_vertexOldToNew; // used to re-map uncompiled Morph Target vertices right after reading them
#endif

	CContentCGF* m_pCompiledCGF;
	ukk  m_pBoneAnimRawData, * m_pBoneAnimRawDataEnd;
	u32       m_numBones;
	i32          m_nNextBone;

	//////////////////////////////////////////////////////////////////////////

	string       m_LastError;

	char         m_filename[260];

	IChunkFile*  m_pChunkFile;
	CContentCGF* m_pCGF;

	// To find really used materials
	u16              MatIdToSubset[MAX_SUB_MATERIALS];
	i32                 nMaterialsUsed;

	ILoaderCGFListener* m_pListener;

	bool                m_bUseReadOnlyMesh;
	bool                m_bAllowStreamSharing;

	i32                 m_maxWeightsPerVertex;

	AllocFncPtr         m_pAllocFnc;
	DestructFncPtr      m_pDestructFnc;
};

#endif //__CGFLoader_h__
