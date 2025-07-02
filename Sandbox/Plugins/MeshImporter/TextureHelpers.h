// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>

namespace TextureHelpers
{

//! CreateDrxTif tries to convert an image to DrxTif.
//! \return Returns path to DrxTif file; or empty string, if conversion failed.
string CreateDrxTif(const string& filePath, const string& settings = "");

string TranslateFilePath(const string& originalFilePath, const string& importedFilePath, const string& sourceTexturePath);

} //endns TextureHelpers

