#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3Logging.h>
#ifdef __APPLE__
#include <mach-o/dyld.h> /* _NSGetExecutablePath */
#else
#ifdef _WIN32
#include <windows.h>
#else
//not Mac, not Windows, let's cross the fingers it is Linux :-)
#include <unistd.h>
#endif
#endif

#include <drx3D/Common/b3FileUtils.h>
#define D3_MAX_EXE_PATH_LEN 4096

i32 ResourcePath::getExePath(tuk path, i32 maxPathLenInBytes)
{
	i32 numBytes = 0;

#if __APPLE__
	uint32_t bufsize = uint32_t(maxPathLenInBytes);

	if (_NSGetExecutablePath(path, &bufsize) != 0)
	{
		drx3DWarning("Cannot find executable path\n");
		return false;
	}
	else
	{
		numBytes = strlen(path);
	}
#else
#ifdef _WIN32
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms683197(v=vs.85).aspx

	HMODULE hModule = GetModuleHandle(NULL);
	numBytes = GetModuleFileNameA(hModule, path, maxPathLenInBytes);

#else
	///http://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c
	numBytes = (i32)readlink("/proc/self/exe", path, maxPathLenInBytes - 1);
	if (numBytes > 0)
	{
		path[numBytes] = 0;
	}
	else
	{
		drx3DWarning("Cannot find executable path\n");
	}
#endif  //_WIN32
#endif  //__APPLE__

	return numBytes;
}

struct TempResourcePath
{
	tuk m_path;
	TempResourcePath(i32 len)
	{
		m_path = (tuk)malloc(len);
		memset(m_path, 0, len);
	}
	virtual ~TempResourcePath()
	{
		free(m_path);
	}
};

static char sAdditionalSearchPath[D3_MAX_EXE_PATH_LEN] = {0};

void ResourcePath::setAdditionalSearchPath(tukk path)
{
	if (path)
	{
		i32 len = strlen(path);
		if (len < (D3_MAX_EXE_PATH_LEN - 1))
		{
			strcpy(sAdditionalSearchPath, path);
			sAdditionalSearchPath[len] = 0;
		}
	}
	else
	{
		sAdditionalSearchPath[0] = 0;
	}
}

bool b3MyFindFile(uk userPointer, tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen)
{
	return b3FileUtils::findFile(orgFileName, relativeFileName, maxRelativeFileNameMaxLen);
}

i32 ResourcePath::findResourcePath(tukk resourceName, tuk resourcePathOut, i32 resourcePathMaxNumBytes, PFN_FIND_FILE findFile, uk userPointer)
{
	if (findFile==0)
	{
		findFile=b3MyFindFile;
	}
	//first find in a resource/<exeName> location, then in various folders within 'data' using b3FileUtils
	char exePath[D3_MAX_EXE_PATH_LEN];

	bool res = findFile(userPointer, resourceName, resourcePathOut, resourcePathMaxNumBytes);
	if (res)
	{
		return strlen(resourcePathOut);
	}

	if (sAdditionalSearchPath[0])
	{
		TempResourcePath tmpPath(resourcePathMaxNumBytes + 1024);
		tuk resourcePathIn = tmpPath.m_path;
		sprintf(resourcePathIn, "%s/%s", sAdditionalSearchPath, resourceName);
		//printf("try resource at %s\n", resourcePath);
		if (findFile(userPointer, resourcePathIn, resourcePathOut, resourcePathMaxNumBytes))
		{
			return strlen(resourcePathOut);
		}
	}

	i32 l = ResourcePath::getExePath(exePath, D3_MAX_EXE_PATH_LEN);
	if (l)
	{
		char pathToExe[D3_MAX_EXE_PATH_LEN];

		i32 exeNamePos = b3FileUtils::extractPath(exePath, pathToExe, D3_MAX_EXE_PATH_LEN);
		if (exeNamePos)
		{
			TempResourcePath tmpPath(resourcePathMaxNumBytes + 1024);
			tuk resourcePathIn = tmpPath.m_path;
			sprintf(resourcePathIn, "%s../data/%s", pathToExe, resourceName);
			//printf("try resource at %s\n", resourcePath);
			if (findFile(userPointer, resourcePathIn, resourcePathOut, resourcePathMaxNumBytes))
			{
				return strlen(resourcePathOut);
			}

			sprintf(resourcePathIn, "%s../resources/%s/%s", pathToExe, &exePath[exeNamePos], resourceName);
			//printf("try resource at %s\n", resourcePath);
			if (findFile(userPointer, resourcePathIn, resourcePathOut, resourcePathMaxNumBytes))
			{
				return strlen(resourcePathOut);
			}
			sprintf(resourcePathIn, "%s.runfiles/google3/third_party/bullet/data/%s", exePath, resourceName);
			//printf("try resource at %s\n", resourcePath);
			if (findFile(userPointer, resourcePathIn, resourcePathOut, resourcePathMaxNumBytes))
			{
				return strlen(resourcePathOut);
			}
			
		}
	}

	return 0;
}
