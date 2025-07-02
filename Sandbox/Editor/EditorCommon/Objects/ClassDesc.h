// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001-2012
// -------------------------------------------------------------------------
//  File name:   ClassDesc.h
//  Created:     8/11/2001 by Timur.
//  Description: Class description of CBaseObject
//
////////////////////////////////////////////////////////////////////////////
#include "IDataBaseManager.h"
#include "ObjectEvent.h"
#include "IPlugin.h"
#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include <DrxEntitySystem/IEntitySystem.h>
#include <DrxEntitySystem/IEntityClass.h>

class CXmlArchive;
class IObjectEnumerator;
struct IParticleEffect;

struct SPreviewDesc
{
	SPreviewDesc() : pParticleEffect(nullptr) {}

	string           modelFile;
	string           materialFile;
	IParticleEffect* pParticleEffect;
};

//! Virtual base class description of CBaseObject.
//! Override this class to create specific Class descriptions for every base object class.
//! Type name is specified like this:
//! Category\Type ex: "TagPoint\Respawn"

//TODO: this is temporary marker for new file specs that aren't implemented yet
static tukk not_implemented = "not_implemented";
class EDITOR_COMMON_API CObjectClassDesc : public IClassDesc, public IDataBaseManagerListener
{
public:
	CObjectClassDesc()
	{
		m_nTextureIcon = 0;
	}

	//! Release class description.
	virtual ObjectType GetObjectType() = 0;
	//! Create instance of editor object.
	virtual CObject*   Create()
	{
		return GetRuntimeClass()->CreateObject();
	}
	//! Get MFC runtime class for this object.
	virtual CRuntimeClass* GetRuntimeClass() = 0;
	//! If this function return not empty string,object of this class must be created with file.
	//! Return root path where to look for files this object supports.
	//! Also wild card for files can be specified, ex: Objects\*.cgf
	virtual tukk GetFileSpec()
	{
		return "";
	}

	virtual tukk GetDataFilesFilterString()                       { return ""; }

	virtual void        EnumerateObjects(IObjectEnumerator* pEnumerator) {}
	virtual void        RegisterAsDataBaseListener(EDataBaseItemType type)
	{
		if (!bRegistered)
		{
			GetIEditor()->GetDBItemManager(type)->AddListener(this);
			bRegistered = true;
		}
	}
	virtual bool           IsCreatedByListEnumeration() { return true; }
	// Get a class that will supercede the current class if this class is a legacy class
	virtual tukk    GetSuccessorClassName()      { return ""; }
	virtual void           OnDataBaseItemEvent(IDataBaseItem* pItem, EDataBaseItemEvent event) override;
	virtual void           OnDataBaseLibraryEvent(IDataBaseLibrary* pLibrary, EDataBaseLibraryEvent event) override;
	virtual void           OnDataBaseEvent(EDataBaseEvent event) override;

	virtual ESystemClassID SystemClassID()            { return ESYSTEM_CLASS_OBJECT; };
	virtual void           Serialize(CXmlArchive& ar) {};
	virtual tukk    GetTextureIcon()           { return 0; };
	i32                    GetTextureIconId();
	virtual bool           RenderTextureOnTop() const { return false; }
	//! For backward compatibility, we want to register some classes (so they are properly loaded)
	//but not enable creation in create pane.
	virtual bool         IsCreatable() const                  { return true; }

	virtual tukk  GetToolClassName()                   { return "EditTool.ObjectCreate"; }

	virtual tukk  UIName()                             { return ClassName(); };

	virtual bool         IsPreviewable() const                { return false; }
	virtual SPreviewDesc GetPreviewDesc(tukk id) const { static SPreviewDesc empty; return empty; }

	// Checks whether an entity class with this class name exists and is exposed to designers
	// Defaults to checking what is returned by ClassName if szClassName is empty
	bool                 IsEntityClassAvailable(tukk szClassName = "") const
	{
		if (IEntityClass* pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(szClassName[0] != '\0' ? szClassName : const_cast<CObjectClassDesc*>(this)->ClassName()))
		{
			return (pClass->GetFlags() & ECLF_INVISIBLE) == 0;
		}

		return false;
	}

private:
	i32 m_nTextureIcon;

protected:
	bool bRegistered;

public:
	CDrxSignal<void(tukk , tukk , tukk )> m_itemAdded;
	CDrxSignal<void(tukk , tukk , tukk )> m_itemChanged;
	CDrxSignal<void(tukk , tukk )>              m_itemRemoved;
	CDrxSignal<void(tukk)>                           m_libraryRemoved;
	CDrxSignal<void()>                                      m_databaseCleared;
};

