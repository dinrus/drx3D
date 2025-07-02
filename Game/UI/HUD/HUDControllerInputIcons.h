// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#ifndef __HUD_CONTROLLER_INPUT_ICONS_H__
#define __HUD_CONTROLLER_INPUT_ICONS_H__

enum eControllerInputTypeVisualization
{
	kCITV_none,
	kCITV_icon,
	kCITV_text
};

struct CControllerInputRenderInfo
{
	public:
	CControllerInputRenderInfo()
	{
		Clear();
	}

	void Clear();
	bool SetIcon(tukk  text);
	bool SetText(tukk  text);
	bool CreateForInput(tukk  mapName, tukk  inputName);
	void operator=(const CControllerInputRenderInfo & fromThis);
	
	ILINE eControllerInputTypeVisualization			GetType() const { return m_type; }
	ILINE tukk 													GetText() const { return m_text; }
	ILINE const ITexture *											GetTexture() const { return m_texture; }

	private:
	eControllerInputTypeVisualization			m_type;
	char																	m_text[32];
	ITexture *                            m_texture;

	static ITexture* GetTexture(tukk name);
};

#endif	// __HUD_CONTROLLER_INPUT_ICONS_H__