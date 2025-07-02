// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/IGameResourceList.h>

namespace sxema2
{
	struct IResourceCollectorArchive : public Serialization::IArchive
	{
		// Note: 
		//	This flag is used by 'GameResource' serializable types and some other custom code.
		//	If some other archive implementation would use it, it could create issues. 
		//	It is not likely, and it would be easy to fix but worth to be mentioned.
		enum 
		{
			ArchiveCaps = (Serialization::IArchive::CUSTOM3 << 1)
		};

		IResourceCollectorArchive()
			: Serialization::IArchive(Serialization::IArchive::OUTPUT | ArchiveCaps)
		{
		}

		virtual ~IResourceCollectorArchive() {}

		using Serialization::IArchive::operator ();
	};

	DECLARE_SHARED_POINTERS(IResourceCollectorArchive)
}
