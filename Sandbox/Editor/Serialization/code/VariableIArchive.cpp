// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "VariableIArchive.h"
#include <drx3D/CoreX/Serialization/StringList.h>
#include <drx3D/CoreX/Serialization/Decorators/Resources.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>

#include "ICryMannequinDefs.h"

//#pragma optimize("", off)
//#pragma inline_depth(0)

using Serialization::CVariableIArchive;

namespace VarUtil
{
_smart_ptr<IVariable> FindChildVariable(const _smart_ptr<IVariable>& pParent, i32k childIndexOverride, tukk const name)
{
	if (0 <= childIndexOverride)
	{
		return pParent->GetVariable(childIndexOverride);
	}
	else
	{
		const bool shouldSearchRecursively = false;
		return pParent->FindVariable(name, shouldSearchRecursively);
	}
}

template<typename T, typename TOut>
bool ReadChildVariableAs(const _smart_ptr<IVariable>& pParent, i32k childIndexOverride, tukk const name, TOut& valueOut)
{
	_smart_ptr<IVariable> pVariable = FindChildVariable(pParent, childIndexOverride, name);
	if (pVariable)
	{
		T tmp;
		pVariable->Get(tmp);
		valueOut = static_cast<TOut>(tmp);
		return true;
	}
	return false;
}

template<typename T>
bool ReadChildVariable(const _smart_ptr<IVariable>& pParent, i32k childIndexOverride, tukk const name, T& valueOut)
{
	return ReadChildVariableAs<T>(pParent, childIndexOverride, name, valueOut);
}
}

CVariableIArchive::CVariableIArchive(const _smart_ptr<IVariable>& pVariable)
	: IArchive(IArchive::INPUT | IArchive::EDIT | IArchive::NO_EMPTY_NAMES)
	, m_pVariable(pVariable)
	, m_childIndexOverride(-1)
{
	DRX_ASSERT(m_pVariable);

	m_structHandlers[TypeID::get < Serialization::IResourceSelector > ().name()] = &CVariableIArchive::SerializeResourceSelector;
	m_structHandlers[TypeID::get < Serialization::RangeDecorator < float >> ().name()] = &CVariableIArchive::SerializeRangeFloat;
	m_structHandlers[TypeID::get < Serialization::RangeDecorator < i32 >> ().name()] = &CVariableIArchive::SerializeRangeInt;
	m_structHandlers[TypeID::get < Serialization::RangeDecorator < u32 >> ().name()] = &CVariableIArchive::SerializeRangeUInt;
	m_structHandlers[TypeID::get < StringListStaticValue > ().name()] = &CVariableIArchive::SerializeStringListStaticValue;
}

CVariableIArchive::~CVariableIArchive()
{
}

bool CVariableIArchive::operator()(bool& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<bool>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(Serialization::IString& value, tukk name, tukk label)
{
	string stringValue;
	const bool readSuccess = VarUtil::ReadChildVariableAs<string>(m_pVariable, m_childIndexOverride, name, stringValue);
	if (readSuccess)
	{
		value.set(stringValue);
		return true;
	}
	return false;
}

bool CVariableIArchive::operator()(Serialization::IWString& value, tukk name, tukk label)
{
	CryFatalError("CVariableIArchive::operator() with IWString is not implemented");
	return false;
}

bool CVariableIArchive::operator()(float& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<float>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(double& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<float>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(i16& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(u16& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(i32& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(u32& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(int64& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(uint64& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(int8& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(u8& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(char& value, tukk name, tukk label)
{
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, value);
}

bool CVariableIArchive::operator()(const Serialization::SStruct& ser, tukk name, tukk label)
{
	tukk const typeName = ser.type().name();
	HandlersMap::const_iterator it = m_structHandlers.find(typeName);
	const bool handlerFound = (it != m_structHandlers.end());
	if (handlerFound)
	{
		StructHandlerFunctionPtr pHandler = it->second;
		return (this->*pHandler)(ser, name, label);
	}

	return SerializeStruct(ser, name, label);
}

bool CVariableIArchive::operator()(Serialization::IContainer& ser, tukk name, tukk label)
{
	_smart_ptr<IVariable> pChild = VarUtil::FindChildVariable(m_pVariable, m_childIndexOverride, name);
	if (pChild)
	{
		i32k elementCount = pChild->GetNumVariables();
		ser.resize(elementCount);

		if (0 < elementCount)
		{
			CVariableIArchive childArchive(pChild);
			childArchive.setFilter(getFilter());
			childArchive.setLastContext(lastContext());

			for (i32 i = 0; i < elementCount; ++i)
			{
				childArchive.m_childIndexOverride = i;

				ser(childArchive, "", "");
				ser.next();
			}
		}
		return true;
	}
	return false;
}

bool CVariableIArchive::SerializeStruct(const Serialization::SStruct& ser, tukk name, tukk label)
{
	_smart_ptr<IVariable> pChild = VarUtil::FindChildVariable(m_pVariable, m_childIndexOverride, name);
	if (pChild)
	{
		CVariableIArchive childArchive(pChild);
		childArchive.setFilter(getFilter());
		childArchive.setLastContext(lastContext());

		ser(childArchive);
		return true;
	}
	return false;
}

bool CVariableIArchive::SerializeResourceSelector(const Serialization::SStruct& ser, tukk name, tukk label)
{
	Serialization::IResourceSelector* pSelector = reinterpret_cast<Serialization::IResourceSelector*>(ser.pointer());

	string stringValue;
	const bool readSuccess = VarUtil::ReadChildVariableAs<string>(m_pVariable, m_childIndexOverride, name, stringValue);
	if (readSuccess)
	{
		pSelector->SetValue(stringValue);
		return true;
	}
	return false;
}

bool CVariableIArchive::SerializeStringListStaticValue(const Serialization::SStruct& ser, tukk name, tukk label)
{
	StringListStaticValue* const pStringListStaticValue = reinterpret_cast<StringListStaticValue*>(ser.pointer());
	const StringListStatic& stringListStatic = pStringListStaticValue->stringList();

	_smart_ptr<IVariable> pChild = VarUtil::FindChildVariable(m_pVariable, m_childIndexOverride, name);
	if (pChild)
	{
		i32 index = -1;
		pChild->Get(index);
		*pStringListStaticValue = index;
		return true;
	}
	return false;
}

bool CVariableIArchive::SerializeRangeFloat(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::RangeDecorator<float>* const pRange = reinterpret_cast<Serialization::RangeDecorator<float>*>(ser.pointer());
	return VarUtil::ReadChildVariableAs<float>(m_pVariable, m_childIndexOverride, name, *pRange->value);
}

bool CVariableIArchive::SerializeRangeInt(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::RangeDecorator<i32>* const pRange = reinterpret_cast<Serialization::RangeDecorator<i32>*>(ser.pointer());
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, *pRange->value);
}

bool CVariableIArchive::SerializeRangeUInt(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::RangeDecorator<u32>* const pRange = reinterpret_cast<Serialization::RangeDecorator<u32>*>(ser.pointer());
	return VarUtil::ReadChildVariableAs<i32>(m_pVariable, m_childIndexOverride, name, *pRange->value);
}

