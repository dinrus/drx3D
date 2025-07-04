#ifdef _WIN32
#include "Win32SharedMemory.h"
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/b3Scalar.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

#include <windows.h>
#include <stdio.h>
//see also https://msdn.microsoft.com/en-us/library/windows/desktop/aa366551%28v=vs.85%29.aspx

struct Win32SharedMemorySegment
{
	i32 m_key;
	HANDLE m_hMapFile;
	uk m_buf;
	TCHAR m_szName[1024];

	Win32SharedMemorySegment()
		: m_hMapFile(0),
		  m_buf(0),
		  m_key(-1)
	{
		m_szName[0] = 0;
	}
};

struct Win32SharedMemoryInteralData
{
	b3AlignedObjectArray<Win32SharedMemorySegment> m_segments;

	Win32SharedMemoryInteralData()
	{
	}
};

Win32SharedMemory::Win32SharedMemory()
{
	m_internalData = new Win32SharedMemoryInteralData;
}
Win32SharedMemory::~Win32SharedMemory()
{
	delete m_internalData;
}

uk Win32SharedMemory::allocateSharedMemory(i32 key, i32 size, bool allowCreation)
{
	{
		Win32SharedMemorySegment* seg = 0;
		i32 i = 0;

		for (i = 0; i < m_internalData->m_segments.size(); i++)
		{
			if (m_internalData->m_segments[i].m_key == key)
			{
				seg = &m_internalData->m_segments[i];
				break;
			}
		}
		if (seg)
		{
			drx3DError("already created shared memory segment using same key");
			return seg->m_buf;
		}
	}

	Win32SharedMemorySegment seg;
	seg.m_key = key;
#ifdef UNICODE
	swprintf_s(seg.m_szName, TEXT("MyFileMappingObject%d"), key);
#else

	sprintf(seg.m_szName, "MyFileMappingObject%d", key);
#endif

	seg.m_hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,  // read/write access
		FALSE,                // do not inherit the name
		seg.m_szName);        // name of mapping object

	if (seg.m_hMapFile == NULL)
	{
		if (allowCreation)
		{
			seg.m_hMapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,  // use paging file
				NULL,                  // default security
				PAGE_READWRITE,        // read/write access
				0,                     // maximum object size (high-order DWORD)
				size,                  // maximum object size (low-order DWORD)
				seg.m_szName);         // name of mapping object
		}
		else
		{
			//drx3DWarning("Could not create file mapping object (%d).\n", GetLastError());
			return 0;
		}
	}

	seg.m_buf = MapViewOfFile(seg.m_hMapFile,       // handle to map object
							  FILE_MAP_ALL_ACCESS,  // read/write permission
							  0,
							  0,
							  size);

	if (seg.m_buf == NULL)
	{
		drx3DWarning("Could not map view of file (%d).\n", GetLastError());
		CloseHandle(seg.m_hMapFile);
		return 0;
	}

	m_internalData->m_segments.push_back(seg);
	return seg.m_buf;
}
void Win32SharedMemory::releaseSharedMemory(i32 key, i32 size)
{
	Win32SharedMemorySegment* seg = 0;
	i32 i = 0;

	for (i = 0; i < m_internalData->m_segments.size(); i++)
	{
		if (m_internalData->m_segments[i].m_key == key)
		{
			seg = &m_internalData->m_segments[i];
			break;
		}
	}

	if (seg == 0)
	{
		drx3DError("Couldn't find shared memory segment");
		return;
	}

	if (seg->m_buf)
	{
		UnmapViewOfFile(seg->m_buf);
		seg->m_buf = 0;
	}

	if (seg->m_hMapFile)
	{
		CloseHandle(seg->m_hMapFile);
		seg->m_hMapFile = 0;
	}

	m_internalData->m_segments.removeAtIndex(i);
}

Win32SharedMemoryServer::Win32SharedMemoryServer()
{
}
Win32SharedMemoryServer::~Win32SharedMemoryServer()
{
}

Win32SharedMemoryClient::Win32SharedMemoryClient()
{
}
Win32SharedMemoryClient::~Win32SharedMemoryClient()
{
}

#endif  //_WIN32
