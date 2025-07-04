// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/AttachmentUpr.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Animation/ModelMesh.h>
#include <drx3D/Animation/SocketSimulation.h>
#include <drx3D/Animation/AttachmentPRow.h>
#include <drx3D/Animation/AttachmentVCloth.h>
#include <drx3D/Animation/CharacterInstance.h>
#include <drx3D/Animation/CharacterUpr.h>
#include <drx3D/Animation/AttachmentMerger.h>
#include <memory>
#include <drx3D/Animation/Command_Commands.h>
#include <drx3D/Animation/Command_Buffer.h>

bool IsSkinFile(const string& fileName)
{
	tukk fileExt = PathUtil::GetExt(fileName.c_str());
	return drx_strcmp(fileExt, DRX_SKIN_FILE_EXT) == 0;
}

u32 CAttachmentUpr::LoadAttachmentList(tukk pathname)
{

	XmlNodeRef nodeAttachList = g_pISystem->LoadXmlFromFile(pathname);
	if (nodeAttachList == 0)
		return 0;

	tukk AttachListTag = nodeAttachList->getTag();
	if (strcmp(AttachListTag, "AttachmentList"))
		return 0;

	RemoveAllAttachments();

	DynArray<CharacterAttachment> arrAttachments;
	u32 numChildren = nodeAttachList->getChildCount();
	arrAttachments.resize(numChildren);
	u32 numValidAttachments = ParseXMLAttachmentList(&arrAttachments[0], numChildren, nodeAttachList);
	if (numValidAttachments)
	{
		arrAttachments.resize(numValidAttachments);
		InitAttachmentList(&arrAttachments[0], arrAttachments.size(), m_pSkelInstance->GetFilePath(), 0, -1);
	}
	return 1;
};

//-----------------------------------------------------------
//parse attachment-list
//-----------------------------------------------------------
u32 CAttachmentUpr::ParseXMLAttachmentList(CharacterAttachment* parrAttachments, u32 numAttachments, XmlNodeRef nodeAttachements)
{
	u32 numValidAttachments = 0;
	for (u32 i = 0; i < numAttachments; i++)
	{
		CharacterAttachment attach;
		XmlNodeRef nodeAttach = nodeAttachements->getChild(i);
		tukk AttachTag = nodeAttach->getTag();
		if (strcmp(AttachTag, "Attachment"))
			continue;                                                     //invalid

		stack_string Type = nodeAttach->getAttr("Type");
		if (Type == "CA_BONE")
			attach.m_Type = CA_BONE;
		if (Type == "CA_FACE")
			attach.m_Type = CA_FACE;
		if (Type == "CA_SKIN")
			attach.m_Type = CA_SKIN;
		if (Type == "CA_PROX")
			attach.m_Type = CA_PROX;
		if (Type == "CA_PROW")
			attach.m_Type = CA_PROW;
		if (Type == "CA_VCLOTH")
			attach.m_Type = CA_VCLOTH;
		if (attach.m_Type == 0xDeadBeef)
			continue;                                                     //invalid

		string AName = nodeAttach->getAttr("AName");
		PathUtil::UnifyFilePath(AName);
		attach.m_strAttachmentName = AName;

		nodeAttach->getAttr("Rotation", attach.m_AbsoluteDefault.q);
		nodeAttach->getAttr("Position", attach.m_AbsoluteDefault.t);

		attach.m_relRotation = nodeAttach->getAttr("RelRotation", attach.m_RelativeDefault.q);
		attach.m_relPosition = nodeAttach->getAttr("RelPosition", attach.m_RelativeDefault.t);

		attach.m_strJointName = nodeAttach->getAttr("BoneName");
		attach.m_strBindingPath = nodeAttach->getAttr("Binding");
		PathUtil::UnifyFilePath(attach.m_strBindingPath);
		attach.m_strSimBindingPath = nodeAttach->getAttr("simBinding");   //only used for the cloth simulation mesh

		nodeAttach->getAttr("ProxyParams", attach.m_ProxyParams);
		nodeAttach->getAttr("ProxyPurpose", attach.m_ProxyPurpose);

		tukk fileExt = PathUtil::GetExt(attach.m_strBindingPath);
		const bool isCGF = (0 == stricmp(fileExt, DRX_GEOMETRY_FILE_EXT));
		if (isCGF)
			attach.m_pStaticObject = gEnv->p3DEngine->LoadStatObj(attach.m_strBindingPath.c_str(), 0, 0, true);

		if (nodeAttach->haveAttr("Material"))
		{
			tukk material = nodeAttach->getAttr("Material");
			attach.m_pMaterial = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material, false);
		}
		if (nodeAttach->haveAttr("MaterialLOD0"))
		{
			tukk material0 = nodeAttach->getAttr("MaterialLOD0");
			attach.m_parrMaterial[0] = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material0, false);
		}
		if (nodeAttach->haveAttr("MaterialLOD1"))
		{
			tukk material1 = nodeAttach->getAttr("MaterialLOD1");
			attach.m_parrMaterial[1] = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material1, false);
		}
		if (nodeAttach->haveAttr("MaterialLOD2"))
		{
			tukk material2 = nodeAttach->getAttr("MaterialLOD2");
			attach.m_parrMaterial[2] = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material2, false);
		}
		if (nodeAttach->haveAttr("MaterialLOD3"))
		{
			tukk material3 = nodeAttach->getAttr("MaterialLOD3");
			attach.m_parrMaterial[3] = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material3, false);
		}
		if (nodeAttach->haveAttr("MaterialLOD4"))
		{
			tukk material4 = nodeAttach->getAttr("MaterialLOD4");
			attach.m_parrMaterial[4] = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material4, false);
		}
		if (nodeAttach->haveAttr("MaterialLOD5"))
		{
			tukk material5 = nodeAttach->getAttr("MaterialLOD5");
			attach.m_parrMaterial[5] = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(material5, false);
		}

		u32 isPendulum = 0;
		nodeAttach->getAttr("PA_PendulumType", isPendulum);
		if (isPendulum == SimulationParams::PENDULUM_CONE || isPendulum == SimulationParams::PENDULUM_HINGE_PLANE || isPendulum == SimulationParams::PENDULUM_HALF_CONE)
		{
			attach.ap.m_nClampType = SimulationParams::ClampType(isPendulum);
			nodeAttach->getAttr("PA_FPS", attach.ap.m_nSimFPS);
			nodeAttach->getAttr("PA_Redirect", attach.ap.m_useRedirect);
			nodeAttach->getAttr("PA_MaxAngle", attach.ap.m_fMaxAngle);
			nodeAttach->getAttr("PA_HRotation", attach.ap.m_vDiskRotation.x);

			nodeAttach->getAttr("PA_Mass", attach.ap.m_fMass);
			nodeAttach->getAttr("PA_Gravity", attach.ap.m_fGravity);
			nodeAttach->getAttr("PA_Damping", attach.ap.m_fDamping);
			nodeAttach->getAttr("PA_Stiffness", attach.ap.m_fStiffness);

			nodeAttach->getAttr("PA_PivotOffset", attach.ap.m_vPivotOffset);
			nodeAttach->getAttr("PA_PendulumOffset", attach.ap.m_vSimulationAxis);
			nodeAttach->getAttr("PA_SimulationAxis", attach.ap.m_vSimulationAxis);
			nodeAttach->getAttr("PA_StiffnessTarget", attach.ap.m_vStiffnessTarget);

			nodeAttach->getAttr("PA_CapsuleX", attach.ap.m_vCapsule.x);
			nodeAttach->getAttr("PA_CapsuleY", attach.ap.m_vCapsule.y);
			nodeAttach->getAttr("PA_ProjectionType", attach.ap.m_nProjectionType);
			attach.ap.m_strDirTransJoint = nodeAttach->getAttr("PA_DirTransJointName");
			u32 IsIdentical = stricmp(attach.ap.m_strDirTransJoint.c_str(), attach.m_strJointName.c_str()) == 0;
			if (attach.ap.m_strDirTransJoint.length() && IsIdentical)
				attach.ap.m_strDirTransJoint.reset();

			char proxytag[] = "PA_Proxy00";
			for (u32 i = 0; i < SimulationParams::MaxCollisionProxies; i++)
			{
				CDrxName strProxyName = CDrxName(nodeAttach->getAttr(proxytag));
				proxytag[9]++;
				if (strProxyName.empty())
					continue;
				attach.ap.m_arrProxyNames.push_back(strProxyName);
			}
		}

		u32 isSpring = 0;
		nodeAttach->getAttr("SA_SpringType", isSpring);
		if (isSpring)
		{
			attach.ap.m_nClampType = SimulationParams::SPRING_ELLIPSOID;
			nodeAttach->getAttr("SA_FPS", attach.ap.m_nSimFPS);
			nodeAttach->getAttr("SA_Radius", attach.ap.m_fRadius);
			nodeAttach->getAttr("SA_ScaleZP", attach.ap.m_vSphereScale.x);
			nodeAttach->getAttr("SA_ScaleZN", attach.ap.m_vSphereScale.y);
			nodeAttach->getAttr("SA_DiskRotX", attach.ap.m_vDiskRotation.x);
			nodeAttach->getAttr("SA_DiskRotZ", attach.ap.m_vDiskRotation.y);
			nodeAttach->getAttr("SA_HRotation", attach.ap.m_vDiskRotation.x);   //just for backwards compatibility

			nodeAttach->getAttr("SA_Redirect", attach.ap.m_useRedirect);

			nodeAttach->getAttr("SA_Mass", attach.ap.m_fMass);
			nodeAttach->getAttr("SA_Gravity", attach.ap.m_fGravity);
			nodeAttach->getAttr("SA_Damping", attach.ap.m_fDamping);
			nodeAttach->getAttr("SA_Stiffness", attach.ap.m_fStiffness);

			nodeAttach->getAttr("SA_PivotOffset", attach.ap.m_vPivotOffset);
			nodeAttach->getAttr("SA_StiffnessTarget", attach.ap.m_vStiffnessTarget);

			attach.ap.m_vCapsule.x = 0;
			nodeAttach->getAttr("SA_CapsuleY", attach.ap.m_vCapsule.y);
			nodeAttach->getAttr("SA_ProjectionType", attach.ap.m_nProjectionType);

			char proxytag[] = "SA_Proxy00";
			for (u32 i = 0; i < SimulationParams::MaxCollisionProxies; i++)
			{
				CDrxName strProxyName = CDrxName(nodeAttach->getAttr(proxytag));
				proxytag[9]++;
				if (strProxyName.empty())
					continue;
				attach.ap.m_arrProxyNames.push_back(strProxyName);
			}
		}

		u32 IsProjection = 0;
		nodeAttach->getAttr("P_Projection", IsProjection);
		if (IsProjection)
		{
			attach.ap.m_nClampType = SimulationParams::TRANSLATIONAL_PROJECTION;
			attach.ap.m_useRedirect = 1;
			nodeAttach->getAttr("P_ProjectionType", attach.ap.m_nProjectionType);
			attach.ap.m_strDirTransJoint = nodeAttach->getAttr("P_DirTransJointName");
			u32 IsIdentical = stricmp(attach.ap.m_strDirTransJoint.c_str(), attach.m_strJointName.c_str()) == 0;
			if (attach.ap.m_strDirTransJoint.length() && IsIdentical)
				attach.ap.m_strDirTransJoint.reset();

			nodeAttach->getAttr("P_TranslationAxis", attach.ap.m_vSimulationAxis);

			nodeAttach->getAttr("P_CapsuleX", attach.ap.m_vCapsule.x);
			nodeAttach->getAttr("P_CapsuleY", attach.ap.m_vCapsule.y);

			nodeAttach->getAttr("P_PivotOffset", attach.ap.m_vPivotOffset);

			char proxytag[] = "P_Proxy00";
			for (u32 i = 0; i < SimulationParams::MaxCollisionProxies; i++)
			{
				CDrxName strProxyName = CDrxName(nodeAttach->getAttr(proxytag));
				proxytag[8]++;
				if (strProxyName.empty())
					continue;
				attach.ap.m_arrProxyNames.push_back(strProxyName);
			}
		}

		attach.ap.m_strProcFunction = nodeAttach->getAttr("ProcFunction");

		if (nodeAttach->haveAttr("PhysPropType"))
		{
			memset(&attach.m_AttPhysInfo, 0, sizeof(attach.m_AttPhysInfo));
			stack_string propType = nodeAttach->getAttr("PhysPropType");

			i32 nRopeOrGrid = !strcmp(propType, "Rope") ? 0 : (!strcmp(propType, "Cloth") ? 1 : -1);
			DynArray<SJointProperty> jp = CDefaultSkeleton::GetPhysInfoProperties_ROPE(attach.m_AttPhysInfo[0], nRopeOrGrid);             //just write the names into jp

			for (i32 nLod = 0; nLod < 2; nLod++)
			{
				bool lodUsed = false;
				u32 numRopeJoints = jp.size();
				for (u32 idx = 1; idx < numRopeJoints; idx++)
				{
					char buf[32];
					drx_sprintf(buf, "lod%d_%s", nLod, jp[idx].name);
					if (jp[idx].type == 0)
						lodUsed |= nodeAttach->getAttr(buf, jp[idx].fval);
					else
						lodUsed |= nodeAttach->getAttr(buf, jp[idx].bval);
				}
				if (lodUsed)
					CDefaultSkeleton::ParsePhysInfoProperties_ROPE(attach.m_AttPhysInfo[nLod], jp);                                         //just init m_PhysInfo from jp
			}
		}

		u32 flags;
		if (nodeAttach->getAttr("Flags", flags))
			attach.m_AttFlags = flags;

		if (attach.m_Type == CA_VCLOTH)
		{
			// Animation Control
			nodeAttach->getAttr("hide", attach.clothParams.hide);
			nodeAttach->getAttr("forceSkinning", attach.clothParams.forceSkinning);
			nodeAttach->getAttr("forceSkinningFpsThreshold", attach.clothParams.forceSkinningFpsThreshold);
			nodeAttach->getAttr("forceSkinningTranslateThreshold", attach.clothParams.forceSkinningTranslateThreshold);
			nodeAttach->getAttr("checkAnimationRewind", attach.clothParams.checkAnimationRewind);
			nodeAttach->getAttr("disableSimulationAtDistance", attach.clothParams.disableSimulationAtDistance);
			nodeAttach->getAttr("disableSimulationTimeRange", attach.clothParams.disableSimulationTimeRange);
			nodeAttach->getAttr("enableSimulationSSaxisSizePerc", attach.clothParams.enableSimulationSSaxisSizePerc);

			// Simulation and Collision
			nodeAttach->getAttr("timeStep", attach.clothParams.timeStep);
			nodeAttach->getAttr("timeStepMax", attach.clothParams.timeStepsMax);
			nodeAttach->getAttr("numIterations", attach.clothParams.numIterations);
			nodeAttach->getAttr("collideEveryNthStep", attach.clothParams.collideEveryNthStep);
			nodeAttach->getAttr("collisionMultipleShiftFactor", attach.clothParams.collisionMultipleShiftFactor);
			nodeAttach->getAttr("gravityFactor", attach.clothParams.gravityFactor);

			// Stiffness and Elasticity
			nodeAttach->getAttr("stretchStiffness", attach.clothParams.stretchStiffness);
			nodeAttach->getAttr("shearStiffness", attach.clothParams.shearStiffness);
			nodeAttach->getAttr("bendStiffness", attach.clothParams.bendStiffness);
			nodeAttach->getAttr("bendStiffnessByTrianglesAngle", attach.clothParams.bendStiffnessByTrianglesAngle);
			nodeAttach->getAttr("pullStiffness", attach.clothParams.pullStiffness);

			// Friction and Damping
			nodeAttach->getAttr("Friction", attach.clothParams.friction);
			nodeAttach->getAttr("rigidDamping", attach.clothParams.rigidDamping);
			nodeAttach->getAttr("springDamping", attach.clothParams.springDamping);
			nodeAttach->getAttr("springDampingPerSubstep", attach.clothParams.springDampingPerSubstep);
			nodeAttach->getAttr("collisionDampingTangential", attach.clothParams.collisionDampingTangential);

			// Nearest Neighbor Distance Constraints
			nodeAttach->getAttr("nearestNeighborDistanceConstraints", attach.clothParams.useNearestNeighborDistanceConstraints);
			nodeAttach->getAttr("nndcAllowedExtension", attach.clothParams.nndcAllowedExtension);
			nodeAttach->getAttr("nndcMaximumShiftFactor", attach.clothParams.nndcMaximumShiftFactor);
			nodeAttach->getAttr("nndcShiftCollisionFactor", attach.clothParams.nndcShiftCollisionFactor);

			// Test Reset Damping
			nodeAttach->getAttr("resetDampingFactor", attach.clothParams.resetDampingFactor);
			nodeAttach->getAttr("resetDampingRange", attach.clothParams.resetDampingRange);

			// Additional
			nodeAttach->getAttr("translationBlend", attach.clothParams.translationBlend);
			nodeAttach->getAttr("rotationBlend", attach.clothParams.rotationBlend);
			nodeAttach->getAttr("externalBlend", attach.clothParams.externalBlend);
			nodeAttach->getAttr("maxAnimDistance", attach.clothParams.maxAnimDistance);
			nodeAttach->getAttr("filterLaplace", attach.clothParams.filterLaplace);

			// Debug
			nodeAttach->getAttr("isMainCharacter", attach.clothParams.isMainCharacter);
			attach.clothParams.renderMeshName = nodeAttach->getAttr("renderMeshName");
			//attach.clothParams.renderBinding = nodeAttach->getAttr( "Binding");
			attach.clothParams.renderBinding = attach.m_strBindingPath;
			attach.clothParams.simMeshName = nodeAttach->getAttr("simMeshName");
			//attach.clothParams.simBinding = nodeAttach->getAttr( "simBinding");
			attach.clothParams.simBinding = attach.m_strSimBindingPath;
			attach.clothParams.material = nodeAttach->getAttr("Material");
			nodeAttach->getAttr("debugDrawVerticesRadius", attach.clothParams.debugDrawVerticesRadius);
			nodeAttach->getAttr("debugDrawCloth", attach.clothParams.debugDrawCloth);
			nodeAttach->getAttr("debugDrawNNDC", attach.clothParams.debugDrawNndc);
			nodeAttach->getAttr("debugPrint", attach.clothParams.debugPrint);
			// overwrite debug settings
			attach.clothParams.debugPrint = 0;
		}

		if (attach.m_Type == CA_PROW)
		{
			attach.m_strRowJointName = nodeAttach->getAttr("RowJointName");

			u32 nPendulumClampMode = 0;
			u32 isPendulum = nodeAttach->getAttr("ROW_ClampMode", nPendulumClampMode);
			if (isPendulum)
			{
				attach.rowap.m_nClampMode = RowSimulationParams::ClampMode(nPendulumClampMode);
				nodeAttach->getAttr("ROW_FPS", attach.rowap.m_nSimFPS);
				nodeAttach->getAttr("ROW_ConeAngle", attach.rowap.m_fConeAngle);
				nodeAttach->getAttr("ROW_ConeRotation", attach.rowap.m_vConeRotation);

				nodeAttach->getAttr("ROW_Mass", attach.rowap.m_fMass);
				nodeAttach->getAttr("ROW_Gravity", attach.rowap.m_fGravity);
				nodeAttach->getAttr("ROW_Damping", attach.rowap.m_fDamping);
				nodeAttach->getAttr("ROW_JointSpring", attach.rowap.m_fJointSpring);
				nodeAttach->getAttr("ROW_RodLength", attach.rowap.m_fRodLength);
				nodeAttach->getAttr("ROW_StiffnessTarget", attach.rowap.m_vStiffnessTarget);
				nodeAttach->getAttr("ROW_Turbulence", attach.rowap.m_vTurbulence);
				nodeAttach->getAttr("ROW_MaxVelocity", attach.rowap.m_fMaxVelocity);

				nodeAttach->getAttr("ROW_Cycle", attach.rowap.m_cycle);
				nodeAttach->getAttr("ROW_RelaxLoops", attach.rowap.m_relaxationLoops);
				nodeAttach->getAttr("ROW_Stretch", attach.rowap.m_fStretch);

				nodeAttach->getAttr("ROW_CapsuleX", attach.rowap.m_vCapsule.x);
				nodeAttach->getAttr("ROW_CapsuleY", attach.rowap.m_vCapsule.y);
				nodeAttach->getAttr("ROW_ProjectionType", attach.rowap.m_nProjectionType);

				char proxytag[] = "ROW_Proxy00";
				for (u32 i = 0; i < SimulationParams::MaxCollisionProxies; i++)
				{
					CDrxName strProxyName = CDrxName(nodeAttach->getAttr(proxytag));
					proxytag[10]++;
					if (strProxyName.empty())
						continue;
					attach.rowap.m_arrProxyNames.push_back(strProxyName);
				}
			}
		}
		parrAttachments[numValidAttachments++] = attach;
	}
	return numValidAttachments;
}

//-----------------------------------------------------------
//init attachment-list
//-----------------------------------------------------------
void CAttachmentUpr::InitAttachmentList(const CharacterAttachment* parrAttachments, u32 numAttachments, const string pathname, u32 nLoadingFlags, i32 nKeepModelInMemory)
{
	u32 nLogWarnings = (nLoadingFlags & CA_DisableLogWarnings) == 0;
	CSkeletonPose& rSkelPose = (CSkeletonPose&)*m_pSkelInstance->GetISkeletonPose();
	CDefaultSkeleton& rDefaultSkeleton = *m_pSkelInstance->m_pDefaultSkeleton;

	// flush once, because, here we will execute immediate commands
	UpdateBindings();

	bool bHasVertexAnimation = false;
	CCharInstance* pICharacter = m_pSkelInstance;
	for (u32 i = 0; i < numAttachments; i++)
	{
		// copy attachment, so we don't rely on vector-s memory
		const CharacterAttachment& attach = parrAttachments[i];
		u32 nSizeOfBindingFilePath = attach.m_strBindingPath.size();

		tukk fileExt = PathUtil::GetExt(attach.m_strBindingPath);
		bool IsCDF = (0 == stricmp(fileExt, "cdf"));
		bool IsCGA = (0 == stricmp(fileExt, "cga"));
		bool IsCGF = (0 == stricmp(fileExt, "cgf"));

		bool IsSKEL = (0 == stricmp(fileExt, DRX_SKEL_FILE_EXT));
		bool IsSKIN = (0 == stricmp(fileExt, DRX_SKIN_FILE_EXT));

		if (attach.m_Type == CA_BONE)
		{
			CAttachmentBONE* pAttachment = (CAttachmentBONE*)CreateAttachment(attach.m_strAttachmentName, CA_BONE, attach.m_strJointName.c_str());
			if (pAttachment == 0)
				continue;

			QuatT defaultTransform;
			if (pAttachment->m_nJointID < 0)
			{
				defaultTransform.SetIdentity();
			}
			else
			{
				defaultTransform = rDefaultSkeleton.GetDefaultAbsJointByID(pAttachment->m_nJointID);
			}

			QuatT qt = attach.m_AbsoluteDefault;
			QuatT rel = defaultTransform.GetInverted() * qt;
			if (attach.m_relPosition)
				rel.t = attach.m_RelativeDefault.t;
			if (attach.m_relRotation)
				rel.q = attach.m_RelativeDefault.q;
			qt = defaultTransform * rel;
			pAttachment->SetAttAbsoluteDefault(qt);
			pAttachment->ProjectAttachment();

			pAttachment->SetFlags(attach.m_AttFlags);
			pAttachment->HideAttachment(attach.m_AttFlags & FLAGS_ATTACH_HIDE_ATTACHMENT);
			SimulationParams& ap = pAttachment->GetSimulationParams();
			ap = attach.ap;
			CSkeletonPose& rSkelPose = (CSkeletonPose&)*m_pSkelInstance->GetISkeletonPose();

			if (IsSKEL || IsCGA || IsCDF)
			{
				ICharacterInstance* pIChildCharacter = g_pCharacterUpr->CreateInstance(attach.m_strBindingPath, nLoadingFlags);
				if (pIChildCharacter == 0 && nSizeOfBindingFilePath && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: no character as attachment created: %s", pathname.c_str());

				if (pIChildCharacter)
				{
					CSKELAttachment* pCharacterAttachment = new CSKELAttachment();
					pCharacterAttachment->m_pCharInstance = pIChildCharacter;
					IAttachmentObject* pIAttachmentObject = (IAttachmentObject*)pCharacterAttachment;
					pAttachment->Immediate_AddBinding(pIAttachmentObject);
					if (attach.m_pMaterial && pIAttachmentObject)
						pIAttachmentObject->SetReplacementMaterial(attach.m_pMaterial);
					bHasVertexAnimation |= pIChildCharacter->HasVertexAnimation();
				}
			}
			if (IsCGF)
			{
				IStatObj* pIStatObj = g_pISystem->GetI3DEngine()->LoadStatObj(attach.m_strBindingPath, 0, 0, true, 0);
				if (pIStatObj == 0 && nSizeOfBindingFilePath && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: no static object as attachment created: %s", pathname.c_str());

				if (pIStatObj)
				{
					CCGFAttachment* pStatAttachment = new CCGFAttachment();
					pStatAttachment->pObj = pIStatObj;
					IAttachmentObject* pIAttachmentObject = (IAttachmentObject*)pStatAttachment;
					pAttachment->Immediate_AddBinding(pIAttachmentObject);
					if (attach.m_pMaterial && pIAttachmentObject)
						pIAttachmentObject->SetReplacementMaterial(attach.m_pMaterial);
				}
			}

			//this should have its own type; its not really an attachment
			for (i32 nLod = 0; nLod < 2; nLod++)
			{
				if (*(alias_cast<i32k*>(attach.m_AttPhysInfo[nLod].spring_angle)) == 0x12345678)
					pICharacter->m_SkeletonPose.m_physics.SetJointPhysInfo(pAttachment->GetJointID(), attach.m_AttPhysInfo[nLod], nLod);
			}
		}

		//-----------------------------------------------------------------------------------

		if (attach.m_Type == CA_FACE)
		{
			CAttachmentFACE* pAttachment = (CAttachmentFACE*)CreateAttachment(attach.m_strAttachmentName, CA_FACE, 0, 0);
			if (pAttachment == 0)
				continue;

			pAttachment->SetAttAbsoluteDefault(attach.m_AbsoluteDefault);
			pAttachment->SetFlags(attach.m_AttFlags);
			pAttachment->HideAttachment(attach.m_AttFlags & FLAGS_ATTACH_HIDE_ATTACHMENT);
			SimulationParams& ap = pAttachment->GetSimulationParams();
			ap = attach.ap;

			if (IsSKEL || IsCGA || IsCDF)
			{
				ICharacterInstance* pIChildCharacter = g_pCharacterUpr->CreateInstance(attach.m_strBindingPath, nLoadingFlags);
				if (pIChildCharacter == 0 && nSizeOfBindingFilePath && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: no character created: %s", pathname.c_str());

				if (pIChildCharacter)
				{
					CSKELAttachment* pCharacterAttachment = new CSKELAttachment();
					pCharacterAttachment->m_pCharInstance = pIChildCharacter;
					IAttachmentObject* pIAttachmentObject = (IAttachmentObject*)pCharacterAttachment;
					pAttachment->Immediate_AddBinding(pIAttachmentObject);
					if (attach.m_pMaterial && pIAttachmentObject)
						pIAttachmentObject->SetReplacementMaterial(attach.m_pMaterial);
				}
			}
			if (IsCGF)
			{
				IStatObj* pIStatObj = g_pISystem->GetI3DEngine()->LoadStatObj(attach.m_strBindingPath, 0, 0, true, 0);
				if (pIStatObj == 0 && nSizeOfBindingFilePath && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: no static object as attachment created: %s", pathname.c_str());

				if (pIStatObj)
				{
					CCGFAttachment* pStatAttachment = new CCGFAttachment();
					pStatAttachment->pObj = pIStatObj;
					IAttachmentObject* pIAttachmentObject = (IAttachmentObject*)pStatAttachment;
					pAttachment->Immediate_AddBinding(pIAttachmentObject);
					if (attach.m_pMaterial && pIAttachmentObject)
						pIAttachmentObject->SetReplacementMaterial(attach.m_pMaterial);
				}
			}

		}

		//-----------------------------------------------------------------------------------

		if (attach.m_Type == CA_SKIN)
		{
			CAttachmentSKIN* pAttachment = (CAttachmentSKIN*)CreateAttachment(attach.m_strAttachmentName, CA_SKIN);
			if (pAttachment == 0)
				continue;

			if (IsSKIN == 0 && attach.m_strBindingPath.size() > 0)
				g_pILog->LogError("DinrusXAnimation: a skin-attachment must be a SKIN-file. You can't use this file: '%s' in attachment '%s'", attach.m_strBindingPath.c_str(), attach.m_strAttachmentName.c_str());

			if (IsSKIN)
			{
				ISkin* pIModelSKIN = g_pCharacterUpr->LoadModelSKIN(attach.m_strBindingPath, nLoadingFlags);
				if (pIModelSKIN == 0 && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: skin-attachment not created: CDF: %s  SKIN: %s", pathname.c_str(), attach.m_strBindingPath.c_str());

				if (pIModelSKIN)
				{
					CSKINAttachment* pSkinInstance = new CSKINAttachment();
					pSkinInstance->m_pIAttachmentSkin = pAttachment;
					IAttachmentObject* pIAttachmentObject = (IAttachmentObject*)pSkinInstance;
					if (pAttachment->Immediate_AddBinding(pIAttachmentObject, pIModelSKIN, nLoadingFlags))
					{
						pAttachment->SetFlags(attach.m_AttFlags);
						bHasVertexAnimation |= (attach.m_AttFlags & FLAGS_ATTACH_SW_SKINNING) != 0;
						pAttachment->HideAttachment(attach.m_AttFlags & FLAGS_ATTACH_HIDE_ATTACHMENT);

						if (pIAttachmentObject)
						{
							if (attach.m_pMaterial)
							{
								for (u32 nLOD = 0; nLOD < g_nMaxGeomLodLevels; nLOD++)
								{
									pIAttachmentObject->SetReplacementMaterial(attach.m_pMaterial, nLOD);
								}
							}
							for (u32 nLOD = 0; nLOD < g_nMaxGeomLodLevels; nLOD++)
							{
								if (attach.m_parrMaterial[nLOD])
									pIAttachmentObject->SetReplacementMaterial(attach.m_parrMaterial[nLOD], nLOD);
							}

							if (const CModelMesh* pModelMesh = ((CSkin*)pIModelSKIN)->GetModelMesh(0))
							{
								pAttachment->m_vertexAnimation.CreateFrameStates(pModelMesh->m_softwareMesh.GetVertexFrames(), *(pICharacter->m_pDefaultSkeleton.get()));
							}
						}

					}

					if (nKeepModelInMemory == 0 || nKeepModelInMemory == 1)
					{
						ISkin* pISKIN = pIModelSKIN;
						CSkin* pCSKIN = (CSkin*)pISKIN;
						if (nKeepModelInMemory)
							pCSKIN->SetKeepInMemory(true);
						else
							pCSKIN->SetKeepInMemory(false);
					}
				}
			}
		}

		//-------------------------------------------------------------------------

		if (attach.m_Type == CA_PROX)
		{
			QuatT qt = attach.m_AbsoluteDefault;
			i32 jointid = rDefaultSkeleton.GetJointIDByName(attach.m_strJointName);
			if (jointid >= 0)
			{
				QuatT defaultTransform = rDefaultSkeleton.GetDefaultAbsJointByID(jointid);
				QuatT rel = defaultTransform.GetInverted() * qt;
				if (attach.m_relPosition)
					rel.t = attach.m_RelativeDefault.t;
				if (attach.m_relRotation)
					rel.q = attach.m_RelativeDefault.q;
				qt = defaultTransform * rel;
			}
			CProxy proxy;
			proxy.m_pAttachmentUpr = this;
			proxy.m_strProxyName = attach.m_strAttachmentName;
			proxy.m_nProxyCRC32 = CCrc32::ComputeLowercase(proxy.m_strProxyName.c_str());
			proxy.m_strJointName = attach.m_strJointName;
			proxy.m_ProxyAbsoluteDefault = qt;
			proxy.m_params = attach.m_ProxyParams;
			proxy.m_nPurpose = attach.m_ProxyPurpose;
			m_arrProxies.push_back(proxy);
		}

		//-------------------------------------------------------------------------
		if (attach.m_Type == CA_PROW)
		{
			CAttachmentPROW* pCAttachmentPROW = (CAttachmentPROW*)CreateAttachment(attach.m_strAttachmentName, CA_PROW, attach.m_strRowJointName.c_str());
			if (pCAttachmentPROW == 0)
				continue;
			(RowSimulationParams&)pCAttachmentPROW->m_rowparams = attach.rowap;
		}
		//-------------------------------------------------------------------------
		if (attach.m_Type == CA_VCLOTH)
		{
			CAttachmentVCLOTH* pCAttachmentVCloth = (CAttachmentVCLOTH*)CreateAttachment(attach.m_strAttachmentName, CA_VCLOTH);
			if (pCAttachmentVCloth == 0)
				continue;

			if (IsSKIN == 0)
				g_pILog->LogError("DinrusXAnimation: a rendermesh for vertex-cloth must be a SKIN-file. You can't use this file: %s", pathname.c_str());

			tukk fileExt = PathUtil::GetExt(attach.m_strSimBindingPath);
			bool IsSimSKIN = (0 == stricmp(fileExt, DRX_SKIN_FILE_EXT));
			if (IsSimSKIN == 0)
				g_pILog->LogError("DinrusXAnimation: a simulation-mesh must be a SKIN-file. You can't use this file: %s", pathname.c_str());

			if (IsSKIN && IsSimSKIN)
			{
				ISkin* pIModelSKIN = g_pCharacterUpr->LoadModelSKIN(attach.m_strBindingPath, nLoadingFlags);
				if (pIModelSKIN == 0 && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: skin-attachment not created: CDF: %s  SKIN: %s", pathname.c_str(), attach.m_strBindingPath.c_str());

				ISkin* pIModelSimSKIN = g_pCharacterUpr->LoadModelSKIN(attach.m_strSimBindingPath, nLoadingFlags);
				if (pIModelSimSKIN == 0 && nLogWarnings)
					g_pILog->LogError("DinrusXAnimation: skin-attachment not created: CDF: %s  SKIN: %s", pathname.c_str(), attach.m_strSimBindingPath.c_str());

				if (pIModelSKIN && pIModelSimSKIN)
				{
					CSKINAttachment* pSkinInstance = new CSKINAttachment();
					pSkinInstance->m_pIAttachmentSkin = pCAttachmentVCloth;
					IAttachmentObject* pIAttachmentObject = (IAttachmentObject*)pSkinInstance;
					if (pCAttachmentVCloth->Immediate_AddBinding(pIAttachmentObject, pIModelSKIN, nLoadingFlags))
					{
						pCAttachmentVCloth->SetFlags(attach.m_AttFlags | FLAGS_ATTACH_SW_SKINNING);
						bHasVertexAnimation = true;
						pCAttachmentVCloth->HideAttachment(attach.m_AttFlags & FLAGS_ATTACH_HIDE_ATTACHMENT);

						if (pIAttachmentObject)
						{
							if (attach.m_pMaterial)
							{
								for (u32 nLOD = 0; nLOD < g_nMaxGeomLodLevels; nLOD++)
								{
									pIAttachmentObject->SetReplacementMaterial(attach.m_pMaterial, nLOD);
								}
							}
							for (u32 nLOD = 0; nLOD < g_nMaxGeomLodLevels; nLOD++)
							{
								if (attach.m_parrMaterial[nLOD])
									pIAttachmentObject->SetReplacementMaterial(attach.m_parrMaterial[nLOD], nLOD);
							}

							if (const CModelMesh* pModelMesh = ((CSkin*)pIModelSKIN)->GetModelMesh(0))
								pCAttachmentVCloth->m_vertexAnimation.CreateFrameStates(pModelMesh->m_softwareMesh.GetVertexFrames(), *(pICharacter->m_pDefaultSkeleton.get()));
						}
						pCAttachmentVCloth->AddSimBinding(*pIModelSimSKIN, nLoadingFlags);
						pCAttachmentVCloth->AddClothParams(attach.clothParams);
						pCAttachmentVCloth->ComputeClothCacheKey();
					}

					if (nKeepModelInMemory == 0 || nKeepModelInMemory == 1)
					{
						if (nKeepModelInMemory)
						{
							((CSkin*)pIModelSKIN)->SetKeepInMemory(true);
							((CSkin*)pIModelSimSKIN)->SetKeepInMemory(true);
						}
						else
						{
							((CSkin*)pIModelSKIN)->SetKeepInMemory(false);
							((CSkin*)pIModelSimSKIN)->SetKeepInMemory(false);
						}
					}
				}
			}
		}
	}
	pICharacter->SetHasVertexAnimation(bHasVertexAnimation);

	u32 count = GetAttachmentCount();
	ProjectAllAttachment();
	VerifyProxyLinks();

	u32 nproxies = m_arrProxies.size();
	for (u32 i = 0; i < nproxies; i++)
	{
		i32 idx = m_arrProxies[i].m_nJointID;
		m_arrProxies[i].m_ProxyModelRelative = rDefaultSkeleton.GetDefaultAbsJointByID(idx) * m_arrProxies[i].m_ProxyRelativeDefault;
		m_arrProxies[i].m_ProxyModelRelativePrev = m_arrProxies[i].m_ProxyModelRelative;
	}

	u32 numAttachmnets = GetAttachmentCount();
	for (u32 i = 0; i < numAttachmnets; i++)
	{
		IAttachment* pIAttachment = GetInterfaceByIndex(i);
		if (pIAttachment->GetType() == CA_BONE)
		{
			i32 id = pIAttachment->GetJointID();
			tukk jname = rDefaultSkeleton.GetJointNameByID(id);
			pIAttachment->PostUpdateSimulationParams(1, jname);
		}
		if (pIAttachment->GetType() == CA_FACE)
			pIAttachment->PostUpdateSimulationParams(1);
		if (pIAttachment->GetType() == CA_PROW)
			pIAttachment->PostUpdateSimulationParams(1);
	}

	m_TypeSortingRequired++;
}

void CAttachmentUpr::MergeCharacterAttachments()
{
	DEFINE_PROFILER_FUNCTION();

	// check for invalidated bindings first
	for (auto it = m_mergedAttachments.begin(); it < m_mergedAttachments.end(); )
	{
		CAttachmentMerged* pAttachment = *it;
		if (!pAttachment->AreAttachmentBindingsValid())
		{
			pAttachment->Invalidate();
			it = m_mergedAttachments.erase(it);
		}
		else
			++it;
	}

	// try to merge anything that's not merged yet
	CAttachmentMerger::Instance().MergeAttachments(m_arrAttachments, m_mergedAttachments, this);
	m_attachmentMergingRequired = 0;
}

i32 CAttachmentUpr::FindExtraBone(IAttachment* pAttachment)
{
	auto it = std::find(m_extraBones.begin(), m_extraBones.end(), pAttachment);
	return it != m_extraBones.end() ? i32(it - m_extraBones.begin()) : -1;
}

void CAttachmentUpr::UpdateBindings()
{
	m_modificationCommandBuffer.Execute();
}

i32 CAttachmentUpr::AddExtraBone(IAttachment* pAttachment)
{
	auto it = std::find(m_extraBones.begin(), m_extraBones.end(), static_cast<IAttachment*>(NULL));
	if (it != m_extraBones.end())
	{
		*it = pAttachment;
	}
	else
	{
		it = m_extraBones.push_back(pAttachment);
	}

	return i32(it - m_extraBones.begin());
}

void CAttachmentUpr::RemoveExtraBone(IAttachment* pAttachment)
{
	i32 extraBone = FindExtraBone(pAttachment);
	if (extraBone != -1)
		m_extraBones[extraBone] = NULL;
}

IAttachment* CAttachmentUpr::CreateAttachment(tukk szAttName, u32 type, tukk szJointName, bool bCallProject)
{
	string strAttachmentName = szAttName;
	strAttachmentName.MakeLower();

	IAttachment* pIAttachmentName = GetInterfaceByName(strAttachmentName.c_str());
	if (pIAttachmentName)
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Attachment name '%s' is already in use, attachment will not be created", strAttachmentName.c_str());
		return 0;
	}

	u32 nameCRC = CCrc32::ComputeLowercase(strAttachmentName.c_str());
	IAttachment* pIAttachmentCRC32 = GetInterfaceByNameCRC(nameCRC);
	if (pIAttachmentCRC32)
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Attachment CRC32 for '%s' clashes with attachment name '%s' (crc's are created using lower case only), attachment will not be created", strAttachmentName.c_str(), pIAttachmentCRC32->GetName());
		return 0;
	}

	CDefaultSkeleton& rDefaultSkeleton = *m_pSkelInstance->m_pDefaultSkeleton;

	//----------------------------------------------------------------------------------
	if (type == CA_BONE)
	{
		if (szJointName == 0)
			return 0;

		CAttachmentBONE* pAttachment = new CAttachmentBONE();
		pAttachment->m_pAttachmentUpr = this;
		pAttachment->m_strSocketName = strAttachmentName.c_str();
		pAttachment->m_nSocketCRC32 = nameCRC;
		pAttachment->m_strJointName = szJointName;
		if (bCallProject)
			pAttachment->ProjectAttachment();
		m_arrAttachments.push_back(pAttachment);
		m_TypeSortingRequired++;
		return pAttachment;
	}

	if (type == CA_FACE)
	{
		CAttachmentFACE* pAttachment = new CAttachmentFACE();
		pAttachment->m_pAttachmentUpr = this;
		pAttachment->m_strSocketName = strAttachmentName.c_str();
		pAttachment->m_nSocketCRC32 = nameCRC;
		//	if( bCallProject )
		//		pAttachment->ProjectAttachment();
		m_arrAttachments.push_back(pAttachment);
		m_TypeSortingRequired++;
		return pAttachment;
	}

	if (type == CA_SKIN)
	{
		CAttachmentSKIN* pAttachment = new CAttachmentSKIN();
		pAttachment->m_pAttachmentUpr = this;
		pAttachment->m_strSocketName = strAttachmentName.c_str();
		pAttachment->m_nSocketCRC32 = nameCRC;
		m_arrAttachments.push_back(pAttachment);
		m_TypeSortingRequired++;
		return pAttachment;
	}

	if (type == CA_PROW)
	{
		if (szJointName == 0)
			return 0;
		CAttachmentPROW* pAttachment = new CAttachmentPROW();
		pAttachment->m_pAttachmentUpr = this;
		pAttachment->m_strSocketName = strAttachmentName.c_str();
		pAttachment->m_nSocketCRC32 = nameCRC;
		pAttachment->m_strRowJointName = szJointName;
		m_arrAttachments.push_back(pAttachment);
		m_TypeSortingRequired++;
		return pAttachment;
	}

	if (type == CA_VCLOTH)
	{
		CAttachmentVCLOTH* pAttachment = new CAttachmentVCLOTH();
		pAttachment->m_pAttachmentUpr = this;
		pAttachment->m_strSocketName = strAttachmentName.c_str();
		pAttachment->m_nSocketCRC32 = nameCRC;
		m_arrAttachments.push_back(pAttachment);
		m_TypeSortingRequired++;
		return pAttachment;
	}

	m_TypeSortingRequired++;
	return 0;
};

IAttachment* CAttachmentUpr::CreateVClothAttachment(const SVClothAttachmentParams& params)
{
	CAttachmentVCLOTH* pAttachmentVCloth = static_cast<CAttachmentVCLOTH*>(CreateAttachment(params.attachmentName.c_str(), CA_VCLOTH));
	if (!pAttachmentVCloth)
		return nullptr;

	const bool log = (params.skinLoadingFlags & CA_DisableLogWarnings) != 0;
	tukk renderMeshSkin = params.vclothParams.renderBinding.c_str();
	tukk simMeshSkin = params.vclothParams.simBinding.c_str();
	tukk pathName = GetSkelInstance()->GetFilePath();

	const bool isRenderMeshSkinFile = IsSkinFile(renderMeshSkin);
	if (!isRenderMeshSkinFile && log)
		g_pILog->LogError("DinrusXAnimation[VCloth]: a rendermesh (%s) for vertex-cloth must be a SKIN-file. You can't use this file: %s", renderMeshSkin, pathName);

	const bool isSimMeshSkinFile = IsSkinFile(simMeshSkin);
	if (!isSimMeshSkinFile && log)
		g_pILog->LogError("DinrusXAnimation[VCloth]: a simulation-mesh (%s) must be a SKIN-file. You can't use this file: %s", simMeshSkin, pathName);

	if (isRenderMeshSkinFile && isSimMeshSkinFile)
	{
		ISkin* pModelSKIN = g_pCharacterUpr->LoadModelSKIN(params.vclothParams.renderBinding.c_str(), params.skinLoadingFlags);
		if (!pModelSKIN && log)
		{
			g_pILog->LogError("DinrusXAnimation[VCloth]: skin-attachment not created: CDF: %s  SKIN: %s", pathName, renderMeshSkin);
		}

		ISkin* pModelSimSKIN = g_pCharacterUpr->LoadModelSKIN(params.vclothParams.simBinding.c_str(), params.skinLoadingFlags);
		if (!pModelSimSKIN && log)
		{
			g_pILog->LogError("DinrusXAnimation[VCloth]: skin-attachment not created: CDF: %s  SKIN: %s", pathName, simMeshSkin);
		}

		if (pModelSKIN && pModelSimSKIN)
		{
			CSKINAttachment* pSkinInstance = new CSKINAttachment();
			pSkinInstance->m_pIAttachmentSkin = pAttachmentVCloth;
			IAttachmentObject* pAttachmentObject = static_cast<IAttachmentObject*>(pSkinInstance);
			if (pAttachmentVCloth->Immediate_AddBinding(pAttachmentObject, pModelSKIN, params.skinLoadingFlags))
			{
				pAttachmentVCloth->SetFlags(params.flags | FLAGS_ATTACH_SW_SKINNING);
				pAttachmentVCloth->HideAttachment(params.flags & FLAGS_ATTACH_HIDE_ATTACHMENT);

				if (pAttachmentObject)
				{
					
					if (!params.vclothParams.material.empty())
					{
						if (IMaterial* pMaterial = g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(params.vclothParams.material.c_str(), false))
						{
							for (u32 nLOD = 0; nLOD < g_nMaxGeomLodLevels; ++nLOD)
							{
								pAttachmentObject->SetReplacementMaterial(pMaterial, nLOD);
							}
						}
					}
					
					for (u32 nLOD = 0; nLOD < g_nMaxGeomLodLevels; ++nLOD)
					{
						if (!params.vclothParams.materialLods[nLOD].empty())
						{
							if (IMaterial* pMaterial = params.vclothParams.material.empty() ? nullptr : g_pISystem->GetI3DEngine()->GetMaterialUpr()->LoadMaterial(params.vclothParams.material.c_str(), false))
							{
								pAttachmentObject->SetReplacementMaterial(pMaterial, nLOD);
							}
						}
					}

					if (const CModelMesh* pModelMesh = static_cast<CSkin*>(pModelSKIN)->GetModelMesh(0))
					{
						pAttachmentVCloth->m_vertexAnimation.CreateFrameStates(pModelMesh->m_softwareMesh.GetVertexFrames(), *(m_pSkelInstance->m_pDefaultSkeleton.get()));
					}
				}
				pAttachmentVCloth->AddSimBinding(*pModelSimSKIN, params.skinLoadingFlags);
				pAttachmentVCloth->AddClothParams(params.vclothParams);
				pAttachmentVCloth->ComputeClothCacheKey();

				m_pSkelInstance->SetHasVertexAnimation(true);
			}
		}
	}

	return pAttachmentVCloth;
}

ICharacterInstance* CAttachmentUpr::GetSkelInstance() const
{
	return m_pSkelInstance;
}

float CAttachmentUpr::GetExtent(EGeomForm eForm)
{
	CGeomExtent& extent = m_Extents.Make(eForm);

	// Add attachments.
	extent.Clear();
	extent.ReserveParts(m_arrAttachments.size());

	// Add attachments.
	for (const auto& pa : m_arrAttachments)
	{
		float fExt = 0.f;
		if (pa)
		{
			if (IAttachmentObject* pAttachmentObject = pa->GetIAttachmentObject())
			{
				if (ICharacterInstance* pSkinInstance = pAttachmentObject->GetICharacterInstance())
					fExt = pSkinInstance->GetExtent(eForm);
				else if (IStatObj* pStatObj = pAttachmentObject->GetIStatObj())
					fExt = pStatObj->GetExtent(eForm);
				else if (IAttachmentSkin* pSkin = pAttachmentObject->GetIAttachmentSkin())
					fExt = pSkin->GetExtent(eForm);
			}
		}
		extent.AddPart(fExt);
	}

	return extent.TotalExtent();
}

void CAttachmentUpr::GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const
{
	CGeomExtent const& ext = m_Extents[eForm];
	for (auto part : ext.RandomPartsAliasSum(points, seed))
	{
		if (part.iPart < m_arrAttachments.size())
		{
			// Choose attachment.
			if (IAttachment* pAttachment = m_arrAttachments[part.iPart])
			{
				if (IAttachmentObject* pAttachmentObject = pAttachment->GetIAttachmentObject())
				{
					if (ICharacterInstance* pCharInstance = pAttachmentObject->GetICharacterInstance())
						pCharInstance->GetRandomPoints(part.aPoints, seed, eForm);
					else if (IStatObj* pStatObj = pAttachmentObject->GetIStatObj())
						pStatObj->GetRandomPoints(part.aPoints, seed, eForm);
					else if (IAttachmentSkin* pSkin = pAttachmentObject->GetIAttachmentSkin())
						pSkin->GetRandomPoints(part.aPoints, seed, eForm);
				}
				for (auto& point : part.aPoints)
					point <<= QuatTS(pAttachment->GetAttModelRelative());
			}
		}
	}
}

void CAttachmentUpr::PhysicalizeAttachment(i32 idx, IPhysicalEntity* pent, i32 nLod)
{
	if (!pent)
		if (!(pent = m_pSkelInstance->GetISkeletonPose()->GetCharacterPhysics()))
			return;
	PhysicalizeAttachment(idx, nLod, pent, m_pSkelInstance->m_SkeletonPose.m_physics.GetOffset());
}

i32k idbit = ilog2(((FLAGS_ATTACH_ID_MASK ^ FLAGS_ATTACH_ID_MASK - 1) + 1) >> 1);           // bit shift for FLAG_ATTACH_ID_MASK

void CAttachmentUpr::PhysicalizeAttachment(i32 idx, i32 nLod, IPhysicalEntity* pent, const Vec3& offset)
{
	IStatObj* pStatObj;
	IAttachment* pIAttachment = GetInterfaceByIndex(idx);
	if (pIAttachment == 0)
		return;

	bool bWasPhysicalized = false;
	if ((pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED) == 0)
	{
		if (pIAttachment->GetIAttachmentObject())
		{
			bWasPhysicalized = pIAttachment->GetIAttachmentObject()->PhysicalizeAttachment(this, idx, nLod, pent, offset);
		}
	}
	//
	if (bWasPhysicalized)
		return;

	// old path
	if (!(pIAttachment = GetInterfaceByIndex(idx)) || pIAttachment->GetType() != CA_BONE || !(pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED) ||
	    !pIAttachment->GetIAttachmentObject() || !(pStatObj = pIAttachment->GetIAttachmentObject()->GetIStatObj()) ||
	    pIAttachment->IsAttachmentHidden())
		return;

	i32 iJoint = pIAttachment->GetJointID();
	pe_articgeomparams gp;
	const Skeleton::CPoseData& rPoseData = m_pSkelInstance->m_SkeletonPose.GetPoseData();
	Matrix34 mtx = Matrix34(rPoseData.GetJointAbsolute(iJoint) * ((CAttachmentFACE*)pIAttachment)->m_AttRelativeDefault);
	mtx.AddTranslation(offset);
	gp.pMtx3x4 = &mtx;
	//FIXME:
	gp.idbody = m_pSkelInstance->GetISkeletonPose()->getBonePhysParentOrSelfIndex(iJoint, nLod);
	if (gp.idbody >= 0)
		while ((iJoint = m_pSkelInstance->m_SkeletonPose.m_physics.getBonePhysParentIndex(gp.idbody, nLod)) >= 0 &&
		       (m_pSkelInstance->m_SkeletonPose.m_physics.GetModelJointPointer(gp.idbody)->m_PhysInfo.flags & all_angles_locked) == all_angles_locked)
			gp.idbody = iJoint;
	gp.flags = 0;
	if (pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED_COLLISIONS)
		gp.flags = geom_colltype_solid | geom_colltype_solid | geom_floats | geom_colltype_explosion;
	if (pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED_RAYS)
		gp.flags |= geom_colltype_ray;
	i32 id = (pIAttachment->GetFlags() & FLAGS_ATTACH_ID_MASK) >> idbit;
	if (!id)
	{
		id = ilog2(((m_physAttachIds ^ m_physAttachIds + 1) + 1) >> 1);               // least significant 0 bit index
		m_physAttachIds |= 1 << id;
		pIAttachment->SetFlags(pIAttachment->GetFlags() | id << idbit);
	}
	pStatObj->Physicalize(pent, &gp, m_pSkelInstance->m_pDefaultSkeleton->GetJointCount() + id);
	pIAttachment->SetFlags(pIAttachment->GetFlags() | FLAGS_ATTACH_WAS_PHYSICALIZED);
}

void CAttachmentUpr::DephysicalizeAttachment(i32 idx, IPhysicalEntity* pent)
{
	if (!pent)
		if (!(pent = m_pSkelInstance->GetISkeletonPose()->GetCharacterPhysics()))
			return;
	IAttachment* pIAttachment = GetInterfaceByIndex(idx);
	i32 id = (pIAttachment->GetFlags() & FLAGS_ATTACH_ID_MASK) >> idbit;
	if (id)
	{
		m_physAttachIds &= ~(1 << id);
		pIAttachment->SetFlags(pIAttachment->GetFlags() & ~FLAGS_ATTACH_ID_MASK);
	}
	pent->RemoveGeometry(m_pSkelInstance->m_pDefaultSkeleton->GetJointCount() + id);
	if (pIAttachment)
		pIAttachment->SetFlags(pIAttachment->GetFlags() & ~FLAGS_ATTACH_WAS_PHYSICALIZED);
}

i32 CAttachmentUpr::UpdatePhysicalizedAttachment(i32 idx, IPhysicalEntity* pent, const QuatT& offset)
{
	IStatObj* pStatObj;
	IAttachment* pIAttachment;

	bool WasHandled = false;
	if (!(pIAttachment = GetInterfaceByIndex(idx)) || !(pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED) ||
	    !pIAttachment->GetIAttachmentObject() /* || !(pStatObj=pIAttachment->GetIAttachmentObject()->GetIStatObj()) || !pStatObj->GetPhysGeom()*/)
	{
		if (pIAttachment && pIAttachment->GetIAttachmentObject())
		{
			WasHandled = pIAttachment->GetIAttachmentObject()->UpdatePhysicalizedAttachment(this, idx, pent, offset);
		}
	}

	if (WasHandled)
		return 0;

	if (!(pIAttachment = GetInterfaceByIndex(idx)) || pIAttachment->GetType() != CA_BONE || !(pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED) ||
	    !pIAttachment->GetIAttachmentObject() || !(pStatObj = pIAttachment->GetIAttachmentObject()->GetIStatObj()) || !pStatObj->GetPhysGeom())
		return 0;

	i32 changed = 0;
	if (pIAttachment->IsAttachmentHidden() && pIAttachment->GetFlags() & FLAGS_ATTACH_WAS_PHYSICALIZED)
		DephysicalizeAttachment(idx, pent), changed = 1;
	else if (!pIAttachment->IsAttachmentHidden() && !(pIAttachment->GetFlags() & FLAGS_ATTACH_WAS_PHYSICALIZED))
		PhysicalizeAttachment(idx, 0, pent, offset.t), changed = 1;

	pe_status_awake sa;
	if (!pent->GetStatus(&sa))
	{
		i32 iJoint = pIAttachment->GetJointID();
		pe_params_part pp;
		Matrix34 mtx = Matrix34(offset * pIAttachment->GetAttModelRelative());
		i32 id = (pIAttachment->GetFlags() & FLAGS_ATTACH_ID_MASK) >> idbit;
		pp.partid = m_pSkelInstance->m_pDefaultSkeleton->GetJointCount() + id;
		pp.pMtx3x4 = &mtx;
		const CDefaultSkeleton::SJoint* pJoint = m_pSkelInstance->m_SkeletonPose.m_physics.GetModelJointPointer(iJoint);
		pp.bRecalcBBox = !pJoint->m_PhysInfo.pPhysGeom;
		pent->SetParams(&pp);
	}
	return changed;
}

i32 CAttachmentUpr::UpdatePhysAttachmentHideState(i32 idx, IPhysicalEntity* pent, const Vec3& offset)
{
	IStatObj* pStatObj;
	IAttachment* pIAttachment;

	if (!(pIAttachment = GetInterfaceByIndex(idx)) || pIAttachment->GetType() != CA_BONE || !(pIAttachment->GetFlags() & FLAGS_ATTACH_PHYSICALIZED) ||
	    !pIAttachment->GetIAttachmentObject() || !(pStatObj = pIAttachment->GetIAttachmentObject()->GetIStatObj()) || !pStatObj->GetPhysGeom())
		return 0;

	if (pIAttachment->IsAttachmentHidden() && pIAttachment->GetFlags() & FLAGS_ATTACH_WAS_PHYSICALIZED)
	{
		DephysicalizeAttachment(idx, pent);
		return 2;
	}
	if (!pIAttachment->IsAttachmentHidden() && !(pIAttachment->GetFlags() & FLAGS_ATTACH_WAS_PHYSICALIZED))
	{
		PhysicalizeAttachment(idx, 0, pent, offset);
		return 1;
	}
	return 3;
}

i32 CAttachmentUpr::RemoveAttachmentByInterface(const IAttachment* pAttachment, u32 loadingFlags)
{
	return RemoveAttachmentByName(pAttachment->GetName(), loadingFlags);
}

i32 CAttachmentUpr::RemoveAttachmentByName(tukk szName, u32 loadingFlags)
{
	i32k index = GetIndexByName(szName);
	if (index == -1)
	{
		return 0;
	}

	RemoveAttachmentByIndex(index, loadingFlags);
	return 1;
};

i32 CAttachmentUpr::RemoveAttachmentByNameCRC(u32 nameCRC, u32 loadingFlags)
{
	i32k index = GetIndexByNameCRC(nameCRC);
	if (index == -1)
	{
		return 0;
	}

	RemoveAttachmentByIndex(index, loadingFlags);
	return 1;
};

void CAttachmentUpr::RemoveAttachmentByIndex(u32 index, u32 loadingFlags)
{
	IAttachment* const pAttachment = GetInterfaceByIndex(index);
	assert(pAttachment);
	assert(pAttachment->GetType() == CA_BONE || pAttachment->GetType() == CA_FACE || pAttachment->GetType() == CA_SKIN || pAttachment->GetType() == CA_PROW || pAttachment->GetType() == CA_VCLOTH);

	if (pAttachment->GetIAttachmentObject())
	{
		pAttachment->GetIAttachmentObject()->OnRemoveAttachment(this, index);
	}

	for (auto && pMergedAttachment : m_mergedAttachments)
	{
		if (pMergedAttachment->HasAttachment(pAttachment))
		{
			pMergedAttachment->Invalidate();
			break; // TODO: Why is there a break here?
		}
	}

	pAttachment->ClearBinding(loadingFlags);
	m_arrAttachments.erase(index);

	m_Extents.Clear();
	m_TypeSortingRequired++;
}

u32 CAttachmentUpr::ProjectAllAttachment()
{
	u32 numAttachments = m_arrAttachments.size();
	for (u32 i = 0; i < numAttachments; i++)
	{
		u32 flags = m_arrAttachments[i]->GetFlags();
		flags &= (~FLAGS_ATTACH_PROJECTED);
		m_arrAttachments[i]->SetFlags(flags);
	}
	return 1;
}

IAttachment* CAttachmentUpr::GetInterfaceByName(tukk szName) const
{
	i32 idx = GetIndexByName(szName);
	if (idx == -1) return 0;
	return GetInterfaceByIndex(idx);
};

IAttachment* CAttachmentUpr::GetInterfaceByIndex(u32 c) const
{
	size_t size = m_arrAttachments.size();
	if (size == 0) return 0;
	if (size <= c) return 0;
	return m_arrAttachments[c];
};

i32 CAttachmentUpr::GetIndexByName(tukk szName) const
{
	u32k nameCRC = CCrc32::ComputeLowercase(szName);
	return GetIndexByNameCRC(nameCRC);
}

IAttachment* CAttachmentUpr::GetInterfaceByNameCRC(u32 nameCRC) const
{
	i32 idx = GetIndexByNameCRC(nameCRC);
	if (idx == -1) return 0;
	return GetInterfaceByIndex(idx);
};

i32 CAttachmentUpr::GetIndexByNameCRC(u32 nameCRC) const
{
	i32k num = GetAttachmentCount();
	for (i32 i = 0; i < num; i++)
	{
		const IAttachment* pA = m_arrAttachments[i];
		if (pA->GetNameCRC() == nameCRC)
			return i;
	}
	return -1;
}

IAttachment* CAttachmentUpr::GetInterfaceByPhysId(i32 id) const
{
	if ((id -= m_pSkelInstance->m_pDefaultSkeleton->GetJointCount()) < 0)
		return 0;
	for (i32 i = GetAttachmentCount() - 1; i >= 0; i--)
	{
		if ((m_arrAttachments[i]->GetFlags() & FLAGS_ATTACH_ID_MASK) >> idbit == id)
			return GetInterfaceByIndex(i);
	}
	return 0;
}

bool CAttachmentUpr::NeedsHierarchicalUpdate()
{
	u32k numAttachments = GetAttachmentCount();
	for (u32 i = 0; i < numAttachments; i++)
	{
		IAttachmentObject* pIAttachmentObject = m_arrAttachments[i]->GetIAttachmentObject();
		if (pIAttachmentObject == 0)
			continue;

		ICharacterInstance* pICharacterInstance = pIAttachmentObject->GetICharacterInstance();
		if (pICharacterInstance == 0)
			continue;

		IAnimationSet* pIAnimationSet = pICharacterInstance->GetIAnimationSet();
		u32 numAnimations = pIAnimationSet->GetAnimationCount();
		if (numAnimations == 0)
			continue;

		ISkeletonAnim* pISkeletonAnim = pICharacterInstance->GetISkeletonAnim();
		u32 numAnims = 0;
		for (u32 l = 0; l < numVIRTUALLAYERS; l++)
		{
			numAnims += pISkeletonAnim->GetNumAnimsInFIFO(l);
		}
		if (numAnims)
			return 1;
	}

	return 0;
}

void CAttachmentUpr::UpdateAllRemapTables()
{
	if (m_TypeSortingRequired)
		SortByType();

	for (u32 r = 0; r < m_sortedRanges[eRange_BoneExecuteUnsafe].end; r++)
	{
		IAttachment* pIAttachment = m_arrAttachments[r];
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		pCAttachmentBone->m_nJointID = -1;   //request re-projection
		pCAttachmentBone->m_Simulation.m_arrChildren.resize(0);
	}
	u32 x = FLAGS_ATTACH_PROJECTED ^ -1;
	for (u32 i = m_sortedRanges[0].begin; i < m_sortedRanges[eRange_SkinMesh].end; i++)
	{
		m_arrAttachments[i]->SetFlags(m_arrAttachments[i]->GetFlags() & x);
	}
	for (u32 i = m_sortedRanges[eRange_SkinMesh].begin; i < m_sortedRanges[eRange_SkinMesh].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentSKIN* pCAttachmentSkin = (CAttachmentSKIN*)pIAttachment;
		pCAttachmentSkin->UpdateRemapTable();
	}
}

void CAttachmentUpr::VerifyProxyLinks()
{
	//verify and re-adjust proxy links
	u32 numAttachmnets = GetAttachmentCount();
	for (u32 a = 0; a < numAttachmnets; a++)
	{
		IAttachment* pIAttachment = GetInterfaceByIndex(a);

		if (pIAttachment->GetType() == CA_BONE)
		{
			CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
			u32 numUsedProxies = pCAttachmentBone->m_Simulation.m_arrProxyNames.size();
			if (numUsedProxies == 0)
				continue;

			string arrProxyNames[100];
			for (u32 p = 0; p < numUsedProxies; p++)
			{
				arrProxyNames[p] = pCAttachmentBone->m_Simulation.m_arrProxyNames[p].c_str();
			}

			pCAttachmentBone->m_Simulation.m_arrProxyNames.resize(0);
			pCAttachmentBone->m_Simulation.m_arrProxyIndex.resize(0);
			u32 numAllProxies = m_arrProxies.size();
			for (u32 crc = 0; crc < numUsedProxies; crc++)
			{
				u32 nCRC32lower = CCrc32::ComputeLowercase(arrProxyNames[crc].c_str());
				for (u32 p = 0; p < numAllProxies; p++)
				{
					if (nCRC32lower == m_arrProxies[p].m_nProxyCRC32)
					{
						pCAttachmentBone->m_Simulation.m_arrProxyNames.push_back(CDrxName(arrProxyNames[crc]));
						pCAttachmentBone->m_Simulation.m_arrProxyIndex.push_back(p);
						break;
					}
				}
			}
		}

		if (pIAttachment->GetType() == CA_FACE)
		{
			CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
			u32 numUsedProxies = pCAttachmentFace->m_Simulation.m_arrProxyNames.size();
			if (numUsedProxies == 0)
				continue;

			string arrProxyNames[100];
			for (u32 p = 0; p < numUsedProxies; p++)
			{
				arrProxyNames[p] = pCAttachmentFace->m_Simulation.m_arrProxyNames[p].c_str();
			}

			pCAttachmentFace->m_Simulation.m_arrProxyNames.resize(0);
			pCAttachmentFace->m_Simulation.m_arrProxyIndex.resize(0);
			u32 numAllProxies = m_arrProxies.size();
			for (u32 crc = 0; crc < numUsedProxies; crc++)
			{
				u32 nCRC32lower = CCrc32::ComputeLowercase(arrProxyNames[crc].c_str());
				for (u32 p = 0; p < numAllProxies; p++)
				{
					if (nCRC32lower == m_arrProxies[p].m_nProxyCRC32)
					{
						pCAttachmentFace->m_Simulation.m_arrProxyNames.push_back(CDrxName(arrProxyNames[crc]));
						pCAttachmentFace->m_Simulation.m_arrProxyIndex.push_back(p);
						break;
					}
				}
			}
		}

	}

	const CDefaultSkeleton& rDefaultSkeleton = *m_pSkelInstance->m_pDefaultSkeleton;
	u32 numProxies = m_arrProxies.size();
	for (u32 i = 0; i < numProxies; i++)
	{
		tukk strJointName = m_arrProxies[i].m_strJointName.c_str();
		i16 nJointID = rDefaultSkeleton.GetJointIDByName(strJointName);
		if (nJointID < 0)
		{
			DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Proxy '%s' specified wrong joint name '%s'", m_arrProxies[i].m_strProxyName.c_str(), strJointName);
			m_arrProxies.erase(m_arrProxies.begin() + i);
			numProxies = m_arrProxies.size();
			--i;
			continue;
		}
		m_arrProxies[i].m_nJointID = nJointID;
		QuatT jointQT = rDefaultSkeleton.GetDefaultAbsJointByID(nJointID);
		m_arrProxies[i].m_ProxyRelativeDefault = jointQT.GetInverted() * m_arrProxies[i].m_ProxyAbsoluteDefault;
	}
}

void CAttachmentUpr::PrepareAllRedirectedTransformations(Skeleton::CPoseData& rPoseData)
{
	const f32 fIPlaybackScale = m_pSkelInstance->GetPlaybackScale();
	const f32 fLPlaybackScale = m_pSkelInstance->m_SkeletonAnim.GetLayerPlaybackScale(0);
	const f32 fAverageFrameTime = g_AverageFrameTime * fIPlaybackScale * fLPlaybackScale ? g_AverageFrameTime * fIPlaybackScale * fLPlaybackScale : g_AverageFrameTime;
	m_fTurbulenceGlobal += gf_PI * fAverageFrameTime, m_fTurbulenceLocal = 0;

	DEFINE_PROFILER_FUNCTION();
	const QuatTS& rPhysLocation = m_pSkelInstance->m_location;
	u32 nproxies = m_arrProxies.size();
	for (u32 i = 0; i < nproxies; i++)
	{
		i32 idx = m_arrProxies[i].m_nJointID;
		m_arrProxies[i].m_ProxyModelRelative = rPoseData.GetJointAbsolute(idx) * m_arrProxies[i].m_ProxyRelativeDefault;
#ifndef _RELEASE
		if (m_pSkelInstance->m_CharEditMode == 3)
		{
			const Vec3 pos = rPoseData.GetJointAbsolute(idx).t;
			g_pAuxGeom->DrawLine(rPhysLocation * pos, RGBA8(0xff, 0x00, 0x00, 0xff), rPhysLocation * m_arrProxies[i].m_ProxyModelRelative.t, RGBA8(0x00, 0xff, 0x00, 0xff));
		}
#endif
	}

#ifdef EDITOR_PCDEBUGCODE
	if (m_nDrawProxies)
	{
		for (u32 i = 0; i < nproxies; i++)
		{
			QuatTS wlocation = rPhysLocation * m_arrProxies[i].m_ProxyModelRelative;
			if (m_nDrawProxies & 2 && m_arrProxies[i].m_nHideProxy == 0 && m_arrProxies[i].m_nPurpose == 0)
				m_arrProxies[i].Draw(wlocation, RGBA8(0xe7, 0xc0, 0xbc, 0xff), 16, m_pSkelInstance->m_Viewdir);
			if (m_nDrawProxies & 4 && m_arrProxies[i].m_nHideProxy == 0 && m_arrProxies[i].m_nPurpose == 1)
				m_arrProxies[i].Draw(wlocation, RGBA8(0xa0, 0xe7, 0x80, 0xff), 16, m_pSkelInstance->m_Viewdir);
			if (m_nDrawProxies & 8 && m_arrProxies[i].m_nHideProxy == 0 && m_arrProxies[i].m_nPurpose == 2)
				m_arrProxies[i].Draw(wlocation, RGBA8(0xff, 0xa0, 0x80, 0xff), 16, m_pSkelInstance->m_Viewdir);
		}
	}
#endif

#ifdef EDITOR_PCDEBUGCODE
	//Verification();
#endif
}

void CAttachmentUpr::GenerateProxyModelRelativeTransformations(Skeleton::CPoseData& rPoseData)
{
	const QuatTS& rPhysLocation = m_pSkelInstance->m_location;
	u32 nproxies = m_arrProxies.size();
	for (u32 i = 0; i < nproxies; i++)
	{
		m_arrProxies[i].m_ProxyModelRelativePrev = m_arrProxies[i].m_ProxyModelRelative;
		m_arrProxies[i].m_ProxyModelRelativePrev.q.Normalize();
	}
}

void CAttachmentUpr::CreateCommands(Command::CBuffer& buffer)
{
	if (m_TypeSortingRequired)
		SortByType();

	buffer.CreateCommand<Command::PrepareAllRedirectedTransformations>();

	u32 m = GetRedirectedJointCount();
	for (u32 i = 0; i < m; i++)
	{
		Command::UpdateRedirectedJoint* cmd = buffer.CreateCommand<Command::UpdateRedirectedJoint>();
		cmd->m_attachmentBone = static_cast<CAttachmentBONE*>(m_arrAttachments[i].get());
	}

	buffer.CreateCommand<Command::GenerateProxyModelRelativeTransformations>();

	for (u32 i = m_sortedRanges[eRange_VertexClothOrPendulumRow].begin; i < m_sortedRanges[eRange_VertexClothOrPendulumRow].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_PROW)
			continue;
		Command::UpdatePendulumRow* cmd = buffer.CreateCommand<Command::UpdatePendulumRow>();
		cmd->m_attachmentPendulumRow = static_cast<CAttachmentPROW*>(pIAttachment);
	}
}

i32 CAttachmentUpr::GenerateAttachedInstanceContexts()
{
	if (m_TypeSortingRequired)
		SortByType();

	auto& sc = g_pCharacterUpr->GetContextSyncQueue();

	i32 result = 0;

	u32 m =
	  m_sortedRanges[eRange_BoneExecuteUnsafe].end - m_sortedRanges[eRange_BoneExecute].begin;

	for (u32 i = m_sortedRanges[eRange_BoneExecute].begin;
	     i < m_sortedRanges[eRange_BoneExecuteUnsafe].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentBONE* pCAttachmentBone = static_cast<CAttachmentBONE*>(pIAttachment);
		if (pCAttachmentBone->m_nJointID < 0)
			continue;
		IAttachmentObject* pIAttachmentObject = pCAttachmentBone->m_pIAttachmentObject;
		DRX_ASSERT(pIAttachmentObject != nullptr);
		if (pIAttachmentObject->GetAttachmentType() == IAttachmentObject::eAttachment_Skeleton)
		{
			CCharInstance* pChildInstance =
			  static_cast<CCharInstance*>(pIAttachmentObject->GetICharacterInstance());
			if (pChildInstance)
			{
				CharacterInstanceProcessing::CContextQueue& queue = g_pCharacterUpr->GetContextSyncQueue();
				CharacterInstanceProcessing::SContext& ctx = queue.AppendContext();
				result += 1;
				i32 numChildren = pChildInstance->m_AttachmentUpr.GenerateAttachedInstanceContexts();
				result += numChildren;
				ctx.Initialize(pChildInstance, pCAttachmentBone, m_pSkelInstance, numChildren);
			}
		}
	}
	return result;
}

void CAttachmentUpr::UpdateLocationsExceptExecute(Skeleton::CPoseData& rPoseData)
{
	if (m_TypeSortingRequired)
		SortByType();

	for (u32 i = m_sortedRanges[eRange_BoneEmpty].begin; i < m_sortedRanges[eRange_BoneEmpty].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		pCAttachmentBone->Update_Empty(rPoseData);
	}
	for (u32 i = m_sortedRanges[eRange_BoneStatic].begin; i < m_sortedRanges[eRange_BoneStatic].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		pCAttachmentBone->Update_Static(rPoseData);
	}

	if (m_pSkelInstance->GetAnimationLOD() < 1)
	{
		for (u32 i = m_sortedRanges[eRange_FaceEmpty].begin; i < m_sortedRanges[eRange_FaceEmpty].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
			pCAttachmentFace->Update_Empty(rPoseData);
		}
	}

	for (u32 i = m_sortedRanges[eRange_FaceStatic].begin; i < m_sortedRanges[eRange_FaceStatic].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
		pCAttachmentFace->Update_Static(rPoseData);
	}
}

void CAttachmentUpr::UpdateLocationsExecute(Skeleton::CPoseData& rPoseData)
{
	if (m_TypeSortingRequired)
		SortByType();

	for (u32 i = m_sortedRanges[eRange_BoneExecute].begin; i < m_sortedRanges[eRange_BoneExecute].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentBONE* pCAttachmentBone = static_cast<CAttachmentBONE*>(pIAttachment);
		pCAttachmentBone->Update_Execute(rPoseData);
	}

	for (u32 i = m_sortedRanges[eRange_FaceExecute].begin; i < m_sortedRanges[eRange_FaceExecute].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
		pCAttachmentFace->Update_Execute(rPoseData);
	}
}

void CAttachmentUpr::UpdateLocationsExecuteUnsafe(Skeleton::CPoseData& rPoseData)
{
	if (m_TypeSortingRequired)
		SortByType();

	for (u32 i = m_sortedRanges[eRange_BoneExecuteUnsafe].begin; i < m_sortedRanges[eRange_BoneExecuteUnsafe].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		pCAttachmentBone->Update_Execute(rPoseData);
	}

	for (u32 i = m_sortedRanges[eRange_FaceExecuteUnsafe].begin; i < m_sortedRanges[eRange_FaceExecuteUnsafe].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
		pCAttachmentFace->Update_Execute(rPoseData);
	}

}

u32 CAttachmentUpr::RemoveAllAttachments()
{
	u32k loadingFlags = CA_SkipSkelRecreation | CA_ImmediateMode | (m_pSkelInstance->m_CharEditMode ? CA_CharEditModel : 0);

	u32k counter = GetAttachmentCount();
	for (u32 i = 0; i < counter; i++)
	{
		m_arrAttachments[i]->ClearBinding(loadingFlags);
	}
	for (u32 i = counter; i > 0; i--)
	{
		RemoveAttachmentByIndex(i - 1, loadingFlags);
	}

	return 1;
}

void CAttachmentUpr::Serialize(TSerialize ser)
{
	if (ser.GetSerializationTarget() != eST_Network)
	{
		ser.BeginGroup("CAttachmentUpr");
		i32 numAttachments = GetAttachmentCount();
		for (i32 i = 0; i < numAttachments; i++)
		{
			GetInterfaceByIndex(i)->Serialize(ser);
		}
		ser.EndGroup();
	}
}

#if !defined(_RELEASE)
void VisualizeEmptySocket(const Matrix34& WorldMat34, const QuatT& rAttModelRelative, ColorB col)
{
	g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
	Matrix34 FinalMat34 = WorldMat34 * Matrix34(rAttModelRelative);
	Vec3 pos = FinalMat34.GetTranslation();
	static Ang3 angle1(0, 0, 0);
	static Ang3 angle2(0, 0, 0);
	static Ang3 angle3(0, 0, 0);
	angle1 += Ang3(0.01f, +0.02f, +0.03f);
	angle2 -= Ang3(0.01f, -0.02f, -0.03f);
	angle3 += Ang3(0.03f, -0.02f, +0.01f);

	AABB aabb1 = AABB(Vec3(-0.05f, -0.05f, -0.05f), Vec3(+0.05f, +0.05f, +0.05f));
	AABB aabb2 = AABB(Vec3(-0.005f, -0.005f, -0.005f), Vec3(+0.005f, +0.005f, +0.005f));

	Matrix33 m33;
	OBB obb;
	m33 = Matrix33::CreateRotationXYZ(angle1);
	obb = OBB::CreateOBBfromAABB(m33, aabb1);
	g_pAuxGeom->DrawOBB(obb, pos, 0, col, eBBD_Extremes_Color_Encoded);
	obb = OBB::CreateOBBfromAABB(m33, aabb1);
	g_pAuxGeom->DrawOBB(obb, pos, 0, col, eBBD_Extremes_Color_Encoded);

	m33 = Matrix33::CreateRotationXYZ(angle2);
	obb = OBB::CreateOBBfromAABB(m33, aabb1);
	g_pAuxGeom->DrawOBB(obb, pos, 0, col, eBBD_Extremes_Color_Encoded);
	obb = OBB::CreateOBBfromAABB(m33, aabb2);
	g_pAuxGeom->DrawOBB(obb, pos, 0, col, eBBD_Extremes_Color_Encoded);

	m33 = Matrix33::CreateRotationXYZ(angle3);
	obb = OBB::CreateOBBfromAABB(m33, aabb1);
	g_pAuxGeom->DrawOBB(obb, pos, 0, col, eBBD_Extremes_Color_Encoded);
	obb = OBB::CreateOBBfromAABB(m33, aabb2);
	g_pAuxGeom->DrawOBB(obb, pos, 0, col, eBBD_Extremes_Color_Encoded);

	obb = OBB::CreateOBBfromAABB(rAttModelRelative.q, aabb1);
	g_pAuxGeom->DrawOBB(obb, pos, 0, RGBA8(0xff, 0xff, 0xff, 0xff), eBBD_Extremes_Color_Encoded);
}
#endif

void CAttachmentUpr::DrawAttachments(SRendParams& rParams, const Matrix34& rWorldMat34, const SRenderingPassInfo& passInfo, const f32 fZoomFactor, const f32 fZoomDistanceSq)
{
	DEFINE_PROFILER_FUNCTION();

	u32k numAttachments = m_arrAttachments.size();
	if (numAttachments == 0)
		return;

	if (m_TypeSortingRequired)
		SortByType();

#if !defined(_RELEASE)
	g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
	if (Console::GetInst().ca_DrawAttachments == 0)
		return;
#endif

	u32 InRecursion = passInfo.IsRecursivePass();
	const bool InShadow = passInfo.IsShadowPass();

	//Only save the zoom sq distance if not called from recursive render call
	//this is because zoom sq distance is used to cull attachments and zoom is modified for recursive calls (see e_RecursionViewDistRatio)
	if (InRecursion == 0)
		m_fZoomDistanceSq = fZoomDistanceSq;

	u32 uHideFlags = 0;
	if (InRecursion > 1)
		uHideFlags |= FLAGS_ATTACH_HIDE_RECURSION;
	else if (InShadow)
		uHideFlags |= FLAGS_ATTACH_HIDE_SHADOW_PASS;
	else
		uHideFlags |= FLAGS_ATTACH_HIDE_MAIN_PASS;

	const bool bDrawMergedAttachments = Console::GetInst().ca_DrawAttachmentsMergedForShadows != 0;
	const bool bDrawNearest = (rParams.dwFObjFlags & FOB_NEAREST) != 0;

	{
		LOADING_TIME_PROFILE_SECTION_NAMED("BoneAttachments");
		if (m_numRedirectionWithAttachment)
		{
			for (u32 i = 0; i < m_sortedRanges[eRange_BoneRedirect].end; i++)
			{
				IAttachment* pIAttachment = m_arrAttachments[i];
				CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
				IAttachmentObject* pIAttachmentObject = pCAttachmentBone->m_pIAttachmentObject;

				if ((pIAttachment->GetFlags() & FLAGS_ATTACH_EXCLUDE_FROM_NEAREST) != 0)
				{
					rParams.dwFObjFlags &= ~FOB_NEAREST;
				}
				else if (bDrawNearest)
				{
					rParams.dwFObjFlags |= FOB_NEAREST;
				}

				if (pIAttachmentObject == 0)
					continue;              //most likely all of them are 0
				if (pCAttachmentBone->m_AttFlags & uHideFlags)
					continue;
				if (pCAttachmentBone->m_nJointID < 0)
					continue;              //No success! Maybe next time
				Matrix34 FinalMat34 = (((rParams.dwFObjFlags & FOB_NEAREST) != 0) ? *rParams.pNearestMatrix : rWorldMat34) * Matrix34(pCAttachmentBone->m_AttModelRelative * pCAttachmentBone->m_addTransformation);
				rParams.pMatrix = &FinalMat34;
				pIAttachmentObject->RenderAttachment(rParams, passInfo);
			}
		}

		for (u32 i = m_sortedRanges[eRange_BoneStatic].begin; i < m_sortedRanges[eRange_BoneExecuteUnsafe].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;

			if ((pIAttachment->GetFlags() & FLAGS_ATTACH_EXCLUDE_FROM_NEAREST) != 0)
			{
				rParams.dwFObjFlags &= ~FOB_NEAREST;
			}
			else if (bDrawNearest)
			{
				rParams.dwFObjFlags |= FOB_NEAREST;
			}

			if (pCAttachmentBone->m_AttFlags & uHideFlags)
				continue;
			if (pCAttachmentBone->m_nJointID < 0)
				continue;                //No success! Maybe next time
			if (bDrawMergedAttachments && (pIAttachment->GetFlags() & FLAGS_ATTACH_MERGED_FOR_SHADOWS) && (passInfo.IsShadowPass()))
				continue;
			if (!(rParams.nCustomFlags & COB_POST_3D_RENDER) && (pCAttachmentBone->m_AttFlags & FLAGS_ATTACH_VISIBLE) == 0)
				continue;
			Matrix34 FinalMat34 = (((rParams.dwFObjFlags & FOB_NEAREST) != 0) ? *rParams.pNearestMatrix : rWorldMat34) * Matrix34(pCAttachmentBone->m_AttModelRelative * pCAttachmentBone->m_addTransformation);
			rParams.pMatrix = &FinalMat34;
			 
			// store rParams.pNearestMatrix
			Matrix34* pNearestMatrixOld = rParams.pNearestMatrix;

			// propagate relative transformations in pNearestMatrix 
			if (rParams.pNearestMatrix)
			{
				rParams.pNearestMatrix = &FinalMat34;
			}

			pCAttachmentBone->m_pIAttachmentObject->RenderAttachment(rParams, passInfo);

			// restore rParams.pNearestMatrix
			rParams.pNearestMatrix = pNearestMatrixOld;
		}
	}

	{
		LOADING_TIME_PROFILE_SECTION_NAMED("FaceAttachments");
		for (u32 i = m_sortedRanges[eRange_FaceStatic].begin; i < m_sortedRanges[eRange_FaceExecute].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
			if (pCAttachmentFace->m_AttFlags & uHideFlags)
				continue;
			if ((pCAttachmentFace->m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
				continue;                //no success! maybe next time
			if (bDrawMergedAttachments && (pIAttachment->GetFlags() & FLAGS_ATTACH_MERGED_FOR_SHADOWS) && (passInfo.IsShadowPass()))
				continue;
			if (!(rParams.nCustomFlags & COB_POST_3D_RENDER) && (pCAttachmentFace->m_AttFlags & FLAGS_ATTACH_VISIBLE) == 0)
				continue;                //Distance culling. Object is too small for rendering
			Matrix34 FinalMat34 = (((rParams.dwFObjFlags & FOB_NEAREST) != 0) ? *rParams.pNearestMatrix : rWorldMat34) * Matrix34(pCAttachmentFace->m_AttModelRelative * pCAttachmentFace->m_addTransformation);
			rParams.pMatrix = &FinalMat34;
			pCAttachmentFace->m_pIAttachmentObject->RenderAttachment(rParams, passInfo);
		}
	}
#if !defined(_RELEASE)
	// for debugdrawing - drawOffset prevents attachments from overdrawing each other, drawScale helps scaling depending on attachment count
	// used for Skin and Cloth Attachments for now
	static ICVar* p_e_debug_draw = gEnv->pConsole->GetCVar("e_DebugDraw");
	float drawOffset = 0.0f;
	float debugDrawScale = 1.0f;

	if (p_e_debug_draw->GetIVal() == 20)
	{
		i32 attachmentCount = m_sortedRanges[eRange_SkinMesh].GetNumElements();
		attachmentCount += m_sortedRanges[eRange_VertexClothOrPendulumRow].GetNumElements();
		debugDrawScale = 0.5f + (10.0f / max(1.0f, (float)attachmentCount));
		debugDrawScale = min(debugDrawScale, 1.5f);
	}
#endif
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("SkinAttachments");
		for (u32 i = m_sortedRanges[eRange_SkinMesh].begin; i < m_sortedRanges[eRange_SkinMesh].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
	
			if ((pIAttachment->GetFlags() & FLAGS_ATTACH_EXCLUDE_FROM_NEAREST) != 0)
			{
				rParams.dwFObjFlags &= ~FOB_NEAREST;
			}			
			else if (bDrawNearest)
			{
				rParams.dwFObjFlags |= FOB_NEAREST;		
			}

			CAttachmentSKIN* pCAttachmentSkin = (CAttachmentSKIN*)pIAttachment;
			if (pCAttachmentSkin->m_AttFlags & uHideFlags)
				continue;
			if (bDrawMergedAttachments && (pIAttachment->GetFlags() & FLAGS_ATTACH_MERGED_FOR_SHADOWS) && passInfo.IsShadowPass())
				continue;
			if (pCAttachmentSkin->m_pIAttachmentObject == 0)
				continue;
			u32 nDrawModel = m_pSkelInstance->m_rpFlags & CS_FLAG_DRAW_MODEL;
			if (nDrawModel == 0)
				continue;
			const f32 fRadiusSqr = m_pSkelInstance->m_SkeletonPose.GetAABB().GetRadiusSqr();
			if (fRadiusSqr == 0.0f)
				continue;  //if radius is zero, then the object is most probably not visible and we can continue
			if (!(rParams.nCustomFlags & COB_POST_3D_RENDER) && fZoomDistanceSq > fRadiusSqr)
				continue;  //too small to render. cancel the update
			
			pCAttachmentSkin->DrawAttachment(rParams, passInfo, ((rParams.dwFObjFlags & FOB_NEAREST) != 0) ? *rParams.pNearestMatrix : rWorldMat34, fZoomFactor);

#if !defined(_RELEASE)
			// pMaterial is set to NULL above, but restored in DrawAttachment with correct material
			if (p_e_debug_draw->GetIVal() == 20)
			{
				Vec3 drawLoc = rWorldMat34.GetTranslation();
				drawLoc.z += drawOffset;
				drawOffset += DebugDrawAttachment(pCAttachmentSkin, pCAttachmentSkin->GetISkin(), drawLoc, rParams.pMaterial, debugDrawScale,passInfo);
			}
#endif
		}
	}
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("VertexClothAttachments");
		for (u32 i = m_sortedRanges[eRange_VertexClothOrPendulumRow].begin; i < m_sortedRanges[eRange_VertexClothOrPendulumRow].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			if (pIAttachment->GetType() != CA_VCLOTH)
				continue;
			CAttachmentVCLOTH* pCAttachmentVCloth = (CAttachmentVCLOTH*)pIAttachment;
			if (pCAttachmentVCloth->m_AttFlags & uHideFlags)
				continue;
			if (pCAttachmentVCloth->m_pIAttachmentObject == 0)
				continue;
			u32 nDrawModel = m_pSkelInstance->m_rpFlags & CS_FLAG_DRAW_MODEL;
			if (nDrawModel == 0)
				continue;
			const f32 fRadiusSqr = m_pSkelInstance->m_SkeletonPose.GetAABB().GetRadiusSqr();
			if (fRadiusSqr == 0.0f)
				continue;   //if radius is zero, then the object is most probably not visible and we can continue
			if (!(rParams.nCustomFlags & COB_POST_3D_RENDER) && fZoomDistanceSq > fRadiusSqr)
				continue;   //too small to render. cancel the update

			pCAttachmentVCloth->InitializeCloth();
			pCAttachmentVCloth->DrawAttachment(rParams, passInfo, ((rParams.dwFObjFlags & FOB_NEAREST) != 0) ? *rParams.pNearestMatrix : rWorldMat34, fZoomFactor);

#if !defined(_RELEASE)
			if (p_e_debug_draw->GetIVal() == 20)
			{
				Vec3 drawLoc = rWorldMat34.GetTranslation();
				drawLoc.z += drawOffset;
				drawOffset += DebugDrawAttachment(pCAttachmentVCloth, pCAttachmentVCloth->GetISkin(), drawLoc, rParams.pMaterial, debugDrawScale,passInfo);
			}
#endif
		}
	}
#if !defined(_RELEASE)
	if (Console::GetInst().ca_DrawEmptyAttachments)
	{
		for (u32 i = m_sortedRanges[eRange_BoneEmpty].begin; i < m_sortedRanges[eRange_BoneEmpty].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
			if (pCAttachmentBone->m_AttFlags & uHideFlags)
				continue;
			if (pCAttachmentBone->m_nJointID < 0)
				return;                                               //no success! maybe next time
			VisualizeEmptySocket(rWorldMat34, QuatT(pCAttachmentBone->m_AttModelRelative.q, pCAttachmentBone->m_AttModelRelative.t), RGBA8(0xff, 0x00, 0x1f, 0xff));
		}
		for (u32 i = m_sortedRanges[eRange_FaceEmpty].begin; i < m_sortedRanges[eRange_FaceEmpty].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
			if (pCAttachmentFace->m_AttFlags & uHideFlags)
				continue;
			if ((pCAttachmentFace->m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
				return;                                               //no success! maybe next time
			VisualizeEmptySocket(rWorldMat34, pCAttachmentFace->m_AttModelRelative, RGBA8(0x1f, 0x00, 0xff, 0xff));
		}
	}

	if (Console::GetInst().ca_DrawAttachmentOBB)
	{
		g_pAuxGeom->SetRenderFlags(e_Def3DPublicRenderflags);
		for (u32 i = m_sortedRanges[eRange_BoneStatic].begin; i < m_sortedRanges[eRange_BoneExecuteUnsafe].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
			if (pCAttachmentBone->m_AttFlags & uHideFlags)
				continue;
			if (pCAttachmentBone->m_nJointID < 0)
				continue;                                             //no success! maybe next time
			if (!(rParams.nCustomFlags & COB_POST_3D_RENDER) && (pCAttachmentBone->m_AttFlags & FLAGS_ATTACH_VISIBLE) == 0)
				continue;                                             //Distance culling. Object is too small for rendering
			Matrix34 FinalMat34 = rWorldMat34 * Matrix34(pCAttachmentBone->m_AttModelRelative);
			Vec3 obbPos = FinalMat34.GetTranslation();
			if (rParams.dwFObjFlags & FOB_NEAREST)
				obbPos += passInfo.GetCamera().GetPosition();   // Convert to world space
			AABB caabb = pCAttachmentBone->m_pIAttachmentObject->GetAABB();
			OBB obb2 = OBB::CreateOBBfromAABB(Matrix33(FinalMat34), caabb);
			g_pAuxGeom->DrawOBB(obb2, obbPos, 0, RGBA8(0xff, 0x00, 0x1f, 0xff), eBBD_Extremes_Color_Encoded);
		}
		for (u32 i = m_sortedRanges[eRange_FaceStatic].begin; i < m_sortedRanges[eRange_FaceExecute].end; i++)
		{
			IAttachment* pIAttachment = m_arrAttachments[i];
			CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
			if (pCAttachmentFace->m_AttFlags & uHideFlags)
				continue;
			if ((pCAttachmentFace->m_AttFlags & FLAGS_ATTACH_PROJECTED) == 0)
				continue;                                             //no success! maybe next time
			if ((pCAttachmentFace->m_AttFlags & FLAGS_ATTACH_VISIBLE) == 0)
				continue;                                             //Distance culling. Object is too small for rendering
			Matrix34 FinalMat34 = rWorldMat34 * Matrix34(pCAttachmentFace->m_AttModelRelative);
			Vec3 obbPos = FinalMat34.GetTranslation();
			if (rParams.dwFObjFlags & FOB_NEAREST)
				obbPos += passInfo.GetCamera().GetPosition();   // Convert to world space
			AABB caabb = pCAttachmentFace->m_pIAttachmentObject->GetAABB();
			OBB obb2 = OBB::CreateOBBfromAABB(Matrix33(FinalMat34), caabb);
			g_pAuxGeom->DrawOBB(obb2, obbPos, 0, RGBA8(0x1f, 0x00, 0xff, 0xff), eBBD_Extremes_Color_Encoded);
		}
	}
#endif

}

void CAttachmentUpr::DrawMergedAttachments(SRendParams& rParams, const Matrix34& rWorldMat34, const SRenderingPassInfo& passInfo, const f32 fZoomFactor, const f32 fZoomDistanceSq)
{
	const bool bDrawMergedAttachments = Console::GetInst().ca_DrawAttachmentsMergedForShadows != 0;
	if (bDrawMergedAttachments)
	{
		if (m_attachmentMergingRequired)
			MergeCharacterAttachments();

		if (passInfo.IsShadowPass())
		{
			for (u32 i = 0; i < m_mergedAttachments.size(); i++)
			{
				CAttachmentMerged* pCAttachmentMerged = static_cast<CAttachmentMerged*>(m_mergedAttachments[i].get());
				const f32 fRadiusSqr = m_pSkelInstance->m_SkeletonPose.GetAABB().GetRadiusSqr();
				if (fRadiusSqr == 0.0f)
					continue; //if radius is zero, then the object is most probably not visible and we can continue
				if (!(rParams.nCustomFlags & COB_POST_3D_RENDER) && fZoomDistanceSq > fRadiusSqr)
					continue; //too small to render. cancel the update

				pCAttachmentMerged->DrawAttachment(rParams, passInfo, rWorldMat34, fZoomFactor);
			}
		}
	}
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

IProxy* CAttachmentUpr::CreateProxy(tukk szProxyName, tukk szJointName)
{
	string strProxyName = szProxyName;
	strProxyName.MakeLower();
	IProxy* pIProxyName = GetProxyInterfaceByName(strProxyName.c_str());
	if (pIProxyName)
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Attachment name '%s' is already in use, attachment will not be created", strProxyName.c_str());
		return 0;
	}

	u32 nameCRC = CCrc32::ComputeLowercase(strProxyName.c_str());
	IAttachment* pIAttachmentCRC32 = GetInterfaceByNameCRC(nameCRC);
	if (pIAttachmentCRC32)
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Attachment CRC32 for '%s' clashes with attachment name '%s' (crc's are created using lower case only), attachment will not be created", strProxyName.c_str(), pIAttachmentCRC32->GetName());
		return 0;
	}
	CDefaultSkeleton& rDefaultSkeleton = *m_pSkelInstance->m_pDefaultSkeleton;
	if (szJointName == 0)
		return 0;

	CProxy proxy;
	proxy.m_pAttachmentUpr = this;
	proxy.m_strProxyName = strProxyName.c_str();
	proxy.m_nProxyCRC32 = CCrc32::ComputeLowercase(proxy.m_strProxyName.c_str());
	proxy.m_strJointName = szJointName;
	proxy.m_nJointID = rDefaultSkeleton.GetJointIDByName(proxy.m_strJointName.c_str());
	if (proxy.m_nJointID < 0)
	{
		DrxWarning(VALIDATOR_MODULE_ANIMATION, VALIDATOR_ERROR, "DinrusXAnimation: Proxy '%s' specified wrong joint name '%s'", strProxyName.c_str(), proxy.m_strJointName.c_str());
		return 0;
	}
	proxy.m_ProxyAbsoluteDefault = rDefaultSkeleton.GetDefaultAbsJointByID(proxy.m_nJointID);
	proxy.m_params = Vec4(0, 0, 0, 0.25f);
	QuatT jointQT = rDefaultSkeleton.GetDefaultAbsJointByID(proxy.m_nJointID);
	proxy.m_ProxyRelativeDefault = jointQT.GetInverted() * proxy.m_ProxyAbsoluteDefault;
	m_arrProxies.push_back(proxy);
	return GetProxyInterfaceByCRC(proxy.m_nProxyCRC32);
}

IProxy* CAttachmentUpr::GetProxyInterfaceByIndex(u32 c) const
{
	size_t size = m_arrProxies.size();
	if (size == 0) return 0;
	if (size <= c) return 0;
	IProxy* pIProxy = (IProxy*)&m_arrProxies[c];
	return pIProxy;
};

IProxy* CAttachmentUpr::GetProxyInterfaceByName(tukk szName) const
{
	i32 idx = GetProxyIndexByName(szName);
	if (idx == -1) return 0;
	return GetProxyInterfaceByIndex(idx);
}

IProxy* CAttachmentUpr::GetProxyInterfaceByCRC(u32 nameCRC) const
{
	i32 idx = GetProxyIndexByCRC(nameCRC);
	if (idx == -1) return 0;
	return GetProxyInterfaceByIndex(idx);
};

i32 CAttachmentUpr::GetProxyIndexByName(tukk szName) const
{
	i32 num = GetProxyCount();
	for (i32 i = 0; i < num; i++)
	{
		IProxy* pA = GetProxyInterfaceByIndex(i);
		if (stricmp(pA->GetName(), szName) == 0)
			return i;
	}
	return -1;
}

i32 CAttachmentUpr::GetProxyIndexByCRC(u32 nameCRC) const
{
	i32 num = GetProxyCount();
	for (i32 i = 0; i < num; i++)
	{
		IProxy* pA = GetProxyInterfaceByIndex(i);
		if (pA->GetNameCRC() == nameCRC)
			return i;
	}
	return -1;
}

i32 CAttachmentUpr::RemoveProxyByInterface(const IProxy* pIProxy)
{
	tukk pName = pIProxy->GetName();
	return RemoveProxyByName(pName);
}

i32 CAttachmentUpr::RemoveProxyByName(tukk szName)
{
	i32 index = GetProxyIndexByName(szName);
	if (index == -1)
		return 0;
	RemoveProxyByIndex(index);
	return 1;
};

i32 CAttachmentUpr::RemoveProxyByNameCRC(u32 nameCRC)
{
	i32 index = GetIndexByNameCRC(nameCRC);
	if (index == -1)
		return 0;
	RemoveProxyByIndex(index);
	return 1;
};

void CAttachmentUpr::RemoveProxyByIndex(u32 n)
{
	size_t size = m_arrProxies.size();
	if (size == 0)
		return;
	if (size <= n)
		return;
	IProxy* pIProxy = GetProxyInterfaceByIndex(n);
	if (pIProxy == 0)
		return;
	m_arrProxies[n] = m_arrProxies[size - 1];
	m_arrProxies.pop_back();
	VerifyProxyLinks();
}

size_t CAttachmentUpr::SizeOfAllAttachments() const
{
	if (IDrxSizer* pSizer = gEnv->pSystem->CreateSizer())
	{
		pSizer->AddObject(this);
		pSizer->End();

		const auto totalSize = pSizer->GetTotalSize();

		pSizer->Release();

		return totalSize;
	}
	else
	{
		return 0;
	}
}

void CAttachmentUpr::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_arrAttachments);
}

void CAttachmentUpr::SortByType()
{
	LOADING_TIME_PROFILE_SECTION;
	m_TypeSortingRequired = 0;
	CDefaultSkeleton& rDefaultSkeleton = *m_pSkelInstance->m_pDefaultSkeleton;
	memset(m_sortedRanges, 0, sizeof(m_sortedRanges));
	u32 numAttachments = m_arrAttachments.size(), n = 0;
	if (numAttachments == 0)
		return;
	VerifyProxyLinks();
	IAttachment** parr = (IAttachment**)&m_arrAttachments[0];
	STACK_ARRAY(IAttachment*, parr2, numAttachments);
	for (u32 a = 0; a < numAttachments; a++)
	{
		if (parr[a]->GetType() != CA_BONE)
			continue;
		CAttachmentBONE* pb = (CAttachmentBONE*)parr[a];
		tukk strJointName = pb->m_strJointName.c_str();
		if (pb->m_Simulation.m_useRedirect)
			parr2[n++] = parr[a], pb->m_nJointID = rDefaultSkeleton.GetJointIDByName(strJointName);
	}
	m_sortedRanges[eRange_BoneRedirect].end = m_sortedRanges[eRange_BoneEmpty].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_BONE)
			continue;
		CAttachmentBONE* pb = (CAttachmentBONE*)parr[i];
		if (pb->m_Simulation.m_useRedirect)
			continue;
		if (parr[i]->GetIAttachmentObject() == 0)
		{
			parr2[n++] = parr[i];
			pb->m_nJointID = rDefaultSkeleton.GetJointIDByName(pb->m_strJointName.c_str());
		}
	}
	m_sortedRanges[eRange_BoneEmpty].end = m_sortedRanges[eRange_BoneStatic].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_BONE)
			continue;
		CAttachmentBONE* pb = (CAttachmentBONE*)parr[i];
		if (pb->m_Simulation.m_useRedirect)
			continue;
		IAttachmentObject* p = pb->m_pIAttachmentObject;
		if (p && p->GetAttachmentType() == IAttachmentObject::eAttachment_StatObj)
		{
			parr2[n++] = parr[i];
			pb->m_nJointID = rDefaultSkeleton.GetJointIDByName(pb->m_strJointName.c_str());
		}
	}
	m_sortedRanges[eRange_BoneStatic].end = m_sortedRanges[eRange_BoneExecute].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_BONE)
			continue;
		CAttachmentBONE* pb = (CAttachmentBONE*)parr[i];
		if (pb->m_Simulation.m_useRedirect)
			continue;
		IAttachmentObject* p = pb->m_pIAttachmentObject;
		if (p)
		{
			IAttachmentObject::EType t = p->GetAttachmentType();
			u32 isSkeleton = t == IAttachmentObject::eAttachment_Skeleton;
			if (isSkeleton)
			{
				parr2[n++] = parr[i];
				pb->m_nJointID = rDefaultSkeleton.GetJointIDByName(pb->m_strJointName.c_str());
			}
		}
	}
	m_sortedRanges[eRange_BoneExecute].end = m_sortedRanges[eRange_BoneExecuteUnsafe].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_BONE)
			continue;
		CAttachmentBONE* pb = (CAttachmentBONE*)parr[i];
		if (pb->m_Simulation.m_useRedirect)
			continue;
		IAttachmentObject* p = pb->m_pIAttachmentObject;
		if (p)
		{
			IAttachmentObject::EType t = p->GetAttachmentType();
			u32 isEntity = t == IAttachmentObject::eAttachment_Entity;
			u32 isLight = t == IAttachmentObject::eAttachment_Light;
			u32 isEffect = t == IAttachmentObject::eAttachment_Effect;
			if (isEntity + isLight + isEffect)
			{
				parr2[n++] = parr[i];
				pb->m_nJointID = rDefaultSkeleton.GetJointIDByName(pb->m_strJointName.c_str());
			}
		}
	}
	m_sortedRanges[eRange_BoneExecuteUnsafe].end = m_sortedRanges[eRange_FaceEmpty].begin = n;
	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_FACE)
			continue;
		if (parr[i]->GetIAttachmentObject() == 0)
			parr2[n++] = parr[i];
	}
	m_sortedRanges[eRange_FaceEmpty].end = m_sortedRanges[eRange_FaceStatic].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_FACE)
			continue;
		IAttachmentObject* p = parr[i]->GetIAttachmentObject();
		if (p && p->GetAttachmentType() == IAttachmentObject::eAttachment_StatObj)
			parr2[n++] = parr[i];
	}
	m_sortedRanges[eRange_FaceStatic].end = m_sortedRanges[eRange_FaceExecute].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_FACE)
			continue;
		if (parr[i]->GetIAttachmentObject())
		{
			IAttachmentObject::EType t = parr[i]->GetIAttachmentObject()->GetAttachmentType();
			u32 isSkeleton = t == IAttachmentObject::eAttachment_Skeleton;
			if (isSkeleton)
				parr2[n++] = parr[i];
		}
	}
	m_sortedRanges[eRange_FaceExecute].end = m_sortedRanges[eRange_FaceExecuteUnsafe].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() != CA_FACE)
			continue;
		if (parr[i]->GetIAttachmentObject())
		{
			IAttachmentObject::EType t = parr[i]->GetIAttachmentObject()->GetAttachmentType();
			u32 isEntity = t == IAttachmentObject::eAttachment_Entity;
			u32 isLight = t == IAttachmentObject::eAttachment_Light;
			u32 isEffect = t == IAttachmentObject::eAttachment_Effect;
			if (isEntity + isLight + isEffect)
				parr2[n++] = parr[i];
		}
	}
	m_sortedRanges[eRange_FaceExecuteUnsafe].end = m_sortedRanges[eRange_SkinMesh].begin = n;

	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() == CA_SKIN)
			PREFAST_SUPPRESS_WARNING(6386) parr2[n++] = parr[i];
	}
	m_sortedRanges[eRange_SkinMesh].end = m_sortedRanges[eRange_VertexClothOrPendulumRow].begin = n;
	for (u32 i = 0; i < numAttachments; i++)
	{
		if (parr[i]->GetType() == CA_PROW || parr[i]->GetType() == CA_VCLOTH)
			PREFAST_SUPPRESS_WARNING(6386) parr2[n++] = parr[i];
	}
	m_sortedRanges[eRange_VertexClothOrPendulumRow].end = n;

	if (n != numAttachments)
		DrxFatalError("DinrusXAnimation: sorting error: %s", m_pSkelInstance->GetFilePath());
	for (u32 i = 0; i < numAttachments; i++)
	{
		parr[i] = parr2[i];
	}
	if (m_sortedRanges[eRange_BoneRedirect].end)
	{
		STACK_ARRAY(u8, parrflags, m_sortedRanges[eRange_BoneRedirect].end);
		memset(parrflags, 0, m_sortedRanges[eRange_BoneRedirect].end);
		for (u32 i = 0; i < m_sortedRanges[eRange_BoneRedirect].end; i++)
		{
			u32 s = 0x7fff, t = 0;
			for (u32 r = 0; r < m_sortedRanges[eRange_BoneRedirect].end; r++)
			{
				CAttachmentBONE* pb = (CAttachmentBONE*)parr[r];
				if (s > u32((pb->m_nJointID + 1) | parrflags[r] << 0x10))
				{
					t = r;
					s = pb->m_nJointID + 1;
				}
			}
			parr2[i] = parr[t], parrflags[t] = 1;
		}
		m_numRedirectionWithAttachment = 0;

		for (u32 r = 0; r < m_sortedRanges[eRange_BoneRedirect].end; r++)
		{
			parr[r] = parr2[r];
			CAttachmentBONE* pb = (CAttachmentBONE*)parr[r];
			if (pb->m_pIAttachmentObject)
				m_numRedirectionWithAttachment++;
			if (pb->m_Simulation.m_arrChildren.size())
				continue;
			pb->m_Simulation.m_nAnimOverrideJoint =
			  rDefaultSkeleton.GetJointIDByName("all_blendWeightPendulum");
			i32 parentid = pb->m_nJointID;
			if (parentid < 0)
				continue;
			bool arrChildren[MAX_JOINT_AMOUNT];
			for (u32 i = 0; i < MAX_JOINT_AMOUNT; i++)
			{
				arrChildren[i] = 0;
			}
			pb->m_Simulation.m_arrChildren.resize(0);
			pb->m_Simulation.m_arrChildren.reserve(16);
			u32 numJoints = rDefaultSkeleton.GetJointCount();
			for (u32 i = parentid + 1; i < numJoints; i++)
			{
				if (rDefaultSkeleton.GetJointParentIDByID(i) == parentid)
					arrChildren[i] = 1;
			}
			for (u32 pid = 0; pid < MAX_JOINT_AMOUNT; pid++)
			{
				if (arrChildren[pid] == 0)
					continue;
				for (u32 i = pid + 1; i < numJoints; i++)
				{
					if (rDefaultSkeleton.GetJointParentIDByID(i) == pid)
						arrChildren[i] = 1;
				}
			}
			for (u32 i = 0; i < MAX_JOINT_AMOUNT; i++)
			{
				if (arrChildren[i])
					pb->m_Simulation.m_arrChildren.push_back(i);
			}
		}
	}

	for (u32 r = m_sortedRanges[eRange_VertexClothOrPendulumRow].begin;
	     r < m_sortedRanges[eRange_VertexClothOrPendulumRow].end; r++)
	{
		if (parr[r]->GetType() != CA_PROW)
			continue;
		CAttachmentPROW* pb = (CAttachmentPROW*)parr[r];
		pb->m_nRowJointID = rDefaultSkeleton.GetJointIDByName(pb->m_strRowJointName.c_str());
		if (pb->m_nRowJointID < 0)
			continue;
		tukk psrc = pb->m_strRowJointName.c_str();
		tukk pstr = DrxStringUtils::stristr(psrc, "_x00_");
		if (pstr == 0)
			continue;                                                                                                // invalid name
		char pJointName[256];
		drx_strcpy(pJointName, psrc);
		JointIdType jindices[MAX_JOINT_AMOUNT];
		u16& numParticles = pb->m_rowparams.m_nParticlesPerRow;
		numParticles = 0;
		for (i32 idx = i32(pstr - psrc) + 2; numParticles < MAX_JOINTS_PER_ROW; numParticles++)
		{
			if ((jindices[numParticles] = rDefaultSkeleton.GetJointIDByName(pJointName)) == JointIdType(-1))
				break;
			if (++pJointName[idx + 1] == 0x3a)
				pJointName[idx]++, pJointName[idx + 1] = 0x30;
		}
		pb->m_rowparams.m_arrParticles.resize(numParticles);
		u32 numJoints = rDefaultSkeleton.GetJointCount();
		for (u32 r = 0; r < numParticles; r++)
		{
			pb->m_rowparams.m_arrParticles[r].m_childID = JointIdType(-1);
			JointIdType parentid = jindices[r];
			if (parentid == JointIdType(-1))
				continue;                                                                                              // invalid
			for (u32 i = parentid + 1; i < numJoints; i++)
			{
				if (rDefaultSkeleton.GetJointParentIDByID(i) == parentid)
				{
					pb->m_rowparams.m_arrParticles[r].m_childID = i;
					break;
				}
			}
		}
		for (u32 i = 0; i < numParticles; i++)
		{
			pb->m_rowparams.m_arrParticles[i].m_vDistance.y = 0.07f;                                                   // use 7cm by default
			if (pb->m_rowparams.m_arrParticles[i].m_childID != JointIdType(-1))
				pb->m_rowparams.m_arrParticles[i].m_vDistance.y =
				  rDefaultSkeleton.GetDefaultRelJointByID(pb->m_rowparams.m_arrParticles[i].m_childID).t.GetLength();    // set the vertical distance
		}
		for (u32 i = 0; i < numParticles; i++)
		{
			u32 i0 = i;
			u32 i1 = (i + 1) % numParticles;
			JointIdType id0 = jindices[i0];
			Vec3 j0 = rDefaultSkeleton.GetDefaultAbsJointByID(id0).q.GetColumn0() *
			          pb->m_rowparams.m_arrParticles[i0].m_vDistance.y +
			          rDefaultSkeleton.GetDefaultAbsJointByID(id0).t;
			JointIdType id1 = jindices[i1];
			Vec3 j1 = rDefaultSkeleton.GetDefaultAbsJointByID(id1).q.GetColumn0() *
			          pb->m_rowparams.m_arrParticles[i1].m_vDistance.y +
			          rDefaultSkeleton.GetDefaultAbsJointByID(id1).t;
			pb->m_rowparams.m_arrParticles[i0].m_vDistance.x = (j0 - j1).GetLength();                                    // set the horizontal distance
			pb->m_rowparams.m_arrParticles[i].m_jointID = id0;
		}
	}

	RequestMergeCharacterAttachments();
}

#ifdef EDITOR_PCDEBUGCODE
void CAttachmentUpr::Verification()
{
	i32 s = -1;
	for (u32 i = 0; i < m_sortedRanges[eRange_BoneRedirect].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_BONE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		if (pCAttachmentBone->m_Simulation.m_nClampType == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		i32 nJointCount = m_pSkelInstance->m_pDefaultSkeleton->GetJointCount();
		if (pCAttachmentBone->m_nJointID >= nJointCount)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		if (pCAttachmentBone->m_Simulation.m_useRedirect != 1)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		if (pCAttachmentBone->m_nJointID < s)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		s = pCAttachmentBone->m_nJointID;
	}

	for (u32 i = m_sortedRanges[eRange_BoneEmpty].begin; i < m_sortedRanges[eRange_BoneEmpty].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_BONE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		if (pCAttachmentBone->m_pIAttachmentObject)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		if (pCAttachmentBone->m_nJointID >= 0)
		{
			i32 nJointCount = m_pSkelInstance->m_pDefaultSkeleton->GetJointCount();
			if (pCAttachmentBone->m_nJointID > nJointCount)
				DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		}
	}

	for (u32 i = m_sortedRanges[eRange_BoneStatic].begin; i < m_sortedRanges[eRange_BoneStatic].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_BONE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		if (pCAttachmentBone->m_Simulation.m_useRedirect && pCAttachmentBone->m_Simulation.m_nClampType)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		if (pCAttachmentBone->m_nJointID >= 0)
		{
			i32 nJointCount = m_pSkelInstance->m_pDefaultSkeleton->GetJointCount();
			if (pCAttachmentBone->m_nJointID > nJointCount)
				DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		}
		IAttachmentObject* p = pIAttachment->GetIAttachmentObject();
		if (p == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject::EType eAttachmentType = p->GetAttachmentType();
		u32 isStatic = eAttachmentType == IAttachmentObject::eAttachment_StatObj;
		if (isStatic == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}

	for (u32 i = m_sortedRanges[eRange_BoneExecute].begin; i < m_sortedRanges[eRange_BoneExecute].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_BONE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
		if (pCAttachmentBone->m_nJointID >= 0)
		{
			u32 nJointCount = m_pSkelInstance->m_pDefaultSkeleton->GetJointCount();
			if (pCAttachmentBone->m_nJointID > nJointCount)
				DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		}
		IAttachmentObject* p = pIAttachment->GetIAttachmentObject();
		if (p == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject::EType eAttachmentType = p->GetAttachmentType();
		u32 isSkeleton = eAttachmentType == IAttachmentObject::eAttachment_Skeleton;
		u32 isEntity = eAttachmentType == IAttachmentObject::eAttachment_Entity;
		u32 isLight = eAttachmentType == IAttachmentObject::eAttachment_Light;
		u32 isEffect = eAttachmentType == IAttachmentObject::eAttachment_Effect;
		if ((isSkeleton | isEntity | isLight | isEffect) == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		if (p == 0 && pCAttachmentBone->m_Simulation.m_useRedirect && pCAttachmentBone->m_Simulation.m_nClampType)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}

	for (u32 i = m_sortedRanges[eRange_FaceEmpty].begin; i < m_sortedRanges[eRange_FaceEmpty].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_FACE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject* p = pIAttachment->GetIAttachmentObject();
		if (p)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}

	for (u32 i = m_sortedRanges[eRange_FaceStatic].begin; i < m_sortedRanges[eRange_FaceStatic].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_FACE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject* p = pIAttachment->GetIAttachmentObject();
		if (p == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject::EType eAttachmentType = p->GetAttachmentType();
		u32 isStatic = eAttachmentType == IAttachmentObject::eAttachment_StatObj;
		if (isStatic == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}

	for (u32 i = m_sortedRanges[eRange_FaceExecute].begin; i < m_sortedRanges[eRange_FaceExecute].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_FACE)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject* p = pIAttachment->GetIAttachmentObject();
		if (p == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
		IAttachmentObject::EType eAttachmentType = p->GetAttachmentType();
		u32 isSkeleton = eAttachmentType == IAttachmentObject::eAttachment_Skeleton;
		u32 isEntity = eAttachmentType == IAttachmentObject::eAttachment_Entity;
		u32 isLight = eAttachmentType == IAttachmentObject::eAttachment_Light;
		u32 isEffect = eAttachmentType == IAttachmentObject::eAttachment_Effect;
		if ((isSkeleton | isEntity | isLight | isEffect) == 0)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}

	for (u32 i = m_sortedRanges[eRange_SkinMesh].begin; i < m_sortedRanges[eRange_SkinMesh].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_SKIN)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}
	for (u32 i = m_sortedRanges[eRange_VertexClothOrPendulumRow].begin; i < m_sortedRanges[eRange_VertexClothOrPendulumRow].end; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		if (pIAttachment->GetType() != CA_PROW && pIAttachment->GetType() != CA_VCLOTH)
			DrxFatalError("DinrusXAnimation: setup conflict in attachment: %s", pIAttachment->GetName());
	}

	//---------------------------------------------------------------
	return;

	float fColorYellow[4] = { 1, 1, 0, 1 };
	float fColorRed[4] = { 1, 0, 0, 1 };
	float fColorGreen[4] = { 0, 1, 0, 1 };
	float fColorBlue[4] = { 0, 0, 1, 1 };

	u32 numAttachments = m_arrAttachments.size();
	for (u32 i = 0; i < numAttachments; i++)
	{
		IAttachment* pIAttachment = m_arrAttachments[i];
		u32 nType = pIAttachment->GetType();
		if (nType == CA_BONE)
		{
			CAttachmentBONE* pCAttachmentBone = (CAttachmentBONE*)pIAttachment;
			u32 IsJiggleJoint = (pIAttachment->GetIAttachmentObject() == 0 && pCAttachmentBone->m_Simulation.m_nClampType && pCAttachmentBone->m_Simulation.m_useRedirect);
			if (IsJiggleJoint)
			{
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorYellow, false, "CA_JIGL: %s refcount: %d  children: %d", pIAttachment->GetName(), pCAttachmentBone->m_nRefCounter, pCAttachmentBone->m_Simulation.m_arrChildren.size()), g_YLine += 10.0f;
			}
			else
			{
				IAttachmentObject* p = pIAttachment->GetIAttachmentObject();
				if (p)
					g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorRed, false, "CA_BONE: %s refcount: %d", pIAttachment->GetName(), pCAttachmentBone->m_nRefCounter), g_YLine += 10.0f;
				else
					g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorRed, false, "CA_BONE: %s (empty) refcount: %d", pIAttachment->GetName(), pCAttachmentBone->m_nRefCounter), g_YLine += 10.0f;
			}
		}
		if (nType == CA_FACE)
		{
			CAttachmentFACE* pCAttachmentFace = (CAttachmentFACE*)pIAttachment;
			IAttachmentObject* p = pCAttachmentFace->m_pIAttachmentObject;
			if (p)
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorBlue, false, "CA_FACE: %s refcount: %d", pIAttachment->GetName(), pCAttachmentFace->m_nRefCounter), g_YLine += 10.0f;
			else
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorBlue, false, "CA_FACE: %s (empty) refcount: %d", pIAttachment->GetName(), pCAttachmentFace->m_nRefCounter), g_YLine += 10.0f;
		}
		if (nType == CA_SKIN)
		{
			CAttachmentSKIN* pCAttachmentSkin = (CAttachmentSKIN*)pIAttachment;
			IAttachmentObject* p = pCAttachmentSkin->m_pIAttachmentObject;
			if (p)
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorGreen, false, "CA_SKIN: %s  refcount: %d", pIAttachment->GetName(), pCAttachmentSkin->m_nRefCounter), g_YLine += 10.0f;
			else
				g_pAuxGeom->Draw2dLabel(1, g_YLine, 1.0f, fColorGreen, false, "CA_SKIN: %s (empty)  refcount: %d", pIAttachment->GetName(), pCAttachmentSkin->m_nRefCounter), g_YLine += 10.0f;
		}
	}
}

#endif

#if !defined(_RELEASE)

float CAttachmentUpr::DebugDrawAttachment(IAttachment* pAttachment, ISkin* pSkin, Vec3 drawLoc, IMaterial* pMaterial, float drawScale,const SRenderingPassInfo &passInfo)
{
	if (!pMaterial || !pAttachment || !pSkin || !pSkin->GetIRenderMesh(0))
		return 0.0f;

	i32k nTexMemUsage = pSkin->GetIRenderMesh(0)->GetTextureMemoryUsage(pMaterial);
	ICVar* p_texBudget = gEnv->pConsole->GetCVar("ca_AttachmentTextureMemoryBudget");         // the closer to this limit, the more red we draw it for clarity
	const float fTextMemBudget = p_texBudget ? p_texBudget->GetIVal() * 1024 * 1024 : 1.0f;
	const float white = max(1.0f - (nTexMemUsage / fTextMemBudget), 0.0f);

	float color[4] = { 1, white, white, 1 };
	float fDist = (passInfo.GetCamera().GetPosition() - drawLoc).GetLength();

	static float scalar = 60.0f;
	float drawOffset = (drawScale * (fDist / scalar));

	IRenderAuxText::DrawLabelExF(drawLoc, drawScale, color, true, true, "%s (%d kb)", pAttachment->GetName(), nTexMemUsage / 1024);

	return drawOffset;
}
#endif

tukk CAttachmentUpr::GetProcFunctionName(u32 idx) const
{
	return 0;
}

tukk CAttachmentUpr::ExecProcFunction(u32 nCRC32, Skeleton::CPoseData* pPoseData, tukk pstrFunction) const
{
	return 0;
}

void CAttachmentUpr::OnHideAttachment(const IAttachment* pAttachment, u32 nHideType, bool bHide)
{
	if (nHideType & FLAGS_ATTACH_HIDE_SHADOW_PASS)
	{
		const bool bWasHidden = (pAttachment->GetFlags() & FLAGS_ATTACH_HIDE_SHADOW_PASS) != 0;
		if (bHide ^ bWasHidden)
		{
			for (auto pMergedAttachment : m_mergedAttachments)
			{
				if (pMergedAttachment->HasAttachment(pAttachment))
				{
					pMergedAttachment->HideMergedAttachment(pAttachment, bHide);
					break;
				}
			}
		}
	}
}

uk CAttachmentUpr::CModificationCommandBuffer::Alloc(size_t size, size_t align)
{
	DRX_ASSERT(align <= 32);
	u32 currentOffset = m_memory.size();
	// allocate enough memory that it fits with alignment
	m_memory.resize(m_memory.size() + size + align);
	tuk mem = &m_memory[currentOffset];
	tuk memAligned = Align(mem, align);
	size_t padding = (size_t) (memAligned - mem);
	// and adjust to actual padding afterwards
	m_memory.resize(m_memory.size() - align + padding);
	m_commandOffsets.push_back(currentOffset + padding);
	return &m_memory[m_commandOffsets[m_commandOffsets.size() - 1]];
}

void CAttachmentUpr::CModificationCommandBuffer::Clear()
{
	m_memory.resize(0);
	m_commandOffsets.resize(0);
	m_memory.reserve(kMaxMemory);
	m_commandOffsets.reserve(kMaxOffsets);
}

CAttachmentUpr::CModificationCommandBuffer::~CModificationCommandBuffer()
{

	// fail if the command buffer has not been flushed before destruction
	// We can not do it here, because some commands
	// might access structures that are no longer available
	// (e.g. the default skeleton in the character instance).
	DRX_ASSERT(m_commandOffsets.size() == 0);
}

void CAttachmentUpr::CModificationCommandBuffer::Execute()
{
	for (auto i : m_commandOffsets)
	{
		CModificationCommand* cmd = reinterpret_cast<CModificationCommand*>(&m_memory[i]);
		cmd->Execute();
		cmd->~CModificationCommand();
	}
	Clear();
}

void CAttachmentUpr::ClearAttachmentObject(SAttachmentBase* pAttachment, u32 nLoadingFlags)
{
	class CClearAttachmentObject : public CModificationCommand
	{
	public:
		CClearAttachmentObject(SAttachmentBase* pAttachment, u32 nLoadingFlags) : CModificationCommand(pAttachment), m_nLoadingFlags(nLoadingFlags) {}
		virtual void Execute() override
		{
			m_pAttachment->Immediate_ClearBinding(m_nLoadingFlags);
		}
	private:
		u32 m_nLoadingFlags;
	};
	CMD_BUF_PUSH_COMMAND(m_modificationCommandBuffer, CClearAttachmentObject, pAttachment, nLoadingFlags);
}

void CAttachmentUpr::AddAttachmentObject(SAttachmentBase* pAttachment, IAttachmentObject* pModel, ISkin* pISkin /*= 0*/, u32 nLoadingFlags /*= 0*/)
{
	class CAddAttachmentObject : public CModificationCommand
	{
	public:
		CAddAttachmentObject(SAttachmentBase* pAttachment, IAttachmentObject* pModel, ISkin* pISkin, u32 nLoadingFlags) : CModificationCommand(pAttachment), m_pModel(pModel), m_nLoadingFlags(nLoadingFlags)
		{
			m_pSkin = static_cast<CSkin*>(pISkin);
		}
		virtual void Execute() override
		{
			m_pAttachment->Immediate_AddBinding(m_pModel, m_pSkin, m_nLoadingFlags);
		}
	private:
		// the caller allocated the object and passes ownership to the command
		// until the command is executed, when executed the command will in turn pass the
		// ownership of the AttachmentObject to the attachment slot (SAttachmentBase)
		IAttachmentObject* m_pModel;
		// the CSkin will eventually be stored in a _smart_ptr in the IAttachmentObject
		_smart_ptr<CSkin>  m_pSkin;
		u32             m_nLoadingFlags;
	};
	CMD_BUF_PUSH_COMMAND(m_modificationCommandBuffer, CAddAttachmentObject, pAttachment, pModel, pISkin, nLoadingFlags);
}

void CAttachmentUpr::SwapAttachmentObject(SAttachmentBase* pAttachment, IAttachment* pNewAttachment)
{
	class CSwapAttachmentObject : public CModificationCommand
	{
	public:
		CSwapAttachmentObject(SAttachmentBase* pAttachment, IAttachment* pNewAttachment) : CModificationCommand(pAttachment), m_pNewAttachment(pNewAttachment) {}
		virtual void Execute() override
		{
			m_pAttachment->Immediate_SwapBinding(m_pNewAttachment);
		}
	private:
		IAttachment* m_pNewAttachment;
	};
	CMD_BUF_PUSH_COMMAND(m_modificationCommandBuffer, CSwapAttachmentObject, pAttachment, pNewAttachment);
}
