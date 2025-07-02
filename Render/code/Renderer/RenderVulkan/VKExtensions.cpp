// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <drx3D/Render/StdAfx.h>

#include <drx3D/Render/VKExtensions.hpp>

namespace NDrxVulkan { namespace Extensions 
{
	namespace EXT_debug_marker
	{
		bool                              IsSupported          = false;

		PFN_vkDebugMarkerSetObjectTagEXT  SetObjectTag         = nullptr;
		PFN_vkDebugMarkerSetObjectNameEXT SetObjectName        = nullptr;
		PFN_vkCmdDebugMarkerBeginEXT      CmdDebugMarkerBegin  = nullptr;
		PFN_vkCmdDebugMarkerEndEXT        CmdDebugMarkerEnd    = nullptr;
		PFN_vkCmdDebugMarkerInsertEXT     CmdDebugMarkerInsert = nullptr;
	}

	void Init(CDevice* pDevice, const std::vector<tukk >& loadedExtensions)
	{
		for (auto extensionName : loadedExtensions)
		{
			if (strcmp(extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
			{
				EXT_debug_marker::SetObjectTag         = (PFN_vkDebugMarkerSetObjectTagEXT)  vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkDebugMarkerSetObjectTagEXT");
				EXT_debug_marker::SetObjectName        = (PFN_vkDebugMarkerSetObjectNameEXT) vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkDebugMarkerSetObjectNameEXT");
				EXT_debug_marker::CmdDebugMarkerBegin  = (PFN_vkCmdDebugMarkerBeginEXT)      vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdDebugMarkerBeginEXT");
				EXT_debug_marker::CmdDebugMarkerEnd    = (PFN_vkCmdDebugMarkerEndEXT)        vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdDebugMarkerEndEXT");
				EXT_debug_marker::CmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)     vkGetDeviceProcAddr(pDevice->GetVkDevice(), "vkCmdDebugMarkerInsertEXT");

				EXT_debug_marker::IsSupported = true;
			}
		}
	}
}}

