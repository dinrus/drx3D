// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Util/Variable.h"
#include "Serialization.h"

namespace Serialization
{
class CVariableIArchive
	: public IArchive
{
public:
	CVariableIArchive(const _smart_ptr<IVariable>& pVariable);
	virtual ~CVariableIArchive();

	// IArchive
	virtual bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(IString& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(IWString& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(float& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(double& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(int64& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(uint64& value, tukk name = "", tukk label = 0) override;

	virtual bool operator()(int8& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(char& value, tukk name = "", tukk label = 0);

	virtual bool operator()(const SStruct& ser, tukk name = "", tukk label = 0) override;
	virtual bool operator()(IContainer& ser, tukk name = "", tukk label = 0) override;
	//virtual bool operator()( IPointer& ptr, tukk name = "", tukk label = 0 ) override;
	// ~IArchive

	using IArchive::operator();

private:
	bool SerializeResourceSelector(const SStruct& ser, tukk name, tukk label);

	bool SerializeStruct(const SStruct& ser, tukk name, tukk label);

	bool SerializeStringListStaticValue(const SStruct& ser, tukk name, tukk label);

	bool SerializeRangeFloat(const SStruct& ser, tukk name, tukk label);
	bool SerializeRangeInt(const SStruct& ser, tukk name, tukk label);
	bool SerializeRangeUInt(const SStruct& ser, tukk name, tukk label);

private:
	_smart_ptr<IVariable> m_pVariable;
	i32                   m_childIndexOverride;

	typedef bool (CVariableIArchive::*                 StructHandlerFunctionPtr)(const SStruct&, tukk , tukk );
	typedef std::map<string, StructHandlerFunctionPtr> HandlersMap;
	HandlersMap m_structHandlers;   // TODO: have only one of these.
};
}

