// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __particleitem_h__
#define __particleitem_h__
#pragma once

#include "BaseLibraryItem.h"
#include <Drx3DEngine/I3DEngine.h>
#include <DrxParticleSystem/IParticles.h>

/*! CParticleItem contain definition of particle system spawning parameters.
 *
 */
class SANDBOX_API CParticleItem : public CBaseLibraryItem
{
public:
	CParticleItem();
	CParticleItem(IParticleEffect* pEffect);
	~CParticleItem();

	virtual EDataBaseItemType GetType() const { return EDB_TYPE_PARTICLE; };

	virtual void              SetName(const string& name);
	void                      Serialize(SerializeContext& ctx);

	//////////////////////////////////////////////////////////////////////////
	// Child particle systems.
	//////////////////////////////////////////////////////////////////////////
	//! Get number of sub Particles childs.
	i32            GetChildCount() const;
	//! Get sub Particles child by index.
	CParticleItem* GetChild(i32 index) const;
	//! Remove all sub Particles.
	void           ClearChilds();
	//! Insert sub particles in between other particles.
	//void InsertChild( i32 slot,CParticleItem *mtl );
	//! Find slot where sub Particles stored.
	//! @retun slot index if Particles found, -1 if Particles not found.
	i32            FindChild(CParticleItem* mtl);
	//! Sets new parent of item; may be 0.
	void           SetParent(CParticleItem* pParent);
	//! Returns parent Particles.
	CParticleItem* GetParent() const;

	void           GenerateIdRecursively();

	//! Called after particle parameters where updated.
	void Update();

	//! Get particle effect assigned to this particle item.
	IParticleEffect* GetEffect() const;

	void             DebugEnable(i32 iEnable = -1);
	i32              GetEnabledState() const;

	virtual void     GatherUsedResources(CUsedResources& resources);

	void             AddAllChildren();

private:
	_smart_ptr<IParticleEffect> m_pEffect;

	//! Parent of this material (if this is sub material).
	CParticleItem*                        m_pParentParticles;
	//! Array of sub particle items.
	std::vector<TSmartPtr<CParticleItem>> m_childs;
	bool m_bSaveEnabled, m_bDebugEnabled;

	// asset resolving
	typedef std::map<IVariable::EDataType, u32> TResolveReq;
	TResolveReq m_ResolveRequests;
};

#endif // __particleitem_h__

