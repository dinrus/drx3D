#include "../WorldImporter.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#ifdef USE_GIMPACT
#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>
#endif
WorldImporter::WorldImporter(DynamicsWorld* world)
	: m_dynamicsWorld(world),
	  m_verboseMode(0),
	  m_importerFlags(0)
{
}

WorldImporter::~WorldImporter()
{
}

void WorldImporter::deleteAllData()
{
	i32 i;
	for (i = 0; i < m_allocatedConstraints.size(); i++)
	{
		if (m_dynamicsWorld)
			m_dynamicsWorld->removeConstraint(m_allocatedConstraints[i]);
		delete m_allocatedConstraints[i];
	}
	m_allocatedConstraints.clear();

	for (i = 0; i < m_allocatedRigidBodies.size(); i++)
	{
		if (m_dynamicsWorld)
			m_dynamicsWorld->removeRigidBody(RigidBody::upcast(m_allocatedRigidBodies[i]));
		delete m_allocatedRigidBodies[i];
	}

	m_allocatedRigidBodies.clear();

	for (i = 0; i < m_allocatedCollisionShapes.size(); i++)
	{
		delete m_allocatedCollisionShapes[i];
	}
	m_allocatedCollisionShapes.clear();

	for (i = 0; i < m_allocatedBvhs.size(); i++)
	{
		delete m_allocatedBvhs[i];
	}
	m_allocatedBvhs.clear();

	for (i = 0; i < m_allocatedTriangleInfoMaps.size(); i++)
	{
		delete m_allocatedTriangleInfoMaps[i];
	}
	m_allocatedTriangleInfoMaps.clear();
	for (i = 0; i < m_allocatedTriangleIndexArrays.size(); i++)
	{
		delete m_allocatedTriangleIndexArrays[i];
	}
	m_allocatedTriangleIndexArrays.clear();
	for (i = 0; i < m_allocatedNames.size(); i++)
	{
		delete[] m_allocatedNames[i];
	}
	m_allocatedNames.clear();

	for (i = 0; i < m_allocatedbtStridingMeshInterfaceDatas.size(); i++)
	{
		StridingMeshInterfaceData* curData = m_allocatedbtStridingMeshInterfaceDatas[i];

		for (i32 a = 0; a < curData->m_numMeshParts; a++)
		{
			MeshPartData* curPart = &curData->m_meshPartsPtr[a];
			if (curPart->m_vertices3f)
				delete[] curPart->m_vertices3f;

			if (curPart->m_vertices3d)
				delete[] curPart->m_vertices3d;

			if (curPart->m_indices32)
				delete[] curPart->m_indices32;

			if (curPart->m_3indices16)
				delete[] curPart->m_3indices16;

			if (curPart->m_indices16)
				delete[] curPart->m_indices16;

			if (curPart->m_3indices8)
				delete[] curPart->m_3indices8;
		}
		delete[] curData->m_meshPartsPtr;
		delete curData;
	}
	m_allocatedbtStridingMeshInterfaceDatas.clear();

	for (i = 0; i < m_indexArrays.size(); i++)
	{
		AlignedFree(m_indexArrays[i]);
	}
	m_indexArrays.clear();

	for (i = 0; i < m_shortIndexArrays.size(); i++)
	{
		AlignedFree(m_shortIndexArrays[i]);
	}
	m_shortIndexArrays.clear();

	for (i = 0; i < m_charIndexArrays.size(); i++)
	{
		AlignedFree(m_charIndexArrays[i]);
	}
	m_charIndexArrays.clear();

	for (i = 0; i < m_floatVertexArrays.size(); i++)
	{
		AlignedFree(m_floatVertexArrays[i]);
	}
	m_floatVertexArrays.clear();

	for (i = 0; i < m_doubleVertexArrays.size(); i++)
	{
		AlignedFree(m_doubleVertexArrays[i]);
	}
	m_doubleVertexArrays.clear();
}

CollisionShape* WorldImporter::convertCollisionShape(CollisionShapeData* shapeData)
{
	CollisionShape* shape = 0;

	switch (shapeData->m_shapeType)
	{
		case STATIC_PLANE_PROXYTYPE:
		{
			StaticPlaneShapeData* planeData = (StaticPlaneShapeData*)shapeData;
			Vec3 planeNormal, localScaling;
			planeNormal.deSerializeFloat(planeData->m_planeNormal);
			localScaling.deSerializeFloat(planeData->m_localScaling);
			shape = createPlaneShape(planeNormal, planeData->m_planeConstant);
			shape->setLocalScaling(localScaling);

			break;
		}
		case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE:
		{
			ScaledTriangleMeshShapeData* scaledMesh = (ScaledTriangleMeshShapeData*)shapeData;
			CollisionShapeData* colShapeData = (CollisionShapeData*)&scaledMesh->m_trimeshShapeData;
			colShapeData->m_shapeType = TRIANGLE_MESH_SHAPE_PROXYTYPE;
			CollisionShape* childShape = convertCollisionShape(colShapeData);
			BvhTriangleMeshShape* meshShape = (BvhTriangleMeshShape*)childShape;
			Vec3 localScaling;
			localScaling.deSerializeFloat(scaledMesh->m_localScaling);

			shape = createScaledTrangleMeshShape(meshShape, localScaling);
			break;
		}
		case GIMPACT_SHAPE_PROXYTYPE:
		{
#ifdef USE_GIMPACT
			btGImpactMeshShapeData* gimpactData = (GImpactMeshShapeData*)shapeData;
			if (gimpactData->m_gimpactSubType == CONST_GIMPACT_TRIMESH_SHAPE)
			{
				btStridingMeshInterfaceData* interfaceData = createStridingMeshInterfaceData(&gimpactData->m_meshInterface);
				btTriangleIndexVertexArray* meshInterface = createMeshInterface(*interfaceData);

				btGImpactMeshShape* gimpactShape = createGimpactShape(meshInterface);
				Vec3 localScaling;
				localScaling.deSerializeFloat(gimpactData->m_localScaling);
				gimpactShape->setLocalScaling(localScaling);
				gimpactShape->setMargin(Scalar(gimpactData->m_collisionMargin));
				gimpactShape->updateBound();
				shape = gimpactShape;
			}
			else
			{
				printf("unsupported gimpact sub type\n");
			}
#endif  //USE_GIMPACT
			break;
		}
			//The btCapsuleShape* API has issue passing the margin/scaling/halfextents unmodified through the API
			//so deal with this
		case CAPSULE_SHAPE_PROXYTYPE:
		{
			CapsuleShapeData* capData = (CapsuleShapeData*)shapeData;

			switch (capData->m_upAxis)
			{
				case 0:
				{
					shape = createCapsuleShapeX(1, 1);
					break;
				}
				case 1:
				{
					shape = createCapsuleShapeY(1, 1);
					break;
				}
				case 2:
				{
					shape = createCapsuleShapeZ(1, 1);
					break;
				}
				default:
				{
					printf("ошибка: wrong up axis for btCapsuleShape\n");
				}
			};
			if (shape)
			{
				CapsuleShape* cap = (CapsuleShape*)shape;
				cap->deSerializeFloat(capData);
			}
			break;
		}
		case CYLINDER_SHAPE_PROXYTYPE:
		case CONE_SHAPE_PROXYTYPE:
		case BOX_SHAPE_PROXYTYPE:
		case SPHERE_SHAPE_PROXYTYPE:
		case MULTI_SPHERE_SHAPE_PROXYTYPE:
		case CONVEX_HULL_SHAPE_PROXYTYPE:
		{
			ConvexInternalShapeData* bsd = (ConvexInternalShapeData*)shapeData;
			Vec3 implicitShapeDimensions;
			implicitShapeDimensions.deSerializeFloat(bsd->m_implicitShapeDimensions);
			Vec3 localScaling;
			localScaling.deSerializeFloat(bsd->m_localScaling);
			Vec3 margin(bsd->m_collisionMargin, bsd->m_collisionMargin, bsd->m_collisionMargin);
			switch (shapeData->m_shapeType)
			{
				case BOX_SHAPE_PROXYTYPE:
				{
					BoxShape* box = (BoxShape*)createBoxShape(implicitShapeDimensions / localScaling + margin);
					//box->initializePolyhedralFeatures();
					shape = box;

					break;
				}
				case SPHERE_SHAPE_PROXYTYPE:
				{
					shape = createSphereShape(implicitShapeDimensions.getX());
					break;
				}

				case CYLINDER_SHAPE_PROXYTYPE:
				{
					CylinderShapeData* cylData = (CylinderShapeData*)shapeData;
					Vec3 halfExtents = implicitShapeDimensions + margin;
					switch (cylData->m_upAxis)
					{
						case 0:
						{
							shape = createCylinderShapeX(halfExtents.getY(), halfExtents.getX());
							break;
						}
						case 1:
						{
							shape = createCylinderShapeY(halfExtents.getX(), halfExtents.getY());
							break;
						}
						case 2:
						{
							shape = createCylinderShapeZ(halfExtents.getX(), halfExtents.getZ());
							break;
						}
						default:
						{
							printf("unknown Cylinder up axis\n");
						}
					};

					break;
				}
				case CONE_SHAPE_PROXYTYPE:
				{
					ConeShapeData* conData = (ConeShapeData*)shapeData;
					Vec3 halfExtents = implicitShapeDimensions;  //+margin;
					switch (conData->m_upIndex)
					{
						case 0:
						{
							shape = createConeShapeX(halfExtents.getY(), halfExtents.getX());
							break;
						}
						case 1:
						{
							shape = createConeShapeY(halfExtents.getX(), halfExtents.getY());
							break;
						}
						case 2:
						{
							shape = createConeShapeZ(halfExtents.getX(), halfExtents.getZ());
							break;
						}
						default:
						{
							printf("unknown Cone up axis\n");
						}
					};

					break;
				}
				case MULTI_SPHERE_SHAPE_PROXYTYPE:
				{
					MultiSphereShapeData* mss = (MultiSphereShapeData*)bsd;
					i32 numSpheres = mss->m_localPositionArraySize;

					AlignedObjectArray<Vec3> tmpPos;
					AlignedObjectArray<Scalar> radii;
					radii.resize(numSpheres);
					tmpPos.resize(numSpheres);
					i32 i;
					for (i = 0; i < numSpheres; i++)
					{
						tmpPos[i].deSerializeFloat(mss->m_localPositionArrayPtr[i].m_pos);
						radii[i] = mss->m_localPositionArrayPtr[i].m_radius;
					}
					shape = createMultiSphereShape(&tmpPos[0], &radii[0], numSpheres);
					break;
				}
				case CONVEX_HULL_SHAPE_PROXYTYPE:
				{
					//	i32 sz = sizeof(ConvexHullShapeData);
					//	i32 sz2 = sizeof(ConvexInternalShapeData);
					//	i32 sz3 = sizeof(CollisionShapeData);
					ConvexHullShapeData* convexData = (ConvexHullShapeData*)bsd;
					i32 numPoints = convexData->m_numUnscaledPoints;

					AlignedObjectArray<Vec3> tmpPoints;
					tmpPoints.resize(numPoints);
					i32 i;
					for (i = 0; i < numPoints; i++)
					{
#ifdef DRX3D_USE_DOUBLE_PRECISION
						if (convexData->m_unscaledPointsDoublePtr)
							tmpPoints[i].deSerialize(convexData->m_unscaledPointsDoublePtr[i]);
						if (convexData->m_unscaledPointsFloatPtr)
							tmpPoints[i].deSerializeFloat(convexData->m_unscaledPointsFloatPtr[i]);
#else
						if (convexData->m_unscaledPointsFloatPtr)
							tmpPoints[i].deSerialize(convexData->m_unscaledPointsFloatPtr[i]);
						if (convexData->m_unscaledPointsDoublePtr)
							tmpPoints[i].deSerializeDouble(convexData->m_unscaledPointsDoublePtr[i]);
#endif  //DRX3D_USE_DOUBLE_PRECISION
					}
					ConvexHullShape* hullShape = createConvexHullShape();
					for (i = 0; i < numPoints; i++)
					{
						hullShape->addPoint(tmpPoints[i]);
					}
					hullShape->setMargin(bsd->m_collisionMargin);
					//hullShape->initializePolyhedralFeatures();
					shape = hullShape;
					break;
				}
				default:
				{
					printf("ошибка: cannot create shape type (%d)\n", shapeData->m_shapeType);
				}
			}

			if (shape)
			{
				shape->setMargin(bsd->m_collisionMargin);

				Vec3 localScaling;
				localScaling.deSerializeFloat(bsd->m_localScaling);
				shape->setLocalScaling(localScaling);
			}
			break;
		}
		case TRIANGLE_MESH_SHAPE_PROXYTYPE:
		{
			TriangleMeshShapeData* trimesh = (TriangleMeshShapeData*)shapeData;
			StridingMeshInterfaceData* interfaceData = createStridingMeshInterfaceData(&trimesh->m_meshInterface);
			TriangleIndexVertexArray* meshInterface = createMeshInterface(*interfaceData);
			if (!meshInterface->getNumSubParts())
			{
				return 0;
			}

			Vec3 scaling;
			scaling.deSerializeFloat(trimesh->m_meshInterface.m_scaling);
			meshInterface->setScaling(scaling);

			OptimizedBvh* bvh = 0;
#if 1
			if (trimesh->m_quantizedFloatBvh)
			{
				OptimizedBvh** bvhPtr = m_bvhMap.find(trimesh->m_quantizedFloatBvh);
				if (bvhPtr && *bvhPtr)
				{
					bvh = *bvhPtr;
				}
				else
				{
					bvh = createOptimizedBvh();
					bvh->deSerializeFloat(*trimesh->m_quantizedFloatBvh);
				}
			}
			if (trimesh->m_quantizedDoubleBvh)
			{
				OptimizedBvh** bvhPtr = m_bvhMap.find(trimesh->m_quantizedDoubleBvh);
				if (bvhPtr && *bvhPtr)
				{
					bvh = *bvhPtr;
				}
				else
				{
					bvh = createOptimizedBvh();
					bvh->deSerializeDouble(*trimesh->m_quantizedDoubleBvh);
				}
			}
#endif

			BvhTriangleMeshShape* trimeshShape = createBvhTriangleMeshShape(meshInterface, bvh);
			trimeshShape->setMargin(trimesh->m_collisionMargin);
			shape = trimeshShape;

			if (trimesh->m_triangleInfoMap)
			{
				TriangleInfoMap* map = createTriangleInfoMap();
				map->deSerialize(*trimesh->m_triangleInfoMap);
				trimeshShape->setTriangleInfoMap(map);

#ifdef USE_INTERNAL_EDGE_UTILITY
				gContactAddedCallback = btAdjustInternalEdgeContactsCallback;
#endif  //USE_INTERNAL_EDGE_UTILITY
			}

			//printf("trimesh->m_collisionMargin=%f\n",trimesh->m_collisionMargin);
			break;
		}
		case COMPOUND_SHAPE_PROXYTYPE:
		{
			CompoundShapeData* compoundData = (CompoundShapeData*)shapeData;
			CompoundShape* compoundShape = createCompoundShape();

			AlignedObjectArray<CollisionShape*> childShapes;
			for (i32 i = 0; i < compoundData->m_numChildShapes; i++)
			{
				CollisionShapeData* cd = compoundData->m_childShapePtr[i].m_childShape;

				CollisionShape* childShape = convertCollisionShape(cd);
				if (childShape)
				{
					Transform2 localTransform;
					localTransform.deSerializeFloat(compoundData->m_childShapePtr[i].m_transform);
					compoundShape->addChildShape(localTransform, childShape);
				}
				else
				{
#ifdef _DEBUG
					printf("ошибка: couldn't create childShape for compoundShape\n");
#endif
				}
			}
			shape = compoundShape;

			break;
		}
		case SOFTBODY_SHAPE_PROXYTYPE:
		{
			return 0;
		}
		default:
		{
#ifdef _DEBUG
			printf("unsupported shape type (%d)\n", shapeData->m_shapeType);
#endif
		}
	}

	return shape;
}

tuk WorldImporter::duplicateName(tukk name)
{
	if (name)
	{
		i32 l = (i32)strlen(name);
		tuk newName = new char[l + 1];
		memcpy(newName, name, l);
		newName[l] = 0;
		m_allocatedNames.push_back(newName);
		return newName;
	}
	return 0;
}

void WorldImporter::convertConstraintBackwardsCompatible281(TypedConstraintData* constraintData, RigidBody* rbA, RigidBody* rbB, i32 fileVersion)
{
	TypedConstraint* constraint = 0;

	switch (constraintData->m_objectType)
	{
		case POINT2POINT_CONSTRAINT_TYPE:
		{
			Point2PointConstraintDoubleData* p2pData = (Point2PointConstraintDoubleData*)constraintData;
			if (rbA && rbB)
			{
				Vec3 pivotInA, pivotInB;
				pivotInA.deSerializeDouble(p2pData->m_pivotInA);
				pivotInB.deSerializeDouble(p2pData->m_pivotInB);
				constraint = createPoint2PointConstraint(*rbA, *rbB, pivotInA, pivotInB);
			}
			else
			{
				Vec3 pivotInA;
				pivotInA.deSerializeDouble(p2pData->m_pivotInA);
				constraint = createPoint2PointConstraint(*rbA, pivotInA);
			}
			break;
		}
		case HINGE_CONSTRAINT_TYPE:
		{
			HingeConstraint* hinge = 0;

			HingeConstraintDoubleData* hingeData = (HingeConstraintDoubleData*)constraintData;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(hingeData->m_rbAFrame);
				rbBFrame.deSerializeDouble(hingeData->m_rbBFrame);
				hinge = createHingeConstraint(*rbA, *rbB, rbAFrame, rbBFrame, hingeData->m_useReferenceFrameA != 0);
			}
			else
			{
				Transform2 rbAFrame;
				rbAFrame.deSerializeDouble(hingeData->m_rbAFrame);
				hinge = createHingeConstraint(*rbA, rbAFrame, hingeData->m_useReferenceFrameA != 0);
			}
			if (hingeData->m_enableAngularMotor)
			{
				hinge->enableAngularMotor(true, (Scalar)hingeData->m_motorTargetVelocity, (Scalar)hingeData->m_maxMotorImpulse);
			}
			hinge->setAngularOnly(hingeData->m_angularOnly != 0);
			hinge->setLimit(Scalar(hingeData->m_lowerLimit), Scalar(hingeData->m_upperLimit), Scalar(hingeData->m_limitSoftness), Scalar(hingeData->m_biasFactor), Scalar(hingeData->m_relaxationFactor));

			constraint = hinge;
			break;
		}
		case CONETWIST_CONSTRAINT_TYPE:
		{
			ConeTwistConstraintData* coneData = (ConeTwistConstraintData*)constraintData;
			ConeTwistConstraint* coneTwist = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(coneData->m_rbAFrame);
				rbBFrame.deSerializeFloat(coneData->m_rbBFrame);
				coneTwist = createConeTwistConstraint(*rbA, *rbB, rbAFrame, rbBFrame);
			}
			else
			{
				Transform2 rbAFrame;
				rbAFrame.deSerializeFloat(coneData->m_rbAFrame);
				coneTwist = createConeTwistConstraint(*rbA, rbAFrame);
			}
			coneTwist->setLimit((Scalar)coneData->m_swingSpan1, (Scalar)coneData->m_swingSpan2, (Scalar)coneData->m_twistSpan, (Scalar)coneData->m_limitSoftness,
								(Scalar)coneData->m_biasFactor, (Scalar)coneData->m_relaxationFactor);
			coneTwist->setDamping((Scalar)coneData->m_damping);

			constraint = coneTwist;
			break;
		}

		case D6_SPRING_CONSTRAINT_TYPE:
		{
			Generic6DofSpringConstraintData* dofData = (Generic6DofSpringConstraintData*)constraintData;
			//	i32 sz = sizeof(Generic6DofSpringConstraintData);
			Generic6DofSpringConstraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(dofData->m_6dofData.m_rbAFrame);
				rbBFrame.deSerializeFloat(dofData->m_6dofData.m_rbBFrame);
				dof = createGeneric6DofSpringConstraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_6dofData.m_useLinearReferenceFrameA != 0);
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpringConstraint: requires rbA && rbB\n");
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeFloat(dofData->m_6dofData.m_angularLowerLimit);
				angUpperLimit.deSerializeFloat(dofData->m_6dofData.m_angularUpperLimit);
				linLowerLimit.deSerializeFloat(dofData->m_6dofData.m_linearLowerLimit);
				linUpperlimit.deSerializeFloat(dofData->m_6dofData.m_linearUpperLimit);

				angLowerLimit.setW(0.f);
				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);

				i32 i;
				if (fileVersion > 280)
				{
					for (i = 0; i < 6; i++)
					{
						dof->setStiffness(i, (Scalar)dofData->m_springStiffness[i]);
						dof->setEquilibriumPoint(i, (Scalar)dofData->m_equilibriumPoint[i]);
						dof->enableSpring(i, dofData->m_springEnabled[i] != 0);
						dof->setDamping(i, (Scalar)dofData->m_springDamping[i]);
					}
				}
			}

			constraint = dof;
			break;
		}
		case D6_CONSTRAINT_TYPE:
		{
			Generic6DofConstraintData* dofData = (Generic6DofConstraintData*)constraintData;
			Generic6DofConstraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(dofData->m_rbAFrame);
				rbBFrame.deSerializeFloat(dofData->m_rbBFrame);
				dof = createGeneric6DofConstraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_useLinearReferenceFrameA != 0);
			}
			else
			{
				if (rbB)
				{
					Transform2 rbBFrame;
					rbBFrame.deSerializeFloat(dofData->m_rbBFrame);
					dof = createGeneric6DofConstraint(*rbB, rbBFrame, dofData->m_useLinearReferenceFrameA != 0);
				}
				else
				{
					printf("Error in btWorldImporter::createGeneric6DofConstraint: missing rbB\n");
				}
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeFloat(dofData->m_angularLowerLimit);
				angUpperLimit.deSerializeFloat(dofData->m_angularUpperLimit);
				linLowerLimit.deSerializeFloat(dofData->m_linearLowerLimit);
				linUpperlimit.deSerializeFloat(dofData->m_linearUpperLimit);

				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);
			}

			constraint = dof;
			break;
		}
		case SLIDER_CONSTRAINT_TYPE:
		{
			SliderConstraintData* sliderData = (SliderConstraintData*)constraintData;
			SliderConstraint* slider = 0;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(sliderData->m_rbAFrame);
				rbBFrame.deSerializeFloat(sliderData->m_rbBFrame);
				slider = createSliderConstraint(*rbA, *rbB, rbAFrame, rbBFrame, sliderData->m_useLinearReferenceFrameA != 0);
			}
			else
			{
				Transform2 rbBFrame;
				rbBFrame.deSerializeFloat(sliderData->m_rbBFrame);
				slider = createSliderConstraint(*rbB, rbBFrame, sliderData->m_useLinearReferenceFrameA != 0);
			}
			slider->setLowerLinLimit((Scalar)sliderData->m_linearLowerLimit);
			slider->setUpperLinLimit((Scalar)sliderData->m_linearUpperLimit);
			slider->setLowerAngLimit((Scalar)sliderData->m_angularLowerLimit);
			slider->setUpperAngLimit((Scalar)sliderData->m_angularUpperLimit);
			slider->setUseFrameOffset(sliderData->m_useOffsetForConstraintFrame != 0);
			constraint = slider;
			break;
		}

		default:
		{
			printf("unknown constraint type\n");
		}
	};

	if (constraint)
	{
		constraint->setDbgDrawSize((Scalar)constraintData->m_dbgDrawSize);
		///those fields didn't exist and set to zero for pre-280 versions, so do a check here
		if (fileVersion >= 280)
		{
			constraint->setBreakingImpulseThreshold((Scalar)constraintData->m_breakingImpulseThreshold);
			constraint->setEnabled(constraintData->m_isEnabled != 0);
			constraint->setOverrideNumSolverIterations(constraintData->m_overrideNumSolverIterations);
		}

		if (constraintData->m_name)
		{
			tuk newname = duplicateName(constraintData->m_name);
			m_nameConstraintMap.insert(newname, constraint);
			m_objectNameMap.insert(constraint, newname);
		}
		if (m_dynamicsWorld)
			m_dynamicsWorld->addConstraint(constraint, constraintData->m_disableCollisionsBetweenLinkedBodies != 0);
	}
}

void WorldImporter::convertConstraintFloat(TypedConstraintFloatData* constraintData, RigidBody* rbA, RigidBody* rbB, i32 fileVersion)
{
	TypedConstraint* constraint = 0;

	switch (constraintData->m_objectType)
	{
		case POINT2POINT_CONSTRAINT_TYPE:
		{
			Point2PointConstraintFloatData* p2pData = (Point2PointConstraintFloatData*)constraintData;
			if (rbA && rbB)
			{
				Vec3 pivotInA, pivotInB;
				pivotInA.deSerializeFloat(p2pData->m_pivotInA);
				pivotInB.deSerializeFloat(p2pData->m_pivotInB);
				constraint = createPoint2PointConstraint(*rbA, *rbB, pivotInA, pivotInB);
			}
			else
			{
				Vec3 pivotInA;
				pivotInA.deSerializeFloat(p2pData->m_pivotInA);
				constraint = createPoint2PointConstraint(*rbA, pivotInA);
			}
			break;
		}
		case HINGE_CONSTRAINT_TYPE:
		{
			HingeConstraint* hinge = 0;
			HingeConstraintFloatData* hingeData = (HingeConstraintFloatData*)constraintData;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(hingeData->m_rbAFrame);
				rbBFrame.deSerializeFloat(hingeData->m_rbBFrame);
				hinge = createHingeConstraint(*rbA, *rbB, rbAFrame, rbBFrame, hingeData->m_useReferenceFrameA != 0);
			}
			else
			{
				Transform2 rbAFrame;
				rbAFrame.deSerializeFloat(hingeData->m_rbAFrame);
				hinge = createHingeConstraint(*rbA, rbAFrame, hingeData->m_useReferenceFrameA != 0);
			}
			if (hingeData->m_enableAngularMotor)
			{
				hinge->enableAngularMotor(true, hingeData->m_motorTargetVelocity, hingeData->m_maxMotorImpulse);
			}
			hinge->setAngularOnly(hingeData->m_angularOnly != 0);
			hinge->setLimit(Scalar(hingeData->m_lowerLimit), Scalar(hingeData->m_upperLimit), Scalar(hingeData->m_limitSoftness), Scalar(hingeData->m_biasFactor), Scalar(hingeData->m_relaxationFactor));

			constraint = hinge;
			break;
		}
		case CONETWIST_CONSTRAINT_TYPE:
		{
			ConeTwistConstraintData* coneData = (ConeTwistConstraintData*)constraintData;
			ConeTwistConstraint* coneTwist = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(coneData->m_rbAFrame);
				rbBFrame.deSerializeFloat(coneData->m_rbBFrame);
				coneTwist = createConeTwistConstraint(*rbA, *rbB, rbAFrame, rbBFrame);
			}
			else
			{
				Transform2 rbAFrame;
				rbAFrame.deSerializeFloat(coneData->m_rbAFrame);
				coneTwist = createConeTwistConstraint(*rbA, rbAFrame);
			}
			coneTwist->setLimit(coneData->m_swingSpan1, coneData->m_swingSpan2, coneData->m_twistSpan, coneData->m_limitSoftness, coneData->m_biasFactor, coneData->m_relaxationFactor);
			coneTwist->setDamping(coneData->m_damping);

			constraint = coneTwist;
			break;
		}

		case D6_SPRING_CONSTRAINT_TYPE:
		{
			Generic6DofSpringConstraintData* dofData = (Generic6DofSpringConstraintData*)constraintData;
			//	i32 sz = sizeof(Generic6DofSpringConstraintData);
			Generic6DofSpringConstraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(dofData->m_6dofData.m_rbAFrame);
				rbBFrame.deSerializeFloat(dofData->m_6dofData.m_rbBFrame);
				dof = createGeneric6DofSpringConstraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_6dofData.m_useLinearReferenceFrameA != 0);
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpringConstraint: requires rbA && rbB\n");
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeFloat(dofData->m_6dofData.m_angularLowerLimit);
				angUpperLimit.deSerializeFloat(dofData->m_6dofData.m_angularUpperLimit);
				linLowerLimit.deSerializeFloat(dofData->m_6dofData.m_linearLowerLimit);
				linUpperlimit.deSerializeFloat(dofData->m_6dofData.m_linearUpperLimit);

				angLowerLimit.setW(0.f);
				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);

				i32 i;
				if (fileVersion > 280)
				{
					for (i = 0; i < 6; i++)
					{
						dof->setStiffness(i, dofData->m_springStiffness[i]);
						dof->setEquilibriumPoint(i, dofData->m_equilibriumPoint[i]);
						dof->enableSpring(i, dofData->m_springEnabled[i] != 0);
						dof->setDamping(i, dofData->m_springDamping[i]);
					}
				}
			}

			constraint = dof;
			break;
		}
		case D6_CONSTRAINT_TYPE:
		{
			Generic6DofConstraintData* dofData = (Generic6DofConstraintData*)constraintData;
			Generic6DofConstraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(dofData->m_rbAFrame);
				rbBFrame.deSerializeFloat(dofData->m_rbBFrame);
				dof = createGeneric6DofConstraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_useLinearReferenceFrameA != 0);
			}
			else
			{
				if (rbB)
				{
					Transform2 rbBFrame;
					rbBFrame.deSerializeFloat(dofData->m_rbBFrame);
					dof = createGeneric6DofConstraint(*rbB, rbBFrame, dofData->m_useLinearReferenceFrameA != 0);
				}
				else
				{
					printf("Error in btWorldImporter::createGeneric6DofConstraint: missing rbB\n");
				}
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeFloat(dofData->m_angularLowerLimit);
				angUpperLimit.deSerializeFloat(dofData->m_angularUpperLimit);
				linLowerLimit.deSerializeFloat(dofData->m_linearLowerLimit);
				linUpperlimit.deSerializeFloat(dofData->m_linearUpperLimit);

				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);
			}

			constraint = dof;
			break;
		}
		case SLIDER_CONSTRAINT_TYPE:
		{
			SliderConstraintData* sliderData = (SliderConstraintData*)constraintData;
			SliderConstraint* slider = 0;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(sliderData->m_rbAFrame);
				rbBFrame.deSerializeFloat(sliderData->m_rbBFrame);
				slider = createSliderConstraint(*rbA, *rbB, rbAFrame, rbBFrame, sliderData->m_useLinearReferenceFrameA != 0);
			}
			else
			{
				Transform2 rbBFrame;
				rbBFrame.deSerializeFloat(sliderData->m_rbBFrame);
				slider = createSliderConstraint(*rbB, rbBFrame, sliderData->m_useLinearReferenceFrameA != 0);
			}
			slider->setLowerLinLimit(sliderData->m_linearLowerLimit);
			slider->setUpperLinLimit(sliderData->m_linearUpperLimit);
			slider->setLowerAngLimit(sliderData->m_angularLowerLimit);
			slider->setUpperAngLimit(sliderData->m_angularUpperLimit);
			slider->setUseFrameOffset(sliderData->m_useOffsetForConstraintFrame != 0);
			constraint = slider;
			break;
		}
		case GEAR_CONSTRAINT_TYPE:
		{
			GearConstraintFloatData* gearData = (GearConstraintFloatData*)constraintData;
			GearConstraint* gear = 0;
			if (rbA && rbB)
			{
				Vec3 axisInA, axisInB;
				axisInA.deSerializeFloat(gearData->m_axisInA);
				axisInB.deSerializeFloat(gearData->m_axisInB);
				gear = createGearConstraint(*rbA, *rbB, axisInA, axisInB, gearData->m_ratio);
			}
			else
			{
				Assert(0);
				//perhaps a gear against a 'fixed' body, while the 'fixed' body is not serialized?
				//GearConstraint(RigidBody& rbA, btRigidBody& rbB, const Vec3& axisInA,const Vec3& axisInB, Scalar ratio=1.f);
			}
			constraint = gear;
			break;
		}
		case D6_SPRING_2_CONSTRAINT_TYPE:
		{
			Generic6DofSpring2ConstraintData* dofData = (Generic6DofSpring2ConstraintData*)constraintData;

			Generic6DofSpring2Constraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeFloat(dofData->m_rbAFrame);
				rbBFrame.deSerializeFloat(dofData->m_rbBFrame);
				dof = createGeneric6DofSpring2Constraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_rotateOrder);
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpring2Constraint: requires rbA && rbB\n");
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeFloat(dofData->m_angularLowerLimit);
				angUpperLimit.deSerializeFloat(dofData->m_angularUpperLimit);
				linLowerLimit.deSerializeFloat(dofData->m_linearLowerLimit);
				linUpperlimit.deSerializeFloat(dofData->m_linearUpperLimit);

				angLowerLimit.setW(0.f);
				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);

				i32 i;
				if (fileVersion > 280)
				{
					//6-dof: 3 linear followed by 3 angular
					for (i = 0; i < 3; i++)
					{
						dof->setStiffness(i, dofData->m_linearSpringStiffness.m_floats[i], dofData->m_linearSpringStiffnessLimited[i] != 0);
						dof->setEquilibriumPoint(i, dofData->m_linearEquilibriumPoint.m_floats[i]);
						dof->enableSpring(i, dofData->m_linearEnableSpring[i] != 0);
						dof->setDamping(i, dofData->m_linearSpringDamping.m_floats[i], (dofData->m_linearSpringDampingLimited[i] != 0));
					}
					for (i = 0; i < 3; i++)
					{
						dof->setStiffness(i + 3, dofData->m_angularSpringStiffness.m_floats[i], (dofData->m_angularSpringStiffnessLimited[i] != 0));
						dof->setEquilibriumPoint(i + 3, dofData->m_angularEquilibriumPoint.m_floats[i]);
						dof->enableSpring(i + 3, dofData->m_angularEnableSpring[i] != 0);
						dof->setDamping(i + 3, dofData->m_angularSpringDamping.m_floats[i], dofData->m_angularSpringDampingLimited[i]);
					}
				}
			}

			constraint = dof;
			break;
		}
		case FIXED_CONSTRAINT_TYPE:
		{
			Generic6DofSpring2Constraint* dof = 0;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				//compute a shared world frame, and compute frameInA, frameInB relative to this
				Transform2 sharedFrame;
				sharedFrame.setIdentity();
				Vec3 centerPos = Scalar(0.5) * (rbA->getWorldTransform().getOrigin() +
													   rbB->getWorldTransform().getOrigin());
				sharedFrame.setOrigin(centerPos);
				rbAFrame = rbA->getWorldTransform().inverse() * sharedFrame;
				rbBFrame = rbB->getWorldTransform().inverse() * sharedFrame;

				dof = createGeneric6DofSpring2Constraint(*rbA, *rbB, rbAFrame, rbBFrame, RO_XYZ);
				dof->setLinearUpperLimit(Vec3(0, 0, 0));
				dof->setLinearLowerLimit(Vec3(0, 0, 0));
				dof->setAngularUpperLimit(Vec3(0, 0, 0));
				dof->setAngularLowerLimit(Vec3(0, 0, 0));
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpring2Constraint: requires rbA && rbB\n");
			}

			constraint = dof;
			break;
		}
		default:
		{
			printf("unknown constraint type\n");
		}
	};

	if (constraint)
	{
		constraint->setDbgDrawSize(constraintData->m_dbgDrawSize);
		///those fields didn't exist and set to zero for pre-280 versions, so do a check here
		if (fileVersion >= 280)
		{
			constraint->setBreakingImpulseThreshold(constraintData->m_breakingImpulseThreshold);
			constraint->setEnabled(constraintData->m_isEnabled != 0);
			constraint->setOverrideNumSolverIterations(constraintData->m_overrideNumSolverIterations);
		}

		if (constraintData->m_name)
		{
			tuk newname = duplicateName(constraintData->m_name);
			m_nameConstraintMap.insert(newname, constraint);
			m_objectNameMap.insert(constraint, newname);
		}
		if (m_dynamicsWorld)
			m_dynamicsWorld->addConstraint(constraint, constraintData->m_disableCollisionsBetweenLinkedBodies != 0);
	}
}

void WorldImporter::convertConstraintDouble(TypedConstraintDoubleData* constraintData, RigidBody* rbA, RigidBody* rbB, i32 fileVersion)
{
	TypedConstraint* constraint = 0;

	switch (constraintData->m_objectType)
	{
		case POINT2POINT_CONSTRAINT_TYPE:
		{
			Point2PointConstraintDoubleData2* p2pData = (Point2PointConstraintDoubleData2*)constraintData;
			if (rbA && rbB)
			{
				Vec3 pivotInA, pivotInB;
				pivotInA.deSerializeDouble(p2pData->m_pivotInA);
				pivotInB.deSerializeDouble(p2pData->m_pivotInB);
				constraint = createPoint2PointConstraint(*rbA, *rbB, pivotInA, pivotInB);
			}
			else
			{
				Vec3 pivotInA;
				pivotInA.deSerializeDouble(p2pData->m_pivotInA);
				constraint = createPoint2PointConstraint(*rbA, pivotInA);
			}
			break;
		}
		case HINGE_CONSTRAINT_TYPE:
		{
			HingeConstraint* hinge = 0;

			HingeConstraintDoubleData2* hingeData = (HingeConstraintDoubleData2*)constraintData;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(hingeData->m_rbAFrame);
				rbBFrame.deSerializeDouble(hingeData->m_rbBFrame);
				hinge = createHingeConstraint(*rbA, *rbB, rbAFrame, rbBFrame, hingeData->m_useReferenceFrameA != 0);
			}
			else
			{
				Transform2 rbAFrame;
				rbAFrame.deSerializeDouble(hingeData->m_rbAFrame);
				hinge = createHingeConstraint(*rbA, rbAFrame, hingeData->m_useReferenceFrameA != 0);
			}
			if (hingeData->m_enableAngularMotor)
			{
				hinge->enableAngularMotor(true, (Scalar)hingeData->m_motorTargetVelocity, (Scalar)hingeData->m_maxMotorImpulse);
			}
			hinge->setAngularOnly(hingeData->m_angularOnly != 0);
			hinge->setLimit(Scalar(hingeData->m_lowerLimit), Scalar(hingeData->m_upperLimit), Scalar(hingeData->m_limitSoftness), Scalar(hingeData->m_biasFactor), Scalar(hingeData->m_relaxationFactor));

			constraint = hinge;
			break;
		}
		case CONETWIST_CONSTRAINT_TYPE:
		{
			ConeTwistConstraintDoubleData* coneData = (ConeTwistConstraintDoubleData*)constraintData;
			ConeTwistConstraint* coneTwist = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(coneData->m_rbAFrame);
				rbBFrame.deSerializeDouble(coneData->m_rbBFrame);
				coneTwist = createConeTwistConstraint(*rbA, *rbB, rbAFrame, rbBFrame);
			}
			else
			{
				Transform2 rbAFrame;
				rbAFrame.deSerializeDouble(coneData->m_rbAFrame);
				coneTwist = createConeTwistConstraint(*rbA, rbAFrame);
			}
			coneTwist->setLimit((Scalar)coneData->m_swingSpan1, (Scalar)coneData->m_swingSpan2, (Scalar)coneData->m_twistSpan, (Scalar)coneData->m_limitSoftness,
								(Scalar)coneData->m_biasFactor, (Scalar)coneData->m_relaxationFactor);
			coneTwist->setDamping((Scalar)coneData->m_damping);

			constraint = coneTwist;
			break;
		}

		case D6_SPRING_CONSTRAINT_TYPE:
		{
			Generic6DofSpringConstraintDoubleData2* dofData = (Generic6DofSpringConstraintDoubleData2*)constraintData;
			//	i32 sz = sizeof(Generic6DofSpringConstraintData);
			Generic6DofSpringConstraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(dofData->m_6dofData.m_rbAFrame);
				rbBFrame.deSerializeDouble(dofData->m_6dofData.m_rbBFrame);
				dof = createGeneric6DofSpringConstraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_6dofData.m_useLinearReferenceFrameA != 0);
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpringConstraint: requires rbA && rbB\n");
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeDouble(dofData->m_6dofData.m_angularLowerLimit);
				angUpperLimit.deSerializeDouble(dofData->m_6dofData.m_angularUpperLimit);
				linLowerLimit.deSerializeDouble(dofData->m_6dofData.m_linearLowerLimit);
				linUpperlimit.deSerializeDouble(dofData->m_6dofData.m_linearUpperLimit);

				angLowerLimit.setW(0.f);
				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);

				i32 i;
				if (fileVersion > 280)
				{
					for (i = 0; i < 6; i++)
					{
						dof->setStiffness(i, (Scalar)dofData->m_springStiffness[i]);
						dof->setEquilibriumPoint(i, (Scalar)dofData->m_equilibriumPoint[i]);
						dof->enableSpring(i, dofData->m_springEnabled[i] != 0);
						dof->setDamping(i, (Scalar)dofData->m_springDamping[i]);
					}
				}
			}

			constraint = dof;
			break;
		}
		case D6_CONSTRAINT_TYPE:
		{
			Generic6DofConstraintDoubleData2* dofData = (Generic6DofConstraintDoubleData2*)constraintData;
			Generic6DofConstraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(dofData->m_rbAFrame);
				rbBFrame.deSerializeDouble(dofData->m_rbBFrame);
				dof = createGeneric6DofConstraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_useLinearReferenceFrameA != 0);
			}
			else
			{
				if (rbB)
				{
					Transform2 rbBFrame;
					rbBFrame.deSerializeDouble(dofData->m_rbBFrame);
					dof = createGeneric6DofConstraint(*rbB, rbBFrame, dofData->m_useLinearReferenceFrameA != 0);
				}
				else
				{
					printf("Error in btWorldImporter::createGeneric6DofConstraint: missing rbB\n");
				}
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeDouble(dofData->m_angularLowerLimit);
				angUpperLimit.deSerializeDouble(dofData->m_angularUpperLimit);
				linLowerLimit.deSerializeDouble(dofData->m_linearLowerLimit);
				linUpperlimit.deSerializeDouble(dofData->m_linearUpperLimit);

				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);
			}

			constraint = dof;
			break;
		}
		case SLIDER_CONSTRAINT_TYPE:
		{
			SliderConstraintDoubleData* sliderData = (SliderConstraintDoubleData*)constraintData;
			SliderConstraint* slider = 0;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(sliderData->m_rbAFrame);
				rbBFrame.deSerializeDouble(sliderData->m_rbBFrame);
				slider = createSliderConstraint(*rbA, *rbB, rbAFrame, rbBFrame, sliderData->m_useLinearReferenceFrameA != 0);
			}
			else
			{
				Transform2 rbBFrame;
				rbBFrame.deSerializeDouble(sliderData->m_rbBFrame);
				slider = createSliderConstraint(*rbB, rbBFrame, sliderData->m_useLinearReferenceFrameA != 0);
			}
			slider->setLowerLinLimit((Scalar)sliderData->m_linearLowerLimit);
			slider->setUpperLinLimit((Scalar)sliderData->m_linearUpperLimit);
			slider->setLowerAngLimit((Scalar)sliderData->m_angularLowerLimit);
			slider->setUpperAngLimit((Scalar)sliderData->m_angularUpperLimit);
			slider->setUseFrameOffset(sliderData->m_useOffsetForConstraintFrame != 0);
			constraint = slider;
			break;
		}
		case GEAR_CONSTRAINT_TYPE:
		{
			GearConstraintDoubleData* gearData = (GearConstraintDoubleData*)constraintData;
			GearConstraint* gear = 0;
			if (rbA && rbB)
			{
				Vec3 axisInA, axisInB;
				axisInA.deSerializeDouble(gearData->m_axisInA);
				axisInB.deSerializeDouble(gearData->m_axisInB);
				gear = createGearConstraint(*rbA, *rbB, axisInA, axisInB, gearData->m_ratio);
			}
			else
			{
				Assert(0);
				//perhaps a gear against a 'fixed' body, while the 'fixed' body is not serialized?
				//GearConstraint(RigidBody& rbA, btRigidBody& rbB, const Vec3& axisInA,const Vec3& axisInB, Scalar ratio=1.f);
			}
			constraint = gear;
			break;
		}

		case D6_SPRING_2_CONSTRAINT_TYPE:
		{
			Generic6DofSpring2ConstraintDoubleData2* dofData = (Generic6DofSpring2ConstraintDoubleData2*)constraintData;

			Generic6DofSpring2Constraint* dof = 0;

			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				rbAFrame.deSerializeDouble(dofData->m_rbAFrame);
				rbBFrame.deSerializeDouble(dofData->m_rbBFrame);
				dof = createGeneric6DofSpring2Constraint(*rbA, *rbB, rbAFrame, rbBFrame, dofData->m_rotateOrder);
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpring2Constraint: requires rbA && rbB\n");
			}

			if (dof)
			{
				Vec3 angLowerLimit, angUpperLimit, linLowerLimit, linUpperlimit;
				angLowerLimit.deSerializeDouble(dofData->m_angularLowerLimit);
				angUpperLimit.deSerializeDouble(dofData->m_angularUpperLimit);
				linLowerLimit.deSerializeDouble(dofData->m_linearLowerLimit);
				linUpperlimit.deSerializeDouble(dofData->m_linearUpperLimit);

				angLowerLimit.setW(0.f);
				dof->setAngularLowerLimit(angLowerLimit);
				dof->setAngularUpperLimit(angUpperLimit);
				dof->setLinearLowerLimit(linLowerLimit);
				dof->setLinearUpperLimit(linUpperlimit);

				i32 i;
				if (fileVersion > 280)
				{
					//6-dof: 3 linear followed by 3 angular
					for (i = 0; i < 3; i++)
					{
						dof->setStiffness(i, dofData->m_linearSpringStiffness.m_floats[i], dofData->m_linearSpringStiffnessLimited[i]);
						dof->setEquilibriumPoint(i, dofData->m_linearEquilibriumPoint.m_floats[i]);
						dof->enableSpring(i, dofData->m_linearEnableSpring[i] != 0);
						dof->setDamping(i, dofData->m_linearSpringDamping.m_floats[i], (dofData->m_linearSpringDampingLimited[i] != 0));
					}
					for (i = 0; i < 3; i++)
					{
						dof->setStiffness(i + 3, dofData->m_angularSpringStiffness.m_floats[i], (dofData->m_angularSpringStiffnessLimited[i] != 0));
						dof->setEquilibriumPoint(i + 3, dofData->m_angularEquilibriumPoint.m_floats[i]);
						dof->enableSpring(i + 3, dofData->m_angularEnableSpring[i] != 0);
						dof->setDamping(i + 3, dofData->m_angularSpringDamping.m_floats[i], (dofData->m_angularSpringDampingLimited[i] != 0));
					}
				}
			}

			constraint = dof;
			break;
		}
		case FIXED_CONSTRAINT_TYPE:
		{
			Generic6DofSpring2Constraint* dof = 0;
			if (rbA && rbB)
			{
				Transform2 rbAFrame, rbBFrame;
				//compute a shared world frame, and compute frameInA, frameInB relative to this
				Transform2 sharedFrame;
				sharedFrame.setIdentity();
				Vec3 centerPos = Scalar(0.5) * (rbA->getWorldTransform().getOrigin() +
													   rbB->getWorldTransform().getOrigin());
				sharedFrame.setOrigin(centerPos);
				rbAFrame = rbA->getWorldTransform().inverse() * sharedFrame;
				rbBFrame = rbB->getWorldTransform().inverse() * sharedFrame;

				dof = createGeneric6DofSpring2Constraint(*rbA, *rbB, rbAFrame, rbBFrame, RO_XYZ);
				dof->setLinearUpperLimit(Vec3(0, 0, 0));
				dof->setLinearLowerLimit(Vec3(0, 0, 0));
				dof->setAngularUpperLimit(Vec3(0, 0, 0));
				dof->setAngularLowerLimit(Vec3(0, 0, 0));
			}
			else
			{
				printf("Error in btWorldImporter::createGeneric6DofSpring2Constraint: requires rbA && rbB\n");
			}

			constraint = dof;
			break;
		}

		default:
		{
			printf("unknown constraint type\n");
		}
	};

	if (constraint)
	{
		constraint->setDbgDrawSize((Scalar)constraintData->m_dbgDrawSize);
		///those fields didn't exist and set to zero for pre-280 versions, so do a check here
		if (fileVersion >= 280)
		{
			constraint->setBreakingImpulseThreshold((Scalar)constraintData->m_breakingImpulseThreshold);
			constraint->setEnabled(constraintData->m_isEnabled != 0);
			constraint->setOverrideNumSolverIterations(constraintData->m_overrideNumSolverIterations);
		}

		if (constraintData->m_name)
		{
			tuk newname = duplicateName(constraintData->m_name);
			m_nameConstraintMap.insert(newname, constraint);
			m_objectNameMap.insert(constraint, newname);
		}
		if (m_dynamicsWorld)
			m_dynamicsWorld->addConstraint(constraint, constraintData->m_disableCollisionsBetweenLinkedBodies != 0);
	}
}

TriangleIndexVertexArray* WorldImporter::createMeshInterface(StridingMeshInterfaceData& meshData)
{
	TriangleIndexVertexArray* meshInterface = createTriangleMeshContainer();

	for (i32 i = 0; i < meshData.m_numMeshParts; i++)
	{
		IndexedMesh meshPart;
		meshPart.m_numTriangles = meshData.m_meshPartsPtr[i].m_numTriangles;
		meshPart.m_numVertices = meshData.m_meshPartsPtr[i].m_numVertices;

		if (meshData.m_meshPartsPtr[i].m_indices32)
		{
			meshPart.m_indexType = PHY_INTEGER;
			meshPart.m_triangleIndexStride = 3 * sizeof(i32);
			i32* indexArray = (i32*)AlignedAlloc(sizeof(i32) * 3 * meshPart.m_numTriangles, 16);
			m_indexArrays.push_back(indexArray);
			for (i32 j = 0; j < 3 * meshPart.m_numTriangles; j++)
			{
				indexArray[j] = meshData.m_meshPartsPtr[i].m_indices32[j].m_value;
			}
			meshPart.m_triangleIndexBase = (u8k*)indexArray;
		}
		else
		{
			if (meshData.m_meshPartsPtr[i].m_3indices16)
			{
				meshPart.m_indexType = PHY_SHORT;
				meshPart.m_triangleIndexStride = sizeof(i16) * 3;  //sizeof(ShortIntIndexTripletData);

				i16* indexArray = (i16*)AlignedAlloc(sizeof(i16) * 3 * meshPart.m_numTriangles, 16);
				m_shortIndexArrays.push_back(indexArray);

				for (i32 j = 0; j < meshPart.m_numTriangles; j++)
				{
					indexArray[3 * j] = meshData.m_meshPartsPtr[i].m_3indices16[j].m_values[0];
					indexArray[3 * j + 1] = meshData.m_meshPartsPtr[i].m_3indices16[j].m_values[1];
					indexArray[3 * j + 2] = meshData.m_meshPartsPtr[i].m_3indices16[j].m_values[2];
				}

				meshPart.m_triangleIndexBase = (u8k*)indexArray;
			}
			if (meshData.m_meshPartsPtr[i].m_indices16)
			{
				meshPart.m_indexType = PHY_SHORT;
				meshPart.m_triangleIndexStride = 3 * sizeof(i16);
				i16* indexArray = (i16*)AlignedAlloc(sizeof(i16) * 3 * meshPart.m_numTriangles, 16);
				m_shortIndexArrays.push_back(indexArray);
				for (i32 j = 0; j < 3 * meshPart.m_numTriangles; j++)
				{
					indexArray[j] = meshData.m_meshPartsPtr[i].m_indices16[j].m_value;
				}

				meshPart.m_triangleIndexBase = (u8k*)indexArray;
			}

			if (meshData.m_meshPartsPtr[i].m_3indices8)
			{
				meshPart.m_indexType = PHY_UCHAR;
				meshPart.m_triangleIndexStride = sizeof(u8) * 3;

				u8* indexArray = (u8*)AlignedAlloc(sizeof(u8) * 3 * meshPart.m_numTriangles, 16);
				m_charIndexArrays.push_back(indexArray);

				for (i32 j = 0; j < meshPart.m_numTriangles; j++)
				{
					indexArray[3 * j] = meshData.m_meshPartsPtr[i].m_3indices8[j].m_values[0];
					indexArray[3 * j + 1] = meshData.m_meshPartsPtr[i].m_3indices8[j].m_values[1];
					indexArray[3 * j + 2] = meshData.m_meshPartsPtr[i].m_3indices8[j].m_values[2];
				}

				meshPart.m_triangleIndexBase = (u8k*)indexArray;
			}
		}

		if (meshData.m_meshPartsPtr[i].m_vertices3f)
		{
			meshPart.m_vertexType = PHY_FLOAT;
			meshPart.m_vertexStride = sizeof(Vec3FloatData);
			Vec3FloatData* vertices = (Vec3FloatData*)AlignedAlloc(sizeof(Vec3FloatData) * meshPart.m_numVertices, 16);
			m_floatVertexArrays.push_back(vertices);

			for (i32 j = 0; j < meshPart.m_numVertices; j++)
			{
				vertices[j].m_floats[0] = meshData.m_meshPartsPtr[i].m_vertices3f[j].m_floats[0];
				vertices[j].m_floats[1] = meshData.m_meshPartsPtr[i].m_vertices3f[j].m_floats[1];
				vertices[j].m_floats[2] = meshData.m_meshPartsPtr[i].m_vertices3f[j].m_floats[2];
				vertices[j].m_floats[3] = meshData.m_meshPartsPtr[i].m_vertices3f[j].m_floats[3];
			}
			meshPart.m_vertexBase = (u8k*)vertices;
		}
		else
		{
			meshPart.m_vertexType = PHY_DOUBLE;
			meshPart.m_vertexStride = sizeof(Vec3DoubleData);

			Vec3DoubleData* vertices = (Vec3DoubleData*)AlignedAlloc(sizeof(Vec3DoubleData) * meshPart.m_numVertices, 16);
			m_doubleVertexArrays.push_back(vertices);

			for (i32 j = 0; j < meshPart.m_numVertices; j++)
			{
				vertices[j].m_floats[0] = meshData.m_meshPartsPtr[i].m_vertices3d[j].m_floats[0];
				vertices[j].m_floats[1] = meshData.m_meshPartsPtr[i].m_vertices3d[j].m_floats[1];
				vertices[j].m_floats[2] = meshData.m_meshPartsPtr[i].m_vertices3d[j].m_floats[2];
				vertices[j].m_floats[3] = meshData.m_meshPartsPtr[i].m_vertices3d[j].m_floats[3];
			}
			meshPart.m_vertexBase = (u8k*)vertices;
		}

		if (meshPart.m_triangleIndexBase && meshPart.m_vertexBase)
		{
			meshInterface->addIndexedMesh(meshPart, meshPart.m_indexType);
		}
	}

	return meshInterface;
}

StridingMeshInterfaceData* WorldImporter::createStridingMeshInterfaceData(StridingMeshInterfaceData* interfaceData)
{
	//create a new btStridingMeshInterfaceData that is an exact copy of shapedata and store it in the WorldImporter
	StridingMeshInterfaceData* newData = new StridingMeshInterfaceData;

	newData->m_scaling = interfaceData->m_scaling;
	newData->m_numMeshParts = interfaceData->m_numMeshParts;
	newData->m_meshPartsPtr = new MeshPartData[newData->m_numMeshParts];

	for (i32 i = 0; i < newData->m_numMeshParts; i++)
	{
		MeshPartData* curPart = &interfaceData->m_meshPartsPtr[i];
		MeshPartData* curNewPart = &newData->m_meshPartsPtr[i];

		curNewPart->m_numTriangles = curPart->m_numTriangles;
		curNewPart->m_numVertices = curPart->m_numVertices;

		if (curPart->m_vertices3f)
		{
			curNewPart->m_vertices3f = new Vec3FloatData[curNewPart->m_numVertices];
			memcpy(curNewPart->m_vertices3f, curPart->m_vertices3f, sizeof(Vec3FloatData) * curNewPart->m_numVertices);
		}
		else
			curNewPart->m_vertices3f = NULL;

		if (curPart->m_vertices3d)
		{
			curNewPart->m_vertices3d = new Vec3DoubleData[curNewPart->m_numVertices];
			memcpy(curNewPart->m_vertices3d, curPart->m_vertices3d, sizeof(Vec3DoubleData) * curNewPart->m_numVertices);
		}
		else
			curNewPart->m_vertices3d = NULL;

		i32 numIndices = curNewPart->m_numTriangles * 3;
		///the m_3indices8 was not initialized in some drx3D versions, this can cause crashes at loading time
		///we catch it by only dealing with m_3indices8 if none of the other indices are initialized
		bool uninitialized3indices8Workaround = false;

		if (curPart->m_indices32)
		{
			uninitialized3indices8Workaround = true;
			curNewPart->m_indices32 = new IntIndexData[numIndices];
			memcpy(curNewPart->m_indices32, curPart->m_indices32, sizeof(IntIndexData) * numIndices);
		}
		else
			curNewPart->m_indices32 = NULL;

		if (curPart->m_3indices16)
		{
			uninitialized3indices8Workaround = true;
			curNewPart->m_3indices16 = new ShortIntIndexTripletData[curNewPart->m_numTriangles];
			memcpy(curNewPart->m_3indices16, curPart->m_3indices16, sizeof(ShortIntIndexTripletData) * curNewPart->m_numTriangles);
		}
		else
			curNewPart->m_3indices16 = NULL;

		if (curPart->m_indices16)
		{
			uninitialized3indices8Workaround = true;
			curNewPart->m_indices16 = new ShortIntIndexData[numIndices];
			memcpy(curNewPart->m_indices16, curPart->m_indices16, sizeof(ShortIntIndexData) * numIndices);
		}
		else
			curNewPart->m_indices16 = NULL;

		if (!uninitialized3indices8Workaround && curPart->m_3indices8)
		{
			curNewPart->m_3indices8 = new CharIndexTripletData[curNewPart->m_numTriangles];
			memcpy(curNewPart->m_3indices8, curPart->m_3indices8, sizeof(CharIndexTripletData) * curNewPart->m_numTriangles);
		}
		else
			curNewPart->m_3indices8 = NULL;
	}

	m_allocatedbtStridingMeshInterfaceDatas.push_back(newData);

	return (newData);
}

#ifdef USE_INTERNAL_EDGE_UTILITY
extern ContactAddedCallback gContactAddedCallback;

static bool AdjustInternalEdgeContactsCallback(ManifoldPoint& cp, const CollisionObject2* colObj0, i32 partId0, i32 index0, const CollisionObject2* colObj1, i32 partId1, i32 index1)
{
	AdjustInternalEdgeContacts(cp, colObj1, colObj0, partId1, index1);
	//AdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1, DRX3D_TRIANGLE_CONVEX_BACKFACE_MODE);
	//AdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1, DRX3D_TRIANGLE_CONVEX_DOUBLE_SIDED+DRX3D_TRIANGLE_CONCAVE_DOUBLE_SIDED);
	return true;
}
#endif  //USE_INTERNAL_EDGE_UTILITY

CollisionObject2* WorldImporter::createCollisionObject(const Transform2& startTransform, CollisionShape* shape, tukk bodyName)
{
	return createRigidBody(false, 0, startTransform, shape, bodyName);
}

void WorldImporter::setDynamicsWorldInfo(const Vec3& gravity, const ContactSolverInfo& solverInfo)
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->setGravity(gravity);
		m_dynamicsWorld->getSolverInfo() = solverInfo;
	}
}

RigidBody* WorldImporter::createRigidBody(bool isDynamic, Scalar mass, const Transform2& startTransform, CollisionShape* shape, tukk bodyName)
{
	Vec3 localInertia;
	localInertia.setZero();

	if (mass)
		shape->calculateLocalInertia(mass, localInertia);

	RigidBody* body = new RigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);

	if (m_dynamicsWorld)
		m_dynamicsWorld->addRigidBody(body);

	if (bodyName)
	{
		tuk newname = duplicateName(bodyName);
		m_objectNameMap.insert(body, newname);
		m_nameBodyMap.insert(newname, body);
	}
	m_allocatedRigidBodies.push_back(body);
	return body;
}

CollisionShape* WorldImporter::createPlaneShape(const Vec3& planeNormal, Scalar planeConstant)
{
	StaticPlaneShape* shape = new StaticPlaneShape(planeNormal, planeConstant);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}
CollisionShape* WorldImporter::createBoxShape(const Vec3& halfExtents)
{
	BoxShape* shape = new BoxShape(halfExtents);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}
CollisionShape* WorldImporter::createSphereShape(Scalar radius)
{
	SphereShape* shape = new SphereShape(radius);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createCapsuleShapeX(Scalar radius, Scalar height)
{
	CapsuleShapeX* shape = new CapsuleShapeX(radius, height);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createCapsuleShapeY(Scalar radius, Scalar height)
{
	CapsuleShape* shape = new CapsuleShape(radius, height);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createCapsuleShapeZ(Scalar radius, Scalar height)
{
	CapsuleShapeZ* shape = new CapsuleShapeZ(radius, height);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createCylinderShapeX(Scalar radius, Scalar height)
{
	CylinderShapeX* shape = new CylinderShapeX(Vec3(height, radius, radius));
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createCylinderShapeY(Scalar radius, Scalar height)
{
	CylinderShape* shape = new CylinderShape(Vec3(radius, height, radius));
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createCylinderShapeZ(Scalar radius, Scalar height)
{
	CylinderShapeZ* shape = new CylinderShapeZ(Vec3(radius, radius, height));
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createConeShapeX(Scalar radius, Scalar height)
{
	ConeShapeX* shape = new ConeShapeX(radius, height);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createConeShapeY(Scalar radius, Scalar height)
{
	ConeShape* shape = new ConeShape(radius, height);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CollisionShape* WorldImporter::createConeShapeZ(Scalar radius, Scalar height)
{
	ConeShapeZ* shape = new ConeShapeZ(radius, height);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

TriangleIndexVertexArray* WorldImporter::createTriangleMeshContainer()
{
	TriangleIndexVertexArray* in = new TriangleIndexVertexArray();
	m_allocatedTriangleIndexArrays.push_back(in);
	return in;
}

OptimizedBvh* WorldImporter::createOptimizedBvh()
{
	OptimizedBvh* bvh = new OptimizedBvh();
	m_allocatedBvhs.push_back(bvh);
	return bvh;
}

TriangleInfoMap* WorldImporter::createTriangleInfoMap()
{
	TriangleInfoMap* tim = new TriangleInfoMap();
	m_allocatedTriangleInfoMaps.push_back(tim);
	return tim;
}

BvhTriangleMeshShape* WorldImporter::createBvhTriangleMeshShape(StridingMeshInterface* trimesh, OptimizedBvh* bvh)
{
	if (bvh)
	{
		BvhTriangleMeshShape* bvhTriMesh = new BvhTriangleMeshShape(trimesh, bvh->isQuantized(), false);
		bvhTriMesh->setOptimizedBvh(bvh);
		m_allocatedCollisionShapes.push_back(bvhTriMesh);
		return bvhTriMesh;
	}

	BvhTriangleMeshShape* ts = new BvhTriangleMeshShape(trimesh, true);
	m_allocatedCollisionShapes.push_back(ts);
	return ts;
}
CollisionShape* WorldImporter::createConvexTriangleMeshShape(StridingMeshInterface* trimesh)
{
	return 0;
}
GImpactMeshShape* WorldImporter::createGimpactShape(StridingMeshInterface* trimesh)
{
#ifdef USE_GIMPACT
	GImpactMeshShape* shape = new GImpactMeshShape(trimesh);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
#else
	return 0;
#endif
}
ConvexHullShape* WorldImporter::createConvexHullShape()
{
	ConvexHullShape* shape = new ConvexHullShape();
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

CompoundShape* WorldImporter::createCompoundShape()
{
	CompoundShape* shape = new CompoundShape();
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

ScaledBvhTriangleMeshShape* WorldImporter::createScaledTrangleMeshShape(BvhTriangleMeshShape* meshShape, const Vec3& localScaling)
{
	ScaledBvhTriangleMeshShape* shape = new ScaledBvhTriangleMeshShape(meshShape, localScaling);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

MultiSphereShape* WorldImporter::createMultiSphereShape(const Vec3* positions, const Scalar* radi, i32 numSpheres)
{
	MultiSphereShape* shape = new MultiSphereShape(positions, radi, numSpheres);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

class HeightfieldTerrainShape* WorldImporter::createHeightfieldShape(i32 heightStickWidth, i32 heightStickLength,
	ukk heightfieldData, Scalar heightScale,
	Scalar minHeight, Scalar maxHeight,
	i32 upAxis, i32 heightDataType,
	bool flipQuadEdges)
{

	HeightfieldTerrainShape* shape = new HeightfieldTerrainShape(heightStickWidth, heightStickLength,
			heightfieldData, heightScale, minHeight, maxHeight, upAxis, PHY_ScalarType(heightDataType), flipQuadEdges);
	m_allocatedCollisionShapes.push_back(shape);
	return shape;
}

RigidBody& WorldImporter::getFixedBody()
{
	static RigidBody s_fixed(0, 0, 0);
	s_fixed.setMassProps(Scalar(0.), Vec3(Scalar(0.), Scalar(0.), Scalar(0.)));
	return s_fixed;
}

Point2PointConstraint* WorldImporter::createPoint2PointConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& pivotInA, const Vec3& pivotInB)
{
	Point2PointConstraint* p2p = new Point2PointConstraint(rbA, rbB, pivotInA, pivotInB);
	m_allocatedConstraints.push_back(p2p);
	return p2p;
}

Point2PointConstraint* WorldImporter::createPoint2PointConstraint(RigidBody& rbA, const Vec3& pivotInA)
{
	Point2PointConstraint* p2p = new Point2PointConstraint(rbA, pivotInA);
	m_allocatedConstraints.push_back(p2p);
	return p2p;
}

HingeConstraint* WorldImporter::createHingeConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& rbAFrame, const Transform2& rbBFrame, bool useReferenceFrameA)
{
	HingeConstraint* hinge = new HingeConstraint(rbA, rbB, rbAFrame, rbBFrame, useReferenceFrameA);
	m_allocatedConstraints.push_back(hinge);
	return hinge;
}

HingeConstraint* WorldImporter::createHingeConstraint(RigidBody& rbA, const Transform2& rbAFrame, bool useReferenceFrameA)
{
	HingeConstraint* hinge = new HingeConstraint(rbA, rbAFrame, useReferenceFrameA);
	m_allocatedConstraints.push_back(hinge);
	return hinge;
}

ConeTwistConstraint* WorldImporter::createConeTwistConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& rbAFrame, const Transform2& rbBFrame)
{
	ConeTwistConstraint* cone = new ConeTwistConstraint(rbA, rbB, rbAFrame, rbBFrame);
	m_allocatedConstraints.push_back(cone);
	return cone;
}

ConeTwistConstraint* WorldImporter::createConeTwistConstraint(RigidBody& rbA, const Transform2& rbAFrame)
{
	ConeTwistConstraint* cone = new ConeTwistConstraint(rbA, rbAFrame);
	m_allocatedConstraints.push_back(cone);
	return cone;
}

Generic6DofConstraint* WorldImporter::createGeneric6DofConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA)
{
	Generic6DofConstraint* dof = new Generic6DofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA);
	m_allocatedConstraints.push_back(dof);
	return dof;
}

Generic6DofConstraint* WorldImporter::createGeneric6DofConstraint(RigidBody& rbB, const Transform2& frameInB, bool useLinearReferenceFrameB)
{
	Generic6DofConstraint* dof = new Generic6DofConstraint(rbB, frameInB, useLinearReferenceFrameB);
	m_allocatedConstraints.push_back(dof);
	return dof;
}

Generic6DofSpring2Constraint* WorldImporter::createGeneric6DofSpring2Constraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, i32 rotateOrder)
{
	Generic6DofSpring2Constraint* dof = new Generic6DofSpring2Constraint(rbA, rbB, frameInA, frameInB, (RotateOrder)rotateOrder);
	m_allocatedConstraints.push_back(dof);
	return dof;
}

Generic6DofSpringConstraint* WorldImporter::createGeneric6DofSpringConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA)
{
	Generic6DofSpringConstraint* dof = new Generic6DofSpringConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA);
	m_allocatedConstraints.push_back(dof);
	return dof;
}

SliderConstraint* WorldImporter::createSliderConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA)
{
	SliderConstraint* slider = new SliderConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA);
	m_allocatedConstraints.push_back(slider);
	return slider;
}

SliderConstraint* WorldImporter::createSliderConstraint(RigidBody& rbB, const Transform2& frameInB, bool useLinearReferenceFrameA)
{
	SliderConstraint* slider = new SliderConstraint(rbB, frameInB, useLinearReferenceFrameA);
	m_allocatedConstraints.push_back(slider);
	return slider;
}

GearConstraint* WorldImporter::createGearConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& axisInA, const Vec3& axisInB, Scalar ratio)
{
	GearConstraint* gear = new GearConstraint(rbA, rbB, axisInA, axisInB, ratio);
	m_allocatedConstraints.push_back(gear);
	return gear;
}

// query for data
i32 WorldImporter::getNumCollisionShapes() const
{
	return m_allocatedCollisionShapes.size();
}

CollisionShape* WorldImporter::getCollisionShapeByIndex(i32 index)
{
	return m_allocatedCollisionShapes[index];
}

CollisionShape* WorldImporter::getCollisionShapeByName(tukk name)
{
	CollisionShape** shapePtr = m_nameShapeMap.find(name);
	if (shapePtr && *shapePtr)
	{
		return *shapePtr;
	}
	return 0;
}

RigidBody* WorldImporter::getRigidBodyByName(tukk name)
{
	RigidBody** bodyPtr = m_nameBodyMap.find(name);
	if (bodyPtr && *bodyPtr)
	{
		return *bodyPtr;
	}
	return 0;
}

TypedConstraint* WorldImporter::getConstraintByName(tukk name)
{
	TypedConstraint** constraintPtr = m_nameConstraintMap.find(name);
	if (constraintPtr && *constraintPtr)
	{
		return *constraintPtr;
	}
	return 0;
}

tukk WorldImporter::getNameForPointer(ukk ptr) const
{
	tukk const* namePtr = m_objectNameMap.find(ptr);
	if (namePtr && *namePtr)
		return *namePtr;
	return 0;
}

i32 WorldImporter::getNumRigidBodies() const
{
	return m_allocatedRigidBodies.size();
}

CollisionObject2* WorldImporter::getRigidBodyByIndex(i32 index) const
{
	return m_allocatedRigidBodies[index];
}
i32 WorldImporter::getNumConstraints() const
{
	return m_allocatedConstraints.size();
}

TypedConstraint* WorldImporter::getConstraintByIndex(i32 index) const
{
	return m_allocatedConstraints[index];
}

i32 WorldImporter::getNumBvhs() const
{
	return m_allocatedBvhs.size();
}
OptimizedBvh* WorldImporter::getBvhByIndex(i32 index) const
{
	return m_allocatedBvhs[index];
}

i32 WorldImporter::getNumTriangleInfoMaps() const
{
	return m_allocatedTriangleInfoMaps.size();
}

TriangleInfoMap* WorldImporter::getTriangleInfoMapByIndex(i32 index) const
{
	return m_allocatedTriangleInfoMaps[index];
}

void WorldImporter::convertRigidBodyFloat(RigidBodyFloatData* colObjData)
{
	Scalar mass = Scalar(colObjData->m_inverseMass ? 1.f / colObjData->m_inverseMass : 0.f);
	Vec3 localInertia;
	localInertia.setZero();
	CollisionShape** shapePtr = m_shapeMap.find(colObjData->m_collisionObjectData.m_collisionShape);
	if (shapePtr && *shapePtr)
	{
		Transform2 startTransform;
		colObjData->m_collisionObjectData.m_worldTransform.m_origin.m_floats[3] = 0.f;
		startTransform.deSerializeFloat(colObjData->m_collisionObjectData.m_worldTransform);

		//	startTransform.setBasis(Matrix3x3::getIdentity());
		CollisionShape* shape = (CollisionShape*)*shapePtr;
		if (shape->isNonMoving())
		{
			mass = 0.f;
		}
		if (mass)
		{
			shape->calculateLocalInertia(mass, localInertia);
		}
		bool isDynamic = mass != 0.f;
		RigidBody* body = createRigidBody(isDynamic, mass, startTransform, shape, colObjData->m_collisionObjectData.m_name);
		body->setFriction(colObjData->m_collisionObjectData.m_friction);
		body->setRestitution(colObjData->m_collisionObjectData.m_restitution);
		Vec3 linearFactor, angularFactor;
		linearFactor.deSerializeFloat(colObjData->m_linearFactor);
		angularFactor.deSerializeFloat(colObjData->m_angularFactor);
		body->setLinearFactor(linearFactor);
		body->setAngularFactor(angularFactor);

#ifdef USE_INTERNAL_EDGE_UTILITY
		if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
		{
			BvhTriangleMeshShape* trimesh = (BvhTriangleMeshShape*)shape;
			if (trimesh->getTriangleInfoMap())
			{
				body->setCollisionFlags(body->getCollisionFlags() | CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK);
			}
		}
#endif  //USE_INTERNAL_EDGE_UTILITY
		m_bodyMap.insert(colObjData, body);
	}
	else
	{
		printf("ошибка: no shape found\n");
	}
}

void WorldImporter::convertRigidBodyDouble(RigidBodyDoubleData* colObjData)
{
	Scalar mass = Scalar(colObjData->m_inverseMass ? 1.f / colObjData->m_inverseMass : 0.f);
	Vec3 localInertia;
	localInertia.setZero();
	CollisionShape** shapePtr = m_shapeMap.find(colObjData->m_collisionObjectData.m_collisionShape);
	if (shapePtr && *shapePtr)
	{
		Transform2 startTransform;
		colObjData->m_collisionObjectData.m_worldTransform.m_origin.m_floats[3] = 0.f;
		startTransform.deSerializeDouble(colObjData->m_collisionObjectData.m_worldTransform);

		//	startTransform.setBasis(Matrix3x3::getIdentity());
		CollisionShape* shape = (CollisionShape*)*shapePtr;
		if (shape->isNonMoving())
		{
			mass = 0.f;
		}
		if (mass)
		{
			shape->calculateLocalInertia(mass, localInertia);
		}
		bool isDynamic = mass != 0.f;
		RigidBody* body = createRigidBody(isDynamic, mass, startTransform, shape, colObjData->m_collisionObjectData.m_name);
		body->setFriction(Scalar(colObjData->m_collisionObjectData.m_friction));
		body->setRestitution(Scalar(colObjData->m_collisionObjectData.m_restitution));
		Vec3 linearFactor, angularFactor;
		linearFactor.deSerializeDouble(colObjData->m_linearFactor);
		angularFactor.deSerializeDouble(colObjData->m_angularFactor);
		body->setLinearFactor(linearFactor);
		body->setAngularFactor(angularFactor);

#ifdef USE_INTERNAL_EDGE_UTILITY
		if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
		{
			BvhTriangleMeshShape* trimesh = (BvhTriangleMeshShape*)shape;
			if (trimesh->getTriangleInfoMap())
			{
				body->setCollisionFlags(body->getCollisionFlags() | CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK);
			}
		}
#endif  //USE_INTERNAL_EDGE_UTILITY
		m_bodyMap.insert(colObjData, body);
	}
	else
	{
		printf("ошибка: no shape found\n");
	}
}
