// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "MaterialImageListCtrl.h"
#include "MaterialManager.h"
#include "FilePathUtil.h"

#define ME_BG_TEXTURE "%EDITOR%/Materials/Stripes.dds"

BEGIN_MESSAGE_MAP(CMaterialImageListCtrl, CImageListCtrl)
ON_WM_CREATE()
ON_WM_SIZE()
ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
i32 CMaterialImageListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	i32 nRes = __super::OnCreate(lpCreateStruct);

	CRect rc(0, 0, m_itemSize.cx, m_itemSize.cy);
	m_renderCtrl.Create(this, rc, WS_CHILD);
	m_renderCtrl.SetGrid(false);
	m_renderCtrl.SetAxis(false);
	m_nColor = -1;
	m_nModel = EMMT_Default;
	LoadModel();
	m_renderCtrl.SetBackgroundTexture(ME_BG_TEXTURE);
	m_renderCtrl.SetClearColor(ColorF(0, 0, 0));
	return nRes;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::OnSize(UINT nType, i32 cx, i32 cy)
{
	m_itemSize = CSize(cy - 8, cy - 8);
	InvalidateAllBitmaps();

	__super::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
CMaterialImageListCtrl::CMaterialImageListCtrl()
{
	//m_pStatObj = gEnv->p3DEngine->LoadStatObj( "%EDITOR%/Objects/Sphere.cgf" );
	m_bkgrBrush.DeleteObject();
	m_bkgrBrush.CreateSysColorBrush(COLOR_3DFACE);

	if (gEnv->pRenderer)
	{
		gEnv->pRenderer->AddAsyncTextureCompileListener(this);
		gEnv->pRenderer->SetTextureStreamListener(this);
	}
}

//////////////////////////////////////////////////////////////////////////
CMaterialImageListCtrl::~CMaterialImageListCtrl()
{
	if (gEnv->pRenderer)
	{
		gEnv->pRenderer->RemoveAsyncTextureCompileListener(this);
		gEnv->pRenderer->SetTextureStreamListener(nullptr);
	}
}

//////////////////////////////////////////////////////////////////////////
CImageListCtrlItem* CMaterialImageListCtrl::AddMaterial(CMaterial* pMaterial, uk pUserData)
{
	CMtlItem* pItem = new CMtlItem;
	pItem->pMaterial = pMaterial;
	pItem->pUserData = pUserData;
	pItem->text = pMaterial->GetShortName();
	pMaterial->GetAnyTextureFilenames(pItem->vVisibleTextures);
	InsertItem(pItem);
	return pItem;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::SetMaterial(i32 nItemIndex, CMaterial* pMaterial, uk pUserData)
{
	assert(nItemIndex <= (i32)m_items.size());
	CMtlItem* pItem = (CMtlItem*)static_cast<CImageListCtrlItem*>(m_items[nItemIndex]);
	pItem->vVisibleTextures.clear();
	pItem->pMaterial = pMaterial;
	pItem->pUserData = pUserData;
	pItem->text = pMaterial->GetShortName();
	pMaterial->GetAnyTextureFilenames(pItem->vVisibleTextures);
	pItem->bBitmapValid = false;
	InvalidateItemRect(pItem);

	CRect rc;
	GetClientRect(rc);
	InvalidateRect(rc, FALSE);
}

//////////////////////////////////////////////////////////////////////////
CMaterialImageListCtrl::CMtlItem* CMaterialImageListCtrl::FindMaterialItem(CMaterial* pMaterial)
{
	Items items;
	GetAllItems(items);
	for (i32 i = 1; i < items.size(); i++) // 0 material is current preview not selectable
	{
		CImageListCtrlItem* pItem = items[i];
		CMtlItem* pMtlItem = (CMtlItem*)pItem;
		if (pMtlItem->pMaterial == pMaterial)
			return pMtlItem;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::SelectMaterial(CMaterial* pMaterial)
{
	ResetSelection();
	CMtlItem* pMtlItem = FindMaterialItem(pMaterial);
	if (pMtlItem)
	{
		SelectItem(pMtlItem);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::InvalidateMaterial(CMaterial* pMaterial)
{
	Items items;
	GetAllItems(items);
	for (i32 i = 0; i < items.size(); i++)
	{
		CImageListCtrlItem* pItem = items[i];
		CMtlItem* pMtlItem = (CMtlItem*)pItem;
		if (pMtlItem->pMaterial == pMaterial)
		{
			pMtlItem->vVisibleTextures.clear();
			pMaterial->GetAnyTextureFilenames(pMtlItem->vVisibleTextures);
			pMtlItem->bBitmapValid = false;
			InvalidateItemRect(pMtlItem);
		}
	}

	CRect rc;
	GetClientRect(rc);
	InvalidateRect(rc, FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::DeleteAllItems()
{
	m_renderCtrl.SetMaterial(nullptr);
	__super::DeleteAllItems();
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::OnUpdateItem(CImageListCtrlItem* pItem)
{
	// Make a bitmap from image.

	CMtlItem* pMtlItem = (CMtlItem*)pItem;

	CImageEx image;

	bool bPreview = false;
	if (pMtlItem->pMaterial)
	{
		const bool bTerrain = (0 == strcmp(pMtlItem->pMaterial->GetShaderName(), "Terrain.Layer"));

		if (!(pMtlItem->pMaterial->GetFlags() & MTL_FLAG_NOPREVIEW) || bTerrain)
		{
			if (!m_renderCtrl.GetObject())
				LoadModel();

			m_renderCtrl.MoveWindow(pItem->rect);
			m_renderCtrl.ShowWindow(SW_SHOW);
			if (bTerrain)
			{
				XmlNodeRef node = XmlHelpers::CreateXmlNode("Material");
				CBaseLibraryItem::SerializeContext ctx(node, false);
				pMtlItem->pMaterial->Serialize(ctx);

				if (!m_pMatPreview)
				{
					i32 flags = 0;
					if (node->getAttr("MtlFlags", flags))
					{
						flags |= MTL_FLAG_UIMATERIAL;
						node->setAttr("MtlFlags", flags);
					}
					m_pMatPreview = GetIEditorImpl()->GetMaterialManager()->CreateMaterial("_NewPreview_", node);
				}
				else
				{
					CBaseLibraryItem::SerializeContext ctx(node, true);
					m_pMatPreview->Serialize(ctx);
				}
				m_pMatPreview->SetShaderName("Illum");
				m_pMatPreview->Update();
				m_renderCtrl.SetMaterial(m_pMatPreview);
			}
			else
			{
				m_renderCtrl.SetMaterial(pMtlItem->pMaterial);
			}

			m_renderCtrl.GetImage(image);
			m_renderCtrl.ShowWindow(SW_HIDE);
		}
		bPreview = true;
	}

	if (!bPreview)
	{
		image.Allocate(pItem->rect.Width(), pItem->rect.Height());
		image.Clear();
	}

	u32* pImageData = image.GetData();
	i32 w = image.GetWidth();
	i32 h = image.GetHeight();

	if (pItem->bitmap.GetSafeHandle() != NULL)
		pItem->bitmap.DeleteObject();

	VERIFY(pItem->bitmap.CreateBitmap(w, h, 1, 32, pImageData));
	pItem->bBitmapValid = true;

	// ForceTexturesLoading() might trigger OnUploadedStreamedTexture() ASAP, which updates bBitmapValid
	if (CMaterial* pMat = static_cast<CMaterial*>(m_renderCtrl.GetMaterial()))
		pMat->GetMatInfo()->ForceTexturesLoading(std::max(w, h));
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::OnSelectItem(CImageListCtrlItem* pItem, bool bSelected)
{
	__super::OnSelectItem(pItem, bSelected);

	if (m_selectMtlFunc && bSelected && pItem)
		m_selectMtlFunc((CMtlItem*)pItem);
}

#define MENU_USE_DEFAULT   1
#define MENU_USE_BOX       2
#define MENU_USE_PLANE     3
#define MENU_USE_SPHERE    4
#define MENU_USE_TEAPOT    5
#define MENU_BG_BLACK      6
#define MENU_BG_GRAY       7
#define MENU_BG_WHITE      8
#define MENU_BG_TEXTURE    9
#define MENU_USE_BACKLIGHT 10

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::OnCompilationStarted(tukk source, tukk target, i32 nPending)
{
}

void CMaterialImageListCtrl::OnCompilationFinished(tukk source, tukk target, ERcExitCode eReturnCode)
{
	// update all previews who's texture(s) changed
	for (i32 i = 0; i < m_items.size(); ++i)
	{
		CImageListCtrlItem* pItem = m_items[i];
		CMtlItem* pMtlItem = (CMtlItem*)pItem;

		std::vector<string>::const_iterator founds = std::find(pMtlItem->vVisibleTextures.begin(), pMtlItem->vVisibleTextures.end(), source);
		std::vector<string>::const_iterator foundt = std::find(pMtlItem->vVisibleTextures.begin(), pMtlItem->vVisibleTextures.end(), target);
		if ((founds != pMtlItem->vVisibleTextures.end()) || (foundt != pMtlItem->vVisibleTextures.end()))
		{
			pMtlItem->bBitmapValid = false;
			InvalidateItemRect(pMtlItem);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::OnUploadedStreamedTexture(uk pHandle)
{
	tukk target = PathUtil::GamePathToDrxPakPath(((ITexture*)pHandle)->GetName()).c_str();
	tukk source = PathUtil::ReplaceExtension(target, "tif").c_str();

	// update all previews who's texture(s) changed
	for (i32 i = 0; i < m_items.size(); ++i)
	{
		CImageListCtrlItem* pItem = m_items[i];
		CMtlItem* pMtlItem = (CMtlItem*)pItem;

		std::vector<string>::const_iterator founds = std::find(pMtlItem->vVisibleTextures.begin(), pMtlItem->vVisibleTextures.end(), source);
		std::vector<string>::const_iterator foundt = std::find(pMtlItem->vVisibleTextures.begin(), pMtlItem->vVisibleTextures.end(), target);
		if ((founds != pMtlItem->vVisibleTextures.end()) || (foundt != pMtlItem->vVisibleTextures.end()))
		{
			pMtlItem->bBitmapValid = false;
			InvalidateItemRect(pMtlItem);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING | ((m_nModel == EMMT_Default) ? MF_CHECKED : 0), MENU_USE_DEFAULT, "Use Default Object");
	menu.AppendMenu(MF_STRING | ((m_nModel == EMMT_Plane) ? MF_CHECKED : 0), MENU_USE_PLANE, "Use Plane");
	menu.AppendMenu(MF_STRING | ((m_nModel == EMMT_Box) ? MF_CHECKED : 0), MENU_USE_BOX, "Use Box");
	menu.AppendMenu(MF_STRING | ((m_nModel == EMMT_Sphere) ? MF_CHECKED : 0), MENU_USE_SPHERE, "Use Sphere");
	menu.AppendMenu(MF_STRING | ((m_nModel == EMMT_Teapot) ? MF_CHECKED : 0), MENU_USE_TEAPOT, "Use Teapot");
	menu.AppendMenu(MF_SEPARATOR, 0, "");
	menu.AppendMenu(MF_STRING | ((m_nColor == 0) ? MF_CHECKED : 0), MENU_BG_BLACK, "Black Background");
	menu.AppendMenu(MF_STRING | ((m_nColor == 1) ? MF_CHECKED : 0), MENU_BG_GRAY, "Gray Background");
	menu.AppendMenu(MF_STRING | ((m_nColor == 2) ? MF_CHECKED : 0), MENU_BG_WHITE, "White Background");
	menu.AppendMenu(MF_STRING | ((m_nColor == -1) ? MF_CHECKED : 0), MENU_BG_TEXTURE, "Texture Background");
	menu.AppendMenu(MF_SEPARATOR, 0, "");
	menu.AppendMenu(MF_STRING | ((m_renderCtrl.UseBackLight()) ? MF_CHECKED : 0), MENU_USE_BACKLIGHT, "Use Back Light");

	ClientToScreen(&point);

	i32 cmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY, point.x, point.y, this);
	switch (cmd)
	{
	case MENU_USE_DEFAULT:
		m_nModel = EMMT_Default;
		LoadModel();
		break;
	case MENU_USE_PLANE:
		m_nModel = EMMT_Plane;
		LoadModel();
		break;
	case MENU_USE_BOX:
		m_nModel = EMMT_Box;
		LoadModel();
		break;
	case MENU_USE_SPHERE:
		m_nModel = EMMT_Sphere;
		LoadModel();
		break;
	case MENU_USE_TEAPOT:
		m_nModel = EMMT_Teapot;
		LoadModel();
		break;
	case MENU_BG_BLACK:
		m_renderCtrl.SetClearColor(ColorF(0, 0, 0));
		m_renderCtrl.SetBackgroundTexture("");
		m_nColor = 0;
		break;
	case MENU_BG_GRAY:
		m_renderCtrl.SetClearColor(ColorF(0.5f, 0.5f, 0.5f));
		m_renderCtrl.SetBackgroundTexture("");
		m_nColor = 1;
		break;
	case MENU_BG_WHITE:
		m_renderCtrl.SetClearColor(ColorF(1, 1, 1));
		m_renderCtrl.SetBackgroundTexture("");
		m_nColor = 2;
		break;
	case MENU_BG_TEXTURE:
		m_renderCtrl.SetBackgroundTexture(ME_BG_TEXTURE);
		m_nColor = -1;
		break;

	case MENU_USE_BACKLIGHT:
		m_renderCtrl.UseBackLight(!m_renderCtrl.UseBackLight());
		break;
	}
	InvalidateAllBitmaps();
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::LoadModel()
{
	switch (m_nModel)
	{
	case EMMT_Default:
		m_renderCtrl.LoadFile("%EDITOR%/Objects/MtlPlane.cgf");
		m_renderCtrl.SetCameraLookAt(1.0f, Vec3(0.0f, 0.0f, -1.0f));
		break;
	case EMMT_Sphere:
		m_renderCtrl.LoadFile("%EDITOR%/Objects/MtlSphere.cgf");
		m_renderCtrl.SetCameraLookAt(1.6f, Vec3(0.1f, -1.0f, -0.1f));
		break;
	case EMMT_Box:
		m_renderCtrl.LoadFile("%EDITOR%/Objects/MtlBox.cgf");
		m_renderCtrl.SetCameraRadius(1.6f);
		break;
	case EMMT_Teapot:
		m_renderCtrl.LoadFile("%EDITOR%/Objects/MtlTeapot.cgf");
		m_renderCtrl.SetCameraRadius(1.6f);
		break;
	case EMMT_Plane:
		m_renderCtrl.LoadFile("%EDITOR%/Objects/MtlPlane.cgf");
		m_renderCtrl.SetCameraLookAt(1.0f, Vec3(0.0f, 0.0f, -1.0f));
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMaterialImageListCtrl::CalcLayout(bool bUpdateScrollBar /*=true */)
{
	static bool bNoRecurse = false;
	if (bNoRecurse)
		return;
	bNoRecurse = true;

	CRect rc;
	GetClientRect(rc);

	rc.left += m_borderSize.cx;
	rc.right -= m_borderSize.cx;
	rc.top += m_borderSize.cy;
	rc.bottom -= m_borderSize.cy;

	i32 nBottomTextSize = 14;

	i32 cy = rc.Height();

	// First item is big.
	if (m_items.size() > 0)
	{
		CImageListCtrlItem* pItem = m_items[0];
		pItem->rect.left = rc.left;
		pItem->rect.top = rc.top;
		pItem->rect.right = rc.left + cy;
		pItem->rect.bottom = rc.bottom - nBottomTextSize;
		rc.left = pItem->rect.right + m_borderSize.cx * 2;
	}

	i32 cx = rc.Width() - m_borderSize.cx;

	i32 itemSize = cy;

	// Adjust all other bitmaps as tight as possible.
	u32 numItems = m_items.size() - 1;
	i32 div = 0;
	for (div = 1; div < 1000 && itemSize > 0; div++)
	{
		i32 nX = cx / (itemSize + 2);
		if (nX >= numItems)
			break;
		if (nX > 0)
		{
			i32 nY = numItems / nX + 1;
			if (nY * (itemSize + 2) < cy)
			{
				//itemSize = itemSize -= 2;
				break;
			}
		}
		itemSize = itemSize -= 2;
	}
	if (itemSize < 0)
		itemSize = 0;

	i32 x = rc.left;
	i32 y = rc.top;
	for (i32 i = 1; i < (i32)m_items.size(); i++)
	{
		CImageListCtrlItem* pItem = m_items[i];
		pItem->rect.left = x;
		pItem->rect.top = y;
		pItem->rect.right = x + itemSize;
		pItem->rect.bottom = y + itemSize - nBottomTextSize;
		x += itemSize + 2;
		if (x + itemSize >= rc.right)
		{
			x = rc.left;
			y += itemSize + 2;
		}
	}

	bNoRecurse = false;
}

