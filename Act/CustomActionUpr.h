// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _CUSTOMACTIONMANAGER_H_
#define _CUSTOMACTIONMANAGER_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/FlowGraph/IFlowSystem.h>
#include <drx3D/Act/ICustomActions.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include "CustomAction.h"

///////////////////////////////////////////////////
// CCustomActionUpr keeps track of all CustomActions
///////////////////////////////////////////////////
class CCustomActionUpr : public ICustomActionUpr
{
public:
	CCustomActionUpr();
	virtual ~CCustomActionUpr();

public:
	// ICustomActionUpr
	virtual bool           StartAction(IEntity* pObject, tukk szCustomActionGraphName, ICustomActionListener* pListener = NULL);
	virtual bool           SucceedAction(IEntity* pObject, tukk szCustomActionGraphName, ICustomActionListener* pListener = NULL);
	virtual bool           SucceedWaitAction(IEntity* pObject);
	virtual bool           SucceedWaitCompleteAction(IEntity* pObject);
	virtual bool           AbortAction(IEntity* pObject);
	virtual bool           EndAction(IEntity* pObject, bool bSuccess);
	virtual void           LoadLibraryActions(tukk sPath);
	virtual void           ClearActiveActions();
	virtual void           ClearLibraryActions();
	virtual size_t         GetNumberOfCustomActionsFromLibrary() const { return m_actionsLib.size(); }
	virtual ICustomAction* GetCustomActionFromLibrary(tukk szCustomActionGraphName);
	virtual ICustomAction* GetCustomActionFromLibrary(const size_t index);
	virtual size_t         GetNumberOfActiveCustomActions() const;
	virtual ICustomAction* GetActiveCustomAction(const IEntity* pObject);
	virtual ICustomAction* GetActiveCustomAction(const size_t index);
	virtual bool           UnregisterListener(ICustomActionListener* pEventListener);
	virtual void           Serialize(TSerialize ser);
	// ~ICustomActionUpr

	// Removes deleted Action from the list of active actions
	void Update();

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CustomActionUpr");

		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_actionsLib);
		pSizer->AddObject(m_activeActions);
	}

protected:
	// Adds an Action in the list of active actions
	ICustomAction* AddActiveCustomAction(IEntity* pObject, tukk szCustomActionGraphName, ICustomActionListener* pListener = NULL);

	// Called when entity is removed
	void OnEntityRemove(IEntity* pEntity);

private:
	// Library of all defined Actions
	typedef std::map<string, CCustomAction> TCustomActionsLib;
	TCustomActionsLib m_actionsLib;

	// List of all active Actions (including suspended and to be deleted)
	typedef std::list<CCustomAction> TActiveActions;
	TActiveActions m_activeActions;
};

#endif
