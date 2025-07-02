// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Schema/GUID.h>

class CSchematycEntityDrsComponent final : public IEntityComponent, DRS::IResponseUpr::IListener, DRS::ISpeakerUpr::IListener
{
public:

	struct SResponseStartedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SResponseStartedSignal>& typeInfo);

		i32   m_signalId;
	};
	struct SResponseFinishedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SResponseFinishedSignal>& typeInfo);

		i32   m_signalId;
		i32   m_result;  //ProcessingResult_NoResponseDefined, ProcessingResult_ConditionsNotMet, ProcessingResult_Done, ProcessingResult_Canceled	
	};
	struct SLineStartedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SLineStartedSignal>& typeInfo);

		sxema::CSharedString  m_text;
		sxema::CSharedString  m_speakerName;
		//animation, audioTrigger... do we need these as well?
	};
	struct SLineEndedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SLineEndedSignal>& typeInfo);

		sxema::CSharedString  m_text;
		sxema::CSharedString  m_speakerName;
		bool    m_bWasCanceled;
		//animation, audioTrigger... do we need these as well?
	};

	CSchematycEntityDrsComponent() = default;
	virtual ~CSchematycEntityDrsComponent() = default;

	//IEntityComponent
	virtual void Initialize() override;
	virtual void OnShutDown() override;
	// ~IEntityComponent

	// DRS::IResponseUpr::IListener
	virtual void OnSignalProcessingStarted(SSignalInfos& signal, DRS::IResponseInstance* pStartedResponse) override;
	virtual void OnSignalProcessingFinished(SSignalInfos& signal, DRS::IResponseInstance* pFinishedResponse, eProcessingResult outcome) override;
	// ~DRS::IResponseUpr::IListener

	// DRS::ISpeakerUpr::IListener
	virtual void OnLineEvent(const DRS::IResponseActor* pSpeaker, const CHashedString& lineID, DRS::ISpeakerUpr::IListener::eLineEvent lineEvent, const DRS::IDialogLine* pLine) override;
	// ~DRS::ISpeakerUpr::IListener

	static void ReflectType(sxema::CTypeDesc<CSchematycEntityDrsComponent>& desc);
	static void Register(sxema::IEnvRegistrar& registrar);

private:
	template <typename SIGNAL> inline void OutputSignal(const SIGNAL& signal)
	{
		if (GetEntity()->GetSchematycObject())
			GetEntity()->GetSchematycObject()->ProcessSignal(signal, m_guid);
	}

	void SendSignal(const sxema::CSharedString& signalName, const sxema::CSharedString& contextFloatName, float contextFloatValue, const sxema::CSharedString& contextStringName, const sxema::CSharedString& contextStringValue);
	
	void SetFloatVariable(const sxema::CSharedString& collectionName, const sxema::CSharedString& variableName, float value);
	void SetStringVariable(const sxema::CSharedString& collectionName, const sxema::CSharedString& variableName, const sxema::CSharedString& value);
	void SetIntVariable(const sxema::CSharedString& collectionName, const sxema::CSharedString& variableName, i32 value);

	DRS::IVariableCollection* GetVariableCollection(const sxema::CSharedString& collectionName);

	sxema::CSharedString m_nameOverride;
	sxema::CSharedString m_globalVariableCollectionToUse;

	IEntityDynamicResponseComponent* m_pDrsEntityComp;
};
