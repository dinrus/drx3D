// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "HyperGraph/HyperGraphNode.h"

class CCommentBoxNode : public CHyperNode
{
public:
	static tukk GetClassType()
	{
		return "_commentbox";
	}
	CCommentBoxNode();
	~CCommentBoxNode();

	// CHyperNode
	void                          Init() override {}
	CHyperNode*                   Clone() override;
	virtual void                  Serialize(XmlNodeRef& node, bool bLoading, CObjectArchive* ar) override;

	virtual bool                  IsEditorSpecialNode() const override { return true; }
	virtual bool                  IsFlowNode() const override          { return false; }
	virtual bool                  IsTooltipShowable() const override   { return false; }
	virtual tukk           GetDescription() const override      { return "Comment with a colored box."; }
	virtual tukk           GetInfoAsString() const override     { return "Class: CommentBox"; }

	virtual void                  OnZoomChange(float zoom) override;
	virtual void                  OnInputsChanged() override;
	virtual void                  SetPos(Gdiplus::PointF pos) override;

	virtual const Gdiplus::RectF* GetResizeBorderRect() const override { return &m_resizeBorderRect; }
	virtual void                  SetResizeBorderRect(const Gdiplus::RectF& newRelBordersRect) override;
	// ~CHyperNode

	virtual void SetBorderRect(const Gdiplus::RectF& newAbsBordersRect);

private:
	void OnPossibleSizeChange();

	Gdiplus::RectF m_resizeBorderRect;
};

