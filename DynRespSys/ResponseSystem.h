// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   The dynamic Response Segment is the main starting point for creating VariableCollections
   queuing signals, init all systems, updating all systems...

************************************************************************/

#pragma once

#include <drx3D/CoreX/String/HashedString.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>

#if defined (DRS_COLLECT_DEBUG_DATA)
	#include "ResponseSystemDebugDataProvider.h"
#endif
#include "VariableCollection.h"
#include "ResponseSystemDataImportHelper.h"
#include "DialogLineDatabase.h"
#include "ResponseUpr.h"
#include "SpeakerUpr.h"

struct ICVar;

namespace DrxDRS
{
class CVariableCollectionUpr;
class CDataImportHelper;
class CDialogLineDatabase;

class CResponseActor final : public DRS::IResponseActor
{
public:
	CResponseActor(const string& name, EntityId linkedEntityID, tukk szGlobalVariableCollectionToUse);
	virtual ~CResponseActor() override;

	//////////////////////////////////////////////////////////
	// IResponseActor implementation
	virtual const string&              GetName() const override { return m_name; }
	virtual CVariableCollection*       GetLocalVariables() override;
	virtual const CVariableCollection* GetLocalVariables() const override;
	virtual EntityId                   GetLinkedEntityID() const override { return m_linkedEntityID; }
	virtual IEntity*                   GetLinkedEntity() const override;
	virtual void                       SetAuxAudioObjectID(DrxAudio::AuxObjectId overrideAuxProxy) override { m_auxAudioObjectIdToUse = overrideAuxProxy; }
	virtual DrxAudio::AuxObjectId      GetAuxAudioObjectID() const override { return m_auxAudioObjectIdToUse; }
	virtual DRS::SignalInstanceId      QueueSignal(const CHashedString& signalName, DRS::IVariableCollectionSharedPtr pSignalContext = nullptr, DRS::IResponseUpr::IListener* pSignalListener = nullptr) override;
	//////////////////////////////////////////////////////////

	const CHashedString&               GetNameHashed() const { return m_nameHashed; }
	const CHashedString&               GetCollectionName() const { return m_variableCollectionName; }
	VariableCollectionSharedPtr        GetNonGlobalVariableCollection() { return m_pNonGlobalVariableCollection; }  //for editor only

private:
	const EntityId              m_linkedEntityID;
	const CHashedString         m_nameHashed;
	const CHashedString         m_variableCollectionName;
	const string                m_name;
	VariableCollectionSharedPtr m_pNonGlobalVariableCollection;
	DrxAudio::AuxObjectId       m_auxAudioObjectIdToUse;
};

//////////////////////////////////////////////////////////////////////////

class CResponseSystem final : public DRS::IDynamicResponseSystem, public ISystemEventListener
{
public:
	CResponseSystem();
	virtual ~CResponseSystem() override;

	static CResponseSystem* GetInstance() { return s_pInstance; }
	static DrxGUID GetSchematycPackageGUID() { return "981168e2-f16d-46b7-bfaa-e11966204d47"_drx_guid; }

	//////////////////////////////////////////////////////////
	// DRS::IDynamicResponseSystem implementation
	virtual bool                                     Init() override;
	virtual bool                                     ReInit() override;
	virtual void                                     Update() override;

	virtual void                                     Reset(u32 resetHint = (u32)DRS::IDynamicResponseSystem::eResetHint_All) override;

	virtual CVariableCollection*                     CreateVariableCollection(const CHashedString& signalName) override;
	virtual CVariableCollection*                     GetCollection(const CHashedString& name) override;
	virtual CVariableCollection*                     GetCollection(const CHashedString& name, DRS::IResponseInstance* pResponseInstance) override;
	virtual void                                     ReleaseVariableCollection(DRS::IVariableCollection* pToBeReleased) override;
	virtual DRS::IVariableCollectionSharedPtr        CreateContextCollection() override;

	virtual bool                                     CancelSignalProcessing(const CHashedString& signalName, DRS::IResponseActor* pSender = nullptr, DRS::SignalInstanceId instanceToSkip = DRS::s_InvalidSignalId) override;

	virtual CResponseActor*                          CreateResponseActor(tukk szActorName, EntityId entityID = INVALID_ENTITYID, tukk szGlobalVariableCollectionToUse = nullptr) override;
	virtual bool                                     ReleaseResponseActor(DRS::IResponseActor* pActorToFree) override;
	virtual CResponseActor*                          GetResponseActor(const CHashedString& actorName) override;

	virtual CDataImportHelper*                       GetCustomDataformatHelper() override;
	virtual DRS::ActionSerializationClassFactory&    GetActionSerializationFactory() override;
	virtual DRS::ConditionSerializationClassFactory& GetConditionSerializationFactory() override;
	virtual CDialogLineDatabase*                     GetDialogLineDatabase() const override { return m_pDialogLineDatabase; }
	virtual CResponseUpr*                        GetResponseUpr() const override    { return m_pResponseUpr; }
	virtual CSpeakerUpr*                         GetSpeakerUpr() const override     { return m_pSpeakerUpr; }

	virtual DRS::ValuesListPtr                       GetCurrentState(u32 saveHints) const override;
	virtual void                                     SetCurrentState(const DRS::ValuesList& outCollectionsList) override;
	virtual void                                     Serialize(Serialization::IArchive& ar) override;

#if !defined(_RELEASE)
	virtual void        SetCurrentDrsUserName(tukk szNewDrsUserName) override;
	virtual tukk GetCurrentDrsUserName() const override { return m_currentDrsUserName.c_str(); }
	string m_currentDrsUserName;
#endif
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// ISystemEventListener implementation
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR pWparam, UINT_PTR pLparam) override;
	//////////////////////////////////////////////////////////

	VariableCollectionSharedPtr CreateLocalCollection(const string& name);  //remark: name not really relevant, since we do not allow GetCollectionByName for local collections

	CVariableCollectionUpr* GetVariableCollectionUpr() const { return m_pVariableCollectionUpr; }
	//we store the current time for the DRS ourselves in a variable, so that we can use this variable in conditions and it allows us to save/load/modify the current DRS time easily
	float                       GetCurrentDrsTime() const            { return m_currentTime.GetSeconds(); }

#if defined(DRS_COLLECT_DEBUG_DATA)
	CResponseSystemDebugDataProvider m_responseSystemDebugDataProvider;
	#define DRS_DEBUG_DATA_ACTION(actionToExecute) CResponseSystem::GetInstance()->m_responseSystemDebugDataProvider.actionToExecute;
#else
	#define DRS_DEBUG_DATA_ACTION(actionToExecute)
#endif

private:
	void        RegisterSchematycEnvPackage(sxema::IEnvRegistrar& registrar);

	void        InternalReset(u32 resetFlags);

	typedef std::vector<CResponseActor*> ResponseActorList;

	static CResponseSystem*     s_pInstance;
	CSpeakerUpr*            m_pSpeakerUpr;
	CDialogLineDatabase*        m_pDialogLineDatabase;
	CDataImportHelper*          m_pDataImporterHelper;
	CVariableCollectionUpr* m_pVariableCollectionUpr;
	CResponseUpr*           m_pResponseUpr;
	ResponseActorList           m_createdActors;
	string                      m_filesFolder;
	CTimeValue                  m_currentTime;
	bool                        m_bIsCurrentlyUpdating;
	u32                      m_pendingResetRequest;

	ICVar*                      m_pUsedFileFormat;
	ICVar*                      m_pDataPath;
};

enum { ESYSTEM_EVENT_INITIALIZE_DRS = ESYSTEM_EVENT_GAME_POST_INIT_DONE };
}
