// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Move serialization utils to separate header?

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>

#include <drx3D/Schema/TypeDesc.h>

namespace DrxTransform
{

//! Specialized class to represent an angle, used to avoid converting back and forth between radians and degrees
//! The type stores the angle in radians internally, since this is what the majority of engine systems need
class CAngle
{
public:
	constexpr CAngle() : m_value(0.f) {}

	constexpr static CAngle FromDegrees(float value)      { return CAngle(DEG2RAD(value)); }
	constexpr static CAngle FromRadians(float value)      { return CAngle(value); }

	float&                  GetUnderlyingValueAsRadians() { return m_value; }

	constexpr float         ToDegrees() const             { return RAD2DEG(m_value); }
	constexpr float         ToRadians() const             { return m_value; }
	constexpr CAngle        Absolute() const              { return CAngle(m_value < 0 ? -m_value : m_value); }

	constexpr bool          operator<(CAngle rhs) const   { return m_value < rhs.m_value; }
	constexpr bool          operator>(CAngle rhs) const   { return m_value > rhs.m_value; }
	constexpr bool          operator<=(CAngle rhs) const  { return m_value <= rhs.m_value; }
	constexpr bool          operator>=(CAngle rhs) const  { return m_value >= rhs.m_value; }
	constexpr bool          operator==(CAngle rhs) const  { return m_value == rhs.m_value; }
	constexpr bool          operator!=(CAngle rhs) const  { return m_value != rhs.m_value; }
	constexpr CAngle        operator-(CAngle rhs) const   { return CAngle(m_value - rhs.m_value); }
	constexpr CAngle        operator+(CAngle rhs) const   { return CAngle(m_value + rhs.m_value); }
	constexpr CAngle        operator*(float rhs) const    { return CAngle(m_value * rhs); }
	constexpr CAngle        operator/(float rhs) const    { return CAngle(m_value / rhs); }
	constexpr CAngle        operator-() const             { return CAngle(-m_value); }

	// These should be constexpr when we migrate to C++14
	CAngle&     operator-=(CAngle rhs) { m_value -= rhs.m_value; return *this; }
	CAngle&     operator+=(CAngle rhs) { m_value += rhs.m_value; return *this; }
	CAngle&     operator*=(float rhs)  { m_value *= rhs; return *this; }
	CAngle&     operator/=(float rhs)  { m_value /= rhs; return *this; }

	static void ReflectType(sxema::CTypeDesc<CAngle>& desc)
	{
		desc.SetGUID("{81CA6E46-8FA7-4C07-AC51-411EEB8BD1FE}"_drx_guid);
		desc.SetLabel("Angle");
		desc.SetDescription("Used to simplify conversion between radians and degrees");
	}

protected:
	explicit constexpr CAngle(float value) : m_value(value) {}

	float m_value;
};

inline bool Serialize(Serialization::IArchive& archive, CAngle& value, tukk szName, tukk szLabel)
{
	return archive(Serialization::RadiansAsDeg(value.GetUnderlyingValueAsRadians()), szName, szLabel);
}

constexpr CAngle operator"" _radians(zu64 value)
{
	return CAngle::FromRadians((float)value);
}

constexpr CAngle operator"" _degrees(zu64 value)
{
	return CAngle::FromDegrees((float)value);
}

constexpr CAngle operator"" _radians(long double value)
{
	return CAngle::FromRadians((float)value);
}

constexpr CAngle operator"" _degrees(long double value)
{
	return CAngle::FromDegrees((float)value);
}

class CAngles3
{
public:
	constexpr CAngles3() : x(0_radians), y(0_radians), z(0_radians) {}

	constexpr CAngles3(CAngle _x, CAngle _y, CAngle _z)
		: x(_x)
		, y(_y)
		, z(_z) {}

	CAngles3(Quat current, Quat previous)
	{
		Quat delta = current / previous;
		Ang3 angles = Ang3(delta);

		x = CAngle::FromRadians(angles.x);
		y = CAngle::FromRadians(angles.y);
		z = CAngle::FromRadians(angles.z);
	}

	void Multiply(Vec3 vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
	}

	constexpr bool operator==(const CAngles3& rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}

	constexpr bool operator!=(const CAngles3& rhs) const
	{
		return !(*this == rhs);
	}

	constexpr CAngles3 operator*(float rhs) const { return CAngles3(x * rhs, y * rhs, z * rhs); }
	constexpr CAngles3 operator-() const          { return CAngles3(-x, -y, -z); }

	CAngles3&          operator*=(float rhs)      { x *= rhs; y *= rhs; z *= rhs; return *this; }

	Ang3               ToAng3() const             { return Ang3(x.ToRadians(), y.ToRadians(), z.ToRadians()); }

	// Converts the angles to degree
	inline Vec3 ToDegrees() const
	{
		return Vec3(x.ToDegrees(), y.ToDegrees(), z.ToDegrees());
	}

	inline void Serialize(Serialization::IArchive& archive)
	{
		archive(x, "x", "x");
		archive(y, "y", "y");
		archive(z, "z", "z");
	}

	static void ReflectType(sxema::CTypeDesc<CAngles3>& desc)
	{
		desc.SetGUID("{F56078B5-E0D2-4FFA-981B-00B8E317DBFE}"_drx_guid);
		desc.SetLabel("3D Angle");
		desc.SetDescription("Angle in 3 axes");
	}

	CAngle x, y, z;
};

template<i32 TMinDegrees = -360, i32 TMaxDegrees = 360>
struct CClampedAngle : public CAngle
{
	constexpr CClampedAngle() = default;

	// Implicit
	constexpr CClampedAngle(CAngle angle)
		: CAngle(ClampedAngle(angle.ToRadians()))
	{
	}

	CClampedAngle& operator=(CAngle other)  { m_value = other.ToRadians(); Clamp(); return *this; }
	CClampedAngle& operator-=(CAngle rhs)   { m_value -= rhs.ToRadians(); Clamp(); return *this; }
	CClampedAngle& operator+=(CAngle rhs)   { m_value += rhs.ToRadians(); Clamp(); return *this; }
	CClampedAngle& operator*=(float factor) { m_value *= factor; Clamp(); return *this; }
	CClampedAngle& operator/=(float factor) { m_value /= factor; Clamp(); return *this; }

protected:
	inline void Clamp()
	{
		m_value = ClampedAngle(m_value);
	}

	constexpr static float ClampedAngle(float value)
	{
		return clamp_tpl(value, DEG2RAD(TMinDegrees), DEG2RAD(TMaxDegrees));
	}
};

template<i32 TMinDegrees, i32 TMaxDegrees>
inline void ReflectType(sxema::CTypeDesc<CClampedAngle<TMinDegrees, TMaxDegrees>>& desc)
{
	desc.SetGUID("{CCB36A48-8D5B-4E0D-8213-506923CD6E60}"_drx_guid);
	desc.SetLabel("Angle");
}

template<i32 TMinDegrees, i32 TMaxDegrees>
inline bool Serialize(Serialization::IArchive& archive, CClampedAngle<TMinDegrees, TMaxDegrees>& value, tukk szName, tukk szLabel)
{
	if (archive(Serialization::RadiansWithRangeAsDeg(value.GetUnderlyingValueAsRadians(), DEG2RAD((float)TMinDegrees), DEG2RAD((float)TMaxDegrees)), szName, szLabel))
	{
		return true;
	}

	return false;
}

} // DrxTransform

using DrxTransform::operator"" _radians;
using DrxTransform::operator"" _degrees;
