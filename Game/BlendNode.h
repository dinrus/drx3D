// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание:  BlendNodes and BlendGroups for ScreenEffects

-------------------------------------------------------------------------
История:
- 23:1:2008   Created by Benito G.R. - Refactor'd from John N. ScreenEffects.h/.cpp

*************************************************************************/

#ifndef _BLEND_NODE_H_
#define _BLEND_NODE_H_

//-BlendJobNode------------------------
// Has all of the information for one blend.
// That is, a blend type, an effect, and a speed.
// Speed is how fast progress goes from 0 to 1.

struct IBlendedEffect;
struct IBlendType;

class CBlendGroup;

class CBlendJobNode
{
	
	friend class CBlendGroup;

	public:
		CBlendJobNode();
		~CBlendJobNode();

		void Init(IBlendType* pBlend, IBlendedEffect* pFx, float speed);
		void Update(float frameTime);
		bool Done() const{ return (m_progress==1.0f);}

		void Reset();

		void GetMemoryUsage(IDrxSizer *pSizer ) const {}
	private:
		IBlendedEffect	*m_myEffect;
		IBlendType			*m_blendType;
	
		float m_speed;
		float m_progress;

};


// A blend group is a queue of blend jobs.
class CBlendGroup 
{
	public:
		CBlendGroup();
		~CBlendGroup();

		void Update(float frameTime);
		void AddJob(IBlendType* pBlend, IBlendedEffect* pFx, float speed);
		bool HasJobs();
		void Reset();

		void GetMemoryUsage(IDrxSizer* s) const;

	private:

		void AllocateMinJobs();
		typedef std::vector<CBlendJobNode*> TJobVector;
		TJobVector			m_jobs; 

		i32						m_currentJob;
		u32					m_nextFreeSlot;
		u32					m_activeJobs;

		u32					m_maxActiveJobs;
			
};

#endif