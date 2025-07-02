// –†–∞–∑—Ä–∞–±–æ—Ç–∫–∞ 2018-2025 DinrusPro / Dinrus Group. –Õ÷œ ƒËÌÛÒ.

/*************************************************************************

-------------------------------------------------------------------------
–ò—Å—Ç–æ—Ä–∏—è:
- 31:05:2010  Created by Jens Schˆbel

*************************************************************************/

#ifndef STEREOFRAMEWORK_H
#define STEREOFRAMEWORK_H

#include <drx3D/Game/Stereo3D/StereoZoom.h>

namespace Stereo3D
{	
	void Update(float deltaTime);

	namespace Zoom
	{
		void SetFinalPlaneDist(float planeDist, float transitionTime);
		void SetFinalEyeDist(float eyeDist, float transitionTime);
		void ReturnToNormalSetting(float);
	} //endns zoom

	namespace Weapon
	{
		i32k MAX_RAY_IDS = 5;
		const QueuedRayID INVALID_RAY_ID = 0;

		class CWeaponCheck
		{
		public:
			CWeaponCheck() 
				: m_closestDist(0.0f)
				, m_closestCastDist(1000.f) 	  
				, m_numFramesWaiting(0)
				, m_numResults(0)
			{
			}

			~CWeaponCheck();
			void Update(float deltaTime);
			void OnRayCastResult(const QueuedRayID &rayID, const RayCastResult &result);
			float GetCurrentPlaneDist();

		private:
			void CastRays();

			float  m_closestDist;
			float  m_closestCastDist;
			QueuedRayID m_rayIDs[MAX_RAY_IDS];
			u8  m_numFramesWaiting;
			u8  m_numResults;
		};
	} // 
} //endns Stereo3D

#endif
