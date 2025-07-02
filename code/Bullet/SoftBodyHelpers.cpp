
#include <drx3D/Physics/SoftBody/SoftBodyInternals.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <algorithm>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Maths/Linear/ConvexHull.h>
#include <drx3D/Maths/Linear/ConvexHullComputer.h>
#include <map>
#include <vector>

static void drawVertex ( IDebugDraw* idraw,
		const Vec3& x, Scalar s, const Vec3& c )
{
	idraw->drawLine ( x - Vec3 ( s, 0, 0 ), x + Vec3 ( s, 0, 0 ), c );
	idraw->drawLine ( x - Vec3 ( 0, s, 0 ), x + Vec3 ( 0, s, 0 ), c );
	idraw->drawLine ( x - Vec3 ( 0, 0, s ), x + Vec3 ( 0, 0, s ), c );
}

//
static void drawBox ( IDebugDraw* idraw,
		const Vec3& mins,
		const Vec3& maxs,
		const Vec3& color )
{
	const Vec3 c[] = {Vec3 ( mins.x(), mins.y(), mins.z() ),
					  Vec3 ( maxs.x(), mins.y(), mins.z() ),
					  Vec3 ( maxs.x(), maxs.y(), mins.z() ),
					  Vec3 ( mins.x(), maxs.y(), mins.z() ),
					  Vec3 ( mins.x(), mins.y(), maxs.z() ),
					  Vec3 ( maxs.x(), mins.y(), maxs.z() ),
					  Vec3 ( maxs.x(), maxs.y(), maxs.z() ),
					  Vec3 ( mins.x(), maxs.y(), maxs.z() )
					 };
	idraw->drawLine ( c[0], c[1], color );
	idraw->drawLine ( c[1], c[2], color );
	idraw->drawLine ( c[2], c[3], color );
	idraw->drawLine ( c[3], c[0], color );
	idraw->drawLine ( c[4], c[5], color );
	idraw->drawLine ( c[5], c[6], color );
	idraw->drawLine ( c[6], c[7], color );
	idraw->drawLine ( c[7], c[4], color );
	idraw->drawLine ( c[0], c[4], color );
	idraw->drawLine ( c[1], c[5], color );
	idraw->drawLine ( c[2], c[6], color );
	idraw->drawLine ( c[3], c[7], color );
}

//
static void drawTree ( IDebugDraw* idraw,
		const DbvtNode* node,
		i32 depth,
		const Vec3& ncolor,
		const Vec3& lcolor,
		i32 mindepth,
		i32 maxdepth )
{
	if ( node )
	{
		if ( node->isinternal() && ( ( depth < maxdepth ) || ( maxdepth < 0 ) ) )
		{
			drawTree ( idraw, node->childs[0], depth + 1, ncolor, lcolor, mindepth, maxdepth );
			drawTree ( idraw, node->childs[1], depth + 1, ncolor, lcolor, mindepth, maxdepth );
		}

		if ( depth >= mindepth )
		{
			const Scalar scl = ( Scalar ) ( node->isinternal() ? 1 : 1 );
			const Vec3 mi = node->volume.Center() - node->volume.Extents() * scl;
			const Vec3 mx = node->volume.Center() + node->volume.Extents() * scl;
			drawBox ( idraw, mi, mx, node->isleaf() ? lcolor : ncolor );
		}
	}
}

//
template <typename T>
static inline T sum ( const AlignedObjectArray<T>& items )
{
	T v;

	if ( items.size() )
	{
		v = items[0];

		for ( i32 i = 1, ni = items.size(); i < ni; ++i )
		{
			v += items[i];
		}
	}

	return ( v );
}

//
template <typename T, typename Q>
static inline void add ( AlignedObjectArray<T>& items, const Q& value )
{
	for ( i32 i = 0, ni = items.size(); i < ni; ++i )
	{
		items[i] += value;
	}
}

//
template <typename T, typename Q>
static inline void mul ( AlignedObjectArray<T>& items, const Q& value )
{
	for ( i32 i = 0, ni = items.size(); i < ni; ++i )
	{
		items[i] *= value;
	}
}

//
template <typename T>
static inline T average ( const AlignedObjectArray<T>& items )
{
	const Scalar n = ( Scalar ) ( items.size() > 0 ? items.size() : 1 );
	return ( sum ( items ) / n );
}

#if 0
//
inline static Scalar		tetravolume ( const Vec3& x0,
		const Vec3& x1,
		const Vec3& x2,
		const Vec3& x3 )
{
	const Vec3	a = x1 - x0;
	const Vec3	b = x2 - x0;
	const Vec3	c = x3 - x0;
	return ( Dot ( a, Cross ( b, c ) ) );
}

#endif

//
#if 0
static Vec3		stresscolor ( Scalar stress )
{
	static const Vec3	spectrum[] =	{	Vec3 ( 1, 0, 1 ),
			Vec3 ( 0, 0, 1 ),
			Vec3 ( 0, 1, 1 ),
			Vec3 ( 0, 1, 0 ),
			Vec3 ( 1, 1, 0 ),
			Vec3 ( 1, 0, 0 ),
			Vec3 ( 1, 0, 0 )
								   };
	static i32k		ncolors = sizeof ( spectrum ) / sizeof ( spectrum[0] ) - 1;
	static const Scalar	one = 1;
	stress = d3Max<Scalar> ( 0, d3Min<Scalar> ( 1, stress ) ) * ncolors;
	i32k				sel = ( i32 ) stress;
	const Scalar			frc = stress - sel;
	return ( spectrum[sel] + ( spectrum[sel+1] - spectrum[sel] ) *frc );
}

#endif

//
void SoftBodyHelpers::Draw ( SoftBody* psb,
		IDebugDraw* idraw,
		i32 drawflags )
{
	const Scalar scl = ( Scalar ) 0.1;
	const Scalar nscl = scl * 5;
	const Vec3 lcolor = Vec3 ( 0, 0, 0 );
	const Vec3 ncolor = Vec3 ( 1, 1, 1 );
	const Vec3 ccolor = Vec3 ( 1, 0, 0 );
	i32 i, j, nj;

	/* Clusters	*/

	if ( 0 != ( drawflags & fDrawFlags::Clusters ) )
	{
		srand ( 1806 );

		for ( i = 0; i < psb->m_clusters.size(); ++i )
		{
			if ( psb->m_clusters[i]->m_collide )
			{
				Vec3 color ( rand() / ( Scalar ) RAND_MAX,
							 rand() / ( Scalar ) RAND_MAX,
							 rand() / ( Scalar ) RAND_MAX );
				color = color.normalized() * 0.75;
				AlignedObjectArray<Vec3> vertices;
				vertices.resize ( psb->m_clusters[i]->m_nodes.size() );

				for ( j = 0, nj = vertices.size(); j < nj; ++j )
				{
					vertices[j] = psb->m_clusters[i]->m_nodes[j]->m_x;
				}

#define USE_NEW_CONVEX_HULL_COMPUTER
#ifdef USE_NEW_CONVEX_HULL_COMPUTER
				ConvexHullComputer computer;

				i32 stride = sizeof ( Vec3 );

				i32 count = vertices.size();

				Scalar shrink = 0.f;

				Scalar shrinkClamp = 0.f;

				computer.compute ( &vertices[0].getX(), stride, count, shrink, shrinkClamp );

				for ( i32 i = 0; i < computer.faces.size(); i++ )
				{
					i32 face = computer.faces[i];
					//printf("face=%d\n",face);
					const ConvexHullComputer::Edge* firstEdge = &computer.edges[face];
					const ConvexHullComputer::Edge* edge = firstEdge->getNextEdgeOfFace();

					i32 v0 = firstEdge->getSourceVertex();
					i32 v1 = firstEdge->getTargetVertex();

					while ( edge != firstEdge )
					{
						i32 v2 = edge->getTargetVertex();
						idraw->drawTriangle ( computer.vertices[v0], computer.vertices[v1], computer.vertices[v2], color, 1 );
						edge = edge->getNextEdgeOfFace();
						v0 = v1;
						v1 = v2;
					};
				}

#else

				HullDesc hdsc ( QF_TRIANGLES, vertices.size(), &vertices[0] );

				HullResult hres;

				HullLibrary hlib;

				hdsc.mMaxVertices = vertices.size();

				hlib.CreateConvexHull ( hdsc, hres );

				const Vec3 center = average ( hres.m_OutputVertices );

				add ( hres.m_OutputVertices, -center );

				mul ( hres.m_OutputVertices, ( Scalar ) 1 );

				add ( hres.m_OutputVertices, center );

				for ( j = 0; j < ( i32 ) hres.mNumFaces; ++j )
				{
					i32k idx[] = {hres.m_Indices[j * 3 + 0], hres.m_Indices[j * 3 + 1], hres.m_Indices[j * 3 + 2]};
					idraw->drawTriangle ( hres.m_OutputVertices[idx[0]],
							hres.m_OutputVertices[idx[1]],
							hres.m_OutputVertices[idx[2]],
							color, 1 );
				}

				hlib.ReleaseResult ( hres );

#endif
			}

			/* Velocities	*/
#if 0

			for ( i32 j = 0;j < psb->m_clusters[i].m_nodes.size();++j )
			{
				const SoftBody::Cluster&	c = psb->m_clusters[i];
				const Vec3				r = c.m_nodes[j]->m_x - c.m_com;
				const Vec3				v = c.m_lv + Cross ( c.m_av, r );
				idraw->drawLine ( c.m_nodes[j]->m_x, c.m_nodes[j]->m_x + v, Vec3 ( 1, 0, 0 ) );
			}

#endif
			/* Frame		*/
			//		SoftBody::Cluster& c=*psb->m_clusters[i];
			//		idraw->drawLine(c.m_com,c.m_framexform*Vec3(10,0,0),Vec3(1,0,0));
			//		idraw->drawLine(c.m_com,c.m_framexform*Vec3(0,10,0),Vec3(0,1,0));
			//		idraw->drawLine(c.m_com,c.m_framexform*Vec3(0,0,10),Vec3(0,0,1));
		}
	}

	else
	{
		/* Nodes	*/
		if ( 0 != ( drawflags & fDrawFlags::Nodes ) )
		{
			for ( i = 0; i < psb->m_nodes.size(); ++i )
			{
				const SoftBody::Node& n = psb->m_nodes[i];

				if ( 0 == ( n.m_material->m_flags & SoftBody::fMaterial::DebugDraw ) )
					continue;

				idraw->drawLine ( n.m_x - Vec3 ( scl, 0, 0 ), n.m_x + Vec3 ( scl, 0, 0 ), Vec3 ( 1, 0, 0 ) );

				idraw->drawLine ( n.m_x - Vec3 ( 0, scl, 0 ), n.m_x + Vec3 ( 0, scl, 0 ), Vec3 ( 0, 1, 0 ) );

				idraw->drawLine ( n.m_x - Vec3 ( 0, 0, scl ), n.m_x + Vec3 ( 0, 0, scl ), Vec3 ( 0, 0, 1 ) );
			}
		}

		/* Links	*/

		if ( 0 != ( drawflags & fDrawFlags::Links ) )
		{
			for ( i = 0; i < psb->m_links.size(); ++i )
			{
				const SoftBody::Link& l = psb->m_links[i];

				if ( 0 == ( l.m_material->m_flags & SoftBody::fMaterial::DebugDraw ) )
					continue;

				idraw->drawLine ( l.m_n[0]->m_x, l.m_n[1]->m_x, lcolor );
			}
		}

		/* Normals	*/

		if ( 0 != ( drawflags & fDrawFlags::Normals ) )
		{
			for ( i = 0; i < psb->m_nodes.size(); ++i )
			{
				const SoftBody::Node& n = psb->m_nodes[i];

				if ( 0 == ( n.m_material->m_flags & SoftBody::fMaterial::DebugDraw ) )
					continue;

				const Vec3 d = n.m_n * nscl;

				idraw->drawLine ( n.m_x, n.m_x + d, ncolor );

				idraw->drawLine ( n.m_x, n.m_x - d, ncolor * 0.5 );
			}
		}

		/* Contacts	*/

		if ( 0 != ( drawflags & fDrawFlags::Contacts ) )
		{
			static const Vec3 axis[] = {Vec3 ( 1, 0, 0 ),
					Vec3 ( 0, 1, 0 ),
					Vec3 ( 0, 0, 1 )
									   };

			for ( i = 0; i < psb->m_rcontacts.size(); ++i )
			{
				const SoftBody::RContact& c = psb->m_rcontacts[i];
				const Vec3 o = c.m_node->m_x - c.m_cti.m_normal *
							   ( Dot ( c.m_node->m_x, c.m_cti.m_normal ) + c.m_cti.m_offset );
				const Vec3 x = Cross ( c.m_cti.m_normal, axis[c.m_cti.m_normal.minAxis() ] ).normalized();
				const Vec3 y = Cross ( x, c.m_cti.m_normal ).normalized();
				idraw->drawLine ( o - x * nscl, o + x * nscl, ccolor );
				idraw->drawLine ( o - y * nscl, o + y * nscl, ccolor );
				idraw->drawLine ( o, o + c.m_cti.m_normal * nscl * 3, Vec3 ( 1, 1, 0 ) );
			}
		}

		/* Faces	*/

		if ( 0 != ( drawflags & fDrawFlags::Faces ) )
		{
			const Scalar scl = ( Scalar ) 0.8;
			const Scalar alp = ( Scalar ) 1;
			const Vec3 col ( 0, ( Scalar ) 0.7, 0 );

			for ( i = 0; i < psb->m_faces.size(); ++i )
			{
				const SoftBody::Face& f = psb->m_faces[i];

				if ( 0 == ( f.m_material->m_flags & SoftBody::fMaterial::DebugDraw ) )
					continue;

				const Vec3 x[] = {f.m_n[0]->m_x, f.m_n[1]->m_x, f.m_n[2]->m_x};

				const Vec3 c = ( x[0] + x[1] + x[2] ) / 3;

				idraw->drawTriangle ( ( x[0] - c ) * scl + c,
						( x[1] - c ) * scl + c,
						( x[2] - c ) * scl + c,
						col, alp );
			}
		}

		/* Tetras	*/

		if ( 0 != ( drawflags & fDrawFlags::Tetras ) )
		{
			const Scalar scl = ( Scalar ) 0.8;
			const Scalar alp = ( Scalar ) 1;
			const Vec3 col ( ( Scalar ) 0.3, ( Scalar ) 0.3, ( Scalar ) 0.7 );

			for ( i32 i = 0; i < psb->m_tetras.size(); ++i )
			{
				const SoftBody::Tetra& t = psb->m_tetras[i];

				if ( 0 == ( t.m_material->m_flags & SoftBody::fMaterial::DebugDraw ) )
					continue;

				const Vec3 x[] = {t.m_n[0]->m_x, t.m_n[1]->m_x, t.m_n[2]->m_x, t.m_n[3]->m_x};

				const Vec3 c = ( x[0] + x[1] + x[2] + x[3] ) / 4;

				idraw->drawTriangle ( ( x[0] - c ) * scl + c, ( x[1] - c ) * scl + c, ( x[2] - c ) * scl + c, col, alp );

				idraw->drawTriangle ( ( x[0] - c ) * scl + c, ( x[1] - c ) * scl + c, ( x[3] - c ) * scl + c, col, alp );

				idraw->drawTriangle ( ( x[1] - c ) * scl + c, ( x[2] - c ) * scl + c, ( x[3] - c ) * scl + c, col, alp );

				idraw->drawTriangle ( ( x[2] - c ) * scl + c, ( x[0] - c ) * scl + c, ( x[3] - c ) * scl + c, col, alp );
			}
		}
	}

	/* Anchors	*/

	if ( 0 != ( drawflags & fDrawFlags::Anchors ) )
	{
		for ( i = 0; i < psb->m_anchors.size(); ++i )
		{
			const SoftBody::Anchor& a = psb->m_anchors[i];
			const Vec3 q = a.m_body->getWorldTransform() * a.m_local;
			drawVertex ( idraw, a.m_node->m_x, 0.25, Vec3 ( 1, 0, 0 ) );
			drawVertex ( idraw, q, 0.25, Vec3 ( 0, 1, 0 ) );
			idraw->drawLine ( a.m_node->m_x, q, Vec3 ( 1, 1, 1 ) );
		}

		for ( i = 0; i < psb->m_nodes.size(); ++i )
		{
			const SoftBody::Node& n = psb->m_nodes[i];

			if ( 0 == ( n.m_material->m_flags & SoftBody::fMaterial::DebugDraw ) )
				continue;

			if ( n.m_im <= 0 )
			{
				drawVertex ( idraw, n.m_x, 0.25, Vec3 ( 1, 0, 0 ) );
			}
		}
	}

	/* Notes	*/

	if ( 0 != ( drawflags & fDrawFlags::Notes ) )
	{
		for ( i = 0; i < psb->m_notes.size(); ++i )
		{
			const SoftBody::Note& n = psb->m_notes[i];
			Vec3 p = n.m_offset;

			for ( i32 j = 0; j < n.m_rank; ++j )
			{
				p += n.m_nodes[j]->m_x * n.m_coords[j];
			}

			idraw->draw3dText ( p, n.m_text );
		}
	}

	/* Node tree	*/

	if ( 0 != ( drawflags & fDrawFlags::NodeTree ) )
		DrawNodeTree ( psb, idraw );

	/* Face tree	*/
	if ( 0 != ( drawflags & fDrawFlags::FaceTree ) )
		DrawFaceTree ( psb, idraw );

	/* Cluster tree	*/
	if ( 0 != ( drawflags & fDrawFlags::ClusterTree ) )
		DrawClusterTree ( psb, idraw );

	/* Joints		*/
	if ( 0 != ( drawflags & fDrawFlags::Joints ) )
	{
		for ( i = 0; i < psb->m_joints.size(); ++i )
		{
			const SoftBody::Joint* pj = psb->m_joints[i];

			switch ( pj->Type() )
			{

				case SoftBody::Joint::eType::Linear:
					{
						const SoftBody::LJoint* pjl = ( const SoftBody::LJoint* ) pj;
						const Vec3 a0 = pj->m_bodies[0].xform() * pjl->m_refs[0];
						const Vec3 a1 = pj->m_bodies[1].xform() * pjl->m_refs[1];
						idraw->drawLine ( pj->m_bodies[0].xform().getOrigin(), a0, Vec3 ( 1, 1, 0 ) );
						idraw->drawLine ( pj->m_bodies[1].xform().getOrigin(), a1, Vec3 ( 0, 1, 1 ) );
						drawVertex ( idraw, a0, 0.25, Vec3 ( 1, 1, 0 ) );
						drawVertex ( idraw, a1, 0.25, Vec3 ( 0, 1, 1 ) );
					}

					break;

				case SoftBody::Joint::eType::Angular:
					{
						//const SoftBody::AJoint*	pja=(const SoftBody::AJoint*)pj;
						const Vec3 o0 = pj->m_bodies[0].xform().getOrigin();
						const Vec3 o1 = pj->m_bodies[1].xform().getOrigin();
						const Vec3 a0 = pj->m_bodies[0].xform().getBasis() * pj->m_refs[0];
						const Vec3 a1 = pj->m_bodies[1].xform().getBasis() * pj->m_refs[1];
						idraw->drawLine ( o0, o0 + a0 * 10, Vec3 ( 1, 1, 0 ) );
						idraw->drawLine ( o0, o0 + a1 * 10, Vec3 ( 1, 1, 0 ) );
						idraw->drawLine ( o1, o1 + a0 * 10, Vec3 ( 0, 1, 1 ) );
						idraw->drawLine ( o1, o1 + a1 * 10, Vec3 ( 0, 1, 1 ) );
						break;
					}

				default:
					{
					}
			}
		}
	}
}

//
void SoftBodyHelpers::DrawInfos ( SoftBody* psb,
		IDebugDraw* idraw,
		bool masses,
		bool areas,
		bool /*stress*/ )
{
	for ( i32 i = 0; i < psb->m_nodes.size(); ++i )
	{
		const SoftBody::Node& n = psb->m_nodes[i];
		char text[2048] = {0};
		char buff[1024];

		if ( masses )
		{
			sprintf ( buff, " M(%.2f)", 1 / n.m_im );
			strcat ( text, buff );
		}

		if ( areas )
		{
			sprintf ( buff, " A(%.2f)", n.m_area );
			strcat ( text, buff );
		}

		if ( text[0] )
			idraw->draw3dText ( n.m_x, text );
	}
}

//
void SoftBodyHelpers::DrawNodeTree ( SoftBody* psb,
		IDebugDraw* idraw,
		i32 mindepth,
		i32 maxdepth )
{
	drawTree ( idraw, psb->m_ndbvt.m_root, 0, Vec3 ( 1, 0, 1 ), Vec3 ( 1, 1, 1 ), mindepth, maxdepth );
}

//
void SoftBodyHelpers::DrawFaceTree ( SoftBody* psb,
		IDebugDraw* idraw,
		i32 mindepth,
		i32 maxdepth )
{
	drawTree ( idraw, psb->m_fdbvt.m_root, 0, Vec3 ( 0, 1, 0 ), Vec3 ( 1, 0, 0 ), mindepth, maxdepth );
}

//
void SoftBodyHelpers::DrawClusterTree ( SoftBody* psb,
		IDebugDraw* idraw,
		i32 mindepth,
		i32 maxdepth )
{
	drawTree ( idraw, psb->m_cdbvt.m_root, 0, Vec3 ( 0, 1, 1 ), Vec3 ( 1, 0, 0 ), mindepth, maxdepth );
}

//The SoftBody object from the BulletSDK includes an array of Nodes and Links. These links appear
// to be first set up to connect a node to between 5 and 6 of its neighbors [480 links],
//and then to the rest of the nodes after the execution of the Floyd-Warshall graph algorithm
//[another 930 links].
//The way the links are stored by default, we have a number of cases where adjacent links share a node in common
// - this leads to the creation of a data dependency through memory.
//The PSolve_Links() function reads and writes nodes as it iterates over each link.
//So, we now have the possibility of a data dependency between iteration X
//that processes link L with iteration X+1 that processes link L+1
//because L and L+1 have one node in common, and iteration X updates the positions of that node,
//and iteration X+1 reads in the position of that shared node.
//
//Such a memory dependency limits the ability of a modern CPU to speculate beyond
//a certain point because it has to respect a possible dependency
//- this prevents the CPU from making full use of its out-of-order resources.
//If we re-order the links such that we minimize the cases where a link L and L+1 share a common node,
//we create a temporal gap between when the node position is written,
//and when it is subsequently read. This in turn allows the CPU to continue execution without
//risking a dependency violation. Such a reordering would result in significant speedups on
//modern CPUs with lots of execution resources.
//In our testing, we see it have a tremendous impact not only on the A7,
//but also on all x86 cores that ship with modern Macs.
//The attached source file includes a single function (ReoptimizeLinkOrder) which can be called on a
//SoftBody object in the solveConstraints() function before the actual solver is invoked,
//or right after generateBendingConstraints() once we have all 1410 links.

//===================================================================
//
//
// This function takes in a list of interdependent Links and tries
// to maximize the distance between calculation
// of dependent links.  This increases the amount of parallelism that can
// be exploited by out-of-order instruction processors with large but
// (inevitably) finite instruction windows.
//
//===================================================================

// A small structure to track lists of dependent link calculations

class LinkDeps_t
{

	public:
		i32 value;         // A link calculation that is dependent on this one
		// Positive values = "input A" while negative values = "input B"
		LinkDeps_t* next;  // Next dependence in the list
};

typedef LinkDeps_t* LinkDepsPtr_t;

// Dependency list constants
#define REOP_NOT_DEPENDENT -1
#define REOP_NODE_COMPLETE -2  // Must be less than REOP_NOT_DEPENDENT

void SoftBodyHelpers::ReoptimizeLinkOrder ( SoftBody* psb /* This can be replaced by a SoftBody pointer */ )
{
	i32 i, nLinks = psb->m_links.size(), nNodes = psb->m_nodes.size();
	SoftBody::Link* lr;
	i32 ar, br;
	SoftBody::Node* node0 = & ( psb->m_nodes[0] );
	SoftBody::Node* node1 = & ( psb->m_nodes[1] );
	LinkDepsPtr_t linkDep;
	i32 readyListHead, readyListTail, linkNum, linkDepFrees, depLink;

	// Allocate temporary buffers
	i32* nodeWrittenAt = new i32[nNodes + 1];  // What link calculation produced this node's current values?
	i32* linkDepA = new i32[nLinks];           // Link calculation input is dependent upon prior calculation #N
	i32* linkDepB = new i32[nLinks];
	i32* readyList = new i32[nLinks];                              // List of ready-to-process link calculations (# of links, maximum)
	LinkDeps_t* linkDepFreeList = new LinkDeps_t[2 * nLinks];      // Dependent-on-me list elements (2x# of links, maximum)
	LinkDepsPtr_t* linkDepListStarts = new LinkDepsPtr_t[nLinks];  // Start nodes of dependent-on-me lists, one for each link

	// Copy the original, unsorted links to a side buffer
	SoftBody::Link* linkBuffer = new SoftBody::Link[nLinks];
	memcpy ( linkBuffer, & ( psb->m_links[0] ), sizeof ( SoftBody::Link ) * nLinks );

	// Clear out the node setup and ready list

	for ( i = 0; i < nNodes + 1; i++ )
	{
		nodeWrittenAt[i] = REOP_NOT_DEPENDENT;
	}

	for ( i = 0; i < nLinks; i++ )
	{
		linkDepListStarts[i] = NULL;
	}

	readyListHead = readyListTail = linkDepFrees = 0;

	// Initial link analysis to set up data structures

	for ( i = 0; i < nLinks; i++ )
	{
		// Note which prior link calculations we are dependent upon & build up dependence lists
		lr = & ( psb->m_links[i] );
		ar = ( lr->m_n[0] - node0 ) / ( node1 - node0 );
		br = ( lr->m_n[1] - node0 ) / ( node1 - node0 );

		if ( nodeWrittenAt[ar] > REOP_NOT_DEPENDENT )
		{
			linkDepA[i] = nodeWrittenAt[ar];
			linkDep = &linkDepFreeList[linkDepFrees++];
			linkDep->value = i;
			linkDep->next = linkDepListStarts[nodeWrittenAt[ar]];
			linkDepListStarts[nodeWrittenAt[ar]] = linkDep;
		}

		else
		{
			linkDepA[i] = REOP_NOT_DEPENDENT;
		}

		if ( nodeWrittenAt[br] > REOP_NOT_DEPENDENT )
		{
			linkDepB[i] = nodeWrittenAt[br];
			linkDep = &linkDepFreeList[linkDepFrees++];
			linkDep->value = - ( i + 1 );
			linkDep->next = linkDepListStarts[nodeWrittenAt[br]];
			linkDepListStarts[nodeWrittenAt[br]] = linkDep;
		}

		else
		{
			linkDepB[i] = REOP_NOT_DEPENDENT;
		}

		// Add this link to the initial ready list, if it is not dependent on any other links

		if ( ( linkDepA[i] == REOP_NOT_DEPENDENT ) && ( linkDepB[i] == REOP_NOT_DEPENDENT ) )
		{
			readyList[readyListTail++] = i;
			linkDepA[i] = linkDepB[i] = REOP_NODE_COMPLETE;  // Probably not needed now
		}

		// Update the nodes to mark which ones are calculated by this link
		nodeWrittenAt[ar] = nodeWrittenAt[br] = i;
	}

	// Process the ready list and create the sorted list of links
	// -- By treating the ready list as a queue, we maximize the distance between any
	//    inter-dependent node calculations
	// -- All other (non-related) nodes in the ready list will automatically be inserted
	//    in between each set of inter-dependent link calculations by this loop
	i = 0;

	while ( readyListHead != readyListTail )
	{
		// Use ready list to select the next link to process
		linkNum = readyList[readyListHead++];
		// Copy the next-to-calculate link back into the original link array
		psb->m_links[i++] = linkBuffer[linkNum];

		// Free up any link inputs that are dependent on this one
		linkDep = linkDepListStarts[linkNum];

		while ( linkDep )
		{
			depLink = linkDep->value;

			if ( depLink >= 0 )
			{
				linkDepA[depLink] = REOP_NOT_DEPENDENT;
			}

			else
			{
				depLink = -depLink - 1;
				linkDepB[depLink] = REOP_NOT_DEPENDENT;
			}

			// Add this dependent link calculation to the ready list if *both* inputs are clear

			if ( ( linkDepA[depLink] == REOP_NOT_DEPENDENT ) && ( linkDepB[depLink] == REOP_NOT_DEPENDENT ) )
			{
				readyList[readyListTail++] = depLink;
				linkDepA[depLink] = linkDepB[depLink] = REOP_NODE_COMPLETE;  // Probably not needed now
			}

			linkDep = linkDep->next;
		}
	}

	// Delete the temporary buffers
	delete[] nodeWrittenAt;

	delete[] linkDepA;

	delete[] linkDepB;

	delete[] readyList;

	delete[] linkDepFreeList;

	delete[] linkDepListStarts;

	delete[] linkBuffer;
}

//
void SoftBodyHelpers::DrawFrame ( SoftBody* psb,
		IDebugDraw* idraw )
{
	if ( psb->m_pose.m_bframe )
	{
		static const Scalar ascl = 10;
		static const Scalar nscl = ( Scalar ) 0.1;
		const Vec3 com = psb->m_pose.m_com;
		const Matrix3x3 trs = psb->m_pose.m_rot * psb->m_pose.m_scl;
		const Vec3 Xaxis = ( trs * Vec3 ( 1, 0, 0 ) ).normalized();
		const Vec3 Yaxis = ( trs * Vec3 ( 0, 1, 0 ) ).normalized();
		const Vec3 Zaxis = ( trs * Vec3 ( 0, 0, 1 ) ).normalized();
		idraw->drawLine ( com, com + Xaxis * ascl, Vec3 ( 1, 0, 0 ) );
		idraw->drawLine ( com, com + Yaxis * ascl, Vec3 ( 0, 1, 0 ) );
		idraw->drawLine ( com, com + Zaxis * ascl, Vec3 ( 0, 0, 1 ) );

		for ( i32 i = 0; i < psb->m_pose.m_pos.size(); ++i )
		{
			const Vec3 x = com + trs * psb->m_pose.m_pos[i];
			drawVertex ( idraw, x, nscl, Vec3 ( 1, 0, 1 ) );
		}
	}
}

//
SoftBody* SoftBodyHelpers::CreateRope ( SoftBodyWorldInfo& worldInfo, const Vec3& from,
		const Vec3& to,
		i32 res,
		i32 fixeds )
{
	/* Create nodes	*/
	i32k r = res + 2;
	Vec3* x = new Vec3[r];
	Scalar* m = new Scalar[r];
	i32 i;

	for ( i = 0; i < r; ++i )
	{
		const Scalar t = i / ( Scalar ) ( r - 1 );
		x[i] = lerp ( from, to, t );
		m[i] = 1;
	}

	SoftBody* psb = new SoftBody ( &worldInfo, r, x, m );

	if ( fixeds & 1 )
		psb->setMass ( 0, 0 );

	if ( fixeds & 2 )
		psb->setMass ( r - 1, 0 );

	delete[] x;

	delete[] m;

	/* Create links	*/
	for ( i = 1; i < r; ++i )
	{
		psb->appendLink ( i - 1, i );
	}

	/* Finished		*/
	return ( psb );
}

//
SoftBody* SoftBodyHelpers::CreatePatch ( SoftBodyWorldInfo& worldInfo, const Vec3& corner00,
		const Vec3& corner10,
		const Vec3& corner01,
		const Vec3& corner11,
		i32 resx,
		i32 resy,
		i32 fixeds,
		bool gendiags,
		Scalar perturbation )
{
#define IDX(_x_, _y_) ((_y_)*rx + (_x_))
	/* Create nodes	*/

	if ( ( resx < 2 ) || ( resy < 2 ) )
		return ( 0 );

	i32k rx = resx;

	i32k ry = resy;

	i32k tot = rx * ry;

	Vec3* x = new Vec3[tot];

	Scalar* m = new Scalar[tot];

	i32 iy;

	for ( iy = 0; iy < ry; ++iy )
	{
		const Scalar ty = iy / ( Scalar ) ( ry - 1 );
		const Vec3 py0 = lerp ( corner00, corner01, ty );
		const Vec3 py1 = lerp ( corner10, corner11, ty );

		for ( i32 ix = 0; ix < rx; ++ix )
		{
			const Scalar tx = ix / ( Scalar ) ( rx - 1 );
			Scalar pert = perturbation * Scalar ( rand() / RAND_MAX);
			Vec3 temp1 = py1;
			temp1.setY ( py1.getY() + pert );
			Vec3 temp = py0;
			pert = perturbation * Scalar ( rand() / RAND_MAX);
			temp.setY ( py0.getY() + pert );
			x[IDX ( ix, iy ) ] = lerp ( temp, temp1, tx );
			m[IDX ( ix, iy ) ] = 1;
		}
	}

	SoftBody* psb = new SoftBody ( &worldInfo, tot, x, m );

	if ( fixeds & 1 )
		psb->setMass ( IDX ( 0, 0 ), 0 );

	if ( fixeds & 2 )
		psb->setMass ( IDX ( rx - 1, 0 ), 0 );

	if ( fixeds & 4 )
		psb->setMass ( IDX ( 0, ry - 1 ), 0 );

	if ( fixeds & 8 )
		psb->setMass ( IDX ( rx - 1, ry - 1 ), 0 );

	delete[] x;

	delete[] m;

	/* Create links	and faces */
	for ( iy = 0; iy < ry; ++iy )
	{
		for ( i32 ix = 0; ix < rx; ++ix )
		{
			i32k idx = IDX ( ix, iy );
			const bool mdx = ( ix + 1 ) < rx;
			const bool mdy = ( iy + 1 ) < ry;

			if ( mdx )
				psb->appendLink ( idx, IDX ( ix + 1, iy ) );

			if ( mdy )
				psb->appendLink ( idx, IDX ( ix, iy + 1 ) );

			if ( mdx && mdy )
			{
				if ( ( ix + iy ) & 1 )
				{
					psb->appendFace ( IDX ( ix, iy ), IDX ( ix + 1, iy ), IDX ( ix + 1, iy + 1 ) );
					psb->appendFace ( IDX ( ix, iy ), IDX ( ix + 1, iy + 1 ), IDX ( ix, iy + 1 ) );

					if ( gendiags )
					{
						psb->appendLink ( IDX ( ix, iy ), IDX ( ix + 1, iy + 1 ) );
					}
				}

				else
				{
					psb->appendFace ( IDX ( ix, iy + 1 ), IDX ( ix, iy ), IDX ( ix + 1, iy ) );
					psb->appendFace ( IDX ( ix, iy + 1 ), IDX ( ix + 1, iy ), IDX ( ix + 1, iy + 1 ) );

					if ( gendiags )
					{
						psb->appendLink ( IDX ( ix + 1, iy ), IDX ( ix, iy + 1 ) );
					}
				}
			}
		}
	}

	/* Finished		*/
#undef IDX
	return ( psb );
}

//
SoftBody* SoftBodyHelpers::CreatePatchUV ( SoftBodyWorldInfo& worldInfo,
		const Vec3& corner00,
		const Vec3& corner10,
		const Vec3& corner01,
		const Vec3& corner11,
		i32 resx,
		i32 resy,
		i32 fixeds,
		bool gendiags,
		float* tex_coords )
{
	/*
	*
	*  corners:
	*
	*  [0][0]     corner00 ------- corner01   [resx][0]
	*                |                |
	*                |                |
	*  [0][resy]  corner10 -------- corner11  [resx][resy]
	*
	*
	*
	*
	*
	*
	*   "fixedgs" map:
	*
	*  corner00     -->   +1
	*  corner01     -->   +2
	*  corner10     -->   +4
	*  corner11     -->   +8
	*  upper middle -->  +16
	*  left middle  -->  +32
	*  right middle -->  +64
	*  lower middle --> +128
	*  center       --> +256
	*
	*
	*   tex_coords size   (resx-1)*(resy-1)*12
	*
	*
	*
	*     SINGLE QUAD INTERNALS
	*
	*  1) SoftBody's nodes and links,
	*     diagonal link is optional ("gendiags")
	*
	*
	*    node00 ------ node01
	*      | .
	*      |   .
	*      |     .
	*      |       .
	*      |         .
	*    node10        node11
	*
	*
	*
	*   2) Faces:
	*      two triangles,
	*      UV Coordinates (hier example for single quad)
	*
	*     (0,1)          (0,1)  (1,1)
	*     1 |\            3 \-----| 2
	*       | \              \    |
	*       |  \              \   |
	*       |   \              \  |
	*       |    \              \ |
	*     2 |-----\ 3            \| 1
	*     (0,0)    (1,0)       (1,0)
	*
	*
	*
	*
	*
	*
	*/

#define IDX(_x_, _y_) ((_y_)*rx + (_x_))
	/* Create nodes		*/
	if ( ( resx < 2 ) || ( resy < 2 ) )
		return ( 0 );

	i32k rx = resx;

	i32k ry = resy;

	i32k tot = rx * ry;

	Vec3* x = new Vec3[tot];

	Scalar* m = new Scalar[tot];

	i32 iy;

	for ( iy = 0; iy < ry; ++iy )
	{
		const Scalar ty = iy / ( Scalar ) ( ry - 1 );
		const Vec3 py0 = lerp ( corner00, corner01, ty );
		const Vec3 py1 = lerp ( corner10, corner11, ty );

		for ( i32 ix = 0; ix < rx; ++ix )
		{
			const Scalar tx = ix / ( Scalar ) ( rx - 1 );
			x[IDX ( ix, iy ) ] = lerp ( py0, py1, tx );
			m[IDX ( ix, iy ) ] = 1;
		}
	}

	SoftBody* psb = new SoftBody ( &worldInfo, tot, x, m );

	if ( fixeds & 1 )
		psb->setMass ( IDX ( 0, 0 ), 0 );

	if ( fixeds & 2 )
		psb->setMass ( IDX ( rx - 1, 0 ), 0 );

	if ( fixeds & 4 )
		psb->setMass ( IDX ( 0, ry - 1 ), 0 );

	if ( fixeds & 8 )
		psb->setMass ( IDX ( rx - 1, ry - 1 ), 0 );

	if ( fixeds & 16 )
		psb->setMass ( IDX ( ( rx - 1 ) / 2, 0 ), 0 );

	if ( fixeds & 32 )
		psb->setMass ( IDX ( 0, ( ry - 1 ) / 2 ), 0 );

	if ( fixeds & 64 )
		psb->setMass ( IDX ( rx - 1, ( ry - 1 ) / 2 ), 0 );

	if ( fixeds & 128 )
		psb->setMass ( IDX ( ( rx - 1 ) / 2, ry - 1 ), 0 );

	if ( fixeds & 256 )
		psb->setMass ( IDX ( ( rx - 1 ) / 2, ( ry - 1 ) / 2 ), 0 );

	delete[] x;

	delete[] m;

	i32 z = 0;

	/* Create links	and faces	*/
	for ( iy = 0; iy < ry; ++iy )
	{
		for ( i32 ix = 0; ix < rx; ++ix )
		{
			const bool mdx = ( ix + 1 ) < rx;
			const bool mdy = ( iy + 1 ) < ry;

			i32 node00 = IDX ( ix, iy );
			i32 node01 = IDX ( ix + 1, iy );
			i32 node10 = IDX ( ix, iy + 1 );
			i32 node11 = IDX ( ix + 1, iy + 1 );

			if ( mdx )
				psb->appendLink ( node00, node01 );

			if ( mdy )
				psb->appendLink ( node00, node10 );

			if ( mdx && mdy )
			{
				psb->appendFace ( node00, node10, node11 );

				if ( tex_coords )
				{
					tex_coords[z + 0] = CalculateUV ( resx, resy, ix, iy, 0 );
					tex_coords[z + 1] = CalculateUV ( resx, resy, ix, iy, 1 );
					tex_coords[z + 2] = CalculateUV ( resx, resy, ix, iy, 0 );
					tex_coords[z + 3] = CalculateUV ( resx, resy, ix, iy, 2 );
					tex_coords[z + 4] = CalculateUV ( resx, resy, ix, iy, 3 );
					tex_coords[z + 5] = CalculateUV ( resx, resy, ix, iy, 2 );
				}

				psb->appendFace ( node11, node01, node00 );

				if ( tex_coords )
				{
					tex_coords[z + 6] = CalculateUV ( resx, resy, ix, iy, 3 );
					tex_coords[z + 7] = CalculateUV ( resx, resy, ix, iy, 2 );
					tex_coords[z + 8] = CalculateUV ( resx, resy, ix, iy, 3 );
					tex_coords[z + 9] = CalculateUV ( resx, resy, ix, iy, 1 );
					tex_coords[z + 10] = CalculateUV ( resx, resy, ix, iy, 0 );
					tex_coords[z + 11] = CalculateUV ( resx, resy, ix, iy, 1 );
				}

				if ( gendiags )
					psb->appendLink ( node00, node11 );

				z += 12;
			}
		}
	}

	/* Finished	*/
#undef IDX
	return ( psb );
}

float SoftBodyHelpers::CalculateUV ( i32 resx, i32 resy, i32 ix, i32 iy, i32 id )
{
	/*
	*
	*
	*    node00 --- node01
	*      |          |
	*    node10 --- node11
	*
	*
	*   ID map:
	*
	*   node00 s --> 0
	*   node00 t --> 1
	*
	*   node01 s --> 3
	*   node01 t --> 1
	*
	*   node10 s --> 0
	*   node10 t --> 2
	*
	*   node11 s --> 3
	*   node11 t --> 2
	*
	*
	*/

	float tc = 0.0f;

	if ( id == 0 )
	{
		tc = ( 1.0f / ( ( resx - 1 ) ) * ix );
	}

	else
		if ( id == 1 )
		{
			tc = ( 1.0f / ( ( resy - 1 ) ) * ( resy - 1 - iy ) );
		}

		else
			if ( id == 2 )
			{
				tc = ( 1.0f / ( ( resy - 1 ) ) * ( resy - 1 - iy - 1 ) );
			}

			else
				if ( id == 3 )
				{
					tc = ( 1.0f / ( ( resx - 1 ) ) * ( ix + 1 ) );
				}

	return tc;
}

//
SoftBody* SoftBodyHelpers::CreateEllipsoid ( SoftBodyWorldInfo& worldInfo, const Vec3& center,
		const Vec3& radius,
		i32 res )
{

	struct Hammersley
	{
		static void Generate ( Vec3* x, i32 n )
		{
			for ( i32 i = 0; i < n; i++ )
			{
				Scalar p = 0.5, t = 0;

				for ( i32 j = i; j; p *= 0.5, j >>= 1 )
					if ( j & 1 )
						t += p;

				Scalar w = 2 * t - 1;

				Scalar a = ( SIMD_PI + 2 * i * SIMD_PI ) / n;

				Scalar s = Sqrt ( 1 - w * w );

				*x++ = Vec3 ( s * Cos ( a ), s * Sin ( a ), w );
			}
		}
	};

	AlignedObjectArray<Vec3> vtx;
	vtx.resize ( 3 + res );
	Hammersley::Generate ( &vtx[0], vtx.size() );

	for ( i32 i = 0; i < vtx.size(); ++i )
	{
		vtx[i] = vtx[i] * radius + center;
	}

	return ( CreateFromConvexHull ( worldInfo, &vtx[0], vtx.size() ) );
}

//
SoftBody* SoftBodyHelpers::CreateFromTriMesh ( SoftBodyWorldInfo& worldInfo, const Scalar* vertices,
		i32k* triangles,
		i32 ntriangles, bool randomizeConstraints )
{
	i32 maxidx = 0;
	i32 i, j, ni;

	for ( i = 0, ni = ntriangles * 3; i < ni; ++i )
	{
		maxidx = d3Max ( triangles[i], maxidx );
	}

	++maxidx;

	AlignedObjectArray<bool> chks;
	AlignedObjectArray<Vec3> vtx;
	chks.resize ( maxidx * maxidx, false );
	vtx.resize ( maxidx );

	for ( i = 0, j = 0, ni = maxidx * 3; i < ni; ++j, i += 3 )
	{
		vtx[j] = Vec3 ( vertices[i], vertices[i + 1], vertices[i + 2] );
	}

	SoftBody* psb = new SoftBody ( &worldInfo, vtx.size(), &vtx[0], 0 );

	for ( i = 0, ni = ntriangles * 3; i < ni; i += 3 )
	{
		i32k idx[] = {triangles[i], triangles[i + 1], triangles[i + 2]};
#define IDX(_x_, _y_) ((_y_)*maxidx + (_x_))

		for ( i32 j = 2, k = 0; k < 3; j = k++ )
		{
			if ( !chks[IDX ( idx[j], idx[k] ) ] )
			{
				chks[IDX ( idx[j], idx[k] ) ] = true;
				chks[IDX ( idx[k], idx[j] ) ] = true;
				psb->appendLink ( idx[j], idx[k] );
			}
		}

#undef IDX
		psb->appendFace ( idx[0], idx[1], idx[2] );
	}

	if ( randomizeConstraints )
	{
		psb->randomizeConstraints();
	}

	return ( psb );
}

//
SoftBody* SoftBodyHelpers::CreateFromConvexHull ( SoftBodyWorldInfo& worldInfo, const Vec3* vertices,
		i32 nvertices, bool randomizeConstraints )
{
	HullDesc hdsc ( QF_TRIANGLES, nvertices, vertices );
	HullResult hres;
	HullLibrary hlib; /*??*/
	hdsc.mMaxVertices = nvertices;
	hlib.CreateConvexHull ( hdsc, hres );
	SoftBody* psb = new SoftBody ( &worldInfo, ( i32 ) hres.mNumOutputVertices,
			&hres.m_OutputVertices[0], 0 );

	for ( i32 i = 0; i < ( i32 ) hres.mNumFaces; ++i )
	{
		i32k idx[] = {static_cast<i32> ( hres.m_Indices[i * 3 + 0] ),
						   static_cast<i32> ( hres.m_Indices[i * 3 + 1] ),
						   static_cast<i32> ( hres.m_Indices[i * 3 + 2] )
						  };

		if ( idx[0] < idx[1] )
			psb->appendLink ( idx[0], idx[1] );

		if ( idx[1] < idx[2] )
			psb->appendLink ( idx[1], idx[2] );

		if ( idx[2] < idx[0] )
			psb->appendLink ( idx[2], idx[0] );

		psb->appendFace ( idx[0], idx[1], idx[2] );
	}

	hlib.ReleaseResult ( hres );

	if ( randomizeConstraints )
	{
		psb->randomizeConstraints();
	}

	return ( psb );
}

static i32 nextLine ( tukk buffer )
{
	i32 numBytesRead = 0;

	while ( *buffer != '\n' )
	{
		buffer++;
		numBytesRead++;
	}

	if ( buffer[0] == 0x0a )
	{
		buffer++;
		numBytesRead++;
	}

	return numBytesRead;
}

/* Create from TetGen .ele, .face, .node data							*/
SoftBody* SoftBodyHelpers::CreateFromTetGenData ( SoftBodyWorldInfo& worldInfo,
		tukk ele,
		tukk face,
		tukk node,
		bool bfacelinks,
		bool btetralinks,
		bool bfacesfromtetras )
{
	AlignedObjectArray<Vec3> pos;
	i32 nnode = 0;
	i32 ndims = 0;
	i32 nattrb = 0;
	i32 hasbounds = 0;
	i32 result = sscanf ( node, "%d %d %d %d", &nnode, &ndims, &nattrb, &hasbounds );
	result = sscanf ( node, "%d %d %d %d", &nnode, &ndims, &nattrb, &hasbounds );
	node += nextLine ( node );

	pos.resize ( nnode );

	for ( i32 i = 0; i < pos.size(); ++i )
	{
		i32 index = 0;
		//i32			bound=0;
		float x, y, z;
		sscanf ( node, "%d %f %f %f", &index, &x, &y, &z );

		//	sn>>index;
		//	sn>>x;sn>>y;sn>>z;
		node += nextLine ( node );

		//for(i32 j=0;j<nattrb;++j)
		//	sn>>a;

		//if(hasbounds)
		//	sn>>bound;

		pos[index].setX ( Scalar ( x ) );
		pos[index].setY ( Scalar ( y ) );
		pos[index].setZ ( Scalar ( z ) );
	}

	SoftBody* psb = new SoftBody ( &worldInfo, nnode, &pos[0], 0 );

#if 0

	if ( face && face[0] )
	{
		i32								nface = 0;
		sf >> nface;
		sf >> hasbounds;

		for ( i32 i = 0;i < nface;++i )
		{
			i32			index = 0;
			i32			bound = 0;
			i32			ni[3];
			sf >> index;
			sf >> ni[0];
			sf >> ni[1];
			sf >> ni[2];
			sf >> bound;
			psb->appendFace ( ni[0], ni[1], ni[2] );

			if ( btetralinks )
			{
				psb->appendLink ( ni[0], ni[1], 0, true );
				psb->appendLink ( ni[1], ni[2], 0, true );
				psb->appendLink ( ni[2], ni[0], 0, true );
			}
		}
	}

#endif

	if ( ele && ele[0] )
	{
		i32 ntetra = 0;
		i32 ncorner = 0;
		i32 neattrb = 0;
		sscanf ( ele, "%d %d %d", &ntetra, &ncorner, &neattrb );
		ele += nextLine ( ele );

		//se>>ntetra;se>>ncorner;se>>neattrb;

		for ( i32 i = 0; i < ntetra; ++i )
		{
			i32 index = 0;
			i32 ni[4];

			//se>>index;
			//se>>ni[0];se>>ni[1];se>>ni[2];se>>ni[3];
			sscanf ( ele, "%d %d %d %d %d", &index, &ni[0], &ni[1], &ni[2], &ni[3] );
			ele += nextLine ( ele );
			//for(i32 j=0;j<neattrb;++j)
			//	se>>a;
			psb->appendTetra ( ni[0], ni[1], ni[2], ni[3] );

			if ( btetralinks )
			{
				psb->appendLink ( ni[0], ni[1], 0, true );
				psb->appendLink ( ni[1], ni[2], 0, true );
				psb->appendLink ( ni[2], ni[0], 0, true );
				psb->appendLink ( ni[0], ni[3], 0, true );
				psb->appendLink ( ni[1], ni[3], 0, true );
				psb->appendLink ( ni[2], ni[3], 0, true );
			}
		}
	}

	psb->initializeDmInverse();

	psb->m_tetraScratches.resize ( psb->m_tetras.size() );
	psb->m_tetraScratchesTn.resize ( psb->m_tetras.size() );
	printf ( "Nodes:  %u\r\n", psb->m_nodes.size() );
	printf ( "Links:  %u\r\n", psb->m_links.size() );
	printf ( "Faces:  %u\r\n", psb->m_faces.size() );
	printf ( "Tetras: %u\r\n", psb->m_tetras.size() );
	return ( psb );
}

SoftBody* SoftBodyHelpers::CreateFromVtkFile ( SoftBodyWorldInfo& worldInfo, tukk vtk_file )
{
	std::ifstream fs;
	fs.open ( vtk_file );
	Assert ( fs );

	typedef AlignedObjectArray<i32> Index;
	STxt line;
	AlignedObjectArray<Vec3> X;
	Vec3 position;
	AlignedObjectArray<Index> indices;
	bool reading_points = false;
	bool reading_tets = false;
	size_t n_points = 0;
	size_t n_tets = 0;
	size_t x_count = 0;
	size_t indices_count = 0;

	while ( std::getline ( fs, line ) )
	{
		std::stringstream ss ( line );

		if ( line.size() == ( size_t ) ( 0 ) )
		{
		}
		else
			if ( line.substr ( 0, 6 ) == "POINTS" )
			{
				reading_points = true;
				reading_tets = false;
				ss.ignore ( 128, ' ' );  // ignore "POINTS"
				ss >> n_points;
				X.resize ( n_points );
			}

			else
				if ( line.substr ( 0, 5 ) == "CELLS" )
				{
					reading_points = false;
					reading_tets = true;
					ss.ignore ( 128, ' ' );  // ignore "CELLS"
					ss >> n_tets;
					indices.resize ( n_tets );
				}

				else
					if ( line.substr ( 0, 10 ) == "CELL_TYPES" )
					{
						reading_points = false;
						reading_tets = false;
					}

					else
						if ( reading_points )
						{
							Scalar p;
							ss >> p;
							position.setX ( p );
							ss >> p;
							position.setY ( p );
							ss >> p;
							position.setZ ( p );
							//printf("v %f %f %f\n", position.getX(), position.getY(), position.getZ());
							X[x_count++] = position;
						}

						else
							if ( reading_tets )
							{
								i32 d;
								ss >> d;

								if ( d != 4 )
								{
									printf ( "Load deformable failed: Only Tetrahedra are supported in VTK file.\n" );
									fs.close();
									return 0;
								}

								ss.ignore ( 128, ' ' );  // ignore "4"

								Index tet;
								tet.resize ( 4 );

								for ( size_t i = 0; i < 4; i++ )
								{
									ss >> tet[i];
									//printf("%d ", tet[i]);
								}

								//printf("\n");
								indices[indices_count++] = tet;
							}
	}

	SoftBody* psb = new SoftBody ( &worldInfo, n_points, &X[0], 0 );

	for ( i32 i = 0; i < n_tets; ++i )
	{
		const Index& ni = indices[i];
		psb->appendTetra ( ni[0], ni[1], ni[2], ni[3] );
		{
			psb->appendLink ( ni[0], ni[1], 0, true );
			psb->appendLink ( ni[1], ni[2], 0, true );
			psb->appendLink ( ni[2], ni[0], 0, true );
			psb->appendLink ( ni[0], ni[3], 0, true );
			psb->appendLink ( ni[1], ni[3], 0, true );
			psb->appendLink ( ni[2], ni[3], 0, true );
		}
	}

	generateBoundaryFaces ( psb );

	psb->initializeDmInverse();
	psb->m_tetraScratches.resize ( psb->m_tetras.size() );
	psb->m_tetraScratchesTn.resize ( psb->m_tetras.size() );
	printf ( "Nodes:  %u\r\n", psb->m_nodes.size() );
	printf ( "Links:  %u\r\n", psb->m_links.size() );
	printf ( "Faces:  %u\r\n", psb->m_faces.size() );
	printf ( "Tetras: %u\r\n", psb->m_tetras.size() );

	fs.close();
	return psb;
}

void SoftBodyHelpers::generateBoundaryFaces ( SoftBody* psb )
{
	i32 counter = 0;

	for ( i32 i = 0; i < psb->m_nodes.size(); ++i )
	{
		psb->m_nodes[i].index = counter++;
	}

	typedef AlignedObjectArray<i32> Index;

	AlignedObjectArray<Index> indices;
	indices.resize ( psb->m_tetras.size() );

	for ( i32 i = 0; i < indices.size(); ++i )
	{
		Index index;
		index.push_back ( psb->m_tetras[i].m_n[0]->index );
		index.push_back ( psb->m_tetras[i].m_n[1]->index );
		index.push_back ( psb->m_tetras[i].m_n[2]->index );
		index.push_back ( psb->m_tetras[i].m_n[3]->index );
		indices[i] = index;
	}

	std::map<std::vector<i32>, std::vector<i32> > dict;

	for ( i32 i = 0; i < indices.size(); ++i )
	{
		for ( i32 j = 0; j < 4; ++j )
		{
			std::vector<i32> f;

			if ( j == 0 )
			{
				f.push_back ( indices[i][1] );
				f.push_back ( indices[i][0] );
				f.push_back ( indices[i][2] );
			}

			if ( j == 1 )
			{
				f.push_back ( indices[i][3] );
				f.push_back ( indices[i][0] );
				f.push_back ( indices[i][1] );
			}

			if ( j == 2 )
			{
				f.push_back ( indices[i][3] );
				f.push_back ( indices[i][1] );
				f.push_back ( indices[i][2] );
			}

			if ( j == 3 )
			{
				f.push_back ( indices[i][2] );
				f.push_back ( indices[i][0] );
				f.push_back ( indices[i][3] );
			}

			std::vector<i32> f_sorted = f;

			std::sort ( f_sorted.begin(), f_sorted.end() );

			if ( dict.find ( f_sorted ) != dict.end() )
			{
				dict.erase ( f_sorted );
			}

			else
			{
				dict.insert ( std::make_pair ( f_sorted, f ) );
			}
		}
	}

	for ( std::map<std::vector<i32>, std::vector<i32> >::iterator it = dict.begin(); it != dict.end(); ++it )
	{
		std::vector<i32> f = it->second;
		psb->appendFace ( f[0], f[1], f[2] );
		//printf("f %d %d %d\n", f[0] + 1, f[1] + 1, f[2] + 1);
	}
}

//Write the surface mesh to an obj file.
void SoftBodyHelpers::writeObj ( tukk filename, const SoftBody* psb )
{
	std::ofstream fs;
	fs.open ( filename );
	Assert ( fs );

	if ( psb->m_tetras.size() > 0 )
	{
		// For tetrahedron mesh, we need to re-index the surface mesh for it to be in obj file/
		std::map<i32, i32> dict;

		for ( i32 i = 0; i < psb->m_faces.size(); i++ )
		{
			for ( i32 d = 0; d < 3; d++ )
			{
				i32 index = psb->m_faces[i].m_n[d]->index;

				if ( dict.find ( index ) == dict.end() )
				{
					i32 dict_size = dict.size();
					dict[index] = dict_size;
					fs << "v";

					for ( i32 k = 0; k < 3; k++ )
					{
						fs << " " << psb->m_nodes[index].m_x[k];
					}

					fs << "\n";
				}
			}
		}

		// Write surface mesh.

		for ( i32 i = 0; i < psb->m_faces.size(); ++i )
		{
			fs << "f";

			for ( i32 n = 0; n < 3; n++ )
			{
				fs << " " << dict[psb->m_faces[i].m_n[n]->index] + 1;
			}

			fs << "\n";
		}
	}

	else
	{
		// For trimesh, directly write out all the nodes and faces.xs
		for ( i32 i = 0; i < psb->m_nodes.size(); ++i )
		{
			fs << "v";

			for ( i32 d = 0; d < 3; d++ )
			{
				fs << " " << psb->m_nodes[i].m_x[d];
			}

			fs << "\n";
		}

		for ( i32 i = 0; i < psb->m_faces.size(); ++i )
		{
			fs << "f";

			for ( i32 n = 0; n < 3; n++ )
			{
				fs << " " << psb->m_faces[i].m_n[n]->index + 1;
			}

			fs << "\n";
		}
	}

	fs.close();
}


void SoftBodyHelpers::writeState ( tukk file, const SoftBody* psb )
{
	std::ofstream fs;
	fs.open ( file );
	Assert ( fs );
	fs << std::scientific << std::setprecision ( 16 );

	// Only write out for trimesh, directly write out all the nodes and faces.xs

	for ( i32 i = 0; i < psb->m_nodes.size(); ++i )
	{
		fs << "q";

		for ( i32 d = 0; d < 3; d++ )
		{
			fs << " " << psb->m_nodes[i].m_q[d];
		}

		fs << "\n";
	}

	for ( i32 i = 0; i < psb->m_nodes.size(); ++i )
	{
		fs << "v";

		for ( i32 d = 0; d < 3; d++ )
		{
			fs << " " << psb->m_nodes[i].m_v[d];
		}

		fs << "\n";
	}

	fs.close();
}

void SoftBodyHelpers::duplicateFaces ( tukk filename, const SoftBody* psb )
{
	std::ifstream fs_read;
	fs_read.open ( filename );
	STxt line;
	Vec3 pos;
	AlignedObjectArray<AlignedObjectArray<i32> > additional_faces;

	while ( std::getline ( fs_read, line ) )
	{
		std::stringstream ss ( line );

		if ( line[0] == 'v' )
		{
		}
		else
			if ( line[0] == 'f' )
			{
				ss.ignore();
				i32 id0, id1, id2;
				ss >> id0;
				ss >> id1;
				ss >> id2;
				AlignedObjectArray<i32> new_face;
				new_face.push_back ( id1 );
				new_face.push_back ( id0 );
				new_face.push_back ( id2 );
				additional_faces.push_back ( new_face );
			}
	}

	fs_read.close();

	std::ofstream fs_write;
	fs_write.open ( filename, std::ios_base::app );

	for ( i32 i = 0; i < additional_faces.size(); ++i )
	{
		fs_write << "f";

		for ( i32 n = 0; n < 3; n++ )
		{
			fs_write << " " << additional_faces[i][n];
		}

		fs_write << "\n";
	}

	fs_write.close();
}

// Given a simplex with vertices a,b,c,d, find the barycentric weights of p in this simplex
void SoftBodyHelpers::getBarycentricWeights ( const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& p, Vec4& bary )
{
	Vec3 vap = p - a;
	Vec3 vbp = p - b;

	Vec3 vab = b - a;
	Vec3 vac = c - a;
	Vec3 vad = d - a;

	Vec3 vbc = c - b;
	Vec3 vbd = d - b;
	Scalar va6 = ( vbp.cross ( vbd ) ).dot ( vbc );
	Scalar vb6 = ( vap.cross ( vac ) ).dot ( vad );
	Scalar vc6 = ( vap.cross ( vad ) ).dot ( vab );
	Scalar vd6 = ( vap.cross ( vab ) ).dot ( vac );
	Scalar v6 = Scalar ( 1 ) / ( vab.cross ( vac ).dot ( vad ) );
	bary = Vec4 ( va6 * v6, vb6 * v6, vc6 * v6, vd6 * v6 );
}

// Given a simplex with vertices a,b,c, find the barycentric weights of p in this simplex. bary[3] = 0.
void SoftBodyHelpers::getBarycentricWeights ( const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& p, Vec4& bary )
{
	Vec3 v0 = b - a, v1 = c - a, v2 = p - a;
	Scalar d00 = Dot ( v0, v0 );
	Scalar d01 = Dot ( v0, v1 );
	Scalar d11 = Dot ( v1, v1 );
	Scalar d20 = Dot ( v2, v0 );
	Scalar d21 = Dot ( v2, v1 );
	Scalar invDenom = 1.0 / ( d00 * d11 - d01 * d01 );
	bary[1] = ( d11 * d20 - d01 * d21 ) * invDenom;
	bary[2] = ( d00 * d21 - d01 * d20 ) * invDenom;
	bary[0] = 1.0 - bary[1] - bary[2];
	bary[3] = 0;
}

// Iterate through all render nodes to find the simulation tetrahedron that contains the render node and record the barycentric weights
// If the node is not inside any tetrahedron, assign it to the tetrahedron in which the node has the least negative barycentric weight
void SoftBodyHelpers::interpolateBarycentricWeights ( SoftBody* psb )
{
	psb->m_z.resize ( 0 );
	psb->m_renderNodesInterpolationWeights.resize ( psb->m_renderNodes.size() );
	psb->m_renderNodesParents.resize ( psb->m_renderNodes.size() );

	for ( i32 i = 0; i < psb->m_renderNodes.size(); ++i )
	{
		const Vec3& p = psb->m_renderNodes[i].m_x;
		Vec4 bary;
		Vec4 optimal_bary;
		Scalar min_bary_weight = -1e3;
		AlignedObjectArray<const SoftBody::Node*> optimal_parents;

		for ( i32 j = 0; j < psb->m_tetras.size(); ++j )
		{
			const SoftBody::Tetra& t = psb->m_tetras[j];
			getBarycentricWeights ( t.m_n[0]->m_x, t.m_n[1]->m_x, t.m_n[2]->m_x, t.m_n[3]->m_x, p, bary );
			Scalar new_min_bary_weight = bary[0];

			for ( i32 k = 1; k < 4; ++k )
			{
				new_min_bary_weight = d3Min ( new_min_bary_weight, bary[k] );
			}

			if ( new_min_bary_weight > min_bary_weight )
			{
				AlignedObjectArray<const SoftBody::Node*> parents;
				parents.push_back ( t.m_n[0] );
				parents.push_back ( t.m_n[1] );
				parents.push_back ( t.m_n[2] );
				parents.push_back ( t.m_n[3] );
				optimal_parents = parents;
				optimal_bary = bary;
				min_bary_weight = new_min_bary_weight;
				// stop searching if p is inside the tetrahedron at hand

				if ( bary[0] >= 0. && bary[1] >= 0. && bary[2] >= 0. && bary[3] >= 0. )
				{
					break;
				}
			}
		}

		psb->m_renderNodesInterpolationWeights[i] = optimal_bary;

		psb->m_renderNodesParents[i] = optimal_parents;
	}
}

// Iterate through all render nodes to find the simulation triangle that's closest to the node in the barycentric sense.
void SoftBodyHelpers::extrapolateBarycentricWeights ( SoftBody* psb )
{
	psb->m_renderNodesInterpolationWeights.resize ( psb->m_renderNodes.size() );
	psb->m_renderNodesParents.resize ( psb->m_renderNodes.size() );
	psb->m_z.resize ( psb->m_renderNodes.size() );

	for ( i32 i = 0; i < psb->m_renderNodes.size(); ++i )
	{
		const Vec3& p = psb->m_renderNodes[i].m_x;
		Vec4 bary;
		Vec4 optimal_bary;
		Scalar min_bary_weight = -SIMD_INFINITY;
		AlignedObjectArray<const SoftBody::Node*> optimal_parents;
		Scalar dist = 0, optimal_dist = 0;

		for ( i32 j = 0; j < psb->m_faces.size(); ++j )
		{
			const SoftBody::Face& f = psb->m_faces[j];
			Vec3 n = Cross ( f.m_n[1]->m_x - f.m_n[0]->m_x, f.m_n[2]->m_x - f.m_n[0]->m_x );
			Vec3 unit_n = n.normalized();
			dist = ( p - f.m_n[0]->m_x ).dot ( unit_n );
			Vec3 proj_p = p - dist * unit_n;
			getBarycentricWeights ( f.m_n[0]->m_x, f.m_n[1]->m_x, f.m_n[2]->m_x, proj_p, bary );
			Scalar new_min_bary_weight = bary[0];

			for ( i32 k = 1; k < 3; ++k )
			{
				new_min_bary_weight = d3Min ( new_min_bary_weight, bary[k] );
			}

			// p is out of the current best triangle, we found a traingle that's better
			bool better_than_closest_outisde = ( new_min_bary_weight > min_bary_weight && min_bary_weight < 0. );

			// p is inside of the current best triangle, we found a triangle that's better
			bool better_than_best_inside = ( new_min_bary_weight >= 0 && min_bary_weight >= 0 && Fabs ( dist ) < Fabs ( optimal_dist ) );

			if ( better_than_closest_outisde || better_than_best_inside )
			{
				AlignedObjectArray<const SoftBody::Node*> parents;
				parents.push_back ( f.m_n[0] );
				parents.push_back ( f.m_n[1] );
				parents.push_back ( f.m_n[2] );
				optimal_parents = parents;
				optimal_bary = bary;
				optimal_dist = dist;
				min_bary_weight = new_min_bary_weight;
			}
		}

		psb->m_renderNodesInterpolationWeights[i] = optimal_bary;

		psb->m_renderNodesParents[i] = optimal_parents;
		psb->m_z[i] = optimal_dist;
	}
}
