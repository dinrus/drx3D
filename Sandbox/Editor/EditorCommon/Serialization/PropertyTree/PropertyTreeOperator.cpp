/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

//#include "stdafx.h"
#include "PropertyTreeOperator.h"
#include "PropertyRow.h"
#include <drx3D/CoreX/Serialization/yasli/Enum.h>
#include <drx3D/CoreX/Serialization/yasli/STL.h>
#include <drx3D/CoreX/Serialization/yasli/Pointers.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/PointersImpl.h>

YASLI_ENUM_BEGIN_NESTED(PropertyTreeOperator, Type, "PropertyTreeOp")
YASLI_ENUM_VALUE_NESTED(PropertyTreeOperator, REPLACE, "Replace")
YASLI_ENUM_VALUE_NESTED(PropertyTreeOperator, ADD, "Add")
YASLI_ENUM_VALUE_NESTED(PropertyTreeOperator, REMOVE, "Remove")
YASLI_ENUM_END()

PropertyTreeOperator::PropertyTreeOperator(const TreePath& path, PropertyRow* row)
: type_(REPLACE)
, path_(path)
, index_(-1)
, row_(row)
{
}

PropertyTreeOperator::PropertyTreeOperator()
: type_(NONE)
, index_(-1)
{
}

PropertyTreeOperator::~PropertyTreeOperator()
{
}

void PropertyTreeOperator::YASLI_SERIALIZE_METHOD(yasli::Archive& ar)
{
    ar(type_, "type", "Type");
    ar(path_, "path", "Path");
    ar(row_, "row", "Row");
    ar(index_, "index", "Index");
}


