// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SubstanceBasePinItem.h"
#include <NodeGraph/AbstractNodeItem.h>

namespace EditorSubstance
{
	namespace OutputEditor
	{


		class CSubstanceOutPinItem : public CSubstanceBasePinItem
		{
		public:
			CSubstanceOutPinItem(DrxGraphEditor::CAbstractNodeItem& nodeItem, EOutputPinType pinType)
				: CSubstanceBasePinItem(nodeItem, pinType)
			{}
			virtual ~CSubstanceOutPinItem() {}
			virtual bool IsInputPin() const override { return false; }
			virtual bool IsOutputPin() const override { return true; }
			virtual tukk GetStyleId() const override
			{
				return "Pin::Substance";
			}
		};


	}
}


