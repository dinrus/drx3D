// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   The DialogLine Database holds all DialogLines and is able to pick the correct one, given a lineID.
   A SDialogLineSet contains several SDialogLines which are picked by some criteria (random, sequential...)

************************************************************************/

#pragma once

#include <drx3D/CoreX/String/HashedString.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>

namespace DrxDRS
{
class CDialogLine final : public DRS::IDialogLine
{
public:
	CDialogLine();

	//////////////////////////////////////////////////////////
	// IDialogLine implementation
	virtual const string& GetText() const override                                     { return m_text; }
	virtual const string& GetStartAudioTrigger() const override                        { return m_audioStartTrigger; }
	virtual const string& GetEndAudioTrigger() const override                          { return m_audioStopTrigger; }
	virtual const string& GetLipsyncAnimation() const override                         { return m_lipsyncAnimation; }
	virtual const string& GetStandaloneFile() const override                           { return m_standaloneFile; }
	virtual const string& GetCustomData() const override                               { return m_customData; }
	virtual float         GetPauseLength() const override                              { return m_pauseLength; }
	
	virtual void          SetText(const string& text) override                         { m_text = text; }
	virtual void          SetStartAudioTrigger(const string& trigger) override         { m_audioStartTrigger = trigger; }
	virtual void          SetEndAudioTrigger(const string& trigger) override           { m_audioStopTrigger = trigger; }
	virtual void          SetLipsyncAnimation(const string& lipsyncAnimation) override { m_lipsyncAnimation = lipsyncAnimation; }
	virtual void          SetStandaloneFile(const string& value) override              { m_standaloneFile = value; }
	virtual void          SetCustomData(const string& customData) override             { m_customData = customData; }
	virtual void          SetPauseLength(float length) override                        { m_pauseLength = length; }
	
	virtual void          Serialize(Serialization::IArchive& ar) override;
	//////////////////////////////////////////////////////////

private:
	string m_text;              //todo: optimize into an (compressed) pool + also use wstring
	string m_audioStartTrigger; //todo: optimize by storing directly the TAudioControlID, problems occur, when ID can not be obtained because the bank is not loaded, and when saving the data to file
	string m_audioStopTrigger;
	string m_lipsyncAnimation;
	string m_standaloneFile;
	string m_customData;
	float  m_pauseLength;
};

class CDialogLineSet final : public DRS::IDialogLineSet
{
public:
	CDialogLineSet();
	const CDialogLine* PickLine();
	const CDialogLine* GetFollowUpLine(const CDialogLine* pCurrentLine);
	void               Reset();
	bool               HasAvailableLines();
	void               OnLineCanceled(const CDialogLine* pCanceledLine);

	void               SetLastPickedLine(i32 value) { m_lastPickedLine = value; }
	i32                GetLastPickedLine() const    { return m_lastPickedLine; }

	//////////////////////////////////////////////////////////
	// IDialogLineSet implementation
	virtual void              SetLineId(const CHashedString& lineId) override { m_lineId = lineId; }
	virtual void              SetPriority(i32 priority) override              { m_priority = priority; }
	virtual void              SetFlags(u32 flags) override                 { m_flags = flags; }
	virtual void              SetMaxQueuingDuration(float length) override    { m_maxQueuingDuration = length; }
	virtual CHashedString     GetLineId() const override                      { return m_lineId; }
	virtual i32               GetPriority() const override                    { return m_priority; }
	virtual u32            GetFlags() const override                       { return m_flags; }
	virtual float             GetMaxQueuingDuration() const override          { return m_maxQueuingDuration; }
	virtual u32            GetLineCount() const override                   { return m_lines.size(); }
	virtual DRS::IDialogLine* GetLineByIndex(u32 index) override;
	virtual DRS::IDialogLine* InsertLine(u32 index=1) override;
	virtual bool              RemoveLine(u32 index) override;
	virtual void              Serialize(Serialization::IArchive& ar) override;
	//////////////////////////////////////////////////////////

private:
	CHashedString            m_lineId;
	i32                      m_priority;
	u32                   m_flags; //eDialogLineSetFlags
	i32                      m_lastPickedLine;
	std::vector<CDialogLine> m_lines;
	float                    m_maxQueuingDuration;
};

class CDialogLineDatabase final : public DRS::IDialogLineDatabase
{

public:
	CDialogLineDatabase();
	virtual ~CDialogLineDatabase() override;
	bool            InitFromFiles(tukk szFilePath);
	//will reset all temporary data (for example the data to pick variations in sequential order)
	void            Reset();

	//////////////////////////////////////////////////////////
	// IDialogLineDatabase implementation
	virtual bool                       Save(tukk szFilePath) override;
	virtual u32                     GetLineSetCount() const override;
	virtual DRS::IDialogLineSet*       GetLineSetByIndex(u32 index) override;
	virtual CDialogLineSet*            GetLineSetById(const CHashedString& lineID) override;
	virtual DRS::IDialogLineSet*       InsertLineSet(u32 index=-1) override;
	virtual bool                       RemoveLineSet(u32 index) override;
	virtual bool                       ExecuteScript(u32 index) override;
	virtual void                       Serialize(Serialization::IArchive& ar) override;
	virtual void                       SerializeLinesHistory(Serialization::IArchive& ar) override;
	//////////////////////////////////////////////////////////

	void GetAllLineData(DRS::ValuesList* pOutCollectionsList, bool bSkipDefaultValues); //stores the current state
	void SetAllLineData(DRS::ValuesListIterator start, DRS::ValuesListIterator end);    //restores a state

private:
	CHashedString GenerateUniqueId(const string& root);

	typedef std::vector<CDialogLineSet> DialogLineSetList;
	DialogLineSetList  m_lineSets;

	i32                m_drsDialogBinaryFileFormatCVar;
	static tukk s_szFilename;
};
}
