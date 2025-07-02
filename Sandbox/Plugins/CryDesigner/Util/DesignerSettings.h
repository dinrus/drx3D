// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>

class DesignerObject;

namespace Designer
{
struct ConsoleVarsForExcluisveMode
{
	ConsoleVarsForExcluisveMode()
	{
		memset(this, 0, sizeof(*this));
	}
	i32 r_DisplayInfo;
	i32 r_HDRRendering;
	i32 r_PostProcessEffects;
	i32 r_ssdo;
	i32 e_Vegetation;
	i32 e_WaterOcean;
	i32 e_WaterVolumes;
	i32 e_Terrain;
	i32 e_Shadows;
	i32 e_Particles;
	i32 e_Clouds;
	i32 e_SkyBox;
};

struct DesignerExclusiveMode
{
	DesignerExclusiveMode()
	{
		m_OldTimeOfDay = NULL;
		m_OldTimeOfTOD = 0;
		m_OldCameraTM = Matrix34::CreateIdentity();
		m_OldObjectHideMask = 0;
		m_bOldLockCameraMovement = false;

		m_bExclusiveMode = false;
	}

	void EnableExclusiveMode(bool bEnable);
	void SetTimeOfDayForExclusiveMode();
	void SetCVForExclusiveMode();
	void SetObjectsFlagForExclusiveMode();

	void RestoreTimeOfDay();
	void RestoreCV();
	void RestoreObjectsFlag();

	void CenterCameraForExclusiveMode();

	void SetTime(ITimeOfDay* pTOD, float fTime);

	ConsoleVarsForExcluisveMode  m_OldConsoleVars;
	std::map<CBaseObject*, bool> m_ObjectHiddenFlagMap;
	XmlNodeRef                   m_OldTimeOfDay;
	float                        m_OldTimeOfTOD;
	Matrix34                     m_OldCameraTM;
	i32                          m_OldObjectHideMask;
	bool                         m_bOldLockCameraMovement;

	bool                         m_bExclusiveMode;
};

struct DesignerSettings
{
	bool  bExclusiveMode;
	bool  bSeamlessEdit;
	bool  bDisplayBackFaces;
	bool  bKeepPivotCentered;
	bool  bHighlightElements;
	float fHighlightBoxSize;
	bool  bDisplayDimensionHelper;
	bool  bDisplayTriangulation;
	bool  bDisplaySubdividedResult;
	bool  bDisplayVertexNormals;
	bool  bDisplayPolygonNormals;

	void Load();
	void Save();

	void Update(bool continuous);

	DesignerSettings();
	void Serialize(Serialization::IArchive& ar);
};

extern DesignerExclusiveMode gExclusiveModeSettings;
extern DesignerSettings gDesignerSettings;
}

