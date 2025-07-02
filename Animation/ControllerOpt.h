// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if !defined(RESOURCE_COMPILER)
	#include <drx3D/Eng3D/CGFContent.h>
	#include <drx3D/Animation/QuatQuantization.h>
	#include <drx3D/Animation/ControllerPQ.h>

class IControllerOpt : public IController
{
public:
	virtual void        SetRotationKeyTimes(u32 num, tuk pData) = 0;
	virtual void        SetPositionKeyTimes(u32 num, tuk pData) = 0;
	virtual void        SetRotationKeys(u32 num, tuk pData) = 0;
	virtual void        SetPositionKeys(u32 num, tuk pData) = 0;

	virtual u32      GetRotationFormat() const = 0;
	virtual u32      GetRotationKeyTimesFormat() const = 0;
	virtual u32      GetPositionFormat() const = 0;
	virtual u32      GetPositionKeyTimesFormat() const = 0;

	virtual tukk GetRotationKeyData() const = 0;
	virtual tukk GetRotationKeyTimesData() const = 0;
	virtual tukk GetPositionKeyData() const = 0;
	virtual tukk GetPositionKeyTimesData() const = 0;

};

//TYPEDEF_AUTOPTR(IControllerOpt);
typedef IControllerOpt* IControllerOpt_AutoPtr;

template<class _PosController, class _RotController>
class CControllerOpt : public IControllerOpt, _PosController, _RotController
{
public:
	//Creation interface

	CControllerOpt(){}

	~CControllerOpt(){}

	u32 numKeys() const
	{
		// now its hack, because num keys might be different
		return max(this->GetRotationNumCount(), this->GetPositionNumCount());
	}

	JointState GetOPS(f32 key, Quat& quat, Vec3& pos, Diag33& scale) const
	{
		typedef CControllerOpt<_PosController, _RotController> TSelf;
		return TSelf::GetO(key, quat) | TSelf::GetP(key, pos) | TSelf::GetS(key, scale);
	}

	JointState GetOP(f32 key, Quat& quat, Vec3& pos) const
	{
		typedef CControllerOpt<_PosController, _RotController> TSelf;
		return TSelf::GetO(key, quat) | TSelf::GetP(key, pos);
	}

	JointState GetO(f32 key, Quat& quat) const
	{
		return this->GetRotationValue(key, quat);
	}

	JointState GetP(f32 key, Vec3& pos) const
	{
		return this->GetPositionValue(key, pos);
	}

	JointState GetS(f32 key, Diag33& scale) const
	{
		return 0;
	}

	// returns the start time
	size_t SizeOfController() const
	{
		size_t res(sizeof(*this));
		res += this->GetRotationsSize();
		res += this->GetPositionsSize();
		return res;
	}

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//static_cast<_PosController*>(this)->_PosController::GetMemoryUsage(pSizer);
		//static_cast<_RotController*>(this)->_RotController::GetMemoryUsage(pSizer);
	}

	size_t ApproximateSizeOfThis() const
	{
		size_t res(sizeof(*this));
		res += this->GetRotationsSize();
		res += this->GetPositionsSize();
		return res;
	}

	size_t GetRotationKeysNum() const
	{
		return this->GetRotationNumCount();
	}

	size_t GetPositionKeysNum() const
	{
		return this->GetPositionNumCount();
	}

	void SetRotationKeyTimes(u32 num, tuk pData)
	{
		this->SetRotKeyTimes/*_RotController::SetKeyTimes*/ (num, pData);
	}

	void SetPositionKeyTimes(u32 num, tuk pData)
	{
		this->SetPosKeyTimes/*_PosController::SetKeyTimes*/ (num, pData);
	}

	void SetRotationKeys(u32 num, tuk pData)
	{
		this->SetRotationData(num, pData);
	}

	void SetPositionKeys(u32 num, tuk pData)
	{
		this->SetPositionData(num, pData);
	}

	u32 GetRotationFormat() const
	{
		return this->GetRotationType();
	}

	u32 GetRotationKeyTimesFormat() const
	{
		return this->GetRotationKeyTimeFormat();
	}

	u32 GetPositionFormat() const
	{
		return this->GetPositionType();
	}

	u32 GetPositionKeyTimesFormat() const
	{
		return this->GetPositionKeyTimeFormat();
	}

	tukk GetRotationKeyData() const
	{
		return this->GetRotationData();
	}

	tukk GetRotationKeyTimesData() const
	{
		return this->GetRotKeyTimes();
	}

	tukk GetPositionKeyData() const
	{
		return this->GetPositionData();
	}

	tukk GetPositionKeyTimesData() const
	{
		return this->GetPosKeyTimes();
	}

	IControllerOpt* CreateController()
	{
		return (IControllerOpt*)new CControllerOpt<_PosController, _RotController>();
	}

};

//  предварительные объявления
struct ControllerData;
static u32 GetKeySelector(f32 normalized_time, f32& difference_time, const ControllerData& rConData);

struct ControllerData
{
	ControllerData() {}

	ControllerData(i32 iType, i32 iKeyType) :
		m_nDataOffs(0),
		m_nKeysOffs(0),
		m_iCount(0),
		m_eTimeFormat(iKeyType),
		m_eCompressionType(iType)
	{
	}

	void Init(i32 iType, i32 iKeyType)
	{
		m_nDataOffs = 0;
		m_nKeysOffs = 0;
		m_iCount = 0;
		m_eTimeFormat = iKeyType;
		m_eCompressionType = iType;
	}

	// call function to select template implementation of GetKeyData
	u32 GetKey(f32 normalizedTime, f32& differenceTime) const
	{
		return GetKeySelector(normalizedTime, differenceTime, *this);
	}

	template<typename Type>
	u32 GetKeyByteData(f32 normalized_time, f32& difference_time, ukk p_data) const
	{
		const Type* data = reinterpret_cast<const Type*>(p_data);

		f32 realtimef = normalized_time;
		Type realtime = (Type)realtimef;

		u32 numKey = GetNumCount();

		Type keytime_start = data[0];
		Type keytime_end = data[numKey - 1];

		if (realtime < keytime_start)
		{
			return 0;
		}

		if (realtime >= keytime_end)
		{
			return numKey;
		}

		//-------------
		i32 nPos = numKey >> 1;
		i32 nStep = numKey >> 2;

		// use binary search
		//TODO: Need check efficiency of []operator. Maybe wise use pointer
		while (nStep)
		{
			if (realtime < data[nPos])
				nPos = nPos - nStep;
			else if (realtime > data[nPos])
				nPos = nPos + nStep;
			else
				break;

			nStep = nStep >> 1;
		}

		// fine-tuning needed since time is not linear
		while (realtime >= data[nPos])
			nPos++;

		while (realtime < data[nPos - 1])
			nPos--;

		// possible error if encoder uses nonlinear methods!!!
		if (data[nPos] == data[nPos - 1])
		{
			difference_time = 0.0f;
		}
		else
		{
			f32 prevtime = (f32)data[nPos - 1];
			f32 time = (f32)data[nPos];
			difference_time = (realtimef - prevtime) / (time - prevtime);
		}

		assert(difference_time >= 0.0f && difference_time <= 1.0f);
		return nPos;
	}

	u32 GetKeyBitData(f32 normalized_time, f32& difference_time) const
	{
		f32 realtime = normalized_time;

		u32 numKey = (u32)GetHeader()->m_Size;//m_arrKeys.size();

		f32 keytime_start = (float)GetHeader()->m_Start;
		f32 keytime_end = (float)GetHeader()->m_End;
		f32 test_end = keytime_end;

		if (realtime < keytime_start)
			test_end += realtime;

		if (realtime < keytime_start)
		{
			difference_time = 0;
			return 0;
		}

		if (realtime >= keytime_end)
		{
			difference_time = 0;
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
		difference_time = (f32)(internalTime - (f32)nearestLeft) / ((f32)nearestRight - (f32)nearestLeft);

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

		return nPos - 1;
	}

	// util functions for bitset encoding
	struct Header
	{
		u16 m_Start;
		u16 m_End;
		u16 m_Size;
	};

	inline const Header* GetHeader() const
	{
		return (const Header*)GetData();
	};

	ILINE tuk       GetData()       { return reinterpret_cast<tuk>(this) + m_nDataOffs; }
	ILINE tuk       GetKeys()       { return reinterpret_cast<tuk>(this) + m_nKeysOffs; }
	ILINE tukk GetData() const { return reinterpret_cast<tukk >(this) + m_nDataOffs; }
	ILINE tukk GetKeys() const { return reinterpret_cast<tukk >(this) + m_nKeysOffs; }

	inline u16     GetKeyData(i32 i) const
	{
		u16k* pData = reinterpret_cast<u16k*>(GetData());
		return pData[3 + i];
	};

	size_t                  GetKeysNum() const         { return GetNumCount(); }
	size_t                  GetNumCount() const        { return static_cast<size_t>(getTimeFormat() == eBitset ? GetHeader()->m_Size : m_iCount); }

	EKeyTimesFormat         getTimeFormat() const      { return static_cast<EKeyTimesFormat>(m_eTimeFormat); }
	ECompressionInformation getCompressionType() const { return static_cast<ECompressionInformation>(m_eCompressionType); }

	void                    GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	// Data will be within same allocation as controllers, so offsets ought to be less than 2gb away
	i32  m_nDataOffs;
	i32  m_nKeysOffs;

	u16 m_iCount;

	// using unsigned chars to store enums to save storage space
	u8 m_eTimeFormat;
	u8 m_eCompressionType;

};

// If the size of this structure changes, RC_GetSizeOfControllerOptNonVirtual also needs
// to be updated.

class CControllerOptNonVirtual : public IControllerOpt
{
public:

	// ============ new interface ===========//
public:

	CControllerOptNonVirtual() {};

	CControllerOptNonVirtual(i32 iRotType, i32 iRotKeyType, i32 iPosType, i32 iPosKeyType) :
		m_position(iPosType, iPosKeyType),
		m_rotation(iRotType, iRotKeyType)
	{
	}

	void Init(i32 iRotType, i32 iRotKeyType, i32 iPosType, i32 iPosKeyType)
	{
		m_position.Init(iPosType, iPosKeyType);
		m_rotation.Init(iRotType, iRotKeyType);

	}

	// Deliberately nerf'd - instance must be immutable, as it can be moved by defrag
	void AddRef() const override   {}
	void Release()  const override {}

	~CControllerOptNonVirtual(){}

	JointState GetOPS(f32 normalizedTime, Quat& quat, Vec3& pos, Diag33& scale) const override;
	JointState GetOP(f32 normalizedTime, Quat& quat, Vec3& pos) const override;
	JointState GetO(f32 normalizedTime, Quat& quat) const override;
	JointState GetP(f32 normalizedTime, Vec3& pos) const override;
	JointState GetS(f32 normalizedTime, Diag33& pos) const override;

	Vec3       GetPosValue(f32 normalizedTime) const
	{
		//DEFINE_PROFILER_SECTION("ControllerPQ::GetValue");
		Vec3 pos;

		f32 t;
		u32 key = m_position.GetKey(normalizedTime, t);

		IF (key == 0, true)
		{
			DrxPrefetch(m_position.GetKeys());
			GetPosValueFromKey(0, pos);
		}
		else
		{
			IF (key < m_position.GetNumCount(), true)
			{
				// assume that the 48bit(6byte) encodings are used(can be wrong but should be right the most time)
				tukk pKeys = m_position.GetKeys();
				DrxPrefetch(&pKeys[(key - 1) * 6]);
				DrxPrefetch(&pKeys[key * 6]);

				DRX_ALIGN(16) Vec3 p1;
				DRX_ALIGN(16) Vec3 p2;

				GetPosValueFromKey(key - 1, p1);
				GetPosValueFromKey(key, p2);

				pos.SetLerp(p1, p2, t);
			}
			else
			{
				GetPosValueFromKey(m_position.GetNumCount() - 1, pos);
			}
		}

		return pos;
	}

	Quat GetRotValue(f32 normalizedTime) const
	{
		//DEFINE_PROFILER_SECTION("ControllerPQ::GetValue");
		Quat pos;

		f32 t;
		u32 key = m_rotation.GetKey(normalizedTime, t);

		IF (key == 0, true)
		{
			DrxPrefetch(m_rotation.GetKeys());

			DRX_ALIGN(16) Quat p1;
			GetRotValueFromKey(0, p1);
			pos = p1;
		}
		else
		{
			IF (key < m_rotation.GetNumCount(), true)
			{
				// assume that the 48bit(6byte) encodings are used(can be wrong but should be right the most time)
				tukk pKeys = m_rotation.GetKeys();
				DrxPrefetch(&pKeys[(key - 1) * 6]);
				DrxPrefetch(&pKeys[key * 6]);

				//	Quat p1, p2;
				DRX_ALIGN(16) Quat p1;
				DRX_ALIGN(16) Quat p2;

				GetRotValueFromKey(key - 1, p1);
				GetRotValueFromKey(key, p2);

				pos.SetNlerp(p1, p2, t);
			}
			else
			{
				DRX_ALIGN(16) Quat p1;
				assert(key - 1 < m_rotation.GetNumCount());
				GetRotValueFromKey(m_rotation.GetNumCount() - 1, p1);
				pos = p1;
			}
		}

		return pos;
	}

	template<typename CompressionType, typename ValueType>
	void load_value(u32 key, tukk data, ValueType& val) const
	{
		const CompressionType* p = reinterpret_cast<const CompressionType*>(data);
		p[key].ToExternalType(val);
	}

	void GetRotValueFromKey(u32 key, Quat& val) const
	{
		ECompressionInformation format = m_rotation.getCompressionType();
		tukk pKeys = m_rotation.GetKeys();

		// branches ordered by probability
		IF (format == eSmallTree48BitQuat, true)
		{
			load_value<SmallTree48BitQuat>(key, pKeys, val);
		}
		else
		{
			IF (format == eSmallTree64BitExtQuat, true)
			{
				load_value<SmallTree64BitExtQuat>(key, pKeys, val);
			}
			else
			{
				IF (format == eSmallTree64BitQuat, true)
				{
					load_value<SmallTree64BitQuat>(key, pKeys, val);
				}
				else
				{
					IF (format == eNoCompressQuat, true)
					{
						load_value<NoCompressQuat>(key, pKeys, val);
					}
					else
					{
						DrxFatalError("Unknown Rotation Compression format %i\n", format);
					}
				}
			}
		}

	}

	void GetPosValueFromKey(u32 key, Vec3& val) const
	{
		// branches ordered by probability
		IF (m_position.getCompressionType() == eNoCompressVec3, 1)
		{
			load_value<NoCompressVec3>(key, m_position.GetKeys(), val);
		}
		else
		{
			val = ZERO;
			//DrxFatalError("Unknown Position Compression format %i", m_position.getCompressionType());
		}
	}

	void SetRotationKeyTimes(u32 num, tuk pData) override
	{
		if (eNoFormat == m_rotation.getTimeFormat()) return;

		m_rotation.m_iCount = static_cast<u16>(num);
		m_rotation.m_nDataOffs = static_cast<i32>(pData - reinterpret_cast<tuk>(&m_rotation));
	}

	void SetPositionKeyTimes(u32 num, tuk pData) override
	{
		if (eNoFormat == m_position.getTimeFormat()) return;

		m_position.m_iCount = static_cast<u16>(num);
		m_position.m_nDataOffs = static_cast<i32>(pData - reinterpret_cast<tuk>(&m_position));
	}

	void SetRotationKeys(u32 num, tuk pData) override
	{
		if (eNoFormat == m_rotation.getTimeFormat()) return;

		m_rotation.m_nKeysOffs = static_cast<i32>(pData - reinterpret_cast<tuk>(&m_rotation));
	}

	void SetPositionKeys(u32 num, tuk pData) override
	{
		if (eNoFormat == m_position.getTimeFormat()) return;

		m_position.m_nKeysOffs = static_cast<i32>(pData - reinterpret_cast<tuk>(&m_position));
	}

	u32      GetRotationFormat() const override         { return m_rotation.getCompressionType(); }
	u32      GetRotationKeyTimesFormat() const override { return m_rotation.getTimeFormat(); }

	u32      GetPositionFormat() const override         { return m_position.getCompressionType(); }
	u32      GetPositionKeyTimesFormat() const override { return m_position.getTimeFormat(); }

	tukk GetRotationKeyData() const override        { return m_rotation.GetKeys(); }
	tukk GetRotationKeyTimesData() const override   { return m_rotation.GetData(); }

	tukk GetPositionKeyData() const override        { return m_position.GetKeys(); }
	tukk GetPositionKeyTimesData() const override   { return m_position.GetData(); }

	size_t      SizeOfController() const override
	{
		return sizeof(*this);
	}
	size_t ApproximateSizeOfThis() const override
	{
		i32 sizeOfRotKey = ControllerHelper::GetRotationFormatSizeOf(m_rotation.getCompressionType());
		i32 sizeOfPosKey = ControllerHelper::GetPositionsFormatSizeOf(m_position.getCompressionType());

		return sizeof(*this) + sizeOfRotKey * m_rotation.GetNumCount() + sizeOfPosKey * m_position.GetNumCount();
	}

	virtual size_t GetRotationKeysNum() const override     { return m_rotation.GetNumCount(); }
	virtual size_t GetPositionKeysNum() const override     { return m_position.GetNumCount(); }
	virtual size_t GetScaleKeysNum() const override        { return 0; }

	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const {}

private:
	ControllerData m_rotation;
	ControllerData m_position;

};

TYPEDEF_AUTOPTR(CControllerOptNonVirtual);

static u32 GetKeySelector(f32 normalized_time, f32& difference_time, const ControllerData& rConData)
{
	ukk data = rConData.GetData();

	EKeyTimesFormat format = rConData.getTimeFormat();

	// branches ordered by probability
	IF (format == eByte, true)
	{
		return rConData.GetKeyByteData<u8>(normalized_time, difference_time, data);
	}
	else
	{
		IF (format == eUINT16, 1)
		{
			return rConData.GetKeyByteData<u16>(normalized_time, difference_time, data);
		}
		else
		{
			IF (format == eF32, 1)
			{
				return rConData.GetKeyByteData<f32>(normalized_time, difference_time, data);
			}
			else
			{
				return rConData.GetKeyBitData(normalized_time, difference_time);
			}
		}
	}
}

#else

// Essentially, the DBA needs to reserve space for the CControllerOptNonVirtual instances.
// They need to be allocated within the same allocation as the track data, as the
// CControllerOptNonVirtual instances store offsets to the data, and the allocation as a
// whole gets defragged and relocated.

// sizeof(CControllerOptNonVirtual) can't be done in RC, because:

// a) The struct depends on a bunch of things that will conflict with RC types
// b) The vtable pointer means the size may be wrong.

// So we have this. If it's wrong, you'll get warnings when DBAs are streamed.
inline size_t RC_GetSizeOfControllerOptNonVirtual(size_t pointerSize)
{
	size_t icontrollerSize = Align(pointerSize + sizeof(u32), pointerSize);
	size_t controllerSize = Align(icontrollerSize + sizeof(u32), pointerSize);
	return Align(controllerSize + sizeof(u32) * 6, pointerSize);
}

#endif // !defined(RESOURCE_COMPILER)
