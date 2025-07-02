#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

HeightfieldTerrainShape::HeightfieldTerrainShape(
	i32 heightStickWidth, i32 heightStickLength,
	const float* heightfieldData, Scalar minHeight, Scalar maxHeight,
	i32 upAxis, bool flipQuadEdges)
	: m_userValue3(0), m_triangleInfoMap(0)
{
	initialize(heightStickWidth, heightStickLength, heightfieldData,
			   /*heightScale=*/1, minHeight, maxHeight, upAxis, PHY_FLOAT,
			   flipQuadEdges);
}

HeightfieldTerrainShape::HeightfieldTerrainShape(
	i32 heightStickWidth, i32 heightStickLength, const double* heightfieldData,
	Scalar minHeight, Scalar maxHeight, i32 upAxis, bool flipQuadEdges)
	: m_userValue3(0), m_triangleInfoMap(0)
{
	initialize(heightStickWidth, heightStickLength, heightfieldData,
			   /*heightScale=*/1, minHeight, maxHeight, upAxis, PHY_DOUBLE,
			   flipQuadEdges);
}

HeightfieldTerrainShape::HeightfieldTerrainShape(
	i32 heightStickWidth, i32 heightStickLength, const short* heightfieldData, Scalar heightScale,
	Scalar minHeight, Scalar maxHeight, i32 upAxis, bool flipQuadEdges)
	: m_userValue3(0), m_triangleInfoMap(0)
{
	initialize(heightStickWidth, heightStickLength, heightfieldData,
			   heightScale, minHeight, maxHeight, upAxis, PHY_SHORT,
			   flipQuadEdges);
}

HeightfieldTerrainShape::HeightfieldTerrainShape(
	i32 heightStickWidth, i32 heightStickLength, u8k* heightfieldData, Scalar heightScale,
	Scalar minHeight, Scalar maxHeight, i32 upAxis, bool flipQuadEdges)
	: m_userValue3(0), m_triangleInfoMap(0)
{
	initialize(heightStickWidth, heightStickLength, heightfieldData,
			   heightScale, minHeight, maxHeight, upAxis, PHY_UCHAR,
			   flipQuadEdges);
}

HeightfieldTerrainShape::HeightfieldTerrainShape(
	i32 heightStickWidth, i32 heightStickLength, ukk heightfieldData,
	Scalar heightScale, Scalar minHeight, Scalar maxHeight, i32 upAxis,
	PHY_ScalarType hdt, bool flipQuadEdges)
	:m_userValue3(0),
	m_triangleInfoMap(0)
{
	// legacy constructor: Assumes PHY_FLOAT means Scalar.
#ifdef DRX3D_USE_DOUBLE_PRECISION
	if (hdt == PHY_FLOAT) hdt = PHY_DOUBLE;
#endif
	initialize(heightStickWidth, heightStickLength, heightfieldData,
			   heightScale, minHeight, maxHeight, upAxis, hdt,
			   flipQuadEdges);
}

HeightfieldTerrainShape::HeightfieldTerrainShape(i32 heightStickWidth, i32 heightStickLength, ukk heightfieldData, Scalar maxHeight, i32 upAxis, bool useFloatData, bool flipQuadEdges)
	:	m_userValue3(0),
	m_triangleInfoMap(0)
{
	// legacy constructor: support only Scalar or u8 data,
	// and min height is zero.
	PHY_ScalarType hdt = (useFloatData) ? PHY_FLOAT : PHY_UCHAR;
#ifdef DRX3D_USE_DOUBLE_PRECISION
	if (hdt == PHY_FLOAT) hdt = PHY_DOUBLE;
#endif
	Scalar minHeight = 0.0f;

	// previously, height = uchar * maxHeight / 65535.
	// So to preserve legacy behavior, heightScale = maxHeight / 65535
	Scalar heightScale = maxHeight / 65535;

	initialize(heightStickWidth, heightStickLength, heightfieldData,
			   heightScale, minHeight, maxHeight, upAxis, hdt,
			   flipQuadEdges);
}

void HeightfieldTerrainShape::initialize(
	i32 heightStickWidth, i32 heightStickLength, ukk heightfieldData,
	Scalar heightScale, Scalar minHeight, Scalar maxHeight, i32 upAxis,
	PHY_ScalarType hdt, bool flipQuadEdges)
{
	// validation
	Assert(heightStickWidth > 1);   // && "bad width");
	Assert(heightStickLength > 1);  // && "bad length");
	Assert(heightfieldData);        // && "null heightfield data");
	// Assert(heightScale) -- do we care?  Trust caller here
	Assert(minHeight <= maxHeight);                                    // && "bad min/max height");
	Assert(upAxis >= 0 && upAxis < 3);                                 // && "bad upAxis--should be in range [0,2]");
	Assert(hdt != PHY_UCHAR || hdt != PHY_FLOAT || hdt != PHY_DOUBLE || hdt != PHY_SHORT);  // && "Bad height data type enum");

	// initialize member variables
	m_shapeType = TERRAIN_SHAPE_PROXYTYPE;
	m_heightStickWidth = heightStickWidth;
	m_heightStickLength = heightStickLength;
	m_minHeight = minHeight;
	m_maxHeight = maxHeight;
	m_width = (Scalar)(heightStickWidth - 1);
	m_length = (Scalar)(heightStickLength - 1);
	m_heightScale = heightScale;
	m_heightfieldDataUnknown = heightfieldData;
	m_heightDataType = hdt;
	m_flipQuadEdges = flipQuadEdges;
	m_useDiamondSubdivision = false;
	m_useZigzagSubdivision = false;
	m_flipTriangleWinding = false;
	m_upAxis = upAxis;
	m_localScaling.setVal(Scalar(1.), Scalar(1.), Scalar(1.));
	
	m_vboundsChunkSize = 0;
	m_vboundsGridWidth = 0;
	m_vboundsGridLength = 0;

	// determine min/max axis-aligned bounding box (aabb) values
	switch (m_upAxis)
	{
		case 0:
		{
			m_localAabbMin.setVal(m_minHeight, 0, 0);
			m_localAabbMax.setVal(m_maxHeight, m_width, m_length);
			break;
		}
		case 1:
		{
			m_localAabbMin.setVal(0, m_minHeight, 0);
			m_localAabbMax.setVal(m_width, m_maxHeight, m_length);
			break;
		};
		case 2:
		{
			m_localAabbMin.setVal(0, 0, m_minHeight);
			m_localAabbMax.setVal(m_width, m_length, m_maxHeight);
			break;
		}
		default:
		{
			//need to get valid m_upAxis
			Assert(0);  // && "Bad m_upAxis");
		}
	}

	// remember origin (defined as exact middle of aabb)
	m_localOrigin = Scalar(0.5) * (m_localAabbMin + m_localAabbMax);
}

HeightfieldTerrainShape::~HeightfieldTerrainShape()
{
	clearAccelerator();
}

void HeightfieldTerrainShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	Vec3 halfExtents = (m_localAabbMax - m_localAabbMin) * m_localScaling * Scalar(0.5);

	Vec3 localOrigin(0, 0, 0);
	localOrigin[m_upAxis] = (m_minHeight + m_maxHeight) * Scalar(0.5);
	localOrigin *= m_localScaling;

	Matrix3x3 abs_b = t.getBasis().absolute();
	Vec3 center = t.getOrigin();
	Vec3 extent = halfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
	extent += Vec3(getMargin(), getMargin(), getMargin());

	aabbMin = center - extent;
	aabbMax = center + extent;
}

/// This returns the "raw" (user's initial) height, not the actual height.
/// The actual height needs to be adjusted to be relative to the center
///   of the heightfield's AABB.
Scalar
HeightfieldTerrainShape::getRawHeightFieldValue(i32 x, i32 y) const
{
	Scalar val = 0.f;
	switch (m_heightDataType)
	{
		case PHY_FLOAT:
		{
			val = m_heightfieldDataFloat[(y * m_heightStickWidth) + x];
			break;
		}

		case PHY_DOUBLE:
		{
			val = m_heightfieldDataDouble[(y * m_heightStickWidth) + x];
			break;
		}

		case PHY_UCHAR:
		{
			u8 heightFieldValue = m_heightfieldDataUnsignedChar[(y * m_heightStickWidth) + x];
			val = heightFieldValue * m_heightScale;
			break;
		}

		case PHY_SHORT:
		{
			short hfValue = m_heightfieldDataShort[(y * m_heightStickWidth) + x];
			val = hfValue * m_heightScale;
			break;
		}

		default:
		{
			Assert(!"Bad m_heightDataType");
		}
	}

	return val;
}

/// this returns the vertex in bullet-local coordinates
void HeightfieldTerrainShape::getVertex(i32 x, i32 y, Vec3& vertex) const
{
	Assert(x >= 0);
	Assert(y >= 0);
	Assert(x < m_heightStickWidth);
	Assert(y < m_heightStickLength);

	Scalar height = getRawHeightFieldValue(x, y);

	switch (m_upAxis)
	{
		case 0:
		{
			vertex.setVal(
				height - m_localOrigin.getX(),
				(-m_width / Scalar(2.0)) + x,
				(-m_length / Scalar(2.0)) + y);
			break;
		}
		case 1:
		{
			vertex.setVal(
				(-m_width / Scalar(2.0)) + x,
				height - m_localOrigin.getY(),
				(-m_length / Scalar(2.0)) + y);
			break;
		};
		case 2:
		{
			vertex.setVal(
				(-m_width / Scalar(2.0)) + x,
				(-m_length / Scalar(2.0)) + y,
				height - m_localOrigin.getZ());
			break;
		}
		default:
		{
			//need to get valid m_upAxis
			Assert(0);
		}
	}

	vertex *= m_localScaling;
}

static inline i32
getQuantized(
	Scalar x)
{
	if (x < 0.0)
	{
		return (i32)(x - 0.5);
	}
	return (i32)(x + 0.5);
}

// Equivalent to std::minmax({a, b, c}).
// Performs at most 3 comparisons.
static HeightfieldTerrainShape::Range minmaxRange(Scalar a, Scalar b, Scalar c)
{
	if (a > b)
	{
		if (b > c)
			return HeightfieldTerrainShape::Range(c, a);
		else if (a > c)
			return HeightfieldTerrainShape::Range(b, a);
		else
			return HeightfieldTerrainShape::Range(b, c);
	}
	else
	{
		if (a > c)
			return HeightfieldTerrainShape::Range(c, b);
		else if (b > c)
			return HeightfieldTerrainShape::Range(a, b);
		else
			return HeightfieldTerrainShape::Range(a, c);
	}
}

/// given input vector, return quantized version
/**
  This routine is basically determining the gridpoint indices for a given
  input vector, answering the question: "which gridpoint is closest to the
  provided point?".

  "with clamp" means that we restrict the point to be in the heightfield's
  axis-aligned bounding box.
 */
void HeightfieldTerrainShape::quantizeWithClamp(i32* out, const Vec3& point, i32 /*isMax*/) const
{
	Vec3 clampedPoint(point);
	clampedPoint.setMax(m_localAabbMin);
	clampedPoint.setMin(m_localAabbMax);

	out[0] = getQuantized(clampedPoint.getX());
	out[1] = getQuantized(clampedPoint.getY());
	out[2] = getQuantized(clampedPoint.getZ());
}

/// process all triangles within the provided axis-aligned bounding box
/**
  basic algorithm:
    - convert input aabb to local coordinates (scale down and shift for local origin)
    - convert input aabb to a range of heightfield grid points (quantize)
    - iterate over all triangles in that subset of the grid
 */
void HeightfieldTerrainShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	// scale down the input aabb's so they are in local (non-scaled) coordinates
	Vec3 localAabbMin = aabbMin * Vec3(1.f / m_localScaling[0], 1.f / m_localScaling[1], 1.f / m_localScaling[2]);
	Vec3 localAabbMax = aabbMax * Vec3(1.f / m_localScaling[0], 1.f / m_localScaling[1], 1.f / m_localScaling[2]);

	// account for local origin
	localAabbMin += m_localOrigin;
	localAabbMax += m_localOrigin;

	//quantize the aabbMin and aabbMax, and adjust the start/end ranges
	i32 quantizedAabbMin[3];
	i32 quantizedAabbMax[3];
	quantizeWithClamp(quantizedAabbMin, localAabbMin, 0);
	quantizeWithClamp(quantizedAabbMax, localAabbMax, 1);

	// expand the min/max quantized values
	// this is to catch the case where the input aabb falls between grid points!
	for (i32 i = 0; i < 3; ++i)
	{
		quantizedAabbMin[i]--;
		quantizedAabbMax[i]++;
	}

	i32 startX = 0;
	i32 endX = m_heightStickWidth - 1;
	i32 startJ = 0;
	i32 endJ = m_heightStickLength - 1;

	switch (m_upAxis)
	{
		case 0:
		{
			if (quantizedAabbMin[1] > startX)
				startX = quantizedAabbMin[1];
			if (quantizedAabbMax[1] < endX)
				endX = quantizedAabbMax[1];
			if (quantizedAabbMin[2] > startJ)
				startJ = quantizedAabbMin[2];
			if (quantizedAabbMax[2] < endJ)
				endJ = quantizedAabbMax[2];
			break;
		}
		case 1:
		{
			if (quantizedAabbMin[0] > startX)
				startX = quantizedAabbMin[0];
			if (quantizedAabbMax[0] < endX)
				endX = quantizedAabbMax[0];
			if (quantizedAabbMin[2] > startJ)
				startJ = quantizedAabbMin[2];
			if (quantizedAabbMax[2] < endJ)
				endJ = quantizedAabbMax[2];
			break;
		};
		case 2:
		{
			if (quantizedAabbMin[0] > startX)
				startX = quantizedAabbMin[0];
			if (quantizedAabbMax[0] < endX)
				endX = quantizedAabbMax[0];
			if (quantizedAabbMin[1] > startJ)
				startJ = quantizedAabbMin[1];
			if (quantizedAabbMax[1] < endJ)
				endJ = quantizedAabbMax[1];
			break;
		}
		default:
		{
			//need to get valid m_upAxis
			Assert(0);
		}
	}

	// TODO If m_vboundsGrid is available, use it to determine if we really need to process this area
	
	const Range aabbUpRange(aabbMin[m_upAxis], aabbMax[m_upAxis]);
	for (i32 j = startJ; j < endJ; j++)
	{
		for (i32 x = startX; x < endX; x++)
		{
			Vec3 vertices[3];
			i32 indices[3] = { 0, 1, 2 };
			if (m_flipTriangleWinding)
			{
				indices[0] = 2;
				indices[2] = 0;
			}

			if (m_flipQuadEdges || (m_useDiamondSubdivision && !((j + x) & 1)) || (m_useZigzagSubdivision && !(j & 1)))
			{
				getVertex(x, j, vertices[indices[0]]);
				getVertex(x, j + 1, vertices[indices[1]]);
				getVertex(x + 1, j + 1, vertices[indices[2]]);

				// Skip triangle processing if the triangle is out-of-AABB.
				Range upRange = minmaxRange(vertices[0][m_upAxis], vertices[1][m_upAxis], vertices[2][m_upAxis]);

				if (upRange.overlaps(aabbUpRange))
					callback->processTriangle(vertices, 2 * x, j);
			
				// already set: getVertex(x, j, vertices[indices[0]])

				// equivalent to: getVertex(x + 1, j + 1, vertices[indices[1]]);
				vertices[indices[1]] = vertices[indices[2]];

				getVertex(x + 1, j, vertices[indices[2]]);
				upRange.min = d3Min(upRange.min, vertices[indices[2]][m_upAxis]);
				upRange.max = d3Max(upRange.max, vertices[indices[2]][m_upAxis]);

				if (upRange.overlaps(aabbUpRange))
					callback->processTriangle(vertices, 2 * x + 1, j);
			}
			else
			{
				getVertex(x, j, vertices[indices[0]]);
				getVertex(x, j + 1, vertices[indices[1]]);
				getVertex(x + 1, j, vertices[indices[2]]);

				// Skip triangle processing if the triangle is out-of-AABB.
				Range upRange = minmaxRange(vertices[0][m_upAxis], vertices[1][m_upAxis], vertices[2][m_upAxis]);

				if (upRange.overlaps(aabbUpRange))
					callback->processTriangle(vertices, 2 * x, j);

				// already set: getVertex(x, j + 1, vertices[indices[1]]);

				// equivalent to: getVertex(x + 1, j, vertices[indices[0]]);
				vertices[indices[0]] = vertices[indices[2]];

				getVertex(x + 1, j + 1, vertices[indices[2]]);
				upRange.min = d3Min(upRange.min, vertices[indices[2]][m_upAxis]);
				upRange.max = d3Max(upRange.max, vertices[indices[2]][m_upAxis]);

				if (upRange.overlaps(aabbUpRange))
					callback->processTriangle(vertices, 2 * x + 1, j);
			}
		}
	}
}

void HeightfieldTerrainShape::calculateLocalInertia(Scalar, Vec3& inertia) const
{
	//moving concave objects not supported

	inertia.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
}

void HeightfieldTerrainShape::setLocalScaling(const Vec3& scaling)
{
	m_localScaling = scaling;
}
const Vec3& HeightfieldTerrainShape::getLocalScaling() const
{
	return m_localScaling;
}

namespace
{
	struct GridRaycastState
	{
		i32 x;  // Next quad coords
		i32 z;
		i32 prev_x;  // Previous quad coords
		i32 prev_z;
		Scalar param;      // Exit param for previous quad
		Scalar prevParam;  // Enter param for previous quad
		Scalar maxDistanceFlat;
		Scalar maxDistance3d;
	};
}

// TODO Does it really need to take 3D vectors?
/// Iterates through a virtual 2D grid of unit-sized square cells,
/// and executes an action on each cell intersecting the given segment, ordered from begin to end.
/// Initially inspired by http://www.cse.yorku.ca/~amana/research/grid.pdf
template <typename Action_T>
void gridRaycast(Action_T& quadAction, const Vec3& beginPos, const Vec3& endPos, i32 indices[3])
{
	GridRaycastState rs;
	rs.maxDistance3d = beginPos.distance(endPos);
	if (rs.maxDistance3d < 0.0001)
	{
		// Consider the ray is too small to hit anything
		return;
	}
	

	Scalar rayDirectionFlatX = endPos[indices[0]] - beginPos[indices[0]];
	Scalar rayDirectionFlatZ = endPos[indices[2]] - beginPos[indices[2]];
	rs.maxDistanceFlat = Sqrt(rayDirectionFlatX * rayDirectionFlatX + rayDirectionFlatZ * rayDirectionFlatZ);

	if (rs.maxDistanceFlat < 0.0001)
	{
		// Consider the ray vertical
		rayDirectionFlatX = 0;
		rayDirectionFlatZ = 0;
	}
	else
	{
		rayDirectionFlatX /= rs.maxDistanceFlat;
		rayDirectionFlatZ /= rs.maxDistanceFlat;
	}

	i32k xiStep = rayDirectionFlatX > 0 ? 1 : rayDirectionFlatX < 0 ? -1 : 0;
	i32k ziStep = rayDirectionFlatZ > 0 ? 1 : rayDirectionFlatZ < 0 ? -1 : 0;

	const float infinite = 9999999;
	const Scalar paramDeltaX = xiStep != 0 ? 1.f / Fabs(rayDirectionFlatX) : infinite;
	const Scalar paramDeltaZ = ziStep != 0 ? 1.f / Fabs(rayDirectionFlatZ) : infinite;

	// pos = param * dir
	Scalar paramCrossX;  // At which value of `param` we will cross a x-axis lane?
	Scalar paramCrossZ;  // At which value of `param` we will cross a z-axis lane?

	// paramCrossX and paramCrossZ are initialized as being the first cross
	// X initialization
	if (xiStep != 0)
	{
		if (xiStep == 1)
		{
			paramCrossX = (ceil(beginPos[indices[0]]) - beginPos[indices[0]]) * paramDeltaX;
		}
		else
		{
			paramCrossX = (beginPos[indices[0]] - floor(beginPos[indices[0]])) * paramDeltaX;
		}
	}
	else
	{
		paramCrossX = infinite;  // Will never cross on X
	}

	// Z initialization
	if (ziStep != 0)
	{
		if (ziStep == 1)
		{
			paramCrossZ = (ceil(beginPos[indices[2]]) - beginPos[indices[2]]) * paramDeltaZ;
		}
		else
		{
			paramCrossZ = (beginPos[indices[2]] - floor(beginPos[indices[2]])) * paramDeltaZ;
		}
	}
	else
	{
		paramCrossZ = infinite;  // Will never cross on Z
	}

	rs.x = static_cast<i32>(floor(beginPos[indices[0]]));
	rs.z = static_cast<i32>(floor(beginPos[indices[2]]));

	// Workaround cases where the ray starts at an integer position
	if (paramCrossX == 0.0)
	{
		paramCrossX += paramDeltaX;
		// If going backwards, we should ignore the position we would get by the above flooring,
		// because the ray is not heading in that direction
		if (xiStep == -1)
		{
			rs.x -= 1;
		}
	}

	if (paramCrossZ == 0.0)
	{
		paramCrossZ += paramDeltaZ;
		if (ziStep == -1)
			rs.z -= 1;
	}

	rs.prev_x = rs.x;
	rs.prev_z = rs.z;
	rs.param = 0;

	while (true)
	{
		rs.prev_x = rs.x;
		rs.prev_z = rs.z;
		rs.prevParam = rs.param;

		if (paramCrossX < paramCrossZ)
		{
			// X lane
			rs.x += xiStep;
			// Assign before advancing the param,
			// to be in sync with the initialization step
			rs.param = paramCrossX;
			paramCrossX += paramDeltaX;
		}
		else
		{
			// Z lane
			rs.z += ziStep;
			rs.param = paramCrossZ;
			paramCrossZ += paramDeltaZ;
		}

		if (rs.param > rs.maxDistanceFlat)
		{
			rs.param = rs.maxDistanceFlat;
			quadAction(rs);
			break;
		}
		else
		{
			quadAction(rs);
		}
	}
}

struct ProcessTrianglesAction
{
	const HeightfieldTerrainShape* shape;
	bool flipQuadEdges;
	bool useDiamondSubdivision;
	i32 width;
	i32 length;
	TriangleCallback* callback;

	void exec(i32 x, i32 z) const
	{
		if (x < 0 || z < 0 || x >= width || z >= length)
		{
			return;
		}

		Vec3 vertices[3];

		// TODO Since this is for raycasts, we could greatly benefit from an early exit on the first hit

		// Check quad
		if (flipQuadEdges || (useDiamondSubdivision && (((z + x) & 1) > 0)))
		{
			// First triangle
			shape->getVertex(x, z, vertices[0]);
			shape->getVertex(x + 1, z, vertices[1]);
			shape->getVertex(x + 1, z + 1, vertices[2]);
			callback->processTriangle(vertices, x, z);

			// Second triangle
			shape->getVertex(x, z, vertices[0]);
			shape->getVertex(x + 1, z + 1, vertices[1]);
			shape->getVertex(x, z + 1, vertices[2]);
			callback->processTriangle(vertices, x, z);
		}
		else
		{
			// First triangle
			shape->getVertex(x, z, vertices[0]);
			shape->getVertex(x, z + 1, vertices[1]);
			shape->getVertex(x + 1, z, vertices[2]);
			callback->processTriangle(vertices, x, z);

			// Second triangle
			shape->getVertex(x + 1, z, vertices[0]);
			shape->getVertex(x, z + 1, vertices[1]);
			shape->getVertex(x + 1, z + 1, vertices[2]);
			callback->processTriangle(vertices, x, z);
		}
	}

	void operator()(const GridRaycastState& bs) const
	{
		exec(bs.prev_x, bs.prev_z);
	}
};

struct ProcessVBoundsAction
{
	const AlignedObjectArray<HeightfieldTerrainShape::Range>& vbounds;
	i32 width;
	i32 length;
	i32 chunkSize;

	Vec3 rayBegin;
	Vec3 rayEnd;
	Vec3 rayDir;

	i32* m_indices;
	ProcessTrianglesAction processTriangles;

	ProcessVBoundsAction(const AlignedObjectArray<HeightfieldTerrainShape::Range>& bnd, i32* indices)
		: vbounds(bnd),
		m_indices(indices)
	{
	}
	void operator()(const GridRaycastState& rs) const
	{
		i32 x = rs.prev_x;
		i32 z = rs.prev_z;

		if (x < 0 || z < 0 || x >= width || z >= length)
		{
			return;
		}

		const HeightfieldTerrainShape::Range chunk = vbounds[x + z * width];

		Vec3 enterPos;
		Vec3 exitPos;

		if (rs.maxDistanceFlat > 0.0001)
		{
			Scalar flatTo3d = chunkSize * rs.maxDistance3d / rs.maxDistanceFlat;
			Scalar enterParam3d = rs.prevParam * flatTo3d;
			Scalar exitParam3d = rs.param * flatTo3d;
			enterPos = rayBegin + rayDir * enterParam3d;
			exitPos = rayBegin + rayDir * exitParam3d;

			// We did enter the flat projection of the AABB,
			// but we have to check if we intersect it on the vertical axis
			if (enterPos[1] > chunk.max && exitPos[m_indices[1]] > chunk.max)
			{
				return;
			}
			if (enterPos[1] < chunk.min && exitPos[m_indices[1]] < chunk.min)
			{
				return;
			}
		}
		else
		{
			// Consider the ray vertical
			// (though we shouldn't reach this often because there is an early check up-front)
			enterPos = rayBegin;
			exitPos = rayEnd;
		}

		gridRaycast(processTriangles, enterPos, exitPos, m_indices);
		// Note: it could be possible to have more than one grid at different levels,
		// to do this there would be a branch using a pointer to another ProcessVBoundsAction
	}
};

// TODO How do I interrupt the ray when there is a hit? `callback` does not return any result
/// Performs a raycast using a hierarchical Bresenham algorithm.
/// Does not allocate any memory by itself.
void HeightfieldTerrainShape::performRaycast(TriangleCallback* callback, const Vec3& raySource, const Vec3& rayTarget) const
{
	// Transform to cell-local
	Vec3 beginPos = raySource / m_localScaling;
	Vec3 endPos = rayTarget / m_localScaling;
	beginPos += m_localOrigin;
	endPos += m_localOrigin;

	ProcessTrianglesAction processTriangles;
	processTriangles.shape = this;
	processTriangles.flipQuadEdges = m_flipQuadEdges;
	processTriangles.useDiamondSubdivision = m_useDiamondSubdivision;
	processTriangles.callback = callback;
	processTriangles.width = m_heightStickWidth - 1;
	processTriangles.length = m_heightStickLength - 1;

	// TODO Transform vectors to account for m_upAxis
	i32 indices[3] = { 0, 1, 2 };
	if (m_upAxis == 2)
	{
		indices[1] = 2;
		indices[2] = 1;
	}
	i32 iBeginX = static_cast<i32>(floor(beginPos[indices[0]]));
	i32 iBeginZ = static_cast<i32>(floor(beginPos[indices[2]]));
	i32 iEndX = static_cast<i32>(floor(endPos[indices[0]]));
	i32 iEndZ = static_cast<i32>(floor(endPos[indices[2]]));

	if (iBeginX == iEndX && iBeginZ == iEndZ)
	{
		// The ray will never cross quads within the plane,
		// so directly process triangles within one quad
		// (typically, vertical rays should end up here)
		processTriangles.exec(iBeginX, iEndZ);
		return;
	}

	

	if (m_vboundsGrid.size()==0)
	{
		// Process all quads intersecting the flat projection of the ray
		gridRaycast(processTriangles, beginPos, endPos, &indices[0]);
	}
	else
	{
		Vec3 rayDiff = endPos - beginPos;
		Scalar flatDistance2 = rayDiff[indices[0]] * rayDiff[indices[0]] + rayDiff[indices[2]] * rayDiff[indices[2]];
		if (flatDistance2 < m_vboundsChunkSize * m_vboundsChunkSize)
		{
			// Don't use chunks, the ray is too short in the plane
			gridRaycast(processTriangles, beginPos, endPos, &indices[0]);
			return;
		}

		ProcessVBoundsAction processVBounds(m_vboundsGrid, &indices[0]);
		processVBounds.width = m_vboundsGridWidth;
		processVBounds.length = m_vboundsGridLength;
		processVBounds.rayBegin = beginPos;
		processVBounds.rayEnd = endPos;
		processVBounds.rayDir = rayDiff.normalized();
		processVBounds.processTriangles = processTriangles;
		processVBounds.chunkSize = m_vboundsChunkSize;
		// The ray is long, run raycast on a higher-level grid
		gridRaycast(processVBounds, beginPos / m_vboundsChunkSize, endPos / m_vboundsChunkSize, indices);
	}
}

/// Builds a grid data structure storing the min and max heights of the terrain in chunks.
/// if chunkSize is zero, that accelerator is removed.
/// If you modify the heights, you need to rebuild this accelerator.
void HeightfieldTerrainShape::buildAccelerator(i32 chunkSize)
{
	if (chunkSize <= 0)
	{
		clearAccelerator();
		return;
	}

	m_vboundsChunkSize = chunkSize;
	i32 nChunksX = m_heightStickWidth / chunkSize;
	i32 nChunksZ = m_heightStickLength / chunkSize;

	if (m_heightStickWidth % chunkSize > 0)
	{
		++nChunksX;  // In case terrain size isn't dividable by chunk size
	}
	if (m_heightStickLength % chunkSize > 0)
	{
		++nChunksZ;
	}

	if (m_vboundsGridWidth != nChunksX || m_vboundsGridLength != nChunksZ)
	{
		clearAccelerator();
		m_vboundsGridWidth = nChunksX;
		m_vboundsGridLength = nChunksZ;
	}

	if (nChunksX == 0 || nChunksZ == 0)
	{
		return;
	}

	// This data structure is only reallocated if the required size changed
	m_vboundsGrid.resize(nChunksX * nChunksZ);
	
	// Compute min and max height for all chunks
	for (i32 cz = 0; cz < nChunksZ; ++cz)
	{
		i32 z0 = cz * chunkSize;

		for (i32 cx = 0; cx < nChunksX; ++cx)
		{
			i32 x0 = cx * chunkSize;

			Range r;

			r.min = getRawHeightFieldValue(x0, z0);
			r.max = r.min;

			// Compute min and max height for this chunk.
			// We have to include one extra cell to account for neighbors.
			// Here is why:
			// Say we have a flat terrain, and a plateau that fits a chunk perfectly.
			//
			//   Left        Right
			// 0---0---0---1---1---1
			// |   |   |   |   |   |
			// 0---0---0---1---1---1
			// |   |   |   |   |   |
			// 0---0---0---1---1---1
			//           x
			//
			// If the AABB for the Left chunk did not share vertices with the Right,
			// then we would fail collision tests at x due to a gap.
			//
			for (i32 z = z0; z < z0 + chunkSize + 1; ++z)
			{
				if (z >= m_heightStickLength)
				{
					continue;
				}

				for (i32 x = x0; x < x0 + chunkSize + 1; ++x)
				{
					if (x >= m_heightStickWidth)
					{
						continue;
					}

					Scalar height = getRawHeightFieldValue(x, z);

					if (height < r.min)
					{
						r.min = height;
					}
					else if (height > r.max)
					{
						r.max = height;
					}
				}
			}

			m_vboundsGrid[cx + cz * nChunksX] = r;
		}
	}
}

void HeightfieldTerrainShape::clearAccelerator()
{
	m_vboundsGrid.clear();
}
