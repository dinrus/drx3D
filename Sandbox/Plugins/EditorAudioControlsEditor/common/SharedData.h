// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include <memory>
#include <DrxAudio/IAudioSystem.h>

namespace ACE
{
enum class EAssetType
{
	None,
	Trigger,
	Parameter,
	Switch,
	State,
	Environment,
	Preload,
	Folder,
	Library,
	NumTypes
};

enum class EPakStatus
{
	None   = 0,
	InPak  = BIT(0),
	OnDisk = BIT(1),
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EPakStatus);

enum class EItemFlags
{
	None          = 0,
	IsPlaceHolder = BIT(0),
	IsLocalized   = BIT(1),
	IsConnected   = BIT(2),
	IsContainer   = BIT(3),
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EItemFlags);

enum class EErrorCode
{
	None                     = 0,
	UnkownPlatform           = BIT(0),
	NonMatchedActivityRadius = BIT(1),
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EErrorCode);

using ControlId = DrxAudio::IdType;
static ControlId const s_aceInvalidId = 0;
using ControlIds = std::vector<ControlId>;

struct IConnection;
using ConnectionPtr = std::shared_ptr<IConnection>;

class CAsset;
using Assets = std::vector<CAsset*>;

class CControl;
using Controls = std::vector<CControl*>;

class CLibrary;
using Libraries = std::vector<CLibrary*>;

class CFolder;
using Folders = std::vector<CFolder*>;

using Platforms = std::vector<char const*>;
using FileNames = std::set<string>;
using AssetNames = std::vector<string>;

using Scope = u32;
static constexpr char const* const s_szGlobalScopeName = "global";
static constexpr Scope GlobalScopeId = DrxAudio::StringToId(s_szGlobalScopeName);

using PlatformIndexType = u16;
} //endns ACE

