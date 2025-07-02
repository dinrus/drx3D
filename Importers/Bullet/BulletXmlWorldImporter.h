#ifndef DRX3D_DRX3D_XML_WORLD_IMPORTER_H
#define DRX3D_DRX3D_XML_WORLD_IMPORTER_H

#include <drx3D/Maths/Linear/Scalar.h>

class DynamicsWorld;

namespace tinyxml2
{
class XMLNode;
};

struct ConvexInternalShapeData;
struct CollisionShapeData;
#ifdef DRX3D_USE_DOUBLE_PRECISION
struct RigidBodyDoubleData;
struct TypedConstraintDoubleData;
#define RigidBodyData RigidBodyDoubleData
#define TypedConstraintData2 TypedConstraintDoubleData
#else
struct RigidBodyFloatData;
struct TypedConstraintFloatData;
#define TypedConstraintData2 TypedConstraintFloatData
#define RigidBodyData RigidBodyFloatData
#endif  //DRX3D_USE_DOUBLE_PRECISION

struct CompoundShapeChildData;

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include "WorldImporter.h"

class BulletXmlWorldImporter : public WorldImporter
{
protected:
	AlignedObjectArray<CollisionShapeData*> m_collisionShapeData;
	AlignedObjectArray<AlignedObjectArray<CompoundShapeChildData>*> m_compoundShapeChildDataArrays;
	AlignedObjectArray<RigidBodyData*> m_rigidBodyData;
	AlignedObjectArray<TypedConstraintData2*> m_constraintData;
	HashMap<HashPtr, uk> m_pointerLookup;
	i32 m_fileVersion;
	bool m_fileOk;

	void auto_serialize_root_level_children(tinyxml2::XMLNode* pParent);
	void auto_serialize(tinyxml2::XMLNode* pParent);

	void deSerializeVector3FloatData(tinyxml2::XMLNode* pParent, AlignedObjectArray<Vec3FloatData>& vectors);

	void fixupCollisionDataPointers(CollisionShapeData* shapeData);
	void fixupConstraintData(TypedConstraintData2* tcd);

	//collision shapes data
	void deSerializeCollisionShapeData(tinyxml2::XMLNode* pParent, CollisionShapeData* colShapeData);
	void deSerializeConvexInternalShapeData(tinyxml2::XMLNode* pParent);
	void deSerializeStaticPlaneShapeData(tinyxml2::XMLNode* pParent);
	void deSerializeCompoundShapeData(tinyxml2::XMLNode* pParent);
	void deSerializeCompoundShapeChildData(tinyxml2::XMLNode* pParent);
	void deSerializeConvexHullShapeData(tinyxml2::XMLNode* pParent);
	void deSerializeDynamicsWorldData(tinyxml2::XMLNode* parent);

	///bodies
	void deSerializeRigidBodyFloatData(tinyxml2::XMLNode* pParent);

	///constraints
	void deSerializeGeneric6DofConstraintData(tinyxml2::XMLNode* pParent);

public:
	BulletXmlWorldImporter(DynamicsWorld* world);

	virtual ~BulletXmlWorldImporter();

	bool loadFile(tukk fileName);
};

#endif  //DRX3D_DRX3D_XML_WORLD_IMPORTER_H
