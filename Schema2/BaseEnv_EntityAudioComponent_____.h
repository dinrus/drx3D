// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/BaseEnv_EntityComponentBase.h>
#include <drx3D/Schema2/BaseEnv_EntitySignals.h>
//#include <drx3D/Schema2/EntityInterface.h>

//#include <drx3D/Schema2/GameAudio/GameAudioEntityController.h>

#include <drx3D/Schema2/TemplateUtils_Delegate.h>

namespace sxema
{
	namespace AudioProxyAttachType
	{
		enum EValue
		{
			FixedOffset = 0,
			CharacterJoint,
		};
	}

	namespace Entity
	{
		class CAudioComponent : public CComponentBase, public CEntityInterface<CAudioComponent>
		{
		private:
			typedef std::vector<u32> TDynamicAudioProxies;

			struct SPreviewProperties
			{
				SPreviewProperties()
					: showAudioProxies(false)
				{
				}

				void Serialize(Serialization::IArchive& archive);

				bool  showAudioProxies;
			};

		public:
			struct SAudioProxyProperties
			{
				SAudioProxyProperties()
					: attachType(AudioProxyAttachType::FixedOffset)
					, offset(ZERO)
					, obstructionType(GameAudio::eObstructionType_MultiRay)
				{
				}

				void Serialize(Serialization::IArchive& archive);

				AudioProxyAttachType::EValue attachType;
				Vec3    offset;
				string  characterJoint;
				IGameAudio::EObstructionType  obstructionType;
			};

			struct SAuxAudioProxy
			{
				SAuxAudioProxy()
				{
				}

				void Serialize(Serialization::IArchive& archive);

				string name;
				SAudioProxyProperties properties;
			};

			typedef std::vector<SAuxAudioProxy> TAuxiliarProxies;

			struct SAudioSwitchSetting
			{
				SAudioSwitchSetting()
				{
				}

				SAudioSwitchSetting(string _switchName, string _switchStateName)
					: switchName(_switchName)
					, switchStateName(_switchStateName)
				{
				}

				sxema2::SAudioSwitchName switchName;
				sxema2::SAudioSwitchStateName switchStateName;

				void Serialize(Serialization::IArchive& archive);
			};

			typedef std::vector<SAudioSwitchSetting> TAudioSwitchSettings;

			struct SProperties
			{
				SProperties()
					: dynamicAudioObjectsObstructionType(GameAudio::eObstructionType_MultiRay)
				{

				}

				void Serialize(Serialization::IArchive& archive);

				SAudioProxyProperties defaultProxyProperties;
				TAuxiliarProxies      auxiliaryProxies;
				TAudioSwitchSettings  audioSwitchSettings;

				IGameAudio::EObstructionType dynamicAudioObjectsObstructionType;
			};

			// IComponent
			virtual bool Init(const SComponentParams& params) override;
			virtual void Run(ESimulationMode simulationMode) override;
			virtual IAnyPtr GetPreviewProperties() const override;
			virtual void Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const IAnyConstPtr& pPreviewProperties) const override;
			virtual void Shutdown() override;
			// ~IComponent

			// Audio playback
			void ExecuteTrigger(tukk szAuxProxyName, const TAudioControlID triggerId, const TAudioControlID stopTriggerId);
			void ExecuteTrigger(u32k proxyIndex, const TAudioControlID triggerId, const TAudioControlID stopTriggerId);
			void StopTrigger(const TAudioControlID triggerId, const string& auxProxyName);
			
			void SetSwitchState(const sxema2::SAudioSwitchName& switchName, const sxema2::SAudioSwitchStateName& switchStateName);
			void SetRtpc(tukk szAuxProxyName, const sxema2::SRtpcName& rtpcName, const float rtpcValue);
			void SetRtpcForProxy(u32k proxyIndex, tukk szRtpcName, const float rtpcValue);
			
			u32 GetAudioProxyIndex(tukk szAuxProxyName) const;
			u32 GetAudioProxyId(tukk szAuxProxyName) const;

			u32 AddDynamicAudioProxyAtCharacterJoint(i32k jointId);

			void AttachAudioController(GameAudio::CEntityControllerPtr pChildController);
			bool DetachAudioController(GameAudio::CEntityControllerPtr pChildController);
			void CloneAudioController(GameAudio::CEntityController& controller) const;

			static void Register();
			static void GenerateAudioProxyStringList(Serialization::IArchive& archive, Serialization::StringList& stringList, i32& iDefaultValue);

			static const SGUID COMPONENT_GUID;
			static const SGUID SET_SWITCH_STATE_FUNCTION_GUID;
			static const SGUID SET_RTPC_FUNCTION_GUID;

		private:

			// Misc
			void   Initialize();
			u32 GetAudioProxiesBoundToCharacterJointCount() const;
			void   GetLocalOffsetForProxyFromProperties(const ICharacterInstance* pCharacterInstance, const SAudioProxyProperties& proxyProperties, Matrix34& outLocation);
			void   GetLocalOffsetForProxyFromJoint(const ICharacterInstance& characterInstance, i32k jointId, Matrix34& outLocation);
			u32 FindAuxProxyIndexByName(tukk szAuxProxyName) const;
			const ICharacterInstance* GetCharacter() const;

			bool AudioProxyNeedsUpdate(u32k proxyIdx) const;
			void RegisterForUpdate();
			void UnregisterForUpdate();
			void Update(const SUpdateContext& updateContext);

			static void GenerateAudioProxyStringListForSettings(Serialization::IArchive& archive, Serialization::StringList& stringList, i32& iDefaultValue);
			static void GenerateStringListFromProxies(const TAuxiliarProxies* pProxies, Serialization::StringList& stringList, i32& iDefaultValue);

		private:

			SProperties                  m_properties;
			GameAudio::CEntityController m_audioController;
			TDynamicAudioProxies         m_dynamicAudioProxies;

			struct
			{
				CUpdateScope                 update;
			} m_signalScopes;
		};
	}
}