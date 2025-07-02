// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _BREAKABLE_GLASS_RENDERNODE_
#define _BREAKABLE_GLASS_RENDERNODE_

#pragma once

// Includes
#include <drx3D/CoreX/Renderer/RendElements/CREBreakableGlass.h>
#include <drx3D/CoreX/Renderer/RendElements/CREBreakableGlassHelpers.h>

// Forward declares
class CREBreakableGlass;
struct IParticleEffect;

//==================================================================================================
// Name: CBreakableGlassRenderNode
// Desc: Breakable glass sim render node
// Author: Chris Bunner
//==================================================================================================
class CBreakableGlassRenderNode : public IBreakableGlassRenderNode, DinrusX3dEngBase
{
public:
	CBreakableGlassRenderNode();
	virtual ~CBreakableGlassRenderNode();

	// IBreakableGlassRenderNode interface
	virtual bool   InitialiseNode(const SBreakableGlassInitParams& params, const Matrix34& matrix);
	virtual void   ReleaseNode(bool bImmediate);

	virtual void   SetId(u16k id);
	virtual u16 GetId();

	virtual void   Update(SBreakableGlassUpdateParams& params);
	virtual bool   HasGlassShattered();
	virtual bool   HasActiveFragments();
	virtual void   ApplyImpactToGlass(const EventPhysCollision* pPhysEvent);
	virtual void   ApplyExplosionToGlass(const EventPhysCollision* pPhysEvent);
	virtual void   DestroyPhysFragment(SGlassPhysFragment* pPhysFrag);

	virtual void   SetCVars(const SBreakableGlassCVars* pCVars);

	// IRenderNode interface
	virtual tukk      GetName() const;
	virtual EERType          GetRenderNodeType();
	virtual tukk      GetEntityClassName() const;
	virtual void             GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void             SetMaterial(IMaterial* pMaterial);
	virtual IMaterial*       GetMaterial(Vec3* pHitPos = NULL) const;
	virtual void             SetMatrix(const Matrix34& matrix);
	virtual Vec3             GetPos(bool worldOnly = true) const;
	virtual const AABB       GetBBox() const;
	virtual void             SetBBox(const AABB& worldSpaceBoundingBox);
	virtual void             FillBBox(AABB& aabb);
	virtual void             OffsetPosition(const Vec3& delta);
	virtual float            GetMaxViewDist();
	virtual IPhysicalEntity* GetPhysics() const;
	virtual void             SetPhysics(IPhysicalEntity* pPhysics);
	virtual void             Render(const SRendParams& renderParams, const SRenderingPassInfo& passInfo);
	virtual IMaterial*       GetMaterialOverride();

private:
	void PhysicalizeGlass();
	void DephysicalizeGlass();

	void PhysicalizeGlassFragment(SGlassPhysFragment& physFrag, const Vec3& centerOffset);
	void DephysicalizeGlassFragment(SGlassPhysFragment& physFrag);

	void CalculateImpactPoint(const Vec3& pt, Vec2& impactPt);
	void UpdateGlassState(const EventPhysCollision* pPhysEvent);
	void SetParticleEffectColours(IParticleEffect* pEffect, const Vec4& rgba);
	void PlayBreakageEffect(const EventPhysCollision* pPhysEvent);

	TGlassPhysFragmentArray            m_physFrags;
	TGlassPhysFragmentIdArray          m_deadPhysFrags;

	SBreakableGlassInitParams          m_glassParams;
	SBreakableGlassState               m_lastGlassState;

	AABB                               m_planeBBox; // Plane's world space bounding box
	AABB                               m_fragBBox;  // Phys fragments' world space bounding box
	Matrix34                           m_matrix;
	float                              m_maxViewDist;
	CREBreakableGlass*                 m_pBreakableGlassRE;
	IPhysicalEntity*                   m_pPhysEnt;

	static const SBreakableGlassCVars* s_pCVars;

	Vec4                               m_glassTintColour;
	u16                             m_id;
	u8                              m_state;
	u8                              m_nextPhysFrag;
};//------------------------------------------------------------------------------------------------

#endif // _BREAKABLE_GLASS_RENDERNODE_
