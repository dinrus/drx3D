// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "IPropertyTreeWidget.h"

PROPERTY_TREE_API CPropertyTreeWidgetFactory& GetPropertyTreeWidgetFactory()
{
	static CPropertyTreeWidgetFactory sFactory;
	return sFactory;
}

