// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Implements the Editor->Game communication interface.
  
 -------------------------------------------------------------------------
  История:
  - 30:8:2004   11:17 : Created by M�rcio Martins

*************************************************************************/
#ifndef __EDITORGAME_H__
#define __EDITORGAME_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/CoreX/Game/IGameRef.h>
#include <DrxSandbox/IEditorGame.h>

struct IGameStartup;
class CEquipmentSystemInterface;

class CEditorGame :
	public IEditorGame
{
public:
	CEditorGame();
    CEditorGame(tukk binariesDir);
	virtual ~CEditorGame();

	virtual bool Init(ISystem *pSystem, IGameToEditorInterface *pGameToEditorInterface) override;
    virtual i32 Update(bool haveFocus, u32 updateFlags) override;
	virtual void Shutdown() override;
	virtual bool SetGameMode(bool bGameMode) override;
	virtual IEntity * GetPlayer() override;
	virtual void SetPlayerPosAng(Vec3 pos,Vec3 viewDir) override;
	virtual void HidePlayer(bool bHide) override;
	virtual void OnBeforeLevelLoad() override;
	virtual void OnAfterLevelInit(tukk levelName, tukk levelFolder) override;
	virtual void OnAfterLevelLoad(tukk levelName, tukk levelFolder) override;
	virtual void OnCloseLevel() override;
	virtual void OnSaveLevel() override;
	virtual bool BuildEntitySerializationList(XmlNodeRef output) override;
	virtual bool GetAdditionalMinimapData(XmlNodeRef output) override;

	virtual IFlowSystem * GetIFlowSystem() override;
	virtual IGameTokenSystem* GetIGameTokenSystem() override;
	virtual IEquipmentSystemInterface* GetIEquipmentSystemInterface() override;

	virtual bool IsMultiplayerGameRules() override { return m_bUsingMultiplayerGameRules; }
	virtual bool SupportsMultiplayerGameRules() override { return true; }
	virtual void ToggleMultiplayerGameRules() override;

	virtual void RegisterTelemetryTimelineRenderers(Telemetry::ITelemetryRepository* pRepository) override;

	virtual void OnDisplayRenderUpdated( bool displayHelpers ) override;
	virtual void OnEntitySelectionChanged(EntityId entityId, bool isSelected) override {}
	virtual void OnReloadScripts(EReloadScriptsType scriptsType) override {}

private:
    void InitMembers(tukk binariesDir);
    void FillSystemInitParams(SSysInitParams &startupParams, ISystem* system);
	void InitUIEnums(IGameToEditorInterface* pGTE);
	void InitGlobalFileEnums(IGameToEditorInterface* pGTE);
	void InitActionEnums(IGameToEditorInterface* pGTE);
	void InitEntityClassesEnums(IGameToEditorInterface* pGTE);
	void InitLevelTypesEnums(IGameToEditorInterface* pGTE);
	void InitEntityArchetypeEnums(IGameToEditorInterface* pGTE, tukk levelFolder = NULL, tukk levelName = NULL);
	void InitForceFeedbackEnums(IGameToEditorInterface* pGTE);
	void InitActionInputEnums(IGameToEditorInterface* pGTE);
	void InitReadabilityEnums(IGameToEditorInterface* pGTE);
	void InitActionMapsEnums(IGameToEditorInterface* pGTE);
	void InitLedgeTypeEnums(IGameToEditorInterface* pGTE);
	void InitSmartMineTypeEnums(IGameToEditorInterface* pGTE);
	void InitDamageTypeEnums(IGameToEditorInterface *pGTE);
	void InitDialogBuffersEnum(IGameToEditorInterface* pGTE);
	void InitTurretEnum(IGameToEditorInterface* pGTE);
	void InitDoorPanelEnum(IGameToEditorInterface* pGTE);
	void InitModularBehaviorTreeEnum(IGameToEditorInterface* pGTE);

	bool ConfigureNetContext( bool on );
	static void OnChangeEditorMode( ICVar * );
	void EnablePlayer(bool bPlayer);
	static void ResetClient(IConsoleCmdArgs*);
	static tukk GetGameRulesName();
	void ScanBehaviorTrees(const string& folderName, std::vector<string>& behaviorTrees);

	IGameRef m_pGame;
	IGameStartup* m_pGameStartup;
	CEquipmentSystemInterface* m_pEquipmentSystemInterface;

	bool m_bEnabled;
	bool m_bGameMode;
	bool m_bPlayer;
	bool m_bUsingMultiplayerGameRules;

	tukk m_binariesDir;
	IGameToEditorInterface* m_pGTE;

	static ICVar* s_pEditorGameMode;
	static CEditorGame* s_pEditorGame;
	
};


#endif //__EDITORGAME_H__
