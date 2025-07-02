#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>

//
typedef AlignedObjectArray<DbvtNode*> tNodeArray;
typedef AlignedObjectArray<const DbvtNode*> tConstNodeArray;

//
struct DbvtNodeEnumerator : Dbvt::ICollide
{
	tConstNodeArray nodes;
	void Process(const DbvtNode* n) { nodes.push_back(n); }
};

//
static DBVT_INLINE i32 indexof(const DbvtNode* node)
{
	return (node->parent->childs[1] == node);
}

//
static DBVT_INLINE DbvtVolume merge(const DbvtVolume& a,
									  const DbvtVolume& b)
{
#ifdef DRX3D_USE_SSE
	ATTRIBUTE_ALIGNED16(char locals[sizeof(DbvtAabbMm)]);
	DbvtVolume* ptr = (DbvtVolume*)locals;
	DbvtVolume& res = *ptr;
#else
	DbvtVolume res;
#endif
	Merge(a, b, res);
	return (res);
}

// volume+edge lengths
static DBVT_INLINE Scalar size(const DbvtVolume& a)
{
	const Vec3 edges = a.Lengths();
	return (edges.x() * edges.y() * edges.z() +
			edges.x() + edges.y() + edges.z());
}

//
static void getmaxdepth(const DbvtNode* node, i32 depth, i32& maxdepth)
{
	if (node->isinternal())
	{
		getmaxdepth(node->childs[0], depth + 1, maxdepth);
		getmaxdepth(node->childs[1], depth + 1, maxdepth);
	}
	else
		maxdepth = d3Max(maxdepth, depth);
}

//
static DBVT_INLINE void deletenode(Dbvt* pdbvt,
								   DbvtNode* node)
{
	AlignedFree(pdbvt->m_free);
	pdbvt->m_free = node;
}

//
static void recursedeletenode(Dbvt* pdbvt,
							  DbvtNode* node)
{
	if (node == 0) return;
	if (!node->isleaf())
	{
		recursedeletenode(pdbvt, node->childs[0]);
		recursedeletenode(pdbvt, node->childs[1]);
	}
	if (node == pdbvt->m_root) pdbvt->m_root = 0;
	deletenode(pdbvt, node);
}

//
static DBVT_INLINE DbvtNode* createnode(Dbvt* pdbvt,
										  DbvtNode* parent,
										  uk data)
{
	DbvtNode* node;
	if (pdbvt->m_free)
	{
		node = pdbvt->m_free;
		pdbvt->m_free = 0;
	}
	else
	{
		node = new (AlignedAlloc(sizeof(DbvtNode), 16)) DbvtNode();
	}
	node->parent = parent;
	node->data = data;
	node->childs[1] = 0;
	return (node);
}

//
static DBVT_INLINE DbvtNode* createnode(Dbvt* pdbvt,
										  DbvtNode* parent,
										  const DbvtVolume& volume,
										  uk data)
{
	DbvtNode* node = createnode(pdbvt, parent, data);
	node->volume = volume;
	return (node);
}

//
static DBVT_INLINE DbvtNode* createnode(Dbvt* pdbvt,
										  DbvtNode* parent,
										  const DbvtVolume& volume0,
										  const DbvtVolume& volume1,
										  uk data)
{
	DbvtNode* node = createnode(pdbvt, parent, data);
	Merge(volume0, volume1, node->volume);
	return (node);
}

//
static void insertleaf(Dbvt* pdbvt,
					   DbvtNode* root,
					   DbvtNode* leaf)
{
	if (!pdbvt->m_root)
	{
		pdbvt->m_root = leaf;
		leaf->parent = 0;
	}
	else
	{
		if (!root->isleaf())
		{
			do
			{
				root = root->childs[Select(leaf->volume,
										   root->childs[0]->volume,
										   root->childs[1]->volume)];
			} while (!root->isleaf());
		}
		DbvtNode* prev = root->parent;
		DbvtNode* node = createnode(pdbvt, prev, leaf->volume, root->volume, 0);
		if (prev)
		{
			prev->childs[indexof(root)] = node;
			node->childs[0] = root;
			root->parent = node;
			node->childs[1] = leaf;
			leaf->parent = node;
			do
			{
				if (!prev->volume.Contain(node->volume))
					Merge(prev->childs[0]->volume, prev->childs[1]->volume, prev->volume);
				else
					break;
				node = prev;
			} while (0 != (prev = node->parent));
		}
		else
		{
			node->childs[0] = root;
			root->parent = node;
			node->childs[1] = leaf;
			leaf->parent = node;
			pdbvt->m_root = node;
		}
	}
}

//
static DbvtNode* removeleaf(Dbvt* pdbvt,
							  DbvtNode* leaf)
{
	if (leaf == pdbvt->m_root)
	{
		pdbvt->m_root = 0;
		return (0);
	}
	else
	{
		DbvtNode* parent = leaf->parent;
		DbvtNode* prev = parent->parent;
		DbvtNode* sibling = parent->childs[1 - indexof(leaf)];
		if (prev)
		{
			prev->childs[indexof(parent)] = sibling;
			sibling->parent = prev;
			deletenode(pdbvt, parent);
			while (prev)
			{
				const DbvtVolume pb = prev->volume;
				Merge(prev->childs[0]->volume, prev->childs[1]->volume, prev->volume);
				if (NotEqual(pb, prev->volume))
				{
					prev = prev->parent;
				}
				else
					break;
			}
			return (prev ? prev : pdbvt->m_root);
		}
		else
		{
			pdbvt->m_root = sibling;
			sibling->parent = 0;
			deletenode(pdbvt, parent);
			return (pdbvt->m_root);
		}
	}
}

//
static void fetchleaves(Dbvt* pdbvt,
						DbvtNode* root,
						tNodeArray& leaves,
						i32 depth = -1)
{
	if (root->isinternal() && depth)
	{
		fetchleaves(pdbvt, root->childs[0], leaves, depth - 1);
		fetchleaves(pdbvt, root->childs[1], leaves, depth - 1);
		deletenode(pdbvt, root);
	}
	else
	{
		leaves.push_back(root);
	}
}

//
static bool leftOfAxis(const DbvtNode* node,
					   const Vec3& org,
					   const Vec3& axis)
{
	return Dot(axis, node->volume.Center() - org) <= 0;
}

// Partitions leaves such that leaves[0, n) are on the
// left of axis, and leaves[n, count) are on the right
// of axis. returns N.
static i32 split(DbvtNode** leaves,
				 i32 count,
				 const Vec3& org,
				 const Vec3& axis)
{
	i32 begin = 0;
	i32 end = count;
	for (;;)
	{
		while (begin != end && leftOfAxis(leaves[begin], org, axis))
		{
			++begin;
		}

		if (begin == end)
		{
			break;
		}

		while (begin != end && !leftOfAxis(leaves[end - 1], org, axis))
		{
			--end;
		}

		if (begin == end)
		{
			break;
		}

		// swap out of place nodes
		--end;
		DbvtNode* temp = leaves[begin];
		leaves[begin] = leaves[end];
		leaves[end] = temp;
		++begin;
	}

	return begin;
}

//
static DbvtVolume bounds(DbvtNode** leaves,
						   i32 count)
{
#ifdef DRX3D_USE_SSE
	ATTRIBUTE_ALIGNED16(char locals[sizeof(DbvtVolume)]);
	DbvtVolume* ptr = (DbvtVolume*)locals;
	DbvtVolume& volume = *ptr;
	volume = leaves[0]->volume;
#else
	DbvtVolume volume = leaves[0]->volume;
#endif
	for (i32 i = 1, ni = count; i < ni; ++i)
	{
		Merge(volume, leaves[i]->volume, volume);
	}
	return (volume);
}

//
static void bottomup(Dbvt* pdbvt,
					 DbvtNode** leaves,
					 i32 count)
{
	while (count > 1)
	{
		Scalar minsize = SIMD_INFINITY;
		i32 minidx[2] = {-1, -1};
		for (i32 i = 0; i < count; ++i)
		{
			for (i32 j = i + 1; j < count; ++j)
			{
				const Scalar sz = size(merge(leaves[i]->volume, leaves[j]->volume));
				if (sz < minsize)
				{
					minsize = sz;
					minidx[0] = i;
					minidx[1] = j;
				}
			}
		}
		DbvtNode* n[] = {leaves[minidx[0]], leaves[minidx[1]]};
		DbvtNode* p = createnode(pdbvt, 0, n[0]->volume, n[1]->volume, 0);
		p->childs[0] = n[0];
		p->childs[1] = n[1];
		n[0]->parent = p;
		n[1]->parent = p;
		leaves[minidx[0]] = p;
		leaves[minidx[1]] = leaves[count - 1];
		--count;
	}
}

//
static DbvtNode* topdown(Dbvt* pdbvt,
						   DbvtNode** leaves,
						   i32 count,
						   i32 bu_treshold)
{
	static const Vec3 axis[] = {Vec3(1, 0, 0),
									 Vec3(0, 1, 0),
									 Vec3(0, 0, 1)};
	Assert(bu_treshold > 2);
	if (count > 1)
	{
		if (count > bu_treshold)
		{
			const DbvtVolume vol = bounds(leaves, count);
			const Vec3 org = vol.Center();
			i32 partition;
			i32 bestaxis = -1;
			i32 bestmidp = count;
			i32 splitcount[3][2] = {{0, 0}, {0, 0}, {0, 0}};
			i32 i;
			for (i = 0; i < count; ++i)
			{
				const Vec3 x = leaves[i]->volume.Center() - org;
				for (i32 j = 0; j < 3; ++j)
				{
					++splitcount[j][Dot(x, axis[j]) > 0 ? 1 : 0];
				}
			}
			for (i = 0; i < 3; ++i)
			{
				if ((splitcount[i][0] > 0) && (splitcount[i][1] > 0))
				{
					i32k midp = (i32)Fabs(Scalar(splitcount[i][0] - splitcount[i][1]));
					if (midp < bestmidp)
					{
						bestaxis = i;
						bestmidp = midp;
					}
				}
			}
			if (bestaxis >= 0)
			{
				partition = split(leaves, count, org, axis[bestaxis]);
				Assert(partition != 0 && partition != count);
			}
			else
			{
				partition = count / 2 + 1;
			}
			DbvtNode* node = createnode(pdbvt, 0, vol, 0);
			node->childs[0] = topdown(pdbvt, &leaves[0], partition, bu_treshold);
			node->childs[1] = topdown(pdbvt, &leaves[partition], count - partition, bu_treshold);
			node->childs[0]->parent = node;
			node->childs[1]->parent = node;
			return (node);
		}
		else
		{
			bottomup(pdbvt, leaves, count);
			return (leaves[0]);
		}
	}
	return (leaves[0]);
}

//
static DBVT_INLINE DbvtNode* sort(DbvtNode* n, DbvtNode*& r)
{
	DbvtNode* p = n->parent;
	Assert(n->isinternal());
	if (p > n)
	{
		i32k i = indexof(n);
		i32k j = 1 - i;
		DbvtNode* s = p->childs[j];
		DbvtNode* q = p->parent;
		Assert(n == p->childs[i]);
		if (q)
			q->childs[indexof(p)] = n;
		else
			r = n;
		s->parent = n;
		p->parent = n;
		n->parent = q;
		p->childs[0] = n->childs[0];
		p->childs[1] = n->childs[1];
		n->childs[0]->parent = p;
		n->childs[1]->parent = p;
		n->childs[i] = p;
		n->childs[j] = s;
		Swap(p->volume, n->volume);
		return (p);
	}
	return (n);
}

#if 0
static DBVT_INLINE DbvtNode*	walkup(DbvtNode* n,i32 count)
{
	while(n&&(count--)) n=n->parent;
	return(n);
}
#endif

//
// Api
//

//
Dbvt::Dbvt()
{
	m_root = 0;
	m_free = 0;
	m_lkhd = -1;
	m_leaves = 0;
	m_opath = 0;
}

//
Dbvt::~Dbvt()
{
	clear();
}

//
void Dbvt::clear()
{
	if (m_root)
		recursedeletenode(this, m_root);
	AlignedFree(m_free);
	m_free = 0;
	m_lkhd = -1;
	m_stkStack.clear();
	m_opath = 0;
}

//
void Dbvt::optimizeBottomUp()
{
	if (m_root)
	{
		tNodeArray leaves;
		leaves.reserve(m_leaves);
		fetchleaves(this, m_root, leaves);
		bottomup(this, &leaves[0], leaves.size());
		m_root = leaves[0];
	}
}

//
void Dbvt::optimizeTopDown(i32 bu_treshold)
{
	if (m_root)
	{
		tNodeArray leaves;
		leaves.reserve(m_leaves);
		fetchleaves(this, m_root, leaves);
		m_root = topdown(this, &leaves[0], leaves.size(), bu_treshold);
	}
}

//
void Dbvt::optimizeIncremental(i32 passes)
{
	if (passes < 0) passes = m_leaves;
	if (m_root && (passes > 0))
	{
		do
		{
			DbvtNode* node = m_root;
			unsigned bit = 0;
			while (node->isinternal())
			{
				node = sort(node, m_root)->childs[(m_opath >> bit) & 1];
				bit = (bit + 1) & (sizeof(unsigned) * 8 - 1);
			}
			update(node);
			++m_opath;
		} while (--passes);
	}
}

//
DbvtNode* Dbvt::insert(const DbvtVolume& volume, uk data)
{
	DbvtNode* leaf = createnode(this, 0, volume, data);
	insertleaf(this, m_root, leaf);
	++m_leaves;
	return (leaf);
}

//
void Dbvt::update(DbvtNode* leaf, i32 lookahead)
{
	DbvtNode* root = removeleaf(this, leaf);
	if (root)
	{
		if (lookahead >= 0)
		{
			for (i32 i = 0; (i < lookahead) && root->parent; ++i)
			{
				root = root->parent;
			}
		}
		else
			root = m_root;
	}
	insertleaf(this, root, leaf);
}

//
void Dbvt::update(DbvtNode* leaf, DbvtVolume& volume)
{
	DbvtNode* root = removeleaf(this, leaf);
	if (root)
	{
		if (m_lkhd >= 0)
		{
			for (i32 i = 0; (i < m_lkhd) && root->parent; ++i)
			{
				root = root->parent;
			}
		}
		else
			root = m_root;
	}
	leaf->volume = volume;
	insertleaf(this, root, leaf);
}

//
bool Dbvt::update(DbvtNode* leaf, DbvtVolume& volume, const Vec3& velocity, Scalar margin)
{
	if (leaf->volume.Contain(volume)) return (false);
	volume.Expand(Vec3(margin, margin, margin));
	volume.SignedExpand(velocity);
	update(leaf, volume);
	return (true);
}

//
bool Dbvt::update(DbvtNode* leaf, DbvtVolume& volume, const Vec3& velocity)
{
	if (leaf->volume.Contain(volume)) return (false);
	volume.SignedExpand(velocity);
	update(leaf, volume);
	return (true);
}

//
bool Dbvt::update(DbvtNode* leaf, DbvtVolume& volume, Scalar margin)
{
	if (leaf->volume.Contain(volume)) return (false);
	volume.Expand(Vec3(margin, margin, margin));
	update(leaf, volume);
	return (true);
}

//
void Dbvt::remove(DbvtNode* leaf)
{
	removeleaf(this, leaf);
	deletenode(this, leaf);
	--m_leaves;
}

//
void Dbvt::write(IWriter* iwriter) const
{
	DbvtNodeEnumerator nodes;
	nodes.nodes.reserve(m_leaves * 2);
	enumNodes(m_root, nodes);
	iwriter->Prepare(m_root, nodes.nodes.size());
	for (i32 i = 0; i < nodes.nodes.size(); ++i)
	{
		const DbvtNode* n = nodes.nodes[i];
		i32 p = -1;
		if (n->parent) p = nodes.nodes.findLinearSearch(n->parent);
		if (n->isinternal())
		{
			i32k c0 = nodes.nodes.findLinearSearch(n->childs[0]);
			i32k c1 = nodes.nodes.findLinearSearch(n->childs[1]);
			iwriter->WriteNode(n, i, p, c0, c1);
		}
		else
		{
			iwriter->WriteLeaf(n, i, p);
		}
	}
}

//
void Dbvt::clone(Dbvt& dest, IClone* iclone) const
{
	dest.clear();
	if (m_root != 0)
	{
		AlignedObjectArray<sStkCLN> stack;
		stack.reserve(m_leaves);
		stack.push_back(sStkCLN(m_root, 0));
		do
		{
			i32k i = stack.size() - 1;
			const sStkCLN e = stack[i];
			DbvtNode* n = createnode(&dest, e.parent, e.node->volume, e.node->data);
			stack.pop_back();
			if (e.parent != 0)
				e.parent->childs[i & 1] = n;
			else
				dest.m_root = n;
			if (e.node->isinternal())
			{
				stack.push_back(sStkCLN(e.node->childs[0], n));
				stack.push_back(sStkCLN(e.node->childs[1], n));
			}
			else
			{
				iclone->CloneLeaf(n);
			}
		} while (stack.size() > 0);
	}
}

//
i32 Dbvt::maxdepth(const DbvtNode* node)
{
	i32 depth = 0;
	if (node) getmaxdepth(node, 1, depth);
	return (depth);
}

//
i32 Dbvt::countLeaves(const DbvtNode* node)
{
	if (node->isinternal())
		return (countLeaves(node->childs[0]) + countLeaves(node->childs[1]));
	else
		return (1);
}

//
void Dbvt::extractLeaves(const DbvtNode* node, AlignedObjectArray<const DbvtNode*>& leaves)
{
	if (node->isinternal())
	{
		extractLeaves(node->childs[0], leaves);
		extractLeaves(node->childs[1], leaves);
	}
	else
	{
		leaves.push_back(node);
	}
}

//
#if DBVT_ENABLE_BENCHMARK

#include <stdio.h>
#include <stdlib.h>
#include <drx3D/Maths/Linear/QuickProf.h>

/*
q6600,2.4ghz

/Ox /Ob2 /Oi /Ot /I "." /I "..\.." /I "..\..\src" /D "NDEBUG" /D "_LIB" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "WIN32"
/GF /FD /MT /GS- /Gy /arch:SSE2 /Zc:wchar_t- /Fp"..\..\out\release8\build\libbulletcollision\libbulletcollision.pch"
/Fo"..\..\out\release8\build\libbulletcollision\\"
/Fd"..\..\out\release8\build\libbulletcollision\bulletcollision.pdb"
/W3 /nologo /c /Wp64 /Zi /errorReport:prompt

Benchmarking dbvt...
World scale: 100.000000
Extents base: 1.000000
Extents range: 4.000000
Leaves: 8192
sizeof(DbvtVolume): 32 bytes
sizeof(DbvtNode):   44 bytes
[1] DbvtVolume intersections: 3499 ms (-1%)
[2] DbvtVolume merges: 1934 ms (0%)
[3] Dbvt::collideTT: 5485 ms (-21%)
[4] Dbvt::collideTT self: 2814 ms (-20%)
[5] Dbvt::collideTT xform: 7379 ms (-1%)
[6] Dbvt::collideTT xform,self: 7270 ms (-2%)
[7] Dbvt::rayTest: 6314 ms (0%),(332143 r/s)
[8] insert/remove: 2093 ms (0%),(1001983 ir/s)
[9] updates (teleport): 1879 ms (-3%),(1116100 u/s)
[10] updates (jitter): 1244 ms (-4%),(1685813 u/s)
[11] optimize (incremental): 2514 ms (0%),(1668000 o/s)
[12] DbvtVolume notequal: 3659 ms (0%)
[13] culling(OCL+fullsort): 2218 ms (0%),(461 t/s)
[14] culling(OCL+qsort): 3688 ms (5%),(2221 t/s)
[15] culling(KDOP+qsort): 1139 ms (-1%),(7192 t/s)
[16] insert/remove batch(256): 5092 ms (0%),(823704 bir/s)
[17] DbvtVolume select: 3419 ms (0%)
*/

struct DbvtBenchmark
{
	struct NilPolicy : Dbvt::ICollide
	{
		NilPolicy() : m_pcount(0), m_depth(-SIMD_INFINITY), m_checksort(true) {}
		void Process(const DbvtNode*, const DbvtNode*) { ++m_pcount; }
		void Process(const DbvtNode*) { ++m_pcount; }
		void Process(const DbvtNode*, Scalar depth)
		{
			++m_pcount;
			if (m_checksort)
			{
				if (depth >= m_depth)
					m_depth = depth;
				else
					printf("wrong depth: %f (should be >= %f)\r\n", depth, m_depth);
			}
		}
		i32 m_pcount;
		Scalar m_depth;
		bool m_checksort;
	};
	struct P14 : Dbvt::ICollide
	{
		struct Node
		{
			const DbvtNode* leaf;
			Scalar depth;
		};
		void Process(const DbvtNode* leaf, Scalar depth)
		{
			Node n;
			n.leaf = leaf;
			n.depth = depth;
		}
		static i32 sortfnc(const Node& a, const Node& b)
		{
			if (a.depth < b.depth) return (+1);
			if (a.depth > b.depth) return (-1);
			return (0);
		}
		AlignedObjectArray<Node> m_nodes;
	};
	struct P15 : Dbvt::ICollide
	{
		struct Node
		{
			const DbvtNode* leaf;
			Scalar depth;
		};
		void Process(const DbvtNode* leaf)
		{
			Node n;
			n.leaf = leaf;
			n.depth = dot(leaf->volume.Center(), m_axis);
		}
		static i32 sortfnc(const Node& a, const Node& b)
		{
			if (a.depth < b.depth) return (+1);
			if (a.depth > b.depth) return (-1);
			return (0);
		}
		AlignedObjectArray<Node> m_nodes;
		Vec3 m_axis;
	};
	static Scalar RandUnit()
	{
		return (rand() / (Scalar)RAND_MAX);
	}
	static Vec3 RandVector3()
	{
		return (Vec3(RandUnit(), RandUnit(), RandUnit()));
	}
	static Vec3 RandVector3(Scalar cs)
	{
		return (RandVector3() * cs - Vec3(cs, cs, cs) / 2);
	}
	static DbvtVolume RandVolume(Scalar cs, Scalar eb, Scalar es)
	{
		return (DbvtVolume::FromCE(RandVector3(cs), Vec3(eb, eb, eb) + RandVector3() * es));
	}
	static Transform2 RandTransform2(Scalar cs)
	{
		Transform2 t;
		t.setOrigin(RandVector3(cs));
		t.setRotation(Quat(RandUnit() * SIMD_PI * 2, RandUnit() * SIMD_PI * 2, RandUnit() * SIMD_PI * 2).normalized());
		return (t);
	}
	static void RandTree(Scalar cs, Scalar eb, Scalar es, i32 leaves, Dbvt& dbvt)
	{
		dbvt.clear();
		for (i32 i = 0; i < leaves; ++i)
		{
			dbvt.insert(RandVolume(cs, eb, es), 0);
		}
	}
};

void Dbvt::benchmark()
{
	static const Scalar cfgVolumeCenterScale = 100;
	static const Scalar cfgVolumeExentsBase = 1;
	static const Scalar cfgVolumeExentsScale = 4;
	static i32k cfgLeaves = 8192;
	static const bool cfgEnable = true;

	//[1] DbvtVolume intersections
	bool cfgBenchmark1_Enable = cfgEnable;
	static i32k cfgBenchmark1_Iterations = 8;
	static i32k cfgBenchmark1_Reference = 3499;
	//[2] DbvtVolume merges
	bool cfgBenchmark2_Enable = cfgEnable;
	static i32k cfgBenchmark2_Iterations = 4;
	static i32k cfgBenchmark2_Reference = 1945;
	//[3] Dbvt::collideTT
	bool cfgBenchmark3_Enable = cfgEnable;
	static i32k cfgBenchmark3_Iterations = 512;
	static i32k cfgBenchmark3_Reference = 5485;
	//[4] Dbvt::collideTT self
	bool cfgBenchmark4_Enable = cfgEnable;
	static i32k cfgBenchmark4_Iterations = 512;
	static i32k cfgBenchmark4_Reference = 2814;
	//[5] Dbvt::collideTT xform
	bool cfgBenchmark5_Enable = cfgEnable;
	static i32k cfgBenchmark5_Iterations = 512;
	static const Scalar cfgBenchmark5_OffsetScale = 2;
	static i32k cfgBenchmark5_Reference = 7379;
	//[6] Dbvt::collideTT xform,self
	bool cfgBenchmark6_Enable = cfgEnable;
	static i32k cfgBenchmark6_Iterations = 512;
	static const Scalar cfgBenchmark6_OffsetScale = 2;
	static i32k cfgBenchmark6_Reference = 7270;
	//[7] Dbvt::rayTest
	bool cfgBenchmark7_Enable = cfgEnable;
	static i32k cfgBenchmark7_Passes = 32;
	static i32k cfgBenchmark7_Iterations = 65536;
	static i32k cfgBenchmark7_Reference = 6307;
	//[8] insert/remove
	bool cfgBenchmark8_Enable = cfgEnable;
	static i32k cfgBenchmark8_Passes = 32;
	static i32k cfgBenchmark8_Iterations = 65536;
	static i32k cfgBenchmark8_Reference = 2105;
	//[9] updates (teleport)
	bool cfgBenchmark9_Enable = cfgEnable;
	static i32k cfgBenchmark9_Passes = 32;
	static i32k cfgBenchmark9_Iterations = 65536;
	static i32k cfgBenchmark9_Reference = 1879;
	//[10] updates (jitter)
	bool cfgBenchmark10_Enable = cfgEnable;
	static const Scalar cfgBenchmark10_Scale = cfgVolumeCenterScale / 10000;
	static i32k cfgBenchmark10_Passes = 32;
	static i32k cfgBenchmark10_Iterations = 65536;
	static i32k cfgBenchmark10_Reference = 1244;
	//[11] optimize (incremental)
	bool cfgBenchmark11_Enable = cfgEnable;
	static i32k cfgBenchmark11_Passes = 64;
	static i32k cfgBenchmark11_Iterations = 65536;
	static i32k cfgBenchmark11_Reference = 2510;
	//[12] DbvtVolume notequal
	bool cfgBenchmark12_Enable = cfgEnable;
	static i32k cfgBenchmark12_Iterations = 32;
	static i32k cfgBenchmark12_Reference = 3677;
	//[13] culling(OCL+fullsort)
	bool cfgBenchmark13_Enable = cfgEnable;
	static i32k cfgBenchmark13_Iterations = 1024;
	static i32k cfgBenchmark13_Reference = 2231;
	//[14] culling(OCL+qsort)
	bool cfgBenchmark14_Enable = cfgEnable;
	static i32k cfgBenchmark14_Iterations = 8192;
	static i32k cfgBenchmark14_Reference = 3500;
	//[15] culling(KDOP+qsort)
	bool cfgBenchmark15_Enable = cfgEnable;
	static i32k cfgBenchmark15_Iterations = 8192;
	static i32k cfgBenchmark15_Reference = 1151;
	//[16] insert/remove batch
	bool cfgBenchmark16_Enable = cfgEnable;
	static i32k cfgBenchmark16_BatchCount = 256;
	static i32k cfgBenchmark16_Passes = 16384;
	static i32k cfgBenchmark16_Reference = 5138;
	//[17] select
	bool cfgBenchmark17_Enable = cfgEnable;
	static i32k cfgBenchmark17_Iterations = 4;
	static i32k cfgBenchmark17_Reference = 3390;

	Clock wallclock;
	printf("Benchmarking dbvt...\r\n");
	printf("\tWorld scale: %f\r\n", cfgVolumeCenterScale);
	printf("\tExtents base: %f\r\n", cfgVolumeExentsBase);
	printf("\tExtents range: %f\r\n", cfgVolumeExentsScale);
	printf("\tLeaves: %u\r\n", cfgLeaves);
	printf("\tsizeof(DbvtVolume): %u bytes\r\n", sizeof(DbvtVolume));
	printf("\tsizeof(DbvtNode):   %u bytes\r\n", sizeof(DbvtNode));
	if (cfgBenchmark1_Enable)
	{  // Benchmark 1
		srand(380843);
		AlignedObjectArray<DbvtVolume> volumes;
		AlignedObjectArray<bool> results;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			volumes[i] = DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		printf("[1] DbvtVolume intersections: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark1_Iterations; ++i)
		{
			for (i32 j = 0; j < cfgLeaves; ++j)
			{
				for (i32 k = 0; k < cfgLeaves; ++k)
				{
					results[k] = Intersect(volumes[j], volumes[k]);
				}
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark1_Reference) * 100 / time);
	}
	if (cfgBenchmark2_Enable)
	{  // Benchmark 2
		srand(380843);
		AlignedObjectArray<DbvtVolume> volumes;
		AlignedObjectArray<DbvtVolume> results;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			volumes[i] = DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		printf("[2] DbvtVolume merges: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark2_Iterations; ++i)
		{
			for (i32 j = 0; j < cfgLeaves; ++j)
			{
				for (i32 k = 0; k < cfgLeaves; ++k)
				{
					Merge(volumes[j], volumes[k], results[k]);
				}
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark2_Reference) * 100 / time);
	}
	if (cfgBenchmark3_Enable)
	{  // Benchmark 3
		srand(380843);
		Dbvt dbvt[2];
		DbvtBenchmark::NilPolicy policy;
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[0]);
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[1]);
		dbvt[0].optimizeTopDown();
		dbvt[1].optimizeTopDown();
		printf("[3] Dbvt::collideTT: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark3_Iterations; ++i)
		{
			Dbvt::collideTT(dbvt[0].m_root, dbvt[1].m_root, policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark3_Reference) * 100 / time);
	}
	if (cfgBenchmark4_Enable)
	{  // Benchmark 4
		srand(380843);
		Dbvt dbvt;
		DbvtBenchmark::NilPolicy policy;
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[4] Dbvt::collideTT self: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark4_Iterations; ++i)
		{
			Dbvt::collideTT(dbvt.m_root, dbvt.m_root, policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark4_Reference) * 100 / time);
	}
	if (cfgBenchmark5_Enable)
	{  // Benchmark 5
		srand(380843);
		Dbvt dbvt[2];
		AlignedObjectArray<Transform2> transforms;
		DbvtBenchmark::NilPolicy policy;
		transforms.resize(cfgBenchmark5_Iterations);
		for (i32 i = 0; i < transforms.size(); ++i)
		{
			transforms[i] = DbvtBenchmark::RandTransform2(cfgVolumeCenterScale * cfgBenchmark5_OffsetScale);
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[0]);
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[1]);
		dbvt[0].optimizeTopDown();
		dbvt[1].optimizeTopDown();
		printf("[5] Dbvt::collideTT xform: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark5_Iterations; ++i)
		{
			Dbvt::collideTT(dbvt[0].m_root, dbvt[1].m_root, transforms[i], policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark5_Reference) * 100 / time);
	}
	if (cfgBenchmark6_Enable)
	{  // Benchmark 6
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<Transform2> transforms;
		DbvtBenchmark::NilPolicy policy;
		transforms.resize(cfgBenchmark6_Iterations);
		for (i32 i = 0; i < transforms.size(); ++i)
		{
			transforms[i] = DbvtBenchmark::RandTransform2(cfgVolumeCenterScale * cfgBenchmark6_OffsetScale);
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[6] Dbvt::collideTT xform,self: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark6_Iterations; ++i)
		{
			Dbvt::collideTT(dbvt.m_root, dbvt.m_root, transforms[i], policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark6_Reference) * 100 / time);
	}
	if (cfgBenchmark7_Enable)
	{  // Benchmark 7
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<Vec3> rayorg;
		AlignedObjectArray<Vec3> raydir;
		DbvtBenchmark::NilPolicy policy;
		rayorg.resize(cfgBenchmark7_Iterations);
		raydir.resize(cfgBenchmark7_Iterations);
		for (i32 i = 0; i < rayorg.size(); ++i)
		{
			rayorg[i] = DbvtBenchmark::RandVector3(cfgVolumeCenterScale * 2);
			raydir[i] = DbvtBenchmark::RandVector3(cfgVolumeCenterScale * 2);
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[7] Dbvt::rayTest: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark7_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark7_Iterations; ++j)
			{
				Dbvt::rayTest(dbvt.m_root, rayorg[j], rayorg[j] + raydir[j], policy);
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		unsigned rays = cfgBenchmark7_Passes * cfgBenchmark7_Iterations;
		printf("%u ms (%i%%),(%u r/s)\r\n", time, (time - cfgBenchmark7_Reference) * 100 / time, (rays * 1000) / time);
	}
	if (cfgBenchmark8_Enable)
	{  // Benchmark 8
		srand(380843);
		Dbvt dbvt;
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[8] insert/remove: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark8_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark8_Iterations; ++j)
			{
				dbvt.remove(dbvt.insert(DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale), 0));
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k ir = cfgBenchmark8_Passes * cfgBenchmark8_Iterations;
		printf("%u ms (%i%%),(%u ir/s)\r\n", time, (time - cfgBenchmark8_Reference) * 100 / time, ir * 1000 / time);
	}
	if (cfgBenchmark9_Enable)
	{  // Benchmark 9
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<const DbvtNode*> leaves;
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		dbvt.extractLeaves(dbvt.m_root, leaves);
		printf("[9] updates (teleport): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark9_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark9_Iterations; ++j)
			{
				dbvt.update(const_cast<DbvtNode*>(leaves[rand() % cfgLeaves]),
							DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale));
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k up = cfgBenchmark9_Passes * cfgBenchmark9_Iterations;
		printf("%u ms (%i%%),(%u u/s)\r\n", time, (time - cfgBenchmark9_Reference) * 100 / time, up * 1000 / time);
	}
	if (cfgBenchmark10_Enable)
	{  // Benchmark 10
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<const DbvtNode*> leaves;
		AlignedObjectArray<Vec3> vectors;
		vectors.resize(cfgBenchmark10_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (DbvtBenchmark::RandVector3() * 2 - Vec3(1, 1, 1)) * cfgBenchmark10_Scale;
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		dbvt.extractLeaves(dbvt.m_root, leaves);
		printf("[10] updates (jitter): ");
		wallclock.reset();

		for (i32 i = 0; i < cfgBenchmark10_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark10_Iterations; ++j)
			{
				const Vec3& d = vectors[j];
				DbvtNode* l = const_cast<DbvtNode*>(leaves[rand() % cfgLeaves]);
				DbvtVolume v = DbvtVolume::FromMM(l->volume.Mins() + d, l->volume.Maxs() + d);
				dbvt.update(l, v);
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k up = cfgBenchmark10_Passes * cfgBenchmark10_Iterations;
		printf("%u ms (%i%%),(%u u/s)\r\n", time, (time - cfgBenchmark10_Reference) * 100 / time, up * 1000 / time);
	}
	if (cfgBenchmark11_Enable)
	{  // Benchmark 11
		srand(380843);
		Dbvt dbvt;
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[11] optimize (incremental): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark11_Passes; ++i)
		{
			dbvt.optimizeIncremental(cfgBenchmark11_Iterations);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k op = cfgBenchmark11_Passes * cfgBenchmark11_Iterations;
		printf("%u ms (%i%%),(%u o/s)\r\n", time, (time - cfgBenchmark11_Reference) * 100 / time, op / time * 1000);
	}
	if (cfgBenchmark12_Enable)
	{  // Benchmark 12
		srand(380843);
		AlignedObjectArray<DbvtVolume> volumes;
		AlignedObjectArray<bool> results;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			volumes[i] = DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		printf("[12] DbvtVolume notequal: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark12_Iterations; ++i)
		{
			for (i32 j = 0; j < cfgLeaves; ++j)
			{
				for (i32 k = 0; k < cfgLeaves; ++k)
				{
					results[k] = NotEqual(volumes[j], volumes[k]);
				}
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark12_Reference) * 100 / time);
	}
	if (cfgBenchmark13_Enable)
	{  // Benchmark 13
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<Vec3> vectors;
		DbvtBenchmark::NilPolicy policy;
		vectors.resize(cfgBenchmark13_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (DbvtBenchmark::RandVector3() * 2 - Vec3(1, 1, 1)).normalized();
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[13] culling(OCL+fullsort): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark13_Iterations; ++i)
		{
			static const Scalar offset = 0;
			policy.m_depth = -SIMD_INFINITY;
			dbvt.collideOCL(dbvt.m_root, &vectors[i], &offset, vectors[i], 1, policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k t = cfgBenchmark13_Iterations;
		printf("%u ms (%i%%),(%u t/s)\r\n", time, (time - cfgBenchmark13_Reference) * 100 / time, (t * 1000) / time);
	}
	if (cfgBenchmark14_Enable)
	{  // Benchmark 14
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<Vec3> vectors;
		DbvtBenchmark::P14 policy;
		vectors.resize(cfgBenchmark14_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (DbvtBenchmark::RandVector3() * 2 - Vec3(1, 1, 1)).normalized();
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		policy.m_nodes.reserve(cfgLeaves);
		printf("[14] culling(OCL+qsort): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark14_Iterations; ++i)
		{
			static const Scalar offset = 0;
			policy.m_nodes.resize(0);
			dbvt.collideOCL(dbvt.m_root, &vectors[i], &offset, vectors[i], 1, policy, false);
			policy.m_nodes.quickSort(DbvtBenchmark::P14::sortfnc);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k t = cfgBenchmark14_Iterations;
		printf("%u ms (%i%%),(%u t/s)\r\n", time, (time - cfgBenchmark14_Reference) * 100 / time, (t * 1000) / time);
	}
	if (cfgBenchmark15_Enable)
	{  // Benchmark 15
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<Vec3> vectors;
		DbvtBenchmark::P15 policy;
		vectors.resize(cfgBenchmark15_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (DbvtBenchmark::RandVector3() * 2 - Vec3(1, 1, 1)).normalized();
		}
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		policy.m_nodes.reserve(cfgLeaves);
		printf("[15] culling(KDOP+qsort): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark15_Iterations; ++i)
		{
			static const Scalar offset = 0;
			policy.m_nodes.resize(0);
			policy.m_axis = vectors[i];
			dbvt.collideKDOP(dbvt.m_root, &vectors[i], &offset, 1, policy);
			policy.m_nodes.quickSort(DbvtBenchmark::P15::sortfnc);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k t = cfgBenchmark15_Iterations;
		printf("%u ms (%i%%),(%u t/s)\r\n", time, (time - cfgBenchmark15_Reference) * 100 / time, (t * 1000) / time);
	}
	if (cfgBenchmark16_Enable)
	{  // Benchmark 16
		srand(380843);
		Dbvt dbvt;
		AlignedObjectArray<DbvtNode*> batch;
		DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		batch.reserve(cfgBenchmark16_BatchCount);
		printf("[16] insert/remove batch(%u): ", cfgBenchmark16_BatchCount);
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark16_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark16_BatchCount; ++j)
			{
				batch.push_back(dbvt.insert(DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale), 0));
			}
			for (i32 j = 0; j < cfgBenchmark16_BatchCount; ++j)
			{
				dbvt.remove(batch[j]);
			}
			batch.resize(0);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k ir = cfgBenchmark16_Passes * cfgBenchmark16_BatchCount;
		printf("%u ms (%i%%),(%u bir/s)\r\n", time, (time - cfgBenchmark16_Reference) * 100 / time, i32(ir * 1000.0 / time));
	}
	if (cfgBenchmark17_Enable)
	{  // Benchmark 17
		srand(380843);
		AlignedObjectArray<DbvtVolume> volumes;
		AlignedObjectArray<i32> results;
		AlignedObjectArray<i32> indices;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		indices.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			indices[i] = i;
			volumes[i] = DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			Swap(indices[i], indices[rand() % cfgLeaves]);
		}
		printf("[17] DbvtVolume select: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark17_Iterations; ++i)
		{
			for (i32 j = 0; j < cfgLeaves; ++j)
			{
				for (i32 k = 0; k < cfgLeaves; ++k)
				{
					i32k idx = indices[k];
					results[idx] = Select(volumes[idx], volumes[j], volumes[k]);
				}
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark17_Reference) * 100 / time);
	}
	printf("\r\n\r\n");
}
#endif
