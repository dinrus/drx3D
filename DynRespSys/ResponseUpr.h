// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   This manager handles all dynamic Responses. He is responsible for loading them from file,
   Handling of Signals, Checking the rules, find the correct response

************************************************************************/

#pragma once

#include "Response.h"
#include <drx3D/CoreX/String/DrxString.h>

namespace DrxDRS
{
class CVariableCollection;
class CResponseActor;
typedef std::shared_ptr<CVariableCollection> VariableCollectionSharedPtr;

struct SSignal
{
	SSignal(const CHashedString& signalName, CResponseActor* pSender, VariableCollectionSharedPtr pSignalContext);

	DRS::SignalInstanceId       m_id;  //a unique id for this instance of the signal
	CHashedString               m_signalName;
	CResponseActor*             m_pSender;
	VariableCollectionSharedPtr m_pSignalContext;
};

typedef std::shared_ptr<CResponse> ResponsePtr;

//--------------------------------------------------------------------------------------------------

class CResponseUpr final : public DRS::IResponseUpr
{
public:
	static string s_currentSignal;  //only needed during serialization

	enum EUsedFileFormat
	{
		eUFF_JSON = 1,
		eUFF_XML  = 2,
		eUFF_BIN  = 3
	};

	enum ESerializeHint
	{
		eSH_ReplaceAllExisting        = BIT(0), //will remove existing ones
		eSH_CollapsedResponseSegments = BIT(1), //will only display responseSegments hierarchy (editor only)
		eSH_EvaluateResponses         = BIT(2), //will display if the condition is currently met (editor only)
	};

	typedef std::unordered_map<CHashedString, ResponsePtr>                                   MappedSignals;
	typedef std::vector<SSignal>                                                             SignalList;
	typedef std::vector<CResponseInstance*>                                                  ResponseInstanceList;
	typedef std::vector<std::pair<DRS::IResponseUpr::IListener*, DRS::SignalInstanceId>> ListenerList;

	CResponseUpr();
	virtual ~CResponseUpr() override;

	//////////////////////////////////////////////////////////
	// IResponseUpr implementation
	virtual bool                  AddListener(DRS::IResponseUpr::IListener* pNewListener, DRS::SignalInstanceId signalID = DRS::s_InvalidSignalId) override;
	virtual bool                  RemoveListener(DRS::IResponseUpr::IListener* pListenerToRemove) override;

	virtual DynArray<tukk > GetRecentSignals(DRS::IResponseUpr::eSignalFilter filter = DRS::IResponseUpr::eSF_All) override;

	virtual void                  SerializeResponse(Serialization::IArchive& ar, const DynArray<stack_string>& signalNames, DRS::IResponseActor* pActorForEvaluation = nullptr) override;
	virtual void                  SerializeRecentResponse(Serialization::IArchive& ar, const DynArray<stack_string>& signalNames, i32 maxElemets = -1) override;
	virtual void                  SerializeVariableChanges(Serialization::IArchive& ar, const stack_string& variableName = "ALL", i32 maxElemets = -1) override;

	virtual bool                  AddResponse(const stack_string& signalName) override;
	virtual bool                  RemoveResponse(const stack_string& signalName) override;
	//////////////////////////////////////////////////////////////////////////

	//will load all DynamicResponse Definitions from the folder
	void               SetFileFormat(EUsedFileFormat format);
	bool               LoadFromFiles(tukk szDataPath);
	bool               SaveToFiles(tukk szDataPath);
	void               SerializeResponseStates(Serialization::IArchive& ar);

	ResponsePtr        GetResponse(const CHashedString& signalName);
	bool			   HasMappingForSignal(const CHashedString& signalName);
	void			   OnActorRemoved(const CResponseActor* pActor);

	void               QueueSignal(const SSignal& signal);
	bool               CancelSignalProcessing(const SSignal& signal);
	bool               IsSignalProcessed(const SSignal& signal);
	void               Update();

	void               GetAllResponseData(DRS::ValuesList* pOutCollectionsList, bool bSkipDefaultValues);
	void               SetAllResponseData(DRS::ValuesListIterator start, DRS::ValuesListIterator end);

	CResponseInstance* CreateInstance(SSignal& signal, CResponse* pResponse);
	void               ReleaseInstance(CResponseInstance* pInstance, bool removeFromRunningInstances = true);
	void               Reset(bool bResetExecutionCounter, bool bClearAllResponseMappings = false);
	void               Serialize(Serialization::IArchive& ar);

private:
	bool _LoadFromFiles(const string& dataPath);

	void InformListenerAboutSignalProcessingStarted(const SSignal& signal, DRS::IResponseInstance* pInstance);
	void InformListenerAboutSignalProcessingFinished(const CHashedString& signalName, CResponseActor* pSender, const VariableCollectionSharedPtr& pSignalContext, const DRS::SignalInstanceId signalID, DRS::IResponseInstance* pInstance, DRS::IResponseUpr::IListener::eProcessingResult outcome);

	EUsedFileFormat      m_usedFileFormat;

	MappedSignals        m_mappedSignals;

	ResponseInstanceList m_runningResponses;

	ListenerList         m_listeners;

	SignalList           m_currentlyQueuedSignals;
};
} //endns DrxDRS
