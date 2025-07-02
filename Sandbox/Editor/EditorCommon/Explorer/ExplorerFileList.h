// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ExplorerDataProvider.h"
#include "EntryList.h"
#include <drx3D/Sys/File/IFileChangeMonitor.h>
#include <drx3D/CoreX/smartptr.h>

namespace Explorer
{

struct ActionContext;

struct IEntryLoader : _i_reference_target_t
{
	virtual ~IEntryLoader() {}
	virtual bool Load(EntryBase* entry, tukk filename) = 0;
	virtual bool Save(EntryBase* entry, tukk filename) = 0;
};

struct IEntryDependencyExtractor : _i_reference_target_t
{
	virtual ~IEntryDependencyExtractor() {}
	virtual void Extract(vector<string>* paths, const EntryBase* entry) = 0;
};

struct EDITOR_COMMON_API JSONLoader : IEntryLoader
{
	bool Load(EntryBase* entry, tukk filename) override;
	bool Save(EntryBase* entry, tukk filename) override;
};

template<class TSelf>
struct SelfLoader : IEntryLoader
{
	bool Load(EntryBase* entry, tukk filename) override
	{
		return ((SEntry<TSelf>*)entry)->content.Load();
	}
	bool Save(EntryBase* entry, tukk filename) override
	{
		return ((SEntry<TSelf>*)entry)->content.Save();
	}
};

template<class T>
struct CEntryLoader : IEntryLoader
{
	bool Load(EntryBase* entry, tukk filename) override
	{
		return ((SEntry<T>*)entry)->content.Load(filename);
	}
	bool Save(EntryBase* entry, tukk filename) override
	{
		return ((SEntry<T>*)entry)->content.Save(filename);
	}
};

struct ScanLoadedFile;

enum FormatUsage
{
	FORMAT_LOAD = 1 << 0,
	FORMAT_SAVE = 1 << 1,
	FORMAT_LIST = 1 << 2,
	FORMAT_MAIN = 1 << 3
};

struct EntryFormat
{
	string                   extension;
	string                   path;
	i32                      usage;
	_smart_ptr<IEntryLoader> loader;

	string                   MakeFilename(tukk entryPath) const;

	EntryFormat()
		: usage()
	{
	}
};
typedef vector<EntryFormat> EntryFormats;

struct EntryType
{
	EntryListBase*                        entryList;
	bool                                  fileListenerRegistered;
	_smart_ptr<IEntryDependencyExtractor> dependencyExtractor;
	EntryFormats                          formats;
	EntryType() : entryList(nullptr), fileListenerRegistered(false) {}

	EntryType& AddFormat(tukk extension, IEntryLoader* loader, i32 usage = FORMAT_LIST | FORMAT_LOAD | FORMAT_SAVE)
	{
		EntryFormat format;
		format.extension = extension;
		format.usage = usage;
		format.loader.reset(loader);
		formats.push_back(format);
		return *this;
	}
	EntryType& AddFile(tukk path)
	{
		EntryFormat format;
		format.path = path;
		format.usage = FORMAT_LIST | FORMAT_LOAD | FORMAT_SAVE;
		formats.push_back(format);
		return *this;
	}
};

class EDITOR_COMMON_API ExplorerFileList : public IExplorerEntryProvider, public IFileChangeListener
{
	Q_OBJECT
public:
	ExplorerFileList() : m_explorerData(0), m_addPakColumn(false), m_explorerPakColumn(-1), m_dataIcon("") {}
	~ExplorerFileList();

	void                         AddPakColumn()                { m_addPakColumn = true; }
	void                         SetDataIcon(tukk icon) { m_dataIcon = icon; }

	template<class T> EntryType& AddEntryType(IEntryDependencyExtractor* extractor = 0)
	{
		return AddEntryType(new CEntryList<T>(), extractor);
	}
	template<class T> SEntry<T>* AddSingleFile(tukk path, tukk label, IEntryLoader* loader)
	{
		EntryType* f = AddSingleFileEntryType(new CEntryList<T>, path, label, loader);
		return ((CEntryList<T>*)f->entryList)->AddEntry(0, path, label, false);
	}

	ExplorerEntryId              AddEntry(tukk filename, bool resetIfExists);
	bool                         AddAndSaveEntry(tukk filename);
	EntryBase*                   GetEntryBaseByPath(tukk path);
	EntryBase*                   GetEntryBaseById(uint id);
	template<class T> SEntry<T>* GetEntryByPath(tukk path) { return static_cast<SEntry<T>*>(GetEntryBaseByPath(path)); }
	template<class T> SEntry<T>* GetEntryById(uint id)            { return static_cast<SEntry<T>*>(GetEntryBaseById(id)); }
	void                         Populate();

	// IExplorerDataProvider:
	i32         GetEntryCount() const override;
	uint        GetEntryIdByIndex(i32 index) const override;
	bool        GetEntrySerializer(Serialization::SStruct* out, uint id) const override;
	void        UpdateEntry(ExplorerEntry* entry) override;
	void        RevertEntry(uint id) override;
	string      GetSaveFilename(uint id) override;

	bool        SaveEntry(ActionOutput* output, uint id) override;
	bool        SaveAll(ActionOutput* output) override;
	void        SetExplorerData(ExplorerData* explorerData, i32 subtree) override;
	void        CheckIfModified(uint id, tukk reason, bool continuousChange) override;
	void        GetEntryActions(vector<ExplorerAction>* actions, uint id, ExplorerData* explorerData) override;
	bool        HasBackgroundLoading() const override;
	bool        LoadOrGetChangedEntry(uint id) override;
	tukk CanonicalExtension() const override;
	void        GetDependencies(vector<string>* paths, uint id) override;
	// ^^^

	void       OnFileChange(tukk filename, EChangeType eType) override;

	static i32 GetFilePakState(tukk path);

public slots:
	void             OnBackgroundFileLoaded(const ScanLoadedFile&);
	void             OnBackgroundLoadingFinished();
signals:
	void             SignalEntryDeleted(tukk path);
private:
	string           GetSaveFileExtension(tukk entryPath) const;
	void             ActionSaveAs(ActionContext& x);
	void             ActionDelete(ActionContext& x);
	bool             CanBeSaved(tukk path) const;

	EntryBase*       GetEntry(uint id, const EntryType** format = 0) const;
	EntryType*       GetEntryTypeByExtension(tukk ext);
	const EntryType* GetEntryTypeByExtension(tukk ext) const;
	EntryType*       GetEntryTypeByPath(tukk singleFile);
	const EntryType* GetEntryTypeByPath(tukk singleFile) const;

	EntryType& AddEntryType(EntryListBase* list, IEntryDependencyExtractor* extractor);
	EntryType* AddSingleFileEntryType(EntryListBase* list, tukk path, tukk label, IEntryLoader* loader);

	void       UpdateEntryPakState(const ExplorerEntryId& id, i32 pakState);
	void       UpdateEntryPakState(const ExplorerEntryId& id, tukk filename, const EntryType* format = 0);

	vector<EntryType> m_entryTypes;
	ExplorerData*     m_explorerData;
	bool              m_addPakColumn;
	i32               m_explorerPakColumn;
	tukk       m_dataIcon;
};

}

