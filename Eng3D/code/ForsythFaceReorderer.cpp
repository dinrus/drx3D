// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>

#if DRX_PLATFORM_WINDOWS

	#include <drx3D/Eng3D/ForsythFaceReorderer.h>

	#include <cmath>    // powf()

ForsythFaceReorderer::ForsythFaceReorderer()
	: m_cacheSize(0)
	, m_cacheUsedSize(0)
{
	ZeroArray(m_cache);
	ZeroArray(m_scoreTable_valency);
	ZeroArray(m_scoreTable_cachePosition);

	computeValencyScoreTable();
}

bool ForsythFaceReorderer::reorderFaces(
  const size_t cacheSize,
  const uint verticesPerFace,
  const size_t indexCount,
  u32k* const inVertexIndices,
  u32* const outVertexIndices,
  u32* const outFaceToOldFace)
{
	clear();

	if (verticesPerFace < sk_minVerticesPerFace || verticesPerFace > sk_maxVerticesPerFace)
	{
		return false;
	}
	if (indexCount <= 0)
	{
		return true;
	}
	if (indexCount % verticesPerFace != 0)
	{
		return false;
	}
	if (inVertexIndices == 0)
	{
		return false;
	}
	if (outVertexIndices == 0)
	{
		return false;
	}

	if ((cacheSize < verticesPerFace) || (cacheSize > sk_maxCacheSize))
	{
		return false;
	}

	m_cacheSize = (i32)cacheSize;
	m_cacheUsedSize = 0;
	computeCacheScoreTable(verticesPerFace);

	u32k faceCount = indexCount / verticesPerFace;
	if (indexCount / verticesPerFace >= (u32) - 1)
	{
		// Face count is too high
		return false;
	}

	size_t writtenFaceCount = 0;

	u32 vertexCount;
	{
		// TODO: use minVertexIndex also. It will allow to use less memory for ranged indices.
		// For example indices in ranges [800;899] will use memory size 100, not 900.
		u32 maxVertexIndex = 0;
		for (size_t i = 0; i < indexCount; ++i)
		{
			if (inVertexIndices[i] > maxVertexIndex)
			{
				maxVertexIndex = inVertexIndices[i];
			}
		}
		if (((size_t)maxVertexIndex) + 1 >= (u32) - 1)
		{
			// Vertex count is too high
			return false;
		}
		vertexCount = maxVertexIndex + 1;
	}

	// Allocate and initialize arrays
	{
		{
			Vertex initVertex;
			initVertex.m_pFaceList = 0;
			initVertex.m_aliveFaceCount = 0;
			initVertex.m_posInCache = -1;
			initVertex.m_score = 0;
			m_vertices.resize(vertexCount, initVertex);
		}

		m_deadFacesBitArray.resize((faceCount + 7) / 8, 0);

		m_faceScores.resize(faceCount, 0);

		m_vertexFaceLists.resize(faceCount * verticesPerFace);
	}

	// Fill per-vertex face lists
	{
		for (size_t i = 0; i < indexCount; ++i)
		{
			u32k vertexIndex = inVertexIndices[i];
			if (m_vertices[vertexIndex].m_aliveFaceCount >= sk_maxValency)
			{
				// Vertex valency is too high
				return false;
			}
			++m_vertices[vertexIndex].m_aliveFaceCount;
		}

		u32 pos = 0;
		for (u32 vi = 0; vi < vertexCount; ++vi)
		{
			Vertex& v = m_vertices[vi];
			v.m_pFaceList = &m_vertexFaceLists[pos];
			pos += v.m_aliveFaceCount;
			v.m_aliveFaceCount = 0;
		}
		assert(pos == faceCount * verticesPerFace);

		u32k* pVertexIndex = &inVertexIndices[0];
		for (u32 fi = 0; fi < faceCount; ++fi, pVertexIndex += verticesPerFace)
		{
			for (uint j = 0; j < verticesPerFace; ++j)
			{
				Vertex& v = m_vertices[pVertexIndex[j]];
				v.m_pFaceList[v.m_aliveFaceCount++] = fi;
			}
		}
	}

	// Compute vertex and face scores
	{
		for (u32 vi = 0; vi < vertexCount; ++vi)
		{
			computeVertexScore(m_vertices[vi]);
		}

		u32k* pVertexIndex = &inVertexIndices[0];
		for (u32 fi = 0; fi < faceCount; ++fi, pVertexIndex += verticesPerFace)
		{
			m_faceScores[fi] = 0;
			for (uint j = 0; j < verticesPerFace; ++j)
			{
				const Vertex& v = m_vertices[pVertexIndex[j]];
				m_faceScores[fi] += v.m_score;
			}
		}
	}

	// Add faces with highest scores to the output buffer, one by one.
	u32 faceSearchCursor = 0;
	u32 bestFaceToAdd;
	for (;; )
	{
		// Find face with highest score
		{
			bestFaceToAdd = (u32) - 1;
			float highestScore = -1;
			for (i32 i = 0; i < m_cacheUsedSize; ++i)
			{
				const Vertex& v = m_vertices[m_cache[i]];
				u32k* const pFaces = v.m_pFaceList;
				for (valency_type j = 0; j < v.m_aliveFaceCount; ++j)
				{
					u32k faceIndex = pFaces[j];
					if (highestScore < m_faceScores[faceIndex])
					{
						highestScore = m_faceScores[faceIndex];
						bestFaceToAdd = faceIndex;
					}
				}
			}

			if (bestFaceToAdd == (u32) - 1)
			{
				bestFaceToAdd = findBestFaceToAdd(faceSearchCursor);
				assert(bestFaceToAdd != (u32) - 1);
			}
		}

		// Add the best face to the output buffer
		{
			size_t writtenIndexCount = writtenFaceCount * verticesPerFace;
			for (uint j = 0; j < verticesPerFace; ++j)
			{
				outVertexIndices[writtenIndexCount + j] = inVertexIndices[(size_t)bestFaceToAdd * verticesPerFace + j];
			}
			if (outFaceToOldFace)
			{
				outFaceToOldFace[writtenFaceCount] = bestFaceToAdd;
			}
			if (++writtenFaceCount == faceCount)
			{
				// We're done.
				return true;
			}
		}

		// Make changes to the cache, vertex & cache scores, vertex face lists
		{
			m_deadFacesBitArray[bestFaceToAdd >> 3] |= 1 << (bestFaceToAdd & 7);

			for (i32 j = verticesPerFace - 1; j >= 0; --j)
			{
				u32k vertexIndex = inVertexIndices[(size_t)bestFaceToAdd * verticesPerFace + j];
				moveVertexToCacheTop(vertexIndex);
				removeFaceFromVertex(vertexIndex, bestFaceToAdd);
			}

			for (i32 i = 0; i < m_cacheUsedSize; ++i)
			{
				Vertex& v = m_vertices[m_cache[i]];
				if (i >= m_cacheSize)
				{
					v.m_posInCache = -1;
				}
				const float oldScore = v.m_score;
				computeVertexScore(v);
				const float differenceScore = v.m_score - oldScore;
				u32k* const pFaces = v.m_pFaceList;
				for (valency_type j = 0; j < v.m_aliveFaceCount; ++j)
				{
					m_faceScores[pFaces[j]] += differenceScore;
				}
			}
			if (m_cacheUsedSize > m_cacheSize)
			{
				m_cacheUsedSize = m_cacheSize;
			}
		}
	}
}

void ForsythFaceReorderer::clear()
{
	m_vertices.clear();
	m_deadFacesBitArray.clear();
	m_faceScores.clear();
	m_vertexFaceLists.clear();
}

void ForsythFaceReorderer::computeCacheScoreTable(i32k verticesPerFace)
{
	static const float lastFaceScore = 0.75f;
	static const float cacheDecayPower = 1.5f;

	// Vertices of last added face should have *same* fixed score,
	// because otherwise results will depend on the order of vertices
	// in face (5,6,7 and 7,5,6 will produce different results).
	for (i32 j = 0; j < verticesPerFace; ++j)
	{
		m_scoreTable_cachePosition[j] = lastFaceScore;
	}

	for (i32 i = verticesPerFace; i < m_cacheSize; ++i)
	{
		const float x = 1.0f - ((i - verticesPerFace) / (m_cacheSize - verticesPerFace));
		m_scoreTable_cachePosition[i] = powf(x, cacheDecayPower);
	}
}

void ForsythFaceReorderer::computeValencyScoreTable()
{
	// Lower number of alive faces in the vertex produces higher score.
	// It allows to  get rid of lone vertices quickly.

	static const float valencyPower = -0.5f;
	static const float valencyScale = 2.0f;

	m_scoreTable_valency[0] = 0;
	for (valency_type i = 1; i < sk_valencyTableSize; ++i)
	{
		m_scoreTable_valency[i] = valencyScale * powf(i, valencyPower);
	}
}

void ForsythFaceReorderer::computeVertexScore(ForsythFaceReorderer::Vertex& v)
{
	if (v.m_aliveFaceCount > 0)
	{
		assert(v.m_posInCache < m_cacheSize);
		const float valencyScore = (v.m_aliveFaceCount < sk_valencyTableSize) ? m_scoreTable_valency[v.m_aliveFaceCount] : 0;
		// Preventing "SCA: warning C6385: Invalid data: accessing 'm_scoreTable_cachePosition', the readable size is '200' bytes, but '484' bytes might be read"
		PREFAST_SUPPRESS_WARNING(6385) const float cacheScore = (v.m_posInCache >= 0) ? m_scoreTable_cachePosition[v.m_posInCache] : 0;
		v.m_score = valencyScore + cacheScore;
	}
}

void ForsythFaceReorderer::moveVertexToCacheTop(u32k vertexIndex)
{
	i32k oldPosInCache = m_vertices[vertexIndex].m_posInCache;
	for (i32 dst = (oldPosInCache >= 0) ? oldPosInCache : m_cacheUsedSize; dst > 0; --dst)
	{
		u32k v = m_cache[dst - 1];
		m_cache[dst] = v;
		++m_vertices[v].m_posInCache;
	}
	m_cache[0] = vertexIndex;
	m_vertices[vertexIndex].m_posInCache = 0;
	if (oldPosInCache < 0)
	{
		++m_cacheUsedSize;
	}
}

void ForsythFaceReorderer::removeFaceFromVertex(u32k vertexIndex, u32k faceIndex)
{
	Vertex& v = m_vertices[vertexIndex];
	assert(v.m_aliveFaceCount > 0);
	u32* const pFaces = v.m_pFaceList;
	for (i32 j = 0;; ++j)
	{
		if (pFaces[j] == faceIndex)
		{
			pFaces[j] = pFaces[--v.m_aliveFaceCount];
			return;
		}
	}
}

u32 ForsythFaceReorderer::findBestFaceToAdd(u32& faceSearchCursor) const
{
	assert(!m_faceScores.empty());
	assert(faceSearchCursor < m_faceScores.size());
	while (m_deadFacesBitArray[faceSearchCursor >> 3] & (1 << (faceSearchCursor & 7)))
	{
		++faceSearchCursor;
	}
	return faceSearchCursor++;
}

#endif

// eof
