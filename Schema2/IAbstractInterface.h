// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/Deprecated/IStringPool.h>
#include <drx3D/Schema2/Deprecated/Variant.h>

namespace sxema2
{
	struct IAbstractInterfaceFunction
	{
		virtual ~IAbstractInterfaceFunction() {}

		virtual SGUID GetGUID() const = 0;
		virtual SGUID GetOwnerGUID() const = 0; // #SchematycTODO : Rename GetAbstractInterfaceGUID()?
		virtual void SetName(tukk szName) = 0;
		virtual tukk GetName() const = 0;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) = 0;
		virtual tukk GetFileName() const = 0;
		virtual void SetAuthor(tukk szAuthor) = 0;
		virtual tukk GetAuthor() const = 0;
		virtual void SetDescription(tukk szDescription) = 0;
		virtual tukk GetDescription() const = 0;
		virtual void AddInput(tukk szName, tukk szDescription, const IAnyPtr& pValue) = 0;
		virtual size_t GetInputCount() const = 0;
		virtual tukk GetInputName(size_t inputIdx) const = 0;
		virtual tukk GetInputDescription(size_t inputIdx) const = 0;
		virtual IAnyConstPtr GetInputValue(size_t inputIdx) const = 0;
		virtual TVariantConstArray GetVariantInputs() const = 0;
		virtual void AddOutput(tukk szName, tukk szDescription, const IAnyPtr& pValue) = 0;
		virtual size_t GetOutputCount() const = 0;
		virtual tukk GetOutputName(size_t outputIdx) const = 0;
		virtual tukk GetOutputDescription(size_t outputIdx) const = 0;
		virtual IAnyConstPtr GetOutputValue(size_t outputIdx) const = 0;
		virtual TVariantConstArray GetVariantOutputs() const = 0;

		template <typename TYPE> inline void AddInput(tukk szName, tukk szDescription, const TYPE& value)
		{
			AddInput(szName, szDescription, MakeAnyShared(value));
		}

		inline void AddInput(tukk szName, tukk szDescription, tukk value)
		{
			AddInput(szName, szDescription, MakeAnyShared(CPoolString(value)));
		}

		template <typename TYPE> inline void AddOutput(tukk szName, tukk szDescription, const TYPE& value)
		{
			AddOutput(szName, szDescription, MakeAnyShared(value));
		}

		inline void AddOutput(tukk szName, tukk szDescription, tukk value)
		{
			AddOutput(szName, szDescription, MakeAnyShared(CPoolString(value)));
		}
	};

	DECLARE_SHARED_POINTERS(IAbstractInterfaceFunction)

	struct IAbstractInterface
	{
		virtual ~IAbstractInterface() {}

		virtual SGUID GetGUID() const = 0;
		virtual void SetName(tukk szName) = 0;
		virtual tukk GetName() const = 0;
		virtual void SetNamespace(tukk szNamespace) = 0;
		virtual tukk GetNamespace() const = 0;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) = 0;
		virtual tukk GetFileName() const = 0;
		virtual void SetAuthor(tukk szAuthor) = 0;
		virtual tukk GetAuthor() const = 0;
		virtual void SetDescription(tukk szDescription) = 0;
		virtual tukk GetDescription() const = 0;
		virtual IAbstractInterfaceFunctionPtr AddFunction(const SGUID& guid) = 0;
		virtual size_t GetFunctionCount() const = 0;
		virtual IAbstractInterfaceFunctionConstPtr GetFunction(size_t functionIdx) const = 0;
	};

	DECLARE_SHARED_POINTERS(IAbstractInterface)
}
