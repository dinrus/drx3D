// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Item.h"

namespace ACE
{
namespace Impl
{
namespace Fmod
{
static string const s_soundBanksFolderName = "Banks";
static string const s_eventsFolderName = "Events";
static string const s_parametersFolderName = "Parameters";
static string const s_snapshotsFolderName = "Snapshots";
static string const s_returnsFolderName = "Returns";
static string const s_vcasFolderName = "VCAs";

namespace Utils
{
ControlId GetId(EItemType const type, string const& name, CItem* const pParent, CItem const& rootItem);
string    GetPathName(CItem const* const pItem, CItem const& rootItem);
string    GetTypeName(EItemType const type);
} //endns Utils
} //endns Fmod
} //endns Impl
} //endns ACE

