// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/Containers/DrxArray.h>

//! Stream Configuration options
#define ENABLE_NORMALSTREAM_SUPPORT 1

//////////////////////////////////////////////////////////////////////
// All possible primitive types
enum PublicRenderPrimitiveType
{
	prtTriangleList,
	prtTriangleStrip,
	prtLineList,
	prtLineStrip
};

//////////////////////////////////////////////////////////////////////
struct InputLayoutHandle
{
	typedef u8 ValueType;
	ValueType value;

	constexpr InputLayoutHandle() : value(Unspecified) { }
	constexpr InputLayoutHandle(ValueType v) : value(v) { }

	// Test operators
	template<typename T> bool operator ==(const T other) const { return value == other; }
	template<typename T> bool operator !=(const T other) const { return value != other; }
	// Range operators
	template<typename T> bool operator <=(const T other) const { return value <= other; }
	template<typename T> bool operator >=(const T other) const { return value >= other; }
	// Sorting operators
	template<typename T> bool operator < (const T other) const { return value <  other; }
	template<typename T> bool operator > (const T other) const { return value >  other; }

	// Auto cast for array access operator []
	operator ValueType() const { return value; }

	// Not an enum, because of SWIG
	static constexpr ValueType Unspecified = ValueType(~0);
};

// InputLayout API (pre-allocated sampler states)
// TODO: Move to DeviceObjects.h
struct EDefaultInputLayouts : InputLayoutHandle
{
	enum PreDefs : ValueType
	{
		Empty                 =  0,    //!< Empty layout for pull-model shaders

		// Base stream
		P3F_C4B_T2F           =  1,
		P3S_C4B_T2S           =  2,
		P3S_N4B_C4B_T2S       =  3,

		P3F_C4B_T4B_N3F2      =  4,    //!< Particles.
		TP3F_C4B_T2F          =  5,    //!< Full screen quad
		P3F_T3F               =  6,    //!< Fog shadows
		P3F_T2F_T3F           =  7,    //!< Mesh baker

		P3F                   =  8,    //!< Raw Geometry

		P2S_N4B_C4B_T1F       =  9,    //!< Terrain sector
		P3F_C4B_T2S           = 10,    //!< Road elements

		// Additional streams
		T4F_B4F               = 11,    //!< Tangent/Bitangent
		T4S_B4S               = 12,    //!< Tangent/Bitangent
		Q4F                   = 13,    //!< QTangent
		Q4S                   = 14,    //!< QTangent
		N3F                   = 15,    //!< Normal
		W4B_I4S               = 16,    //!< Skinned weights/indices stream
		V3F                   = 17,    //!< Velocity stream
		W2F                   = 18,    //!< Morph-buddy weights

		V4Fi                  = 19,    //!< Instanced Vec4 stream

		PreAllocated          = 20,    // from this value and up custom input layouts are assigned
		MaxRenderMesh         = PreAllocated
	};
};

//////////////////////////////////////////////////////////////////////
typedef Vec4_tpl<i16> Vec4sf;   //!< Used for tangents only.

//! bNeedNormals=1 - float normals; bNeedNormals=2 - byte normals
inline InputLayoutHandle VertFormatForComponents(bool bNeedCol, bool bHasTC, bool bHasPS, bool bHasNormal)
{
	InputLayoutHandle RequestedVertFormat;

	if (bHasPS)
		RequestedVertFormat = EDefaultInputLayouts::P3F_C4B_T4B_N3F2;
	else if (bHasNormal)
		RequestedVertFormat = EDefaultInputLayouts::P3S_N4B_C4B_T2S;
	else
		RequestedVertFormat = EDefaultInputLayouts::P3S_C4B_T2S;

	return RequestedVertFormat;
}

struct UCol
{
	union
	{
		u32 dcolor;
		u8  bcolor[4];

		struct { u8 b, g, r, a; };
		struct { u8 z, y, x, w; };
	};

	//! Get normal vector from unsigned 8bit integers (can't point up/down and is not normal).
	ILINE Vec3 GetN()
	{
		return Vec3
		       (
		  (bcolor[0] - 128.0f) / 127.5f,
		  (bcolor[1] - 128.0f) / 127.5f,
		  (bcolor[2] - 128.0f) / 127.5f
		       );
	}

	AUTO_STRUCT_INFO;
};

struct Vec3f16 : public DrxHalf4
{
	inline Vec3f16()
	{
	}
	inline Vec3f16(f32 _x, f32 _y, f32 _z)
	{
		x = DrxConvertFloatToHalf(_x);
		y = DrxConvertFloatToHalf(_y);
		z = DrxConvertFloatToHalf(_z);
		w = DrxConvertFloatToHalf(1.0f);
	}
	float operator[](i32 i) const
	{
		assert(i <= 3);
		return DrxConvertHalfToFloat(((DrxHalf*)this)[i]);
	}
	inline Vec3f16& operator=(const Vec3& sl)
	{
		x = DrxConvertFloatToHalf(sl.x);
		y = DrxConvertFloatToHalf(sl.y);
		z = DrxConvertFloatToHalf(sl.z);
		w = DrxConvertFloatToHalf(1.0f);
		return *this;
	}
	inline Vec3f16& operator=(const Vec4& sl)
	{
		x = DrxConvertFloatToHalf(sl.x);
		y = DrxConvertFloatToHalf(sl.y);
		z = DrxConvertFloatToHalf(sl.z);
		w = DrxConvertFloatToHalf(sl.w);
		return *this;
	}
	inline Vec3 ToVec3() const
	{
		Vec3 v;
		v.x = DrxConvertHalfToFloat(x);
		v.y = DrxConvertHalfToFloat(y);
		v.z = DrxConvertHalfToFloat(z);
		return v;
	}
};

struct Vec2f16 : public DrxHalf2
{
	inline Vec2f16()
	{
	}
	inline Vec2f16(f32 _x, f32 _y)
	{
		x = DrxConvertFloatToHalf(_x);
		y = DrxConvertFloatToHalf(_y);
	}
	Vec2f16& operator=(const Vec2& sl)
	{
		x = DrxConvertFloatToHalf(sl.x);
		y = DrxConvertFloatToHalf(sl.y);
		return *this;
	}
	float operator[](i32 i) const
	{
		assert(i <= 1);
		return DrxConvertHalfToFloat(((DrxHalf*)this)[i]);
	}
	inline Vec2 ToVec2() const
	{
		Vec2 v;
		v.x = DrxConvertHalfToFloat(x);
		v.y = DrxConvertHalfToFloat(y);
		return v;
	}
};

struct SVF_P3F_C4B_T2F
{
	Vec3 xyz;
	UCol color;
	Vec2 st;
};
struct SVF_TP3F_C4B_T2F
{
	Vec4 pos;
	UCol color;
	Vec2 st;
};
struct SVF_P3S_C4B_T2S
{
	Vec3f16 xyz;
	UCol    color;
	Vec2f16 st;

	AUTO_STRUCT_INFO;
};

struct SVF_P3F_C4B_T2S
{
	Vec3    xyz;
	UCol    color;
	Vec2f16 st;

	AUTO_STRUCT_INFO;
};

struct SVF_P3S_N4B_C4B_T2S
{
	Vec3f16 xyz;
	UCol    normal;
	UCol    color;
	Vec2f16 st;
};

struct SVF_P2S_N4B_C4B_T1F
{
	DrxHalf2 xy;
	UCol     normal;
	UCol     color;
	float    z;
};

struct SVF_T2F
{
	Vec2 st;
};
struct SVF_W4B_I4S
{
	UCol   weights;
	u16 indices[4];
};
struct SVF_C4B_C4B
{
	UCol coef0;
	UCol coef1;
};
struct SVF_P3F_P3F_I4B
{
	Vec3 thin;
	Vec3 fat;
	UCol index;
};
struct SVF_P3F
{
	Vec3 xyz;
};
struct SVF_P3F_T3F
{
	Vec3 p;
	Vec3 st;
};
struct SVF_P3F_T2F_T3F
{
	Vec3 p;
	Vec2 st0;
	Vec3 st1;
};
struct SVF_TP3F_T2F_T3F
{
	Vec4 p;
	Vec2 st0;
	Vec3 st1;
};
struct SVF_P2F_T4F_C4F
{
	Vec2 p;
	Vec4 st;
	Vec4 color;
};

struct SVF_P3F_C4B_T4B_N3F2
{
	Vec3 xyz;
	UCol color;
	UCol st;
	Vec3 xaxis;
	Vec3 yaxis;
};

struct SVF_C4B_T2S
{
	UCol    color;
	Vec2f16 st;
};

//! Signed norm value packing [-1,+1].
namespace PackingSNorm
{
ILINE i16 tPackF2B(const float f)
{
	return (i16)(f * 32767.0f);
}

ILINE i16 tPackS2B(i16k s)
{
	return (i16)(s * 32767);
}

ILINE float tPackB2F(i16k i)
{
	return (float)((float)i / 32767.0f);
}

ILINE i16 tPackB2S(i16k s)
{
	// OPT: "(s >> 15) + !(s >> 15)" works as well
	return (i16)(s / 32767);
}

#ifdef DRX_TYPE_SIMD4

ILINE Vec4sf tPackF2Bv(const Vec4H<f32>& v)
{
	Vec4sf vs;

	vs.x = tPackF2B(v.x);
	vs.y = tPackF2B(v.y);
	vs.z = tPackF2B(v.z);
	vs.w = tPackF2B(v.w);

	return vs;
}

#endif

ILINE Vec4sf tPackF2Bv(const Vec4f& v)
{
	Vec4sf vs;

	vs.x = tPackF2B(v.x);
	vs.y = tPackF2B(v.y);
	vs.z = tPackF2B(v.z);
	vs.w = tPackF2B(v.w);

	return vs;
}

ILINE Vec4sf tPackF2Bv(const Vec3& v)
{
	Vec4sf vs;

	vs.x = tPackF2B(v.x);
	vs.y = tPackF2B(v.y);
	vs.z = tPackF2B(v.z);
	vs.w = tPackF2B(1.0f);

	return vs;
}

ILINE Vec4 tPackB2F(const Vec4sf& v)
{
	Vec4 vs;

	vs.x = tPackB2F(v.x);
	vs.y = tPackB2F(v.y);
	vs.z = tPackB2F(v.z);
	vs.w = tPackB2F(v.w);

	return vs;
}

ILINE void tPackB2F(const Vec4sf& v, Vec4& vDst)
{
	vDst.x = tPackB2F(v.x);
	vDst.y = tPackB2F(v.y);
	vDst.z = tPackB2F(v.z);
	vDst.w = 1.0f;
}

ILINE void tPackB2FScale(const Vec4sf& v, Vec4& vDst, const Vec3& vScale)
{
	vDst.x = (float)v.x * vScale.x;
	vDst.y = (float)v.y * vScale.y;
	vDst.z = (float)v.z * vScale.z;
	vDst.w = 1.0f;
}

ILINE void tPackB2FScale(const Vec4sf& v, Vec3& vDst, const Vec3& vScale)
{
	vDst.x = (float)v.x * vScale.x;
	vDst.y = (float)v.y * vScale.y;
	vDst.z = (float)v.z * vScale.z;
}

ILINE void tPackB2F(const Vec4sf& v, Vec3& vDst)
{
	vDst.x = tPackB2F(v.x);
	vDst.y = tPackB2F(v.y);
	vDst.z = tPackB2F(v.z);
}
};

//! Pip => Graphics Pipeline structures, used for inputs for the GPU's Input Assembler.
//! These structures are optimized for fast decoding (ALU and bandwidth) and might be slow to encode on-the-fly
struct SPipTangents
{
	SPipTangents() {}

private:
	Vec4sf Tangent;
	Vec4sf Bitangent;

public:
	explicit SPipTangents(const Vec4sf& othert, const Vec4sf& otherb, i16k& othersign)
	{
		using namespace PackingSNorm;
		Tangent = othert;
		Tangent.w = PackingSNorm::tPackS2B(othersign);
		Bitangent = otherb;
		Bitangent.w = PackingSNorm::tPackS2B(othersign);
	}

	explicit SPipTangents(const Vec4sf& othert, const Vec4sf& otherb, const SPipTangents& othersign)
	{
		Tangent = othert;
		Tangent.w = othersign.Tangent.w;
		Bitangent = otherb;
		Bitangent.w = othersign.Bitangent.w;
	}

	explicit SPipTangents(const Vec4sf& othert, const Vec4sf& otherb)
	{
		Tangent = othert;
		Bitangent = otherb;
	}

	explicit SPipTangents(const Vec3& othert, const Vec3& otherb, i16k& othersign)
	{
		Tangent = Vec4sf(PackingSNorm::tPackF2B(othert.x), PackingSNorm::tPackF2B(othert.y), PackingSNorm::tPackF2B(othert.z), PackingSNorm::tPackS2B(othersign));
		Bitangent = Vec4sf(PackingSNorm::tPackF2B(otherb.x), PackingSNorm::tPackF2B(otherb.y), PackingSNorm::tPackF2B(otherb.z), PackingSNorm::tPackS2B(othersign));
	}

	explicit SPipTangents(const Vec3& othert, const Vec3& otherb, const SPipTangents& othersign)
	{
		Tangent = Vec4sf(PackingSNorm::tPackF2B(othert.x), PackingSNorm::tPackF2B(othert.y), PackingSNorm::tPackF2B(othert.z), othersign.Tangent.w);
		Bitangent = Vec4sf(PackingSNorm::tPackF2B(otherb.x), PackingSNorm::tPackF2B(otherb.y), PackingSNorm::tPackF2B(otherb.z), othersign.Bitangent.w);
	}

	explicit SPipTangents(const Quat& other, i16k& othersign)
	{
		Vec3 othert = other.GetColumn0();
		Vec3 otherb = other.GetColumn1();

		Tangent = Vec4sf(PackingSNorm::tPackF2B(othert.x), PackingSNorm::tPackF2B(othert.y), PackingSNorm::tPackF2B(othert.z), PackingSNorm::tPackS2B(othersign));
		Bitangent = Vec4sf(PackingSNorm::tPackF2B(otherb.x), PackingSNorm::tPackF2B(otherb.y), PackingSNorm::tPackF2B(otherb.z), PackingSNorm::tPackS2B(othersign));
	}

	void ExportTo(Vec4sf& othert, Vec4sf& otherb) const
	{
		othert = Tangent;
		otherb = Bitangent;
	}

	//! Get normal tangent and bitangent vectors.
	void GetTB(Vec4& othert, Vec4& otherb) const
	{
		othert = PackingSNorm::tPackB2F(Tangent);
		otherb = PackingSNorm::tPackB2F(Bitangent);
	}

	//! Get normal vector (perpendicular to tangent and bitangent plane).
	ILINE Vec3 GetN() const
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z),
		btg3(btg.x, btg.y, btg.z);

		// assumes w 1 or -1
		return tng3.Cross(btg3) * tng.w;
	}

	//! Get normal vector (perpendicular to tangent and bitangent plane).
	void GetN(Vec3& othern) const
	{
		othern = GetN();
	}

	//! Get the tangent-space basis as individual normal vectors (tangent, bitangent and normal).
	void GetTBN(Vec3& othert, Vec3& otherb, Vec3& othern) const
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z),
		btg3(btg.x, btg.y, btg.z);

		// assumes w 1 or -1
		othert = tng3;
		otherb = btg3;
		othern = tng3.Cross(btg3) * tng.w;
	}

	//! Get normal vector sign (reflection).
	ILINE i16 GetR() const
	{
		return PackingSNorm::tPackB2S(Tangent.w);
	}

	//! Get normal vector sign (reflection).
	void GetR(i16& sign) const
	{
		sign = GetR();
	}

	void TransformBy(const Matrix34& trn)
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z),
		btg3(btg.x, btg.y, btg.z);

		tng3 = trn.TransformVector(tng3);
		btg3 = trn.TransformVector(btg3);

		*this = SPipTangents(tng3, btg3, PackingSNorm::tPackB2S(Tangent.w));
	}

	void TransformSafelyBy(const Matrix34& trn)
	{
		Vec4 tng, btg;
		GetTB(tng, btg);

		Vec3 tng3(tng.x, tng.y, tng.z),
		btg3(btg.x, btg.y, btg.z);

		tng3 = trn.TransformVector(tng3);
		btg3 = trn.TransformVector(btg3);

		// normalize in case "trn" wasn't length-preserving
		tng3.Normalize();
		btg3.Normalize();

		*this = SPipTangents(tng3, btg3, PackingSNorm::tPackB2S(Tangent.w));
	}

	friend struct SMeshTangents;

	AUTO_STRUCT_INFO;
};

struct SPipQTangents
{
	SPipQTangents() {}

private:
	Vec4sf QTangent;

public:
	explicit SPipQTangents(const Vec4sf& other)
	{
		QTangent = other;
	}

	bool operator==(const SPipQTangents& other) const
	{
		return
		  QTangent[0] == other.QTangent[0] ||
		  QTangent[1] == other.QTangent[1] ||
		  QTangent[2] == other.QTangent[2] ||
		  QTangent[3] == other.QTangent[3];
	}

	bool operator!=(const SPipQTangents& other) const
	{
		return !(*this == other);
	}

	//! Get quaternion.
	ILINE Quat GetQ() const
	{
		Quat q;

		q.v.x = PackingSNorm::tPackB2F(QTangent.x);
		q.v.y = PackingSNorm::tPackB2F(QTangent.y);
		q.v.z = PackingSNorm::tPackB2F(QTangent.z);
		q.w = PackingSNorm::tPackB2F(QTangent.w);

		return q;
	}

	//! Get normal vector from quaternion.
	ILINE Vec3 GetN() const
	{
		const Quat q = GetQ();

		return q.GetColumn2() * (q.w < 0.0f ? -1.0f : +1.0f);
	}

	//! Get the tangent-space basis as individual normal vectors (tangent, bitangent and normal).
	void GetTBN(Vec3& othert, Vec3& otherb, Vec3& othern) const
	{
		const Quat q = GetQ();

		othert = q.GetColumn0();
		otherb = q.GetColumn1();
		othern = q.GetColumn2() * (q.w < 0.0f ? -1.0f : +1.0f);
	}

	friend struct SMeshQTangents;
};

struct SPipNormal : public Vec3
{
	SPipNormal() {}

	explicit SPipNormal(const Vec3& othern)
	{
		x = othern.x;
		y = othern.y;
		z = othern.z;
	}

	//! Get normal vector.
	ILINE Vec3 GetN() const
	{
		return *this;
	}

	//! Get normal vector.
	void GetN(Vec3& othern) const
	{
		othern = GetN();
	}

	void TransformBy(const Matrix34& trn)
	{
		*this = SPipNormal(trn.TransformVector(*this));
	}

	void TransformSafelyBy(const Matrix34& trn)
	{
		// normalize in case "trn" wasn't length-preserving
		*this = SPipNormal(trn.TransformVector(*this).normalize());
	}

	friend struct SMeshNormal;
};

//==================================================================================================

typedef SVF_P3F_C4B_T2F SAuxVertex;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Sizes
//extern i32k m_VertexSize[];

// we don't care about truncation of the struct member offset, because
// it's a very small integer (even fits into a signed byte)
//#pragma warning(push)
//#pragma warning(disable:4311)

//============================================================================
// Custom vertex streams definitions
// NOTE: If you add new stream ID also include vertex declarations creating in
//       CD3D9Renderer::EF_InitD3DVertexDeclarations (D3DRendPipeline.cpp)

//! Stream IDs.
enum EStreamIDs
{
	VSF_GENERAL,                 //!< General vertex buffer.
	VSF_TANGENTS,                //!< Tangents buffer.
	VSF_QTANGENTS,               //!< Tangents buffer.
	VSF_HWSKIN_INFO,             //!< HW skinning buffer.
	VSF_VERTEX_VELOCITY,         //!< Velocity buffer.
#if ENABLE_NORMALSTREAM_SUPPORT
	VSF_NORMALS,                 //!< Normals, used for skinning.
#endif
	//   <- Insert new stream IDs here.
	VSF_NUM,                     //!< Number of vertex streams.

	VSF_MORPHBUDDY         = 8,  //!< Morphing (from m_pMorphBuddy).
	VSF_INSTANCED          = 9,  //!< Data is for instance stream.
	VSF_MORPHBUDDY_WEIGHTS = 15, //!< Morphing weights.
};

//! Stream Masks (Used during updating).
enum EStreamMasks
{
	VSM_GENERAL         = BIT(VSF_GENERAL),
	VSM_TANGENTS        = BIT(VSF_TANGENTS) | BIT( VSF_QTANGENTS),
	VSM_HWSKIN          = BIT(VSF_HWSKIN_INFO),
	VSM_VERTEX_VELOCITY = BIT(VSF_VERTEX_VELOCITY),
#if ENABLE_NORMALSTREAM_SUPPORT
	VSM_NORMALS         = BIT(VSF_NORMALS),
#endif

	VSM_MORPHBUDDY      = BIT(VSF_MORPHBUDDY),
	VSM_INSTANCED       = BIT(VSF_INSTANCED),

	VSM_MASK            = MASK(VSF_NUM),
};

//==================================================================================================================

//#pragma warning(pop)
//! \endcond