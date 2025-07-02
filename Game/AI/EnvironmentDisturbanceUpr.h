// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef EnvironmentDisturbanceUpr_h
#define EnvironmentDisturbanceUpr_h

#include <drx3D/AI/IVisionMap.h>

namespace GameAI
{
	class ObservableEvent
	{
	public:
		ObservableEvent()
			: m_expirationTime(0.0f)
			, m_position(ZERO)
		{};

		void Initialize( const Vec3& position, float duration, u8 faction, tukk signal );
		void Release();

		bool IsExpired( CTimeValue currentTime );
		bool HasBeenObservedBy( EntityId entityId );
		void SetObservedBy( EntityId entityId );
		VisionID GetVisionId() { return m_visionId; }
		const Vec3& GetPosition() { return m_position; }
		tukk GetSignal() { return m_signal; }

	private:
		CTimeValue m_expirationTime;
		VisionID   m_visionId;
		Vec3       m_position;
		string     m_signal;

		std::vector<EntityId> m_obeservedBy;
	};

	class EnvironmentDisturbanceUpr
	{
	public:
		EnvironmentDisturbanceUpr()
		{
		};

		~EnvironmentDisturbanceUpr();;

		void Reset();
		void Update();

		void AddObservableEvent( const Vec3& position, float duration, tukk signal, EntityId sourceEntityId = 0 );

	private:
		void RemoveExpiredEvents();

		typedef std::vector<ObservableEvent> ObservableEvents;
		ObservableEvents m_observableEvents;
	};
}

#endif //EnvironmentDisturbanceUpr_h
