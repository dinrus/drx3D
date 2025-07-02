// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/EntitySystem.h>
#include <drx3D/Entity/IEntityClass.h>
#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/CoreX/Renderer/IRenderMesh.h>

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Proxy for storage of entity attributes.
//////////////////////////////////////////////////////////////////////////
class CEntityComponentClipVolume final : public IClipVolumeComponent
{
	DRX_ENTITY_COMPONENT_CLASS_GUID(CEntityComponentClipVolume, IClipVolumeComponent, "CEntityComponentClipVolume", "80652532-9245-4cd7-a906-2e7839ebb7a4"_drx_guid);

	CEntityComponentClipVolume();
	virtual ~CEntityComponentClipVolume();

public:
	// IEntityComponent.h interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void   Initialize() override;
	virtual void   ProcessEvent(const SEntityEvent& event) override;
	virtual uint64 GetEventMask() const final;
	//////////////////////////////////////////////////////////////////////////

	// IEntityComponent
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetProxyType() const override                { return ENTITY_PROXY_CLIPVOLUME; }
	virtual void         Release() final                              { delete this; };
	virtual void         LegacySerializeXML(XmlNodeRef& entityNode, XmlNodeRef& componentNode, bool bLoading) override;
	virtual void         GameSerialize(TSerialize serialize) override {};
	virtual bool         NeedGameSerialize() override                 { return false; }
	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const override;
	//////////////////////////////////////////////////////////////////////////

	//~IClipVolumeComponent
	virtual void         SetGeometryFilename(tukk sFilename) final;
	virtual void         UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces) override;
	virtual IClipVolume* GetClipVolume() const override { return m_pClipVolume; }
	virtual IBSPTree3D*  GetBspTree() const override    { return m_pBspTree; }
	virtual void         SetProperties(bool bIgnoresOutdoorAO, u8 viewDistRatio) override;
	//~IClipVolumeComponent

private:
	bool LoadFromFile(tukk szFilePath);

private:
	// Engine clip volume
	IClipVolume* m_pClipVolume;

	// Render Mesh
	_smart_ptr<IRenderMesh> m_pRenderMesh;

	// BSP tree
	IBSPTree3D* m_pBspTree;

	// In-game stat obj
	string m_GeometryFileName;

	// Clip volume flags
	u32 m_nFlags;

	// View dist ratio
	u32 m_viewDistRatio;
};

DECLARE_SHARED_POINTERS(CEntityComponentClipVolume)
