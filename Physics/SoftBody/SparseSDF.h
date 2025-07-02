#ifndef DRX3D_SPARSE_SDF_H
#define DRX3D_SPARSE_SDF_H

#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>

// Fast Hash

#if !defined(get16bits)
#define get16bits(d) ((((u32)(((u8k*)(d))[1])) << 8) + (u32)(((u8k*)(d))[0]))
#endif
//
// super hash function by Paul Hsieh
//
inline u32 HsiehHash(tukk data, i32 len)
{
	u32 hash = len, tmp;
	len >>= 2;

	/* Main loop */
	for (; len > 0; len--)
	{
		hash += get16bits(data);
		tmp = (get16bits(data + 2) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		data += 2 * sizeof(unsigned short);
		hash += hash >> 11;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

template <i32k CELLSIZE>
struct SparseSdf
{
	//
	// Inner types
	//
	struct IntFrac
	{
		i32 b;
		i32 i;
		Scalar f;
	};
	struct Cell
	{
		Scalar d[CELLSIZE + 1][CELLSIZE + 1][CELLSIZE + 1];
		i32 c[3];
		i32 puid;
		unsigned hash;
		const CollisionShape* pclient;
		Cell* next;
	};
	//
	// Fields
	//

	AlignedObjectArray<Cell*> cells;
	Scalar voxelsz;
	Scalar m_defaultVoxelsz;
	i32 puid;
	i32 ncells;
	i32 m_clampCells;
	i32 nprobes;
	i32 nqueries;

	~SparseSdf()
	{
		Reset();
	}
	//
	// Methods
	//

	//
	void Initialize(i32 hashsize = 2383, i32 clampCells = 256 * 1024)
	{
		//avoid a crash due to running out of memory, so clamp the maximum number of cells allocated
		//if this limit is reached, the SDF is reset (at the cost of some performance during the reset)
		m_clampCells = clampCells;
		cells.resize(hashsize, 0);
		m_defaultVoxelsz = 0.25;
		Reset();
	}
	//

	void setDefaultVoxelsz(Scalar sz)
	{
		m_defaultVoxelsz = sz;
	}

	void Reset()
	{
		for (i32 i = 0, ni = cells.size(); i < ni; ++i)
		{
			Cell* pc = cells[i];
			cells[i] = 0;
			while (pc)
			{
				Cell* pn = pc->next;
				delete pc;
				pc = pn;
			}
		}
		voxelsz = m_defaultVoxelsz;
		puid = 0;
		ncells = 0;
		nprobes = 1;
		nqueries = 1;
	}
	//
	void GarbageCollect(i32 lifetime = 256)
	{
		i32k life = puid - lifetime;
		for (i32 i = 0; i < cells.size(); ++i)
		{
			Cell*& root = cells[i];
			Cell* pp = 0;
			Cell* pc = root;
			while (pc)
			{
				Cell* pn = pc->next;
				if (pc->puid < life)
				{
					if (pp)
						pp->next = pn;
					else
						root = pn;
					delete pc;
					pc = pp;
					--ncells;
				}
				pp = pc;
				pc = pn;
			}
		}
		//printf("GC[%d]: %d cells, PpQ: %f\r\n",puid,ncells,nprobes/(Scalar)nqueries);
		nqueries = 1;
		nprobes = 1;
		++puid;  ///@todo: Reset puid's when i32 range limit is reached	*/
		/* else setup a priority list...						*/
	}
	//
	i32 RemoveReferences(CollisionShape* pcs)
	{
		i32 refcount = 0;
		for (i32 i = 0; i < cells.size(); ++i)
		{
			Cell*& root = cells[i];
			Cell* pp = 0;
			Cell* pc = root;
			while (pc)
			{
				Cell* pn = pc->next;
				if (pc->pclient == pcs)
				{
					if (pp)
						pp->next = pn;
					else
						root = pn;
					delete pc;
					pc = pp;
					++refcount;
				}
				pp = pc;
				pc = pn;
			}
		}
		return (refcount);
	}
	//
	Scalar Evaluate(const Vec3& x,
					  const CollisionShape* shape,
					  Vec3& normal,
					  Scalar margin)
	{
		/* Lookup cell			*/
		const Vec3 scx = x / voxelsz;
		const IntFrac ix = Decompose(scx.x());
		const IntFrac iy = Decompose(scx.y());
		const IntFrac iz = Decompose(scx.z());
		const unsigned h = Hash(ix.b, iy.b, iz.b, shape);
		Cell*& root = cells[static_cast<i32>(h % cells.size())];
		Cell* c = root;
		++nqueries;
		while (c)
		{
			++nprobes;
			if ((c->hash == h) &&
				(c->c[0] == ix.b) &&
				(c->c[1] == iy.b) &&
				(c->c[2] == iz.b) &&
				(c->pclient == shape))
			{
				break;
			}
			else
			{
				// printf("c->hash/c[0][1][2]=%d,%d,%d,%d\n", c->hash, c->c[0], c->c[1],c->c[2]);
				//printf("h,ixb,iyb,izb=%d,%d,%d,%d\n", h,ix.b, iy.b, iz.b);

				c = c->next;
			}
		}
		if (!c)
		{
			++nprobes;
			++ncells;
			//i32 sz = sizeof(Cell);
			if (ncells > m_clampCells)
			{
				//static i32 numResets = 0;
				//numResets++;
				//printf("numResets=%d\n",numResets);
				Reset();
			}

			c = new Cell();
			c->next = root;
			root = c;
			c->pclient = shape;
			c->hash = h;
			c->c[0] = ix.b;
			c->c[1] = iy.b;
			c->c[2] = iz.b;
			BuildCell(*c);
		}
		c->puid = puid;
		/* Extract infos		*/
		i32k o[] = {ix.i, iy.i, iz.i};
		const Scalar d[] = {c->d[o[0] + 0][o[1] + 0][o[2] + 0],
							  c->d[o[0] + 1][o[1] + 0][o[2] + 0],
							  c->d[o[0] + 1][o[1] + 1][o[2] + 0],
							  c->d[o[0] + 0][o[1] + 1][o[2] + 0],
							  c->d[o[0] + 0][o[1] + 0][o[2] + 1],
							  c->d[o[0] + 1][o[1] + 0][o[2] + 1],
							  c->d[o[0] + 1][o[1] + 1][o[2] + 1],
							  c->d[o[0] + 0][o[1] + 1][o[2] + 1]};
		/* Normal	*/
#if 1
		const Scalar gx[] = {d[1] - d[0], d[2] - d[3],
							   d[5] - d[4], d[6] - d[7]};
		const Scalar gy[] = {d[3] - d[0], d[2] - d[1],
							   d[7] - d[4], d[6] - d[5]};
		const Scalar gz[] = {d[4] - d[0], d[5] - d[1],
							   d[7] - d[3], d[6] - d[2]};
		normal.setX(Lerp(Lerp(gx[0], gx[1], iy.f),
						 Lerp(gx[2], gx[3], iy.f), iz.f));
		normal.setY(Lerp(Lerp(gy[0], gy[1], ix.f),
						 Lerp(gy[2], gy[3], ix.f), iz.f));
		normal.setZ(Lerp(Lerp(gz[0], gz[1], ix.f),
						 Lerp(gz[2], gz[3], ix.f), iy.f));
		normal.safeNormalize();
#else
		normal = Vec3(d[1] - d[0], d[3] - d[0], d[4] - d[0]).normalized();
#endif
		/* Distance	*/
		const Scalar d0 = Lerp(Lerp(d[0], d[1], ix.f),
								 Lerp(d[3], d[2], ix.f), iy.f);
		const Scalar d1 = Lerp(Lerp(d[4], d[5], ix.f),
								 Lerp(d[7], d[6], ix.f), iy.f);
		return (Lerp(d0, d1, iz.f) - margin);
	}
	//
	void BuildCell(Cell& c)
	{
		const Vec3 org = Vec3((Scalar)c.c[0],
										(Scalar)c.c[1],
										(Scalar)c.c[2]) *
							  CELLSIZE * voxelsz;
		for (i32 k = 0; k <= CELLSIZE; ++k)
		{
			const Scalar z = voxelsz * k + org.z();
			for (i32 j = 0; j <= CELLSIZE; ++j)
			{
				const Scalar y = voxelsz * j + org.y();
				for (i32 i = 0; i <= CELLSIZE; ++i)
				{
					const Scalar x = voxelsz * i + org.x();
					c.d[i][j][k] = DistanceToShape(Vec3(x, y, z),
												   c.pclient);
				}
			}
		}
	}
	//
	static inline Scalar DistanceToShape(const Vec3& x,
										   const CollisionShape* shape)
	{
		Transform2 unit;
		unit.setIdentity();
		if (shape->isConvex())
		{
			GjkEpaSolver2::sResults res;
			const ConvexShape* csh = static_cast<const ConvexShape*>(shape);
			return (GjkEpaSolver2::SignedDistance(x, 0, csh, unit, res));
		}
		return (0);
	}
	//
	static inline IntFrac Decompose(Scalar x)
	{
		/* That one need a lot of improvements...	*/
		/* Remove test, faster floor...				*/
		IntFrac r;
		x /= CELLSIZE;
		i32k o = x < 0 ? (i32)(-x + 1) : 0;
		x += o;
		r.b = (i32)x;
		const Scalar k = (x - r.b) * CELLSIZE;
		r.i = (i32)k;
		r.f = k - r.i;
		r.b -= o;
		return (r);
	}
	//
	static inline Scalar Lerp(Scalar a, Scalar b, Scalar t)
	{
		return (a + (b - a) * t);
	}

	//
	static inline u32 Hash(i32 x, i32 y, i32 z, const CollisionShape* shape)
	{
		struct S
		{
			i32 x, y, z, w;
			uk p;
		};

		S myset;
		//memset may be needed in case of additional (uninitialized) padding!
		//memset(&myset, 0, sizeof(S));

		myset.x = x;
		myset.y = y;
		myset.z = z;
		myset.w = 0;
		myset.p = (uk )shape;
		tukk ptr = (tukk)&myset;

		u32 result = HsiehHash(ptr, sizeof(S));

		return result;
	}
};

#endif  //DRX3D_SPARSE_SDF_H
