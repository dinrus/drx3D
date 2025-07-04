// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   the set variable action, sets the value of a specified variable to to a specific value
   can be used for any variable-collection (global, local, context)
   a "set-operation" can be specified, to not only set a new value, but to increment/decrement the old value

   /************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseAction.h>
#include "Variable.h"

namespace DrxDRS
{
class CVariableCollection;

class CActionSetVariable final : public IVariableUsingBase, public DRS::IResponseAction
{
public:
	enum EChangeOperation
	{
		eChangeOperation_Set       = 0,
		eChangeOperation_Increment = 1,
		eChangeOperation_Decrement = 2
	};

	CActionSetVariable() : m_changeOperation(eChangeOperation_Set), m_cooldown(0.0f) {}
	CActionSetVariable(const CHashedString& pCollection, const CHashedString& variableName, CVariableValue targetValue, EChangeOperation operation, float cooldown);

	//////////////////////////////////////////////////////////
	// IResponseAction implementation
	virtual DRS::IResponseActionInstanceUniquePtr Execute(DRS::IResponseInstance* pResponseInstance) override;
	virtual string                                GetVerboseInfo() const override;
	virtual void                                  Serialize(Serialization::IArchive& ar) override;
	virtual tukk                           GetType() const override { return "Change Variable"; }
	//////////////////////////////////////////////////////////

private:
	CVariableValue   m_valueToSet;
	EChangeOperation m_changeOperation;
	float            m_cooldown;
};
} //endns DrxDRS
