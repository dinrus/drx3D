// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/CompressionUpr.h>
#include <drx3D/Network/ISerialize.h>
#include  <drx3D/Network/Utils.h>
#include  <drx3D/Network/ICompressionPolicy.h>
#include <drx3D/Network/SimpleSerialize.h>
#include  <drx3D/Network/SerializationChunk.h>
#include  <drx3D/Network/RangedIntPolicy.h>
#include  <drx3D/Network/OwnChannelCompressionPolicy.h>
#include  <drx3D/Network/ErrorDistributionEncoding.h>
#include <queue>

bool CCompressionUpr::CCompareChunks::operator()(CSerializationChunk* p0, CSerializationChunk* p1) const
{
	return *p0 < *p1;
}

/*
 * CCompressionRegistry
 */

CCompressionRegistry* CCompressionRegistry::m_pSelf = 0;

void CCompressionRegistry::RegisterPolicy(const string& name, CompressionPolicyCreator create)
{
	VectorMap<string, CompressionPolicyCreator>::iterator iter = m_policyFactories.find(name);
	if (iter != m_policyFactories.end())
		DrxFatalError("Duplicated policy implementation name %s", name.c_str());
	m_policyFactories.insert(std::make_pair(name, create));
}

ICompressionPolicyPtr CCompressionRegistry::CreatePolicyOfType(const string& type, u32 key)
{
	VectorMap<string, CompressionPolicyCreator>::iterator iter = m_policyFactories.find(type);
	if (iter == m_policyFactories.end())
		return 0;
	else
		return iter->second(key);
}

void CCompressionRegistry::Create()
{
	m_pSelf = new CCompressionRegistry;
}

/*
 * CCompressionUpr
 */

CCompressionUpr::CCompressionUpr()
{
	m_manageIntervalSeconds = 0;

	m_timeValue = gEnv->pTimer->GetAsyncTime();

	m_pDefaultPolicy = 0;
	m_pTemporaryChunk = new CSerializationChunk;

	m_threadRunning = false;
	m_threadRequestQuit = false;
}

CCompressionUpr::~CCompressionUpr()
{
	TerminateThread();
	ClearCompressionPolicies(true);
}

void CCompressionUpr::ClearCompressionPolicies(bool includingDefault)
{
	stl::free_container(m_chunks);
	m_chunkToId.clear();
	m_compressionPolicies.clear();
	if (includingDefault)
		m_pDefaultPolicy = NULL;

	m_policiesManageList.clear();
}

void CCompressionUpr::Reset(bool useCompression, bool unloading)
{
	if (m_pDefaultPolicy == NULL)
	{
		m_pDefaultPolicy = CCompressionRegistry::Get()->CreatePolicyOfType("Default", 0);
	}
	NET_ASSERT(m_pDefaultPolicy != NULL);

	ClearCompressionPolicies(false);

	if (!unloading)
	{
		u32 defaultNameKey;
		StringToKey("dflt", defaultNameKey);

		m_compressionPolicies.insert(std::make_pair(defaultNameKey, m_pDefaultPolicy));
		m_compressionPolicies.insert(std::make_pair(0, m_pDefaultPolicy));

		// Start by loading the engine defaults
		tukk fileName = "%engine%/Config/DefaultScripts/CompressionPolicy.xml";
		XmlNodeRef config = gEnv->pSystem->LoadXmlFromFile(fileName);
		if (config)
		{
			LoadCompressionPolicy(fileName, config, useCompression);
		}
		
		// Now load the game specific compression policy, if it exists
		fileName = "Scripts/Network/CompressionPolicy.xml";
		if (gEnv->pDrxPak->IsFileExist(fileName))
		{
			XmlNodeRef config = gEnv->pSystem->LoadXmlFromFile(fileName);
			if (config)
			{
				LoadCompressionPolicy(fileName, config, useCompression);
			}
		}
	}
}


void CCompressionUpr::LoadCompressionPolicy(tukk fileName, XmlNodeRef rootNode, bool bUseCompression)
{
	std::queue<XmlNodeRef> waitingToLoad;
	for (i32 i = 0; i < rootNode->getChildCount(); i++)
		waitingToLoad.push(rootNode->getChild(i));

	u32 skipCount = 0;
	while (!waitingToLoad.empty() && skipCount < waitingToLoad.size())
	{
		XmlNodeRef loading = waitingToLoad.front();
		waitingToLoad.pop();

		string name = loading->getAttr("name");
		bool processed = true;

		if (0 == strcmp("Distributions", loading->getTag()))
		{
			m_useDirectory = loading->getAttr("use");
			m_accumulateDirectory = loading->getAttr("accumulate");

			u32 manageInterval = 0;
			if (loading->getAttr("manageInterval", manageInterval))
			{
				m_manageIntervalSeconds = manageInterval;
				StartManageThread();
			}
		}
		else if (name.empty())
		{
			NetWarning("Policy with no name at %s:%d", fileName, loading->getLine());
		}
		else
		{
			u32 nameKey;
			if (!StringToKey(name.c_str(), nameKey))
			{
				NetWarning("Unable to convert policy name '%s' to a four character code; ignoring it (found at %s:%d)", name.c_str(), fileName, loading->getLine());
			}
			else
			{
				if (0 == strcmp("Alias", loading->getTag()))
				{
					string is = loading->getAttr("is");
					if (is.empty())
					{
						NetWarning("Alias with no basis found at %s:%d", fileName, loading->getLine());
					}
					else
					{
						u32 isKey;
						if (!StringToKey(is.c_str(), isKey))
						{
							NetWarning("Unable to convert alias basis '%s' to a four character code; ignoring it (found at %s:%d)", is.c_str(), fileName, loading->getLine());
						}
						else
						{
							TCompressionPoliciesMap::iterator iter = m_compressionPolicies.find(isKey);
							if (iter == m_compressionPolicies.end())
							{
								processed = false;
							}
							else
							{
								m_compressionPolicies[nameKey] = iter->second;
							}
						}
					}
				}
				else if (0 == strcmp("Policy", loading->getTag()))
				{
					ICompressionPolicyPtr pPolicy;
					if (bUseCompression)
						pPolicy = CreatePolicy(loading, fileName, nameKey);
					else if (nameKey == 'eid')
						pPolicy = CCompressionRegistry::Get()->CreatePolicyOfType("SimpleEntityId", 'eid');
					else
						pPolicy = m_pDefaultPolicy;
					if (!pPolicy)
					{
						NetWarning("Failed to create policy %s at %s:%d; reverting to default", name.c_str(), fileName, loading->getLine());
						pPolicy = m_pDefaultPolicy;
					}

					pPolicy->Init(this);

					if (m_manageIntervalSeconds != 0)
						pPolicy->Manage(this);

					m_compressionPolicies[nameKey] = pPolicy;
					m_policiesManageList.push_back(nameKey);

				}
			}
		}

		if (processed)
		{
			skipCount = 0;
		}
		else
		{
			waitingToLoad.push(loading);
			skipCount++;
		}
	}
}

void CCompressionUpr::TerminateThread()
{
	if (!m_threadRunning)
		return;

	RequestTerminate();
	gEnv->pThreadUpr->JoinThread(this, eJM_Join);
	m_threadRunning = false;
}

void CCompressionUpr::RequestTerminate()
{
	m_threadRequestQuit = true;
}

void CCompressionUpr::StartManageThread()
{
	if (m_threadRunning)
		return;

	m_threadRequestQuit = false;
	m_threadRunning = true;

	gEnv->pThreadUpr->SpawnThread(this, "CompressionUpr");
}

void CCompressionUpr::ThreadEntry()
{
	while (!m_threadRequestQuit)
	{
		DrxSleep(60);

		ManagePolicies();
	}
}

void CCompressionUpr::ManagePolicies()
{
	if (m_manageIntervalSeconds == 0)
		return;

	CTimeValue val = gEnv->pTimer->GetAsyncTime();

	if (val.GetDifferenceInSeconds(m_timeValue) < m_manageIntervalSeconds)
		return;

	CErrorDistribution::LogPerformance();

	m_timeValue = val;

	while (!m_policiesManageList.empty())
	{
		u32 name = m_policiesManageList.front();
		m_policiesManageList.pop_front();

		NetLogAlways("ManagePolicies: %i", name);

		TCompressionPoliciesMap::iterator iter = m_compressionPolicies.find(name);
		if (iter != m_compressionPolicies.end() && iter->second->Manage(this))
		{
			m_policiesManageList.push_back(name);
			break;
		}
	}
}

ICompressionPolicyPtr CCompressionUpr::CreatePolicy(XmlNodeRef node, const string& filename, u32 key)
{
	if (!node)
		return ICompressionPolicyPtr();

	ICompressionPolicyPtr pPolicy;
	string impl = node->getAttr("impl");
	if (impl.empty())
	{
		ICompressionPolicyPtr pOwn(CreatePolicy(node->findChild("Own"), filename, key));
		ICompressionPolicyPtr pOther(CreatePolicy(node->findChild("Other"), filename, key));

		if (!pOwn || !pOther)
			return ICompressionPolicyPtr();

		pPolicy = new COwnChannelCompressionPolicy(key, pOwn, pOther);
	}
	else
	{
		pPolicy = CCompressionRegistry::Get()->CreatePolicyOfType(impl, key);
		if (!pPolicy)
		{
			NetWarning("Implementation '%s' could not be created for policy in %s:%d", impl.c_str(), filename.c_str(), node->getLine());
			return 0;
		}

		if (!pPolicy->Load(node, filename))
		{
			NetWarning("Couldn't load parameters for policy implemented by '%s' in %s:%d", impl.c_str(), filename.c_str(), node->getLine());
			return 0;
		}
	}

	return pPolicy;
}

ICompressionPolicyPtr CCompressionUpr::GetCompressionPolicy(u32 key)
{
	TCompressionPoliciesMap::iterator iter = m_compressionPolicies.find(key);
	if (iter == m_compressionPolicies.end())
		return GetCompressionPolicyFallback(key);
	return iter->second;
}

ICompressionPolicyPtr CCompressionUpr::GetCompressionPolicyFallback(u32 key)
{
	if ((key & 0xff000000) == (u32)ISerialize::ENUM_POLICY_TAG)
	{
		ICompressionPolicyPtr pPolicy = CreateRangedInt(key & 0xffffff, key);
		m_compressionPolicies.insert(std::make_pair(key, pPolicy));
		return pPolicy;
	}
	NetWarning("Compression policy %s not found", KeyToString(key).c_str());
	return m_pDefaultPolicy;
}

ChunkID CCompressionUpr::GameContextGetChunkID(IGameContext* pCtx, EntityId id, NetworkAspectType nAspect, u8 nProfile, NetworkAspectType skipCompression)
{
	ASSERT_PRIMARY_THREAD;

	m_pTemporaryChunk->Reset();
	if (!m_pTemporaryChunk->Init(pCtx, id, nAspect, nProfile, skipCompression))
		return InvalidChunkID;

	std::map<CSerializationChunkPtr, ChunkID, CCompareChunks>::iterator iter = m_chunkToId.find(m_pTemporaryChunk);
	if (iter == m_chunkToId.end())
	{
		iter = m_chunkToId.insert(std::make_pair(m_pTemporaryChunk, static_cast<ChunkID>(m_chunks.size()))).first;
		m_chunks.push_back(m_pTemporaryChunk);
#if ENABLE_DEBUG_KIT
		m_pTemporaryChunk->Dump(id, iter->second);
#endif
		m_pTemporaryChunk = new CSerializationChunk;
	}
	return iter->second;
}

bool CCompressionUpr::IsChunkEmpty(ChunkID chunkID)
{
	if (chunkID == InvalidChunkID)
		return true;
	CSerializationChunkPtr pChunk = GetChunk(chunkID);
	if (pChunk)
		return pChunk->IsEmpty();
	else
		return true;
}

void CCompressionUpr::BufferToStream(ChunkID chunk, u8 profile, CByteInputStream& in, CNetOutputSerializeImpl& out)
{
	if (CSerializationChunkPtr pChunk = GetChunk(chunk))
		pChunk->EncodeToStream(in, out, chunk, profile);
}

void CCompressionUpr::StreamToBuffer(ChunkID chunk, u8 profile, CNetInputSerializeImpl& in, CByteOutputStream& out)
{
	if (CSerializationChunkPtr pChunk = GetChunk(chunk))
		pChunk->DecodeFromStream(in, out, chunk, profile);
}

ICompressionPolicyPtr CCompressionUpr::CreateRangedInt(i32 nMax, u32 key)
{
	CRangedIntPolicy pol;
	pol.SetValues(0, nMax);
	return new CCompressionPolicy<CRangedIntPolicy>(key, pol);

	//	return m_pDefaultPolicy;
}

void CCompressionUpr::GetMemoryStatistics(IDrxSizer* pSizer)
{
	SIZER_COMPONENT_NAME(pSizer, "CCompressionUpr");

	pSizer->Add(*this);
	pSizer->AddHashMap(m_compressionPolicies);
	pSizer->AddContainer(m_chunks);
	pSizer->AddContainer(m_chunkToId);

	for (size_t i = 0; i < m_chunks.size(); ++i)
		m_chunks[i]->GetMemoryStatistics(pSizer);

	for (TCompressionPoliciesMap::const_iterator it = m_compressionPolicies.begin(); it != m_compressionPolicies.end(); ++it)
		it->second->GetMemoryStatistics(pSizer);
}
const string& CCompressionUpr::GetAccDirectory() const
{
	return m_accumulateDirectory;
}

const string& CCompressionUpr::GetUseDirectory() const
{
	return m_useDirectory;
}
