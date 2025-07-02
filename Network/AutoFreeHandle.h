// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __AUTOFREEHANDLE_H__
#define __AUTOFREEHANDLE_H__

#pragma once

class CAutoFreeHandle
{
public:
	CAutoFreeHandle(TMemHdl& hdl) : m_hdl(hdl) {}
	~CAutoFreeHandle()
	{
		MMM().FreeHdl(m_hdl);
		m_hdl = CMementoMemoryUpr::InvalidHdl;
	}

	TMemHdl Grab()
	{
		TMemHdl out = m_hdl;
		m_hdl = CMementoMemoryUpr::InvalidHdl;
		return out;
	}

	TMemHdl Peek()
	{
		return m_hdl;
	}

private:
	TMemHdl& m_hdl;
};

#endif
