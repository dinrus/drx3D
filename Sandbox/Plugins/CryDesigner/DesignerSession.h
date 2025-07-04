// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "StdAfx.h"

#include "DrxCore/smartptr.h"
#include "DrxSandbox/DrxSignal.h"

#include "Core/Common.h"
#include "Core/PolygonMesh.h"
#include "Core/Polygon.h"
#include "Tools/ToolCommon.h"

#include "IDataBaseManager.h"

#include "Util/ElementSet.h"
#include "Util/ExcludedEdgeManager.h"

class CBaseObject;

namespace Designer
{
class Model;
class ModelCompiler;

// This class stores some non-tool specific settings and also handles notifications between designer components
class DesignerSession : public IDataBaseManagerListener
{
public:
	~DesignerSession();
	static DesignerSession* GetInstance();

	Model*         GetModel() const;
	CBaseObject*   GetBaseObject() const;
	ModelCompiler* GetCompiler() const;
	MainContext    GetMainContext();
	void           SetBaseObject(CBaseObject* pBaseObject);

	// called when we have a selection of designer-only objects
	void BeginSession();
	void EndSession();
	bool GetIsActive();

	void OnEditorNotifyEvent(EEditorNotifyEvent event);

	i32  GetCurrentSubMatID() const;
	void SetCurrentSubMatID(i32 idx);

	void ClearCache();

	ElementSet* GetSelectedElements();
	void        ClearPolygonSelections();

	_smart_ptr<PolygonMesh> GetSelectionMesh();
	void UpdateSelectionMeshFromSelectedElements() { UpdateSelectionMeshFromSelectedElements(GetMainContext()); }
	void UpdateSelectionMeshFromSelectedElements(MainContext& mc);

	void ReleaseSelectionMesh();

	void UpdateSelectionMesh(
		PolygonPtr pPolygon,
		ModelCompiler* pCompiler,
		CBaseObject* pObj,
		bool bForce = false);

	void UpdateSelectionMesh(
		std::vector<PolygonPtr> polygons,
		ModelCompiler* pCompiler,
		CBaseObject* pObj,
		bool bForce = false);

	void OnDataBaseItemEvent(
		IDataBaseItem* pItem,
		EDataBaseItemEvent event) override;

	ExcludedEdgeManager* GetExcludedEdgeManager();

	// Get tools that are incompatible with this session
	std::vector<EDesignerTool> GetIncompatibleSubtools();

	CDrxSignal <void (EDesignerNotify event, TDesignerNotifyParam param)> signalDesignerEvent;

private:
	DesignerSession();
	static DesignerSession* s_session;

	_smart_ptr<CBaseObject> m_pBaseObject;
	i32 m_currentMaterialIntex;

	ExcludedEdgeManager m_pExcludedEdgeManager;
	ElementSet m_pSelectedElements;
	bool m_bActiveSession;
	bool m_bSessionFinished;

	_smart_ptr<PolygonMesh>     m_pSelectionMesh;
};

}
