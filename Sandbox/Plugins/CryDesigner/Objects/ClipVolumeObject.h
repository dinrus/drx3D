// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Objects/EntityObject.h"
#include "Objects/DesignerBaseObject.h"

namespace Designer
{

class ClipVolumeObject : public DesignerBaseObject<CEntityObject>
{
public:
	DECLARE_DYNCREATE(ClipVolumeObject)
	ClipVolumeObject();
	virtual ~ClipVolumeObject();

	void Display(CObjectRenderHelper& objRenderHelper) override;

	bool Init(CBaseObject* prev, const string& file) override;
	void InitVariables() override {}
	void Serialize(CObjectArchive& ar) override;

	bool CreateGameObject() override;
	void EntityLinked(const string& name, DrxGUID targetEntityId);

	void LoadFromCGF();

	void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	ModelCompiler* GetCompiler() const override;

	void           GetLocalBounds(AABB& box) override;
	bool           HitTest(HitContext& hc) override;

	void           UpdateGameResource() override;
	string        GenerateGameFilename();

	void           PostLoad(CObjectArchive& ar) override;
	void           InvalidateTM(i32 nWhyFlags);

	void           ExportBspTree(IChunkFile* pChunkFile) const;

	void           OnEvent(ObjectEvent event) override;
	void           SetHidden(bool bHidden, bool bAnimated = false) override;
	bool           IsHiddenByOption() override;

	std::vector<EDesignerTool> GetIncompatibleSubtools() override;

	virtual void Reload(bool bReloadScript = false) override {}

private:
	void       UpdateCollisionData(const DynArray<Vec3>& meshFaces);
	void       OnPropertyChanged(IVariable* var);

	IStatObj*  GetIStatObj() override;
	void       DeleteThis() override { delete this; }

	CVariable<bool>      mv_filled;
	CVariable<bool>      mv_ignoreOutdoorAO;
	std::vector<Lineseg> m_MeshEdges;

	i32                  m_nUniqFileIdForExport;

	static i32           s_nGlobalFileClipVolumeID;
};

class ClipVolumeClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()            { return OBJTYPE_VOLUMESOLID; };
	tukk    ClassName()                { return "ClipVolume"; };
	tukk    Category()                 { return "Area"; };
	CRuntimeClass* GetRuntimeClass()          { return RUNTIME_CLASS(ClipVolumeObject); };
	tukk    GetTextureIcon()           { return "%EDITOR%/ObjectIcons/ClipVolume.bmp"; };
	virtual bool   RenderTextureOnTop() const { return true; }
	virtual tukk GetToolClassName() { return "EditTool.CreateClipVolumeTool"; }
};
}

