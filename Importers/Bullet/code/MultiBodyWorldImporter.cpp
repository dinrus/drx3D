#include <drx3D/Importers/Bullet/MultiBodyWorldImporter.h>
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Importers/BulletFile.h>
#include <drx3D/Importers/Bullet/WorldImporter.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>

struct MultiBodyWorldImporterInternalData
{
	MultiBodyDynamicsWorld* m_mbDynamicsWorld;
	HashMap<HashPtr, MultiBody*> m_mbMap;
};

MultiBodyWorldImporter::MultiBodyWorldImporter(MultiBodyDynamicsWorld* world)
	: BulletWorldImporter(world)
{
	m_data = new MultiBodyWorldImporterInternalData;
	m_data->m_mbDynamicsWorld = world;
}

MultiBodyWorldImporter::~MultiBodyWorldImporter()
{
	delete m_data;
}

void MultiBodyWorldImporter::deleteAllData()
{
	BulletWorldImporter::deleteAllData();
}

static CollisionObject2DoubleData* getBody0FromContactManifold(PersistentManifoldDoubleData* manifold)
{
	return (CollisionObject2DoubleData*)manifold->m_body0;
}
static CollisionObject2DoubleData* getBody1FromContactManifold(PersistentManifoldDoubleData* manifold)
{
	return (CollisionObject2DoubleData*)manifold->m_body1;
}
static CollisionObject2FloatData* getBody0FromContactManifold(PersistentManifoldFloatData* manifold)
{
	return (CollisionObject2FloatData*)manifold->m_body0;
}
static CollisionObject2FloatData* getBody1FromContactManifold(PersistentManifoldFloatData* manifold)
{
	return (CollisionObject2FloatData*)manifold->m_body1;
}

template <class T>
void syncContactManifolds(T** contactManifolds, i32 numContactManifolds, MultiBodyWorldImporterInternalData* m_data)
{
	m_data->m_mbDynamicsWorld->updateAabbs();
	m_data->m_mbDynamicsWorld->computeOverlappingPairs();
	Dispatcher* dispatcher = m_data->m_mbDynamicsWorld->getDispatcher();

	DispatcherInfo& dispatchInfo = m_data->m_mbDynamicsWorld->getDispatchInfo();

	if (dispatcher)
	{
		OverlappingPairCache* pairCache = m_data->m_mbDynamicsWorld->getBroadphase()->getOverlappingPairCache();
		if (dispatcher)
		{
			dispatcher->dispatchAllCollisionPairs(pairCache, dispatchInfo, dispatcher);
		}
		i32 numExistingManifolds = m_data->m_mbDynamicsWorld->getDispatcher()->getNumManifolds();
		ManifoldArray manifoldArray;
		for (i32 i = 0; i < pairCache->getNumOverlappingPairs(); i++)
		{
			BroadphasePair& pair = pairCache->getOverlappingPairArray()[i];
			if (pair.m_algorithm)
			{
				pair.m_algorithm->getAllContactManifolds(manifoldArray);
				//for each existing manifold, search a matching manifoldData and reconstruct
				for (i32 m = 0; m < manifoldArray.size(); m++)
				{
					PersistentManifold* existingManifold = manifoldArray[m];
					i32 uid0 = existingManifold->getBody0()->getBroadphaseHandle()->m_uniqueId;
					i32 uid1 = existingManifold->getBody1()->getBroadphaseHandle()->m_uniqueId;
					i32 matchingManifoldIndex = -1;
					for (i32 q = 0; q < numContactManifolds; q++)
					{
						if (uid0 == getBody0FromContactManifold(contactManifolds[q])->m_uniqueId && uid1 == getBody1FromContactManifold(contactManifolds[q])->m_uniqueId)
						{
							matchingManifoldIndex = q;
						}
					}
					if (matchingManifoldIndex >= 0)
					{
						existingManifold->deSerialize(contactManifolds[matchingManifoldIndex]);
					}
					else
					{
						existingManifold->setNumContacts(0);
						//printf("Issue: cannot find maching contact manifold (%d, %d), may cause issues in determinism.\n", uid0, uid1);
					}

					manifoldArray.clear();
				}
			}
		}
	}
}

template <class T>
void syncMultiBody(T* mbd, MultiBody* mb, MultiBodyWorldImporterInternalData* m_data, AlignedObjectArray<Quat>& scratchQ, AlignedObjectArray<Vec3>& scratchM)
{
	bool isFixedBase = mbd->m_baseMass == 0;
	bool canSleep = false;
	Vec3 baseInertia;
	baseInertia.deSerialize(mbd->m_baseInertia);

	Vec3 baseWorldPos;
	baseWorldPos.deSerialize(mbd->m_baseWorldPosition);
	mb->setBasePos(baseWorldPos);
	Quat baseWorldRot;
	baseWorldRot.deSerialize(mbd->m_baseWorldOrientation);
	mb->setWorldToBaseRot(baseWorldRot.inverse());
	Vec3 baseLinVal;
	baseLinVal.deSerialize(mbd->m_baseLinearVelocity);
	Vec3 baseAngVel;
	baseAngVel.deSerialize(mbd->m_baseAngularVelocity);
	mb->setBaseVel(baseLinVal);
	mb->setBaseOmega(baseAngVel);

	for (i32 i = 0; i < mbd->m_numLinks; i++)
	{
		mb->getLink(i).m_absFrameTotVelocity.m_topVec.deSerialize(mbd->m_links[i].m_absFrameTotVelocityTop);
		mb->getLink(i).m_absFrameTotVelocity.m_bottomVec.deSerialize(mbd->m_links[i].m_absFrameTotVelocityBottom);
		mb->getLink(i).m_absFrameLocVelocity.m_topVec.deSerialize(mbd->m_links[i].m_absFrameLocVelocityTop);
		mb->getLink(i).m_absFrameLocVelocity.m_bottomVec.deSerialize(mbd->m_links[i].m_absFrameLocVelocityBottom);

		switch (mbd->m_links[i].m_jointType)
		{
			case MultibodyLink::eFixed:
			{
				break;
			}
			case MultibodyLink::ePrismatic:
			{
				mb->setJointPos(i, mbd->m_links[i].m_jointPos[0]);
				mb->setJointVel(i, mbd->m_links[i].m_jointVel[0]);
				break;
			}
			case MultibodyLink::eRevolute:
			{
				mb->setJointPos(i, mbd->m_links[i].m_jointPos[0]);
				mb->setJointVel(i, mbd->m_links[i].m_jointVel[0]);
				break;
			}
			case MultibodyLink::eSpherical:
			{
				Scalar jointPos[4] = {(Scalar)mbd->m_links[i].m_jointPos[0], (Scalar)mbd->m_links[i].m_jointPos[1], (Scalar)mbd->m_links[i].m_jointPos[2], (Scalar)mbd->m_links[i].m_jointPos[3]};
				Scalar jointVel[3] = {(Scalar)mbd->m_links[i].m_jointVel[0], (Scalar)mbd->m_links[i].m_jointVel[1], (Scalar)mbd->m_links[i].m_jointVel[2]};
				mb->setJointPosMultiDof(i, jointPos);
				mb->setJointVelMultiDof(i, jointVel);

				break;
			}
			case MultibodyLink::ePlanar:
			{
				break;
			}
			default:
			{
			}
		}
	}
	mb->forwardKinematics(scratchQ, scratchM);
	mb->updateCollisionObjectWorldTransforms(scratchQ, scratchM);
}

template <class T>
void convertMultiBody(T* mbd, MultiBodyWorldImporterInternalData* m_data)
{
	bool isFixedBase = mbd->m_baseMass == 0;
	bool canSleep = false;
	Vec3 baseInertia;
	baseInertia.deSerialize(mbd->m_baseInertia);
	MultiBody* mb = new MultiBody(mbd->m_numLinks, mbd->m_baseMass, baseInertia, isFixedBase, canSleep);
	mb->setHasSelfCollision(false);

	Vec3 baseWorldPos;
	baseWorldPos.deSerialize(mbd->m_baseWorldPosition);

	Quat baseWorldOrn;
	baseWorldOrn.deSerialize(mbd->m_baseWorldOrientation);
	mb->setBasePos(baseWorldPos);
	mb->setWorldToBaseRot(baseWorldOrn.inverse());

	m_data->m_mbMap.insert(mbd, mb);
	for (i32 i = 0; i < mbd->m_numLinks; i++)
	{
		Vec3 localInertiaDiagonal;
		localInertiaDiagonal.deSerialize(mbd->m_links[i].m_linkInertia);
		Quat parentRotToThis;
		parentRotToThis.deSerialize(mbd->m_links[i].m_zeroRotParentToThis);
		Vec3 parentComToThisPivotOffset;
		parentComToThisPivotOffset.deSerialize(mbd->m_links[i].m_parentComToThisPivotOffset);
		Vec3 thisPivotToThisComOffset;
		thisPivotToThisComOffset.deSerialize(mbd->m_links[i].m_thisPivotToThisComOffset);

		switch (mbd->m_links[i].m_jointType)
		{
			case MultibodyLink::eFixed:
			{
				mb->setupFixed(i, mbd->m_links[i].m_linkMass, localInertiaDiagonal, mbd->m_links[i].m_parentIndex,
							   parentRotToThis, parentComToThisPivotOffset, thisPivotToThisComOffset);
				//search for the collider
				//mbd->m_links[i].m_linkCollider
				break;
			}
			case MultibodyLink::ePrismatic:
			{
				Vec3 jointAxis;
				jointAxis.deSerialize(mbd->m_links[i].m_jointAxisBottom[0]);
				bool disableParentCollision = true;  //todo
				mb->setupPrismatic(i, mbd->m_links[i].m_linkMass, localInertiaDiagonal, mbd->m_links[i].m_parentIndex,
								   parentRotToThis, jointAxis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
				mb->setJointPos(i, mbd->m_links[i].m_jointPos[0]);
				mb->finalizeMultiDof();
				mb->setJointVel(i, mbd->m_links[i].m_jointVel[0]);
				break;
			}
			case MultibodyLink::eRevolute:
			{
				Vec3 jointAxis;
				jointAxis.deSerialize(mbd->m_links[i].m_jointAxisTop[0]);
				bool disableParentCollision = true;  //todo
				mb->setupRevolute(i, mbd->m_links[i].m_linkMass, localInertiaDiagonal, mbd->m_links[i].m_parentIndex,
								  parentRotToThis, jointAxis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
				mb->setJointPos(i, mbd->m_links[i].m_jointPos[0]);
				mb->finalizeMultiDof();
				mb->setJointVel(i, mbd->m_links[i].m_jointVel[0]);
				break;
			}
			case MultibodyLink::eSpherical:
			{
				Assert(0);
				bool disableParentCollision = true;  //todo
				mb->setupSpherical(i, mbd->m_links[i].m_linkMass, localInertiaDiagonal, mbd->m_links[i].m_parentIndex,
								   parentRotToThis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
				Scalar jointPos[4] = {(Scalar)mbd->m_links[i].m_jointPos[0], (Scalar)mbd->m_links[i].m_jointPos[1], (Scalar)mbd->m_links[i].m_jointPos[2], (Scalar)mbd->m_links[i].m_jointPos[3]};
				Scalar jointVel[3] = {(Scalar)mbd->m_links[i].m_jointVel[0], (Scalar)mbd->m_links[i].m_jointVel[1], (Scalar)mbd->m_links[i].m_jointVel[2]};
				mb->setJointPosMultiDof(i, jointPos);
				mb->finalizeMultiDof();
				mb->setJointVelMultiDof(i, jointVel);

				break;
			}
			case MultibodyLink::ePlanar:
			{
				Assert(0);
				break;
			}
			default:
			{
				Assert(0);
			}
		}
	}
}

bool MultiBodyWorldImporter::convertAllObjects(bParse::BulletFile* bulletFile2)
{
	bool result = false;
	AlignedObjectArray<Quat> scratchQ;
	AlignedObjectArray<Vec3> scratchM;

	if (m_importerFlags & eRESTORE_EXISTING_OBJECTS)
	{
		//check if the snapshot is valid for the existing world
		//equal number of objects, # links etc
		if ((bulletFile2->m_multiBodies.size() != m_data->m_mbDynamicsWorld->getNumMultibodies()))
		{
			printf("MultiBodyWorldImporter::convertAllObjects ошибка: expected %d multibodies, got %d.\n", m_data->m_mbDynamicsWorld->getNumMultibodies(), bulletFile2->m_multiBodies.size());
			result = false;
			return result;
		}
		result = true;

		//convert all multibodies
		if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
		{
			//for (i32 i = 0; i < bulletFile2->m_multiBodies.size(); i++)
			for (i32 i = bulletFile2->m_multiBodies.size() - 1; i >= 0; i--)
			{
				MultiBodyDoubleData* mbd = (MultiBodyDoubleData*)bulletFile2->m_multiBodies[i];
				MultiBody* mb = m_data->m_mbDynamicsWorld->getMultiBody(i);
				if (mbd->m_numLinks != mb->getNumLinks())
				{
					printf("MultiBodyWorldImporter::convertAllObjects ошибка: mismatch in number of links in a body (expected %d, found %d).\n", mbd->m_numLinks, mb->getNumLinks() );
					result = false;
					return result;
				} else
				{
					syncMultiBody(mbd, mb, m_data, scratchQ, scratchM);
				}
			}

			for (i32 i = bulletFile2->m_rigidBodies.size() - 1; i >= 0; i--)
			{
				RigidBodyDoubleData* rbd = (RigidBodyDoubleData*)bulletFile2->m_rigidBodies[i];
				i32 foundRb = -1;
				i32 uid = rbd->m_collisionObjectData.m_uniqueId;
				for (i32 i = 0; i < m_data->m_mbDynamicsWorld->getNumCollisionObjects(); i++)
				{
					if (uid == m_data->m_mbDynamicsWorld->getCollisionObjectArray()[i]->getBroadphaseHandle()->m_uniqueId)
					{
						foundRb = i;
						break;
					}
				}
				if (foundRb >= 0)
				{
					RigidBody* rb = RigidBody::upcast(m_data->m_mbDynamicsWorld->getCollisionObjectArray()[foundRb]);
					if (rb)
					{
						Transform2 tr;
						tr.deSerializeDouble(rbd->m_collisionObjectData.m_worldTransform);
						rb->setWorldTransform(tr);
						Vec3 linVel, angVel;
						linVel.deSerializeDouble(rbd->m_linearVelocity);
						angVel.deSerializeDouble(rbd->m_angularVelocity);
						rb->setLinearVelocity(linVel);
						rb->setAngularVelocity(angVel);
					}
					else
					{
						printf("MultiBodyWorldImporter::convertAllObjects ошибка: cannot find btRigidBody with bodyUniqueId %d\n", uid);
						result = false;
					}
				}
				else
				{
					printf("Error in btMultiBodyWorldImporter::convertAllObjects: didn't find bodyUniqueId: %d\n", uid);
					result = false;
				}
			}

			//todo: check why body1 pointer is not properly deserialized
			for (i32 i = 0; i < bulletFile2->m_contactManifolds.size(); i++)
			{
				PersistentManifoldDoubleData* manifoldData = (PersistentManifoldDoubleData*)bulletFile2->m_contactManifolds[i];
				{
					uk ptr = bulletFile2->findLibPointer(manifoldData->m_body0);
					if (ptr)
					{
						manifoldData->m_body0 = (CollisionObject2DoubleData*)ptr;
					}
				}

				{
					uk ptr = bulletFile2->findLibPointer(manifoldData->m_body1);
					if (ptr)
					{
						manifoldData->m_body1 = (CollisionObject2DoubleData*)ptr;
					}
				}
			}

			if (bulletFile2->m_contactManifolds.size())
			{
				syncContactManifolds((PersistentManifoldDoubleData**)&bulletFile2->m_contactManifolds[0], bulletFile2->m_contactManifolds.size(), m_data);
			}
		}
		else
		{
			//single precision version
			//for (i32 i = 0; i < bulletFile2->m_multiBodies.size(); i++)
			for (i32 i = bulletFile2->m_multiBodies.size() - 1; i >= 0; i--)
			{
				MultiBodyFloatData* mbd = (MultiBodyFloatData*)bulletFile2->m_multiBodies[i];
				MultiBody* mb = m_data->m_mbDynamicsWorld->getMultiBody(i);
				if (mbd->m_numLinks != mb->getNumLinks())
				{
					printf("MultiBodyWorldImporter::convertAllObjects ошибка: mismatch in number of links in a body (expected %d, found %d).\n", mbd->m_numLinks, mb->getNumLinks() );
					result = false;
					return result;
				} else
				{
					syncMultiBody(mbd, mb, m_data, scratchQ, scratchM);
				}
			}

			for (i32 i = bulletFile2->m_rigidBodies.size() - 1; i >= 0; i--)
			{
				RigidBodyFloatData* rbd = (RigidBodyFloatData*)bulletFile2->m_rigidBodies[i];
				i32 foundRb = -1;
				i32 uid = rbd->m_collisionObjectData.m_uniqueId;
				for (i32 i = 0; i < m_data->m_mbDynamicsWorld->getNumCollisionObjects(); i++)
				{
					if (uid == m_data->m_mbDynamicsWorld->getCollisionObjectArray()[i]->getBroadphaseHandle()->m_uniqueId)
					{
						foundRb = i;
						break;
					}
				}
				if (foundRb >= 0)
				{
					RigidBody* rb = RigidBody::upcast(m_data->m_mbDynamicsWorld->getCollisionObjectArray()[foundRb]);
					if (rb)
					{
						Transform2 tr;
						tr.deSerializeFloat(rbd->m_collisionObjectData.m_worldTransform);
						rb->setWorldTransform(tr);
						Vec3 linVel, angVel;
						linVel.deSerializeFloat(rbd->m_linearVelocity);
						angVel.deSerializeFloat(rbd->m_angularVelocity);
						rb->setLinearVelocity(linVel);
						rb->setAngularVelocity(angVel);
					}
					else
					{
						printf("MultiBodyWorldImporter::convertAllObjects ошибка: cannot find btRigidBody with bodyUniqueId %d\n", uid);
						result = false;
					}
				}
				else
				{
					printf("Error in btMultiBodyWorldImporter::convertAllObjects: didn't find bodyUniqueId: %d\n", uid);
					result = false;
				}
			}

			//todo: check why body1 pointer is not properly deserialized
			for (i32 i = 0; i < bulletFile2->m_contactManifolds.size(); i++)
			{
				PersistentManifoldFloatData* manifoldData = (PersistentManifoldFloatData*)bulletFile2->m_contactManifolds[i];
				{
					uk ptr = bulletFile2->findLibPointer(manifoldData->m_body0);
					if (ptr)
					{
						manifoldData->m_body0 = (CollisionObject2FloatData*)ptr;
					}
				}
				{
					uk ptr = bulletFile2->findLibPointer(manifoldData->m_body1);
					if (ptr)
					{
						manifoldData->m_body1 = (CollisionObject2FloatData*)ptr;
					}
				}
			}

			if (bulletFile2->m_contactManifolds.size())
			{
				syncContactManifolds((PersistentManifoldFloatData**)&bulletFile2->m_contactManifolds[0], bulletFile2->m_contactManifolds.size(), m_data);
			}
		}
	}
	else
	{
		result = BulletWorldImporter::convertAllObjects(bulletFile2);

		//convert all multibodies
		for (i32 i = 0; i < bulletFile2->m_multiBodies.size(); i++)
		{
			if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
			{
				MultiBodyDoubleData* mbd = (MultiBodyDoubleData*)bulletFile2->m_multiBodies[i];
				convertMultiBody(mbd, m_data);
			}
			else
			{
				MultiBodyFloatData* mbd = (MultiBodyFloatData*)bulletFile2->m_multiBodies[i];
				convertMultiBody(mbd, m_data);
			}
		}

		//forward kinematics, so that the link world transforms are valid, for collision detection
		for (i32 i = 0; i < m_data->m_mbMap.size(); i++)
		{
			MultiBody** ptr = m_data->m_mbMap.getAtIndex(i);
			if (ptr)
			{
				MultiBody* mb = *ptr;
				mb->finalizeMultiDof();
				Vec3 linvel = mb->getBaseVel();
				Vec3 angvel = mb->getBaseOmega();
				mb->forwardKinematics(scratchQ, scratchM);
			}
		}

		//convert all multibody link colliders
		for (i32 i = 0; i < bulletFile2->m_multiBodyLinkColliders.size(); i++)
		{
			if (bulletFile2->getFlags() & bParse::FD_DOUBLE_PRECISION)
			{
				MultiBodyLinkColliderDoubleData* mblcd = (MultiBodyLinkColliderDoubleData*)bulletFile2->m_multiBodyLinkColliders[i];

				MultiBody** ptr = m_data->m_mbMap[mblcd->m_multiBody];
				if (ptr)
				{
					MultiBody* multiBody = *ptr;

					CollisionShape** shapePtr = m_shapeMap.find(mblcd->m_colObjData.m_collisionShape);
					if (shapePtr && *shapePtr)
					{
						Transform2 startTransform;
						mblcd->m_colObjData.m_worldTransform.m_origin.m_floats[3] = 0.f;
						startTransform.deSerializeDouble(mblcd->m_colObjData.m_worldTransform);

						CollisionShape* shape = (CollisionShape*)*shapePtr;
						if (shape)
						{
							MultiBodyLinkCollider* col = new MultiBodyLinkCollider(multiBody, mblcd->m_link);
							col->setCollisionShape(shape);
							//CollisionObject2* body = createCollisionObject(startTransform,shape,mblcd->m_colObjData.m_name);
							col->setFriction(Scalar(mblcd->m_colObjData.m_friction));
							col->setRestitution(Scalar(mblcd->m_colObjData.m_restitution));
							//m_bodyMap.insert(colObjData,body);
							if (mblcd->m_link == -1)
							{
								col->setWorldTransform(multiBody->getBaseWorldTransform());
								multiBody->setBaseCollider(col);
							}
							else
							{
								col->setWorldTransform(multiBody->getLink(mblcd->m_link).m_cachedWorldTransform);
								multiBody->getLink(mblcd->m_link).m_collider = col;
							}
							i32 mbLinkIndex = mblcd->m_link;

							bool isDynamic = (mbLinkIndex < 0 && multiBody->hasFixedBase()) ? false : true;
							i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
							i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

#if 0
							i32 colGroup = 0, colMask = 0;
							i32 collisionFlags = mblcd->m_colObjData.m_collisionFlags;
							if (collisionFlags & URDF_HAS_COLLISION_GROUP)
							{
								collisionFilterGroup = colGroup;
							}
							if (collisionFlags & URDF_HAS_COLLISION_MASK)
							{
								collisionFilterMask = colMask;
							}
#endif
							m_data->m_mbDynamicsWorld->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);
						}
					}
					else
					{
						printf("ошибка: no shape found\n");
					}
#if 0
					//base and fixed? -> static, otherwise flag as dynamic

					world1->addCollisionObject(col, collisionFilterGroup, collisionFilterMask);
#endif
				}
			}
		}

		for (i32 i = 0; i < m_data->m_mbMap.size(); i++)
		{
			MultiBody** ptr = m_data->m_mbMap.getAtIndex(i);
			if (ptr)
			{
				MultiBody* mb = *ptr;
				mb->finalizeMultiDof();

				m_data->m_mbDynamicsWorld->addMultiBody(mb);
			}
		}
	}
	return result;
}