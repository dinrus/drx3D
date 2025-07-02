// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>

#include <drx3D/Entity/IEntity.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/DynRespSys/IDynamicResponseAction.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/CoreX/Game/IGameFramework.h>

#include <drx3D/DynRespSys/ResponseSystemDataImportHelper.h>
#include <drx3D/DynRespSys/DialogLineDatabase.h>
#include <drx3D/DynRespSys/ResponseSystem.h>
#include <drx3D/DynRespSys/ResponseUpr.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/VariableCollection.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/SchematycEntityDrsComponent.h>
#include <drx3D/DynRespSys/ActionCancelSignal.h>
#include <drx3D/DynRespSys/ActionSpeakLine.h>
#include <drx3D/DynRespSys/ActionSetVariable.h>
#include <drx3D/DynRespSys/ActionCopyVariable.h>
#include <drx3D/DynRespSys/ActionExecuteResponse.h>
#include <drx3D/DynRespSys/ActionResetTimer.h>
#include <drx3D/DynRespSys/ActionSendSignal.h>
#include <drx3D/DynRespSys/ActionSetActor.h>
#include <drx3D/DynRespSys/ActionSetGameToken.h>
#include <drx3D/DynRespSys/ActionWait.h>
#include <drx3D/DynRespSys/SpecialConditionsImpl.h>
#include <drx3D/DynRespSys/ConditionImpl.h>

using namespace DrxDRS;

CResponseSystem* CResponseSystem::s_pInstance = nullptr;

void ReloadResponseDefinition(IConsoleCmdArgs* pArgs)
{
	CResponseSystem::GetInstance()->ReInit();
}

//--------------------------------------------------------------------------------------------------
void SendDrsSignal(IConsoleCmdArgs* pArgs)
{
	if (pArgs->GetArgCount() >= 3)
	{
		CResponseSystem* pDrs = CResponseSystem::GetInstance();
		if (pDrs)
		{
			DRS::IResponseActor* pSender = pDrs->GetResponseActor(pArgs->GetArg(1));
			if (pSender)
			{
				pSender->QueueSignal(pArgs->GetArg(2));
				return;
			}
			else
			{
				DrsLogWarning((string("Could not find an actor with that name: ") + pArgs->GetArg(1)).c_str());
			}
		}
	}

	DrsLogWarning("Format: SendDrsSignal <senderEntityName> <signalname>");
}

//--------------------------------------------------------------------------------------------------
void ChangeFileFormat(ICVar* pICVar)
{
	if (!pICVar)
		return;

	tukk pVal = pICVar->GetString();

	CResponseUpr::EUsedFileFormat fileFormat = CResponseUpr::eUFF_JSON;
	if (strcmp(pVal, "BIN") == 0)
		fileFormat = CResponseUpr::eUFF_BIN;
	else if (strcmp(pVal, "XML") == 0)
		fileFormat = CResponseUpr::eUFF_XML;

	CResponseSystem::GetInstance()->GetResponseUpr()->SetFileFormat(fileFormat);
}

//--------------------------------------------------------------------------------------------------
CResponseSystem::CResponseSystem()
{
	m_bIsCurrentlyUpdating = false;
	m_pendingResetRequest = DRS::IDynamicResponseSystem::eResetHint_Nothing;
	m_pDataImporterHelper = nullptr;

	m_pSpeakerUpr = new CSpeakerUpr();
	m_pDialogLineDatabase = new CDialogLineDatabase();
	m_pVariableCollectionUpr = new CVariableCollectionUpr();
	m_pResponseUpr = new CResponseUpr();
	DRX_ASSERT(s_pInstance == nullptr && "ResponseSystem was created more than once!");
	s_pInstance = this;

	REGISTER_COMMAND("drs_sendSignal", SendDrsSignal, VF_NULL, "Sends a DRS Signal by hand. Useful for testing. Format SendDrsSignal <senderEntityName> <signalname>");

	m_pUsedFileFormat = REGISTER_STRING_CB("drs_fileFormat", "JSON", VF_NULL, "Specifies the file format to use (JSON, XML, BIN)", ::ChangeFileFormat);
    m_pDataPath = REGISTER_STRING("drs_dataPath", "Libs" DRX_NATIVE_PATH_SEPSTR "DynamicResponseSystem", VF_NULL, "Specifies the path where to find the response and dialogline files");

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CResponseSystem");

#if defined(DRS_COLLECT_DEBUG_DATA)
	m_responseSystemDebugDataProvider.Init();
#endif
}

//--------------------------------------------------------------------------------------------------
CResponseSystem::~CResponseSystem()
{
	if (gEnv->pSchematyc != nullptr)
	{
		gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(GetSchematycPackageGUID());
	}

	for (CResponseActor* pActor : m_createdActors)
	{
		delete pActor;
	}
	m_createdActors.clear();

	gEnv->pConsole->UnregisterVariable("drs_fileFormat", true);
	gEnv->pConsole->UnregisterVariable("drs_dataPath", true);
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	delete m_pSpeakerUpr;
	delete m_pDialogLineDatabase;

	delete m_pDataImporterHelper;
	delete m_pResponseUpr;
	delete m_pVariableCollectionUpr;
	s_pInstance = nullptr;
}

//--------------------------------------------------------------------------------------------------
bool CResponseSystem::Init()
{	
	m_filesFolder = PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR + m_pDataPath->GetString();
	m_pSpeakerUpr->Init();
	m_pDialogLineDatabase->InitFromFiles(m_filesFolder + DRX_NATIVE_PATH_SEPSTR "DialogLines");
	
	m_currentTime.SetSeconds(0.0f);

	m_pVariableCollectionUpr->GetCollection(CVariableCollection::s_globalCollectionName)->CreateVariable("CurrentTime", 0.0f);

	return m_pResponseUpr->LoadFromFiles(m_filesFolder + DRX_NATIVE_PATH_SEPSTR "Responses");
}

//--------------------------------------------------------------------------------------------------
bool CResponseSystem::ReInit()
{
	Reset();  //stop running responses and reset variables
	m_pDialogLineDatabase->InitFromFiles(m_filesFolder + DRX_NATIVE_PATH_SEPSTR "DialogLines");
	return m_pResponseUpr->LoadFromFiles(m_filesFolder + DRX_NATIVE_PATH_SEPSTR "Responses");
}

//--------------------------------------------------------------------------------------------------
void CResponseSystem::Update()
{
	if (m_pendingResetRequest != DRS::IDynamicResponseSystem::eResetHint_Nothing)
	{
		InternalReset(m_pendingResetRequest);
		m_pendingResetRequest = DRS::IDynamicResponseSystem::eResetHint_Nothing;
	}

	m_currentTime += gEnv->pTimer->GetFrameTime();
	
	m_pVariableCollectionUpr->GetCollection(CVariableCollection::s_globalCollectionName)->SetVariableValue("CurrentTime", m_currentTime.GetSeconds());

	m_bIsCurrentlyUpdating = true;
	m_pVariableCollectionUpr->Update();
	m_pResponseUpr->Update();
	m_pSpeakerUpr->Update();
	m_bIsCurrentlyUpdating = false;
}

//--------------------------------------------------------------------------------------------------
CVariableCollection* CResponseSystem::CreateVariableCollection(const CHashedString& name)
{
	return m_pVariableCollectionUpr->CreateVariableCollection(name);
}
//--------------------------------------------------------------------------------------------------
void CResponseSystem::ReleaseVariableCollection(DRS::IVariableCollection* pToBeReleased)
{
	m_pVariableCollectionUpr->ReleaseVariableCollection(static_cast<CVariableCollection*>(pToBeReleased));
}

//--------------------------------------------------------------------------------------------------
CVariableCollection* CResponseSystem::GetCollection(const CHashedString& name)
{
	return m_pVariableCollectionUpr->GetCollection(name);
}

//--------------------------------------------------------------------------------------------------
CVariableCollection* CResponseSystem::GetCollection(const CHashedString& name, DRS::IResponseInstance* pResponseInstance)
{
	// taken from IVariableUsingBase::GetCurrentCollection
	if (name == CVariableCollection::s_localCollectionName)  //local variable collection
	{
		CResponseActor* pCurrentActor = static_cast<CResponseInstance*>(pResponseInstance)->GetCurrentActor();
		return pCurrentActor->GetLocalVariables();
	}
	else if (name == CVariableCollection::s_contextCollectionName)  //context variable collection
	{
		return static_cast<CResponseInstance*>(pResponseInstance)->GetContextVariablesImpl().get();
	}
	else
	{
		static CVariableCollectionUpr* const pVariableCollectionMgr = CResponseSystem::GetInstance()->GetVariableCollectionUpr();
		return pVariableCollectionMgr->GetCollection(name);
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
CDataImportHelper* CResponseSystem::GetCustomDataformatHelper()
{
	if (!m_pDataImporterHelper)
	{
		m_pDataImporterHelper = new CDataImportHelper();
	}
	return m_pDataImporterHelper;
}

//--------------------------------------------------------------------------------------------------
CResponseActor* CResponseSystem::CreateResponseActor(tukk szActorName, EntityId entityID, tukk szGlobalVariableCollectionToUse)
{
	CResponseActor* newActor = nullptr;
	CResponseActor* pExistingActor = GetResponseActor(CHashedString(szActorName));
	if(pExistingActor)
	{
		string actorname(szActorName);
		u32 uniqueActorNameCounter = 1;
		while (pExistingActor)
		{
			actorname.Format("%s_%i", szActorName, uniqueActorNameCounter++);
			pExistingActor = GetResponseActor(CHashedString(actorname));
		}
		DrsLogInfo(string().Format("Actor with name: '%s' was already existing. Renamed it to '%s'", szActorName, actorname).c_str());
		newActor = new CResponseActor(actorname, entityID, szGlobalVariableCollectionToUse);
	}
	else
	{
		newActor = new CResponseActor(szActorName, entityID, szGlobalVariableCollectionToUse);
	}
	
	m_createdActors.push_back(newActor);
	return newActor;
}

//--------------------------------------------------------------------------------------------------
bool CResponseSystem::ReleaseResponseActor(DRS::IResponseActor* pActorToFree)
{
	for (ResponseActorList::iterator it = m_createdActors.begin(), itEnd = m_createdActors.end(); it != itEnd; ++it)
	{
		if (*it == pActorToFree)
		{
			delete *it;
			m_createdActors.erase(it);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------------------------------------
CResponseActor* CResponseSystem::GetResponseActor(const CHashedString& actorName)
{
	for (CResponseActor* pActor : m_createdActors)
	{
		if (pActor->GetNameHashed() == actorName)
		{
			return pActor;
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
void CResponseSystem::Reset(u32 resetHint)
{
	if (m_bIsCurrentlyUpdating)
	{
		//we delay the actual execution until the beginning of the next update, because we do not want the reset to happen, while we are iterating over the currently running responses
		m_pendingResetRequest = resetHint;
	}
	else
	{
		InternalReset(resetHint);
	}
}

//--------------------------------------------------------------------------------------------------
bool CResponseSystem::CancelSignalProcessing(const CHashedString& signalName, DRS::IResponseActor* pSender /* = nullptr */, DRS::SignalInstanceId instanceToSkip /* = s_InvalidSignalId */)
{
	SSignal temp(signalName, static_cast<CResponseActor*>(pSender), nullptr);
	temp.m_id = instanceToSkip;
	return m_pResponseUpr->CancelSignalProcessing(temp);
}

//--------------------------------------------------------------------------------------------------
DRS::ActionSerializationClassFactory& CResponseSystem::GetActionSerializationFactory()
{
	return Serialization::ClassFactory<DRS::IResponseAction>::the();
}

//--------------------------------------------------------------------------------------------------
DRS::ConditionSerializationClassFactory& CResponseSystem::GetConditionSerializationFactory()
{
	return Serialization::ClassFactory<DRS::IResponseCondition>::the();
}

//--------------------------------------------------------------------------------------------------
void CResponseSystem::Serialize(Serialization::IArchive& ar)
{
	if (ar.openBlock("GlobalVariableCollections", "Global Variable Collections"))
	{
		m_pVariableCollectionUpr->Serialize(ar);
		ar.closeBlock();
	}

#if defined(HASHEDSTRING_STORES_SOURCE_STRING)
	if (ar.isEdit())
	{
		if (ar.openBlock("LocalVariableCollections", "Local Variable Collections"))
		{
			for (CResponseActor* pActor : m_createdActors)
			{
				if (pActor->GetNonGlobalVariableCollection())
				{
					VariableCollectionSharedPtr pCollection = pActor->GetNonGlobalVariableCollection();
					ar(*pCollection, pCollection->GetName().m_textCopy, pCollection->GetName().m_textCopy);
				}
				else
				{
					string temp = "uses global collection: " + pActor->GetCollectionName().GetText();
					ar(temp, pActor->GetName(), pActor->GetName());
				}
			}
			ar.closeBlock();
		}
	}
#endif

	m_pResponseUpr->SerializeResponseStates(ar);

	m_currentTime.SetSeconds(m_pVariableCollectionUpr->GetCollection(CVariableCollection::s_globalCollectionName)->GetVariableValue("CurrentTime").GetValueAsFloat());  //we serialize our DRS time manually via the time variable
}

//--------------------------------------------------------------------------------------------------
DRS::IVariableCollectionSharedPtr CResponseSystem::CreateContextCollection()
{
	static i32 uniqueNameCounter = 0;
	return std::make_shared<CVariableCollection>(string().Format("Context_%i", ++uniqueNameCounter));  //drs-todo: pool me
}

//--------------------------------------------------------------------------------------------------
void CResponseSystem::OnSystemEvent(ESystemEvent event, UINT_PTR pWparam, UINT_PTR pLparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
		//stop active responses + active speakers on level change
		Reset(DRS::IDynamicResponseSystem::eResetHint_StopRunningResponses | DRS::IDynamicResponseSystem::eResetHint_Speaker);
		break;
	case ESYSTEM_EVENT_INITIALIZE_DRS:  //= ESYSTEM_EVENT_REGISTER_SXEMA_ENV
		{
			Init();

			if (gEnv->pSchematyc)
			{
				tukk szName = "DynamicResponseSystem";
				tukk szDescription = "Dynamic response system";
				sxema::EnvPackageCallback callback = SXEMA_MEMBER_DELEGATE(&CResponseSystem::RegisterSchematycEnvPackage, *this);
				gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(SXEMA_MAKE_ENV_PACKAGE(GetSchematycPackageGUID(), szName, sxema::g_szDinrus, szDescription, callback));
			}
			break;
		}
	case ESYSTEM_EVENT_GAME_FRAMEWORK_ABOUT_TO_SHUTDOWN:
		{
			m_pSpeakerUpr->Shutdown();
			m_pResponseUpr->Reset(false, true);  //We have to release all mapped responses here, because they might contain game specific actions/conditions.
		}
		break;
	}
}

//--------------------------------------------------------------------------------------------------
void CResponseSystem::RegisterSchematycEnvPackage(sxema::IEnvRegistrar& registrar)
{
	CSchematycEntityDrsComponent::Register(registrar);
}

//--------------------------------------------------------------------------------------------------
VariableCollectionSharedPtr CResponseSystem::CreateLocalCollection(const string& name)
{
	return std::make_shared<CVariableCollection>(name);  //drs-todo: pool me
}

//--------------------------------------------------------------------------------------------------
#if !defined(_RELEASE)
void CResponseSystem::SetCurrentDrsUserName(tukk szNewDrsUserName)
{
	m_currentDrsUserName = szNewDrsUserName;
}
#endif

//--------------------------------------------------------------------------------------------------
void CResponseSystem::InternalReset(u32 resetFlags)
{
	if (resetFlags & (DRS::IDynamicResponseSystem::eResetHint_StopRunningResponses | DRS::IDynamicResponseSystem::eResetHint_ResetAllResponses))
	{
		m_pResponseUpr->Reset((resetFlags& DRS::IDynamicResponseSystem::eResetHint_ResetAllResponses) > 0);
	}
	if (resetFlags & DRS::IDynamicResponseSystem::eResetHint_Variables)
	{
		m_pVariableCollectionUpr->Reset();
		m_currentTime.SetSeconds(0.0f);
		m_pVariableCollectionUpr->GetCollection(CVariableCollection::s_globalCollectionName)->SetVariableValue("CurrentTime", m_currentTime.GetSeconds());

		//also reset the local variable collections (which are not managed by the variable collection manager)
		for (CResponseActor* pActor : m_createdActors)
		{
			if (pActor->GetNonGlobalVariableCollection())
			{
				pActor->GetNonGlobalVariableCollection()->Reset();
			}
		}
	}
	if (resetFlags & DRS::IDynamicResponseSystem::eResetHint_Speaker)
	{
		m_pSpeakerUpr->Reset();
		m_pDialogLineDatabase->Reset();
	}
	if (resetFlags & DRS::IDynamicResponseSystem::eResetHint_DebugInfos)
	{
		DRS_DEBUG_DATA_ACTION(Reset());
	}
}
constexpr tukk szDrsDataTag = "<DRS_DATA>";
constexpr tukk szDrsVariablesStartTag = "VariablesStart";
constexpr tukk szDrsVariablesEndTag = "VariablesEnd";
constexpr tukk szDrsResponsesStartTag = "ResponsesStart";
constexpr tukk szDrsResponsesEndTag = "ResponsesEnd";
constexpr tukk szDrsLinesStartTag = "LinesStart";
constexpr tukk szDrsLinesEndTag = "LinesEnd";

//--------------------------------------------------------------------------------------------------
DRS::ValuesListPtr CResponseSystem::GetCurrentState(u32 saveHints) const
{
	const bool bSkipDefaultValues = (saveHints & SaveHints_SkipDefaultValues) > 0;

	DRS::ValuesListPtr variableList(new DRS::ValuesList(), [](DRS::ValuesList* pList)
	{
		pList->clear();
		delete pList;
	});

	std::pair<DRS::ValuesString, DRS::ValuesString> temp;
	temp.first = szDrsDataTag;
	if (saveHints & SaveHints_Variables)
	{
		temp.second = szDrsVariablesStartTag;
		variableList->push_back(temp);
		m_pVariableCollectionUpr->GetAllVariableCollections(variableList.get(), bSkipDefaultValues);
		temp.second = szDrsVariablesEndTag;
		variableList->push_back(temp);
	}

	if (saveHints & SaveHints_ResponseData)
	{
		temp.second = szDrsResponsesStartTag;
		variableList->push_back(temp);
		m_pResponseUpr->GetAllResponseData(variableList.get(), bSkipDefaultValues);
		temp.second = szDrsResponsesEndTag;
		variableList->push_back(temp);
	}

	if (saveHints & SaveHints_LineData)
	{
		temp.second = szDrsLinesStartTag;
		variableList->push_back(temp);
		m_pDialogLineDatabase->GetAllLineData(variableList.get(), bSkipDefaultValues);
		temp.second = szDrsLinesEndTag;
		variableList->push_back(temp);
	}

	return variableList;
}

//--------------------------------------------------------------------------------------------------
void CResponseSystem::SetCurrentState(const DRS::ValuesList& collectionsList)
{
	DRS::ValuesListIterator itStartOfVariables = collectionsList.begin();
	DRS::ValuesListIterator itStartOfResponses = collectionsList.begin();
	DRS::ValuesListIterator itStartOfLines = collectionsList.begin();
	DRS::ValuesListIterator itEndOfVariables = collectionsList.begin();
	DRS::ValuesListIterator itEndOfResponses = collectionsList.begin();
	DRS::ValuesListIterator itEndOfLines = collectionsList.begin();

	for (DRS::ValuesListIterator it = collectionsList.begin(); it != collectionsList.end(); ++it)
	{
		if (it->first == szDrsDataTag)
		{
			if (it->second == szDrsVariablesStartTag)
				itStartOfVariables = it + 1;
			if (it->second == szDrsVariablesEndTag)
				itEndOfVariables = it;
		}
	}

	for (DRS::ValuesListIterator it = collectionsList.begin(); it != collectionsList.end(); ++it)
	{
		if (it->first == szDrsDataTag)
		{
			if (it->second == szDrsResponsesStartTag)
				itStartOfResponses = it + 1;
			if (it->second == szDrsResponsesEndTag)
				itEndOfResponses = it;
		}
	}

	for (DRS::ValuesListIterator it = collectionsList.begin(); it != collectionsList.end(); ++it)
	{
		if (it->first == szDrsDataTag)
		{
			if (it->second == szDrsLinesStartTag)
				itStartOfLines = it + 1;
			if (it->second == szDrsLinesEndTag)
				itEndOfLines = it;
		}
	}

	if (itStartOfVariables != itEndOfVariables)
	{
		m_pVariableCollectionUpr->SetAllVariableCollections(itStartOfVariables, itEndOfVariables);
		//we serialize our DRS time manually via the "CurrentTime" variable
		m_currentTime.SetSeconds(m_pVariableCollectionUpr->GetCollection(CVariableCollection::s_globalCollectionName)->GetVariableValue("CurrentTime").GetValueAsFloat());
	}

	if (itStartOfResponses != itEndOfResponses)
	{
		m_pResponseUpr->SetAllResponseData(itStartOfResponses, itEndOfResponses);
	}

	if (itStartOfLines != itEndOfLines)
	{
		m_pDialogLineDatabase->SetAllLineData(itStartOfLines, itEndOfLines);
	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

CResponseActor::CResponseActor(const string& name, EntityId usedEntityID, tukk szGlobalVariableCollectionToUse)
	: m_linkedEntityID(usedEntityID)
	, m_name(name)
	, m_auxAudioObjectIdToUse(DrxAudio::InvalidAuxObjectId)
	, m_nameHashed(name)
	, m_variableCollectionName(CHashedString(szGlobalVariableCollectionToUse))
{
	if (szGlobalVariableCollectionToUse)
	{
		m_pNonGlobalVariableCollection = nullptr; // we don't store a pointer to the global variable collection (for now), because we would not be informed if it`s removed from the outside.
	}
	else
	{
		m_pNonGlobalVariableCollection = CResponseSystem::GetInstance()->CreateLocalCollection(name);
	}
}

//--------------------------------------------------------------------------------------------------
CResponseActor::~CResponseActor()
{
	CResponseSystem::GetInstance()->GetSpeakerUpr()->OnActorRemoved(this);
	CResponseSystem::GetInstance()->GetResponseUpr()->OnActorRemoved(this);
	m_pNonGlobalVariableCollection = nullptr;  //Remark: if we are using a global variable collection we are not releasing it. We assume it`s handled outside	
}

//--------------------------------------------------------------------------------------------------
IEntity* CResponseActor::GetLinkedEntity() const
{
	if (m_linkedEntityID == INVALID_ENTITYID)  //if no actor exists, we assume it's a virtual speaker (radio, walkie talkie...) and therefore use the client actor
	{
		if (gEnv->pGameFramework)
		{
			return gEnv->pGameFramework->GetClientEntity();
		}
		return nullptr;
	}
	// We are using the User Data of the IResponseActor to store the ID of the Entity associated with that Actor.
	return gEnv->pEntitySystem->GetEntity(m_linkedEntityID);
}

//--------------------------------------------------------------------------------------------------
DRS::SignalInstanceId CResponseActor::QueueSignal(const CHashedString& signalName, DRS::IVariableCollectionSharedPtr pSignalContext /*= nullptr*/, DRS::IResponseUpr::IListener* pSignalListener /*= nullptr*/)
{
	CResponseUpr* pResponseMgr = CResponseSystem::GetInstance()->GetResponseUpr();
	SSignal signal(signalName, this, std::static_pointer_cast<CVariableCollection>(pSignalContext));
	if (pSignalListener)
	{
		pResponseMgr->AddListener(pSignalListener, signal.m_id);
	}
	pResponseMgr->QueueSignal(signal);
	return signal.m_id;
}

//--------------------------------------------------------------------------------------------------
CVariableCollection* CResponseActor::GetLocalVariables()
{
	if (m_pNonGlobalVariableCollection)
	{
		return m_pNonGlobalVariableCollection.get();
	}
	else
	{
		CVariableCollection* pLocalVariables = CResponseSystem::GetInstance()->GetCollection(m_variableCollectionName);
		if (!pLocalVariables)
		{
			pLocalVariables = CResponseSystem::GetInstance()->CreateVariableCollection(m_variableCollectionName);
		}
		return pLocalVariables;
	}
}

//--------------------------------------------------------------------------------------------------
const CVariableCollection* CResponseActor::GetLocalVariables() const
{
	if (m_pNonGlobalVariableCollection)
	{
		return m_pNonGlobalVariableCollection.get();
	}
	else
	{
		const CVariableCollection* pLocalVariables = CResponseSystem::GetInstance()->GetCollection(m_variableCollectionName);
		if (!pLocalVariables)
		{
			pLocalVariables = CResponseSystem::GetInstance()->CreateVariableCollection(m_variableCollectionName);
		}
		return pLocalVariables;
	}
}

namespace DRS
{
SERIALIZATION_CLASS_NULL(IResponseAction, "");
SERIALIZATION_CLASS_NULL(IResponseCondition, "");
}

REGISTER_DRS_ACTION(CActionCancelSignal, "CancelSignal", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionCancelSpeaking, "CancelSpeaking", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionSetVariable, "ChangeVariable", "11DD11");
REGISTER_DRS_ACTION(CActionCopyVariable, "CopyVariable", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionExecuteResponse, "ExecuteResponse", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionResetTimerVariable, "ResetTimerVariable", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionSendSignal, "SendSignal", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionSetActor, "SetActor", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionSetActorByVariable, "SetActorFromVariable", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionSetGameToken, "SetGameToken", DEFAULT_DRS_ACTION_COLOR);
REGISTER_DRS_ACTION(CActionSpeakLine, "SpeakLine", "00FF00");
REGISTER_DRS_ACTION(CActionWait, "Wait", DEFAULT_DRS_ACTION_COLOR);

REGISTER_DRS_CONDITION(CExecutionLimitCondition, "Execution Limit", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CGameTokenCondition, "GameToken", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CInheritConditionsCondition, "Inherit Conditions", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CPlaceholderCondition, "Placeholder", "FF66CC");
REGISTER_DRS_CONDITION(CRandomCondition, "Random", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CTimeSinceCondition, "TimeSince", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CTimeSinceResponseCondition, "TimeSinceResponse", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CVariableEqualCondition, "Variable equal to ", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CVariableLargerCondition, "Variable greater than ", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CVariableRangeCondition, "Variable in range ", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CVariableSmallerCondition, "Variable less than ", DEFAULT_DRS_CONDITION_COLOR);
REGISTER_DRS_CONDITION(CVariableAgainstVariablesCondition, "Variable to Variable", DEFAULT_DRS_CONDITION_COLOR);
