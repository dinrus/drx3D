// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if !defined(_RELEASE)
	#define MANUAL_FRAME_STEP_ENABLED 1
#else
	#define MANUAL_FRAME_STEP_ENABLED 0
#endif

#include <drx3D/Input/IInput.h>
#include <drx3D/Network/NetHelpers.h>

enum class EManualFrameStepResult
{
	Continue,
	Block
};

#if MANUAL_FRAME_STEP_ENABLED

class CManualFrameStepController final
	: public IManualFrameStepController
	, public CNetMessageSinkHelper<CManualFrameStepController, INetMessageSink>
	, public IInputEventListener
	, public ISystemEventListener
{
private:
	enum : u8
	{
		kEnableMask = 0,
		kDisableMask = (u8) ~0
	};

	struct SCVars
	{
		static void Register();
		static void Unregister();

		static float manualFrameStepFrequency;
	};

	struct SNetMessage : public ISerializable
	{
		SNetMessage()
			: numFrames(0)
		{
		}

		SNetMessage(u8k _numFrames)
			: numFrames(_numFrames)
		{
		}

		// ISerializable
		virtual void SerializeWith(TSerialize ser) override
		{
			ser.Value("numFrames", numFrames, 'u8');
		}
		//~ISerializable

		u8 numFrames;
	};

public:
	CManualFrameStepController();
	~CManualFrameStepController();

	EManualFrameStepResult Update();

	// IManualFrameStepController
	virtual void DefineProtocols(IProtocolBuilder* pBuilder) override { DefineProtocol(pBuilder); }
	// ~IManualFrameStepController

	// INetMessageSink
	virtual void DefineProtocol(IProtocolBuilder* pBuilder) override;
	// ~INetMessageSink

	NET_DECLARE_SIMPLE_ATSYNC_MESSAGE(OnNetworkMessage, SNetMessage);

private:
	bool IsEnabled() const;
	void Enable(bool bEnable);
	void GenerateRequest(u8k numFrames);
	void ProcessCommand(u8k numFrames);
	void NetSyncClients(const SNetMessage& netMessage);
	void NetRequestStepToServer(const SNetMessage& netMessage);

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& inputEvent) override;
	// ~IInputEventListener

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	static void GetFramesFolder(stack_string& outFolder);
	static void DisplayDebugInfo();

private:
	i32  m_framesLeft;
	u32 m_framesGenerated;
	float  m_previousFixedStep;
	float  m_heldTimer;
	bool   m_pendingRequest;
	bool   m_previousStepSmoothing;
};

#else
class CManualFrameStepController final
	: public IManualFrameStepController
{
public:
	inline EManualFrameStepResult Update()                                   { return EManualFrameStepResult::Continue; }
	
	// IManualFrameStepController
	virtual void DefineProtocols(IProtocolBuilder* pBuilder) override {};
	// ~IManualFrameStepController
};
#endif
