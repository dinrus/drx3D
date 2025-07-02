// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <map>
#include <vector>

#include <drx3D/CoreX/Serialization/yasli/ClassFactory.h>
#include <drx3D/CoreX/Serialization/yasli/TypeID.h>
#include <drx3D/CoreX/Serialization/IClassFactory.h>

namespace Serialization
{
using yasli::ClassFactory;
using yasli::TypeDescription;
}

#define SERIALIZATION_CLASS_NULL(BaseType, name)                                                           YASLI_CLASS_NULL(BaseType, name)
#define SERIALIZATION_CLASS_NAME(BaseType, Type, name, label)                                              YASLI_CLASS_NAME(BaseType, Type, name, label)
#define SERIALIZATION_CLASS_NAME_FOR_FACTORY(Factory, BaseType, Type, name, label)                         YASLI_CLASS_NAME_FOR_FACTORY(Factory, BaseType, Type, name, label)
#define SERIALIZATION_CLASS_ANNOTATION(BaseType, Type, attributeName, attributeValue)                      YASLI_CLASS_ANNOTATION(BaseType, Type, attributeName, attributeValue)
#define SERIALIZATION_CLASS_ANNOTATION_FOR_FACTORY(factory, BaseType, Type, attributeName, attributeValue) YASLI_CLASS_ANNOTATION_FOR_FACTORY(factory, BaseType, Type, attributeName, attributeValue)
