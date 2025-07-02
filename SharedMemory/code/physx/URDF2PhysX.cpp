
#include "URDF2PhysX.h"

#include "PhysXUrdfImporter.h"
#include <drx3D/Common/b3Logging.h>
#include "PxArticulationReducedCoordinate.h"
#include "PxArticulationJointReducedCoordinate.h"
#include "PxRigidActorExt.h"
#include "PxArticulation.h"
#include "PxRigidBodyExt.h"
#include "PxRigidBody.h"
#include <drx3D/Maths/Linear/Threads.h>
#include "PxRigidActorExt.h"
#include "PxArticulationBase.h"
#include "PxArticulationLink.h"
#include "PxMaterial.h"
#include "PxCooking.h"
#include "PxScene.h"
#include "PxRigidStatic.h"
#include "PxRigidDynamic.h"
#include "PxActor.h"
#include "PxAggregate.h"
#include <drx3D/Common/b3FileUtils.h>
#include "PhysXUserData.h"
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include "../../Importers/ImportObjDemo/LoadMeshFromObj.h"
#include "../../Importers/ImportSTLDemo/LoadMeshFromSTL.h"
#include "../../Importers/ImportURDFDemo/UrdfParser.h"




struct URDF2PhysXCachedData
{
	URDF2PhysXCachedData()
		: m_currentMultiBodyLinkIndex(-1),
		m_articulation(0),
		m_rigidStatic(0),
		m_rigidDynamic(0),
		m_totalNumJoints1(0)
	{
	}
	//these arrays will be initialized in the 'InitURDF2BulletCache'

	AlignedObjectArray<i32> m_urdfLinkParentIndices;
	AlignedObjectArray<i32> m_urdfLinkIndices2BulletLinkIndices;
	AlignedObjectArray<class physx::PxArticulationLink*> m_urdfLink2physxLink;
	AlignedObjectArray<Transform2> m_urdfLinkLocalInertialFrames;

	i32 m_currentMultiBodyLinkIndex;

	physx::PxArticulationReducedCoordinate* m_articulation;
	physx::PxRigidStatic* m_rigidStatic;
	physx::PxRigidDynamic* m_rigidDynamic;

	AlignedObjectArray<physx::PxTransform> m_linkTransWorldSpace;
	AlignedObjectArray<i32> m_urdfLinkIndex;
	AlignedObjectArray<i32> m_parentUrdfLinkIndex;
	AlignedObjectArray<physx::PxReal> m_linkMasses;
	AlignedObjectArray<physx::PxArticulationJointType::Enum> m_jointTypes;

	AlignedObjectArray<physx::PxTransform> m_parentLocalPoses;
	AlignedObjectArray<physx::PxTransform> m_childLocalPoses;
	AlignedObjectArray<UrdfGeomTypes> m_geomTypes;
	AlignedObjectArray<physx::PxVec3> m_geomDimensions;
	AlignedObjectArray<physx::PxVec3> m_linkMaterials;
	AlignedObjectArray<physx::PxTransform>  m_geomLocalPoses;
	


	//this will be initialized in the constructor
	i32 m_totalNumJoints1;
	i32 getParentUrdfIndex(i32 linkIndex) const
	{
		return m_urdfLinkParentIndices[linkIndex];
	}
	i32 getMbIndexFromUrdfIndex(i32 urdfIndex) const
	{
		if (urdfIndex == -2)
			return -2;
		return m_urdfLinkIndices2BulletLinkIndices[urdfIndex];
	}

	void registerMultiBody(i32 urdfLinkIndex, class physx::PxArticulationLink* body, const Transform2& worldTransform, Scalar mass, const Vec3& localInertiaDiagonal,  const Transform2& localInertialFrame)
	{
		m_urdfLink2physxLink[urdfLinkIndex] = body;
		m_urdfLinkLocalInertialFrames[urdfLinkIndex] = localInertialFrame;
	}

	class physx::PxArticulationLink* getPhysxLinkFromLink(i32 urdfLinkIndex)
	{
		return m_urdfLink2physxLink[urdfLinkIndex];
	}

	void registerRigidBody(i32 urdfLinkIndex, class physx::PxArticulationLink* body, const Transform2& worldTransform, Scalar mass, const Vec3& localInertiaDiagonal, const class CollisionShape* compound, const Transform2& localInertialFrame)
	{
		Assert(m_urdfLink2physxLink[urdfLinkIndex] == 0);

		m_urdfLink2physxLink[urdfLinkIndex] = body;
		m_urdfLinkLocalInertialFrames[urdfLinkIndex] = localInertialFrame;
	}
};


void ComputeParentIndices(const URDFImporterInterface& u2b, URDF2PhysXCachedData& cache, i32 urdfLinkIndex, i32 urdfParentIndex)
{
	cache.m_urdfLinkParentIndices[urdfLinkIndex] = urdfParentIndex;
	cache.m_urdfLinkIndices2BulletLinkIndices[urdfLinkIndex] = cache.m_currentMultiBodyLinkIndex++;

	AlignedObjectArray<i32> childIndices;
	u2b.getLinkChildIndices(urdfLinkIndex, childIndices);
	for (i32 i = 0; i < childIndices.size(); i++)
	{
		ComputeParentIndices(u2b, cache, childIndices[i], urdfLinkIndex);
	}
}

void ComputeTotalNumberOfJoints(const URDFImporterInterface& u2b, URDF2PhysXCachedData& cache, i32 linkIndex)
{
	AlignedObjectArray<i32> childIndices;
	u2b.getLinkChildIndices(linkIndex, childIndices);
	cache.m_totalNumJoints1 += childIndices.size();
	for (i32 i = 0; i < childIndices.size(); i++)
	{
		i32 childIndex = childIndices[i];
		ComputeTotalNumberOfJoints(u2b, cache, childIndex);
	}
}


void InitURDF2BulletCache(const URDFImporterInterface& u2b, URDF2PhysXCachedData& cache, i32 flags)
{
	//compute the number of links, and compute parent indices array (and possibly other cached data?)
	cache.m_totalNumJoints1 = 0;

	i32 rootLinkIndex = u2b.getRootLinkIndex();
	if (rootLinkIndex >= 0)
	{
		ComputeTotalNumberOfJoints(u2b, cache, rootLinkIndex);
		i32 numTotalLinksIncludingBase = 1 + cache.m_totalNumJoints1;

		cache.m_urdfLinkParentIndices.resize(numTotalLinksIncludingBase);
		cache.m_urdfLinkIndices2BulletLinkIndices.resize(numTotalLinksIncludingBase);
		cache.m_urdfLink2physxLink.resize(numTotalLinksIncludingBase);
		cache.m_urdfLinkLocalInertialFrames.resize(numTotalLinksIncludingBase);

		cache.m_currentMultiBodyLinkIndex = -1;  //multi body base has 'link' index -1

		bool maintainLinkOrder = (flags & CUF_MAINTAIN_LINK_ORDER) != 0;
		if (maintainLinkOrder)
		{
			URDF2PhysXCachedData cache2 = cache;

			ComputeParentIndices(u2b, cache2, rootLinkIndex, -2);

			for (i32 j = 0; j<numTotalLinksIncludingBase; j++)
			{
				cache.m_urdfLinkParentIndices[j] = cache2.m_urdfLinkParentIndices[j];
				cache.m_urdfLinkIndices2BulletLinkIndices[j] = j - 1;
			}
		}
		else
		{
			ComputeParentIndices(u2b, cache, rootLinkIndex, -2);
		}

	}
}


struct ArticulationCreationInterface
{
	physx::PxFoundation* m_foundation;
	physx::PxPhysics* m_physics;
	physx::PxCooking* m_cooking;
	physx::PxScene* m_scene;
	CommonFileIOInterface* m_fileIO;

	b3AlignedObjectArray<i32> m_mb2urdfLink;
	void addLinkMapping(i32 urdfLinkIndex, i32 mbLinkIndex)
	{
		if (m_mb2urdfLink.size() < (mbLinkIndex + 1))
		{
			m_mb2urdfLink.resize((mbLinkIndex + 1), -2);
		}
		//    m_urdf2mbLink[urdfLinkIndex] = mbLinkIndex;
		m_mb2urdfLink[mbLinkIndex] = urdfLinkIndex;
	}
};

static Vec4 colors4[4] =
{
	Vec4(1, 0, 0, 1),
	Vec4(0, 1, 0, 1),
	Vec4(0, 1, 1, 1),
	Vec4(1, 1, 0, 1),
};

static Vec4 selectColor4()
{
	static SpinMutex sMutex;
	sMutex.lock();
	static i32 curColor = 0;
	Vec4 color = colors4[curColor];
	curColor++;
	curColor &= 3;
	sMutex.unlock();
	return color;
}




static physx::PxConvexMesh* createPhysXConvex(physx::PxU32 numVerts, const physx::PxVec3* verts, ArticulationCreationInterface& creation)
{
	physx::PxCookingParams params = creation.m_cooking->getParams();

	// Use the new (default) PxConvexMeshCookingType::eQUICKHULL
	params.convexMeshCookingType = physx::PxConvexMeshCookingType::eQUICKHULL;

	// If the gaussMapLimit is chosen higher than the number of output vertices, no gauss map is added to the convex mesh data (here 256).
	// If the gaussMapLimit is chosen lower than the number of output vertices, a gauss map is added to the convex mesh data (here 16).
	i32 gaussMapLimit = 256;

	params.gaussMapLimit = gaussMapLimit;
	creation.m_cooking->setParams(params);

	// Setup the convex mesh descriptor
	physx::PxConvexMeshDesc desc;

	// We provide points only, therefore the PxConvexFlag::eCOMPUTE_CONVEX flag must be specified
	desc.points.data = verts;
	desc.points.count = numVerts;
	desc.points.stride = sizeof(physx::PxVec3);
	desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	physx::PxU32 meshSize = 0;

	// Directly insert mesh into PhysX
	physx::PxConvexMesh* convex = creation.m_cooking->createConvexMesh(desc, creation.m_physics->getPhysicsInsertionCallback());
	PX_ASSERT(convex);

	return convex;
}


i32 convertLinkPhysXShapes(const URDFImporterInterface& u2b, URDF2PhysXCachedData& cache, ArticulationCreationInterface& creation, i32 urdfLinkIndex, tukk pathPrefix, const Transform2& localInertiaFrame,
	physx::PxArticulationReducedCoordinate* articulation, i32 mbLinkIndex, physx::PxRigidActor* linkPtr)
{
	i32 numShapes = 0;

	URDFLinkContactInfo contactInfo;
	u2b.getLinkContactInfo(urdfLinkIndex, contactInfo);
	
	//static friction, dynamic frictoin, restitution
	cache.m_linkMaterials.push_back(physx::PxVec3(contactInfo.m_lateralFriction, contactInfo.m_lateralFriction, contactInfo.m_restitution));
	physx::PxMaterial* material = creation.m_physics->createMaterial(contactInfo.m_lateralFriction, contactInfo.m_lateralFriction, contactInfo.m_restitution);
	
	const UrdfLink* link = u2b.getUrdfLink(urdfLinkIndex);//.convertLinkCollisionShapes m_data->m_urdfParser.getModel().m_links.getAtIndex(linkIndex);
	Assert(linkPtr);
	if (linkPtr)
	{

		for (i32 v = 0; v < link->m_collisionArray.size(); v++)
		{
			const UrdfCollision& col = link->m_collisionArray[v];
			const UrdfCollision* collision = &col;
			Transform2 childTrans = col.m_linkLocalFrame;
			Transform2 localTrans = localInertiaFrame.inverse()*childTrans;
			physx::PxTransform tr;
			tr.p = physx::PxVec3(localTrans.getOrigin()[0], localTrans.getOrigin()[1], localTrans.getOrigin()[2]);
			tr.q = physx::PxQuat(localTrans.getRotation()[0], localTrans.getRotation()[1], localTrans.getRotation()[2], localTrans.getRotation()[3]);

			physx::PxShape* shape = 0;
			cache.m_geomTypes.push_back(col.m_geometry.m_type);
			

			switch (col.m_geometry.m_type)
			{
			case URDF_GEOM_PLANE:
			{
				Vec3 planeNormal = col.m_geometry.m_planeNormal;
				Scalar planeConstant = 0;  //not available?
				
				Vec3 planeRefAxis(1, 0, 0);
				Quat diffQuat = shortestArcQuat(planeRefAxis, planeNormal);
				shape = physx::PxRigidActorExt::createExclusiveShape(*linkPtr, physx::PxPlaneGeometry(), *material);
				
				Transform2 diffTr;
				diffTr.setIdentity();
				diffTr.setRotation(diffQuat);
				Transform2 localTrans = localInertiaFrame.inverse()*childTrans*diffTr;
				physx::PxTransform tr;
				tr.p = physx::PxVec3(localTrans.getOrigin()[0], localTrans.getOrigin()[1], localTrans.getOrigin()[2]);
				tr.q = physx::PxQuat(localTrans.getRotation()[0], localTrans.getRotation()[1], localTrans.getRotation()[2], localTrans.getRotation()[3]);

				physx::PxTransform localPose = tr;
				shape->setLocalPose(localPose);
				numShapes++;
				break;
			}
			case URDF_GEOM_CAPSULE:
			{
				Scalar radius = collision->m_geometry.m_capsuleRadius;
				Scalar height = collision->m_geometry.m_capsuleHeight;

				//static PxShape* createExclusiveShape(PxRigidActor& actor, const PxGeometry& geometry, PxMaterial*const* materials, PxU16 materialCount,
				//	PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE)
				//{
				physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
				
				//shape = PxGetPhysics().createShape(physx::PxCapsuleGeometry(radius, 0.5*height), &material, 1, false, shapeFlags);

				shape = physx::PxRigidActorExt::createExclusiveShape(*linkPtr, physx::PxCapsuleGeometry(radius, 0.5*height), *material);

				Transform2 childTrans = col.m_linkLocalFrame;
				Transform2 x2z;
				x2z.setIdentity();
				x2z.setRotation(Quat(Vec3(0, 1, 0), SIMD_HALF_PI));
				Transform2 localTrans = localInertiaFrame.inverse()*childTrans*x2z;
				physx::PxTransform tr;
				tr.p = physx::PxVec3(localTrans.getOrigin()[0], localTrans.getOrigin()[1], localTrans.getOrigin()[2]);
				tr.q = physx::PxQuat(localTrans.getRotation()[0], localTrans.getRotation()[1], localTrans.getRotation()[2], localTrans.getRotation()[3]);

				shape->setLocalPose(tr);
				cache.m_geomLocalPoses.push_back(tr);
				numShapes++;
				//CapsuleShapeZ* capsuleShape = new CapsuleShapeZ(radius, height);
				//shape = capsuleShape;
				//shape->setMargin(gUrdfDefaultCollisionMargin);
				break;
			}

			case URDF_GEOM_CYLINDER:
			{
				Scalar cylRadius = collision->m_geometry.m_capsuleRadius;
				Scalar cylHalfLength = 0.5 * collision->m_geometry.m_capsuleHeight;
				cache.m_geomDimensions.push_back(physx::PxVec3(cylRadius, cylHalfLength,0));
				//if (m_data->m_flags & CUF_USE_IMPLICIT_CYLINDER)
				//{
				//	Vec3 halfExtents(cylRadius, cylRadius, cylHalfLength);
				//	CylinderShapeZ* cylZShape = new CylinderShapeZ(halfExtents);
				//	shape = cylZShape;
				//}
				//else
				{
					AlignedObjectArray<physx::PxVec3> vertices;
					
					i32 numSteps = 32;
					for (i32 i = 0; i < numSteps; i++)
					{
						physx::PxVec3 vert(cylRadius * Sin(SIMD_2_PI * (float(i) / numSteps)), cylRadius * btCos(SIMD_2_PI * (float(i) / numSteps)), cylHalfLength);
						vertices.push_back(vert);
						vert[2] = -cylHalfLength;
						vertices.push_back(vert);
					}

					physx::PxConvexMesh* convexMesh = createPhysXConvex(vertices.size(), &vertices[0], creation);

					shape  = physx::PxRigidActorExt::createExclusiveShape(*linkPtr,
						physx::PxConvexMeshGeometry(convexMesh), *material);

					shape->setLocalPose(tr);
					cache.m_geomLocalPoses.push_back(tr);
					numShapes++;
					
				}
				break;
			}
			case URDF_GEOM_BOX:
			{
				Vec3 extents = collision->m_geometry.m_boxSize;
				shape = physx::PxRigidActorExt::createExclusiveShape(*linkPtr, physx::PxBoxGeometry(extents[0] * 0.5, extents[1] * 0.5, extents[2] * 0.5), *material);
				cache.m_geomDimensions.push_back(physx::PxVec3(extents[0]*0.5, extents[1] * 0.5, extents[2] * 0.5));
				shape->setLocalPose(tr);
				cache.m_geomLocalPoses.push_back(tr);
				numShapes++;
				break;
			}
			case URDF_GEOM_SPHERE:
			{
				Scalar radius = collision->m_geometry.m_sphereRadius;
				shape = physx::PxRigidActorExt::createExclusiveShape(*linkPtr, physx::PxSphereGeometry(radius), *material);
				cache.m_geomDimensions.push_back(physx::PxVec3(radius,0,0));
				shape->setLocalPose(tr);
				cache.m_geomLocalPoses.push_back(tr);
				numShapes++;
				break;
			}
			case URDF_GEOM_MESH:
			{
				AlignedObjectArray<physx::PxVec3> vertices;
				GLInstanceGraphicsShape* glmesh = 0;
				switch (collision->m_geometry.m_meshFileType)
				{
					case UrdfGeometry::FILE_OBJ:
					{
						char relativeFileName[1024];
						char pathPrefix[1024];
						pathPrefix[0] = 0;
						if (creation.m_fileIO->findResourcePath(collision->m_geometry.m_meshFileName.c_str(), relativeFileName, 1024))
						{
							b3FileUtils::extractPath(relativeFileName, pathPrefix, 1024);
						}
						glmesh = LoadMeshFromObj(collision->m_geometry.m_meshFileName.c_str(), pathPrefix, creation.m_fileIO);
						
						break;
					}
					case UrdfGeometry::FILE_STL:
					{
						glmesh = LoadMeshFromSTL(collision->m_geometry.m_meshFileName.c_str(), creation.m_fileIO);
						break;
					}
					default:
					{

					}
				}
				if (glmesh && glmesh->m_numvertices)
				{
					for (i32 i = 0; i < glmesh->m_numvertices; i++)
					{
						physx::PxVec3 vert(
							glmesh->m_vertices->at(i).xyzw[0] * collision->m_geometry.m_meshScale[0],
							glmesh->m_vertices->at(i).xyzw[1] * collision->m_geometry.m_meshScale[1],
							glmesh->m_vertices->at(i).xyzw[2] * collision->m_geometry.m_meshScale[2]);
						vertices.push_back(vert);
					}

					physx::PxConvexMesh* convexMesh = createPhysXConvex(vertices.size(), &vertices[0], creation);

					shape = physx::PxRigidActorExt::createExclusiveShape(*linkPtr,
						physx::PxConvexMeshGeometry(convexMesh), *material);

					shape->setLocalPose(tr);
					cache.m_geomLocalPoses.push_back(tr);
					numShapes++;
				}
				break;

			}
			default:
			{
				printf("unknown physx shape\n");
			}
			}

			if (shape)
			{
				//see https://github.com/NVIDIAGameWorks/PhysX/issues/21
				physx::PxReal contactOffset = shape->getContactOffset();
				physx::PxReal restOffset = shape->getContactOffset();

				//shape->setContactOffset(physx::PxReal(.03));
				//shape->setRestOffset(physx::PxReal(.01)); //
			}
		}

	}
	
	
	return numShapes;
}

Transform2 ConvertURDF2PhysXInternal(
	const PhysXURDFImporter& u2b,
	ArticulationCreationInterface& creation,
	URDF2PhysXCachedData& cache, i32 urdfLinkIndex,
	const Transform2& parentTransformInWorldSpace, 
	bool createActiculation, tukk pathPrefix,
	i32 flags, 
	UrdfVisualShapeCache2* cachedLinkGraphicsShapesIn, 
	UrdfVisualShapeCache2* cachedLinkGraphicsShapesOut, 
	bool recursive)
{
	D3_PROFILE("ConvertURDF2PhysXInternal");
	//drx3DPrintf("start converting/extracting data from URDF interface\n");

	Transform2 linkTransformInWorldSpace;
	linkTransformInWorldSpace.setIdentity();

	i32 mbLinkIndex = cache.getMbIndexFromUrdfIndex(urdfLinkIndex);

	i32 urdfParentIndex = cache.getParentUrdfIndex(urdfLinkIndex);
	i32 mbParentIndex = cache.getMbIndexFromUrdfIndex(urdfParentIndex);
	
	

	//drx3DPrintf("mb link index = %d\n",mbLinkIndex);

	Transform2 parentLocalInertialFrame;
	parentLocalInertialFrame.setIdentity();
	Scalar parentMass(1);
	Vec3 parentLocalInertiaDiagonal(1, 1, 1);

	if (urdfParentIndex == -2)
	{
		//drx3DPrintf("root link has no parent\n");
	}
	else
	{
		//drx3DPrintf("urdf parent index = %d\n",urdfParentIndex);
		//drx3DPrintf("mb parent index = %d\n",mbParentIndex);
		//parentRigidBody = cache.getRigidBodyFromLink(urdfParentIndex);
		u2b.getMassAndInertia2(urdfParentIndex, parentMass, parentLocalInertiaDiagonal, parentLocalInertialFrame, flags);
	}

	Scalar mass = 0;
	Transform2 localInertialFrame;
	localInertialFrame.setIdentity();
	Vec3 localInertiaDiagonal(0, 0, 0);
	u2b.getMassAndInertia2(urdfLinkIndex, mass, localInertiaDiagonal, localInertialFrame, flags);

	Transform2 parent2joint;
	parent2joint.setIdentity();

	i32 jointType;
	Vec3 jointAxisInJointSpace;
	Scalar jointLowerLimit;
	Scalar jointUpperLimit;
	Scalar jointDamping;
	Scalar jointFriction;
	Scalar jointMaxForce;
	Scalar jointMaxVelocity;

	bool hasParentJoint = u2b.getJointInfo2(urdfLinkIndex, parent2joint, linkTransformInWorldSpace, jointAxisInJointSpace, jointType, jointLowerLimit, jointUpperLimit, jointDamping, jointFriction, jointMaxForce, jointMaxVelocity);

	Transform2 axis2Reference;
	axis2Reference.setIdentity();

	switch (jointType)
	{
	case URDFContinuousJoint:
	case URDFPrismaticJoint:
	case URDFRevoluteJoint:
	{
		//rotate from revolute 'axis' to standard X axis
		Vec3 refAxis(1, 0, 0);
		Vec3 axis = jointAxisInJointSpace;
		//Quat axis2ReferenceRot(Vec3(0, 1, 0), SIMD_HALF_PI);// = shortestArcQuat(refAxis, Vec3(axis[0], axis[1], axis[2]));
		Quat axis2ReferenceRot = shortestArcQuat(refAxis, Vec3(axis[0], axis[1], axis[2]));
		axis2Reference.setRotation(axis2ReferenceRot);
		break;
	}
	default:
	{
	}
	};
	
	parent2joint = parent2joint*axis2Reference;
	//localInertialFrame = axis2Reference.inverse()*localInertialFrame;

	STxt linkName = u2b.getLinkName(urdfLinkIndex);

	if (flags & CUF_USE_SDF)
	{
		parent2joint = parentTransformInWorldSpace.inverse() * linkTransformInWorldSpace;
	}
	else
	{
		if (flags & CUF_USE_MJCF)
		{
			linkTransformInWorldSpace = parentTransformInWorldSpace * linkTransformInWorldSpace;
		}
		else
		{
			linkTransformInWorldSpace = parentTransformInWorldSpace * parent2joint;
		}
	}

	
	i32 graphicsIndex;
	{
		D3_PROFILE("convertLinkVisualShapes");


		
		graphicsIndex = u2b.convertLinkVisualShapes3(urdfLinkIndex, pathPrefix, localInertialFrame, u2b.getUrdfLink(urdfLinkIndex), u2b.getUrdfModel(), -1, u2b.getBodyUniqueId(), creation.m_fileIO);
		

#if 0
		if (cachedLinkGraphicsShapesIn && cachedLinkGraphicsShapesIn->m_cachedUrdfLinkVisualShapeIndices.size() > (mbLinkIndex + 1))
		{
			graphicsIndex = cachedLinkGraphicsShapesIn->m_cachedUrdfLinkVisualShapeIndices[mbLinkIndex + 1];
			UrdfMaterialColor matColor = cachedLinkGraphicsShapesIn->m_cachedUrdfLinkColors[mbLinkIndex + 1];
			u2b.setLinkColor2(urdfLinkIndex, matColor);
		}
		else
		{
			graphicsIndex = 
			if (cachedLinkGraphicsShapesOut)
			{
				cachedLinkGraphicsShapesOut->m_cachedUrdfLinkVisualShapeIndices.push_back(graphicsIndex);
				UrdfMaterialColor matColor;
				u2b.getLinkColor2(urdfLinkIndex, matColor);
				cachedLinkGraphicsShapesOut->m_cachedUrdfLinkColors.push_back(matColor);
			}
		}
#endif
	}
	
	if (1)
	{
		UrdfMaterialColor matColor;
		Vec4 color2 = selectColor4();
		Vec3 specular(0.5, 0.5, 0.5);
		if (u2b.getLinkColor2(urdfLinkIndex, matColor))
		{
			color2 = matColor.m_rgbaColor;
			specular = matColor.m_specularColor;
		}

		/*
		if (visual->material.get())
		{
		color.setVal(visual->material->color.r,visual->material->color.g,visual->material->color.b);//,visual->material->color.a);
		}
		*/
		if (mass)
		{
			if (!(flags & CUF_USE_URDF_INERTIA))
			{
				//drx3DAssert(0);
				//compoundShape->calculateLocalInertia(mass, localInertiaDiagonal);
				Assert(localInertiaDiagonal[0] < 1e10);
				Assert(localInertiaDiagonal[1] < 1e10);
				Assert(localInertiaDiagonal[2] < 1e10);
			}
			URDFLinkContactInfo contactInfo;
			u2b.getLinkContactInfo(urdfLinkIndex, contactInfo);
			//temporary inertia scaling until we load inertia from URDF
			if (contactInfo.m_flags & URDF_CONTACT_HAS_INERTIA_SCALING)
			{
				localInertiaDiagonal *= contactInfo.m_inertiaScaling;
			}
		}

		
		Transform2 inertialFrameInWorldSpace = linkTransformInWorldSpace * localInertialFrame;
		bool canSleep = (flags & CUF_ENABLE_SLEEPING) != 0;

		physx::PxRigidActor* linkPtr = 0;
		
		physx::PxRigidBody* rbLinkPtr = 0;
		

		physx::PxTransform tr;
		tr.p = physx::PxVec3(linkTransformInWorldSpace.getOrigin().x(), linkTransformInWorldSpace.getOrigin().y(), linkTransformInWorldSpace.getOrigin().z());
		tr.q = physx::PxQuat(linkTransformInWorldSpace.getRotation().x(), linkTransformInWorldSpace.getRotation().y(), linkTransformInWorldSpace.getRotation().z(), linkTransformInWorldSpace.getRotation().w());
		bool isFixedBase = (mass == 0);  //todo: figure out when base is fixed
		

		if (!createActiculation)
		{

			if (isFixedBase)
			{
				physx::PxRigidStatic* s = creation.m_physics->createRigidStatic(tr);
				if ((cache.m_rigidStatic == 0) && (cache.m_rigidDynamic == 0))
				{
					cache.m_rigidStatic = s;
				}
				linkPtr = s;
			}
			else
			{
				
				physx::PxRigidDynamic* d = creation.m_physics->createRigidDynamic(tr);
				linkPtr = d;
				if ((cache.m_rigidStatic == 0) && (cache.m_rigidDynamic == 0))
				{
					cache.m_rigidDynamic = d;
				}
				rbLinkPtr = d;
								
			}
		}
		else
		{
			
			cache.m_linkTransWorldSpace.push_back(tr);
			cache.m_urdfLinkIndex.push_back(urdfLinkIndex);
			cache.m_parentUrdfLinkIndex.push_back(urdfParentIndex);
			cache.m_linkMasses.push_back(mass);

			if (cache.m_articulation == 0)
			{

				

				cache.m_articulation = creation.m_physics->createArticulationReducedCoordinate();

				if (isFixedBase)
				{
					cache.m_articulation->setArticulationFlags(physx::PxArticulationFlag::eFIX_BASE);
				}



				physx::PxArticulationLink* base = cache.m_articulation->createLink(NULL,tr);
				
				linkPtr = base;
				rbLinkPtr = base;

				
				//physx::PxRigidActorExt::createExclusiveShape(*base, PxBoxGeometry(0.5f, 0.25f, 1.5f), *gMaterial);
				physx::PxRigidBody& body = *base;
				
				//Now create the slider and fixed joints...

				//cache.m_articulation->setSolverIterationCounts(4);//todo: API?
				cache.m_articulation->setSolverIterationCounts(32);//todo: API?
				
				cache.m_jointTypes.push_back(physx::PxArticulationJointType::eUNDEFINED);
				cache.m_parentLocalPoses.push_back(physx::PxTransform());
				cache.m_childLocalPoses.push_back(physx::PxTransform());

				// Stabilization can create artefacts on jointed objects so we just disable it
				//cache.m_articulation->setStabilizationThreshold(0.0f);
				//cache.m_articulation->setMaxProjectionIterations(16);
				//cache.m_articulation->setSeparationTolerance(0.001f);

#if 0
				i32 totalNumJoints = cache.m_totalNumJoints1;
				cache.m_bulletMultiBody = creation.allocateMultiBody(urdfLinkIndex, totalNumJoints, mass, localInertiaDiagonal, isFixedBase, canSleep);
				if (flags & CUF_GLOBAL_VELOCITIES_MB)
				{
					cache.m_bulletMultiBody->useGlobalVelocities(true);
				}
				if (flags & CUF_USE_MJCF)
				{
					cache.m_bulletMultiBody->setBaseWorldTransform(linkTransformInWorldSpace);
				}

				
#endif

				cache.registerMultiBody(urdfLinkIndex, base, inertialFrameInWorldSpace, mass, localInertiaDiagonal, localInertialFrame);
			}
			else
			{

				physx::PxArticulationLink* parentLink = cache.getPhysxLinkFromLink(urdfParentIndex);
				
				physx::PxArticulationLink* childLink = cache.m_articulation->createLink(parentLink, tr);
				linkPtr = childLink;
				rbLinkPtr = childLink;
				
				physx::PxArticulationJointReducedCoordinate* joint = static_cast<physx::PxArticulationJointReducedCoordinate*>(childLink->getInboundJoint());
			
				switch (jointType)
				{
					case URDFFixedJoint:
					{
						joint->setJointType(physx::PxArticulationJointType::eFIX);
						break;
					}
					case URDFSphericalJoint:
					{
						joint->setJointType(physx::PxArticulationJointType::eSPHERICAL);
						joint->setMotion(physx::PxArticulationAxis::eTWIST, physx::PxArticulationMotion::eFREE);
						joint->setMotion(physx::PxArticulationAxis::eSWING2, physx::PxArticulationMotion::eFREE);
						joint->setMotion(physx::PxArticulationAxis::eSWING1, physx::PxArticulationMotion::eFREE);
						break;
					}
					case URDFContinuousJoint:
					case URDFRevoluteJoint:
					{
						joint->setJointType(physx::PxArticulationJointType::eREVOLUTE);
						joint->setMotion(physx::PxArticulationAxis::eTWIST, physx::PxArticulationMotion::eFREE);
						
						break;
					}
					case URDFPrismaticJoint:
					{
						joint->setJointType(physx::PxArticulationJointType::ePRISMATIC);
						joint->setMotion(physx::PxArticulationAxis::eX, physx::PxArticulationMotion::eFREE);
						break;
					}
					default:
					{
						joint->setJointType(physx::PxArticulationJointType::eFIX);
						Assert(0);
					}
				};
				
				cache.m_jointTypes.push_back(joint->getJointType());
				Transform2 offsetInA, offsetInB;

				offsetInA = parentLocalInertialFrame.inverse() *   parent2joint;
				offsetInB = (axis2Reference.inverse()*localInertialFrame).inverse();

				physx::PxTransform parentPose(physx::PxVec3(offsetInA.getOrigin()[0], offsetInA.getOrigin()[1], offsetInA.getOrigin()[2]),
					physx::PxQuat(offsetInA.getRotation()[0], offsetInA.getRotation()[1], offsetInA.getRotation()[2], offsetInA.getRotation()[3]));

				physx::PxTransform childPose(physx::PxVec3(offsetInB.getOrigin()[0], offsetInB.getOrigin()[1], offsetInB.getOrigin()[2]),
					physx::PxQuat(offsetInB.getRotation()[0], offsetInB.getRotation()[1], offsetInB.getRotation()[2], offsetInB.getRotation()[3]));

				cache.m_parentLocalPoses.push_back(parentPose);
				cache.m_childLocalPoses.push_back(childPose);

				joint->setParentPose(parentPose);
				joint->setChildPose(childPose);
			
				cache.registerMultiBody(urdfLinkIndex, childLink, inertialFrameInWorldSpace, mass, localInertiaDiagonal, localInertialFrame);
			}
			
			
		}


		if (linkPtr)
		{
			//todo: mem leaks
			MyPhysXUserData* userData = new MyPhysXUserData();
			userData->m_graphicsUniqueId = graphicsIndex;
			userData->m_bodyUniqueId = u2b.getBodyUniqueId();
			userData->m_linkIndex = mbLinkIndex;
			linkPtr->userData = userData;
		}

		//create collision shapes

		//physx::PxRigidActorExt::createExclusiveShape
		convertLinkPhysXShapes(u2b, cache, creation, urdfLinkIndex, pathPrefix, localInertialFrame, cache.m_articulation, mbLinkIndex, linkPtr);

		
		if (rbLinkPtr && mass)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*rbLinkPtr, mass);
		}
		
		//base->setMass(massOut);
		//base->setMassSpaceInertiaTensor(diagTensor);
		//base->setCMassLocalPose(PxTransform(com, orient));


		//create a joint if necessary
		if (hasParentJoint)
		{
			Transform2 offsetInA, offsetInB;
			offsetInA = parentLocalInertialFrame.inverse() * parent2joint;
			offsetInB = localInertialFrame.inverse();
			Quat parentRotToThis = offsetInB.getRotation() * offsetInA.inverse().getRotation();

			bool disableParentCollision = true;

			if (createActiculation && cache.m_articulation)
			{
#if 0
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointDamping = jointDamping;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointFriction = jointFriction;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointLowerLimit = jointLowerLimit;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointUpperLimit = jointUpperLimit;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointMaxForce = jointMaxForce;
				cache.m_bulletMultiBody->getLink(mbLinkIndex).m_jointMaxVelocity = jointMaxVelocity;
#endif
			}
		}


		if (createActiculation)
		{

		}
		else
		{
			
		}
	}//was if 'shape/compountShape'

	AlignedObjectArray<i32> urdfChildIndices;
	u2b.getLinkChildIndices(urdfLinkIndex, urdfChildIndices);

	i32 numChildren = urdfChildIndices.size();

	if (recursive)
	{
		for (i32 i = 0; i < numChildren; i++)
		{
			i32 urdfChildLinkIndex = urdfChildIndices[i];

			ConvertURDF2PhysXInternal(u2b, creation, cache, urdfChildLinkIndex, linkTransformInWorldSpace, createActiculation, pathPrefix, flags, cachedLinkGraphicsShapesIn, cachedLinkGraphicsShapesOut, recursive);
		}
	}
	return linkTransformInWorldSpace;
}







physx::PxBase* URDF2PhysX(physx::PxFoundation* foundation, physx::PxPhysics* physics, physx::PxCooking* cooking, physx::PxScene* scene, class PhysXURDFImporter& u2p, i32 flags, tukk pathPrefix, const Transform2& rootTransformInWorldSpace, struct CommonFileIOInterface* fileIO, bool createActiculation)
{
	URDF2PhysXCachedData cache;
	InitURDF2BulletCache(u2p, cache, flags);
	i32 urdfLinkIndex = u2p.getRootLinkIndex();
	i32 rootIndex = u2p.getRootLinkIndex();
	D3_PROFILE("ConvertURDF2Bullet");

	UrdfVisualShapeCache2 cachedLinkGraphicsShapesOut;
	UrdfVisualShapeCache2 cachedLinkGraphicsShapes;

	ArticulationCreationInterface creation;
	creation.m_foundation = foundation;
	creation.m_physics = physics;
	creation.m_cooking = cooking;
	creation.m_scene = scene;
	creation.m_fileIO = fileIO;

	

	bool recursive = (flags & CUF_MAINTAIN_LINK_ORDER) == 0;
	
	if (recursive)
	{
		ConvertURDF2PhysXInternal(u2p, creation, cache, urdfLinkIndex, rootTransformInWorldSpace, createActiculation, pathPrefix, flags, &cachedLinkGraphicsShapes, &cachedLinkGraphicsShapesOut, recursive);

	}
	else
	{
#if 0
		AlignedObjectArray<Transform2> parentTransforms;
		if (urdfLinkIndex >= parentTransforms.size())
		{
			parentTransforms.resize(urdfLinkIndex + 1);
		}
		parentTransforms[urdfLinkIndex] = rootTransformInWorldSpace;
		AlignedObjectArray<childParentIndex> allIndices;

		GetAllIndices(u2b, cache, urdfLinkIndex, -1, allIndices);
		allIndices.quickSort(MyIntCompareFunc);

		for (i32 i = 0; i < allIndices.size(); i++)
		{
			i32 urdfLinkIndex = allIndices[i].m_index;
			i32 parentIndex = allIndices[i].m_parentIndex;
			Transform2 parentTr = parentIndex >= 0 ? parentTransforms[parentIndex] : rootTransformInWorldSpace;
			Transform2 tr = ConvertURDF2BulletInternal(u2b, creation, cache, urdfLinkIndex, parentTr, world1, createActiculation, pathPrefix, flags, cachedLinkGraphicsShapes, &cachedLinkGraphicsShapesOut, recursive);
			if ((urdfLinkIndex + 1) >= parentTransforms.size())
			{
				parentTransforms.resize(urdfLinkIndex + 1);
			}
			parentTransforms[urdfLinkIndex] = tr;
		}
#endif

	}
#if 0
	if (cachedLinkGraphicsShapes && cachedLinkGraphicsShapesOut.m_cachedUrdfLinkVisualShapeIndices.size() > cachedLinkGraphicsShapes->m_cachedUrdfLinkVisualShapeIndices.size())
	{
		*cachedLinkGraphicsShapes = cachedLinkGraphicsShapesOut;
	}
#endif

	
	if (cache.m_articulation)
	{
#ifdef DEBUG_ARTICULATIONS
		printf("\n-----------------\n");
		printf("m_linkTransWorldSpace\n");
		for (i32 i = 0; i < cache.m_linkTransWorldSpace.size(); i++)
		{
			printf("PxTransform(PxVec3(%f,%f,%f), PxQuat(%f,%f,%f,%f),\n",
				cache.m_linkTransWorldSpace[i].p.x, cache.m_linkTransWorldSpace[i].p.y, cache.m_linkTransWorldSpace[i].p.z,
				cache.m_linkTransWorldSpace[i].q.x, cache.m_linkTransWorldSpace[i].q.y, cache.m_linkTransWorldSpace[i].q.z, cache.m_linkTransWorldSpace[i].q.w);
		}
		printf("m_parentLocalPoses\n");
		for (i32 i = 0; i < cache.m_parentLocalPoses.size(); i++)
		{
			printf("PxTransform(PxVec3(%f,%f,%f), PxQuat(%f,%f,%f,%f),\n",
				cache.m_parentLocalPoses[i].p.x, cache.m_parentLocalPoses[i].p.y, cache.m_parentLocalPoses[i].p.z,
				cache.m_parentLocalPoses[i].q.x, cache.m_parentLocalPoses[i].q.y, cache.m_parentLocalPoses[i].q.z, cache.m_parentLocalPoses[i].q.w);
		}

		printf("m_childLocalPoses\n");
		for (i32 i = 0; i < cache.m_childLocalPoses.size(); i++)
		{
			printf("PxTransform(PxVec3(%f,%f,%f), PxQuat(%f,%f,%f,%f),\n",
				cache.m_childLocalPoses[i].p.x, cache.m_childLocalPoses[i].p.y, cache.m_childLocalPoses[i].p.z,
				cache.m_childLocalPoses[i].q.x, cache.m_childLocalPoses[i].q.y, cache.m_childLocalPoses[i].q.z, cache.m_childLocalPoses[i].q.w);
		}

		printf("m_geomDimensions\n");
		for (i32 i = 0; i < cache.m_geomDimensions.size(); i++)
		{
			printf("PxVec3(%f,%f,%f),\n",
				cache.m_geomDimensions[i].x, cache.m_geomDimensions[i].y, cache.m_geomDimensions[i].z);
		}

		printf("m_geomLocalPoses\n");
		for (i32 i = 0; i < cache.m_geomLocalPoses.size(); i++)
		{
			printf("PxTransform(PxVec3(%f,%f,%f), PxQuat(%f,%f,%f,%f),\n",
				cache.m_geomLocalPoses[i].p.x, cache.m_geomLocalPoses[i].p.y, cache.m_geomLocalPoses[i].p.z,
				cache.m_geomLocalPoses[i].q.x, cache.m_geomLocalPoses[i].q.y, cache.m_geomLocalPoses[i].q.z, cache.m_geomLocalPoses[i].q.w);
		}

		printf("m_linkMaterials\n");
		for (i32 i = 0; i < cache.m_linkMaterials.size(); i++)
		{
			printf("PxVec3(%f,%f,%f),\n",
				cache.m_linkMaterials[i].x, cache.m_linkMaterials[i].y, cache.m_linkMaterials[i].z);
		}
#endif //DEBUG_ARTICULATIONS

		//see also https://github.com/NVIDIAGameWorks/PhysX/issues/43
		if ((flags & CUF_USE_SELF_COLLISION) == 0)
		{
			physx::PxU32 nbActors = cache.m_articulation->getNbLinks();;     // d3Max number of actors expected in the aggregate
			bool selfCollisions = false;
			physx::PxAggregate* aggregate = physics->createAggregate(nbActors, selfCollisions);
			aggregate->addArticulation(*cache.m_articulation);
			scene->addAggregate(*aggregate);
		}
		else
		{
			scene->addArticulation(*cache.m_articulation);
		}

		return  cache.m_articulation;
	}

	if (cache.m_rigidStatic)
	{
		
		scene->addActor(*cache.m_rigidStatic);
		return cache.m_rigidStatic;
	}
	if (cache.m_rigidDynamic)
	{
		scene->addActor(*cache.m_rigidDynamic);
		return cache.m_rigidDynamic;
	}
	return NULL;

}
