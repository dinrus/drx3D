// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Move serialization utils to separate header?

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Math.h>

#include <drx3D/CoreX/Math/Angle.h>

#include <drx3D/Schema/TypeDesc.h>

namespace sxema
{
	template<i32 TMin, i32 TMax, i32 TSoftMin = 0, i32 TSoftMax = 100, typename TType = float>
	struct Range
	{
		Range() : value(0) {}
		Range(TType val) : value(val) {}

		operator TType() const { return value; }
		operator TType&() { return value; }
		void operator=(const TType other) { value = other; }
		bool operator==(const Range &rhs) const { return value == rhs.value; }

		TType value;
	};
	template<i32 TMin, i32 TMax, i32 TSoftMin, i32 TSoftMax>
	inline void ReflectType(CTypeDesc<Range<TMin, TMax, TSoftMin, TSoftMax, float>>& desc)
	{
		desc.SetGUID("{A3A15703-B420-42F0-97B7-4957B20CE376}"_drx_guid);
		desc.SetLabel("Range");
	}
	template<i32 TMin, i32 TMax, i32 TSoftMin, i32 TSoftMax>
	inline void ReflectType(CTypeDesc<Range<TMin, TMax, TSoftMin, TSoftMax, i32>>& desc)
	{
		desc.SetGUID("{D6B663CD-94BD-4B26-B2DF-0AF244FD437C}"_drx_guid);
		desc.SetLabel("Range");
	}
	template<i32 TMin, i32 TMax, i32 TSoftMin, i32 TSoftMax, typename TType>
	inline bool Serialize(Serialization::IArchive& archive, Range<TMin, TMax, TSoftMin, TSoftMax, TType>& value, tukk szName, tukk szLabel)
	{
		return archive(Serialization::Range(value.value, (TType)TMin, (TType)TMax, (TType)TSoftMin, (TType)TSoftMax), szName, szLabel);
	}

	typedef Range<0, std::numeric_limits<i32>::max()> PositiveFloat;

	template<typename T>
	struct UnitLength
	{
		UnitLength() : value(ZERO) {}
		UnitLength(const T& val) : value(val) {}
		operator T() const { return value; }
		operator T&() { return value; }
		operator const T&() const { return value; }
		void operator=(const T& other) { value = other; }
		bool operator==(const UnitLength &rhs) const { return value == rhs.value; }

		T value;
	};
	inline void ReflectType(CTypeDesc<UnitLength<Vec3>>& desc)
	{
		desc.SetGUID("{C5539EB6-D206-46AF-B247-8A105DF071D6}"_drx_guid);
		desc.SetLabel("Unit-length three-dimensional vector");
	}
	inline bool Serialize(Serialization::IArchive& archive, UnitLength<Vec3>& value, tukk szName, tukk szLabel)
	{
		if (archive(value.value, szName, szLabel) && archive.isEdit())
		{
			value.value.Normalize();
			return true;
		}

		return false;
	}
} // sxema
