// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SPATIAL_HASH_GRID_
#define _SPATIAL_HASH_GRID_
#pragma once

#if GLASSCFG_USE_HASH_GRID

//==================================================================================================
// Name: CSpatialHashGrid
// Desc: Templated 2D spatial hashing grid spanning positively from origin
// Author: Chris Bunner
//==================================================================================================
template<class T, u32 GridSize, u32 BucketSize>
class CSpatialHashGrid
{
public:
	CSpatialHashGrid(const float gridWidth, const float gridHeight);
	~CSpatialHashGrid();

	u32 HashPosition(const float x, const float y);
	void   AddElementToGrid(const float x, const float y, const T& elem);
	void   AddElementToGrid(u32k index, const T& elem);
	void   RemoveElementFromGrid(const float x, const float y, const T& elem);
	void   RemoveElementFromGrid(u32k index, const T& elem);

	void   ClearGrid();

	#ifndef RELEASE
	void DebugDraw();
	#endif

	// Bucket accessors
	ILINE const DrxFixedArray<T, BucketSize>* const GetBucket(u32k index)
	{
		return (index < m_area) ? &m_buckets[index] : NULL;
	}

	ILINE u32 GetNumBuckets()
	{
		return m_area;
	}

	// Resizing
	ILINE void Resize(const float gridWidth, const float gridHeight)
	{
		m_invCellWidth = m_fGridSize / gridWidth;
		m_invCellHeight = m_fGridSize / gridHeight;
	}

private:
	CSpatialHashGrid(const CSpatialHashGrid& rhs);
	CSpatialHashGrid& operator=(const CSpatialHashGrid& rhs);

	u32                       m_gridSize;
	u32                       m_area;

	float                        m_fGridSize;
	float                        m_invCellWidth;
	float                        m_invCellHeight;

	DrxFixedArray<T, BucketSize> m_buckets[GridSize * GridSize];
};//------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Constructor
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
CSpatialHashGrid<T, GridSize, BucketSize>::CSpatialHashGrid(const float gridWidth, const float gridHeight)
	: m_gridSize(GridSize)
	, m_area(GridSize * GridSize)
	, m_fGridSize((float)GridSize)
	, m_invCellWidth(0.0f)
	, m_invCellHeight(0.0f)
{
	Resize(gridWidth, gridHeight);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Destructor
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
CSpatialHashGrid<T, GridSize, BucketSize>::~CSpatialHashGrid()
{
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: HashPosition
// Desc: Hashes a position to cell index and performs simple bounds checking
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
u32 CSpatialHashGrid<T, GridSize, BucketSize >::HashPosition(const float x, const float y)
{
	u32k actualCellIndex = (u32)(y * m_invCellHeight * m_fGridSize + x * m_invCellWidth);
	u32k errorCellIndex = (u32) - 1;

	float cellX = x * m_invCellWidth;
	float cellY = y * m_invCellHeight;

	// Handle out of bounds
	cellX = (float)__fsel(cellX, cellX, -1.0f);
	cellX = (float)__fsel(m_fGridSize - cellX, cellX, -1.0f);
	cellY = (float)__fsel(cellY, cellY, -1.0f);
	cellY = (float)__fsel(m_fGridSize - cellY, cellY, -1.0f);

	cellX = min(cellX, cellY);

	u32k cellIndex = (cellX < 0.0f) ? errorCellIndex : actualCellIndex;
	return cellIndex;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: AddElementToGrid
// Desc: Hashes and adds a unique element to the grid buckets
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
void CSpatialHashGrid<T, GridSize, BucketSize >::AddElementToGrid(const float x, const float y, const T& elem)
{
	u32 elemCell = HashPosition(x, y);
	AddElementToGrid(elemCell, elem);
}

template<class T, u32 GridSize, u32 BucketSize>
void CSpatialHashGrid<T, GridSize, BucketSize >::AddElementToGrid(u32k index, const T& elem)
{
	if (index < m_area)
	{
		// Only add unique elements
		DrxFixedArray<T, BucketSize>* pBucket = &m_buckets[index];
		const uint numElems = pBucket->size();
		const T* pElems = pBucket->begin();
		i32 elemIndex = -1;

		if (numElems < BucketSize)
		{
			for (uint i = 0; i < numElems; ++i)
			{
				if (pElems[i] == elem)
				{
					elemIndex = i;
					break;
				}
			}

			if (elemIndex == -1)
			{
				pBucket->push_back(elem);
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: RemoveElementFromGrid
// Desc: Hashes and removes an element from the grid buckets
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
void CSpatialHashGrid<T, GridSize, BucketSize >::RemoveElementFromGrid(const float x, const float y, const T& elem)
{
	u32 elemCell = HashPosition(x, y);
	RemoveElementFromGrid(elemCell, elem);
}

template<class T, u32 GridSize, u32 BucketSize>
void CSpatialHashGrid<T, GridSize, BucketSize >::RemoveElementFromGrid(u32k index, const T& elem)
{
	if (index < m_area)
	{
		DrxFixedArray<T, BucketSize>* pBucket = &m_buckets[index];
		const uint numElems = pBucket->size();
		T* pElems = pBucket->begin();

		for (uint i = 0; i < numElems; ++i)
		{
			if (pElems[i] == elem)
			{
				pElems[i] = pElems[numElems - 1];
				pBucket->pop_back();
				break;
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ClearGrid
// Desc: Clears all grid buckets
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
void CSpatialHashGrid<T, GridSize, BucketSize >::ClearGrid()
{
	for (u32 i = 0; i < m_area; ++i)
	{
		m_buckets[i].clear();
	}
}//-------------------------------------------------------------------------------------------------

	#ifndef RELEASE
//--------------------------------------------------------------------------------------------------
// Name: DebugDraw
// Desc: Draws the grid element counts to the screen
//--------------------------------------------------------------------------------------------------
template<class T, u32 GridSize, u32 BucketSize>
void CSpatialHashGrid<T, GridSize, BucketSize >::DebugDraw()
{
	IRenderAuxGeom* pRenderer = gEnv->pRenderer->GetIRenderAuxGeom();
	SAuxGeomRenderFlags oldFlags = pRenderer->GetRenderFlags();

	SAuxGeomRenderFlags newFlags = e_Def2DPublicRenderflags;
	newFlags.SetCullMode(e_CullModeNone);
	newFlags.SetDepthWriteFlag(e_DepthWriteOff);
	newFlags.SetAlphaBlendMode(e_AlphaBlended);
	pRenderer->SetRenderFlags(newFlags);

	// Draw black backing
	const float invWidth  = 1.0f / float(pRenderer->GetCamera().GetViewSurfaceX());
	const float invHeight = 1.0f / float(pRenderer->GetCamera().GetViewSurfaceZ());

	const float minX = (322.5f - m_fGridSize * 15.0f) * invWidth;
	const float maxX = (322.5f) * invWidth;
	const float minY = (370.0f) * invHeight;
	const float maxY = (370.0f + m_fGridSize * 15.0f) * invHeight;

	Vec3 quad[6] =
	{
		Vec3(minX, minY, 0.0f),
		Vec3(minX, maxY, 0.0f),
		Vec3(maxX, maxY, 0.0f),

		Vec3(minX, minY, 0.0f),
		Vec3(maxX, maxY, 0.0f),
		Vec3(maxX, minY, 0.0f)
	};

	pRenderer->DrawTriangles(quad, 6, ColorB(0, 0, 0, 127));
	pRenderer->SetRenderFlags(oldFlags);

	// Draw hash grid data
	const ColorF col[8] =
	{
		ColorF(0.0f, 0.0f, 0.0f, 1.0f),
		ColorF(1.0f, 0.0f, 0.0f, 1.0f),
		ColorF(0.5f, 1.0f, 0.0f, 1.0f),
		ColorF(0.0f, 1.0f, 0.0f, 1.0f),
		ColorF(0.0f, 1.0f, 0.5f, 1.0f),
		ColorF(0.0f, 0.5f, 1.0f, 1.0f),
		ColorF(0.0f, 0.0f, 1.0f, 1.0f),
		ColorF(0.5f, 0.0f, 1.0f, 1.0f),
	};

	float x = 315.0f, y = 370.0f;

	for (u32 i = 0; i < m_gridSize; ++i)
	{
		for (u32 j = 0; j < m_gridSize; ++j)
		{
			i32 index = i * m_gridSize + j;
			i32 count = (i32)m_buckets[index].size();
			ColorF textCol = col[min < i32 > (count, 7)];

			if (count > 0)
			{
				textCol.r = textCol.r * 0.4f + 0.6f;
				textCol.g = textCol.g * 0.4f + 0.6f;
				textCol.b = textCol.b * 0.4f + 0.6f;
			}

			IRenderAuxText::Draw2dLabel(x, y, 1.0f, &textCol.r, false, "%i", count);
			y += 15.0f;
		}

		x -= 15.0f;
		y = 370.0f;
	}
}//-------------------------------------------------------------------------------------------------
	#endif // !RELEASE

#endif // GLASSCFG_USE_HASH_GRID
#endif // _SPATIAL_HASH_GRID_
