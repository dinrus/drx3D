
#ifndef D3_DRX3D_DEFAULT_FILE_IO_H
#define D3_DRX3D_DEFAULT_FILE_IO_H

#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/Common/ResourcePath.h>

#include <stdio.h>
#include <string.h>

#define D3_FILEIO_MAX_FILES 1024

struct DefaultFileIO : public CommonFileIOInterface
{
	static bool FileIOPluginFindFile(uk userPtr, tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen)
	{
		DefaultFileIO* fileIo = (DefaultFileIO*) userPtr;
		return fileIo->findFile(orgFileName, relativeFileName, maxRelativeFileNameMaxLen);
	}

	char m_prefix[1024];
	FILE* m_fileHandles[D3_FILEIO_MAX_FILES];
	i32 m_numFileHandles;

	DefaultFileIO(i32 fileIOType=0, tukk pathPrefix=0)
		:CommonFileIOInterface(fileIOType, m_prefix),
		m_numFileHandles(0)
	{
		m_prefix[0] = 0;
		if (pathPrefix)
		{
			sprintf(m_prefix,"%s", pathPrefix);
		}
		for (i32 i=0;i<D3_FILEIO_MAX_FILES;i++)
		{
			m_fileHandles[i]=0;
		}
	}

	virtual ~DefaultFileIO()
	{
	}
	virtual i32 fileOpen(tukk fileName, tukk mode)
	{
		//search a free slot
		i32 slot = -1;
		for (i32 i=0;i<D3_FILEIO_MAX_FILES;i++)
		{
			if (m_fileHandles[i]==0)
			{
				slot=i;
				break;
			}
		}
		if (slot>=0)
		{
			FILE*f = ::fopen(fileName, mode);
			if (f)
			{
				m_fileHandles[slot]=f;
			} else
			{
				slot=-1;
			}
		}
		return slot;
	}
	virtual i32 fileRead(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			FILE* f = m_fileHandles[fileHandle];
			if (f)
			{
				i32 readBytes = ::fread(destBuffer, 1, numBytes, f);
				return readBytes;
			}
		}
		return -1;
			
	}
	virtual i32 fileWrite(i32 fileHandle,tukk sourceBuffer, i32 numBytes)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			FILE* f = m_fileHandles[fileHandle];
			if (f)
			{
				return ::fwrite(sourceBuffer, 1, numBytes,m_fileHandles[fileHandle]);
			}
		}
		return -1;
	}
	virtual void fileClose(i32 fileHandle)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			FILE* f = m_fileHandles[fileHandle];
			if (f)
			{
				::fclose(f);
				m_fileHandles[fileHandle]=0;
			}
		}
	}

	virtual bool findResourcePath(tukk fileName, tuk relativeFileName, i32 relativeFileNameSizeInBytes)
	{
		return ResourcePath::findResourcePath(fileName, relativeFileName, relativeFileNameSizeInBytes, DefaultFileIO::FileIOPluginFindFile, this)>0;
	}


	virtual bool findFile(tukk orgFileName, tuk relativeFileName, i32 maxRelativeFileNameMaxLen)
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
		tukk prefix[] = {m_prefix, "./", "./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
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
	virtual tuk readLine(i32 fileHandle, tuk destBuffer, i32 numBytes)
	{
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			FILE* f = m_fileHandles[fileHandle];
			if (f)
			{
                                memset(destBuffer, 0, numBytes);
				tuk txt = ::fgets(destBuffer, numBytes, m_fileHandles[fileHandle]);
				for (i32 i=0;i<numBytes;i++)
				{
					if (destBuffer[i]=='\r'||destBuffer[i]=='\n' || destBuffer[i]==0)
					{
						destBuffer[i] = 0;
						break;
					}
				}
				return txt;
			}
		}
		return 0;
	}
	virtual i32 getFileSize(i32 fileHandle)
	{
		i32 size = 0;
		if (fileHandle>=0 && fileHandle < D3_FILEIO_MAX_FILES)
		{
			FILE* f = m_fileHandles[fileHandle];
			if (f)
			{
				
				if (fseek(f, 0, SEEK_END) || (size = ftell(f)) == EOF || fseek(f, 0, SEEK_SET))
				{
					printf("Ошибка: Cannot access file to determine size\n");
				}
			}
		}
		return size;
	}

	virtual void enableFileCaching(bool enable)
	{
		(void) enable;
	}
};

#endif //D3_DRX3D_DEFAULT_FILE_IO_H
