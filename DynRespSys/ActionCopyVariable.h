// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   A DRS action that copies the value of one variable to another variable

/************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseAction.h>
#include "Variable.h"

namespace DrxDRS
{
	class CActionCopyVariable final : public DRS::IResponseAction
	{
	public:
		enum EChangeOperation
		{
			eChangeOperation_Set = 0,
			eChangeOperation_Increment = 1,
			eChangeOperation_Decrement = 2,
			eChangeOperation_Swap = 3,
		};

		//////////////////////////////////////////////////////////
		// IResponseAction implementation
		virtual DRS::IResponseActionInstanceUniquePtr Execute(DRS::IResponseInstance* pResponseInstance) override;
		virtual string                                GetVerboseInfo() const override;
		virtual void                                  Serialize(Serialization::IArchive& ar) override;
		virtual tukk                           GetType() const override { return "Copy Variable"; }
		//////////////////////////////////////////////////////////

	private:
		IVariableUsingBase m_targetVariable;
		IVariableUsingBase m_sourceVariable;
		
		float m_cooldown = 0.0f;
		EChangeOperation m_changeOperation = eChangeOperation_Set;
	};
}
