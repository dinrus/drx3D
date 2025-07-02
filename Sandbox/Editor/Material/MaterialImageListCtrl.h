// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MaterialImageListCtrl_h__
#define __MaterialImageListCtrl_h__
#pragma once

#include "Controls\ImageListCtrl.h"
#include "Controls\PreviewModelCtrl.h"
#include "Material.h"
#include <drx3D/CoreX/Renderer/IRenderer.h> // IAsyncTextureCompileListener

enum EMILC_ModelType
{
	EMMT_Default = 0,
	EMMT_Box,
	EMMT_Sphere,
	EMMT_Teapot,
	EMMT_Plane
};

//////////////////////////////////////////////////////////////////////////
class CMaterialImageListCtrl : public CImageListCtrl, private IAsyncTextureCompileListener, public ITextureStreamListener
{
public:
	typedef Functor1<CImageListCtrlItem*> SelectCallback;

	struct CMtlItem : public CImageListCtrlItem
	{
		_smart_ptr<CMaterial> pMaterial;
		std::vector<string>  vVisibleTextures;
	};

	CMaterialImageListCtrl();
	~CMaterialImageListCtrl();

	CImageListCtrlItem* AddMaterial(CMaterial* pMaterial, uk pUserData = NULL);
	void                SetMaterial(i32 nItemIndex, CMaterial* pMaterial, uk pUserData = NULL);
	void                SelectMaterial(CMaterial* pMaterial);
	CMtlItem*           FindMaterialItem(CMaterial* pMaterial);
	void                InvalidateMaterial(CMaterial* pMaterial);

	void                SetSelectMaterialCallback(SelectCallback func) { m_selectMtlFunc = func; };

	void                DeleteAllItems() override;

protected:
	afx_msg i32  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	void         LoadModel();
	virtual void OnSelectItem(CImageListCtrlItem* pItem, bool bSelected);
	virtual void OnUpdateItem(CImageListCtrlItem* pItem);
	virtual void CalcLayout(bool bUpdateScrollBar = true);

private:
	// Compile listener
	virtual void OnCompilationStarted(tukk source, tukk target, i32 nPending);
	virtual void OnCompilationFinished(tukk source, tukk target, ERcExitCode eReturnCode);

	virtual void OnCompilationQueueTriggered(i32 nPending) {}
	virtual void OnCompilationQueueDepleted() {}

	// Stream listener
	virtual void OnCreatedStreamedTexture(uk pHandle, tukk name, i32 nMips, i32 nMinMipAvailable) {}
	virtual void OnUploadedStreamedTexture(uk pHandle);
	virtual void OnDestroyedStreamedTexture(uk pHandle) {}

	virtual void OnTextureWantsMip(uk pHandle, i32 nMinMip) {}
	virtual void OnTextureHasMip(uk pHandle, i32 nMinMip) {}

	virtual void OnBegunUsingTextures(uk * pHandles, size_t numHandles) {}
	virtual void OnEndedUsingTextures(uk * pHandle, size_t numHandles) {}
private:
	_smart_ptr<CMaterial> m_pMatPreview;

	CPreviewModelCtrl     m_renderCtrl;
	IStatObj*             m_pStatObj;
	SelectCallback        m_selectMtlFunc;
	EMILC_ModelType       m_nModel;
	i32                   m_nColor;
};

#endif //__MaterialImageListCtrl_h__

