// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SIMPLEOUTPUTSTREAM_H__
#define __SIMPLEOUTPUTSTREAM_H__

#pragma once

#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/SimpleStreamDefs.h>

class CSimpleOutputStream
{
public:
	CSimpleOutputStream(size_t numRecords);
	virtual ~CSimpleOutputStream();

	void Sync();
	void Put(tukk key, tukk value);
	void Put(tukk key, float value);
	void Put(tukk key, i32 value);
	void Put(tukk key, uint64 value);
	void Put(tukk key, int64 value);
	void Put(tukk key, EntityId value);
	void Put(tukk key, Vec2 value);
	void Put(tukk key, Vec3 value);
	void Put(tukk key, Ang3 value) { Put(key, Vec3(value)); }
	void Put(tukk key, Quat value);
	void Put(tukk key, SNetObjectID value);

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CSimpleOutputStream");

		pSizer->Add(*this);
		if (m_records)
			pSizer->Add(m_records, m_numRecords);
	}

	void Put(const SStreamRecord& record);

private:
	virtual void Flush(const SStreamRecord* pRecords, size_t numRecords) = 0;

	size_t         m_numRecords;
	size_t         m_curRecord;
	SStreamRecord* m_records;
};

#endif
