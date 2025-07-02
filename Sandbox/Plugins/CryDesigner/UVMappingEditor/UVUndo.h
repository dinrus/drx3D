// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "UVElement.h"
#include "Core/UVIslandManager.h"

namespace Designer {
namespace UVMapping
{

class Model;

class UVIslandUndo : public IUndoObject
{
public:
	UVIslandUndo();

	tukk GetDescription() override { return "UV Mapping Editor : Edit"; }
	void        Undo(bool bUndo) override;
	void        Redo() override;

protected:
	struct UndoData
	{
		_smart_ptr<UVIslandManager> pUVIslandMgr;
		i32                         subMatID;
	};

	void Apply(const UndoData& undoData);
	void Record(DrxGUID& objGUID, UndoData& undoData);

	DrxGUID  m_ObjGUID;
	UndoData m_undoData;
	UndoData m_redoData;
};

class UVProjectionUndo : public IUndoObject
{
public:
	UVProjectionUndo();

	tukk GetDescription() override { return "UV Mapping Editor : UVProjectionUndo"; }
	void        Undo(bool bUndo) override;
	void        Redo() override;

private:

	void SavePolygons(std::vector<PolygonPtr>& polygons);
	void UndoPolygons(const std::vector<PolygonPtr>& polygons);

	DrxGUID                 m_ObjGUID;
	std::vector<PolygonPtr> m_UndoPolygons;
	std::vector<PolygonPtr> m_RedoPolygons;
};

class UVSelectionUndo : public IUndoObject
{
public:
	UVSelectionUndo();

	tukk GetDescription() override { return "UV Mapping Editor : Selection"; }
	void        Undo(bool bUndo) override;
	void        Redo() override;

private:

	void RefreshUVIslands(UVElementSetPtr pElementSet);
	void Apply(UVElementSetPtr pElementSet);

	DrxGUID         m_ObjGUID;
	UVElementSetPtr m_UndoSelections;
	UVElementSetPtr m_RedoSelections;
};

class UVMoveUndo : public IUndoObject
{
public:
	UVMoveUndo();

	tukk GetDescription() override { return "UV Mapping Editor : Move"; }
	void        Undo(bool bUndo) override;
	void        Redo() override;

private:
	void GatherInvolvedPolygons(std::vector<PolygonPtr>& outInvolvedPolygons);
	void GatherInvolvedPolygons(std::vector<PolygonPtr>& involvedPolygons, std::vector<PolygonPtr>& outInvolvedPolygons);
	void ApplyInvolvedPolygons(std::vector<PolygonPtr>& involvedPolygons);

	DrxGUID                 m_ObjGUID;
	std::vector<PolygonPtr> m_UndoPolygons;
	std::vector<PolygonPtr> m_RedoPolygons;
};

}
}

