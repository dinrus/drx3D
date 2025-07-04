// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/DrxEndian.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/Renderer/VertexFormats.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>

//! \cond INTERNAL
//! 2D Texture coordinates used by CMesh.
struct SMeshTexCoord
{
	SMeshTexCoord() {}

private:
	float s, t;

public:
	explicit SMeshTexCoord(float x, float y)
	{
		s = x;
		t = y;
	}

	explicit SMeshTexCoord(const Vec2f16& other)
	{
		const Vec2 uv = other.ToVec2();

		s = uv.x;
		t = uv.y;
	}

	explicit SMeshTexCoord(const Vec2& other)
	{
		s = other.x;
		t = other.y;
	}

	explicit SMeshTexCoord(const Vec4& other)
	{
		s = other.x;
		t = other.y;
	}

	void ExportTo(Vec2f16& other) const
	{
		other = Vec2f16(s, t);
	}

	void ExportTo(float& others, float& othert) const
	{
		others = s;
		othert = t;
	}

	bool operator==(const SMeshTexCoord& other) const
	{
		return (s == other.s) && (t == other.t);
	}

	bool operator!=(const SMeshTexCoord& other) const
	{
		return !(*this == other);
	}

	bool operator<(const SMeshTexCoord& other) const
	{
		return (s != other.s) ? (s < other.s) : (t < other.t);
	}

	bool IsEquivalent(const Vec2& other, float epsilon = 0.003f) const
	{
		return
		  (fabs_tpl(s - other.x) <= epsilon) &&
		  (fabs_tpl(t - other.y) <= epsilon);
	}

	bool IsEquivalent(const SMeshTexCoord& other, float epsilon = 0.00005f) const
	{
		return
		  (fabs_tpl(s - other.s) <= epsilon) &&
		  (fabs_tpl(t - other.t) <= epsilon);
	}

	ILINE Vec2 GetUV() const
	{
		return Vec2(s, t);
	}

	void GetUV(Vec2& otheruv) const
	{
		otheruv = GetUV();
	}

	void GetUV(Vec4& otheruv) const
	{
		otheruv = Vec4(s, t, 0.0f, 1.0f);
	}

	void Lerp(const SMeshTexCoord& other, float pos)
	{
		Vec2 texA;
		Vec2 texB;
		this->GetUV();
		other.GetUV();

		texA.SetLerp(texA, texB, pos);

		*this = SMeshTexCoord(texA);
	}

	AUTO_STRUCT_INFO;
};

//! RGBA Color description structure used by CMesh.
struct SMeshColor
{
	SMeshColor() {}

private:
	u8 r, g, b, a;

public:
	explicit SMeshColor(u8 otherr, u8 otherg, u8 otherb, u8 othera)
	{
		r = otherr;
		g = otherg;
		b = otherb;
		a = othera;
	}

	explicit SMeshColor(const Vec4& otherc)
	{
		r = FtoI(otherc.x);
		g = FtoI(otherc.y);
		b = FtoI(otherc.z);
		a = FtoI(otherc.w);
	}

	void TransferRGBTo(SMeshColor& other) const
	{
		other.r = r;
		other.g = g;
		other.b = b;
	}

	void TransferATo(SMeshColor& other) const
	{
		other.a = a;
	}

	void MaskA(u8 maska)
	{
		a &= maska;
	}

	bool operator==(const SMeshColor& other) const
	{
		return (r == other.r) && (g == other.g) && (b == other.b) && (a == other.a);
	}

	bool operator!=(const SMeshColor& other) const
	{
		return !(*this == other);
	}

	bool operator<(const SMeshColor& other) const
	{
		return (r != other.r) ? (r < other.r) : (g != other.g) ? (g < other.g) : (b != other.b) ? (b < other.b) : (a < other.a);
	}

	ILINE ColorB GetRGBA() const
	{
		return ColorB(r, g, b, a);
	}

	void GetRGBA(ColorB& otherc) const
	{
		otherc = GetRGBA();
	}

	void GetRGBA(Vec4& otherc) const
	{
		otherc = Vec4(r, g, b, a);
	}

	void Lerp(const SMeshColor& other, float pos)
	{
		Vec4 clrA;
		Vec4 clrB;
		this->GetRGBA(clrA);
		other.GetRGBA(clrB);

		clrA.SetLerp(clrA, clrB, pos);

		*this = SMeshColor(clrA);
	}

	AUTO_STRUCT_INFO;
};

//! Defines a single triangle face in the CMesh topology.
struct SMeshFace
{
	i32           v[3];    //!< Indices to vertex, normals and optionally tangent basis arrays.
	u8 nSubset; //!< Index to mesh subsets array.

	AUTO_STRUCT_INFO;
};

//! 3D Normal Vector used by CMesh.
struct SMeshNormal
{
	SMeshNormal() {}

private:
	Vec3 Normal;

public:
	explicit SMeshNormal(const Vec3& othern)
	{
		Normal = othern;
	}

	bool operator==(const SMeshNormal& othern) const
	{
		return (Normal.x == othern.Normal.x) && (Normal.y == othern.Normal.y) && (Normal.z == othern.Normal.z);
	}

	bool operator!=(const SMeshNormal& othern) const
	{
		return !(*this == othern);
	}

	bool operator<(const SMeshNormal& othern) const
	{
		return (Normal.x != othern.Normal.x) ? (Normal.x < othern.Normal.x) : (Normal.y != othern.Normal.y) ? (Normal.y < othern.Normal.y) : (Normal.z < othern.Normal.z);
	}

	bool IsEquivalent(const Vec3& othern, float epsilon = 0.00005f) const
	{
		return
		  Normal.IsEquivalent(othern, epsilon);
	}

	bool IsEquivalent(const SMeshNormal& othern, float epsilon = 0.00005f) const
	{
		return
		  IsEquivalent(othern.Normal, epsilon);
	}

	ILINE Vec3 GetN() const
	{
		return Normal;
	}

	void GetN(Vec3& othern) const
	{
		othern = GetN();
	}

	void RotateBy(const Matrix33& rot)
	{
		Normal = rot * Normal;
	}

	void RotateSafelyBy(const Matrix33& rot)
	{
		Normal = rot * Normal;
		// normalize in case "rot" wasn't length-preserving
		Normal.Normalize();
	}

	void RotateBy(const Matrix34& trn)
	{
		Normal = trn.TransformVector(Normal);
	}

	void RotateSafelyBy(const Matrix34& trn)
	{
		Normal = trn.TransformVector(Normal);
		// normalize in case "trn" wasn't length-preserving
		Normal.Normalize();
	}

	void Slerp(const SMeshNormal& other, float pos)
	{
		Vec3 nrmA = this->GetN();
		Vec3 nrmB = other.GetN();

		nrmA.Normalize();
		nrmB.Normalize();

		nrmA.SetSlerp(nrmA, nrmB, pos);

		*this = SMeshNormal(nrmA);
	}

	AUTO_STRUCT_INFO;
};

//! Mesh tangents (tangent space normals).
struct SMeshTangents
{
	SMeshTangents() {}

private:
	Vec4sf Tangent;
	Vec4sf Bitangent;

public:
	explicit SMeshTangents(const Vec4sf& othert, const Vec4sf& otherb)
	{
		Tangent = othert;
		Bitangent = otherb;
	}

	explicit SMeshTangents(const SPipTangents& other)
	{
		Tangent = other.Tangent;
		Bitangent = other.Bitangent;
	}

	explicit SMeshTangents(const Vec4& othert, const Vec4& otherb)
	{
		Tangent = PackingSNorm::tPackF2Bv(othert);
		Bitangent = PackingSNorm::tPackF2Bv(otherb);
	}

	explicit SMeshTangents(const Vec3& othert, const Vec3& otherb, const Vec3& othern)
	{
		// TODO: can be optimized to use only integer arithmetic
		i16 othersign = 1;
		if (othert.Cross(otherb).Dot(othern) < 0)
			othersign = -1;

		Tangent = Vec4sf(PackingSNorm::tPackF2B(othert.x), PackingSNorm::tPackF2B(othert.y), PackingSNorm::tPackF2B(othert.z), PackingSNorm::tPackS2B(othersign));
		Bitangent = Vec4sf(PackingSNorm::tPackF2B(otherb.x), PackingSNorm::tPackF2B(otherb.y), PackingSNorm::tPackF2B(otherb.z), PackingSNorm::tPackS2B(othersign));
	}

	explicit SMeshTangents(const Vec3& othert, const Vec3& otherb, i16k& othersign)
	{
		Tangent = Vec4sf(PackingSNorm::tPackF2B(othert.x), PackingSNorm::tPackF2B(othert.y), PackingSNorm::tPackF2B(othert.z), PackingSNorm::tPackS2B(othersign));
		Bitangent = Vec4sf(PackingSNorm::tPackF2B(otherb.x), PackingSNorm::tPackF2B(otherb.y), PackingSNorm::tPackF2B(otherb.z), PackingSNorm::tPackS2B(othersign));
	}

	void ExportTo(Vec4sf& othert, Vec4sf& otherb) const
	{
		othert = Tangent;
		otherb = Bitangent;
	}

	void ExportTo(SPipTangents& other) const
	{
		other.Tangent = Tangent;
		other.Bitangent = Bitangent;
	}

	bool operator==(const SMeshTangents& other) const
	{
		return
		  Tangent[0] == other.Tangent[0] ||
		  Tangent[1] == other.Tangent[1] ||
		  Tangent[2] == other.Tangent[2] ||
		  Tangent[3] == other.Tangent[3] ||
		  Bitangent[0] == other.Bitangent[0] ||
		  Bitangent[1] == other.Bitangent[1] ||
		  Bitangent[2] == other.Bitangent[2] ||
		  Bitangent[3] == other.Bitangent[3];
	}

	bool operator!=(const SMeshTangents& other) const
	{
		return !(*this == other);
	}

	bool IsEquivalent(const Vec3& othert, const Vec3& otherb, i16k& othersign, float epsilon = 0.01f) const
	{
		// TODO: can be optimized to use only integer arithmetic
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		return
		  (tng.w == othersign) &&
		  (btg.w == othersign) &&
		  (tng3.Dot(othert) >= (1.0f - epsilon)) &&
		  (btg3.Dot(otherb) >= (1.0f - epsilon));
	}

	void GetTB(Vec4sf& othert, Vec4sf& otherb) const
	{
		othert = Tangent;
		otherb = Bitangent;
	}

	void GetTB(Vec4& othert, Vec4& otherb) const
	{
		othert = PackingSNorm::tPackB2F(Tangent);
		otherb = PackingSNorm::tPackB2F(Bitangent);
	}

	void GetTB(Vec3& othert, Vec3& otherb) const
	{
		const Vec4 t = PackingSNorm::tPackB2F(Tangent);
		const Vec4 b = PackingSNorm::tPackB2F(Bitangent);

		othert = Vec3(t.x, t.y, t.z);
		otherb = Vec3(b.x, b.y, b.z);
	}

	ILINE Vec3 GetN() const
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		// assumes w 1 or -1
		return tng3.Cross(btg3) * tng.w;
	}

	void GetN(Vec3& othern) const
	{
		othern = GetN();
	}

	void GetTBN(Vec3& othert, Vec3& otherb, Vec3& othern) const
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		// assumes w 1 or -1
		othert = tng3;
		otherb = btg3;
		othern = tng3.Cross(btg3) * tng.w;
	}

	ILINE i16 GetR() const
	{
		return PackingSNorm::tPackB2S(Tangent.w);
	}

	void GetR(i16& sign) const
	{
		sign = GetR();
	}

	void RotateBy(const Matrix33& rot)
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		tng3 = rot * tng3;
		btg3 = rot * btg3;

		*this = SMeshTangents(tng3, btg3, PackingSNorm::tPackB2S(Tangent.w));
	}

	void RotateSafelyBy(const Matrix33& rot)
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		tng3 = rot * tng3;
		btg3 = rot * btg3;

		// normalize in case "rot" wasn't length-preserving
		tng3.Normalize();
		btg3.Normalize();

		*this = SMeshTangents(tng3, btg3, PackingSNorm::tPackB2S(Tangent.w));
	}

	void RotateBy(const Matrix34& trn)
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		tng3 = trn.TransformVector(tng3);
		btg3 = trn.TransformVector(btg3);

		*this = SMeshTangents(tng3, btg3, PackingSNorm::tPackB2S(Tangent.w));
	}

	void RotateSafelyBy(const Matrix34& trn)
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z);
		Vec3 btg3(btg.x, btg.y, btg.z);

		tng3 = trn.TransformVector(tng3);
		btg3 = trn.TransformVector(btg3);

		// normalize in case "rot" wasn't length-preserving
		tng3.Normalize();
		btg3.Normalize();

		*this = SMeshTangents(tng3, btg3, PackingSNorm::tPackB2S(Tangent.w));
	}

	void SlerpTowards(const SMeshTangents& other, const SMeshNormal& normal, float pos)
	{
		Vec3 tngA, btgA;
		Vec3 tngB, btgB;
		this->GetTB(tngA, btgA);
		other.GetTB(tngB, btgB);

		// Q: necessary?
		tngA.Normalize();
		tngB.Normalize();
		btgA.Normalize();
		btgB.Normalize();

		tngA.SetSlerp(tngA, tngB, pos);
		btgA.SetSlerp(btgA, btgB, pos);

		*this = SMeshTangents(tngA, btgA, normal.GetN());
	}

	AUTO_STRUCT_INFO;
};

struct SMeshQTangents
{
	SMeshQTangents() {}

private:
	Vec4sf TangentBitangent;

public:
	explicit SMeshQTangents(const SPipQTangents& other)
	{
		TangentBitangent = other.QTangent;
	}

	explicit SMeshQTangents(const Quat& other)
	{
		TangentBitangent.x = PackingSNorm::tPackF2B(other.v.x);
		TangentBitangent.y = PackingSNorm::tPackF2B(other.v.y);
		TangentBitangent.z = PackingSNorm::tPackF2B(other.v.z);
		TangentBitangent.w = PackingSNorm::tPackF2B(other.w);
	}

	void ExportTo(SPipQTangents& other)
	{
		other.QTangent = TangentBitangent;
	}

	ILINE Quat GetQ() const
	{
		Quat q;

		q.v.x = PackingSNorm::tPackB2F(TangentBitangent.x);
		q.v.y = PackingSNorm::tPackB2F(TangentBitangent.y);
		q.v.z = PackingSNorm::tPackB2F(TangentBitangent.z);
		q.w = PackingSNorm::tPackB2F(TangentBitangent.w);

		return q;
	}

	AUTO_STRUCT_INFO;
};

//! For skinning, every vertex has 4 bones and 4 weights.
struct SMeshBoneMapping_u16
{
	typedef u16 BoneId;
	typedef u8  Weight;

	BoneId boneIds[4];
	Weight weights[4];

	AUTO_STRUCT_INFO;
};

struct SMeshBoneMapping_uint8
{
	typedef u8 BoneId;
	typedef u8 Weight;

	BoneId boneIds[4];
	Weight weights[4];

	AUTO_STRUCT_INFO;
};

//! Subset of mesh is a continuous range of vertices and indices that share same material.
struct SMeshSubset
{
	Vec3  vCenter;
	float fRadius;
	float fTexelDensity;

	i32   nFirstIndexId;
	i32   nNumIndices;

	i32   nFirstVertId;
	i32   nNumVerts;

	i32   nMatID;            //!< Material Sub-object id.
	i32   nMatFlags;         //!< Special Material flags.
	i32   nPhysicalizeType;  //!< Type of physicalization for this subset.

	SMeshSubset()
		: vCenter(0, 0, 0)
		, fRadius(0)
		, fTexelDensity(0)
		, nFirstIndexId(0)
		, nNumIndices(0)
		, nFirstVertId(0)
		, nNumVerts(0)
		, nMatID(0)
		, nMatFlags(0)
		, nPhysicalizeType(PHYS_GEOM_TYPE_DEFAULT)
	{
	}

	void GetMemoryUsage(class IDrxSizer* pSizer) const
	{
	}

	//! Fix numVerts.
	void FixRanges(vtx_idx* pIndices)
	{
		i32 startVertexToMerge = nFirstVertId;
		i32 startIndexToMerge = nFirstIndexId;
		i32 numIndiciesToMerge = nNumIndices;
		// find good min and max AGAIN
		i32 maxVertexInUse = 0;
		for (i32 n = 0; n < numIndiciesToMerge; n++)
		{
			i32 i = (i32)pIndices[n + startIndexToMerge];
			startVertexToMerge = (i < startVertexToMerge ? i : startVertexToMerge);// min
			maxVertexInUse = (i > maxVertexInUse ? i : maxVertexInUse);// max
		}
		nNumVerts = maxVertexInUse - startVertexToMerge + 1;
	}
};

class CMeshHelpers
{
public:
	template<class TPosition, class TTexCoordinates, class TIndex>
	static bool ComputeTexMappingAreas(
	  size_t indexCount, const TIndex* pIndices,
	  size_t vertexCount,
	  const TPosition* pPositions, size_t stridePositions,
	  const TTexCoordinates* pTexCoords, size_t strideTexCoords,
	  float& computedPosArea, float& computedTexArea, tukk & errorText)
	{
		static const float minPosArea = 10e-6f;
		static const float minTexArea = 10e-8f;

		computedPosArea = 0;
		computedTexArea = 0;
		errorText = "?";

		if (indexCount <= 0)
		{
			errorText = "index count is 0";
			return false;
		}

		if (vertexCount <= 0)
		{
			errorText = "vertex count is 0";
			return false;
		}

		if ((pIndices == NULL) || (pPositions == NULL))
		{
			errorText = "indices and/or positions are NULL";
			return false;
		}

		if (pTexCoords == NULL)
		{
			errorText = "texture coordinates are NULL";
			return false;
		}

		if (indexCount % 3 != 0)
		{
			assert(0);
			errorText = "bad number of indices";
			return false;
		}

		// Compute average geometry area of face

		i32 count = 0;
		float posAreaSum = 0;
		float texAreaSum = 0;
		for (size_t i = 0; i < indexCount; i += 3)
		{
			const size_t index0 = pIndices[i];
			const size_t index1 = pIndices[i + 1];
			const size_t index2 = pIndices[i + 2];

			if ((index0 >= vertexCount) || (index1 >= vertexCount) || (index2 >= vertexCount))
			{
				errorText = "bad vertex index detected";
				return false;
			}

			const Vec3 pos0 = ToVec3(*(const TPosition*) (((tukk)pPositions) + index0 * stridePositions));
			const Vec3 pos1 = ToVec3(*(const TPosition*) (((tukk)pPositions) + index1 * stridePositions));
			const Vec3 pos2 = ToVec3(*(const TPosition*) (((tukk)pPositions) + index2 * stridePositions));

			const Vec2 tex0 = ToVec2(*(const TTexCoordinates*) (((tukk)pTexCoords) + index0 * strideTexCoords));
			const Vec2 tex1 = ToVec2(*(const TTexCoordinates*) (((tukk)pTexCoords) + index1 * strideTexCoords));
			const Vec2 tex2 = ToVec2(*(const TTexCoordinates*) (((tukk)pTexCoords) + index2 * strideTexCoords));

			const float posArea = ((pos1 - pos0).Cross(pos2 - pos0)).GetLength() * 0.5f;
			const float texArea = fabsf((tex1 - tex0).Cross(tex2 - tex0)) * 0.5f;

			if ((posArea >= minPosArea) && (texArea >= minTexArea))
			{
				posAreaSum += posArea;
				texAreaSum += texArea;
				++count;
			}
		}

		if (count == 0 || (posAreaSum < minPosArea) || (texAreaSum < minTexArea))
		{
			errorText = "faces are too small or have stretched mapping";
			return false;
		}

		computedPosArea = posAreaSum;
		computedTexArea = texAreaSum;
		return true;
	}

	template<class TPosition>
	static bool CollectFaceAreas(size_t indexCount, const vtx_idx* pIndices, size_t vertexCount,
	                             const TPosition* pPositions, size_t stridePositions, std::vector<float>& areas)
	{
		static const float minFaceArea = 10e-6f;

		if (indexCount % 3 != 0)
		{
			return false;
		}

		areas.reserve(areas.size() + indexCount / 3);

		for (size_t i = 0; i < indexCount; i += 3)
		{
			const uint index0 = pIndices[i];
			const uint index1 = pIndices[i + 1];
			const uint index2 = pIndices[i + 2];

			if ((index0 >= vertexCount) || (index1 >= vertexCount) || (index2 >= vertexCount))
			{
				return false;
			}

			const Vec3 pos0 = ToVec3(*(const TPosition*) (((tukk)pPositions) + index0 * stridePositions));
			const Vec3 pos1 = ToVec3(*(const TPosition*) (((tukk)pPositions) + index1 * stridePositions));
			const Vec3 pos2 = ToVec3(*(const TPosition*) (((tukk)pPositions) + index2 * stridePositions));

			const float faceArea = ((pos1 - pos0).Cross(pos2 - pos0)).GetLength() * 0.5f;

			if (faceArea >= minFaceArea)
			{
				areas.push_back(faceArea);
			}
		}

		return true;
	}

private:
	template<class T>
	inline static Vec3 ToVec3(const T& v)
	{
		return v.ToVec3();
	}

	template<class T>
	inline static Vec2 ToVec2(const T& v)
	{
		Vec2 uv;
		v.GetUV(uv);

		return uv;
	}
};

template<>
inline Vec3 CMeshHelpers::ToVec3<Vec3>(const Vec3& v)
{
	return v;
}

template<>
inline Vec2 CMeshHelpers::ToVec2<Vec2>(const Vec2& v)
{
	return v;
}

template<>
inline Vec2 CMeshHelpers::ToVec2<Vec2f16>(const Vec2f16& v)
{
	return v.ToVec2();
}

template<>
inline Vec2 CMeshHelpers::ToVec2<SMeshTexCoord>(const SMeshTexCoord& v)
{
	return v.GetUV();
}

//////////////////////////////////////////////////////////////////////////
// Описание:
//    General purpose mesh class.
//////////////////////////////////////////////////////////////////////////
class CMesh
{
public:
	enum EStream
	{
		POSITIONS = 0,
		POSITIONSF16,
		NORMALS,
		FACES,
		TOPOLOGY_IDS,
		TEXCOORDS,
		COLORS_0,
		COLORS_1,
		INDICES,
		TANGENTS,
		BONEMAPPING,
		VERT_MATS,
		QTANGENTS,
		P3S_C4B_T2S,

		EXTRABONEMAPPING,     //!< Extra stream. Does not have a stream ID in the CGF. Its data is saved at the end of the BONEMAPPING stream.

		LAST_STREAM,
	};

	SMeshFace* m_pFaces;      //!< Faces are used in mesh processing/compilation.
	i32*     m_pTopologyIds;

	// Geometry data.
	vtx_idx*                 m_pIndices; //!< Indices are used for the final render-mesh.
	Vec3*                    m_pPositions;
	Vec3f16*                 m_pPositionsF16;

	SMeshNormal*             m_pNorms;
	SMeshTangents*           m_pTangents;
	SMeshQTangents*          m_pQTangents;
	SMeshTexCoord*           m_pTexCoord;
	SMeshColor*              m_pColor0;
	SMeshColor*              m_pColor1;

	i32*                     m_pVertMats;
	SVF_P3S_C4B_T2S*         m_pP3S_C4B_T2S;

	SMeshBoneMapping_u16* m_pBoneMapping;      //!< Bone-mapping for the final render-mesh.
	SMeshBoneMapping_u16* m_pExtraBoneMapping; //!< Bone indices and weights for bones 5 to 8.

	i32                      m_nCoorCount;  //!< Number of texture coordinates in m_pTexCoord array.
	i32                      m_streamSize[LAST_STREAM];

	//! Bounding box.
	AABB m_bbox;

	//! Array of mesh subsets.
	DynArray<SMeshSubset> m_subsets;

	//! Mask that indicate if this stream is using not allocated in Mesh pointer.
	//! For instance, if (m_nSharedStreamMask & (1<<NORMALS)), then normals stream is shared.
	u32 m_nSharedStreamMask;

	//! Texture space area divided by geometry area. Zero if cannot compute.
	float m_texMappingDensity;

	//! Geometric mean value calculated from the areas of this mesh faces.
	float m_geometricMeanFaceArea;

	// morph deltas (referenced by m_verticesDeltaOffsets)
	DynArray<Vec4> m_vertexDeltas;

	// map of all morphs per vertex
	// TODO: this is CSMorphsBitfield
	DynArray<uint64> m_vertexMorphsBitfield;

	// number of total morphs
	u32 m_numMorphs;

	// per vertex offsets into m_vertexDeltas
	DynArray<u32> m_verticesDeltaOffsets;

	//////////////////////////////////////////////////////////////////////////
	void GetMemoryUsage(class IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_subsets);
		pSizer->AddObject(m_vertexDeltas);
		pSizer->AddObject(m_vertexMorphsBitfield);
		pSizer->AddObject(m_numMorphs);
		pSizer->AddObject(m_verticesDeltaOffsets);

		for (i32 stream = 0; stream < LAST_STREAM; stream++)
		{
			uk pStream;
			i32 nElementSize = 0;
			GetStreamInfo(stream, pStream, nElementSize);
			pSizer->AddObject(pStream, m_streamSize[stream] * nElementSize);
		}

	}

	//////////////////////////////////////////////////////////////////////////
	CMesh()
	{
		m_pFaces = NULL;
		m_pTopologyIds = NULL;
		m_pIndices = NULL;
		m_pPositions = NULL;
		m_pPositionsF16 = NULL;
		m_pNorms = NULL;
		m_pTangents = NULL;
		m_pQTangents = NULL;
		m_pTexCoord = NULL;
		m_pColor0 = NULL;
		m_pColor1 = NULL;
		m_pVertMats = NULL;
		m_pP3S_C4B_T2S = NULL;
		m_pBoneMapping = NULL;
		m_pExtraBoneMapping = NULL;

		m_nCoorCount = 0;

		memset(m_streamSize, 0, sizeof(m_streamSize));

		m_bbox.Reset();

		m_nSharedStreamMask = 0;

		m_texMappingDensity = 0.0f;
		m_geometricMeanFaceArea = 0.0f;
		m_numMorphs = 0;
	}

	virtual ~CMesh()
	{
		FreeStreams();
	}

	void FreeStreams()
	{
		for (i32 i = 0; i < LAST_STREAM; ++i)
		{
			ReallocStream(i, 0);
		}
	}

	i32 GetFaceCount() const
	{
		return m_streamSize[FACES];
	}
	i32 GetVertexCount() const
	{
		return max(max(m_streamSize[POSITIONS], m_streamSize[POSITIONSF16]), m_streamSize[P3S_C4B_T2S]);
	}
	i32 GetTexCoordCount() const
	{
		return m_nCoorCount;
	}
	i32 GetTangentCount() const
	{
		return m_streamSize[TANGENTS];
	}
	i32 GetSubSetCount() const
	{
		return m_subsets.size();
	}
	i32 GetIndexCount() const
	{
		return m_streamSize[INDICES];
	}

	void SetFaceCount(i32 nNewCount)
	{
		ReallocStream(FACES, nNewCount);
	}

	void SetVertexCount(i32 nNewCount)
	{
		if (GetVertexCount() != nNewCount || GetVertexCount() == 0)
		{
			ReallocStream(POSITIONS, nNewCount);
			ReallocStream(POSITIONSF16, 0);
			ReallocStream(NORMALS, nNewCount);

			if (m_pColor0)
			{
				ReallocStream(COLORS_0, nNewCount);
			}

			if (m_pColor1)
			{
				ReallocStream(COLORS_1, nNewCount);
			}

			if (m_pVertMats)
			{
				ReallocStream(VERT_MATS, nNewCount);
			}
		}
	}

	void SetTexCoordsCount(i32 nNewCount)
	{
		if (m_nCoorCount != nNewCount || m_nCoorCount == 0)
		{
			ReallocStream(TEXCOORDS, nNewCount);
			m_nCoorCount = nNewCount;
		}
	}

	void SetTexCoordsAndTangentsCount(i32 nNewCount)
	{
		if (m_nCoorCount != nNewCount || m_nCoorCount == 0)
		{
			ReallocStream(TEXCOORDS, nNewCount);
			ReallocStream(TANGENTS, nNewCount);
			m_nCoorCount = nNewCount;
		}
	}

	void SetIndexCount(i32 nNewCount)
	{
		ReallocStream(INDICES, nNewCount);
	}

	//! Set specific stream as shared.
	void SetSharedStream(i32 stream, uk pStream, i32 nElementCount)
	{
		assert(stream >= 0 && stream < LAST_STREAM);
		if ((m_nSharedStreamMask & (1 << stream)) == 0)
		{
			ReallocStream(stream, 0);
			m_nSharedStreamMask |= (1 << stream);
		}
		SetStreamData(stream, pStream, nElementCount);
	}

	template<class T>
	T* GetStreamPtr(i32 stream, i32* pElementCount = 0) const
	{
		uk pStream = 0;
		i32 nElementSize = 0;
		GetStreamInfo(stream, pStream, nElementSize);

		if (nElementSize != sizeof(T))
		{
			assert(0);
			pStream = 0;
		}

		i32k nElementCount = (pStream ? this->m_streamSize[stream] : 0);

		if (pElementCount)
		{
			*pElementCount = nElementCount;
		}
		return (T*)pStream;
	}

	void GetStreamInfo(i32 stream, uk & pStream, i32& nElementSize) const
	{
		pStream = 0;
		nElementSize = 0;
		assert(stream >= 0 && stream < LAST_STREAM);
		switch (stream)
		{
		case POSITIONS:
			pStream = m_pPositions;
			nElementSize = sizeof(Vec3);
			break;
		case POSITIONSF16:
			pStream = m_pPositionsF16;
			nElementSize = sizeof(Vec3f16);
			break;
		case NORMALS:
			pStream = m_pNorms;
			nElementSize = sizeof(Vec3);
			break;
		case VERT_MATS:
			pStream = m_pVertMats;
			nElementSize = sizeof(i32);
			break;
		case FACES:
			pStream = m_pFaces;
			nElementSize = sizeof(SMeshFace);
			break;
		case TOPOLOGY_IDS:
			pStream = m_pTopologyIds;
			nElementSize = sizeof(i32);
			break;
		case TEXCOORDS:
			pStream = m_pTexCoord;
			nElementSize = sizeof(SMeshTexCoord);
			break;
		case COLORS_0:
			pStream = m_pColor0;
			nElementSize = sizeof(SMeshColor);
			break;
		case COLORS_1:
			pStream = m_pColor1;
			nElementSize = sizeof(SMeshColor);
			break;
		case INDICES:
			pStream = m_pIndices;
			nElementSize = sizeof(vtx_idx);
			break;
		case TANGENTS:
			pStream = m_pTangents;
			nElementSize = sizeof(SMeshTangents);
			break;
		case QTANGENTS:
			pStream = m_pQTangents;
			nElementSize = sizeof(SMeshQTangents);
			break;
		case BONEMAPPING:
			pStream = m_pBoneMapping;
			nElementSize = sizeof(SMeshBoneMapping_u16);
			break;
		case EXTRABONEMAPPING:
			pStream = m_pExtraBoneMapping;
			nElementSize = sizeof(SMeshBoneMapping_u16);
			break;
		case P3S_C4B_T2S:
			pStream = m_pP3S_C4B_T2S;
			nElementSize = sizeof(SVF_P3S_C4B_T2S);
			break;
		default:
			// unknown stream
			assert(0);
			break;
		}
	}

	virtual void ReallocStream(i32 stream, i32 nNewCount)
	{
		if (stream < 0 || stream >= LAST_STREAM)
		{
			assert(0);
			return;
		}

		if (m_nSharedStreamMask & (1 << stream))
		{
			m_nSharedStreamMask &= ~(1 << stream);

			if (nNewCount <= 0)
			{
				SetStreamData(stream, 0, 0);
			}
			else
			{
				i32k nOldCount = m_streamSize[stream];
				uk pOldElements = 0;
				i32 nElementSize = 0;
				GetStreamInfo(stream, pOldElements, nElementSize);

				uk const pNewElements = realloc(0, nNewCount * nElementSize);
				if (!pNewElements)
				{
					// allocation failed
					assert(0);
					SetStreamData(stream, 0, 0);
					return;
				}

				if (nOldCount > 0)
				{
					memcpy(pNewElements, pOldElements, min(nOldCount, nNewCount) * nElementSize);
				}
				if (nNewCount > nOldCount)
				{
					memset((tuk)pNewElements + nOldCount * nElementSize, 0, (nNewCount - nOldCount) * nElementSize);
				}

				SetStreamData(stream, pNewElements, nNewCount);
			}
		}
		else
		{
			i32k nOldCount = m_streamSize[stream];
			if (nOldCount == nNewCount)
			{
				// stream already has required size
				return;
			}

			uk pOldElements = 0;
			i32 nElementSize = 0;
			GetStreamInfo(stream, pOldElements, nElementSize);

			if (nNewCount <= 0)
			{
				free(pOldElements);
				SetStreamData(stream, 0, 0);
			}
			else
			{
				uk const pNewElements = realloc(pOldElements, nNewCount * nElementSize);
				if (!pNewElements)
				{
					// allocation failed
					assert(0);
					free(pOldElements);
					SetStreamData(stream, 0, 0);
					return;
				}

				if (nNewCount > nOldCount)
				{
					memset((tuk)pNewElements + nOldCount * nElementSize, 0, (nNewCount - nOldCount) * nElementSize);
				}

				SetStreamData(stream, pNewElements, nNewCount);
			}
		}
	}

	//! Copy mesh from source mesh.
	void CopyFrom(const CMesh& mesh)
	{
		for (i32 stream = 0; stream < LAST_STREAM; stream++)
		{
			ReallocStream(stream, mesh.m_streamSize[stream]);
			if (mesh.m_streamSize[stream] > 0)
			{
				uk pSrcStream = 0;
				uk pTrgStream = 0;
				i32 nElementSize = 0;
				mesh.GetStreamInfo(stream, pSrcStream, nElementSize);
				GetStreamInfo(stream, pTrgStream, nElementSize);
				if (pSrcStream && pTrgStream)
				{
					memcpy(pTrgStream, pSrcStream, m_streamSize[stream] * nElementSize);
				}
			}
		}
		m_bbox = mesh.m_bbox;
		m_subsets = mesh.m_subsets;
		m_texMappingDensity = mesh.m_texMappingDensity;
		m_geometricMeanFaceArea = mesh.m_geometricMeanFaceArea;

		m_vertexDeltas = mesh.m_vertexDeltas;
		m_vertexMorphsBitfield = mesh.m_vertexMorphsBitfield;
		m_numMorphs = mesh.m_numMorphs;
		m_verticesDeltaOffsets = mesh.m_verticesDeltaOffsets;
	}

	bool CompareStreams(const CMesh& mesh) const
	{
		for (i32 stream = 0; stream < LAST_STREAM; stream++)
		{
			if (m_streamSize[stream] != mesh.m_streamSize[stream])
			{
				return false;
			}

			if (m_streamSize[stream])
			{
				uk pStream1 = 0;
				uk pStream2 = 0;
				i32 nElementSize1 = 0;
				i32 nElementSize2 = 0;
				GetStreamInfo(stream, pStream1, nElementSize1);
				mesh.GetStreamInfo(stream, pStream2, nElementSize2);

				assert(nElementSize1 == nElementSize2);

				if ((pStream1 && !pStream2) || (!pStream1 && pStream2))
				{
					return false;
				}

				if (pStream1 && pStream2)
				{
					if (memcmp(pStream1, pStream2, m_streamSize[stream] * nElementSize1) != 0)
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	//! Add streams from source mesh to the end of existing streams.
	tukk AppendStreamsFrom(const CMesh& mesh)
	{
		return AppendStreamsFrom(mesh, 0, -1, 0, -1);
	}

	//! Add streams from source mesh to the end of existing streams.
	tukk AppendStreamsFrom(const CMesh& mesh, i32 fromVertex, i32 vertexCount, i32 fromFace, i32 faceCount)
	{
		if (GetIndexCount() > 0 || mesh.GetIndexCount() > 0)
		{
			assert(0);
			return "Cmesh::Append() cannot handle meshes with indices, it can handle faces only";
		}

		// Non-ranged requests should start from 0th element and element count should be <0.
		if ((vertexCount < 0 && fromVertex != 0) || (faceCount < 0 && fromFace != 0))
		{
			assert(0);
			return "Cmesh::Append(): Bad CMesh parameters";
		}
		if (vertexCount < 0)
		{
			vertexCount = mesh.GetVertexCount();
		}
		if (faceCount < 0)
		{
			faceCount = mesh.GetFaceCount();
		}

		i32k oldVertexCount = GetVertexCount();
		i32k oldFaceCount = GetFaceCount();
		i32k nOldCoorCount = GetTexCoordCount();

		if (GetTexCoordCount() != 0 && GetTexCoordCount() != oldVertexCount)
		{
			assert(0);
			return "Cmesh::Append(): Mismatch in target CMesh vert/tcoord counts";
		}

		if (mesh.GetTexCoordCount() != 0 && mesh.GetTexCoordCount() != mesh.GetVertexCount())
		{
			assert(0);
			return "Cmesh::Append(): Mismatch in source CMesh vert/tcoord counts";
		}

		for (i32 stream = 0; stream < LAST_STREAM; ++stream)
		{
			i32k oldCount = (stream == FACES) ? oldFaceCount : oldVertexCount;

			i32k from = (stream == FACES) ? fromFace : fromVertex;
			i32k count = (stream == FACES) ? faceCount : vertexCount;

			i32k oldStreamSize = m_streamSize[stream];
			i32k streamSize = mesh.m_streamSize[stream];

			if (oldStreamSize <= 0 && (count <= 0 || streamSize <= 0))
			{
				continue;
			}

			ReallocStream(stream, oldCount + count);

			if (count > 0)
			{
				uk pSrcStream = 0;
				uk pTrgStream = 0;
				i32 srcElementSize = 0;
				i32 trgElementSize = 0;
				mesh.GetStreamInfo(stream, pSrcStream, srcElementSize);
				GetStreamInfo(stream, pTrgStream, trgElementSize);

				assert(srcElementSize == trgElementSize);

				if (pSrcStream && pTrgStream)
				{
					memcpy(
					  (tuk)pTrgStream + oldCount * trgElementSize,
					  (tuk)pSrcStream + from * srcElementSize,
					  count * srcElementSize);
				}
			}
		}

		{
			i32k nOffset = oldVertexCount - fromVertex;
			i32k newFaceCount = GetFaceCount();

			for (i32 i = oldFaceCount; i < newFaceCount; ++i)
			{
				m_pFaces[i].v[0] += nOffset;
				m_pFaces[i].v[1] += nOffset;
				m_pFaces[i].v[2] += nOffset;
			}
		}

		m_bbox.Add(mesh.m_bbox.min);
		m_bbox.Add(mesh.m_bbox.max);

		return 0;
	}

	void RemoveRangeFromStream(i32 stream, i32 nFirst, i32 nCount)
	{
		if (stream < 0 || stream >= LAST_STREAM)
		{
			assert(0);
			return;
		}

		if (m_nSharedStreamMask & (1 << stream))
		{
			// Make shared stream non-shared
			ReallocStream(stream, m_streamSize[stream]);
		}

		i32k nTotalCount = m_streamSize[stream];

		i32 nElementSize;
		uk pStream = 0;
		GetStreamInfo(stream, pStream, nElementSize);

		if (nFirst >= nTotalCount || nTotalCount <= 0 || pStream == 0)
		{
			return;
		}
		if (nFirst + nCount > nTotalCount)
		{
			nCount = nTotalCount - nFirst;
		}
		if (nCount <= 0)
		{
			return;
		}

		i32k nTailCount = nTotalCount - (nFirst + nCount);
		if (nTailCount > 0)
		{
			tuk const pRangeStart = (tuk)pStream + nFirst * nElementSize;
			tuk const pRangeEnd = (tuk)pStream + (nFirst + nCount) * nElementSize;
			memmove(pRangeStart, pRangeEnd, nTailCount * nElementSize);
		}

		ReallocStream(stream, nTotalCount - nCount);
	}

	bool Validate(tukk* const ppErrorDescription) const
	{
		i32k vertexCount = GetVertexCount();
		i32k faceCount = GetFaceCount();
		i32k indexCount = GetIndexCount();

		if ((faceCount <= 0) && (indexCount <= 0))
		{
			if (vertexCount > 0)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "no any indices, but vertices exist";
				return false;
			}
		}

		if ((uint)vertexCount > (sizeof(vtx_idx) == 2 ? 0xffff : 0x7fffffff))
		{
			if (ppErrorDescription)
			{
				ppErrorDescription[0] =
				  (sizeof(vtx_idx) == 2)
				  ? "vertex count is greater or equal than 64K"
				  : "vertex count is greater or equal than 2G";
			}
			return false;
		}

		if (faceCount > 0)
		{
			if (vertexCount <= 0)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "no any vertices, but faces exist";
				return false;
			}
		}

		if (indexCount > 0)
		{
			if (vertexCount <= 0)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "no any vertices, but indices exist";
				return false;
			}
		}

		for (i32 i = 0; i < faceCount; ++i)
		{
			const SMeshFace& face = m_pFaces[i];
			for (i32 j = 0; j < 3; ++j)
			{
				i32k v = face.v[j];
				if ((v < 0) || (v >= vertexCount))
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a face refers vertex outside of vertex array";
					return false;
				}
			}
		}

		for (i32 i = 0; i < indexCount; i++)
		{
			if ((uint)m_pIndices[i] >= (uint)vertexCount)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "an index refers vertex outside of vertex array";
				return false;
			}
		}

		if (GetTexCoordCount() != 0 && GetTexCoordCount() != vertexCount)
		{
			if (ppErrorDescription) ppErrorDescription[0] = "number of texture coordinates is different from number of vertices";
			return false;
		}

		if (!_finite(m_bbox.min.x) || !_finite(m_bbox.min.y) || !_finite(m_bbox.min.z) ||
		    !_finite(m_bbox.max.x) || !_finite(m_bbox.max.y) || !_finite(m_bbox.max.z))
		{
			if (ppErrorDescription) ppErrorDescription[0] = "bounding box contains damaged data";
			return false;
		}

		if (m_bbox.IsReset())
		{
			if (ppErrorDescription) ppErrorDescription[0] = "bounding box is not set";
			return false;
		}

		if (m_bbox.max.x < m_bbox.min.x ||
		    m_bbox.max.y < m_bbox.min.y ||
		    m_bbox.max.z < m_bbox.min.z)
		{
			if (ppErrorDescription) ppErrorDescription[0] = "bounding box min is greater than max";
			return false;
		}

		if (m_bbox.min.GetDistance(m_bbox.max) < 0.001f)
		{
			if (ppErrorDescription) ppErrorDescription[0] = "bounding box is less than 1 mm in size";
			return false;
		}

		for (i32 s = 0, subsetCount = m_subsets.size(); s < subsetCount; ++s)
		{
			const SMeshSubset& subset = m_subsets[s];

			if (subset.nNumIndices <= 0)
			{
				if (subset.nNumVerts > 0)
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset without indices contains vertices";
					return false;
				}
				continue;
			}
			else if (subset.nNumVerts <= 0)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset has indices but vertices are missing";
				return false;
			}

			if (subset.nFirstIndexId < 0)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset has negative start position in index array";
				return false;
			}
			if (subset.nFirstIndexId + subset.nNumIndices > indexCount)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset refers indices outside of index array";
				return false;
			}
			if (subset.nFirstVertId < 0)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset has negative start position in vertex array";
				return false;
			}
			if (subset.nFirstVertId + subset.nNumVerts > vertexCount)
			{
				if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset refers vertices outside of vertex array";
				return false;
			}

			for (i32 ii = subset.nFirstIndexId; ii < subset.nFirstIndexId + subset.nNumIndices; ++ii)
			{
				const uint index = m_pIndices[ii];
				if (index < (uint)subset.nFirstVertId)
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset refers a vertex lying before subset vertices";
					return false;
				}
				if (index >= (uint)(subset.nFirstVertId + subset.nNumVerts))
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset refers a vertex lying after subset vertices";
					return false;
				}

				Vec3 p(ZERO);
				const Vec3* pp = &p;
				if (m_pPositions)
				{
					pp = &m_pPositions[index];
				}
				else if (m_pPositionsF16)
				{
					p = m_pPositionsF16[index].ToVec3();
				}
				else if (m_pP3S_C4B_T2S)
				{
					p = m_pP3S_C4B_T2S[index].xyz.ToVec3();
				}
				if (!_finite(pp->x))
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset contains a vertex with damaged x component";
					return false;
				}
				if (!_finite(pp->y))
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset contains a vertex with damaged y component";
					return false;
				}
				if (!_finite(pp->z))
				{
					if (ppErrorDescription) ppErrorDescription[0] = "a mesh subset contains a vertex with damaged z component";
					return false;
				}
			}
		}

		return true;
	}

	bool ComputeSubsetTexMappingAreas(
	  size_t subsetIndex,
	  float& computedPosArea, float& computedTexArea, tukk & errorText)
	{
		computedPosArea = 0.0f;
		computedTexArea = 0.0f;
		errorText = "";

		if (subsetIndex >= (size_t)m_subsets.size())
		{
			errorText = "subset index is bad";
			return false;
		}

		if (GetIndexCount() <= 0)
		{
			errorText = "missing indices";
			return false;
		}

		if ((GetVertexCount() <= 0) || ((m_pPositions == NULL) && (m_pPositionsF16 == NULL)))
		{
			errorText = "missing vertices";
			return false;
		}

		if ((m_pTexCoord == NULL) || (GetTexCoordCount() <= 0))
		{
			errorText = "missing texture coordinates";
			return false;
		}

		const SMeshSubset& subset = m_subsets[subsetIndex];

		if ((subset.nNumIndices <= 0) || (subset.nFirstIndexId < 0))
		{
			errorText = "missing or bad indices in subset";
			return false;
		}

		bool ok;
		if (m_pPositions)
		{
			ok = CMeshHelpers::ComputeTexMappingAreas(
			  subset.nNumIndices, &m_pIndices[subset.nFirstIndexId],
			  GetVertexCount(),
			  &m_pPositions[0], sizeof(m_pPositions[0]),
			  &m_pTexCoord[0], sizeof(m_pTexCoord[0]),
			  computedPosArea, computedTexArea, errorText);
		}
		else
		{
			ok = CMeshHelpers::ComputeTexMappingAreas(
			  subset.nNumIndices, &m_pIndices[subset.nFirstIndexId],
			  GetVertexCount(),
			  &m_pPositionsF16[0], sizeof(m_pPositionsF16[0]),
			  &m_pTexCoord[0], sizeof(m_pTexCoord[0]),
			  computedPosArea, computedTexArea, errorText);
		}

		return ok;
	}

	//! \note This function doesn't work for "old" uncompressed meshes (with faces instead of indices).
	bool RecomputeTexMappingDensity()
	{
		m_texMappingDensity = 0;

		if (GetFaceCount() > 0)
		{
			// uncompressed mesh - not supported
			return false;
		}

		if ((GetIndexCount() <= 0) || (GetVertexCount() <= 0) || ((m_pPositions == NULL) && (m_pPositionsF16 == NULL)))
		{
			return false;
		}

		if ((m_pTexCoord == NULL) || (GetTexCoordCount() <= 0))
		{
			return false;
		}

		float totalPosArea = 0;
		float totalTexArea = 0;

		for (size_t i = 0, count = m_subsets.size(); i < count; ++i)
		{
			const SMeshSubset& subset = m_subsets[i];

			float posArea;
			float texArea;
			tukk errorText = "";

			const bool ok = ComputeSubsetTexMappingAreas(i, posArea, texArea, errorText);

			if (ok)
			{
				totalPosArea += posArea;
				totalTexArea += texArea;
			}
		}

		if (totalPosArea <= 0)
		{
			return false;
		}

		m_texMappingDensity = totalTexArea / totalPosArea;
		return true;
	}

	bool RecomputeGeometricMeanFaceArea()
	{
		m_geometricMeanFaceArea = 0.0f;

		if (GetFaceCount() > 0)
		{
			// uncompressed mesh - not supported
			return false;
		}

		if ((GetIndexCount() <= 0) || (GetVertexCount() <= 0) || ((m_pPositions == NULL) && (m_pPositionsF16 == NULL)))
		{
			return false;
		}

		std::vector<float> areas;
		const size_t subsetCount = m_subsets.size();

		for (size_t i = 0; i < subsetCount; ++i)
		{
			CollectSubsetFaceAreas(m_subsets[i], areas);
		}

		const size_t areasCount = areas.size();
		if (areasCount == 0)
		{
			return false;
		}

		float fGeometricTotal = 0.0f;
		for (size_t i = 0; i < areasCount; ++i)
		{
			fGeometricTotal += logf(areas[i]);
		}

		// Cannot use expf(), because junk bits in xmm0 sometimes raise an "invalid floating operation"  (msvc 19.00.23026).
		m_geometricMeanFaceArea = (float)exp((double)fGeometricTotal / areasCount);

		assert(m_geometricMeanFaceArea > 0.0f);

		return true;
	}

	bool CollectSubsetFaceAreas(const SMeshSubset& subset, std::vector<float>& areas)
	{
		if ((subset.nNumIndices <= 0) || (subset.nFirstIndexId < 0))
		{
			return false;
		}

		bool ok = false;
		if (m_pPositions != NULL)
		{
			ok = CMeshHelpers::CollectFaceAreas(subset.nNumIndices, &m_pIndices[subset.nFirstIndexId],
			                                    GetVertexCount(), &m_pPositions[0], sizeof(m_pPositions[0]), areas);
		}
		else if (m_pPositionsF16 != NULL)
		{
			ok = CMeshHelpers::CollectFaceAreas(subset.nNumIndices, &m_pIndices[subset.nFirstIndexId],
			                                    GetVertexCount(), &m_pPositionsF16[0], sizeof(m_pPositionsF16[0]), areas);
		}

		return ok;
	}

	//! Estimate the size of the render mesh.
	u32 EstimateRenderMeshMemoryUsage() const
	{
		const size_t cSizeStream[VSF_NUM] = {
			0U,
			sizeof(SPipTangents),        // VSF_TANGENTS
			sizeof(SPipQTangents),       // VSF_QTANGENTS
			sizeof(SVF_W4B_I4S),         // VSF_HWSKIN_INFO
			sizeof(SVF_P3F),             // VSF_VERTEX_VELOCITY
#if ENABLE_NORMALSTREAM_SUPPORT
			sizeof(SPipNormal),          // VSF_NORMALS
#endif
		};

		u32 nMeshSize = 0;
		u32 activeStreams = (GetVertexCount()) ? 1U << VSF_GENERAL : 0U;
		activeStreams |=
		  (m_pQTangents) ? (1U << VSF_QTANGENTS) :
		  (m_pTangents) ? (1U << VSF_TANGENTS) : 0U;
		if (m_pBoneMapping)
			activeStreams |= 1U << VSF_HWSKIN_INFO;
		for (u32 i = 0; i < VSF_NUM; i++)
		{
			if (activeStreams & (1U << i))
			{
				nMeshSize += ((i == VSF_GENERAL) ? sizeof(SVF_P3S_C4B_T2S) : cSizeStream[i]) * GetVertexCount();
				nMeshSize += TARGET_DEFAULT_ALIGN - (nMeshSize & (TARGET_DEFAULT_ALIGN - 1));
			}
		}
		if (GetIndexCount())
		{
			nMeshSize += GetIndexCount() * sizeof(vtx_idx);
			nMeshSize += TARGET_DEFAULT_ALIGN - (nMeshSize & (TARGET_DEFAULT_ALIGN - 1));
		}

		return nMeshSize;
	}

	//! This function used when we do not have an actual mesh, but only vertex/index count of it.
	static u32 ApproximateRenderMeshMemoryUsage(i32 nVertexCount, i32 nIndexCount)
	{
		u32 nMeshSize = 0;
		nMeshSize += nVertexCount * sizeof(SVF_P3S_C4B_T2S);
		nMeshSize += nVertexCount * sizeof(SPipTangents);

		nMeshSize += nIndexCount * sizeof(vtx_idx);
		return nMeshSize;
	}

private:
	//! Set stream size.
	void SetStreamData(i32 stream, uk pStream, i32 nNewCount)
	{
		if (stream < 0 || stream >= LAST_STREAM)
		{
			assert(0);
			return;
		}
		m_streamSize[stream] = nNewCount;
		switch (stream)
		{
		case POSITIONS:
			m_pPositions = (Vec3*)pStream;
			break;
		case POSITIONSF16:
			m_pPositionsF16 = (Vec3f16*)pStream;
			break;
		case NORMALS:
			m_pNorms = (SMeshNormal*)pStream;
			break;
		case VERT_MATS:
			m_pVertMats = (i32*)pStream;
			break;
		case FACES:
			m_pFaces = (SMeshFace*)pStream;
			break;
		case TOPOLOGY_IDS:
			m_pTopologyIds = (i32*)pStream;
			break;
		case TEXCOORDS:
			m_pTexCoord = (SMeshTexCoord*)pStream;
			m_nCoorCount = nNewCount;
			break;
		case COLORS_0:
			m_pColor0 = (SMeshColor*)pStream;
			break;
		case COLORS_1:
			m_pColor1 = (SMeshColor*)pStream;
			break;
		case INDICES:
			m_pIndices = (vtx_idx*)pStream;
			break;
		case TANGENTS:
			m_pTangents = (SMeshTangents*)pStream;
			break;
		case QTANGENTS:
			m_pQTangents = (SMeshQTangents*)pStream;
			break;
		case BONEMAPPING:
			m_pBoneMapping = (SMeshBoneMapping_u16*)pStream;
			break;
		case EXTRABONEMAPPING:
			m_pExtraBoneMapping = (SMeshBoneMapping_u16*)pStream;
			break;
		case P3S_C4B_T2S:
			m_pP3S_C4B_T2S = (SVF_P3S_C4B_T2S*)pStream;
			m_nCoorCount = nNewCount;
			break;
		default:
			// unknown stream
			assert(0);
			break;
		}
	}
};
//! \endcond

// Описание:
//! Editable mesh interface.
//! IndexedMesh can be created directly or loaded from CGF file, before rendering it is converted into IRenderMesh.
//! IStatObj is used to host IIndexedMesh, and corresponding IRenderMesh.
struct IIndexedMesh
{
	/*! Structure used for read-only access to mesh data. Used by GetMesh() function */
	struct SMeshDescription
	{
		const SMeshFace*     m_pFaces;      //!< Pointer to array of faces.
		const Vec3*          m_pVerts;      //!< Pointer to array of vertices in f32 format.
		const Vec3f16*       m_pVertsF16;   //!< Pointer to array of vertices in f16 format.
		const SMeshNormal*   m_pNorms;      //!< Pointer to array of normals.
		const SMeshColor*    m_pColor;      //!< Pointer to array of vertex colors.
		const SMeshTexCoord* m_pTexCoord;   //!< Pointer to array of texture coordinates.
		const vtx_idx*       m_pIndices;    //!< pointer to array of indices.
		i32                  m_nFaceCount;  //!< Number of elements m_pFaces array.
		i32                  m_nVertCount;  //!< Number of elements in m_pVerts, m_pNorms and m_pColor arrays.
		i32                  m_nCoorCount;  //!< Number of elements in m_pTexCoord array.
		i32                  m_nIndexCount; //!< Number of elements in m_pIndices array.
	};

	// <interfuscator:shuffle>
	virtual ~IIndexedMesh() {}

	//! Release indexed mesh.
	virtual void Release() = 0;

	//! Gives read-only access to mesh data.
	virtual void   GetMeshDescription(SMeshDescription& meshDesc) const = 0;

	virtual CMesh* GetMesh() = 0;

	virtual void   SetMesh(CMesh& mesh) = 0;

	//! Frees vertex and face streams. Calling this function invalidates SMeshDescription pointers.
	virtual void FreeStreams() = 0;

	//! Return number of allocated faces.
	virtual i32 GetFaceCount() const = 0;

	//! Reallocates faces. Calling this function invalidates SMeshDescription pointers.
	virtual void SetFaceCount(i32 nNewCount) = 0;

	//! Return number of allocated vertices, normals and colors.
	virtual i32 GetVertexCount() const = 0;

	//! Reallocates vertices, normals and colors. Calling this function invalidates SMeshDescription pointers.
	virtual void SetVertexCount(i32 nNewCount) = 0;

	//! Reallocates colors. Calling this function invalidates SMeshDescription pointers.
	virtual void SetColorCount(i32 nNewCount) = 0;

	//! Return number of allocated texture coordinates.
	virtual i32 GetTexCoordCount() const = 0;

	//! Reallocates texture coordinates. Calling this function invalidates SMeshDescription pointers.
	virtual void SetTexCoordCount(i32 nNewCount) = 0;

	//! Return number of allocated tangents.
	virtual i32 GetTangentCount() const = 0;

	//! Reallocates tangents. Calling this function invalidates SMeshDescription pointers.
	virtual void SetTangentCount(i32 nNewCount) = 0;

	//! Get number of indices in the mesh.
	virtual i32 GetIndexCount() const = 0;

	//! Set number of indices in the mesh.
	virtual void SetIndexCount(i32 nNewCount) = 0;

	//! Allocates m_pBoneMapping in CMesh.
	virtual void AllocateBoneMapping() = 0;

	// Subset access.
	virtual i32                GetSubSetCount() const = 0;
	virtual void               SetSubSetCount(i32 nSubsets) = 0;
	virtual const SMeshSubset& GetSubSet(i32 nIndex) const = 0;
	virtual void               SetSubsetBounds(i32 nIndex, const Vec3& vCenter, float fRadius) = 0;
	virtual void               SetSubsetIndexVertexRanges(i32 nIndex, i32 nFirstIndexId, i32 nNumIndices, i32 nFirstVertId, i32 nNumVerts) = 0;
	virtual void               SetSubsetMaterialId(i32 nIndex, i32 nMatID) = 0;
	virtual void               SetSubsetMaterialProperties(i32 nIndex, i32 nMatFlags, i32 nPhysicalizeType) = 0;

	// Mesh bounding box.
	virtual void SetBBox(const AABB& box) = 0;
	virtual AABB GetBBox() const = 0;
	virtual void CalcBBox() = 0;

	virtual void RestoreFacesFromIndices() = 0;
	// </interfuscator:shuffle>

#if DRX_PLATFORM_WINDOWS
	//! Optimizes mesh.
	virtual void Optimize(tukk szComment = NULL) = 0;
#endif
};
