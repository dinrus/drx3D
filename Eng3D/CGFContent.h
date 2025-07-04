// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Eng3D/IIndexedMesh.h>   // <> required for Interfuscator
#include <drx3D/Eng3D/IChunkFile.h> // <> required for Interfuscator
#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/String/StringUtils.h>

struct CMaterialCGF;

#define CGF_NODE_NAME_LOD_PREFIX     "$lod"
#define CGF_NODE_NAME_PHYSICS_PROXY0 "PhysicsProxy"
#define CGF_NODE_NAME_PHYSICS_PROXY1 "$collision"
#define CGF_NODE_NAME_PHYSICS_PROXY2 "$physics_proxy"

//! This structure represents CGF node.
struct CNodeCGF : public _cfg_reference_target<CNodeCGF>
{
	enum ENodeType
	{
		NODE_MESH,
		NODE_LIGHT,
		NODE_HELPER,
	};
	enum EPhysicalizeFlags
	{
		ePhysicsalizeFlag_MeshNotNeeded = BIT(2), //!< When set physics data doesn't need additional Mesh indices or vertices.
		ePhysicsalizeFlag_NoBreaking    = BIT(3), //!< node is unsuitable for procedural 3d breaking
	};

	ENodeType     type;
	char          name[64];
	string        properties;
	Matrix34      localTM;                      //!< Local space transformation matrix.
	Matrix34      worldTM;                      //!< World space transformation matrix.
	CNodeCGF*     pParent;                      //!< Pointer to parent node.
	CNodeCGF*     pSharedMesh;                  //!< Not NULL if this node is sharing mesh and physics from referenced Node.
	CMesh*        pMesh;                        //!< Pointer to mesh loaded for this node. (Only when type == NODE_MESH).

	HelperTypes   helperType;                   //!< Only relevant if type==NODE_HELPER.
	Vec3          helperSize;                   //!< Only relevant if type==NODE_HELPER.

	CMaterialCGF* pMaterial;                     //!< Material node.

	//! Physical data of the node with mesh.
	i32            nPhysicalizeFlags;           //!< Saved into the nFlags2 chunk member.
	DynArray<char> physicalGeomData[4];

	//////////////////////////////////////////////////////////////////////////
	//! Used internally.
	i32 nChunkId;                               //!< Chunk id as loaded from CGF.
	i32 nParentChunkId;                         //!< Chunk id of parent Node.
	i32 nObjectChunkId;                         //!< Chunk id of the corresponding mesh.
	i32 pos_cont_id;                            //!< Position controller chunk id.
	i32 rot_cont_id;                            //!< Rotation controller chunk id.
	i32 scl_cont_id;                            //!< Scale controller chunk id.
	//////////////////////////////////////////////////////////////////////////

	//! True if worldTM is identity.
	bool bIdentityMatrix;

	//! True when this node is invisible physics proxy.
	bool bPhysicsProxy;

	//! These values are not saved, but are only used for loading empty mesh chunks.
	struct MeshInfo
	{
		i32   nVerts;
		i32   nIndices;
		i32   nSubsets;
		Vec3  bboxMin;
		Vec3  bboxMax;
		float fGeometricMean;
	};
	MeshInfo    meshInfo;

	DrxSkinVtx* pSkinInfo;                       //!< for skinning with skeleton meshes (deformable objects)

	//! Constructor.
	void Init()
	{
		type = NODE_MESH;
		localTM.SetIdentity();
		worldTM.SetIdentity();
		pParent = 0;
		pSharedMesh = 0;
		pMesh = 0;
		pMaterial = 0;
		helperType = HP_POINT;
		helperSize.Set(0, 0, 0);
		nPhysicalizeFlags = 0;
		nChunkId = 0;
		nParentChunkId = 0;
		nObjectChunkId = 0;
		pos_cont_id = rot_cont_id = scl_cont_id = 0;
		bIdentityMatrix = true;
		bPhysicsProxy = false;
		pSkinInfo = 0;

		ZeroStruct(meshInfo);
	}

	CNodeCGF()
	{
		Init();
	}

	explicit CNodeCGF(_cfg_reference_target<CNodeCGF>::DeleteFncPtr pDeleteFnc)
		: _cfg_reference_target<CNodeCGF>(pDeleteFnc)
	{
		Init();
	}

	~CNodeCGF()
	{
		if (!pSharedMesh)
		{
			delete pMesh;
		}
		if (pSkinInfo)
		{
			delete[] pSkinInfo;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// structures for skinning
//////////////////////////////////////////////////////////////////////////

struct TFace
{
	u16 i0, i1, i2;
	TFace() {}
	TFace(u16 v0, u16 v1, u16 v2) { i0 = v0; i1 = v1; i2 = v2; }
	TFace(const DrxFace& face)  { i0 = face[0]; i1 = face[1]; i2 = face[2]; }
	void operator=(const TFace& f)               { i0 = f.i0; i1 = f.i1; i2 = f.i2;  }
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
	AUTO_STRUCT_INFO;
};

struct PhysicalProxy
{
	u32           ChunkID;
	DynArray<Vec3>   m_arrPoints;
	DynArray<u16> m_arrIndices;
	DynArray<char>   m_arrMaterials;
};

struct MorphTargets
{
	u32                           MeshID;
	string                           m_strName;
	DynArray<SMeshMorphTargetVertex> m_arrIntMorph;
	DynArray<SMeshMorphTargetVertex> m_arrExtMorph;
};

typedef MorphTargets* MorphTargetsPtr;

struct IntSkinVertex
{
	Vec3   __obsolete0; //!< Thin/fat vertex position. Must be removed in the next RC refactoring.
	Vec3   pos;         //!< Vertex-position of model.2
	Vec3   __obsolete2; //!< Thin/fat vertex position. Must be removed in the next RC refactoring.
	u16 boneIDs[4];
	f32    weights[4];
	ColorB color;       //!< Index for blend-array.
	void   GetMemoryUsage(IDrxSizer* pSizer) const {}
	AUTO_STRUCT_INFO;
};

//////////////////////////////////////////////////////////////////////////
// TCB Controller implementation.
//////////////////////////////////////////////////////////////////////////

//! Stores position, scale and orientation (in the logarithmic space, i.e. instead of quaternion, its logarithm is stored).
struct PQLogS
{
	Vec3   position;
	Vec3   rotationLog; //!< Logarithm of the rotation.
	Diag33 scale;

	void   Blend(const PQLogS& from, const PQLogS& to, const f32 blendFactor)
	{
		assert(blendFactor >= 0 && blendFactor <= 1);

		position = Vec3::CreateLerp(from.position, to.position, blendFactor);

		const Quat quatFrom = Quat::exp(from.rotationLog);
		const Quat quatTo = Quat::exp(to.rotationLog);
		rotationLog = Quat::log(Quat::CreateNlerp(quatFrom, quatTo, blendFactor));

		scale = Diag33::CreateLerp(from.scale, to.scale, blendFactor);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct CControllerType
{
	u16 m_controllertype;
	u16 m_index;
	CControllerType()
	{
		m_controllertype = 0xffff;
		m_index = 0xffff;
	}
};

struct TCBFlags
{
	u8 f0, f1;
	TCBFlags() { f0 = f1 = 0; }
};

//! Structure for recreating controllers
struct CControllerInfo
{
	u32 m_nControllerID;
	u32 m_nPosKeyTimeTrack;
	u32 m_nPosTrack;
	u32 m_nRotKeyTimeTrack;
	u32 m_nRotTrack;

	CControllerInfo() : m_nControllerID(~0), m_nPosKeyTimeTrack(~0), m_nPosTrack(~0), m_nRotKeyTimeTrack(~0), m_nRotTrack(~0) {}

	AUTO_STRUCT_INFO;
};

struct MeshCollisionInfo
{
	AABB            m_aABB;
	OBB             m_OBB;
	Vec3            m_Pos;
	DynArray<i16> m_arrIndexes;
	i32           m_iBoneId;

	MeshCollisionInfo()
	{
		// This didn't help much.
		// The BBs are reset to opposite infinites,
		// but never clamped/grown by any member points.
		m_aABB.min.zero();
		m_aABB.max.zero();
		m_OBB.m33.SetIdentity();
		m_OBB.h.zero();
		m_OBB.c.zero();
		m_Pos.zero();
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_arrIndexes);
	}
};

struct SJointsAimIK_Rot
{
	tukk m_strJointName;
	i16       m_nJointIdx;
	i16       m_nPosIndex;
	u8       m_nPreEvaluate;
	u8       m_nAdditive;
	i16       m_nRotJointParentIdx;
	SJointsAimIK_Rot()
	{
		m_strJointName = 0;
		m_nJointIdx = -1;
		m_nPosIndex = -1;
		m_nPreEvaluate = 0;
		m_nAdditive = 0;
		m_nRotJointParentIdx = -1;
	};
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct SJointsAimIK_Pos
{
	tukk m_strJointName;
	i16       m_nJointIdx;
	u8       m_nAdditive;
	u8       m_nEmpty;
	SJointsAimIK_Pos()
	{
		m_strJointName = 0;
		m_nJointIdx = -1;
		m_nAdditive = 0;
		m_nEmpty = 0;
	};
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct DirectionalBlends
{
	string      m_AnimToken;
	u32      m_AnimTokenCRC32;
	tukk m_strParaJointName;
	i16       m_nParaJointIdx;
	i16       m_nRotParaJointIdx;
	tukk m_strStartJointName;
	i16       m_nStartJointIdx;
	i16       m_nRotStartJointIdx;
	tukk m_strReferenceJointName;
	i32       m_nReferenceJointIdx;
	DirectionalBlends()
	{
		m_AnimTokenCRC32 = 0;
		m_strParaJointName = 0;
		m_nParaJointIdx = -1;
		m_nRotParaJointIdx = -1;
		m_strStartJointName = 0;
		m_nStartJointIdx = -1;
		m_nRotStartJointIdx = -1;
		m_strReferenceJointName = 0;
		m_nReferenceJointIdx = 1;                     // By default we use the Pelvis.
	};
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct CSkinningInfo : public _reference_target_t
{
	DynArray<DrxBoneDescData>   m_arrBonesDesc;     //!< Animation-bones.

	DynArray<SJointsAimIK_Rot>  m_LookIK_Rot;       //!< Rotational joints used for Look-IK.
	DynArray<SJointsAimIK_Pos>  m_LookIK_Pos;       //!< Positional joints used for Look-IK.
	DynArray<DirectionalBlends> m_LookDirBlends;    //!< Positional joints used for Look-IK.

	DynArray<SJointsAimIK_Rot>  m_AimIK_Rot;        //!< Rotational joints used for Aim-IK.
	DynArray<SJointsAimIK_Pos>  m_AimIK_Pos;        //!< Positional joints used for Aim-IK.
	DynArray<DirectionalBlends> m_AimDirBlends;     //!< Positional joints used for Aim-IK.

	DynArray<PhysicalProxy>     m_arrPhyBoneMeshes; //!< Collision proxy.
	DynArray<MorphTargetsPtr>   m_arrMorphTargets;
	DynArray<TFace>             m_arrIntFaces;
	DynArray<IntSkinVertex>     m_arrIntVertices;
	DynArray<u16>            m_arrExt2IntMap;
	DynArray<BONE_ENTITY>       m_arrBoneEntities;  //!< Physical-bones.
	DynArray<MeshCollisionInfo> m_arrCollisions;

	u32                      m_numChunks;

	CSkinningInfo(){}

	~CSkinningInfo()
	{
		for (DynArray<MorphTargetsPtr>::iterator it = m_arrMorphTargets.begin(), end = m_arrMorphTargets.end(); it != end; ++it)
		{
			delete *it;
		}
	}

	i32 GetJointIDByName(tukk strJointName) const
	{
		u32 numJoints = m_arrBonesDesc.size();
		for (u32 i = 0; i < numJoints; i++)
		{
			if (stricmp(m_arrBonesDesc[i].m_arrBoneName, strJointName) == 0)
				return i;
		}
		return -1;
	}

	//! \return name of bone from bone table, or empty string if nId is out of range
	tukk GetJointNameByID(i32 nJointID) const
	{
		i32 numJoints = m_arrBonesDesc.size();
		if (nJointID >= 0 && nJointID < numJoints)
			return m_arrBonesDesc[nJointID].m_arrBoneName;
		return "";     // invalid bone id
	}

};

// Fixed-size data stored for each vertex in a VCloth mesh.
struct SVClothVertexAttributes
{
	// Nearest Neighbor Distance Constraints
	u32 nndcIdx;
	u32 nndcNextParent;
	f32    nndcDist;

	AUTO_STRUCT_INFO;
};

struct SVClothLink
{
	i32 i1, i2;
	f32   lenSqr;

	AUTO_STRUCT_INFO;
};

enum EVClothLink
{
	eVClothLink_Stretch = 0,
	eVClothLink_Shear   = 1,
	eVClothLink_Bend    = 2,

	eVClothLink_COUNT
};

struct SVClothChunkVertex
{
	SVClothVertexAttributes attributes;
	i32                     linkCount[eVClothLink_COUNT];

	AUTO_STRUCT_INFO;
};

struct SVClothVertex
{
	SVClothVertexAttributes attributes;

	AUTO_STRUCT_INFO;
};

struct SVClothNndcNotAttachedOrderedIdx
{
	i32 nndcNotAttachedOrderedIdx;
	SVClothNndcNotAttachedOrderedIdx() : nndcNotAttachedOrderedIdx(-1) {}

	AUTO_STRUCT_INFO;
};

struct SVClothBendTrianglePair
{
	// Params
	f32    angle;      //!< initial angle between triangles
	u32 p0, p1;     //!< shared edge
	u32 p2;         //!< first triangle // oriented 0,1,2
	u32 p3;         //!< second triangle // reverse oriented 1,0,3
	u32 idxNormal0; //!< idx of BendTriangle for first triangle
	u32 idxNormal1; //!< idx of BendTriangle for second triangle
	SVClothBendTrianglePair() : p0(-1), p1(-1), p2(-1), p3(-1), idxNormal0(-1), idxNormal1(-1), angle(0) {}

	AUTO_STRUCT_INFO;
};

struct SVClothBendTriangle
{
	u32 p0, p1, p2; //!< Indices of according triangle
	SVClothBendTriangle() : p0(-1), p1(-1), p2(-1) {}

	AUTO_STRUCT_INFO;
};

struct SVClothInfoCGF
{
	DynArray<SVClothVertex>                    m_vertices;
	DynArray<SVClothBendTrianglePair>          m_trianglePairs;
	DynArray<SVClothBendTriangle>              m_triangles;
	DynArray<SVClothNndcNotAttachedOrderedIdx> m_nndcNotAttachedOrderedIdx;
	DynArray<SVClothLink>                      m_links[eVClothLink_COUNT];
};

//! This structure represents Material inside CGF.
struct CMaterialCGF : public _cfg_reference_target<CMaterialCGF>
{
	char  name[128]; //!< Material name;
	i32   nFlags;    //!< Material flags.
	i32   nPhysicalizeType;
	bool  bOldMaterial;
	float shOpacity;

	//! Array of sub materials.
	DynArray<CMaterialCGF*> subMaterials;

	//////////////////////////////////////////////////////////////////////////
	//! Used internally.
	i32 nChunkId;
	//////////////////////////////////////////////////////////////////////////

	void Init()
	{
		nFlags = 0;
		nChunkId = 0;
		bOldMaterial = false;
		nPhysicalizeType = PHYS_GEOM_TYPE_DEFAULT;
		shOpacity = 1.f;
	}

	CMaterialCGF() { Init(); }

	explicit CMaterialCGF(_cfg_reference_target<CMaterialCGF>::DeleteFncPtr pDeleteFnc)
		: _cfg_reference_target<CMaterialCGF>(pDeleteFnc)
	{ Init(); }
};

//! Info about physicalization of the CGF.
struct CPhysicalizeInfoCGF
{
	bool  bWeldVertices;
	float fWeldTolerance; //!< Min Distance between vertices when they collapse to single vertex if bWeldVertices enabled.

	// breakable physics
	i32   nGranularity;
	i32   nMode;

	Vec3* pRetVtx;
	i32   nRetVtx;
	i32*  pRetTets;
	i32   nRetTets;

	CPhysicalizeInfoCGF() : bWeldVertices(true), fWeldTolerance(0.01f), nMode(-1), nGranularity(-1), pRetVtx(0),
		nRetVtx(0), pRetTets(0), nRetTets(0){}

	~CPhysicalizeInfoCGF()
	{
		if (pRetVtx)
		{
			delete[]pRetVtx;
			pRetVtx = 0;
		}
		if (pRetTets)
		{
			delete[]pRetTets;
			pRetTets = 0;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// Serialized skinnable foliage data
//////////////////////////////////////////////////////////////////////////

struct SSpineRC
{
	SSpineRC() { pVtx = 0; pSegDim = 0; }
	~SSpineRC() { if (pVtx) delete[] pVtx; if (pSegDim) delete[] pSegDim; }

	Vec3* pVtx;
	Vec4* pSegDim;
	i32   nVtx;
	float len;
	Vec3  navg;
	i32   iAttachSpine;
	i32   iAttachSeg;
};

struct SFoliageInfoCGF
{
	SFoliageInfoCGF() { nSpines = 0; pSpines = 0; pBoneMapping = 0; }
	~SFoliageInfoCGF()
	{
		if (pSpines)
		{
			for (i32 i = 1; i < nSpines; i++)      // spines 1..n-1 use the same buffer, so make sure they don't delete it
			{
				pSpines[i].pVtx = 0, pSpines[i].pSegDim = 0;
			}
			delete[] pSpines;
		}
		if (pBoneMapping) delete[] pBoneMapping;
	}

	SSpineRC*                      pSpines;
	i32                            nSpines;
	struct SMeshBoneMapping_uint8* pBoneMapping;
	i32                            nSkinnedVtx;
	DynArray<u16>               chunkBoneIds;
};

//////////////////////////////////////////////////////////////////////////
struct CExportInfoCGF
{
	bool         bMergeAllNodes;
	bool         bUseCustomNormals;
	bool         bCompiledCGF;
	bool         bHavePhysicsProxy;
	bool         bHaveAutoLods;
	bool         bNoMesh;
	bool         bWantF32Vertices;
	bool         b8WeightsPerVertex;
	bool         bMakeVCloth;

	bool         bFromColladaXSI;
	bool         bFromColladaMAX;
	bool         bFromColladaMAYA;

	u32 rc_version[4];         //!< Resource compiler version.
	char         rc_version_string[16]; //!< Resource compiler version as a string.

	u32 authorToolVersion;
};

//! This class contain all info loaded from the CGF file.
class CContentCGF
{
public:
	//////////////////////////////////////////////////////////////////////////
	CContentCGF(tukk filename)
	{
		drx_strcpy(m_filename, filename);
		memset(&m_exportInfo, 0, sizeof(m_exportInfo));
		m_exportInfo.bMergeAllNodes = true;
		m_exportInfo.bUseCustomNormals = false;
		m_exportInfo.bWantF32Vertices = false;
		m_exportInfo.b8WeightsPerVertex = false;
		m_exportInfo.bMakeVCloth = false;
		m_pCommonMaterial = 0;
		m_bConsoleFormat = false;
		m_pOwnChunkFile = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	virtual ~CContentCGF()
	{
		// Free nodes.
		m_nodes.clear();
		if (m_pOwnChunkFile)
		{
			m_pOwnChunkFile->Release();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	tukk GetFilename() const
	{
		return m_filename;
	}

	void SetFilename(tukk filename)
	{
		drx_strcpy(m_filename, filename);
	}

	//////////////////////////////////////////////////////////////////////////
	//! Access to CGF nodes.
	void AddNode(CNodeCGF* pNode)
	{
		m_nodes.push_back(pNode);
	}

	i32 GetNodeCount() const
	{
		return m_nodes.size();
	}

	CNodeCGF* GetNode(i32 i)
	{
		return m_nodes[i];
	}

	const CNodeCGF* GetNode(i32 i) const
	{
		return m_nodes[i];
	}

	void ClearNodes()
	{
		m_nodes.clear();
	}

	void RemoveNode(CNodeCGF* pNode)
	{
		assert(pNode);
		for (i32 i = 0; i < m_nodes.size(); ++i)
		{
			if (m_nodes[i] == pNode)
			{
				pNode->pParent = 0;
				m_nodes.erase(i);
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// Access to CGF materials.
	void AddMaterial(CMaterialCGF* pNode)
	{
		m_materials.push_back(pNode);
	}

	i32 GetMaterialCount() const
	{
		return m_materials.size();
	}

	CMaterialCGF* GetMaterial(i32 i)
	{
		return m_materials[i];
	}

	const CMaterialCGF* GetMaterial(i32 i) const
	{
		return m_materials[i];
	}

	void ClearMaterials()
	{
		m_materials.clear();
	}

	CMaterialCGF* GetCommonMaterial() const
	{
		return m_pCommonMaterial;
	}

	void SetCommonMaterial(CMaterialCGF* pMtl)
	{
		m_pCommonMaterial = pMtl;
	}

	DynArray<i32>& GetUsedMaterialIDs()
	{
		return m_usedMaterialIds;
	}

	const DynArray<i32>& GetUsedMaterialIDs() const
	{
		return m_usedMaterialIds;
	}

	//////////////////////////////////////////////////////////////////////////

	CPhysicalizeInfoCGF* GetPhysicalizeInfo()
	{
		return &m_physicsInfo;
	}

	const CPhysicalizeInfoCGF* GetPhysicalizeInfo() const
	{
		return &m_physicsInfo;
	}

	CExportInfoCGF* GetExportInfo()
	{
		return &m_exportInfo;
	}

	const CExportInfoCGF* GetExportInfo() const
	{
		return &m_exportInfo;
	}

	CSkinningInfo* GetSkinningInfo()
	{
		return &m_SkinningInfo;
	}

	const CSkinningInfo* GetSkinningInfo() const
	{
		return &m_SkinningInfo;
	}

	SFoliageInfoCGF* GetFoliageInfo()
	{
		return &m_foliageInfo;
	}

	SVClothInfoCGF* GetVClothInfo()
	{
		return &m_vclothInfo;
	}

	bool GetConsoleFormat()
	{
		return m_bConsoleFormat;
	}

	bool ValidateMeshes(tukk* const ppErrorDescription) const
	{
		for (i32 i = 0; i < m_nodes.size(); ++i)
		{
			const CNodeCGF* const pNode = m_nodes[i];
			if (pNode && pNode->pMesh && (!pNode->pMesh->Validate(ppErrorDescription)))
			{
				return false;
			}
		}
		return true;
	}

	//! Set chunk file that this CGF owns.
	void SetChunkFile(IChunkFile* pChunkFile)
	{
		m_pOwnChunkFile = pChunkFile;
	}

public:
	bool m_bConsoleFormat;

private:
	char                               m_filename[260];
	CSkinningInfo                      m_SkinningInfo;
	DynArray<_smart_ptr<CNodeCGF>>     m_nodes;
	DynArray<_smart_ptr<CMaterialCGF>> m_materials;
	DynArray<i32>                      m_usedMaterialIds;
	_smart_ptr<CMaterialCGF>           m_pCommonMaterial;

	CPhysicalizeInfoCGF                m_physicsInfo;
	CExportInfoCGF                     m_exportInfo;
	SFoliageInfoCGF                    m_foliageInfo;
	SVClothInfoCGF                     m_vclothInfo;

	IChunkFile*                        m_pOwnChunkFile;
};

//! \endcond