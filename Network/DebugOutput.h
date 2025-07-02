// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <deque>
#include <drx3D/Network/SessionID.h>
#include <drx3D/Network/NetResolver.h>

class CDebugOutput
{
public:
	virtual ~CDebugOutput(){}
	void Put(u8 val);

	void Run(DRXSOCKET sock);

	template<class T>
	void Write(const T& value)
	{
		DrxAutoLock<CDebugOutput> lk(*this);
		u8k* pBuffer = (u8*)&value;
		for (i32 i = 0; i < sizeof(value); i++)
			Put(pBuffer[i]);
	}

	void Write(const string& value)
	{
		DrxAutoLock<CDebugOutput> lk(*this);
		Write((u32)value.length());
		for (u32 i = 0; i < value.length(); i++)
			Put(value[i]);
	}

	CSessionID m_sessionID;

	void WriteValue(const TNetAddressVec& vec)
	{
		DrxAutoLock<CDebugOutput> lk(*this);
		Write(vec.size());
		for (size_t i = 0; i < vec.size(); i++)
			Write(CNetAddressResolver().ToString(vec[i]));
	}

	virtual void Lock() = 0;
	virtual void Unlock() = 0;

private:
	static i32k DATA_SIZE = 128 - sizeof(i32);
	struct SBuf
	{
		SBuf() : ready(true), sz(0), pos(0) { ZeroMemory(data, sizeof(data)); }
		bool ready;
		i32  sz;
		i32  pos;
		char data[DATA_SIZE];
	};

	std::deque<SBuf> m_buffers;
};
