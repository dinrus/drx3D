#ifndef __BCOMMON_H__
#define __BCOMMON_H__

#include <assert.h>
//#include "bLog.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3HashMap.h>

namespace bParse
{
class bMain;
class bFileData;
class bFile;
class bDNA;

// delete uk undefined
typedef struct bStructHandle
{
	i32 unused;
} bStructHandle;
typedef b3AlignedObjectArray<bStructHandle*> bListBasePtr;
typedef b3HashMap<b3HashPtr, bStructHandle*> bPtrMap;
}  // namespace bParse

#endif  //__BCOMMON_H__
