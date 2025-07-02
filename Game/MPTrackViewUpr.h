// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef MP_TRACK_VIEW_MANAGER
#define MP_TRACK_VIEW_MANAGER

#include <drx3D/Game/GameRules.h>
#include <drx3D/Movie/IMovieSystem.h>

class CMPTrackViewUpr : public IMovieListener
{
public:
	CMPTrackViewUpr();
	~CMPTrackViewUpr();

	void Init();
	void Update();

	void Server_SynchAnimationTimes(CGameRules::STrackViewParameters& params); 
	void Client_SynchAnimationTimes(const CGameRules::STrackViewParameters& params);
	void AnimationRequested(const CGameRules::STrackViewRequestParameters& params);
	bool HasTrackviewFinished(const DrxHashStringId& id) const;

	IAnimSequence* FindTrackviewSequence(i32 trackviewId);

private:

	// IMovieListener
	virtual void OnMovieEvent(IMovieListener::EMovieEvent movieEvent, IAnimSequence* pAnimSequence);
	// ~IMovieListener

	i32 m_FinishedTrackViews[CGameRules::STrackViewParameters::sMaxTrackViews];
	float m_FinishedTrackViewTimes[CGameRules::STrackViewParameters::sMaxTrackViews];
	i32 m_FinishedTrackViewCount;
	bool m_movieListener;

};

#endif