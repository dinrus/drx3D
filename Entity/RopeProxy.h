// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//  предварительные объявления.
struct SEntityEvent;
struct IPhysicalEntity;
struct IPhysicalWorld;

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Implements rope proxy class for entity.
//////////////////////////////////////////////////////////////////////////
class CEntityComponentRope : public IEntityRopeComponent
{
	DRX_ENTITY_COMPONENT_CLASS_GUID(CEntityComponentRope, IEntityRopeComponent, "CEntityComponentRope", "dfae2b7e-15bb-4f3d-bd09-e0c8e560bf85"_drx_guid);

	CEntityComponentRope();
	virtual ~CEntityComponentRope();

public:
	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void   Initialize() final;
	virtual void   ProcessEvent(const SEntityEvent& event) final;
	virtual uint64 GetEventMask() const final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetProxyType() const final { return ENTITY_PROXY_ROPE; }
	virtual void         Release() final            { delete this; };
	virtual void         Update(SEntityUpdateContext& ctx) final;
	virtual void         LegacySerializeXML(XmlNodeRef& entityNode, XmlNodeRef& componentNode, bool bLoading) override final;
	virtual bool         NeedGameSerialize() final;
	virtual void         GameSerialize(TSerialize ser) final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	/// IEntityRopeComponent
	//////////////////////////////////////////////////////////////////////////
	virtual IRopeRenderNode* GetRopeRenderNode() final { return m_pRopeRenderNode; };
	//////////////////////////////////////////////////////////////////////////

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const final
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	void PreserveParams();

protected:
	IRopeRenderNode* m_pRopeRenderNode;
	i32              m_nSegmentsOrg;
	float            m_texTileVOrg;

	i32              m_segmentsCount = 0;
	float            m_texureTileV = 0;
};
