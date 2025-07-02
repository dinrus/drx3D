// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MaterialPickTool_h__
#define __MaterialPickTool_h__

#if _MSC_VER > 1000
	#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
class CMaterialPickTool : public CEditTool
{
public:
	DECLARE_DYNCREATE(CMaterialPickTool)

	CMaterialPickTool();

	//////////////////////////////////////////////////////////////////////////
	// CEditTool implementation
	//////////////////////////////////////////////////////////////////////////
	virtual string GetDisplayName() const override { return "Pick Material"; }
	virtual bool   MouseCallback(CViewport* view, EMouseEvent event, CPoint& point, i32 flags);
	virtual void   Display(DisplayContext& dc);
	//////////////////////////////////////////////////////////////////////////

protected:

	bool OnMouseMove(CViewport* view, UINT nFlags, CPoint point);
	void SetMaterial(IMaterial* pMaterial);

	virtual ~CMaterialPickTool();
	// Delete itself.
	void DeleteThis() { delete this; };

	_smart_ptr<IMaterial> m_pMaterial;
	string               m_displayString;
	CPoint                m_Mouse2DPosition;
	SRayHitInfo           m_HitInfo;
};

#endif // __MaterialPickTool_h__

