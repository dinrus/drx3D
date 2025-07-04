// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#include  <drx3D/Network/PredictiveFloatTracker.h>

///////////////////////////////////////////////////////////////////////////////////////////
// CPredictiveFloatTracker::Sequence
///////////////////////////////////////////////////////////////////////////////////////////
void CPredictiveFloatTracker::Sequence::Use()
{
	m_lastUseTime = gEnv->pTimer->GetAsyncTime();
}

float CPredictiveFloatTracker::Sequence::GetSecondsFromLastUse() const
{
	return gEnv->pTimer->GetAsyncTime().GetDifferenceInSeconds(m_lastUseTime);
}

///////////////////////////////////////////////////////////////////////////////////////////
// CPredictiveFloatTracker
///////////////////////////////////////////////////////////////////////////////////////////
CPredictiveFloatTracker::CPredictiveFloatTracker(const string& dir, i32 errorThreshold, float quantizationScale)
{
	m_quantizationScale = quantizationScale;
	m_errorThreshold = errorThreshold;
	m_directory = dir;
}

CPredictiveFloatTracker::~CPredictiveFloatTracker()
{
	DrxAutoLock<DrxMutex> lock(m_mutex);

	for (auto k : m_sequences)
		delete k.second;

	m_sequences.clear();
}

CPredictiveFloatTracker::Sequence* CPredictiveFloatTracker::GetSequence(i32 mementoId)
{
	auto it = m_sequences.find(mementoId);
	if (it != m_sequences.end())
	{
		it->second->Use();
		return it->second;
	}

	Sequence* s = new Sequence;
	s->Use();

	m_sequences[mementoId] = s;

	return s;
}

void CPredictiveFloatTracker::LogSequence(Sequence* pSequence, const string& file)
{
	FILE* f = gEnv->pDrxPak->FOpen(file.c_str(), "wb");
	if (!f)
		return;

	float size = 0.f;
	for (auto e : pSequence->mEntries)
		size += e.size;

	gEnv->pDrxPak->FPrintf(f, "Avg size: %3.3f\n", size / (float)pSequence->mEntries.size());

	for (auto e : pSequence->mEntries)
	{
		i32 error = e.predicted - e.quantized;
		gEnv->pDrxPak->FPrintf(f, "BSEQ: % 5i SEQ: % 5i E: % 7i P: % 3.3f V: % 3.3f Q: % 3.3f DV: % 3.3f A: % 2i dT: % 7i T: %07i s: %0.3f sym: %i %i\n", e.sequenceId - e.mementoAge, e.sequenceId, error, e.predicted * m_quantizationScale, e.lastValue * m_quantizationScale, e.quantized * m_quantizationScale, (e.quantized - (i32)e.lastValue) * m_quantizationScale, e.mementoAge, e.timeFraction32 - e.lastTime, e.timeFraction32 & 0xfffff, e.size, e.symSize, e.totalSize);
	}

	gEnv->pDrxPak->FClose(f);
}

void CPredictiveFloatTracker::Manage(tukk szPolicyName, i32 channel)
{
	m_mutex.Lock();
	std::map<i32, Sequence*> map_copy = m_sequences;
	m_mutex.Unlock();

	for (auto entry : map_copy)
	{
		string file;
		Sequence copy;
		{
			DrxAutoLock<DrxMutex> lock(m_mutex);

			Sequence* s = entry.second;
			file.Format("%s%s_%i_%i.log", m_directory.c_str(), szPolicyName, channel, entry.first);

			copy.mEntries = s->mEntries;
		}


		LogSequence(&copy, file);
	}
}

void CPredictiveFloatTracker::Track(i32 mementoId, i32 mementoSequence, u32 lastValue, u32 lastDelta, u32 lastTime, i32 quantized, i32 predicted, u32 mementoAge, u32 timeFraction32, float size, u32 symSize, u32 totalSize)
{
/*	if (quantized == lastValue)
		return;

	if (abs(quantized - predicted) < m_errorThreshold)
		return;*/

	DrxAutoLock<DrxMutex> lock(m_mutex);

	SequenceEntry entry = {
		mementoSequence,
		lastValue,
		lastDelta,
		lastTime,
		quantized,
		predicted,
		mementoAge,
		timeFraction32,
		size,
		symSize, 
		totalSize
	};

	Sequence* s = GetSequence(mementoId);
	s->mEntries.push_back(entry);
}
