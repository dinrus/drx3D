#include <drx3D/Physics/Collision/BroadPhase/b3DynamicBvh.h>

//
typedef b3AlignedObjectArray<b3DbvtNode*> b3NodeArray;
typedef b3AlignedObjectArray<const b3DbvtNode*> b3ConstNodeArray;

//
struct b3DbvtNodeEnumerator : b3DynamicBvh::ICollide
{
	b3ConstNodeArray nodes;
	void Process(const b3DbvtNode* n) { nodes.push_back(n); }
};

//
static D3_DBVT_INLINE i32 b3IndexOf(const b3DbvtNode* node)
{
	return (node->parent->childs[1] == node);
}

//
static D3_DBVT_INLINE b3DbvtVolume b3Merge(const b3DbvtVolume& a,
										   const b3DbvtVolume& b)
{
#if (D3_DBVT_MERGE_IMPL == D3_DBVT_IMPL_SSE)
	D3_ATTRIBUTE_ALIGNED16(char locals[sizeof(b3DbvtAabbMm)]);
	b3DbvtVolume& res = *(b3DbvtVolume*)locals;
#else
	b3DbvtVolume res;
#endif
	b3Merge(a, b, res);
	return (res);
}

// volume+edge lengths
static D3_DBVT_INLINE b3Scalar b3Size(const b3DbvtVolume& a)
{
	const b3Vec3 edges = a.Lengths();
	return (edges.x * edges.y * edges.z +
			edges.x + edges.y + edges.z);
}

//
static void b3GetMaxDepth(const b3DbvtNode* node, i32 depth, i32& maxdepth)
{
	if (node->isinternal())
	{
		b3GetMaxDepth(node->childs[0], depth + 1, maxdepth);
		b3GetMaxDepth(node->childs[1], depth + 1, maxdepth);
	}
	else
		maxdepth = d3Max(maxdepth, depth);
}

//
static D3_DBVT_INLINE void b3DeleteNode(b3DynamicBvh* pdbvt,
										b3DbvtNode* node)
{
	b3AlignedFree(pdbvt->m_free);
	pdbvt->m_free = node;
}

//
static void b3RecurseDeleteNode(b3DynamicBvh* pdbvt,
								b3DbvtNode* node)
{
	if (!node->isleaf())
	{
		b3RecurseDeleteNode(pdbvt, node->childs[0]);
		b3RecurseDeleteNode(pdbvt, node->childs[1]);
	}
	if (node == pdbvt->m_root) pdbvt->m_root = 0;
	b3DeleteNode(pdbvt, node);
}

//
static D3_DBVT_INLINE b3DbvtNode* b3CreateNode(b3DynamicBvh* pdbvt,
											   b3DbvtNode* parent,
											   uk data)
{
	b3DbvtNode* node;
	if (pdbvt->m_free)
	{
		node = pdbvt->m_free;
		pdbvt->m_free = 0;
	}
	else
	{
		node = new (b3AlignedAlloc(sizeof(b3DbvtNode), 16)) b3DbvtNode();
	}
	node->parent = parent;
	node->data = data;
	node->childs[1] = 0;
	return (node);
}

//
static D3_DBVT_INLINE b3DbvtNode* b3CreateNode(b3DynamicBvh* pdbvt,
											   b3DbvtNode* parent,
											   const b3DbvtVolume& volume,
											   uk data)
{
	b3DbvtNode* node = b3CreateNode(pdbvt, parent, data);
	node->volume = volume;
	return (node);
}

//
static D3_DBVT_INLINE b3DbvtNode* b3CreateNode(b3DynamicBvh* pdbvt,
											   b3DbvtNode* parent,
											   const b3DbvtVolume& volume0,
											   const b3DbvtVolume& volume1,
											   uk data)
{
	b3DbvtNode* node = b3CreateNode(pdbvt, parent, data);
	b3Merge(volume0, volume1, node->volume);
	return (node);
}

//
static void b3InsertLeaf(b3DynamicBvh* pdbvt,
						 b3DbvtNode* root,
						 b3DbvtNode* leaf)
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
				root = root->childs[b3Select(leaf->volume,
											 root->childs[0]->volume,
											 root->childs[1]->volume)];
			} while (!root->isleaf());
		}
		b3DbvtNode* prev = root->parent;
		b3DbvtNode* node = b3CreateNode(pdbvt, prev, leaf->volume, root->volume, 0);
		if (prev)
		{
			prev->childs[b3IndexOf(root)] = node;
			node->childs[0] = root;
			root->parent = node;
			node->childs[1] = leaf;
			leaf->parent = node;
			do
			{
				if (!prev->volume.Contain(node->volume))
					b3Merge(prev->childs[0]->volume, prev->childs[1]->volume, prev->volume);
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
static b3DbvtNode* b3RemoveLeaf(b3DynamicBvh* pdbvt,
								b3DbvtNode* leaf)
{
	if (leaf == pdbvt->m_root)
	{
		pdbvt->m_root = 0;
		return (0);
	}
	else
	{
		b3DbvtNode* parent = leaf->parent;
		b3DbvtNode* prev = parent->parent;
		b3DbvtNode* sibling = parent->childs[1 - b3IndexOf(leaf)];
		if (prev)
		{
			prev->childs[b3IndexOf(parent)] = sibling;
			sibling->parent = prev;
			b3DeleteNode(pdbvt, parent);
			while (prev)
			{
				const b3DbvtVolume pb = prev->volume;
				b3Merge(prev->childs[0]->volume, prev->childs[1]->volume, prev->volume);
				if (b3NotEqual(pb, prev->volume))
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
			b3DeleteNode(pdbvt, parent);
			return (pdbvt->m_root);
		}
	}
}

//
static void b3FetchLeaves(b3DynamicBvh* pdbvt,
						  b3DbvtNode* root,
						  b3NodeArray& leaves,
						  i32 depth = -1)
{
	if (root->isinternal() && depth)
	{
		b3FetchLeaves(pdbvt, root->childs[0], leaves, depth - 1);
		b3FetchLeaves(pdbvt, root->childs[1], leaves, depth - 1);
		b3DeleteNode(pdbvt, root);
	}
	else
	{
		leaves.push_back(root);
	}
}

static bool b3LeftOfAxis(const b3DbvtNode* node,
						 const b3Vec3& org,
						 const b3Vec3& axis)
{
	return b3Dot(axis, node->volume.Center() - org) <= 0;
}

// Partitions leaves such that leaves[0, n) are on the
// left of axis, and leaves[n, count) are on the right
// of axis. returns N.
static i32 b3Split(b3DbvtNode** leaves,
				   i32 count,
				   const b3Vec3& org,
				   const b3Vec3& axis)
{
	i32 begin = 0;
	i32 end = count;
	for (;;)
	{
		while (begin != end && b3LeftOfAxis(leaves[begin], org, axis))
		{
			++begin;
		}

		if (begin == end)
		{
			break;
		}

		while (begin != end && !b3LeftOfAxis(leaves[end - 1], org, axis))
		{
			--end;
		}

		if (begin == end)
		{
			break;
		}

		// swap out of place nodes
		--end;
		b3DbvtNode* temp = leaves[begin];
		leaves[begin] = leaves[end];
		leaves[end] = temp;
		++begin;
	}

	return begin;
}

//
static b3DbvtVolume b3Bounds(b3DbvtNode** leaves,
							 i32 count)
{
#if D3_DBVT_MERGE_IMPL == D3_DBVT_IMPL_SSE
	D3_ATTRIBUTE_ALIGNED16(char locals[sizeof(b3DbvtVolume)]);
	b3DbvtVolume& volume = *(b3DbvtVolume*)locals;
	volume = leaves[0]->volume;
#else
	b3DbvtVolume volume = leaves[0]->volume;
#endif
	for (i32 i = 1, ni = count; i < ni; ++i)
	{
		b3Merge(volume, leaves[i]->volume, volume);
	}
	return (volume);
}

//
static void b3BottomUp(b3DynamicBvh* pdbvt,
					   b3DbvtNode** leaves,
					   i32 count)
{
	while (count > 1)
	{
		b3Scalar minsize = D3_INFINITY;
		i32 minidx[2] = {-1, -1};
		for (i32 i = 0; i < count; ++i)
		{
			for (i32 j = i + 1; j < count; ++j)
			{
				const b3Scalar sz = b3Size(b3Merge(leaves[i]->volume, leaves[j]->volume));
				if (sz < minsize)
				{
					minsize = sz;
					minidx[0] = i;
					minidx[1] = j;
				}
			}
		}
		b3DbvtNode* n[] = {leaves[minidx[0]], leaves[minidx[1]]};
		b3DbvtNode* p = b3CreateNode(pdbvt, 0, n[0]->volume, n[1]->volume, 0);
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
static b3DbvtNode* b3TopDown(b3DynamicBvh* pdbvt,
							 b3DbvtNode** leaves,
							 i32 count,
							 i32 bu_treshold)
{
	static const b3Vec3 axis[] = {b3MakeVector3(1, 0, 0),
									 b3MakeVector3(0, 1, 0),
									 b3MakeVector3(0, 0, 1)};
	drx3DAssert(bu_treshold > 1);
	if (count > 1)
	{
		if (count > bu_treshold)
		{
			const b3DbvtVolume vol = b3Bounds(leaves, count);
			const b3Vec3 org = vol.Center();
			i32 partition;
			i32 bestaxis = -1;
			i32 bestmidp = count;
			i32 splitcount[3][2] = {{0, 0}, {0, 0}, {0, 0}};
			i32 i;
			for (i = 0; i < count; ++i)
			{
				const b3Vec3 x = leaves[i]->volume.Center() - org;
				for (i32 j = 0; j < 3; ++j)
				{
					++splitcount[j][b3Dot(x, axis[j]) > 0 ? 1 : 0];
				}
			}
			for (i = 0; i < 3; ++i)
			{
				if ((splitcount[i][0] > 0) && (splitcount[i][1] > 0))
				{
					i32k midp = (i32)b3Fabs(b3Scalar(splitcount[i][0] - splitcount[i][1]));
					if (midp < bestmidp)
					{
						bestaxis = i;
						bestmidp = midp;
					}
				}
			}
			if (bestaxis >= 0)
			{
				partition = b3Split(leaves, count, org, axis[bestaxis]);
				drx3DAssert(partition != 0 && partition != count);
			}
			else
			{
				partition = count / 2 + 1;
			}
			b3DbvtNode* node = b3CreateNode(pdbvt, 0, vol, 0);
			node->childs[0] = b3TopDown(pdbvt, &leaves[0], partition, bu_treshold);
			node->childs[1] = b3TopDown(pdbvt, &leaves[partition], count - partition, bu_treshold);
			node->childs[0]->parent = node;
			node->childs[1]->parent = node;
			return (node);
		}
		else
		{
			b3BottomUp(pdbvt, leaves, count);
			return (leaves[0]);
		}
	}
	return (leaves[0]);
}

//
static D3_DBVT_INLINE b3DbvtNode* b3Sort(b3DbvtNode* n, b3DbvtNode*& r)
{
	b3DbvtNode* p = n->parent;
	drx3DAssert(n->isinternal());
	if (p > n)
	{
		i32k i = b3IndexOf(n);
		i32k j = 1 - i;
		b3DbvtNode* s = p->childs[j];
		b3DbvtNode* q = p->parent;
		drx3DAssert(n == p->childs[i]);
		if (q)
			q->childs[b3IndexOf(p)] = n;
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
		b3Swap(p->volume, n->volume);
		return (p);
	}
	return (n);
}

#if 0
static D3_DBVT_INLINE b3DbvtNode*	walkup(b3DbvtNode* n,i32 count)
{
	while(n&&(count--)) n=n->parent;
	return(n);
}
#endif

//
// Api
//

//
b3DynamicBvh::b3DynamicBvh()
{
	m_root = 0;
	m_free = 0;
	m_lkhd = -1;
	m_leaves = 0;
	m_opath = 0;
}

//
b3DynamicBvh::~b3DynamicBvh()
{
	clear();
}

//
void b3DynamicBvh::clear()
{
	if (m_root)
		b3RecurseDeleteNode(this, m_root);
	b3AlignedFree(m_free);
	m_free = 0;
	m_lkhd = -1;
	m_stkStack.clear();
	m_opath = 0;
}

//
void b3DynamicBvh::optimizeBottomUp()
{
	if (m_root)
	{
		b3NodeArray leaves;
		leaves.reserve(m_leaves);
		b3FetchLeaves(this, m_root, leaves);
		b3BottomUp(this, &leaves[0], leaves.size());
		m_root = leaves[0];
	}
}

//
void b3DynamicBvh::optimizeTopDown(i32 bu_treshold)
{
	if (m_root)
	{
		b3NodeArray leaves;
		leaves.reserve(m_leaves);
		b3FetchLeaves(this, m_root, leaves);
		m_root = b3TopDown(this, &leaves[0], leaves.size(), bu_treshold);
	}
}

//
void b3DynamicBvh::optimizeIncremental(i32 passes)
{
	if (passes < 0) passes = m_leaves;
	if (m_root && (passes > 0))
	{
		do
		{
			b3DbvtNode* node = m_root;
			unsigned bit = 0;
			while (node->isinternal())
			{
				node = b3Sort(node, m_root)->childs[(m_opath >> bit) & 1];
				bit = (bit + 1) & (sizeof(unsigned) * 8 - 1);
			}
			update(node);
			++m_opath;
		} while (--passes);
	}
}

//
b3DbvtNode* b3DynamicBvh::insert(const b3DbvtVolume& volume, uk data)
{
	b3DbvtNode* leaf = b3CreateNode(this, 0, volume, data);
	b3InsertLeaf(this, m_root, leaf);
	++m_leaves;
	return (leaf);
}

//
void b3DynamicBvh::update(b3DbvtNode* leaf, i32 lookahead)
{
	b3DbvtNode* root = b3RemoveLeaf(this, leaf);
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
	b3InsertLeaf(this, root, leaf);
}

//
void b3DynamicBvh::update(b3DbvtNode* leaf, b3DbvtVolume& volume)
{
	b3DbvtNode* root = b3RemoveLeaf(this, leaf);
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
	b3InsertLeaf(this, root, leaf);
}

//
bool b3DynamicBvh::update(b3DbvtNode* leaf, b3DbvtVolume& volume, const b3Vec3& velocity, b3Scalar margin)
{
	if (leaf->volume.Contain(volume)) return (false);
	volume.Expand(b3MakeVector3(margin, margin, margin));
	volume.SignedExpand(velocity);
	update(leaf, volume);
	return (true);
}

//
bool b3DynamicBvh::update(b3DbvtNode* leaf, b3DbvtVolume& volume, const b3Vec3& velocity)
{
	if (leaf->volume.Contain(volume)) return (false);
	volume.SignedExpand(velocity);
	update(leaf, volume);
	return (true);
}

//
bool b3DynamicBvh::update(b3DbvtNode* leaf, b3DbvtVolume& volume, b3Scalar margin)
{
	if (leaf->volume.Contain(volume)) return (false);
	volume.Expand(b3MakeVector3(margin, margin, margin));
	update(leaf, volume);
	return (true);
}

//
void b3DynamicBvh::remove(b3DbvtNode* leaf)
{
	b3RemoveLeaf(this, leaf);
	b3DeleteNode(this, leaf);
	--m_leaves;
}

//
void b3DynamicBvh::write(IWriter* iwriter) const
{
	b3DbvtNodeEnumerator nodes;
	nodes.nodes.reserve(m_leaves * 2);
	enumNodes(m_root, nodes);
	iwriter->Prepare(m_root, nodes.nodes.size());
	for (i32 i = 0; i < nodes.nodes.size(); ++i)
	{
		const b3DbvtNode* n = nodes.nodes[i];
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
void b3DynamicBvh::clone(b3DynamicBvh& dest, IClone* iclone) const
{
	dest.clear();
	if (m_root != 0)
	{
		b3AlignedObjectArray<sStkCLN> stack;
		stack.reserve(m_leaves);
		stack.push_back(sStkCLN(m_root, 0));
		do
		{
			i32k i = stack.size() - 1;
			const sStkCLN e = stack[i];
			b3DbvtNode* n = b3CreateNode(&dest, e.parent, e.node->volume, e.node->data);
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
i32 b3DynamicBvh::maxdepth(const b3DbvtNode* node)
{
	i32 depth = 0;
	if (node) b3GetMaxDepth(node, 1, depth);
	return (depth);
}

//
i32 b3DynamicBvh::countLeaves(const b3DbvtNode* node)
{
	if (node->isinternal())
		return (countLeaves(node->childs[0]) + countLeaves(node->childs[1]));
	else
		return (1);
}

//
void b3DynamicBvh::extractLeaves(const b3DbvtNode* node, b3AlignedObjectArray<const b3DbvtNode*>& leaves)
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
#if D3_DBVT_ENABLE_BENCHMARK

#include <stdio.h>
#include <stdlib.h>

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
sizeof(b3DbvtVolume): 32 bytes
sizeof(b3DbvtNode):   44 bytes
[1] b3DbvtVolume intersections: 3499 ms (-1%)
[2] b3DbvtVolume merges: 1934 ms (0%)
[3] b3DynamicBvh::collideTT: 5485 ms (-21%)
[4] b3DynamicBvh::collideTT self: 2814 ms (-20%)
[5] b3DynamicBvh::collideTT xform: 7379 ms (-1%)
[6] b3DynamicBvh::collideTT xform,self: 7270 ms (-2%)
[7] b3DynamicBvh::rayTest: 6314 ms (0%),(332143 r/s)
[8] insert/remove: 2093 ms (0%),(1001983 ir/s)
[9] updates (teleport): 1879 ms (-3%),(1116100 u/s)
[10] updates (jitter): 1244 ms (-4%),(1685813 u/s)
[11] optimize (incremental): 2514 ms (0%),(1668000 o/s)
[12] b3DbvtVolume notequal: 3659 ms (0%)
[13] culling(OCL+fullsort): 2218 ms (0%),(461 t/s)
[14] culling(OCL+qsort): 3688 ms (5%),(2221 t/s)
[15] culling(KDOP+qsort): 1139 ms (-1%),(7192 t/s)
[16] insert/remove batch(256): 5092 ms (0%),(823704 bir/s)
[17] b3DbvtVolume select: 3419 ms (0%)
*/

struct b3DbvtBenchmark
{
	struct NilPolicy : b3DynamicBvh::ICollide
	{
		NilPolicy() : m_pcount(0), m_depth(-D3_INFINITY), m_checksort(true) {}
		void Process(const b3DbvtNode*, const b3DbvtNode*) { ++m_pcount; }
		void Process(const b3DbvtNode*) { ++m_pcount; }
		void Process(const b3DbvtNode*, b3Scalar depth)
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
		b3Scalar m_depth;
		bool m_checksort;
	};
	struct P14 : b3DynamicBvh::ICollide
	{
		struct Node
		{
			const b3DbvtNode* leaf;
			b3Scalar depth;
		};
		void Process(const b3DbvtNode* leaf, b3Scalar depth)
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
		b3AlignedObjectArray<Node> m_nodes;
	};
	struct P15 : b3DynamicBvh::ICollide
	{
		struct Node
		{
			const b3DbvtNode* leaf;
			b3Scalar depth;
		};
		void Process(const b3DbvtNode* leaf)
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
		b3AlignedObjectArray<Node> m_nodes;
		b3Vec3 m_axis;
	};
	static b3Scalar RandUnit()
	{
		return (rand() / (b3Scalar)RAND_MAX);
	}
	static b3Vec3 RandVector3()
	{
		return (b3Vec3(RandUnit(), RandUnit(), RandUnit()));
	}
	static b3Vec3 RandVector3(b3Scalar cs)
	{
		return (RandVector3() * cs - b3Vec3(cs, cs, cs) / 2);
	}
	static b3DbvtVolume RandVolume(b3Scalar cs, b3Scalar eb, b3Scalar es)
	{
		return (b3DbvtVolume::FromCE(RandVector3(cs), b3Vec3(eb, eb, eb) + RandVector3() * es));
	}
	static b3Transform RandTransform(b3Scalar cs)
	{
		b3Transform t;
		t.setOrigin(RandVector3(cs));
		t.setRotation(b3Quat(RandUnit() * D3_PI * 2, RandUnit() * D3_PI * 2, RandUnit() * D3_PI * 2).normalized());
		return (t);
	}
	static void RandTree(b3Scalar cs, b3Scalar eb, b3Scalar es, i32 leaves, b3DynamicBvh& dbvt)
	{
		dbvt.clear();
		for (i32 i = 0; i < leaves; ++i)
		{
			dbvt.insert(RandVolume(cs, eb, es), 0);
		}
	}
};

void b3DynamicBvh::benchmark()
{
	static const b3Scalar cfgVolumeCenterScale = 100;
	static const b3Scalar cfgVolumeExentsBase = 1;
	static const b3Scalar cfgVolumeExentsScale = 4;
	static i32k cfgLeaves = 8192;
	static const bool cfgEnable = true;

	//[1] b3DbvtVolume intersections
	bool cfgBenchmark1_Enable = cfgEnable;
	static i32k cfgBenchmark1_Iterations = 8;
	static i32k cfgBenchmark1_Reference = 3499;
	//[2] b3DbvtVolume merges
	bool cfgBenchmark2_Enable = cfgEnable;
	static i32k cfgBenchmark2_Iterations = 4;
	static i32k cfgBenchmark2_Reference = 1945;
	//[3] b3DynamicBvh::collideTT
	bool cfgBenchmark3_Enable = cfgEnable;
	static i32k cfgBenchmark3_Iterations = 512;
	static i32k cfgBenchmark3_Reference = 5485;
	//[4] b3DynamicBvh::collideTT self
	bool cfgBenchmark4_Enable = cfgEnable;
	static i32k cfgBenchmark4_Iterations = 512;
	static i32k cfgBenchmark4_Reference = 2814;
	//[5] b3DynamicBvh::collideTT xform
	bool cfgBenchmark5_Enable = cfgEnable;
	static i32k cfgBenchmark5_Iterations = 512;
	static const b3Scalar cfgBenchmark5_OffsetScale = 2;
	static i32k cfgBenchmark5_Reference = 7379;
	//[6] b3DynamicBvh::collideTT xform,self
	bool cfgBenchmark6_Enable = cfgEnable;
	static i32k cfgBenchmark6_Iterations = 512;
	static const b3Scalar cfgBenchmark6_OffsetScale = 2;
	static i32k cfgBenchmark6_Reference = 7270;
	//[7] b3DynamicBvh::rayTest
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
	static const b3Scalar cfgBenchmark10_Scale = cfgVolumeCenterScale / 10000;
	static i32k cfgBenchmark10_Passes = 32;
	static i32k cfgBenchmark10_Iterations = 65536;
	static i32k cfgBenchmark10_Reference = 1244;
	//[11] optimize (incremental)
	bool cfgBenchmark11_Enable = cfgEnable;
	static i32k cfgBenchmark11_Passes = 64;
	static i32k cfgBenchmark11_Iterations = 65536;
	static i32k cfgBenchmark11_Reference = 2510;
	//[12] b3DbvtVolume notequal
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

	b3Clock wallclock;
	printf("Benchmarking dbvt...\r\n");
	printf("\tWorld scale: %f\r\n", cfgVolumeCenterScale);
	printf("\tExtents base: %f\r\n", cfgVolumeExentsBase);
	printf("\tExtents range: %f\r\n", cfgVolumeExentsScale);
	printf("\tLeaves: %u\r\n", cfgLeaves);
	printf("\tsizeof(b3DbvtVolume): %u bytes\r\n", sizeof(b3DbvtVolume));
	printf("\tsizeof(b3DbvtNode):   %u bytes\r\n", sizeof(b3DbvtNode));
	if (cfgBenchmark1_Enable)
	{  // Benchmark 1
		srand(380843);
		b3AlignedObjectArray<b3DbvtVolume> volumes;
		b3AlignedObjectArray<bool> results;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			volumes[i] = b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		printf("[1] b3DbvtVolume intersections: ");
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
		b3AlignedObjectArray<b3DbvtVolume> volumes;
		b3AlignedObjectArray<b3DbvtVolume> results;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			volumes[i] = b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		printf("[2] b3DbvtVolume merges: ");
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
		b3DynamicBvh dbvt[2];
		b3DbvtBenchmark::NilPolicy policy;
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[0]);
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[1]);
		dbvt[0].optimizeTopDown();
		dbvt[1].optimizeTopDown();
		printf("[3] b3DynamicBvh::collideTT: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark3_Iterations; ++i)
		{
			b3DynamicBvh::collideTT(dbvt[0].m_root, dbvt[1].m_root, policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark3_Reference) * 100 / time);
	}
	if (cfgBenchmark4_Enable)
	{  // Benchmark 4
		srand(380843);
		b3DynamicBvh dbvt;
		b3DbvtBenchmark::NilPolicy policy;
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[4] b3DynamicBvh::collideTT self: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark4_Iterations; ++i)
		{
			b3DynamicBvh::collideTT(dbvt.m_root, dbvt.m_root, policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark4_Reference) * 100 / time);
	}
	if (cfgBenchmark5_Enable)
	{  // Benchmark 5
		srand(380843);
		b3DynamicBvh dbvt[2];
		b3AlignedObjectArray<b3Transform> transforms;
		b3DbvtBenchmark::NilPolicy policy;
		transforms.resize(cfgBenchmark5_Iterations);
		for (i32 i = 0; i < transforms.size(); ++i)
		{
			transforms[i] = b3DbvtBenchmark::RandTransform(cfgVolumeCenterScale * cfgBenchmark5_OffsetScale);
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[0]);
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt[1]);
		dbvt[0].optimizeTopDown();
		dbvt[1].optimizeTopDown();
		printf("[5] b3DynamicBvh::collideTT xform: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark5_Iterations; ++i)
		{
			b3DynamicBvh::collideTT(dbvt[0].m_root, dbvt[1].m_root, transforms[i], policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark5_Reference) * 100 / time);
	}
	if (cfgBenchmark6_Enable)
	{  // Benchmark 6
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<b3Transform> transforms;
		b3DbvtBenchmark::NilPolicy policy;
		transforms.resize(cfgBenchmark6_Iterations);
		for (i32 i = 0; i < transforms.size(); ++i)
		{
			transforms[i] = b3DbvtBenchmark::RandTransform(cfgVolumeCenterScale * cfgBenchmark6_OffsetScale);
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[6] b3DynamicBvh::collideTT xform,self: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark6_Iterations; ++i)
		{
			b3DynamicBvh::collideTT(dbvt.m_root, dbvt.m_root, transforms[i], policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		printf("%u ms (%i%%)\r\n", time, (time - cfgBenchmark6_Reference) * 100 / time);
	}
	if (cfgBenchmark7_Enable)
	{  // Benchmark 7
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<b3Vec3> rayorg;
		b3AlignedObjectArray<b3Vec3> raydir;
		b3DbvtBenchmark::NilPolicy policy;
		rayorg.resize(cfgBenchmark7_Iterations);
		raydir.resize(cfgBenchmark7_Iterations);
		for (i32 i = 0; i < rayorg.size(); ++i)
		{
			rayorg[i] = b3DbvtBenchmark::RandVector3(cfgVolumeCenterScale * 2);
			raydir[i] = b3DbvtBenchmark::RandVector3(cfgVolumeCenterScale * 2);
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[7] b3DynamicBvh::rayTest: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark7_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark7_Iterations; ++j)
			{
				b3DynamicBvh::rayTest(dbvt.m_root, rayorg[j], rayorg[j] + raydir[j], policy);
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		unsigned rays = cfgBenchmark7_Passes * cfgBenchmark7_Iterations;
		printf("%u ms (%i%%),(%u r/s)\r\n", time, (time - cfgBenchmark7_Reference) * 100 / time, (rays * 1000) / time);
	}
	if (cfgBenchmark8_Enable)
	{  // Benchmark 8
		srand(380843);
		b3DynamicBvh dbvt;
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[8] insert/remove: ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark8_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark8_Iterations; ++j)
			{
				dbvt.remove(dbvt.insert(b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale), 0));
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k ir = cfgBenchmark8_Passes * cfgBenchmark8_Iterations;
		printf("%u ms (%i%%),(%u ir/s)\r\n", time, (time - cfgBenchmark8_Reference) * 100 / time, ir * 1000 / time);
	}
	if (cfgBenchmark9_Enable)
	{  // Benchmark 9
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<const b3DbvtNode*> leaves;
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		dbvt.extractLeaves(dbvt.m_root, leaves);
		printf("[9] updates (teleport): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark9_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark9_Iterations; ++j)
			{
				dbvt.update(const_cast<b3DbvtNode*>(leaves[rand() % cfgLeaves]),
							b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale));
			}
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k up = cfgBenchmark9_Passes * cfgBenchmark9_Iterations;
		printf("%u ms (%i%%),(%u u/s)\r\n", time, (time - cfgBenchmark9_Reference) * 100 / time, up * 1000 / time);
	}
	if (cfgBenchmark10_Enable)
	{  // Benchmark 10
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<const b3DbvtNode*> leaves;
		b3AlignedObjectArray<b3Vec3> vectors;
		vectors.resize(cfgBenchmark10_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (b3DbvtBenchmark::RandVector3() * 2 - b3Vec3(1, 1, 1)) * cfgBenchmark10_Scale;
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		dbvt.extractLeaves(dbvt.m_root, leaves);
		printf("[10] updates (jitter): ");
		wallclock.reset();

		for (i32 i = 0; i < cfgBenchmark10_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark10_Iterations; ++j)
			{
				const b3Vec3& d = vectors[j];
				b3DbvtNode* l = const_cast<b3DbvtNode*>(leaves[rand() % cfgLeaves]);
				b3DbvtVolume v = b3DbvtVolume::FromMM(l->volume.Mins() + d, l->volume.Maxs() + d);
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
		b3DynamicBvh dbvt;
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
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
		b3AlignedObjectArray<b3DbvtVolume> volumes;
		b3AlignedObjectArray<bool> results;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			volumes[i] = b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		printf("[12] b3DbvtVolume notequal: ");
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
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<b3Vec3> vectors;
		b3DbvtBenchmark::NilPolicy policy;
		vectors.resize(cfgBenchmark13_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (b3DbvtBenchmark::RandVector3() * 2 - b3Vec3(1, 1, 1)).normalized();
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		printf("[13] culling(OCL+fullsort): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark13_Iterations; ++i)
		{
			static const b3Scalar offset = 0;
			policy.m_depth = -D3_INFINITY;
			dbvt.collideOCL(dbvt.m_root, &vectors[i], &offset, vectors[i], 1, policy);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k t = cfgBenchmark13_Iterations;
		printf("%u ms (%i%%),(%u t/s)\r\n", time, (time - cfgBenchmark13_Reference) * 100 / time, (t * 1000) / time);
	}
	if (cfgBenchmark14_Enable)
	{  // Benchmark 14
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<b3Vec3> vectors;
		b3DbvtBenchmark::P14 policy;
		vectors.resize(cfgBenchmark14_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (b3DbvtBenchmark::RandVector3() * 2 - b3Vec3(1, 1, 1)).normalized();
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		policy.m_nodes.reserve(cfgLeaves);
		printf("[14] culling(OCL+qsort): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark14_Iterations; ++i)
		{
			static const b3Scalar offset = 0;
			policy.m_nodes.resize(0);
			dbvt.collideOCL(dbvt.m_root, &vectors[i], &offset, vectors[i], 1, policy, false);
			policy.m_nodes.quickSort(b3DbvtBenchmark::P14::sortfnc);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k t = cfgBenchmark14_Iterations;
		printf("%u ms (%i%%),(%u t/s)\r\n", time, (time - cfgBenchmark14_Reference) * 100 / time, (t * 1000) / time);
	}
	if (cfgBenchmark15_Enable)
	{  // Benchmark 15
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<b3Vec3> vectors;
		b3DbvtBenchmark::P15 policy;
		vectors.resize(cfgBenchmark15_Iterations);
		for (i32 i = 0; i < vectors.size(); ++i)
		{
			vectors[i] = (b3DbvtBenchmark::RandVector3() * 2 - b3Vec3(1, 1, 1)).normalized();
		}
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		policy.m_nodes.reserve(cfgLeaves);
		printf("[15] culling(KDOP+qsort): ");
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark15_Iterations; ++i)
		{
			static const b3Scalar offset = 0;
			policy.m_nodes.resize(0);
			policy.m_axis = vectors[i];
			dbvt.collideKDOP(dbvt.m_root, &vectors[i], &offset, 1, policy);
			policy.m_nodes.quickSort(b3DbvtBenchmark::P15::sortfnc);
		}
		i32k time = (i32)wallclock.getTimeMilliseconds();
		i32k t = cfgBenchmark15_Iterations;
		printf("%u ms (%i%%),(%u t/s)\r\n", time, (time - cfgBenchmark15_Reference) * 100 / time, (t * 1000) / time);
	}
	if (cfgBenchmark16_Enable)
	{  // Benchmark 16
		srand(380843);
		b3DynamicBvh dbvt;
		b3AlignedObjectArray<b3DbvtNode*> batch;
		b3DbvtBenchmark::RandTree(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale, cfgLeaves, dbvt);
		dbvt.optimizeTopDown();
		batch.reserve(cfgBenchmark16_BatchCount);
		printf("[16] insert/remove batch(%u): ", cfgBenchmark16_BatchCount);
		wallclock.reset();
		for (i32 i = 0; i < cfgBenchmark16_Passes; ++i)
		{
			for (i32 j = 0; j < cfgBenchmark16_BatchCount; ++j)
			{
				batch.push_back(dbvt.insert(b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale), 0));
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
		b3AlignedObjectArray<b3DbvtVolume> volumes;
		b3AlignedObjectArray<i32> results;
		b3AlignedObjectArray<i32> indices;
		volumes.resize(cfgLeaves);
		results.resize(cfgLeaves);
		indices.resize(cfgLeaves);
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			indices[i] = i;
			volumes[i] = b3DbvtBenchmark::RandVolume(cfgVolumeCenterScale, cfgVolumeExentsBase, cfgVolumeExentsScale);
		}
		for (i32 i = 0; i < cfgLeaves; ++i)
		{
			b3Swap(indices[i], indices[rand() % cfgLeaves]);
		}
		printf("[17] b3DbvtVolume select: ");
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
