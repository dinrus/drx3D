// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SyncedFileSet.h>

#if SERVER_FILE_SYNC_MODE

	#include <drx3D/Network/INetwork.h>
	#include  <drx3D/Network/NetChannel.h>
	#include  <drx3D/Network/ContextView.h>

class CSend_BeginSyncFiles : public INetSendable
{
public:
	CSend_BeginSyncFiles(CContextView* pView, u32 syncSeq)
		: INetSendable(eMPF_BlocksStateChange, eNRT_ReliableUnordered)
		, m_pView(pView)
		, m_syncSeq(syncSeq)
	{
	}

	// INetSendable
	size_t GetSize()
	{
		return sizeof(*this);
	}

	EMessageSendResult Send(INetSender* pSender)
	{
		pSender->BeginMessage(m_pView->GetConfig().pBeginSyncFiles);
		pSender->ser.Value("syncseq", m_syncSeq, 'flid');
		return eMSR_SentOk;
	}

	void UpdateState(u32 nFromSeq, ENetSendableStateUpdate update)
	{
	}

	tukk GetDescription()
	{
		return "BeginSyncFiles";
	}

	void GetPositionInfo(SMessagePositionInfo& pos)
	{
	}
	// ~INetSendable

private:
	CContextView* m_pView;
	u32        m_syncSeq;
};

class CSend_BeginFile : public INetSendable
{
public:
	CSend_BeginFile(CContextView* pView, u32 id, const string& filename)
		: INetSendable(eMPF_BlocksStateChange, eNRT_ReliableUnordered)
		, m_pView(pView)
		, m_filename(filename)
		, m_id(id)
	{
	}

	// INetSendable
	size_t GetSize()
	{
		return sizeof(*this);
	}

	EMessageSendResult Send(INetSender* pSender)
	{
		pSender->BeginMessage(m_pView->GetConfig().pBeginSyncFile);
		pSender->ser.Value("id", m_id, 'flid');
		pSender->ser.Value("name", m_filename);
		return eMSR_SentOk;
	}

	void UpdateState(u32 nFromSeq, ENetSendableStateUpdate update)
	{
	}

	tukk GetDescription()
	{
		return "BeginFile";
	}

	void GetPositionInfo(SMessagePositionInfo& pos)
	{
	}
	// ~INetSendable

private:
	CContextView* m_pView;
	u32        m_id;
	string        m_filename;
};

class CSend_AddFileData : public INetSendable
{
public:
	CSend_AddFileData(CContextView* pView, u32 id, u32 base, u32 length, const CSyncedFileDataLock& flk)
		: INetSendable(eMPF_BlocksStateChange, eNRT_ReliableUnordered)
		, m_pView(pView)
		, m_id(id)
		, m_base(base)
		, m_length(length)
		, m_flk(flk)
	{
		NET_ASSERT(m_length <= CSyncedFileSet::MAX_SEND_CHUNK_SIZE);
		NET_ASSERT(flk.Ok());
	}

	// INetSendable
	size_t GetSize()
	{
		return sizeof(*this);
	}

	EMessageSendResult Send(INetSender* pSender)
	{
		pSender->BeginMessage(m_pView->GetConfig().pAddFileData);
		pSender->ser.Value("id", m_id, 'flid');
		pSender->ser.Value("length", m_length, 'flsz');
		NET_ASSERT(m_length <= CSyncedFileSet::MAX_SEND_CHUNK_SIZE);
		for (u32 i = 0; i < m_length; i++)
			pSender->ser.Value("data", const_cast<u8&>(m_flk.GetData()[m_base + i]), 'u8');
		return eMSR_SentOk;
	}

	void UpdateState(u32 nFromSeq, ENetSendableStateUpdate update)
	{
	}

	tukk GetDescription()
	{
		return "AddData";
	}

	void GetPositionInfo(SMessagePositionInfo& pos)
	{
	}
	// ~INetSendable

private:
	CContextView*       m_pView;
	u32              m_id;
	u32              m_base;
	u32              m_length;
	CSyncedFileDataLock m_flk;
};

class CSend_EndFile : public INetSendable
{
public:
	CSend_EndFile(CContextView* pView, u32 id)
		: INetSendable(eMPF_BlocksStateChange, eNRT_ReliableUnordered)
		, m_pView(pView)
		, m_id(id)
	{
	}

	// INetSendable
	size_t GetSize()
	{
		return sizeof(*this);
	}

	void UpdateState(u32 nFromSeq, ENetSendableStateUpdate update)
	{
	}

	EMessageSendResult Send(INetSender* pSender)
	{
		pSender->BeginMessage(m_pView->GetConfig().pEndSyncFile);
		pSender->ser.Value("id", m_id, 'flid');
		return eMSR_SentOk;
	}

	tukk GetDescription()
	{
		return "EndFile";
	}

	void GetPositionInfo(SMessagePositionInfo& pos)
	{
	}
	// ~INetSendable

private:
	CContextView* m_pView;
	u32        m_id;
};

class CSend_AllFilesSynced : public INetSendable
{
public:
	CSend_AllFilesSynced(CContextView* pView, u32 syncSeq)
		: INetSendable(eMPF_BlocksStateChange, eNRT_ReliableUnordered)
		, m_pView(pView)
		, m_syncSeq(syncSeq)
	{
	}

	// INetSendable
	size_t GetSize()
	{
		return sizeof(*this);
	}

	EMessageSendResult Send(INetSender* pSender)
	{
		pSender->BeginMessage(m_pView->GetConfig().pAllFilesSynced);
		pSender->ser.Value("syncseq", m_syncSeq, 'flid');
		return eMSR_SentOk;
	}

	void UpdateState(u32 nFromSeq, ENetSendableStateUpdate update)
	{
	}

	tukk GetDescription()
	{
		return "AllFilesSynced";
	}

	void GetPositionInfo(SMessagePositionInfo& pos)
	{
	}
	// ~INetSendable

private:
	CContextView* m_pView;
	u32        m_syncSeq;
};

CSyncedFileSet::CSyncedFileSet() : m_syncsPerformed(1)
{
}

CSyncedFileSet::~CSyncedFileSet()
{
}

void CSyncedFileSet::AddFile(tukk name)
{
	ASSERT_GLOBAL_LOCK;
	string sname = name;
	m_files.insert(std::make_pair(sname, new CSyncedFile(sname)));
}

void CSyncedFileSet::SendFilesTo(CNetChannel* pSender)
{
	ASSERT_GLOBAL_LOCK;

	std::vector<SSendableHandle> endFiles;

	SSendableHandle beginSync;
	pSender->NetAddSendable(new CSend_BeginSyncFiles(pSender->GetContextView(), m_syncsPerformed), 0, 0, &beginSync);

	u32 id = 0;
	for (TFiles::const_iterator it = m_files.begin(); it != m_files.end(); ++it)
	{
		CSyncedFileDataLock flk(it->second);
		if (!flk.Ok())
			continue;

		SSendableHandle shdl;
		pSender->NetAddSendable(new CSend_BeginFile(pSender->GetContextView(), id, flk.GetFilename()), 1, &beginSync, &shdl);

		for (u32 ofs = 0; ofs < flk.GetLength(); ofs += CSyncedFileSet::MAX_SEND_CHUNK_SIZE)
		{
			u32 thisChunkLength = std::min(CSyncedFileSet::MAX_SEND_CHUNK_SIZE, flk.GetLength() - ofs);
			NET_ASSERT(thisChunkLength);
			pSender->NetAddSendable(new CSend_AddFileData(pSender->GetContextView(), id, ofs, thisChunkLength, flk), 1, &shdl, &shdl);
		}

		pSender->NetAddSendable(new CSend_EndFile(pSender->GetContextView(), id), 1, &shdl, &shdl);

		endFiles.push_back(shdl);

		id++;
	}

	pSender->NetAddSendable(new CSend_AllFilesSynced(pSender->GetContextView(), m_syncsPerformed), endFiles.size(), &endFiles[0], NULL);

	do
	{
		m_syncsPerformed++;
	}
	while (!m_syncsPerformed);
}

bool CSyncedFileSet::BeginUpdateFile(tukk name, u32 id)
{
	TUpdatingFiles::iterator itUpd = m_updating.lower_bound(id);
	if (itUpd != m_updating.end() && itUpd->first == id)
		return false; // already updating id

	string sname = name;
	TFiles::iterator itFile = m_files.find(sname);
	if (itFile == m_files.end())
		return false; // no such file

	CSyncedFilePtr pFile = itFile->second;
	if (!pFile->BeginUpdate())
		return false;

	m_updating.insert(itUpd, std::make_pair(id, pFile));
	return true;
}

bool CSyncedFileSet::AddFileData(u32 id, u8k* pData, u32 length)
{
	TUpdatingFiles::iterator itUpd = m_updating.find(id);
	if (itUpd == m_updating.end())
		return false;

	CSyncedFilePtr pFile = itUpd->second;
	return pFile->PutData(pData, length);
}

bool CSyncedFileSet::EndUpdateFile(u32 id)
{
	TUpdatingFiles::iterator itUpd = m_updating.find(id);
	if (itUpd == m_updating.end())
		return false;

	CSyncedFilePtr pFile = itUpd->second;
	m_updating.erase(itUpd);
	return pFile->EndUpdate(true);
}

CSyncedFilePtr CSyncedFileSet::GetSyncedFile(tukk name)
{
	string sname = name;
	TFiles::iterator it = m_files.find(name);
	if (it == m_files.end())
		return CSyncedFilePtr();
	else
		return it->second;
}

#endif // SERVER_FILE_SYNC_MODE
