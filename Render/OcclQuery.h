// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   OcclQuery.h : Occlusion queries unified interface

   Revision история:
* Created by Tiago Sousa
* Todo: replace flares/CREOcclusionQuery with common queries

   =============================================================================*/

#ifndef __OCCLQUERY_H__
#define __OCCLQUERY_H__

class COcclusionQuery
{
public:
	COcclusionQuery() : m_nVisSamples(~0), m_nCheckFrame(0), m_nDrawFrame(0), m_nOcclusionID(0)
	{
	}

	~COcclusionQuery()
	{
		Release();
	}

	void   Create();
	void   Release();

	void   BeginQuery();
	void   EndQuery();

	u32 GetVisibleSamples(bool bAsynchronous);

	i32    GetDrawFrame() const
	{
		return m_nDrawFrame;
	}

	bool IsReady();

private:

	i32      m_nVisSamples;
	i32      m_nCheckFrame;
	i32      m_nDrawFrame;

	UINT_PTR m_nOcclusionID; // This will carry a pointer D3DOcclusionQuery, so it needs to be 64-bit on Windows 64
};

#endif
