#include <drx3D/Physics/Collision/BroadPhase/DbvtBroadphase.h>
#include <drx3D/Maths/Linear/Threads.h>
Scalar gDbvtMargin = Scalar(0.05);
//
// Profiling
//

#if DBVT_BP_PROFILE || DBVT_BP_ENABLE_BENCHMARK
#include <stdio.h>
#endif

#if DBVT_BP_PROFILE
struct ProfileScope
{
	__forceinline ProfileScope(Clock& clock, u64& value) : m_clock(&clock), m_value(&value), m_base(clock.getTimeMicroseconds())
	{
	}
	__forceinline ~ProfileScope()
	{
		(*m_value) += m_clock->getTimeMicroseconds() - m_base;
	}
	Clock* m_clock;
	u64* m_value;
	u64 m_base;
};
#define SPC(_value_) ProfileScope spc_scope(m_clock, _value_)
#else
#define SPC(_value_)
#endif

//
// Helpers
//

//
template <typename T>
static inline void listappend(T* item, T*& list)
{
	item->links[0] = 0;
	item->links[1] = list;
	if (list) list->links[0] = item;
	list = item;
}

//
template <typename T>
static inline void listremove(T* item, T*& list)
{
	if (item->links[0])
		item->links[0]->links[1] = item->links[1];
	else
		list = item->links[1];
	if (item->links[1]) item->links[1]->links[0] = item->links[0];
}

//
template <typename T>
static inline i32 listcount(T* root)
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
static inline void clear(T& value)
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
struct DbvtTreeCollider : Dbvt::ICollide
{
	DbvtBroadphase* pbp;
	DbvtProxy* proxy;
	DbvtTreeCollider(DbvtBroadphase* p) : pbp(p) {}
	void Process(const DbvtNode* na, const DbvtNode* nb)
	{
		if (na != nb)
		{
			DbvtProxy* pa = (DbvtProxy*)na->data;
			DbvtProxy* pb = (DbvtProxy*)nb->data;
#if DBVT_BP_SORTPAIRS
			if (pa->m_uniqueId > pb->m_uniqueId)
				Swap(pa, pb);
#endif
			pbp->m_paircache->addOverlappingPair(pa, pb);
			++pbp->m_newpairs;
		}
	}
	void Process(const DbvtNode* n)
	{
		Process(n, proxy->leaf);
	}
};

//
// DbvtBroadphase
//

//
DbvtBroadphase::DbvtBroadphase(OverlappingPairCache* paircache)
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
	m_paircache = paircache ? paircache : new (AlignedAlloc(sizeof(HashedOverlappingPairCache), 16)) HashedOverlappingPairCache();
	m_gid = 0;
	m_pid = 0;
	m_cid = 0;
	for (i32 i = 0; i <= STAGECOUNT; ++i)
	{
		m_stageRoots[i] = 0;
	}
#if DRX3D_THREADSAFE
	m_rayTestStacks.resize(DRX3D_MAX_THREAD_COUNT);
#else
	m_rayTestStacks.resize(1);
#endif
#if DBVT_BP_PROFILE
	clear(m_profiling);
#endif
}

//
DbvtBroadphase::~DbvtBroadphase()
{
	if (m_releasepaircache)
	{
		m_paircache->~OverlappingPairCache();
		AlignedFree(m_paircache);
	}
}

//
BroadphaseProxy* DbvtBroadphase::createProxy(const Vec3& aabbMin,
												 const Vec3& aabbMax,
												 i32 /*shapeType*/,
												 uk userPtr,
												 i32 collisionFilterGroup,
												 i32 collisionFilterMask,
												 Dispatcher* /*dispatcher*/)
{
	DbvtProxy* proxy = new (AlignedAlloc(sizeof(DbvtProxy), 16)) DbvtProxy(aabbMin, aabbMax, userPtr,
																				   collisionFilterGroup,
																				   collisionFilterMask);

	DbvtAabbMm aabb = DbvtVolume::FromMM(aabbMin, aabbMax);

	//bproxy->aabb			=	DbvtVolume::FromMM(aabbMin,aabbMax);
	proxy->stage = m_stageCurrent;
	proxy->m_uniqueId = ++m_gid;
	proxy->leaf = m_sets[0].insert(aabb, proxy);
	listappend(proxy, m_stageRoots[m_stageCurrent]);
	if (!m_deferedcollide)
	{
		DbvtTreeCollider collider(this);
		collider.proxy = proxy;
		m_sets[0].collideTV(m_sets[0].m_root, aabb, collider);
		m_sets[1].collideTV(m_sets[1].m_root, aabb, collider);
	}
	return (proxy);
}

//
void DbvtBroadphase::destroyProxy(BroadphaseProxy* absproxy,
									Dispatcher* dispatcher)
{
	DbvtProxy* proxy = (DbvtProxy*)absproxy;
	if (proxy->stage == STAGECOUNT)
		m_sets[1].remove(proxy->leaf);
	else
		m_sets[0].remove(proxy->leaf);
	listremove(proxy, m_stageRoots[proxy->stage]);
	m_paircache->removeOverlappingPairsContainingProxy(proxy, dispatcher);
	AlignedFree(proxy);
	m_needcleanup = true;
}

void DbvtBroadphase::getAabb(BroadphaseProxy* absproxy, Vec3& aabbMin, Vec3& aabbMax) const
{
	DbvtProxy* proxy = (DbvtProxy*)absproxy;
	aabbMin = proxy->m_aabbMin;
	aabbMax = proxy->m_aabbMax;
}

struct BroadphaseRayTester : Dbvt::ICollide
{
	BroadphaseRayCallback& m_rayCallback;
	BroadphaseRayTester(BroadphaseRayCallback& orgCallback)
		: m_rayCallback(orgCallback)
	{
	}
	void Process(const DbvtNode* leaf)
	{
		DbvtProxy* proxy = (DbvtProxy*)leaf->data;
		m_rayCallback.process(proxy);
	}
};

void DbvtBroadphase::rayTest(const Vec3& rayFrom, const Vec3& rayTo, BroadphaseRayCallback& rayCallback, const Vec3& aabbMin, const Vec3& aabbMax)
{
	BroadphaseRayTester callback(rayCallback);
	AlignedObjectArray<const DbvtNode*>* stack = &m_rayTestStacks[0];
#if DRX3D_THREADSAFE
	// for this function to be threadsafe, each thread must have a separate copy
	// of this stack.  This could be thread-local static to avoid dynamic allocations,
	// instead of just a local.
	i32 threadIndex = GetCurrentThreadIndex();
	AlignedObjectArray<const DbvtNode*> localStack;
	//todo(erwincoumans, "why do we get tsan issue here?")
	if (0)//threadIndex < m_rayTestStacks.size())
	//if (threadIndex < m_rayTestStacks.size())
	{
		// use per-thread preallocated stack if possible to avoid dynamic allocations
		stack = &m_rayTestStacks[threadIndex];
	}
	else
	{
		stack = &localStack;
	}
#endif

	m_sets[0].rayTestInternal(m_sets[0].m_root,
							  rayFrom,
							  rayTo,
							  rayCallback.m_rayDirectionInverse,
							  rayCallback.m_signs,
							  rayCallback.m_lambda_max,
							  aabbMin,
							  aabbMax,
							  *stack,
							  callback);

	m_sets[1].rayTestInternal(m_sets[1].m_root,
							  rayFrom,
							  rayTo,
							  rayCallback.m_rayDirectionInverse,
							  rayCallback.m_signs,
							  rayCallback.m_lambda_max,
							  aabbMin,
							  aabbMax,
							  *stack,
							  callback);
}

struct BroadphaseAabbTester : Dbvt::ICollide
{
	BroadphaseAabbCallback& m_aabbCallback;
	BroadphaseAabbTester(BroadphaseAabbCallback& orgCallback)
		: m_aabbCallback(orgCallback)
	{
	}
	void Process(const DbvtNode* leaf)
	{
		DbvtProxy* proxy = (DbvtProxy*)leaf->data;
		m_aabbCallback.process(proxy);
	}
};

void DbvtBroadphase::aabbTest(const Vec3& aabbMin, const Vec3& aabbMax, BroadphaseAabbCallback& aabbCallback)
{
	BroadphaseAabbTester callback(aabbCallback);

	const ATTRIBUTE_ALIGNED16(DbvtVolume) bounds = DbvtVolume::FromMM(aabbMin, aabbMax);
	//process all children, that overlap with  the given AABB bounds
	m_sets[0].collideTV(m_sets[0].m_root, bounds, callback);
	m_sets[1].collideTV(m_sets[1].m_root, bounds, callback);
}

//
void DbvtBroadphase::setAabb(BroadphaseProxy* absproxy,
							   const Vec3& aabbMin,
							   const Vec3& aabbMax,
							   Dispatcher* /*dispatcher*/)
{
	DbvtProxy* proxy = (DbvtProxy*)absproxy;
	ATTRIBUTE_ALIGNED16(DbvtVolume)
	aabb = DbvtVolume::FromMM(aabbMin, aabbMax);
#if DBVT_BP_PREVENTFALSEUPDATE
	if (NotEqual(aabb, proxy->leaf->volume))
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
			if (Intersect(proxy->leaf->volume, aabb))
			{ /* Moving				*/

				const Vec3 delta = aabbMin - proxy->m_aabbMin;
				Vec3 velocity(((proxy->m_aabbMax - proxy->m_aabbMin) / 2) * m_prediction);
				if (delta[0] < 0) velocity[0] = -velocity[0];
				if (delta[1] < 0) velocity[1] = -velocity[1];
				if (delta[2] < 0) velocity[2] = -velocity[2];
				if (
					m_sets[0].update(proxy->leaf, aabb, velocity, gDbvtMargin)

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
		listremove(proxy, m_stageRoots[proxy->stage]);
		proxy->m_aabbMin = aabbMin;
		proxy->m_aabbMax = aabbMax;
		proxy->stage = m_stageCurrent;
		listappend(proxy, m_stageRoots[m_stageCurrent]);
		if (docollide)
		{
			m_needcleanup = true;
			if (!m_deferedcollide)
			{
				DbvtTreeCollider collider(this);
				m_sets[1].collideTTpersistentStack(m_sets[1].m_root, proxy->leaf, collider);
				m_sets[0].collideTTpersistentStack(m_sets[0].m_root, proxy->leaf, collider);
			}
		}
	}
}

//
void DbvtBroadphase::setAabbForceUpdate(BroadphaseProxy* absproxy,
										  const Vec3& aabbMin,
										  const Vec3& aabbMax,
										  Dispatcher* /*dispatcher*/)
{
	DbvtProxy* proxy = (DbvtProxy*)absproxy;
	ATTRIBUTE_ALIGNED16(DbvtVolume)
	aabb = DbvtVolume::FromMM(aabbMin, aabbMax);
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
	listremove(proxy, m_stageRoots[proxy->stage]);
	proxy->m_aabbMin = aabbMin;
	proxy->m_aabbMax = aabbMax;
	proxy->stage = m_stageCurrent;
	listappend(proxy, m_stageRoots[m_stageCurrent]);
	if (docollide)
	{
		m_needcleanup = true;
		if (!m_deferedcollide)
		{
			DbvtTreeCollider collider(this);
			m_sets[1].collideTTpersistentStack(m_sets[1].m_root, proxy->leaf, collider);
			m_sets[0].collideTTpersistentStack(m_sets[0].m_root, proxy->leaf, collider);
		}
	}
}

//
void DbvtBroadphase::calculateOverlappingPairs(Dispatcher* dispatcher)
{
	collide(dispatcher);
#if DBVT_BP_PROFILE
	if (0 == (m_pid % DBVT_BP_PROFILING_RATE))
	{
		printf("fixed(%u) dynamics(%u) pairs(%u)\r\n", m_sets[1].m_leaves, m_sets[0].m_leaves, m_paircache->getNumOverlappingPairs());
		u32 total = m_profiling.m_total;
		if (total <= 0) total = 1;
		printf("ddcollide: %u%% (%uus)\r\n", (50 + m_profiling.m_ddcollide * 100) / total, m_profiling.m_ddcollide / DBVT_BP_PROFILING_RATE);
		printf("fdcollide: %u%% (%uus)\r\n", (50 + m_profiling.m_fdcollide * 100) / total, m_profiling.m_fdcollide / DBVT_BP_PROFILING_RATE);
		printf("cleanup:   %u%% (%uus)\r\n", (50 + m_profiling.m_cleanup * 100) / total, m_profiling.m_cleanup / DBVT_BP_PROFILING_RATE);
		printf("total:     %uus\r\n", total / DBVT_BP_PROFILING_RATE);
		const u64 sum = m_profiling.m_ddcollide +
								  m_profiling.m_fdcollide +
								  m_profiling.m_cleanup;
		printf("leaked: %u%% (%uus)\r\n", 100 - ((50 + sum * 100) / total), (total - sum) / DBVT_BP_PROFILING_RATE);
		printf("job counts: %u%%\r\n", (m_profiling.m_jobcount * 100) / ((m_sets[0].m_leaves + m_sets[1].m_leaves) * DBVT_BP_PROFILING_RATE));
		clear(m_profiling);
		m_clock.reset();
	}
#endif

	performDeferredRemoval(dispatcher);
}

void DbvtBroadphase::performDeferredRemoval(Dispatcher* dispatcher)
{
	if (m_paircache->hasDeferredRemoval())
	{
		BroadphasePairArray& overlappingPairArray = m_paircache->getOverlappingPairArray();

		//perform a sort, to find duplicates and to sort 'invalid' pairs to the end
		overlappingPairArray.quickSort(BroadphasePairSortPredicate());

		i32 invalidPair = 0;

		i32 i;

		BroadphasePair previousPair;
		previousPair.m_pProxy0 = 0;
		previousPair.m_pProxy1 = 0;
		previousPair.m_algorithm = 0;

		for (i = 0; i < overlappingPairArray.size(); i++)
		{
			BroadphasePair& pair = overlappingPairArray[i];

			bool isDuplicate = (pair == previousPair);

			previousPair = pair;

			bool needsRemoval = false;

			if (!isDuplicate)
			{
				//important to perform AABB check that is consistent with the broadphase
				DbvtProxy* pa = (DbvtProxy*)pair.m_pProxy0;
				DbvtProxy* pb = (DbvtProxy*)pair.m_pProxy1;
				bool hasOverlap = Intersect(pa->leaf->volume, pb->leaf->volume);

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
				Assert(!pair.m_algorithm);
			}

			if (needsRemoval)
			{
				m_paircache->cleanOverlappingPair(pair, dispatcher);

				pair.m_pProxy0 = 0;
				pair.m_pProxy1 = 0;
				invalidPair++;
			}
		}

		//perform a sort, to sort 'invalid' pairs to the end
		overlappingPairArray.quickSort(BroadphasePairSortPredicate());
		overlappingPairArray.resize(overlappingPairArray.size() - invalidPair);
	}
}

//
void DbvtBroadphase::collide(Dispatcher* dispatcher)
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

	SPC(m_profiling.m_total);
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
	DbvtProxy* current = m_stageRoots[m_stageCurrent];
	if (current)
	{
#if DBVT_BP_ACCURATESLEEPING
		DbvtTreeCollider collider(this);
#endif
		do
		{
			DbvtProxy* next = current->links[1];
			listremove(current, m_stageRoots[current->stage]);
			listappend(current, m_stageRoots[STAGECOUNT]);
#if DBVT_BP_ACCURATESLEEPING
			m_paircache->removeOverlappingPairsContainingProxy(current, dispatcher);
			collider.proxy = current;
			Dbvt::collideTV(m_sets[0].m_root, current->aabb, collider);
			Dbvt::collideTV(m_sets[1].m_root, current->aabb, collider);
#endif
			m_sets[0].remove(current->leaf);
			ATTRIBUTE_ALIGNED16(DbvtVolume)
			curAabb = DbvtVolume::FromMM(current->m_aabbMin, current->m_aabbMax);
			current->leaf = m_sets[1].insert(curAabb, current);
			current->stage = STAGECOUNT;
			current = next;
		} while (current);
		m_fixedleft = m_sets[1].m_leaves;
		m_needcleanup = true;
	}
	/* collide dynamics		*/
	{
		DbvtTreeCollider collider(this);
		if (m_deferedcollide)
		{
			SPC(m_profiling.m_fdcollide);
			m_sets[0].collideTTpersistentStack(m_sets[0].m_root, m_sets[1].m_root, collider);
		}
		if (m_deferedcollide)
		{
			SPC(m_profiling.m_ddcollide);
			m_sets[0].collideTTpersistentStack(m_sets[0].m_root, m_sets[0].m_root, collider);
		}
	}
	/* clean up				*/
	if (m_needcleanup)
	{
		SPC(m_profiling.m_cleanup);
		BroadphasePairArray& pairs = m_paircache->getOverlappingPairArray();
		if (pairs.size() > 0)
		{
			i32 ni = d3Min(pairs.size(), d3Max<i32>(m_newpairs, (pairs.size() * m_cupdates) / 100));
			for (i32 i = 0; i < ni; ++i)
			{
				BroadphasePair& p = pairs[(m_cid + i) % pairs.size()];
				DbvtProxy* pa = (DbvtProxy*)p.m_pProxy0;
				DbvtProxy* pb = (DbvtProxy*)p.m_pProxy1;
				if (!Intersect(pa->leaf->volume, pb->leaf->volume))
				{
#if DBVT_BP_SORTPAIRS
					if (pa->m_uniqueId > pb->m_uniqueId)
						Swap(pa, pb);
#endif
					m_paircache->removeOverlappingPair(pa, pb, dispatcher);
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
		m_updates_ratio = m_updates_done / (Scalar)m_updates_call;
	}
	else
	{
		m_updates_ratio = 0;
	}
	m_updates_done /= 2;
	m_updates_call /= 2;
}

//
void DbvtBroadphase::optimize()
{
	m_sets[0].optimizeTopDown();
	m_sets[1].optimizeTopDown();
}

//
OverlappingPairCache* DbvtBroadphase::getOverlappingPairCache()
{
	return (m_paircache);
}

//
const OverlappingPairCache* DbvtBroadphase::getOverlappingPairCache() const
{
	return (m_paircache);
}

//
void DbvtBroadphase::getBroadphaseAabb(Vec3& aabbMin, Vec3& aabbMax) const
{
	ATTRIBUTE_ALIGNED16(DbvtVolume)
	bounds;

	if (!m_sets[0].empty())
		if (!m_sets[1].empty())
			Merge(m_sets[0].m_root->volume,
				  m_sets[1].m_root->volume, bounds);
		else
			bounds = m_sets[0].m_root->volume;
	else if (!m_sets[1].empty())
		bounds = m_sets[1].m_root->volume;
	else
		bounds = DbvtVolume::FromCR(Vec3(0, 0, 0), 0);
	aabbMin = bounds.Mins();
	aabbMax = bounds.Maxs();
}

void DbvtBroadphase::resetPool(Dispatcher* dispatcher)
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

		m_gid = 0;
		m_pid = 0;
		m_cid = 0;
		for (i32 i = 0; i <= STAGECOUNT; ++i)
		{
			m_stageRoots[i] = 0;
		}
	}
}

//
void DbvtBroadphase::printStats()
{
}

//
#if DBVT_BP_ENABLE_BENCHMARK

struct BroadphaseBenchmark
{
	struct Experiment
	{
		tukk name;
		i32 object_count;
		i32 update_count;
		i32 spawn_count;
		i32 iterations;
		Scalar speed;
		Scalar amplitude;
	};
	struct Object
	{
		Vec3 center;
		Vec3 extents;
		BroadphaseProxy* proxy;
		Scalar time;
		void update(Scalar speed, Scalar amplitude, BroadphaseInterface* pbi)
		{
			time += speed;
			center[0] = Cos(time * (Scalar)2.17) * amplitude +
						Sin(time) * amplitude / 2;
			center[1] = Cos(time * (Scalar)1.38) * amplitude +
						Sin(time) * amplitude;
			center[2] = Sin(time * (Scalar)0.777) * amplitude;
			pbi->setAabb(proxy, center - extents, center + extents, 0);
		}
	};
	static i32 UnsignedRand(i32 range = RAND_MAX - 1) { return (rand() % (range + 1)); }
	static Scalar UnitRand() { return (UnsignedRand(16384) / (Scalar)16384); }
	static void OutputTime(tukk name, Clock& c, unsigned count = 0)
	{
		const u64 us = c.getTimeMicroseconds();
		const u64 ms = (us + 500) / 1000;
		const Scalar sec = us / (Scalar)(1000 * 1000);
		if (count > 0)
			printf("%s : %u us (%u ms), %.2f/s\r\n", name, us, ms, count / sec);
		else
			printf("%s : %u us (%u ms)\r\n", name, us, ms);
	}
};

void DbvtBroadphase::benchmark(BroadphaseInterface* pbi)
{
	static const BroadphaseBenchmark::Experiment experiments[] =
		{
			{"1024o.10%", 1024, 10, 0, 8192, (Scalar)0.005, (Scalar)100},
			/*{"4096o.10%",4096,10,0,8192,(Scalar)0.005,(Scalar)100},
		{"8192o.10%",8192,10,0,8192,(Scalar)0.005,(Scalar)100},*/
		};
	static i32k nexperiments = sizeof(experiments) / sizeof(experiments[0]);
	AlignedObjectArray<BroadphaseBenchmark::Object*> objects;
	Clock wallclock;
	/* Begin			*/
	for (i32 iexp = 0; iexp < nexperiments; ++iexp)
	{
		const BroadphaseBenchmark::Experiment& experiment = experiments[iexp];
		i32k object_count = experiment.object_count;
		i32k update_count = (object_count * experiment.update_count) / 100;
		i32k spawn_count = (object_count * experiment.spawn_count) / 100;
		const Scalar speed = experiment.speed;
		const Scalar amplitude = experiment.amplitude;
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
			BroadphaseBenchmark::Object* po = new BroadphaseBenchmark::Object();
			po->center[0] = BroadphaseBenchmark::UnitRand() * 50;
			po->center[1] = BroadphaseBenchmark::UnitRand() * 50;
			po->center[2] = BroadphaseBenchmark::UnitRand() * 50;
			po->extents[0] = BroadphaseBenchmark::UnitRand() * 2 + 2;
			po->extents[1] = BroadphaseBenchmark::UnitRand() * 2 + 2;
			po->extents[2] = BroadphaseBenchmark::UnitRand() * 2 + 2;
			po->time = BroadphaseBenchmark::UnitRand() * 2000;
			po->proxy = pbi->createProxy(po->center - po->extents, po->center + po->extents, 0, po, 1, 1, 0, 0);
			objects.push_back(po);
		}
		BroadphaseBenchmark::OutputTime("\tInitialization", wallclock);
		/* First update		*/
		wallclock.reset();
		for (i32 i = 0; i < objects.size(); ++i)
		{
			objects[i]->update(speed, amplitude, pbi);
		}
		BroadphaseBenchmark::OutputTime("\tFirst update", wallclock);
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
		BroadphaseBenchmark::OutputTime("\tUpdate", wallclock, experiment.iterations);
		/* Clean up			*/
		wallclock.reset();
		for (i32 i = 0; i < objects.size(); ++i)
		{
			pbi->destroyProxy(objects[i]->proxy, 0);
			delete objects[i];
		}
		objects.resize(0);
		BroadphaseBenchmark::OutputTime("\tRelease", wallclock);
	}
}
#else
void DbvtBroadphase::benchmark(BroadphaseInterface*)
{
}
#endif

#if DBVT_BP_PROFILE
#undef SPC
#endif
