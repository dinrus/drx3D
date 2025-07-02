// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

//! Used to collect the assets needed for streaming and to gather statistics.
struct IResourceCollector
{
	// <interfuscator:shuffle>
	//! \param dwMemSize 0xffffffff if size is unknown.
	//! \return true if new resource was added, false if resource was already registered
	virtual bool AddResource(tukk szFileName, u32k dwMemSize) = 0;

	//! \param szFileName Needs to be registered before with AddResource().
	//! \param pInstance Must not be 0.
	virtual void AddInstance(tukk szFileName, uk pInstance) = 0;

	//! \param szFileName Needs to be registered before with AddResource().
	virtual void OpenDependencies(tukk szFileName) = 0;

	virtual void CloseDependencies() = 0;

	//! Resets the internal data structure for the resource collector.
	virtual void Reset() = 0;
	// </interfuscator:shuffle>
protected:
	virtual ~IResourceCollector() {}
};

class NullResCollector : public IResourceCollector
{
public:
	virtual bool AddResource(tukk szFileName, u32k dwMemSize) { return true; }
	virtual void AddInstance(tukk szFileName, uk pInstance)        {}
	virtual void OpenDependencies(tukk szFileName)                    {}
	virtual void CloseDependencies()                                         {}
	virtual void Reset()                                                     {}

	virtual ~NullResCollector() {}
};

//! \endcond