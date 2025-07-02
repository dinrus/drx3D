// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __MANNEQUININTERFACE_H__
#define __MANNEQUININTERFACE_H__

#include <drx3D/Act/IDrxMannequin.h>

class CProceduralClipFactory;

class CMannequinInterface : public IMannequin
{
public:
	CMannequinInterface();
	~CMannequinInterface();

	// IMannequin
	virtual void                         UnloadAll();
	virtual void                         ReloadAll();

	virtual IAnimationDatabaseUpr&   GetAnimationDatabaseUpr();
	virtual IActionController*           CreateActionController(IEntity* pEntity, SAnimationContext& context);
	virtual IActionController*           FindActionController(const IEntity& entity);
	virtual IMannequinEditorUpr*     GetMannequinEditorUpr();
	virtual CMannequinUserParamsUpr& GetMannequinUserParamsUpr();
	virtual IProceduralClipFactory&      GetProceduralClipFactory();

	virtual void                         AddMannequinGameListener(IMannequinGameListener* pListener);
	virtual void                         RemoveMannequinGameListener(IMannequinGameListener* pListener);
	virtual u32                       GetNumMannequinGameListeners();
	virtual IMannequinGameListener*      GetMannequinGameListener(u32 idx);
	virtual void                         SetSilentPlaybackMode(bool bSilentPlaybackMode);
	virtual bool                         IsSilentPlaybackMode() const;
	// ~IMannequin

private:
	void RegisterCVars();

private:
	class CAnimationDatabaseUpr*        m_pAnimationDatabaseUpr;
	std::vector<IMannequinGameListener*>    m_mannequinGameListeners;
	CMannequinUserParamsUpr             m_userParamsUpr;
	std::unique_ptr<CProceduralClipFactory> m_pProceduralClipFactory;
	bool m_bSilentPlaybackMode;
};

#endif //!__MANNEQUININTERFACE_H__
