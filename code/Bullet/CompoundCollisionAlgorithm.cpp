#include <drx3D/Physics/Collision/Dispatch/CompoundCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

ShapePairCallback gCompoundChildShapePairCallback = 0;

CompoundCollisionAlgorithm::CompoundCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_isSwapped(isSwapped),
	  m_sharedManifold(ci.m_manifold)
{
	m_ownsManifold = false;

	const CollisionObject2Wrapper* colObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	Assert(colObjWrap->getCollisionShape()->isCompound());

	const CompoundShape* compoundShape = static_cast<const CompoundShape*>(colObjWrap->getCollisionShape());
	m_compoundShapeRevision = compoundShape->getUpdateRevision();

	preallocateChildAlgorithms(body0Wrap, body1Wrap);
}

void CompoundCollisionAlgorithm::preallocateChildAlgorithms(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
{
	const CollisionObject2Wrapper* colObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const CollisionObject2Wrapper* otherObjWrap = m_isSwapped ? body0Wrap : body1Wrap;
	Assert(colObjWrap->getCollisionShape()->isCompound());

	const CompoundShape* compoundShape = static_cast<const CompoundShape*>(colObjWrap->getCollisionShape());

	i32 numChildren = compoundShape->getNumChildShapes();
	i32 i;

	m_childCollisionAlgorithms.resize(numChildren);
	for (i = 0; i < numChildren; i++)
	{
		if (compoundShape->getDynamicAabbTree())
		{
			m_childCollisionAlgorithms[i] = 0;
		}
		else
		{
			const CollisionShape* childShape = compoundShape->getChildShape(i);

			CollisionObject2Wrapper childWrap(colObjWrap, childShape, colObjWrap->getCollisionObject(), colObjWrap->getWorldTransform(), -1, i);  //wrong child trans, but unused (hopefully)
			m_childCollisionAlgorithms[i] = m_dispatcher->findAlgorithm(&childWrap, otherObjWrap, m_sharedManifold, DRX3D_CONTACT_POINT_ALGORITHMS);

			AlignedObjectArray<CollisionAlgorithm*> m_childCollisionAlgorithmsContact;
			AlignedObjectArray<CollisionAlgorithm*> m_childCollisionAlgorithmsClosestPoints;
		}
	}
}

void CompoundCollisionAlgorithm::removeChildAlgorithms()
{
	i32 numChildren = m_childCollisionAlgorithms.size();
	i32 i;
	for (i = 0; i < numChildren; i++)
	{
		if (m_childCollisionAlgorithms[i])
		{
			m_childCollisionAlgorithms[i]->~CollisionAlgorithm();
			m_dispatcher->freeCollisionAlgorithm(m_childCollisionAlgorithms[i]);
		}
	}
}

CompoundCollisionAlgorithm::~CompoundCollisionAlgorithm()
{
	removeChildAlgorithms();
}

struct CompoundLeafCallback : Dbvt::ICollide
{
public:
	const CollisionObject2Wrapper* m_compoundColObjWrap;
	const CollisionObject2Wrapper* m_otherObjWrap;
	Dispatcher* m_dispatcher;
	const DispatcherInfo& m_dispatchInfo;
	ManifoldResult* m_resultOut;
	CollisionAlgorithm** m_childCollisionAlgorithms;
	PersistentManifold* m_sharedManifold;

	CompoundLeafCallback(const CollisionObject2Wrapper* compoundObjWrap, const CollisionObject2Wrapper* otherObjWrap, Dispatcher* dispatcher, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut, CollisionAlgorithm** childCollisionAlgorithms, PersistentManifold* sharedManifold)
		: m_compoundColObjWrap(compoundObjWrap), m_otherObjWrap(otherObjWrap), m_dispatcher(dispatcher), m_dispatchInfo(dispatchInfo), m_resultOut(resultOut), m_childCollisionAlgorithms(childCollisionAlgorithms), m_sharedManifold(sharedManifold)
	{
	}

	void ProcessChildShape(const CollisionShape* childShape, i32 index)
	{
		Assert(index >= 0);
		const CompoundShape* compoundShape = static_cast<const CompoundShape*>(m_compoundColObjWrap->getCollisionShape());
		Assert(index < compoundShape->getNumChildShapes());

		if (gCompoundChildShapePairCallback)
		{
			if (!gCompoundChildShapePairCallback(m_otherObjWrap->getCollisionShape(), childShape))
				return;
		}

		//backup
		Transform2 orgTrans = m_compoundColObjWrap->getWorldTransform();

		const Transform2& childTrans = compoundShape->getChildTransform(index);
		Transform2 newChildWorldTrans = orgTrans * childTrans;

		//perform an AABB check first
		Vec3 aabbMin0, aabbMax0;
		childShape->getAabb(newChildWorldTrans, aabbMin0, aabbMax0);

		Vec3 extendAabb(m_resultOut->m_closestPointDistanceThreshold, m_resultOut->m_closestPointDistanceThreshold, m_resultOut->m_closestPointDistanceThreshold);
		aabbMin0 -= extendAabb;
		aabbMax0 += extendAabb;

		Vec3 aabbMin1, aabbMax1;
		m_otherObjWrap->getCollisionShape()->getAabb(m_otherObjWrap->getWorldTransform(), aabbMin1, aabbMax1);


		if (TestAabbAgainstAabb2(aabbMin0, aabbMax0, aabbMin1, aabbMax1))
		{
			Transform2 preTransform2 = childTrans;
			if (this->m_compoundColObjWrap->m_preTransform2)
			{
				preTransform2 = preTransform2 *(*(this->m_compoundColObjWrap->m_preTransform2));
			}
			CollisionObject2Wrapper compoundWrap(this->m_compoundColObjWrap, childShape, m_compoundColObjWrap->getCollisionObject(), newChildWorldTrans, preTransform2, -1, index);

			CollisionAlgorithm* algo = 0;
			bool allocatedAlgorithm = false;

			if (m_resultOut->m_closestPointDistanceThreshold > 0)
			{
				algo = m_dispatcher->findAlgorithm(&compoundWrap, m_otherObjWrap, 0, DRX3D_CLOSEST_POINT_ALGORITHMS);
				allocatedAlgorithm = true;
			}
			else
			{
				//the contactpoint is still projected back using the original inverted worldtrans
				if (!m_childCollisionAlgorithms[index])
				{
					m_childCollisionAlgorithms[index] = m_dispatcher->findAlgorithm(&compoundWrap, m_otherObjWrap, m_sharedManifold, DRX3D_CONTACT_POINT_ALGORITHMS);
				}
				algo = m_childCollisionAlgorithms[index];
			}

			const CollisionObject2Wrapper* tmpWrap = 0;

			///detect swapping case
			if (m_resultOut->getBody0Internal() == m_compoundColObjWrap->getCollisionObject())
			{
				tmpWrap = m_resultOut->getBody0Wrap();
				m_resultOut->setBody0Wrap(&compoundWrap);
				m_resultOut->setShapeIdentifiersA(-1, index);
			}
			else
			{
				tmpWrap = m_resultOut->getBody1Wrap();
				m_resultOut->setBody1Wrap(&compoundWrap);
				m_resultOut->setShapeIdentifiersB(-1, index);
			}

			algo->processCollision(&compoundWrap, m_otherObjWrap, m_dispatchInfo, m_resultOut);

#if 0
			if (m_dispatchInfo.m_debugDraw && (m_dispatchInfo.m_debugDraw->getDebugMode() & IDebugDraw::DBG_DrawAabb))
			{
				Vec3 worldAabbMin,worldAabbMax;
				m_dispatchInfo.m_debugDraw->drawAabb(aabbMin0,aabbMax0,Vec3(1,1,1));
				m_dispatchInfo.m_debugDraw->drawAabb(aabbMin1,aabbMax1,Vec3(1,1,1));
			}
#endif

			if (m_resultOut->getBody0Internal() == m_compoundColObjWrap->getCollisionObject())
			{
				m_resultOut->setBody0Wrap(tmpWrap);
			}
			else
			{
				m_resultOut->setBody1Wrap(tmpWrap);
			}
			if (allocatedAlgorithm)
			{
				algo->~CollisionAlgorithm();
				m_dispatcher->freeCollisionAlgorithm(algo);
			}
		}
	}
	void Process(const DbvtNode* leaf)
	{
		i32 index = leaf->dataAsInt;

		const CompoundShape* compoundShape = static_cast<const CompoundShape*>(m_compoundColObjWrap->getCollisionShape());
		const CollisionShape* childShape = compoundShape->getChildShape(index);

#if 0
		if (m_dispatchInfo.m_debugDraw && (m_dispatchInfo.m_debugDraw->getDebugMode() & IDebugDraw::DBG_DrawAabb))
		{
			Vec3 worldAabbMin,worldAabbMax;
			Transform2	orgTrans = m_compoundColObjWrap->getWorldTransform();
			Transform2Aabb(leaf->volume.Mins(),leaf->volume.Maxs(),0.,orgTrans,worldAabbMin,worldAabbMax);
			m_dispatchInfo.m_debugDraw->drawAabb(worldAabbMin,worldAabbMax,Vec3(1,0,0));
		}
#endif

		ProcessChildShape(childShape, index);
	}
};

void CompoundCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	const CollisionObject2Wrapper* colObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const CollisionObject2Wrapper* otherObjWrap = m_isSwapped ? body0Wrap : body1Wrap;

	Assert(colObjWrap->getCollisionShape()->isCompound());
	const CompoundShape* compoundShape = static_cast<const CompoundShape*>(colObjWrap->getCollisionShape());

	///CompoundShape might have changed:
	////make sure the internal child collision algorithm caches are still valid
	if (compoundShape->getUpdateRevision() != m_compoundShapeRevision)
	{
		///clear and update all
		removeChildAlgorithms();

		preallocateChildAlgorithms(body0Wrap, body1Wrap);
		m_compoundShapeRevision = compoundShape->getUpdateRevision();
	}

	if (m_childCollisionAlgorithms.size() == 0)
		return;

	const Dbvt* tree = compoundShape->getDynamicAabbTree();
	//use a dynamic aabb tree to cull potential child-overlaps
	CompoundLeafCallback callback(colObjWrap, otherObjWrap, m_dispatcher, dispatchInfo, resultOut, &m_childCollisionAlgorithms[0], m_sharedManifold);

	///we need to refresh all contact manifolds
	///note that we should actually recursively traverse all children, CompoundShape can nested more then 1 level deep
	///so we should add a 'refreshManifolds' in the CollisionAlgorithm
	{
		i32 i;
		manifoldArray.resize(0);
		for (i = 0; i < m_childCollisionAlgorithms.size(); i++)
		{
			if (m_childCollisionAlgorithms[i])
			{
				m_childCollisionAlgorithms[i]->getAllContactManifolds(manifoldArray);
				for (i32 m = 0; m < manifoldArray.size(); m++)
				{
					if (manifoldArray[m]->getNumContacts())
					{
						resultOut->setPersistentManifold(manifoldArray[m]);
						resultOut->refreshContactPoints();
						resultOut->setPersistentManifold(0);  //??necessary?
					}
				}
				manifoldArray.resize(0);
			}
		}
	}

	if (tree)
	{
		Vec3 localAabbMin, localAabbMax;
		Transform2 otherInCompoundSpace;
		otherInCompoundSpace = colObjWrap->getWorldTransform().inverse() * otherObjWrap->getWorldTransform();
		otherObjWrap->getCollisionShape()->getAabb(otherInCompoundSpace, localAabbMin, localAabbMax);
		Vec3 extraExtends(resultOut->m_closestPointDistanceThreshold, resultOut->m_closestPointDistanceThreshold, resultOut->m_closestPointDistanceThreshold);
		localAabbMin -= extraExtends;
		localAabbMax += extraExtends;

		const ATTRIBUTE_ALIGNED16(DbvtVolume) bounds = DbvtVolume::FromMM(localAabbMin, localAabbMax);
		//process all children, that overlap with  the given AABB bounds
		tree->collideTVNoStackAlloc(tree->m_root, bounds, stack2, callback);
	}
	else
	{
		//iterate over all children, perform an AABB check inside ProcessChildShape
		i32 numChildren = m_childCollisionAlgorithms.size();
		i32 i;
		for (i = 0; i < numChildren; i++)
		{
			callback.ProcessChildShape(compoundShape->getChildShape(i), i);
		}
	}

	{
		//iterate over all children, perform an AABB check inside ProcessChildShape
		i32 numChildren = m_childCollisionAlgorithms.size();
		i32 i;
		manifoldArray.resize(0);
		const CollisionShape* childShape = 0;
		Transform2 orgTrans;

		Transform2 newChildWorldTrans;
		Vec3 aabbMin0, aabbMax0, aabbMin1, aabbMax1;

		for (i = 0; i < numChildren; i++)
		{
			if (m_childCollisionAlgorithms[i])
			{
				childShape = compoundShape->getChildShape(i);
				//if not longer overlapping, remove the algorithm
				orgTrans = colObjWrap->getWorldTransform();

				const Transform2& childTrans = compoundShape->getChildTransform(i);
				newChildWorldTrans = orgTrans * childTrans;

				//perform an AABB check first
				childShape->getAabb(newChildWorldTrans, aabbMin0, aabbMax0);
				otherObjWrap->getCollisionShape()->getAabb(otherObjWrap->getWorldTransform(), aabbMin1, aabbMax1);

				if (!TestAabbAgainstAabb2(aabbMin0, aabbMax0, aabbMin1, aabbMax1))
				{
					m_childCollisionAlgorithms[i]->~CollisionAlgorithm();
					m_dispatcher->freeCollisionAlgorithm(m_childCollisionAlgorithms[i]);
					m_childCollisionAlgorithms[i] = 0;
				}
			}
		}
	}
}

Scalar CompoundCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	Assert(0);
	//needs to be fixed, using CollisionObject2Wrapper and NOT modifying internal data structures
	CollisionObject2* colObj = m_isSwapped ? body1 : body0;
	CollisionObject2* otherObj = m_isSwapped ? body0 : body1;

	Assert(colObj->getCollisionShape()->isCompound());

	CompoundShape* compoundShape = static_cast<CompoundShape*>(colObj->getCollisionShape());

	//We will use the OptimizedBVH, AABB tree to cull potential child-overlaps
	//If both proxies are Compound, we will deal with that directly, by performing sequential/parallel tree traversals
	//given Proxy0 and Proxy1, if both have a tree, Tree0 and Tree1, this means:
	//determine overlapping nodes of Proxy1 using Proxy0 AABB against Tree1
	//then use each overlapping node AABB against Tree0
	//and vise versa.

	Scalar hitFraction = Scalar(1.);

	i32 numChildren = m_childCollisionAlgorithms.size();
	i32 i;
	Transform2 orgTrans;
	Scalar frac;
	for (i = 0; i < numChildren; i++)
	{
		//CollisionShape* childShape = compoundShape->getChildShape(i);

		//backup
		orgTrans = colObj->getWorldTransform();

		const Transform2& childTrans = compoundShape->getChildTransform(i);
		//Transform2	newChildWorldTrans = orgTrans*childTrans ;
		colObj->setWorldTransform(orgTrans * childTrans);

		//CollisionShape* tmpShape = colObj->getCollisionShape();
		//colObj->internalSetTemporaryCollisionShape( childShape );
		frac = m_childCollisionAlgorithms[i]->calculateTimeOfImpact(colObj, otherObj, dispatchInfo, resultOut);
		if (frac < hitFraction)
		{
			hitFraction = frac;
		}
		//revert back
		//colObj->internalSetTemporaryCollisionShape( tmpShape);
		colObj->setWorldTransform(orgTrans);
	}
	return hitFraction;
}
