// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/IChunkFile.h>

struct IChunkFile;
class CContentCGF;
struct ChunkDesc;

#define ROTCHANNEL (0xaa)
#define POSCHANNEL (0x55)

//////////////////////////////////////////////////////////////////////////
struct CHeaderTCB : public _reference_target_t
{
	DynArray<IController_AutoPtr>   m_pControllers;
	DynArray<u32>                m_arrControllerId;

	u32                          m_nAnimFlags;
	u32                          m_nCompression;

	i32                           m_nStart;
	i32                           m_nEnd;

	DynArray<TCBFlags>              m_TrackVec3FlagsQQQ;
	DynArray<TCBFlags>              m_TrackQuatFlagsQQQ;
	DynArray<DynArray<DrxTCB3Key>*> m_TrackVec3QQQ;
	DynArray<DynArray<DrxTCBQKey>*> m_TrackQuat;
	DynArray<CControllerType>       m_arrControllersTCB;

	CHeaderTCB()
	{
		m_nAnimFlags = 0;
		m_nCompression = -1;
	}

	~CHeaderTCB()
	{
		for (DynArray<DynArray<DrxTCB3Key>*>::iterator it = m_TrackVec3QQQ.begin(), itEnd = m_TrackVec3QQQ.end(); it != itEnd; ++it)
			delete *it;
		for (DynArray<DynArray<DrxTCBQKey>*>::iterator it = m_TrackQuat.begin(), itEnd = m_TrackQuat.end(); it != itEnd; ++it)
			delete *it;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_arrControllerId);
	}

private:
	CHeaderTCB(const CHeaderTCB&);
	CHeaderTCB& operator=(const CHeaderTCB&);
};

//////////////////////////////////////////////////////////////////////////
class ILoaderCAFListener
{
public:
	virtual ~ILoaderCAFListener(){}
	virtual void Warning(tukk format) = 0;
	virtual void Error(tukk format) = 0;
};

class CLoaderTCB
{
public:
	CLoaderTCB();
	~CLoaderTCB();

	CHeaderTCB* LoadTCB(tukk filename, ILoaderCAFListener* pListener);
	CHeaderTCB* LoadTCB(tukk filename, IChunkFile* pChunkFile, ILoaderCAFListener* pListener);

	tukk GetLastError() const         { return m_LastError; }
	//	CContentCGF* GetCContentCAF() { return m_pCompiledCGF; }
	bool        GetHasNewControllers() const { return m_bHasNewControllers; };
	bool        GetHasOldControllers() const { return m_bHasOldControllers; };
	void        SetLoadOldChunks(bool v)     { m_bLoadOldChunks = v; };
	bool        GetLoadOldChunks() const     { return m_bLoadOldChunks; };

	void        SetLoadOnlyCommon(bool b)    { m_bLoadOnlyCommon = b; }
	bool        GetLoadOnlyOptions() const   { return m_bLoadOnlyCommon; }

private:
	bool LoadChunksTCB(IChunkFile* chunkFile);
	bool ReadTiming(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadMotionParameters(IChunkFile::ChunkDesc* pChunkDesc);
	bool ReadController(IChunkFile::ChunkDesc* pChunkDesc);
	//void Warning(tukk szFormat, ...);

private:
	string      m_LastError;

	string      m_filename;

	CHeaderTCB* m_pSkinningInfo;

	bool        m_bHasNewControllers;
	bool        m_bLoadOldChunks;
	bool        m_bHasOldControllers;
	bool        m_bLoadOnlyCommon;

	//bool m_bHaveVertexColors;
	//bool m_bOldMaterials;
};
