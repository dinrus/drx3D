#include <drx3D/DynamicsCommon.h>

//for inverse dynamics, DeepMimic implementation
#include "RBDModel.h"
#include "RBDUtil.h"
#include "KinTree.h"

#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyMLCPConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLink.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointLimitConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyFixedConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySliderConstraint.h>


struct TempLink
{
	i32 m_parentIndex;
	const CollisionObject2* m_collider;
	double m_mass;
	i32 m_jointType;
	i32 m_dofOffset;
	i32 m_dofCount;
	Vec3 m_dVector;
	Vec3 m_eVector;
	Quat m_zeroRotParentToThis;
	Quat m_this_to_body1;
};

bool ExtractJointBodyFromTempLinks(AlignedObjectArray<TempLink>& links, Eigen::MatrixXd& bodyDefs, Eigen::MatrixXd& jointMat)
{

	bool result = true;

	i32 num_joints = links.size();

	AlignedObjectArray<Vec3> bodyToLinkPositions;
	AlignedObjectArray<Quat> bodyToLinkRotations;
	AlignedObjectArray<Quat> dVectorRot;
	bodyToLinkRotations.resize(num_joints);
	bodyToLinkPositions.resize(num_joints);
	dVectorRot.resize(num_joints);

	jointMat.resize(num_joints, 19);
	bodyDefs.resize(num_joints, 17);
	for (i32 i = 0; i < num_joints * 19; i++)
	{
		jointMat(i) = SIMD_INFINITY;
	}
	for (i32 i = 0; i < num_joints * 17; i++)
	{
		bodyDefs(i) = SIMD_INFINITY;
	}

	for (i32 i = 0; i < num_joints * 17; i++)
	{
		bodyDefs(i) = SIMD_INFINITY;
	}

	Scalar unk = -12345;

	i32 totalDofs = 0;

	for (i32 j = 0; j < num_joints; ++j)
	{

		i32 i = j;
		i32 parentIndex = links[j].m_parentIndex;

		cShape::eShape shapeType = cShape::eShapeNull;
		double param0 = 0, param1 = 0, param2 = 0;
		if (links[j].m_collider)
		{
			const CollisionShape* collisionShape = links[j].m_collider->getCollisionShape();
			if (collisionShape->isCompound())
			{
				const CompoundShape* compound = (const CompoundShape*)collisionShape;
				if (compound->getNumChildShapes() > 0)
				{
					collisionShape = compound->getChildShape(0);
				}
			}
			
			switch (collisionShape->getShapeType())
			{
			case BOX_SHAPE_PROXYTYPE:
			{
				shapeType = cShape::eShapeBox;
				BoxShape* box = (BoxShape*)collisionShape;
				param0 = box->getHalfExtentsWithMargin()[0] * 2;
				param1 = box->getHalfExtentsWithMargin()[1] * 2;
				param2 = box->getHalfExtentsWithMargin()[2] * 2;
				
				break;
			}
			case SPHERE_SHAPE_PROXYTYPE:
			{
				SphereShape* sphere = (SphereShape*)collisionShape;
				param0 = sphere->getRadius() * 2;
				param1 = sphere->getRadius() * 2;
				param2 = sphere->getRadius() * 2;
				shapeType = cShape::eShapeSphere;
				break;
			}
			case CAPSULE_SHAPE_PROXYTYPE:
			{
				CapsuleShape* caps = (CapsuleShape*)collisionShape;
				param0 = caps->getRadius() * 2;
				param1 = caps->getHalfHeight() * 2;
				param2 = caps->getRadius() * 2;
				shapeType = cShape::eShapeCapsule;
				break;
			}
			default:
			{
				//approximate by its box
				Transform2 identity;
				identity.setIdentity();
				Vec3 aabbMin, aabbMax;
				collisionShape->getAabb(identity, aabbMin, aabbMax);
				Vec3 halfExtents = (aabbMax - aabbMin) * Scalar(0.5);
				Scalar margin = collisionShape->getMargin();
				Scalar lx = Scalar(2.) * (halfExtents.x() + margin);
				Scalar ly = Scalar(2.) * (halfExtents.y() + margin);
				Scalar lz = Scalar(2.) * (halfExtents.z() + margin);
				param0 = lx;
				param1 = ly;
				param2 = lz;
				shapeType = cShape::eShapeBox;
			}
			}
		}


		Vec3 body_attach_pt1 = links[j].m_dVector;

		//tQuaternion body_to_parent_body = parent_to_parent_body * this_to_parent * body_to_this;
		//tQuaternion parent_to_parent_body = parent_body_to_parent.inverse();


		bodyDefs(i, cKinTree::eBodyParam0) = param0;
		bodyDefs(i, cKinTree::eBodyParam1) = param1;
		bodyDefs(i, cKinTree::eBodyParam2) = param2;

		bodyDefs(i, cKinTree::eBodyParamShape) = shapeType;
		bodyDefs(i, cKinTree::eBodyParamMass) = links[j].m_mass;


		bodyDefs(i, cKinTree::eBodyParamColGroup) = unk;
		bodyDefs(i, cKinTree::eBodyParamEnableFallContact) = unk;

		bodyDefs(i, cKinTree::eBodyColorR) = unk;
		bodyDefs(i, cKinTree::eBodyColorG) = unk;
		bodyDefs(i, cKinTree::eBodyColorB) = unk;
		bodyDefs(i, cKinTree::eBodyColorA) = unk;

		dVectorRot[j] = links[j].m_this_to_body1;

		Vec3 body_attach_pt2 = quatRotate(links[j].m_this_to_body1.inverse(), body_attach_pt1);
		bodyToLinkPositions[i] = body_attach_pt2;
		bodyDefs(i, cKinTree::eBodyParamAttachX) = body_attach_pt2[0];
		bodyDefs(i, cKinTree::eBodyParamAttachY) = body_attach_pt2[1];
		bodyDefs(i, cKinTree::eBodyParamAttachZ) = body_attach_pt2[2];
		Scalar bodyAttachThetaX = 0;
		Scalar bodyAttachThetaY = 0;
		Scalar bodyAttachThetaZ = 0;
		Quat body_to_this1 = links[j].m_this_to_body1.inverse();



		body_to_this1.getEulerZYX(bodyAttachThetaZ, bodyAttachThetaY, bodyAttachThetaX);
		bodyDefs(i, cKinTree::eBodyParamAttachThetaX) = bodyAttachThetaX;
		bodyDefs(i, cKinTree::eBodyParamAttachThetaY) = bodyAttachThetaY;
		bodyDefs(i, cKinTree::eBodyParamAttachThetaZ) = bodyAttachThetaZ;

		jointMat(i, cKinTree::eJointDescType) = links[j].m_jointType;
		jointMat(i, cKinTree::eJointDescParent) = parentIndex;


		Vec3 jointAttachPointMy = links[j].m_eVector;
		Vec3 jointAttachPointMyv0 = jointAttachPointMy;
		Vec3 parentBodyAttachPtMy(0, 0, 0);
		Quat parentBodyToLink;
		parentBodyToLink = Quat::getIdentity();
		Quat linkToParentBody = Quat::getIdentity();
		i32 parent_joint = links[j].m_parentIndex;

		if (parent_joint != gInvalidIdx)
		{
			parentBodyAttachPtMy = bodyToLinkPositions[parent_joint];
			parentBodyToLink = bodyToLinkRotations[parent_joint];
			linkToParentBody = parentBodyToLink.inverse();
		}
		parentBodyAttachPtMy = quatRotate(linkToParentBody, parentBodyAttachPtMy);
		//bodyToLinkRotations
		jointAttachPointMy += parentBodyAttachPtMy;
		jointAttachPointMy = quatRotate(linkToParentBody.inverse(), jointAttachPointMy);

		Vec3 parent_body_attach_pt1(0, 0, 0);
		if (parentIndex >= 0)
		{
			parent_body_attach_pt1 = links[parentIndex].m_dVector;
		}
		Quat myparent_body_to_body(0, 0, 0, 1);
		Quat mybody_to_parent_body(0, 0, 0, 1);
		Quat parent_body_to_body1 = links[i].m_zeroRotParentToThis;
		Quat body_to_parent_body1 = parent_body_to_body1.inverse();

		bodyToLinkRotations[i] = body_to_this1;

		jointMat(i, cKinTree::eJointDescAttachX) = jointAttachPointMy[0];
		jointMat(i, cKinTree::eJointDescAttachY) = jointAttachPointMy[1];
		jointMat(i, cKinTree::eJointDescAttachZ) = jointAttachPointMy[2];


		Quat parent2parent_body2(0, 0, 0, 1);

		if (parent_joint >= 0)
		{
			//parent2parent_body2 = bulletMB->getLink(parent_joint).m_dVectorRot;
			parent2parent_body2 = dVectorRot[parent_joint];
		}
		///Quaternion this2bodyA = bulletMB->getLink(j).m_dVectorRot;
		Quat this2bodyA = dVectorRot[j];

		Quat parent_body_2_body = links[j].m_zeroRotParentToThis;
		Quat combined2 = parent_body_2_body.inverse();
		Quat recoverthis2parent = parent2parent_body2.inverse()*combined2*this2bodyA;// body2this.inverse();
		Scalar eulZ, eulY, eulX;
		recoverthis2parent.getEulerZYX(eulZ, eulY, eulX);


		jointMat(i, cKinTree::eJointDescAttachThetaX) = eulX;
		jointMat(i, cKinTree::eJointDescAttachThetaY) = eulY;
		jointMat(i, cKinTree::eJointDescAttachThetaZ) = eulZ;


		jointMat(i, cKinTree::eJointDescLimLow0) = unk;
		jointMat(i, cKinTree::eJointDescLimLow1) = unk;
		jointMat(i, cKinTree::eJointDescLimLow2) = unk;
		jointMat(i, cKinTree::eJointDescLimHigh0) = unk;
		jointMat(i, cKinTree::eJointDescLimHigh1) = unk;
		jointMat(i, cKinTree::eJointDescLimHigh2) = unk;
		jointMat(i, cKinTree::eJointDescTorqueLim) = unk;

		jointMat(i, cKinTree::eJointDescForceLim) = unk;
		jointMat(i, cKinTree::eJointDescIsEndEffector) = unk;
		jointMat(i, cKinTree::eJointDescDiffWeight) = unk;
		jointMat(i, cKinTree::eJointDescParamOffset) = totalDofs;
		totalDofs += links[j].m_dofCount;

	}
	return result;
}


void ExtractJointBodyFromBullet(const MultiBody* bulletMB, Eigen::MatrixXd& bodyDefs, Eigen::MatrixXd& jointMat)
{
	AlignedObjectArray<TempLink> links;

	i32 numBaseShapes = 0;
	if (bulletMB->getBaseCollider())
	{
		switch (bulletMB->getBaseCollider()->getCollisionShape()->getShapeType())
		{
			case CAPSULE_SHAPE_PROXYTYPE:
			case SPHERE_SHAPE_PROXYTYPE:
			case BOX_SHAPE_PROXYTYPE:
			{
				numBaseShapes++;
				break;
			}
			case COMPOUND_SHAPE_PROXYTYPE:
			{
				CompoundShape* compound = (CompoundShape*)bulletMB->getBaseCollider()->getCollisionShape();
				numBaseShapes += compound->getNumChildShapes();
				break;
			}
			default:
			{
			}
		}

	}

	//links include the 'base' and its childlinks
	i32 baseLink = numBaseShapes? 1 : 0;
	links.resize(bulletMB->getNumLinks() + baseLink);
	for (i32 i = 0; i < links.size(); i++)
	{
		memset(&links[i], 0xffffffff, sizeof(TempLink));
	}

	i32 totalDofs = 0;
	if (numBaseShapes)
	{
		//links[0] is the root/base
		links[0].m_parentIndex = -1;
		links[0].m_collider = bulletMB->getBaseCollider();
		links[0].m_mass = bulletMB->getBaseMass();
		links[0].m_jointType = (bulletMB->hasFixedBase()) ? cKinTree::eJointTypeFixed : cKinTree::eJointTypeNone;
		links[0].m_dofOffset = 0;
		links[0].m_dofCount = 7;
		links[0].m_dVector.setVal(0, 0, 0);
		links[0].m_eVector.setVal(0, 0, 0);
		links[0].m_zeroRotParentToThis = Quat(0, 0, 0, 1);
		links[0].m_this_to_body1 = Quat(0, 0, 0, 1);
		totalDofs = 7;
	}
	

	for (i32 j = 0; j < bulletMB->getNumLinks(); ++j)
	{
		i32 parentIndex = bulletMB->getLink(j).m_parent;
		links[j + baseLink].m_parentIndex = parentIndex + baseLink;
		links[j + baseLink].m_collider = bulletMB->getLinkCollider(j);
		links[j + baseLink].m_mass = bulletMB->getLink(j).m_mass;

		i32 jointType = 0;
		Quat this_to_body1(0, 0, 0, 1);
		i32 dofCount = 0;

		if ((baseLink)==0 &&j == 0)//for 'root' either use fixed or none
		{
			dofCount = 7;
			links[j].m_parentIndex = -1;
			links[j].m_jointType = (bulletMB->hasFixedBase()) ? cKinTree::eJointTypeFixed : cKinTree::eJointTypeNone;
			links[j].m_dofOffset = 0;
			links[j].m_dofCount = dofCount;
			
			
			links[j].m_zeroRotParentToThis = Quat(0, 0, 0, 1);
			//links[j].m_dVector.setVal(0, 0, 0);
			links[j].m_dVector = bulletMB->getLink(j).m_dVector;
			links[j].m_eVector.setVal(0, 0, 0);
			//links[j].m_eVector = bulletMB->getLink(j).m_eVector;
			links[j].m_zeroRotParentToThis = bulletMB->getLink(j).m_zeroRotParentToThis;

			links[j].m_this_to_body1 = Quat(0, 0, 0, 1);
			totalDofs = 7;
		}
		else
		{

			switch (bulletMB->getLink(j).m_jointType)
			{
			case MultibodyLink::eFixed:
			{
				jointType = cKinTree::eJointTypeFixed;
				break;
			}
			case MultibodyLink::ePrismatic:
			{
				dofCount = 1;
				Vec3 refAxis(1, 0, 0);
				Vec3 axis = bulletMB->getLink(j).getAxisTop(0);
				this_to_body1 = shortestArcQuat(refAxis, Vec3(axis[0], axis[1], axis[2]));
				jointType = cKinTree::eJointTypePrismatic;
				break;
			}
			case MultibodyLink::eSpherical:
			{
				dofCount = 4;//??
				jointType = cKinTree::eJointTypeSpherical;
				break;
			}
			case MultibodyLink::eRevolute:
			{
				dofCount = 1;
				Vec3 refAxis(0, 0, 1);
				Vec3 axis = bulletMB->getLink(j).getAxisTop(0);
				this_to_body1 = shortestArcQuat(refAxis, Vec3(axis[0], axis[1], axis[2]));
				jointType = cKinTree::eJointTypeRevolute;
				break;
			}
			default:
			{
			}
			}
			links[j + baseLink].m_jointType = jointType;
			links[j + baseLink].m_dofOffset = totalDofs;
			links[j + baseLink].m_dofCount = dofCount;
			links[j + baseLink].m_dVector = bulletMB->getLink(j).m_dVector;
			links[j + baseLink].m_eVector = bulletMB->getLink(j).m_eVector;
			links[j + baseLink].m_zeroRotParentToThis = bulletMB->getLink(j).m_zeroRotParentToThis;
			links[j + baseLink].m_this_to_body1 = this_to_body1;
		}

		totalDofs += dofCount;
	}

	ExtractJointBodyFromTempLinks(links, bodyDefs, jointMat);
}
