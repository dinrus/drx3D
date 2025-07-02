// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct SAnimSettings;

namespace CafCompressionHelper
{
bool CompressAnimationForPreview(string* outputPath, string* errorMessage, const string& animationPath, const SAnimSettings& animSettings, bool ignorePresets, i32 sessionIndex);
bool MoveCompressionResult(string* errorMessage, tukk createdFile, tukk destinationFile);
void CleanUpCompressionResult(tukk createdFile);
bool CompressAnimation(const string& animationPath, string& outErrorMessage, bool* failReported);

bool CheckIfNeedUpdate(tukk animationPath);
}

