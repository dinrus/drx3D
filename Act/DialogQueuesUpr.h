// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DIALOGQUEUESMANAGER_H__
#define __DIALOGQUEUESMANAGER_H__

class CDialogSession;

#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/Network/ISerializeHelper.h>

#ifndef _RELEASE
	#define DEBUGINFO_DIALOGBUFFER
#endif

// this works like a 'buffered channel' feature:.
// - Dialogs can be assigned to a buffer.
// - When a dialog that is being played is assigned to a buffer, any other dialog that tries to play through that buffer is hold in a waiting list, until all previous ones finish
class CDialogQueuesUpr
{
public:
	typedef i32 TDialogId;

	CDialogQueuesUpr();
	void      Reset();
	TDialogId Play(u32 queueID, const string& name);
	bool      IsDialogWaiting(u32 queueID, TDialogId dialogId);
	void      NotifyDialogDone(u32 queueID, TDialogId dialogId);
	u32    BufferEnumToId(u32 queueEnum); // queue enum means the values stored in the .xml definition file
	bool      IsBufferFree(u32 queueID);
	void      Serialize(TSerialize ser);
	void      Update();

	static u32k NO_QUEUE = (u32) - 1;  //use this special queueID to mark dialogs outside of the queuing system (will always be played)

private:

	TDialogId CreateNewDialogId()                  { DrxInterlockedIncrement(&m_uniqueDialogID); return m_uniqueDialogID; }
	bool      IsQueueIDValid(u32 queueID) const { return queueID < m_numBuffers; }  //Will return false for "no_queue" dialogs, which causes the dialog not to be queued, but to be started directly

	typedef std::vector<u32>  TBuffer;
	typedef std::vector<TBuffer> TBuffersList;

	TBuffersList m_buffersList;
	u32       m_numBuffers;     // == m_buffersList.size().
	i32          m_uniqueDialogID; // dialog ids come from increasing this

#ifdef DEBUGINFO_DIALOGBUFFER
	typedef std::vector<string>         TBufferNames;
	typedef std::map<TDialogId, string> TDialogNames;
	TBufferNames m_bufferNames;
	TDialogNames m_dialogNames;
#endif
};

#endif
