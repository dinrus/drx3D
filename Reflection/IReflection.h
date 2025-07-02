// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IDrxPlugin.h>
#include <drx3D/CoreX/Extension/DrxGUID.h>
#include <drx3D/CoreX/Type/Type.h>

#ifdef DRXREFLECTION_EXPORTS
	#define DRX_REFLECTION_API DLL_EXPORT
#else
	#define DRX_REFLECTION_API DLL_IMPORT
#endif

namespace Drx {
namespace Reflection {

struct IReflection;
struct ITypeDesc;
class CDescExtension;

inline IReflection& GetReflectionRegistry()
{
	DRX_ASSERT_MESSAGE(gEnv->pReflection, "Reflection module not yet initialized.");
	return *gEnv->pReflection;
}

template<typename TYPE, TYPE INVALID_INDEX = (zu64)~0>
struct Index
{
	typedef TYPE ValueType;

	static const ValueType Invalid = INVALID_INDEX;

	Index()
		: m_value(Invalid)
	{}

	Index(ValueType index)
		: m_value(index)
	{}

	Index(const Index& rh)
		: m_value(rh.m_value)
	{}

	bool IsValid() const
	{
		return (m_value != Invalid);
	}

	operator ValueType() const
	{
		return m_value;
	}

	Index& operator++()
	{
		++m_value;
		return *this;
	}

	Index& operator--()
	{
		--m_value;
		return *this;
	}

	Index operator++(i32)
	{
		Index tmp(m_value);
		++m_value;
		return tmp;
	}

	Index operator--(i32)
	{
		Index tmp(m_value);
		--m_value;
		return tmp;
	}

	Index& operator+=(Index index)
	{
		m_value += index.m_value;
		return *this;
	}

	Index& operator-=(Index index)
	{
		m_value -= index.m_value;
		return *this;
	}

	Index& operator+=(ValueType value)
	{
		m_value += value;
		return *this;
	}

	Index& operator-=(ValueType value)
	{
		m_value -= value;
		return *this;
	}

	Index operator+(Index index) const
	{
		return Index(m_value + index.m_value);
	}

	Index& operator+(ValueType value) const
	{
		return Index(m_value + value);
	}

	Index operator-(Index index) const
	{
		return Index(m_value - index.m_value);
	}

	Index& operator-(ValueType value) const
	{
		return Index(m_value - value);
	}

private:
	ValueType m_value;
};

typedef Index<size_t> TypeIndex;
typedef Index<size_t> ExtensionIndex;

struct SSourceFileInfo
{
	SSourceFileInfo()
		: m_line(~0)
	{}

	SSourceFileInfo(tukk szFile, u32 line, tukk szFunction = "")
		: m_file(szFile)
		, m_line(line)
		, m_function(szFunction)
	{}

	SSourceFileInfo(const SSourceFileInfo& srcPos)
		: m_file(srcPos.GetFile())
		, m_line(srcPos.GetLine())
		, m_function(srcPos.GetFunction())
	{}

	tukk GetFile() const     { return m_file.c_str(); }
	u32      GetLine() const     { return m_line; }
	tukk GetFunction() const { return m_function.c_str(); }

	// Operators
	SSourceFileInfo& operator=(const SSourceFileInfo& rhs)
	{
		m_file = rhs.GetFile();
		m_line = rhs.GetLine();
		m_function = rhs.GetFunction();

		return *this;
	}

	bool operator==(const SSourceFileInfo& rhs) const
	{
		return (m_file == rhs.m_file && m_line == rhs.m_line && m_function == rhs.m_function);
	}

	bool operator!=(const SSourceFileInfo& rhs) const
	{
		return (m_file != rhs.m_file || m_line != rhs.m_line || m_function != rhs.m_function);
	}

private:
	string m_file;
	string m_function;
	i32  m_line;
};

#define __SOURCE_INFO__ Drx::Reflection::SSourceFileInfo(__FILE__, __LINE__, __FUNCTION__)

struct ISystemTypeRegistry
{
	virtual ~ISystemTypeRegistry() {}

	virtual bool                 UseType(const ITypeDesc& typeDesc) = 0;

	virtual const DrxGUID&       GetGuid() const = 0;
	virtual tukk          GetLabel() const = 0;
	virtual const DrxTypeDesc&   GetTypeDesc() const = 0;
	virtual DrxTypeId            GetTypeId() const = 0;

	virtual TypeIndex::ValueType GetTypeCount() const = 0;
	virtual const ITypeDesc*     FindTypeByIndex(TypeIndex index) const = 0;
	virtual const ITypeDesc*     FindTypeByGuid(const DrxGUID& guid) const = 0;
	virtual const ITypeDesc*     FindTypeById(DrxTypeId typeId) const = 0;
};

typedef void (* ReflectTypeFunction)(ITypeDesc& typeDesc);

struct IReflection : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IReflection, "4465B4A8-4E5F-4813-85E0-A187A849AA7B"_drx_guid);

	virtual ~IReflection() {}

	virtual ITypeDesc*           Register(const DrxTypeDesc& typeDesc, const DrxGUID& guid, ReflectTypeFunction pReflectFunc, const SSourceFileInfo& srcPos) = 0;

	virtual TypeIndex::ValueType GetTypeCount() const = 0;
	virtual const ITypeDesc*     FindTypeByIndex(TypeIndex index) const = 0;
	virtual const ITypeDesc*     FindTypeByGuid(DrxGUID guid) const = 0;
	virtual const ITypeDesc*     FindTypeById(DrxTypeId typeId) const = 0;

	virtual size_t               GetSystemRegistriesCount() const = 0;
	virtual ISystemTypeRegistry* FindSystemRegistryByIndex(size_t index) const = 0;
	virtual ISystemTypeRegistry* FindSystemRegistryByGuid(const DrxGUID& guid) const = 0;
	virtual ISystemTypeRegistry* FindSystemRegistryById(DrxTypeId typeId) const = 0;

	virtual void                 RegisterSystemRegistry(ISystemTypeRegistry* pSystemRegistry) = 0;
};

struct IObject
{
	virtual ~IObject() {}

	virtual bool             IsReflected() const = 0;

	virtual DrxTypeId        GetTypeId() const = 0;
	virtual TypeIndex        GetTypeIndex() const = 0;
	virtual const ITypeDesc* GetTypeDesc() const = 0;

	virtual bool             IsEqualType(const IObject* pObject) const = 0;
	virtual bool             IsEqualObject(const IObject* pOtherObject) const = 0;
};

} // ~Reflection namespace
} // ~Drx namespace
