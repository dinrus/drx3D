// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Deprecated/IStringPool.h>

namespace sxema2
{
	// String pool.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CStringPool : public IStringPool
	{
	public:

		CStringPool();

		~CStringPool();

		// IStringPool
		virtual CPoolStringData* Insert(tukk data) override;
		virtual void Erase(CPoolStringData* pString) override;
		// ~IStringPool

	private:

		struct StringSortPredicate
		{
			inline bool operator () (u32 lhs, const CPoolStringData* rhs) const
			{
				return lhs < rhs->CRC32();
			}

			inline bool operator () (const CPoolStringData* lhs, u32 rhs) const
			{
				return lhs->CRC32() < rhs;
			}
		};

		typedef std::vector<CPoolStringData*> TStringVector;

		TStringVector	m_strings;
	};
}
