// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/FileUtils.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/StackString.h>

namespace sxema
{
namespace FileUtils
{
void EnumFilesInFolder(tukk szFolderName, tukk szExtension, FileEnumCallback callback, const FileEnumFlags& flags)
{
	SXEMA_CORE_ASSERT(szFolderName);
	if (szFolderName)
	{
		SXEMA_CORE_ASSERT(callback);
		if (callback)
		{
			if (flags.Check(EFileEnumFlags::Recursive))
			{
				CStackString searchPath = szFolderName;
				searchPath.append("/*.*");

				_finddata_t findData;
				intptr_t handle = gEnv->pDrxPak->FindFirst(searchPath.c_str(), &findData);
				if (handle >= 0)
				{
					do
					{
						if ((findData.name[0] != '.') && ((findData.attrib & _A_SUBDIR) != 0))
						{
							if (!flags.Check(EFileEnumFlags::IgnoreUnderscoredFolders) || (findData.name[0] != '_'))
							{
								CStackString subName = szFolderName;
								subName.append("/");
								subName.append(findData.name);
								EnumFilesInFolder(subName.c_str(), szExtension, callback, flags);
							}
						}
					}
					while (gEnv->pDrxPak->FindNext(handle, &findData) >= 0);
				}
			}

			CStackString searchPath = szFolderName;
			searchPath.append("/");
			searchPath.append(szExtension ? szExtension : "*.*");

			_finddata_t findData;
			intptr_t handle = gEnv->pDrxPak->FindFirst(searchPath.c_str(), &findData);
			if (handle >= 0)
			{
				do
				{
					if ((findData.name[0] != '.') && ((findData.attrib & _A_SUBDIR) == 0))
					{
						CStackString subName = szFolderName;
						subName.append("/");
						subName.append(findData.name);
						callback(subName.c_str(), findData.attrib);
					}
				}
				while (gEnv->pDrxPak->FindNext(handle, &findData) >= 0);
			}
		}
	}
}
} // FileUtils
} // sxema
