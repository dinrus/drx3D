
#include <drx3D/CoreX/Math/Drx_Camera.h>
//------------------------------------------------------------------------------
//---                         ADDITIONAL-TEST                                ---
//------------------------------------------------------------------------------

extern DRX_ALIGN(64) u32 BoxSides[];

	
//! A box can easily straddle one of the view-frustum planes far outside the view-frustum and in this case the previous test would return CULL_OVERLAP.
//! \note With this check, we make sure the AABB is really not visble.
bool CCamera::AdditionalCheck(const AABB& aabb) const
{
	Vec3d m = (aabb.min + aabb.max) * 0.5;
	u32 o = 1; //will be reset to 0 if center is outside
	o &= isneg(m_fp[0] | m);
	o &= isneg(m_fp[2] | m);
	o &= isneg(m_fp[3] | m);
	o &= isneg(m_fp[4] | m);
	o &= isneg(m_fp[5] | m);
	o &= isneg(m_fp[1] | m);
	if (o) return CULL_OVERLAP; //if obb-center is in view-frustum, then stop further calculation

	Vec3d vmin(aabb.min - GetPosition());  //AABB in camera-space
	Vec3d vmax(aabb.max - GetPosition());  //AABB in camera-space

	u32 frontx8 = 0; // make the flags using the fact that the upper bit in f32 is its sign

	union f64_u
	{
		f64   floatVal;
		int64 intVal;
	};
	f64_u uminx, uminy, uminz, umaxx, umaxy, umaxz;
	uminx.floatVal = vmin.x;
	uminy.floatVal = vmin.y;
	uminz.floatVal = vmin.z;
	umaxx.floatVal = vmax.x;
	umaxy.floatVal = vmax.y;
	umaxz.floatVal = vmax.z;

	frontx8 |= (-uminx.intVal >> 0x3f) & 0x008; //if (AABB.min.x>0.0f)  frontx8|=0x008;
	frontx8 |= (umaxx.intVal >> 0x3f) & 0x010;  //if (AABB.max.x<0.0f)  frontx8|=0x010;
	frontx8 |= (-uminy.intVal >> 0x3f) & 0x020; //if (AABB.min.y>0.0f)  frontx8|=0x020;
	frontx8 |= (umaxy.intVal >> 0x3f) & 0x040;  //if (AABB.max.y<0.0f)  frontx8|=0x040;
	frontx8 |= (-uminz.intVal >> 0x3f) & 0x080; //if (AABB.min.z>0.0f)  frontx8|=0x080;
	frontx8 |= (umaxz.intVal >> 0x3f) & 0x100;  //if (AABB.max.z<0.0f)  frontx8|=0x100;

	//check if camera is inside the aabb
	if (frontx8 == 0) return CULL_OVERLAP; //AABB is patially visible

	Vec3d v[8] = {
		Vec3d(vmin.x, vmin.y, vmin.z),
		Vec3d(vmax.x, vmin.y, vmin.z),
		Vec3d(vmin.x, vmax.y, vmin.z),
		Vec3d(vmax.x, vmax.y, vmin.z),
		Vec3d(vmin.x, vmin.y, vmax.z),
		Vec3d(vmax.x, vmin.y, vmax.z),
		Vec3d(vmin.x, vmax.y, vmax.z),
		Vec3d(vmax.x, vmax.y, vmax.z)
	};

	//---------------------------------------------------------------------
	//---            find the silhouette-vertices of the AABB            ---
	//---------------------------------------------------------------------
	u32 p0 = BoxSides[frontx8 + 0];
	u32 p1 = BoxSides[frontx8 + 1];
	u32 p2 = BoxSides[frontx8 + 2];
	u32 p3 = BoxSides[frontx8 + 3];
	u32 p4 = BoxSides[frontx8 + 4];
	u32 p5 = BoxSides[frontx8 + 5];
	u32 sideamount = BoxSides[frontx8 + 7];

	if (sideamount == 4)
	{
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//-----  and clip them against the 4 side-frustum-planes of the AABB        -
		//--------------------------------------------------------------------------
		Vec3d s0 = v[p0] % v[p1];
		if ((s0 | m_cltp) > 0 && (s0 | m_crtp) > 0 && (s0 | m_crbp) > 0 && (s0 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s1 = v[p1] % v[p2];
		if ((s1 | m_cltp) > 0 && (s1 | m_crtp) > 0 && (s1 | m_crbp) > 0 && (s1 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s2 = v[p2] % v[p3];
		if ((s2 | m_cltp) > 0 && (s2 | m_crtp) > 0 && (s2 | m_crbp) > 0 && (s2 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s3 = v[p3] % v[p0];
		if ((s3 | m_cltp) > 0 && (s3 | m_crtp) > 0 && (s3 | m_crbp) > 0 && (s3 | m_clbp) > 0) return CULL_EXCLUSION;
	}

	if (sideamount == 6)
	{
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//---    and clip them against the 6 side-frustum-planes of the AABB      ---
		//--------------------------------------------------------------------------
		Vec3d s0 = v[p0] % v[p1];
		if ((s0 | m_cltp) > 0 && (s0 | m_crtp) > 0 && (s0 | m_crbp) > 0 && (s0 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s1 = v[p1] % v[p2];
		if ((s1 | m_cltp) > 0 && (s1 | m_crtp) > 0 && (s1 | m_crbp) > 0 && (s1 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s2 = v[p2] % v[p3];
		if ((s2 | m_cltp) > 0 && (s2 | m_crtp) > 0 && (s2 | m_crbp) > 0 && (s2 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s3 = v[p3] % v[p4];
		if ((s3 | m_cltp) > 0 && (s3 | m_crtp) > 0 && (s3 | m_crbp) > 0 && (s3 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s4 = v[p4] % v[p5];
		if ((s4 | m_cltp) > 0 && (s4 | m_crtp) > 0 && (s4 | m_crbp) > 0 && (s4 | m_clbp) > 0) return CULL_EXCLUSION;
		Vec3d s5 = v[p5] % v[p0];
		if ((s5 | m_cltp) > 0 && (s5 | m_crtp) > 0 && (s5 | m_crbp) > 0 && (s5 | m_clbp) > 0) return CULL_EXCLUSION;
	}
	return CULL_OVERLAP; //AABB is patially visible
}

//------------------------------------------------------------------------------
//---                         ADDITIONAL-TEST                                ---
//------------------------------------------------------------------------------

//! A box can easily straddle one of the view-frustum planes far outside the view-frustum and in this case the previous test would return CULL_OVERLAP.
//! \note With this check, we make sure the OBB is really not visible.
bool CCamera::AdditionalCheck(const Vec3& wpos, const OBB& obb, f32 uscale) const
{
	Vec3 CamInOBBSpace = wpos - GetPosition();
	Vec3 iCamPos = -CamInOBBSpace * obb.m33;
	u32 front8 = 0;
	AABB aabb = AABB((obb.c - obb.h) * uscale, (obb.c + obb.h) * uscale);
	if (iCamPos.x < aabb.min.x)  front8 |= 0x008;
	if (iCamPos.x > aabb.max.x)  front8 |= 0x010;
	if (iCamPos.y < aabb.min.y)  front8 |= 0x020;
	if (iCamPos.y > aabb.max.y)  front8 |= 0x040;
	if (iCamPos.z < aabb.min.z)  front8 |= 0x080;
	if (iCamPos.z > aabb.max.z)  front8 |= 0x100;

	if (front8 == 0) return CULL_OVERLAP;

	//the transformed OBB-vertices in cam-space
	Vec3 v[8] = {
		obb.m33 * Vec3(aabb.min.x, aabb.min.y, aabb.min.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.max.x, aabb.min.y, aabb.min.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.min.x, aabb.max.y, aabb.min.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.max.x, aabb.max.y, aabb.min.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.min.x, aabb.min.y, aabb.max.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.max.x, aabb.min.y, aabb.max.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.min.x, aabb.max.y, aabb.max.z) + CamInOBBSpace,
		obb.m33 * Vec3(aabb.max.x, aabb.max.y, aabb.max.z) + CamInOBBSpace
	};

	//---------------------------------------------------------------------
	//---            find the silhouette-vertices of the OBB            ---
	//---------------------------------------------------------------------
	u32 p0 = BoxSides[front8 + 0];
	u32 p1 = BoxSides[front8 + 1];
	u32 p2 = BoxSides[front8 + 2];
	u32 p3 = BoxSides[front8 + 3];
	u32 p4 = BoxSides[front8 + 4];
	u32 p5 = BoxSides[front8 + 5];
	u32 sideamount = BoxSides[front8 + 7];

	if (sideamount == 4)
	{
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//-----  and clip them against the 4 side-frustum-planes of the OBB        -
		//--------------------------------------------------------------------------
		Vec3 s0 = v[p0] % v[p1];
		if (((s0 | m_cltp) >= 0) && ((s0 | m_crtp) >= 0) && ((s0 | m_crbp) >= 0) && ((s0 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s1 = v[p1] % v[p2];
		if (((s1 | m_cltp) >= 0) && ((s1 | m_crtp) >= 0) && ((s1 | m_crbp) >= 0) && ((s1 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s2 = v[p2] % v[p3];
		if (((s2 | m_cltp) >= 0) && ((s2 | m_crtp) >= 0) && ((s2 | m_crbp) >= 0) && ((s2 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s3 = v[p3] % v[p0];
		if (((s3 | m_cltp) >= 0) && ((s3 | m_crtp) >= 0) && ((s3 | m_crbp) >= 0) && ((s3 | m_clbp) >= 0)) return CULL_EXCLUSION;
	}

	if (sideamount == 6)
	{
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//---    and clip them against the 6 side-frustum-planes of the OBB      ---
		//--------------------------------------------------------------------------
		Vec3 s0 = v[p0] % v[p1];
		if (((s0 | m_cltp) >= 0) && ((s0 | m_crtp) >= 0) && ((s0 | m_crbp) >= 0) && ((s0 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s1 = v[p1] % v[p2];
		if (((s1 | m_cltp) >= 0) && ((s1 | m_crtp) >= 0) && ((s1 | m_crbp) >= 0) && ((s1 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s2 = v[p2] % v[p3];
		if (((s2 | m_cltp) >= 0) && ((s2 | m_crtp) >= 0) && ((s2 | m_crbp) >= 0) && ((s2 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s3 = v[p3] % v[p4];
		if (((s3 | m_cltp) >= 0) && ((s3 | m_crtp) >= 0) && ((s3 | m_crbp) >= 0) && ((s3 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s4 = v[p4] % v[p5];
		if (((s4 | m_cltp) >= 0) && ((s4 | m_crtp) >= 0) && ((s4 | m_crbp) >= 0) && ((s4 | m_clbp) >= 0)) return CULL_EXCLUSION;
		Vec3 s5 = v[p5] % v[p0];
		if (((s5 | m_cltp) >= 0) && ((s5 | m_crtp) >= 0) && ((s5 | m_crbp) >= 0) && ((s5 | m_clbp) >= 0)) return CULL_EXCLUSION;
	}
	//now we are 100% sure that the OBB is visible on the screen
	return CULL_OVERLAP;
}
