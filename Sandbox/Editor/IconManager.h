// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct IStatObj;
struct IMaterial;

#include "IIconManager.h"

/*!
 *	CIconManager contains map of icon names to icon textures,
 *	Ensuring that only one instance of texture for specified icon will be allocated.
 *	Also release textures when editor exit.
 *	Note: This seems like it could be improved, why does the icon manager have to store objects?
 */
class CIconManager : public IIconManager, public IAutoEditorNotifyListener
{
public:
	// Construction
	CIconManager();
	~CIconManager();

	void Init();
	void Done();

	// Unload all loaded resources.
	void Reset();

	// Operations
	virtual i32        GetIconTexture(EIcon icon);

	virtual IStatObj*  GetObject(EStatObject object);
	virtual i32        GetIconTexture(tukk szIconName);
	virtual i32        GetIconTexture(tukk szIconName, DrxIcon& icon);

	virtual IMaterial* GetHelperMaterial();

	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

	virtual void OnNewDocument()   { Reset(); };
	virtual void OnLoadDocument()  { Reset(); };
	virtual void OnCloseDocument() { Reset(); };
	virtual void OnMissionChange() { Reset(); };
	//////////////////////////////////////////////////////////////////////////

private:
	std::unordered_map<string, i32, stl::hash_strcmp<string>>  m_textures;

	_smart_ptr<IMaterial> m_pHelperMtl;

	IStatObj*             m_objects[eStatObject_COUNT];
	i32                   m_icons[eIcon_COUNT];

	//////////////////////////////////////////////////////////////////////////
	// Icons bitmaps.
	//////////////////////////////////////////////////////////////////////////
	typedef std::map<string, CBitmap*> IconsMap;
	IconsMap m_iconBitmapsMap;
};

