#ifndef GIMPACT_SHAPE_H
#define GIMPACT_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Shapes/StridingMeshInterface.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>
#include <drx3D/Physics/Collision/Shapes/TetrahedronShape.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <drx3D/Physics/Collision/Gimpact/GImpactQuantizedBvh.h>  // box tree class

//! declare Quantized trees, (you can change to float based trees)
typedef GImpactQuantizedBvh GImpactBoxSet;

enum eGIMPACT_SHAPE_TYPE
{
	CONST_GIMPACT_COMPOUND_SHAPE = 0,
	CONST_GIMPACT_TRIMESH_SHAPE_PART,
	CONST_GIMPACT_TRIMESH_SHAPE
};

//! Helper class for tetrahedrons
class TetrahedronShapeEx : public BU_Simplex1to4
{
public:
	TetrahedronShapeEx()
	{
		m_numVertices = 4;
	}

	SIMD_FORCE_INLINE void setVertices(
		const Vec3& v0, const Vec3& v1,
		const Vec3& v2, const Vec3& v3)
	{
		m_vertices[0] = v0;
		m_vertices[1] = v1;
		m_vertices[2] = v2;
		m_vertices[3] = v3;
		recalcLocalAabb();
	}
};

//! Base class for gimpact shapes
class GImpactShapeInterface : public ConcaveShape
{
protected:
	AABB m_localAABB;
	bool m_needs_update;
	Vec3 localScaling;
	GImpactBoxSet m_box_set;  // optionally boxset

	//! use this function for perfofm refit in bounding boxes
	//! use this function for perfofm refit in bounding boxes
	virtual void calcLocalAABB()
	{
		lockChildShapes();
		if (m_box_set.getNodeCount() == 0)
		{
			m_box_set.buildSet();
		}
		else
		{
			m_box_set.update();
		}
		unlockChildShapes();

		m_localAABB = m_box_set.getGlobalBox();
	}

public:
	GImpactShapeInterface()
	{
		m_shapeType = GIMPACT_SHAPE_PROXYTYPE;
		m_localAABB.invalidate();
		m_needs_update = true;
		localScaling.setVal(1.f, 1.f, 1.f);
	}

	//! performs refit operation
	/*!
	Updates the entire Box set of this shape.
	\pre postUpdate() must be called for attemps to calculating the box set, else this function
		will does nothing.
	\post if m_needs_update == true, then it calls calcLocalAABB();
	*/
	SIMD_FORCE_INLINE void updateBound()
	{
		if (!m_needs_update) return;
		calcLocalAABB();
		m_needs_update = false;
	}

	//! If the Bounding box is not updated, then this class attemps to calculate it.
	/*!
    \post Calls updateBound() for update the box set.
    */
	void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		AABB transformedbox = m_localAABB;
		transformedbox.appy_transform(t);
		aabbMin = transformedbox.m_min;
		aabbMax = transformedbox.m_max;
	}

	//! Tells to this object that is needed to refit the box set
	virtual void postUpdate()
	{
		m_needs_update = true;
	}

	//! Obtains the local box, which is the global calculated box of the total of subshapes
	SIMD_FORCE_INLINE const AABB& getLocalBox()
	{
		return m_localAABB;
	}

	virtual i32 getShapeType() const
	{
		return GIMPACT_SHAPE_PROXYTYPE;
	}

	/*!
	\post You must call updateBound() for update the box set.
	*/
	virtual void setLocalScaling(const Vec3& scaling)
	{
		localScaling = scaling;
		postUpdate();
	}

	virtual const Vec3& getLocalScaling() const
	{
		return localScaling;
	}

	virtual void setMargin(Scalar margin)
	{
		m_collisionMargin = margin;
		i32 i = getNumChildShapes();
		while (i--)
		{
			CollisionShape* child = getChildShape(i);
			child->setMargin(margin);
		}

		m_needs_update = true;
	}

	//! Subshape member functions
	//!@{

	//! Base method for determinig which kind of GIMPACT shape we get
	virtual eGIMPACT_SHAPE_TYPE getGImpactShapeType() const = 0;

	//! gets boxset
	SIMD_FORCE_INLINE const GImpactBoxSet* getBoxSet() const
	{
		return &m_box_set;
	}

	//! Determines if this class has a hierarchy structure for sorting its primitives
	SIMD_FORCE_INLINE bool hasBoxSet() const
	{
		if (m_box_set.getNodeCount() == 0) return false;
		return true;
	}

	//! Obtains the primitive manager
	virtual const PrimitiveManagerBase* getPrimitiveManager() const = 0;

	//! Gets the number of children
	virtual i32 getNumChildShapes() const = 0;

	//! if true, then its children must get transforms.
	virtual bool childrenHasTransform2() const = 0;

	//! Determines if this shape has triangles
	virtual bool needsRetrieveTriangles() const = 0;

	//! Determines if this shape has tetrahedrons
	virtual bool needsRetrieveTetrahedrons() const = 0;

	virtual void getBulletTriangle(i32 prim_index, TriangleShapeEx& triangle) const = 0;

	virtual void getBulletTetrahedron(i32 prim_index, TetrahedronShapeEx& tetrahedron) const = 0;

	//! call when reading child shapes
	virtual void lockChildShapes() const
	{
	}

	virtual void unlockChildShapes() const
	{
	}

	//! if this trimesh
	SIMD_FORCE_INLINE void getPrimitiveTriangle(i32 index, PrimitiveTriangle& triangle) const
	{
		getPrimitiveManager()->get_primitive_triangle(index, triangle);
	}

	//! Retrieves the bound from a child
	/*!
    */
	virtual void getChildAabb(i32 child_index, const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		AABB child_aabb;
		getPrimitiveManager()->get_primitive_box(child_index, child_aabb);
		child_aabb.appy_transform(t);
		aabbMin = child_aabb.m_min;
		aabbMax = child_aabb.m_max;
	}

	//! Gets the children
	virtual CollisionShape* getChildShape(i32 index) = 0;

	//! Gets the child
	virtual const CollisionShape* getChildShape(i32 index) const = 0;

	//! Gets the children transform
	virtual Transform2 getChildTransform(i32 index) const = 0;

	//! Sets the children transform
	/*!
	\post You must call updateBound() for update the box set.
	*/
	virtual void setChildTransform2(i32 index, const Transform2& transform) = 0;

	//!@}

	//! virtual method for ray collision
	virtual void rayTest(const Vec3& rayFrom, const Vec3& rayTo, CollisionWorld::RayResultCallback& resultCallback) const
	{
		(void)rayFrom;
		(void)rayTo;
		(void)resultCallback;
	}

	//! Function for retrieve triangles.
	/*!
	It gives the triangles in local space
	*/
	virtual void processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
	{
		(void)callback;
		(void)aabbMin;
		(void)aabbMax;
	}

	//! Function for retrieve triangles.
	/*!
	It gives the triangles in local space
	*/
	virtual void processAllTrianglesRay(TriangleCallback* /*callback*/, const Vec3& /*rayFrom*/, const Vec3& /*rayTo*/) const
	{
	}

	//!@}
};

//! GImpactCompoundShape allows to handle multiple CollisionShape objects at once
/*!
This class only can manage Convex subshapes
*/
class GImpactCompoundShape : public GImpactShapeInterface
{
public:
	//! compound primitive manager
	class CompoundPrimitiveManager : public PrimitiveManagerBase
	{
	public:
		virtual ~CompoundPrimitiveManager() {}
		GImpactCompoundShape* m_compoundShape;

		CompoundPrimitiveManager(const CompoundPrimitiveManager& compound)
			: PrimitiveManagerBase()
		{
			m_compoundShape = compound.m_compoundShape;
		}

		CompoundPrimitiveManager(GImpactCompoundShape* compoundShape)
		{
			m_compoundShape = compoundShape;
		}

		CompoundPrimitiveManager()
		{
			m_compoundShape = NULL;
		}

		virtual bool is_trimesh() const
		{
			return false;
		}

		virtual i32 get_primitive_count() const
		{
			return (i32)m_compoundShape->getNumChildShapes();
		}

		virtual void get_primitive_box(i32 prim_index, AABB& primbox) const
		{
			Transform2 prim_trans;
			if (m_compoundShape->childrenHasTransform2())
			{
				prim_trans = m_compoundShape->getChildTransform(prim_index);
			}
			else
			{
				prim_trans.setIdentity();
			}
			const CollisionShape* shape = m_compoundShape->getChildShape(prim_index);
			shape->getAabb(prim_trans, primbox.m_min, primbox.m_max);
		}

		virtual void get_primitive_triangle(i32 prim_index, PrimitiveTriangle& triangle) const
		{
			Assert(0);
			(void)prim_index;
			(void)triangle;
		}
	};

protected:
	CompoundPrimitiveManager m_primitive_manager;
	AlignedObjectArray<Transform2> m_childTransforms;
	AlignedObjectArray<CollisionShape*> m_childShapes;

public:
	GImpactCompoundShape(bool children_has_transform = true)
	{
		(void)children_has_transform;
		m_primitive_manager.m_compoundShape = this;
		m_box_set.setPrimitiveManager(&m_primitive_manager);
	}

	virtual ~GImpactCompoundShape()
	{
	}

	//! if true, then its children must get transforms.
	virtual bool childrenHasTransform2() const
	{
		if (m_childTransforms.size() == 0) return false;
		return true;
	}

	//! Obtains the primitive manager
	virtual const PrimitiveManagerBase* getPrimitiveManager() const
	{
		return &m_primitive_manager;
	}

	//! Obtains the compopund primitive manager
	SIMD_FORCE_INLINE CompoundPrimitiveManager* getCompoundPrimitiveManager()
	{
		return &m_primitive_manager;
	}

	//! Gets the number of children
	virtual i32 getNumChildShapes() const
	{
		return m_childShapes.size();
	}

	//! Use this method for adding children. Only Convex shapes are allowed.
	void addChildShape(const Transform2& localTransform2, CollisionShape* shape)
	{
		Assert(shape->isConvex());
		m_childTransforms.push_back(localTransform2);
		m_childShapes.push_back(shape);
	}

	//! Use this method for adding children. Only Convex shapes are allowed.
	void addChildShape(CollisionShape* shape)
	{
		Assert(shape->isConvex());
		m_childShapes.push_back(shape);
	}

	//! Gets the children
	virtual CollisionShape* getChildShape(i32 index)
	{
		return m_childShapes[index];
	}

	//! Gets the children
	virtual const CollisionShape* getChildShape(i32 index) const
	{
		return m_childShapes[index];
	}

	//! Retrieves the bound from a child
	/*!
    */
	virtual void getChildAabb(i32 child_index, const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		if (childrenHasTransform2())
		{
			m_childShapes[child_index]->getAabb(t * m_childTransforms[child_index], aabbMin, aabbMax);
		}
		else
		{
			m_childShapes[child_index]->getAabb(t, aabbMin, aabbMax);
		}
	}

	//! Gets the children transform
	virtual Transform2 getChildTransform(i32 index) const
	{
		Assert(m_childTransforms.size() == m_childShapes.size());
		return m_childTransforms[index];
	}

	//! Sets the children transform
	/*!
	\post You must call updateBound() for update the box set.
	*/
	virtual void setChildTransform2(i32 index, const Transform2& transform)
	{
		Assert(m_childTransforms.size() == m_childShapes.size());
		m_childTransforms[index] = transform;
		postUpdate();
	}

	//! Determines if this shape has triangles
	virtual bool needsRetrieveTriangles() const
	{
		return false;
	}

	//! Determines if this shape has tetrahedrons
	virtual bool needsRetrieveTetrahedrons() const
	{
		return false;
	}

	virtual void getBulletTriangle(i32 prim_index, TriangleShapeEx& triangle) const
	{
		(void)prim_index;
		(void)triangle;
		Assert(0);
	}

	virtual void getBulletTetrahedron(i32 prim_index, TetrahedronShapeEx& tetrahedron) const
	{
		(void)prim_index;
		(void)tetrahedron;
		Assert(0);
	}

	//! Calculates the exact inertia tensor for this shape
	virtual void calculateLocalInertia(Scalar mass, Vec3& inertia) const;

	virtual tukk getName() const
	{
		return "GImpactCompound";
	}

	virtual eGIMPACT_SHAPE_TYPE getGImpactShapeType() const
	{
		return CONST_GIMPACT_COMPOUND_SHAPE;
	}
};

//! This class manages a sub part of a mesh supplied by the StridingMeshInterface interface.
/*!
- Simply create this shape by passing the StridingMeshInterface to the constructor GImpactMeshShapePart, then you must call updateBound() after creating the mesh
- When making operations with this shape, you must call <b>lock</b> before accessing to the trimesh primitives, and then call <b>unlock</b>
- You can handle deformable meshes with this shape, by calling postUpdate() every time when changing the mesh vertices.

*/
class GImpactMeshShapePart : public GImpactShapeInterface
{
public:
	//! Trimesh primitive manager
	/*!
	Manages the info from StridingMeshInterface object and controls the Lock/Unlock mechanism
	*/
	class TrimeshPrimitiveManager : public PrimitiveManagerBase
	{
	public:
		Scalar m_margin;
		StridingMeshInterface* m_meshInterface;
		Vec3 m_scale;
		i32 m_part;
		i32 m_lock_count;
		u8k* vertexbase;
		i32 numverts;
		PHY_ScalarType type;
		i32 stride;
		u8k* indexbase;
		i32 indexstride;
		i32 numfaces;
		PHY_ScalarType indicestype;

		TrimeshPrimitiveManager()
		{
			m_meshInterface = NULL;
			m_part = 0;
			m_margin = 0.01f;
			m_scale = Vec3(1.f, 1.f, 1.f);
			m_lock_count = 0;
			vertexbase = 0;
			numverts = 0;
			stride = 0;
			indexbase = 0;
			indexstride = 0;
			numfaces = 0;
		}

		TrimeshPrimitiveManager(const TrimeshPrimitiveManager& manager)
			: PrimitiveManagerBase()
		{
			m_meshInterface = manager.m_meshInterface;
			m_part = manager.m_part;
			m_margin = manager.m_margin;
			m_scale = manager.m_scale;
			m_lock_count = 0;
			vertexbase = 0;
			numverts = 0;
			stride = 0;
			indexbase = 0;
			indexstride = 0;
			numfaces = 0;
		}

		TrimeshPrimitiveManager(
			StridingMeshInterface* meshInterface, i32 part)
		{
			m_meshInterface = meshInterface;
			m_part = part;
			m_scale = m_meshInterface->getScaling();
			m_margin = 0.1f;
			m_lock_count = 0;
			vertexbase = 0;
			numverts = 0;
			stride = 0;
			indexbase = 0;
			indexstride = 0;
			numfaces = 0;
		}

		virtual ~TrimeshPrimitiveManager() {}

		void lock()
		{
			if (m_lock_count > 0)
			{
				m_lock_count++;
				return;
			}
			m_meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexbase, numverts,
				type, stride, &indexbase, indexstride, numfaces, indicestype, m_part);

			m_lock_count = 1;
		}

		void unlock()
		{
			if (m_lock_count == 0) return;
			if (m_lock_count > 1)
			{
				--m_lock_count;
				return;
			}
			m_meshInterface->unLockReadOnlyVertexBase(m_part);
			vertexbase = NULL;
			m_lock_count = 0;
		}

		virtual bool is_trimesh() const
		{
			return true;
		}

		virtual i32 get_primitive_count() const
		{
			return (i32)numfaces;
		}

		SIMD_FORCE_INLINE i32 get_vertex_count() const
		{
			return (i32)numverts;
		}

		SIMD_FORCE_INLINE void get_indices(i32 face_index, u32& i0, u32& i1, u32& i2) const
		{
			if (indicestype == PHY_SHORT)
			{
				unsigned short* s_indices = (unsigned short*)(indexbase + face_index * indexstride);
				i0 = s_indices[0];
				i1 = s_indices[1];
				i2 = s_indices[2];
			}
			else if (indicestype == PHY_INTEGER)
			{
				u32* i_indices = (u32*)(indexbase + face_index * indexstride);
				i0 = i_indices[0];
				i1 = i_indices[1];
				i2 = i_indices[2];
			}
			else
			{
				Assert(indicestype == PHY_UCHAR);
				u8* i_indices = (u8*)(indexbase + face_index * indexstride);
				i0 = i_indices[0];
				i1 = i_indices[1];
				i2 = i_indices[2];
			}
		}

		SIMD_FORCE_INLINE void get_vertex(u32 vertex_index, Vec3& vertex) const
		{
			if (type == PHY_DOUBLE)
			{
				double* dvertices = (double*)(vertexbase + vertex_index * stride);
				vertex[0] = Scalar(dvertices[0] * m_scale[0]);
				vertex[1] = Scalar(dvertices[1] * m_scale[1]);
				vertex[2] = Scalar(dvertices[2] * m_scale[2]);
			}
			else
			{
				float* svertices = (float*)(vertexbase + vertex_index * stride);
				vertex[0] = svertices[0] * m_scale[0];
				vertex[1] = svertices[1] * m_scale[1];
				vertex[2] = svertices[2] * m_scale[2];
			}
		}

		virtual void get_primitive_box(i32 prim_index, AABB& primbox) const
		{
			PrimitiveTriangle triangle;
			get_primitive_triangle(prim_index, triangle);
			primbox.calc_from_triangle_margin(
				triangle.m_vertices[0],
				triangle.m_vertices[1], triangle.m_vertices[2], triangle.m_margin);
		}

		virtual void get_primitive_triangle(i32 prim_index, PrimitiveTriangle& triangle) const
		{
			u32 indices[3];
			get_indices(prim_index, indices[0], indices[1], indices[2]);
			get_vertex(indices[0], triangle.m_vertices[0]);
			get_vertex(indices[1], triangle.m_vertices[1]);
			get_vertex(indices[2], triangle.m_vertices[2]);
			triangle.m_margin = m_margin;
		}

		SIMD_FORCE_INLINE void get_bullet_triangle(i32 prim_index, TriangleShapeEx& triangle) const
		{
			u32 indices[3];
			get_indices(prim_index, indices[0], indices[1], indices[2]);
			get_vertex(indices[0], triangle.m_vertices1[0]);
			get_vertex(indices[1], triangle.m_vertices1[1]);
			get_vertex(indices[2], triangle.m_vertices1[2]);
			triangle.setMargin(m_margin);
		}
	};

protected:
	TrimeshPrimitiveManager m_primitive_manager;

public:
	GImpactMeshShapePart()
	{
		m_box_set.setPrimitiveManager(&m_primitive_manager);
	}

	GImpactMeshShapePart(StridingMeshInterface* meshInterface, i32 part);
	virtual ~GImpactMeshShapePart();

	//! if true, then its children must get transforms.
	virtual bool childrenHasTransform2() const
	{
		return false;
	}

	//! call when reading child shapes
	virtual void lockChildShapes() const;
	virtual void unlockChildShapes() const;

	//! Gets the number of children
	virtual i32 getNumChildShapes() const
	{
		return m_primitive_manager.get_primitive_count();
	}

	//! Gets the children
	virtual CollisionShape* getChildShape(i32 index)
	{
		(void)index;
		Assert(0);
		return NULL;
	}

	//! Gets the child
	virtual const CollisionShape* getChildShape(i32 index) const
	{
		(void)index;
		Assert(0);
		return NULL;
	}

	//! Gets the children transform
	virtual Transform2 getChildTransform(i32 index) const
	{
		(void)index;
		Assert(0);
		return Transform2();
	}

	//! Sets the children transform
	/*!
	\post You must call updateBound() for update the box set.
	*/
	virtual void setChildTransform2(i32 index, const Transform2& transform)
	{
		(void)index;
		(void)transform;
		Assert(0);
	}

	//! Obtains the primitive manager
	virtual const PrimitiveManagerBase* getPrimitiveManager() const
	{
		return &m_primitive_manager;
	}

	SIMD_FORCE_INLINE TrimeshPrimitiveManager* getTrimeshPrimitiveManager()
	{
		return &m_primitive_manager;
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3& inertia) const;

	virtual tukk getName() const
	{
		return "GImpactMeshShapePart";
	}

	virtual eGIMPACT_SHAPE_TYPE getGImpactShapeType() const
	{
		return CONST_GIMPACT_TRIMESH_SHAPE_PART;
	}

	//! Determines if this shape has triangles
	virtual bool needsRetrieveTriangles() const
	{
		return true;
	}

	//! Determines if this shape has tetrahedrons
	virtual bool needsRetrieveTetrahedrons() const
	{
		return false;
	}

	virtual void getBulletTriangle(i32 prim_index, TriangleShapeEx& triangle) const
	{
		m_primitive_manager.get_bullet_triangle(prim_index, triangle);
	}

	virtual void getBulletTetrahedron(i32 prim_index, TetrahedronShapeEx& tetrahedron) const
	{
		(void)prim_index;
		(void)tetrahedron;
		Assert(0);
	}

	SIMD_FORCE_INLINE i32 getVertexCount() const
	{
		return m_primitive_manager.get_vertex_count();
	}

	SIMD_FORCE_INLINE void getVertex(i32 vertex_index, Vec3& vertex) const
	{
		m_primitive_manager.get_vertex(vertex_index, vertex);
	}

	SIMD_FORCE_INLINE void setMargin(Scalar margin)
	{
		m_primitive_manager.m_margin = margin;
		postUpdate();
	}

	SIMD_FORCE_INLINE Scalar getMargin() const
	{
		return m_primitive_manager.m_margin;
	}

	virtual void setLocalScaling(const Vec3& scaling)
	{
		m_primitive_manager.m_scale = scaling;
		postUpdate();
	}

	virtual const Vec3& getLocalScaling() const
	{
		return m_primitive_manager.m_scale;
	}

	SIMD_FORCE_INLINE i32 getPart() const
	{
		return (i32)m_primitive_manager.m_part;
	}

	virtual void processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const;
	virtual void processAllTrianglesRay(TriangleCallback* callback, const Vec3& rayFrom, const Vec3& rayTo) const;
};

//! This class manages a mesh supplied by the StridingMeshInterface interface.
/*!
Set of GImpactMeshShapePart parts
- Simply create this shape by passing the StridingMeshInterface to the constructor GImpactMeshShape, then you must call updateBound() after creating the mesh

- You can handle deformable meshes with this shape, by calling postUpdate() every time when changing the mesh vertices.

*/
class GImpactMeshShape : public GImpactShapeInterface
{
	StridingMeshInterface* m_meshInterface;

protected:
	AlignedObjectArray<GImpactMeshShapePart*> m_mesh_parts;
	void buildMeshParts(StridingMeshInterface* meshInterface)
	{
		for (i32 i = 0; i < meshInterface->getNumSubParts(); ++i)
		{
			GImpactMeshShapePart* newpart = new GImpactMeshShapePart(meshInterface, i);
			m_mesh_parts.push_back(newpart);
		}
	}

	//! use this function for perfofm refit in bounding boxes
	virtual void calcLocalAABB()
	{
		m_localAABB.invalidate();
		i32 i = m_mesh_parts.size();
		while (i--)
		{
			m_mesh_parts[i]->updateBound();
			m_localAABB.merge(m_mesh_parts[i]->getLocalBox());
		}
	}

public:
	GImpactMeshShape(StridingMeshInterface* meshInterface)
	{
		m_meshInterface = meshInterface;
		buildMeshParts(meshInterface);
	}

	virtual ~GImpactMeshShape()
	{
		i32 i = m_mesh_parts.size();
		while (i--)
		{
			GImpactMeshShapePart* part = m_mesh_parts[i];
			delete part;
		}
		m_mesh_parts.clear();
	}

	StridingMeshInterface* getMeshInterface()
	{
		return m_meshInterface;
	}

	const StridingMeshInterface* getMeshInterface() const
	{
		return m_meshInterface;
	}

	i32 getMeshPartCount() const
	{
		return m_mesh_parts.size();
	}

	GImpactMeshShapePart* getMeshPart(i32 index)
	{
		return m_mesh_parts[index];
	}

	const GImpactMeshShapePart* getMeshPart(i32 index) const
	{
		return m_mesh_parts[index];
	}

	virtual void setLocalScaling(const Vec3& scaling)
	{
		localScaling = scaling;

		i32 i = m_mesh_parts.size();
		while (i--)
		{
			GImpactMeshShapePart* part = m_mesh_parts[i];
			part->setLocalScaling(scaling);
		}

		m_needs_update = true;
	}

	virtual void setMargin(Scalar margin)
	{
		m_collisionMargin = margin;

		i32 i = m_mesh_parts.size();
		while (i--)
		{
			GImpactMeshShapePart* part = m_mesh_parts[i];
			part->setMargin(margin);
		}

		m_needs_update = true;
	}

	//! Tells to this object that is needed to refit all the meshes
	virtual void postUpdate()
	{
		i32 i = m_mesh_parts.size();
		while (i--)
		{
			GImpactMeshShapePart* part = m_mesh_parts[i];
			part->postUpdate();
		}

		m_needs_update = true;
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3& inertia) const;

	//! Obtains the primitive manager
	virtual const PrimitiveManagerBase* getPrimitiveManager() const
	{
		Assert(0);
		return NULL;
	}

	//! Gets the number of children
	virtual i32 getNumChildShapes() const
	{
		Assert(0);
		return 0;
	}

	//! if true, then its children must get transforms.
	virtual bool childrenHasTransform2() const
	{
		Assert(0);
		return false;
	}

	//! Determines if this shape has triangles
	virtual bool needsRetrieveTriangles() const
	{
		Assert(0);
		return false;
	}

	//! Determines if this shape has tetrahedrons
	virtual bool needsRetrieveTetrahedrons() const
	{
		Assert(0);
		return false;
	}

	virtual void getBulletTriangle(i32 prim_index, TriangleShapeEx& triangle) const
	{
		(void)prim_index;
		(void)triangle;
		Assert(0);
	}

	virtual void getBulletTetrahedron(i32 prim_index, TetrahedronShapeEx& tetrahedron) const
	{
		(void)prim_index;
		(void)tetrahedron;
		Assert(0);
	}

	//! call when reading child shapes
	virtual void lockChildShapes() const
	{
		Assert(0);
	}

	virtual void unlockChildShapes() const
	{
		Assert(0);
	}

	//! Retrieves the bound from a child
	/*!
    */
	virtual void getChildAabb(i32 child_index, const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		(void)child_index;
		(void)t;
		(void)aabbMin;
		(void)aabbMax;
		Assert(0);
	}

	//! Gets the children
	virtual CollisionShape* getChildShape(i32 index)
	{
		(void)index;
		Assert(0);
		return NULL;
	}

	//! Gets the child
	virtual const CollisionShape* getChildShape(i32 index) const
	{
		(void)index;
		Assert(0);
		return NULL;
	}

	//! Gets the children transform
	virtual Transform2 getChildTransform(i32 index) const
	{
		(void)index;
		Assert(0);
		return Transform2();
	}

	//! Sets the children transform
	/*!
	\post You must call updateBound() for update the box set.
	*/
	virtual void setChildTransform2(i32 index, const Transform2& transform)
	{
		(void)index;
		(void)transform;
		Assert(0);
	}

	virtual eGIMPACT_SHAPE_TYPE getGImpactShapeType() const
	{
		return CONST_GIMPACT_TRIMESH_SHAPE;
	}

	virtual tukk getName() const
	{
		return "GImpactMesh";
	}

	virtual void rayTest(const Vec3& rayFrom, const Vec3& rayTo, CollisionWorld::RayResultCallback& resultCallback) const;

	//! Function for retrieve triangles.
	/*!
	It gives the triangles in local space
	*/
	virtual void processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	virtual void processAllTrianglesRay(TriangleCallback* callback, const Vec3& rayFrom, const Vec3& rayTo) const;

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct GImpactMeshShapeData
{
	CollisionShapeData m_collisionShapeData;

	StridingMeshInterfaceData m_meshInterface;

	Vec3FloatData m_localScaling;

	float m_collisionMargin;

	i32 m_gimpactSubType;
};

SIMD_FORCE_INLINE i32 GImpactMeshShape::calculateSerializeBufferSize() const
{
	return sizeof(GImpactMeshShapeData);
}

#endif  //GIMPACT_MESH_SHAPE_H
