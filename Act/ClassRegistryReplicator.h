// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CLASSREGISTRYREPLICATOR_H__
#define __CLASSREGISTRYREPLICATOR_H__

#pragma once

class CClassRegistryReplicator
{
public:
	CClassRegistryReplicator() { Reset(); }

	bool   RegisterClassName(const string& name, u16 id);
	bool   ClassNameFromId(string& name, u16 id) const;
	bool   ClassIdFromName(u16& id, const string& name) const;
	size_t NumClassIds() { return m_classIdToName.size(); }
	void   Reset();
	u32 GetHash();
	void   DumpClasses();

	void   GetMemoryStatistics(IDrxSizer* s) const;

private:
	u16                   m_numClassIds;
	std::vector<string>      m_classIdToName;
	std::map<string, u16> m_classNameToId;
};

#endif
