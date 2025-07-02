#include "../FractureDynamicsWorld.h"
#include "../FractureBody.h"
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Dispatch/UnionFind.h>

FractureDynamicsWorld::FractureDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, ConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration)
	: DiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration),
	  m_fracturingMode(true)
{
}

void FractureDynamicsWorld::glueCallback()
{
	i32 numManifolds = getDispatcher()->getNumManifolds();

	///first build the islands based on axis aligned bounding box overlap

	UnionFind unionFind;

	i32 index = 0;
	{
		i32 i;
		for (i = 0; i < getCollisionObjectArray().size(); i++)
		{
			CollisionObject2* collisionObject = getCollisionObjectArray()[i];
			//	RigidBody* body = RigidBody::upcast(collisionObject);
			//Adding filtering here
#ifdef STATIC_SIMULATION_ISLAND_OPTIMIZATION
			if (!collisionObject->isStaticOrKinematicObject())
			{
				collisionObject->setIslandTag(index++);
			}
			else
			{
				collisionObject->setIslandTag(-1);
			}
#else
			collisionObject->setIslandTag(i);
			index = i + 1;
#endif
		}
	}

	unionFind.reset(index);

	i32 numElem = unionFind.getNumElements();

	for (i32 i = 0; i < numManifolds; i++)
	{
		PersistentManifold* manifold = getDispatcher()->getManifoldByIndexInternal(i);
		if (!manifold->getNumContacts())
			continue;

		Scalar minDist = 1e30f;
		for (i32 v = 0; v < manifold->getNumContacts(); v++)
		{
			minDist = d3Min(minDist, manifold->getContactPoint(v).getDistance());
		}
		if (minDist > 0.)
			continue;

		CollisionObject2* colObj0 = (CollisionObject2*)manifold->getBody0();
		CollisionObject2* colObj1 = (CollisionObject2*)manifold->getBody1();
		i32 tag0 = (colObj0)->getIslandTag();
		i32 tag1 = (colObj1)->getIslandTag();
		//RigidBody* body0 = RigidBody::upcast(colObj0);
		//RigidBody* body1 = RigidBody::upcast(colObj1);

		if (!colObj0->isStaticOrKinematicObject() && !colObj1->isStaticOrKinematicObject())
		{
			unionFind.unite(tag0, tag1);
		}
	}

	numElem = unionFind.getNumElements();

	index = 0;
	for (i32 ai = 0; ai < getCollisionObjectArray().size(); ai++)
	{
		CollisionObject2* collisionObject = getCollisionObjectArray()[ai];
		if (!collisionObject->isStaticOrKinematicObject())
		{
			i32 tag = unionFind.find(index);

			collisionObject->setIslandTag(tag);

			//Set the correct object offset in Collision Object Array
#if STATIC_SIMULATION_ISLAND_OPTIMIZATION
			unionFind.getElement(index).m_sz = ai;
#endif  //STATIC_SIMULATION_ISLAND_OPTIMIZATION

			index++;
		}
	}
	unionFind.sortIslands();

	i32 endIslandIndex = 1;
	i32 startIslandIndex;

	AlignedObjectArray<CollisionObject2*> removedObjects;

	///iterate over all islands
	for (startIslandIndex = 0; startIslandIndex < numElem; startIslandIndex = endIslandIndex)
	{
		i32 islandId = unionFind.getElement(startIslandIndex).m_id;
		for (endIslandIndex = startIslandIndex + 1; (endIslandIndex < numElem) && (unionFind.getElement(endIslandIndex).m_id == islandId); endIslandIndex++)
		{
		}

		i32 fractureObjectIndex = -1;

		i32 numObjects = 0;

		i32 idx;
		for (idx = startIslandIndex; idx < endIslandIndex; idx++)
		{
			i32 i = unionFind.getElement(idx).m_sz;
			CollisionObject2* colObj0 = getCollisionObjectArray()[i];
			if (colObj0->getInternalType() & CUSTOM_FRACTURE_TYPE)
			{
				fractureObjectIndex = i;
			}
			RigidBody* otherObject = RigidBody::upcast(colObj0);
			if (!otherObject || !otherObject->getInvMass())
				continue;
			numObjects++;
		}

		///Then for each island that contains at least two objects and one fracture object
		if (fractureObjectIndex >= 0 && numObjects > 1)
		{
			FractureBody* fracObj = (FractureBody*)getCollisionObjectArray()[fractureObjectIndex];

			///glueing objects means creating a new compound and removing the old objects
			///delay the removal of old objects to avoid array indexing problems
			removedObjects.push_back(fracObj);
			m_fractureBodies.remove(fracObj);

			AlignedObjectArray<Scalar> massArray;

			AlignedObjectArray<Vec3> oldImpulses;
			AlignedObjectArray<Vec3> oldCenterOfMassesWS;

			oldImpulses.push_back(fracObj->getLinearVelocity() / 1. / fracObj->getInvMass());
			oldCenterOfMassesWS.push_back(fracObj->getCenterOfMassPosition());

			Scalar totalMass = 0.f;

			CompoundShape* compound = new CompoundShape();
			if (fracObj->getCollisionShape()->isCompound())
			{
				Transform2 tr;
				tr.setIdentity();
				CompoundShape* oldCompound = (CompoundShape*)fracObj->getCollisionShape();
				for (i32 c = 0; c < oldCompound->getNumChildShapes(); c++)
				{
					compound->addChildShape(oldCompound->getChildTransform(c), oldCompound->getChildShape(c));
					massArray.push_back(fracObj->m_masses[c]);
					totalMass += fracObj->m_masses[c];
				}
			}
			else
			{
				Transform2 tr;
				tr.setIdentity();
				compound->addChildShape(tr, fracObj->getCollisionShape());
				massArray.push_back(fracObj->m_masses[0]);
				totalMass += fracObj->m_masses[0];
			}

			for (idx = startIslandIndex; idx < endIslandIndex; idx++)
			{
				i32 i = unionFind.getElement(idx).m_sz;

				if (i == fractureObjectIndex)
					continue;

				CollisionObject2* otherCollider = getCollisionObjectArray()[i];

				RigidBody* otherObject = RigidBody::upcast(otherCollider);
				//don't glue/merge with static objects right now, otherwise everything gets stuck to the ground
				///todo: expose this as a callback
				if (!otherObject || !otherObject->getInvMass())
					continue;

				oldImpulses.push_back(otherObject->getLinearVelocity() * (1.f / otherObject->getInvMass()));
				oldCenterOfMassesWS.push_back(otherObject->getCenterOfMassPosition());

				removedObjects.push_back(otherObject);
				m_fractureBodies.remove((FractureBody*)otherObject);

				Scalar curMass = 1.f / otherObject->getInvMass();

				if (otherObject->getCollisionShape()->isCompound())
				{
					Transform2 tr;
					CompoundShape* oldCompound = (CompoundShape*)otherObject->getCollisionShape();
					for (i32 c = 0; c < oldCompound->getNumChildShapes(); c++)
					{
						tr = fracObj->getWorldTransform().inverseTimes(otherObject->getWorldTransform() * oldCompound->getChildTransform(c));
						compound->addChildShape(tr, oldCompound->getChildShape(c));
						massArray.push_back(curMass / (Scalar)oldCompound->getNumChildShapes());
					}
				}
				else
				{
					Transform2 tr;
					tr = fracObj->getWorldTransform().inverseTimes(otherObject->getWorldTransform());
					compound->addChildShape(tr, otherObject->getCollisionShape());
					massArray.push_back(curMass);
				}
				totalMass += curMass;
			}

			Transform2 shift;
			shift.setIdentity();
			CompoundShape* newCompound = FractureBody::shiftTransformDistributeMass(compound, totalMass, shift);
			i32 numChildren = newCompound->getNumChildShapes();
			Assert(numChildren == massArray.size());

			Vec3 localInertia;
			newCompound->calculateLocalInertia(totalMass, localInertia);
			FractureBody* newBody = new FractureBody(totalMass, 0, newCompound, localInertia, &massArray[0], numChildren, this);
			newBody->recomputeConnectivity(this);
			newBody->setWorldTransform(fracObj->getWorldTransform() * shift);

			//now the linear/angular velocity is still zero, apply the impulses

			for (i32 i = 0; i < oldImpulses.size(); i++)
			{
				Vec3 rel_pos = oldCenterOfMassesWS[i] - newBody->getCenterOfMassPosition();
				const Vec3& imp = oldImpulses[i];
				newBody->applyImpulse(imp, rel_pos);
			}

			addRigidBody(newBody);
		}
	}

	//remove the objects from the world at the very end,
	//otherwise the island tags would not match the world collision object array indices anymore
	while (removedObjects.size())
	{
		CollisionObject2* otherCollider = removedObjects[removedObjects.size() - 1];
		removedObjects.pop_back();

		RigidBody* otherObject = RigidBody::upcast(otherCollider);
		if (!otherObject || !otherObject->getInvMass())
			continue;
		removeRigidBody(otherObject);
	}
}

struct FracturePair
{
	FractureBody* m_fracObj;
	AlignedObjectArray<PersistentManifold*> m_contactManifolds;
};

void FractureDynamicsWorld::solveConstraints(ContactSolverInfo& solverInfo)
{
	// todo: after fracture we should run the solver again for better realism
	// for example
	//	save all velocities and if one or more objects fracture:
	//	1) revert all velocties
	//	2) apply impulses for the fracture bodies at the contact locations
	//	3)and run the constaint solver again

	DiscreteDynamicsWorld::solveConstraints(solverInfo);

	fractureCallback();
}

FractureBody* FractureDynamicsWorld::addNewBody(const Transform2& oldTransform, Scalar* masses, CompoundShape* oldCompound)
{
	i32 i;

	Transform2 shift;
	shift.setIdentity();
	Vec3 localInertia;
	CompoundShape* newCompound = FractureBody::shiftTransform(oldCompound, masses, shift, localInertia);
	Scalar totalMass = 0;
	for (i = 0; i < newCompound->getNumChildShapes(); i++)
		totalMass += masses[i];
	//newCompound->calculateLocalInertia(totalMass,localInertia);

	FractureBody* newBody = new FractureBody(totalMass, 0, newCompound, localInertia, masses, newCompound->getNumChildShapes(), this);
	newBody->recomputeConnectivity(this);

	newBody->setCollisionFlags(newBody->getCollisionFlags() | CollisionObject2::CF_CUSTOM_MATERIAL_CALLBACK);
	newBody->setWorldTransform(oldTransform * shift);
	addRigidBody(newBody);
	return newBody;
}

void FractureDynamicsWorld::addRigidBody(RigidBody* body)
{
	if (body->getInternalType() & CUSTOM_FRACTURE_TYPE)
	{
		FractureBody* fbody = (FractureBody*)body;
		m_fractureBodies.push_back(fbody);
	}
	DiscreteDynamicsWorld::addRigidBody(body);
}

void FractureDynamicsWorld::removeRigidBody(RigidBody* body)
{
	if (body->getInternalType() & CUSTOM_FRACTURE_TYPE)
	{
		FractureBody* fbody = (FractureBody*)body;
		AlignedObjectArray<TypedConstraint*> tmpConstraints;

		for (i32 i = 0; i < fbody->getNumConstraintRefs(); i++)
		{
			tmpConstraints.push_back(fbody->getConstraintRef(i));
		}

		//remove all constraints attached to this rigid body too
		for (i32 i = 0; i < tmpConstraints.size(); i++)
			DiscreteDynamicsWorld::removeConstraint(tmpConstraints[i]);

		m_fractureBodies.remove(fbody);
	}

	DiscreteDynamicsWorld::removeRigidBody(body);
}

void FractureDynamicsWorld::breakDisconnectedParts(FractureBody* fracObj)
{
	if (!fracObj->getCollisionShape()->isCompound())
		return;

	CompoundShape* compound = (CompoundShape*)fracObj->getCollisionShape();
	i32 numChildren = compound->getNumChildShapes();

	if (numChildren <= 1)
		return;

	//compute connectivity
	UnionFind unionFind;

	AlignedObjectArray<i32> tags;
	tags.resize(numChildren);
	i32 i, index = 0;
	for (i = 0; i < numChildren; i++)
	{
#ifdef STATIC_SIMULATION_ISLAND_OPTIMIZATION
		tags[i] = index++;
#else
		tags[i] = i;
		index = i + 1;
#endif
	}

	unionFind.reset(index);
	i32 numElem = unionFind.getNumElements();
	for (i = 0; i < fracObj->m_connections.size(); i++)
	{
		Connection& connection = fracObj->m_connections[i];
		if (connection.m_strength > 0.)
		{
			i32 tag0 = tags[connection.m_childIndex0];
			i32 tag1 = tags[connection.m_childIndex1];
			unionFind.unite(tag0, tag1);
		}
	}
	numElem = unionFind.getNumElements();

	index = 0;
	for (i32 ai = 0; ai < numChildren; ai++)
	{
		i32 tag = unionFind.find(index);
		tags[ai] = tag;
		//Set the correct object offset in Collision Object Array
#if STATIC_SIMULATION_ISLAND_OPTIMIZATION
		unionFind.getElement(index).m_sz = ai;
#endif  //STATIC_SIMULATION_ISLAND_OPTIMIZATION
		index++;
	}
	unionFind.sortIslands();

	i32 endIslandIndex = 1;
	i32 startIslandIndex;

	AlignedObjectArray<CollisionObject2*> removedObjects;

	i32 numIslands = 0;

	for (startIslandIndex = 0; startIslandIndex < numElem; startIslandIndex = endIslandIndex)
	{
		i32 islandId = unionFind.getElement(startIslandIndex).m_id;
		for (endIslandIndex = startIslandIndex + 1; (endIslandIndex < numElem) && (unionFind.getElement(endIslandIndex).m_id == islandId); endIslandIndex++)
		{
		}

		//	i32 fractureObjectIndex = -1;

		i32 numShapes = 0;

		CompoundShape* newCompound = new CompoundShape();
		AlignedObjectArray<Scalar> masses;

		i32 idx;
		for (idx = startIslandIndex; idx < endIslandIndex; idx++)
		{
			i32 i = unionFind.getElement(idx).m_sz;
			//		CollisionShape* shape = compound->getChildShape(i);
			newCompound->addChildShape(compound->getChildTransform(i), compound->getChildShape(i));
			masses.push_back(fracObj->m_masses[i]);
			numShapes++;
		}
		if (numShapes)
		{
			FractureBody* newBody = addNewBody(fracObj->getWorldTransform(), &masses[0], newCompound);
			newBody->setLinearVelocity(fracObj->getLinearVelocity());
			newBody->setAngularVelocity(fracObj->getAngularVelocity());

			numIslands++;
		}
	}

	removeRigidBody(fracObj);  //should it also be removed from the array?
}

#include <stdio.h>

void FractureDynamicsWorld::fractureCallback()
{
	AlignedObjectArray<FracturePair> sFracturePairs;

	if (!m_fracturingMode)
	{
		glueCallback();
		return;
	}

	i32 numManifolds = getDispatcher()->getNumManifolds();

	sFracturePairs.clear();

	for (i32 i = 0; i < numManifolds; i++)
	{
		PersistentManifold* manifold = getDispatcher()->getManifoldByIndexInternal(i);
		if (!manifold->getNumContacts())
			continue;

		Scalar totalImpact = 0.f;
		for (i32 p = 0; p < manifold->getNumContacts(); p++)
		{
			totalImpact += manifold->getContactPoint(p).m_appliedImpulse;
		}

		//		printf("totalImpact=%f\n",totalImpact);

		static float maxImpact = 0;
		if (totalImpact > maxImpact)
			maxImpact = totalImpact;

		//some threshold otherwise resting contact would break objects after a while
		if (totalImpact < 40.f)
			continue;

		//		printf("strong impact\n");

		//@todo: add better logic to decide what parts to fracture
		//For example use the idea from the SIGGRAPH talk about the fracture in the movie 2012:
		//
		//Breaking thresholds can be stored as connectivity information between child shapes in the fracture object
		//
		//You can calculate some "impact value" by simulating all the individual child shapes
		//as rigid bodies, without constraints, running it in a separate simulation world
		//(or by running the constraint solver without actually modifying the dynamics world)
		//Then measure some "impact value" using the offset and applied impulse for each child shape
		//weaken the connections based on this "impact value" and only break
		//if this impact value exceeds the breaking threshold.
		//you can propagate the weakening and breaking of connections using the connectivity information

		i32 f0 = m_fractureBodies.findLinearSearch((FractureBody*)manifold->getBody0());
		i32 f1 = m_fractureBodies.findLinearSearch((FractureBody*)manifold->getBody1());

		if (f0 == f1 == m_fractureBodies.size())
			continue;

		if (f0 < m_fractureBodies.size())
		{
			i32 j = f0;

			//		CollisionObject2* colOb = (CollisionObject2*)manifold->getBody1();
			//		RigidBody* otherOb = RigidBody::upcast(colOb);
			//	if (!otherOb->getInvMass())
			//		continue;

			i32 pi = -1;

			for (i32 p = 0; p < sFracturePairs.size(); p++)
			{
				if (sFracturePairs[p].m_fracObj == m_fractureBodies[j])
				{
					pi = p;
					break;
				}
			}

			if (pi < 0)
			{
				FracturePair p;
				p.m_fracObj = m_fractureBodies[j];
				p.m_contactManifolds.push_back(manifold);
				sFracturePairs.push_back(p);
			}
			else
			{
				Assert(sFracturePairs[pi].m_contactManifolds.findLinearSearch(manifold) == sFracturePairs[pi].m_contactManifolds.size());
				sFracturePairs[pi].m_contactManifolds.push_back(manifold);
			}
		}

		if (f1 < m_fractureBodies.size())
		{
			i32 j = f1;
			{
				//CollisionObject2* colOb = (CollisionObject2*)manifold->getBody0();
				//RigidBody* otherOb = RigidBody::upcast(colOb);
				//	if (!otherOb->getInvMass())
				//		continue;

				i32 pi = -1;

				for (i32 p = 0; p < sFracturePairs.size(); p++)
				{
					if (sFracturePairs[p].m_fracObj == m_fractureBodies[j])
					{
						pi = p;
						break;
					}
				}
				if (pi < 0)
				{
					FracturePair p;
					p.m_fracObj = m_fractureBodies[j];
					p.m_contactManifolds.push_back(manifold);
					sFracturePairs.push_back(p);
				}
				else
				{
					Assert(sFracturePairs[pi].m_contactManifolds.findLinearSearch(manifold) == sFracturePairs[pi].m_contactManifolds.size());
					sFracturePairs[pi].m_contactManifolds.push_back(manifold);
				}
			}
		}

		//
	}

	//printf("m_fractureBodies size=%d\n",m_fractureBodies.size());
	//printf("sFracturePairs size=%d\n",sFracturePairs.size());
	if (!sFracturePairs.size())
		return;

	{
		//		printf("fracturing\n");

		for (i32 i = 0; i < sFracturePairs.size(); i++)
		{
			//check impulse/displacement at impact

			//weaken/break connections (and propagate breaking)

			//compute connectivity of connected child shapes

			if (sFracturePairs[i].m_fracObj->getCollisionShape()->isCompound())
			{
				Transform2 tr;
				tr.setIdentity();
				CompoundShape* oldCompound = (CompoundShape*)sFracturePairs[i].m_fracObj->getCollisionShape();
				if (oldCompound->getNumChildShapes() > 1)
				{
					bool needsBreakingCheck = false;

					//weaken/break the connections

					//@todo: propagate along the connection graph
					for (i32 j = 0; j < sFracturePairs[i].m_contactManifolds.size(); j++)
					{
						PersistentManifold* manifold = sFracturePairs[i].m_contactManifolds[j];
						for (i32 k = 0; k < manifold->getNumContacts(); k++)
						{
							ManifoldPoint& pt = manifold->getContactPoint(k);
							if (manifold->getBody0() == sFracturePairs[i].m_fracObj)
							{
								for (i32 f = 0; f < sFracturePairs[i].m_fracObj->m_connections.size(); f++)
								{
									Connection& connection = sFracturePairs[i].m_fracObj->m_connections[f];
									if ((connection.m_childIndex0 == pt.m_index0) ||
										(connection.m_childIndex1 == pt.m_index0))
									{
										connection.m_strength -= pt.m_appliedImpulse;
										if (connection.m_strength < 0)
										{
											//remove or set to zero
											connection.m_strength = 0.f;
											needsBreakingCheck = true;
										}
									}
								}
							}
							else
							{
								for (i32 f = 0; f < sFracturePairs[i].m_fracObj->m_connections.size(); f++)
								{
									Connection& connection = sFracturePairs[i].m_fracObj->m_connections[f];
									if ((connection.m_childIndex0 == pt.m_index1) ||
										(connection.m_childIndex1 == pt.m_index1))
									{
										connection.m_strength -= pt.m_appliedImpulse;
										if (connection.m_strength < 0)
										{
											//remove or set to zero
											connection.m_strength = 0.f;
											needsBreakingCheck = true;
										}
									}
								}
							}
						}
					}

					if (needsBreakingCheck)
					{
						breakDisconnectedParts(sFracturePairs[i].m_fracObj);
					}
				}
			}
		}
	}

	sFracturePairs.clear();
}
