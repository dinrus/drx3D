// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>
#include <memory>

namespace MeshImporter
{
	class CSceneManager;
	struct IImportFile;
}

class CTextureManager
{
private:
	struct STexture
	{
		string sourceFilePath;  // Texture file path read from source file.
		string targetFilePath;  // New file path of this texture (relative). Can be appended to target directory.
		string rcSettings;

		STexture();
	};
public:
	typedef STexture* TextureHandle;

	CTextureManager() {}
	CTextureManager(const CTextureManager& other) = delete;
	CTextureManager& operator=(const CTextureManager& other) = delete;

	void Init(const MeshImporter::CSceneManager& sceneManager);

	void          SetRcSettings(TextureHandle tex, const string& rcSettings);

	i32           GetTextureCount() const;
	TextureHandle GetTextureFromIndex(i32 index);
	TextureHandle GetTextureFromSourcePath(const string& name);

	string        GetTargetPath(TextureHandle tex) const;
	string        GetSourcePath(TextureHandle tex) const;
	string        GetRcSettings(TextureHandle tex) const;

	bool CopyTextures(const string& dir);

private:
	bool          AddTexture(const MeshImporter::IImportFile& importFile, const string& originalFilePath, const string& rcSettings);
	bool CopyTexture(TextureHandle tex, const string& dir) const;


private:
	string        TranslateFilePath(const MeshImporter::IImportFile& importFile, const string& sourcePath) const;

	string m_sourceDirectory; // Directory of source FBX file.
	std::vector<std::unique_ptr<STexture>> m_textures;
};

