/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <stdarg.h>

#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/yasli/Helpers.h>
#include <drx3D/CoreX/Serialization/yasli/Serializer.h>
#include <drx3D/CoreX/Serialization/yasli/KeyValue.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>

#define YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(type) template <> struct IsDefaultSerializaeble<type> { static const bool value = true; };

namespace yasli{

class Archive;
struct CallbackInterface;

class Object;
class KeyValueInterface;
class EnumDescription;
template <class Enum>
EnumDescription& getEnumDescription();
bool serializeEnum(const EnumDescription& desc, Archive& ar, i32& value, tukk name, tukk label);
template<class T> constexpr bool HasSerializeOverride();

// Context is used to pass arguments to nested serialization functions.
// Example of usage:
//
// void EntityLibrary::serialize(yasli::Archive& ar)
// {
//   yasli::Context libraryContext(ar, this);
//   ar(entities, "entities");
// }
//
// void Entity::serialize(yasli::Archive& ar)
// {
//   if (EntityLibrary* library = ar.context<EntityLibrary>()) {
//     ...
//   }
// }
//
// You may have multiple contexts of different types, but note that the context
// lookup complexity is O(n) in respect to number of contexts.
struct Context {
	uk object;
	TypeID type;
	Context* previousContext;
	Archive* archive;

	Context() :  object(0), previousContext(0), archive(0) {}
	template<class T>
	void set(T* object);
	template<class T>
	Context(Archive& ar, T* context);
	template<class T>
	Context(T* context);
	~Context();
};

struct BlackBox;
class Archive{
public:
	enum ArchiveCaps{
		INPUT          = 1 << 0,
		OUTPUT         = 1 << 1,
		TEXT           = 1 << 2,
		BINARY         = 1 << 3,
		EDIT           = 1 << 4,
		INPLACE        = 1 << 5,
		NO_EMPTY_NAMES = 1 << 6,
		VALIDATION     = 1 << 7,
		DOCUMENTATION  = 1 << 8,
		XML_VERSION_1  = 1 << 9,
		CUSTOM1        = 1 << 10,
		CUSTOM2        = 1 << 11,
		CUSTOM3        = 1 << 12
	};

	Archive(i32 caps)
	: caps_(caps)
	, filter_(YASLI_DEFAULT_FILTER)
	, modifiedRow_(nullptr)
	, lastContext_(0)
	{
	}
	virtual ~Archive() {}

	bool isInput() const{ return caps_ & INPUT ? true : false; }
	bool isOutput() const{ return caps_ & OUTPUT ? true : false; }
	bool isEdit() const{
#if YASLI_NO_EDITING
		return false;
#else
		return (caps_ & EDIT)  != 0;
#endif
	}
	bool isInPlace() const { return (caps_ & INPLACE) != 0; }
	bool isValidation() const { return (caps_ & VALIDATION) != 0; }
	bool caps(i32 caps) const { return (caps_ & caps) == caps; }

	void setFilter(i32 filter){
		filter_ = filter;
	}
	i32 getFilter() const{ return filter_; }
	bool filter(i32 flags) const{
		YASLI_ASSERT(flags != 0 && "flags is supposed to be a bit mask");
		YASLI_ASSERT(filter_ && "Filter is not set!");
		return (filter_ & flags) != 0;
	}

	tukk getModifiedRowName() const { return modifiedRow_ ? modifiedRow_ : ""; }
	void setModifiedRowName(tukk modifiedRow) { modifiedRow_ = modifiedRow; }

	virtual bool operator()(bool& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(char& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(u8& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(i8& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(i16& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(u16& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(i32& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(u32& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(i64& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(u64& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(float& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(double& value, tukk name = "", tukk label = 0) { notImplemented(); return false; }

	virtual bool operator()(StringInterface& value, tukk name = "", tukk label = 0)    { notImplemented(); return false; }
	virtual bool operator()(WStringInterface& value, tukk name = "", tukk label = 0)    { notImplemented(); return false; }
	virtual bool operator()(const Serializer& ser, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(BlackBox& ser, tukk name = "", tukk label = 0) { notImplemented(); return false; }
	virtual bool operator()(ContainerInterface& ser, tukk name = "", tukk label = 0) { return false; }
	virtual bool operator()(PointerInterface& ptr, tukk name = "", tukk label = 0);
	virtual bool operator()(Object& obj, tukk name = "", tukk label = 0) { return false; }
	virtual bool operator()(KeyValueInterface& keyValue, tukk name = "", tukk label = 0) { return operator()(Serializer(keyValue), name, label); }
	virtual bool operator()(CallbackInterface& callback, tukk name = "", tukk label = 0) { return false; }

	// No point in supporting long double since it is represented as double on MSVC
	bool operator()(long double& value, tukk name = "", tukk label = 0)         { notImplemented(); return false; }

	template<class T>
	bool operator()(const T& value, tukk name = "", tukk label = 0);

	// Error and Warning calls are used for diagnostics and validation of the
	// values. Output depends on the specific implementation of Archive,
	// for example PropertyTree uses it to show bubbles with errors in UI
	// next to the mentioned property.
	template<class T> void error(const T& value, tukk format, ...);
	template<class T> void warning(const T& value, tukk format, ...);

	void error(ukk value, const yasli::TypeID& type, tukk format, ...);
	// Used to add tooltips in PropertyTree
	void doc(tukk docString);

	// block call are osbolete, please do not use
	virtual bool openBlock(tukk name, tukk label,tukk icon=0) { return true; }
	virtual void closeBlock() {}

	template<class T>
	T* context() const {
		return (T*)contextByType(TypeID::get<T>());
	}

	uk contextByType(const TypeID& type) const {
		for (Context* current = lastContext_; current != 0; current = current->previousContext)
			if (current->type == type)
				return current->object;
		return 0;
	}

	Context* setLastContext(Context* context) {
		Context* previousContext = lastContext_;
		lastContext_ = context;
		return previousContext;
	}
	Context* lastContext() const{ return lastContext_; }
protected:
	virtual void validatorMessage(bool error, ukk handle, const TypeID& type, tukk message) {}
	virtual void documentLastField(tukk text) {}

	void notImplemented() { YASLI_ASSERT(0 && "Not implemented!"); }

	i32 caps_;
	i32 filter_;
	tukk modifiedRow_;
	Context* lastContext_;
};

namespace Helpers{

template<class T>
struct SerializeStruct{
	static bool invoke(Archive& ar, T& value, tukk name, tukk label){
		Serializer serializer(value);
		return ar(serializer, name, label);
	};
};

//Enum classes may define an enum type that is not sizeof(i32), therefore reinterpret_cast is dangerous and leads to bugs.
template<class Enum, i32 size = sizeof(Enum)>
struct SerializeEnum{
	static bool invoke(Archive& ar, Enum& value, tukk name, tukk label) {
		static_assert(size < sizeof(i32), "Enum of integer size should use specialized template, bigger than i32 should not be serialized");
		const EnumDescription& enumDescription = getEnumDescription<Enum>();
		i32 valueHolder = (i32)value;
		bool ret = serializeEnum(enumDescription, ar, valueHolder, name, label);
		value = (Enum)valueHolder;
		return ret;
	};
};

template<class Enum>
struct SerializeEnum<Enum, sizeof(i32)>{
	static bool invoke(Archive& ar, Enum& value, tukk name, tukk label) {
		const EnumDescription& enumDescription = getEnumDescription<Enum>();
		return serializeEnum(enumDescription, ar, reinterpret_cast<i32&>(value), name, label);
	};
};

template<class T>
struct SerializeArray{};

template<class T, i32 Size>
struct SerializeArray<T[Size]>{
	static bool invoke(Archive& ar, T value[Size], tukk name, tukk label){
		ContainerArray<T, Size> ser(value);
		return ar(static_cast<ContainerInterface&>(ser), name, label);
	}
};

}

template<class T>
Context::Context(Archive& ar, T* context) {
	archive = &ar;
	object = (uk )context;
	type = TypeID::get<T>();
	if (archive)
		previousContext = ar.setLastContext(this);
	else
		previousContext = 0;
}

template<class T>
Context::Context(T* context) {
    archive = 0;
    previousContext = 0;
    set<T>(context);
}

template<class T>
void Context::set(T* object) {
	this->object = (uk )object;
	type = TypeID::get<T>();
}

inline Context::~Context() {
	if (archive)
		archive->setLastContext(previousContext);
}

inline void Archive::doc(tukk docString)
{
#if !YASLI_NO_EDITING
	if (caps_ & DOCUMENTATION)
		documentLastField(docString); 
#endif
}

template<class T>
void Archive::error(const T& value, tukk format, ...)
{
#if !YASLI_NO_EDITING
	if ((caps_ & VALIDATION) == 0)
		return;
	va_list args;
	va_start(args, format);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	validatorMessage(true, &value, TypeID::get<T>(), buf);
#endif
}

inline void Archive::error(ukk handle, const yasli::TypeID& type, tukk format, ...)
{
#if !YASLI_NO_EDITING
	if ((caps_ & VALIDATION) == 0)
		return;
	va_list args;
	va_start(args, format);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	validatorMessage(true, handle, type, buf);
#endif
}

template<class T>
void Archive::warning(const T& value, tukk format, ...)
{
#if !YASLI_NO_EDITING
	if ((caps_ & VALIDATION) == 0)
		return;
	va_list args;
	va_start(args, format);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	validatorMessage(false, &value, TypeID::get<T>(), buf);
#endif
}

template<class T>
bool Archive::operator()(const T& value, tukk name, tukk label) {
	static_assert(HasSerializeOverride<T>(), "Type has no serialize method/override!");
	return YASLI_SERIALIZE_OVERRIDE(*this, const_cast<T&>(value), name, label);
}

inline bool Archive::operator()(PointerInterface& ptr, tukk name, tukk label)
{
	Serializer ser(ptr);
	return operator()(ser, name, label);
}

template<class T, i32 Size>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, T object[Size], tukk name, tukk label)
{
	YASLI_ASSERT(0);
	return false;
}

// Archives are using yasli integer typedefs (taken from stdint.h). These
// types will be selected directly by Archive::operator() overloads. Those,
// however, do not cover all C++ integer types, as compilers usually have
// multiple integer types of the same type (like i32 and long in MSVC/x86). We
// would like to handle handle all C++ standard types, but not add duplicated
// and unportable methods to each archive implementations. The remaining types
// are cast to types of the same in functions below. E.g. long will be cast to
// i32 on MSVC/x86 and long long will be cast to long on GCC/x64.
template<class T, class CastTo>
struct CastInteger
{
	static bool invoke(Archive& ar, T& value, tukk name, tukk label)
	{
		return ar(reinterpret_cast<CastTo&>(value), name, label);
	}
};

template<class T>
bool castInteger(Archive& ar, T& v, tukk name, tukk label)
{
	using namespace Helpers;
	return 
		Select< IsSigned<T>,
			SelectIntSize< sizeof(T),
				CastInteger<T, i8>,
				CastInteger<T, i16>,
				CastInteger<T, i32>,
				CastInteger<T, i64>
			>,
			SelectIntSize< sizeof(T),
				CastInteger<T, u8>,
				CastInteger<T, u16>,
				CastInteger<T, u32>,
				CastInteger<T, u64>
			>
		>::type::invoke(ar, v, name, label);
}
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, u8& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, unsigned short& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, u32& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, u64& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, zu64& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed char& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed short& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, i32& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, i64& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, z64& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
#ifdef _NATIVE_WCHAR_T_DEFINED // MSVC
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, wchar_t& v, tukk name, tukk label) { return castInteger(ar, v, name, label); }
#endif

template<class T>
inline auto YASLI_SERIALIZE_OVERRIDE(Archive& ar, T& object, tukk name, tukk label)->decltype(std::declval<T&>().YASLI_SERIALIZE_METHOD(std::declval<Archive&>()), bool())
{
	return Helpers::SerializeStruct<T>::invoke(ar, object, name, label);
}

template<class T>
inline typename std::enable_if<std::is_enum<T>::value, bool>::type YASLI_SERIALIZE_OVERRIDE(Archive& ar, T& object, tukk name, tukk label)
{
	return Helpers::SerializeEnum<T>::invoke(ar, object, name, label);
}

template<class T>
inline typename std::enable_if<std::is_array<T>::value, bool>::type YASLI_SERIALIZE_OVERRIDE(Archive& ar, T& object, tukk name, tukk label)
{
	return Helpers::SerializeArray<T>::invoke(ar, object, name, label);
}

inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, Serializer& object, tukk name, tukk label)
{
	return Helpers::SerializeStruct<Serializer>::invoke(ar, object, name, label);
}

namespace Helpers {

template <typename T> struct IsDefaultSerializaeble { static const bool value = false; };

template<class T>
constexpr auto HasSerializeOverride(i32)->decltype(YASLI_SERIALIZE_OVERRIDE(std::declval<Archive&>(), std::declval<T&>(), std::declval<tukk >(), std::declval<tukk >()), bool())
{
	return true;
}

template<class T>
constexpr bool HasSerializeOverride(...)
{
	return false;
}

// N.B. List of default serializeable types must be in sync with Archive interface.
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(bool)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(char)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(u8)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(i8)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(i16)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(u16)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(i32)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(u32)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(i64)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(u64)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(float)
YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE(double)

}


template<class T>
constexpr bool IsDefaultSerializeable()
{
	return Helpers::IsDefaultSerializaeble<T>::value;
}

template<class T>
constexpr bool HasSerializeOverride()
{
	return Helpers::HasSerializeOverride<T>(0);
}

template<class T>
constexpr bool IsSerializeable()
{
	return IsDefaultSerializeable<T>() || HasSerializeOverride<T>();
}

}

#undef YASLI_HELPERS_DECLARE_DEFAULT_SERIALIZEABLE_TYPE

#include <drx3D/CoreX/Serialization/yasli/SerializerImpl.h>

// vim: ts=4 sw=4:
