// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#if DRX_PLATFORM_WINDOWS
    #include <drx3D/CoreX/Platform/DrxWindows.h>
    #include <wbemidl.h>
    #include <intrin.h>
    #include <d3d9.h>
    #include <ddraw.h>
    #include <dxgi.h>
    #include <d3d11.h>
    #include <d3d12.h>
    #include <map>
    #include <drx3D/Sys/IDrxPak.h>
    #include <drx3D/Sys/IConsole.h>
    #include <drx3D/CoreX/String/StringUtils.h>

    #include <drx3D/Sys/System.h>
    #include <drx3D/Sys/AutoDetectCPUTestSuit.h>
    #include <drx3D/Sys/AutoDetectSpec.h>

    #pragma warning(push)
    #pragma warning(disable: 4244)

// Обе функции находятся в CPUDetect.cpp
bool        IsAMD();
bool        IsIntel();

static void TrimExcessiveWhiteSpaces(tuk pStr)
{
    size_t len(strlen(pStr));
    bool remove(true);
    for (size_t i(0); i < len; ++i)
    {
        if (pStr[i] == ' ' && remove)
        {
            size_t newlen(len - 1);

            size_t j(i + 1);
            for (; j < len && pStr[j] == ' '; ++j)
                --newlen;

            size_t ii(i);
            for (; j < len + 1; ++j, ++ii)
                pStr[ii] = pStr[j];

            assert(newlen == strlen(pStr));
            len = newlen;
            remove = false;
        }
        else
            remove = pStr[i] == ' ';
    }

    if (len > 0 && pStr[len - 1] == ' ')
        pStr[len - 1] = '\0';
}

static void GetCPUName(tuk pName, size_t bufferSize)
{
    if (!pName || !bufferSize)
        return;

    char name[12 * 4 + 1];

    i32 CPUInfo[4];
    __cpuid(CPUInfo, 0x80000000);
    if (CPUInfo[0] >= 0x80000004)
    {
        __cpuid(CPUInfo, 0x80000002);
        ((i32*)name)[0] = CPUInfo[0];
        ((i32*)name)[1] = CPUInfo[1];
        ((i32*)name)[2] = CPUInfo[2];
        ((i32*)name)[3] = CPUInfo[3];

        __cpuid(CPUInfo, 0x80000003);
        ((i32*)name)[4] = CPUInfo[0];
        ((i32*)name)[5] = CPUInfo[1];
        ((i32*)name)[6] = CPUInfo[2];
        ((i32*)name)[7] = CPUInfo[3];

        __cpuid(CPUInfo, 0x80000004);
        ((i32*)name)[8] = CPUInfo[0];
        ((i32*)name)[9] = CPUInfo[1];
        ((i32*)name)[10] = CPUInfo[2];
        ((i32*)name)[11] = CPUInfo[3];

        name[48] = '\0';
    }
    else
        name[0] = '\0';

    drx_sprintf(pName, bufferSize, name);
}

void Win32SysInspect::GetOS(SPlatformInfo::EWinVersion& ver, bool& is64Bit, tuk pName, size_t bufferSize)
{
    ver = SPlatformInfo::WinUndetected;
    is64Bit = false;

    if (pName && bufferSize)
        pName[0] = '\0';

    //RtlGetVersion is required to ensure correct information without manifest on Windows 8.1 and later
    typedef LONG (NTAPI * fnRtlGetVersion)(PRTL_OSVERSIONINFOEXW lpVersionInformation);
    static auto RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");
    RTL_OSVERSIONINFOEXW sysInfo = { 0 };
    sysInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    if (RtlGetVersion && RtlGetVersion(&sysInfo) == 0)
    {
        if (sysInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (sysInfo.dwMajorVersion == 5)
            {
                if (sysInfo.dwMinorVersion == 0)
                    ver = SPlatformInfo::Win2000;
                else if (sysInfo.dwMinorVersion == 1)
                    ver = SPlatformInfo::WinXP;
                else if (sysInfo.dwMinorVersion == 2)
                {
                    if (sysInfo.wProductType == VER_NT_WORKSTATION)
                        ver = SPlatformInfo::WinXP; // 64 bit windows actually but this will be detected later anyway
                    else if (sysInfo.wProductType == VER_NT_SERVER || sysInfo.wProductType == VER_NT_DOMAIN_CONTROLLER)
                        ver = SPlatformInfo::WinSrv2003;
                }
            }
            else if (sysInfo.dwMajorVersion == 6)
            {
                if (sysInfo.dwMinorVersion == 0)
                    ver = SPlatformInfo::WinVista;
                else if (sysInfo.dwMinorVersion == 1)
                    ver = SPlatformInfo::Win7;
                else if (sysInfo.dwMinorVersion == 2)
                    ver = SPlatformInfo::Win8;
                else if (sysInfo.dwMinorVersion == 3)
                    ver = SPlatformInfo::Win8Point1;
            }
            else if (sysInfo.dwMajorVersion == 10)
            {
                if (sysInfo.dwMinorVersion == 0)
                    ver = SPlatformInfo::Win10;
            }
            else if (sysInfo.dwMajorVersion == 11)
            {
                if (sysInfo.dwMinorVersion == 0)
                    ver = SPlatformInfo::Win11;
            }
        }

        typedef BOOL (WINAPI * FP_GetSystemWow64Directory)(LPSTR, UINT);
        FP_GetSystemWow64Directory pgsw64d((FP_GetSystemWow64Directory) GetProcAddress(GetModuleHandle("kernel32"), "GetSystemWow64DirectoryA"));
        if (pgsw64d)
        {
            char str[MAX_PATH];
            if (!pgsw64d(str, sizeof(str)))
                is64Bit = GetLastError() != ERROR_CALL_NOT_IMPLEMENTED;
            else
                is64Bit = true;
        }

        if (pName && bufferSize)
        {
            tukk windowsVersionText(0);
            switch (ver)
            {
            case SPlatformInfo::Win2000:
                windowsVersionText = "Windows 2000";
                break;
            case SPlatformInfo::WinXP:
                windowsVersionText = "Windows XP";
                break;
            case SPlatformInfo::WinSrv2003:
                windowsVersionText = "Windows Server 2003";
                break;
            case SPlatformInfo::WinVista:
                windowsVersionText = "Windows Vista";
                break;
            case SPlatformInfo::Win7:
                windowsVersionText = "Windows 7";
                break;
            case SPlatformInfo::Win8:
                windowsVersionText = "Windows 8";
                break;
            case SPlatformInfo::Win8Point1:
                windowsVersionText = "Windows 8.1";
                break;
            case SPlatformInfo::Win10:
                windowsVersionText = "Windows 10";
                break;
            case SPlatformInfo::Win11:
                windowsVersionText = "Windows 11";
                break;
            default:
                windowsVersionText = "Windows";
                break;
            }

            char sptext[32];
            sptext[0] = '\0';
            if (sysInfo.wServicePackMajor > 0)
                drx_sprintf(sptext, "SP %d ", sysInfo.wServicePackMajor);

            drx_sprintf(pName, bufferSize, "%s %s %s(build %d.%d.%d)", windowsVersionText, is64Bit ? "64 bit" : "32 bit",
                        sptext, sysInfo.dwMajorVersion, sysInfo.dwMinorVersion, sysInfo.dwBuildNumber);
        }
    }
}

static void GetOSName(tuk pName, size_t bufferSize)
{
    SPlatformInfo::EWinVersion winVer(SPlatformInfo::WinUndetected);
    bool is64bit(false);

    Win32SysInspect::GetOS(winVer, is64bit, pName, bufferSize);
}

bool Win32SysInspect::IsVistaKB940105Required()
{
    #if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
    OSVERSIONINFOEX osv = { sizeof(osv) };
    osv.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
    osv.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
    osv.wServicePackMajor = 0;

    const DWORDLONG dwlConditionMask = VerSetConditionMask(
      VerSetConditionMask(
        VerSetConditionMask(
          0, VER_MAJORVERSION, VER_EQUAL),
        VER_MINORVERSION, VER_EQUAL),
      VER_SERVICEPACKMAJOR, VER_EQUAL);
    if (!VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
    {
        // This QFE only ever applies to Windows Vista RTM. Windows Vista SP1 already has this fix,
        // and earlier versions of Windows do not implement WDDM
        return false;
    }

    //MEMORYSTATUSEX mex;
    //memset(&mex, 0, sizeof(mex));
    //mex.dwLength = sizeof(mex);
    //GlobalMemoryStatusEx(&mex);

    //if (mex.ullTotalVirtual >= 4294836224)
    //{
    //  // If there is 4 GB of VA space total for this process, then we are a
    //  // 32-bit Large Address Aware application running on a Windows 64-bit OS.

    //  // We could be a 32-bit Large Address Aware application running on a
    //  // Windows 32-bit OS and get up to 3 GB, but that has stability implications.
    //  // Therefore, we recommend the QFE for all 32-bit versions of the OS.

    //  // No need for the fix unless the game is pushing 4 GB of VA
    //  return false;
    //}

    tukk sysFile = "dxgkrnl.sys";

    // Ensure we are checking the system copy of the file
    char sysPath[MAX_PATH];
    GetSystemDirectory(sysPath, sizeof(sysPath));

    drx_strcat(sysPath, "\\drivers\\");
    drx_strcat(sysPath, sysFile);

    char buf[2048];
    if (!GetFileVersionInfo(sysPath, 0, sizeof(buf), buf))
    {
        // This should never happen, but we'll assume it's a newer .sys file since we've
        // narrowed the test to a Windows Vista RTM OS.
        return false;
    }

    VS_FIXEDFILEINFO* ver;
    UINT size;
    if (!VerQueryValue(buf, "\\", (uk *) &ver, &size) || size != sizeof(VS_FIXEDFILEINFO) || ver->dwSignature != 0xFEEF04BD)
    {
        // This should never happen, but we'll assume it's a newer .sys file since we've
        // narrowed the test to a Windows Vista RTM OS.
        return false;
    }

    // File major.minor.build.qfe version comparison
    //   WORD major = HIWORD( ver->dwFileVersionMS ); WORD minor = LOWORD( ver->dwFileVersionMS );
    //   WORD build = HIWORD( ver->dwFileVersionLS ); WORD qfe = LOWORD( ver->dwFileVersionLS );

    if (ver->dwFileVersionMS > MAKELONG(0, 6) || (ver->dwFileVersionMS == MAKELONG(0, 6) && ver->dwFileVersionLS >= MAKELONG(20648, 6000)))
    {
        // QFE fix version of dxgkrnl.sys is 6.0.6000.20648
        return false;
    }

    return true;
    #else
    return false; // The QFE is not required for a 64-bit native application as it has 8 TB of VA
    #endif
}

static void GetSystemMemory(uint64& totSysMem)
{
    typedef BOOL (WINAPI * FP_GlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
    FP_GlobalMemoryStatusEx pgmsex((FP_GlobalMemoryStatusEx) GetProcAddress(GetModuleHandle("kernel32"), "GlobalMemoryStatusEx"));
    if (pgmsex)
    {
        MEMORYSTATUSEX memStats;
        memStats.dwLength = sizeof(memStats);
        if (pgmsex(&memStats))
            totSysMem = memStats.ullTotalPhys;
        else
            totSysMem = 0;
    }
    else
    {
        MEMORYSTATUS memStats;
        memStats.dwLength = sizeof(memStats);
        GlobalMemoryStatus(&memStats);
        totSysMem = memStats.dwTotalPhys;
    }
}

static bool IsVistaOrAbove()
{
    typedef BOOL (WINAPI * FP_VerifyVersionInfo)(LPOSVERSIONINFOEX, DWORD, DWORDLONG);
    FP_VerifyVersionInfo pvvi((FP_VerifyVersionInfo) GetProcAddress(GetModuleHandle("kernel32"), "VerifyVersionInfoA"));

    if (pvvi)
    {
        typedef ULONGLONG (WINAPI * FP_VerSetConditionMask)(ULONGLONG, DWORD, BYTE);
        FP_VerSetConditionMask pvscm((FP_VerSetConditionMask) GetProcAddress(GetModuleHandle("kernel32"), "VerSetConditionMask"));
        assert(pvscm);

        OSVERSIONINFOEX osvi;
        memset(&osvi, 0, sizeof(osvi));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 0;
        osvi.wServicePackMajor = 0;
        osvi.wServicePackMinor = 0;

        ULONGLONG mask(0);
        mask = pvscm(mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
        mask = pvscm(mask, VER_MINORVERSION, VER_GREATER_EQUAL);
        mask = pvscm(mask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
        mask = pvscm(mask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

        if (pvvi(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR, mask))
            return true;
    }

    return false;
}

// Preferred solution to determine the number of available CPU cores, works reliably only on WinVista/Win7 32/64 and above
// See http://msdn2.microsoft.com/en-us/library/ms686694.aspx for reasons
static void GetNumCPUCoresGlpi(u32& totAvailToSystem, u32& totAvailToProcess)
{
    typedef BOOL (WINAPI * FP_GetLogicalProcessorInformation)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);
    FP_GetLogicalProcessorInformation pglpi((FP_GetLogicalProcessorInformation) GetProcAddress(GetModuleHandle("kernel32"), "GetLogicalProcessorInformation"));
    if (pglpi && IsVistaOrAbove())
    {
        u64 bufferSize(0);
        pglpi(0, &bufferSize);

        uk pBuffer(malloc(bufferSize));

        SYSTEM_LOGICAL_PROCESSOR_INFORMATION* pLogProcInfo((SYSTEM_LOGICAL_PROCESSOR_INFORMATION*) pBuffer);
        if (pLogProcInfo && pglpi(pLogProcInfo, &bufferSize))
        {
            DWORD_PTR processAffinity, systemAffinity;
            GetProcessAffinityMask(GetCurrentProcess(), &processAffinity, &systemAffinity);

            u64 numEntries(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
            for (u64 i(0); i < numEntries; ++i)
            {
                switch (pLogProcInfo[i].Relationship)
                {
                case RelationProcessorCore:
                    {
                        ++totAvailToSystem;
                        if (pLogProcInfo[i].ProcessorMask & processAffinity)
                            ++totAvailToProcess;
                    }
                    break;

                default:
                    break;
                }
            }
        }

        free(pBuffer);
    }
}

class CApicExtractor
{
public:
    CApicExtractor(u32 logProcsPerPkg = 1, u32 coresPerPkg = 1)
    {
        SetPackageTopology(logProcsPerPkg, coresPerPkg);
    }

    u8 SmtId(u8 apicId) const
    {
        return apicId & m_smtIdMask.mask;
    }

    u8 CoreId(u8 apicId) const
    {
        return (apicId & m_coreIdMask.mask) >> m_smtIdMask.width;
    }

    u8 PackageId(u8 apicId) const
    {
        return (apicId & m_pkgIdMask.mask) >> (m_smtIdMask.width + m_coreIdMask.width);
    }

    u8 PackageCoreId(u8 apicId) const
    {
        return (apicId & (m_pkgIdMask.mask | m_coreIdMask.mask)) >> m_smtIdMask.width;
    }

    u32 GetLogProcsPerPkg() const
    {
        return m_logProcsPerPkg;
    }

    u32 GetCoresPerPkg() const
    {
        return m_coresPerPkg;
    }

    void SetPackageTopology(u32 logProcsPerPkg, u32 coresPerPkg)
    {
        m_logProcsPerPkg = (u8) logProcsPerPkg;
        m_coresPerPkg = (u8) coresPerPkg;

        m_smtIdMask.width = GetMaskWidth(m_logProcsPerPkg / m_coresPerPkg);
        m_coreIdMask.width = GetMaskWidth(m_coresPerPkg);
        m_pkgIdMask.width = 8 - (m_smtIdMask.width + m_coreIdMask.width);

        m_pkgIdMask.mask = (u8) (0xFF << (m_smtIdMask.width + m_coreIdMask.width));
        m_coreIdMask.mask = (u8) ((0xFF << m_smtIdMask.width) ^ m_pkgIdMask.mask);
        m_smtIdMask.mask = (u8) ~(0xFF << m_smtIdMask.width);
    }

private:
    u8 GetMaskWidth(u8 maxIds) const
    {
        --maxIds;
        u8 msbIdx(8);
        u8 msbMask(0x80);
        while (msbMask && !(msbMask & maxIds))
        {
            --msbIdx;
            msbMask >>= 1;
        }
        return msbIdx;
    }

    struct IdMask
    {
        u8 width;
        u8 mask;
    };

    u8 m_logProcsPerPkg;
    u8 m_coresPerPkg;
    IdMask        m_smtIdMask;
    IdMask        m_coreIdMask;
    IdMask        m_pkgIdMask;
};

// Fallback solution for WinXP 32/64
static void GetNumCPUCoresApic(u32& totAvailToSystem, u32& totAvailToProcess)
{
    u32 numLogicalPerPhysical(1);
    u32 numCoresPerPhysical(1);

    i32 CPUInfo[4];
    __cpuid(CPUInfo, 0x00000001);
    if ((CPUInfo[3] & 0x10000000) != 0) // Hyperthreading / Multicore bit set
    {
        numLogicalPerPhysical = (CPUInfo[1] & 0x00FF0000) >> 16;

        if (IsIntel())
        {
            __cpuid(CPUInfo, 0x00000000);
            if (CPUInfo[0] >= 0x00000004)
            {
                __cpuidex(CPUInfo, 4, 0);
                numCoresPerPhysical = ((CPUInfo[0] & 0xFC000000) >> 26) + 1;
            }
        }
        else if (IsAMD())
        {
            __cpuid(CPUInfo, 0x80000000);
            if (CPUInfo[0] >= 0x80000008)
            {
                __cpuid(CPUInfo, 0x80000008);
                if (CPUInfo[2] & 0x0000F000)
                    numCoresPerPhysical = 1 << ((CPUInfo[2] & 0x0000F000) >> 12);
                else
                    numCoresPerPhysical = (CPUInfo[2] & 0xFF) + 1;
            }
        }
    }

    HANDLE hCurProcess(GetCurrentProcess());
    HANDLE hCurThread(GetCurrentThread());

    i32k c_maxLogicalProcessors(sizeof(DWORD_PTR) * 8);
    u8 apicIds[c_maxLogicalProcessors] = { 0 };
    u8 items(0);

    DWORD_PTR processAffinity, systemAffinity;
    GetProcessAffinityMask(hCurProcess, &processAffinity, &systemAffinity);

    if (systemAffinity == 1)
    {
        assert(numLogicalPerPhysical == 1);
        apicIds[items++] = 0;
    }
    else
    {
        if (processAffinity != systemAffinity)
            SetProcessAffinityMask(hCurProcess, systemAffinity);

        DWORD_PTR prevThreadAffinity(0);
        for (DWORD_PTR threadAffinity = 1; threadAffinity && threadAffinity <= systemAffinity; threadAffinity <<= 1)
        {
            if (systemAffinity & threadAffinity)
            {
                if (!prevThreadAffinity)
                {
                    assert(!items);
                    prevThreadAffinity = SetThreadAffinityMask(hCurThread, threadAffinity);
                }
                else
                {
                    assert(items > 0);
                    SetThreadAffinityMask(hCurThread, threadAffinity);
                }

                DrxSleep(0);

                i32 CPUInfo[4];
                __cpuid(CPUInfo, 0x00000001);
                apicIds[items++] = (u8) ((CPUInfo[1] & 0xFF000000) >> 24);
            }
        }

        SetProcessAffinityMask(hCurProcess, processAffinity);
        SetThreadAffinityMask(hCurThread, prevThreadAffinity);
        DrxSleep(0);
    }

    CApicExtractor apicExtractor(numLogicalPerPhysical, numCoresPerPhysical);

    totAvailToSystem = 0;
    {
        u8 pkgCoreIds[c_maxLogicalProcessors] = { 0 };
        for (u32 i(0); i < items; ++i)
        {
            u32 j(0);
            for (; j < totAvailToSystem; ++j)
            {
                if (pkgCoreIds[j] == apicExtractor.PackageCoreId(apicIds[i]))
                    break;
            }
            if (j == totAvailToSystem)
            {
                pkgCoreIds[j] = apicExtractor.PackageCoreId(apicIds[i]);
                ++totAvailToSystem;
            }
        }
    }

    totAvailToProcess = 0;
    {
        u8 pkgCoreIds[c_maxLogicalProcessors] = { 0 };
        for (u32 i(0); i < items; ++i)
        {
            if (processAffinity & ((DWORD_PTR) 1 << i))
            {
                u32 j(0);
                for (; j < totAvailToProcess; ++j)
                {
                    if (pkgCoreIds[j] == apicExtractor.PackageCoreId(apicIds[i]))
                        break;
                }
                if (j == totAvailToProcess)
                {
                    pkgCoreIds[j] = apicExtractor.PackageCoreId(apicIds[i]);
                    ++totAvailToProcess;
                }
            }
        }
    }
}

void Win32SysInspect::GetNumCPUCores(u32& totAvailToSystem, u32& totAvailToProcess)
{
    totAvailToSystem = 0;
    totAvailToProcess = 0;

    GetNumCPUCoresGlpi(totAvailToSystem, totAvailToProcess);

    if (!totAvailToSystem)
        GetNumCPUCoresApic(totAvailToSystem, totAvailToProcess);
}

static Win32SysInspect::DXFeatureLevel GetFeatureLevel(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
    default:
    case D3D_FEATURE_LEVEL_9_1:
        return Win32SysInspect::DXFL_9_1;
    case D3D_FEATURE_LEVEL_9_2:
        return Win32SysInspect::DXFL_9_2;
    case D3D_FEATURE_LEVEL_9_3:
        return Win32SysInspect::DXFL_9_3;
    case D3D_FEATURE_LEVEL_10_0:
        return Win32SysInspect::DXFL_10_0;
    case D3D_FEATURE_LEVEL_10_1:
        return Win32SysInspect::DXFL_10_1;
    case D3D_FEATURE_LEVEL_11_0:
        return Win32SysInspect::DXFL_11_0;
    case D3D_FEATURE_LEVEL_11_1:
        return Win32SysInspect::DXFL_11_1;
    case D3D_FEATURE_LEVEL_12_0:
        return Win32SysInspect::DXFL_12_0;
    case D3D_FEATURE_LEVEL_12_1:
        return Win32SysInspect::DXFL_12_1;
    }
}

static i32 GetDXGIAdapterOverride()
{
    #if DRX_PLATFORM_WINDOWS
    ICVar* pCVar = gEnv->pConsole ? gEnv->pConsole->GetCVar("r_overrideDXGIAdapter") : 0;
    return pCVar ? pCVar->GetIVal() : -1;
    #else
    return -1;
    #endif
}

static void LogDeviceInfo(u32 adapterIdx, const DXGI_ADAPTER_DESC1& ad, Win32SysInspect::DXFeatureLevel fl, bool displaysConnected)
{
    const bool suitableDevice = fl >= Win32SysInspect::DXFL_11_0 && displaysConnected;

    DrxLogAlways("- %s (vendor = 0x%.4x, device = 0x%.4x)", DrxStringUtils::WStrToUTF8(ad.Description).c_str(), ad.VendorId, ad.DeviceId);
    DrxLogAlways("  - Adapter index: %d", adapterIdx);
    DrxLogAlways("  - Dedicated video memory: %d MB", ad.DedicatedVideoMemory >> 20);
    DrxLogAlways("  - Feature level: %s", GetFeatureLevelAsString(fl));
    DrxLogAlways("  - Displays connected: %s", displaysConnected ? "yes" : "no");
    DrxLogAlways("  - Suitable rendering device: %s", suitableDevice ? "yes" : "no");
}

static bool FindGPU(DXGI_ADAPTER_DESC1& adapterDesc, Win32SysInspect::DXFeatureLevel& featureLevel)
{
    memset(&adapterDesc, 0, sizeof(adapterDesc));
    featureLevel = Win32SysInspect::DXFL_Undefined;

    if (!IsVistaOrAbove())
        return false;

    typedef HRESULT (WINAPI * FP_CreateDXGIFactory1)(REFIID, uk *);
    FP_CreateDXGIFactory1 pCDXGIF = (FP_CreateDXGIFactory1) GetProcAddress(LoadLibraryA("dxgi.dll"), "CreateDXGIFactory1");

    IDXGIFactory1* pFactory = 0;
    if (pCDXGIF && SUCCEEDED(pCDXGIF(__uuidof(IDXGIFactory1), (uk *) &pFactory)) && pFactory)
    {
        typedef HRESULT(WINAPI * FP_D3D11CreateDevice)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, CONST D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
        typedef HRESULT(WINAPI * FP_D3D12CreateDevice)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, uk );
        FP_D3D11CreateDevice pD3D11CD = (FP_D3D11CreateDevice)GetProcAddress(LoadLibraryA("d3d11.dll"), "D3D11CreateDevice");
        FP_D3D12CreateDevice pD3D12CD = (FP_D3D12CreateDevice)GetProcAddress(LoadLibraryA("d3d12.dll"), "D3D12CreateDevice");

        if (pD3D11CD || pD3D12CD)
        {
            i32k r_overrideDXGIAdapter = GetDXGIAdapterOverride();
            const bool logDeviceInfo = !gEnv->pRenderer && r_overrideDXGIAdapter < 0;

            if (logDeviceInfo)
                DrxLogAlways("Logging video adapters:");

            u32 nAdapter = r_overrideDXGIAdapter >= 0 ? r_overrideDXGIAdapter : 0;
            IDXGIAdapter1* pAdapter = 0;
            while (pFactory->EnumAdapters1(nAdapter, &pAdapter) != DXGI_ERROR_NOT_FOUND)
            {
                if (pAdapter)
                {
                    IDXGIOutput* pOutput = 0;
                    const bool displaysConnected = SUCCEEDED(pAdapter->EnumOutputs(0, &pOutput)) && pOutput;
                    SAFE_RELEASE(pOutput);

                    DXGI_ADAPTER_DESC1 ad;
                    pAdapter->GetDesc1(&ad);

                    if (!displaysConnected && r_overrideDXGIAdapter >= 0)
                        DrxLogAlways("No display connected to DXGI adapter override %d. Adapter cannot be used for rendering.", r_overrideDXGIAdapter);

                    HRESULT hr;
                    ID3D11Device* pDevice11 = 0;
                    ID3D12Device* pDevice12 = 0;
                    D3D_FEATURE_LEVEL levels[] = {
                        D3D_FEATURE_LEVEL_11_1,
                        D3D_FEATURE_LEVEL_11_0,
                        D3D_FEATURE_LEVEL_10_1,
                        D3D_FEATURE_LEVEL_10_0,
                        D3D_FEATURE_LEVEL_9_3,
                        D3D_FEATURE_LEVEL_9_2,
                        D3D_FEATURE_LEVEL_9_1 };
                    D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL_9_1;

                    //DirectX 12
                    if (pD3D12CD && SUCCEEDED(hr = pD3D12CD(pAdapter, deviceFeatureLevel, IID_PPV_ARGS(&pDevice12))) && pDevice12)
                    {
                        D3D12_FEATURE_DATA_FEATURE_LEVELS Featurelevels = { 0 };
                        pDevice12->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &Featurelevels, sizeof(Featurelevels));
                        const Win32SysInspect::DXFeatureLevel fl = GetFeatureLevel(Featurelevels.MaxSupportedFeatureLevel);

                        if (logDeviceInfo)
                            LogDeviceInfo(nAdapter, ad, fl, displaysConnected);

                        if (featureLevel < fl && displaysConnected)
                        {
                            adapterDesc = ad;
                            featureLevel = fl;
                        }

                        SAFE_RELEASE(pDevice12);
                    }
                    //DirectX 11.1
                    else if (pD3D11CD && SUCCEEDED(hr = pD3D11CD(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, levels, DRX_ARRAY_COUNT(levels), D3D11_SDK_VERSION, &pDevice11, &deviceFeatureLevel, NULL)) && pDevice11)
                    {
                        const Win32SysInspect::DXFeatureLevel fl = GetFeatureLevel(pDevice11->GetFeatureLevel());

                        if (logDeviceInfo)
                            LogDeviceInfo(nAdapter, ad, fl, displaysConnected);

                        if (featureLevel < fl && displaysConnected)
                        {
                            adapterDesc = ad;
                            featureLevel = fl;
                        }

                        SAFE_RELEASE(pDevice11);
                    }
                    //DirectX 11.0
                    else if (pD3D11CD && SUCCEEDED(hr = pD3D11CD(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &pDevice11, &deviceFeatureLevel, NULL)) && pDevice11)
                    {
                        const Win32SysInspect::DXFeatureLevel fl = GetFeatureLevel(pDevice11->GetFeatureLevel());

                        if (logDeviceInfo)
                            LogDeviceInfo(nAdapter, ad, fl, displaysConnected);

                        if (featureLevel < fl && displaysConnected)
                        {
                            adapterDesc = ad;
                            featureLevel = fl;
                        }

                        SAFE_RELEASE(pDevice11);
                    }

                    SAFE_RELEASE(pAdapter);
                }
                if (r_overrideDXGIAdapter >= 0)
                    break;
                ++nAdapter;
            }
        }
    }
    SAFE_RELEASE(pFactory);
    return featureLevel != Win32SysInspect::DXFL_Undefined;
}

bool Win32SysInspect::IsDX11Supported()
{
    DXGI_ADAPTER_DESC1 adapterDesc = { 0 };
    DXFeatureLevel featureLevel = Win32SysInspect::DXFL_Undefined;
    return FindGPU(adapterDesc, featureLevel) && featureLevel >= DXFL_11_0;
}

bool Win32SysInspect::IsDX12Supported()
{
    DXGI_ADAPTER_DESC1 adapterDesc = { 0 };
    DXFeatureLevel featureLevel = Win32SysInspect::DXFL_Undefined;
    return FindGPU(adapterDesc, featureLevel) && featureLevel >= DXFL_12_0;
}

bool Win32SysInspect::GetGPUInfo(tuk pName, size_t bufferSize, u32& vendorID, u32& deviceID, u32& totLocalVidMem, DXFeatureLevel& featureLevel)
{
    if (pName && bufferSize)
        pName[0] = '\0';

    vendorID = 0;
    deviceID = 0;
    totLocalVidMem = 0;
    featureLevel = Win32SysInspect::DXFL_Undefined;

    DXGI_ADAPTER_DESC1 adapterDesc = { 0 };
    const bool gpuFound = FindGPU(adapterDesc, featureLevel);
    if (gpuFound)
    {
        vendorID = adapterDesc.VendorId;
        deviceID = adapterDesc.DeviceId;

        if (pName && bufferSize)
            drx_sprintf(pName, bufferSize, "%s", DrxStringUtils::WStrToUTF8(adapterDesc.Description).c_str());

        totLocalVidMem = adapterDesc.DedicatedVideoMemory;
    }

    return gpuFound;
}

class CGPURating
{
public:
    CGPURating();
    ~CGPURating();

    i32 GetRating(u32 vendorId, u32 deviceId) const;

private:
    struct SGPUID
    {
        SGPUID(u32 vendorId, u32 deviceId)
            : vendor(vendorId)
            , device(deviceId)
        {
        }

        bool operator<(const SGPUID& rhs) const
        {
            if (vendor == rhs.vendor)
                return device < rhs.device;
            else
                return vendor < rhs.vendor;
        }

        u32 vendor;
        u32 device;
    };

    typedef std::map<SGPUID, i32> GPURatingMap;

private:
    GPURatingMap m_gpuRatingMap;

};

static size_t SafeReadLine(IDrxPak* pPak, FILE* f, tuk buffer, size_t bufferSize)
{
    assert(buffer && bufferSize);

    // read up to (bufferSize - 1) characters from stream
    size_t len(0);
    i32 c(0);
    while ((c = pPak->Getc(f)) != EOF && len < bufferSize - 1)
    {
        if (c == '\r' || c == '\n')
            break;

        buffer[len] = (char) c;
        ++len;
    }

    // write trailing zero
    buffer[len] = '\0';

    // read to end of line if necessary
    if (c != EOF && c != '\r' && c != '\n')
    {
        while ((c = pPak->Getc(f)) != EOF)
        {
            if (c == '\r' || c == '\n')
                break;
        }
    }

    // handle CR/LF for file coming from different platforms
    if (c == '\r')
    {
        c = pPak->Getc(f);
        if (c != EOF && c != '\n')
            pPak->Ungetc(c, f);
    }

    return len;
}

    #define BUILDPATH_GPURATING(x) "%engine%/config/gpu/" x

CGPURating::CGPURating()
{
    IDrxPak* pPak(gEnv->pDrxPak);

    _finddata_t fd;
    intptr_t h(pPak->FindFirst(BUILDPATH_GPURATING("*.txt"), &fd));
    if (h != -1)
    {
        do
        {
            char filename[128];
            drx_sprintf(filename, BUILDPATH_GPURATING("%s"), fd.name);

            FILE* f(pPak->FOpen(filename, "rb"));
            if (f)
            {
                size_t lineNr(0);
                while (!pPak->FEof(f))
                {
                    char line[1024];
                    line[0] = '\0';
                    size_t len(SafeReadLine(pPak, f, line, sizeof(line)));
                    ++lineNr;

                    if (len > 2 && line[0] != '/' && line[1] != '/')
                    {
                        u32 vendorId(0), deviceId(0);
                        i32 rating(0);
                        if (_snscanf(line, sizeof(line), "%x,%x,%d", &vendorId, &deviceId, &rating) == 3)
                        {
                            GPURatingMap::iterator it(m_gpuRatingMap.find(SGPUID(vendorId, deviceId)));
                            if (it == m_gpuRatingMap.end())
                                m_gpuRatingMap.insert(GPURatingMap::value_type(SGPUID(vendorId, deviceId), rating));
                            else
                                DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING,
                                           "%s line %d contains a multiple defined GPU rating!", filename, lineNr);
                        }
                        else
                            DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING,
                                       "%s line %d contains incomplete GPU rating!", filename, lineNr);
                    }
                }

                pPak->FClose(f);
            }
        }
        while (0 == pPak->FindNext(h, &fd));

        pPak->FindClose(h);
    }
}

CGPURating::~CGPURating()
{
}

i32 CGPURating::GetRating(u32 vendorId, u32 deviceId) const
{
    GPURatingMap::const_iterator it(m_gpuRatingMap.find(SGPUID(vendorId, deviceId)));
    if (it != m_gpuRatingMap.end())
        return (*it).second;
    else
        return 0;
}

i32 Win32SysInspect::GetGPURating(u32 vendorId, u32 deviceId)
{
    return 0; // All GPUs unrated as the database is out of date

    //CGPURating gpuRatingDb;
    //return gpuRatingDb.GetRating(vendorId, deviceId);
}

static i32 GetFinalSpecValue(i32 cpuRating, u32 totSysMemMB, i32 gpuRating, u32 totVidMemMB, ESystemConfigSpec maxConfigSpec)
{
    i32 sysMemRating = 1;
    if (totSysMemMB >= Win32SysInspect::SafeMemoryThreshold(12228))
        sysMemRating = 3;
    else if (totSysMemMB >= Win32SysInspect::SafeMemoryThreshold(8192))
        sysMemRating = 2;

    cpuRating = sysMemRating < cpuRating ? sysMemRating : cpuRating;

    // just a sanity check, GPU should reflect overall GPU perf including memory (higher rated GPUs usually come with enough memory)
    if (totVidMemMB < Win32SysInspect::SafeMemoryThreshold(1024))
        gpuRating = 1;

    i32 finalRating = cpuRating < gpuRating ? cpuRating : gpuRating;

    return min(finalRating, (i32) maxConfigSpec);
}

void CSystem::AutoDetectSpec(const bool detectResolution)
{
    DrxLogAlways("Running machine spec auto detect (%d bit)...", sizeof(uk ) << 3);

    char tempBuf[512];

    // get OS
    SPlatformInfo::EWinVersion winVer(SPlatformInfo::WinUndetected);
    bool is64bit(false);
    Win32SysInspect::GetOS(winVer, is64bit, tempBuf, sizeof(tempBuf));
    DrxLogAlways("- %s", tempBuf);

    // get system memory
    uint64 totSysMem(0);
    GetSystemMemory(totSysMem);
    DrxLogAlways("- System memory");
    DrxLogAlways("--- %d MB", totSysMem >> 20);

    // get CPU name
    GetCPUName(tempBuf, sizeof(tempBuf));
    TrimExcessiveWhiteSpaces(tempBuf);
    DrxLogAlways("- %s", tempBuf);

    // get number of CPU cores
    u32 numSysCores(1), numProcCores(1);
    Win32SysInspect::GetNumCPUCores(numSysCores, numProcCores);
    DrxLogAlways("--- Number of available cores: %d (out of %d)", numProcCores, numSysCores);
    i32k numLogicalProcs = gEnv->pi.numLogicalProcessors;
    DrxLogAlways("--- Number of logical processors: %d", numLogicalProcs);

    // get CPU rating
    i32k cpuRating = numLogicalProcs >= 8 ? 3 : (numLogicalProcs >= 6 ? 2 : 1);

    // get GPU info
    u32 gpuVendorId(0), gpuDeviceId(0), totVidMem(0);
    Win32SysInspect::DXFeatureLevel featureLevel(Win32SysInspect::DXFL_Undefined);
    Win32SysInspect::GetGPUInfo(tempBuf, sizeof(tempBuf), gpuVendorId, gpuDeviceId, totVidMem, featureLevel);

    DrxLogAlways("- %s (vendor = 0x%.4x, device = 0x%.4x)", tempBuf, gpuVendorId, gpuDeviceId);
    DrxLogAlways("--- Dedicated video memory: %d MB", totVidMem >> 20);
    DrxLogAlways("--- Feature level: %s", GetFeatureLevelAsString(featureLevel));

    // get GPU rating
    i32k gpuRating = (totVidMem >> 20) >= Win32SysInspect::SafeMemoryThreshold(4096) ? 3 : ((totVidMem >> 20) >= Win32SysInspect::SafeMemoryThreshold(2048) ? 2 : 1);

    // get final rating
    i32 finalSpecValue(GetFinalSpecValue(cpuRating, totSysMem >> 20, gpuRating, totVidMem >> 20, CONFIG_VERYHIGH_SPEC));
    DrxLogAlways("- Final rating: Machine class %d", finalSpecValue);

    m_sys_spec->Set(finalSpecValue);

    if (detectResolution)
    {
        if ((m_rWidth->GetFlags() & VF_WASINCONFIG) == 0)
            m_rWidth->Set(GetSystemMetrics(SM_CXFULLSCREEN));
        if ((m_rHeight->GetFlags() & VF_WASINCONFIG) == 0)
            m_rHeight->Set(GetSystemMetrics(SM_CYFULLSCREEN));
        if ((m_rFullscreen->GetFlags() & VF_WASINCONFIG) == 0)
            m_rFullscreen->Set(1);
    }
}

    #pragma warning(pop)

#else

    #include <drx3D/Sys/System.h>

void CSystem::AutoDetectSpec(const bool detectResolution)
{
}

#endif
