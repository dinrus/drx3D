// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __sequencerundo_h__
#define __sequencerundo_h__

#if _MSC_VER > 1000
	#pragma once
#endif

class CSequencerTrack;
class CSequencerNode;
class CSequencerSequence;

/** Undo object stored when track is modified.
 */
class CUndoSequencerSequenceModifyObject : public IUndoObject
{
public:
	CUndoSequencerSequenceModifyObject(CSequencerTrack* track, CSequencerSequence* pSequence);
protected:
	virtual tukk GetDescription() { return "Track Modify"; };

	virtual void        Undo(bool bUndo);
	virtual void        Redo();

private:
	TSmartPtr<CSequencerTrack>    m_pTrack;
	TSmartPtr<CSequencerSequence> m_pSequence;
	XmlNodeRef                    m_undo;
	XmlNodeRef                    m_redo;
};

class CAnimSequenceUndo
	: public CUndo
{
public:
	CAnimSequenceUndo(CSequencerSequence* pSeq, tukk sName)
		: CUndo(sName)
	{
		//	CUndo::Record( new CUndoSequencerAnimSequenceObject(pSeq) );
	}
};

#endif // __sequencerundo_h__

