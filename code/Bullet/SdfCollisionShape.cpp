#include <drx3D/Physics/Collision/Shapes/SdfCollisionShape.h>
#include <drx3D/Physics/Collision/Shapes/MiniSDF.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>

ATTRIBUTE_ALIGNED16(struct)
SdfCollisionShapeInternalData
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Vec3 m_localScaling;
	Scalar m_margin;
	MiniSDF m_sdf;

	SdfCollisionShapeInternalData()
		: m_localScaling(1, 1, 1),
		  m_margin(0)
	{
	}
};

bool SdfCollisionShape::initializeSDF(tukk sdfData, i32 sizeInBytes)
{
	bool valid = m_data->m_sdf.load(sdfData, sizeInBytes);
	return valid;
}
SdfCollisionShape::SdfCollisionShape()
{
	m_shapeType = SDF_SHAPE_PROXYTYPE;
	m_data = new SdfCollisionShapeInternalData();

	//"E:/develop/bullet3/data/toys/ground_hole64_64_8.cdf");//ground_cube.cdf");
	/*u32 field_id=0;
	Eigen::Vec3d x (1,10,1);
	Eigen::Vec3d gradient;
	double dist = m_data->m_sdf.interpolate(field_id, x, &gradient);
	printf("dist=%g\n", dist);
	*/
}
SdfCollisionShape::~SdfCollisionShape()
{
	delete m_data;
}

void SdfCollisionShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	Assert(m_data->m_sdf.isValid());
	Vec3 localAabbMin = m_data->m_sdf.m_domain.m_min;
	Vec3 localAabbMax = m_data->m_sdf.m_domain.m_max;
	Scalar margin(0);
	Transform2Aabb(localAabbMin, localAabbMax, margin, t, aabbMin, aabbMax);
}

void SdfCollisionShape::setLocalScaling(const Vec3& scaling)
{
	m_data->m_localScaling = scaling;
}
const Vec3& SdfCollisionShape::getLocalScaling() const
{
	return m_data->m_localScaling;
}
void SdfCollisionShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	inertia.setVal(0, 0, 0);
}
tukk SdfCollisionShape::getName() const
{
	return "SdfCollisionShape";
}
void SdfCollisionShape::setMargin(Scalar margin)
{
	m_data->m_margin = margin;
}
Scalar SdfCollisionShape::getMargin() const
{
	return m_data->m_margin;
}

void SdfCollisionShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	//not yet
}

bool SdfCollisionShape::queryPoint(const Vec3& ptInSDF, Scalar& distOut, Vec3& normal)
{
	i32 field = 0;
	Vec3 grad;
	double dist;
	bool hasResult = m_data->m_sdf.interpolate(field, dist, ptInSDF, &grad);
	if (hasResult)
	{
		normal.setVal(grad[0], grad[1], grad[2]);
		distOut = dist;
	}
	return hasResult;
}
