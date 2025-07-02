/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <map>

#include <drx3D/CoreX/Serialization/yasli/Assert.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>
#include <drx3D/CoreX/Serialization/yasli/Config.h>

namespace yasli {

class TypeDescription{
public:
	TypeDescription(const TypeID& typeID, tukk name, tukk label)
	: name_(name)
	, label_(label)
	, typeID_(typeID)
	{
#if YASLI_NO_RTTI
#if 0
		const size_t bufLen = sizeof(typeID.typeInfo_->name);
		strncpy(typeID.typeInfo_->name, name, bufLen - 1);
		typeID.typeInfo_->name[bufLen - 1] = '\0';
#endif
#endif
	}
	tukk name() const{ return name_; }
	tukk label() const{ return label_; }
	TypeID typeID() const{ return typeID_; }

protected:
	tukk name_;
	tukk label_;
	TypeID typeID_;
};

class ClassFactoryBase{
public: 
	ClassFactoryBase(TypeID baseType)
	: baseType_(baseType)
	, nullLabel_(0)
	{
	}

	virtual ~ClassFactoryBase() {}

	virtual size_t size() const = 0;
	virtual const TypeDescription* descriptionByIndex(i32 index) const = 0;	
	virtual const TypeDescription* descriptionByRegisteredName(tukk typeName) const = 0;
	virtual tukk findAnnotation(tukk registeredTypeName, tukk annotationName) const = 0;
	virtual void serializeNewByIndex(Archive& ar, i32 index, tukk name, tukk label) = 0;

	bool setNullLabel(tukk label){ nullLabel_ = label ? label : ""; return true; }
	tukk nullLabel() const{ return nullLabel_; }
protected:
	TypeID baseType_;
	tukk nullLabel_;
};


struct TypeNameWithFactory
{
	string registeredName;
	ClassFactoryBase* factory;

	TypeNameWithFactory(tukk registeredName, ClassFactoryBase* factory = 0)
	: registeredName(registeredName)
	, factory(factory)
	{
	}
};

YASLI_INLINE bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::TypeNameWithFactory& value, tukk name, tukk label);

}

// vim:ts=4 sw=4:
