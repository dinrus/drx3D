#include <drx3D/Importers/Bullet/BulletXmlWorldImporter.h>
#include <X/tinyxml2/tinyxml2.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Importers/Bullet/string_split.h>

using namespace tinyxml2;

struct MyLocalCaster
{
	uk m_ptr;
	i32 m_int;
	MyLocalCaster()
		: m_ptr(0)
	{
	}
};

BulletXmlWorldImporter::BulletXmlWorldImporter(DynamicsWorld* world)
	: WorldImporter(world),
	  m_fileVersion(-1),
	  m_fileOk(false)
{
}

BulletXmlWorldImporter::~BulletXmlWorldImporter()
{
}

#if 0
static i32 get_double_attribute_by_name(const XMLElement* pElement, tukk attribName,double* value)
{
	if ( !pElement )
		return 0;

	const XMLAttribute* pAttrib=pElement->FirstAttribute();
	while (pAttrib)
	{
		if (pAttrib->Name()==attribName)
			if (pAttrib->QueryDoubleVal(value)==TIXML_SUCCESS)
				return 1;
		pAttrib=pAttrib->Next();
	}
	return 0;
}
#endif

static i32 get_int_attribute_by_name(const XMLElement* pElement, tukk attribName, i32* value)
{
	if (!pElement)
		return 0;

	const XMLAttribute* pAttrib = pElement->FirstAttribute();
	while (pAttrib)
	{
		if (!strcmp(pAttrib->Name(), attribName))
			if (pAttrib->QueryIntVal(value) == XML_SUCCESS)
				return 1;
		//		if (pAttrib->QueryDoubleVal(&dval)==TIXML_SUCCESS) printf( " d=%1.1f", dval);
		pAttrib = pAttrib->Next();
	}
	return 0;
}

void stringToFloatArray(const STxt& string, AlignedObjectArray<float>& floats)
{
	AlignedObjectArray<STxt> pieces;

	bullet_utils::split(pieces, string, " ");
	for (i32 i = 0; i < pieces.size(); ++i)
	{
		Assert(pieces[i] != "");
		floats.push_back((float)atof(pieces[i].c_str()));
	}
}

static Vec3FloatData TextToVector3Data(tukk txt)
{
	Assert(txt);
	AlignedObjectArray<float> floats;
	stringToFloatArray(txt, floats);
	Assert(floats.size() == 4);

	Vec3FloatData vec4;
	vec4.m_floats[0] = floats[0];
	vec4.m_floats[1] = floats[1];
	vec4.m_floats[2] = floats[2];
	vec4.m_floats[3] = floats[3];
	return vec4;
}

void BulletXmlWorldImporter::deSerializeVector3FloatData(XMLNode* pParent, AlignedObjectArray<Vec3FloatData>& vectors)
{
	XMLNode* flNode = pParent->FirstChildElement("m_floats");
	Assert(flNode);
	while (flNode && flNode->FirstChildElement())
	{
		XMLText* pText = flNode->FirstChildElement()->ToText();
		//		printf("value = %s\n",pText->Val());
		Vec3FloatData vec4 = TextToVector3Data(pText->Val());
		vectors.push_back(vec4);
		flNode = flNode->NextSibling();
	}
}

#define SET_INT_VALUE(xmlnode, targetdata, argname)                                                          \
	Assert((xmlnode)->FirstChildElement(#argname) && (xmlnode)->FirstChildElement(#argname)->ToElement()); \
	if ((xmlnode)->FirstChildElement(#argname) && (xmlnode)->FirstChildElement(#argname)->ToElement())       \
		(targetdata)->argname = (i32)atof(xmlnode->FirstChildElement(#argname)->ToElement()->GetText());

#define SET_FLOAT_VALUE(xmlnode, targetdata, argname)                                                        \
	Assert((xmlnode)->FirstChildElement(#argname) && (xmlnode)->FirstChildElement(#argname)->ToElement()); \
	if ((xmlnode)->FirstChildElement(#argname) && (xmlnode)->FirstChildElement(#argname)->ToElement())       \
		(targetdata)->argname = (float)atof(xmlnode->FirstChildElement(#argname)->ToElement()->GetText());

#define SET_POINTER_VALUE(xmlnode, targetdata, argname, pointertype) \
	{                                                                \
		XMLNode* node = xmlnode->FirstChildElement(#argname);        \
		Assert(node);                                              \
		if (node)                                                    \
		{                                                            \
			tukk txt = (node)->ToElement()->GetText();        \
			MyLocalCaster cast;                                      \
			cast.m_int = (i32)atof(txt);                             \
			(targetdata).argname = (pointertype)cast.m_ptr;          \
		}                                                            \
	}

#define SET_VECTOR4_VALUE(xmlnode, targetdata, argname)                            \
	{                                                                              \
		XMLNode* flNode = xmlnode->FirstChildElement(#argname);                    \
		Assert(flNode);                                                          \
		if (flNode && flNode->FirstChildElement())                                 \
		{                                                                          \
			tukk txt = flNode->FirstChildElement()->ToElement()->GetText(); \
			Vec3FloatData vec4 = TextToVector3Data(txt);                      \
			(targetdata)->argname.m_floats[0] = vec4.m_floats[0];                  \
			(targetdata)->argname.m_floats[1] = vec4.m_floats[1];                  \
			(targetdata)->argname.m_floats[2] = vec4.m_floats[2];                  \
			(targetdata)->argname.m_floats[3] = vec4.m_floats[3];                  \
		}                                                                          \
	}

#define SET_MATRIX33_VALUE(n, targetdata, argname)                                      \
	{                                                                                   \
		XMLNode* xmlnode = n->FirstChildElement(#argname);                              \
		Assert(xmlnode);                                                              \
		if (xmlnode)                                                                    \
		{                                                                               \
			XMLNode* eleNode = xmlnode->FirstChildElement("m_el");                      \
			Assert(eleNode);                                                          \
			if (eleNode && eleNode->FirstChildElement())                                \
			{                                                                           \
				tukk txt = eleNode->FirstChildElement()->ToElement()->GetText(); \
				Vec3FloatData vec4 = TextToVector3Data(txt);                       \
				(targetdata)->argname.m_el[0].m_floats[0] = vec4.m_floats[0];           \
				(targetdata)->argname.m_el[0].m_floats[1] = vec4.m_floats[1];           \
				(targetdata)->argname.m_el[0].m_floats[2] = vec4.m_floats[2];           \
				(targetdata)->argname.m_el[0].m_floats[3] = vec4.m_floats[3];           \
                                                                                        \
				XMLNode* n1 = eleNode->FirstChildElement()->NextSibling();              \
				Assert(n1);                                                           \
				if (n1)                                                                 \
				{                                                                       \
					tukk txt = n1->ToElement()->GetText();                       \
					Vec3FloatData vec4 = TextToVector3Data(txt);                   \
					(targetdata)->argname.m_el[1].m_floats[0] = vec4.m_floats[0];       \
					(targetdata)->argname.m_el[1].m_floats[1] = vec4.m_floats[1];       \
					(targetdata)->argname.m_el[1].m_floats[2] = vec4.m_floats[2];       \
					(targetdata)->argname.m_el[1].m_floats[3] = vec4.m_floats[3];       \
                                                                                        \
					XMLNode* n2 = n1->NextSibling();                                    \
					Assert(n2);                                                       \
					if (n2)                                                             \
					{                                                                   \
						tukk txt = n2->ToElement()->GetText();                   \
						Vec3FloatData vec4 = TextToVector3Data(txt);               \
						(targetdata)->argname.m_el[2].m_floats[0] = vec4.m_floats[0];   \
						(targetdata)->argname.m_el[2].m_floats[1] = vec4.m_floats[1];   \
						(targetdata)->argname.m_el[2].m_floats[2] = vec4.m_floats[2];   \
						(targetdata)->argname.m_el[2].m_floats[3] = vec4.m_floats[3];   \
					}                                                                   \
				}                                                                       \
			}                                                                           \
		}                                                                               \
	}

#define SET_TRANSFORM_VALUE(n, targetdata, argname)                     \
	{                                                                   \
		XMLNode* trNode = n->FirstChildElement(#argname);               \
		Assert(trNode);                                               \
		if (trNode)                                                     \
		{                                                               \
			SET_VECTOR4_VALUE(trNode, &(targetdata)->argname, m_origin) \
			SET_MATRIX33_VALUE(trNode, &(targetdata)->argname, m_basis) \
		}                                                               \
	}

void BulletXmlWorldImporter::deSerializeCollisionShapeData(XMLNode* pParent, CollisionShapeData* colShapeData)
{
	SET_INT_VALUE(pParent, colShapeData, m_shapeType)
	colShapeData->m_name = 0;
}

void BulletXmlWorldImporter::deSerializeConvexHullShapeData(XMLNode* pParent)
{
	MyLocalCaster cast;
	get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int);

	ConvexHullShapeData* convexHullData = (ConvexHullShapeData*)AlignedAlloc(sizeof(ConvexHullShapeData), 16);

	XMLNode* xmlConvexInt = pParent->FirstChildElement("m_convexInternalShapeData");
	Assert(xmlConvexInt);

	XMLNode* xmlColShape = xmlConvexInt->FirstChildElement("m_collisionShapeData");
	Assert(xmlColShape);

	deSerializeCollisionShapeData(xmlColShape, &convexHullData->m_convexInternalShapeData.m_collisionShapeData);

	SET_FLOAT_VALUE(xmlConvexInt, &convexHullData->m_convexInternalShapeData, m_collisionMargin)
	SET_VECTOR4_VALUE(xmlConvexInt, &convexHullData->m_convexInternalShapeData, m_localScaling)
	SET_VECTOR4_VALUE(xmlConvexInt, &convexHullData->m_convexInternalShapeData, m_implicitShapeDimensions)

	//convexHullData->m_unscaledPointsFloatPtr
	//#define SET_POINTER_VALUE(xmlnode, targetdata, argname, pointertype)

	{
		XMLNode* node = pParent->FirstChildElement("m_unscaledPointsFloatPtr");
		Assert(node);
		if (node)
		{
			tukk txt = (node)->ToElement()->GetText();
			MyLocalCaster cast;
			cast.m_int = (i32)atof(txt);
			(*convexHullData).m_unscaledPointsFloatPtr = (Vec3FloatData*)cast.m_ptr;
		}
	}

	SET_POINTER_VALUE(pParent, *convexHullData, m_unscaledPointsFloatPtr, Vec3FloatData*);
	SET_POINTER_VALUE(pParent, *convexHullData, m_unscaledPointsDoublePtr, Vec3DoubleData*);
	SET_INT_VALUE(pParent, convexHullData, m_numUnscaledPoints);

	m_collisionShapeData.push_back((CollisionShapeData*)convexHullData);
	m_pointerLookup.insert(cast.m_ptr, convexHullData);
}

void BulletXmlWorldImporter::deSerializeCompoundShapeChildData(XMLNode* pParent)
{
	MyLocalCaster cast;
	get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int);

	i32 numChildren = 0;
	AlignedObjectArray<CompoundShapeChildData>* compoundChildArrayPtr = new AlignedObjectArray<CompoundShapeChildData>;
	{
		XMLNode* transNode = pParent->FirstChildElement("m_transform");
		XMLNode* colShapeNode = pParent->FirstChildElement("m_childShape");
		XMLNode* marginNode = pParent->FirstChildElement("m_childMargin");
		XMLNode* childTypeNode = pParent->FirstChildElement("m_childShapeType");

		i32 i = 0;
		while (transNode && colShapeNode && marginNode && childTypeNode)
		{
			compoundChildArrayPtr->expandNonInitializing();
			SET_VECTOR4_VALUE(transNode, &compoundChildArrayPtr->at(i).m_transform, m_origin)
			SET_MATRIX33_VALUE(transNode, &compoundChildArrayPtr->at(i).m_transform, m_basis)

			tukk txt = (colShapeNode)->ToElement()->GetText();
			MyLocalCaster cast;
			cast.m_int = (i32)atof(txt);
			compoundChildArrayPtr->at(i).m_childShape = (CollisionShapeData*)cast.m_ptr;

			Assert(childTypeNode->ToElement());
			if (childTypeNode->ToElement())
			{
				compoundChildArrayPtr->at(i).m_childShapeType = (i32)atof(childTypeNode->ToElement()->GetText());
			}

			Assert(marginNode->ToElement());
			if (marginNode->ToElement())
			{
				compoundChildArrayPtr->at(i).m_childMargin = (float)atof(marginNode->ToElement()->GetText());
			}

			transNode = transNode->NextSiblingElement("m_transform");
			colShapeNode = colShapeNode->NextSiblingElement("m_childShape");
			marginNode = marginNode->NextSiblingElement("m_childMargin");
			childTypeNode = childTypeNode->NextSiblingElement("m_childShapeType");
			i++;
		}

		numChildren = i;
	}

	Assert(numChildren);
	if (numChildren)
	{
		m_compoundShapeChildDataArrays.push_back(compoundChildArrayPtr);
		CompoundShapeChildData* cd = &compoundChildArrayPtr->at(0);
		m_pointerLookup.insert(cast.m_ptr, cd);
	}
}

void BulletXmlWorldImporter::deSerializeCompoundShapeData(XMLNode* pParent)
{
	MyLocalCaster cast;
	get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int);

	CompoundShapeData* compoundData = (CompoundShapeData*)AlignedAlloc(sizeof(CompoundShapeData), 16);

	XMLNode* xmlColShape = pParent->FirstChildElement("m_collisionShapeData");
	Assert(xmlColShape);
	deSerializeCollisionShapeData(xmlColShape, &compoundData->m_collisionShapeData);

	SET_INT_VALUE(pParent, compoundData, m_numChildShapes);

	XMLNode* xmlShapeData = pParent->FirstChildElement("m_collisionShapeData");
	Assert(xmlShapeData);

	{
		XMLNode* node = pParent->FirstChildElement("m_childShapePtr");
		Assert(node);
		while (node)
		{
			tukk txt = (node)->ToElement()->GetText();
			MyLocalCaster cast;
			cast.m_int = (i32)atof(txt);
			compoundData->m_childShapePtr = (CompoundShapeChildData*)cast.m_ptr;
			node = node->NextSiblingElement("m_childShapePtr");
		}
		//SET_POINTER_VALUE(xmlColShape, *compoundData,m_childShapePtr,btCompoundShapeChildData*);
	}
	SET_FLOAT_VALUE(pParent, compoundData, m_collisionMargin);

	m_collisionShapeData.push_back((CollisionShapeData*)compoundData);
	m_pointerLookup.insert(cast.m_ptr, compoundData);
}

void BulletXmlWorldImporter::deSerializeStaticPlaneShapeData(XMLNode* pParent)
{
	MyLocalCaster cast;
	get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int);

	StaticPlaneShapeData* planeData = (StaticPlaneShapeData*)AlignedAlloc(sizeof(StaticPlaneShapeData), 16);

	XMLNode* xmlShapeData = pParent->FirstChildElement("m_collisionShapeData");
	Assert(xmlShapeData);
	deSerializeCollisionShapeData(xmlShapeData, &planeData->m_collisionShapeData);

	SET_VECTOR4_VALUE(pParent, planeData, m_localScaling);
	SET_VECTOR4_VALUE(pParent, planeData, m_planeNormal);
	SET_FLOAT_VALUE(pParent, planeData, m_planeConstant);

	m_collisionShapeData.push_back((CollisionShapeData*)planeData);
	m_pointerLookup.insert(cast.m_ptr, planeData);
}

void BulletXmlWorldImporter::deSerializeDynamicsWorldData(XMLNode* pParent)
{
	ContactSolverInfo solverInfo;
	//Vec3 gravity(0,0,0);

	//setDynamicsWorldInfo(gravity,solverInfo);

	//gravity and world info
}

void BulletXmlWorldImporter::deSerializeConvexInternalShapeData(XMLNode* pParent)
{
	MyLocalCaster cast;
	get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int);

	ConvexInternalShapeData* convexShape = (ConvexInternalShapeData*)AlignedAlloc(sizeof(ConvexInternalShapeData), 16);
	memset(convexShape, 0, sizeof(ConvexInternalShapeData));

	XMLNode* xmlShapeData = pParent->FirstChildElement("m_collisionShapeData");
	Assert(xmlShapeData);

	deSerializeCollisionShapeData(xmlShapeData, &convexShape->m_collisionShapeData);

	SET_FLOAT_VALUE(pParent, convexShape, m_collisionMargin)
	SET_VECTOR4_VALUE(pParent, convexShape, m_localScaling)
	SET_VECTOR4_VALUE(pParent, convexShape, m_implicitShapeDimensions)

	m_collisionShapeData.push_back((CollisionShapeData*)convexShape);
	m_pointerLookup.insert(cast.m_ptr, convexShape);
}

/*
enum btTypedConstraintType
{
	POINT2POINT_CONSTRAINT_TYPE=3,
	HINGE_CONSTRAINT_TYPE,
	CONETWIST_CONSTRAINT_TYPE,
//	D6_CONSTRAINT_TYPE,
	SLIDER_CONSTRAINT_TYPE,
	CONTACT_CONSTRAINT_TYPE,
	D6_SPRING_CONSTRAINT_TYPE,
	GEAR_CONSTRAINT_TYPE,
	MAX_CONSTRAINT_TYPE
};
*/

void BulletXmlWorldImporter::deSerializeGeneric6DofConstraintData(XMLNode* pParent)
{
	MyLocalCaster cast;
	get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int);

	Generic6DofConstraintData2* dof6Data = (Generic6DofConstraintData2*)AlignedAlloc(sizeof(Generic6DofConstraintData2), 16);

	XMLNode* n = pParent->FirstChildElement("m_typeConstraintData");
	if (n)
	{
		SET_POINTER_VALUE(n, dof6Data->m_typeConstraintData, m_rbA, RigidBodyData*);
		SET_POINTER_VALUE(n, dof6Data->m_typeConstraintData, m_rbB, RigidBodyData*);
		dof6Data->m_typeConstraintData.m_name = 0;  //tbd
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_objectType);
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_userConstraintType);
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_userConstraintId);
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_needsFeedback);
		SET_FLOAT_VALUE(n, &dof6Data->m_typeConstraintData, m_appliedImpulse);
		SET_FLOAT_VALUE(n, &dof6Data->m_typeConstraintData, m_dbgDrawSize);
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_disableCollisionsBetweenLinkedBodies);
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_overrideNumSolverIterations);
		SET_FLOAT_VALUE(n, &dof6Data->m_typeConstraintData, m_breakingImpulseThreshold);
		SET_INT_VALUE(n, &dof6Data->m_typeConstraintData, m_isEnabled);
	}

	SET_TRANSFORM_VALUE(pParent, dof6Data, m_rbAFrame);
	SET_TRANSFORM_VALUE(pParent, dof6Data, m_rbBFrame);
	SET_VECTOR4_VALUE(pParent, dof6Data, m_linearUpperLimit);
	SET_VECTOR4_VALUE(pParent, dof6Data, m_linearLowerLimit);
	SET_VECTOR4_VALUE(pParent, dof6Data, m_angularUpperLimit);
	SET_VECTOR4_VALUE(pParent, dof6Data, m_angularLowerLimit);
	SET_INT_VALUE(pParent, dof6Data, m_useLinearReferenceFrameA);
	SET_INT_VALUE(pParent, dof6Data, m_useOffsetForConstraintFrame);

	m_constraintData.push_back((TypedConstraintData2*)dof6Data);
	m_pointerLookup.insert(cast.m_ptr, dof6Data);
}

void BulletXmlWorldImporter::deSerializeRigidBodyFloatData(XMLNode* pParent)
{
	MyLocalCaster cast;

	if (!get_int_attribute_by_name(pParent->ToElement(), "pointer", &cast.m_int))
	{
		m_fileOk = false;
		return;
	}

	RigidBodyData* rbData = (RigidBodyData*)AlignedAlloc(sizeof(RigidBodyData), 16);

	XMLNode* n = pParent->FirstChildElement("m_collisionObjectData");

	if (n)
	{
		SET_POINTER_VALUE(n, rbData->m_collisionObjectData, m_collisionShape, uk );
		SET_TRANSFORM_VALUE(n, &rbData->m_collisionObjectData, m_worldTransform);
		SET_TRANSFORM_VALUE(n, &rbData->m_collisionObjectData, m_interpolationWorldTransform);
		SET_VECTOR4_VALUE(n, &rbData->m_collisionObjectData, m_interpolationLinearVelocity)
		SET_VECTOR4_VALUE(n, &rbData->m_collisionObjectData, m_interpolationAngularVelocity)
		SET_VECTOR4_VALUE(n, &rbData->m_collisionObjectData, m_anisotropicFriction)
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_contactProcessingThreshold);
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_deactivationTime);
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_friction);
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_restitution);
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_hitFraction);
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_ccdSweptSphereRadius);
		SET_FLOAT_VALUE(n, &rbData->m_collisionObjectData, m_ccdMotionThreshold);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_hasAnisotropicFriction);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_collisionFlags);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_islandTag1);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_companionId);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_activationState1);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_internalType);
		SET_INT_VALUE(n, &rbData->m_collisionObjectData, m_checkCollideWith);
	}

	//	SET_VECTOR4_VALUE(pParent,rbData,m_linearVelocity);

	SET_MATRIX33_VALUE(pParent, rbData, m_invInertiaTensorWorld);

	SET_VECTOR4_VALUE(pParent, rbData, m_linearVelocity)
	SET_VECTOR4_VALUE(pParent, rbData, m_angularVelocity)
	SET_VECTOR4_VALUE(pParent, rbData, m_angularFactor)
	SET_VECTOR4_VALUE(pParent, rbData, m_linearFactor)
	SET_VECTOR4_VALUE(pParent, rbData, m_gravity)
	SET_VECTOR4_VALUE(pParent, rbData, m_gravity_acceleration)
	SET_VECTOR4_VALUE(pParent, rbData, m_invInertiaLocal)
	SET_VECTOR4_VALUE(pParent, rbData, m_totalTorque)
	SET_VECTOR4_VALUE(pParent, rbData, m_totalForce)
	SET_FLOAT_VALUE(pParent, rbData, m_inverseMass);
	SET_FLOAT_VALUE(pParent, rbData, m_linearDamping);
	SET_FLOAT_VALUE(pParent, rbData, m_angularDamping);
	SET_FLOAT_VALUE(pParent, rbData, m_additionalDampingFactor);
	SET_FLOAT_VALUE(pParent, rbData, m_additionalLinearDampingThresholdSqr);
	SET_FLOAT_VALUE(pParent, rbData, m_additionalAngularDampingThresholdSqr);
	SET_FLOAT_VALUE(pParent, rbData, m_additionalAngularDampingFactor);
	SET_FLOAT_VALUE(pParent, rbData, m_angularSleepingThreshold);
	SET_FLOAT_VALUE(pParent, rbData, m_linearSleepingThreshold);
	SET_INT_VALUE(pParent, rbData, m_additionalDamping);

	m_rigidBodyData.push_back(rbData);
	m_pointerLookup.insert(cast.m_ptr, rbData);

	//	rbData->m_collisionObjectData.m_collisionShape = (uk ) (i32)atof(txt);
}

/*
	TETRAHEDRAL_SHAPE_PROXYTYPE,
	CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE,
	,
	CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE,
	CUSTOM_POLYHEDRAL_SHAPE_TYPE,
//implicit convex shapes
IMPLICIT_CONVEX_SHAPES_START_HERE,
	SPHERE_SHAPE_PROXYTYPE,
	MULTI_SPHERE_SHAPE_PROXYTYPE,
	CAPSULE_SHAPE_PROXYTYPE,
	CONE_SHAPE_PROXYTYPE,
	CONVEX_SHAPE_PROXYTYPE,
	CYLINDER_SHAPE_PROXYTYPE,
	UNIFORM_SCALING_SHAPE_PROXYTYPE,
	MINKOWSKI_SUM_SHAPE_PROXYTYPE,
	MINKOWSKI_DIFFERENCE_SHAPE_PROXYTYPE,
	BOX_2D_SHAPE_PROXYTYPE,
	CONVEX_2D_SHAPE_PROXYTYPE,
	CUSTOM_CONVEX_SHAPE_TYPE,
//concave shapes
CONCAVE_SHAPES_START_HERE,
	//keep all the convex shapetype below here, for the check IsConvexShape in broadphase proxy!
	TRIANGLE_MESH_SHAPE_PROXYTYPE,
	SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE,
	///used for demo integration FAST/Swift collision library and drx3D
	FAST_CONCAVE_MESH_PROXYTYPE,
	//terrain
	TERRAIN_SHAPE_PROXYTYPE,
///Used for GIMPACT Trimesh integration
	GIMPACT_SHAPE_PROXYTYPE,
///Multimaterial mesh
    MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE,

	,
	,
	CUSTOM_CONCAVE_SHAPE_TYPE,
CONCAVE_SHAPES_END_HERE,

	,

	SOFTBODY_SHAPE_PROXYTYPE,
	HFFLUID_SHAPE_PROXYTYPE,
	HFFLUID_BUOYANT_CONVEX_SHAPE_PROXYTYPE,
	INVALID_SHAPE_PROXYTYPE,

	MAX_BROADPHASE_COLLISION_TYPES
*/

void BulletXmlWorldImporter::fixupConstraintData(TypedConstraintData2* tcd)
{
	if (tcd->m_rbA)
	{
		RigidBodyData** ptrptr = (RigidBodyData**)m_pointerLookup.find(tcd->m_rbA);
		Assert(ptrptr);
		tcd->m_rbA = ptrptr ? *ptrptr : 0;
	}
	if (tcd->m_rbB)
	{
		RigidBodyData** ptrptr = (RigidBodyData**)m_pointerLookup.find(tcd->m_rbB);
		Assert(ptrptr);
		tcd->m_rbB = ptrptr ? *ptrptr : 0;
	}
}

void BulletXmlWorldImporter::fixupCollisionDataPointers(CollisionShapeData* shapeData)
{
	switch (shapeData->m_shapeType)
	{
		case COMPOUND_SHAPE_PROXYTYPE:
		{
			CompoundShapeData* compound = (CompoundShapeData*)shapeData;

			uk * cdptr = m_pointerLookup.find((uk )compound->m_childShapePtr);
			CompoundShapeChildData** c = (CompoundShapeChildData**)cdptr;
			Assert(c);
			if (c)
			{
				compound->m_childShapePtr = *c;
			}
			else
			{
				compound->m_childShapePtr = 0;
			}
			break;
		}

		case CONVEX_HULL_SHAPE_PROXYTYPE:
		{
			ConvexHullShapeData* convexData = (ConvexHullShapeData*)shapeData;
			Vec3FloatData** ptrptr = (Vec3FloatData**)m_pointerLookup.find((uk )convexData->m_unscaledPointsFloatPtr);
			Assert(ptrptr);
			if (ptrptr)
			{
				convexData->m_unscaledPointsFloatPtr = *ptrptr;
			}
			else
			{
				convexData->m_unscaledPointsFloatPtr = 0;
			}
			break;
		}

		case BOX_SHAPE_PROXYTYPE:
		case TRIANGLE_SHAPE_PROXYTYPE:
		case STATIC_PLANE_PROXYTYPE:
		case EMPTY_SHAPE_PROXYTYPE:
			break;

		default:
		{
			Assert(0);
		}
	}
}

void BulletXmlWorldImporter::auto_serialize_root_level_children(XMLNode* pParent)
{
	i32 numChildren = 0;
	Assert(pParent);
	if (pParent)
	{
		XMLNode* pChild;
		for (pChild = pParent->FirstChildElement(); pChild != 0; pChild = pChild->NextSibling(), numChildren++)
		{
			//			printf("child Name=%s\n", pChild->Val());
			if (!strcmp(pChild->Val(), "Vec3FloatData"))
			{
				MyLocalCaster cast;
				get_int_attribute_by_name(pChild->ToElement(), "pointer", &cast.m_int);

				AlignedObjectArray<Vec3FloatData> v;
				deSerializeVector3FloatData(pChild, v);
				i32 numVectors = v.size();
				Vec3FloatData* vectors = (Vec3FloatData*)AlignedAlloc(sizeof(Vec3FloatData) * numVectors, 16);
				for (i32 i = 0; i < numVectors; i++)
					vectors[i] = v[i];
				m_floatVertexArrays.push_back(vectors);
				m_pointerLookup.insert(cast.m_ptr, vectors);
				continue;
			}

			if (!strcmp(pChild->Val(), "Generic6DofConstraintData"))
			{
				deSerializeGeneric6DofConstraintData(pChild);
				continue;
			}

			if (!strcmp(pChild->Val(), "StaticPlaneShapeData"))
			{
				deSerializeStaticPlaneShapeData(pChild);
				continue;
			}

			if (!strcmp(pChild->Val(), "CompoundShapeData"))
			{
				deSerializeCompoundShapeData(pChild);
				continue;
			}

			if (!strcmp(pChild->Val(), "CompoundShapeChildData"))
			{
				deSerializeCompoundShapeChildData(pChild);
				continue;
			}

			if (!strcmp(pChild->Val(), "ConvexHullShapeData"))
			{
				deSerializeConvexHullShapeData(pChild);
				continue;
			}

			if (!strcmp(pChild->Val(), "DynamicsWorldFloatData"))
			{
				deSerializeDynamicsWorldData(pChild);
				continue;
			}

			if (!strcmp(pChild->Val(), "ConvexInternalShapeData"))
			{
				deSerializeConvexInternalShapeData(pChild);
				continue;
			}
			if (!strcmp(pChild->Val(), "RigidBodyFloatData"))
			{
				deSerializeRigidBodyFloatData(pChild);
				continue;
			}

			//printf("Ошибка: btBulletXmlWorldImporter doesn't support %s yet\n", pChild->Val());
			//	Assert(0);
		}
	}

	///=================================================================
	///fixup pointers in various places, in the right order

	//fixup compoundshape child data
	for (i32 i = 0; i < m_compoundShapeChildDataArrays.size(); i++)
	{
		AlignedObjectArray<CompoundShapeChildData>* childDataArray = m_compoundShapeChildDataArrays[i];
		for (i32 c = 0; c < childDataArray->size(); c++)
		{
			CompoundShapeChildData* childData = &childDataArray->at(c);
			CollisionShapeData** ptrptr = (CollisionShapeData**)m_pointerLookup[childData->m_childShape];
			Assert(ptrptr);
			if (ptrptr)
			{
				childData->m_childShape = *ptrptr;
			}
		}
	}

	for (i32 i = 0; i < this->m_collisionShapeData.size(); i++)
	{
		CollisionShapeData* shapeData = m_collisionShapeData[i];
		fixupCollisionDataPointers(shapeData);
	}

	///now fixup pointers
	for (i32 i = 0; i < m_rigidBodyData.size(); i++)
	{
		RigidBodyData* rbData = m_rigidBodyData[i];

		uk * ptrptr = m_pointerLookup.find(rbData->m_collisionObjectData.m_collisionShape);
		//Assert(ptrptr);
		rbData->m_collisionObjectData.m_broadphaseHandle = 0;
		rbData->m_collisionObjectData.m_rootCollisionShape = 0;
		rbData->m_collisionObjectData.m_name = 0;  //tbd
		if (ptrptr)
		{
			rbData->m_collisionObjectData.m_collisionShape = *ptrptr;
		}
	}

	for (i32 i = 0; i < m_constraintData.size(); i++)
	{
		TypedConstraintData2* tcd = m_constraintData[i];
		fixupConstraintData(tcd);
	}
	///=================================================================
	///convert data into drx3D data in the right order

	///convert collision shapes
	for (i32 i = 0; i < this->m_collisionShapeData.size(); i++)
	{
		CollisionShapeData* shapeData = m_collisionShapeData[i];
		CollisionShape* shape = convertCollisionShape(shapeData);
		if (shape)
		{
			m_shapeMap.insert(shapeData, shape);
		}
		if (shape && shapeData->m_name)
		{
			tuk newname = duplicateName(shapeData->m_name);
			m_objectNameMap.insert(shape, newname);
			m_nameShapeMap.insert(newname, shape);
		}
	}

	for (i32 i = 0; i < m_rigidBodyData.size(); i++)
	{
#ifdef DRX3D_USE_DOUBLE_PRECISION
		convertRigidBodyDouble(m_rigidBodyData[i]);
#else
		convertRigidBodyFloat(m_rigidBodyData[i]);
#endif
	}

	for (i32 i = 0; i < m_constraintData.size(); i++)
	{
		TypedConstraintData2* tcd = m_constraintData[i];
		//		bool isDoublePrecision = false;
		RigidBody* rbA = 0;
		RigidBody* rbB = 0;
		{
			CollisionObject2** ptrptr = m_bodyMap.find(tcd->m_rbA);
			if (ptrptr)
			{
				rbA = RigidBody::upcast(*ptrptr);
			}
		}
		{
			CollisionObject2** ptrptr = m_bodyMap.find(tcd->m_rbB);
			if (ptrptr)
			{
				rbB = RigidBody::upcast(*ptrptr);
			}
		}
		if (rbA || rbB)
		{
			Assert(0);  //todo
						  //convertConstraint(tcd,rbA,rbB,isDoublePrecision, m_fileVersion);
		}
	}
}

void BulletXmlWorldImporter::auto_serialize(XMLNode* pParent)
{
	//	XMLElement* root = pParent->FirstChildElement("bullet_physics");
	if (pParent)
	{
		XMLNode* pChild;
		for (pChild = pParent->FirstChildElement(); pChild != 0; pChild = pChild->NextSibling())
		{
			//if (pChild->Type()==XMLNode::TINYXML_ELEMENT)
			{
				//				printf("root Name=%s\n", pChild->Val());
				auto_serialize_root_level_children(pChild);
			}
		}
	}
	else
	{
		printf("ERROR: no bullet_physics element\n");
	}
}

bool BulletXmlWorldImporter::loadFile(tukk fileName)
{
	XMLDocument doc;

	XMLError loadOkay = doc.LoadFile(fileName);

	if (loadOkay == XML_SUCCESS)
	{
		if (get_int_attribute_by_name(doc.FirstChildElement()->ToElement(), "version", &m_fileVersion))
		{
			if (m_fileVersion == 281)
			{
				m_fileOk = true;
				i32 itemcount;
				get_int_attribute_by_name(doc.FirstChildElement()->ToElement(), "itemcount", &itemcount);

				auto_serialize(&doc);
				return m_fileOk;
			}
		}
	}
	return false;
}
