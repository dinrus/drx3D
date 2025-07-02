// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CUNDOENTITYPARAM__H__
#define __CUNDOENTITYPARAM__H__

#include "Util/BoostPythonHelpers.h"
#include "EntityObject.h"

class CUndoEntityParam : public IUndoObject
{
public:
	CUndoEntityParam(tukk pObjectName, tukk pVarPath, tukk pUndoDescription = "Set Entity Param");

protected:
	tukk GetDescription();
	void        Undo(bool bUndo);
	void        Redo();

private:
	string            m_entityName;
	string            m_paramName;
	SPyWrappedProperty m_undo;
	SPyWrappedProperty m_redo;
	tukk        m_undoDescription;
};

#endif // __CUNDOENTITYPARAM__H__

