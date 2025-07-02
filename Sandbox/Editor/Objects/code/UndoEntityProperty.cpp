// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "UndoEntityProperty.h"
#include "EntityObject.h"

CUndoEntityProperty::CUndoEntityProperty(tukk pEntityName, tukk pPropertyName, tukk pUndoDescription)
{
	m_entityName = pEntityName;
	m_propertyName = pPropertyName;
	m_undoDescription = pUndoDescription;
	m_undo = PyGetEntityProperty(pEntityName, pPropertyName);
}

tukk CUndoEntityProperty::GetDescription()
{
	return m_undoDescription;
}

void CUndoEntityProperty::Undo(bool bUndo)
{
	if (bUndo)
	{
		m_redo = PyGetEntityProperty(m_entityName, m_propertyName);
	}
	PySetEntityProperty(m_entityName, m_propertyName, m_undo);
}

void CUndoEntityProperty::Redo()
{
	PySetEntityProperty(m_entityName, m_propertyName, m_redo);
}

