// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FIXEDSIZEARENA_H__
#define __FIXEDSIZEARENA_H__

#pragma once

// manages a small fixed size pool of objects
template<i32 SZ, i32 N>
class CFixedSizeArena
{
public:
	CFixedSizeArena()
	{
		m_nFree = N;
		for (i32 i = 0; i < m_nFree; i++)
		{
			m_vFree[i] = i;
		}
	}

	// create an object; returns 0 if pool is full
	template<class T>
	ILINE T* Construct()
	{
		NET_ASSERT(sizeof(T) <= SZ);
		if (m_nFree <= 0)
			return 0;
		NET_ASSERT(m_nFree - 1 < N);
		i32 which = m_vFree[--m_nFree];
		uk p = m_members[which].data;
		return new(p) T();
	}

	// create an object with a constructor parameter; returns 0 if pool is full
	template<class T, class A0>
	ILINE T* Construct(const A0& a0)
	{
		NET_ASSERT(sizeof(T) <= SZ);
		if (!m_nFree)
			return 0;
		NET_ASSERT(m_nFree - 1 < N);
		i32 which = m_vFree[--m_nFree];
		uk p = m_members[which].data;
		return new(p) T(a0);
	}

	// destroy an object
	template<class T>
	ILINE void Dispose(T* p)
	{
		p->~T();
		NET_ASSERT(m_nFree < N);
		u8* pp = (u8*) p;
		i32 which;
		// TODO: should be able to optimize away this loop
		for (which = 0; which < N; ++which)
			if (pp == m_members[which].data)
				break;
		NET_ASSERT(which != N);
		if (which == N)
			return;
		m_vFree[m_nFree++] = which;
	}

private:
	union Member
	{
		uk dummy;  // for alignment
		u8 data[SZ];
	};
	i32    m_nFree;
	i32    m_vFree[N];
	Member m_members[N];
};

#endif
