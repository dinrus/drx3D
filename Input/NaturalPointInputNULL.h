// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
struct CNaturalPointInputNull : public INaturalPointInput
{
public:
	CNaturalPointInputNull(){};
	virtual ~CNaturalPointInputNull(){};

	virtual bool Init()      { return true; }
	virtual void Update()    {};
	virtual bool IsEnabled() { return true; }

	virtual void Recenter()  {};

	// Summary;:
	//		Get raw skeleton data
	virtual bool GetNaturalPointData(NP_RawData& npRawData) const { return true; }

};
