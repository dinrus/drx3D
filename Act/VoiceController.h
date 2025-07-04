// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VOICECONTROLLER_H__
#define __VOICECONTROLLER_H__

#pragma once

#ifndef OLD_VOICE_SYSTEM_DEPRECATED

	#include "ISound.h"

class CGameContext;

class CVoiceController
{
public:
	CVoiceController(CGameContext* pGameContext);
	~CVoiceController();
	bool Init();
	void Enable(const bool e);
	void PlayerIdSet(EntityId id);

	void GetMemoryStatistics(IDrxSizer* s)
	{
		s->Add(*this);
	}
private:
	IMicrophone* m_pMicrophone;
	INetContext* m_pNetContext;
	bool         m_enabled;
	bool         m_recording;

	struct CDataReader : public IVoiceDataReader
	{
		CDataReader()
		{
			m_pMicrophone = 0;
		}

		IMicrophone* m_pMicrophone;

		virtual bool Update()
		{
			if (m_pMicrophone)
			{
				m_pMicrophone->Update();
				return true;
			}
			else
				return false;
		}

		virtual u32 GetSampleCount()
		{
			return m_pMicrophone ? m_pMicrophone->GetDataSize() / 2 : 0;
		}

		virtual i16* GetSamples()
		{
			return m_pMicrophone ? m_pMicrophone->GetData() : 0;
		}
	};

	CDataReader m_DataReader;
};

#endif

#endif
