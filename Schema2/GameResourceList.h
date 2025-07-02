// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IGameResourceList.h>
#include <drx3D/Schema2/TemplateUtils_StringHashWrapper.h>

namespace sxema2
{
	class CGameResourceList : public IGameResourceList
	{
	private:
		typedef TemplateUtils::CStringHashWrapper<TemplateUtils::CStringHashCRC32, TemplateUtils::CEmptyStringConversion, string> ResourceName;

		struct SResource
		{
			SResource(ResourceName _name, IGameResourceList::EType _type)
				: name(_name)
				, type(_type)
			{

			}

			inline bool operator==(const SResource& rhs) const { return (name == rhs.name) && (type == rhs.type); }
			inline bool operator< (const SResource& rhs) const { return (type < rhs.type) || ((type == rhs.type) && (name < rhs.name)); }

			ResourceName             name;
			IGameResourceList::EType type;
		};

	protected:

		// IGameResourceList
		virtual void   AddResource(tukk szResource, EType type) override;
		virtual size_t GetResourceCount() const override;
		virtual SItem  GetResourceItemAt(size_t idx) const override;
		virtual void   Sort() override;
		//~IGameResourceList

	private:
		std::vector<SResource> m_resources;
	};
}
