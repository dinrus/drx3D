#ifndef DRX3D_SOFT_BODY_SOLVER_VERTEX_BUFFER_H
#define DRX3D_SOFT_BODY_SOLVER_VERTEX_BUFFER_H

class VertexBufferDescriptor
{
public:
	enum BufferTypes
	{
		CPU_BUFFER,
		DX11_BUFFER,
		OPENGL_BUFFER
	};

protected:
	bool m_hasVertexPositions;
	bool m_hasNormals;

	i32 m_vertexOffset;
	i32 m_vertexStride;

	i32 m_normalOffset;
	i32 m_normalStride;

public:
	VertexBufferDescriptor()
	{
		m_hasVertexPositions = false;
		m_hasNormals = false;
		m_vertexOffset = 0;
		m_vertexStride = 0;
		m_normalOffset = 0;
		m_normalStride = 0;
	}

	virtual ~VertexBufferDescriptor()
	{
	}

	virtual bool hasVertexPositions() const
	{
		return m_hasVertexPositions;
	}

	virtual bool hasNormals() const
	{
		return m_hasNormals;
	}

	/**
	 * Return the type of the vertex buffer descriptor.
	 */
	virtual BufferTypes getBufferType() const = 0;

	/**
	 * Return the vertex offset in floats from the base pointer.
	 */
	virtual i32 getVertexOffset() const
	{
		return m_vertexOffset;
	}

	/**
	 * Return the vertex stride in number of floats between vertices.
	 */
	virtual i32 getVertexStride() const
	{
		return m_vertexStride;
	}

	/**
	 * Return the vertex offset in floats from the base pointer.
	 */
	virtual i32 getNormalOffset() const
	{
		return m_normalOffset;
	}

	/**
	 * Return the vertex stride in number of floats between vertices.
	 */
	virtual i32 getNormalStride() const
	{
		return m_normalStride;
	}
};

class CPUVertexBufferDescriptor : public VertexBufferDescriptor
{
protected:
	float *m_basePointer;

public:
	/**
	 * vertexBasePointer is pointer to beginning of the buffer.
	 * vertexOffset is the offset in floats to the first vertex.
	 * vertexStride is the stride in floats between vertices.
	 */
	CPUVertexBufferDescriptor(float *basePointer, i32 vertexOffset, i32 vertexStride)
	{
		m_basePointer = basePointer;
		m_vertexOffset = vertexOffset;
		m_vertexStride = vertexStride;
		m_hasVertexPositions = true;
	}

	/**
	 * vertexBasePointer is pointer to beginning of the buffer.
	 * vertexOffset is the offset in floats to the first vertex.
	 * vertexStride is the stride in floats between vertices.
	 */
	CPUVertexBufferDescriptor(float *basePointer, i32 vertexOffset, i32 vertexStride, i32 normalOffset, i32 normalStride)
	{
		m_basePointer = basePointer;

		m_vertexOffset = vertexOffset;
		m_vertexStride = vertexStride;
		m_hasVertexPositions = true;

		m_normalOffset = normalOffset;
		m_normalStride = normalStride;
		m_hasNormals = true;
	}

	virtual ~CPUVertexBufferDescriptor()
	{
	}

	/**
	 * Return the type of the vertex buffer descriptor.
	 */
	virtual BufferTypes getBufferType() const
	{
		return CPU_BUFFER;
	}

	/**
	 * Return the base pointer in memory to the first vertex.
	 */
	virtual float *getBasePointer() const
	{
		return m_basePointer;
	}
};

#endif  // #ifndef DRX3D_SOFT_BODY_SOLVER_VERTEX_BUFFER_H
