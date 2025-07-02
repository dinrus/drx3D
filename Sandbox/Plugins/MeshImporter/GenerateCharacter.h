// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/CoreX/String/DrxString.h>

namespace FbxTool
{

class CScene;

} //endns FbxTool

// Utility functions and classes that import a FBX scene and generate files with default settings in a sequential fashion.
// These functions are used, for example, in the CharacterTool drag&drop feature and the AssetBrowser import.

struct ICharacterGenerator
{
public:
	virtual ~ICharacterGenerator() {}

	virtual void CreateMaterial() = 0;
	virtual void CreateSkeleton() = 0;
	virtual void CreateSkin() = 0;
	virtual void CreateAllAnimations() = 0;
	virtual void CreateCharacterParameters() = 0;
	virtual void CreateCharacterDefinition() = 0;

	virtual std::vector<string> GetOutputFiles() const = 0;

	virtual const FbxTool::CScene* GetScene() const = 0;
	virtual FbxTool::CScene* GetScene() = 0;
};

std::unique_ptr<ICharacterGenerator> CreateCharacterGenerator(const string& inputFilePath);

std::vector<string> GenerateMaterialAndTextures(const string& inputFilePath);
std::vector<string> GenerateAllAnimations(const string& inputFilePath);
