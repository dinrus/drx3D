// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   DrxGame Source File.
   -------------------------------------------------------------------------
   Имя файла:   AIHandler.h
   Version:     v1.00
   Описание:

   -------------------------------------------------------------------------
   История:
   - 8:10:2004   12:04 : Created by Kirill Bulatsev

 *********************************************************************/

#ifndef __AIHandler_H__
#define __AIHandler_H__

#pragma once

#include <drx3D/Act/AIFaceUpr.h>
#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Act/AnimActionTriState.h>
#include <drx3D/Act/IMovementController.h> // for the exactpositioning types

struct IScriptSystem;
struct IScriptTable;
struct ISystem;
struct IAISignalExtraData;
class CAIProxy;
class CMovementRequest;

//
//------------------------------------------------------------------------------
class CAnimActionAIAction : public CAnimActionTriState
{
public:
	typedef CAnimActionTriState TBase;

	enum EAIActionType
	{
		EAT_Looping,
		EAT_OneShot,
	};

	// TODO: Move mannequin priorities into DinrusAction or get this from a central location
	enum
	{
		NORMAL_ACTION_PRIORITY = 3, // currently equal to PP_Action, just above movement, but underneath urgent actions, hit death, etc.
		URGENT_ACTION_PRIORITY = 4  // currently equal to PP_Urgent_Action, just above normal actions, but underneath hit death, etc.
	};

	DEFINE_ACTION("AIAction");

	CAnimActionAIAction(bool isUrgent, FragmentID fragmentID, IAnimatedCharacter& animChar, EAIActionType type, const string& value, IAnimActionTriStateListener& listener, bool skipIntro = false)
		: TBase((isUrgent ? URGENT_ACTION_PRIORITY : NORMAL_ACTION_PRIORITY), fragmentID, animChar, (type == EAT_OneShot), -1, skipIntro, &listener)
		, m_type(type)
		, m_value(value)
		, m_isUrgent(isUrgent)
	{
		InitMovementControlMethods(eMCM_AnimationHCollision, eMCM_Entity);
	}

	// overrides ----------------------------------------------------------------
	EPriorityComparison ComparePriority(const IAction& actionCurrent) const override
	{
		if (m_isUrgent)
		{
			return (IAction::Installed == actionCurrent.GetStatus() && IAction::Installing & ~actionCurrent.GetFlags()) ? Higher : TBase::ComparePriority(actionCurrent);
		}
		else
		{
			return TBase::ComparePriority(actionCurrent);
		}
	}
	// ~overrides ---------------------------------------------------------------

	EAIActionType GetType() const  { return m_type; }
	const string& GetValue() const { return m_value; }

private:
	EAIActionType m_type;
	string        m_value;
	bool          m_isUrgent;
};

// ----------------------------------------------------------------------------

class CAIHandler
	: public IAnimationGraphStateListener
	  , IExactPositioningListener
	  , IAnimActionTriStateListener
{
	friend class CAIProxy;

	// implementation of IAnimationGraphStateListener
	virtual void SetOutput(tukk output, tukk value) {}
	virtual void QueryComplete(TAnimationGraphQueryID queryID, bool succeeded);
	virtual void DestroyedState(IAnimationGraphState*);

	//------------------  IExactPositioningListener
	virtual void ExactPositioningQueryComplete(TExactPositioningQueryID queryID, bool succeeded);
	virtual void ExactPositioningNotifyFinishPoint(const Vec3& pt);
	//------------------  ~IExactPositioningListener

	//------------------  IAnimActionTriStateListener
	virtual void OnAnimActionTriStateEnter(CAnimActionTriState* pActionTriState);
	virtual void OnAnimActionTriStateMiddle(CAnimActionTriState* pActionTriState);
	virtual void OnAnimActionTriStateExit(CAnimActionTriState* pActionTriState);
	//------------------  ~IAnimActionTriStateListener
public:
	CAIHandler(IGameObject* pGameObject);
	~CAIHandler(void);

	void Init();
	void Reset(EObjectResetType type);
	void OnReused(IGameObject* pGameObject);

	void HandleCoverRequest(SOBJECTSTATE& state, CMovementRequest& mr);
	void HandleMannequinRequest(SOBJECTSTATE& state, CMovementRequest& mr);
	void HandleExactPositioning(SOBJECTSTATE& state, CMovementRequest& mr);

	void Update();
	void AIMind(SOBJECTSTATE& state);
	void AISignal(i32 signalID, tukk signalText, u32 crc, IEntity* pSender, const IAISignalExtraData* pData);
	void Release();

	/// Plays a readability set (Group + Response)
	/// Note: A response delay of 0 means a random delay is chosen
	bool HasReadibilitySoundFinished() const { return m_bSoundFinished; }

	bool SetAGInput(EAIAGInput input, tukk value, const bool isUrgent = false);

	bool SetMannequinAGInput(IActionController* pActionController, EAIAGInput input, tukk value, const bool isUrgent = false);
	void FinishRunningAGActions();

	/// Returns an estimate of how long the triggered animation would play when triggered now.
	/// (with the current stance, vehicle state, ...)
	CTimeValue            GetEstimatedAGAnimationLength(EAIAGInput input, tukk value);

	bool                  ResetAGInput(EAIAGInput input);

	bool                  ResetMannequinAGInput(EAIAGInput input);
	IAnimationGraphState* GetAGState();
	bool                  IsSignalAnimationPlayed(tukk value);
	bool                  IsActionAnimationStarted(tukk value);
	bool                  IsAnimationBlockingMovement() const;
	EActorTargetPhase     GetActorTargetPhase() const { return m_eActorTargetPhase; }

	IActionController*    GetActionController();

	void                  OnDialogLoaded(struct ILipSync* pLipSync);
	void                  OnDialogFailed(struct ILipSync* pLipSync);

	enum ESetFlags
	{
		SET_IMMEDIATE,
		SET_DELAYED,
		SET_ON_SERILIAZE,
	};

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	bool SetCharacter(tukk character, ESetFlags setFlags = SET_IMMEDIATE);
#endif
	void SetBehavior(tukk szBehavior, const IAISignalExtraData* pData = 0, ESetFlags setFlags = SET_IMMEDIATE);

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	tukk GetCharacter();

	void        CallCharacterConstructor();
#endif
	void        CallBehaviorConstructor(const IAISignalExtraData* pData);

	void        ResendTargetSignalsNextFrame();

	// For temporary debug info retrieving in AIProxy.
	bool        DEBUG_IsPlayingSignalAnimation() const      { return m_playingSignalAnimation; }
	bool        DEBUG_IsPlayingActionAnimation() const      { return m_playingActionAnimation; }
	tukk DEBUG_GetCurrentSignaAnimationName() const  { return m_currentSignalAnimName.c_str(); }
	tukk DEBUG_GetCurrentActionAnimationName() const { return m_currentActionAnimName.c_str(); }

protected:
	class CAnimActionExactPositioning* CreateExactPositioningAction(bool isOneShot, float loopDuration, tukk szFragmentID, bool isNavigationalSO, const QuatT& exactStartLocation);

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	tukk GetInitialCharacterName();
#endif
	tukk GetInitialBehaviorName();
	tukk GetBehaviorFileName(tukk szBehaviorName);

	IActor*     GetActor() const;

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	void ResetCharacter();
#endif
	void ResetBehavior();
	void ResetAnimationData();

	void SetInitialBehaviorAndCharacter();

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	tukk        CheckAndGetBehaviorTransition(tukk szSignalText) const;
#endif
	bool               CallScript(IScriptTable* scriptTable, tukk funcName, float* pValue = NULL, IEntity* pSender = NULL, const IAISignalExtraData* pData = NULL);
	bool               GetMostLikelyTable(IScriptTable* table, SmartScriptTable& dest);
	bool               FindOrLoadTable(IScriptTable* pGlobalTable, tukk szTableName, SmartScriptTable& tableOut);
	void               FindOrLoadBehavior(tukk szBehaviorName, SmartScriptTable& pBehaviorTable);

	void               Serialize(TSerialize ser);
	void               SerializeScriptAI(TSerialize& ser);

	void               SetAlertness(i32 value, bool triggerEvent = false);
	/// Iterates to the next valid readability sound while testing readability sounds.
	void               NextReadabilityTest();

	void               ResetCommonTables();
	bool               SetCommonTables();

	static tukk GetAGInputName(EAIAGInput input);

	void               MakeFace(CAIFaceUpr::e_ExpressionEvent expression);

	IScriptTable* m_pScriptObject;
	IEntity*      m_pEntity;
	IGameObject*  m_pGameObject;

	bool          m_bSoundFinished;

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	SmartScriptTable m_pCharacter;
#endif
	SmartScriptTable m_pBehavior;
	SmartScriptTable m_pPreviousBehavior;
#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	SmartScriptTable m_pDefaultBehavior;

	SmartScriptTable m_pDefaultCharacter;
#endif
	SmartScriptTable m_pDEFAULTDefaultBehavior;
	SmartScriptTable m_pBehaviorTable;
	SmartScriptTable m_pBehaviorTableAVAILABLE;
	SmartScriptTable m_pBehaviorTableINTERNAL;

	i32              m_CurrentAlertness;
	bool             m_CurrentExclusive;

#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	bool                   m_bDelayedCharacterConstructor;
#endif
	bool                   m_bDelayedBehaviorConstructor;

	string                 m_sBehaviorName;
#ifdef USE_DEPRECATED_AI_CHARACTER_SYSTEM
	string                 m_sDefaultBehaviorName;
	string                 m_sCharacterName;
	string                 m_sPrevCharacterName;

	string                 m_sFirstCharacterName;
#endif
	string                 m_sFirstBehaviorName;

	IAnimationGraphState*  m_pAGState;
	string                 m_sQueriedActionAnimation;
	string                 m_sQueriedSignalAnimation;
	bool                   m_bSignaledAnimationStarted;

	typedef std::set<string> SetStrings;
	SetStrings                m_setPlayedSignalAnimations;
	SetStrings                m_setStartedActionAnimations;

	TAnimationGraphQueryID    m_actorTargetStartedQueryID;
	TAnimationGraphQueryID    m_actorTargetEndQueryID;
	TExactPositioningQueryID* m_curActorTargetStartedQueryID;
	TExactPositioningQueryID* m_curActorTargetEndQueryID;
	bool                      m_bAnimationStarted;
	EActorTargetPhase         m_eActorTargetPhase;
	i32                       m_actorTargetId;

	string                    m_sAGActionSOAutoState;

	bool                      m_playingSignalAnimation;
	bool                      m_playingActionAnimation;
	string                    m_currentSignalAnimName;
	string                    m_currentActionAnimName;

	Vec3                      m_vAnimationTargetPosition;
	Quat                      m_qAnimationTargetOrientation;

	CAIFaceUpr            m_FaceUpr;

	EAITargetType             m_lastTargetType;
	EAITargetThreat           m_lastTargetThreat;
	tAIObjectID               m_lastTargetID;
	Vec3                      m_lastTargetPos;

	struct SAnimActionTracker
	{
		SAnimActionTracker() { m_animActions.reserve(4); }

		void ReleaseAll();                         // cancel all animactions and deregister listeners
		void ForceFinishAll();                     // forcefinish all animactions (keep listening for events)
		void StopAll();                            // stop all animactions (keep listening for events)
		void StopAllActionsOfType(bool isOneShot); // stop all animactions of specified type (keep listening for events)
		void Add(CAnimActionTriState* pAction);
		void Remove(CAnimActionTriState* pAction);
		bool IsNotEmpty() const { return !m_animActions.empty(); }

	private:
		typedef _smart_ptr<CAnimActionTriState> AnimActionPtr;
		typedef std::vector<AnimActionPtr>      TAnimActionVector;
		TAnimActionVector m_animActions;
	};

	SAnimActionTracker m_animActionTracker;

private:
	void DoReadibilityPackForAIObjectsOfType(u16 nType, tukk szText, float fResponseDelay);

	float m_timeSinceEvent;
};

#endif // __AIHandler_H__
