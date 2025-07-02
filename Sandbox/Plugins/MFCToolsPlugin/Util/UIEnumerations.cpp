// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "UIEnumerations.h"

//////////////////////////////////////////////////////////////////////////
CUIEnumerations& CUIEnumerations::GetUIEnumerationsInstance()
{
	static CUIEnumerations oGeneralProxy;
	return oGeneralProxy;
}

//////////////////////////////////////////////////////////////////////////
CUIEnumerations::TDValuesContainer& CUIEnumerations::GetStandardNameContainer()
{
	static TDValuesContainer cValuesContainer;
	static bool boInit(false);

	if (!boInit)
	{
		boInit = true;

		XmlNodeRef oRootNode;
		XmlNodeRef oEnumaration;
		XmlNodeRef oEnumerationItem;

		i32 nNumberOfEnumarations(0);
		i32 nCurrentEnumaration(0);

		i32 nNumberOfEnumerationItems(0);
		i32 nCurrentEnumarationItem(0);

		oRootNode = GetISystem()->GetXmlUtils()->LoadXmlFromFile("Editor\\PropertyEnumerations.xml");
		nNumberOfEnumarations = oRootNode ? oRootNode->getChildCount() : 0;

		for (nCurrentEnumaration = 0; nCurrentEnumaration < nNumberOfEnumarations; ++nCurrentEnumaration)
		{
			TDValues cValues;
			oEnumaration = oRootNode->getChild(nCurrentEnumaration);

			nNumberOfEnumerationItems = oEnumaration->getChildCount();
			for (nCurrentEnumarationItem = 0; nCurrentEnumarationItem < nNumberOfEnumerationItems; ++nCurrentEnumarationItem)
			{
				oEnumerationItem = oEnumaration->getChild(nCurrentEnumarationItem);

				tukk szKey(NULL);
				tukk szValue(NULL);
				oEnumerationItem->getAttributeByIndex(0, &szKey, &szValue);

				cValues.push_back(szValue);
			}

			tukk szKey(NULL);
			tukk szValue(NULL);
			oEnumaration->getAttributeByIndex(0, &szKey, &szValue);

			cValuesContainer.insert(TDValuesContainer::value_type(szValue, cValues));
		}
	}

	return cValuesContainer;
}
//////////////////////////////////////////////////////////////////////////

