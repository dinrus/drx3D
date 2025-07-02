#ifndef DRX3D_URDF_IMPORTER_H
#define DRX3D_URDF_IMPORTER_H

#include "URDFImporterInterface.h"
#include "UrdfRenderingInterface.h"
#include "UrdfParser.h"

struct BulletURDFTexture
{
	i32 m_width;
	i32 m_height;
	u8* textureData1;
	bool m_isCached;
};

///URDFImporter can deal with URDF and (soon) SDF files
class URDFImporter : public URDFImporterInterface
{
	struct URDFInternalData* m_data;

public:
	URDFImporter(struct GUIHelperInterface* helper, UrdfRenderingInterface* customConverter, struct CommonFileIOInterface* fileIO=0,double globalScaling=1, i32 flags=0);

	virtual ~URDFImporter();

	virtual bool loadURDF(tukk fileName, bool forceFixedBase = false);

	//warning: some quick test to load SDF: we 'activate' a model, so we can re-use URDF code path
	virtual bool loadSDF(tukk fileName, bool forceFixedBase = false);
	virtual i32 getNumModels() const;
	virtual void activateModel(i32 modelIndex);
	virtual void setBodyUniqueId(i32 bodyId);
	virtual i32 getBodyUniqueId() const;
	tukk getPathPrefix();

	void printTree();  //for debugging

	virtual i32 getRootLinkIndex() const;

	virtual void getLinkChildIndices(i32 linkIndex, AlignedObjectArray<i32>& childLinkIndices) const;

	virtual STxt getBodyName() const;

	virtual STxt getLinkName(i32 linkIndex) const;

	virtual bool getLinkColor(i32 linkIndex, Vec4& colorRGBA) const;

	virtual bool getLinkColor2(i32 linkIndex, UrdfMaterialColor& matCol) const;

	virtual void setLinkColor2(i32 linkIndex, struct UrdfMaterialColor& matCol) const;

	virtual bool getLinkContactInfo(i32 urdflinkIndex, URDFLinkContactInfo& contactInfo) const;

	virtual bool getLinkAudioSource(i32 linkIndex, SDFAudioSource& audioSource) const;

	virtual STxt getJointName(i32 linkIndex) const;

	virtual void getMassAndInertia(i32 linkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame) const;
	virtual void getMassAndInertia2(i32 urdfLinkIndex, Scalar& mass, Vec3& localInertiaDiagonal, Transform2& inertialFrame, i32 flags) const;

	virtual bool getJointInfo(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction) const;
	virtual bool getJointInfo2(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity) const;
	virtual bool getJointInfo3(i32 urdfLinkIndex, Transform2& parent2joint, Transform2& linkTransformInWorld, Vec3& jointAxisInJointSpace, i32& jointType, Scalar& jointLowerLimit, Scalar& jointUpperLimit, Scalar& jointDamping, Scalar& jointFriction, Scalar& jointMaxForce, Scalar& jointMaxVelocity, Scalar& twistLimit) const;

	virtual bool getRootTransformInWorld(Transform2& rootTransformInWorld) const;
	virtual void setRootTransformInWorld(const Transform2& rootTransformInWorld);

	virtual i32 convertLinkVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& inertialFrame) const;

	virtual void convertLinkVisualShapes2(i32 linkIndex, i32 urdfIndex, tukk pathPrefix, const Transform2& inertialFrame, class CollisionObject2* colObj, i32 bodyUniqueId) const;

	class CollisionShape* convertURDFToCollisionShape(const struct UrdfCollision* collision, tukk urdfPathPrefix) const;

	virtual i32 getUrdfFromCollisionShape(const CollisionShape* collisionShape, UrdfCollision& collision) const;

	///todo(erwincoumans) refactor this convertLinkCollisionShapes/memory allocation

	virtual const struct UrdfModel* getUrdfModel() const;

	virtual class CompoundShape* convertLinkCollisionShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame) const;

	virtual i32 getCollisionGroupAndMask(i32 linkIndex, i32& colGroup, i32& colMask) const;

	virtual i32 getNumAllocatedCollisionShapes() const;
	virtual class CollisionShape* getAllocatedCollisionShape(i32 index);

	virtual i32 getNumAllocatedMeshInterfaces() const;
	virtual class StridingMeshInterface* getAllocatedMeshInterface(i32 index);

	virtual i32 getNumAllocatedTextures() const;
	virtual i32 getAllocatedTexture(i32 index) const;

	virtual void setEnableTinyRenderer(bool enable);
	void convertURDFToVisualShapeInternal(const struct UrdfVisual* visual, tukk urdfPathPrefix, const class Transform2& visualTransform, AlignedObjectArray<struct GLInstanceVertex>& verticesOut, AlignedObjectArray<i32>& indicesOut, AlignedObjectArray<struct BulletURDFTexture>& texturesOut, struct b3ImportMeshData& meshData) const;
	const struct UrdfDeformable& getDeformableModel() const;
	const struct UrdfReducedDeformable& getReducedDeformableModel() const;
};

#endif  //DRX3D_URDF_IMPORTER_H
