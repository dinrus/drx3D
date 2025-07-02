// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace AsseThumbnailsGenerator
{

void GenerateThumbnailsAsync(const string& folder, const std::function<void()>& finalize = std::function<void()>());

};
