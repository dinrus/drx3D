// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CUNDOENTITYPROPERTY__H__
#define __CUNDOENTITYPROPERTY__H__

#include "Util/BoostPythonHelpers.h"
#include "EntityObject.h"

class CUndoEntityProperty : public IUndoObject
{
public:
	CUndoEntityProperty(tukk pEntityName, tukk pPropertyName, tukk pUndoDescription = "Set Entity Property");

protected:
	tukk GetDescription();
	void        SetEntityProperty(SPyWrappedProperty& value);
	void        Undo(bool bUndo);
	void        Redo();

private:
	string            m_entityName;
	string            m_propertyName;
	SPyWrappedProperty m_undo;
	SPyWrappedProperty m_redo;
	tukk        m_undoDescription;
};

#endif // __CUNDOENTITYPROPERTY__H__

