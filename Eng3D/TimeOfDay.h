// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   TimeOfDay.h
//  Version:     v1.00
//  Created:     25/10/2005 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TimeOfDay_h__
#define __TimeOfDay_h__

#include <drx3D/Eng3D/ITimeOfDay.h>
#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>

class CEnvironmentPreset;
//////////////////////////////////////////////////////////////////////////
// ITimeOfDay interface implementation.
//////////////////////////////////////////////////////////////////////////
class CTimeOfDay : public ITimeOfDay
{
public:
	CTimeOfDay();
	~CTimeOfDay();

	//////////////////////////////////////////////////////////////////////////
	// ITimeOfDay
	//////////////////////////////////////////////////////////////////////////

	virtual i32   GetPresetCount() const override { return m_presets.size(); }
	virtual bool  GetPresetsInfos(SPresetInfo* resultArray, u32 arraySize) const override;
	virtual bool  SetCurrentPreset(tukk szPresetName) override;
	virtual tukk GetCurrentPresetName() const override;
	virtual bool  AddNewPreset(tukk szPresetName) override;
	virtual bool  RemovePreset(tukk szPresetName) override;
	virtual bool  SavePreset(tukk szPresetName) const override;
	virtual bool  LoadPreset(tukk szFilePath) override;
	virtual void  ResetPreset(tukk szPresetName) override;

	virtual bool  ImportPreset(tukk szPresetName, tukk szFilePath) override;
	virtual bool  ExportPreset(tukk szPresetName, tukk szFilePath) const override;

	virtual i32   GetVariableCount() override { return ITimeOfDay::PARAM_TOTAL; };
	virtual bool  GetVariableInfo(i32 nIndex, SVariableInfo& varInfo) override;
	virtual void  SetVariableValue(i32 nIndex, float fValue[3]) override;

	virtual bool  InterpolateVarInRange(i32 nIndex, float fMin, float fMax, u32 nCount, Vec3* resultArray) const override;
	virtual uint  GetSplineKeysCount(i32 nIndex, i32 nSpline) const override;
	virtual bool  GetSplineKeysForVar(i32 nIndex, i32 nSpline, SBezierKey* keysArray, u32 keysArraySize) const override;
	virtual bool  SetSplineKeysForVar(i32 nIndex, i32 nSpline, const SBezierKey* keysArray, u32 keysArraySize) override;
	virtual bool  UpdateSplineKeyForVar(i32 nIndex, i32 nSpline, float fTime, float newValue) override;
	virtual float GetAnimTimeSecondsIn24h() override;

	virtual void  ResetVariables() override;

	// Time of day is specified in hours.
	virtual void  SetTime(float fHour, bool bForceUpdate = false) override;
	virtual void  SetSunPos(float longitude, float latitude) override;
	virtual float GetSunLatitude() override      { return m_sunRotationLatitude; }
	virtual float GetSunLongitude() override     { return m_sunRotationLongitude; }
	virtual float GetTime() override             { return m_fTime; };

	virtual void  SetPaused(bool paused) override { m_bPaused = paused; }

	virtual void  SetAdvancedInfo(const SAdvancedInfo& advInfo) override;
	virtual void  GetAdvancedInfo(SAdvancedInfo& advInfo) override;

	float         GetHDRMultiplier() const { return m_fHDRMultiplier; }

	virtual void  Update(bool bInterpolate = true, bool bForceUpdate = false) override;

	virtual void  Serialize(XmlNodeRef& node, bool bLoading) override;
	virtual void  Serialize(TSerialize ser) override;

	virtual void  SetTimer(ITimer* pTimer) override;

	virtual void  NetSerialize(TSerialize ser, float lag, u32 flags) override;

	virtual void  Tick() override;

	virtual void  SetEnvironmentSettings(const SEnvironmentInfo& envInfo) override;

	virtual void  SaveInternalState(struct IDataWriteStream& writer) override;
	virtual void  LoadInternalState(struct IDataReadStream& reader) override;

	//////////////////////////////////////////////////////////////////////////

	void BeginEditMode() override { m_bEditMode = true; };
	void EndEditMode() override { m_bEditMode = false; };

protected:
	virtual bool RegisterListenerImpl(IListener* const pListener, tukk const szDbgName, const bool staticName) override;
	virtual void UnRegisterListenerImpl(IListener* const pListener) override;

private:
	CTimeOfDay(const CTimeOfDay&);
	CTimeOfDay(const CTimeOfDay&&);
	CTimeOfDay&    operator=(const CTimeOfDay&);
	CTimeOfDay&    operator=(const CTimeOfDay&&);

	SVariableInfo& GetVar(ETimeOfDayParamID id);
	void           UpdateEnvLighting(bool forceUpdate);
	void NotifyOnChange(const IListener::EChangeType changeType, tukk const szPresetName);

private:
	typedef std::map<string, CEnvironmentPreset> TPresetsSet;
	typedef CListenerSet<IListener*> TListenerSet;

private:
	TPresetsSet         m_presets;
	CEnvironmentPreset* m_pCurrentPreset;
	string              m_currentPresetName;

	SVariableInfo       m_vars[ITimeOfDay::PARAM_TOTAL];

	float               m_fTime;
	float               m_sunRotationLatitude;
	float               m_sunRotationLongitude;

	bool                m_bEditMode;
	bool                m_bPaused;
	bool                m_bSunLinkedToTOD;

	SAdvancedInfo       m_advancedInfo;
	ITimer*             m_pTimer;
	float               m_fHDRMultiplier;
	ICVar*              m_pTimeOfDaySpeedCVar;
	DrxAudio::ControlId m_timeOfDayRtpcId;
	TListenerSet        m_listeners;
};

#endif //__TimeOfDay_h__
