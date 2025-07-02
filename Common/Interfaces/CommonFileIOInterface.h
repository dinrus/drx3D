#ifndef COMMON_FILE_IO_INTERFACE_H
#define COMMON_FILE_IO_INTERFACE_H

#include <drxtypes.h>

struct CommonFileIOInterface
{
	i32 m_fileIOType;
	tukk m_pathPrefix;

	CommonFileIOInterface(i32 fileIOType, tukk pathPrefix)
		:m_fileIOType(fileIOType),
		m_pathPrefix(pathPrefix)
	{
	}

	virtual ~CommonFileIOInterface()
	{
	}
	virtual i32 fileOpen(tukk fileName, tukk mode)=0;
	virtual i32 fileRead(i32 fileHandle, tuk destBuffer, i32 numBytes)=0;
	virtual i32 fileWrite(i32 fileHandle,tukk sourceBuffer, i32 numBytes)=0;
	virtual void fileClose(i32 fileHandle)=0;
	virtual bool findResourcePath(tukk fileName,  tuk resourcePathOut, i32 resourcePathMaxNumBytes)=0;
	virtual tuk readLine(i32 fileHandle, tuk destBuffer, i32 numBytes)=0;
	virtual i32 getFileSize(i32 fileHandle)=0;
	virtual void enableFileCaching(bool enable) = 0;
};

#endif //COMMON_FILE_IO_INTERFACE_H