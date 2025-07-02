// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/Model.h>

#include<drx3D/Animation/FacialInstance.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/CoreX/String/StringUtils.h>

uint CDefaultSkeleton::s_guidLast = 0;

CDefaultSkeleton::CDefaultSkeleton(tukk pSkeletonFilePath, u32 type, uint64 nCRC64) : m_strFilePath(pSkeletonFilePath), m_nKeepInMemory(0), m_nFilePathCRC64(nCRC64)
{
	m_guid = ++s_guidLast;

	m_pFacialModel = 0;
	m_ObjectType = type;
	m_nRefCounter = 0;
	m_nInstanceCounter = 0;
	m_arrAnimationLOD.reserve(8);

	m_pJointsCRCToIDMap = NULL;
	m_AABBExtension.min = Vec3(ZERO);
	m_AABBExtension.max = Vec3(ZERO);
	m_bHasPhysics2 = 0;
	m_usePhysProxyBBox = 0;

	m_ModelMeshEnabled = false;
}

CDefaultSkeleton::~CDefaultSkeleton()
{
	m_ModelMesh.AbortStream();
	IPhysicalWorld* pIPhysicalWorld = g_pIPhysicalWorld;
	IGeomUpr* pPhysGeomUpr = pIPhysicalWorld ? pIPhysicalWorld->GetGeomUpr() : NULL;
	if (pPhysGeomUpr)
	{
		u32 numJoints = m_arrModelJoints.size();
		for (u32 i = 0; i < numJoints; i++)
		{
			phys_geometry* pPhysGeom = m_arrModelJoints[i].m_PhysInfo.pPhysGeom;
			if (pPhysGeom == 0)
				continue; //joint is not physical geometry
			if ((INT_PTR)pPhysGeom == -1)
				DrxFatalError("Joint '%s' (model '%s') was physicalized but failed to load geometry for some reason. Please check the setup", m_arrModelJoints[i].m_strJointName.c_str(), GetModelFilePath());
			else if ((UINT_PTR)pPhysGeom < 0x400)
				DrxFatalError("Joint '%s' (model '%s') somehow didn't get geometry index processing. At a certain stage the pointer holds an index of a mesh in cgf, and it is supposed to be processed and converted to a geometry pointer", m_arrModelJoints[i].m_strJointName.c_str(), GetModelFilePath());

			PREFAST_ASSUME(pPhysGeom); // would be skipped if null
			if (pPhysGeom->pForeignData)
				pPhysGeomUpr->UnregisterGeometry((phys_geometry*)pPhysGeom->pForeignData);
			pPhysGeomUpr->UnregisterGeometry(pPhysGeom);
		}
		if (m_pJointsCRCToIDMap)
			delete m_pJointsCRCToIDMap;
	}

	SAFE_RELEASE(m_pFacialModel);
	if (m_nInstanceCounter)
		DrxFatalError("The model '%s' still has %d skel-instances. Something went wrong with the ref-counting", m_strFilePath.c_str(), m_nInstanceCounter);
	if (m_nRefCounter)
		DrxFatalError("The model '%s' has the value %d in the m_nRefCounter, while calling the destructor. Something went wrong with the ref-counting", m_strFilePath.c_str(), m_nRefCounter);
	g_pILog->LogToFile("CDefaultSkeleton Release: %s", m_strFilePath.c_str());
	g_pCharacterUpr->UnregisterModelSKEL(this);
}

//////////////////////////////////////////////////////////////////////////
void CDefaultSkeleton::CreateFacialInstance()
{
	CModelMesh* pModelMesh = GetModelMesh();
	if (pModelMesh)
	{
		m_pFacialModel = new CFacialModel(this);
		m_pFacialModel->AddRef();
	}
}

//////////////////////////////////////////////////////////////////////////
void CDefaultSkeleton::VerifyHierarchy()
{
	struct
	{
		struct snode
		{
			u32 m_nJointCRC32Lower; //CRC32 of lowercase of the joint-name.
			i32  m_nOffsetChildren;  //this is 0 if there are no children
			u16 m_numChildren;      //how many children does this joint have
			i16  m_idxParent;        //index of parent-joint. if the idx==-1 then this joint is the root. Usually this values are > 0
			i16  m_idxFirst;         //first child of this joint
			i16  m_idxNext;          //sibling of this joint
		};

		void rebuild(u32 numJoints, CDefaultSkeleton::SJoint* pModelJoints)
		{
			for (u32 j = 0; j < numJoints; j++)
			{
				m_arrHierarchy[j].m_nJointCRC32Lower = pModelJoints[j].m_nJointCRC32Lower;
				m_arrHierarchy[j].m_idxParent = pModelJoints[j].m_idxParent;
				m_arrHierarchy[j].m_nOffsetChildren = 0;
				m_arrHierarchy[j].m_numChildren = 0;
				m_arrHierarchy[j].m_idxFirst = 0;              //first child of this joint
				m_arrHierarchy[j].m_idxNext = 0;               //sibling of this joint
			}
			for (u32 j = 1; j < numJoints; j++)
			{
				i16 p = m_arrHierarchy[j].m_idxParent;
				m_arrHierarchy[p].m_numChildren++;
				if (m_arrHierarchy[p].m_nOffsetChildren == 0)
					m_arrHierarchy[p].m_nOffsetChildren = j - p;
				if (m_arrHierarchy[p].m_idxFirst == 0)
					m_arrHierarchy[p].m_idxFirst = j; //this is the first born child
			}
			for (u32 j = 0; j < numJoints; j++)
			{
				u32 numOffset = m_arrHierarchy[j].m_nOffsetChildren;
				u32 numChildren = m_arrHierarchy[j].m_numChildren;
				for (u32 s = 1; s < numChildren; s++)
					m_arrHierarchy[numOffset + j + s - 1].m_idxNext = numOffset + j + s; //and here come all its little brothers & sisters
			}
			for (u32 j = 0; j < numJoints; j++)
			{
				i32 nOffsetChildren1 = pModelJoints[j].m_nOffsetChildren;   //this is 0 if there are no children
				u16 numChildren1 = pModelJoints[j].m_numChildren;          //how many children does this joint have
				i32 nOffsetChildren2 = m_arrHierarchy[j].m_nOffsetChildren; //this is 0 if there are no children
				u16 numChildren2 = m_arrHierarchy[j].m_numChildren;        //how many children does this joint have
				if (numChildren1 != numChildren2)
					DrxFatalError("ModelError: numChildren must be identical");
				if (nOffsetChildren1 != nOffsetChildren2)
					DrxFatalError("ModelError: child offset must be identical");
				if (nOffsetChildren1 < 0 || nOffsetChildren2 < 0)
					DrxFatalError("ModelError: offset must be nonnegative");
				if (numChildren1 && nOffsetChildren1 == 0)
					DrxFatalError("ModelError: nOffsetChildren1 invalid");
				if (numChildren1 == 0 && nOffsetChildren1)
					DrxFatalError("ModelError: child offset not initialized");
			}
		}

		void parse(i32 idx, CDefaultSkeleton::SJoint* pModelJoints)
		{
			m_idxnode++;
			i32 s = m_arrHierarchy[idx].m_idxNext;
			i32 c = m_arrHierarchy[idx].m_idxFirst;
			if (s && c)
			{
				if (s >= c)
					DrxFatalError("ModelError: offset for siblings must be higher than first child");
			}
			if (s) parse(s, pModelJoints);
			if (c) parse(c, pModelJoints);
		}

		snode  m_arrHierarchy[MAX_JOINT_AMOUNT];
		u32 m_idxnode;
	} vh;

	//check that there are no duplicated CRC32s
	u32 numJoints = m_arrModelJoints.size();
	for (u32 i = 1; i < numJoints; i++)
	{
		u32 nCRC32low = m_arrModelJoints[i].m_nJointCRC32Lower;
		for (u32 j = 0; j < i; j++)
		{
			if (m_arrModelJoints[j].m_nJointCRC32Lower == nCRC32low)
				DrxFatalError("ModelError: duplicated CRC32 in SKEL");
		}
	}
	//check the integrity of the skeleton structure
	vh.rebuild(numJoints, &m_arrModelJoints[0]);
	vh.m_idxnode = 0;
	vh.parse(0, &m_arrModelJoints[0]);
	if (vh.m_idxnode != numJoints)
		DrxFatalError("ModelError:: node-count must be identical");
}

//////////////////////////////////////////////////////////////////////////

i32 GetMeshApproxFlags(tukk str, i32 len)
{
	i32 flags = 0;
	if (DrxStringUtils::strnstr(str, "box", len))
		flags |= mesh_approx_box;
	else if (DrxStringUtils::strnstr(str, "cylinder", len))
		flags |= mesh_approx_cylinder;
	else if (DrxStringUtils::strnstr(str, "capsule", len))
		flags |= mesh_approx_capsule;
	else if (DrxStringUtils::strnstr(str, "sphere", len))
		flags |= mesh_approx_sphere;
	return flags;
}
template<class T> void _swap(T& op1, T& op2) { T tmp = op1; op1 = op2; op2 = tmp; }

bool                   CDefaultSkeleton::SetupPhysicalProxies(const DynArray<PhysicalProxy>& arrPhyBoneMeshes, const DynArray<BONE_ENTITY>& arrBoneEntitiesSrc, IMaterial* pIMaterial, tukk filename)
{
	//set children
	m_bHasPhysics2 = false;
	u32 numBoneEntities = arrBoneEntitiesSrc.size();
	u32 numJoints = m_arrModelJoints.size();
	if (numBoneEntities > numJoints)
		DrxFatalError("numBoneEntities must <= numJoints");

	for (u32 j = 0; j < numJoints; j++)
		m_arrModelJoints[j].m_numLevels = 0, m_arrModelJoints[j].m_numChildren = 0, m_arrModelJoints[j].m_nOffsetChildren = 0;
	for (u32 j = 1; j < numJoints; j++)
	{
		i16 p = m_arrModelJoints[j].m_idxParent;
		m_arrModelJoints[p].m_numChildren++;
		if (m_arrModelJoints[p].m_nOffsetChildren == 0)
			m_arrModelJoints[p].m_nOffsetChildren = j - p;
	}
	//set deepness-level inside hierarchy
	for (u32 i = 0; i < numJoints; i++)
	{
		i32 parent = m_arrModelJoints[i].m_idxParent;
		while (parent >= 0)
		{
			m_arrModelJoints[i].m_numLevels++;
			parent = m_arrModelJoints[parent].m_idxParent;
		}
	}

	//return 1;

	BONE_ENTITY be;
	memset(&be, 0, sizeof(BONE_ENTITY));
	be.ParentID = -1;
	be.phys.nPhysGeom = -1;
	be.phys.flags = -1;
	DynArray<BONE_ENTITY> arrBoneEntitiesSorted;
	arrBoneEntitiesSorted.resize(numJoints);
	float dfltApproxTol = 0.2f;
	for (u32 id = 0; id < numJoints; id++)
	{
		arrBoneEntitiesSorted[id] = be;
		u32 nCRC32 = m_arrModelJoints[id].m_nJointCRC32;
		for (u32 e = 0; e < numBoneEntities; e++)
			if (arrBoneEntitiesSrc[e].ControllerID == nCRC32) { arrBoneEntitiesSorted[id] = arrBoneEntitiesSrc[e];  break; }
		arrBoneEntitiesSorted[id].BoneID = id;
		arrBoneEntitiesSorted[id].ParentID = m_arrModelJoints[id].m_idxParent;
		arrBoneEntitiesSorted[id].nChildren = m_arrModelJoints[id].m_numChildren;
		arrBoneEntitiesSorted[id].ControllerID = m_arrModelJoints[id].m_nJointCRC32;
		if (GetMeshApproxFlags(arrBoneEntitiesSorted[id].prop, strnlen(arrBoneEntitiesSorted[id].prop, sizeof(arrBoneEntitiesSorted[id].prop))))
			dfltApproxTol = 0.05f;
	}

	//loop over all BoneEntities and set the flags "joint_no_gravity" and "joint_isolated_accelerations" in m_PhysInfo.flags
	for (u32 i = 0; i < numBoneEntities; i++)
	{
		m_arrModelJoints[i].m_PhysInfo.flags &= ~(joint_no_gravity | joint_isolated_accelerations);
		if (arrBoneEntitiesSorted[i].prop[0] == 0)
		{
			m_arrModelJoints[i].m_PhysInfo.flags |= joint_no_gravity | joint_isolated_accelerations;
		}
		else
		{
			if (!DrxStringUtils::strnstr(arrBoneEntitiesSorted[i].prop, "gravity", sizeof(arrBoneEntitiesSorted[i].prop)))
				m_arrModelJoints[i].m_PhysInfo.flags |= joint_no_gravity;
			if (!DrxStringUtils::strnstr(arrBoneEntitiesSorted[i].prop, "active_phys", sizeof(arrBoneEntitiesSorted[i].prop)))
				m_arrModelJoints[i].m_PhysInfo.flags |= joint_isolated_accelerations;
			if (tukk ptr = DrxStringUtils::strnstr(arrBoneEntitiesSorted[i].prop, "mass", sizeof(arrBoneEntitiesSorted[i].prop)))
			{
				//DrxFatalError("did we ever use this path???");
				f32 mass = 0.0f;
				for (ptr += 4; *ptr && !(*ptr >= '0' && *ptr <= '9'); ptr++)
					;
				if (*ptr) mass = (float)atof(ptr);
				m_arrModelJoints[i].m_fMass = max(m_arrModelJoints[i].m_fMass, mass);
			}
		}
	}

	//link the proxies to the joints
	if (!g_pIPhysicalWorld)
	{
		return false;
	}

	IGeomUpr* pPhysicalGeometryUpr = g_pIPhysicalWorld->GetGeomUpr();
	if (!pPhysicalGeometryUpr)
	{
		return false;
	}

	if (!arrPhyBoneMeshes.empty() && !pIMaterial)
	{
		g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, filename, "Error loading skeleton: material definition is missing for physical proxies.");
		return false;
	}

	//link the proxies to the joints
	i32 useOnlyBoxes = 0;
	u32 nHasPhysicsGeom = 0;
	u32 numPBM = arrPhyBoneMeshes.size();
	for (u32 p = 0; p < numPBM; p++)
	{
		// the chunks from which the physical geometry is read are the bone mesh chunks.
		PhysicalProxy pbm = arrPhyBoneMeshes[p];
		u32 numFaces = pbm.m_arrMaterials.size();
		u32 flags = (numFaces <= 20 ? mesh_SingleBB : mesh_OBB | mesh_AABB | mesh_AABB_rotated) | mesh_multicontact0;
		u32k flagsAllPrim = mesh_approx_box | ((mesh_approx_sphere | mesh_approx_cylinder | mesh_approx_capsule) & (useOnlyBoxes - 1));

		// Assign custom material to physics.
		i32 defSurfaceIdx = pbm.m_arrMaterials.empty() ? 0 : pbm.m_arrMaterials[0];
		i32 surfaceTypesId[MAX_SUB_MATERIALS];
		memset(surfaceTypesId, 0, sizeof(surfaceTypesId));
		i32 numIds = pIMaterial->FillSurfaceTypeIds(surfaceTypesId);

		//After loading, pPhysGeom is set to the value equal to the ChunkID in the file where the physical geometry (BoneMesh) chunk is kept.
		//To initialize a bone with a proxy, we loop over all joints and replace the ChunkID in pPhysGeom with the actual physical geometry object pointers.
		for (u32 i = 0; i < numJoints; ++i)
		{
			INT_PTR cid = INT_PTR(m_arrModelJoints[i].m_PhysInfo.pPhysGeom);
			if (pbm.ChunkID != cid)
				continue;

			u32 flagsPrim = GetMeshApproxFlags(arrBoneEntitiesSorted[i].prop, strnlen(arrBoneEntitiesSorted[i].prop, sizeof(arrBoneEntitiesSorted[i].prop)));
			IGeometry* pPhysicalGeometry = pPhysicalGeometryUpr->CreateMesh(&pbm.m_arrPoints[0], &pbm.m_arrIndices[0], &pbm.m_arrMaterials[0], 0, numFaces, 
				flags | (flagsPrim ? flagsPrim : flagsAllPrim), ((useOnlyBoxes|flagsPrim) ? 1.5f : dfltApproxTol));
			if (pPhysicalGeometry == 0)
			{
				g_pISystem->Warning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, filename, "Physics: Failed to create phys mesh");
				assert(pPhysicalGeometry);
				return false;
			}
			phys_geometry* pg = pPhysicalGeometryUpr->RegisterGeometry(pPhysicalGeometry, defSurfaceIdx, &surfaceTypesId[0], numIds);

			i32 id = pg->pGeom->GetPrimitiveId(0, 0);
			if (id < 0)
				id = pg->surface_idx;
			if (u32(id) < u32(pg->nMats))
				id = pg->pMatMapping[id];
			*(i32*)(m_arrModelJoints[i].m_PhysInfo.spring_angle + 1) = id; //surface type index for rope physicalization (it's not ready at rc stage)
			if (strnicmp(m_arrModelJoints[i].m_strJointName.c_str(), "rope", 4))
				m_arrModelJoints[i].m_PhysInfo.pPhysGeom = pg, nHasPhysicsGeom++;
			else
				g_pIPhysicalWorld->GetGeomUpr()->UnregisterGeometry(pg), m_arrModelJoints[i].m_PhysInfo.flags = -1;
			pPhysicalGeometry->Release();
		}
	}

	for (u32 i = 0; i < numJoints; ++i)
	{
		INT_PTR cid = INT_PTR(m_arrModelJoints[i].m_PhysInfo.pPhysGeom);
		if (cid >= -1 && cid < 0x400)
			m_arrModelJoints[i].m_PhysInfo.pPhysGeom = 0;

		for (i32 j = 0; m_arrModelJoints[i].m_PhysInfo.flags != -1 && m_arrModelJoints[i].m_PhysInfo.pPhysGeom && j < 3; j++)
		{
			// lock axes with 0 limits range
			m_arrModelJoints[i].m_PhysInfo.flags |= (m_arrModelJoints[i].m_PhysInfo.max[j] - m_arrModelJoints[i].m_PhysInfo.min[j] <= 0.0f) * angle0_locked << j;
		}
	}

	if (nHasPhysicsGeom)
	{
		mesh_data* pmesh;
		IGeometry* pMeshGeom;
		tukk pcloth;
		IGeomUpr* pGeoman = g_pIPhysicalWorld->GetGeomUpr();
		for (i32 i = arrBoneEntitiesSorted.size() - 1; i >= 0; i--)
		{
			if (arrBoneEntitiesSorted[i].prop[0] == 0)
				continue;
			CDefaultSkeleton::SJoint* pBone = &m_arrModelJoints[i];
			if (pBone->m_PhysInfo.pPhysGeom == 0)
				continue;
			u32 type = pBone->m_PhysInfo.pPhysGeom->pGeom->GetType();
			if (type == GEOM_TRIMESH)
			{
				pmesh = (mesh_data*)(pMeshGeom = pBone->m_PhysInfo.pPhysGeom->pGeom)->GetData();
				if (pmesh->nIslands == 2 && (pcloth = DrxStringUtils::strnstr(arrBoneEntitiesSorted[i].prop, "cloth_proxy", sizeof(arrBoneEntitiesSorted[i].prop))))
				{
					//DrxFatalError("cloth proxy found");
					i32 itri, j, isle = isneg(pmesh->pIslands[1].V - pmesh->pIslands[0].V);
					for (itri = pmesh->pIslands[isle].itri, j = 0; j < pmesh->pIslands[isle].nTris; itri = pmesh->pTri2Island[isle].inext, j++)
					{
						for (i32 ivtx = 0; ivtx < 3; ivtx++)
							_swap(pmesh->pIndices[itri * 3 + ivtx], pmesh->pIndices[j * 3 + 2 - ivtx]);
						_swap(pmesh->pMats[itri], pmesh->pMats[j]);
					}

					phys_geometry* pgeomMain, * pgeomCloth;
					i32 flags = GetMeshApproxFlags(arrBoneEntitiesSorted[i].prop, static_cast<i32>(pcloth - arrBoneEntitiesSorted[i].prop));
					flags |= (flags || pmesh->pIslands[isle ^ 1].nTris < 20) ? mesh_SingleBB : mesh_OBB;
					pMeshGeom = pGeoman->CreateMesh(pmesh->pVertices, pmesh->pIndices + j, pmesh->pMats + j, 0, pmesh->pIslands[isle ^ 1].nTris, flags, 1.0f);
					pgeomMain = pGeoman->RegisterGeometry(pMeshGeom, pBone->m_PhysInfo.pPhysGeom->surface_idx, pBone->m_PhysInfo.pPhysGeom->pMatMapping, pBone->m_PhysInfo.pPhysGeom->nMats);

					flags = GetMeshApproxFlags(pcloth, sizeof(arrBoneEntitiesSorted[i].prop) - (pcloth - arrBoneEntitiesSorted[i].prop));
					flags |= (flags || pmesh->pIslands[isle].nTris < 20) ? mesh_SingleBB : mesh_OBB;
					pMeshGeom = pGeoman->CreateMesh(pmesh->pVertices, pmesh->pIndices, pmesh->pMats, 0, j, flags, 1.0f);
					pgeomCloth = pGeoman->RegisterGeometry(pMeshGeom, pBone->m_PhysInfo.pPhysGeom->surface_idx, pBone->m_PhysInfo.pPhysGeom->pMatMapping, pBone->m_PhysInfo.pPhysGeom->nMats);
					pgeomMain->pForeignData = pgeomCloth;

					pGeoman->UnregisterGeometry(pBone->m_PhysInfo.pPhysGeom);
					pBone->m_PhysInfo.pPhysGeom = pgeomMain;
					continue;
				}

				i32 flags = GetMeshApproxFlags(arrBoneEntitiesSorted[i].prop, sizeof(arrBoneEntitiesSorted[i].prop));
				if (!flags)
					continue;

				pBone->m_PhysInfo.pPhysGeom->pGeom = pGeoman->CreateMesh(pmesh->pVertices, pmesh->pIndices, pmesh->pMats, 0, pmesh->nTris, flags | mesh_SingleBB, 1.0f);
				pMeshGeom->Release();
			}
		}
		m_bHasPhysics2 = true;
	}

	IStatObj *pSkelCGF = gEnv->p3DEngine->LoadStatObj(string(filename) + ".cgf", nullptr, nullptr, false, IStatObj::ELoadingFlagsNoErrorIfFail);
	if (pSkelCGF && !pSkelCGF->IsDefaultObject())
	{
		std::map<u32,i32> mapJoints;
		for(u32 i = 0; i < numJoints; i++)
		{
			mapJoints.insert(std::pair<u32,i32>(CCrc32::ComputeLowercase(m_arrModelJoints[i].m_strJointName), i));
			DrxBonePhysics& phys = m_arrModelJoints[i].m_PhysInfo;
			if (phys.pPhysGeom)
			{
				gEnv->pPhysicalWorld->GetGeomUpr()->UnregisterGeometry(phys.pPhysGeom);
				phys.pPhysGeom = nullptr;
			}
		}
		m_bHasPhysics2 = true;
		for(i32 i = 0; i < pSkelCGF->GetSubObjectCount(); i++)
		{
			IStatObj::SSubObject& slot = *pSkelCGF->GetSubObject(i);
			auto idx = mapJoints.find(CCrc32::ComputeLowercase(slot.name));
			if (idx != mapJoints.end() && !slot.name.compareNoCase(m_arrModelJoints[idx->second].m_strJointName) && slot.pStatObj && slot.pStatObj->GetPhysGeom())
			{
				DrxBonePhysics& phys = m_arrModelJoints[idx->second].m_PhysInfo;
				(phys.pPhysGeom = slot.pStatObj->GetPhysGeom())->nRefCount++;
				Vec3i lim[2];
				Ang3 frame0;
				sscanf_s(slot.properties.c_str(), "%d %d %d %d %d %d %f %f %f %f %f %f %f %f %f",
					&lim[0].x, &lim[0].y, &lim[0].z, &lim[1].x, &lim[1].y, &lim[1].z, 
					&phys.spring_tension[0], &phys.spring_tension[1], &phys.spring_tension[2], 
					&phys.damping[0], &phys.damping[1], &phys.damping[2],
					&frame0.x, &frame0.y, &frame0.z);
				*(Vec3*)phys.min = DEG2RAD(Vec3(lim[0]));
				*(Vec3*)phys.max = DEG2RAD(Vec3(lim[1]));
				phys.flags = joint_no_gravity | joint_isolated_accelerations;
				for(i32 j = 0; j < 3; j++)
				{
					phys.flags |= (angle0_locked << j) * isneg(phys.max[j] - phys.min[j] - 0.01f);
					float unlim = 1.0f + isneg(gf_PI*1.999f - phys.max[j] + phys.min[j]);
					phys.max[j] *= unlim; phys.min[j] *= unlim;
				}
				*(Matrix33*)phys.framemtx = Matrix33(DEG2RAD(frame0));
				m_bHasPhysics2 = true;
			}
		}
		pSkelCGF->Release();

		// transform framemtx from child frame to phys parent frame
		for(u32 i = 0; i < numJoints; i++)
			if (m_arrModelJoints[i].m_PhysInfo.pPhysGeom)
			{
				i32 idxParent = m_arrModelJoints[i].m_idxParent;
				while (idxParent >= 0 && !m_arrModelJoints[idxParent].m_PhysInfo.pPhysGeom)
					idxParent = m_arrModelJoints[idxParent].m_idxParent;
				if (idxParent >= 0)
					*(Matrix33*)m_arrModelJoints[i].m_PhysInfo.framemtx = Matrix33(!GetDefaultAbsJointByID(idxParent).q * GetDefaultAbsJointByID(i).q) * *(Matrix33*)m_arrModelJoints[i].m_PhysInfo.framemtx;
			}
	}

	return true;
}

bool CDefaultSkeleton::ParsePhysInfoProperties_ROPE(DrxBonePhysics& pi, const DynArray<SJointProperty>& props)
{
	u32 numProps = props.size();
	if (numProps == 0)
		return 0;
	if (props[0].type < 2) //the first meber must be a tukk 
		return 0;

	if (!strcmp(props[0].strval, "Rope"))
	{
		*(alias_cast<i32*>(pi.spring_angle)) = 0x12345678;

		pi.flags &= ~joint_isolated_accelerations;
		for (u32 i = 0; i < numProps; i++)
		{
			if (!strcmp(props[i].name, "Gravity"))
				pi.framemtx[1][0] = props[i].fval == 1.0f ? 9.81f : props[i].fval;
			if (!strcmp(props[i].name, "JointLimit"))
				pi.min[0] = -DEG2RAD(props[i].fval), pi.spring_tension[0] = 0;
			if (!strcmp(props[i].name, "JointLimitIncrease"))
				pi.spring_angle[2] = props[i].fval;
			if (!strcmp(props[i].name, "MaxTimestep"))
				pi.spring_tension[1] = props[i].fval;
			if (!strcmp(props[i].name, "Stiffness"))
				pi.max[0] = DEG2RAD(props[i].fval);
			if (!strcmp(props[i].name, "StiffnessDecay"))
				pi.max[1] = DEG2RAD(props[i].fval);
			if (!strcmp(props[i].name, "Damping"))
				pi.max[2] = DEG2RAD(props[i].fval);
			if (!strcmp(props[i].name, "Friction"))
				pi.spring_tension[2] = props[i].fval;
			if (!strcmp(props[i].name, "SimpleBlending"))
				(pi.flags &= ~4) |= (props[i].bval ? 0 : 4);
			if (!strcmp(props[i].name, "Mass"))
				pi.min[1] = DEG2RAD(-props[i].fval);
			if (!strcmp(props[i].name, "Thickness"))
				pi.min[2] = DEG2RAD(-props[i].fval);
			if (!strcmp(props[i].name, "StiffnessControlBone"))
				pi.framemtx[0][1] = props[i].fval + (props[i].fval > 0.0f);
			if (!strcmp(props[i].name, "SleepSpeed"))
				pi.damping[0] = props[i].fval + 1.0f;
			else if (!strcmp(props[i].name, "HingeY"))
				(pi.flags &= ~8) |= (props[i].bval ? 8 : 0);
			else if (!strcmp(props[i].name, "HingeZ"))
				(pi.flags &= ~16) |= (props[i].bval ? 16 : 0);

			if (!strcmp(props[i].name, "EnvCollisions"))
				(pi.flags &= ~1) |= (props[i].bval ? 0 : 1);
			if (!strcmp(props[i].name, "BodyCollisions"))
				(pi.flags &= ~2) |= (props[i].bval ? 0 : 2);
		}
		return true;
	}

	if (!strcmp(props[0].strval, "Cloth"))
	{
		*(alias_cast<i32*>(pi.spring_angle)) = 0x12345678;

		pi.flags |= joint_isolated_accelerations;
		for (u32 i = 0; i < numProps; i++)
		{
			if (!strcmp(props[i].name, "MaxTimestep"))
				pi.damping[0] = props[i].fval;
			if (!strcmp(props[i].name, "MaxStretch"))
				pi.damping[1] = props[i].fval;
			if (!strcmp(props[i].name, "Stiffness"))
				pi.max[2] = DEG2RAD(props[i].fval);
			if (!strcmp(props[i].name, "Thickness"))
				pi.damping[2] = props[i].fval;
			if (!strcmp(props[i].name, "Friction"))
				pi.spring_tension[2] = props[i].fval;
			if (!strcmp(props[i].name, "StiffnessNorm"))
				pi.max[0] = DEG2RAD(props[i].fval);
			if (!strcmp(props[i].name, "StiffnessTang"))
				pi.max[1] = DEG2RAD(props[i].fval);
			if (!strcmp(props[i].name, "Damping"))
				pi.spring_tension[0] = props[i].fval;
			if (!strcmp(props[i].name, "AirResistance"))
				pi.spring_tension[1] = props[i].fval;
			if (!strcmp(props[i].name, "StiffnessAnim"))
				pi.min[0] = props[i].fval;
			if (!strcmp(props[i].name, "StiffnessDecayAnim"))
				pi.min[1] = props[i].fval;
			if (!strcmp(props[i].name, "DampingAnim"))
				pi.min[2] = props[i].fval;
			else if (!strcmp(props[i].name, "MaxIters"))
				pi.framemtx[0][2] = props[i].fval;
			else if (!strcmp(props[i].name, "MaxDistAnim"))
				pi.spring_angle[2] = props[i].fval;
			else if (!strcmp(props[i].name, "CharacterSpace"))
				pi.framemtx[0][1] = props[i].fval;

			if (!strcmp(props[i].name, "EnvCollisions"))
				(pi.flags &= ~1) |= (props[i].bval ? 0 : 1);
			if (!strcmp(props[i].name, "BodyCollisions"))
				(pi.flags &= ~2) |= (props[i].bval ? 0 : 2);
		}
		return true;
	}

	return false;
}

DynArray<SJointProperty> CDefaultSkeleton::GetPhysInfoProperties_ROPE(const DrxBonePhysics& pi, i32 nRopeOrGrid)
{
	DynArray<SJointProperty> res;
	if (nRopeOrGrid == 0)
	{
		res.push_back(SJointProperty("Type", "Rope"));
		res.push_back(SJointProperty("Gravity", pi.framemtx[1][0]));

		float jl = pi.spring_tension[0];
		if (pi.min[0] != 0) jl = RAD2DEG(fabs_tpl(pi.min[0]));
		res.push_back(SJointProperty("JointLimit", jl));
		res.push_back(SJointProperty("JointLimitIncrease", pi.spring_angle[2]));

		float jli = pi.spring_tension[1];
		if (jli <= 0 || jli >= 1) jli = 0.02f;
		res.push_back(SJointProperty("MaxTimestep", jli));
		res.push_back(SJointProperty("Stiffness", max(0.001f, RAD2DEG(pi.max[0]))));
		res.push_back(SJointProperty("StiffnessDecay", RAD2DEG(pi.max[1])));
		res.push_back(SJointProperty("Damping", RAD2DEG(pi.max[2])));
		res.push_back(SJointProperty("Friction", pi.spring_tension[2]));
		res.push_back(SJointProperty("SimpleBlending", !(pi.flags & 4)));
		res.push_back(SJointProperty("Mass", RAD2DEG(fabs_tpl(pi.min[1]))));
		res.push_back(SJointProperty("Thickness", RAD2DEG(fabs_tpl(pi.min[2]))));
		res.push_back(SJointProperty("SleepSpeed", pi.damping[0] - 1.0f));
		res.push_back(SJointProperty("HingeY", (pi.flags & 8) != 0));
		res.push_back(SJointProperty("HingeZ", (pi.flags & 16) != 0));
		res.push_back(SJointProperty("StiffnessControlBone", (float)FtoI(pi.framemtx[0][1] - 1.0f) * (pi.framemtx[0][1] >= 2.0f && pi.framemtx[0][1] < 100.0f)));
		res.push_back(SJointProperty("EnvCollisions", !(pi.flags & 1)));
		res.push_back(SJointProperty("BodyCollisions", !(pi.flags & 2)));
	}

	if (nRopeOrGrid > 0)
	{
		res.push_back(SJointProperty("Type", "Cloth"));
		res.push_back(SJointProperty("MaxTimestep", pi.damping[0]));
		res.push_back(SJointProperty("MaxStretch", pi.damping[1]));
		res.push_back(SJointProperty("Stiffness", RAD2DEG(pi.max[2])));
		res.push_back(SJointProperty("Thickness", pi.damping[2]));
		res.push_back(SJointProperty("Friction", pi.spring_tension[2]));
		res.push_back(SJointProperty("StiffnessNorm", RAD2DEG(pi.max[0])));
		res.push_back(SJointProperty("StiffnessTang", RAD2DEG(pi.max[1])));
		res.push_back(SJointProperty("Damping", pi.spring_tension[0]));
		res.push_back(SJointProperty("AirResistance", pi.spring_tension[1]));
		res.push_back(SJointProperty("StiffnessAnim", pi.min[0]));
		res.push_back(SJointProperty("StiffnessDecayAnim", pi.min[1]));
		res.push_back(SJointProperty("DampingAnim", pi.min[2]));
		res.push_back(SJointProperty("MaxIters", (float)FtoI(pi.framemtx[0][2])));
		res.push_back(SJointProperty("MaxDistAnim", pi.spring_angle[2]));
		res.push_back(SJointProperty("CharacterSpace", pi.framemtx[0][1]));

		res.push_back(SJointProperty("EnvCollisions", !(pi.flags & 1)));
		res.push_back(SJointProperty("BodyCollisions", !(pi.flags & 2)));
	}
	return res;
}

// TODO: All the hard codednames need to be refactored away.
void CDefaultSkeleton::InitializeHardcodedJointsProperty()
{
	u32 numJoints = m_arrModelJoints.size();
	for (u32 i = 0; i < numJoints; i++)
	{
		tukk BoneName = m_arrModelJoints[i].m_strJointName.c_str();
		// NOTE: Needed by CSkeletonPhysics to build the Articulated Entity
		// self-collision properties.
		if (strstr(BoneName, "Forearm"))
			m_arrModelJoints[i].m_flags |= eJointFlag_NameHasForearm;
		if (strstr(BoneName, "Calf"))
			m_arrModelJoints[i].m_flags |= eJointFlag_NameHasCalf;
		if (strstr(BoneName, "Pelvis") || strstr(BoneName, "Head") || strstr(BoneName, "Spine") || strstr(BoneName, "Thigh"))
			m_arrModelJoints[i].m_flags |= eJointFlag_NameHasPelvisOrHeadOrSpineOrThigh;
	}
}

//-----------------------------------------------------------------------------
i32 CDefaultSkeleton::RemapIdx(const CDefaultSkeleton* pCDefaultSkeletonSrc, i32k idx)
{
#if !defined(_RELEASE)
	if (idx < 0) DrxFatalError("can't remap negative index");
#endif
	i32 nidx = GetJointIDByCRC32(pCDefaultSkeletonSrc->m_arrModelJoints[idx].m_nJointCRC32Lower);
#if !defined(_RELEASE)
	if (nidx < 0) DrxFatalError("index remapping failed");
#endif
	return nidx;
}

bool FindJointInParentHierarchy(const CDefaultSkeleton* const pDefaultSkeleton, i32k jointIdToSearch, i16k jointIdToStartSearchFrom)
{
	DRX_ASSERT(pDefaultSkeleton);
	DRX_ASSERT(0 <= jointIdToSearch);
	DRX_ASSERT(0 <= jointIdToStartSearchFrom);

	i32 currentJoint = jointIdToStartSearchFrom;
	while (currentJoint != -1)
	{
		if (currentJoint == jointIdToSearch)
		{
			return true;
		}
		currentJoint = pDefaultSkeleton->GetJointParentIDByID(currentJoint);
	}
	return false;
}

void CDefaultSkeleton::CopyAndAdjustSkeletonParams(const CDefaultSkeleton* pCDefaultSkeletonSrc)
{
	m_pAnimationSet = pCDefaultSkeletonSrc->m_pAnimationSet;
	m_animListIDs = pCDefaultSkeletonSrc->m_animListIDs;
	//m_ModelMesh          = pCDefaultSkeletonSrc->m_ModelMesh;		//just forget the Modelmesh

	m_ModelAABB = pCDefaultSkeletonSrc->m_ModelAABB;
	m_AABBExtension = pCDefaultSkeletonSrc->m_AABBExtension;

	for (u32 i = 0; i < MAX_FEET_AMOUNT; i++)
		m_strFeetLockIKHandle[i] = pCDefaultSkeletonSrc->m_strFeetLockIKHandle[i];

	//remap animation lod
	m_arrAnimationLOD = pCDefaultSkeletonSrc->m_arrAnimationLOD;

	//remap facial model
	if (pCDefaultSkeletonSrc->m_pFacialModel)
	{
		m_pFacialModel = pCDefaultSkeletonSrc->m_pFacialModel;
		m_pFacialModel->AddRef();
	}

	//remap include list
	m_BBoxIncludeList = pCDefaultSkeletonSrc->m_BBoxIncludeList;
	u32 numIncludeList = m_BBoxIncludeList.size();
	for (u32 i = 0; i < numIncludeList; i++)
		m_BBoxIncludeList[i] = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_BBoxIncludeList[i]);

	m_usePhysProxyBBox = pCDefaultSkeletonSrc->m_usePhysProxyBBox;

	//remap capsule shadow list
	m_ShadowCapsulesList = pCDefaultSkeletonSrc->m_ShadowCapsulesList;
	u32 numShadowCapsulesList = m_ShadowCapsulesList.size();
	for (u32 i = 0; i < numShadowCapsulesList; i++)
		for (u32 n = 0; n < 2; n++)
			m_ShadowCapsulesList[i].arrJoints[n] = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_ShadowCapsulesList[i].arrJoints[n]);

	//remap limb IK
	m_IKLimbTypes = pCDefaultSkeletonSrc->m_IKLimbTypes;
	u32 numLimbTypes = m_IKLimbTypes.size();
	for (u32 i = 0; i < numLimbTypes; i++)
	{
		u32 numJointChain = pCDefaultSkeletonSrc->m_IKLimbTypes[i].m_arrJointChain.size();
		for (u32 j = 0; j < numJointChain; j++)
			m_IKLimbTypes[i].m_arrJointChain[j].m_idxJoint = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_IKLimbTypes[i].m_arrJointChain[j].m_idxJoint);
		u32 numLimbChildren = pCDefaultSkeletonSrc->m_IKLimbTypes[i].m_arrLimbChildren.size();
		for (u32 j = 0; j < numLimbChildren; j++)
			m_IKLimbTypes[i].m_arrLimbChildren[j] = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_IKLimbTypes[i].m_arrLimbChildren[j]);
		u32 numRootToEndEffector = pCDefaultSkeletonSrc->m_IKLimbTypes[i].m_arrRootToEndEffector.size();
		for (u32 j = 0; j < numRootToEndEffector; j++)
			m_IKLimbTypes[i].m_arrRootToEndEffector[j] = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_IKLimbTypes[i].m_arrRootToEndEffector[j]);
	}

	//remap ADIK
	m_ADIKTargets = pCDefaultSkeletonSrc->m_ADIKTargets;
	u32 numADIKTargets = pCDefaultSkeletonSrc->m_ADIKTargets.size();
	for (u32 i = 0; i < numADIKTargets; i++)
	{
		m_ADIKTargets[i].m_idxWeight = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_ADIKTargets[i].m_idxWeight);
		m_ADIKTargets[i].m_idxTarget = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_ADIKTargets[i].m_idxTarget);
	}

	//adjust recoil
	m_recoilDesc = pCDefaultSkeletonSrc->m_recoilDesc;
	if (pCDefaultSkeletonSrc->m_recoilDesc.m_weaponRightJointIndex > 0)
		m_recoilDesc.m_weaponRightJointIndex = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_recoilDesc.m_weaponRightJointIndex);
	if (pCDefaultSkeletonSrc->m_recoilDesc.m_weaponLeftJointIndex > 0)
		m_recoilDesc.m_weaponLeftJointIndex = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_recoilDesc.m_weaponLeftJointIndex);
	u32 numRecoilJoints = m_recoilDesc.m_joints.size();
	for (u32 i = 0; i < numRecoilJoints; i++)
		m_recoilDesc.m_joints[i].m_nIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_recoilDesc.m_joints[i].m_nIdx);

	//adjust look-ik
	m_poseBlenderLookDesc = pCDefaultSkeletonSrc->m_poseBlenderLookDesc;
	u32 numBlendsLook = m_poseBlenderLookDesc.m_blends.size();
	for (u32 i = 0; i < numBlendsLook; i++)
	{
		m_poseBlenderLookDesc.m_blends[i].m_nParaJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderLookDesc.m_blends[i].m_nParaJointIdx);
		m_poseBlenderLookDesc.m_blends[i].m_nStartJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderLookDesc.m_blends[i].m_nStartJointIdx);
		m_poseBlenderLookDesc.m_blends[i].m_nReferenceJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderLookDesc.m_blends[i].m_nReferenceJointIdx);
		m_poseBlenderLookDesc.m_blends[i].m_nRotParaJointIdx = -1;
		m_poseBlenderLookDesc.m_blends[i].m_nRotStartJointIdx = -1;
	}

	u32 numRotationsLook = m_poseBlenderLookDesc.m_rotations.size();
	for (u32 i = 0; i < numRotationsLook; i++)
	{
		m_poseBlenderLookDesc.m_rotations[i].m_nPosIndex = -1;
		i32 jidx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderLookDesc.m_rotations[i].m_nJointIdx);
		m_poseBlenderLookDesc.m_rotations[i].m_nJointIdx = jidx;
		i32k parentJointIndex = GetJointParentIDByID(m_poseBlenderLookDesc.m_rotations[i].m_nJointIdx);
		for (u32 p = 0; p < numRotationsLook; ++p)
		{
			const SJointsAimIK_Rot& otherAimIkRot = m_poseBlenderLookDesc.m_rotations[p];
			if (m_poseBlenderLookDesc.m_rotations[i].m_nPreEvaluate && otherAimIkRot.m_nPreEvaluate)
			{
				const bool isCurrentJointParentOfPreviousJoint = FindJointInParentHierarchy(this, jidx, otherAimIkRot.m_nJointIdx);
				if (isCurrentJointParentOfPreviousJoint)
					m_poseBlenderLookDesc.m_error++;
			}
			if (otherAimIkRot.m_nJointIdx == parentJointIndex)
			{
				m_poseBlenderLookDesc.m_rotations[i].m_nRotJointParentIdx = p;
				break;
			}
		}
	}
	u32 numPositionLook = m_poseBlenderLookDesc.m_positions.size();
	for (u32 i = 0; i < numPositionLook; i++)
		m_poseBlenderLookDesc.m_positions[i].m_nJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderLookDesc.m_positions[i].m_nJointIdx);
	for (u32 i = 0; i < numBlendsLook; ++i)
	{
		for (u32 r = 0; r < numRotationsLook; ++r)
		{
			i32k jointIndex = m_poseBlenderLookDesc.m_rotations[r].m_nJointIdx;
			if (jointIndex == m_poseBlenderLookDesc.m_blends[i].m_nParaJointIdx)
				m_poseBlenderLookDesc.m_blends[i].m_nRotParaJointIdx = r;
			if (jointIndex == m_poseBlenderLookDesc.m_blends[i].m_nStartJointIdx)
				m_poseBlenderLookDesc.m_blends[i].m_nRotStartJointIdx = r;
		}
	}
	u32 numRotLook = m_poseBlenderLookDesc.m_rotations.size();
	u32 numPosLook = m_poseBlenderLookDesc.m_positions.size();
	for (u32 r = 0; r < numRotLook; r++)
	{
		tukk pRotName = m_poseBlenderLookDesc.m_rotations[r].m_strJointName;
		if (pRotName == 0)
			continue;
		for (u32 p = 0; p < numPosLook; p++)
		{
			tukk pPosName = m_poseBlenderLookDesc.m_positions[p].m_strJointName;
			if (pPosName == 0)
				continue;
			u32 SameJoint = strcmp(pRotName, pPosName) == 0;
			if (SameJoint)
			{
				m_poseBlenderLookDesc.m_rotations[r].m_nPosIndex = p;
				break;
			}
		}
	}

	//adjust aim-ik
	m_poseBlenderAimDesc = pCDefaultSkeletonSrc->m_poseBlenderAimDesc;
	u32 numBlends = m_poseBlenderAimDesc.m_blends.size();
	for (u32 i = 0; i < numBlends; i++)
	{
		m_poseBlenderAimDesc.m_blends[i].m_nParaJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderAimDesc.m_blends[i].m_nParaJointIdx);
		m_poseBlenderAimDesc.m_blends[i].m_nStartJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderAimDesc.m_blends[i].m_nStartJointIdx);
		m_poseBlenderAimDesc.m_blends[i].m_nReferenceJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderAimDesc.m_blends[i].m_nReferenceJointIdx);
		m_poseBlenderAimDesc.m_blends[i].m_nRotParaJointIdx = -1;
		m_poseBlenderAimDesc.m_blends[i].m_nRotStartJointIdx = -1;
	}
	u32 numRotations = m_poseBlenderAimDesc.m_rotations.size();
	for (u32 i = 0; i < numRotations; i++)
	{
		m_poseBlenderAimDesc.m_rotations[i].m_nPosIndex = -1;
		i32 jidx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderAimDesc.m_rotations[i].m_nJointIdx);
		m_poseBlenderAimDesc.m_rotations[i].m_nJointIdx = jidx;
		i32k parentJointIndex = GetJointParentIDByID(m_poseBlenderAimDesc.m_rotations[i].m_nJointIdx);
		for (u32 p = 0; p < numRotations; ++p)
		{
			const SJointsAimIK_Rot& otherAimIkRot = m_poseBlenderAimDesc.m_rotations[p];
			if (m_poseBlenderAimDesc.m_rotations[i].m_nPreEvaluate && otherAimIkRot.m_nPreEvaluate)
			{
				const bool isCurrentJointParentOfPreviousJoint = FindJointInParentHierarchy(this, jidx, otherAimIkRot.m_nJointIdx);
				if (isCurrentJointParentOfPreviousJoint)
					m_poseBlenderAimDesc.m_error++;
			}
			if (otherAimIkRot.m_nJointIdx == parentJointIndex)
			{
				m_poseBlenderAimDesc.m_rotations[i].m_nRotJointParentIdx = p;
				break;
			}
		}
	}
	u32 numPosition = m_poseBlenderAimDesc.m_positions.size();
	for (u32 i = 0; i < numPosition; i++)
		m_poseBlenderAimDesc.m_positions[i].m_nJointIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderAimDesc.m_positions[i].m_nJointIdx);
	u32 numProcAdjust = m_poseBlenderAimDesc.m_procAdjustments.size();
	for (u32 i = 0; i < numProcAdjust; i++)
		m_poseBlenderAimDesc.m_procAdjustments[i].m_nIdx = RemapIdx(pCDefaultSkeletonSrc, pCDefaultSkeletonSrc->m_poseBlenderAimDesc.m_procAdjustments[i].m_nIdx);
	for (u32 i = 0; i < numBlends; ++i)
	{
		for (u32 r = 0; r < numRotations; ++r)
		{
			i32k jointIndex = m_poseBlenderAimDesc.m_rotations[r].m_nJointIdx;
			if (jointIndex == m_poseBlenderAimDesc.m_blends[i].m_nParaJointIdx)
				m_poseBlenderAimDesc.m_blends[i].m_nRotParaJointIdx = r;
			if (jointIndex == m_poseBlenderAimDesc.m_blends[i].m_nStartJointIdx)
				m_poseBlenderAimDesc.m_blends[i].m_nRotStartJointIdx = r;
		}
	}
	u32 numRot = m_poseBlenderAimDesc.m_rotations.size();
	u32 numPos = m_poseBlenderAimDesc.m_positions.size();
	for (u32 r = 0; r < numRot; r++)
	{
		tukk pRotName = m_poseBlenderAimDesc.m_rotations[r].m_strJointName;
		if (pRotName == 0)
			continue;
		for (u32 p = 0; p < numPos; p++)
		{
			tukk pPosName = m_poseBlenderAimDesc.m_positions[p].m_strJointName;
			if (pPosName == 0)
				continue;
			u32 SameJoint = strcmp(pRotName, pPosName) == 0;
			if (SameJoint)
			{
				m_poseBlenderAimDesc.m_rotations[r].m_nPosIndex = p;
				break;
			}
		}
	}
}

std::pair<u32k*, u32k*> CDefaultSkeleton::FindClosestAnimationLod(i32k lodValue) const
{
	const auto maxLod = m_arrAnimationLOD.size();
	if (lodValue > 0 && maxLod > 0)
	{
		const auto lodIndex = std::min<i32>(lodValue, maxLod) - 1;
		return{ &m_arrAnimationLOD[lodIndex][0], &m_arrAnimationLOD[lodIndex][0] + m_arrAnimationLOD[lodIndex].size() };
	}
	else
	{
		return{ nullptr, nullptr };
	}
}

u32 CDefaultSkeleton::SizeOfDefaultSkeleton() const
{
	u32 nSize = sizeof(CDefaultSkeleton);

	nSize += m_ModelMesh.SizeOfModelMesh();
	nSize += m_arrAnimationLOD.get_alloc_size();
	u32 numAnimLOD = m_arrAnimationLOD.size();
	for (u32 i = 0; i < numAnimLOD; i++)
		nSize += m_arrAnimationLOD[i].get_alloc_size();
	if (m_pFacialModel)
		nSize += m_pFacialModel->SizeOfThis();

	nSize += m_poseDefaultData.GetAllocationLength();
	nSize += m_arrModelJoints.get_alloc_size();
	u32 numModelJoints = m_arrModelJoints.size();
	for (u32 i = 0; i < numModelJoints; i++)
		nSize += m_arrModelJoints[i].m_strJointName.capacity();

	for (u32 i = 0; i < MAX_FEET_AMOUNT; i++)
		nSize += m_strFeetLockIKHandle[i].capacity();

	nSize += m_recoilDesc.m_joints.get_alloc_size();
	nSize += m_recoilDesc.m_ikHandleLeft.capacity();
	nSize += m_recoilDesc.m_ikHandleRight.capacity();

	nSize += m_poseBlenderLookDesc.m_eyeAttachmentLeftName.capacity();  //left eyeball attachment
	nSize += m_poseBlenderLookDesc.m_eyeAttachmentRightName.capacity(); //right eyeball attachment
	nSize += m_poseBlenderLookDesc.m_blends.get_alloc_size();           //parameters for aiming
	nSize += m_poseBlenderLookDesc.m_rotations.get_alloc_size();        //rotational joints used for Look-IK
	nSize += m_poseBlenderLookDesc.m_positions.get_alloc_size();        //positional joints used for Look-IK

	nSize += m_poseBlenderAimDesc.m_blends.get_alloc_size();      //parameters for aiming
	nSize += m_poseBlenderAimDesc.m_rotations.get_alloc_size();   //rotational joints used for Aim-IK
	nSize += m_poseBlenderAimDesc.m_positions.get_alloc_size();   //positional joints used for Aim-IK

	nSize += m_ADIKTargets.get_alloc_size();      //array with Animation Driven IK-Targets

	nSize += m_IKLimbTypes.get_alloc_size();      //array with limbs we can apply a two-bone solver
	u32 numLimb = m_IKLimbTypes.size();
	for (u32 i = 0; i < numLimb; i++)
	{
		nSize += m_IKLimbTypes[i].m_arrJointChain.get_alloc_size();
		nSize += m_IKLimbTypes[i].m_arrLimbChildren.get_alloc_size();
	}
	return nSize;
}

void CDefaultSkeleton::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_strAnimEventFilePath);

	{
		SIZER_COMPONENT_NAME(pSizer, "CModelMesh");
		pSizer->AddObject(m_ModelMesh);
	}

	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "FacialModel");
		pSizer->AddObject(m_pFacialModel);
	}

	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "SkeletonData");
		pSizer->AddObject(m_poseDefaultData);
		pSizer->AddObject(m_arrModelJoints);

		for (u32 i = 0; i < MAX_FEET_AMOUNT; i++)
			pSizer->AddObject(m_strFeetLockIKHandle[i]);

		pSizer->AddObject(m_recoilDesc.m_joints);
		pSizer->AddObject(m_recoilDesc.m_ikHandleLeft);
		pSizer->AddObject(m_recoilDesc.m_ikHandleRight);

		pSizer->AddObject(m_poseBlenderLookDesc.m_eyeAttachmentLeftName);
		pSizer->AddObject(m_poseBlenderLookDesc.m_eyeAttachmentRightName);
		pSizer->AddObject(m_poseBlenderLookDesc.m_blends);
		pSizer->AddObject(m_poseBlenderLookDesc.m_rotations);
		pSizer->AddObject(m_poseBlenderLookDesc.m_positions);

		pSizer->AddObject(m_poseBlenderAimDesc.m_blends);
		pSizer->AddObject(m_poseBlenderAimDesc.m_rotations);
		pSizer->AddObject(m_poseBlenderAimDesc.m_positions);

		pSizer->AddObject(m_ADIKTargets);
		pSizer->AddObject(m_IKLimbTypes);
	}
}

//#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
//----> CModelMesh, CGAs and all interfaces to access the mesh and the IMaterials will be removed in the future
//#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
void CDefaultSkeleton::PrecacheMesh(bool bFullUpdate, i32 nRoundId, i32 nLod)
{
	if (CModelMesh* pModelMesh = GetModelMesh())
	{
		i32k nZoneIdx = bFullUpdate ? 0 : 1;
		pModelMesh->m_stream.nRoundIds[nZoneIdx] = nRoundId;
	}
}

//////////////////////////////////////////////////////////////////////////
u32 CDefaultSkeleton::GetTextureMemoryUsage2(IDrxSizer* pSizer) const
{
	u32 nSize = 0;
	if (pSizer)
	{
		if (m_ModelMesh.m_pIRenderMesh)
			nSize += (u32)m_ModelMesh.m_pIRenderMesh->GetTextureMemoryUsage(m_ModelMesh.m_pIDefaultMaterial, pSizer);
	}
	else
	{
		if (m_ModelMesh.m_pIRenderMesh)
			nSize = (u32)m_ModelMesh.m_pIRenderMesh->GetTextureMemoryUsage(m_ModelMesh.m_pIDefaultMaterial);
	}
	return nSize;
}

//////////////////////////////////////////////////////////////////////////
u32 CDefaultSkeleton::GetMeshMemoryUsage(IDrxSizer* pSizer) const
{
	u32 nSize = 0;
	if (m_ModelMesh.m_pIRenderMesh)
	{
		nSize += m_ModelMesh.m_pIRenderMesh->GetMemoryUsage(0, IRenderMesh::MEM_USAGE_ONLY_STREAMS);
	}
	return nSize;
}
