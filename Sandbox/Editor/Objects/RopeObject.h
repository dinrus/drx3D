// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ShapeObject.h"

//////////////////////////////////////////////////////////////////////////
class CRopeObject : public CShapeObject
{
	DECLARE_DYNCREATE(CRopeObject)
public:
	CRopeObject();

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	virtual bool       Init(CBaseObject* prev, const string& file);
	virtual void       Done();
	virtual bool       SetScale(const Vec3& vScale, i32 nWhyFlags = 0) const { return false; };
	virtual void       SetSelected(bool bSelect);
	virtual bool       CreateGameObject();
	virtual void       InitVariables();
	virtual void       UpdateGameArea();
	virtual void       InvalidateTM(i32 nWhyFlags);
	virtual void       SetMaterial(IEditorMaterial* mtl);
	virtual void       Display(CObjectRenderHelper& objRenderHelper);
	virtual void       OnEvent(ObjectEvent event);
	virtual void       Reload(bool bReloadScript = false);

	void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	virtual void       Serialize(CObjectArchive& ar);
	virtual void       PostLoad(CObjectArchive& ar);
	virtual XmlNodeRef Export(const string& levelPath, XmlNodeRef& xmlNode);
	//////////////////////////////////////////////////////////////////////////

protected:
	virtual i32      GetMinPoints() const    { return 2; };
	virtual i32      GetMaxPoints() const    { return 200; };
	virtual float    GetShapeZOffset() const { return 0.0f; }
	virtual void     CalcBBox();

	IRopeRenderNode* GetRenderNode() const;

	//! Called when Road variable changes.
	void OnParamChange(IVariable* var);
	
	void SerializeProperties(Serialization::IArchive& ar, bool bMultiEdit);

protected:
	IRopeRenderNode::SRopeParams m_ropeParams;

	struct SEndPointLinks
	{
		i32  m_nPhysEntityId;
		Vec3 offset;
		Vec3 object_pos;
		Quat object_q;
	};
	SEndPointLinks m_linkedObjects[2];
	i32            m_endLinksDisplayUpdateCounter;

private:
	void UpdateAudioData();
	void UpdateRopeLinks();

	struct SRopeAudioData
	{
		SRopeAudioData() = default;

		string                   startTriggerName;
		string                   stopTriggerName;
		string                   angleParameterName;
		DrxAudio::EOcclusionType occlusionType = DrxAudio::EOcclusionType::Ignore;
		i32                      segementToAttachTo = 1;
		float                    offset = 0.0f;
	};

	SRopeAudioData m_ropeAudioData;
};

/*!
 * Class Description of CVisAreaShapeObject.
 */
class CRopeObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType          GetObjectType()     { return OBJTYPE_OTHER; };
	tukk         ClassName()         { return "Rope"; };
	tukk         Category()          { return "Misc"; };
	virtual tukk GetTextureIcon()    { return "%EDITOR%/ObjectIcons/rope.bmp"; };
	CRuntimeClass*      GetRuntimeClass()   { return RUNTIME_CLASS(CRopeObject); };
};

