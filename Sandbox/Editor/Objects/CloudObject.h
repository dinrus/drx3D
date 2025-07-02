// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CloudObject_h__
#define __CloudObject_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "Objects/BaseObject.h"

class CCloudObject : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CCloudObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	void          Display(CObjectRenderHelper& objRenderHelper);
	void          GetLocalBounds(AABB& box);
	bool          HitTest(HitContext& hc);

	virtual float GetCreationOffsetFromTerrain() const override { return 0.f; }
	
	virtual void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	//////////////////////////////////////////////////////////////////////////
	// Cloud parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<i32>   mv_spriteRow;
	CVariable<float> m_width;
	CVariable<float> m_height;
	CVariable<float> m_length;
	CVariable<float> m_angleVariations;
	CVariable<float> mv_sizeSprites;
	CVariable<float> mv_randomSizeValue;

protected:
	CCloudObject();

	void DeleteThis() { delete this; };
	void OnSizeChange(IVariable* pVar);

	bool m_bNotSharedGeom;
	bool m_bIgnoreNodeUpdate;
};

/*!
 * Class Description of CCloudObject.
 */
class CCloudObjectClassDesc : public CObjectClassDesc
{
public:
	ObjectType     GetObjectType()     { return OBJTYPE_CLOUD; };
	tukk    ClassName()         { return "CloudVolume"; };
	tukk    Category()          { return ""; };
	CRuntimeClass* GetRuntimeClass()   { return RUNTIME_CLASS(CCloudObject); };
	tukk    GetFileSpec()       { return ""; };
};

#endif // __CloudObject_h__

