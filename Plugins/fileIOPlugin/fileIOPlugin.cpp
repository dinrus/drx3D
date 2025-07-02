#include "fileIOPlugin.h"
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Plugins/PluginContext.h>
#include <stdio.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3HashMap.h>
#include <string.h> //memcpy/strlen
#ifndef D3_EXCLUDE_DEFAULT_FILEIO
#include <drx3D/Common/DefaultFileIO.h>
#endif //D3_EXCLUDE_DEFAULT_FILEIO


#ifdef D3_USE_ZIPFILE_FILEIO
#include "zipFileIO.h"
#endif //D3_USE_ZIPFILE_FILEIO


#ifdef D3_USE_CNS_FILEIO
#include "CNSFileIO.h"
#endif //D3_USE_CNS_FILEIO

#define D3_MAX_FILEIO_INTERFACES 1024

struct WrapperFileHandle
{
	CommonFileIOInterface* childFileIO;
	i32 m_childFileHandle;
};

struct InMemoryFile
{
	tuk m_buffer;
	i32 m_fileSize;
};

struct InMemoryFileAccessor
{
	InMemoryFile* m_file;
	i32 m_curPos;
};

struct InMemoryFileIO : public CommonFileIOInterface
{
	b3HashMap<b3HashString,InMemoryFile*> m_fileCache;
	InMemoryFileAccessor m_fileHandles[D3_MAX_FILEIO_INTERFACES];
	i32 m_numAllocs;
	i32 m_numFrees;

	InMemoryFileIO()
		:CommonFileIOInterface(eInMemoryFileIO,0)
	{
		m_numAllocs=0;
		m_numFrees=0;

		for (i32 i=0;i<D3_FILEIO_MAX_FILES;i++)
		{
			m_fileHandles[i].m_curPos = 0;
			m_fileHandles[i].m_file = 0;
		}
	}

	virtual ~InMemoryFileIO()
	{
		clearCache();
		if (m_numAllocs != m_numFrees)
		{
			printf("Ошибка: InMemoryFile::~InMemoryFileIO (numAllocs %d numFrees %d\n", m_numAllocs, m_numFrees);
		}
	}
	void clearCache()
	{
		for (i32 i=0;i<m_fileCache.size();i++)
		{
			InMemoryFile** memPtr = m_fileCache.getAtIndex(i);
			if (memPtr && *memPtr)
			{
				InMemoryFile* mem = *memPtr;
				freeBuffer(mem->m_buffer);
				m_numFrees++;
				delete (mem);
				m_numFrees++;
			}
		}
		m_fileCache.clear();
	}

	tuk allocateBuffer(i32 len)
	{
		tuk buffer = 0;
		if (len)
		{
			m_numAllocs++;
			buffer = new char[len];
		}
		return buffer;
	}

	void freeBuffer(tuk buffer)
	{
		delete[] buffer;
	}

	virtual i32 registerFile(tukk fileName, tuk buffer, i32 len)
	{
		m_numAllocs++;
		InMemoryFile* f = new InMemoryFile();
		f->m_buffer = buffer;
		f->m_fileSize = len;
		b3HashString key(fileName);
		m_fileCache.insert(key,f);
		return 0;
	}

	void removeFileFromCache(tukk fileName)
	{
		InMemoryFile* f = getInMemoryFile(fileName);
		if (f)
		{
			m_fileCache.remove(fileName);
			freeBuffer(f->m_buffer);
			delete (f);
		}
	}

	InMemoryFile* getInMemoryFile(tukk fileName)
	{
		InMemoryFile** fPtr = m_fileCache[fileName];
		if (fPtr && *fPtr)
		{
			return *fPtr;
		}
		return 0;
	}

	virtual i32 fileOpen(tukk fileName, tukk mode)
	{
		//search a free slot
		i32 slot = -1;
		for (i32 i=0;i<D3_FILEIO_MAX_FILES;i++)
		{
			if (m_fileHandles[i].m_file==0)
			{
				slot=i;
				break;
			}
		}
		if (slot>=0)
		{
			InMemoryFile* f = getInMemoryFile(fileName);
			if (f)
			{
				m_fileHandles[slot].m_curPos = 0;
				m_fileHandles[slot].m_file = f;
			} else
			{
				slot=-1;
			}
		}
		//printf("InMemoryFileIO fileOpen %s, %d\n", fileName, slot);
		return slot;
	}
	virtual i32 fileRead(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			InMemoryFileAccessor& f = m_fileHandles[fileHandle];
			if (f.m_file)
			{
				//if (numBytes>1)
				//	printf("curPos = %d\n", f.m_curPos);
				if (f.m_curPos+numBytes <= f.m_file->m_fileSize)
				{
					memcpy(destBuffer,f.m_file->m_buffer+f.m_curPos,numBytes);
					f.m_curPos+=numBytes;
					//if (numBytes>1)
					//	printf("read %d bytes, now curPos = %d\n", numBytes, f.m_curPos);
					return numBytes;
				} else
				{
					if (numBytes!=1)
					{
						printf("InMemoryFileIO::fileRead Attempt to read beyond end of file\n");
					}
				}

			}
		}
		return 0;
	}

	virtual i32 fileWrite(i32 fileHandle,tukk sourceBuffer, i32 numBytes)
	{
		return 0;
	}
	virtual void fileClose(i32 fileHandle)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			InMemoryFileAccessor& f = m_fileHandles[fileHandle];
			if (f.m_file)
			{
				m_fileHandles[fileHandle].m_file = 0;
				m_fileHandles[fileHandle].m_curPos = 0;
				//printf("InMemoryFileIO fileClose %d\n", fileHandle);
			}
		}
	}
	virtual bool findResourcePath(tukk fileName,  tuk resourcePathOut, i32 resourcePathMaxNumBytes)
	{
		InMemoryFile* f = getInMemoryFile(fileName);
		i32 fileNameLen = strlen(fileName);
		if (f && fileNameLen<(resourcePathMaxNumBytes-1))
		{
			memcpy(resourcePathOut, fileName, fileNameLen);
			resourcePathOut[fileNameLen]=0;
			return true;
		}
		return false;
	}
	virtual tuk readLine(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		i32 numRead = 0;
		i32 endOfFile = 0;
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES )
		{
			InMemoryFileAccessor& f = m_fileHandles[fileHandle];
			if (f.m_file)
			{
				//return ::fgets(destBuffer, numBytes, m_fileHandles[fileHandle]);
				char c = 0;
				do
				{
					i32 bytesRead = fileRead(fileHandle,&c,1);
					if (bytesRead != 1)
					{
						endOfFile = 1;
						c=0;
					}
					if (c && c!='\n')
					{
						if (c!=13)
						{
							destBuffer[numRead++]=c;
						} else
						{
							destBuffer[numRead++]=0;
						}
					}
				} while (c != 0 && c != '\n' && numRead<(numBytes-1));
			}
		}
		if (numRead==0 && endOfFile)
		{
			return 0;
		}

		if (numRead<numBytes)
		{
			if (numRead >=0)
			{
				destBuffer[numRead]=0;
			}
			return &destBuffer[0];
		} else
		{
			if (endOfFile==0)
			{
				printf("InMemoryFileIO::readLine readLine warning: numRead=%d, numBytes=%d\n", numRead, numBytes);
			}
		}
		return 0;
	}
	virtual i32 getFileSize(i32 fileHandle)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES )
		{

			InMemoryFileAccessor& f = m_fileHandles[fileHandle];
			if (f.m_file)
			{
				return f.m_file->m_fileSize;
			}
		}
		return 0;
	}

	virtual void enableFileCaching(bool enable)
	{
		(void)enable;
	}
};

struct WrapperFileIO : public CommonFileIOInterface
{
	CommonFileIOInterface* m_availableFileIOInterfaces[D3_MAX_FILEIO_INTERFACES];
	i32 m_numWrapperInterfaces;

	WrapperFileHandle m_wrapperFileHandles[D3_MAX_FILEIO_INTERFACES];
	InMemoryFileIO m_cachedFiles;
	bool m_enableFileCaching;

	WrapperFileIO()
		:CommonFileIOInterface(0,0),
		m_numWrapperInterfaces(0),
		m_enableFileCaching(true)
	{
		for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
		{
			m_availableFileIOInterfaces[i]=0;
			m_wrapperFileHandles[i].childFileIO=0;
			m_wrapperFileHandles[i].m_childFileHandle=0;
		}
		//addFileIOInterface(&m_cachedFiles);
	}

	virtual ~WrapperFileIO()
	{
		for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
		{
			removeFileIOInterface(i);
		}
		m_cachedFiles.clearCache();
	}

	i32 addFileIOInterface(CommonFileIOInterface* fileIO)
	{
		i32 result = -1;
		for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
		{
			if (m_availableFileIOInterfaces[i]==0)
			{
				m_availableFileIOInterfaces[i]=fileIO;
				result = i;
				break;
			}
		}
		return result;
	}

	CommonFileIOInterface* getFileIOInterface(i32 fileIOIndex)
	{
		if (fileIOIndex>=0 && fileIOIndex<D3_MAX_FILEIO_INTERFACES)
		{
			return m_availableFileIOInterfaces[fileIOIndex];
		}
		return 0;
	}
	void removeFileIOInterface(i32 fileIOIndex)
	{
		if (fileIOIndex>=0 && fileIOIndex<D3_MAX_FILEIO_INTERFACES)
		{
			if (m_availableFileIOInterfaces[fileIOIndex])
			{
				delete m_availableFileIOInterfaces[fileIOIndex];
				m_availableFileIOInterfaces[fileIOIndex]=0;
			}
		}
	}

	virtual i32 fileOpen(tukk fileName, tukk mode)
	{

		//find an available wrapperFileHandle slot
		i32 wrapperFileHandle=-1;
		i32 slot = -1;
		for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
		{
			if (m_wrapperFileHandles[i].childFileIO==0)
			{
				slot=i;
				break;
			}
		}
		if (slot>=0)
		{
			//first check the cache
			i32 cacheHandle = m_cachedFiles.fileOpen(fileName, mode);
			if (cacheHandle>=0)
			{
				m_cachedFiles.fileClose(cacheHandle);
			}
			if (cacheHandle<0)
			{
				for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
				{
					CommonFileIOInterface* childFileIO=m_availableFileIOInterfaces[i];
					if (childFileIO)
					{
						i32 childHandle = childFileIO->fileOpen(fileName, mode);
						if (childHandle>=0)
						{
							if (m_enableFileCaching)
							{
								i32 fileSize = childFileIO->getFileSize(childHandle);
								tuk buffer = 0;
								if (fileSize)
								{
									buffer = m_cachedFiles.allocateBuffer(fileSize);
									if (buffer)
									{
										i32 readBytes = childFileIO->fileRead(childHandle, buffer, fileSize);
										if (readBytes!=fileSize)
										{
											if (readBytes<fileSize)
												{
												fileSize = readBytes;
											} else
											{
												printf("WrapperFileIO ошибка: reading more bytes (%d) then reported file size (%d) of file %s.\n", readBytes, fileSize, fileName);
											}
										}
									} else
									{
										fileSize=0;
									}
								}

								//potentially register a zero byte file, or files that only can be read partially

								m_cachedFiles.registerFile(fileName, buffer, fileSize);
							}

							childFileIO->fileClose(childHandle);
							break;
						}
					}
				}
			}

			{
				i32 childHandle = m_cachedFiles.fileOpen(fileName, mode);
				if (childHandle>=0)
				{
					wrapperFileHandle = slot;
					m_wrapperFileHandles[slot].childFileIO = &m_cachedFiles;
					m_wrapperFileHandles[slot].m_childFileHandle = childHandle;
				} else
				{
					//figure out what wrapper interface to use
					//use the first one that can open the file
					for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
					{
						CommonFileIOInterface* childFileIO=m_availableFileIOInterfaces[i];
						if (childFileIO)
						{
							i32 childHandle = childFileIO->fileOpen(fileName, mode);
							if (childHandle>=0)
							{
								wrapperFileHandle = slot;
								m_wrapperFileHandles[slot].childFileIO = childFileIO;
								m_wrapperFileHandles[slot].m_childFileHandle = childHandle;
								break;
							}
						}
					}
				}
			}
		}
		return wrapperFileHandle;
	}

	virtual i32 fileRead(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		i32 fileReadResult=-1;
		if (fileHandle>=0 && fileHandle<D3_MAX_FILEIO_INTERFACES)
		{
			if (m_wrapperFileHandles[fileHandle].childFileIO)
			{
				fileReadResult = m_wrapperFileHandles[fileHandle].childFileIO->fileRead(
					m_wrapperFileHandles[fileHandle].m_childFileHandle, destBuffer, numBytes);
			}
		}
		return fileReadResult;
	}

	virtual i32 fileWrite(i32 fileHandle,tukk sourceBuffer, i32 numBytes)
	{
		//todo
		return -1;
	}
	virtual void fileClose(i32 fileHandle)
	{
		if (fileHandle>=0 && fileHandle<D3_MAX_FILEIO_INTERFACES)
		{
			if (m_wrapperFileHandles[fileHandle].childFileIO)
			{
				m_wrapperFileHandles[fileHandle].childFileIO->fileClose(
					m_wrapperFileHandles[fileHandle].m_childFileHandle);
				m_wrapperFileHandles[fileHandle].childFileIO = 0;
				m_wrapperFileHandles[fileHandle].m_childFileHandle = -1;
			}
		}
	}
	virtual bool findResourcePath(tukk fileName,  tuk resourcePathOut, i32 resourcePathMaxNumBytes)
	{
		if (m_cachedFiles.findResourcePath(fileName, resourcePathOut, resourcePathMaxNumBytes))
			return true;

		bool found = false;
		for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
		{
			if (m_availableFileIOInterfaces[i])
			{
				found = m_availableFileIOInterfaces[i]->findResourcePath(fileName, resourcePathOut, resourcePathMaxNumBytes);
			}
			if (found)
				break;
		}
		return found;
	}
	virtual tuk readLine(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		tuk result = 0;

		if (fileHandle>=0 && fileHandle<D3_MAX_FILEIO_INTERFACES)
		{
			if (m_wrapperFileHandles[fileHandle].childFileIO)
			{
				result = m_wrapperFileHandles[fileHandle].childFileIO->readLine(
					m_wrapperFileHandles[fileHandle].m_childFileHandle,
					destBuffer, numBytes);
			}
		}
		return result;
	}
	virtual i32 getFileSize(i32 fileHandle)
	{
		i32 numBytes = 0;

		if (fileHandle>=0 && fileHandle<D3_MAX_FILEIO_INTERFACES)
		{
			if (m_wrapperFileHandles[fileHandle].childFileIO)
			{
				numBytes = m_wrapperFileHandles[fileHandle].childFileIO->getFileSize(
					m_wrapperFileHandles[fileHandle].m_childFileHandle);
			}
		}
		return numBytes;
	}

	virtual void enableFileCaching(bool enable)
	{
		m_enableFileCaching = enable;
		if (!enable)
		{
			m_cachedFiles.clearCache();
		}
	}

};


struct FileIOClass
{
	i32 m_testData;

	WrapperFileIO m_fileIO;

	FileIOClass()
		: m_testData(42),
		m_fileIO()
	{
	}
	virtual ~FileIOClass()
	{
	}
};

DRX3D_SHARED_API i32 initPlugin_fileIOPlugin(struct PluginContext* context)
{
	FileIOClass* obj = new FileIOClass();
	context->m_userPointer = obj;

#ifndef D3_EXCLUDE_DEFAULT_FILEIO
	obj->m_fileIO.addFileIOInterface(new DefaultFileIO());
#endif //D3_EXCLUDE_DEFAULT_FILEIO


	return SHARED_MEMORY_MAGIC_NUMBER;
}


DRX3D_SHARED_API i32 executePluginCommand_fileIOPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments)
{
	i32 result=-1;

	FileIOClass* obj = (FileIOClass*)context->m_userPointer;

	printf("text argument:%s\n", arguments->m_text);
	printf("i32 args: [");

	//remove a fileIO type
	if (arguments->m_numInts==1)
	{
		i32 fileIOIndex = arguments->m_ints[0];
		obj->m_fileIO.removeFileIOInterface(fileIOIndex);
	}

	if (arguments->m_numInts==2)
	{
		i32 action = arguments->m_ints[0];
		switch (action)
		{
			case eAddFileIOAction:
			{
				//if the fileIO already exists, skip this action
				i32 fileIOType = arguments->m_ints[1];
				bool alreadyExists = false;

				for (i32 i=0;i<D3_MAX_FILEIO_INTERFACES;i++)
				{
					CommonFileIOInterface* fileIO = obj->m_fileIO.getFileIOInterface(i);
					if (fileIO)
					{
						if (fileIO->m_fileIOType == fileIOType)
						{
							if (fileIO->m_pathPrefix && strcmp(fileIO->m_pathPrefix,arguments->m_text)==0)
							{
								result = i;
								alreadyExists = true;
								break;
							}
						}
					}
				}


				//create new fileIO interface
				if (!alreadyExists)
				{
					switch (fileIOType)
					{
						case ePosixFileIO:
						{
	#ifdef D3_EXCLUDE_DEFAULT_FILEIO
							printf("ePosixFileIO is not enabled in this build.\n");
	#else
							result = obj->m_fileIO.addFileIOInterface(new DefaultFileIO(ePosixFileIO, arguments->m_text));
	#endif
							break;
						}
						case eZipFileIO:
						{
	#ifdef D3_USE_ZIPFILE_FILEIO
							if (arguments->m_text[0])
							{
								result = obj->m_fileIO.addFileIOInterface(new ZipFileIO(eZipFileIO, arguments->m_text, &obj->m_fileIO));
							}
	#else
							printf("eZipFileIO is not enabled in this build.\n");
	#endif
							break;
						}
						case eCNSFileIO:
						{
	#ifdef D3_USE_CNS_FILEIO
							result = obj->m_fileIO.addFileIOInterface(new CNSFileIO(eCNSFileIO, arguments->m_text));
	#else//D3_USE_CNS_FILEIO
							printf("CNSFileIO is not enabled in this build.\n");
	#endif //D3_USE_CNS_FILEIO
							break;
						}
						default:
						{
						}
					}//switch (fileIOType)
				}//if (!alreadyExists)
				break;
			}
			case eRemoveFileIOAction:

			{
				//remove fileIO interface
				i32 fileIOIndex = arguments->m_ints[1];
				obj->m_fileIO.removeFileIOInterface(fileIOIndex);
				result = fileIOIndex;
				break;
			}
			default:
			{
				printf("executePluginCommand_fileIOPlugin: unknown action\n");
			}
		}
	}
	return result;
}

DRX3D_SHARED_API struct CommonFileIOInterface* getFileIOFunc_fileIOPlugin(struct PluginContext* context)
{
	FileIOClass* obj = (FileIOClass*)context->m_userPointer;
	return &obj->m_fileIO;
}

DRX3D_SHARED_API void exitPlugin_fileIOPlugin(struct PluginContext* context)
{
	FileIOClass* obj = (FileIOClass*)context->m_userPointer;
	delete obj;
	context->m_userPointer = 0;
}
