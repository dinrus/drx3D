// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZE_BITS__H__
#define __SERIALIZE_BITS__H__

	#pragma once

#include <drx3D/Act/StdAfx.h>

class CBitArray
{
public:
	/* This is a naive implementation, but actually packs bits without padding, unlike DinrusXNetwork */
	CBitArray(TSerialize* m_ser);

	void          ResetForWrite();
	void          ResetForRead();

	bool          IsReading() { return m_isReading; }
	bool          IsWriting() { return m_isReading == false; }

	i32           NumberOfBitsPushed();
	void          PushBit(i32 bit);
	i32           PopBit();
	void          ReadBits(u8* out, i32 numBits);
	void          WriteBits(u8k* in, i32 numBits);
	inline u32 bitsneeded(u32 v);

	inline void   SerializeInt(i32* v, i32 min, i32 max);
	inline void   SerializeUInt(u32* v, u32 min, u32 max);
	void          SerializeFloat(float* data, float min, float max, i32 totalNumBits, i32 reduceRange = 0);
	inline void   SerializeEntity(EntityId& id);

	inline void   Serialize(bool& v);
	inline void   Serialize(u32& v, u32 min = 0, u32 max = 0xffffffff);
	inline void   Serialize(i32& v, i32 min, i32 max);
	inline void   Serialize(i16& v, i32 min, i32 max);
	inline void   Serialize(u16& v, u16 min = 0, u16 max = 0xffff);
	inline void   Serialize(u8& v, i32 min, i32 max);
	inline void   Serialize(float& f, float min, float max, i32 totalNumBits, i32 reduceRange = 0);
	inline void   Serialize(Vec3& v, float min, float max, i32 numBitsPerElem, i32 reduceRange = 0);
	inline void   Serialize(Quat& v);

	void          WriteToSerializer();

private:
	template<class INT> void SerializeInt_T(INT* v, INT min, INT max);

public:
	enum { maxBytes = 1 << 13 };
	i32           m_bytePos;
	i32           m_bitPos;
	i32           m_numberBytes;
	bool          m_isReading;
	i32           m_multiplier;
	TSerialize*   m_ser;
	u8 m_readByte;
	u8 m_data[maxBytes];
};

/*
   =========================================================================================================
   Implementation
   =========================================================================================================
 */

inline u32 CBitArray::bitsneeded(u32 v)
{
	// See bit twiddling hacks
	static i32k MultiplyDeBruijnBitPosition[32] =
	{
		0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
		8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31
	};

	v |= v >> 1; // first round down to one less than a power of 2
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	return 1 + MultiplyDeBruijnBitPosition[(u32)(v * 0x07C4ACDDU) >> 27];
}

template<class INT>
NO_INLINE_WEAK void CBitArray::SerializeInt_T(INT* v, INT min, INT max)
{
	INT range = max - min;
	INT nbits = bitsneeded(range);
	u8 c;

	if (IsReading())
	{
		INT multiplier = 1;
		*v = 0;
		while (nbits > 8)
		{
			ReadBits(&c, 8);
			*v |= multiplier * c;   // Note: there is no need for endian swapping with this method
			multiplier = multiplier * 256;
			nbits = nbits - 8;
		}
		ReadBits(&c, nbits);
		*v |= multiplier * c;
		*v = *v + min;
	}
	else
	{
		INT tmp = std::min(*v - min, (INT)0);
		if (tmp > range) tmp = range;
		while (nbits > 8)
		{
			c = tmp & 0xff;
			WriteBits(&c, 8);   // Note: there is no need for endian swapping with this method
			tmp = tmp >> 8;
			nbits = nbits - 8;
		}
		c = tmp & 0xff;
		WriteBits(&c, nbits);
	}
}

inline void CBitArray::Serialize(bool& v)
{
	i32 tmp = v ? 1 : 0;
	SerializeInt_T<i32>(&tmp, 0, 1);
	v = (tmp != 0);
}

inline void CBitArray::SerializeInt(i32* v, i32 min, i32 max)
{
	SerializeInt_T<i32>(v, min, max);
}

inline void CBitArray::SerializeUInt(u32* v, u32 min, u32 max)
{
	SerializeInt_T<u32>(v, min, max);
}

inline void CBitArray::Serialize(u32& v, u32 min, u32 max)
{
	u32 tmp = v;
	SerializeUInt(&tmp, min, max);
	v = (u32)tmp;
}

inline void CBitArray::Serialize(i32& v, i32 min, i32 max)
{
	i32 tmp = v;
	SerializeInt(&tmp, min, max);
	v = (i32)tmp;
}

inline void CBitArray::Serialize(i16& v, i32 min, i32 max)
{
	i32 tmp = v;
	SerializeInt(&tmp, min, max);
	v = (i16)tmp;
}

inline void CBitArray::Serialize(u16& v, u16 min, u16 max)
{
	u32 tmp = v;
	SerializeUInt(&tmp, min, max);
	v = (u16)tmp;
}

inline void CBitArray::Serialize(u8& v, i32 min, i32 max)
{
	i32 tmp = v;
	SerializeInt(&tmp, min, max);
	v = (u8)tmp;
}

inline void CBitArray::Serialize(float& v, float min, float max, i32 totalNumBits, i32 reduceRange)
{
	SerializeFloat(&v, min, max, totalNumBits, reduceRange);
}

inline void CBitArray::Serialize(Vec3& v, float min, float max, i32 numBitsPerElem, i32 reduceRange)
{
	SerializeFloat(&v.x, min, max, numBitsPerElem, reduceRange);
	SerializeFloat(&v.y, min, max, numBitsPerElem, reduceRange);
	SerializeFloat(&v.z, min, max, numBitsPerElem, reduceRange);
}

inline void CBitArray::Serialize(Quat& q)
{
	// Should this compression migratate to Drx_Quat.h ?
	float quat[4];
	if (IsWriting())
	{
		u32 out = 0;
		u32 i;
		float scale = 1.0f;

		quat[0] = q.w;
		quat[1] = q.v.x;
		quat[2] = q.v.y;
		quat[3] = q.v.z;

		u32 largest = 0;
		for (i = 1; i < 4; i++)
		{
			if (fabsf(quat[i]) > fabsf(quat[largest]))
				largest = i;
		}

		// Scale the quat so that reconstruction always deals with positive value
		scale = (float)__fsel(quat[largest], 1.f, -1.f);

		out |= largest; // first 2 bits denote which is the largest

		u32 entry = 0;
		u32 multiply = 4;
		for (i = 0; i < 4; i++)
		{
			if (i != largest)
			{
				// Encode each remaining value in 10 bits, using range 0-1022. NB, range is chosen so zero is reproduced correctly
				i32 val = (i32)((((scale * quat[i]) + 0.7071f) * (1022.f / 1.4142f)) + 0.5f);
				if (val < 0) val = 0;
				if (val > 1022) val = 1022;
				out |= val * multiply;
				multiply *= 1024;
				entry++;
			}
		}
		Serialize(out);
	}
	else // Reading
	{
		u32 in;
		Serialize(in);

		static i32 idx[4][3] = {
			{ 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 }, { 0, 1, 2 }
		};
		i32 mv = in & 3;
		i32* indices = idx[mv];
		u32 c0 = (in >> 2) & 1023;
		u32 c1 = (in >> 12) & 1023;
		u32 c2 = (in >> 22) & 1023;
		float outDatai0 = (c0 * (1.4142f / 1022.f)) - 0.7071f;
		float outDatai1 = (c1 * (1.4142f / 1022.f)) - 0.7071f;
		float outDatai2 = (c2 * (1.4142f / 1022.f)) - 0.7071f;
		float sumOfSqs = 1.f - outDatai0 * outDatai0;
		quat[indices[0]] = outDatai0;
		sumOfSqs -= outDatai1 * outDatai1;
		quat[indices[1]] = outDatai1;
		sumOfSqs -= outDatai2 * outDatai2;
		quat[indices[2]] = outDatai2;
		sumOfSqs = (float)__fsel(sumOfSqs, sumOfSqs, 0.0f);
		quat[mv] = sqrtf(sumOfSqs);

		q.w = quat[0];
		q.v.x = quat[1];
		q.v.y = quat[2];
		q.v.z = quat[3];
	}
}

inline void CBitArray::SerializeFloat(float* data, float min, float max, i32 totalNumBits, i32 reduceRange)
{
	i32 range = (1 << totalNumBits) - 1 - reduceRange;

	if (IsReading())
	{
		i32 n;
		SerializeInt(&n, 0, range);
		*data = ((float)n) * (max - min) / (float)(range) + min;
	}
	else
	{
		float f = clamp_tpl(*data, min, max);
		i32 n = (i32)(((float)range) / (max - min) * (f - min));
		SerializeInt(&n, 0, range);
	}
}

inline void CBitArray::SerializeEntity(EntityId& entId)
{
	INetContext* pContext = gEnv->pGameFramework->GetNetContext();
	if (IsReading())
	{
		SNetObjectID netId;
		Serialize(netId.id);
		if (pContext)
		{
			pContext->Resaltify(netId);
			entId = pContext->GetEntityID(netId);
		}
		else
		{
			entId = 0;
			GameWarning("brk: can't decode entid, pContext==NULL!");
		}
	}
	else
	{
		SNetObjectID netId;
		if (pContext)
		{
			netId = pContext->GetNetID(entId, false);
		}
		else
		{
			GameWarning("brk: can't send entid, pContext==NULL!");
		}
		Serialize(netId.id);
	}
}

#endif
