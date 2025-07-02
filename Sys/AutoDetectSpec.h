// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _AUTODETECTSPEC_
#define _AUTODETECTSPEC_

#pragma once

#if DRX_PLATFORM_WINDOWS

// exposed AutoDetectSpec() helper functions for reuse in DinrusSystem
namespace Win32SysInspect
{
enum DXFeatureLevel
{
	DXFL_Undefined,
	DXFL_9_1,
	DXFL_9_2,
	DXFL_9_3,
	DXFL_10_0,
	DXFL_10_1,
	DXFL_11_0,
	DXFL_11_1,
	DXFL_12_0,
	DXFL_12_1
};

static tukk GetFeatureLevelAsString(DXFeatureLevel featureLevel)
{
	switch (featureLevel)
	{
	default:
	case Win32SysInspect::DXFL_Undefined:
		return "unknown";
	case Win32SysInspect::DXFL_9_1:
		return "D3D 9_1 (SM 2.0)";
	case Win32SysInspect::DXFL_9_2:
		return "D3D 9_2 (SM 2.0)";
	case Win32SysInspect::DXFL_9_3:
		return "D3D 9_3 (SM 2.x)";
	case Win32SysInspect::DXFL_10_0:
		return "D3D 10_0 (SM 4.0)";
	case Win32SysInspect::DXFL_10_1:
		return "D3D 10_1 (SM 4.x)";
	case Win32SysInspect::DXFL_11_0:
		return "D3D 11_0 (SM 5.0)";
	case Win32SysInspect::DXFL_11_1:
		return "D3D 11_1 (SM 5.x)";
	case Win32SysInspect::DXFL_12_0:
		return "D3D 12_0 (SM 6.0)";
	case Win32SysInspect::DXFL_12_1:
		return "D3D 12_1 (SM 6.0)";
	}
}

void          GetNumCPUCores(u32& totAvailToSystem, u32& totAvailToProcess);
bool          IsDX11Supported();
bool          IsDX12Supported();
bool          GetGPUInfo(tuk pName, size_t bufferSize, u32& vendorID, u32& deviceID, u32& totLocalVidMem, DXFeatureLevel& featureLevel);
i32           GetGPURating(u32 vendorId, u32 deviceId);
void          GetOS(SPlatformInfo::EWinVersion& ver, bool& is64Bit, tuk pName, size_t bufferSize);
bool          IsVistaKB940105Required();

inline size_t SafeMemoryThreshold(size_t memMB)
{
	return (memMB * 8) / 10;
}
}

#endif // #if DRX_PLATFORM_WINDOWS

#endif // #ifndef _AUTODETECTSPEC_
