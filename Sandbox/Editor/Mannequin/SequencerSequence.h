// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SEQUENCER_SEQUENCE__H__
#define __SEQUENCER_SEQUENCE__H__

#include "ISequencerSystem.h"

class CSequencerSequence
	: public _i_reference_target_t
{
public:
	CSequencerSequence();
	virtual ~CSequencerSequence();

	void            SetName(tukk name);
	tukk     GetName() const;

	void            SetTimeRange(Range timeRange);
	Range           GetTimeRange() { return m_timeRange; }

	i32             GetNodeCount() const;
	CSequencerNode* GetNode(i32 index) const;

	CSequencerNode* FindNodeByName(tukk sNodeName);
	void            ReorderNode(CSequencerNode* node, CSequencerNode* pPivotNode, bool next);

	bool            AddNode(CSequencerNode* node);
	void            RemoveNode(CSequencerNode* node);

	void            RemoveAll();

	void            UpdateKeys();

private:
	typedef std::vector<_smart_ptr<CSequencerNode>> SequencerNodes;
	SequencerNodes m_nodes;

	string         m_name;
	Range          m_timeRange;
};

#endif

