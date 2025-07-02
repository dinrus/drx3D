// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "UndoEntityParam.h"
#include "EntityObject.h"

CUndoEntityParam::CUndoEntityParam(tukk pObjectName, tukk pVarPath, tukk pUndoDescription)
{
	m_entityName = pObjectName;
	m_paramName = pVarPath;
	m_undo = PyGetEntityParam(pObjectName, pVarPath);
	m_undoDescription = pUndoDescription;

}

tukk CUndoEntityParam::GetDescription()
{
	return m_undoDescription;
}

void CUndoEntityParam::Undo(bool bUndo)
{
	if (bUndo)
	{
		m_redo = PyGetEntityParam(m_entityName, m_paramName);
	}
	PySetEntityParam(m_entityName, m_paramName, m_undo);
}

void CUndoEntityParam::Redo()
{
	PySetEntityParam(m_entityName, m_paramName, m_redo);
}

