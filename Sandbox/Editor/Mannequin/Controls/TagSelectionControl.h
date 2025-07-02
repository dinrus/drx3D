// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __TAG_SELECTION_CONTROL__H__
#define __TAG_SELECTION_CONTROL__H__

#include <drx3D/Act/IDrxMannequin.h>
#include "Controls/PropertyCtrl.h"

class CTagSelectionControl
	: public CXTResizeDialog
{
	DECLARE_DYNCREATE(CTagSelectionControl)

public:
	CTagSelectionControl();
	virtual ~CTagSelectionControl();

	void                  SetTagDef(const CTagDefinition* pTagDef);
	const CTagDefinition* GetTagDef() const;

	CVarBlockPtr          GetVarBlock() const;

	TagState              GetTagState() const;
	void                  SetTagState(const TagState tagState);

	typedef Functor1<CTagSelectionControl*> OnTagStateChangeCallback;
	void SetOnTagStateChangeCallback(OnTagStateChangeCallback onTagStateChangeCallback);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK()     {}
	virtual void OnCancel() {}

	void         Reset();

	void         OnInternalVariableChange(IVariable* pVar);
	void         OnTagStateChange();

private:
	const CTagDefinition*                m_pTagDef;

	CVarBlockPtr                         m_pVarBlock;
	std::vector<CSmartVariable<bool>>    m_tagVarList;
	std::vector<CSmartVariableEnum<i32>> m_tagGroupList;

	CPropertyCtrl                        m_propertyControl;

	bool                     m_ignoreInternalVariableChange;

	OnTagStateChangeCallback m_onTagStateChangeCallback;
};

#endif

