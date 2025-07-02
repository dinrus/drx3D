// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef intersectionchecks_h
#define intersectionchecks_h

typedef i32 (*intersection_check)(const primitive*,const primitive*,prim_inters*);
i32 default_intersection(const primitive*,const primitive*, prim_inters *pinters);

i32 tri_tri_intersection(const triangle *ptri1, const triangle *ptri2, prim_inters *pinters);
i32 tri_box_intersection(const triangle *ptri, const box *pbox, prim_inters *pinters);
i32 box_tri_intersection(const box *pbox, const triangle *ptri, prim_inters *pinters);
i32 tri_cylinder_intersection(const triangle *ptri, const cylinder *pcyl, prim_inters *pinters);
i32 cylinder_tri_intersection(const cylinder *pcyl, const triangle *ptri, prim_inters *pinters);
i32 tri_sphere_intersection(const triangle *ptri, const sphere *psphere, prim_inters *pinters);
i32 sphere_tri_intersection(const sphere *psphere, const triangle *ptri, prim_inters *pinters);
i32 tri_capsule_intersection(const triangle *ptri, const capsule *pcaps, prim_inters *pinters);
i32 capsule_tri_intersection(const capsule *pcyl, const triangle *ptri, prim_inters *pinters);
i32 tri_ray_intersection(const triangle *ptri, const ray *pray, prim_inters *pinters);
i32 ray_tri_intersection(const ray *pray, const triangle *ptri, prim_inters *pinters);
i32 tri_plane_intersection(const triangle *ptri, const plane *pplane, prim_inters *pinters);
i32 plane_tri_intersection(const plane *pplane, const triangle *ptri, prim_inters *pinters);
i32 box_box_intersection(const box *pbox1, const box *pbox2, prim_inters *pinters);
i32 box_cylinder_intersection(const box *pbox, const cylinder *pcyl, prim_inters *pinters);
i32 cylinder_box_intersection(const cylinder *pcyl, const box *pbox, prim_inters *pinters);
i32 box_sphere_intersection(const box *pbox, const sphere *psphere, prim_inters *pinters);
i32 sphere_box_intersection(const sphere *psphere, const box *pbox, prim_inters *pinters);
i32 box_capsule_intersection(const box *pbox, const capsule *pcapsule, prim_inters *pinters);
i32 capsule_box_intersection(const capsule *pcaps, const box *pbox, prim_inters *pinters);
i32 box_ray_intersection(const box *pbox, const ray *pray, prim_inters *pinters);
i32 ray_box_intersection(const ray *pray, const box *pbox, prim_inters *pinters);
i32 box_plane_intersection(const box *pbox, const plane *pplane, prim_inters *pinters);
i32 plane_box_intersection(const plane *pplane, const box *pbox, prim_inters *pinters);
i32 cylinder_cylinder_intersection(const cylinder *pcyl1, const cylinder *pcyl2, prim_inters *pinters);
i32 cylinder_sphere_intersection(const cylinder *pcyl, const sphere *psphere, prim_inters *pinters);
i32 sphere_cylinder_intersection(const sphere *psphere, const cylinder *pcyl, prim_inters *pinters);
i32 cylinder_capsule_intersection(const cylinder *pcyl, const capsule *pcaps, prim_inters *pinters);
i32 capsule_cylinder_intersection(const capsule *pcaps, const cylinder *pcyl, prim_inters *pinters);
i32 cylinder_ray_intersection(const cylinder *pcyl, const ray *pray, prim_inters *pinters);
i32 ray_cylinder_intersection(const ray *pray, const cylinder *pcyl, prim_inters *pinters);
i32 cylinder_plane_intersection(const cylinder *pcyl, const plane *pplane, prim_inters *pinters);
i32 plane_cylinder_intersection(const plane *pplane, const cylinder *pcyl, prim_inters *pinters);
i32 sphere_sphere_intersection(const sphere *psphere1, const sphere *psphere2, prim_inters *pinters);
i32 sphere_ray_intersection(const sphere *psphere, const ray *pray, prim_inters *pinters);
i32 ray_sphere_intersection(const ray *pray, const sphere *psphere, prim_inters *pinters);
i32 capsule_capsule_intersection(const capsule *pcaps1, const capsule *pcaps2, prim_inters *pinters);
i32 capsule_ray_intersection(const capsule *pcaps, const ray *pray, prim_inters *pinters);
i32 ray_capsule_intersection(const ray *pray, const capsule *pcaps, prim_inters *pinters);
i32 sphere_plane_intersection(const sphere *psphere, const plane *pplane, prim_inters *pinters);
i32 plane_sphere_intersection(const plane *pplane, const sphere *psphere, prim_inters *pinters);
i32 ray_plane_intersection(const ray *pray, const plane *pplane, prim_inters *pinters);
i32 plane_ray_intersection(const plane *pplane, const ray *pray, prim_inters *pinters);
i32 capsule_sphere_intersection(const cylinder *pcaps, const sphere *psphere, prim_inters *pinters);
i32 sphere_capsule_intersection(const sphere *psphere, const capsule *pcaps, prim_inters *pinters);

class CIntersectionChecker {
public:
	//CIntersectionChecker();
	ILINE i32 Check(i32 type1,i32 type2, const primitive* prim1,const primitive *prim2, prim_inters* pinters) {
		return table[type1][type2](prim1,prim2, pinters);
	}
	ILINE i32 CheckExists(i32 type1,i32 type2) {
		return table[type1][type2]!=default_intersection;
	}
	static intersection_check table[NPRIMS][NPRIMS];
	static intersection_check default_function;
	i32 CheckAux(i32 type1,i32 type2, const primitive* prim1,const primitive *prim2, prim_inters* pinters);
};
extern CIntersectionChecker g_Intersector;

#endif
