// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/BasicTypes.h>

namespace sxema2
{
	struct IQuickSearchOptions
	{
		virtual ~IQuickSearchOptions() {}

		virtual tukk GetToken() const = 0;
		virtual size_t GetCount() const = 0;
		virtual tukk GetLabel(size_t optionIdx) const = 0;
		virtual tukk GetDescription(size_t optionIdx) const = 0;
		virtual tukk GetWikiLink(size_t optionIdx) const = 0;
	};

	namespace QuickSearchUtils
	{
		inline size_t FindOption(const IQuickSearchOptions& options, tukk szLabel)
		{
			DRX_ASSERT(szLabel);
			if(szLabel)
			{
				for(size_t optionIdx = 0, optionCount = options.GetCount(); optionIdx < optionCount; ++ optionIdx)
				{
					if(strcmp(szLabel, options.GetLabel(optionIdx)) == 0)
					{
						return optionIdx;
					}
				}
			}
			return INVALID_INDEX;
		}
	}

	struct SQuickSearchOption
	{
		inline SQuickSearchOption(const IQuickSearchOptions& _options, const size_t _optionIdx = INVALID_INDEX)
			: options(_options)
			, optionIdx(_optionIdx)
		{}

		inline tukk c_str() const
		{
			return optionIdx < options.GetCount() ? options.GetLabel(optionIdx) : "";
		}

		inline void operator = (tukk szLabel)
		{
			optionIdx = QuickSearchUtils::FindOption(options, szLabel);
		}

		inline void Serialize(Serialization::IArchive& archive)
		{
			archive(optionIdx, "optionIdx");
		}

		const IQuickSearchOptions& options;
		size_t                     optionIdx;
	};
}
