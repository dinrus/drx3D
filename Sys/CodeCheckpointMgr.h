// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     08/04/2010 by Will W (based on work by Matthew J)
//  Описание: A central class to track code checkpoint registration
// -------------------------------------------------------------------------
//  История: Created by Will Wilson based on work by Matthew Jack.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CODE_CHECKPOINT_MGR_H_
#define __CODE_CHECKPOINT_MGR_H_

#pragma once

#include <drx3D/Sys/ICodeCheckpointMgr.h>

/// The global manager of code coverage checkpoints. Must be thread safe.
class CCodeCheckpointMgr : public ICodeCheckpointMgr
{
public:

	CCodeCheckpointMgr();
	virtual ~CCodeCheckpointMgr();

	// ICodeCheckpointMgr implementation

	/// Used by code checkpoints to register themselves with the manager.
	virtual void RegisterCheckpoint(CCodeCheckpoint* pCheckpoint);

	//! Manually unregister checkpoints when their module gets invalidated
	virtual void UnRegisterCheckpoint(tukk szName);

	/// Looks up a checkpoint by name. If not found it bookmarks that record and returns an index to it.
	virtual size_t GetCheckpointIndex(tukk name);

	/// Performs a cheap lookup by index, will return NULL if checkpoint has not yet been registered.
	virtual const CCodeCheckpoint* GetCheckpoint(size_t checkpointIdx) const;

	/// Performs cheap lookup for checkpoint name, will always return a valid name for a valid input index.
	virtual tukk GetCheckPointName(size_t checkpointIdx) const;

	/// Returns the total number of checkpoints
	virtual size_t GetTotalCount() const;

	/// Returns the total number of checkpoints that have been hit at least once.
	virtual size_t GetTotalEncountered() const;

	/// Frees this instance from memory
	virtual void Release();

private:
	/// Returns index for record matching name or ~0 if not found.
	size_t FindRecordByName(tukk name) const;

private:
	struct CheckpointRecord
	{
		CCodeCheckpoint* m_pCheckpoint;   /// The checkpoint if registered otherwise NULL
		tukk      m_name;          /// The name of the checkpoint (owned if m_pCheckpoint is NULL)
	};

	mutable DrxCriticalSection m_critSection; /// Used to protect access to m_checkpoints from multiple threads

	typedef std::vector<CheckpointRecord> TCheckpointVector;
	TCheckpointVector m_checkpoints;          /// Set of checkpoints either registered or requested via indices
};

#endif // __CODE_CHECKPOINT_MGR_H_
