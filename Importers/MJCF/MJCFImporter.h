#ifndef DRX3D_MJCF_IMPORTER_H
#define DRX3D_MJCF_IMPORTER_H

#include <drx3D/Importers/URDF/URDFImporterInterface.h>
#include <drx3D/Importers/URDF/UrdfRenderingInterface.h>

struct MJCFErrorLogger
{
	virtual ~MJCFErrorLogger() {}
	virtual void reportError(tukk error) = 0;
	virtual void reportWarning(tukk warning) = 0;
	virtual void printMessage(tukk msg) = 0;
};

struct MJCFURDFTexture
{
	i32 m_width;
	i32 m_height;
	u8* textureData1;
	bool m_isCached;
};

class BulletMJCFImporter : public URDFImporterInterface
{
	struct BulletMJCFImporterInternalData* m_data;

	void convertURDFToVisualShapeInternal(const struct UrdfVisual* visual, tukk urdfPathPrefix, const Transform2& visualTransform, AlignedObjectArray<struct GLInstanceVertex>& verticesOut, AlignedObjectArray<i32>& indicesOut, AlignedObjectArray<MJCFURDFTexture>& texturesOut) const;

public:
	BulletMJCFImporter(struct GUIHelperInterface* helper, UrdfRenderingInterface* customConverter,  struct CommonFileIOInterface* fileIO, i32 flags);
	virtual ~BulletMJCFImporter();

	virtual bool parseMJCFString(tukk xmlString, MJCFErrorLogger* logger);

	virtual bool loadMJCF(tukk fileName, MJCFErrorLogger* logger, bool forceFixedBase = false);

	virtual bool loadURDF(tukk fileName, bool forceFixedBase = false)
	{
		return false;
	}

	virtual bool loadSDF(tukk fileName, bool forceFixedBase = false) { return false; }

	virtual tukk getPathPrefix();

	///return >=0 for the root link index, -1 if there is no root link
	virtual i32 getRootLinkIndex() const;

	///pure virtual interfaces, precondition is a valid linkIndex (you can assert/terminate if the linkIndex is out of range)
	virtual STxt getLinkName(i32 linkIndex) const;

	virtual STxt getBodyName() const;

	/// optional method to provide the link color. return true if the color is available and copied into colorRGBA, return false otherwise
	virtual bool getLinkColor(i32 linkIndex, Vec4& colorRGBA) const;
	bool getLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const;

	//optional method to get collision group (type) and mask (affinity)
	virtual i32 getCollisionGroupAndMask(i32 linkIndex, i32& colGroup, i32& colMask) const;

	///this API will likely change, don't override it!
	virtual bool getLinkContactInfo(i32 linkIndex, URDFLinkContactInfo& contactInfo) const;

	virtual STxt getJointName(i32 linkIndex) const;

	//fill mass and inertial data. If inertial data is missing, please initialize mass, inertia to sensitive values, and inertialFrame to identity.
	virtual void getMassAndInertia(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame) const;

	///fill an array of child link indices for this link, AlignedObjectArray behaves like a std::vector so just use push_back and resize(0) if needed
	virtual void getLinkChildIndices(i32 urdfLinkIndex, AlignedObjectArray<i32>& childLinkIndices) const;

	virtual bool getJointInfo(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction) const;
	virtual bool getJointInfo2(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity) const;

	virtual bool getRootTransformInWorld(Transform2& rootTransformInWorld) const;

	virtual i32 convertLinkVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& inertialFrame) const;

	virtual void convertLinkVisualShapes2(i32 linkIndex, i32 urdfIndex, tukk pathPrefix, const Transform2& inertialFrame, class CollisionObject2* colObj, i32 objectIndex) const;
	virtual void setBodyUniqueId(i32 bodyId);
	virtual i32 getBodyUniqueId() const;

	virtual class CompoundShape* convertLinkCollisionShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const;
	virtual i32 getNumAllocatedCollisionShapes() const;
	virtual class CollisionShape* getAllocatedCollisionShape(i32 index);
	virtual i32 getNumModels() const;
	virtual void activateModel(i32 modelIndex);

	virtual i32 getNumAllocatedMeshInterfaces() const;
	virtual StridingMeshInterface* getAllocatedMeshInterface(i32 index);
};

#endif  //DRX3D_MJCF_IMPORTER_H
