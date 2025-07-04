// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifndef RESOURCE_COMPILER
#include <drx3D/Animation/GlobalAnimationHeader.h>
#include <drx3D/CoreX/String/NameCRCHelper.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>
#include <drx3D/Animation/AnimEventList.h>

struct ModelAnimationHeader;
class CSkeletonAnim;
class CAnimationSet;

// The flags used in the nFlags member.
enum CA_VEG_Flags
{
	CA_VEG_I2M    = 0x001,  //temporary solution until we have DrxManequin
	CA_BIG_ENDIAN = 0x002
};

struct VirtualExample1D
{
	u8 i0, i1;
	f32   w0, w1;
};

struct VirtualExample2D
{
	u8 i0, i1, i2, i3;
	f32   w0, w1, w2, w3;
};

struct VirtualExample3D
{
	u8 i0, i1, i2, i3, i4, i5, i6, i7;
	f32   w0, w1, w2, w3, w4, w5, w6, w7;
};

struct BSParameter
{
	Vec4     m_Para;
	f32      m_fPlaybackScale;
	u8    m_PreInitialized[4];
	u8    m_UseDirectlyForDeltaMotion[4];
	u32   i0;
	f32      w0;
	u32   i1;
	f32      w1;
	SCRCName m_animName;

	BSParameter()
	{
		m_Para = Vec4(0, 0, 0, 0); //4D will be the maximum
		m_fPlaybackScale = 1.0f;
		m_PreInitialized[0] = 0;
		m_PreInitialized[1] = 0;
		m_PreInitialized[2] = 0;
		m_PreInitialized[3] = 0;
		m_UseDirectlyForDeltaMotion[0] = 0;
		m_UseDirectlyForDeltaMotion[1] = 0;
		m_UseDirectlyForDeltaMotion[2] = 0;
		m_UseDirectlyForDeltaMotion[3] = 0;
		i0 = 0;
		w0 = 0.0f;
		i1 = 0;
		w1 = 0.0f;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
#if STORE_CRCNAME_STRINGS
		pSizer->AddObject(m_animName.m_name);
#endif
	}
};

struct BSBlendable
{
	u32 num;
	u8  idx[8];

	BSBlendable()
		: num{ 0 }
		, idx{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
	{
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
	}
};

struct DimensionParams
{
	string m_strParaName;
	f32    m_min, m_max;
	u8  m_cells;
	u8  m_nInitialized;
	u8  m_nDimensionFlags;
	u8  m_ParaID;
	f32    m_skey, m_ekey;
	f32    m_ParaScale;        //for combined blend-spaces
	i32    m_ChooseBlendSpace; //for combined blend-spaces
	f32    m_scale;            //just for graphical debugging. no impact on parameterization

	void   init()
	{
		m_min = -9999.0f;   //bad value: must initialized at loading time
		m_max = -9999.0f;   //bad value: must initialized at loading time

		m_cells = 3;         //default value
		m_nInitialized = 0;
		m_nDimensionFlags = 0;
		m_ParaID = 0xff;        //bad value: must initialized at loading time

		m_skey = 0.0f;
		m_ekey = 1.0f;

		m_ParaScale = 1.0f;     //default value
		m_ChooseBlendSpace = 0; //default value

		m_scale = 1.0f;     //just for graphical debugging. no impact on parameterization
	};

	DimensionParams()
	{
		init();
	}
};

struct ExtractionParams
{
	string m_strParaName;
	u32 m_ParaID;

	void   init()
	{
		m_ParaID = -1;        //must initialized at loading time
	};

	ExtractionParams()
	{
		init();
	}
};

//this is a structure to keep information for other Blend-Spaces
struct BlendSpaceInfo
{
	string m_FilePath;        //path-name - unique per-model
	u32 m_FilePathCRC32;   //hash value for searching animations
	i32  m_ParaGroupID;     //id in the list for all Para-Groups
	void   init()
	{
		m_FilePath.clear();     //path-name - unique per-model
		m_FilePathCRC32 = 0;    //hash value for searching animations
		m_ParaGroupID = -1;     //id in the list for all Para-Groups
	};
	BlendSpaceInfo()  { init(); }
};

//////////////////////////////////////////////////////////////////////////
// this is the animation information on the module level (not on the per-model level)
// it doesn't know the name of the animation (which is model-specific), but does know the file name
// Implements some services for bone binding and ref counting
struct DRX_ALIGN(16) GlobalAnimationHeaderLMG: public GlobalAnimationHeader
{
	friend class CAnimationUpr;
	friend class CAnimationSet;

	GlobalAnimationHeaderLMG()
	{
		m_nRef_by_Model = 0;
		m_nTouchedCounter = 0;
		m_numExamples = 0;
		m_Dimensions = 0;
		m_ExtractionParams = 0;
		m_VEG_Flags = 0;
		m_fThreshold = -99999.0f;         //TODO: this must be "per dimension"
	}

	bool LoadAndParseXML(CAnimationSet * pAnimationSet, bool nForceReloading = 0);
	bool LoadFromXML(CAnimationSet * pAnimationSet, XmlNodeRef root);

	void ParameterExtraction(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 nParaID, u32 d);
	void Init_MoveSpeed(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 dim);
	void Init_TurnSpeed(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 dim);
	void Init_TurnAngle(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 dim);
	void Init_TravelAngle(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 dim);
	void Init_SlopeAngle(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 dim);
	void Init_TravelDist(const CAnimationSet * pAnimationSet, const CDefaultSkeleton * pDefaultSkeleton, u32 dim);
#if BLENDSPACE_VISUALIZATION
	void CreateInternalType_Para1D();
#endif

#ifdef EDITOR_PCDEBUGCODE
	bool Export2HTR(tukk szAnimationName, tukk savePath, const CDefaultSkeleton * pDefaultSkeleton, const CSkeletonAnim * pSkeletonAnim) const;
	bool ExportVGrid() const;
#endif

	~GlobalAnimationHeaderLMG()
	{
	};

	void AddRef()
	{
		++m_nRef_by_Model;
	}

	void Release()
	{
		if (!--m_nRef_by_Model)
		{
		}
	}

	ILINE u32 IsAssetLoaded() const   { return 1; }
	ILINE u32 IsAssetOnDemand() const { return 0; }

	size_t       SizeOfLMG() const
	{
		size_t nSize = sizeof(*this);
		size_t nTemp00 = m_FilePath.capacity();
		nSize += nTemp00;
		nSize += m_arrParameter.get_alloc_size();
		nSize += m_arrBSAnnotations.get_alloc_size();
		nSize += m_VirtualExampleGrid1D.get_alloc_size();
		nSize += m_VirtualExampleGrid2D.get_alloc_size();
		nSize += m_VirtualExampleGrid3D.get_alloc_size();

		nSize += m_AnimEventsLMG.GetAllocSize();

		nSize += m_jointList.get_alloc_size();

		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_FilePath);

		pSizer->AddObject(m_arrParameter);
		pSizer->AddObject(m_AnimEventsLMG);
		pSizer->AddObject(m_jointList);
	}

public:
	CAnimEventList m_AnimEventsLMG;

	i32 m_nRef_by_Model;
	u32 m_nTouchedCounter;     //the number of referrers to this global animation record

	u32 m_numExamples;
	u16 m_VEG_Flags;
	u8 m_Dimensions;
	u8 m_ExtractionParams;
	f32 m_fThreshold;             //temporary solution until we have the right assets
	DimensionParams m_DimPara[4];
	ExtractionParams m_ExtPara[4];
	DynArray<BSParameter> m_arrParameter;
	DynArray<BSBlendable> m_arrBSAnnotations;
	DynArray<VirtualExample1D> m_VirtualExampleGrid1D;
	DynArray<VirtualExample2D> m_VirtualExampleGrid2D;
	DynArray<VirtualExample3D> m_VirtualExampleGrid3D;
	DynArray<BlendSpaceInfo> m_arrCombinedBlendSpaces;
	DynArray<u32> m_jointList;              //this is a lits of the most important joints for an Para-Group.

	string m_Status;  //if this Para-Group is ok, then this string is empty

};
#endif