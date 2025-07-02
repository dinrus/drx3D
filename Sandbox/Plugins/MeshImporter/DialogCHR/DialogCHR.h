// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DialogCommon.h"
#include "Viewport.h"
#include "ModelProperties/ModelProperties.h"

class CRcInOrderCaller;
class CSkeletonPoseModifier;

class CSceneElementCommon;
class CSceneModelSkeleton;
class CSplitViewportContainer;
class CSceneViewCommon;
class CSceneContextMenuCommon;
class CModelProperties;
class CMaterialGenerator;
class CTempRcObject;

class CSceneCHR;

namespace FbxTool
{

class CScene;

} //endns FbxTool

namespace FbxMetaData
{

struct SMetaData;

} //endns FbxMetaData

struct ICharacterInstance;
struct IPhysicalEntity;

class QPropertyTree;

class QTemporaryDir;

namespace MeshImporter
{

namespace Private_DialogCHR
{

class CPreviewModeWidget;
struct SViewSettings;

} //endns Private_DialogCHR

class CDialogCHR : public CBaseDialog, public QViewportConsumer
{
private:
	struct SCharacter
	{
		_smart_ptr<ICharacterInstance>    m_pCharInstance;
		IPhysicalEntity*                           m_pPhysicalEntity;
		std::shared_ptr<CSkeletonPoseModifier> m_pPoseModifier;

		SCharacter()
			: m_pPhysicalEntity(nullptr)
		{
		}

		~SCharacter();

		void DestroyPhysics();
	};

	struct SMaterial
	{
		std::unique_ptr<QTemporaryDir> pTempDir;
		string materialName;
	};
public:
	CDialogCHR(QWidget* pParent = nullptr);
	~CDialogCHR();

	// CBaseDialog implementation.

	virtual QString ShowSaveAsDialog();
	virtual bool         SaveAs(SSaveContext& ctx) override;

	virtual i32          GetOpenFileFormatFlags() override { return eFileFormat_CHR; }

	virtual void         AssignScene(const SImportScenePayload* pUserData) override;
	virtual void         UnloadScene() override;

	virtual tukk  GetDialogName() const override;
	virtual bool         MayUnloadScene() override { return true; }
	virtual i32 GetToolButtons() const override;
protected:
	virtual void         OnIdleUpdate() override;
private:
	void SetupUI();

	std::unique_ptr<CModelProperties::SSerializer> CreateSerializer(CSceneElementCommon* pSceneElement);

	void UpdateSkin();
	void UpdateSkeleton();
	void UpdateStatGeom();
	void UpdateCharacter();

	void                 WriteNodeMetaData(std::vector<FbxMetaData::SNodeMeta>& nodeMetaData) const;
	void                 WriteJointPhysicsData(std::vector<FbxMetaData::SJointPhysicsData>& jointPhysicsData) const;
	void                 ReadNodeMetaData(const std::vector<FbxMetaData::SNodeMeta>& nodeMetaData);
	void                 ReadJointPhysicsData(const std::vector<FbxMetaData::SJointPhysicsData>& jointPhysicsData);
	void                 CreateMetaDataChr(FbxMetaData::SMetaData& metaData, const string& sourceFilename) const;
	void                 CreateMetaDataCgf(FbxMetaData::SMetaData& metaData) const;
	void                 CreateMetaDataSkin(FbxMetaData::SMetaData& metaData) const;
	CMaterial* CreateSkeletonMaterial(const QString& filePath);
	void Physicalize();

	void                 RenderJoints(const SRenderContext& rc, ICharacterInstance* pCharInstance);
	void                 RenderCharacter(const SRenderContext& rc, ICharacterInstance* pCharInstance);
	void                 RenderPhysics(const SRenderContext& rc, ICharacterInstance* pCharacter);
	void                 RenderCgf(const SRenderContext& rc);
	virtual void         OnViewportRender(const SRenderContext& rc) override;
	virtual void OnViewportMouse(const SMouseEvent& ev) override;

	void PickJoint(QViewport& viewport, float x, float y);

	void SelectJoint(i32 jointId);

	std::unique_ptr<CSceneModelSkeleton> m_pSceneModel;
	std::unique_ptr<SCharacter> m_pCharacter;

	std::unique_ptr<CTempRcObject> m_skinObject;
	std::unique_ptr<CTempRcObject> m_skeletonObject;
	std::unique_ptr<CTempRcObject> m_statGeomObject;

	_smart_ptr<IStatObj> m_pStatObj;

	std::unique_ptr<CSceneCHR> m_pSceneUserData;

	SMaterial m_material;

	// Bones.
	std::vector<float> m_jointHeat;
	i32 m_hitJointId;

	// User interface.
	CSceneViewCommon* m_pSceneView;
	std::unique_ptr<CSceneContextMenuCommon> m_pSceneContextMenu;
	std::unique_ptr<Private_DialogCHR::SViewSettings> m_pViewSettings;
	CSplitViewportContainer*                    m_pViewportContainer;
	Private_DialogCHR::CPreviewModeWidget* m_pPreviewModeWidget;
	QPropertyTree* m_pPropertyTree;
	std::unique_ptr<CModelProperties> m_pModelProperties;
};

} //endns MeshImporter

