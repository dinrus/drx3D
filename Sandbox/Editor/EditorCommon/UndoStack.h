// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// TODO : bring both CUndoStack and CUndoRedo i32 a single consistent class

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "Expected.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <vector>

class EDITOR_COMMON_API CUndoStack
{
	typedef DynArray<char> DataBuffer;
	struct SUndoState
	{
		string             description;
		DataBuffer         state;
		zu64 sequentialIndex;
	};

public:
	void               PushUndo(DataBuffer* previousContentToMove, tukk description, zu64 sequentialIndex);
	bool               Undo(DataBuffer* newState, const DataBuffer& currentState, i32 count, zu64 sequentialIndex);
	bool               HasUndo() const         { return !m_undos.empty(); }
	bool               Redo(DataBuffer* newState, const DataBuffer& currentState, zu64 sequentialIndex);
	bool               HasRedo() const         { return !m_redos.empty(); }
	zu64 NewestUndoIndex() const { return m_undos.empty() ? 0 : m_undos.back().sequentialIndex; }
	zu64 NewestRedoIndex() const { return m_redos.empty() ? 0 : m_redos.back().sequentialIndex; }

	void               GetUndoActions(std::vector<string>* actionNames, i32 maxActionCount);
private:
	std::vector<SUndoState> m_undos;
	std::vector<SUndoState> m_redos;
};

//////////////////////////////////////////////////////////////////////////

struct IOperator
{
	virtual void Undo() = 0;
	virtual void Redo() = 0;
};

typedef std::shared_ptr<IOperator> PBaseOperator;

class EDITOR_COMMON_API CUndoRedo
{
public:
	CUndoRedo();

	void AddOperator(PBaseOperator op);
	void Undo();
	void Redo();

private:
	std::vector<PBaseOperator> m_undoOperators;
	std::vector<PBaseOperator> m_redoOperators;
};

