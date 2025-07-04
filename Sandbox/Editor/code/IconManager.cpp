// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "IconManager.h"

#include <Drx3DEngine/I3DEngine.h>

#include <gdiplus.h>

#define HELPER_MATERIAL "%EDITOR%/Objects/Helper"

namespace
{
// Object names in this array must correspond to EObject enumeration.
tukk g_ObjectNames[eStatObject_COUNT] =
{
	"%EDITOR%/Objects/Arrow.cgf",
	"%EDITOR%/Objects/Axis.cgf",
	"%EDITOR%/Objects/Sphere.cgf",
	"%EDITOR%/Objects/Anchor.cgf",
	"%EDITOR%/Objects/entrypoint.cgf",
	"%EDITOR%/Objects/hidepoint.cgf",
	"%EDITOR%/Objects/hidepoint_sec.cgf",
	"%EDITOR%/Objects/reinforcement_point.cgf",
};

tukk g_IconNames[eIcon_COUNT] =
{
	"%EDITOR%/Icons/ScaleWarning.png",
	"%EDITOR%/Icons/RotationWarning.png",
};
};

//////////////////////////////////////////////////////////////////////////
CIconManager::CIconManager()
{
	ZeroStruct(m_icons);
	ZeroStruct(m_objects);
}

//////////////////////////////////////////////////////////////////////////
CIconManager::~CIconManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CIconManager::Init()
{
}

//////////////////////////////////////////////////////////////////////////
void CIconManager::Done()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CIconManager::Reset()
{
	I3DEngine* pEngine = GetIEditorImpl()->Get3DEngine();
	// Do not unload objects. but clears them.
	i32 i;
	for (i = 0; i < sizeof(m_objects) / sizeof(m_objects[0]); i++)
	{
		if (m_objects[i] && pEngine)
			m_objects[i]->Release();
		m_objects[i] = 0;
	}
	for (i = 0; i < eIcon_COUNT; i++)
	{
		m_icons[i] = 0;
	}

	// Free icon bitmaps.
	for (IconsMap::iterator it = m_iconBitmapsMap.begin(); it != m_iconBitmapsMap.end(); ++it)
	{
		delete it->second;
	}
	m_iconBitmapsMap.clear();
}

//////////////////////////////////////////////////////////////////////////
i32 CIconManager::GetIconTexture(tukk iconName)
{
	auto textureIt = m_textures.find(iconName);
	if (textureIt != m_textures.end())
	{
		return textureIt->second;
	}

	i32 id = 0;

	CImageEx image;
	// Load icon.
	if (CImageUtil::LoadImage(iconName, image))
	{
		IRenderer* pRenderer(GetIEditorImpl()->GetRenderer());
		if (pRenderer->GetRenderType() != ERenderType::Direct3D11)
			image.SwapRedAndBlue();
		string ext = PathUtil::GetExt(iconName);
		if (stricmp(ext, "bmp") == 0 || stricmp(ext, "jpg") == 0)
		{
			i32 sz = image.GetWidth() * image.GetHeight();
			i32 h = image.GetHeight();
			u8* buf = (u8*)image.GetData();
			for (i32 i = 0; i < sz; i++)
			{
				u32 alpha = max(max(buf[i * 4], buf[i * 4 + 1]), buf[i * 4 + 2]);
				alpha *= 2;
				buf[i * 4 + 3] = (alpha > 255) ? 255 : alpha;
			}
		}

		id = pRenderer->UploadToVideoMemory((u8*)image.GetData(), image.GetWidth(), image.GetHeight(), eTF_R8G8B8A8, eTF_R8G8B8A8, 0, 0, 0);
		m_textures[iconName] = id;
	}
	return id;
}

//////////////////////////////////////////////////////////////////////////
i32 CIconManager::GetIconTexture(EIcon icon)
{
	assert(icon >= 0 && icon < eIcon_COUNT);
	if (m_icons[icon])
		return m_icons[icon];

	m_icons[icon] = GetIconTexture(g_IconNames[icon]);
	return m_icons[icon];
}

//////////////////////////////////////////////////////////////////////////
i32 CIconManager::GetIconTexture(tukk szIconName, DrxIcon& icon)
{
	auto textureIt = m_textures.find(szIconName);
	if (textureIt != m_textures.end())
	{
		return textureIt->second;
	}

	QPixmap iconPixmap = icon.pixmap(QSize(256, 256));
	QImage image = iconPixmap.toImage();

	if (image.format() != QImage::Format_RGBA8888)
	{
		image = image.convertToFormat(QImage::Format_RGBA8888);
	}

	switch (image.format())
	{
		case QImage::Format_RGBA8888:
		{
			i32 textureId = gEnv->pRenderer->UploadToVideoMemory(image.bits(), image.width(), image.height(), eTF_R8G8B8A8, eTF_R8G8B8A8, 0, 0, 0);
			m_textures[szIconName] = textureId;
			return textureId;
		}
	}

	DRX_ASSERT_MESSAGE(false, "Tried to convert unknown Editor icon image format %i to texture!", image.format());
	return 0;
}

//////////////////////////////////////////////////////////////////////////
IMaterial* CIconManager::GetHelperMaterial()
{
	if (!m_pHelperMtl)
		m_pHelperMtl = GetIEditorImpl()->Get3DEngine()->GetMaterialManager()->LoadMaterial(HELPER_MATERIAL);
	return m_pHelperMtl;
};

void CIconManager::OnEditorNotifyEvent(EEditorNotifyEvent event)
{
	switch (event)
	{
	case eNotify_OnBeginNewScene:
	case eNotify_OnBeginSceneOpen:
	case eNotify_OnBeginSceneClose:
	case eNotify_OnMissionChange:
	case eNotify_OnBeginLoad:
		Reset();
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IStatObj* CIconManager::GetObject(EStatObject object)
{
	assert(object >= 0 && object < eStatObject_COUNT);

	if (m_objects[object])
		return m_objects[object];

	// Try to load this object.
	m_objects[object] = GetIEditorImpl()->Get3DEngine()->LoadStatObj(g_ObjectNames[object], NULL, NULL, false);
	if (!m_objects[object])
	{
		DrxLog("Error: Load Failed: %s", g_ObjectNames[object]);
	}
	m_objects[object]->AddRef();
	if (GetHelperMaterial())
		m_objects[object]->SetMaterial(GetHelperMaterial());
	return m_objects[object];
}

