// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include <drx3D/CoreX/Math/Drx_Vector3.h>
#include <drx3D/Eng3D/CGFContent.h>

namespace VClothPreProcessUtils {
struct STriInfo;
}

struct AttachmentVClothPreProcessNndc
{
	// Nearest Neighbor Distance Constraints
	i32   nndcIdx;        // index of closest constraint (i.e., closest attached vertex)
	float nndcDist;       // distance to closest constraint
	i32   nndcNextParent; // index of next parent on path to closest constraint
	AttachmentVClothPreProcessNndc() : nndcIdx(-1), nndcDist(0), nndcNextParent(-1) {}
};

struct SBendTriangle // one triangle which is used for bending forces by neighboring triangle angle
{
	i32  p0, p1, p2; // indices of according triangle
	Vec3 normal;
	SBendTriangle(i32 _p0, i32 _p1, i32 _p2) : p0(_p0), p1(_p1), p2(_p2), normal(0) {}
	SBendTriangle() : p0(-1), p1(-1), p2(-1), normal(0) {}
	//STxt toString() { std::stringstream ss; ss << "Bend Triangle: " << p0 << ":" << p1 << ":" << p2; return ss.str(); }
};

struct SBendTrianglePair // a pair of triangles which share one edge and is used for bending forces by neighboring triangle angle
{
	i32   p0, p1;     // shared edge
	i32   p2;         // first triangle // oriented 0,1,2
	i32   p3;         // second triangle // reverse oriented 1,0,3
	i32   idxNormal0; // idx of BendTriangle for first triangle
	i32   idxNormal1; // idx of BendTriangle for second triangle
	float phi0;       // initial angle
	SBendTrianglePair(i32 _e0, i32 _e1, i32 _p2, i32 _p3, i32 _idxNormal0, i32 _idxNormal1) :
		p0(_e0), p1(_e1), p2(_p2), p3(_p3), idxNormal0(_idxNormal0), idxNormal1(_idxNormal1), phi0(0) {}
	SBendTrianglePair() :
		p0(-1), p1(-1), p2(-1), p3(-1), idxNormal0(-1), idxNormal1(-1), phi0(0) {}
	//STxt toString() { std::stringstream ss; ss << "BendTrianglePair: Edge: " << p0 << ":" << p1 << " VtxTriangles: " << p2 << ":" << p3; return ss.str(); }
};

struct SLink
{
	i32   i1, i2;
	float lenSqr, weight1, weight2;
	bool  skip;

	SLink() : i1(0), i2(0), lenSqr(0.0f), weight1(0.f), weight2(0.f), skip(false) {}
};

struct SAttachmentVClothPreProcessData
{
	// Nearest Neighbor Distance Constraints
	std::vector<AttachmentVClothPreProcessNndc> m_nndc;
	std::vector<i32>                            m_nndcNotAttachedOrderedIdx;

	// Bending by triangle angles, not springs
	std::vector<SBendTrianglePair> m_listBendTrianglePairs; // triangle pairs sharing an edge
	std::vector<SBendTriangle>     m_listBendTriangles;     // triangles which are used for bending

	// Links
	std::vector<SLink> m_links[eVClothLink_COUNT];
};

struct STopology
{
	i32 iStartEdge, iEndEdge, iSorted;
	i32 bFullFan;

	STopology() : iStartEdge(0), iEndEdge(0), iSorted(0), bFullFan(0) {}
};

struct AttachmentVClothPreProcess
{
	bool                                              PreProcess(std::vector<Vec3> const& vtx, std::vector<i32> const& idx, std::vector<bool> const& attached);

	std::vector<AttachmentVClothPreProcessNndc> const& GetNndc() const                     { return m_nndc; }
	std::vector<i32> const&                           GetNndcNotAttachedOrderedIdx() const { return m_nndcNotAttachedOrderedIdx; }

	std::vector<SBendTrianglePair> const&             GetListBendTrianglePair() const      { return m_listBendTrianglePairs; }
	std::vector<SBendTriangle> const&                 GetListBendTriangle() const          { return m_listBendTriangles; }

	std::vector<SLink> const&                         GetLinksStretch() const              { return m_linksStretch; }
	std::vector<SLink> const&                         GetLinksShear() const                { return m_linksShear; }
	std::vector<SLink> const&                         GetLinksBend() const                 { return m_linksBend; }
	std::vector<SLink> const&                         GetLinks(i32 idx) const
	{
		switch (idx)
		{
		default:
		case 0:
			return GetLinksStretch();
		case 1:
			return GetLinksShear();
		case 2:
			return GetLinksBend();
		}
	}

	// helpers which are used in the RC as well as in the engine
	static bool IsAttached(float weight);
	static bool PruneWeldedVertices(std::vector<Vec3>& vtx, std::vector<i32>& idx, std::vector<bool>& attached);
	static bool RemoveDegeneratedTriangles(std::vector<Vec3>& vtx, std::vector<i32>& idx, std::vector<bool>& attached);
	static bool DetermineTriangleNormals(std::vector<Vec3> const& vtx, std::vector<SBendTriangle>& tri);
	static bool DetermineTriangleNormals(i32 nVertices, strided_pointer<Vec3> const& pVertices, std::vector<SBendTriangle>& tri);

private:

	bool NndcDijkstra(std::vector<Vec3> const& vtx, std::vector<i32> const& idx, std::vector<bool> const& attached);
	bool BendByTriangleAngle(std::vector<Vec3> const& vtx, std::vector<i32> const& idx, std::vector<bool> const& attached);
	// links
	bool CreateLinks(std::vector<Vec3> const& vtx, std::vector<i32> const& idx);
	bool CalculateTopology(std::vector<Vec3> const& vtx, std::vector<i32> const& idx, std::vector<VClothPreProcessUtils::STriInfo>& pTopology); // determine neighboring triangles

	// Nearest Neighbor Distance Constraints
	std::vector<AttachmentVClothPreProcessNndc> m_nndc;
	std::vector<i32>                            m_nndcNotAttachedOrderedIdx;

	// Bending by triangle angles, not springs
	std::vector<SBendTrianglePair> m_listBendTrianglePairs; // triangle pairs sharing an edge
	std::vector<SBendTriangle>     m_listBendTriangles;     // triangles which are used for bending

	// Links
	std::vector<SLink> m_linksStretch;
	std::vector<SLink> m_linksShear;
	std::vector<SLink> m_linksBend;

};

inline bool AttachmentVClothPreProcess::IsAttached(float weight)
{
	const float attachedThresh = 0.99f; // weights with a value higher than this are handled as weight=1.0f -> attached
	return weight > attachedThresh;
}
