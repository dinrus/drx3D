// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AbstractNodeGraphViewModel.h"

namespace DrxGraphEditor {

CNodeGraphViewModel::~CNodeGraphViewModel()
{
	SignalDestruction();
}

}

