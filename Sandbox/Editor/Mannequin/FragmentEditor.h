// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FRAGMENT_EDITOR_H__
#define __FRAGMENT_EDITOR_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "Dialogs/ToolbarDialog.h"
#include "SequencerSplitter.h"
#include "Controls\MannDopeSheet.h"

#include "Controls/PropertiesPanel.h"

#include "Dialogs\BaseFrameWnd.h"

#include "IDrxMannequin.h"

#include "MannequinBase.h"

class CFragment;
class IAnimationDatabase;

// CMannFragmentEditor dialog
class CMannFragmentEditor : public CMannDopeSheet
{
	DECLARE_DYNAMIC(CMannFragmentEditor)

public:
	CMannFragmentEditor();
	~CMannFragmentEditor();

	void                     InitialiseToPreviewFile(const XmlNodeRef& xmlSequenceRoot);

	void                     SetMannequinContexts(SMannequinContexts* contexts);
	CSequencerSequence*      GetSequence() { return m_pSequence; };

	SFragmentHistoryContext* GetFragmentHistory()
	{
		return m_fragmentHistory;
	}

	void         SetCurrentFragment();
	void         SetFragment(const FragmentID fragID, const SFragTagState& tagState = SFragTagState(), u32k fragOption = 0);

	ActionScopes GetFragmentScopeMask() const
	{
		return m_fragmentScopeMask;
	}

	FragmentID GetFragmentID() const
	{
		return m_fragID;
	}

	FragmentID GetFragmentOptionIdx() const
	{
		return m_fragOptionIdx;
	}

	void SetTagState(const SFragTagState& tagState)
	{
		m_tagState = tagState;
	}
	const SFragTagState& GetTagState() const
	{
		return m_tagState;
	}

	float GetMaxTime() const
	{
		return m_realTimeRange.end;
	}

	void OnEditorNotifyEvent(EEditorNotifyEvent event);
	void OnAccept();

	void FlushChanges()
	{
		if (m_bEditingFragment)
		{
			StoreChanges();
			m_bEditingFragment = false;
		}
	}

protected:
	//void DrawKeys( CSequencerTrack *track,CDC *dc,CRect &rc,Range &timeRange );
	void UpdateFragmentClips();

	void StoreChanges();

	void OnInternalVariableChange(IVariable* pVar);

	SMannequinContexts*      m_contexts;
	SFragmentHistoryContext* m_fragmentHistory;

	FragmentID               m_fragID;
	u32                   m_fragOptionIdx;
	SFragTagState            m_tagState;

	ActionScopes             m_fragmentScopeMask;

	CFragment                m_fragmentTEMP;

	float                    m_motionParams[eMotionParamID_COUNT];

	bool                     m_bEditingFragment;
};

#endif // __FRAGMENT_EDITOR_H__

