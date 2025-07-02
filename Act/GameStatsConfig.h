// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   GameStatsConfig - configurator for GameSpy Stats&Tracking service
   -------------------------------------------------------------------------
   История:
   - 9:9:2007   15:38 : Created by Stas Spivakov

*************************************************************************/

#ifndef __GAMESTATSCONFIG_H__
#define __GAMESTATSCONFIG_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Game/IGameFramework.h>

class CGameStatsConfig : public IGameStatsConfig
{
	struct SKeyDef
	{
		SKeyDef(i32 key, tukk group) : m_key(key), m_group(group){}
		i32    m_key;
		string m_group;
	};
	typedef std::map<string, SKeyDef> TKeyDefMap;

	struct SCategoryEntry
	{
		i32    m_code;
		string m_keyName;
		string m_display;
	};

	struct SCategory
	{
		string                      m_name;
		i32                         m_mod;
		std::vector<SCategoryEntry> m_entries;
	};

	typedef std::vector<SCategory> TCategoryVector;
public:
	CGameStatsConfig();
	~CGameStatsConfig();
	void ReadConfig();
	//////////////////////////////////////////////////////////////////////////
	// IGameStatsConfig
	virtual i32         GetStatsVersion();
	virtual i32         GetCategoryMod(tukk cat);
	virtual tukk GetValueNameByCode(tukk cat, i32 id);
	virtual i32         GetKeyId(tukk key) const;
	//////////////////////////////////////////////////////////////////////////
	i32                 GetCodeByKeyName(tukk cat, tukk key) const;
private:
	i32             m_version;
	TKeyDefMap      m_map;
	TCategoryVector m_categories;
};

#endif //__GAMESTATSCONFIG_H__
