// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef unprojectionchecks_h
#define unprojectionchecks_h

struct unprojection_mode {
	// cppcheck-suppress uninitMemberVar
	unprojection_mode() { bCheckContact=0; tmin=0; maxcos=0.1f; }
	i32 imode;
	Vec3 dir;	// direction or rotation axis
	Vec3 center; // center of rotation
	float vel; // linear or angular velocity
	float tmax; // maximum unprojection length (not time)
	float tmin; // minimum unprojection length
	float minPtDist; // tolerance value
	float maxcos;
	i32 bCheckContact;

	Matrix33 R0;
	Vec3 offset0;
};

typedef i32 (*unprojection_check)(unprojection_mode*, const primitive*,i32,const primitive*,i32, contact*, geom_contact_area*);

i32 default_unprojection(unprojection_mode*, const primitive*,i32,const primitive*,i32, contact*, geom_contact_area*);
i32 tri_tri_lin_unprojection(unprojection_mode *pmode, const triangle *ptri1,i32 iFeature1,const triangle *ptri2,i32 iFeature2,
														contact *pcontact, geom_contact_area *parea);
i32 tri_box_lin_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const box *pbox,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 box_tri_lin_unprojection(unprojection_mode *pmode, const box *pbox,i32 iFeature1,const triangle *ptri,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 tri_cylinder_lin_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const cylinder *pcyl,i32 iFeature2,
																	contact *pcontact, geom_contact_area *parea);
i32 cylinder_tri_lin_unprojection(unprojection_mode *pmode, const cylinder *pcyl,i32 iFeature1,const triangle *ptri,i32 iFeature2,
																	contact *pcontact, geom_contact_area *parea);
i32 tri_sphere_lin_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const sphere *psphere,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);
i32 sphere_tri_lin_unprojection(unprojection_mode *pmode, const sphere *psphere,i32 iFeature1,const triangle *ptri,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);
i32 tri_capsule_lin_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const capsule *pcaps,i32 iFeature2,
																 contact *pcontact, geom_contact_area *parea);
i32 capsule_tri_lin_unprojection(unprojection_mode *pmode, const capsule *pcaps,i32 iFeature1,const triangle *ptri,i32 iFeature2,
																 contact *pcontact, geom_contact_area *parea);
i32 tri_ray_lin_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const ray *pray,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 ray_tri_lin_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const triangle *ptri,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 tri_plane_lin_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const plane *pplane,i32 iFeature2,
															 contact *pcontact, geom_contact_area *parea);
i32 plane_tri_lin_unprojection(unprojection_mode *pmode, const plane *pplane,i32 iFeature1,const triangle *ptri,i32 iFeature2,
															 contact *pcontact, geom_contact_area *parea);
i32 box_box_lin_unprojection(unprojection_mode *pmode, const box *pbox1,i32 iFeature1, const box *pbox2,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 box_cylinder_lin_unprojection(unprojection_mode *pmode, const box *pbox,i32 iFeature1,const cylinder *pcyl,i32 iFeature2,
																	contact *pcontact, geom_contact_area *parea);
i32 cylinder_box_lin_unprojection(unprojection_mode *pmode, const cylinder *pcyl,i32 iFeature1,const box *pbox,i32 iFeature2,
																	contact *pcontact, geom_contact_area *parea);
i32 box_sphere_lin_unprojection(unprojection_mode *pmode, const box *pbox,i32 iFeature1,const sphere *psph,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);
i32 sphere_box_lin_unprojection(unprojection_mode *pmode, const sphere *psph,i32 iFeature1,const box *pbox,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);
i32 box_capsule_lin_unprojection(unprojection_mode *pmode, const box *pbox,i32 iFeature1,const capsule *pcaps,i32 iFeature2,
																 contact *pcontact, geom_contact_area *parea);
i32 capsule_box_lin_unprojection(unprojection_mode *pmode, const capsule *pcaps,i32 iFeature1,const box *pbox,i32 iFeature2,
																 contact *pcontact, geom_contact_area *parea);
i32 cyl_cyl_lin_unprojection(unprojection_mode *pmode, const cylinder *pcyl1,i32 iFeature1,const cylinder *pcyl2,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 cylinder_sphere_lin_unprojection(unprojection_mode *pmode, const cylinder *pcyl,i32 iFeature1,const sphere *psph,i32 iFeature2,
																		 contact *pcontact, geom_contact_area *parea);
i32 sphere_cylinder_lin_unprojection(unprojection_mode *pmode, const sphere *psph,i32 iFeature1,const cylinder *pcyl,i32 iFeature2,
																		 contact *pcontact, geom_contact_area *parea);
i32 cylinder_capsule_lin_unprojection(unprojection_mode *pmode, const cylinder *pbox,i32 iFeature1,const capsule *pcaps,i32 iFeature2,
																			contact *pcontact, geom_contact_area *parea);
i32 capsule_cylinder_lin_unprojection(unprojection_mode *pmode, const capsule *pcaps,i32 iFeature1,const cylinder *pcyl,i32 iFeature2,
																			contact *pcontact, geom_contact_area *parea);
i32 capsule_capsule_lin_unprojection(unprojection_mode *pmode, const capsule *pcaps1,i32 iFeature1,const capsule *pcaps2,i32 iFeature2,
																		 contact *pcontact, geom_contact_area *parea);
i32 sphere_sphere_lin_unprojection(unprojection_mode *pmode, const sphere *psph1,i32 iFeature1, const sphere *psph2,i32 iFeature2,
																	 contact *pcontact, geom_contact_area *parea);
i32 sphere_capsule_lin_unprojection(unprojection_mode *pmode, const sphere *psph,i32 iFeature1,const capsule *pcaps,i32 iFeature2,
																		contact *pcontact, geom_contact_area *parea);
i32 capsule_sphere_lin_unprojection(unprojection_mode *pmode, const capsule *pcaps,i32 iFeature1,const sphere *psph,i32 iFeature2,
																								contact *pcontact, geom_contact_area *parea);
i32 ray_box_lin_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const box *pbox,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 box_ray_lin_unprojection(unprojection_mode *pmode, const box *pbox,i32 iFeature1,const ray *pray,i32 iFeature2,
														 contact *pcontact, geom_contact_area *parea);
i32 ray_cylinder_lin_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const cylinder *pcyl,i32 iFeature2,
																	contact *pcontact, geom_contact_area *parea);
i32 cylinder_ray_lin_unprojection(unprojection_mode *pmode, const cylinder *pcyl,i32 iFeature1,const ray *pray,i32 iFeature2,
																	contact *pcontact, geom_contact_area *parea);
i32 ray_sphere_lin_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const sphere *psph,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);
i32 sphere_ray_lin_unprojection(unprojection_mode *pmode, const sphere *psph,i32 iFeature1,const ray *pray,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);
i32 ray_capsule_lin_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const capsule *pcaps,i32 iFeature2,
																 contact *pcontact, geom_contact_area *parea);
i32 capsule_ray_lin_unprojection(unprojection_mode *pmode, const capsule *pcaps,i32 iFeature1,const ray *pray,i32 iFeature2,
																 contact *pcontact, geom_contact_area *parea);

i32 tri_tri_rot_unprojection(unprojection_mode *pmode, const triangle *ptri1,i32 iFeature1,const triangle *ptri2,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 tri_ray_rot_unprojection(unprojection_mode *pmode, const triangle *ptri,i32 iFeature1,const ray *pray,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 ray_tri_rot_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const triangle *ptri,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 cyl_ray_rot_unprojection(unprojection_mode *pmode, const cylinder *pcyl,i32 iFeature1,const ray *pray,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 ray_cyl_rot_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1,const cylinder *pcyl,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 box_ray_rot_unprojection(unprojection_mode *pmode, const box *pbox,i32 iFeature1, const ray *pray,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 ray_box_rot_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1, const box *pbox,i32 iFeature2,
					 contact *pcontact, geom_contact_area *parea);
i32 capsule_ray_rot_unprojection(unprojection_mode *pmode, const capsule *pcaps,i32 iFeature1, const ray *pray,i32 iFeature2,
							 contact *pcontact, geom_contact_area *parea);
i32 ray_capsule_rot_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1, const capsule *pcaps,i32 iFeature2,
							 contact *pcontact, geom_contact_area *parea);
i32 sphere_ray_rot_unprojection(unprojection_mode *pmode, const sphere *psph,i32 iFeature1, const ray *pray,i32 iFeature2,
						  contact *pcontact, geom_contact_area *parea);
i32 ray_sphere_rot_unprojection(unprojection_mode *pmode, const ray *pray,i32 iFeature1, const sphere *psph,i32 iFeature2,
																contact *pcontact, geom_contact_area *parea);


class CUnprojectionChecker {
public:
	//CUnprojectionChecker();
	ILINE i32 Check(unprojection_mode *pmode, i32 type1,i32 type2, const primitive *prim1,i32 iFeature1,const primitive *prim2,i32 iFeature2,
		contact *pcontact, geom_contact_area *parea=0)
	{
    unprojection_check func = table[pmode->imode][type1][type2];
		return func(pmode, prim1,iFeature1,prim2,iFeature2, pcontact, parea);
	}
	ILINE i32 CheckExists(i32 imode, i32 type1,i32 type2) {
    unprojection_check func = table[imode][type1][type2];
		return func!=default_unprojection;
	}

	static unprojection_check table[2][NPRIMS][NPRIMS];
};
extern CUnprojectionChecker g_Unprojector;

#endif
