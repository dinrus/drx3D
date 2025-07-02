// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CREOCCLUSIONQUERY_H__
#define __CREOCCLUSIONQUERY_H__

class CRenderMesh;

class CREOcclusionQuery : public CRenderElement
{
	friend class CRender3D;
	bool m_bSucceeded;
public:

	i32           m_nVisSamples;
	i32           m_nCheckFrame;
	i32           m_nDrawFrame;
	Vec3          m_vBoxMin;
	Vec3          m_vBoxMax;

	UINT_PTR      m_nOcclusionID; //!< This will carry a pointer D3DOcclusionQuery, so it needs to be 64-bit on Windows 64

	CRenderMesh*  m_pRMBox;
	static u32 m_nQueriesPerFrameCounter;
	static u32 m_nReadResultNowCounter;
	static u32 m_nReadResultTryCounter;

	CREOcclusionQuery()
	{
		m_nOcclusionID = 0;

		m_nVisSamples = 800 * 600;
		m_nCheckFrame = 0;
		m_nDrawFrame = 0;
		m_vBoxMin = Vec3(0, 0, 0);
		m_vBoxMax = Vec3(0, 0, 0);
		m_pRMBox = NULL;

		mfSetType(eDATA_OcclusionQuery);
		mfUpdateFlags(FCEF_TRANSFORM);
	}

	bool       RT_ReadResult_Try(u32 nDefaultNumSamples);

	ILINE bool HasSucceeded() const { return m_bSucceeded; }

	virtual ~CREOcclusionQuery();

	virtual void mfReset();
	virtual void mfReadResult_Try(u32 nDefaultNumSamples = 1);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

#endif  // __CREOCCLUSIONQUERY_H__
