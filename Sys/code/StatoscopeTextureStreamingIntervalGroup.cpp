// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/StatoscopeTextureStreamingIntervalGroup.h>

#if ENABLE_STATOSCOPE

CStatoscopeTextureStreamingIntervalGroup::CStatoscopeTextureStreamingIntervalGroup()
	: CStatoscopeIntervalGroup('t', "streaming textures",
	                           "['/Textures/' "
	                           "(string filename) "
	                           "(i32 minMipWanted) "
	                           "(i32 minMipAvailable) "
	                           "(i32 inUse)"
	                           "]")
{
}

void CStatoscopeTextureStreamingIntervalGroup::Enable_Impl()
{
	if (gEnv->pRenderer)
	{
		gEnv->pRenderer->SetTextureStreamListener(this);
	}
}

void CStatoscopeTextureStreamingIntervalGroup::Disable_Impl()
{
	if (gEnv->pRenderer)
	{
		gEnv->pRenderer->SetTextureStreamListener(NULL);
	}
}

void CStatoscopeTextureStreamingIntervalGroup::OnCreatedStreamedTexture(uk pHandle, tukk name, i32 nMips, i32 nMinMipAvailable)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		size_t payloadLen =
		  GetValueLength(name) +
		  GetValueLength(0) * 3
		;

		StatoscopeDataWriter::EventBeginInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventBeginInterval>(payloadLen);
		pEv->id = reinterpret_cast<UINT_PTR>(pHandle);
		pEv->classId = GetId();

		tuk pPayload = (tuk)(pEv + 1);
		WriteValue(pPayload, name);
		WriteValue(pPayload, nMips);
		WriteValue(pPayload, nMinMipAvailable);
		WriteValue(pPayload, 0);

		pWriter->EndEvent();
	}
}

void CStatoscopeTextureStreamingIntervalGroup::OnUploadedStreamedTexture(uk pHandle)
{
}

void CStatoscopeTextureStreamingIntervalGroup::OnDestroyedStreamedTexture(uk pHandle)
{
	StatoscopeDataWriter::EventEndInterval* pEv = GetWriter()->BeginEvent<StatoscopeDataWriter::EventEndInterval>();
	pEv->id = reinterpret_cast<UINT_PTR>(pHandle);
	GetWriter()->EndEvent();
}

void CStatoscopeTextureStreamingIntervalGroup::OnTextureWantsMip(uk pHandle, i32 nMinMip)
{
	OnChangedMip(pHandle, 1, nMinMip);
}

void CStatoscopeTextureStreamingIntervalGroup::OnTextureHasMip(uk pHandle, i32 nMinMip)
{
	OnChangedMip(pHandle, 2, nMinMip);
}

void CStatoscopeTextureStreamingIntervalGroup::OnBegunUsingTextures(uk * pHandles, size_t numHandles)
{
	OnChangedTextureUse(pHandles, numHandles, 1);
}

void CStatoscopeTextureStreamingIntervalGroup::OnEndedUsingTextures(uk * pHandles, size_t numHandles)
{
	OnChangedTextureUse(pHandles, numHandles, 0);
}

void CStatoscopeTextureStreamingIntervalGroup::OnChangedTextureUse(uk * pHandles, size_t numHandles, i32 inUse)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		size_t payloadLen = GetValueLength(1);
		u32 classId = GetId();

		pWriter->BeginBlock();

		for (size_t i = 0; i < numHandles; ++i)
		{
			StatoscopeDataWriter::EventModifyInterval* pEv = pWriter->BeginBlockEvent<StatoscopeDataWriter::EventModifyInterval>(payloadLen);
			pEv->id = reinterpret_cast<UINT_PTR>(pHandles[i]);
			pEv->classId = classId;
			pEv->field = StatoscopeDataWriter::EventModifyInterval::FieldSplitIntervalMask | 3;

			tuk pPayload = (tuk)(pEv + 1);
			WriteValue(pPayload, inUse);

			pWriter->EndBlockEvent();
		}

		pWriter->EndBlock();
	}
}

void CStatoscopeTextureStreamingIntervalGroup::OnChangedMip(uk pHandle, i32 field, i32 nMinMip)
{
	CStatoscopeEventWriter* pWriter = GetWriter();

	if (pWriter)
	{
		size_t payloadLen = GetValueLength(nMinMip);
		u32 classId = GetId();

		StatoscopeDataWriter::EventModifyInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventModifyInterval>(payloadLen);
		pEv->id = reinterpret_cast<UINT_PTR>(pHandle);
		pEv->classId = classId;
		pEv->field = StatoscopeDataWriter::EventModifyInterval::FieldSplitIntervalMask | field;

		tuk pPayload = (tuk)(pEv + 1);
		WriteValue(pPayload, nMinMip);

		pWriter->EndEvent();
	}
}

#endif
