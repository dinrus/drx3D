// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class AbstractMeshElement
{
protected:
	virtual void GenMesh() = 0;
	i32          GetMeshDataSize() const;

public:
	AbstractMeshElement() :
		m_meshDirty(true),
		m_vertexBuffer(~0u),
		m_indexBuffer(~0u)
	{}

	virtual ~AbstractMeshElement();

	void                          ValidateMesh();

	i32                           GetVertexCount() const  { return m_vertices.size(); }
	i32                           GetIndexCount()  const  { return m_indices.size();  }

	std::vector<SVF_P3F_C4B_T2F>& GetVertices()           { return m_vertices; }
	std::vector<u16>&          GetIndices()            { return m_indices;  }

	buffer_handle_t               GetVertexBuffer() const { return m_vertexBuffer; }
	buffer_handle_t               GetIndexBuffer()  const { return m_indexBuffer;  }

	void                          MarkDirty()             { m_meshDirty = true; }

protected:
	std::vector<SVF_P3F_C4B_T2F> m_vertices;
	std::vector<u16>          m_indices;
	bool                         m_meshDirty;

	buffer_handle_t              m_vertexBuffer;
	buffer_handle_t              m_indexBuffer;
};
