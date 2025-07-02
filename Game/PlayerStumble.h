// –†–∞–∑—Ä–∞–±–æ—Ç–∫–∞ 2018-2025 DinrusPro / Dinrus Group. –Õ÷œ ƒËÌÛÒ.

/*************************************************************************

-------------------------------------------------------------------------
–ò—Å—Ç–æ—Ä–∏—è:
- 21:07:2010  Created by Jens Schˆbel

*************************************************************************/

#ifndef PLAYERSTUMBLE_H
#define PLAYERSTUMBLE_H

#pragma once

namespace PlayerActor
{
  namespace Stumble
  {
    struct StumbleParameters
    {
      float minDamping;
      float maxDamping;
      float periodicTime;
      float strengthX;
      float strengthY;
      float randomness;
    };

    class CPlayerStumble
    {
    public:
      CPlayerStumble();
      void Disable();
      bool Update(float deltaTime, const pe_status_dynamics &dynamics);
      void Start(StumbleParameters* stumbleParameters);

      inline const Vec3& GetCurrentActionImpulse() const { return m_finalActionImpulse; }
    protected:
    private:
    public:
    protected:
    private:
      StumbleParameters m_stumbleParameter;
      Vec3	m_finalActionImpulse;
      float m_currentPeriodicScaleX;
      float m_currentPeriodicScaleY;
      float m_currentPastTime;
    };
  } //endns Stumble
} //endns Player















#endif

