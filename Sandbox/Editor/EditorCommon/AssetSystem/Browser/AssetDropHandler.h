// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>

#include <memory>
#include <unordered_map>
#include <vector>

class CAsset;
struct IAssetDropImporter;

//! CAssetDropHandler delegates import requests to a set of registered asset drop handlers.
class CAssetDropHandler
{
private:
	struct SStringHasher
	{
		u32 operator()(const string& str) const
		{
			return DrxStringUtils::CalculateHashLowerCase(str.c_str());
		}
	};

	struct SImportParamsEx;

public:
	struct SImportParams
	{
		string outputDirectory;
		bool bHideDialog = false;
	};

public:
	CAssetDropHandler();

	static bool CanImportAny(const std::vector<string>& filePaths);
	static bool CanImportAny(const QStringList& filePaths);

	static std::vector<CAsset*> Import(const std::vector<string>& filePaths, const SImportParams& importParams);
	static std::vector<CAsset*> Import(const QStringList& filePaths, const SImportParams& importParams);

private:
	static std::vector<CAsset*> ImportExt(const string& ext, const std::vector<string>& filePaths, const SImportParamsEx& importParams);
};

