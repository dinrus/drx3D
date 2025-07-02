// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
{
static constexpr char const* s_szImplFolderName = "portaudio";

// XML tags
static constexpr char const* s_szFileTag = "Sample";

// XML attributes
static constexpr char const* s_szPathAttribute = "path";
static constexpr char const* s_szLoopCountAttribute = "loop_count";

// XML values
static constexpr char const* s_szStartValue = "start";
static constexpr char const* s_szStopValue = "stop";
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
