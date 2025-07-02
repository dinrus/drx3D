// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

////////////////////////////////////////////////////////////////////////////
// Device Object Toolkits

/* A class to create and track objects, for which the Creator is a global function (global scope or static class scope).
* Requirements to use the class is:
*
*  THandle        - a Handle-type struct, specifying THandle::Unspecified and appropriate constructor/conversion operator
*  SDescription   - a descriptive struct used to create the object and de-duplicate objects, equality operator needs to be specified
*  CDeviceObject  - a type for the pointer of the resulting object, could be void
*
*  CreateFunction - a C-style global function which takes SDescription and produces CDeviceObject
*
* Objects are tried to be created on registration. Error handling is delegated to CreateFunction, which is allowed to return nullptr.
* CreateFunction can also implement "reserve" behavior through the SDescription which is passed. If it is desired for subsequent calls
* to eventually produce a valid object ("deferred-construction"-style), bRetryable should be true, otherwise false.
*
* To facilitate utilization, there is the PlaceStaticDeviceObjectManagementAPI() macro, which creates a named API on the class
* inside which the macro is called. See below for a full print of an example.
*/

template<typename THandle, class SDescription, class CDeviceObject, const bool bRetryable, CDeviceObject* (*CreateFunction)(const SDescription&)>
class CStaticDeviceObjectStorage
{

public:
	typedef std::pair<SDescription, CDeviceObject*> Element;

	THandle GetHandle(const SDescription& pDescr) threadsafe
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);
		m_accessLock.RLock();
		u32k nContainerSize = m_Container.size();
		for (u32 i = 0; i < nContainerSize; i++)
		{
			const Element& element = m_Container[i];
			if (element.first == pDescr)
				return THandle(i);
		}

		return THandle(THandle::Unspecified);
	}

	THandle ReserveHandle(const SDescription& pDescr) threadsafe
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		u32k nContainerSize = m_Container.size();

		// Reservation is unfailable
		assert(nContainerSize == u32(THandle(nContainerSize)));
		m_Container.emplace_back(std::make_pair(pDescr, nullptr));
		return THandle(nContainerSize);
	}

	THandle GetOrCreateHandle(const SDescription& pDescr) threadsafe
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		u32k nContainerSize = m_Container.size();
		for (u32 i = 0; i < nContainerSize; i++)
		{
			Element& element = m_Container[i];
			if (element.first == pDescr)
			{
				// Creation is retryable
				if (bRetryable)
				{
					if (!element.second)
						element.second = CreateFunction(element.first = pDescr);
				}

				return THandle(i);
			}
		}

		// Creation is failable
		assert(nContainerSize == u32(THandle(nContainerSize)));
		m_Container.emplace_back(std::make_pair(pDescr, CreateFunction(pDescr)));
		return THandle(nContainerSize);
	}

	const Element& Lookup(const THandle hState) const threadsafe
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		assert(hState != THandle::Unspecified);
		assert(hState >= THandle(0) && hState < THandle(static_cast<typename THandle::ValueType>(m_Container.size())));
		return m_Container[u32(hState)];
	}

	Element& Lookup(const THandle hState) threadsafe
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		assert(hState != THandle::Unspecified);
		assert(hState >= THandle(0) && hState < THandle(static_cast<typename THandle::ValueType>(m_Container.size())));
		return m_Container[u32(hState)];
	}

	size_t Size() const threadsafe
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		return m_Container.size();
	}

	void Reserve(u32 hNum) threadsafe
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		m_Container.reserve(hNum);
	}

	void Release(THandle hFirst) threadsafe
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		u32k nContainerSize = m_Container.size();
		for (u32 i = u32(hFirst); i < nContainerSize; i++)
			SAFE_RELEASE(m_Container[i].second);

		m_Container.resize(u32(hFirst));
	}

private:
	std::vector<Element> m_Container;
	DrxRWLock            m_accessLock;
};

/* Helper macro which will create the following API block:
 *
 *	PlaceStaticDeviceObjectManagementAPI(SamplerState, false, CDeviceObjectFactory);
 *
 *	public:
 *		static void AllocatePredefinedSamplerStates();
 *		static void ResetSamplerStates();
 *		static void ReleaseSamplerStates();
 *
 *		static SamplerStateHandle GetSamplerStateHandle        (const SSamplerState& pState) const { return s_SamplerStates.GetHandle        (pState); }
 *		static SamplerStateHandle GetOrCreateSamplerStateHandle(const SSamplerState& pState)       { return s_SamplerStates.GetOrCreateHandle(pState); }
 *		static const std::pair<SSamplerState, CDeviceSamplerState*>& LookupSamplerState(const SamplerStateHandle hState) { return s_SamplerStates.Lookup(hState); }
 *		static       std::pair<SSamplerState, CDeviceSamplerState*>& LookupSamplerState(const SamplerStateHandle hState) { return s_SamplerStates.Lookup(hState); }
 *
 *	private:
 *		static SamplerStateHandle ReserveSamplerStateHandle    (const SSamplerState& pState)       { return s_SamplerStates.ReserveHandle    (pState); }
 *		static void               ReserveSamplerStates         (u32k         hNum  )       {        s_SamplerStates.Reserve          (hNum  ); }
 *
 *		// A heap containing all permutations of SamplerState, they are global and are never evicted
 *		static CDeviceSamplerState* CreateSamplerState(const SSamplerState& pState);
 *		static CStaticDeviceObjectStorage<SamplerStateHandle, SSamplerState, CDeviceSamplerState, false, CreateSamplerState> s_SamplerStates;
 */
#define PlaceStaticDeviceObjectManagementAPI(TypeName, bRetryable, TypeCreator, PreAllocated)                                                                                                                   \
	public:                                                                                                                                                                                                     \
		static void AllocatePredefined ## TypeName ## s();                                                                                                                                                      \
		static void Reset   ## TypeName ## s() { s_ ## TypeName ## s.Release(PreAllocated); }                                                                                                                   \
		static void Release ## TypeName ## s() { s_ ## TypeName ## s.Release(0); }                                                                                                                              \
		static size_t Count ## TypeName ## s() { return s_ ## TypeName ## s.Size(); }                                                                                                                           \
		                                                                                                                                                                                                        \
		static TypeName ## Handle Get         ## TypeName ## Handle(const S ## TypeName & pState) { return s_ ## TypeName ## s.GetHandle        (pState); }                                                     \
		static TypeName ## Handle GetOrCreate ## TypeName ## Handle(const S ## TypeName & pState) { return s_ ## TypeName ## s.GetOrCreateHandle(pState); }                                                     \
		static const std::pair<S ## TypeName, CDevice ## TypeName*>& Lookup ## TypeName(const TypeName ## Handle hState) { return s_ ## TypeName ## s.Lookup(hState); }                                         \
		static       std::pair<S ## TypeName, CDevice ## TypeName*>& Lookup ## TypeName(const TypeName ## Handle hState) { return s_ ## TypeName ## s.Lookup(hState); }                                         \
		                                                                                                                                                                                                        \
	private:                                                                                                                                                                                                    \
		static TypeName ## Handle Reserve     ## TypeName ## Handle(const S ## TypeName & pState) { return s_ ## TypeName ## s.ReserveHandle    (pState); }                                                     \
		static void               Reserve     ## TypeName ## s     (u32k          hNum  ) {        s_ ## TypeName ## s.Reserve          (hNum  ); }                                                     \
		                                                                                                                                                                                                        \
		/* A heap containing all permutations of TypeName, they are global and are never evicted */                                                                                                             \
		static CDevice ## TypeName * Create ## TypeName (const S ## TypeName & pState);                                                                                                                         \
		static CStaticDeviceObjectStorage<TypeName ## Handle, S ## TypeName, CDevice ## TypeName, false, Create ## TypeName> s_ ## TypeName ## s;                                                               \

/* A class to create and track objects, for which the Creator is a member function of the object the Storage is member of (class scope).
 * What the pattern achieves is bidirectional composition, that is, the Storage is composed in Creator and knows about it, instead that only Creator knows of Storage.
 * Requirements to use the class is:
 *
 *  THandle              - a Handle-type struct, specifying THandle::Unspecified and appropriate constructor/conversion operator
 *  SDescription         - a descriptive struct used to create the object and de-duplicate objects, equality operator needs to be specified
 *  CDeviceObject        - a type for the pointer of the resulting object, could be void
 *
 *  CDeviceObjectCreator - a class-type of the Creator
 *  CreateFunction       - a member function in the Creator-class which creates the objects
 *  CreatorStorage_Tag   - a struct resolving the retrieval of the Creator-instance pointer from the respective Storage-instance pointer, with the following interface:
 *
 *  	template<class Creator, class Storage>
 *  	struct CreatorStorage_Tag
 *  	{
 *  		inline static ptrdiff_t offset() { return (ptrdiff_t)(&reinterpret_cast<char&>(reinterpret_cast<Creator*>(nullptr)->m_Storage)); }
 *  		inline static Creator* rebase(uk subbase) { return reinterpret_cast<Creator*>(reinterpret_cast<tuk>(subbase) + offset()); }
 *  	};
 *
 * Objects are tried to be created on registration. Error handling is delegated to CreateFunction, which is allowed to return nullptr.
 * CreateFunction can also implement "reserve" behavior through the SDescription which is passed. If it is desired for subsequent calls
 * to eventually produce a valid object ("deferred-construction"-style), bRetryable should be true, otherwise false.
 *
 * To facilitate utilization, there is the PlaceComposedDeviceObjectManagementAPI() macro, which creates a named API on the class
 * inside which the macro is called. See below for a full print of an example.
 */

template<typename THandle, class SDescription, class CDeviceObject, const bool bRetryable, class CDeviceObjectCreator, CDeviceObject* (CDeviceObjectCreator::*CreateFunction)(const SDescription), class CreatorStorage_Tag>
class CComposedDeviceObjectStorage
{

public:
	typedef std::pair<SDescription, CDeviceObject*> Element;

	THandle GetHandle(const SDescription pDescr) const
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		u32k nContainerSize = m_Container.size();
		for (u32 i = 0; i < nContainerSize; i++)
		{
			const Element& element = m_Container[i];
			if (element.first == pDescr)
				return THandle(i);
		}

		return THandle(THandle::Unspecified);
	}

	THandle ReserveHandle(const SDescription pDescr)
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		u32k nContainerSize = m_Container.size();

		// Reservation is unfailable
		assert(nContainerSize == u32(THandle(nContainerSize)));
		m_Container.emplace_back(std::make_pair(pDescr, nullptr));
		return THandle(nContainerSize);
	}

	THandle GetOrCreateHandle(const SDescription pDescr)
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		// Curiously recurring composition pattern (derive base-class not from template argument, but from statically evaluated offset)
		CDeviceObjectCreator* base = CreatorStorage_Tag::rebase(this);

		u32k nContainerSize = m_Container.size();
		for (u32 i = 0; i < nContainerSize; i++)
		{
			Element& element = m_Container[i];
			if (element.first == pDescr)
			{
				// Creation is retryable
				if (bRetryable)
				{
					if (!element.second)
						element.second = (base->*CreateFunction)(element.first = pDescr);
				}

				return THandle(i);
			}
		}

		// Creation is failable
		assert(nContainerSize == u32(THandle(nContainerSize)));
		m_Container.emplace_back(std::make_pair(pDescr, (base->*CreateFunction)(pDescr)));
		return THandle(nContainerSize);
	}

	const Element& Lookup(const THandle hState) const
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		assert(hState != THandle::Unspecified);
		assert(hState >= THandle(0) && hState < THandle(static_cast<typename THandle::ValueType>(m_Container.size())));
		return m_Container[u32(hState)];
	}

	Element& Lookup(const THandle hState)
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		assert(hState != THandle::Unspecified);
		assert(hState >= THandle(0) && hState < THandle(static_cast<typename THandle::ValueType>(m_Container.size())));
		return m_Container[u32(hState)];
	}

	size_t Size() const
	{
		DrxAutoReadLock<DrxRWLock> lock(m_accessLock);

		return m_Container.size();
	}

	void Reserve(u32 hNum)
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		m_Container.reserve(hNum);
	}

	void Release(THandle hFirst)
	{
		DrxAutoWriteLock<DrxRWLock> lock(m_accessLock);

		u32k nContainerSize = m_Container.size();
		for (u32 i = u32(hFirst); i < nContainerSize; i++)
			SAFE_RELEASE(m_Container[i].second);

		m_Container.resize(u32(hFirst));
	}

private:
	std::vector<Element> m_Container;
	DrxRWLock            m_accessLock;
};

/* Helper macro which will create the following API block:
 *
 *	PlaceComposedDeviceObjectManagementAPI(ResourceView, false, CDeviceResource);
 *
 *	public:
 *		void AllocatePredefinedResourceViews();
 *		void ResetResourceViews();
 *		void ReleaseResourceViews();
 *
 *		ResourceViewHandle GetResourceViewHandle        (const SResourceView& pView) const { return m_ResourceViews.GetHandle        (pView); }
 *		ResourceViewHandle GetOrCreateResourceViewHandle(const SResourceView& pView)       { return m_ResourceViews.GetOrCreateHandle(pView); }
 *		const std::pair<SResourceView, CDeviceResourceView*>& LookupResourceView(const ResourceViewHandle hView) const { return m_ResourceViews.Lookup(hView); }
 *		      std::pair<SResourceView, CDeviceResourceView*>& LookupResourceView(const ResourceViewHandle hView)       { return m_ResourceViews.Lookup(hView); }
 *
 *	private:
 *		ResourceViewHandle ReserveResourceViewHandle    (const SResourceView& pView)       { return m_ResourceViews.ReserveHandle    (pView); }
 *		                   ReserveResourceViews         (u32k         hNum )       {        m_ResourceViews.Reserve          (hNum ); }
 *
 *		// Express relationship between creator and storage
 *		struct ResourceView_CreatorStorage_Tag
 *		{
 *			inline static ptrdiff_t offset() { return (ptrdiff_t)(&reinterpret_cast<char&>(reinterpret_cast<CDeviceResource*>(nullptr)->m_ResourceViews)); }
 *			inline static CDeviceResource* rebase(uk subbase) { return reinterpret_cast<CDeviceResource*>(reinterpret_cast<tuk>(subbase) - offset()); }
 *		};
 *
 *		CDeviceResourceView* CreateResourceView(const SResourceView& pView);
 *		CComposedDeviceObjectStorage<ResourceViewHandle, SResourceView, CDeviceResourceView, false, CDeviceResource, &CDeviceResource::CreateResourceView, ResourceView_CreatorStorage_Tag> m_ResourceViews;
 */
#define PlaceComposedDeviceObjectManagementAPI(TypeName, bRetryable, TypeCreator, PreAllocated)                                                                                                                 \
	public:                                                                                                                                                                                                     \
		void AllocatePredefined ## TypeName ## s();                                                                                                                                                             \
		void Reset   ## TypeName ## s() { m_ ## TypeName ## s.Release(PreAllocated); }                                                                                                                          \
		void Release ## TypeName ## s() { m_ ## TypeName ## s.Release(0); }                                                                                                                                     \
		size_t Count ## TypeName ## s() { return m_ ## TypeName ## s.Size(); }                                                                                                                                  \
		                                                                                                                                                                                                        \
		TypeName ## Handle Get         ## TypeName ## Handle(const S ## TypeName pDesc) const { return m_ ## TypeName ## s.GetHandle        (pDesc); }                                                          \
		TypeName ## Handle GetOrCreate ## TypeName ## Handle(const S ## TypeName pDesc)       { return m_ ## TypeName ## s.GetOrCreateHandle(pDesc); }                                                          \
		const std::pair<S ## TypeName, CDevice ## TypeName*>& Lookup ## TypeName (const TypeName ## Handle hDesc) const { return m_ ## TypeName ## s.Lookup(hDesc); }                                           \
		      std::pair<S ## TypeName, CDevice ## TypeName*>& Lookup ## TypeName (const TypeName ## Handle hDesc)       { return m_ ## TypeName ## s.Lookup(hDesc); }                                           \
		                                                                                                                                                                                                        \
	private:                                                                                                                                                                                                    \
		TypeName ## Handle Reserve     ## TypeName ## Handle(const S ## TypeName pDesc)       { return m_ ## TypeName ## s.ReserveHandle    (pDesc); }                                                          \
		void               Reserve     ## TypeName ## s     (u32k        hNum )       {        m_ ## TypeName ## s.Reserve          (hNum ); }                                                          \
		                                                                                                                                                                                                        \
		/* Express relationship between creator and storage */                                                                                                                                                  \
		struct TypeName ## _CreatorStorage_Tag                                                                                                                                                                  \
		{                                                                                                                                                                                                       \
			inline static ptrdiff_t offset() { return (ptrdiff_t)(&reinterpret_cast<char&>(reinterpret_cast<TypeCreator*>(0)->m_ ## TypeName ## s)); }                                                          \
			inline static TypeCreator* rebase(uk subbase) { return reinterpret_cast<TypeCreator*>(reinterpret_cast<tuk>(subbase) - offset()); }                                                            \
		};                                                                                                                                                                                                      \
		                                                                                                                                                                                                        \
		/* A heap containing all permutations of TypeName, they are contextually related to TypeCreator and are never evicted */                                                                                \
		CDevice ## TypeName * Create ## TypeName (const S ## TypeName pDesc);                                                                                                                                   \
		CComposedDeviceObjectStorage<TypeName ## Handle, S ## TypeName, CDevice ## TypeName, bRetryable, TypeCreator, &TypeCreator::Create ## TypeName, TypeName ## _CreatorStorage_Tag> m_ ## TypeName ## s;   \

////////////////////////////////////////////////////////////////////////////

// TODO: namespace
struct SDeviceObjectHelpers
{
	static void UpdateBuffer(CConstantBuffer* pBuffer, ukk src, size_t size, size_t offset, u32 numDataBlocks);

	struct SShaderInstanceInfo
	{
		SShaderInstanceInfo() : pHwShader(nullptr), pHwShaderInstance(nullptr), pDeviceShader(nullptr) {}

		CHWShader_D3D* pHwShader;
		CDrxNameTSCRC  technique; // temp
		uk          pHwShaderInstance;
		uk          pDeviceShader;
	};

	struct SConstantBufferBindInfo
	{
		EConstantBufferShaderSlot   shaderSlot   = eConstantBufferShaderSlot_PerDraw;
		EShaderStage                shaderStages = EShaderStage_All;
		_smart_ptr<CConstantBuffer> pBuffer;

		void                        swap(SConstantBufferBindInfo& other)
		{
			std::swap(shaderSlot, other.shaderSlot);
			std::swap(shaderStages, other.shaderStages);
			std::swap(pBuffer, other.pBuffer);
		}

	};

	template<typename T, size_t Alignment = DRX_PLATFORM_ALIGNMENT>
	class STypedConstants : private NoCopy
	{
		typedef _smart_ptr<CConstantBuffer> CConstantBufferPtr;

	public:
		STypedConstants(CConstantBufferPtr pBuffer)
			: m_pBuffer(pBuffer)
			, m_pCachedData(AlignCachedData())
			, m_CurrentBufferIndex(0)
		{
			ZeroStruct(m_pCachedData[0]);
		}

		STypedConstants(STypedConstants&& other)
			: m_pBuffer(std::move(other.m_pBuffer))
			, m_pCachedData(AlignCachedData())
			, m_CurrentBufferIndex(std::move(other.m_CurrentBufferIndex))
		{
			*m_pCachedData = *other.m_pCachedData;
		}

		void CommitChanges()
		{
			SDeviceObjectHelpers::UpdateBuffer(m_pBuffer.get(), m_pCachedData, Align(sizeof(T), Alignment), 0, m_CurrentBufferIndex + 1);
		}

		void BeginStereoOverride(bool bCopyLeftEyeData = true)
		{
			DRX_ASSERT(m_CurrentBufferIndex < CCamera::eEye_eCount - 1);

			if (bCopyLeftEyeData)
			{
				m_pCachedData[m_CurrentBufferIndex + 1] = m_pCachedData[m_CurrentBufferIndex];
			}

			++m_CurrentBufferIndex;
		}

		T*   operator->  () { return &m_pCachedData[m_CurrentBufferIndex]; }
		     operator T& () { return  m_pCachedData[m_CurrentBufferIndex]; }

	private:
		// NOTE: enough memory to hold an aligned struct size + the adjustment of a possible unaligned start
		u8              m_CachedMemory[((CCamera::eEye_eCount * sizeof(T) + (Alignment - 1)) & (~(Alignment - 1))) + (Alignment - 1)];
		T* AlignCachedData() { return reinterpret_cast<T*>(Align(uintptr_t(m_CachedMemory), Alignment)); }

	private:
		T*                 m_pCachedData;
		CConstantBufferPtr m_pBuffer;
		i32                m_CurrentBufferIndex;
	};

	class CShaderConstantUpr
	{
		static const EConstantBufferShaderSlot ReflectedBufferShaderSlot = eConstantBufferShaderSlot_PerDraw;  // Which constant buffer is used for reflection
		static i32k                       MaxReflectedBuffers       = 2;                                  // Currently at most vertex and pixel stages required

	public:
		CShaderConstantUpr();
		CShaderConstantUpr(CShaderConstantUpr&& other);
		~CShaderConstantUpr() {};

		void Reset();

		////////// Constant update via shader reflection //////////
		bool AllocateShaderReflection(::CShader* pShader, const CDrxNameTSCRC& technique, uint64 rtFlags, EShaderStage shaderStages);
		void InitShaderReflection(CDeviceGraphicsPSO& pipelineState);
		void InitShaderReflection(CDeviceComputePSO& pipelineState);
		bool IsShaderReflectionValid() const { return m_pShaderReflection && m_pShaderReflection->bValid; }
		void ReleaseShaderReflection();

		void BeginNamedConstantUpdate();
		void EndNamedConstantUpdate(const D3DViewPort* pVP);

		bool SetNamedConstant(const CDrxNameR& paramName, const Vec4 param, EHWShaderClass shaderClass = eHWSC_Pixel);
		bool SetNamedConstantArray(const CDrxNameR& paramName, const Vec4 params[], u32 numParams, EHWShaderClass shaderClass = eHWSC_Pixel);

		////////// Manual constant update via typed buffers //////////
		bool SetTypedConstantBuffer(EConstantBufferShaderSlot shaderSlot, CConstantBuffer* pBuffer, EShaderStage shaderStages = EShaderStage_Pixel);

		template<typename T>
		STypedConstants<T> BeginTypedConstantUpdate(EConstantBufferShaderSlot shaderSlot, EShaderStage shaderStages = EShaderStage_Pixel) const;

		template<typename T>
		void EndTypedConstantUpdate(STypedConstants<T>& constants) const;
		//////////////////////////////////////////////////////////////

		// Returns all managed constant buffers
		const std::vector<SConstantBufferBindInfo>& GetBuffers() const { return m_constantBuffers; }

	private:

		struct SShaderReflection
		{
			struct SBufferUpdateContext
			{
				i32            bufferIndex     = -1;
				EHWShaderClass shaderClass     = eHWSC_Num;  // somewhat redundant but avoids a loop over shader stages in Begin
				uk          pShaderInstance = nullptr;
				Vec4*          pMappedData     = nullptr;
			};

			i32                                                   bufferCount = 0;
			bool                                                  bValid = false;
			std::array<SBufferUpdateContext, MaxReflectedBuffers> bufferUpdateContexts;
		};

	protected:
		void ReleaseReflectedBuffers();
		bool IsBufferUsedForReflection(i32 bufferIndex) const;

		std::vector<SConstantBufferBindInfo> m_constantBuffers;
		std::unique_ptr<SShaderReflection>   m_pShaderReflection;
	};

	// Get shader instances for each shader stage
	typedef std::array<SShaderInstanceInfo, eHWSC_Num> THwShaderInfo;
	static EShaderStage GetShaderInstanceInfo(THwShaderInfo& result, ::CShader* pShader, const CDrxNameTSCRC& technique, uint64 rtFlags, u32 mdFlags, u32 mdvFlags, const UPipelineState pipelineState[eHWSC_Num], bool bAllowTesselation);

	// Check if shader has tessellation support
	static bool CheckTessellationSupport(SShaderItem& shaderItem);
	static bool CheckTessellationSupport(SShaderItem& shaderItem, EShaderTechniqueID techniqueId);
};

template<typename T>
SDeviceObjectHelpers::STypedConstants<T> SDeviceObjectHelpers::CShaderConstantUpr::BeginTypedConstantUpdate(EConstantBufferShaderSlot shaderSlot, EShaderStage shaderStages) const
{
	for (i32 i = 0, end = m_constantBuffers.size(); i < end; ++i)
	{
		auto& cb = m_constantBuffers[i];

		if (cb.shaderSlot == shaderSlot && cb.shaderStages == shaderStages)
		{
			DRX_ASSERT(!IsBufferUsedForReflection(i));
			return std::move(SDeviceObjectHelpers::STypedConstants<T>(cb.pBuffer));
		}
	}

	DRX_ASSERT(false);
	return std::move(SDeviceObjectHelpers::STypedConstants<T>(nullptr));
}

template<typename T>
void SDeviceObjectHelpers::CShaderConstantUpr::EndTypedConstantUpdate(SDeviceObjectHelpers::STypedConstants<T>& constants) const
{
	constants.CommitChanges();
}
