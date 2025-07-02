#ifndef __BCOMMON_H__
#define __BCOMMON_H__

#include <assert.h>
//#include "bLog.h"
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/HashMap.h>

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
typedef AlignedObjectArray<bStructHandle*> bListBasePtr;
typedef HashMap<HashPtr, bStructHandle*> bPtrMap;
}  // namespace bParse

#endif  //__BCOMMON_H__
