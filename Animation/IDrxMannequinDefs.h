// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __I_DRX_MANNEQUINDEFS_H__
#define __I_DRX_MANNEQUINDEFS_H__

#define STORE_TAG_STRINGS 1 //TODO: !_RELEASE

// Set this to 0 to remove animation name strings. This saves about 15% of memory per ADB, but makes debugging very hard
#define STORE_ANIMNAME_STRINGS 1

#if defined(_RELEASE)
	#define STORE_PROCCLIP_STRINGS 0
	#define STORE_SCOPE_STRINGS    0
#else
	#define STORE_PROCCLIP_STRINGS 1
	#define STORE_SCOPE_STRINGS    1
#endif

typedef i32 TagID; // should be u32, but it's too scary because we regularly check (tagID >= 0) instead of (tagID != TAG_ID_INVALID)
static const TagID TAG_ID_INVALID = ~TagID(0);

typedef i32 TagGroupID; // should be u32, but it's too scary because we regularly check (tagGroupID >= 0) instead of (tagGroupID != TAG_ID_INVALID)
static const TagGroupID GROUP_ID_NONE = ~TagGroupID(0);

typedef TagID FragmentID; // should be u32, but it's too scary because we regularly check (fragmentID >= 0) instead of (fragmentID != TAG_ID_INVALID)
static const FragmentID FRAGMENT_ID_INVALID = TAG_ID_INVALID;

static const TagID SCOPE_ID_INVALID = TAG_ID_INVALID;

typedef u32 AnimCRC; // CRC of the name of the animation
static const AnimCRC ANIM_CRC_INVALID = AnimCRC(0);

static u32k OPTION_IDX_RANDOM = 0xfffffffe;
static u32k OPTION_IDX_INVALID = 0xffffffff;

static u32k TAG_SET_IDX_INVALID = ~u32(0);

typedef uint64 ActionScopes;
static const ActionScopes ACTION_SCOPES_ALL = ~ActionScopes(0);
static const ActionScopes ACTION_SCOPES_NONE = ActionScopes(0);

//suggestion: typedef u32 ScopeContextId;
static const TagID SCOPE_CONTEXT_ID_INVALID = TAG_ID_INVALID;

struct SCRCRefHash_CRC32Lowercase
{
	typedef u32 TInt;
	static const TInt INVALID = 0;

	static TInt       CalculateHash(tukk const s)
	{
		assert(s);
		assert(s[0]);

		const TInt crc = CCrc32::ComputeLowercase(s);
		if (crc == INVALID)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Congratulations, you hit the jackpot! The string '%s' has a lowercase CRC32 equal to %u. Unfortunately this number is reserved for errors, so please rename and try again...", s, INVALID);
		}
		return crc;
	}
};

struct SCRCRefHash_CRC32
{
	typedef u32 TInt;
	static const TInt INVALID = 0;

	static TInt       CalculateHash(tukk const s)
	{
		assert(s);
		assert(s[0]);

		const TInt crc = CCrc32::Compute(s);
		if (crc == INVALID)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Congratulations, you hit the jackpot! The string '%s' has a CRC32 equal to %u. Unfortunately this number is reserved for errors, so please rename and try again...", s, INVALID);
		}
		return crc;
	}
};

template<u32 StoreStrings, typename THash = SCRCRefHash_CRC32Lowercase>
struct SCRCRef;

template<typename THash>
struct SCRCRef<0, THash>
{
	typedef typename THash::TInt TInt;
	static const TInt INVALID = THash::INVALID;

	SCRCRef()
		: crc(INVALID)
	{
	}

	explicit SCRCRef(tukk const nameString)
	{
		SetByString(nameString);
	}

	void SetByString(tukk const nameString)
	{
		if (nameString && (nameString[0] != '\0'))
		{
			crc = THash::CalculateHash(nameString);
		}
		else
		{
			crc = INVALID;
		}
	}

	ILINE bool IsEmpty() const
	{
		return (crc == INVALID);
	}

	ILINE tukk c_str() const
	{
		if (crc == INVALID)
		{
			return "";
		}
		return "STRIPPED_CRC_NAMES";
	}

	ILINE SCRCRef<0>& operator=(tukk const s)
	{
		SetByString(s);
		return *this;
	}

	template<u32 StoreStringsRhs>
	ILINE bool operator==(const SCRCRef<StoreStringsRhs, THash>& rhs) const
	{
		return (crc == rhs.crc);
	}

	template<u32 StoreStringsRhs>
	ILINE bool operator!=(const SCRCRef<StoreStringsRhs, THash>& rhs) const
	{
		return (crc != rhs.crc);
	}

	template<u32 StoreStringsRhs>
	ILINE bool operator<(const SCRCRef<StoreStringsRhs, THash>& rhs) const
	{
		return (crc < rhs.crc);
	}

	ILINE u32 ToUInt32() const
	{
		return static_cast<u32>(crc);
	}

	TInt crc;
};

template<typename THash>
struct SCRCRef<1, THash>
{
	typedef typename THash::TInt TInt;
	static const TInt INVALID = THash::INVALID;

	SCRCRef()
		: crc(INVALID)
		, stringValue()
	{
	}

	explicit SCRCRef(tukk const nameString)
		: crc(INVALID)
		, stringValue()
	{
		SetByString(nameString);
	}

	SCRCRef(const SCRCRef<1>& other)
		: crc(INVALID)
		, stringValue()
	{
		SetByString(other.c_str());
	}

	SCRCRef<1>& operator=(const SCRCRef<1>& other)
	{
		if (&other != this)
		{
			SetByString(other.c_str());
		}
		return *this;
	}

	void SetByString(tukk const nameString)
	{
		if (nameString && (nameString[0] != '\0'))
		{
			const size_t lengthPlusOne = strlen(nameString) + 1;
			stringValue.assign(nameString, nameString + lengthPlusOne);

			crc = THash::CalculateHash(nameString);
		}
		else
		{
			stringValue.clear();
			crc = INVALID;
		}
	}

	ILINE bool IsEmpty() const
	{
		return (crc == INVALID);
	}

	ILINE tukk c_str() const
	{
		return stringValue.empty() ? "" : stringValue.data();
	}

	ILINE SCRCRef<1>& operator=(tukk const s)
	{
		SetByString(s);
		return *this;
	}

	template<u32 StoreStringsRhs>
	ILINE bool operator==(const SCRCRef<StoreStringsRhs, THash>& rhs) const
	{
		return (crc == rhs.crc);
	}

	template<u32 StoreStringsRhs>
	ILINE bool operator!=(const SCRCRef<StoreStringsRhs, THash>& rhs) const
	{
		return (crc != rhs.crc);
	}

	template<u32 StoreStringsRhs>
	ILINE bool operator<(const SCRCRef<StoreStringsRhs, THash>& rhs) const
	{
		return (crc < rhs.crc);
	}

	ILINE u32 ToUInt32() const
	{
		return static_cast<u32>(crc);
	}

	TInt crc;

private:
	DynArray<char> stringValue;
};

typedef SCRCRef<STORE_TAG_STRINGS>      STagRef;
typedef SCRCRef<STORE_ANIMNAME_STRINGS> SAnimRef;
typedef SCRCRef<STORE_SCOPE_STRINGS>    SScopeRef;

u32k DEF_PATH_LENGTH = 512;
typedef SCRCRef<1>                      TDefPathString;

typedef SCRCRef<STORE_PROCCLIP_STRINGS> SProcDataCRC;

typedef SCRCRef<1>                      TProcClipTypeName;

typedef CDrxName                        TProcClipString;

enum eSequenceFlags
{
	eSF_Fragment        = BIT(0),
	eSF_TransitionOutro = BIT(1),
	eSF_Transition      = BIT(2)
};

struct SWeightData
{
	enum { MAX_WEIGHTS = 4 };

	SWeightData() { memset(&weights, 0, MAX_WEIGHTS * sizeof(weights[0])); }

	float weights[MAX_WEIGHTS];
};

#endif //__I_DRX_MANNEQUINDEFS_H__
