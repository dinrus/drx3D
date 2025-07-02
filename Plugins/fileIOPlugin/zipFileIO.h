
#include "minizip/unzip.h"

#define D3_ZIP_FILEIO_MAX_FILES   1024

struct ZipFileIO : public CommonFileIOInterface
{
	STxt m_zipfileName;

	unzFile	m_fileHandles[D3_ZIP_FILEIO_MAX_FILES ];
	i32 m_numFileHandles;
	unzFile m_zipfile;
	voidpf m_stream;
	unz_global_info m_global_info;
	bool m_memoryFile;
	b3AlignedObjectArray<char> m_buffer;
	
	ZipFileIO(i32 fileIOType, tukk zipfileName, CommonFileIOInterface* wrapperFileIO)
		:CommonFileIOInterface(fileIOType,0),
		m_zipfileName(zipfileName),
		m_numFileHandles(0),
		m_stream(0),
		m_memoryFile(false)
	{
		m_pathPrefix = m_zipfileName.c_str();
		for (i32 i=0;i<D3_ZIP_FILEIO_MAX_FILES ;i++)
		{
			m_fileHandles[i]=0;
		}
		m_zipfile = unzOpen(m_zipfileName.c_str());

		if (m_zipfile == 0)
		{
			i32 fileIO = wrapperFileIO->fileOpen(m_zipfileName.c_str(), "rb");
			if (fileIO >= 0)
			{
				i32 stream_size = wrapperFileIO->getFileSize(fileIO);
				m_buffer.resize(stream_size);
				i32 read_bytes = wrapperFileIO->fileRead(fileIO, &m_buffer[0], stream_size);
				drx3DAssert(read_bytes == stream_size);
				if (read_bytes != stream_size)
				{
					printf("Ошибка: mismatch reading file %s, expected %d bytes, read %d\n", m_zipfileName.c_str(), stream_size, read_bytes);
				}
				zlib_filefunc_def api;  // callbacks for in-mem file
				
				m_stream = mem_simple_create_file(&api, &m_buffer[0], stream_size);

				m_zipfile = unzAttach(m_stream, &api);
				m_memoryFile = true;
				wrapperFileIO->fileClose(fileIO);
			}
		}
	}

	
		
	static bool FileIOPluginFindFile(uk userPtr, tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen)
	{
		ZipFileIO* fileIo = (ZipFileIO*) userPtr;
		return fileIo->findFile(orgFileName, relativeFileName, maxRelativeFileNameMaxLen);
	}

	void closeZipFile()
	{
		if (m_zipfile)
		{
			if (m_memoryFile)
			{
				unzDetach(&m_zipfile);
			}
			else
			{
				unzClose(m_zipfile);
			}
		}
		m_zipfile = 0;
	}

	virtual ~ZipFileIO()
	{
		for (i32 i=0;i<D3_ZIP_FILEIO_MAX_FILES;i++)
		{
			fileClose(i);
		}
		closeZipFile();
		if (m_stream)
		{
			mem_simple_destroy_file(m_stream);
		}
	}

	
	virtual i32 fileOpen(tukk fileName, tukk mode)
	{
		
		//search a free slot
		i32 slot = -1;
		for (i32 i=0;i<D3_ZIP_FILEIO_MAX_FILES ;i++)
		{
			if (m_fileHandles[i]==0)
			{
				slot=i;
				break;
			}
		}
		if (slot>=0)
		{
			
			if (m_zipfile == NULL)
			{
				printf("%s: not found\n", m_zipfileName.c_str());
				slot = -1;
			} else
			{
				
				i32 result = 0;
				result = unzGetGlobalInfo(m_zipfile, &m_global_info );
				if (result != UNZ_OK)
				{
					printf("could not read file global info from %s\n", m_zipfileName.c_str());
					slot = -1;
				}
			}
			if (slot >=0)
			{
				i32 result = unzLocateFile(m_zipfile, fileName, 0);
				if (result == UNZ_OK)
				{
					unz_file_info info;
					result = unzGetCurrentFileInfo(m_zipfile, &info, NULL, 0, NULL, 0, NULL, 0);
					if (result != UNZ_OK)
					{
						printf("unzGetCurrentFileInfo() != UNZ_OK (%d)\n", result);
						slot=-1;
					}
					else
					{
						result = unzOpenCurrentFile(m_zipfile);
						if (result == UNZ_OK)
						{
							printf("zipFile::fileOpen %s in mode %s in fileHandle %d\n", fileName, mode, slot);
							m_fileHandles[slot] = m_zipfile;
						} else
						{
							slot=-1;
						}
					}
				} else
				{
					slot=-1;
					
				}
			}
		}
		return slot;
	}
	virtual i32 fileRead(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		i32 result = -1;
		if (fileHandle>=0 && fileHandle < D3_ZIP_FILEIO_MAX_FILES )
		{
			unzFile f = m_fileHandles[fileHandle];
			if (f)
			{
				result = unzReadCurrentFile(f, destBuffer,numBytes);
				//::fread(destBuffer, 1, numBytes, f);
			}
		}
		return result;
			
	}
	virtual i32 fileWrite(i32 fileHandle,tukk sourceBuffer, i32 numBytes)
	{
#if 0
		if (fileHandle>=0 && fileHandle < D3_ZIP_FILEIO_MAX_FILES )
		{
			FILE* f = m_fileHandles[fileHandle];
			if (f)
			{
				return ::fwrite(sourceBuffer, 1, numBytes,m_fileHandles[fileHandle]);
			}
		}
#endif
		return -1;
	}
	virtual void fileClose(i32 fileHandle)
	{
		if (fileHandle>=0 && fileHandle < D3_ZIP_FILEIO_MAX_FILES )
		{
			unzFile f = m_fileHandles[fileHandle];
			if (f)
			{
				printf("zipFile::fileClose slot %d\n", fileHandle);
				m_fileHandles[fileHandle]=0;
			}
		}
	}

	virtual bool findResourcePath(tukk fileName, tuk relativeFileName, i32 relativeFileNameSizeInBytes)
	{
		return ResourcePath::findResourcePath(fileName, relativeFileName, relativeFileNameSizeInBytes, ZipFileIO::FileIOPluginFindFile, this);
	}


	virtual bool findFile(tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen)
	{
		i32 fileHandle = -1;
		fileHandle = fileOpen(orgFileName, "rb");
		if (fileHandle>=0)
		{
			//printf("original file found: [%s]\n", orgFileName);
			sprintf(relativeFileName, "%s", orgFileName);
			fileClose(fileHandle);
			return true;
		}

		//printf("Trying various directories, relative to current working directory\n");
		tukk prefix[] = {"./", "./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
		i32 numPrefixes = sizeof(prefix) / sizeof(tukk);

		i32 f = 0;
		bool fileFound = false;

		for (i32 i = 0; !f && i < numPrefixes; i++)
		{
#ifdef _MSC_VER
			sprintf_s(relativeFileName, maxRelativeFileNameMaxLen, "%s%s", prefix[i], orgFileName);
#else
			sprintf(relativeFileName, "%s%s", prefix[i], orgFileName);
#endif
			f = fileOpen(relativeFileName, "rb");
			if (f>=0)
			{
				fileFound = true;
				break;
			}
		}
		if (f>=0)
		{
			fileClose(f);
		}

		return fileFound;
	}
	virtual tuk readLine(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		i32 numRead = 0;
				
		if (fileHandle>=0 && fileHandle < D3_ZIP_FILEIO_MAX_FILES )
		{
			unzFile f = m_fileHandles[fileHandle];
			if (f)
			{
				//return ::fgets(destBuffer, numBytes, m_fileHandles[fileHandle]);
				char c = 0;
				do
				{
					fileRead(fileHandle,&c,1);
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
		if (numRead<numBytes && numRead>0)
		{
			destBuffer[numRead]=0;
			return &destBuffer[0];
		}
		return 0;
	}
	virtual i32 getFileSize(i32 fileHandle)
	{
		i32 size=0;

		if (fileHandle>=0 && fileHandle < D3_ZIP_FILEIO_MAX_FILES )
		{
			unzFile f = m_fileHandles[fileHandle];
			if (f)
			{
				unz_file_info info;
				i32 result = unzGetCurrentFileInfo(f, &info, NULL, 0, NULL, 0, NULL, 0);
				if (result == UNZ_OK)
				{
					size = info.uncompressed_size;
				}
			}
		}
		return size;
	}

	virtual void enableFileCaching(bool enable)
	{
		(void)enable;
	}

};
