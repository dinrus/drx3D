// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Core/Model.h"
#include "Util/ElementSet.h"
#include "ToolFactory.h"

class CBaseObject;

namespace Designer
{

class Model;
class ModelCompiler;

class DesignerUndo : public IUndoObject
{
public:
	DesignerUndo() : m_Tool(eDesigner_Invalid) {}
	DesignerUndo(CBaseObject* pObj, const Model* pModel, tukk undoDescription = NULL);

	static bool           IsAKindOfDesignerTool(CEditTool* pEditor);
	static Model*         GetModel(const DrxGUID& objGUID);
	static ModelCompiler* GetCompiler(const DrxGUID& objGUID);
	static CBaseObject*   GetBaseObject(const DrxGUID& objGUID);
	static MainContext    GetMainContext(const DrxGUID& objGUID);
	static void           RestoreEditTool(Model* pModel, DrxGUID objGUID, EDesignerTool tool);

protected:

	tukk GetDescription()
	{
		return m_undoDescription;
	};

	void SetDescription(tukk description)
	{
		if (description)
			m_undoDescription = description;
	}

	MainContext GetMainContext() const
	{
		return GetMainContext(m_ObjGUID);
	}

	void RestoreEditTool(Model* pModel)
	{
		RestoreEditTool(pModel, m_ObjGUID, m_Tool);
	}

	virtual void Undo(bool bUndo);
	virtual void Redo();

	void         StoreEditorTool();
	void         SwitchTool(EDesignerTool tool) { m_Tool = tool; }
	void         SetObjGUID(DrxGUID guid)       { m_ObjGUID = guid; }
private:
	string           m_undoDescription;
	DrxGUID           m_ObjGUID;
	EDesignerTool     m_Tool;
	_smart_ptr<Model> m_undo;
	Matrix34          m_UndoWorldTM;
	_smart_ptr<Model> m_redo;
	Matrix34          m_RedoWorldTM;
};

class UndoSelection : public DesignerUndo
{
public:

	UndoSelection(ElementSet& selectionContext, CBaseObject* pObj, bool bRetoreAfterUndo, tukk undoDescription = NULL);

	void Undo(bool bUndo);
	void Redo();

private:

	void CopyElements(ElementSet& sourceElements, ElementSet& destElements);
	void ReplacePolygonsWithExistingPolygonsInModel(ElementSet& elements);

	bool       m_bRestoreToolAfterUndo;

	ElementSet m_SelectionContextForUndo;
	ElementSet m_SelectionContextForRedo;

};

class UndoTextureMapping : public IUndoObject
{
public:
	UndoTextureMapping(){}
	UndoTextureMapping(CBaseObject* pObj, tukk undoDescription = NULL) : m_ObjGUID(pObj->GetId())
	{
		SetDescription(undoDescription);
		SaveDesignerTexInfoContext(m_UndoContext);
	}

protected:
	tukk GetDescription()
	{
		return m_undoDescription;
	};

	void SetDescription(tukk description)
	{
		if (description)
			m_undoDescription = description;
	}

	void Undo(bool bUndo);
	void Redo();
	void SetObjGUID(DrxGUID guid) { m_ObjGUID = guid; }

private:

	struct ContextInfo
	{
		DrxGUID m_PolygonGUID;
		TexInfo m_TexInfo;
		i32     m_MatID;
	};

	void RestoreTexInfo(const std::vector<ContextInfo>& contextList);
	void SaveDesignerTexInfoContext(std::vector<ContextInfo>& contextList);

	string                  m_undoDescription;
	DrxGUID                  m_ObjGUID;

	std::vector<ContextInfo> m_UndoContext;
	std::vector<ContextInfo> m_RedoContext;
};

class UndoSubdivision : public IUndoObject
{
public:
	UndoSubdivision(){}
	UndoSubdivision(const MainContext& mc);

	void        Undo(bool bUndo) override;
	void        Redo() override;
	tukk GetDescription() override { return "Designer Subdivision"; }

private:

	void UpdatePanel();

	DrxGUID m_ObjGUID;
	i32     m_SubdivisionLevelForUndo;
	i32     m_SubdivisionLevelForRedo;
};

}

