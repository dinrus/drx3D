// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SimpleOutputStream.h>

CSimpleOutputStream::CSimpleOutputStream(size_t numRecords) : m_numRecords(numRecords), m_curRecord(0), m_records(new SStreamRecord[m_numRecords])
{
}

CSimpleOutputStream::~CSimpleOutputStream()
{
	delete[] m_records;
}

void CSimpleOutputStream::Put(const SStreamRecord& record)
{
	m_records[m_curRecord++] = record;
	if (m_curRecord == m_numRecords)
	{
		Flush(m_records, m_numRecords);
		m_curRecord = 0;
	}
}

void CSimpleOutputStream::Put(tukk key, tukk value)
{
	Put(SStreamRecord(key, value));
}

void CSimpleOutputStream::Put(tukk key, Vec2 value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%f,%f", value.x, value.y);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, Vec3 value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%f,%f,%f", value.x, value.y, value.z);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, SNetObjectID value)
{
	SStreamRecord rec(key);
	value.GetText(rec.value);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, Quat value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%f,%f,%f,%f", value.w, value.v.x, value.v.y, value.v.z);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, i32 value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%d", value);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, int64 value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%" PRId64, value);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, uint64 value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%" PRIu64 ".16x", value);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, float value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%f", value);
	Put(rec);
}

void CSimpleOutputStream::Put(tukk key, EntityId value)
{
	SStreamRecord rec(key);
	drx_sprintf(rec.value, "%.8x", value);
	Put(rec);
}

void CSimpleOutputStream::Sync()
{
	if (m_curRecord)
	{
		Flush(m_records, m_curRecord);
		m_curRecord = 0;
	}
}
