// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/ModelAnimationSet.h>

struct CHeaderTCB;
class CChunkFileReader;

typedef spline::TCBSpline<Vec3> CControllerTCBVec3;

// Internal description of node.
struct NodeDesc
{
	NodeDesc()
	{
		active = 0;
		node_idx = 0xffff;    //index of joint
		parentID = 0xffff;    //if of parent-chunk
		pos_cont_id = 0xffff; // position controller chunk id
		rot_cont_id = 0xffff; // rotation controller chunk id
		scl_cont_id = 0xffff; // scale		controller chunk id
	};

	u16 active;
	u16 node_idx;    //index of joint
	u16 parentID;
	u16 pos_cont_id; // position controller chunk id
	u16 rot_cont_id; // rotation controller chunk id
	u16 scl_cont_id; // scale		controller chunk id
};

//////////////////////////////////////////////////////////////////////////
// Loads AnimObject from CGF/CAF files.
//////////////////////////////////////////////////////////////////////////
class DrxCGALoader
{
public:

	DrxCGALoader() : m_start(0), m_end(0)
	{}

	// Load animation object from cgf or caf.
	CDefaultSkeleton* LoadNewCGA(tukk geomName, CharacterUpr* pUpr, u32 nLoadingFlags);

	//private:
public:
	void InitNodes(CHeaderTCB* pSkinningInfo, CDefaultSkeleton* pCGAModel, tukk animFile, const string& strName, bool bMakeNodes, u32 unique_model_id, u32 nLoadingFlags);

	// Load all animations for this object.
	void   LoadAnimations(tukk cgaFile, CDefaultSkeleton* pCGAModel, u32 unique_model_id, u32 nLoadingFlags);
	bool   LoadAnimationANM(tukk animFile, CDefaultSkeleton* pCGAModel, u32 unique_model_id, u32 nLoadingFlags);
	u32 LoadANM(CDefaultSkeleton* pDefaultSkeleton, tukk pFilePath, tukk pAnimName, DynArray<CControllerTCB>& m_LoadCurrAnimation);

	void   Reset();

	DynArray<CControllerTCBVec3>         m_CtrlVec3;
	DynArray<spline::TCBAngleAxisSpline> m_CtrlQuat;

	// Array of controllers.
	DynArray<CControllerType> m_arrControllers;

	DynArray<NodeDesc>        m_arrChunkNodes;

	i32                       m_start; // Start time of the animation, expressed in frames (constant frame rate of 30fps is assumed).
	i32                       m_end;   // End time of the animation, expressed in frames (constant frame rate of 30fps is assumed).

	u32                    m_DefaultNodeCount;

	//the controllers for CGA are in this array
	DynArray<CControllerTCB> m_arrNodeAnims;

	// Created animation object
	ModelAnimationHeader m_ModelAnimationHeader;
};
