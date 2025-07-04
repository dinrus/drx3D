// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _PRIMATIVES_H_
#define _PRIMATIVES_H_
#pragma once

typedef i32 index_t;
enum { PHYS_MAX_INDICES = 1 << 30 };

#include <drx3D/CoreX/stridedptr.h>

namespace primitives {

////////////////////////// примитивы //////////////////////

struct primitive
{
};

struct box : primitive
{
	enum entype { type = 0 };
	Matrix33 Basis;   //!< v_box = Basis*v_world; Basis = Rotation.T().
	i32      bOriented;
	Vec3     center;
	Vec3     size;
	AUTO_STRUCT_INFO;
};

struct triangle : primitive
{
	enum entype { type = 1 };
	Vec3 pt[3];
	Vec3 n;
	AUTO_STRUCT_INFO;
};

struct indexed_triangle : triangle
{
	i32 idx;
	AUTO_STRUCT_INFO;
};

typedef float (*         getHeightCallback)(i32 ix, i32 iy);
typedef u8 (* getSurfTypeCallback)(i32 ix, i32 iy);

struct grid : primitive
{
	Matrix33 Basis;
	i32      bOriented;
	Vec3     origin;
	Vec2     step, stepr;
	Vec2i    size;
	Vec2i    stride;
	i32      bCyclic;
	grid() { bCyclic = 0; }
	i32   inrange(i32 ix, i32 iy)                       { return bCyclic | -((ix - size.x & - 1 - ix & iy - size.y & - 1 - iy) >> 31); }
	i32   getcell_safe(i32 ix, i32 iy)                  { i32 mask = -inrange(ix, iy); return (iy & size.y - 1) * stride.y + (ix & size.x - 1) * stride.x & mask | size.x * size.y & ~mask; }
	i32   crop(i32 i, i32 icoord, i32 bAllowBorder = 1) { i32 brd = bAllowBorder + (1 << 30 & - bCyclic); return max(-brd, min(size[icoord] - 1 + brd, i)); }
	Vec2i cropxy(const Vec2i& ic, i32 bAllowBorder = 1) { i32 brd = bAllowBorder + (1 << 30 & - bCyclic); return Vec2i(max(-brd, min(size.x - 1 + brd, ic.x)), max(-brd, min(size.y - 1 + brd, ic.y))); }
	i32   iscyclic()                                    { return bCyclic; }
	AUTO_STRUCT_INFO;
};

struct heightfield : grid
{
	enum entype { type = 2 };
	heightfield& operator=(const heightfield& src)
	{
		step = src.step;
		stepr = src.stepr;
		size = src.size;
		stride = src.stride;
		heightscale = src.heightscale;
		typemask = src.typemask;
		heightmask = src.heightmask;
		typehole = src.typehole;
		typepower = src.typepower;
		fpGetHeightCallback = src.fpGetHeightCallback;
		fpGetSurfTypeCallback = src.fpGetSurfTypeCallback;
		return *this;
	}

	ILINE float getheight(i32 ix, i32 iy) const
	{
		float result = (*fpGetHeightCallback)(ix, iy);
		return result * heightscale;
	}
	ILINE i32 gettype(i32 ix, i32 iy) const
	{
		i32 itype = (((*fpGetSurfTypeCallback)(ix, iy)) & typemask) >> typepower, idelta = itype - typehole;
		return itype | ((idelta - 1) >> 31 ^ idelta >> 31);
	}

	float               heightscale;
	unsigned short      typemask, heightmask;
	i32                 typehole;
	i32                 typepower;
	getHeightCallback   fpGetHeightCallback;
	getSurfTypeCallback fpGetSurfTypeCallback;
};

struct ray : primitive
{
	enum entype { type = 3 };
	Vec3 origin;
	Vec3 dir;
	AUTO_STRUCT_INFO;
};

struct sphere : primitive
{
	enum entype { type = 4 };
	Vec3  center;
	float r;
	AUTO_STRUCT_INFO;
};

struct cylinder : primitive
{
	enum entype { type = 5 };
	Vec3  center;
	Vec3  axis;
	float r, hh;
	AUTO_STRUCT_INFO;
};

struct capsule : cylinder
{
	enum entype { type = 6 };
	AUTO_STRUCT_INFO;
};

struct grid3d : primitive
{
	Matrix33 Basis;
	i32      bOriented;
	Vec3     origin;
	Vec3     step, stepr;
	Vec3i    size;
	Vec3i    stride;
};

struct voxelgrid : grid3d
{
	enum entype { type = 7 };
	Matrix33              R;
	Vec3                  offset;
	float                 scale, rscale;
	strided_pointer<Vec3> pVtx;
	index_t*              pIndices;
	Vec3*                 pNormals;
	tuk                 pIds;
	i32*                  pCellTris;
	i32*                  pTriBuf;
};

struct plane : primitive
{
	enum entype { type = 8 };
	Vec3 n;
	Vec3 origin;
	AUTO_STRUCT_INFO;
};

struct coord_plane : plane
{
	Vec3 axes[2];
	AUTO_STRUCT_INFO;
};
}

AUTO_TYPE_INFO(primitives::getHeightCallback);
AUTO_TYPE_INFO(primitives::getSurfTypeCallback);

struct prim_inters
{
	prim_inters() { minPtDist2 = 0.0f; ptbest.zero(); }
	Vec3          pt[2];
	Vec3          n;
	u8 iFeature[2][2];
	float         minPtDist2;
	short         id[2];
	i32           iNode[2];
	Vec3*         ptborder;
	i32           nborderpt, nbordersz;
	Vec3          ptbest;
	i32           nBestPtVal;
};

struct contact
{
	real         t, taux;
	Vec3         pt;
	Vec3         n;
	u32 iFeature[2];
};

i32k NPRIMS = 8; //!< Since plane is currently not supported in collision checks.

//! \cond INTERNAL
//! Used in qhull2d.
struct ptitem2d
{
	Vec2      pt;
	ptitem2d* next, * prev;
	i32       iContact;
};
//! \endcond

struct edgeitem
{
	ptitem2d* pvtx;
	ptitem2d* plist;
	edgeitem* next, * prev;
	float     area, areanorm2;
	edgeitem* next1, * prev1;
	i32       idx;
};

///////////////////// geometry contact structures ///////////////////

struct geom_contact_area
{
	enum entype { polygon, polyline };
	i32   type;
	i32   npt;
	i32   nmaxpt;
	float minedge;
	i32*  piPrim[2];
	i32*  piFeature[2];
	Vec3* pt;
	Vec3  n1; //!< Normal of other object surface (or edge).
};

i32k IFEAT_LOG2 = 23;
i32k IDXMASK = ~(0xFF << IFEAT_LOG2);
i32k TRIEND = 0x80 << IFEAT_LOG2;

struct geom_contact
{
	real               t;
	Vec3               pt;
	Vec3               n;
	Vec3               dir; //!< Unprojection direction.
	i32                iUnprojMode;
	float              vel;   //!< Original velocity along this direction, <0 if least squares normal was used.
	i32                id[2]; //!< External ids for colliding geometry parts.
	i32                iPrim[2];
	i32                iFeature[2];
	i32                iNode[2]; //!< BV-tree nodes of contacting primitives.
	Vec3*              ptborder; //!< Ontersection border.
	i32 (*idxborder)[2];         //!< Primitive index | primitive's feature's id << IFEAT_LOG2.
	i32                nborderpt;
	i32                bClosed;
	Vec3               center;
	bool               bBorderConsecutive;
	geom_contact_area* parea;
};

#endif
