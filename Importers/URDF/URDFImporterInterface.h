#ifndef URDF_IMPORTER_INTERFACE_H
#define URDF_IMPORTER_INTERFACE_H

#include <string>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include "URDFJointTypes.h"
#include "SDFAudioTypes.h"

class URDFImporterInterface
{
public:
	virtual ~URDFImporterInterface() {}

	virtual bool loadURDF(tukk fileName, bool forceFixedBase = false) = 0;

	virtual bool loadSDF(tukk fileName, bool forceFixedBase = false) { return false; }

	virtual tukk getPathPrefix() = 0;

	///return >=0 for the root link index, -1 if there is no root link
	virtual i32 getRootLinkIndex() const = 0;

	///pure virtual interfaces, precondition is a valid linkIndex (you can assert/terminate if the linkIndex is out of range)
	virtual STxt getLinkName(i32 linkIndex) const = 0;

	//various derived class in internal source code break with new pure virtual methods, so provide some default implementation
	virtual STxt getBodyName() const
	{
		return "";
	}

	/// optional method to provide the link color. return true if the color is available and copied into colorRGBA, return false otherwise
	virtual bool getLinkColor(i32 linkIndex, Vec4& colorRGBA) const { return false; }

	virtual bool getLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const { return false; }
	virtual void setLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const {}

	virtual i32 getCollisionGroupAndMask(i32 linkIndex, i32& colGroup, i32& colMask) const { return 0; }
	///this API will likely change, don't override it!
	virtual bool getLinkContactInfo(i32 linkIndex, URDFLinkContactInfo& contactInfo) const { return false; }

	virtual bool getLinkAudioSource(i32 linkIndex, SDFAudioSource& audioSource) const { return false; }

	virtual STxt getJointName(i32 linkIndex) const = 0;

	//fill mass and inertial data. If inertial data is missing, please initialize mass, inertia to sensitive values, and inertialFrame to identity.
	virtual void getMassAndInertia(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame) const = 0;
	virtual void getMassAndInertia2(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame, i32 flags) const
	{
		getMassAndInertia(urdfLinkIndex, mass, localInertiaDiagonal, inertialFrame);
	}

	///fill an array of child link indices for this link, AlignedObjectArray behaves like a std::vector so just use push_back and resize(0) if needed
	virtual void getLinkChildIndices(i32 urdfLinkIndex, AlignedObjectArray<i32>& childLinkIndices) const = 0;

	virtual bool getJointInfo(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction) const = 0;

	virtual bool getJointInfo2(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity) const
	{
		//backwards compatibility for custom file importers
		jointMaxForce = 0;
		jointMaxVelocity = 0;
		return getJointInfo(urdfLinkIndex, parent2joint, linkTransformInWorld, jointAxisInJointSpace, jointType, jointLowerLimit, jointUpperLimit, jointDamping, jointFriction);
	};

	virtual bool getJointInfo3(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity, Scalar& twistLimit) const
	{
		//backwards compatibility for custom file importers
		twistLimit = 0;
		return getJointInfo2(urdfLinkIndex, parent2joint, linkTransformInWorld, jointAxisInJointSpace, jointType, jointLowerLimit, jointUpperLimit, jointDamping, jointFriction, jointMaxForce, jointMaxVelocity);
	};

	virtual bool getRootTransformInWorld(Transform2& rootTransformInWorld) const = 0;
	virtual void setRootTransformInWorld(const Transform2& rootTransformInWorld) {}

	///quick hack: need to rethink the API/dependencies of this
	virtual i32 convertLinkVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& inertialFrame) const { return -1; }

	virtual void convertLinkVisualShapes2(i32 linkIndex, i32 urdfIndex, tukk pathPrefix, const Transform2& inertialFrame, class CollisionObject2* colObj, i32 objectIndex) const {}
	virtual void setBodyUniqueId(i32 bodyId) {}
	virtual i32 getBodyUniqueId() const { return 0; }

	//default implementation for backward compatibility
	virtual class CompoundShape* convertLinkCollisionShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const = 0;
	virtual i32 getUrdfFromCollisionShape(const class CollisionShape* collisionShape, struct UrdfCollision& collision) const
	{
		return 0;
	}

	virtual const struct UrdfLink* getUrdfLink(i32 urdfLinkIndex) const
	{
		return 0;
	}

	virtual const struct UrdfModel* getUrdfModel() const { return 0; };

	virtual i32 getNumAllocatedCollisionShapes() const { return 0; }
	virtual class CollisionShape* getAllocatedCollisionShape(i32 /*index*/) { return 0; }
	virtual i32 getNumModels() const { return 0; }
	virtual void activateModel(i32 /*modelIndex*/) {}
	virtual i32 getNumAllocatedMeshInterfaces() const { return 0; }

	virtual i32 getNumAllocatedTextures() const { return 0; }
	virtual i32 getAllocatedTexture(i32 index) const { return 0; }

	virtual class StridingMeshInterface* getAllocatedMeshInterface(i32 index) { return 0; }
};

#endif  //URDF_IMPORTER_INTERFACE_H
