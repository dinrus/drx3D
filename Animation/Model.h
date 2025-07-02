// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/ModelAnimationSet.h> //embedded
#include <drx3D/Animation/ModelMesh.h>         //embedded
#include <drx3D/Animation/Skeleton.h>

class CFacialModel;

enum EJointFlag
{
	eJointFlag_NameHasForearm                    = BIT(1),
	eJointFlag_NameHasCalf                       = BIT(2),
	eJointFlag_NameHasPelvisOrHeadOrSpineOrThigh = BIT(3),
};

#define NODEINDEX_NONE (MAX_JOINT_AMOUNT - 1)
#define NO_ENTRY_FOUND -1
template<class T, i32 BUCKET_AMOUNT>
class CInt32HashMap
{
private:
	inline u32 HashFunction(u32 key) const
	{
		return (key * 1664525 + 1013904223) % BUCKET_AMOUNT;
	};

	struct Node
	{
		T           value;
		i32       key;
		JointIdType next;
		Node() : next(NODEINDEX_NONE) {};
	};

	struct Bucket
	{
		JointIdType firstNode;
		Bucket() : firstNode(NODEINDEX_NONE) {};
	};

	Bucket      m_buckets[BUCKET_AMOUNT];
	Node*       m_nodes;
	JointIdType m_nextFreeNode;
	i16       m_nodeAmount;
public:
	CInt32HashMap(i16 nodeAmount)
	{
		m_nodes = new Node[nodeAmount];
		m_nodeAmount = nodeAmount;
		m_nextFreeNode = 0;
	}
	~CInt32HashMap()
	{
		delete[] m_nodes;
	}

	void Clear()
	{
		for (i32 i = 0; i < BUCKET_AMOUNT; ++i)
		{
			m_buckets[i].firstNode = NODEINDEX_NONE;
		}
		for (i32 i = 0; i < m_nodeAmount; ++i)
		{
			m_nodes[i].next = NODEINDEX_NONE;
		}
		m_nextFreeNode = 0;
	}

	void Insert(u32 key, T value)
	{
		u32 index = HashFunction(key);
		assert(value != NODEINDEX_NONE);
		assert(m_nextFreeNode < m_nodeAmount);
		assert(index < BUCKET_AMOUNT);
		if (m_buckets[index].firstNode == NODEINDEX_NONE)
		{
			m_nodes[m_nextFreeNode].value = value;
			m_nodes[m_nextFreeNode].key = key;
			m_buckets[index].firstNode = m_nextFreeNode;
			m_nextFreeNode++;
			return;
		}
		else
		{
			JointIdType node = m_buckets[index].firstNode;
			while (1)
			{
				if (m_nodes[node].next == NODEINDEX_NONE)
				{
					m_nodes[m_nextFreeNode].value = value;
					m_nodes[m_nextFreeNode].key = key;
					m_nodes[node].next = m_nextFreeNode;
					m_nextFreeNode++;
					return;
				}
				node = m_nodes[node].next;
			}
		}
	}

	i32 Retrieve(u32 key) const
	{
		u32 index = HashFunction(key);
		JointIdType node = m_buckets[index].firstNode;
		while (node != NODEINDEX_NONE)
		{
			if (m_nodes[node].key == key)
			{
				return m_nodes[node].value;
			}
			node = m_nodes[node].next;
		}
		return NO_ENTRY_FOUND;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_buckets, sizeof(Bucket) * BUCKET_AMOUNT);
		pSizer->AddObject(m_nodes, sizeof(Node) * m_nodeAmount);
		pSizer->AddObject(m_nextFreeNode);
		pSizer->AddObject(m_nodeAmount);
	}
};

//----------------------------------------------------------------------

struct IdxAndName
{
	i32       m_idxJoint;
	tukk m_strJoint;
	IdxAndName()
	{
		m_idxJoint = -1;
		;
		m_strJoint = 0;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_strJoint);
	}
};
struct IKLimbType
{
	LimbIKDefinitionHandle m_nHandle;
	u32                 m_nSolver;      //32-bit string
	u32                 m_nInterations; //only need for iterative solvers
	f32                    m_fThreshold;   //only need for iterative solvers
	f32                    m_fStepSize;    //only need for iterative solvers
	DynArray<IdxAndName>   m_arrJointChain;
	DynArray<i16>        m_arrLimbChildren;
	DynArray<i16>        m_arrRootToEndEffector;
	IKLimbType()
	{
		m_nHandle = 0;
		m_nSolver = 0;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_arrJointChain);
		pSizer->AddObject(m_arrLimbChildren);
		pSizer->AddObject(m_arrRootToEndEffector);
	}
};

struct SRecoilJoints
{
	tukk m_strJointName;
	i16       m_nIdx;
	i16       m_nArm;
	f32         m_fWeight;
	f32         m_fDelay;
	SRecoilJoints()
	{
		m_strJointName = 0;
		m_nIdx = -1;
		m_nArm = 0;
		m_fWeight = 0.0f;
		m_fDelay = 0.0f;
	};
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct ProcAdjust
{
	tukk m_strJointName;
	i16       m_nIdx;
	ProcAdjust()
	{
		m_strJointName = 0;
		m_nIdx = -1;
	};
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct RecoilDesc
{
	RecoilDesc()
	{
		m_weaponRightJointIndex = -1;
		m_weaponLeftJointIndex = -1;
	}
	DynArray<SRecoilJoints> m_joints;
	string                  m_ikHandleLeft;
	string                  m_ikHandleRight;
	i32                   m_weaponRightJointIndex;
	i32                   m_weaponLeftJointIndex;
};

struct PoseBlenderAimDesc
{
	PoseBlenderAimDesc()
	{
		m_error = 0;
	}

	u32                      m_error;
	DynArray<DirectionalBlends> m_blends;
	DynArray<SJointsAimIK_Rot>  m_rotations;
	DynArray<SJointsAimIK_Pos>  m_positions;
	DynArray<ProcAdjust>        m_procAdjustments;
};

struct PoseBlenderLookDesc
{
	PoseBlenderLookDesc()
	{
		m_error = 0;
		m_eyeLimitHalfYawRadians = DEG2RAD(45);
		m_eyeLimitPitchRadiansUp = DEG2RAD(45);
		m_eyeLimitPitchRadiansDown = DEG2RAD(45);
	}

	u32                      m_error;
	f32                         m_eyeLimitHalfYawRadians;
	f32                         m_eyeLimitPitchRadiansUp;
	f32                         m_eyeLimitPitchRadiansDown;
	string                      m_eyeAttachmentLeftName;
	string                      m_eyeAttachmentRightName;
	DynArray<DirectionalBlends> m_blends;
	DynArray<SJointsAimIK_Rot>  m_rotations;
	DynArray<SJointsAimIK_Pos>  m_positions;
};

// Animation Driven IK
struct ADIKTarget
{
	LimbIKDefinitionHandle m_nHandle;
	i32                  m_idxTarget;
	tukk            m_strTarget;
	i32                  m_idxWeight;
	tukk            m_strWeight;
	ADIKTarget()
	{
		m_nHandle = 0;
		m_idxTarget = -1;
		m_strTarget = 0;
		m_idxWeight = -1;
		m_strWeight = 0;
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

//----------------------------------------------------------------------
// CDefault Skeleton
//----------------------------------------------------------------------
class CDefaultSkeleton : public IDefaultSkeleton
{
private:
	static uint s_guidLast;

	friend class CharacterUpr;
public:
	CDefaultSkeleton(tukk pSkeletonFilePath, u32 type, uint64 nCRC64 = 0);
	virtual ~CDefaultSkeleton();

	uint   GetGuid() const                         { return m_guid; }

	void   SetInstanceCounter(u32 numInstances) { m_nInstanceCounter = numInstances; }
	void   SetKeepInMemory(bool nKiM)              { m_nKeepInMemory = nKiM; }
	u32 GetKeepInMemory() const                 { return m_nKeepInMemory; }
	void   AddRef()
	{
		++m_nRefCounter;
		//this check will make sure that nobody can hijack a smart-pointer or manipulate the ref-counter through the interface
		if (m_nInstanceCounter != m_nRefCounter)
			DrxFatalError("nRefCounter and registered Skel-Instances must be identical");
	}
	void Release()
	{
		--m_nRefCounter;
		//this check will make sure that nobody can hijack a smart-pointer or manipulate the ref-counter through the interface
		if (m_nInstanceCounter != m_nRefCounter)
			DrxFatalError("DinrusXAnimation: m_nRefCounter and m_nInstanceCounter must be identical");
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

	virtual tukk GetModelFilePath() const                                        { return m_strFilePath.c_str(); }
	uint64              GetModelFilePathCRC64() const                                   { return m_nFilePathCRC64; }
	tukk         GetModelAnimEventDatabaseCStr() const                           { return m_strAnimEventFilePath.c_str(); }
	void                SetModelAnimEventDatabase(const string& sAnimEventDatabasePath) { m_strAnimEventFilePath = sAnimEventDatabasePath;  }
	u32              SizeOfDefaultSkeleton() const;
	void                CopyAndAdjustSkeletonParams(const CDefaultSkeleton* pCDefaultSkeletonSrc);
	i32               RemapIdx(const CDefaultSkeleton* pCDefaultSkeletonSrc, i32k idx);

	//-----------------------------------------------------------------------
	//-----       Interfaces to access the default-skeleton              -----
	//-----------------------------------------------------------------------
	struct SJoint
	{
		SJoint()
		{
			m_nOffsetChildren = 0;
			m_numChildren = 0;
			m_flags = 0;
			m_idxParent = -1;          //index of parent-joint
			m_numLevels = 0;
			m_nJointCRC32 = 0;
			m_nJointCRC32Lower = 0;

			m_CGAObject = 0;
			m_ObjectID = -1;
			m_NodeID = ~0;
			m_PhysInfo.pPhysGeom = 0;
			m_fMass = 0.0f;
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const { pSizer->AddObject(m_CGAObject); pSizer->AddObject(m_strJointName); }

		//--------------------------------------------------------------------------------

		string m_strJointName;     //the name of the joint
		u32 m_nJointCRC32;      //case sensitive CRC32 of joint-name. Used to access case-sensitive controllers in in CAF files
		u32 m_nJointCRC32Lower; //lower case CRC32 of joint-name.

		i32  m_nOffsetChildren;  //this is 0 if there are no children
		u16 m_numChildren;      //how many children does this joint have
		u16 m_flags;
		i16  m_idxParent;        //index of parent-joint. if the idx==-1 then this joint is the root. Usually this values are > 0
		u16 m_numLevels;

		//#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
		//--------> the rest is deprecated and will disappear sooner or later
		//#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
		DrxBonePhysics       m_PhysInfo;
		f32                  m_fMass;     //this doesn't need to be in every joint
		_smart_ptr<IStatObj> m_CGAObject; // Static object controlled by this joint.
		u32               m_NodeID;    // CGA-node
		u16               m_ObjectID;  // used by CGA
	};

	//! Compact helper structure used for efficient computation of joint<->controller mapping.
	struct SJointDescriptor
	{
		JointIdType id; //!< Index of the joint within its skeleton.
		u32 crc32;   //!< Case-sensitive crc32 sum of the joint name.
	};

	virtual u32 GetJointCount() const
	{
		return m_arrModelJoints.size();
	};

	virtual i32 GetJointIDByCRC32(u32 crc32) const
	{
		if (m_pJointsCRCToIDMap)
			return m_pJointsCRCToIDMap->Retrieve(crc32);
		else
			return -1;
	}
	virtual u32 GetJointCRC32ByID(i32 nJointID) const
	{
		i32 numJoints = m_arrModelJoints.size();
		if (nJointID >= 0 && nJointID < numJoints)
			return m_arrModelJoints[nJointID].m_nJointCRC32Lower;
		else
			return -1;
	}

	virtual tukk GetJointNameByID(i32 nJointID) const    // Return name of bone from bone table, return zero id nId is out of range
	{
		i32 numJoints = m_arrModelJoints.size();
		if (nJointID >= 0 && nJointID < numJoints)
			return m_arrModelJoints[nJointID].m_strJointName.c_str();
		assert("GetJointNameByID - Index out of range!");
		return ""; // invalid bone id
	}
	virtual i32 GetJointIDByName(tukk strJointName) const
	{
		u32 crc32 = CCrc32::ComputeLowercase(strJointName);
		if (m_pJointsCRCToIDMap)
			return m_pJointsCRCToIDMap->Retrieve(crc32);
		else
			return -1;
	}

	virtual i32 GetJointParentIDByID(i32 nChildID) const
	{
		i32 numJoints = m_arrModelJoints.size();
		if (nChildID >= 0 && nChildID < numJoints)
			return m_arrModelJoints[nChildID].m_idxParent;
		assert(0);
		return -1;
	}
	virtual i32 GetControllerIDByID(i32 nJointID) const
	{
		i32 numJoints = m_arrModelJoints.size();
		if (nJointID >= 0 && nJointID < numJoints)
			return m_arrModelJoints[nJointID].m_nJointCRC32;
		else
			return -1;
	}

	virtual i32 GetJointChildrenCountByID(i32 id) const
	{
		i32k numJoints = m_arrModelJoints.size();

		if (id >= 0 && id < numJoints)
		{
			return static_cast<i32>(m_arrModelJoints[id].m_numChildren);
		}

		return -1;
	}

	virtual i32 GetJointChildIDAtIndexByID(i32 id, u32 childIndex) const
	{
		i32k numChildren = GetJointChildrenCountByID(id);

		if (numChildren != -1 && childIndex < numChildren)
		{
			return id + m_arrModelJoints[id].m_nOffsetChildren + childIndex;
		}

		return -1;
	}

	virtual const QuatT& GetDefaultAbsJointByID(u32 nJointIdx) const
	{
		u32 jointCount = m_poseDefaultData.GetJointCount();
		if (nJointIdx < jointCount)
			return m_poseDefaultData.GetJointAbsolute(nJointIdx);
		assert(false);
		return g_IdentityQuatT;
	};
	virtual const QuatT& GetDefaultRelJointByID(u32 nJointIdx) const
	{
		u32 jointCount = m_poseDefaultData.GetJointCount();
		if (nJointIdx < jointCount)
			return m_poseDefaultData.GetJointRelative(nJointIdx);
		assert(false);
		return g_IdentityQuatT;
	};
	virtual const phys_geometry* GetJointPhysGeom(u32 jointIndex) const
	{
		return m_arrModelJoints[jointIndex].m_PhysInfo.pPhysGeom;
	}
	virtual DrxBonePhysics* GetJointPhysInfo(u32 jointIndex)
	{
		return &m_arrModelJoints[jointIndex].m_PhysInfo;
	}
	virtual const DynArray<SBoneShadowCapsule>& GetShadowCapsules() const
	{
		return m_ShadowCapsulesList;
	}

	bool LoadNewSKEL(tukk szFilePath, u32 nLoadingFlags);
	void LoadCHRPARAMS(tukk paramFileName);

	//--> setup of physical proxies
	bool                            SetupPhysicalProxies(const DynArray<PhysicalProxy>& arrPhyBoneMeshes, const DynArray<BONE_ENTITY>& arrBoneEntities, IMaterial* pIMaterial, tukk filename);

	static bool                     ParsePhysInfoProperties_ROPE(DrxBonePhysics& pi, const DynArray<SJointProperty>& props);
	static DynArray<SJointProperty> GetPhysInfoProperties_ROPE(const DrxBonePhysics& pi, i32 nRopeOrGrid);

	CDefaultSkeleton::SJoint*       GetParent(i32 i)
	{
		i32 pidx = m_arrModelJoints[i].m_idxParent;
		if (pidx < 0)
			return 0;
		return &m_arrModelJoints[pidx];
	}

	i32 GetLimbDefinitionIdx(LimbIKDefinitionHandle nHandle) const
	{
		u32 numLimbTypes = m_IKLimbTypes.size();
		for (u32 lt = 0; lt < numLimbTypes; lt++)
		{
			if (nHandle == m_IKLimbTypes[lt].m_nHandle)
				return lt;
		}
		return -1;
	}
	const IKLimbType* GetLimbDefinition(i32 index) const { return &m_IKLimbTypes[index]; }

	void              RebuildJointLookupCaches()
	{
		i32k numBones = m_arrModelJoints.size();

		if (m_pJointsCRCToIDMap)
		{
			delete m_pJointsCRCToIDMap;
		}
		m_pJointsCRCToIDMap = new CInt32HashMap<JointIdType, 512>(numBones);

		for (i32 i = 0; i < numBones; ++i)
		{
			u32k crc32Lower = m_arrModelJoints[i].m_nJointCRC32Lower;
			m_pJointsCRCToIDMap->Insert(crc32Lower, i);
		}

		m_crcOrderedJointDescriptors.clear();
		m_crcOrderedJointDescriptors.reserve(numBones);
		for (i32 i = 0; i < numBones; ++i)
		{
			u32k crc32 = m_arrModelJoints[i].m_nJointCRC32;
			m_crcOrderedJointDescriptors.push_back({ JointIdType(i), crc32 });
		}
		std::sort(m_crcOrderedJointDescriptors.begin(), m_crcOrderedJointDescriptors.end(), [](const SJointDescriptor& lhs, const SJointDescriptor& rhs) { return lhs.crc32 < rhs.crc32; });
	}

	void                       InitializeHardcodedJointsProperty();
	const Skeleton::CPoseData& GetPoseData() const    { return m_poseDefaultData; }
	u32                     SizeOfSkeleton() const { return 0; }
	void                       GetMemoryUsage(IDrxSizer* pSizer) const;
	void                       VerifyHierarchy();

	DynArray<SJoint>                 m_arrModelJoints;    // This is the bone hierarchy. All the bones of the hierarchy are present in this array
	CInt32HashMap<JointIdType, 512>* m_pJointsCRCToIDMap; //this dramatically accelerates access to JointIDs by CRC - overall consumption should be less than 100 kb throughout the game (2-3 kb per ModelSkeleton)
	std::vector<SJointDescriptor>    m_crcOrderedJointDescriptors; //!< Array of joint descriptors, sorted by crc32.

	Skeleton::CPoseData              m_poseDefaultData;

	string                           m_strFeetLockIKHandle[MAX_FEET_AMOUNT];
	DynArray<IKLimbType>             m_IKLimbTypes;
	DynArray<ADIKTarget>             m_ADIKTargets;
	RecoilDesc                       m_recoilDesc;
	PoseBlenderLookDesc              m_poseBlenderLookDesc;
	PoseBlenderAimDesc               m_poseBlenderAimDesc;
	u32                           m_usePhysProxyBBox;
	DynArray<i32>                  m_BBoxIncludeList;
	DynArray<SBoneShadowCapsule>     m_ShadowCapsulesList;
	AABB                             m_AABBExtension;
	AABB                             m_ModelAABB; //AABB of the model in default pose
	bool                             m_bHasPhysics2;
	DynArray<PhysicalProxy>          m_arrBackupPhyBoneMeshes; //collision proxi
	DynArray<BONE_ENTITY>            m_arrBackupBoneEntities;  //physical-bones
	DynArray<DrxBonePhysics>         m_arrBackupPhysInfo;
	uint                             m_guid;

	//-----------------------------------------------------------------------
	// Returns the interface for animations applicable to this model
	//-----------------------------------------------------------------------
	DynArray<u32> m_animListIDs;
	IAnimationSet*       GetIAnimationSet()       { return m_pAnimationSet; }
	const IAnimationSet* GetIAnimationSet() const { return m_pAnimationSet.get(); }
	bool LoadAnimations(class CParamLoader& paramLoader);   //loads animations by using the output of the ParamLoader
	const string         GetDefaultAnimDir();
	u32               LoadAnimationFiles(CParamLoader& paramLoader, u32 listID);  // load animation files that were parsed from the param file into memory (That are not in DBA)
	u32               ReuseAnimationFiles(CParamLoader& paramLoader, u32 listID); // files that are already in memory can be reused

	//! Returns sorted range of crc32 identifiers representing joints belonging to the given animation lod. If the returned range is empty, it signifies that LoD includes all joints.
	std::pair<u32k*, u32k*> FindClosestAnimationLod(i32k lodValue) const;

	_smart_ptr<CAnimationSet>  m_pAnimationSet;
	DynArray<DynArray<u32>> m_arrAnimationLOD;

	//////////////////////////////////////////////////////////////////////////
	void                CreateFacialInstance();// Called on a model after it was completely loaded.
	void                LoadFaceLib(tukk faceLibFile, tukk animDirName, CDefaultSkeleton* pDefaultSkeleton);
	CFacialModel*       GetFacialModel()       { return m_pFacialModel; }
	const CFacialModel* GetFacialModel() const { return m_pFacialModel; }

private:
	const string  m_strFilePath;
	const uint64  m_nFilePathCRC64;
	i32           m_nRefCounter, m_nInstanceCounter;
	i32           m_nKeepInMemory;
	CFacialModel* m_pFacialModel;
	string        m_strAnimEventFilePath;

	//#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
	//----> CModelMeshm, CGAs and all interfaces to access the mesh and the IMaterials will be removed in the future
	//#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
public:
	IMaterial* GetIMaterial() const
	{
		if (m_pCGA_Object)
			return m_pCGA_Object->GetMaterial();
		return m_ModelMesh.m_pIDefaultMaterial;
	}
	TRenderChunkArray* GetRenderMeshMaterials()
	{
		if (m_ModelMesh.m_pIRenderMesh)
			return &m_ModelMesh.m_pIRenderMesh->GetChunks();
		else
			return NULL;
	}
	u32               GetModelMeshCount() const   { return 1; }
	CModelMesh*          GetModelMesh()              { return m_ModelMeshEnabled ? &m_ModelMesh : nullptr; }
	virtual void         PrecacheMesh(bool bFullUpdate, i32 nRoundId, i32 nLod);
	virtual Vec3         GetRenderMeshOffset() const { return m_ModelMesh.m_vRenderMeshOffset; };
	virtual IRenderMesh* GetIRenderMesh() const      { return m_ModelMesh.m_pIRenderMesh;  }
	virtual u32       GetTextureMemoryUsage2(IDrxSizer* pSizer = 0) const;
	virtual u32       GetMeshMemoryUsage(IDrxSizer* pSizer = 0) const;

public:

	u32               m_ObjectType;
	_smart_ptr<IStatObj> m_pCGA_Object;

private:

	CModelMesh m_ModelMesh;
	bool       m_ModelMeshEnabled;
};
