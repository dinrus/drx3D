// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/CGFContent.h>
#include <drx3D/Animation/QuatQuantization.h>
#include <drx3D/Animation/ControllerDefragHeap.h>

class IControllerOpt;
class IKeyTimesInformation;
class IPositionController;
class IRotationController;
class ITrackPositionStorage;
class ITrackRotationStorage;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ControllerHelper
{
u32 GetPositionsFormatSizeOf(u32 format);
u32 GetRotationFormatSizeOf(u32 format);
u32 GetScalingFormatSizeOf(u32 format);
u32 GetKeyTimesFormatSizeOf(u32 format);

// TODO: Rename following methods to "construct".
ITrackPositionStorage* GetPositionControllerPtr(u32 format, uk pData = NULL, size_t numElements = 0);
ITrackRotationStorage* GetRotationControllerPtr(u32 format, uk pData = NULL, size_t numElements = 0);
ITrackPositionStorage* GetScalingControllerPtr(u32 format, uk pData = NULL, size_t numElements = 0);
IKeyTimesInformation*  GetKeyTimesControllerPtr(u32 format, uk pData = NULL, size_t numElements = 0);

extern u8k m_byteTable[256];
extern u8k m_byteLowBit[256];
extern u8k m_byteHighBit[256];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern CControllerDefragHeap g_controllerHeap;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IControllerRelocatableChain
{
public:

	virtual void Relocate(tuk pDst, tuk pSrc) = 0;

	IControllerRelocatableChain()
		: m_pNext(nullptr)
	{
	}

	void                         LinkTo(IControllerRelocatableChain* pNext) { m_pNext = pNext; }

	IControllerRelocatableChain* GetNext() const                            { return m_pNext; }

protected:

	virtual ~IControllerRelocatableChain() {}

private:

	IControllerRelocatableChain* m_pNext;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum EKeyTimesFormat
{
	// Don't add values which increase the maximum enum value over 255 because this will break IControllerOptNonVirtual!!!!
	eF32 = 0,
	eUINT16,
	eByte,
	eF32StartStop,
	eUINT16StartStop,
	eByteStartStop,
	eBitset,
	eNoFormat = 255
};

struct F32Encoder
{
	typedef f32    TKeyTypeTraits;
	typedef u32 TKeyTypeCountTraits;

	static EKeyTimesFormat GetFormat() { return eF32; };
};

struct UINT16Encoder
{
	typedef u16 TKeyTypeTraits;
	typedef u16 TKeyTypeCountTraits;

	static EKeyTimesFormat GetFormat() { return eUINT16; };
};

struct ByteEncoder
{
	typedef u8  TKeyTypeTraits;
	typedef u16 TKeyTypeCountTraits;

	static EKeyTimesFormat GetFormat() { return eByte; };
};

struct F32StartStopEncoder
{
	static EKeyTimesFormat GetFormat() { return eF32StartStop; };
};

struct UINT16StartStopEncoder
{
	static EKeyTimesFormat GetFormat() { return eUINT16StartStop; };
};

struct ByteStartStopEncoder
{
	static EKeyTimesFormat GetFormat() { return eByteStartStop; };
};

struct BitsetEncoder
{
	typedef u16 TKeyTypeTraits;
	typedef u16 TKeyTypeCountTraits;

	static EKeyTimesFormat GetFormat() { return eBitset; };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IKeyTimesInformation : public _reference_target_t
{
public:

	virtual ~IKeyTimesInformation() {}

	// return key value in f32 format
	virtual f32 GetKeyValueFloat(u32 key) const = 0;

	// return key number for normalized_time
	virtual u32                       GetKey(f32 normalized_time, f32& difference_time) = 0;

	virtual u32                       GetNumKeys() const = 0;

	virtual u32                       AssignKeyTime(tukk pData, u32 numElements) = 0;

	virtual u32                       GetFormat() = 0;

	virtual tuk                        GetData() = 0;

	virtual u32                       GetDataRawSize() = 0;

	virtual IControllerRelocatableChain* GetRelocateChain() = 0;
};

template<class TKeyTime, class TEncoder>
class CKeyTimesInformation : public IKeyTimesInformation, public IControllerRelocatableChain
{
public:

	typedef TKeyTime KeyTimeType;

	CKeyTimesInformation()
		: m_pKeys(nullptr)
		, m_numKeys(0)
		, m_lastTime(-1)
	{
	}

	CKeyTimesInformation(TKeyTime* pKeys, u32 numKeys)
		: m_pKeys(pKeys)
		, m_numKeys(numKeys | (numKeys > 0 ? kKeysConstantMask : 0))
		, m_lastTime(-1)
	{
	}

	~CKeyTimesInformation()
	{
		if (!IsConstant() && m_hHdl.IsValid())
			g_controllerHeap.Free(m_hHdl);
	}

	// return key value
	TKeyTime GetKeyValue(u32 key) const { return m_pKeys[key]; };

	// return key value in f32 format
	f32 GetKeyValueFloat(u32 key) const { return (m_pKeys[key]); }

	// return key number for normalized_time
	u32 GetKey(f32 normalized_time, f32& difference_time)
	{
		TKeyTime* pKeys = m_pKeys;
		u32 numKey = GetNumKeys_Int();

		f32 realtime = normalized_time;

		/*
		   if (realtime == m_lastTime)
		   {
		   difference_time =  m_LastDifferenceTime;
		   return m_lastKey;
		   }*/

		m_lastTime = realtime;

		TKeyTime keytime_start = pKeys[0];
		TKeyTime keytime_end = pKeys[numKey - 1];

		f32 test_end = keytime_end;
		if (realtime < keytime_start)
			test_end += realtime;

		if (realtime < keytime_start)
		{
			m_lastKey = 0;
			return 0;
		}

		if (realtime >= keytime_end)
		{
			m_lastKey = numKey;
			return numKey;
		}

		i32 nPos = numKey >> 1;
		i32 nStep = numKey >> 2;

		// use binary search
		while (nStep)
		{
			if (realtime < pKeys[nPos])
				nPos = nPos - nStep;
			else if (realtime > pKeys[nPos])
				nPos = nPos + nStep;
			else
				break;

			nStep = nStep >> 1;
		}

		// fine-tuning needed since time is not linear
		while (realtime > pKeys[nPos])
			nPos++;

		while (realtime < pKeys[nPos - 1])
			nPos--;

		m_lastKey = nPos;

		// possible error if encoder uses nonlinear methods!!!
		m_LastDifferenceTime = difference_time = /*TEncoder::DecodeKeyValue*/ (f32)(realtime - (f32)pKeys[nPos - 1]) / ((f32)pKeys[nPos] - (f32)pKeys[nPos - 1]);

		return nPos;

	}

	u32 GetNumKeys() const { return GetNumKeys_Int(); };

	u32 AssignKeyTime(tukk pData, u32 numElements)
	{
#ifndef _RELEASE
		if (IsConstant())
		{
			__debugbreak();
			return 0;
		}
#endif

		if (m_hHdl.IsValid())
			g_controllerHeap.Free(m_hHdl);

		u32 sz = numElements * sizeof(TKeyTime);

		this->LinkTo(NULL);
		CControllerDefragHdl hdl = g_controllerHeap.AllocPinned(sz, this);
		m_pKeys = (TKeyTime*)g_controllerHeap.WeakPin(hdl);
		m_hHdl = hdl;
		m_numKeys = numElements;

		memcpy(m_pKeys, pData, sz);
		g_controllerHeap.Unpin(hdl);

		return sz;
	}

	u32 GetFormat()
	{
		return TEncoder::GetFormat();
	}

	tuk GetData()
	{
		return (tuk) m_pKeys;
	}

	u32 GetDataRawSize()
	{
		return GetNumKeys_Int() * sizeof(TKeyTime);
	}

	IControllerRelocatableChain* GetRelocateChain()
	{
		return this;
	}

	void Relocate(tuk pDst, tuk pSrc)
	{
		ptrdiff_t offs = reinterpret_cast<tuk>(m_pKeys) - pSrc;
		m_pKeys = reinterpret_cast<TKeyTime*>(pDst + offs);
	}

private:

	CKeyTimesInformation(const CKeyTimesInformation<TKeyTime, TEncoder>&);  //= delete;
	void operator=(const CKeyTimesInformation<TKeyTime, TEncoder>&);        //= delete;

	enum
	{
		kKeysSizeMask     = 0x7fffffff,
		kKeysConstantMask = 0x80000000,
	};

	ILINE u32 IsConstant() const     { return m_numKeys & kKeysConstantMask; }
	ILINE u32 GetNumKeys_Int() const { return m_numKeys & kKeysSizeMask; }

	TKeyTime*            m_pKeys;
	CControllerDefragHdl m_hHdl;
	u32               m_numKeys;
	f32                  m_lastTime;
	f32                  m_LastDifferenceTime;
	u32               m_lastKey;
};

typedef CKeyTimesInformation<f32, F32Encoder>       F32KeyTimesInformation;
typedef CKeyTimesInformation<u16, UINT16Encoder> UINT16KeyTimesInformation;
typedef CKeyTimesInformation<u8, ByteEncoder>    ByteKeyTimesInformation;

template<class TKeyTime, class TEncoder>
class CKeyTimesInformationStartStop : public IKeyTimesInformation
{
public:

	CKeyTimesInformationStartStop() { m_arrKeys[0] = (TKeyTime)0; m_arrKeys[1] = (TKeyTime)0; };
	CKeyTimesInformationStartStop(const TKeyTime* pKeys, u32 numKeys) { assert(numKeys >= 2); m_arrKeys[0] = pKeys[0]; m_arrKeys[1] = pKeys[1]; };

	// return key value in f32 format
	f32 GetKeyValueFloat(u32 key) const { return /*TEncoder::DecodeKeyValue*/ (float)(m_arrKeys[0] + key); }

	// return key number for normalized_time
	u32 GetKey(f32 normalized_time, f32& difference_time)
	{
		f32 realtime = normalized_time;

		if (realtime < m_arrKeys[0])
			return 0;

		if (realtime > m_arrKeys[1])
			return (u32)(m_arrKeys[1] - m_arrKeys[0]);

		u32 nKey = (u32)realtime;
		difference_time = realtime - (float)(nKey);

		return nKey;
	}

	u32 GetNumKeys() const { return (u32)(m_arrKeys[1] - m_arrKeys[0]); }

	u32 AssignKeyTime(tukk pData, u32 numElements)
	{
		memcpy(m_arrKeys, pData, sizeof(TKeyTime) * 2);
		return sizeof(TKeyTime) * 2;
	}

	u32 GetFormat()
	{
		return TEncoder::GetFormat();
	}

	tuk GetData()
	{
		return (tuk)(&m_arrKeys[0]);
	}

	u32 GetDataRawSize()
	{
		return sizeof(TKeyTime) * 2;
	}

	IControllerRelocatableChain* GetRelocateChain()
	{
		return NULL;
	}

private:

	TKeyTime m_arrKeys[2]; // start pos, stop pos.
};

typedef CKeyTimesInformationStartStop<f32, F32StartStopEncoder>       F32SSKeyTimesInformation;
typedef CKeyTimesInformationStartStop<u16, UINT16StartStopEncoder> UINT16SSKeyTimesInformation;
typedef CKeyTimesInformationStartStop<u8, ByteStartStopEncoder>    ByteSSKeyTimesInformation;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move these functions to an utility header
#if DRX_PLATFORM_WINDOWS

	#include <intrin.h>
	#pragma intrinsic(_BitScanForward)
	#pragma intrinsic(_BitScanReverse)

inline u16 GetFirstLowBit(u16 word)
{
	u64 lword(word);
	u64 index;
	if (_BitScanForward(&index, lword))
		return (u16)(index);

	return 16;
}

inline u16 GetFirstHighBit(u16 word)
{
	u64 lword(word);
	u64 index;
	if (_BitScanReverse(&index, lword))
		return (u16)(index);

	return 16;
}

#else   // DRX_PLATFORM_WINDOWS

inline u16 GetFirstLowBitTest(u16 word)
{
	u16 c(0);

	if (word & 1)
		return 0;

	u16 b;
	do
	{
		word = word >> 1;
		b = word & 1;
		++c;
	}
	while (!b && c < 16);

	return c;
}

inline u16 GetFirstHighBitTest(u16 word)
{

	u16 c(0);

	if (word & 0x8000)
		return 15;

	u16 b;
	do
	{
		word = word << 1;
		b = word & 0x8000;
		++c;
	}
	while (!b && c < 16);

	if (c == 16)
		return c;
	else
		return 15 - c;
}

inline u16 GetFirstLowBit(u16 word)
{

	u8 rr = word & 0xFF;
	u16 res = ControllerHelper::m_byteLowBit[rr];
	if (res == 8)
	{
		res = (ControllerHelper::m_byteLowBit[(word >> 8) & 0xFF]) + 8;
	}

	//if (res != GetFirstLowBitTest(word))
	//	i32 a = 0;
	return res;
}

inline u16 GetFirstHighBit(u16 word)
{
	u16 res = ControllerHelper::m_byteHighBit[(word >> 8) & 0xFF] + 8;
	if (res == 24)
	{
		res = (ControllerHelper::m_byteHighBit[word & 0xFF]);
	}
	//if (res != GetFirstHighBitTest(word))
	//	i32 a = 0;

	return res;
}

#endif   // DRX_PLATFORM_WINDOWS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CKeyTimesInformationBitSet : public IKeyTimesInformation
{
public:

	CKeyTimesInformationBitSet()
		: m_pKeys(nullptr)
		, m_numKeys(0)
		, m_lastTime(-1)
	{
	}

	CKeyTimesInformationBitSet(u16* pKeys, u32 numKeys)
		: m_pKeys(pKeys)
		, m_numKeys((numKeys > 0) ? (numKeys | kKeysConstantMask) : 0)
		, m_lastTime(-1)
	{
	}

	~CKeyTimesInformationBitSet()
	{
		if (m_pKeys && !IsConstant())
			DrxModuleFree(m_pKeys);
	}

	// return key value in f32 format
	f32 GetKeyValueFloat(u32 key) const
	{
		if (key == 0)
		{
			// first one

			return (float)GetHeader()->m_Start;
		}
		else if (key >= GetNumKeys() - 1)
		{
			// last one
			return (float)GetHeader()->m_End;
		}
		// worse situation
		i32 keys = GetNumKeys();
		i32 count(0);
		for (i32 i = 0; i < keys; ++i)
		{
			u16 bits = GetKeyData(i);

			for (i32 j = 0; i < 16; ++i)
			{
				if ((bits >> j) & 1)
				{
					++count;
					if ((count - 1) == key)
						return (float)(i * 16 + j);
				}
			}
		}

		return 0;
	}

	// return key number for normalized_time
	u32 GetKey(f32 normalized_time, f32& difference_time)
	{
		f32 realtime = normalized_time;

		if (realtime == m_lastTime)
		{
			difference_time = m_LastDifferenceTime;
			return m_lastKey;
		}
		m_lastTime = realtime;
		u32 numKey = (u32)GetHeader()->m_Size;  //m_arrKeys.size();

		f32 keytime_start = (float)GetHeader()->m_Start;
		f32 keytime_end = (float)GetHeader()->m_End;
		f32 test_end = keytime_end;

		if (realtime < keytime_start)
			test_end += realtime;

		if (realtime < keytime_start)
		{
			difference_time = 0;
			m_lastKey = 0;
			return 0;
		}

		if (realtime >= keytime_end)
		{
			difference_time = 0;
			m_lastKey = numKey;
			return numKey;
		}

		f32 internalTime = realtime - keytime_start;
		u16 uTime = (u16)internalTime;
		u16 piece = (uTime / sizeof(u16)) >> 3;
		u16 bit = /*15 - */ (uTime % 16);
		u16 data = GetKeyData(piece);

		//left
		u16 left = data & (0xFFFF >> (15 - bit));
		u16 leftPiece(piece);
		u16 nearestLeft = 0;
		u16 wBit;
		while ((wBit = GetFirstHighBit(left)) == 16)
		{
			--leftPiece;
			left = GetKeyData(leftPiece);
		}
		nearestLeft = leftPiece * 16 + wBit;

		//right
		u16 right = ((data >> (bit + 1)) & 0xFFFF) << (bit + 1);
		u16 rigthPiece(piece);
		u16 nearestRight = 0;
		while ((wBit = GetFirstLowBit(right)) == 16)
		{
			++rigthPiece;
			right = GetKeyData(rigthPiece);
		}

		nearestRight = ((rigthPiece * sizeof(u16)) << 3) + wBit;
		m_LastDifferenceTime = difference_time = (f32)(internalTime - (f32)nearestLeft) / ((f32)nearestRight - (f32)nearestLeft);

		// count nPos
		u32 nPos(0);
		for (u16 i = 0; i < rigthPiece; ++i)
		{
			u16 data2 = GetKeyData(i);
			nPos += ControllerHelper::m_byteTable[data2 & 255] + ControllerHelper::m_byteTable[data2 >> 8];
		}

		data = GetKeyData(rigthPiece);
		data = ((data << (15 - wBit)) & 0xFFFF) >> (15 - wBit);
		nPos += ControllerHelper::m_byteTable[data & 255] + ControllerHelper::m_byteTable[data >> 8];

		m_lastKey = nPos - 1;

		return m_lastKey;
	}

	u32 GetNumKeys() const { return GetHeader()->m_Size; };

	u32 AssignKeyTime(tukk pData, u32 numElements)
	{
#ifndef _RELEASE
		if (IsConstant())
		{
			__debugbreak();
			return 0;
		}
#endif

		if (m_pKeys)
			DrxModuleFree(m_pKeys);

		u32 sz = sizeof(u16) * numElements;

		m_pKeys = (u16*)DrxModuleMalloc(sz);
		m_numKeys = numElements;

		memcpy(&m_pKeys[0], pData, sz);

		return sz;
	}

	u32 GetFormat()
	{
		return eBitset;
	}

	tuk GetData()
	{
		return (tuk)m_pKeys;
	}

	u32 GetDataRawSize()
	{
		return (m_numKeys & kKeysSizeMask) * sizeof(u16);
	}

	IControllerRelocatableChain* GetRelocateChain()
	{
		return NULL;
	}

private:

	CKeyTimesInformationBitSet(const CKeyTimesInformationBitSet&); //= delete;
	void operator=(const CKeyTimesInformationBitSet&);             //= delete;

	enum
	{
		kKeysSizeMask     = 0x7fffffff,
		kKeysConstantMask = 0x80000000,
	};

	struct Header
	{
		u16 m_Start;
		u16 m_End;
		u16 m_Size;
	};

	Header* GetHeader() const
	{
		return (Header*)m_pKeys;
	}

	u16 GetKeyData(i32 i) const
	{
		return m_pKeys[3 + i];
	}

	bool IsConstant() const
	{
		return (m_numKeys & kKeysConstantMask) != 0;
	}

	u32 GetNumKeys_Int() const
	{
		return m_numKeys & kKeysSizeMask;
	}

	u16* m_pKeys;
	u32  m_numKeys;

	f32     m_lastTime;
	f32     m_LastDifferenceTime;
	u32  m_lastKey;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ITrackPositionStorage : public IControllerRelocatableChain
{
public:
	virtual ~ITrackPositionStorage() {}
	virtual tuk  GetData() = 0;
	virtual u32 AssignData(tukk pData, u32 numElements) = 0;
	virtual u32 GetDataRawSize() = 0;
	virtual u32 GetFormat() = 0;
	virtual void   GetValue(u32 key, Vec3& val) = 0;
	virtual size_t SizeOfPosController() = 0;
};

class ITrackRotationStorage : public IControllerRelocatableChain
{
public:
	virtual ~ITrackRotationStorage() {}
	virtual tuk  GetData() = 0;
	virtual u32 AssignData(tukk pData, u32 numElements) = 0;
	virtual u32 GetDataRawSize() = 0;
	virtual u32 GetFormat() = 0;
	virtual void   GetValue(u32 key, Quat& val) = 0;
	virtual size_t SizeOfRotController() = 0;
};

template<class TData, class TStorage, class TBase>                                              // TODO: TBase and TData can be easily derived from TStorage.
class CTrackDataStorageInt : public TBase
{
public:

	CTrackDataStorageInt()
		: m_numKeys(0)
		, m_pKeys(nullptr)
	{
	}

	CTrackDataStorageInt(TStorage* pData, u32 numElements)
		: m_numKeys(numElements | (numElements > 0 ? kKeysConstantMask : 0))
		, m_pKeys(pData)
	{
	}

	~CTrackDataStorageInt()
	{
		if (!IsConstant() && m_hHdl.IsValid())
			g_controllerHeap.Free(m_hHdl);
	}

	virtual u32 AssignData(tukk pData, u32 numElements)
	{
#ifndef _RELEASE
		if (IsConstant())
		{
			__debugbreak();
			return 0;
		}
#endif

		if (m_hHdl.IsValid())
			g_controllerHeap.Free(m_hHdl);

		u32 sz = numElements * sizeof(TStorage);
		this->LinkTo(NULL);
		CControllerDefragHdl hdl = g_controllerHeap.AllocPinned(sz, this);
		m_hHdl = hdl;
		m_pKeys = (TStorage*)g_controllerHeap.WeakPin(hdl);
		m_numKeys = numElements;
		memcpy(m_pKeys, pData, sz);
		g_controllerHeap.Unpin(hdl);

		return sz;
	}

	tuk GetData()
	{
		return (tuk) m_pKeys;
	}

	u32 GetDataRawSize()
	{
		return GetNumKeys() * sizeof(TStorage);
	}

	void Relocate(tuk pDst, tuk pSrc)
	{
		ptrdiff_t offs = reinterpret_cast<tuk>(m_pKeys) - pSrc;
		m_pKeys = reinterpret_cast<TStorage*>(pDst + offs);
	}

	void GetValue(u32 key, TData& val)
	{
		TStorage l = m_pKeys[key];
		l.ToExternalType(val);
	}

	virtual u32 GetFormat()
	{
		return TStorage::GetFormat();
	}

	virtual size_t SizeOfRotController()
	{
		size_t keys = GetNumKeys();
		size_t s0 = sizeof(*this);
		size_t s1 = keys * sizeof(TStorage);
		return s0 + s1;
	}
	virtual size_t SizeOfPosController()
	{
		size_t keys = GetNumKeys();
		size_t s0 = sizeof(*this);
		size_t s1 = keys * sizeof(TStorage);
		return s0 + s1;
	}

private:

	CTrackDataStorageInt(const CTrackDataStorageInt<TData, TStorage, TBase>&);   //= delete;
	void operator=(const CTrackDataStorageInt<TData, TStorage, TBase>&);         //= delete;

	enum
	{
		kKeysSizeMask     = 0x7fffffff,
		kKeysConstantMask = 0x80000000,
	};

	ILINE u32 IsConstant() const { return m_numKeys & kKeysConstantMask; }
	ILINE u32 GetNumKeys() const { return m_numKeys & kKeysSizeMask; }

	TStorage*            m_pKeys;
	u32               m_numKeys;
	CControllerDefragHdl m_hHdl;
};

template<class TData, class TStorage, class TBase>
class CTrackDataStorage : public CTrackDataStorageInt<TData, TStorage, TBase>    // TODO: This class doesn't really introduce any functionality, it should probably be removed.
{
public:

	typedef TStorage StorageType;

	CTrackDataStorage() {}

	CTrackDataStorage(TStorage* pData, u32 numElements)
		: CTrackDataStorageInt<TData, TStorage, TBase>(pData, numElements)
	{
	}

	virtual void AddRef()  {}
	virtual void Release() { delete this; }
};

typedef CTrackDataStorage<Vec3, NoCompressVec3, ITrackPositionStorage>        NoCompressPosition;
typedef NoCompressPosition                                                    NoCompressPositionPtr;

typedef CTrackDataStorage<Quat, NoCompressQuat, ITrackRotationStorage>        NoCompressRotation;
typedef NoCompressRotation*                                                   NoCompressRotationPtr;

typedef CTrackDataStorage<Quat, SmallTree48BitQuat, ITrackRotationStorage>    SmallTree48BitQuatRotation;
typedef SmallTree48BitQuatRotation*                                           SmallTree48BitQuatRotationPtr;

typedef CTrackDataStorage<Quat, SmallTree64BitQuat, ITrackRotationStorage>    SmallTree64BitQuatRotation;
typedef SmallTree64BitQuatRotation*                                           SmallTree64BitQuatRotationPtr;

typedef CTrackDataStorage<Quat, SmallTree64BitExtQuat, ITrackRotationStorage> SmallTree64BitExtQuatRotation;
typedef SmallTree64BitExtQuatRotation*                                        SmallTree64BitExtQuatRotationPtr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ITrackInformation
{
public:

	ITrackInformation()
		: m_pKeyTimes(nullptr)
	{
	}

	virtual ~ITrackInformation() {}

	u32 GetNumKeys()
	{
		return m_pKeyTimes->GetNumKeys();
	}

	f32 GetTimeFromKey(u32 key)
	{
		return m_pKeyTimes->GetKeyValueFloat(key);
	}

	void SetKeyTimesInformation(_smart_ptr<IKeyTimesInformation>& ptr)
	{
		m_pKeyTimes = ptr;
	}

	_smart_ptr<IKeyTimesInformation>& GetKeyTimesInformation()
	{
		return m_pKeyTimes;
	}

	virtual tuk GetData() = 0;

protected:

	_smart_ptr<IKeyTimesInformation> m_pKeyTimes;
};

class IPositionController : public ITrackInformation
{
public:

	virtual ~IPositionController()
	{
		delete m_pData;
	}

	IPositionController()
		: m_pData(nullptr)
	{
	}

	virtual void   GetValue(f32 normalized_time, Vec3& quat) = 0;

	virtual size_t SizeOfPosController()
	{
		u32k rawSize = m_pKeyTimes->GetDataRawSize();
		const size_t s0 = sizeof(IPositionController);
		const size_t s1 = m_pData->SizeOfPosController();
		return s0 + s1 + rawSize;
	}

	virtual void GetValueFromKey(u32 key, Vec3& val);

	virtual void SetPositionStorage(ITrackPositionStorage* ptr)
	{
		m_pData = ptr;
	}

	virtual u32 GetFormat()
	{
		return m_pData->GetFormat();
	}

	tuk GetData()
	{
		return m_pData->GetData();
	}

protected:

	ITrackPositionStorage* m_pData;
};

class IRotationController : public ITrackInformation
{
public:

	IRotationController()
		: m_pData(nullptr)
	{
	}

	virtual ~IRotationController()
	{
		delete m_pData;
	}

	virtual void   GetValue(f32 normalized_time, Quat& quat) = 0;

	virtual size_t SizeOfRotController()
	{
		u32k rawSize = m_pKeyTimes->GetDataRawSize();
		return sizeof(IRotationController) + m_pData->SizeOfRotController() + rawSize;
	}

	virtual void GetValueFromKey(u32 key, Quat& val);

	virtual void SetRotationStorage(ITrackRotationStorage*& ptr)
	{
		m_pData = ptr;
	}

	virtual u32 GetFormat()
	{
		return m_pData->GetFormat();
	}

	tuk GetData()
	{
		return m_pData->GetData();
	}

protected:

	ITrackRotationStorage* m_pData;
};

struct Vec3Lerp
{
	static inline void Blend(Vec3& res, const Vec3& val1, const Vec3& val2, f32 t)
	{
		res.SetLerp(val1, val2, t);
	}
};

struct QuatLerp
{
	static inline void Blend(Quat& res, const Quat& val1, const Quat& val2, f32 t)
	{
		res.SetNlerp(val1, val2, t);
	}
};

template<class TData, class TInterpolator, class TBase>   // TODO: TData and TBase can be easily derived from TInterpolator.
class CAdvancedTrackInformation : public TBase
{
public:

	virtual ~CAdvancedTrackInformation() {}

	virtual void GetValue(f32 normalized_time, TData& pos)
	{
		//DEFINE_PROFILER_SECTION("ControllerPQ::GetValue");
		f32 t;
		u32 key = this->m_pKeyTimes->GetKey(normalized_time, t);

		if (key == 0)
		{
			DRX_ALIGN(16) TData p1;
			TBase::GetValueFromKey(0, p1);
			pos = p1;
		}
		else
		{
			u32 numKeys = this->GetNumKeys();
			if (key >= numKeys)
			{
				DRX_ALIGN(16) TData p1;
				TBase::GetValueFromKey(key - 1, p1);
				pos = p1;
			}
			else
			{
				//TData p1, p2;
				DRX_ALIGN(16) TData p1;
				DRX_ALIGN(16) TData p2;
				TBase::GetValueFromKey(key - 1, p1);
				TBase::GetValueFromKey(key, p2);

				TInterpolator::Blend(pos, p1, p2, t);
			}
		}
	}
};

typedef CAdvancedTrackInformation<Quat, QuatLerp, IRotationController> RotationTrackInformation;
typedef CAdvancedTrackInformation<Vec3, Vec3Lerp, IPositionController> PositionTrackInformation;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CController : public IController
{
public:

	CController()
		: m_pRotationController(nullptr)
		, m_pPositionController(nullptr)
		, m_pScaleController(nullptr)
	{
	}

	~CController()
	{
		delete m_pRotationController;
		delete m_pPositionController;
		delete m_pScaleController;
	}

	//////////////////////////////////////////////////////////////////////////
	// IController
	virtual JointState GetOPS(f32 normalized_time, Quat& quat, Vec3& pos, Diag33& scale) const override;
	virtual JointState GetOP(f32 normalized_time, Quat& quat, Vec3& pos) const override;
	virtual JointState GetO(f32 normalized_time, Quat& quat) const override;
	virtual JointState GetP(f32 normalized_time, Vec3& pos) const override;
	virtual JointState GetS(f32 normalized_time, Diag33& scl) const override;
	virtual i32      GetO_numKey() const override;
	virtual i32      GetP_numKey() const override;
	virtual size_t     GetRotationKeysNum() const override;
	virtual size_t     GetPositionKeysNum() const override;
	virtual size_t     GetScaleKeysNum() const override;
	virtual size_t     SizeOfController() const override;
	virtual size_t     ApproximateSizeOfThis() const override;
	//////////////////////////////////////////////////////////////////////////

	// TODO: Would be nice to introduce some ownership semantics on the Set*Controller methods instead of using a raw pointer (all of these are a raw pointer typedef at the time of writing).
	void SetRotationController(IRotationController* ptr)
	{
		m_pRotationController = ptr;
	}

	void SetPositionController(IPositionController* ptr)
	{
		m_pPositionController = ptr;
	}

	void SetScaleController(IPositionController* ptr)
	{
		m_pScaleController = ptr;
	}

private:
	IRotationController* m_pRotationController;
	IPositionController* m_pPositionController;
	IPositionController* m_pScaleController;
};

TYPEDEF_AUTOPTR(CController);
