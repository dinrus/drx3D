// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/DrxGUID.h>

#include <drx3D/Reflection/IReflection.h>
#include <drx3D/Reflection/IDescExtension.h>
#include <drx3D/Reflection/IFunctionDesc.h>
#include <drx3D/Reflection/IVariableDesc.h>

namespace Drx {
namespace Reflection {

#define DRX_REFLECTION_REGISTER_BASE(typeDesc, baseType) \
  typeDesc.ReflectBaseType(TypeId::Of<baseType>, __SOURCE_INFO__)

#define DRX_REFLECTION_REGISTER_MEMBER_FUNCTION(typeDesc, funcPtr, label, guid) \
  typeDesc.ReflectFunction(, label, guid, __SOURCE_INFO__)

#define DRX_REFLECTION_REGISTER_MEMBER_VARIABLE(typeDesc, variable, label, guid) \
  typeDesc.ReflectVariable(&variable, label, guid, __SOURCE_INFO__)

//#define DRX_REFLECTION_REGISTER_CONVERSION(typeDesc, typeId, conversionOerator) \
//  typeDesc.AddConversionOperator(typeId, conversionOperator, __SOURCE_INFO__)

enum class ETypeCategory
{
	Unset = 0,

	Abstract,
	Class,
	Enum,
	Fundamental,
	Union
};

struct IEnumValueDesc
{
	virtual ~IEnumValueDesc() {}

	virtual tukk GetLabel() const = 0;
	virtual tukk GetDescription() const = 0;
	virtual size_t      GetValue() const = 0;
};

struct ITypeDesc : public IExtensibleDesc
{
	virtual ~ITypeDesc() {}

	virtual DrxTypeId              GetTypeId() const = 0;
	virtual const DrxTypeDesc&     GetTypeDesc() const = 0;

	virtual bool                   IsAbstract() const = 0;
	virtual bool                   IsArray() const = 0;
	virtual bool                   IsClass() const = 0;
	virtual bool                   IsEnum() const = 0;
	virtual bool                   IsFundamental() const = 0;
	virtual bool                   IsUnion() const = 0;

	virtual const DrxGUID&         GetGuid() const = 0;
	virtual ETypeCategory          GetCategory() const = 0;
	virtual TypeIndex              GetIndex() const = 0;

	virtual tukk            GetRawName() const = 0;
	virtual void                   SetLabel(tukk szLabel) = 0;
	virtual tukk            GetLabel() const = 0;

	virtual void                   SetDescription(tukk szDescription) = 0;
	virtual tukk            GetDescription() const = 0;

	virtual u16                 GetSize() const = 0;
	virtual u16                 GetAlignment() const = 0;

	virtual const SSourceFileInfo& GetSourceInfo() const = 0;

	// Class Members
	virtual bool                 AddBaseType(DrxTypeId typeId, const SSourceFileInfo& srcInfo) = 0;
	virtual size_t               GetBaseTypeCount() const = 0;
	virtual const ITypeDesc*     GetBaseTypeByIndex(size_t index) const = 0;

	virtual IFunctionDesc*       AddFunction(const CMemberFunction& function, tukk szLabel, const DrxGUID& guid, const SSourceFileInfo& srcInfo) = 0;
	virtual size_t               GetFunctionCount() const = 0;
	virtual const IFunctionDesc* GetFunctionByIndex(size_t index) const = 0;

	virtual IVariableDesc*       AddVariable(DrxTypeId typeId, ptrdiff_t offset, tukk szLabel, const DrxGUID& guid, const SSourceFileInfo& srcInfo) = 0;
	virtual size_t               GetVariableCount() const = 0;
	virtual const IVariableDesc* GetVariableByIndex(size_t index) const = 0;

	// Enum Members
	virtual IEnumValueDesc*       AddEnumValue(tukk szName, size_t value, tukk szDescription) = 0;
	virtual size_t                GetEnumValueCount() const = 0;
	virtual const IEnumValueDesc* GetEnumValueByIndex(size_t index) const = 0;

	// Generic Members
	virtual Type::CDefaultConstructor GetDefaultConstructor() const = 0;
	virtual Type::CCopyConstructor    GetCopyConstructor() const = 0;
	virtual Type::CMoveConstructor    GetMoveConstructor() const = 0;

	virtual Type::CDestructor         GetDestructor() const = 0;

	virtual Type::CCopyAssignOperator GetCopyAssignOperator() const = 0;
	virtual Type::CEqualOperator      GetEqualOperator() const = 0;

	virtual bool                      AddConversionOperator(DrxTypeId typeId, Type::CConversionOperator conversionOperator) = 0;
	virtual Type::CConversionOperator GetConversionOperator(DrxTypeId typeId) = 0;

	// Reflection Helpers
	bool ReflectBaseType(DrxTypeId baseTypeId, const SSourceFileInfo& srcInfo)
	{
		return AddBaseType(baseTypeId, srcInfo);
	}

	template<typename FUNCTION_TYPE>
	IFunctionDesc* ReflectFunction(FUNCTION_TYPE funcPtr, tukk szLabel, const DrxGUID& guid, const SSourceFileInfo& srcInfo)
	{
		return AddFunction(CMemberFunctionCreator<CFunctionDesc<decltype(funcPtr)>>(funcPtr), szLabel, guid, srcInfo);
	}

	template<typename VARIABLE_TYPE, typename OBJECT_TYPE>
	IVariableDesc* ReflectVariable(VARIABLE_TYPE OBJECT_TYPE::* pVariable, tukk szLabel, const DrxGUID& guid, const SSourceFileInfo& srcInfo)
	{
		const DrxTypeDesc desc = TypeDescOf<VARIABLE_TYPE>();
		return AddVariable(desc.GetTypeId(), CMemberVariableOffset<VARIABLE_TYPE, OBJECT_TYPE>(pVariable), szLabel, guid, srcInfo);
	}

	IEnumValueDesc* ReflectEnumValue(tukk szLabel, size_t value, tuk szDescription)
	{
		return AddEnumValue(szLabel, value, szDescription);
	}

	bool AddConversion(DrxTypeId typeId, Type::CConversionOperator& conversionOperator)
	{
		return AddConversionOperator(typeId, conversionOperator);
	}

	// Operators
	bool operator==(const ITypeDesc& rhs) const
	{
		return GetGuid() == rhs.GetGuid();
	}

	bool operator!=(const ITypeDesc& rhs) const
	{
		return GetGuid() != rhs.GetGuid();
	}
};

} // ~Reflection namespace
} // ~Drx namespace
