// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Allow registration of patches from outside this system?
// #SchematycTODO : Build library of utility functions for common operations?

#pragma once

#include <drx3D/Schema2/TemplateUtils_Delegate.h>

namespace sxema2
{
	// Document patcher.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CDocPatcher
	{
	public:

		CDocPatcher();

		XmlNodeRef PatchDoc(XmlNodeRef xml, tukk szFileName, bool bFromPak);
		u32 GetCurrentVersion() const;

	private:

		typedef TemplateUtils::CDelegate<bool (XmlNodeRef, tukk )> PatchCallback;

		struct SPatch
		{
			SPatch(u32 _fromVersion, u32 _toVersion, const PatchCallback& _callback);

			u32        fromVersion;
			u32        toVersion;
			PatchCallback callback;
		};

		typedef std::vector<SPatch> PatchVector;

		bool Patch101(XmlNodeRef xml, tukk szFileName) const;
		bool Patch102(XmlNodeRef xml, tukk szFileName) const;
		bool Patch103(XmlNodeRef xml, tukk szFileName) const;
		bool Patch104(XmlNodeRef xml, tukk szFileName) const;

		PatchVector m_patches;
	};
}