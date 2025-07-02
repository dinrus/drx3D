// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   this action sends a new signal, which then might cause a new response

   /************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseAction.h>

namespace DrxDRS
{
class CActionSendSignal final : public DRS::IResponseAction
{
public:
	CActionSendSignal() : m_signalName(), m_bCopyContextVariables(false) {}
	CActionSendSignal(const CHashedString& signalName, bool copyContext) : m_signalName(signalName), m_bCopyContextVariables(copyContext){}

	//////////////////////////////////////////////////////////
	// IResponseAction implementation
	virtual DRS::IResponseActionInstanceUniquePtr Execute(DRS::IResponseInstance* pResponseInstance) override;
	virtual string                                GetVerboseInfo() const override { return m_signalName.GetText(); }
	virtual void                                  Serialize(Serialization::IArchive& ar) override;
	virtual tukk                           GetType() const override        { return "Send Signal"; }
	//////////////////////////////////////////////////////////

private:
	CHashedString m_signalName;
	bool          m_bCopyContextVariables;
};

//For this action response we do not need an instance, because it is an instant effect
}
