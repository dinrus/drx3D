// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QtCore/QObject>

#include <memory>
#include <vector>
#include <map>
#include "../Shared/AnimSettings.h"
#include <drx3D/CoreX/Serialization/Forward.h>
#include <DrxSystem/File/IFileChangeMonitor.h>
#include <DrxAnimation/IDrxAnimation.h>
#include "Explorer/ExplorerDataProvider.h"
#include "Explorer/EntryList.h"
#include "AnimationContent.h"
#include "SkeletonParameters.h"

struct IAnimationSet;
struct ICharacterInstance;

namespace Explorer
{
struct ActionOutput;
struct ActionContext;
}

namespace CharacterTool
{
using namespace Explorer;

struct System;

class AnimationList : public IExplorerEntryProvider, public IFileChangeListener, public IAnimationSetListener
{
	Q_OBJECT
public:
	AnimationList(System* system);
	~AnimationList();

	void                      Populate(ICharacterInstance* character, tukk defaultSkeletonAlias, const AnimationSetFilter& filter, tukk animEventsFilename);
	void                      SetAnimationFilterAndScan(const AnimationSetFilter& filter);
	void                      SetAnimationEventsFilePath(const string& animationEventsFilePath);

	void                      RemoveImportEntry(tukk animationPath);

	bool                      IsLoaded(u32 id) const;
	bool                      ImportAnimation(string* errorMessage, u32 id);
	SEntry<AnimationContent>* GetEntry(u32 id) const;
	bool                      IsNewAnimation(u32 id) const;
	SEntry<AnimationContent>* FindEntryByPath(tukk animationPath);
	u32              FindIdByAlias(tukk animationName);
	bool                      ResaveAnimSettings(tukk filePath);

	bool                      SaveAnimationEntry(ActionOutput* output, u32 id, bool notifyOfChange);

	// IExplorerDataProvider:
	i32          GetEntryCount() const override;
	u32 GetEntryIdByIndex(i32 index) const override;
	bool         GetEntrySerializer(Serialization::SStruct* out, u32 id) const override;
	void         UpdateEntry(ExplorerEntry* entry) override;
	void         RevertEntry(u32 id) override;
	bool         SaveEntry(ActionOutput* output, u32 id) override;
	string       GetSaveFilename(u32 id) override;
	bool         SaveAll(ActionOutput* output) override;
	void         CheckIfModified(u32 id, tukk reason, bool continuousChange) override;
	void         GetEntryActions(vector<ExplorerAction>* actions, u32 id, ExplorerData* explorerData) override;
	bool         LoadOrGetChangedEntry(u32 id) override;
	void         SetExplorerData(ExplorerData* explorerData, i32 subtree) override;
	// ^^^

signals:
	void         SignalForceRecompile(tukk filepath);
private:
	void         ActionImport(ActionContext& x);
	void         ActionForceRecompile(ActionContext& x);
	void         ActionGenerateFootsteps(ActionContext& x);
	void         ActionExportHTR(ActionContext& x);
	void         ActionComputeVEG(ActionContext& x);
	void         ActionCopyExamplePaths(ActionContext& x);

	u32 MakeNextId();
	void         ReloadAnimationList();

	bool         UpdateImportEntry(SEntry<AnimationContent>* entry);
	void         UpdateAnimationEntryByPath(tukk filename);
	void         ScanForImportEntries(std::vector<std::pair<ExplorerEntryId, i32>>* pakColumnValues, bool resetFollows);
	void         SetEntryState(ExplorerEntry* entry, const vector<char>& state);
	void         GetEntryState(vector<char>* state, ExplorerEntry* entry);

	//////////////////////////////////////////////////////////
	// IAnimationSetListener implementation
	virtual void OnAnimationSetAddAnimation(tukk animationPath, tukk animationName) override;
	virtual void OnAnimationSetAboutToBeReloaded() override;
	virtual void OnAnimationSetReloaded() override;
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// IFileChangeListener implementation
	virtual void OnFileChange(tukk filename, EChangeType eType) override;
	//////////////////////////////////////////////////////////

	System*                      m_system;
	IAnimationSet*               m_animationSet;
	ICharacterInstance*          m_character;
	string                       m_defaultSkeletonAlias;
	string                       m_animEventsFilename;

	CEntryList<AnimationContent> m_animations;
	bool                         m_importEntriesInitialized;
	std::vector<string>          m_importEntries;
	typedef std::map<string, u32, stl::less_stricmp<string>> AliasToId;
	AliasToId                    m_aliasToId;

	AnimationSetFilter           m_filter;

	i32                          m_explorerColumnFrames;
	i32                          m_explorerColumnAudio;
	i32                          m_explorerColumnSize;
	i32                          m_explorerColumnPak;
	i32                          m_explorerColumnNew;

	string                       m_commonPrefix;

	bool                         m_bReloading;
};

}

