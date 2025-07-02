// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QObject>
#include <DrxSystem/File/IFileChangeMonitor.h>
#include <drx3D/CoreX/Serialization/StringList.h>
#include <drx3D/CoreX/Serialization/Forward.h>

namespace CharacterTool
{

class SkeletonList : public QObject, public IFileChangeListener
{
	Q_OBJECT
public:
	SkeletonList();
	~SkeletonList();

	void                             Reset();
	bool                             Load();
	bool                             Save();
	bool                             HasSkeletonName(tukk skeletonName) const;
	string                           FindSkeletonNameByPath(tukk path) const;
	string                           FindSkeletonPathByName(tukk name) const;

	const Serialization::StringList& GetSkeletonNames() const { return m_skeletonNames; }

	void                             Serialize(Serialization::IArchive& ar);

signals:
	void SignalSkeletonListModified();
private:
	void OnFileChange(tukk sFilename, EChangeType eType) override;

	Serialization::StringList m_skeletonNames;
	struct SEntry
	{
		string alias;
		string path;

		void Serialize(Serialization::IArchive& ar);

		bool operator<(const SEntry& rhs) const { return alias == rhs.alias ? path < rhs.path : alias < rhs.alias; }
	};
	typedef std::vector<SEntry> SEntries;
	SEntries m_skeletonList;
};

}

