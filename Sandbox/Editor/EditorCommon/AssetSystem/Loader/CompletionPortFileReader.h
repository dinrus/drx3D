// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace AssetLoader
{

struct IFileReader;

std::unique_ptr<IFileReader> CreateCompletionPortFileReader();

}
