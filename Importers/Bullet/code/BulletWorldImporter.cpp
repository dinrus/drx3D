#include <drx3D/Importers/Bullet/BulletWorldImporter.h>
#include <drx3D/Importers/BulletFile.h>

#include <drx3D/DynamicsCommon.h>
#ifndef USE_GIMPACT
#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>
#endif

//#define USE_INTERNAL_EDGE_UTILITY
#ifdef USE_INTERNAL_EDGE_UTILITY
#include <drx3D/Physics/Collision/Dispatch/InternalEdgeUtility.h>
#endif  //USE_INTERNAL_EDGE_UTILITY

BulletWorldImporter::BulletWorldImporter(DynamicsWorld* world)
	: WorldImporter(world)
{
}

BulletWorldImporter::~BulletWorldImporter()
{
}

bool BulletWorldImporter::loadFile(tukk fileName, tukk preSwapFilenameOut)
{
	bParse::BulletFile* bulletFile2 = new bParse::BulletFile(fileName);

	bool result = loadFileFromMemory(bulletFile2);
	//now you could save the file in 'native' format using
	//bulletFile2->writeFile("native.bullet");
	if (result)
	{
		if (preSwapFilenameOut)
		{
			bulletFile2->preSwap();
			bulletFile2->writeFile(preSwapFilenameOut);
		}
	}
	delete bulletFile2;

	return result;
}

bool BulletWorldImporter::loadFileFromMemory(tuk memoryBuffer, i32 len)
{
	bParse::BulletFile* bulletFile2 = new bParse::BulletFile(memoryBuffer, len);

	bool result = loadFileFromMemory(bulletFile2);

	delete bulletFile2;

	return result;
}

bool BulletWorldImporter::loadFileFromMemory(bParse::BulletFile* bulletFile2)
{
	bool ok = (bulletFile2->getFlags() & bParse::FD_OK) != 0;

	if (ok)
		bulletFile2->parse(m_verboseMode);
	else
		return false;

	if (m_verboseMode & bParse::FD_VERBOSE_DUMP_CHUNKS)
	{
		bulletFile2->dumpChunks(bulletFile2->getFileDNA());
	}

	return convertAllObjects(bulletFile2);
}

bool BulletWorldImporter::convertAllObjects(bParse::BulletFile* bulletFile2)
{
	m_shapeMap.clear();
	m_bodyMap.clear();

	i32 i;

	for (i = 0; i < bulletFile2->m_bvhs.size(); i++)
	{
		OptimizedBvh* bvh = createOptimizedBvh();

		if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
		{
			QuantizedBvhDoubleData* bvhData = (QuantizedBvhDoubleData*)bulletFile2->m_bvhs[i];
			bvh->deSerializeDouble(*bvhData);
		}
		else
		{
			QuantizedBvhFloatData* bvhData = (QuantizedBvhFloatData*)bulletFile2->m_bvhs[i];
			bvh->deSerializeFloat(*bvhData);
		}
		m_bvhMap.insert(bulletFile2->m_bvhs[i], bvh);
	}

	for (i = 0; i < bulletFile2->m_collisionShapes.size(); i++)
	{
		CollisionShapeData* shapeData = (CollisionShapeData*)bulletFile2->m_collisionShapes[i];
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

	for (i32 i = 0; i < bulletFile2->m_dynamicsWorldInfo.size(); i++)
	{
		if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
		{
			DynamicsWorldDoubleData* solverInfoData = (DynamicsWorldDoubleData*)bulletFile2->m_dynamicsWorldInfo[i];
			ContactSolverInfo solverInfo;

			Vec3 gravity;
			gravity.deSerializeDouble(solverInfoData->m_gravity);

			solverInfo.m_tau = Scalar(solverInfoData->m_solverInfo.m_tau);
			solverInfo.m_damping = Scalar(solverInfoData->m_solverInfo.m_damping);
			solverInfo.m_friction = Scalar(solverInfoData->m_solverInfo.m_friction);
			solverInfo.m_timeStep = Scalar(solverInfoData->m_solverInfo.m_timeStep);

			solverInfo.m_restitution = Scalar(solverInfoData->m_solverInfo.m_restitution);
			solverInfo.m_maxErrorReduction = Scalar(solverInfoData->m_solverInfo.m_maxErrorReduction);
			solverInfo.m_sor = Scalar(solverInfoData->m_solverInfo.m_sor);
			solverInfo.m_erp = Scalar(solverInfoData->m_solverInfo.m_erp);

			solverInfo.m_erp2 = Scalar(solverInfoData->m_solverInfo.m_erp2);
			solverInfo.m_globalCfm = Scalar(solverInfoData->m_solverInfo.m_globalCfm);
			solverInfo.m_splitImpulsePenetrationThreshold = Scalar(solverInfoData->m_solverInfo.m_splitImpulsePenetrationThreshold);
			solverInfo.m_splitImpulseTurnErp = Scalar(solverInfoData->m_solverInfo.m_splitImpulseTurnErp);

			solverInfo.m_linearSlop = Scalar(solverInfoData->m_solverInfo.m_linearSlop);
			solverInfo.m_warmstartingFactor = Scalar(solverInfoData->m_solverInfo.m_warmstartingFactor);
			solverInfo.m_maxGyroscopicForce = Scalar(solverInfoData->m_solverInfo.m_maxGyroscopicForce);
			solverInfo.m_singleAxisRollingFrictionThreshold = Scalar(solverInfoData->m_solverInfo.m_singleAxisRollingFrictionThreshold);

			solverInfo.m_numIterations = solverInfoData->m_solverInfo.m_numIterations;
			solverInfo.m_solverMode = solverInfoData->m_solverInfo.m_solverMode;
			solverInfo.m_restingContactRestitutionThreshold = solverInfoData->m_solverInfo.m_restingContactRestitutionThreshold;
			solverInfo.m_minimumSolverBatchSize = solverInfoData->m_solverInfo.m_minimumSolverBatchSize;

			solverInfo.m_splitImpulse = solverInfoData->m_solverInfo.m_splitImpulse;

			setDynamicsWorldInfo(gravity, solverInfo);
		}
		else
		{
			DynamicsWorldFloatData* solverInfoData = (DynamicsWorldFloatData*)bulletFile2->m_dynamicsWorldInfo[i];
			ContactSolverInfo solverInfo;

			Vec3 gravity;
			gravity.deSerializeFloat(solverInfoData->m_gravity);

			solverInfo.m_tau = solverInfoData->m_solverInfo.m_tau;
			solverInfo.m_damping = solverInfoData->m_solverInfo.m_damping;
			solverInfo.m_friction = solverInfoData->m_solverInfo.m_friction;
			solverInfo.m_timeStep = solverInfoData->m_solverInfo.m_timeStep;

			solverInfo.m_restitution = solverInfoData->m_solverInfo.m_restitution;
			solverInfo.m_maxErrorReduction = solverInfoData->m_solverInfo.m_maxErrorReduction;
			solverInfo.m_sor = solverInfoData->m_solverInfo.m_sor;
			solverInfo.m_erp = solverInfoData->m_solverInfo.m_erp;

			solverInfo.m_erp2 = solverInfoData->m_solverInfo.m_erp2;
			solverInfo.m_globalCfm = solverInfoData->m_solverInfo.m_globalCfm;
			solverInfo.m_splitImpulsePenetrationThreshold = solverInfoData->m_solverInfo.m_splitImpulsePenetrationThreshold;
			solverInfo.m_splitImpulseTurnErp = solverInfoData->m_solverInfo.m_splitImpulseTurnErp;

			solverInfo.m_linearSlop = solverInfoData->m_solverInfo.m_linearSlop;
			solverInfo.m_warmstartingFactor = solverInfoData->m_solverInfo.m_warmstartingFactor;
			solverInfo.m_maxGyroscopicForce = solverInfoData->m_solverInfo.m_maxGyroscopicForce;
			solverInfo.m_singleAxisRollingFrictionThreshold = solverInfoData->m_solverInfo.m_singleAxisRollingFrictionThreshold;

			solverInfo.m_numIterations = solverInfoData->m_solverInfo.m_numIterations;
			solverInfo.m_solverMode = solverInfoData->m_solverInfo.m_solverMode;
			solverInfo.m_restingContactRestitutionThreshold = solverInfoData->m_solverInfo.m_restingContactRestitutionThreshold;
			solverInfo.m_minimumSolverBatchSize = solverInfoData->m_solverInfo.m_minimumSolverBatchSize;

			solverInfo.m_splitImpulse = solverInfoData->m_solverInfo.m_splitImpulse;

			setDynamicsWorldInfo(gravity, solverInfo);
		}
	}

	for (i = 0; i < bulletFile2->m_rigidBodies.size(); i++)
	{
		if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
		{
			RigidBodyDoubleData* colObjData = (RigidBodyDoubleData*)bulletFile2->m_rigidBodies[i];
			convertRigidBodyDouble(colObjData);
		}
		else
		{
			RigidBodyFloatData* colObjData = (RigidBodyFloatData*)bulletFile2->m_rigidBodies[i];
			convertRigidBodyFloat(colObjData);
		}
	}

	for (i = 0; i < bulletFile2->m_collisionObjects.size(); i++)
	{
		if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
		{
			CollisionObject2DoubleData* colObjData = (CollisionObject2DoubleData*)bulletFile2->m_collisionObjects[i];
			CollisionShape** shapePtr = m_shapeMap.find(colObjData->m_collisionShape);
			if (shapePtr && *shapePtr)
			{
				Transform2 startTransform;
				colObjData->m_worldTransform.m_origin.m_floats[3] = 0.f;
				startTransform.deSerializeDouble(colObjData->m_worldTransform);

				CollisionShape* shape = (CollisionShape*)*shapePtr;
				CollisionObject2* body = createCollisionObject(startTransform, shape, colObjData->m_name);
				body->setFriction(Scalar(colObjData->m_friction));
				body->setRestitution(Scalar(colObjData->m_restitution));

#ifdef USE_INTERNAL_EDGE_UTILITY
				if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
				{
					BvhTriangleMeshShape* trimesh = (BvhTriangleMeshShape*)shape;
					if (trimesh->getTriangleInfoMap())
					{
						body->setCollisionFlags(body->getCollisionFlags() | CollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
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
		else
		{
			CollisionObject2FloatData* colObjData = (CollisionObject2FloatData*)bulletFile2->m_collisionObjects[i];
			CollisionShape** shapePtr = m_shapeMap.find(colObjData->m_collisionShape);
			if (shapePtr && *shapePtr)
			{
				Transform2 startTransform;
				colObjData->m_worldTransform.m_origin.m_floats[3] = 0.f;
				startTransform.deSerializeFloat(colObjData->m_worldTransform);

				CollisionShape* shape = (CollisionShape*)*shapePtr;
				CollisionObject2* body = createCollisionObject(startTransform, shape, colObjData->m_name);

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
	}

	for (i = 0; i < bulletFile2->m_constraints.size(); i++)
	{
		TypedConstraintData2* constraintData = (TypedConstraintData2*)bulletFile2->m_constraints[i];

		CollisionObject2** colAptr = m_bodyMap.find(constraintData->m_rbA);
		CollisionObject2** colBptr = m_bodyMap.find(constraintData->m_rbB);

		RigidBody* rbA = 0;
		RigidBody* rbB = 0;

		if (colAptr)
		{
			rbA = RigidBody::upcast(*colAptr);
			if (!rbA)
				rbA = &getFixedBody();
		}
		if (colBptr)
		{
			rbB = RigidBody::upcast(*colBptr);
			if (!rbB)
				rbB = &getFixedBody();
		}
		if (!rbA && !rbB)
			continue;

		bool isDoublePrecisionData = (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION) != 0;

		if (isDoublePrecisionData)
		{
			if (bulletFile2->getVersion() >= 282)
			{
				TypedConstraintDoubleData* dc = (TypedConstraintDoubleData*)constraintData;
				convertConstraintDouble(dc, rbA, rbB, bulletFile2->getVersion());
			}
			else
			{
				//double-precision constraints were messed up until 2.82, try to recover data...

				TypedConstraintData* oldData = (TypedConstraintData*)constraintData;

				convertConstraintBackwardsCompatible281(oldData, rbA, rbB, bulletFile2->getVersion());
			}
		}
		else
		{
			TypedConstraintFloatData* dc = (TypedConstraintFloatData*)constraintData;
			convertConstraintFloat(dc, rbA, rbB, bulletFile2->getVersion());
		}
	}

	return true;
}
