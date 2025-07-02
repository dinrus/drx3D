// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ICoverSystem_h__
#define __ICoverSystem_h__

#pragma once

struct CoverID
{
	ILINE CoverID()
		: id(0)
	{
	}

	ILINE explicit CoverID(u32 _id)
		: id(_id)
	{
	}

	ILINE CoverID& operator=(const CoverID& other)
	{
		id = other.id;
		return *this;
	}

	ILINE bool operator==(const CoverID& other) const
	{
		return id == other.id;
	}

	ILINE operator u32() const
	{
		return id;
	}

private:
	u32 id;
};

struct CoverSurfaceID
{
	ILINE CoverSurfaceID()
		: id(0)
	{
	}

	ILINE explicit CoverSurfaceID(u32 _id)
		: id(_id)
	{
	}

	ILINE CoverSurfaceID& operator=(const CoverSurfaceID& other)
	{
		id = other.id;
		return *this;
	}

	ILINE operator u32() const
	{
		return id;
	}

private:
	u32 id;
};

u32k CoverIDSurfaceIDShift = 16;
u32k CoverIDLocationIDMask = 0xffff;

struct ICoverSampler
{
	enum ESamplerState
	{
		None = 0,
		InProgress,
		Finished,
		Error,
	};

	struct Sample
	{
		enum
		{
			IntegerPartBitCount = 12,
		};

		enum ESampleFlags
		{
			Edge    = 1 << 0,
			Dynamic = 1 << 1,
		};

		Sample()
		{
		}

		Sample(const Vec3& _position, float _height, u16 _flags = 0)
			: position(_position)
			, height((u16)(_height * ((1 << IntegerPartBitCount) - 1)))
			, flags(_flags)
		{
		}

		Vec3       position; //!< Warning: in CCoverSystem::ReadSurfacesFromFile the current ordering is assumed.

		u16     height; //!< Unsigned fixed point 4:12.
		u16     flags;

		ILINE void SetHeight(float _height)
		{
			height = (u16)(_height * ((1 << IntegerPartBitCount) - 1));
		}

		ILINE float GetHeight() const
		{
			return height * (1.0f / (float)((1 << IntegerPartBitCount) - 1));
		}

		ILINE i32 GetHeightInteger() const
		{
			return height;
		}

		static ILINE float GetHeightToFloatConverter()
		{
			return (1.0f / (float)((1 << IntegerPartBitCount) - 1));
		}
	};

	struct Params
	{
		Params()
			: position(ZERO)
			, direction(ZERO)
			, limitDepth(0.75f)
			, limitHeight(2.75f)
			, limitLeft(12.0f)
			, limitRight(12.0f)
			, heightSamplerInterval(0.25f)
			, widthSamplerInterval(0.25f)
			, floorSearchHeight(1.5f)
			, floorSearchRadius(0.5f)
			, minHeight(0.85f)
			, maxStartHeight(0.5f)
			, simplifyThreshold(0.075f)
			, heightAccuracy(0.05f)
			, maxCurvatureAngleCos(-2.0f)      //!< Less than -1.0f means infinite.
			, referenceEntity(0)
		{
		}

		Vec3     position;
		Vec3     direction;

		float    limitDepth;
		float    limitHeight;
		float    limitLeft;
		float    limitRight;

		float    heightSamplerInterval;
		float    widthSamplerInterval;

		float    floorSearchHeight;
		float    floorSearchRadius;

		float    minHeight;
		float    maxStartHeight;

		float    simplifyThreshold;

		float    heightAccuracy;
		float    maxCurvatureAngleCos;
		IEntity* referenceEntity;
	};

	// <interfuscator:shuffle>
	virtual ~ICoverSampler(){}
	virtual void                 Release() = 0;
	virtual ESamplerState        StartSampling(const Params& params) = 0;
	virtual ESamplerState        Update(float timeLimitPerFrame = 0.00015f, float timeLimitTotal = 2.0f) = 0;
	virtual ESamplerState        GetState() const = 0;

	virtual u32               GetSampleCount() const = 0;
	virtual const struct Sample* GetSamples() const = 0;
	virtual const AABB&          GetAABB() const = 0;
	virtual u32               GetSurfaceFlags() const = 0;

	virtual void                 DebugDraw() const = 0;
	// </interfuscator:shuffle>
};

struct ICoverUser
{
	typedef Functor2<CoverID, ICoverUser*> CoverInvalidatedCallback;
	typedef Functor1<DynArray<Vec3>&> FillCoverEyesCustomMethod;

	enum EStateFlags : u8
	{
		None = 0,
		MovingToCover = BIT(0),
		InCover = BIT(1),
	};
	typedef CEnumFlags<EStateFlags> StateFlags;

	struct Params
	{
		Params()
			: distanceToCover(0.4f)
			, inCoverRadius(0.3f)
			, minEffectiveCoverHeight(0.85f)
			, userID(0)
		{
		}

		float                     distanceToCover;
		float                     inCoverRadius;
		float                     minEffectiveCoverHeight;
		EntityId                  userID;
		FillCoverEyesCustomMethod fillCoverEyesCustomMethod;
		CoverInvalidatedCallback  activeCoverCompromisedCallback;
		CoverInvalidatedCallback  activeCoverInvalidateCallback;
	};

	virtual ~ICoverUser() {}
	virtual void           Reset() = 0;

	virtual void           SetCoverID(const CoverID& coverID) = 0;
	virtual const CoverID& GetCoverID() const = 0;

	virtual void           SetNextCoverID(const CoverID& coverID) = 0;
	virtual const CoverID& GetNextCoverID() const = 0;

	virtual void           SetParams(const Params& params) = 0;
	virtual const Params&  GetParams() const = 0;

	virtual StateFlags     GetState() const = 0;
	virtual void           SetState(const StateFlags& stateFlags) = 0;

	virtual bool           IsCompromised() const = 0;
	virtual float          CalculateEffectiveHeightAt(const Vec3& pos, const CoverID& coverId) const = 0;
	virtual float          GetLocationEffectiveHeight() const = 0;

	virtual void SetCoverBlacklisted(const CoverID& coverID, bool blacklist, float time) = 0;
	virtual bool IsCoverBlackListed(const CoverID& coverId) const = 0;

	// normal pointing out of the cover surface
	virtual Vec3        GetCoverNormal(const Vec3& position) const = 0;
	virtual Vec3        GetCoverLocation(const CoverID& coverID) const = 0;

	virtual void UpdateCoverEyes() = 0;
	virtual const DynArray<Vec3>& GetCoverEyes() const = 0;
};

struct ICoverSystem
{
	struct SurfaceInfo
	{
		enum SurfaceFlags
		{
			Looped  = 1 << 0,
			Dynamic = 1 << 1,
		};

		SurfaceInfo()
			: samples(0)
			, sampleCount(0)
		{
		}

		const ICoverSampler::Sample* samples;
		u32                       sampleCount; //!< Warning: in CCoverSystem::ReadSurfacesFromFile it is assumed sampleCount is followed by flags.
		u32                       flags;
	};

	// <interfuscator:shuffle>
	virtual ~ICoverSystem(){}
	virtual ICoverSampler* CreateCoverSampler(tukk samplerName = "default") = 0;

	virtual ICoverUser*    RegisterEntity(const EntityId entityId, const ICoverUser::Params& params) = 0;
	virtual void           UnregisterEntity(const EntityId entityId) = 0;
	virtual ICoverUser*    GetRegisteredCoverUser(const EntityId entityId) const = 0;

	virtual void           Clear() = 0;
	virtual bool           ReadSurfacesFromFile(tukk fileName) = 0;

	virtual CoverSurfaceID AddSurface(const SurfaceInfo& surfaceInfo) = 0;
	virtual void           RemoveSurface(const CoverSurfaceID& surfaceID) = 0;
	virtual void           UpdateSurface(const CoverSurfaceID& surfaceID, const SurfaceInfo& surfaceInfo) = 0;

	virtual u32         GetSurfaceCount() const = 0;
	virtual bool           GetSurfaceInfo(const CoverSurfaceID& surfaceID, SurfaceInfo* surfaceInfo) const = 0;

	virtual u32         GetCover(const Vec3& center, float range, const Vec3* eyes, u32 eyeCount, float distanceToCover, Vec3* locations, u32 maxLocationCount, u32 maxLocationsPerSurface) const = 0;

	virtual void           DrawSurface(const CoverSurfaceID& surfaceID) = 0;
	// </interfuscator:shuffle>
};

#endif // __ICoverSystem_h
