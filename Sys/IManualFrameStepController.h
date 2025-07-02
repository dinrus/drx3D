// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

struct IProtocolBuilder;

struct IManualFrameStepController
{
	//! Add message sinks to receive network events
	virtual void DefineProtocols(IProtocolBuilder* pBuilder) = 0;
};