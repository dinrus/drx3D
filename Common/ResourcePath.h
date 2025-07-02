#ifndef _D3_RESOURCE_PATH_H
#define _D3_RESOURCE_PATH_H

#include <drxtypes.h>
#include <string>

typedef bool (* PFN_FIND_FILE)(uk userPointer, tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen);

class ResourcePath
{
public:
	static i32 getExePath(tuk path, i32 maxPathLenInBytes);
	static i32 findResourcePath(tukk resourceName, tuk resourcePathOut, i32 resourcePathMaxNumBytes, PFN_FIND_FILE findFile, uk userPointer=0);
	static void setAdditionalSearchPath(tukk path);
};
#endif
