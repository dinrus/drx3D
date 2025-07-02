// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/HUDOnScreenMessageDef.h>
#include <drx3D/Game/HUDControllerInputIcons.h>
#include <drx3D/Act/IItemSystem.h>

SOnScreenMessageDef::SOnScreenMessageDef()
{
	m_pInputRenderInfo = new CControllerInputRenderInfo;
	m_lifespan = 0.f;
}

SOnScreenMessageDef::SOnScreenMessageDef(const SOnScreenMessageDef& _in)
{
	m_pInputRenderInfo = new CControllerInputRenderInfo;
	*m_pInputRenderInfo = *(_in.m_pInputRenderInfo);
	m_lifespan = _in.m_lifespan;
	m_onScreenMessageText = _in.m_onScreenMessageText;
}

SOnScreenMessageDef::~SOnScreenMessageDef()
{
	SAFE_DELETE(m_pInputRenderInfo);
}

void SOnScreenMessageDef::operator=(const SOnScreenMessageDef & fromHere)
{
	m_lifespan = fromHere.m_lifespan;
	*m_pInputRenderInfo = *(fromHere.m_pInputRenderInfo);
	m_onScreenMessageText = fromHere.m_onScreenMessageText;

//DrxLog ("[HUD MESSAGE DEFINITION] Copied: '%s' lifespan=%f (type %d, '%s') vanish=%u", m_onScreenMessageText.c_str(), m_lifespan, m_inputRenderInfo.GetType(), m_inputRenderInfo.GetText(), m_vanishSettings);
}

void SOnScreenMessageDef::Read(const XmlNodeRef xml)
{
	// Read on-screen message text
	m_onScreenMessageText = xml->getAttr("display");

	// Read prompt icon information
	tukk  inputMapName = xml->getAttr("inputMapName");
	tukk  inputName = xml->getAttr("inputName");
	DRX_ASSERT_MESSAGE((inputMapName == NULL) == (inputName == NULL), string().Format("Provided %s", inputMapName ? "inputMapName but not inputName" : "inputName but not inputMapName"));
	if ((inputMapName != NULL) && (inputMapName[0]) && (inputName != NULL) && inputName[0])
	{
		m_pInputRenderInfo->CreateForInput(inputMapName, inputName);
	}

	// Read lifespan
	xml->getAttr("lifespan", m_lifespan);

//DrxLog ("[HUD MESSAGE DEFINITION] Read from XML: '%s' lifespan=%f (type %d, '%s') vanish=%u", m_onScreenMessageText.c_str(), m_lifespan, m_inputRenderInfo.GetType(), m_inputRenderInfo.GetText(), m_vanishSettings);
}
