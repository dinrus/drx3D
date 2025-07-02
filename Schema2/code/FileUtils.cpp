// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>

#include <drx3D/Schema2/FileUtils.h>

#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Schema2/ILog.h>

namespace sxema2
{
	namespace FileUtils
	{
		//////////////////////////////////////////////////////////////////////////
		void EnumFilesInFolder(tukk szFolderName, tukk szExtension, FileEnumCallback callback, EFileEnumFlags flags)
		{
			LOADING_TIME_PROFILE_SECTION_ARGS(szFolderName);
			SXEMA2_SYSTEM_ASSERT(szFolderName);
			if(szFolderName)
			{
				SXEMA2_SYSTEM_ASSERT(callback);
				if(callback)
				{
					if((flags & EFileEnumFlags::Recursive) != 0)
					{
						stack_string	searchPath = szFolderName;
						searchPath.append("/*.*");
						_finddata_t	findData;
						intptr_t		handle = gEnv->pDrxPak->FindFirst(searchPath.c_str(), &findData);
						if(handle >= 0)
						{
							do
							{
								if((findData.name[0] != '.') && ((findData.attrib & _A_SUBDIR) != 0))
								{
									if(((flags & EFileEnumFlags::IgnoreUnderscoredFolders) == 0) || (findData.name[0] != '_'))
									{
										stack_string	subName = szFolderName;
										subName.append("/");
										subName.append(findData.name);
										EnumFilesInFolder(subName.c_str(), szExtension, callback, flags);
									}
								}
							} while(gEnv->pDrxPak->FindNext(handle, &findData) >= 0);
						}
					}
					stack_string	searchPath = szFolderName;
					searchPath.append("/");
					searchPath.append(szExtension ? szExtension : "*.*");
					_finddata_t	findData;
					intptr_t		handle = gEnv->pDrxPak->FindFirst(searchPath.c_str(), &findData);
					if(handle >= 0)
					{
						do
						{
							if((findData.name[0] != '.') && ((findData.attrib & _A_SUBDIR) == 0))
							{
								stack_string	subName = szFolderName;
								subName.append("/");
								subName.append(findData.name);
								callback(subName.c_str(), findData.attrib);
							}
						} while(gEnv->pDrxPak->FindNext(handle, &findData) >= 0);
					}
				}
			}
		}
	}
}
