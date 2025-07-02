// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __bitarray_h__
#define __bitarray_h__

#if _MSC_VER > 1000
	#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
//
// CBitArray is similar to std::vector but faster to clear.
//
//////////////////////////////////////////////////////////////////////////
class CBitArray
{
public:
	struct BitReference
	{
		u32* p;
		u32  mask;
		BitReference(u32* __x, u32 __y)
			: p(__x), mask(__y) {}

	public:
		BitReference() : p(0), mask(0) {}

		operator bool() const {
			return !(!(*p & mask));
		}
		BitReference& operator=(bool __x)
		{
			if (__x)
				*p |= mask;
			else
				*p &= ~mask;
			return *this;
		}
		BitReference& operator=(const BitReference& __x)        { return *this = bool(__x);  }
		bool          operator==(const BitReference& __x) const { return bool(*this) == bool(__x); }
		bool          operator<(const BitReference& __x) const  { return !bool(*this) && bool(__x); }
		BitReference& operator|=(bool __x)
		{
			if (__x)
				*p |= mask;
			return *this;
		}
		BitReference& operator&=(bool __x)
		{
			if (!__x)
				*p &= ~mask;
			return *this;
		}
		void flip() {* p ^= mask; }
	};

	CBitArray() { m_base = NULL; m_bits = NULL; m_size = 0; m_numBits = 0; };
	CBitArray(i32 numBits)  { resize(numBits); };
	~CBitArray()  { if (m_base) free(m_base); };

	void resize(i32 c)
	{
		m_numBits = c;
		i32 newSize = ((c + 63) & (~63)) >> 5;
		if (newSize > m_size)
			Alloc(newSize);
	}
	i32  size() const  { return m_numBits; };
	bool empty() const { return m_numBits == 0; };

	//////////////////////////////////////////////////////////////////////////
	void set()
	{
		memset(m_bits, 0xFFFFFFFF, m_size * sizeof(u32));  // Set all bits.
	}
	//////////////////////////////////////////////////////////////////////////
	void set(i32 numBits)
	{
		i32 num = (numBits >> 3) + 1;
		if (num > (m_size * sizeof(u32))) num = m_size * sizeof(u32);
		memset(m_bits, 0xFFFFFFFF, num);  // Reset num bits.
	}

	//////////////////////////////////////////////////////////////////////////
	void clear()
	{
		memset(m_bits, 0, m_size * sizeof(u32)); // Reset all bits.
	}

	//////////////////////////////////////////////////////////////////////////
	void clear(i32 numBits)
	{
		i32 num = (numBits >> 3) + 1;
		if (num > (m_size * sizeof(u32))) num = m_size * sizeof(u32);
		memset(m_bits, 0, num); // Reset num bits.
	}

	//////////////////////////////////////////////////////////////////////////
	// Check if all bits are 0.
	bool is_zero() const
	{
		for (i32 i = 0; i < m_size; i++)
			if (m_bits[i] != 0)
				return false;
		return true;
	}

	// Count number of set bits.
	i32 count_bits()  const
	{
		i32 c = 0;
		for (i32 i = 0; i < m_size; i++)
		{
			u32 v = m_bits[i];
			for (i32 j = 0; j < 32; j++)
			{
				if (v & (1 << (j & 0x1F))) c++; // if bit set increase bit count.
			}
		}
		return c;
	}

	BitReference       operator[](i32 pos)       { return BitReference(&m_bits[index(pos)], shift(pos)); }
	const BitReference operator[](i32 pos) const { return BitReference(&m_bits[index(pos)], shift(pos)); }

	//////////////////////////////////////////////////////////////////////////
	void swap(CBitArray& bitarr)
	{
		std::swap(m_base, bitarr.m_base);
		std::swap(m_bits, bitarr.m_bits);
		std::swap(m_size, bitarr.m_size);
	}

	CBitArray& operator=(const CBitArray& b)
	{
		if (m_size != b.m_size)
		{
			Alloc(b.m_size);
		}
		memcpy(m_bits, b.m_bits, m_size * sizeof(u32));
		return *this;
	}

	bool checkByte(i32 pos) const { return reinterpret_cast<tuk>(m_bits)[pos] != 0; };

	//////////////////////////////////////////////////////////////////////////
	// Compresses this bit array into the specified one.
	// Uses run length encoding compression.
	void compress(CBitArray& b) const
	{
		i32 i, countz, compsize, bsize;
		tuk out;
		tuk in;

		bsize = m_size * 4;
		compsize = 0;
		in = (tuk)m_bits;
		for (i = 0; i < bsize; i++)
		{
			compsize++;
			if (in[i] == 0)
			{
				countz = 1;
				while (++i < bsize)
				{
					if (in[i] == 0 && countz != 255) countz++; else break;
				}
				i--;
				compsize++;
			}
		}
		b.resize((compsize + 1) << 3);
		out = (tuk)b.m_bits;
		in = (tuk)m_bits;
		*out++ = bsize;
		for (i = 0; i < bsize; i++)
		{
			*out++ = in[i];
			if (in[i] == 0)
			{
				countz = 1;
				while (++i < bsize)
				{
					if (in[i] == 0 && countz != 255) countz++; else break;
				}
				i--;
				*out++ = countz;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Decompress specified bit array in to this one.
	// Uses run length encoding compression.
	void decompress(CBitArray& b)
	{
		i32 raw, decompressed, c;
		tuk out, * in;

		in = (tuk)m_bits;
		out = (tuk)b.m_bits;
		decompressed = 0;
		raw = *in++;
		while (decompressed < raw)
		{
			if (*in != 0)
			{
				*out++ = *in++;
				decompressed++;
			}
			else
			{
				in++;
				c = *in++;
				decompressed += c;
				while (c) { *out++ = 0; c--; };
			}
		}
		m_numBits = decompressed;
	}

	void CopyFromMem(tukk src, i32 size)
	{
		Alloc(size);
		memcpy(m_bits, src, size);
	}
	i32 CopyToMem(tuk trg)
	{
		memcpy(trg, m_bits, m_size);
		return m_size;
	}

private:
	uk   m_base;
	u32* m_bits;
	i32     m_size;
	i32     m_numBits;

	void Alloc(i32 s)
	{
		if (m_base) free(m_base);
		m_size = s;
		m_base = (tuk)malloc(m_size * sizeof(u32) + 32);
		m_bits = (u32*)(((UINT_PTR)m_base + 31) & (~31)); // align by 32.
		memset(m_bits, 0, m_size * sizeof(u32));          // Reset all bits.
	}
	u32 shift(i32 pos) const
	{
		return (1 << (pos & 0x1F));
	}
	u32 index(i32 pos) const
	{
		return pos >> 5;
	}

	friend  i32 concatBitarray(CBitArray& b1, CBitArray& b2, CBitArray& test, CBitArray& res);
};

inline i32 concatBitarray(CBitArray& b1, CBitArray& b2, CBitArray& test, CBitArray& res)
{
	u32 b, any;
	any = 0;
	for (i32 i = 0; i < b1.size(); i++)
	{
		b = b1.m_bits[i] & b2.m_bits[i];
		any |= (b & (~test.m_bits[i])); // test if any different from test(i) bit set.
		res.m_bits[i] = b;
	}
	return any;
}

#endif // __bitarray_h__

