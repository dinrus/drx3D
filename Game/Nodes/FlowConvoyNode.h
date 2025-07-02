// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Inplements a convoy in a FlowNode

-------------------------------------------------------------------------

*************************************************************************/

#ifndef __FLOWCONVOYNODE_H__
#define __FLOWCONVOYNODE_H__

#include <DrxFlowGraph/IFlowSystem.h>
#include <drx3D/Game/Nodes/G2FlowBaseNode.h>

class CConvoyPathIterator
{
	friend class CConvoyPath;
public:
	CConvoyPathIterator();
	void Invalidate();

private:
	i32 Segment;
	float LastDistance;
};

class CConvoyPath
{
public:
	CConvoyPath();
	void SetPath(const std::vector<Vec3> &path);
	Vec3 GetPointAlongPath(float dist, CConvoyPathIterator &iterator);
	float GetTotalLength() const { return m_totalLength;}

private:
	struct SConvoyPathNode
	{
		Vec3 Position;
		float Length, TotalLength;
	};

	std::vector<SConvoyPathNode> m_path;
	float m_totalLength;
};

class CFlowConvoyNode : public CFlowBaseNode<eNCT_Instanced>
{
public:
	static std::vector<CFlowConvoyNode *> gFlowConvoyNodes;
	static i32 OnPhysicsPostStep_static(const EventPhys * pEvent);
	CFlowConvoyNode( SActivationInfo * pActInfo );
	~CFlowConvoyNode();
	virtual IFlowNodePtr Clone( SActivationInfo * pActInfo );
	virtual void Serialize(SActivationInfo *, TSerialize ser);
	virtual void GetConfiguration( SFlowNodeConfig &config );
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo );
	virtual void GetMemoryUsage(IDrxSizer * s) const;
	i32 OnPhysicsPostStep(const EventPhys * pEvent);
	static bool PlayerIsOnaConvoy();
private:
	void DiscoverConvoyCoaches(IEntity *pEntity);
	void InitConvoyCoaches();
	void AwakeCoaches();
	i32 GetCoachIndexPlayerIsOn();
	i32 GetCoachIndexPlayerIsOn2();
	float GetPlayerMaxVelGround();
	void SetPlayerMaxVelGround(float vel);

	void Update(SActivationInfo *pActInfo);

	//tSoundID PlayLineSound(i32 coachIndex, tukk sGroupAndSoundName, const Vec3 &vStart, const Vec3 &vEnd);
	void UpdateLineSounds();
	void StartSounds();
	void SplitLineSound();
	void ConvoyStopSounds();
	void StopAllSounds();
	float GetSpeedSoundParam(i32 coachIndex);
	void SetSoundParams();
	void StartBreakSoundShifted();

	enum EInputs
	{
		IN_PATH,
		IN_LOOPCOUNT,
		IN_SPEED,
		IN_DESIREDSPEED,
		IN_SHIFT,
		IN_SHIFTTIME,
		IN_START_DISTANCE,
		IN_SPLIT_COACH,
		IN_XAXIS_FWD,
		IN_HORN_SOUND,
		IN_BREAK_SOUND,
		IN_START,
		IN_STOP,
	};
	enum EOutputs
	{
		OUT_ONPATHEND,
		OUT_COACHINDEX,
	};

	CConvoyPath m_path;
	float m_speed;
	float m_desiredSpeed;
	float m_ShiftTime;
	float m_MaxShiftTime;
	float m_distanceOnPath; //train last coach end distance on path
	i32 m_splitCoachIndex;  //coach index where to split train (0 is the train engine)
	i32 m_loopCount;
	i32 m_loopTotal;

	bool m_bFirstUpdate;
	bool m_bXAxisFwd;
	CTimeValue m_offConvoyStartTime;
	i32 m_coachIndex;

	float m_splitDistanceOnPath; //the splitted coaches end coach distance on path
	float m_splitSpeed; //splitted coaches speed
	bool m_processNode;
	bool m_atEndOfPath;

	struct SConvoyCoach
	{
		SConvoyCoach();
		IEntity *m_pEntity;
		i32 m_frontWheelBase, m_backWheelBase;
		float m_wheelDistance; //wheel half distance from center
		float m_coachOffset;  //coach half length
		float m_distanceOnPath; //coach center distance on path
		//IEntityAudioProxy* m_pEntitySoundsProxy;
		//tSoundID m_runSoundID;
		//tSoundID m_breakSoundID;
		CConvoyPathIterator m_frontWheelIterator[2], m_backWheelIterator[2];
	};

	//tSoundID m_hornSoundID;
	//tSoundID m_engineStartSoundID;

	std::vector<SConvoyCoach> m_coaches;
	static float m_playerMaxVelGround;
	i32 m_startBreakSoundShifted;
};

#endif
