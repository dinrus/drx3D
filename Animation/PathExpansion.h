// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace PathExpansion
{
// Expand patterns into paths. An example of a pattern before expansion:
// animations/facial/idle_{0,1,2,3}.fsq
// {} is used to specify options for parts of the string.
// output must point to buffer that is at least as large as the pattern.
void SelectRandomPathExpansion(tukk pattern, tuk output);
void EnumeratePathExpansions(tukk pattern, void (* enumCallback)(uk userData, tukk expansion), uk userData);
}
