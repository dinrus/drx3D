/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include <drx3D/CoreX/Serialization/yasli/Assert.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>
#include <drx3D/CoreX/Serialization/yasli/Config.h>

namespace yasli{

class Archive;

typedef bool(*SerializeStructFunc)(uk , Archive&);

typedef bool(*SerializeContainerFunc)(uk , Archive&, size_t index);
typedef size_t(*ContainerResizeFunc)(uk , size_t size);
typedef size_t(*ContainerSizeFunc)(uk );

// Struct serializer. 
//
// This type is used to pass needed struct/class type information through abstract interface.
// Most importantly it captures:
// - pointer to object
// - reference to serialize method (indirectly through pointer to static func.)
// - TypeID
class Serializer{/*{{{*/
	friend class Archive;
public:
	Serializer()
	: object_(0)
	, size_(0)
	, serializeFunc_(0)
	{
	}

	Serializer(const TypeID& type, uk object, size_t size, SerializeStructFunc serialize)
	: type_(type)
	, object_(object)
	, size_(size)
	, serializeFunc_(serialize)
	{
		YASLI_ASSERT(object != 0);
	}

	Serializer(const Serializer& _original)
	: type_(_original.type_)
	, object_(_original.object_)
	, size_(_original.size_)
	, serializeFunc_(_original.serializeFunc_)
	{
	}

	template<class T>
	explicit Serializer(const T& object){
		YASLI_ASSERT(TypeID::get<T>() != TypeID::get<Serializer>());
		type_=  TypeID::get<T>();
		object_ = (uk )(&object);
		size_ = sizeof(T);
		serializeFunc_ = &Serializer::serializeRaw<T>;
	}

	template<class T>
	explicit Serializer(T& object, TypeID type){
		type_ =  type;
		object_ = (uk )(&object);
		size_ = sizeof(T);
		serializeFunc_ = &Serializer::serializeRaw<T>;
	}

	// This constructs Serializer from an object that doesn't have Serialize method. 
	// Such Serializer can not be serialized but conveys object reference and type 
	// information that is needed for Property-archives. Used for decorators.
	template<class T>
	static Serializer forEdit(const T& object)
	{
		Serializer r;
		r.type_ = TypeID::get<T>();
		r.object_ = (uk )&object;
		r.size_ = sizeof(T);
		r.serializeFunc_ = 0;
		return r;
	}

	template<class T>
	T* cast() const{ return type_ == TypeID::get<T>() ? (T*)object_ : 0; }
	bool operator()(Archive& ar) const;
	bool operator()(Archive& ar, tukk name, tukk label) const;
	operator bool() const{ return object_ && serializeFunc_; }
	bool operator==(const Serializer& rhs) { return object_ == rhs.object_ && serializeFunc_ == rhs.serializeFunc_; }
	bool operator!=(const Serializer& rhs) { return !operator==(rhs); }
	uk pointer() const{ return object_; }
	void setPointer(uk p) { object_ = p; }
	const TypeID& type() const{ return type_; }
	void setType(const TypeID& type) { type_ = type; }
	size_t size() const{ return size_; }
	SerializeStructFunc serializeFunc() const{ return serializeFunc_; }

	template<class T>
	static bool serializeRaw(uk rawPointer, Archive& ar){
		YASLI_ESCAPE(rawPointer, return false);
		// If you're getting compile error here, most likely, you have one of the following situations:
		// - The type you're trying to serialize doesn't have "serialize" _method_ implemented.
		// - Type is supposed to be serialized with non-member serialize override function and this function is out of scope.
		((T*)(rawPointer))->YASLI_SERIALIZE_METHOD(ar);
		return true;
	}

private:
	TypeID type_;
	uk object_;
	size_t size_;
	SerializeStructFunc serializeFunc_;
};/*}}}*/
typedef std::vector<Serializer> Serializers;

// ---------------------------------------------------------------------------

// This type is used to generalize access to specific container types.
// It is used by concrete Archive implementations.
class ContainerInterface{
public:
	virtual ~ContainerInterface() {}

	virtual size_t size() const = 0;
	virtual size_t resize(size_t size) = 0;

	virtual bool isFixedSize() const{ return false; }

	//! Returns true if a container is ordered (not sorted), i.e. if order of elements is relevant and can be influenced. Array is ordered, hash map is not.
	virtual bool isOrdered() const { return true; }

	virtual uk pointer() const = 0;
	virtual TypeID elementType() const = 0;
	virtual TypeID containerType() const = 0;

	//! Sets the internal iterator to the container's begin()
	virtual void begin() = 0;

	//! Advances the iterator, returns false when end() is reached.
	virtual bool next() = 0;

	virtual uk elementPointer() const = 0;

	//! Serialize the element pointed by the iterator currently
	virtual bool operator()(Archive& ar, tukk name, tukk label) = 0;

	//! removes the element currently pointed by the iterator, iterator will point to the next element
	virtual void remove() = 0;

	//! Inserts an element before the currently pointed element. Iterator is not advanced
	virtual void insert() = 0;

	//! Moves the current element to a new position in the container. All other elements are shifted by one. Iterator is unchanged.
	virtual void move(i32 index) = 0;

	//! Is container valid
	virtual operator bool() const = 0;

	//! Used to compare with default value
	virtual void serializeNewElement(Archive& ar, tukk name = "", tukk label = 0) const = 0;
};

template<class T, size_t Size>
class ContainerArray : public ContainerInterface{
	friend class Archive;
public:
	explicit ContainerArray(T* array = 0)
	: array_(array)
	, index_(0)
	{
	}

	// from ContainerInterface:
	size_t size() const override { return Size; }
	size_t resize(size_t size) override{
		index_ = 0;
		return Size;
	}

	uk pointer() const override { return reinterpret_cast<uk>(array_); }
	TypeID containerType() const override { return TypeID::get<T[Size]>(); }
	TypeID elementType() const override { return TypeID::get<T>(); }
	uk elementPointer() const override { return &array_[index_]; }
	virtual bool isFixedSize() const override { return true; }

	bool operator()(Archive& ar, tukk name, tukk label) override {
		YASLI_ESCAPE(size_t(index_) < Size, return false);
		return ar(array_[index_], name, label);
	}
	operator bool() const override { return array_ != 0; }

	void begin() override { index_ = 0; }
	bool next() override {
		++index_;
		return size_t(index_) < Size;
	}

	void remove() override {}
	void insert() override {}
	void move(i32 index) override 
	{
		auto distance = index_ - index;
		if (distance > 0)
		{
			auto element = array_[index_];
			std::copy_backward(&array_[index], &array_[index + distance], &array_[index_ + 1]);
			array_[index] = element;
		}
		else if (distance < 0)
		{
			auto element = array_[index_];
			std::copy(&array_[index_ + 1], &array_[index_ + 1 - distance], &array_[index_]);
			array_[index] = element;
		}
	}

	void serializeNewElement(Archive& ar, tukk name, tukk label) const override {
		T element;
		ar(element, name, label);
	}
	// ^^^

private:
	T* array_;
	i32 index_;
};

class ClassFactoryBase;
// Generialized interface over owning polymorphic pointers.
// Used by concrete Archive implementations.
class PointerInterface
{
public:
	virtual ~PointerInterface() {}

	virtual tukk registeredTypeName() const = 0;
	virtual void create(tukk registeredTypeName) const = 0;
	virtual TypeID baseType() const = 0;
	virtual Serializer serializer() const = 0;
	virtual uk get() const = 0;
	virtual ukk handle() const = 0;
	virtual TypeID pointerType() const = 0;
	virtual ClassFactoryBase* factory() const = 0;
	
	void YASLI_SERIALIZE_METHOD(Archive& ar) const;
};

class StringInterface
{
public:
	virtual ~StringInterface() {}

	virtual void set(tukk value) = 0;
	virtual tukk get() const = 0;
	virtual ukk handle() const = 0;
	virtual TypeID type() const = 0;
};
class WStringInterface
{
public:
	virtual ~WStringInterface() {}

	virtual void set(const wchar_t* value) = 0;
	virtual const wchar_t* get() const = 0;
	virtual ukk handle() const = 0;
	virtual TypeID type() const = 0;
};

struct TypeIDWithFactory;
bool serialize(Archive& ar, TypeIDWithFactory& value, tukk name, tukk label);

}
// vim:ts=4 sw=4:
