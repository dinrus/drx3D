// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef overlapchecks_h
#define overlapchecks_h

typedef i32 (*overlap_check)(const primitive*,const primitive*,class COverlapChecker*);

i32 default_overlap_check(const primitive*, const primitive*, class COverlapChecker*);
i32 box_box_overlap_check(const box *box1, const box *box2, class COverlapChecker*);
i32 box_heightfield_overlap_check(const box *pbox, const heightfield *phf, class COverlapChecker *pOverlapper=0);
i32 heightfield_box_overlap_check(const heightfield *phf, const box *pbox, class COverlapChecker *pOverlapper=0);
i32 box_voxgrid_overlap_check(const box *pbox, const voxelgrid *phf, class COverlapChecker *pOverlapper=0);
i32 voxgrid_box_overlap_check(const voxelgrid *phf, const box *pbox, class COverlapChecker *pOverlapper=0);
i32 box_tri_overlap_check(const box *pbox, const triangle *ptri, class COverlapChecker *pOverlapper=0);
i32 tri_box_overlap_check(const triangle *ptri, const box *pbox, class COverlapChecker *pOverlapper=0);
i32 box_ray_overlap_check(const box *pbox, const ray *pray, class COverlapChecker *pOverlapper=(COverlapChecker*)0);
i32 ray_box_overlap_check(const ray *pray, const box *pbox, class COverlapChecker *pOverlapper=0);
i32 box_sphere_overlap_check(const box *pbox, const sphere *psph, class COverlapChecker *pOverlapper=0);
i32 sphere_box_overlap_check(const sphere *psph, const box *pbox, class COverlapChecker *pOverlapper=0);
i32 tri_sphere_overlap_check(const triangle *ptri, const sphere *psph, class COverlapChecker *pOverlapper=0);
i32 sphere_tri_overlap_check(const sphere *psph, const triangle *ptri, class COverlapChecker *pOverlapper=0);
i32 heightfield_sphere_overlap_check(const heightfield *phf, const sphere *psph, class COverlapChecker *pOverlapper=0);
i32 sphere_heightfield_overlap_check(const sphere *psph, const heightfield *phf, class COverlapChecker *pOverlapper=0);
i32 sphere_sphere_overlap_check(const sphere *psph1, const sphere *psph2, class COverlapChecker *pOverlapper=0);

quotientf tri_sphere_dist2(const triangle *ptri, const sphere *psph, i32 &bFace);

class COverlapChecker {
public:

	COverlapChecker() { Init(); }

	void Init() { iPrevCode = -1; }
	ILINE i32 Check(i32 type1,i32 type2, primitive *prim1,primitive *prim2) {
		return table[type1][type2](prim1,prim2,this);
	}
	ILINE i32 CheckExists(i32 type1,i32 type2) {
		return table[type1][type2]!=default_overlap_check;
	}

	static overlap_check table[NPRIMS][NPRIMS];
	static overlap_check default_function;

	//static COverlapCheckerInit init;

	i32 iPrevCode;
	Matrix33 Basis21;
	Matrix33 Basis21abs;
};
//extern COverlapChecker g_Overlapper;

#endif
