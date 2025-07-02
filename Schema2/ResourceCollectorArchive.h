// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IResourceCollectorArchive.h>

namespace GameSerialization
{
	struct SGameResource;
}

namespace sxema2
{
	class CResourceCollectorArchive : public IResourceCollectorArchive
	{
	public:

		CResourceCollectorArchive(IGameResourceListPtr pResourceList);

	protected:

		using IResourceCollectorArchive::operator ();

		// Serialization::IArchive
		virtual bool operator () (bool& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (int8& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (u8& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (i32& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (u32& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (int64& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (uint64& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (float& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (Serialization::IString& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (const Serialization::SStruct& value, tukk szName = "", tukk szLabel = nullptr) override;
		virtual bool operator () (Serialization::IContainer& value, tukk szName = "", tukk szLabel = nullptr) override;

		virtual void validatorMessage(bool bError, ukk handle, const Serialization::TypeID& type, tukk szMessage) override;
		// ~Serialization::IArchive

		void ExtractResource(const GameSerialization::SGameResource* pResource);
		void ExtractResource(tukk szResourcePath);

	private:
		IGameResourceListPtr m_pResourceList;
	};
}

