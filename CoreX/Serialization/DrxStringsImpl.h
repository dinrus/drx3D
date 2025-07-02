// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/DrxStrings.h>

namespace Serialization
{
template<class TFixedStringClass>
class CFixedStringSerializer : public IString
{
public:
	CFixedStringSerializer(TFixedStringClass& str) : str_(str) {}

	void        set(tukk value) { str_ = value; }
	tukk get() const            { return str_.c_str(); }
	ukk handle() const         { return &str_; }
	TypeID      type() const           { return TypeID::get<TFixedStringClass>(); }
private:
	TFixedStringClass& str_;
};

template<class TFixedStringClass>
class CFixedWStringSerializer : public IWString
{
public:
	CFixedWStringSerializer(TFixedStringClass& str) : str_(str) {}

	void           set(const wchar_t* value) { str_ = value; }
	const wchar_t* get() const               { return str_.c_str(); }
	ukk    handle() const            { return &str_; }
	TypeID         type() const              { return TypeID::get<TFixedStringClass>(); }
private:
	TFixedStringClass& str_;
};
}

template<size_t N>
inline bool Serialize(Serialization::IArchive& ar, DrxFixedStringT<N>& value, tukk name, tukk label)
{
	Serialization::CFixedStringSerializer<DrxFixedStringT<N>> str(value);
	return ar(static_cast<Serialization::IString&>(str), name, label);
}

template<size_t N>
inline bool Serialize(Serialization::IArchive& ar, DrxStackStringT<char, N>& value, tukk name, tukk label)
{
	Serialization::CFixedStringSerializer<DrxStackStringT<char, N>> str(value);
	return ar(static_cast<Serialization::IString&>(str), name, label);
}

template<size_t N>
inline bool Serialize(Serialization::IArchive& ar, DrxStackStringT<wchar_t, N>& value, tukk name, tukk label)
{
	Serialization::CFixedWStringSerializer<DrxStackStringT<wchar_t, N>> str(value);
	return ar(static_cast<Serialization::IWString&>(str), name, label);
}
