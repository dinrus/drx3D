#ifndef D3_CONFIG_H
#define D3_CONFIG_H

struct b3Config
{
	i32 m_maxConvexBodies;
	i32 m_maxConvexShapes;
	i32 m_maxBroadphasePairs;
	i32 m_maxContactCapacity;
	i32 m_compoundPairCapacity;

	i32 m_maxVerticesPerFace;
	i32 m_maxFacesPerShape;
	i32 m_maxConvexVertices;
	i32 m_maxConvexIndices;
	i32 m_maxConvexUniqueEdges;

	i32 m_maxCompoundChildShapes;

	i32 m_maxTriConvexPairCapacity;

	b3Config()
		: m_maxConvexBodies(128 * 1024),
		  m_maxVerticesPerFace(64),
		  m_maxFacesPerShape(12),
		  m_maxConvexVertices(8192),
		  m_maxConvexIndices(81920),
		  m_maxConvexUniqueEdges(8192),
		  m_maxCompoundChildShapes(8192),
		  m_maxTriConvexPairCapacity(256 * 1024)
	{
		m_maxConvexShapes = m_maxConvexBodies;
		m_maxBroadphasePairs = 16 * m_maxConvexBodies;
		m_maxContactCapacity = m_maxBroadphasePairs;
		m_compoundPairCapacity = 1024 * 1024;
	}
};

#endif  //D3_CONFIG_H
