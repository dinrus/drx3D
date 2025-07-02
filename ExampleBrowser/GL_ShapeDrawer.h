#ifndef GL_SHAPE_DRAWER_H
#define GL_SHAPE_DRAWER_H

class CollisionShape;
class ShapeHull;
class DiscreteDynamicsWorld;

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Vec3.h>

#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>

/// OpenGL shape drawing
class GL_ShapeDrawer
{
protected:
	struct ShapeCache
	{
		struct Edge
		{
			Vec3 n[2];
			i32 v[2];
		};
		ShapeCache(ConvexShape* s) : m_shapehull(s) {}
		ShapeHull m_shapehull;
		AlignedObjectArray<Edge> m_edges;
	};
	//clean-up memory of dynamically created shape hulls
	AlignedObjectArray<ShapeCache*> m_shapecaches;
	u32 m_texturehandle;
	bool m_textureenabled;
	bool m_textureinitialized;

	ShapeCache* cache(ConvexShape*);

	virtual void drawSceneInternal(const DiscreteDynamicsWorld* world, i32 pass, i32 cameraUpAxis);

public:
	GL_ShapeDrawer();

	virtual ~GL_ShapeDrawer();

	virtual void drawScene(const DiscreteDynamicsWorld* world, bool useShadows, i32 cameraUpAxis);

	///drawOpenGL might allocate temporary memoty, stores pointer in shape userpointer
	virtual void drawOpenGL(Scalar* m, const CollisionShape* shape, const Vec3& color,
	                    i32 debugMode, const Vec3& worldBoundsMin, const Vec3& worldBoundsMax);
	virtual void drawShadow(Scalar* m, const Vec3& extrusion, const CollisionShape* shape,
	                                    const Vec3& worldBoundsMin, const Vec3& worldBoundsMax);

	bool enableTexture(bool enable)
	{
		bool p = m_textureenabled;
		m_textureenabled = enable;
		return (p);
	}
	bool hasTextureEnabled() const
	{
		return m_textureenabled;
	}

	void drawSphere(Scalar r, i32 lats, i32 longs);
	static void drawCoordSystem();
};

void OGL_displaylist_register_shape(CollisionShape* shape);
void OGL_displaylist_clean();

#endif  //GL_SHAPE_DRAWER_H
