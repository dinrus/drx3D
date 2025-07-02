#ifndef DRX3D_VEHICLE_RAYCASTER_H
#define DRX3D_VEHICLE_RAYCASTER_H

#include <drx3D/Maths/Linear/Vec3.h>

/// VehicleRaycaster is provides interface for between vehicle simulation and raycasting
struct VehicleRaycaster
{
	virtual ~VehicleRaycaster()
	{
	}
	struct VehicleRaycasterResult
	{
		VehicleRaycasterResult() : m_distFraction(Scalar(-1.)){};
		Vec3 m_hitPointInWorld;
		Vec3 m_hitNormalInWorld;
		Scalar m_distFraction;
	};

	virtual uk castRay(const Vec3& from, const Vec3& to, VehicleRaycasterResult& result) = 0;
};

#endif  //DRX3D_VEHICLE_RAYCASTER_H
