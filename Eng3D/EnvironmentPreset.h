// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _environment_preset_h_
#define _environment_preset_h_
#pragma once

#include <drx3D/CoreX/Math/Bezier.h>

class CBezierSpline
{
public:
	CBezierSpline();
	~CBezierSpline();

	void              Init(float fDefaultValue);
	float             Evaluate(float t) const;

	void              SetKeys(const SBezierKey* keysArray, u32 keysArraySize) { m_keys.resize(keysArraySize); memcpy(&m_keys[0], keysArray, keysArraySize * sizeof(SBezierKey)); }
	void              GetKeys(SBezierKey* keys) const                                  { memcpy(keys, &m_keys[0], m_keys.size() * sizeof(SBezierKey)); }

	void              InsertKey(SAnimTime time, float value);
	void              UpdateKeyForTime(float fTime, float value);

	void              Resize(size_t nSize)        { m_keys.resize(nSize); }

	size_t            GetKeyCount() const         { return m_keys.size(); }
	const SBezierKey& GetKey(size_t nIndex) const { return m_keys[nIndex]; }
	SBezierKey&       GetKey(size_t nIndex)       { return m_keys[nIndex]; }

	void              Serialize(Serialization::IArchive& ar);
private:
	typedef std::vector<SBezierKey> TKeyContainer;
	TKeyContainer m_keys;

	struct SCompKeyTime
	{
		bool operator()(const TKeyContainer::value_type& l, const TKeyContainer::value_type& r) const { return l.m_time < r.m_time; }
		bool operator()(SAnimTime l, const TKeyContainer::value_type& r) const                        { return l < r.m_time; }
		bool operator()(const TKeyContainer::value_type& l, SAnimTime r) const                        { return l.m_time < r; }
	};
};

//////////////////////////////////////////////////////////////////////////
class CTimeOfDayVariable
{
public:
	CTimeOfDayVariable();
	~CTimeOfDayVariable();

	void                          Init(tukk group, tukk displayName, tukk name, ITimeOfDay::ETimeOfDayParamID nParamId, ITimeOfDay::EVariableType type, float defVal0, float defVal1, float defVal2);
	void                          Update(float time);

	Vec3                          GetInterpolatedAt(float t) const;

	ITimeOfDay::ETimeOfDayParamID GetId() const          { return m_id; }
	ITimeOfDay::EVariableType     GetType() const        { return m_type; }
	tukk                   GetName() const        { return m_name; }
	tukk                   GetDisplayName() const { return m_displayName; }
	tukk                   GetGroupName() const   { return m_group; }
	const Vec3                    GetValue() const       { return m_value; }

	float                         GetMinValue() const    { return m_minValue; }
	float                         GetMaxValue() const    { return m_maxValue; }

	const CBezierSpline*          GetSpline(i32 nIndex) const
	{
		if (nIndex >= 0 && nIndex < Vec3::component_count)
			return &m_spline[nIndex];
		else
			return NULL;
	}

	CBezierSpline* GetSpline(i32 nIndex)
	{
		if (nIndex >= 0 && nIndex < Vec3::component_count)
			return &m_spline[nIndex];
		else
			return NULL;
	}

	size_t GetSplineKeyCount(i32 nSpline) const;
	bool   GetSplineKeys(i32 nSpline, SBezierKey* keysArray, u32 keysArraySize) const;
	bool   SetSplineKeys(i32 nSpline, const SBezierKey* keysArray, u32 keysArraySize);
	bool   UpdateSplineKeyForTime(i32 nSpline, float fTime, float newKey);

	void   Serialize(Serialization::IArchive& ar);
private:
	ITimeOfDay::ETimeOfDayParamID m_id;
	ITimeOfDay::EVariableType     m_type;

	tukk                   m_name;        // Variable name.
	tukk                   m_displayName; // Variable user readable name.
	tukk                   m_group;       // Group name.

	float                         m_minValue;
	float                         m_maxValue;

	Vec3                          m_value;
	CBezierSpline                 m_spline[Vec3::component_count]; //spline for each component in m_value
};

//////////////////////////////////////////////////////////////////////////
class CEnvironmentPreset
{
public:
	CEnvironmentPreset();
	~CEnvironmentPreset();

	void                      ResetVariables();
	void                      Update(float t);

	const CTimeOfDayVariable* GetVar(ITimeOfDay::ETimeOfDayParamID id) const { return &m_vars[id]; }
	CTimeOfDayVariable*       GetVar(ITimeOfDay::ETimeOfDayParamID id)       { return &m_vars[id]; }
	CTimeOfDayVariable*       GetVar(tukk varName);
	bool                      InterpolateVarInRange(ITimeOfDay::ETimeOfDayParamID id, float fMin, float fMax, u32 nCount, Vec3* resultArray) const;

	void                      Serialize(Serialization::IArchive& ar);

	static float              GetAnimTimeSecondsIn24h();
private:
	void                      AddVar(tukk group, tukk displayName, tukk name, ITimeOfDay::ETimeOfDayParamID nParamId, ITimeOfDay::EVariableType type, float defVal0, float defVal1, float defVal2);

	CTimeOfDayVariable m_vars[ITimeOfDay::PARAM_TOTAL];
};

#endif //_environment_preset_h_
