// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "VariablesModel.h"

namespace DrxSchematycEditor {

u32 CAbstractVariablesModelItem::GetIndex() const
{
	return m_model.GetVariableItemIndex(*this);
}

}

