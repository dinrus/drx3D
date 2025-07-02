// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>

namespace NDrxVulkan
{
	namespace Extensions
	{
		namespace EXT_debug_marker
		{
			extern bool                              IsSupported;

			extern PFN_vkDebugMarkerSetObjectTagEXT  SetObjectTag;
			extern PFN_vkDebugMarkerSetObjectNameEXT SetObjectName;
			extern PFN_vkCmdDebugMarkerBeginEXT      CmdDebugMarkerBegin;
			extern PFN_vkCmdDebugMarkerEndEXT        CmdDebugMarkerEnd;
			extern PFN_vkCmdDebugMarkerInsertEXT     CmdDebugMarkerInsert;
		}

		void Init(CDevice* pDevice, const std::vector<tukk >& loadedExtensions);
	}
}