// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Designer
{
class Model;

enum ECompilerFlag
{
	eCompiler_CastShadow  = BIT(1),
	eCompiler_Physicalize = BIT(2),
	eCompiler_General     = eCompiler_CastShadow | eCompiler_Physicalize
};

//! This class plays a role of creating engine resources used for rendering and physicalizing out of the Designer::Model instance.
class ModelCompiler : public _i_reference_target_t
{
public:
	ModelCompiler(i32 nCompilerFlag);
	ModelCompiler(const ModelCompiler& compiler);
	virtual ~ModelCompiler();

	bool         IsValid() const;

	void         Compile(CBaseObject* pBaseObject, Model* pModel, ShelfID shelfID = eShelf_Any, bool bUpdateOnlyRenderNode = false);

	void         DeleteAllRenderNodes();
	void         DeleteRenderNode(ShelfID shelfID);
	IRenderNode* GetRenderNode() { return m_pRenderNode[0]; }
	void         UpdateHighlightPassState(bool bSelected, bool bHighlighted);
	bool         GetIStatObj(_smart_ptr<IStatObj>* pStatObj);

	bool         CreateIndexdMesh(Model* pModel, IIndexedMesh* pMesh, bool bCreateBackFaces);
	void         SaveToCgf(tukk filename);

	void         SetViewDistRatio(i32 nViewDistRatio) { m_viewDistRatio = nViewDistRatio; }
	i32          GetViewDistRatio() const             { return m_viewDistRatio; }

	void         SetRenderFlags(i32 nRenderFlag)      { m_RenderFlags = nRenderFlag; }
	i32          GetRenderFlags() const               { return m_RenderFlags; }

	void         SetStaticObjFlags(i32 nStaticObjFlag);
	i32          GetStaticObjFlags();

	void         AddFlags(i32 nFlags)         { m_nCompilerFlag |= nFlags; }
	void         RemoveFlags(i32 nFlags)      { m_nCompilerFlag &= (~nFlags); }
	bool         CheckFlags(i32 nFlags) const { return (m_nCompilerFlag & nFlags) ? true : false; }

	void         SaveMesh(CArchive& ar, CBaseObject* pObj, Model* pModel);
	bool         LoadMesh(CArchive& ar, CBaseObject* pObj, Model* pModel);
	bool         SaveMesh(i32 nVersion, std::vector<char>& buffer, CBaseObject* pObj, Model* pModel);
	bool         LoadMesh(i32 nVersion, std::vector<char>& buffer, CBaseObject* pObj, Model* pModel);

private:

	bool       UpdateMesh(CBaseObject* pBaseObject, Model* pModel, ShelfID nShelf);
	void       UpdateRenderNode(CBaseObject* pBaseObject, ShelfID nShelf);

	void       RemoveStatObj(ShelfID nShelf);
	void       CreateStatObj(ShelfID nShelf);
	IMaterial* GetMaterialFromBaseObj(CBaseObject* pObj) const;
	void       InvalidateStatObj(IStatObj* pStatObj, bool bPhysics);

private:

	mutable IStatObj*    m_pStatObj[cShelfMax];
	mutable IRenderNode* m_pRenderNode[cShelfMax];

	i32                  m_RenderFlags;
	i32                  m_viewDistRatio;
	i32                  m_nCompilerFlag;
};
}

