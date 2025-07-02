// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/DrxGUID.h>

#include <drx3D/Reflection/IReflection.h>
#include <drx3D/Reflection/IDescExtension.h>

namespace Drx {
namespace Reflection {

struct ITypeDesc;

struct IVariableDesc : public IExtensibleDesc
{
	virtual ~IVariableDesc() {}

	virtual const DrxGUID&         GetGuid() const = 0;
	virtual DrxTypeId              GetTypeId() const = 0;

	virtual tukk            GetLabel() const = 0;

	virtual void                   SetDescription(tukk szDescription) = 0;
	virtual tukk            GetDescription() const = 0;

	virtual bool                   IsMemberVariable() const = 0;
	virtual ptrdiff_t              GetOffset() const = 0;
	virtual uk                  GetPointer() const = 0;

	virtual const SSourceFileInfo& GetSourceInfo() const = 0;
};

template<typename VARIABLE_TYPE, typename OBJECT_TYPE>
class CMemberVariableOffset
{
public:
	CMemberVariableOffset(VARIABLE_TYPE OBJECT_TYPE::* pVariable)
	{
		m_offset = reinterpret_cast<u8*>(&(reinterpret_cast<OBJECT_TYPE*>(1)->*pVariable)) - reinterpret_cast<u8*>(1);
	}

	operator ptrdiff_t() const
	{
		return m_offset;
	}

private:
	ptrdiff_t m_offset;
};

} // ~Drx namespace
} // ~Reflection namespace
