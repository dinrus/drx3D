#include <drx3D/Physics/Collision/BroadPhase/b3DynamicBvhBroadphase.h>
#include <drx3D/Physics/Collision/BroadPhase/b3OverlappingPair.h>

//
// Profiling
//

#if D3_DBVT_BP_PROFILE || D3_DBVT_BP_ENABLE_BENCHMARK
#include <stdio.h>
#endif

#if D3_DBVT_BP_PROFILE
struct b3ProfileScope
{
	__forceinline b3ProfileScope(b3Clock& clock, u64& value) : m_clock(&clock), m_value(&value), m_base(clock.getTimeMicroseconds())
	{
	}
	__forceinline ~b3ProfileScope()
	{
		(*m_value) += m_clock->getTimeMicroseconds() - m_base;
	}
	b3Clock* m_clock;
	u64* m_value;
	u64 m_base;
};
#define b3SPC(_value_) b3ProfileScope spc_scope(m_clock, _value_)
#else
#define b3SPC(_value_)
#endif

//
// Helpers
//

//
template <typename T>
static inline void b3ListAppend(T* item, T*& list)
{
	item->links[0] = 0;
	item->links[1] = list;
	if (list) list->links[0] = item;
	list = item;
}

//
template <typename T>
static inline void b3ListRemove(T* item, T*& list)
{
	if (item->links[0])
		item->links[0]->links[1] = item->links[1];
	else
		list = item->links[1];
	if (item->links[1]) item->links[1]->links[0] = item->links[0];
}

//
template <typename T>
static inline i32 b3ListCount(T* root)
{
	i32 n = 0;
	while (root)
	{
		++n;
		root = root->links[1];
	}
	return (n);
}

//
template <typename T>
static inline void b3Clear(T& value)
{
	static const struct ZeroDummy : T
	{
	} zerodummy;
	value = zerodummy;
}

//
// Colliders
//

/* Tree collider	*/
struct b3DbvtTreeCollider : b3DynamicBvh::ICollide
{
	b3DynamicBvhBroadphase* pbp;
	b3DbvtProxy* proxy;
	b3DbvtTreeCollider(b3DynamicBvhBroadphase* p) : pbp(p) {}
	void Process(const b3DbvtNode* na, const b3DbvtNode* nb)
	{
		if (na != nb)
		{
			b3DbvtProxy* pa = (b3DbvtProxy*)na->data;
			b3DbvtProxy* pb = (b3DbvtProxy*)nb->data;
#if D3_DBVT_BP_SORTPAIRS
			if (pa->m_uniqueId > pb->m_uniqueId)
				b3Swap(pa, pb);
#endif
			pbp->m_paircache->addOverlappingPair(pa->getUid(), pb->getUid());
			++pbp->m_newpairs;
		}
	}
	void Process(const b3DbvtNode* n)
	{
		Process(n, proxy->leaf);
	}
};

//
// b3DynamicBvhBroadphase
//

//
b3DynamicBvhBroadphase::b3DynamicBvhBroadphase(i32 proxyCapacity, b3OverlappingPairCache* paircache)
{
	m_deferedcollide = false;
	m_needcleanup = true;
	m_releasepaircache = (paircache != 0) ? false : true;
	m_prediction = 0;
	m_stageCurrent = 0;
	m_fixedleft = 0;
	m_fupdates = 1;
	m_dupdates = 0;
	m_cupdates = 10;
	m_newpairs = 1;
	m_updates_call = 0;
	m_updates_done = 0;
	m_updates_ratio = 0;
	m_paircache = paircache ? paircache : new (b3AlignedAlloc(sizeof(b3HashedOverlappingPairCache), 16)) b3HashedOverlappingPairCache();

	m_pid = 0;
	m_cid = 0;
	for (i32 i = 0; i <= STAGECOUNT; ++i)
	{
		m_stageRoots[i] = 0;
	}
#if D3_DBVT_BP_PROFILE
	b3Clear(m_profiling);
#endif
	m_proxies.resize(proxyCapacity);
}

//
b3DynamicBvhBroadphase::~b3DynamicBvhBroadphase()
{
	if (m_releasepaircache)
	{
		m_paircache->~b3OverlappingPairCache();
		b3AlignedFree(m_paircache);
	}
}

//
b3BroadphaseProxy* b3DynamicBvhBroadphase::createProxy(const b3Vec3& aabbMin,
													   const b3Vec3& aabbMax,
													   i32 objectId,
													   uk userPtr,
													   i32 collisionFilterGroup,
													   i32 collisionFilterMask)
{
	b3DbvtProxy* mem = &m_proxies[objectId];
	b3DbvtProxy* proxy = new (mem) b3DbvtProxy(aabbMin, aabbMax, userPtr,
											   collisionFilterGroup,
											   collisionFilterMask);

	b3DbvtAabbMm aabb = b3DbvtVolume::FromMM(aabbMin, aabbMax);

	//bproxy->aabb			=	b3DbvtVolume::FromMM(aabbMin,aabbMax);
	proxy->stage = m_stageCurrent;
	proxy->m_uniqueId = objectId;
	proxy->leaf = m_sets[0].insert(aabb, proxy);
	b3ListAppend(proxy, m_stageRoots[m_stageCurrent]);
	if (!m_deferedcollide)
	{
		b3DbvtTreeCollider collider(this);
		collider.proxy = proxy;
		m_sets[0].collideTV(m_sets[0].m_root, aabb, collider);
		m_sets[1].collideTV(m_sets[1].m_root, aabb, collider);
	}
	return (proxy);
}

//
void b3DynamicBvhBroadphase::destroyProxy(b3BroadphaseProxy* absproxy,
										  b3Dispatcher* dispatcher)
{
	b3DbvtProxy* proxy = (b3DbvtProxy*)absproxy;
	if (proxy->stage == STAGECOUNT)
		m_sets[1].remove(proxy->leaf);
	else
		m_sets[0].remove(proxy->leaf);
	b3ListRemove(proxy, m_stageRoots[proxy->stage]);
	m_paircache->removeOverlappingPairsContainingProxy(proxy->getUid(), dispatcher);

	m_needcleanup = true;
}

void b3DynamicBvhBroadphase::getAabb(i32 objectId, b3Vec3& aabbMin, b3Vec3& aabbMax) const
{
	const b3DbvtProxy* proxy = &m_proxies[objectId];
	aabbMin = proxy->m_aabbMin;
	aabbMax = proxy->m_aabbMax;
}
/*
void	b3DynamicBvhBroadphase::getAabb(b3BroadphaseProxy* absproxy,b3Vec3& aabbMin, b3Vec3& aabbMax ) const
{
	b3DbvtProxy*						proxy=(b3DbvtProxy*)absproxy;
	aabbMin = proxy->m_aabbMin;
	aabbMax = proxy->m_aabbMax;
}
*/

struct BroadphaseRayTester : b3DynamicBvh::ICollide
{
	b3BroadphaseRayCallback& m_rayCallback;
	BroadphaseRayTester(b3BroadphaseRayCallback& orgCallback)
		: m_rayCallback(orgCallback)
	{
	}
	void Process(const b3DbvtNode* leaf)
	{
		b3DbvtProxy* proxy = (b3DbvtProxy*)leaf->data;
		m_rayCallback.process(proxy);
	}
};

void b3DynamicBvhBroadphase::rayTest(const b3Vec3& rayFrom, const b3Vec3& rayTo, b3BroadphaseRayCallback& rayCallback, const b3Vec3& aabbMin, const b3Vec3& aabbMax)
{
	BroadphaseRayTester callback(rayCallback);

	m_sets[0].rayTestInternal(m_sets[0].m_root,
							  rayFrom,
							  rayTo,
							  rayCallback.m_rayDirectionInverse,
							  rayCallback.m_signs,
							  rayCallback.m_lambda_max,
							  aabbMin,
							  aabbMax,
							  callback);

	m_sets[1].rayTestInternal(m_sets[1].m_root,
							  rayFrom,
							  rayTo,
							  rayCallback.m_rayDirectionInverse,
							  rayCallback.m_signs,
							  rayCallback.m_lambda_max,
							  aabbMin,
							  aabbMax,
							  callback);
}

struct BroadphaseAabbTester : b3DynamicBvh::ICollide
{
	b3BroadphaseAabbCallback& m_aabbCallback;
	BroadphaseAabbTester(b3BroadphaseAabbCallback& orgCallback)
		: m_aabbCallback(orgCallback)
	{
	}
	void Process(const b3DbvtNode* leaf)
	{
		b3DbvtProxy* proxy = (b3DbvtProxy*)leaf->data;
		m_aabbCallback.process(proxy);
	}
};

void b3DynamicBvhBroadphase::aabbTest(const b3Vec3& aabbMin, const b3Vec3& aabbMax, b3BroadphaseAabbCallback& aabbCallback)
{
	BroadphaseAabbTester callback(aabbCallback);

	const D3_ATTRIBUTE_ALIGNED16(b3DbvtVolume) bounds = b3DbvtVolume::FromMM(aabbMin, aabbMax);
	//process all children, that overlap with  the given AABB bounds
	m_sets[0].collideTV(m_sets[0].m_root, bounds, callback);
	m_sets[1].collideTV(m_sets[1].m_root, bounds, callback);
}

//
void b3DynamicBvhBroadphase::setAabb(i32 objectId,
									 const b3Vec3& aabbMin,
									 const b3Vec3& aabbMax,
									 b3Dispatcher* /*dispatcher*/)
{
	b3DbvtProxy* proxy = &m_proxies[objectId];
	//	b3DbvtProxy*						proxy=(b3DbvtProxy*)absproxy;
	D3_ATTRIBUTE_ALIGNED16(b3DbvtVolume)
	aabb = b3DbvtVolume::FromMM(aabbMin, aabbMax);
#if D3_DBVT_BP_PREVENTFALSEUPDATE
	if (b3NotEqual(aabb, proxy->leaf->volume))
#endif
	{
		bool docollide = false;
		if (proxy->stage == STAGECOUNT)
		{ /* fixed -> dynamic set	*/
			m_sets[1].remove(proxy->leaf);
			proxy->leaf = m_sets[0].insert(aabb, proxy);
			docollide = true;
		}
		else
		{ /* dynamic set				*/
			++m_updates_call;
			if (b3Intersect(proxy->leaf->volume, aabb))
			{ /* Moving				*/

				const b3Vec3 delta = aabbMin - proxy->m_aabbMin;
				b3Vec3 velocity(((proxy->m_aabbMax - proxy->m_aabbMin) / 2) * m_prediction);
				if (delta[0] < 0) velocity[0] = -velocity[0];
				if (delta[1] < 0) velocity[1] = -velocity[1];
				if (delta[2] < 0) velocity[2] = -velocity[2];
				if (
#ifdef D3_DBVT_BP_MARGIN
					m_sets[0].update(proxy->leaf, aabb, velocity, D3_DBVT_BP_MARGIN)
#else
					m_sets[0].update(proxy->leaf, aabb, velocity)
#endif
				)
				{
					++m_updates_done;
					docollide = true;
				}
			}
			else
			{ /* Teleporting			*/
				m_sets[0].update(proxy->leaf, aabb);
				++m_updates_done;
				docollide = true;
			}
		}
		b3ListRemove(proxy, m_stageRoots[proxy->stage]);
		proxy->m_aabbMin = aabbMin;
		proxy->m_aabbMax = aabbMax;
		proxy->stage = m_stageCurrent;
		b3ListAppend(proxy, m_stageRoots[m_stageCurrent]);
		if (docollide)
		{
			m_needcleanup = true;
			if (!m_deferedcollide)
			{
				b3DbvtTreeCollider collider(this);
				m_sets[1].collideTTpersistentStack(m_sets[1].m_root, proxy->leaf, collider);
				m_sets[0].collideTTpersistentStack(m_sets[0].m_root, proxy->leaf, collider);
			}
		}
	}
}

//
void b3DynamicBvhBroadphase::setAabbForceUpdate(b3BroadphaseProxy* absproxy,
												const b3Vec3& aabbMin,
												const b3Vec3& aabbMax,
												b3Dispatcher* /*dispatcher*/)
{
	b3DbvtProxy* proxy = (b3DbvtProxy*)absproxy;
	D3_ATTRIBUTE_ALIGNED16(b3DbvtVolume)
	aabb = b3DbvtVolume::FromMM(aabbMin, aabbMax);
	bool docollide = false;
	if (proxy->stage == STAGECOUNT)
	{ /* fixed -> dynamic set	*/
		m_sets[1].remove(proxy->leaf);
		proxy->leaf = m_sets[0].insert(aabb, proxy);
		docollide = true;
	}
	else
	{ /* dynamic set				*/
		++m_updates_call;
		/* Teleporting			*/
		m_sets[0].update(proxy->leaf, aabb);
		++m_updates_done;
		docollide = true;
	}
	b3ListRemove(proxy, m_stageRoots[proxy->stage]);
	proxy->m_aabbMin = aabbMin;
	proxy->m_aabbMax = aabbMax;
	proxy->stage = m_stageCurrent;
	b3ListAppend(proxy, m_stageRoots[m_stageCurrent]);
	if (docollide)
	{
		m_needcleanup = true;
		if (!m_deferedcollide)
		{
			b3DbvtTreeCollider collider(this);
			m_sets[1].collideTTpersistentStack(m_sets[1].m_root, proxy->leaf, collider);
			m_sets[0].collideTTpersistentStack(m_sets[0].m_root, proxy->leaf, collider);
		}
	}
}

//
void b3DynamicBvhBroadphase::calculateOverlappingPairs(b3Dispatcher* dispatcher)
{
	collide(dispatcher);
#if D3_DBVT_BP_PROFILE
	if (0 == (m_pid % D3_DBVT_BP_PROFILING_RATE))
	{
		printf("fixed(%u) dynamics(%u) pairs(%u)\r\n", m_sets[1].m_leaves, m_sets[0].m_leaves, m_paircache->getNumOverlappingPairs());
		u32 total = m_profiling.m_total;
		if (total <= 0) total = 1;
		printf("ddcollide: %u%% (%uus)\r\n", (50 + m_profiling.m_ddcollide * 100) / total, m_profiling.m_ddcollide / D3_DBVT_BP_PROFILING_RATE);
		printf("fdcollide: %u%% (%uus)\r\n", (50 + m_profiling.m_fdcollide * 100) / total, m_profiling.m_fdcollide / D3_DBVT_BP_PROFILING_RATE);
		printf("cleanup:   %u%% (%uus)\r\n", (50 + m_profiling.m_cleanup * 100) / total, m_profiling.m_cleanup / D3_DBVT_BP_PROFILING_RATE);
		printf("total:     %uus\r\n", total / D3_DBVT_BP_PROFILING_RATE);
		const u64 sum = m_profiling.m_ddcollide +
								  m_profiling.m_fdcollide +
								  m_profiling.m_cleanup;
		printf("leaked: %u%% (%uus)\r\n", 100 - ((50 + sum * 100) / total), (total - sum) / D3_DBVT_BP_PROFILING_RATE);
		printf("job counts: %u%%\r\n", (m_profiling.m_jobcount * 100) / ((m_sets[0].m_leaves + m_sets[1].m_leaves) * D3_DBVT_BP_PROFILING_RATE));
		b3Clear(m_profiling);
		m_clock.reset();
	}
#endif

	performDeferredRemoval(dispatcher);
}

void b3DynamicBvhBroadphase::performDeferredRemoval(b3Dispatcher* dispatcher)
{
	if (m_paircache->hasDeferredRemoval())
	{
		b3BroadphasePairArray& overlappingPairArray = m_paircache->getOverlappingPairArray();

		//perform a sort, to find duplicates and to sort 'invalid' pairs to the end
		overlappingPairArray.quickSort(b3BroadphasePairSortPredicate());

		i32 invalidPair = 0;

		i32 i;

		b3BroadphasePair previousPair = b3MakeBroadphasePair(-1, -1);

		for (i = 0; i < overlappingPairArray.size(); i++)
		{
			b3BroadphasePair& pair = overlappingPairArray[i];

			bool isDuplicate = (pair == previousPair);

			previousPair = pair;

			bool needsRemoval = false;

			if (!isDuplicate)
			{
				//important to perform AABB check that is consistent with the broadphase
				b3DbvtProxy* pa = &m_proxies[pair.x];
				b3DbvtProxy* pb = &m_proxies[pair.y];
				bool hasOverlap = b3Intersect(pa->leaf->volume, pb->leaf->volume);

				if (hasOverlap)
				{
					needsRemoval = false;
				}
				else
				{
					needsRemoval = true;
				}
			}
			else
			{
				//remove duplicate
				needsRemoval = true;
				//should have no algorithm
			}

			if (needsRemoval)
			{
				m_paircache->cleanOverlappingPair(pair, dispatcher);

				pair.x = -1;
				pair.y = -1;
				invalidPair++;
			}
		}

		//perform a sort, to sort 'invalid' pairs to the end
		overlappingPairArray.quickSort(b3BroadphasePairSortPredicate());
		overlappingPairArray.resize(overlappingPairArray.size() - invalidPair);
	}
}

//
void b3DynamicBvhBroadphase::collide(b3Dispatcher* dispatcher)
{
	/*printf("---------------------------------------------------------\n");
	printf("m_sets[0].m_leaves=%d\n",m_sets[0].m_leaves);
	printf("m_sets[1].m_leaves=%d\n",m_sets[1].m_leaves);
	printf("numPairs = %d\n",getOverlappingPairCache()->getNumOverlappingPairs());
	{
		i32 i;
		for (i=0;i<getOverlappingPairCache()->getNumOverlappingPairs();i++)
		{
			printf("pair[%d]=(%d,%d),",i,getOverlappingPairCache()->getOverlappingPairArray()[i].m_pProxy0->getUid(),
				getOverlappingPairCache()->getOverlappingPairArray()[i].m_pProxy1->getUid());
		}
		printf("\n");
	}
*/

	b3SPC(m_profiling.m_total);
	/* optimize				*/
	m_sets[0].optimizeIncremental(1 + (m_sets[0].m_leaves * m_dupdates) / 100);
	if (m_fixedleft)
	{
		i32k count = 1 + (m_sets[1].m_leaves * m_fupdates) / 100;
		m_sets[1].optimizeIncremental(1 + (m_sets[1].m_leaves * m_fupdates) / 100);
		m_fixedleft = d3Max<i32>(0, m_fixedleft - count);
	}
	/* dynamic -> fixed set	*/
	m_stageCurrent = (m_stageCurrent + 1) % STAGECOUNT;
	b3DbvtProxy* current = m_stageRoots[m_stageCurrent];
	if (current)
	{
		b3DbvtTreeCollider collider(this);
		do
		{
			b3DbvtProxy* next = current->links[1];
			b3ListRemove(current, m_stageRoots[current->stage]);
			b3ListAppend(current, m_stageRoots[STAGECOUNT]);
#if D3_DBVT_BP_ACCURATESLEEPING
			m_paircache->removeOverlappingPairsContainingProxy(current, dispatcher);
			collider.proxy = current;
			b3DynamicBvh::collideTV(m_sets[0].m_root, current->aabb, collider);
			b3DynamicBvh::collideTV(m_sets[1].m_root, current->aabb, collider);
#endif
			m_sets[0].remove(current->leaf);
			D3_ATTRIBUTE_ALIGNED16(b3DbvtVolume)
			curAabb = b3DbvtVolume::FromMM(current->m_aabbMin, current->m_aabbMax);
			current->leaf = m_sets[1].insert(curAabb, current);
			current->stage = STAGECOUNT;
			current = next;
		} while (current);
		m_fixedleft = m_sets[1].m_leaves;
		m_needcleanup = true;
	}
	/* collide dynamics		*/
	{
		b3DbvtTreeCollider collider(this);
		if (m_deferedcollide)
		{
			b3SPC(m_profiling.m_fdcollide);
			m_sets[0].collideTTpersistentStack(m_sets[0].m_root, m_sets[1].m_root, collider);
		}
		if (m_deferedcollide)
		{
			b3SPC(m_profiling.m_ddcollide);
			m_sets[0].collideTTpersistentStack(m_sets[0].m_root, m_sets[0].m_root, collider);
		}
	}
	/* clean up				*/
	if (m_needcleanup)
	{
		b3SPC(m_profiling.m_cleanup);
		b3BroadphasePairArray& pairs = m_paircache->getOverlappingPairArray();
		if (pairs.size() > 0)
		{
			i32 ni = d3Min(pairs.size(), d3Max<i32>(m_newpairs, (pairs.size() * m_cupdates) / 100));
			for (i32 i = 0; i < ni; ++i)
			{
				b3BroadphasePair& p = pairs[(m_cid + i) % pairs.size()];
				b3DbvtProxy* pa = &m_proxies[p.x];
				b3DbvtProxy* pb = &m_proxies[p.y];
				if (!b3Intersect(pa->leaf->volume, pb->leaf->volume))
				{
#if D3_DBVT_BP_SORTPAIRS
					if (pa->m_uniqueId > pb->m_uniqueId)
						b3Swap(pa, pb);
#endif
					m_paircache->removeOverlappingPair(pa->getUid(), pb->getUid(), dispatcher);
					--ni;
					--i;
				}
			}
			if (pairs.size() > 0)
				m_cid = (m_cid + ni) % pairs.size();
			else
				m_cid = 0;
		}
	}
	++m_pid;
	m_newpairs = 1;
	m_needcleanup = false;
	if (m_updates_call > 0)
	{
		m_updates_ratio = m_updates_done / (b3Scalar)m_updates_call;
	}
	else
	{
		m_updates_ratio = 0;
	}
	m_updates_done /= 2;
	m_updates_call /= 2;
}

//
void b3DynamicBvhBroadphase::optimize()
{
	m_sets[0].optimizeTopDown();
	m_sets[1].optimizeTopDown();
}

//
b3OverlappingPairCache* b3DynamicBvhBroadphase::getOverlappingPairCache()
{
	return (m_paircache);
}

//
const b3OverlappingPairCache* b3DynamicBvhBroadphase::getOverlappingPairCache() const
{
	return (m_paircache);
}

//
void b3DynamicBvhBroadphase::getBroadphaseAabb(b3Vec3& aabbMin, b3Vec3& aabbMax) const
{
	D3_ATTRIBUTE_ALIGNED16(b3DbvtVolume)
	bounds;

	if (!m_sets[0].empty())
		if (!m_sets[1].empty())
			b3Merge(m_sets[0].m_root->volume,
					m_sets[1].m_root->volume, bounds);
		else
			bounds = m_sets[0].m_root->volume;
	else if (!m_sets[1].empty())
		bounds = m_sets[1].m_root->volume;
	else
		bounds = b3DbvtVolume::FromCR(b3MakeVector3(0, 0, 0), 0);
	aabbMin = bounds.Mins();
	aabbMax = bounds.Maxs();
}

void b3DynamicBvhBroadphase::resetPool(b3Dispatcher* dispatcher)
{
	i32 totalObjects = m_sets[0].m_leaves + m_sets[1].m_leaves;
	if (!totalObjects)
	{
		//reset internal dynamic tree data structures
		m_sets[0].clear();
		m_sets[1].clear();

		m_deferedcollide = false;
		m_needcleanup = true;
		m_stageCurrent = 0;
		m_fixedleft = 0;
		m_fupdates = 1;
		m_dupdates = 0;
		m_cupdates = 10;
		m_newpairs = 1;
		m_updates_call = 0;
		m_updates_done = 0;
		m_updates_ratio = 0;

		m_pid = 0;
		m_cid = 0;
		for (i32 i = 0; i <= STAGECOUNT; ++i)
		{
			m_stageRoots[i] = 0;
		}
	}
}

//
void b3DynamicBvhBroadphase::printStats()
{
}

//
#if D3_DBVT_BP_ENABLE_BENCHMARK

struct b3BroadphaseBenchmark
{
	struct Experiment
	{
		tukk name;
		i32 object_count;
		i32 update_count;
		i32 spawn_count;
		i32 iterations;
		b3Scalar speed;
		b3Scalar amplitude;
	};
	struct Object
	{
		b3Vec3 center;
		b3Vec3 extents;
		b3BroadphaseProxy* proxy;
		b3Scalar time;
		void update(b3Scalar speed, b3Scalar amplitude, b3BroadphaseInterface* pbi)
		{
			time += speed;
			center[0] = b3Cos(time * (b3Scalar)2.17) * amplitude +
						b3Sin(time) * amplitude / 2;
			center[1] = b3Cos(time * (b3Scalar)1.38) * amplitude +
						b3Sin(time) * amplitude;
			center[2] = b3Sin(time * (b3Scalar)0.777) * amplitude;
			pbi->setAabb(proxy, center - extents, center + extents, 0);
		}
	};
	static i32 UnsignedRand(i32 range = RAND_MAX - 1) { return (rand() % (range + 1)); }
	static b3Scalar UnitRand() { return (UnsignedRand(16384) / (b3Scalar)16384); }
	static void OutputTime(tukk name, b3Clock& c, unsigned count = 0)
	{
		const u64 us = c.getTimeMicroseconds();
		const u64 ms = (us + 500) / 1000;
		const b3Scalar sec = us / (b3Scalar)(1000 * 1000);
		if (count > 0)
			printf("%s : %u us (%u ms), %.2f/s\r\n", name, us, ms, count / sec);
		else
			printf("%s : %u us (%u ms)\r\n", name, us, ms);
	}
};

void b3DynamicBvhBroadphase::benchmark(b3BroadphaseInterface* pbi)
{
	static const b3BroadphaseBenchmark::Experiment experiments[] =
		{
			{"1024o.10%", 1024, 10, 0, 8192, (b3Scalar)0.005, (b3Scalar)100},
			/*{"4096o.10%",4096,10,0,8192,(b3Scalar)0.005,(b3Scalar)100},
		{"8192o.10%",8192,10,0,8192,(b3Scalar)0.005,(b3Scalar)100},*/
		};
	static i32k nexperiments = sizeof(experiments) / sizeof(experiments[0]);
	b3AlignedObjectArray<b3BroadphaseBenchmark::Object*> objects;
	b3Clock wallclock;
	/* Begin			*/
	for (i32 iexp = 0; iexp < nexperiments; ++iexp)
	{
		const b3BroadphaseBenchmark::Experiment& experiment = experiments[iexp];
		i32k object_count = experiment.object_count;
		i32k update_count = (object_count * experiment.update_count) / 100;
		i32k spawn_count = (object_count * experiment.spawn_count) / 100;
		const b3Scalar speed = experiment.speed;
		const b3Scalar amplitude = experiment.amplitude;
		printf("Experiment #%u '%s':\r\n", iexp, experiment.name);
		printf("\tObjects: %u\r\n", object_count);
		printf("\tUpdate: %u\r\n", update_count);
		printf("\tSpawn: %u\r\n", spawn_count);
		printf("\tSpeed: %f\r\n", speed);
		printf("\tAmplitude: %f\r\n", amplitude);
		srand(180673);
		/* Create objects	*/
		wallclock.reset();
		objects.reserve(object_count);
		for (i32 i = 0; i < object_count; ++i)
		{
			b3BroadphaseBenchmark::Object* po = new b3BroadphaseBenchmark::Object();
			po->center[0] = b3BroadphaseBenchmark::UnitRand() * 50;
			po->center[1] = b3BroadphaseBenchmark::UnitRand() * 50;
			po->center[2] = b3BroadphaseBenchmark::UnitRand() * 50;
			po->extents[0] = b3BroadphaseBenchmark::UnitRand() * 2 + 2;
			po->extents[1] = b3BroadphaseBenchmark::UnitRand() * 2 + 2;
			po->extents[2] = b3BroadphaseBenchmark::UnitRand() * 2 + 2;
			po->time = b3BroadphaseBenchmark::UnitRand() * 2000;
			po->proxy = pbi->createProxy(po->center - po->extents, po->center + po->extents, 0, po, 1, 1, 0, 0);
			objects.push_back(po);
		}
		b3BroadphaseBenchmark::OutputTime("\tInitialization", wallclock);
		/* First update		*/
		wallclock.reset();
		for (i32 i = 0; i < objects.size(); ++i)
		{
			objects[i]->update(speed, amplitude, pbi);
		}
		b3BroadphaseBenchmark::OutputTime("\tFirst update", wallclock);
		/* Updates			*/
		wallclock.reset();
		for (i32 i = 0; i < experiment.iterations; ++i)
		{
			for (i32 j = 0; j < update_count; ++j)
			{
				objects[j]->update(speed, amplitude, pbi);
			}
			pbi->calculateOverlappingPairs(0);
		}
		b3BroadphaseBenchmark::OutputTime("\tUpdate", wallclock, experiment.iterations);
		/* Clean up			*/
		wallclock.reset();
		for (i32 i = 0; i < objects.size(); ++i)
		{
			pbi->destroyProxy(objects[i]->proxy, 0);
			delete objects[i];
		}
		objects.resize(0);
		b3BroadphaseBenchmark::OutputTime("\tRelease", wallclock);
	}
}
#else
/*void							b3DynamicBvhBroadphase::benchmark(b3BroadphaseInterface*)
{}
*/
#endif

#if D3_DBVT_BP_PROFILE
#undef b3SPC
#endif
