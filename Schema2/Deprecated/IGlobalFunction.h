// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/Deprecated/Variant.h>

namespace sxema2
{
	struct IGlobalFunction
	{
		virtual ~IGlobalFunction() {}

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
		virtual void SetWikiLink(tukk szWikiLink) = 0;
		virtual tukk GetWikiLink() const = 0;
		virtual bool BindInput(size_t paramIdx, tukk szName, tukk szDescription) = 0;
		virtual size_t GetInputCount() const = 0;
		virtual IAnyConstPtr GetInputValue(size_t inputIdx) const = 0;
		virtual tukk GetInputName(size_t inputIdx) const = 0;
		virtual tukk GetInputDescription(size_t inputIdx) const = 0;
		virtual TVariantConstArray GetVariantInputs() const = 0;
		virtual bool BindOutput(size_t paramIdx, tukk szName, tukk szDescription) = 0;
		virtual size_t GetOutputCount() const = 0;
		virtual IAnyConstPtr GetOutputValue(size_t outputIdx) const = 0;
		virtual tukk GetOutputName(size_t outputIdx) const = 0;
		virtual tukk GetOutputDescription(size_t outputIdx) const = 0;
		virtual TVariantConstArray GetVariantOutputs() const = 0;
		virtual void Call(const TVariantConstArray& inputs, const TVariantArray& outputs) const = 0;
		virtual void RegisterInputParamsForResourcePrecache() = 0;
		virtual bool AreInputParamsResources() const = 0;

		template <typename TYPE> inline void BindInput(size_t paramIdx, tukk szName, tukk szDescription, const TYPE& value)
		{
			BindInput_Protected(paramIdx, szName, szDescription, MakeAny(value));
		}

		inline void BindInput(size_t paramIdx, tukk szName, tukk szDescription, tukk value)
		{
			BindInput_Protected(paramIdx, szName, szDescription, MakeAny(CPoolString(value)));
		}

	protected:

		virtual bool BindInput_Protected(size_t paramIdx, tukk szName, tukk szDescription, const IAny& value) = 0;
	};

	DECLARE_SHARED_POINTERS(IGlobalFunction)
}
