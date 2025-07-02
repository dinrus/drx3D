#include <drx3D/Physics/Collision/Dispatch/CompoundCompoundCollisionAlgorithm.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

//USE_LOCAL_STACK will avoid most (often all) dynamic memory allocations due to resizing in processCollision and MycollideTT
#define USE_LOCAL_STACK 1

ShapePairCallback gCompoundCompoundChildShapePairCallback = 0;

CompoundCompoundCollisionAlgorithm::CompoundCompoundCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped)
	: CompoundCollisionAlgorithm(ci, body0Wrap, body1Wrap, isSwapped)
{
	uk ptr = AlignedAlloc(sizeof(HashedSimplePairCache), 16);
	m_childCollisionAlgorithmCache = new (ptr) HashedSimplePairCache();

	const CollisionObject2Wrapper* col0ObjWrap = body0Wrap;
	Assert(col0ObjWrap->getCollisionShape()->isCompound());

	const CollisionObject2Wrapper* col1ObjWrap = body1Wrap;
	Assert(col1ObjWrap->getCollisionShape()->isCompound());

	const CompoundShape* compoundShape0 = static_cast<const CompoundShape*>(col0ObjWrap->getCollisionShape());
	m_compoundShapeRevision0 = compoundShape0->getUpdateRevision();

	const CompoundShape* compoundShape1 = static_cast<const CompoundShape*>(col1ObjWrap->getCollisionShape());
	m_compoundShapeRevision1 = compoundShape1->getUpdateRevision();
}

CompoundCompoundCollisionAlgorithm::~CompoundCompoundCollisionAlgorithm()
{
	removeChildAlgorithms();
	m_childCollisionAlgorithmCache->~HashedSimplePairCache();
	AlignedFree(m_childCollisionAlgorithmCache);
}

void CompoundCompoundCollisionAlgorithm::getAllContactManifolds(ManifoldArray& manifoldArray)
{
	i32 i;
	SimplePairArray& pairs = m_childCollisionAlgorithmCache->getOverlappingPairArray();
	for (i = 0; i < pairs.size(); i++)
	{
		if (pairs[i].m_userPointer)
		{
			((CollisionAlgorithm*)pairs[i].m_userPointer)->getAllContactManifolds(manifoldArray);
		}
	}
}

void CompoundCompoundCollisionAlgorithm::removeChildAlgorithms()
{
	SimplePairArray& pairs = m_childCollisionAlgorithmCache->getOverlappingPairArray();

	i32 numChildren = pairs.size();
	i32 i;
	for (i = 0; i < numChildren; i++)
	{
		if (pairs[i].m_userPointer)
		{
			CollisionAlgorithm* algo = (CollisionAlgorithm*)pairs[i].m_userPointer;
			algo->~CollisionAlgorithm();
			m_dispatcher->freeCollisionAlgorithm(algo);
		}
	}
	m_childCollisionAlgorithmCache->removeAllPairs();
}

struct CompoundCompoundLeafCallback : Dbvt::ICollide
{
	i32 m_numOverlapPairs;

	const CollisionObject2Wrapper* m_compound0ColObjWrap;
	const CollisionObject2Wrapper* m_compound1ColObjWrap;
	Dispatcher* m_dispatcher;
	const DispatcherInfo& m_dispatchInfo;
	ManifoldResult* m_resultOut;

	class HashedSimplePairCache* m_childCollisionAlgorithmCache;

	PersistentManifold* m_sharedManifold;

	CompoundCompoundLeafCallback(const CollisionObject2Wrapper* compound1ObjWrap,
								   const CollisionObject2Wrapper* compound0ObjWrap,
								   Dispatcher* dispatcher,
								   const DispatcherInfo& dispatchInfo,
								   ManifoldResult* resultOut,
								   HashedSimplePairCache* childAlgorithmsCache,
								   PersistentManifold* sharedManifold)
		: m_numOverlapPairs(0), m_compound0ColObjWrap(compound1ObjWrap), m_compound1ColObjWrap(compound0ObjWrap), m_dispatcher(dispatcher), m_dispatchInfo(dispatchInfo), m_resultOut(resultOut), m_childCollisionAlgorithmCache(childAlgorithmsCache), m_sharedManifold(sharedManifold)
	{
	}

	void Process(const DbvtNode* leaf0, const DbvtNode* leaf1)
	{
		DRX3D_PROFILE("CompoundCompoundLeafCallback::Process");
		m_numOverlapPairs++;

		i32 childIndex0 = leaf0->dataAsInt;
		i32 childIndex1 = leaf1->dataAsInt;

		Assert(childIndex0 >= 0);
		Assert(childIndex1 >= 0);

		const CompoundShape* compoundShape0 = static_cast<const CompoundShape*>(m_compound0ColObjWrap->getCollisionShape());
		Assert(childIndex0 < compoundShape0->getNumChildShapes());

		const CompoundShape* compoundShape1 = static_cast<const CompoundShape*>(m_compound1ColObjWrap->getCollisionShape());
		Assert(childIndex1 < compoundShape1->getNumChildShapes());

		const CollisionShape* childShape0 = compoundShape0->getChildShape(childIndex0);
		const CollisionShape* childShape1 = compoundShape1->getChildShape(childIndex1);

		//backup
		Transform2 orgTrans0 = m_compound0ColObjWrap->getWorldTransform();
		const Transform2& childTrans0 = compoundShape0->getChildTransform(childIndex0);
		Transform2 newChildWorldTrans0 = orgTrans0 * childTrans0;

		Transform2 orgTrans1 = m_compound1ColObjWrap->getWorldTransform();
		const Transform2& childTrans1 = compoundShape1->getChildTransform(childIndex1);
		Transform2 newChildWorldTrans1 = orgTrans1 * childTrans1;

		//perform an AABB check first
		Vec3 aabbMin0, aabbMax0, aabbMin1, aabbMax1;
		childShape0->getAabb(newChildWorldTrans0, aabbMin0, aabbMax0);
		childShape1->getAabb(newChildWorldTrans1, aabbMin1, aabbMax1);

		Vec3 thresholdVec(m_resultOut->m_closestPointDistanceThreshold, m_resultOut->m_closestPointDistanceThreshold, m_resultOut->m_closestPointDistanceThreshold);

		aabbMin0 -= thresholdVec;
		aabbMax0 += thresholdVec;

		if (gCompoundCompoundChildShapePairCallback)
		{
			if (!gCompoundCompoundChildShapePairCallback(childShape0, childShape1))
				return;
		}

		if (TestAabbAgainstAabb2(aabbMin0, aabbMax0, aabbMin1, aabbMax1))
		{
			CollisionObject2Wrapper compoundWrap0(this->m_compound0ColObjWrap, childShape0, m_compound0ColObjWrap->getCollisionObject(), newChildWorldTrans0, -1, childIndex0);
			CollisionObject2Wrapper compoundWrap1(this->m_compound1ColObjWrap, childShape1, m_compound1ColObjWrap->getCollisionObject(), newChildWorldTrans1, -1, childIndex1);

			SimplePair* pair = m_childCollisionAlgorithmCache->findPair(childIndex0, childIndex1);
			bool removePair = false;
			CollisionAlgorithm* colAlgo = 0;
			if (m_resultOut->m_closestPointDistanceThreshold > 0)
			{
				colAlgo = m_dispatcher->findAlgorithm(&compoundWrap0, &compoundWrap1, 0, DRX3D_CLOSEST_POINT_ALGORITHMS);
				removePair = true;
			}
			else
			{
				if (pair)
				{
					colAlgo = (CollisionAlgorithm*)pair->m_userPointer;
				}
				else
				{
					colAlgo = m_dispatcher->findAlgorithm(&compoundWrap0, &compoundWrap1, m_sharedManifold, DRX3D_CONTACT_POINT_ALGORITHMS);
					pair = m_childCollisionAlgorithmCache->addOverlappingPair(childIndex0, childIndex1);
					Assert(pair);
					pair->m_userPointer = colAlgo;
				}
			}

			Assert(colAlgo);

			const CollisionObject2Wrapper* tmpWrap0 = 0;
			const CollisionObject2Wrapper* tmpWrap1 = 0;

			tmpWrap0 = m_resultOut->getBody0Wrap();
			tmpWrap1 = m_resultOut->getBody1Wrap();

			m_resultOut->setBody0Wrap(&compoundWrap0);
			m_resultOut->setBody1Wrap(&compoundWrap1);

			m_resultOut->setShapeIdentifiersA(-1, childIndex0);
			m_resultOut->setShapeIdentifiersB(-1, childIndex1);

			colAlgo->processCollision(&compoundWrap0, &compoundWrap1, m_dispatchInfo, m_resultOut);

			m_resultOut->setBody0Wrap(tmpWrap0);
			m_resultOut->setBody1Wrap(tmpWrap1);

			if (removePair)
			{
				colAlgo->~CollisionAlgorithm();
				m_dispatcher->freeCollisionAlgorithm(colAlgo);
			}
		}
	}
};

static DBVT_INLINE bool MyIntersect(const DbvtAabbMm& a,
									const DbvtAabbMm& b, const Transform2& xform, Scalar distanceThreshold)
{
	Vec3 newmin, newmax;
	Transform2Aabb(b.Mins(), b.Maxs(), 0.f, xform, newmin, newmax);
	newmin -= Vec3(distanceThreshold, distanceThreshold, distanceThreshold);
	newmax += Vec3(distanceThreshold, distanceThreshold, distanceThreshold);
	DbvtAabbMm newb = DbvtAabbMm::FromMM(newmin, newmax);
	return Intersect(a, newb);
}

static inline void MycollideTT(const DbvtNode* root0,
							   const DbvtNode* root1,
							   const Transform2& xform,
							   CompoundCompoundLeafCallback* callback, Scalar distanceThreshold)
{
	if (root0 && root1)
	{
		i32 depth = 1;
		i32 treshold = Dbvt::DOUBLE_STACKSIZE - 4;
		AlignedObjectArray<Dbvt::sStkNN> stkStack;
#ifdef USE_LOCAL_STACK
		ATTRIBUTE_ALIGNED16(Dbvt::sStkNN localStack[Dbvt::DOUBLE_STACKSIZE]);
		stkStack.initializeFromBuffer(&localStack, Dbvt::DOUBLE_STACKSIZE, Dbvt::DOUBLE_STACKSIZE);
#else
		stkStack.resize(Dbvt::DOUBLE_STACKSIZE);
#endif
		stkStack[0] = Dbvt::sStkNN(root0, root1);
		do
		{
			Dbvt::sStkNN p = stkStack[--depth];
			if (MyIntersect(p.a->volume, p.b->volume, xform, distanceThreshold))
			{
				if (depth > treshold)
				{
					stkStack.resize(stkStack.size() * 2);
					treshold = stkStack.size() - 4;
				}
				if (p.a->isinternal())
				{
					if (p.b->isinternal())
					{
						stkStack[depth++] = Dbvt::sStkNN(p.a->childs[0], p.b->childs[0]);
						stkStack[depth++] = Dbvt::sStkNN(p.a->childs[1], p.b->childs[0]);
						stkStack[depth++] = Dbvt::sStkNN(p.a->childs[0], p.b->childs[1]);
						stkStack[depth++] = Dbvt::sStkNN(p.a->childs[1], p.b->childs[1]);
					}
					else
					{
						stkStack[depth++] = Dbvt::sStkNN(p.a->childs[0], p.b);
						stkStack[depth++] = Dbvt::sStkNN(p.a->childs[1], p.b);
					}
				}
				else
				{
					if (p.b->isinternal())
					{
						stkStack[depth++] = Dbvt::sStkNN(p.a, p.b->childs[0]);
						stkStack[depth++] = Dbvt::sStkNN(p.a, p.b->childs[1]);
					}
					else
					{
						callback->Process(p.a, p.b);
					}
				}
			}
		} while (depth);
	}
}

void CompoundCompoundCollisionAlgorithm::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	const CollisionObject2Wrapper* col0ObjWrap = body0Wrap;
	const CollisionObject2Wrapper* col1ObjWrap = body1Wrap;

	Assert(col0ObjWrap->getCollisionShape()->isCompound());
	Assert(col1ObjWrap->getCollisionShape()->isCompound());
	const CompoundShape* compoundShape0 = static_cast<const CompoundShape*>(col0ObjWrap->getCollisionShape());
	const CompoundShape* compoundShape1 = static_cast<const CompoundShape*>(col1ObjWrap->getCollisionShape());

	const Dbvt* tree0 = compoundShape0->getDynamicAabbTree();
	const Dbvt* tree1 = compoundShape1->getDynamicAabbTree();
	if (!tree0 || !tree1)
	{
		return CompoundCollisionAlgorithm::processCollision(body0Wrap, body1Wrap, dispatchInfo, resultOut);
	}
	///CompoundShape might have changed:
	////make sure the internal child collision algorithm caches are still valid
	if ((compoundShape0->getUpdateRevision() != m_compoundShapeRevision0) || (compoundShape1->getUpdateRevision() != m_compoundShapeRevision1))
	{
		///clear all
		removeChildAlgorithms();
		m_compoundShapeRevision0 = compoundShape0->getUpdateRevision();
		m_compoundShapeRevision1 = compoundShape1->getUpdateRevision();
	}

	///we need to refresh all contact manifolds
	///note that we should actually recursively traverse all children, CompoundShape can nested more then 1 level deep
	///so we should add a 'refreshManifolds' in the CollisionAlgorithm
	{
		i32 i;
		ManifoldArray manifoldArray;
#ifdef USE_LOCAL_STACK
		PersistentManifold localManifolds[4];
		manifoldArray.initializeFromBuffer(&localManifolds, 0, 4);
#endif
		SimplePairArray& pairs = m_childCollisionAlgorithmCache->getOverlappingPairArray();
		for (i = 0; i < pairs.size(); i++)
		{
			if (pairs[i].m_userPointer)
			{
				CollisionAlgorithm* algo = (CollisionAlgorithm*)pairs[i].m_userPointer;
				algo->getAllContactManifolds(manifoldArray);
				for (i32 m = 0; m < manifoldArray.size(); m++)
				{
					if (manifoldArray[m]->getNumContacts())
					{
						resultOut->setPersistentManifold(manifoldArray[m]);
						resultOut->refreshContactPoints();
						resultOut->setPersistentManifold(0);
					}
				}
				manifoldArray.resize(0);
			}
		}
	}

	CompoundCompoundLeafCallback callback(col0ObjWrap, col1ObjWrap, this->m_dispatcher, dispatchInfo, resultOut, this->m_childCollisionAlgorithmCache, m_sharedManifold);

	const Transform2 xform = col0ObjWrap->getWorldTransform().inverse() * col1ObjWrap->getWorldTransform();
	MycollideTT(tree0->m_root, tree1->m_root, xform, &callback, resultOut->m_closestPointDistanceThreshold);

	//printf("#compound-compound child/leaf overlap =%d                      \r",callback.m_numOverlapPairs);

	//remove non-overlapping child pairs

	{
		Assert(m_removePairs.size() == 0);

		//iterate over all children, perform an AABB check inside ProcessChildShape
		SimplePairArray& pairs = m_childCollisionAlgorithmCache->getOverlappingPairArray();

		i32 i;
		ManifoldArray manifoldArray;

		Vec3 aabbMin0, aabbMax0, aabbMin1, aabbMax1;

		for (i = 0; i < pairs.size(); i++)
		{
			if (pairs[i].m_userPointer)
			{
				CollisionAlgorithm* algo = (CollisionAlgorithm*)pairs[i].m_userPointer;

				{
					const CollisionShape* childShape0 = 0;

					Transform2 newChildWorldTrans0;
					childShape0 = compoundShape0->getChildShape(pairs[i].m_indexA);
					const Transform2& childTrans0 = compoundShape0->getChildTransform(pairs[i].m_indexA);
					newChildWorldTrans0 = col0ObjWrap->getWorldTransform() * childTrans0;
					childShape0->getAabb(newChildWorldTrans0, aabbMin0, aabbMax0);
				}
				Vec3 thresholdVec(resultOut->m_closestPointDistanceThreshold, resultOut->m_closestPointDistanceThreshold, resultOut->m_closestPointDistanceThreshold);
				aabbMin0 -= thresholdVec;
				aabbMax0 += thresholdVec;
				{
					const CollisionShape* childShape1 = 0;
					Transform2 newChildWorldTrans1;

					childShape1 = compoundShape1->getChildShape(pairs[i].m_indexB);
					const Transform2& childTrans1 = compoundShape1->getChildTransform(pairs[i].m_indexB);
					newChildWorldTrans1 = col1ObjWrap->getWorldTransform() * childTrans1;
					childShape1->getAabb(newChildWorldTrans1, aabbMin1, aabbMax1);
				}

				aabbMin1 -= thresholdVec;
				aabbMax1 += thresholdVec;

				if (!TestAabbAgainstAabb2(aabbMin0, aabbMax0, aabbMin1, aabbMax1))
				{
					algo->~CollisionAlgorithm();
					m_dispatcher->freeCollisionAlgorithm(algo);
					m_removePairs.push_back(SimplePair(pairs[i].m_indexA, pairs[i].m_indexB));
				}
			}
		}
		for (i32 i = 0; i < m_removePairs.size(); i++)
		{
			m_childCollisionAlgorithmCache->removeOverlappingPair(m_removePairs[i].m_indexA, m_removePairs[i].m_indexB);
		}
		m_removePairs.clear();
	}
}

Scalar CompoundCompoundCollisionAlgorithm::calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	Assert(0);
	return 0.f;
}
