// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CPredictiveFloatTracker
{
public:
	struct SequenceEntry
	{
		i32 sequenceId;
		u32 lastValue;
		u32 lastDelta;
		u32 lastTime;
		i32 quantized;
		i32 predicted;
		u32 mementoAge;
		u32 timeFraction32;
		float size;
		u32 symSize;
		u32 totalSize;
	};

	struct Sequence
	{
		CTimeValue m_lastUseTime;
		std::vector<SequenceEntry> mEntries;

		void Use();
		float GetSecondsFromLastUse() const;
	};

public:
	CPredictiveFloatTracker(const string& dir, i32 errorThreshold, float quantizationScale);
	virtual ~CPredictiveFloatTracker();

	void LogSequence(Sequence* pSequence, const string& file);
	void Manage(tukk szPolicyName, i32 channel);
	void Track(i32 mementoId, i32 mementoSequence, u32 lastValue, u32 lastDelta, u32 lastTime, i32 quantized, i32 predicted, u32 mementoAge, u32 timeFraction32, float size, u32 symSize, u32 totalSize);

private:
	Sequence* GetSequence(i32 mementoId);

private:
	float m_quantizationScale;
	i32 m_errorThreshold;
	string m_directory;
	std::map<i32, Sequence*> m_sequences;

	DrxMutex m_mutex;
};
