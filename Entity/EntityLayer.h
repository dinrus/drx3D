// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/Entity/IEntityLayer.h>

struct SEntityLayerGarbage
{
	SEntityLayerGarbage(IGeneralMemoryHeap* pHeap, const string& layerName)
		: pHeap(pHeap)
		, layerName(layerName)
		, nAge(0)
	{
	}

	IGeneralMemoryHeap* pHeap;
	string              layerName;
	i32                 nAge;
};
class CEntityLayer;

//////////////////////////////////////////////////////////////////////////
// Structure for deferred layer activation/deactivation processing
// The operations are queued during serialization, then sorted to ensure deactivation
// happens prior to activation.
struct SPostSerializeLayerActivation
{
	typedef void (CEntityLayer::* ActivationFunc)(bool);
	CEntityLayer*  m_layer;
	ActivationFunc m_func;
	bool           enable;
};

typedef std::vector<SPostSerializeLayerActivation> TLayerActivationOpVec;

class CEntityLayer : public IEntityLayer
{
	struct EntityProp
	{
		EntityProp() : m_id(0), m_bIsNoAwake(false), m_bIsHidden(false), m_bEnableScriptUpdate(false)
		{
		}

		EntityProp(EntityId id, bool bIsNoAwake, bool bIsHidden, bool bEnableScriptUpdate)
			: m_id(id)
			, m_bIsNoAwake(bIsNoAwake)
			, m_bIsHidden(bIsHidden)
			, m_bEnableScriptUpdate(bEnableScriptUpdate)
		{
		}

		bool operator==(const EntityProp& other) const
		{
			return (m_id == other.m_id);
		}

		EntityId m_id;
		bool     m_bIsNoAwake          : 1;
		bool     m_bIsHidden           : 1;
		bool     m_bEnableScriptUpdate : 1;
	};

	struct EntityPropFindPred
	{
		explicit EntityPropFindPred(EntityId _idToFind) : idToFind(_idToFind) {}
		bool operator()(const EntityProp& entityProp) { return entityProp.m_id == idToFind; }
		EntityId idToFind;
	};

public:
	typedef std::vector<SEntityLayerGarbage> TGarbageHeaps;

public:
	CEntityLayer(tukk name, u16 id, bool havePhysics, i32 specs, bool defaultLoaded, TGarbageHeaps& garbageHeaps);
	virtual ~CEntityLayer();

	virtual void          SetParentName(tukk szParent) override { if (szParent) m_parentName = szParent; }
	virtual void          AddChild(IEntityLayer* pLayer) override      { return m_childs.push_back(static_cast<CEntityLayer*>(pLayer)); }
	virtual i32           GetNumChildren() const override              { return m_childs.size(); }
	virtual CEntityLayer* GetChild(i32 idx) const override             { return m_childs[idx]; }
	virtual void          AddObject(EntityId id) override;
	virtual void          RemoveObject(EntityId id) override;
	virtual void          Enable(bool bEnable, bool bSerialize = true, bool bAllowRecursive = true) override;
	virtual bool          IsEnabled() const override                 { return (m_isEnabled | m_isEnabledBrush); }
	virtual bool          IsEnabledBrush() const override            { return m_isEnabledBrush; }
	virtual bool          IsSerialized() const override              { return m_isSerialized; }
	virtual bool          IsDefaultLoaded() const override           { return m_defaultLoaded; }
	virtual bool          IncludesEntity(EntityId id) const override { return m_entities.find(id) != m_entities.end(); }
	virtual tukk   GetName() const override                   { return m_name.c_str(); }
	virtual tukk   GetParentName() const override             { return m_parentName.c_str(); }
	virtual u16k  GetId() const override                     { return m_id; }

	void                  GetMemoryUsage(IDrxSizer* pSizer, i32* pOutNumEntities);
	void                  Serialize(TSerialize ser, TLayerActivationOpVec& deferredOps);
	virtual bool          IsSkippedBySpec() const override;

	void                  AddListener(IEntityLayerListener* pListener)    { m_listeners.Add(pListener); }
	void                  RemoveListener(IEntityLayerListener* pListener) { m_listeners.Remove(pListener); }

private:

	void EnableBrushes(bool isEnable);
	void EnableEntities(bool isEnable);
	void ReEvalNeedForHeap();
	void NotifyActivationToListeners(bool bActivated);

private:
	typedef std::unordered_map<EntityId, EntityProp, stl::hash_uint32> TEntityProps;
	typedef CListenerSet<IEntityLayerListener*>                        TListenerSet;

	i32                        m_specs;
	string                     m_name;
	string                     m_parentName;
	bool                       m_isEnabled;
	bool                       m_isEnabledBrush;
	bool                       m_isSerialized;
	bool                       m_havePhysics;
	bool                       m_defaultLoaded;
	bool                       m_wasReEnabled;
	u16                     m_id;
	std::vector<CEntityLayer*> m_childs;
	TEntityProps               m_entities;
	TListenerSet               m_listeners;

	TGarbageHeaps*             m_pGarbageHeaps;
	IGeneralMemoryHeap*        m_pHeap;
};
