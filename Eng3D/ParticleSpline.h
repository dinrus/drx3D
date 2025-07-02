// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     04/03/2015 by Filipe amim
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Eng3D/ParticleMath.h>
#include <drx3D/CoreX/Math/ISplines.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Math/Range.h>

namespace pfx2
{

class CParticleSpline : public ISplineEvaluator
{
public:
	CParticleSpline();

	void         MakeFlatLine(float v) { return MakeLinear(v, v); }
	void         MakeLinear(float v0 = 0.0f, float v1 = 1.0f);

	size_t       GetNumKeys() const { return m_keys.size(); }
	bool         HasKeys() const    { return GetNumKeys() > 0; }

	float        Interpolate(float time) const;
#ifdef DRX_PFX2_USE_SSE
	floatv       Interpolate(const floatv time) const;
#endif
	const Range& GetValueRange() const { return m_valueRange; }

	// ISplineEvaluator
	virtual i32        GetKeyCount() const override { return GetNumKeys(); }
	virtual void       GetKey(i32 i, KeyType& key) const override;
	virtual void       FromSpline(const ISplineEvaluator& spline) override;

	virtual void       Interpolate(float time, ValueType& value) override;

	static  Formatting StringFormatting()             { return Formatting(";,:"); }
	virtual Formatting GetFormatting() const override { return StringFormatting(); }

	virtual bool       FromString(cstr str) override;
	virtual bool       Serialize(Serialization::IArchive& ar, tukk name, tukk label) override;

	struct CSourceSpline : public spline::CSplineKeyInterpolator<spline::ClampedKey<float>>
	{
		typedef spline::ClampedKey<float> key_type;
		virtual Formatting GetFormatting() const override { return CParticleSpline::StringFormatting(); }
	};

private:
	void  Resize(size_t size);
	float DefaultSlope(size_t keyIdx) const;
	float StartSlope(size_t keyIdx) const;
	float EndSlope(size_t keyIdx) const;

	struct SplineKey
	{
		float time;
		float timeMult;
		float value;
		float coeff0;
		float coeff1;
	};

	std::vector<SplineKey>     m_keys;
	std::vector<spline::Flags> m_keyFlags;
	Range                      m_valueRange;
};

// Automatically creates a global Serialize() for Type, which invokes Type.Serialize
// This serializes Type is a single node, rather than the default for structs,
// which creates a node with nested members.
#define SERIALIZATION_WITH_MEMBER_FUNCTION(Type)                                                       \
  inline bool Serialize(Serialization::IArchive & ar, Type & val, tukk name, tukk label) \
  { return val.Serialize(ar, name, label); }                                                           \


SERIALIZATION_WITH_MEMBER_FUNCTION(CParticleSpline)

template<i32 DIMS>
class CParticleMultiSpline : public spline::CMultiSplineEvaluator<CParticleSpline, DIMS>
{
public:
	bool HasKeys() const
	{
		for (const auto& spline : m_splines)
			if (spline.HasKeys())
				return true;
		return false;
	}
	Range GetValueRange() const
	{
		Range range(Range::EMPTY);
		for (const auto& spline : m_splines)
			range |= spline.GetValueRange();
		return range;
	}
	float GetMaxValue() const { return GetValueRange().start; }
	float GetMinValue() const { return GetValueRange().end; }

	bool  Serialize(Serialization::IArchive& ar, tukk name, tukk label) override
	{
		if (ar.isEdit())
		{
			return ar(static_cast<IMultiSplineEvaluator&>(*this), name, label);
		}
		else
		{
			return ar(m_splines, name, label);
		}
	}

protected:
	using spline::CMultiSplineEvaluator<CParticleSpline, DIMS>::m_splines;        // Help the compiler understand what it already should
};

template<i32 DIMS> SERIALIZATION_WITH_MEMBER_FUNCTION(CParticleMultiSpline<DIMS> )

class CParticleDoubleSpline : public CParticleMultiSpline<2>
{
public:
	ILINE float  Interpolate(float time, float unormRand) const;
#ifdef DRX_PFX2_USE_SSE
	ILINE floatv Interpolate(floatv time, floatv unormRand) const;
#endif
};

SERIALIZATION_WITH_MEMBER_FUNCTION(CParticleDoubleSpline)

class CParticleColorSpline : public CParticleMultiSpline<3>
{
public:
	ILINE ColorF  Interpolate(float time) const;
#ifdef DRX_PFX2_USE_SSE
	ILINE ColorFv Interpolate(floatv time) const;
#endif
};

SERIALIZATION_WITH_MEMBER_FUNCTION(CParticleColorSpline)

}

#include "ParticleSplineImpl.h"

