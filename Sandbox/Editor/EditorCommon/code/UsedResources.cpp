// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "../stdafx.h"
#include "../UsedResources.h"
#include <io.h>
#include <drx3D/Sys/File/IDrxPak.h>

CUsedResources::CUsedResources()
{
}

void CUsedResources::Add(tukk pResourceFileName)
{
	if (pResourceFileName && strcmp(pResourceFileName, ""))
	{
		files.insert(pResourceFileName);
	}
}

void CUsedResources::Validate()
{
	IDrxPak* pPak = gEnv->pDrxPak;

	for (TResourceFiles::iterator it = files.begin(); it != files.end(); ++it)
	{
		const string& filename = *it;

		bool fileExists = pPak->IsFileExist(filename);

		if (!fileExists)
		{
			tukk ext = PathUtil::GetExt(filename);
			if (!stricmp(ext, "tif") ||
			    !stricmp(ext, "hdr"))
			{
				fileExists = gEnv->pDrxPak->IsFileExist(PathUtil::ReplaceExtension(filename.GetString(), "dds"));
			}
			else if (!stricmp(ext, "dds"))
			{
				fileExists = gEnv->pDrxPak->IsFileExist(PathUtil::ReplaceExtension(filename.GetString(), "tif")) ||
				             gEnv->pDrxPak->IsFileExist(PathUtil::ReplaceExtension(filename.GetString(), "hdr"));
			}
		}

		if (!fileExists)
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Resource File %s not found,", (tukk)filename);
	}
}

