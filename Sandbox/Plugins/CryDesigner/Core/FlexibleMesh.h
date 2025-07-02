// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Designer
{
class FlexibleMesh : public _i_reference_target_t
{
public:

	FlexibleMesh(){}
	FlexibleMesh(const FlexibleMesh& mesh)
	{
		vertexList = mesh.vertexList;
		normalList = mesh.normalList;
		faceList = mesh.faceList;
		matId2SubsetMap = mesh.matId2SubsetMap;
	}

	void Clear();
	i32  FindMatIdFromSubsetNum(i32 nSubsetNum) const;
	i32  AddMatID(i32 nMatID);
	void Reserve(i32 nSize);
	bool IsValid() const;
	bool IsPassed(const BrushRay& ray, BrushFloat& outT) const;
	bool IsPassedUV(const Ray& ray) const;
	bool IsOverlappedUV(const AABB& aabb) const;
	void FillIndexedMesh(IIndexedMesh* pMesh) const;
	void Join(const FlexibleMesh& mesh);
	void Invert();

	std::vector<Vertex>    vertexList;
	std::vector<BrushVec3> normalList;
	std::vector<SMeshFace> faceList;
	std::map<i32, i32>     matId2SubsetMap;
};
typedef _smart_ptr<FlexibleMesh> FlexibleMeshPtr;
}

