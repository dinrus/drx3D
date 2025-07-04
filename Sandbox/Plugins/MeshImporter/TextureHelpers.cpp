// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "TextureHelpers.h"
#include "RcCaller.h"
#include "ImporterUtil.h"

#include <drx3D/CoreX/ToolsHelpers/SettingsManagerHelpers.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>
#include <DrxSystem/File/LineStreamBuffer.h>

#include <DrxSystem/IProjectManager.h>

#include <QDir>
#include <QFileInfo>

void LogPrintf(tukk szFormat, ...);

namespace Private_TextureHelpers
{

typedef void (* Listener)(tukk);

struct SLineHandler
{
	Listener listener;
	void HandleLine(tukk szLine)
	{
		listener(szLine);
	}
};

void TifConversionListener(tukk szWhat)
{
	LogPrintf("Conversion to TIF: %s\n", szWhat);
}

const bool ConvertToTIF(const string& filename, Listener listener, const wchar_t* szWorkingDirectory)
{
	const string imageMagickFolder = PathUtil::ToUnixPath(PathUtil::Make(PathUtil::GetEnginePath(), "Tools/thirdparty/ImageMagick"));

	SettingsManagerHelpers::CFixedString<wchar_t, MAX_PATH* 3> wRemoteCmdLine;
	wRemoteCmdLine.appendAscii("\"");
	wRemoteCmdLine.appendAscii(PathUtil::Make(imageMagickFolder, "convert.exe").replace("/", "\\"));
	wRemoteCmdLine.appendAscii("\" ");

	// Make sure to *not* write alpha channel.
	// "Normals" preset of RC does not work well with alpha channel.
	wRemoteCmdLine.appendAscii("-type TrueColor ");

	wRemoteCmdLine.appendAscii("\"");
	wRemoteCmdLine.appendAscii(filename.c_str());
	wRemoteCmdLine.appendAscii("\" ");

	wRemoteCmdLine.appendAscii("\"");
	wRemoteCmdLine.appendAscii(PathUtil::ReplaceExtension(filename, ".tif").c_str());
	wRemoteCmdLine.appendAscii("\"");

	SettingsManagerHelpers::CFixedString<wchar_t, MAX_PATH* 3> wEnv;
	wEnv.appendAscii("PATH=");
	wEnv.appendAscii(PathUtil::ToDosPath(imageMagickFolder));
	wEnv.appendAscii("\0", 1);
	wEnv.appendAscii("MAGICK_CODER_MODULE_PATH=");
	wEnv.appendAscii(PathUtil::ToDosPath(PathUtil::Make(imageMagickFolder, "modules\\coders")));
	wEnv.appendAscii("\0", 1);

	SLineHandler lineHandler;
	lineHandler.listener = listener;
	LineStreamBuffer lineBuffer(&lineHandler, &SLineHandler::HandleLine);

	i32 exitCode = 0;
	if (!CResourceCompilerHelper::CallProcess(szWorkingDirectory, wRemoteCmdLine.c_str(), false, &lineBuffer, exitCode, (uk )wEnv.c_str()))
	{
		// not found
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "ImageMagick convert.exe not found in %s", imageMagickFolder.c_str());
		return false;
	}

	return true;
}

bool CreateTif(const string& filePath)
{
	string workingDir = gEnv->pSystem->GetIProjectManager()->GetCurrentProjectDirectoryAbsolute();
	return ConvertToTIF(filePath, TifConversionListener, QString(workingDir.c_str()).toStdWString().c_str());
}

bool CreateDrxTifInternal(const string& filePath, const string& settings)
{
	string rcArguments;
	rcArguments += " /refresh=1";
	rcArguments += " /savesettings=\"";
	rcArguments += settings;
	rcArguments += "\"";

	CRcCaller rcCaller;
	rcCaller.SetAdditionalOptions(rcArguments);
	return rcCaller.Call(filePath);
}

} //endns Private_TextureHelpers

namespace TextureHelpers
{

string CreateDrxTif(const string& filePath, const string& settings /* = "" */)
{
	using namespace Private_TextureHelpers;

	if (!FileExists(filePath))
	{
		return string();
	}

	const bool bTif = !string("tif").compareNoCase(PathUtil::GetExt(filePath.c_str())); 

	if (!bTif && !CreateTif(filePath))
	{
		return string();
	}

	const string tifFilePath = PathUtil::ReplaceExtension(filePath, "tif");

	if (!CreateDrxTifInternal(tifFilePath, settings))
	{
		return string();
	}

	return tifFilePath;
}

string TranslateFilePath(const string& originalFilePath, const string& importedFilePath, const string& sourceTexturePath)
{	
	const QString absSourceOriginalDirectoryPath = QFileInfo(QtUtil::ToQString(originalFilePath)).dir().canonicalPath();
	const QString absSourceDirectoryPath = QFileInfo(QtUtil::ToQString(importedFilePath)).dir().canonicalPath();
	const QString absSourceTexturePath = QFileInfo(QtUtil::ToQString(sourceTexturePath)).canonicalFilePath();

	if (absSourceTexturePath.startsWith(absSourceOriginalDirectoryPath))
	{
		return QtUtil::ToString(QDir(absSourceOriginalDirectoryPath).relativeFilePath(absSourceTexturePath));
	}
	else if (absSourceTexturePath.startsWith(absSourceDirectoryPath))
	{
		// The texture is not relative to original location of FBX file, but relative to the temporary location
		// where we imported it to. This happens when the FBX SDK extracts embedded media. In this case, the
		// target path of the texture should also be relative to the FBX and has the form 'FILENAME.fbm/TEXNAME.ext'.
		return QtUtil::ToString(QDir(absSourceDirectoryPath).relativeFilePath(absSourceTexturePath));
	}
	else
	{
		return PathUtil::GetFile(sourceTexturePath);
	}
}

} //endns TextureHelpers
