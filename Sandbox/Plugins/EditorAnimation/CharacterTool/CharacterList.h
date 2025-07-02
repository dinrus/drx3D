// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QtCore/QObject>
#include "Pointers.h"
#include <drx3D/CoreX/Math/Drx_Math.h>

#include <memory>
#include <vector>
#include <map>
#include "../Shared/AnimSettings.h"
#include <DrxSystem/File/IFileChangeMonitor.h>
#include "SkeletonParameters.h"
#include "CharacterDefinition.h"
#include "Explorer/EntryList.h"
#include "Explorer/ExplorerDataProvider.h"
#include "Explorer/ExplorerFileList.h"

struct IAnimationSet;

namespace CharacterTool {

using namespace Explorer;

struct CharacterContent
{
	enum EngineLoadState
	{
		CHARACTER_NOT_LOADED,
		CHARACTER_LOADED,
		CHARACTER_INCOMPLETE,
		CHARACTER_LOAD_FAILED
	}                   engineLoadState;

	CharacterDefinition cdf;
	bool                hasDefinitionFile;

	CharacterContent() : hasDefinitionFile(true), engineLoadState(CHARACTER_NOT_LOADED) {}

	void Reset()
	{
		cdf = CharacterDefinition();
	}

	void GetDependencies(vector<string>* paths) const;
	void Serialize(Serialization::IArchive& ar);
};

struct CDFDependencies : IEntryDependencyExtractor
{
	void Extract(vector<string>* paths, const EntryBase* entry) override;
};

struct CHRParamsLoader : IEntryLoader
{
	bool Load(EntryBase* entry, tukk filename) override;
	bool Save(EntryBase* entry, tukk filename) override;
};

struct CDFLoader : IEntryLoader
{
	bool Load(EntryBase* entry, tukk filename) override;
	bool Save(EntryBase* entry, tukk filename) override;
};

struct CGALoader : IEntryLoader
{
	bool Load(EntryBase* entry, tukk filename) override;
	bool Save(EntryBase* entry, tukk filename) override;
};

struct CHRParamsDependencies : IEntryDependencyExtractor
{
	void Extract(vector<string>* paths, const EntryBase* entry) override;
};

}

