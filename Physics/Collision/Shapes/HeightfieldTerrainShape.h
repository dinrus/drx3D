#ifndef DRX3D_HEIGHTFIELD_TERRAIN_SHAPE_H
#define DRX3D_HEIGHTFIELD_TERRAIN_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

//HeightfieldTerrainShape simulates a 2D heightfield terrain
/**
  The caller is responsible for maintaining the heightfield array; this
  class does not make a copy.

  The heightfield can be dynamic so long as the min/max height values
  capture the extremes (heights must always be in that range).

  The local origin of the heightfield is assumed to be the exact
  center (as determined by width and length and height, with each
  axis multiplied by the localScaling).

  \b NOTE: be careful with coordinates.  If you have a heightfield with a local
  min height of -100m, and a max height of +500m, you may be tempted to place it
  at the origin (0,0) and expect the heights in world coordinates to be
  -100 to +500 meters.
  Actually, the heights will be -300 to +300m, because bullet will re-center
  the heightfield based on its AABB (which is determined by the min/max
  heights).  So keep in mind that once you create a HeightfieldTerrainShape
  object, the heights will be adjusted relative to the center of the AABB.  This
  is different to the behavior of many rendering engines, but is useful for
  physics engines.

  Most (but not all) rendering and heightfield libraries assume upAxis = 1
  (that is, the y-axis is "up").  This class allows any of the 3 coordinates
  to be "up".  Make sure your choice of axis is consistent with your rendering
  system.

  The heightfield heights are determined from the data type used for the
  heightfieldData array.  

   - u8: height at a point is the uchar value at the
       grid point, multipled by heightScale.  uchar isn't recommended
       because of its inability to deal with negative values, and
       low resolution (8-bit).

   - short: height at a point is the i16 value at that grid
       point, multipled by heightScale.

   - float or dobule: height at a point is the value at that grid point.

  Whatever the caller specifies as minHeight and maxHeight will be honored.
  The class will not inspect the heightfield to discover the actual minimum
  or maximum heights.  These values are used to determine the heightfield's
  axis-aligned bounding box, multiplied by localScaling.

  For usage and testing see the TerrainDemo.
 */
ATTRIBUTE_ALIGNED16(class)
HeightfieldTerrainShape : public ConcaveShape
{
public:
	struct Range
	{
		Range() {}
		Range(Scalar min, Scalar max) : min(min), max(max) {}

		bool overlaps(const Range& other) const
		{
			return !(min > other.max || max < other.min);
		}

		Scalar min;
		Scalar max;
	};

protected:
	Vec3 m_localAabbMin;
	Vec3 m_localAabbMax;
	Vec3 m_localOrigin;

	///terrain data
	i32 m_heightStickWidth;
	i32 m_heightStickLength;
	Scalar m_minHeight;
	Scalar m_maxHeight;
	Scalar m_width;
	Scalar m_length;
	Scalar m_heightScale;
	union {
		u8k* m_heightfieldDataUnsignedChar;
		const short* m_heightfieldDataShort;
		const float* m_heightfieldDataFloat;
		const double* m_heightfieldDataDouble;
		ukk m_heightfieldDataUnknown;
	};

	PHY_ScalarType m_heightDataType;
	bool m_flipQuadEdges;
	bool m_useDiamondSubdivision;
	bool m_useZigzagSubdivision;
	bool m_flipTriangleWinding;
	i32 m_upAxis;

	Vec3 m_localScaling;

	// Accelerator
	AlignedObjectArray<Range> m_vboundsGrid;
	i32 m_vboundsGridWidth;
	i32 m_vboundsGridLength;
	i32 m_vboundsChunkSize;

	
	Scalar m_userValue3;

	struct TriangleInfoMap* m_triangleInfoMap;

	virtual Scalar getRawHeightFieldValue(i32 x, i32 y) const;
	void quantizeWithClamp(i32* out, const Vec3& point, i32 isMax) const;

	/// protected initialization
	/**
	  Handles the work of constructors so that public constructors can be
	  backwards-compatible without a lot of copy/paste.
	 */
	void initialize(i32 heightStickWidth, i32 heightStickLength,
					ukk heightfieldData, Scalar heightScale,
					Scalar minHeight, Scalar maxHeight, i32 upAxis,
					PHY_ScalarType heightDataType, bool flipQuadEdges);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	/// preferred constructors
	HeightfieldTerrainShape(
		i32 heightStickWidth, i32 heightStickLength,
		const float* heightfieldData, Scalar minHeight, Scalar maxHeight,
		i32 upAxis, bool flipQuadEdges);
	HeightfieldTerrainShape(
		i32 heightStickWidth, i32 heightStickLength,
		const double* heightfieldData, Scalar minHeight, Scalar maxHeight,
		i32 upAxis, bool flipQuadEdges);
	HeightfieldTerrainShape(
		i32 heightStickWidth, i32 heightStickLength,
		const short* heightfieldData, Scalar heightScale, Scalar minHeight, Scalar maxHeight,
		i32 upAxis, bool flipQuadEdges);
	HeightfieldTerrainShape(
		i32 heightStickWidth, i32 heightStickLength,
		u8k* heightfieldData, Scalar heightScale, Scalar minHeight, Scalar maxHeight,
		i32 upAxis, bool flipQuadEdges);

	/// legacy constructor
	/**
	  This constructor supports a range of heightfield
	  data types, and allows for a non-zero minimum height value.
	  heightScale is needed for any integer-based heightfield data types.

	  This legacy constructor considers `PHY_FLOAT` to mean `Scalar`.
	  With `DRX3D_USE_DOUBLE_PRECISION`, it will expect `heightfieldData`
	  to be double-precision.
	 */
	HeightfieldTerrainShape(i32 heightStickWidth, i32 heightStickLength,
							  ukk heightfieldData, Scalar heightScale,
							  Scalar minHeight, Scalar maxHeight,
							  i32 upAxis, PHY_ScalarType heightDataType,
							  bool flipQuadEdges);

	/// legacy constructor
	/**
	  The legacy constructor assumes the heightfield has a minimum height
	  of zero.  Only u8 or Scalar data are supported.  For legacy
	  compatibility reasons, heightScale is calculated as maxHeight / 65535 
	  (and is only used when useFloatData = false).
 	 */
	HeightfieldTerrainShape(i32 heightStickWidth, i32 heightStickLength, ukk heightfieldData, Scalar maxHeight, i32 upAxis, bool useFloatData, bool flipQuadEdges);

	virtual ~HeightfieldTerrainShape();

	void setUseDiamondSubdivision(bool useDiamondSubdivision = true) { m_useDiamondSubdivision = useDiamondSubdivision; }

	///could help compatibility with Ogre heightfields. See https://code.google.com/p/bullet/issues/detail?id=625
	void setUseZigzagSubdivision(bool useZigzagSubdivision = true) { m_useZigzagSubdivision = useZigzagSubdivision; }

	void setFlipTriangleWinding(bool flipTriangleWinding)
	{
		m_flipTriangleWinding = flipTriangleWinding;
	}
	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void processAllTriangles(TriangleCallback * callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual void setLocalScaling(const Vec3& scaling);

	virtual const Vec3& getLocalScaling() const;

	void getVertex(i32 x, i32 y, Vec3& vertex) const;

	void performRaycast(TriangleCallback * callback, const Vec3& raySource, const Vec3& rayTarget) const;

	void buildAccelerator(i32 chunkSize = 16);
	void clearAccelerator();

	i32 getUpAxis() const
	{
		return m_upAxis;
	}
	//debugging
	virtual tukk getName() const { return "HEIGHTFIELD"; }

	
	void setUserValue3(Scalar value)
	{
		m_userValue3 = value;
	}
	Scalar getUserValue3() const
	{
		return m_userValue3;
	}
	const struct TriangleInfoMap* getTriangleInfoMap() const
	{
		return m_triangleInfoMap;
	}
	struct TriangleInfoMap* getTriangleInfoMap()
	{
		return m_triangleInfoMap;
	}
	void setTriangleInfoMap(TriangleInfoMap* map)
	{
		m_triangleInfoMap = map;
	}
	u8k* getHeightfieldRawData() const
	{
		return m_heightfieldDataUnsignedChar;
	}
};

#endif  //DRX3D_HEIGHTFIELD_TERRAIN_SHAPE_H
