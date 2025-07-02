#ifndef D3_FILE_UTILS_H
#define D3_FILE_UTILS_H

#include <stdio.h>
#include <drx3D/Common/b3Scalar.h>
#include <stddef.h>  //ptrdiff_h
#include <string.h>

struct b3FileUtils
{
	b3FileUtils()
	{
	}
	virtual ~b3FileUtils()
	{
	}

	static bool findFile(tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen)
	{
		FILE* f = 0;
		f = fopen(orgFileName, "rb");
		if (f)
		{
			//printf("original file found: [%s]\n", orgFileName);
			sprintf(relativeFileName, "%s", orgFileName);
			fclose(f);
			return true;
		}

		//printf("Trying various directories, relative to current working directory\n");
		tukk prefix[] = {"./", "./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
		i32 numPrefixes = sizeof(prefix) / sizeof(tukk);

		f = 0;
		bool fileFound = false;

		for (i32 i = 0; !f && i < numPrefixes; i++)
		{
#ifdef _MSC_VER
			sprintf_s(relativeFileName, maxRelativeFileNameMaxLen, "%s%s", prefix[i], orgFileName);
#else
			sprintf(relativeFileName, "%s%s", prefix[i], orgFileName);
#endif
			f = fopen(relativeFileName, "rb");
			if (f)
			{
				fileFound = true;
				break;
			}
		}
		if (f)
		{
			fclose(f);
		}

		return fileFound;
	}

	static tukk strip2(tukk name, tukk pattern)
	{
		size_t const patlen = strlen(pattern);
		size_t patcnt = 0;
		tukk oriptr;
		tukk patloc;
		// find how many times the pattern occurs in the original string
		for (oriptr = name; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
		{
			patcnt++;
		}
		return oriptr;
	}

	static i32 extractPath(tukk fileName, tuk path, i32 maxPathLength)
	{
		tukk stripped = strip2(fileName, "/");
		stripped = strip2(stripped, "\\");

		ptrdiff_t len = stripped - fileName;
		drx3DAssert((len + 1) < maxPathLength);

		if (len && ((len + 1) < maxPathLength))
		{
			for (i32 i = 0; i < len; i++)
			{
				path[i] = fileName[i];
			}
			path[len] = 0;
		}
		else
		{
			len = 0;
			drx3DAssert(maxPathLength > 0);
			if (maxPathLength > 0)
			{
				path[len] = 0;
			}
		}
		return len;
	}

	static char toLowerChar(const char t)
	{
		if (t >= (char)'A' && t <= (char)'Z')
			return t + ((char)'a' - (char)'A');
		else
			return t;
	}

	static void toLower(tuk str)
	{
		i32 len = strlen(str);
		for (i32 i = 0; i < len; i++)
		{
			str[i] = toLowerChar(str[i]);
		}
	}

	/*static tukk strip2(tukk name, tukk pattern)
	{
		size_t const patlen = strlen(pattern);
		size_t patcnt = 0;
		tukk  oriptr;
		tukk  patloc;
		// find how many times the pattern occurs in the original string
		for (oriptr = name; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
		{
			patcnt++;
		}
		return oriptr;
	}
	*/
};
#endif  //D3_FILE_UTILS_H
