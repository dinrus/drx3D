// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/StatoscopeStreamingIntervalGroup.h>

#if ENABLE_STATOSCOPE

CStatoscopeStreamingIntervalGroup::CStatoscopeStreamingIntervalGroup()
	: CStatoscopeIntervalGroup('s', "streaming",
	                           "['/Streaming/' "
	                           "(string filename) "
	                           "(i32 stage) "
	                           "(i32 priority) "
	                           "(i32 source) "
	                           "(i32 perceptualImportance) "
	                           "(int64 sortKey) "
	                           "(i32 compressedSize) "
	                           "(i32 mediaType)"
	                           "]")
{
}

void CStatoscopeStreamingIntervalGroup::Enable_Impl()
{
	gEnv->pSystem->GetStreamEngine()->SetListener(this);
}

void CStatoscopeStreamingIntervalGroup::Disable_Impl()
{
	gEnv->pSystem->GetStreamEngine()->SetListener(NULL);
}

void CStatoscopeStreamingIntervalGroup::OnStreamEnqueue(ukk pReq, tukk filename, EStreamTaskType source, const StreamReadParams& readParams)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		size_t payloadLen =
		  GetValueLength(filename) +
		  GetValueLength(Stage_Waiting) +
		  GetValueLength(0) * 5 +
		  GetValueLength((int64)0);

		StatoscopeDataWriter::EventBeginInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventBeginInterval>(payloadLen);
		pEv->id = reinterpret_cast<UINT_PTR>(pReq);
		pEv->classId = GetId();

		tuk pPayload = (tuk)(pEv + 1);
		WriteValue(pPayload, filename);
		WriteValue(pPayload, Stage_Waiting);
		WriteValue(pPayload, (i32)readParams.ePriority);
		WriteValue(pPayload, (i32)source);
		WriteValue(pPayload, (i32)readParams.nPerceptualImportance);
		WriteValue(pPayload, (int64)0);
		WriteValue(pPayload, (i32)0);
		WriteValue(pPayload, (i32)0);

		pWriter->EndEvent();
	}
}

void CStatoscopeStreamingIntervalGroup::OnStreamComputedSortKey(ukk pReq, uint64 key)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		size_t payloadLen = GetValueLength((int64)key);
		StatoscopeDataWriter::EventModifyInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventModifyInterval>(payloadLen);
		pEv->id = reinterpret_cast<UINT_PTR>(pReq);
		pEv->classId = GetId();
		pEv->field = 5;

		tuk pPayload = (tuk)(pEv + 1);
		WriteValue(pPayload, (int64)key);

		pWriter->EndEvent();
	}
}

void CStatoscopeStreamingIntervalGroup::OnStreamBeginIO(ukk pReq, u32 compressedSize, u32 readSize, EStreamSourceMediaType mediaType)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		{
			size_t payloadLen = GetValueLength((i32)compressedSize);
			StatoscopeDataWriter::EventModifyInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventModifyInterval>(payloadLen);
			pEv->id = reinterpret_cast<UINT_PTR>(pReq);
			pEv->classId = GetId();
			pEv->field = 6;

			tuk pPayload = (tuk)(pEv + 1);
			WriteValue(pPayload, (i32)compressedSize);

			pWriter->EndEvent();
		}
		{
			size_t payloadLen = GetValueLength((i32)mediaType);
			StatoscopeDataWriter::EventModifyInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventModifyInterval>(payloadLen);
			pEv->id = reinterpret_cast<UINT_PTR>(pReq);
			pEv->classId = GetId();
			pEv->field = 7;

			tuk pPayload = (tuk)(pEv + 1);
			WriteValue(pPayload, (i32)mediaType);

			pWriter->EndEvent();
		}
	}

	WriteChangeStageEvent(pReq, Stage_IO, true);
}

void CStatoscopeStreamingIntervalGroup::OnStreamEndIO(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_IO, false);
}

void CStatoscopeStreamingIntervalGroup::OnStreamBeginInflate(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Inflate, true);
}

void CStatoscopeStreamingIntervalGroup::OnStreamEndInflate(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Inflate, false);
}

void CStatoscopeStreamingIntervalGroup::OnStreamBeginDecrypt(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Decrypt, true);
}

void CStatoscopeStreamingIntervalGroup::OnStreamEndDecrypt(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Decrypt, false);
}

void CStatoscopeStreamingIntervalGroup::OnStreamBeginAsyncCallback(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Async, true);
}

void CStatoscopeStreamingIntervalGroup::OnStreamEndAsyncCallback(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Async, false);
}

void CStatoscopeStreamingIntervalGroup::OnStreamPreempted(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Preempted, true);
}

void CStatoscopeStreamingIntervalGroup::OnStreamResumed(ukk pReq)
{
	WriteChangeStageEvent(pReq, Stage_Preempted, false);
}

void CStatoscopeStreamingIntervalGroup::OnStreamDone(ukk pReq)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		StatoscopeDataWriter::EventEndInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventEndInterval>();
		pEv->id = reinterpret_cast<UINT_PTR>(pReq);
		pWriter->EndEvent();
	}
}

void CStatoscopeStreamingIntervalGroup::WriteChangeStageEvent(ukk pReq, i32 stage, bool entering)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		size_t payloadLen = GetValueLength(stage) * 2;
		StatoscopeDataWriter::EventModifyIntervalBit* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventModifyIntervalBit>(payloadLen);
		pEv->id = reinterpret_cast<UINT_PTR>(pReq);
		pEv->classId = GetId();
		pEv->field = StatoscopeDataWriter::EventModifyInterval::FieldSplitIntervalMask | 1;

		tuk pPayload = (tuk)(pEv + 1);

		if (entering)
		{
			WriteValue(pPayload, (i32)-1);
			WriteValue(pPayload, (i32)stage);
		}
		else
		{
			WriteValue(pPayload, (i32)~stage);
			WriteValue(pPayload, (i32)0);
		}

		pWriter->EndEvent();
	}
}

#endif
