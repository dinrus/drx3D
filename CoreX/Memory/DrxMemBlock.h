// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DRXMEMBLOCK_H__
#define __DRXMEMBLOCK_H__

class DrxMemBlock
{
public:
	union
	{
		uk pv;
		tuk pc;
	};

	u32 size;
	u32 ref;
	bool   own;

	DrxMemBlock()
	{
		pv = 0;
		size = 0;
		ref = 0;
		own = true;
	}

	DrxMemBlock(u32 _size)
	{
		ref = 0;
		pv = 0;
		size = 0;
		Alloc(_size);
	}

	DrxMemBlock(uk _pv, u32 _size)
	{
		ref = 0;
		pv = _pv;
		size = _size;
		own = false;
	}

	~DrxMemBlock()
	{
		if (own)
			Free();
		else
			Detach();
	}

	void AddRef()
	{
		ref++;
	}

	void Release()
	{
		ref--;
		if (!ref)
			delete this;
	}

	void Alloc(u32 _size)
	{
		if (own)
			Free();
		else
			Detach();

		pv = new byte[_size];
		size = _size;
		own = true;
	}

	void Free()
	{
		if (!own)
			return;

		if (pv)
			delete[] (byte*) pv;

		pv = 0;
		size = 0;
		own = false;
	}

	void Attach(uk _pv, i32 _size)
	{
		if (own)
			Free();
		else
			Detach();

		pv = _pv;
		size = _size;
		own = false;
	}

	void Detach()
	{
		if (own)
			return;

		pv = 0;
		size = 0;
	}

};

#endif // __DRXMEMBLOCK_H__
