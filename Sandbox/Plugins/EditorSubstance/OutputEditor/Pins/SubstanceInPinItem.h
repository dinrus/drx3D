// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SubstanceBasePinItem.h"
#include <NodeGraph/AbstractNodeItem.h>

namespace EditorSubstance
{
	namespace OutputEditor
	{

		class CSubstanceInPinItem : public CSubstanceBasePinItem
		{
		public:
			CSubstanceInPinItem(DrxGraphEditor::CAbstractNodeItem& nodeItem, EOutputPinType pinType)
				: CSubstanceBasePinItem(nodeItem, pinType)
			{}
			virtual ~CSubstanceInPinItem() {};
			virtual bool IsInputPin() const override { return true; }
			virtual bool IsOutputPin() const override { return false; }
		};
	}
}



