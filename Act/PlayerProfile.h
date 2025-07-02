// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PLAYERPROFILE_H__
#define __PLAYERPROFILE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

//#include <drx3D/Act/IPlayerProfiles.h>
#include "PlayerProfileUpr.h"

class CPlayerProfile : public IPlayerProfile
{
public:
	static tukk ATTRIBUTES_TAG; // "Attributes";
	static tukk ACTIONMAPS_TAG; // "ActionMaps";
	static tukk VERSION_TAG;    // "Version";

	typedef std::map<string, TFlowInputData, std::less<string>, stl::STLGlobalAllocator<std::pair<const string, TFlowInputData>>> TAttributeMap;

	CPlayerProfile(CPlayerProfileUpr* pUpr, tukk name, tukk userId, bool bIsPreview = false);
	virtual ~CPlayerProfile();

	// IPlayerProfile
	virtual bool Reset();

	// is this the default profile? it cannot be modified
	virtual bool IsDefault() const;

	// override values with console player profile defaults
	void LoadGamerProfileDefaults();

	// name of the profile
	virtual tukk GetName();

	// Id of the profile user
	virtual tukk GetUserId();

	// retrieve an action map
	virtual IActionMap* GetActionMap(tukk name);

	// set the value of an attribute
	virtual bool SetAttribute(tukk name, const TFlowInputData& value);

	// re-set attribute to default value (basically removes it from this profile)
	virtual bool ResetAttribute(tukk name);

	//delete an attribute from attribute map (regardless if has a default)
	virtual void DeleteAttribute(tukk name);

	// get the value of an attribute. if not specified optionally lookup in default profile
	virtual bool GetAttribute(tukk name, TFlowInputData& val, bool bUseDefaultFallback = true) const;

	// get all attributes available
	// all in this profile and inherited from default profile
	virtual IAttributeEnumeratorPtr CreateAttributeEnumerator();

	// save game stuff
	virtual ISaveGameEnumeratorPtr CreateSaveGameEnumerator();
	virtual ISaveGame*             CreateSaveGame();
	virtual ILoadGame*             CreateLoadGame();
	virtual bool                   DeleteSaveGame(tukk name);
	virtual ILevelRotationFile*    GetLevelRotationFile(tukk name);
	// ~IPlayerProfile

	bool                 SerializeXML(CPlayerProfileUpr::IProfileXMLSerializer* pSerializer);

	void                 SetName(tukk name);
	void                 SetUserId(tukk userId);

	const TAttributeMap& GetAttributeMap() const
	{
		return m_attributeMap;
	}

	const TAttributeMap& GetDefaultAttributeMap() const;

	void                 GetMemoryStatistics(IDrxSizer* s);

protected:
	bool LoadAttributes(const XmlNodeRef& root, i32 requiredVersion);
	bool SaveAttributes(const XmlNodeRef& root);

	friend class CAttributeEnumerator;

	CPlayerProfileUpr* m_pUpr;
	string                 m_name;
	string                 m_userId;
	TAttributeMap          m_attributeMap;
	i32                    m_attributesVersion;
	bool                   m_bIsPreview;
};

#endif
