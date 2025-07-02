// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CFacialInstance;
class CFacialEffectorsLibrary;
class CFaceState;
class CFaceIdentifierHandle;
class CCharInstance;

#define EYEMOVEMENT_EFFECTOR_AMOUNT 16

class CEyeMovementFaceAnim : public _i_reference_target_t
{
public:
	CEyeMovementFaceAnim(CFacialInstance* pInstance);

	void Update(float fDeltaTimeSec, const QuatTS& rAnimLocationNext, CCharInstance* pCharacter, CFacialEffectorsLibrary* pEffectorsLibrary, CFaceState* pFaceState);
	void OnExpressionLibraryLoad();

	void GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	enum DirectionID
	{
		DirectionNONE  = -1,

		DirectionEyeUp = 0,
		DirectionEyeUpRight,
		DirectionEyeRight,
		DirectionEyeDownRight,
		DirectionEyeDown,
		DirectionEyeDownLeft,
		DirectionEyeLeft,
		DirectionEyeUpLeft,

		DirectionCOUNT
	};

	enum EyeID
	{
		EyeLeft,
		EyeRight,

		EyeCOUNT
	};

	enum EffectorID
	{
		EffectorEyeLeftDirectionEyeUp,
		EffectorEyeLeftDirectionEyeUpRight,
		EffectorEyeLeftDirectionEyeRight,
		EffectorEyeLeftDirectionEyeDownRight,
		EffectorEyeLeftDirectionEyeDown,
		EffectorEyeLeftDirectionEyeDownLeft,
		EffectorEyeLeftDirectionEyeLeft,
		EffectorEyeLeftDirectionEyeUpLeft,
		EffectorEyeRightDirectionEyeUp,
		EffectorEyeRightDirectionEyeUpRight,
		EffectorEyeRightDirectionEyeRight,
		EffectorEyeRightDirectionEyeDownRight,
		EffectorEyeRightDirectionEyeDown,
		EffectorEyeRightDirectionEyeDownLeft,
		EffectorEyeRightDirectionEyeLeft,
		EffectorEyeRightDirectionEyeUpLeft,

		EffectorCOUNT
	};

	EffectorID EffectorFromEyeAndDirection(EyeID eye, DirectionID direction)
	{
		assert(eye >= 0 && eye < EyeCOUNT);
		assert(direction >= 0 && direction < DirectionCOUNT);
		return static_cast<EffectorID>(eye * DirectionCOUNT + direction);
	}

	EyeID EyeFromEffector(EffectorID effector)
	{
		assert(effector >= 0 && effector < EffectorCOUNT);
		return static_cast<EyeID>(effector / DirectionCOUNT);
	}

	DirectionID DirectionFromEffector(EffectorID effector)
	{
		assert(effector >= 0 && effector < EffectorCOUNT);
		return static_cast<DirectionID>(effector % DirectionCOUNT);
	}

	void                         InitialiseChannels();
	u32                       GetChannelForEffector(EffectorID effector);
	u32                       CreateChannelForEffector(EffectorID effector);
	void                         UpdateEye(const QuatTS& rAnimLocationNext, EyeID eye, const QuatT& additionalRotation);
	DirectionID                  FindEyeDirection(EyeID eye);
	void                         InitialiseBoneIDs();
	void                         FindLookAngleAndStrength(EyeID eye, float& angle, float& strength, const QuatT& additionalRotation);
	void                         DisplayDebugInfoForEye(const QuatTS& rAnimLocationNext, EyeID eye, const string& text);
	void                         CalculateEyeAdditionalRotation(CCharInstance* pCharacter, CFaceState* pFaceState, CFacialEffectorsLibrary* pEffectorsLibrary, QuatT additionalRotation[EyeCOUNT]);
	const CFaceIdentifierHandle* RetrieveEffectorIdentifiers() const;

	CFacialInstance*   m_pInstance;
	static tukk szEyeBoneNames[EyeCOUNT];
	static u32      ms_eyeBoneNamesCrc[EyeCOUNT];

	class CChannelEntry
	{
	public:
		CChannelEntry() : id(~0), loadingAttempted(false) {}
		i32  id;
		bool loadingAttempted;
	};

	CChannelEntry m_channelEntries[EffectorCOUNT];
	i32           m_eyeBoneIDs[EyeCOUNT];
	bool          m_bInitialized;
};
