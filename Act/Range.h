// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   ---------------------------------------------------------------------
   Имя файла:   Range.h
   $Id$
   $DateTime$
   Описание: Single Range donut
   ---------------------------------------------------------------------
   История:
   - 24:08:2007 : Created by Ricardo Pillosu

 *********************************************************************/
#ifndef _RANGE_H_
#define _RANGE_H_

class CPersonalRangeSignaling;

class CRange
{

public:
	// Base ----------------------------------------------------------
	CRange(CPersonalRangeSignaling* pPersonal);
	virtual ~CRange();
	void Init(float fRadius, float fBoundary, tukk sSignal, IAISignalExtraData* pData = NULL);

	// Utils ---------------------------------------------------------
	bool IsInRange(const Vec3& vPos) const;
	bool IsInRangePlusBoundary(const Vec3& vPos) const;

	bool operator<(const CRange& Range) const
	{
		return(m_fRadius < Range.m_fRadius);
	}

	bool IsInRange(float fSquaredDIst) const
	{
		return(fSquaredDIst < m_fRadius);
	}

	bool IsInRangePlusBoundary(float fSquaredDIst) const
	{
		return(fSquaredDIst < m_fBoundary);
	}

	const string& GetSignal() const
	{
		return(m_sSignal);
	}

	IAISignalExtraData* GetSignalData() const
	{
		return(m_pSignalData);
	}

private:

private:
	CPersonalRangeSignaling* m_pPersonal;
	IAISignalExtraData*      m_pSignalData;
	float                    m_fRadius;
	float                    m_fBoundary;
	string                   m_sSignal;
};
#endif // _RANGE_H_
