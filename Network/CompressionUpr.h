// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __COMPRESSIONMANAGER_H__
#define __COMPRESSIONMANAGER_H__

#pragma once

#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/Network/ByteStream.h>
#include <drx3D/Network/INetwork.h>
#include <drx3D/Network/SerializationChunk.h>
#include <drx3D/CoreX/Memory/STLPoolAllocator_ManyElems.h>

struct ICompressionPolicy;
typedef ICompressionPolicy* ICompressionPolicyPtr;
class CNetOutputSerializeImpl;
class CNetInputSerializeImpl;

class CCompressionRegistry
{
public:
	typedef ICompressionPolicyPtr (* CompressionPolicyCreator)(u32);

	static ILINE CCompressionRegistry* Get()
	{
		if (!m_pSelf)
			Create();
		return m_pSelf;
	}

	void RegisterPolicy(const string &name, CompressionPolicyCreator);
	ICompressionPolicyPtr CreatePolicyOfType(const string& type, u32 key);

private:
	CCompressionRegistry() {}

	static void Create();

	static CCompressionRegistry*                m_pSelf;
	VectorMap<string, CompressionPolicyCreator> m_policyFactories;
};

class CCompressionUpr :
	public IThread
{
public:
	CCompressionUpr();
	~CCompressionUpr();

	void                  Reset(bool useCompression, bool unloading);

	virtual void ThreadEntry() override;
	void TerminateThread();
	void RequestTerminate();
	void StartManageThread();
	void ManagePolicies();

	ICompressionPolicyPtr GetCompressionPolicy(u32 key);

	// chunk management
	void       BufferToStream(ChunkID chunk, u8 profile, CByteInputStream& in, CNetOutputSerializeImpl& out);
	void       StreamToBuffer(ChunkID chunk, u8 profile, CNetInputSerializeImpl& in, CByteOutputStream& out);

	ChunkID    GameContextGetChunkID(IGameContext* pCtx, EntityId id, NetworkAspectType nAspect, u8 nProfile, NetworkAspectType skipCompression);

	ILINE bool GameContextRead(IGameContext* pCtx, CByteOutputStream* pOutput, EntityId id, NetworkAspectType nAspect, u8 nProfile, ChunkID chunkID)
	{
		ASSERT_PRIMARY_THREAD;
		CSerializationChunk* pChunk = GetChunk(chunkID);
		if (!pChunk)
			return false;
		return pChunk->FetchFromGame(pCtx, *pOutput, id, nAspect, chunkID, nProfile);
	}

	ILINE ESynchObjectResult GameContextWrite(IGameContext* pCtx, CByteInputStream* pInput, EntityId id, NetworkAspectType nAspect, u8 nProfile, ChunkID chunkID, bool& wasPartialUpdate)
	{
		ASSERT_PRIMARY_THREAD;
		CSerializationChunkPtr pChunk = GetChunk(chunkID);
		if (!pChunk)
		{
			NetWarning("CCompressionUpr::GameContextWrite: failed fetching chunk %d", chunkID);
			return eSOR_Failed;
		}
		return pChunk->UpdateGame(pCtx, *pInput, id, nAspect, chunkID, nProfile, wasPartialUpdate);
	}

	bool IsChunkEmpty(ChunkID chunkID);

	void GetMemoryStatistics(IDrxSizer* pSizer);

	const string& GetAccDirectory() const;
	const string& GetUseDirectory() const;

protected:
	void LoadCompressionPolicy(tukk fileName, XmlNodeRef rootNode, bool bUseCompression);

private:
#if USE_SYSTEM_ALLOCATOR
	typedef std::unordered_map<u32, _smart_ptr<ICompressionPolicy>, stl::hash_uint32>                                                                                                                  TCompressionPoliciesMap;
#else
	typedef std::unordered_map<u32, _smart_ptr<ICompressionPolicy>, stl::hash_uint32, std::equal_to<u32>, stl::STLPoolAllocator_ManyElems<std::pair<u32k, _smart_ptr<ICompressionPolicy>>>> TCompressionPoliciesMap;
#endif
	TCompressionPoliciesMap        m_compressionPolicies;
	std::deque<u32>              m_policiesManageList;
	_smart_ptr<ICompressionPolicy> m_pDefaultPolicy;
	CSerializationChunkPtr         m_pTemporaryChunk;

	 bool                  m_threadRunning;
	 bool                  m_threadRequestQuit;

	string                         m_accumulateDirectory;
	string                         m_useDirectory;
	u32                         m_manageIntervalSeconds;
	CTimeValue                     m_timeValue;

	class CCompareChunks
	{
	public:
		bool operator()(CSerializationChunk*, CSerializationChunk*) const;
	};

	std::vector<CSerializationChunkPtr>                       m_chunks;
	std::map<CSerializationChunkPtr, ChunkID, CCompareChunks> m_chunkToId;
	ILINE CSerializationChunk* GetChunk(ChunkID chunk)
	{
		if (chunk >= m_chunks.size())
			return 0;
		else
			return m_chunks[chunk];
	}

	ICompressionPolicyPtr GetCompressionPolicyFallback(u32 key);
	ICompressionPolicyPtr CreateRangedInt(i32 nMax, u32 key);
	ICompressionPolicyPtr CreatePolicy(XmlNodeRef node, const string& filename, u32 key);
	void                  ClearCompressionPolicies(bool includingDefault);
};

#endif
