// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/DialogLineDatabase.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/DrxStrings.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Math/Random.h>

#if defined(DRS_COLLECT_DEBUG_DATA)
	#include <drx3D/DynRespSys/ResponseSystem.h>
#endif

namespace DrxDRS
{
typedef std::pair<CHashedString, CDialogLineSet> DialogLinePair;
typedef std::pair<string, CDialogLineSet>        SortedDialogLinePair;

struct DialogLinePairSerializer
{
	DialogLinePairSerializer(DrxDRS::DialogLinePair& pair) : pair_(pair) {}
	void Serialize(Serialization::IArchive& ar)
	{
		ar(pair_.first, "lineID", "^LineID");
		ar(pair_.second, "value", "^+[+]");
	}
	DrxDRS::DialogLinePair& pair_;
};
struct SortedDialogLinePairSerializer
{
	SortedDialogLinePairSerializer(DrxDRS::SortedDialogLinePair& pair) : pair_(pair) {}
	void Serialize(Serialization::IArchive& ar)
	{
		ar(pair_.first, "lineID", "^LineID");
		ar(pair_.second, "value", "^+[+]");
	}
	DrxDRS::SortedDialogLinePair& pair_;
};
bool Serialize(Serialization::IArchive& ar, DrxDRS::DialogLinePair& pair, tukk name, tukk label)
{
	DialogLinePairSerializer keyValue(pair);
	return ar(keyValue, name, label);
}
bool Serialize(Serialization::IArchive& ar, DrxDRS::SortedDialogLinePair& pair, tukk name, tukk label)
{
	SortedDialogLinePairSerializer keyValue(pair);
	return ar(keyValue, name, label);
}

tukk CDialogLineDatabase::s_szFilename = "dialoglines.dialog";

//--------------------------------------------------------------------------------------------------
CDialogLineDatabase::CDialogLineDatabase()
{
	m_drsDialogBinaryFileFormatCVar = 0;
	REGISTER_CVAR2("drs_dialogBinaryFileFormat", &m_drsDialogBinaryFileFormatCVar, 0, VF_NULL, "Toggles if the dialog data should be loaded from JSON or Binary.\n");
}

//--------------------------------------------------------------------------------------------------
CDialogLineDatabase::~CDialogLineDatabase()
{
	gEnv->pConsole->UnregisterVariable("drs_dialogBinaryFileFormat", true);
}

//--------------------------------------------------------------------------------------------------
void CDialogLineDatabase::Serialize(Serialization::IArchive& ar)
{
	ar(m_lineSets, "lineSets", "+[+]LineSets");
}

//--------------------------------------------------------------------------------------------------
bool CDialogLineDatabase::InitFromFiles(tukk szFilePath)
{
	string filepath = PathUtil::AddSlash(szFilePath);
	filepath += s_szFilename;
	if (m_drsDialogBinaryFileFormatCVar != 0)
	{
		if (!Serialization::LoadBinaryFile(*this, filepath.c_str()))
		{
			DrxWarning(VALIDATOR_MODULE_DRS, VALIDATOR_WARNING, "Could not load dialog file %s", filepath.c_str());
			return false;
		}
	}
	else
	{
		if (!Serialization::LoadJsonFile(*this, filepath.c_str()))
		{
			DrxWarning(VALIDATOR_MODULE_DRS, VALIDATOR_WARNING, "Could not load dialog file %s", filepath.c_str());
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------
bool CDialogLineDatabase::Save(tukk szFilePath)
{
	GetISystem()->GetIPak()->MakeDir(szFilePath);

	string filepath = PathUtil::AddSlash(szFilePath);
	filepath += s_szFilename;
	if (m_drsDialogBinaryFileFormatCVar != 0)
	{
		if (!Serialization::SaveBinaryFile(filepath.c_str(), *this))
		{
			DrxWarning(VALIDATOR_MODULE_DRS, VALIDATOR_WARNING, "Could not save dialog file %s", filepath.c_str());
			return false;
		}
	}
	else
	{
		if (!Serialization::SaveJsonFile(filepath.c_str(), *this))
		{
			DrxWarning(VALIDATOR_MODULE_DRS, VALIDATOR_WARNING, "Could not save dialog file %s", filepath.c_str());
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------
void CDialogLineDatabase::Reset()
{
	for (CDialogLineSet& currentLineSet : m_lineSets)
	{
		currentLineSet.Reset();
	}
}

//--------------------------------------------------------------------------------------------------
uint CDialogLineDatabase::GetLineSetCount() const
{
	return m_lineSets.size();
}

//--------------------------------------------------------------------------------------------------
DRS::IDialogLineSet* CDialogLineDatabase::GetLineSetByIndex(u32 index)
{
	if (index < m_lineSets.size())
	{
		return &m_lineSets[index];
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
CDialogLineSet* CDialogLineDatabase::GetLineSetById(const CHashedString& lineID)
{
	for (CDialogLineSet& currentLineSet : m_lineSets)
	{
		if (currentLineSet.GetLineId() == lineID)
		{
			return &currentLineSet;
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
DRS::IDialogLineSet* CDialogLineDatabase::InsertLineSet(u32 index)
{
	CDialogLineSet newSet;
	newSet.SetLineId(GenerateUniqueId("NEW_LINE"));

	if (index == (u32)-1)
	{
		m_lineSets.push_back(newSet);
		return &m_lineSets.back();
	}

	return &(*m_lineSets.insert(m_lineSets.begin() + index, newSet));
}

//--------------------------------------------------------------------------------------------------
bool CDialogLineDatabase::RemoveLineSet(u32 index)
{
	if (index < m_lineSets.size())
	{
		m_lineSets.erase(m_lineSets.begin() + index);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------
CHashedString CDialogLineDatabase::GenerateUniqueId(const string& root)
{
	i32 num = 0;
	CHashedString hash(root);
	while (GetLineSetById(hash))
	{
		hash = CHashedString(root + "_" + DrxStringUtils::toString(++num));
	}
	return hash;
}

//--------------------------------------------------------------------------------------------------
void CDialogLineDatabase::SerializeLinesHistory(Serialization::IArchive& ar)
{
#if defined(DRS_COLLECT_DEBUG_DATA)
	CResponseSystem::GetInstance()->m_responseSystemDebugDataProvider.SerializeDialogLinesHistory(ar);
#endif
}

//--------------------------------------------------------------------------------------------------
bool CDialogLineDatabase::ExecuteScript(u32 index)
{
#if defined(DRX_PLATFORM_WINDOWS)
	static i32 maxParallelCmds = 4;  //todo: expose this value to the UI
	static i32 counter = 0;

	const string linescriptpath = PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR "linescript.bat";

	if (gEnv->pDrxPak->IsFileExist(linescriptpath, IDrxPak::eFileLocation_Any))
	{
		DRS::IDialogLineSet* pLineSet = GetLineSetByIndex(index);

		if (pLineSet)
		{
			for (i32 i = 0; i < pLineSet->GetLineCount(); i++)
			{
				const DRS::IDialogLine* pCurrentLine = pLineSet->GetLineByIndex(i);
				tukk szStartTriggerWithoutPrefix = pCurrentLine->GetStartAudioTrigger().c_str();

				const size_t prefixPos = pCurrentLine->GetStartAudioTrigger().find('_');
				if (prefixPos != string::npos)
				{
					szStartTriggerWithoutPrefix = szStartTriggerWithoutPrefix + prefixPos + 1;
				}

				char szBuffer[1024];
				drx_sprintf(szBuffer, "@SET LINE_ID=%s&SET LINE_ID_HASH=%s&SET SUBTITLE=%s&SET AUDIO_TRIGGER=%s&SET AUDIO_TRIGGER_CLEANED=%s&SET ANIMATION_NAME=%s&SET STANDALONE_FILE=%s&SET CUSTOM_DATA=%s&SET VARIATION_INDEX=%s&%s%s",
					pLineSet->GetLineId().GetText().c_str(),
					 DrxStringUtils::toString(pLineSet->GetLineId().GetHash()).c_str(),
					pCurrentLine->GetText().c_str(),
					pCurrentLine->GetStartAudioTrigger().c_str(),
					szStartTriggerWithoutPrefix,
					pCurrentLine->GetLipsyncAnimation().c_str(),
					pCurrentLine->GetStandaloneFile().c_str(),
					pCurrentLine->GetCustomData().c_str(),
					DrxStringUtils::toString(i).c_str(),
					(++counter % maxParallelCmds == 0) ? "" : "start cmd /c ",
					linescriptpath.c_str());

				system(szBuffer);
			}
			return true;
		}
	}
#endif //DRX_PLATFORM_WINDOWS

	return false;
}

//--------------------------------------------------------------------------------------------------
void DrxDRS::CDialogLineDatabase::GetAllLineData(DRS::ValuesList* pOutCollectionsList, bool bSkipDefaultValues)
{
	std::pair<string, string> temp;
	for (const CDialogLineSet& line : m_lineSets)
	{
		if (!bSkipDefaultValues || line.GetLastPickedLine() != 0)
		{
			temp.first = line.GetLineId().GetText();
			temp.second = DrxStringUtils::toString(line.GetLastPickedLine());
			pOutCollectionsList->push_back(temp);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void DrxDRS::CDialogLineDatabase::SetAllLineData(DRS::ValuesListIterator start, DRS::ValuesListIterator end)
{
	for (CDialogLineSet& line : m_lineSets)
	{
		line.Reset();
	}

	for (DRS::ValuesListIterator it = start; it != end; ++it)
	{
		CDialogLineSet* pLine = GetLineSetById(it->first.c_str());
		if (pLine)
		{
			pLine->SetLastPickedLine((i32)atoi(it->second));
		}

	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
CDialogLineSet::CDialogLineSet()
	: m_priority(50)
	, m_flags(IDialogLineSet::EPickModeFlags_RandomVariation)
	, m_lastPickedLine(0)
	, m_maxQueuingDuration(-1.0f) //negative values do mean 'default' which maps to the value set in CSpeakerUpr via cvar 'drs_dialogsDefaultMaxQueueTime'
{
	//m_lines.push_back(CDialogLine());
}

//--------------------------------------------------------------------------------------------------
const CDialogLine* CDialogLineSet::PickLine()
{
	i32k numLines = static_cast<i32>(m_lines.size());

	if (numLines == 0)
	{
		DrsLogWarning("DialogLineSet without a single concrete line in it detected");
		return nullptr;
	}

	if ((m_flags & EPickModeFlags::EPickModeFlags_SequentialVariationOnlyOnce) > 0)
	{
		if (m_lastPickedLine >= numLines)
		{
			return nullptr;
		}
		return &m_lines[m_lastPickedLine++];
	}
	else if (numLines == 1)
	{
		return &m_lines[0];
	}
	else if ((m_flags & IDialogLineSet::EPickModeFlags_RandomVariation) > 0)
	{
		i32 randomLineIndex = 0;
		if (numLines == 2)
		{
			if (m_lastPickedLine == 0)
				randomLineIndex = 1;
		}
		else
		{
			randomLineIndex = drx_random(0, numLines - 2);
			if (randomLineIndex == m_lastPickedLine)
				randomLineIndex = numLines - 1;
		}

		m_lastPickedLine = randomLineIndex;
		return &m_lines[randomLineIndex];
	}
	else if ((m_flags & IDialogLineSet::EPickModeFlags_SequentialVariationRepeat) > 0)
	{
		if (m_lastPickedLine >= numLines)
		{
			m_lastPickedLine = 0;
		}
		return &m_lines[m_lastPickedLine++];
	}
	else if ((m_flags & IDialogLineSet::EPickModeFlags_SequentialVariationClamp) > 0)
	{
		if (m_lastPickedLine >= numLines)
		{
			return &m_lines[numLines - 1];
		}
		return &m_lines[m_lastPickedLine++];
	}
	else if ((m_flags & IDialogLineSet::EPickModeFlags_SequentialAllSuccessively) > 0)
	{
		return &m_lines[0];
	}
	else
	{
		DrsLogWarning("Could not pick a line from the dialogLineSet, because the RandomMode was set to an illegal value");
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
const CDialogLine* CDialogLineSet::GetFollowUpLine(const CDialogLine* pCurrentLine)
{
	if ((m_flags & IDialogLineSet::EPickModeFlags_SequentialAllSuccessively) > 0)
	{
		bool bCurrentLineFound = false;
		for (const CDialogLine& currentLine : m_lines)
		{
			if (bCurrentLineFound) //if the current line was found, this means the next one is the follow up line
			{
				return &currentLine;
			}
			if (pCurrentLine == &currentLine)
			{
				bCurrentLineFound = true;
			}
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
void CDialogLineSet::Reset()
{
	SetLastPickedLine(0);
}

//--------------------------------------------------------------------------------------------------
DRS::IDialogLine* CDialogLineSet::GetLineByIndex(u32 index)
{
	if (index < m_lines.size())
	{
		return &m_lines[index];
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
DRS::IDialogLine* CDialogLineSet::InsertLine(u32 index)
{
	if (index == (u32)-1)
	{
		m_lines.emplace_back(CDialogLine());
		return &m_lines.back();
	}
	return &(*m_lines.insert(m_lines.begin() + index, CDialogLine()));
}

//--------------------------------------------------------------------------------------------------
bool CDialogLineSet::RemoveLine(u32 index)
{
	if (index < m_lines.size())
	{
		m_lines.erase(m_lines.begin() + index);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------
void CDialogLineSet::Serialize(Serialization::IArchive& ar)
{
	ar(m_lineId, "lineID", "lineID");
	ar(m_priority, "priority", "Priority");
	ar(m_lastPickedLine, "lastPicked", "Last Picked variation");
	ar(m_flags, "flags", "Flags");
	ar(m_lines, "lineVariations", "+Lines");
	ar(m_maxQueuingDuration, "maxQueuingDuration", "MaxQueuingDuration");

#if !defined(_RELEASE)
	if (ar.isEdit())
	{
		if (m_lines.empty())
		{
			ar.error(m_priority, "DialogLineSet without a single concrete line");
		}
		if ((m_flags & EPickModeFlags::EPickModeFlags_RandomVariation) == 0)
		{
			ar.error(m_priority, "DialogLineSet without a correct LinePickMode found.");
		}
	}
#endif
}

bool DrxDRS::CDialogLineSet::HasAvailableLines()
{
	if ((m_flags & EPickModeFlags::EPickModeFlags_SequentialVariationOnlyOnce) > 0)
	{
		return m_lastPickedLine < static_cast<i32>(m_lines.size());
	}
	return !m_lines.empty();
}

void DrxDRS::CDialogLineSet::OnLineCanceled(const CDialogLine* pCanceledLine)
{
	if ((m_flags & EPickModeFlags::EPickModeFlags_SequentialVariationOnlyOnce) > 0)
	{
		m_lastPickedLine--; //remark: if a only-once line is canceled, we do allow it to repeat.
	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
CDialogLine::CDialogLine()
	: m_pauseLength(-1.0f)   //negative values mean "default value", specified by drs_dialogsDefaultPauseAfterLines
{}

//--------------------------------------------------------------------------------------------------
void CDialogLine::Serialize(Serialization::IArchive& ar)
{
	ar(m_text, "text", "^Text");
	ar(m_audioStartTrigger, "audioStartTrigger", "AudioStartTrigger");
	ar(m_audioStopTrigger, "audioStopTrigger", "AudioStopTrigger");
	ar(m_lipsyncAnimation, "lipsyncAnim", "LipsyncAnim");
	ar(m_standaloneFile, "standaloneFile", "StandaloneFile");
	ar(m_pauseLength, "pauseLength", "PauseLength");
	ar(m_customData, "customData", "CustomData");

	if (ar.isEdit())
	{
		if (m_text.empty() && m_audioStopTrigger.empty() && m_audioStartTrigger.empty() && m_standaloneFile.empty())
		{
			ar.warning(m_text, "DialogLine without any data found");
		}
	}
}
}