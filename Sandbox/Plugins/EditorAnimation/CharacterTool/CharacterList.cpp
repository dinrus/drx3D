// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include <DrxAnimation/IDrxAnimation.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include "CharacterList.h"
#include "SkeletonContent.h"
#include "Serialization.h"
#include <DrxSystem/File/IFileChangeMonitor.h>
#include "IEditor.h"
#include <IBackgroundTaskManager.h>
#include "GizmoSink.h"
#include "IResourceSelectorHost.h"
#include "ListSelectionDialog.h"
#include "DrxIcon.h"

namespace CharacterTool {

static bool LoadFile(vector<char>* buf, tukk filename)
{
	FILE* f = gEnv->pDrxPak->FOpen(filename, "rb");
	if (!f)
		return false;

	gEnv->pDrxPak->FSeek(f, 0, SEEK_END);
	size_t size = gEnv->pDrxPak->FTell(f);
	gEnv->pDrxPak->FSeek(f, 0, SEEK_SET);

	buf->resize(size);
	bool result = gEnv->pDrxPak->FRead(&(*buf)[0], size, f) == size;
	gEnv->pDrxPak->FClose(f);
	return result;
}

tukk   GetFilename(tukk path);

static string GetFilenameBase(tukk path)
{
	tukk name = GetFilename(path);
	tukk ext = strrchr(name, '.');
	if (ext != 0)
		return string(name, ext);
	else
		return string(name);
}

bool CHRParamsLoader::Load(EntryBase* entryBase, tukk filename)
{
	SEntry<SkeletonContent>* entry = static_cast<SEntry<SkeletonContent>*>(entryBase);

	XmlNodeRef root = GetIEditor()->GetSystem()->LoadXmlFromFile(filename);
	if (root)
		return entry->content.skeletonParameters.LoadFromXML(root, &entry->dataLostDuringLoading);
	else
		return false;
}

bool CHRParamsLoader::Save(EntryBase* entryBase, tukk filename)
{
	SEntry<SkeletonContent>* entry = static_cast<SEntry<SkeletonContent>*>(entryBase);

	XmlNodeRef root = entry->content.skeletonParameters.SaveToXML();
	if (!root)
		return false;

	char path[IDrxPak::g_nMaxPath] = "";
	gEnv->pDrxPak->AdjustFileName(filename, path, 0);
	if (!root->saveToFile(path))
		return false;

	return true;
}

// ---------------------------------------------------------------------------

void CHRParamsDependencies::Extract(vector<string>* paths, const EntryBase* entryBase)
{
	const SEntry<SkeletonContent>* entry = static_cast<const SEntry<SkeletonContent>*>(entryBase);

	entry->content.GetDependencies(&*paths);
}

// ---------------------------------------------------------------------------

bool CDFLoader::Load(EntryBase* entryBase, tukk filename)
{
	SEntry<CharacterContent>* entry = static_cast<SEntry<CharacterContent>*>(entryBase);
	return entry->content.cdf.LoadFromXmlFile(filename);
}

bool CDFLoader::Save(EntryBase* entryBase, tukk filename)
{
	SEntry<CharacterContent>* entry = static_cast<SEntry<CharacterContent>*>(entryBase);
	if (!entry->content.cdf.Save(filename))
		return false;
	return true;
}

// ---------------------------------------------------------------------------

void CDFDependencies::Extract(vector<string>* paths, const EntryBase* entryBase)
{
	const SEntry<CharacterContent>* entry = static_cast<const SEntry<CharacterContent>*>(entryBase);

	entry->content.GetDependencies(&*paths);
}

// ---------------------------------------------------------------------------

bool CGALoader::Load(EntryBase* entryBase, tukk filename)
{
	SEntry<CharacterContent>* entry = static_cast<SEntry<CharacterContent>*>(entryBase);
	entry->content.cdf = CharacterDefinition();
	entry->content.cdf.skeleton = filename;
	entry->content.hasDefinitionFile = false;
	return true;
}

bool CGALoader::Save(EntryBase* entry, tukk filename)
{
	return false;
}

// ---------------------------------------------------------------------------

void CharacterContent::GetDependencies(vector<string>* paths) const
{
	paths->push_back(PathUtil::ReplaceExtension(cdf.skeleton, ".chrparams"));
}

void CharacterContent::Serialize(Serialization::IArchive& ar)
{
	switch (engineLoadState)
	{
	case CHARACTER_NOT_LOADED:
		ar.warning(*this, "Selected character is different from the one in the viewport.");
		break;
	case CHARACTER_INCOMPLETE:
		ar.warning(*this, "An incomplete character cannot be loaded by the engine.");
		break;
	case CHARACTER_LOAD_FAILED:
		if (!hasDefinitionFile)
			ar.error(*this, "Failed to load character in the engine.");
		else
			ar.error(*this, "Failed to load character in the engine. Check if specified skeleton is valid.");
		break;
	default:
		break;
	}

	if (!hasDefinitionFile)
	{
		string msg = "Contains no properties.";
		ar(msg, "msg", "<!");
		return;
	}

	cdf.Serialize(ar);
}

// ---------------------------------------------------------------------------
dll_string AttachmentNameSelector(const SResourceSelectorContext& x, tukk previousValue, ICharacterInstance* characterInstance)
{
	if (!characterInstance)
		return previousValue;

	ListSelectionDialog dialog("CTAttachmentSelection", x.parentWidget);
	dialog.setWindowTitle("Attachment Selection");
	dialog.setWindowIcon(DrxIcon(GetIEditor()->GetResourceSelectorHost()->GetSelector(x.typeName)->GetIconPath()));
	dialog.setModal(true);

	IAttachmentManager* attachmentManager = characterInstance->GetIAttachmentManager();
	if (!attachmentManager)
		return previousValue;

	dialog.SetColumnText(0, "Name");
	dialog.SetColumnText(1, "Type");

	for (i32 i = 0; i < attachmentManager->GetAttachmentCount(); ++i)
	{
		IAttachment* attachment = attachmentManager->GetInterfaceByIndex(i);
		if (!attachment)
			continue;

		dialog.AddRow(attachment->GetName());
		i32 type = attachment->GetType();
		tukk typeString = type == CA_BONE ? "Bone" :
		                         type == CA_SKIN ? "Skin" :
		                         type == CA_FACE ? "Face" : "";
		dialog.AddRowColumn(typeString);
	}

	return dialog.ChooseItem(previousValue);
}
REGISTER_RESOURCE_SELECTOR("Attachment", AttachmentNameSelector, "icons:common/animation_attachement.ico")

}

