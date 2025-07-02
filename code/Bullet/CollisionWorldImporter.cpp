#include <drx3D/Physics/Collision/Dispatch/CollisionWorldImporter.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Maths/Linear/Serializer.h>  //for BulletSerializedArrays definition

#ifdef SUPPORT_GIMPACT_SHAPE_IMPORT
#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>
#endif  //SUPPORT_GIMPACT_SHAPE_IMPORT

CollisionWorldImporter::CollisionWorldImporter(CollisionWorld* world)
	: m_collisionWorld(world),
	  m_verboseMode(0)
{
}

CollisionWorldImporter::~CollisionWorldImporter()
{
}

bool CollisionWorldImporter::convertAllObjects(BulletSerializedArrays* arrays)
{
	m_shapeMap.clear();
	m_bodyMap.clear();

	i32 i;

	for (i = 0; i < arrays->m_bvhsDouble.size(); i++)
	{
		OptimizedBvh* bvh = createOptimizedBvh();
		QuantizedBvhDoubleData* bvhData = arrays->m_bvhsDouble[i];
		bvh->deSerializeDouble(*bvhData);
		m_bvhMap.insert(arrays->m_bvhsDouble[i], bvh);
	}
	for (i = 0; i < arrays->m_bvhsFloat.size(); i++)
	{
		OptimizedBvh* bvh = createOptimizedBvh();
		QuantizedBvhFloatData* bvhData = arrays->m_bvhsFloat[i];
		bvh->deSerializeFloat(*bvhData);
		m_bvhMap.insert(arrays->m_bvhsFloat[i], bvh);
	}

	for (i = 0; i < arrays->m_colShapeData.size(); i++)
	{
		CollisionShapeData* shapeData = arrays->m_colShapeData[i];
		CollisionShape* shape = convertCollisionShape(shapeData);
		if (shape)
		{
			//		printf("shapeMap.insert(%x,%x)\n",shapeData,shape);
			m_shapeMap.insert(shapeData, shape);
		}

		if (shape && shapeData->m_name)
		{
			tuk newname = duplicateName(shapeData->m_name);
			m_objectNameMap.insert(shape, newname);
			m_nameShapeMap.insert(newname, shape);
		}
	}

	for (i = 0; i < arrays->m_collisionObjectDataDouble.size(); i++)
	{
		CollisionObject2DoubleData* colObjData = arrays->m_collisionObjectDataDouble[i];
		CollisionShape** shapePtr = m_shapeMap.find(colObjData->m_collisionShape);
		if (shapePtr && *shapePtr)
		{
			Transform2 startTransform2;
			colObjData->m_worldTransform.m_origin.m_floats[3] = 0.f;
			startTransform2.deSerializeDouble(colObjData->m_worldTransform);

			CollisionShape* shape = (CollisionShape*)*shapePtr;
			CollisionObject2* body = createCollisionObject2(startTransform2, shape, colObjData->m_name);
			body->setFriction(Scalar(colObjData->m_friction));
			body->setRestitution(Scalar(colObjData->m_restitution));

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
	for (i = 0; i < arrays->m_collisionObjectDataFloat.size(); i++)
	{
		CollisionObject2FloatData* colObjData = arrays->m_collisionObjectDataFloat[i];
		CollisionShape** shapePtr = m_shapeMap.find(colObjData->m_collisionShape);
		if (shapePtr && *shapePtr)
		{
			Transform2 startTransform2;
			colObjData->m_worldTransform.m_origin.m_floats[3] = 0.f;
			startTransform2.deSerializeFloat(colObjData->m_worldTransform);

			CollisionShape* shape = (CollisionShape*)*shapePtr;
			CollisionObject2* body = createCollisionObject2(startTransform2, shape, colObjData->m_name);

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

	return true;
}

void CollisionWorldImporter::deleteAllData()
{
	i32 i;

	for (i = 0; i < m_allocatedCollisionObjects.size(); i++)
	{
		if (m_collisionWorld)
			m_collisionWorld->removeCollisionObject(m_allocatedCollisionObjects[i]);
		delete m_allocatedCollisionObjects[i];
	}

	m_allocatedCollisionObjects.clear();

	for (i = 0; i < m_allocatedShapes.size(); i++)
	{
		delete m_allocatedShapes[i];
	}
	m_allocatedShapes.clear();

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

	for (i = 0; i < m_allocatedStridingMeshInterfaceDatas.size(); i++)
	{
		StridingMeshInterfaceData* curData = m_allocatedStridingMeshInterfaceDatas[i];

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
	m_allocatedStridingMeshInterfaceDatas.clear();

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

CollisionShape* CollisionWorldImporter::convertCollisionShape(CollisionShapeData* shapeData)
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
#ifdef SUPPORT_GIMPACT_SHAPE_IMPORT
		case GIMPACT_SHAPE_PROXYTYPE:
		{
			GImpactMeshShapeData* gimpactData = (GImpactMeshShapeData*)shapeData;
			if (gimpactData->m_gimpactSubType == CONST_GIMPACT_TRIMESH_SHAPE)
			{
				StridingMeshInterfaceData* interfaceData = createStridingMeshInterfaceData(&gimpactData->m_meshInterface);
				TriangleIndexVertexArray* meshInterface = createMeshInterface(*interfaceData);

				GImpactMeshShape* gimpactShape = createGimpactShape(meshInterface);
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
			break;
		}
#endif  //SUPPORT_GIMPACT_SHAPE_IMPORT
		//The CapsuleShape* API has issue passing the margin/scaling/halfextents unmodified through the API
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
					printf("ошибка: wrong up axis for CapsuleShape\n");
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
				gContactAddedCallback = AdjustInternalEdgeContactsCallback;
#endif  //USE_INTERNAL_EDGE_UTILITY
			}

			//printf("trimesh->m_collisionMargin=%f\n",trimesh->m_collisionMargin);
			break;
		}
		case COMPOUND_SHAPE_PROXYTYPE:
		{
			CompoundShapeData* compoundData = (CompoundShapeData*)shapeData;
			CompoundShape* compoundShape = createCompoundShape();

			//CompoundShapeChildData* childShapeDataArray = &compoundData->m_childShapePtr[0];

			AlignedObjectArray<CollisionShape*> childShapes;
			for (i32 i = 0; i < compoundData->m_numChildShapes; i++)
			{
				//CompoundShapeChildData* ptr = &compoundData->m_childShapePtr[i];

				CollisionShapeData* cd = compoundData->m_childShapePtr[i].m_childShape;

				CollisionShape* childShape = convertCollisionShape(cd);
				if (childShape)
				{
					Transform2 localTransform2;
					localTransform2.deSerializeFloat(compoundData->m_childShapePtr[i].m_transform);
					compoundShape->addChildShape(localTransform2, childShape);
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

tuk CollisionWorldImporter::duplicateName(tukk name)
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

TriangleIndexVertexArray* CollisionWorldImporter::createMeshInterface(StridingMeshInterfaceData& meshData)
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

StridingMeshInterfaceData* CollisionWorldImporter::createStridingMeshInterfaceData(StridingMeshInterfaceData* interfaceData)
{
	//create a new StridingMeshInterfaceData that is an exact copy of shapedata and store it in the WorldImporter
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

	m_allocatedStridingMeshInterfaceDatas.push_back(newData);

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

/*
RigidBody*  btWorldImporter::createRigidBody(bool isDynamic, Scalar mass, const Transform2& startTransform2,CollisionShape* shape,tukk bodyName)
{
	Vec3 localInertia;
	localInertia.setZero();

	if (mass)
		shape->calculateLocalInertia(mass,localInertia);

	RigidBody* body = new RigidBody(mass,0,shape,localInertia);
	body->setWorldTransform(startTransform2);

	if (m_dynamicsWorld)
		m_dynamicsWorld->addRigidBody(body);

	if (bodyName)
	{
		tuk newname = duplicateName(bodyName);
		m_objectNameMap.insert(body,newname);
		m_nameBodyMap.insert(newname,body);
	}
	m_allocatedRigidBodies.push_back(body);
	return body;

}
*/

CollisionObject2* CollisionWorldImporter::getCollisionObjectByName(tukk name)
{
	CollisionObject2** bodyPtr = m_nameColObjMap.find(name);
	if (bodyPtr && *bodyPtr)
	{
		return *bodyPtr;
	}
	return 0;
}

CollisionObject2* CollisionWorldImporter::createCollisionObject2(const Transform2& startTransform2, CollisionShape* shape, tukk bodyName)
{
	CollisionObject2* colObj = new CollisionObject2();
	colObj->setWorldTransform(startTransform2);
	colObj->setCollisionShape(shape);
	m_collisionWorld->addCollisionObject(colObj);  //todo: flags etc

	if (bodyName)
	{
		tuk newname = duplicateName(bodyName);
		m_objectNameMap.insert(colObj, newname);
		m_nameColObjMap.insert(newname, colObj);
	}
	m_allocatedCollisionObjects.push_back(colObj);

	return colObj;
}

CollisionShape* CollisionWorldImporter::createPlaneShape(const Vec3& planeNormal, Scalar planeConstant)
{
	StaticPlaneShape* shape = new StaticPlaneShape(planeNormal, planeConstant);
	m_allocatedShapes.push_back(shape);
	return shape;
}
CollisionShape* CollisionWorldImporter::createBoxShape(const Vec3& halfExtents)
{
	BoxShape* shape = new BoxShape(halfExtents);
	m_allocatedShapes.push_back(shape);
	return shape;
}
CollisionShape* CollisionWorldImporter::createSphereShape(Scalar radius)
{
	SphereShape* shape = new SphereShape(radius);
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createCapsuleShapeX(Scalar radius, Scalar height)
{
	CapsuleShapeX* shape = new CapsuleShapeX(radius, height);
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createCapsuleShapeY(Scalar radius, Scalar height)
{
	CapsuleShape* shape = new CapsuleShape(radius, height);
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createCapsuleShapeZ(Scalar radius, Scalar height)
{
	CapsuleShapeZ* shape = new CapsuleShapeZ(radius, height);
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createCylinderShapeX(Scalar radius, Scalar height)
{
	CylinderShapeX* shape = new CylinderShapeX(Vec3(height, radius, radius));
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createCylinderShapeY(Scalar radius, Scalar height)
{
	CylinderShape* shape = new CylinderShape(Vec3(radius, height, radius));
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createCylinderShapeZ(Scalar radius, Scalar height)
{
	CylinderShapeZ* shape = new CylinderShapeZ(Vec3(radius, radius, height));
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createConeShapeX(Scalar radius, Scalar height)
{
	ConeShapeX* shape = new ConeShapeX(radius, height);
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createConeShapeY(Scalar radius, Scalar height)
{
	ConeShape* shape = new ConeShape(radius, height);
	m_allocatedShapes.push_back(shape);
	return shape;
}

CollisionShape* CollisionWorldImporter::createConeShapeZ(Scalar radius, Scalar height)
{
	ConeShapeZ* shape = new ConeShapeZ(radius, height);
	m_allocatedShapes.push_back(shape);
	return shape;
}

TriangleIndexVertexArray* CollisionWorldImporter::createTriangleMeshContainer()
{
	TriangleIndexVertexArray* in = new TriangleIndexVertexArray();
	m_allocatedTriangleIndexArrays.push_back(in);
	return in;
}

OptimizedBvh* CollisionWorldImporter::createOptimizedBvh()
{
	OptimizedBvh* bvh = new OptimizedBvh();
	m_allocatedBvhs.push_back(bvh);
	return bvh;
}

TriangleInfoMap* CollisionWorldImporter::createTriangleInfoMap()
{
	TriangleInfoMap* tim = new TriangleInfoMap();
	m_allocatedTriangleInfoMaps.push_back(tim);
	return tim;
}

BvhTriangleMeshShape* CollisionWorldImporter::createBvhTriangleMeshShape(StridingMeshInterface* trimesh, OptimizedBvh* bvh)
{
	if (bvh)
	{
		BvhTriangleMeshShape* bvhTriMesh = new BvhTriangleMeshShape(trimesh, bvh->isQuantized(), false);
		bvhTriMesh->setOptimizedBvh(bvh);
		m_allocatedShapes.push_back(bvhTriMesh);
		return bvhTriMesh;
	}

	BvhTriangleMeshShape* ts = new BvhTriangleMeshShape(trimesh, true);
	m_allocatedShapes.push_back(ts);
	return ts;
}
CollisionShape* CollisionWorldImporter::createConvexTriangleMeshShape(StridingMeshInterface* trimesh)
{
	return 0;
}
#ifdef SUPPORT_GIMPACT_SHAPE_IMPORT
GImpactMeshShape* CollisionWorldImporter::createGimpactShape(StridingMeshInterface* trimesh)
{
	GImpactMeshShape* shape = new GImpactMeshShape(trimesh);
	m_allocatedShapes.push_back(shape);
	return shape;
}
#endif  //SUPPORT_GIMPACT_SHAPE_IMPORT

ConvexHullShape* CollisionWorldImporter::createConvexHullShape()
{
	ConvexHullShape* shape = new ConvexHullShape();
	m_allocatedShapes.push_back(shape);
	return shape;
}

CompoundShape* CollisionWorldImporter::createCompoundShape()
{
	CompoundShape* shape = new CompoundShape();
	m_allocatedShapes.push_back(shape);
	return shape;
}

ScaledBvhTriangleMeshShape* CollisionWorldImporter::createScaledTrangleMeshShape(BvhTriangleMeshShape* meshShape, const Vec3& localScaling)
{
	ScaledBvhTriangleMeshShape* shape = new ScaledBvhTriangleMeshShape(meshShape, localScaling);
	m_allocatedShapes.push_back(shape);
	return shape;
}

MultiSphereShape* CollisionWorldImporter::createMultiSphereShape(const Vec3* positions, const Scalar* radi, i32 numSpheres)
{
	MultiSphereShape* shape = new MultiSphereShape(positions, radi, numSpheres);
	m_allocatedShapes.push_back(shape);
	return shape;
}

// query for data
i32 CollisionWorldImporter::getNumShapes() const
{
	return m_allocatedShapes.size();
}

CollisionShape* CollisionWorldImporter::getCollisionShapeByIndex(i32 index)
{
	return m_allocatedShapes[index];
}

CollisionShape* CollisionWorldImporter::getCollisionShapeByName(tukk name)
{
	CollisionShape** shapePtr = m_nameShapeMap.find(name);
	if (shapePtr && *shapePtr)
	{
		return *shapePtr;
	}
	return 0;
}

tukk CollisionWorldImporter::getNameForPointer(ukk ptr) const
{
	tukk const* namePtr = m_objectNameMap.find(ptr);
	if (namePtr && *namePtr)
		return *namePtr;
	return 0;
}

i32 CollisionWorldImporter::getNumRigidBodies() const
{
	return m_allocatedRigidBodies.size();
}

CollisionObject2* CollisionWorldImporter::getRigidBodyByIndex(i32 index) const
{
	return m_allocatedRigidBodies[index];
}

i32 CollisionWorldImporter::getNumBvhs() const
{
	return m_allocatedBvhs.size();
}
OptimizedBvh* CollisionWorldImporter::getBvhByIndex(i32 index) const
{
	return m_allocatedBvhs[index];
}

i32 CollisionWorldImporter::getNumTriangleInfoMaps() const
{
	return m_allocatedTriangleInfoMaps.size();
}

TriangleInfoMap* CollisionWorldImporter::getTriangleInfoMapByIndex(i32 index) const
{
	return m_allocatedTriangleInfoMaps[index];
}
