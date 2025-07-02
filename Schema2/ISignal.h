// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Separate IEnvSignal and ILibSignal interfaces?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/Deprecated/IStringPool.h>
#include <drx3D/Schema2/Deprecated/Variant.h>

namespace sxema2
{
	struct ISignal
	{
		virtual ~ISignal() {}

		virtual SGUID GetGUID() const = 0;
		virtual void SetSenderGUID(const SGUID& senderGUID) = 0;
		virtual SGUID GetSenderGUID() const = 0;
		virtual void SetName(tukk szName) = 0;
		virtual tukk GetName() const = 0;
		virtual void SetNamespace(tukk szNamespace) = 0;
		virtual tukk GetNamespace() const = 0;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) = 0;
		virtual tukk GetFileName() const = 0;
		virtual void SetAuthor(tukk szAuthor) = 0;
		virtual tukk GetAuthor() const = 0;
		virtual void SetDescription(tukk szDescription) = 0;
		virtual void SetWikiLink(tukk szWikiLink) = 0;
		virtual tukk GetWikiLink() const = 0;
		virtual tukk GetDescription() const = 0;
		virtual size_t GetInputCount() const = 0;
		virtual tukk GetInputName(size_t inputIdx) const = 0;
		virtual tukk GetInputDescription(size_t inputIdx) const = 0;
		virtual IAnyConstPtr GetInputValue(size_t inputIdx) const = 0;
		virtual TVariantConstArray GetVariantInputs() const = 0;

		template <typename TYPE> inline void AddInput(tukk szName, tukk szDescription, const TYPE& value)
		{
			AddInput_Protected(szName, szDescription, MakeAny(value));
		}

		inline void AddInput(tukk szName, tukk szDescription, tukk value)
		{
			AddInput_Protected(szName, szDescription, MakeAny(CPoolString(value)));
		}

		inline void AddInput(tukk szName, tukk szDescription, const IAny& value)
		{
			AddInput_Protected(szName, szDescription, value);
		}

	protected:

		virtual size_t AddInput_Protected(tukk szName, tukk szDescription, const IAny& value) = 0;
	};

	DECLARE_SHARED_POINTERS(ISignal)
}
