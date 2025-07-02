// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/HMDUpr.h>
#include <drx3D/Sys/HMDCVars.h>

#include <drx3D/CoreX/Renderer/IStereoRenderer.h>

#include <drx3D/Sys/IDrxPluginUpr.h>

// Note:
//  We support a single HMD device at a time.
// This manager will be updated accordingly as other HMDs come, based on their system's init/shutdown, etc requirements.
// That may imply changes in the interface.

CHmdUpr::~CHmdUpr()
{
}

// ------------------------------------------------------------------------
void CHmdUpr::RegisterDevice(tukk szDeviceName, IHmdDevice& device)
{
	// Reference counting will be handled inside the vector
	m_availableDeviceMap.insert(TDeviceMap::value_type(szDeviceName, &device));
}

// ------------------------------------------------------------------------
void CHmdUpr::UnregisterDevice(tukk szDeviceName)
{
	auto it = m_availableDeviceMap.find(szDeviceName);
	if (it == m_availableDeviceMap.end())
		return;

	// If we lost selected device, nullify it
	if (m_pHmdDevice == it->second)
	{
		m_pHmdDevice = nullptr;
		if (gEnv->pRenderer != nullptr)
			gEnv->pRenderer->GetIStereoRenderer()->OnHmdDeviceChanged(m_pHmdDevice);
	}

	m_availableDeviceMap.erase(it);
}

// ------------------------------------------------------------------------
void CHmdUpr::OnVirtualRealityDeviceChanged(ICVar *pCVar)
{
	gEnv->pSystem->GetHmdUpr()->SetupAction(EHmdSetupAction::eHmdSetupAction_Init);
}

// ------------------------------------------------------------------------
void CHmdUpr::SetupAction(EHmdSetupAction cmd)
{
	LOADING_TIME_PROFILE_SECTION;
	switch (cmd)
	{
	case EHmdSetupAction::eHmdSetupAction_CreateCvars:
		DrxVR::CVars::Register();
		break;
	// ------------------------------------------------------------------------
	case EHmdSetupAction::eHmdSetupAction_PostInit: // Nothing to do for Oculus after SDK 0.6.0
		break;
	// ------------------------------------------------------------------------
	case EHmdSetupAction::eHmdSetupAction_Init:
		{
			if (gEnv->pConsole)
			{
				ICVar* pVrSupportVar = gEnv->pConsole->GetCVar("sys_vr_support");

				if (pVrSupportVar->GetIVal() > 0)
				{
					m_pHmdDevice = nullptr;

					tukk selectedHmdName = DrxVR::CVars::pSelectedHmdNameVar->GetString();
					TDeviceMap::iterator hmdIt = m_availableDeviceMap.end();

					if (strlen(selectedHmdName) > 0)
					{
						hmdIt = m_availableDeviceMap.find(selectedHmdName);
						if (hmdIt == m_availableDeviceMap.end())
						{
							pVrSupportVar->Set(0);

							if (gEnv->pRenderer != nullptr)
							{
								gEnv->pRenderer->GetIStereoRenderer()->OnHmdDeviceChanged(m_pHmdDevice);
							}

							DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Tried to select unavailable VR device %s!", selectedHmdName);
							return;
						}
					}
					else // No HMD explicitly selected, find a suitable one (since sys_vr_support was 1)
					{
						tukk vrPluginPriorities[] = {
							"Plugin_OculusVR",
							"Plugin_OpenVR"
						};

						for (const auto *plug : vrPluginPriorities)
						{
							if ((hmdIt = m_availableDeviceMap.find(plug)) != m_availableDeviceMap.end())
								break;
						}

						// Resort to whatever is available
						if (hmdIt == m_availableDeviceMap.end())
							hmdIt = m_availableDeviceMap.begin();
						if (hmdIt == m_availableDeviceMap.end())
						{
							pVrSupportVar->Set(0);

							if (gEnv->pRenderer != nullptr)
							{
								gEnv->pRenderer->GetIStereoRenderer()->OnHmdDeviceChanged(m_pHmdDevice);
							}

							DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "VR support was enabled, but no VR device was detected!");
							return;
						}
					}

					m_pHmdDevice = hmdIt->second;

					if (gEnv->pRenderer != nullptr)
					{
						gEnv->pRenderer->GetIStereoRenderer()->OnHmdDeviceChanged(m_pHmdDevice);
					}

					gEnv->pSystem->LoadConfiguration("vr.cfg", 0, eLoadConfigGame);
				}
				else if(m_pHmdDevice != nullptr)
				{
					m_pHmdDevice = nullptr;

					gEnv->pRenderer->GetIStereoRenderer()->OnHmdDeviceChanged(m_pHmdDevice);
				}
			}
		}
		break;

	default:
		assert(0);
	}

}

// ------------------------------------------------------------------------
void CHmdUpr::Action(EHmdAction action)
{
	if (m_pHmdDevice)
	{
		switch (action)
		{
		case EHmdAction::eHmdAction_DrawInfo:
			m_pHmdDevice->UpdateInternal(IHmdDevice::eInternalUpdate_DebugInfo);
			break;
		default:
			assert(0);
		}
	}
}

// ------------------------------------------------------------------------
void CHmdUpr::UpdateTracking(EVRComponent updateType)
{
	if (m_pHmdDevice)
	{
		IRenderer* pRenderer = gEnv->pRenderer;
		i32k frameId = pRenderer->GetFrameID(false);

		m_pHmdDevice->UpdateTrackingState(updateType, frameId);
	}
}

// ------------------------------------------------------------------------
bool CHmdUpr::IsStereoSetupOk() const
{
	if (m_pHmdDevice)
	{
		if (IStereoRenderer* pStereoRenderer = gEnv->pRenderer->GetIStereoRenderer())
		{
			EStereoDevice device = EStereoDevice::STEREO_DEVICE_NONE;
			EStereoMode mode = EStereoMode::STEREO_MODE_NO_STEREO;
			EStereoOutput output = EStereoOutput::STEREO_OUTPUT_STANDARD;

			pStereoRenderer->GetInfo(&device, &mode, &output, 0);

			return (
			  (device == EStereoDevice::STEREO_DEVICE_DEFAULT || device == EStereoDevice::STEREO_DEVICE_FRAMECOMP) &&
			  (mode == EStereoMode::STEREO_MODE_POST_STEREO || mode == EStereoMode::STEREO_MODE_DUAL_RENDERING || mode == EStereoMode::STEREO_MODE_MENU) &&
			  (output == EStereoOutput::STEREO_OUTPUT_SIDE_BY_SIDE || output == EStereoOutput::STEREO_OUTPUT_HMD)
			  );
		}
	}
	return false;
}

// ------------------------------------------------------------------------
HMDCameraSetup CHmdUpr::GetHMDCameraSetup(i32 nEye, float projRatio, float fnear) const
{
	if (m_pHmdDevice)
		return m_pHmdDevice->GetHMDCameraSetup(nEye, projRatio, fnear);

	return HMDCameraSetup{};
}

// ------------------------------------------------------------------------
void CHmdUpr::RecenterPose()
{
	// All HMDs subscribe to our events, so simply notify listeners and reaction will occur
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		(*it)->OnRecentered();
	}
}