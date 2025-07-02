// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
class IImeUpr
{
public:
	virtual ~IImeUpr() {};
	// Check if IME is supported
	virtual bool IsImeSupported() = 0;

	// This is called by Scaleform in the case that IME support is compiled in
	// Returns false if IME should not be used
	virtual bool SetScaleformHandler(IWindowMessageHandler* pHandler) = 0;
};
