// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#ifndef __HUD_ONSCREENMESSAGEDEF_H__
#define __HUD_ONSCREENMESSAGEDEF_H__

struct CControllerInputRenderInfo;

struct SOnScreenMessageDef
{
	public:
	SOnScreenMessageDef();
	SOnScreenMessageDef(const SOnScreenMessageDef& _in);
	~SOnScreenMessageDef();
	void Read(const XmlNodeRef xml);
	void operator=(const SOnScreenMessageDef & fromHere);

	ILINE const bool empty() const
	{
		return m_onScreenMessageText.empty();
	}

	ILINE tukk  GetDisplayText() const
	{
		return m_onScreenMessageText.c_str();
	}

	ILINE const CControllerInputRenderInfo * GetInputRenderInfo() const
	{
		return m_pInputRenderInfo;
	}

	ILINE const float GetLifespan() const
	{
		return m_lifespan;
	}

	private:
	string                          m_onScreenMessageText;
	CControllerInputRenderInfo*     m_pInputRenderInfo;
	float                           m_lifespan;
};

#endif	// __HUD_ONSCREENMESSAGEDEF_H__
