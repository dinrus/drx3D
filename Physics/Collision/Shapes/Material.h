#ifndef DRX3D_MATERIAL_H
#define DRX3D_MATERIAL_H

// Material class to be used by MultimaterialTriangleMeshShape to store triangle properties
class Material
{
	// public members so that materials can change due to world events
public:
	Scalar m_friction;
	Scalar m_restitution;
	i32 pad[2];

	Material() {}
	Material(Scalar fric, Scalar rest)
	{
		m_friction = fric;
		m_restitution = rest;
	}
};

#endif  // DRX3D_MATERIAL_H
