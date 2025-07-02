// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#define MAX_NUM_DB                   4
#define MAX_POLAR_COORDINATES_SMOOTH (MAX_EXEC_QUEUE)

struct SDirInfo
{
	i32 m_nGlobalDirID0;
	f32   m_fWeight;
};

struct DBW
{
	u32 m_nAnimTokenCRC32;
	i32  m_nParaJointIdx;
	i32  m_nRotParaJointIdx;
	i32  m_nStartJointIdx;
	i32  m_nRotStartJointIdx;
	f32    m_fWeight;

	DBW()
	{
		m_nAnimTokenCRC32 = 0;
		m_nParaJointIdx = -1;
		m_nRotParaJointIdx = -1;
		m_nStartJointIdx = -1;
		m_nRotStartJointIdx = -1;
		m_fWeight = 0;
	}
};

struct SDirectionalBlender
{
private:
	friend class CPoseBlenderAim;
	friend class CPoseBlenderLook;

public:
	SDirectionalBlender() { Init(); }
	bool ExecuteDirectionalIK(const SAnimationPoseModifierParams& params, const DirectionalBlends* rDirBlends, u32k numDB, const SJointsAimIK_Rot* rRot, u32k numRotJoints, const SJointsAimIK_Pos* rPos, u32k numPosJoints);
	void AccumulateAimPoses(const SAnimationPoseModifierParams& params, const GlobalAnimationHeaderAIM& rAIM, const QuatT* arrRelPose, const f32 fIKBlend, const Vec3& rAimTarget, i32k nParaJointIdx, i32k nRotParaJointIdx, i32k nStartJointIdx, i32k nRotStartJointIdx, const SJointsAimIK_Rot* rRot, u32k numRotJoints, const SJointsAimIK_Pos* rPos, const f32 CosineY, u32k sides, u32k polarCoordinatesIndex);
	void DebugVEGrid(const SAnimationPoseModifierParams& params, const SDirInfo& rAimInfo, const f32 fAimPoseWeight, const f32 fIKBlend, i32k widx, const SJointsAimIK_Rot* rRot, u32k numRotJoints, const SJointsAimIK_Pos* rPos, u32k numPosJoints);

	void Init()
	{
		m_Set = SDoubleBufferedDataIn();
		m_dataIn = SDoubleBufferedDataIn();

		m_dataOut = SDoubleBufferedDataOut();
		m_Get = SDoubleBufferedDataOut();

		m_nDirIKDistanceFadeOut = 0;
		m_fFieldOfViewSmooth = 0.0f;
		m_fFieldOfViewRate = 0.0f;

		for (u32 i = 0; i < MAX_EXEC_QUEUE * 2; i++)
		{
			m_DirInfo[i].m_fWeight = 0;
			m_DirInfo[i].m_nGlobalDirID0 = -1;
		}

		for (i32 i = 0; i < 4; ++i)
		{
			m_polarCoordinatesSmooth[i] = SPolarCoordinatesSmooth();
		}
	}

	void ClearSmoothingRates()
	{
		m_fFieldOfViewRate = 0.0f;

		for (i32 i = 0; i < MAX_POLAR_COORDINATES_SMOOTH; ++i)
		{
			m_polarCoordinatesSmooth[i].rate.zero();
			m_polarCoordinatesSmooth[i].shouldSnap = true;
		}
	}

	// Double buffering
	struct SDoubleBufferedDataIn
	{
		int8 bUseDirIK;
		int8 nDirLayer;
		Vec3 vDirIKTarget;
		f32  fDirIKFadeoutRadians;                 // look 180-degrees to the left and right
		f32  fDirIKFadeInTime;                     // 1.0f/TransitionTime
		f32  fDirIKFadeOutTime;                    // 1.0f/TransitionTime
		f32  fDirIKMinDistanceSquared;
		f32  fPolarCoordinatesSmoothTimeSeconds;
		Vec2 vPolarCoordinatesOffset;
		Vec2 vPolarCoordinatesMaxRadiansPerSecond;

		SDoubleBufferedDataIn()
			: bUseDirIK(0)
			, nDirLayer(ISkeletonAnim::LayerCount)
			, vDirIKTarget(ZERO)
			, fDirIKFadeoutRadians(gf_PI)
			, fDirIKFadeInTime(1.0 / 0.6f)
			, fDirIKFadeOutTime(1.0f / 0.3f)
			, fDirIKMinDistanceSquared(0.f)
			, fPolarCoordinatesSmoothTimeSeconds(0.2f)
			, vPolarCoordinatesOffset(ZERO)
			, vPolarCoordinatesMaxRadiansPerSecond(DEG2RAD(3600.f), DEG2RAD(3600.f))
		{
		}
	};

	struct SDoubleBufferedDataOut
	{
		f32 fDirIKInfluence;
		f32 fDirIKBlend;     // percentage value between animation and aim-ik

		SDoubleBufferedDataOut()
			: fDirIKInfluence(0.0f)
			, fDirIKBlend(0.0f)
		{
		}
	};

	SDoubleBufferedDataIn  m_Set;
	SDoubleBufferedDataOut m_Get;

	// NOTE: This need to be double-buffered!
	int8     m_numActiveDirPoses; //active animation in the FIFO queue
	SDirInfo m_DirInfo[MAX_EXEC_QUEUE * 2];

private:
	SDoubleBufferedDataIn  m_dataIn;
	SDoubleBufferedDataOut m_dataOut;

	int8                   m_nDirIKDistanceFadeOut; // set and deleted internally
	f32                    m_fFieldOfViewSmooth;
	f32                    m_fFieldOfViewRate;

	struct SPolarCoordinatesSmooth
	{
		Vec2   value;
		Vec2   rate;
		u32 id;
		bool   used;
		bool   shouldSnap;
		bool   smoothingDone;

		SPolarCoordinatesSmooth()
			: value(ZERO)
			, rate(ZERO)
			, id(0)
			, used(false)
			, shouldSnap(false)
			, smoothingDone(false)
		{
		}

		ILINE void Prepare()
		{
			used = smoothingDone;
			smoothingDone = false;
		}

		ILINE void Smooth(const Vec2& targetValue, const f32 deltaTimeSeconds, const f32 smoothTimeSeconds, const Vec2& maxRadiansPerSecond)
		{
			IF_UNLIKELY (smoothingDone)
			{
				return;
			}

			IF_UNLIKELY (shouldSnap)
			{
				value = targetValue;
				rate.zero();
				shouldSnap = false;
				smoothingDone = true;
				return;
			}

			SmoothCDWithMaxRate(value, rate, deltaTimeSeconds, targetValue, smoothTimeSeconds, maxRadiansPerSecond);
			smoothingDone = true;
		}
	};

	SPolarCoordinatesSmooth m_polarCoordinatesSmooth[MAX_POLAR_COORDINATES_SMOOTH];
	ILINE u32            GetOrAssignPolarCoordinateSmoothIndex(u32k id)
	{
		u32 freeIndex = MAX_POLAR_COORDINATES_SMOOTH;
		for (u32 i = 0; i < MAX_POLAR_COORDINATES_SMOOTH; ++i)
		{
			const SPolarCoordinatesSmooth& polarCoordinatesSmooth = m_polarCoordinatesSmooth[i];
			if (polarCoordinatesSmooth.used)
			{
				if (polarCoordinatesSmooth.id == id)
				{
					return i;
				}
			}
			else
			{
				freeIndex = i;
			}
		}

		if (freeIndex != MAX_POLAR_COORDINATES_SMOOTH)
		{
			SPolarCoordinatesSmooth& polarCoordinatesSmooth = m_polarCoordinatesSmooth[freeIndex];
			polarCoordinatesSmooth = SPolarCoordinatesSmooth();
			polarCoordinatesSmooth.id = id;
			polarCoordinatesSmooth.used = true;
			polarCoordinatesSmooth.shouldSnap = true;
			return freeIndex;
		}

		return MAX_POLAR_COORDINATES_SMOOTH;
	}

	DBW m_dbw[MAX_NUM_DB];
};
