// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

typedef u32 TAudioSignalID; // internally, this is now just the index into the audioSignals vector for SP ones, and 'index+amount_SP_signals' for MP ones.
const TAudioSignalID INVALID_AUDIOSIGNAL_ID = -1;
class CScriptbind_GameAudio;
class CGameAudioUtils;

#if !defined(_RELEASE)
struct SSignalAutoComplete;
#endif

class CGameAudio
{
	friend class CAudioSignalPlayer;
#if !defined(_RELEASE)
	friend struct SSignalAutoComplete;
#endif
public:
	CGameAudio();
	~CGameAudio();
	
	TAudioSignalID GetSignalID (tukk pSignalName, bool outputWarning = true);
	void Reset();

	void GetMemoryUsage( IDrxSizer *pSizer ) const;

	CGameAudioUtils* GetUtils() { return m_pUtils; }

#if !defined(_RELEASE)
	static void CmdPlaySignal(IConsoleCmdArgs* pCmdArgs);
	static void CmdPlaySignalOnEntity(IConsoleCmdArgs* pCmdArgs);
	static void CmdStopSignalsOnEntity(IConsoleCmdArgs* pCmdArgs);
	static void CmdPlayAllSignals(IConsoleCmdArgs* pCmdArgs);

	static void CmdCacheAudioFile(IConsoleCmdArgs* pCmdArgs);
	static void CmdRemoveCacheAudioFile(IConsoleCmdArgs* pCmdArgs);
#endif

private:

	enum ECommand
	{
		eCM_Cache_Audio,
		
		eCM_NumCommands,
		eCM_NoCommand = -1
	};

	class CCommand
	{
	public:
	
		CCommand() : m_command( eCM_NoCommand ) {}
		virtual ~CCommand() {}

		virtual void Execute() const = 0;
		virtual bool Init( const XmlNodeRef& commandNode, CGameAudio* pGameAudio ) = 0;
		bool IsACommand() const { return m_command!=eCM_NoCommand; }
		
		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
		}
		
		ECommand m_command;
	};	
	
	//.................................
	class CCacheAudio : public CCommand
	{
	public:
		CCacheAudio() 
			: m_bCache(false)
			, m_bNow(false)
		{}
		virtual bool Init( const XmlNodeRef& commandNode, CGameAudio* pGameAudio );
		virtual void Execute() const;

	private:
		string	m_gameHint;
		bool		m_bCache;
		bool		m_bNow;
	};

	//..................................
	class CAudioSignal
	{
	public:
		enum EFlags
		{
			eF_None = 0,
			eF_PlayRandom = BIT(0),
		};

		void GetMemoryUsage( IDrxSizer *pSizer ) const
		{
			pSizer->AddObject(m_signalName);
			//pSizer->AddContainer(m_sounds);			
			pSizer->AddContainer(m_commands);
		}
		u32 static GetSignalFlagsFromXML(const XmlNodeRef& currentCommandNode);

		u32 m_flags;
		string m_signalName;
		//std::vector<CSound> m_sounds;
		std::vector<const CCommand*> m_commands;

		struct SFlagTableEntry
		{
			tukk name;
			u32k flag;
		};
		static const SFlagTableEntry SoundFlagTable[];

	};

	//..................................
	struct SSignalFile
	{
		SSignalFile(XmlNodeRef _nodeRef, tukk _filename)
			: nodeRef(_nodeRef)
			, filename(_filename)
		{ }
		XmlNodeRef nodeRef;
		tukk filename;
	};

private:

	struct SAudioSignalsData;

	void LoadSignalsFromXMLNode( tukk xmlFilename, const XmlNodeRef& xmlNode, SAudioSignalsData& data );
	const CAudioSignal* GetAudioSignal( TAudioSignalID signalID );
	static CCommand* CreateCommand( tukk pCommand );
	

private:

	typedef std::vector<CAudioSignal> TAudioSignalsVector;
	typedef std::vector<CCommand*> TCommandsVector;
	typedef std::map<string, u32> TNameToSignalIndexMap;

	struct SAudioSignalsData
	{
		~SAudioSignalsData() { Unload(); }
		void Unload();
		TCommandsVector commands;
		TNameToSignalIndexMap nameToSignalIndexMap;  // just to optimize searchs into audioSignals. the stored numeric value is the index into 'audioSignals', which is NOT (or not always) the ID of the signal
		TAudioSignalsVector audioSignals;
	};

	SAudioSignalsData m_SPSignalsData;
	SAudioSignalsData m_MPSignalsData;
	bool	m_MPAudioSignalsLoaded;

	struct SSoundSemanticTranslationTableEntry
	{
		tukk name;
		//const ESoundSemantic semantic;
	};
	static const SSoundSemanticTranslationTableEntry SoundSemanticTranslationTable[];
	
	struct SSoundFlagTranslationTableEntry
	{
		tukk name;
		u32k flag;
	};
	//static const SSoundFlagTranslationTableEntry SoundFlagTranslationTable[];

	struct SCommandNameTranslationTableEntry
	{
		tukk name;
		ECommand command;
	};
	static const SCommandNameTranslationTableEntry CommandNamesTranslataionTable[eCM_NumCommands];

	CScriptbind_GameAudio* m_pScriptbind;
	CGameAudioUtils* m_pUtils;
};
