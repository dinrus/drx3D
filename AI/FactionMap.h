// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IFactionMap.h>

class CFactionXmlDataSource : public IFactionDataSource
{
public:
	CFactionXmlDataSource(tukk szFileName)
		: m_fileName(szFileName)
	{}

	// IFactionDataSource
	virtual bool Load(IFactionMap& factionMap) override;
	// ~IFactionDataSource

private:
	string m_fileName;
};

class CFactionMap : public IFactionMap
{
	friend class CFactionXmlDataSource;

	enum { maxFactionCount = 32 };

	typedef std::unordered_map<u8, string>                                                       FactionNamesById;
	typedef std::unordered_map<string, u8, stl::hash_stricmp<string>, stl::hash_stricmp<string>> FactionIdsByName;

public:
	typedef IFactionMap::ReactionType EReaction;

	CFactionMap();

	// IFactionMap
	virtual u32      GetFactionCount() const override;
	virtual u32      GetMaxFactionCount() const override;

	virtual tukk GetFactionName(u8 factionId) const override;
	virtual u8       GetFactionID(tukk szName) const override;

	virtual u8       CreateOrUpdateFaction(tukk szName, u32 reactionsCount, u8k* pReactions) override;
	virtual void        RemoveFaction(tukk szName) override;

	virtual void        SetReaction(u8 factionOne, u8 factionTwo, IFactionMap::ReactionType reaction) override;
	virtual EReaction   GetReaction(u8 factionOne, u8 factionTwo) const override;

	virtual void        SetDataSource(IFactionDataSource* pDataSource, EDataSourceLoad bLoad) override;
	virtual void        RemoveDataSource(IFactionDataSource* pDataSource) override;

	virtual void        Reload() override;

	virtual void        RegisterFactionReactionChangedCallback(const FactionReactionChangedCallback& callback) override;
	virtual void        UnregisterFactionReactionChangedCallback(const FactionReactionChangedCallback& callback) override;
	// ~IFactionMap

	void Clear();
	void Serialize(TSerialize ser);

private:
	static bool GetReactionType(tukk szReactionName, EReaction* pReactionType);

	static CFactionXmlDataSource s_defaultXmlDataSource;

	IFactionDataSource*          m_pDataSource;
	FactionNamesById             m_namesById;
	FactionIdsByName             m_idsByName;
	u8                        m_reactions[maxFactionCount][maxFactionCount];
	CFunctorsList<FactionReactionChangedCallback> m_factionReactionChangedCallback;
};
