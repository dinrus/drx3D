// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Decorators/Range.h>
#include <Drx3DEngine/CGF/DrxHeaders.h>

#include <memory>
#include <QWidget>
#include <QViewportSettings.h>

class QPropertyTree;

struct SViewSettings
{
	enum EViewportMode
	{
		eViewportMode_RcOutput,
		eViewportMode_SourceView,
		eViewportMode_Split
	};

	enum EShadingMode
	{
		eShadingMode_Default,
		eShadingMode_VertexColors,
		eShadingMode_VertexAlphas,
		eShadingMode_COUNT
	};

	bool          bShowEdges;
	bool          bShowProxies;
	bool          bShowSize;
	bool          bShowPivots;
	bool          bShowJointNames;
	EViewportMode viewportMode;
	EShadingMode shadingMode;
	i32           lod;

	SViewSettings()
		: bShowEdges(false)
		, bShowProxies(false)
		, bShowSize(false)
		, bShowPivots(false)
		, bShowJointNames(true)
		, viewportMode(eViewportMode_RcOutput)
		, shadingMode(eShadingMode_Default)
		, lod(0)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

struct SDisplayOptions
{
	SViewportSettings viewport;

	SDisplayOptions()
		: pUserOptions()
		, pUserOptionsName(nullptr)
		, pUserOptionsLabel(nullptr)
	{}

	void Serialize(Serialization::IArchive& ar)
	{
		if (pUserOptionsName)
		{
			ar(pUserOptions, pUserOptionsName, pUserOptionsLabel);
		}

		// SViewportSettings
		ar(viewport.rendering, "debug", "Debug");
		ar(viewport.camera, "camera", "Camera");
		ar(viewport.grid, "grid", "Grid");
		ar(viewport.lighting, "lighting", "Lighting");
		ar(viewport.background, "background", "Background");
	}

	void SetUserOptions(const Serialization::SStruct& options, tukk szName, tukk szLabel)
	{
		pUserOptions = options;
		pUserOptionsName = szName;
		pUserOptionsLabel = szLabel;
	}
private:
	// Additional, user-provided options.
	Serialization::SStruct pUserOptions;
	tukk            pUserOptionsName;
	tukk            pUserOptionsLabel;
};

class CDisplayOptionsWidget : public QWidget
{
	Q_OBJECT
public:
	CDisplayOptionsWidget(QWidget* pParent = nullptr);

	const SDisplayOptions& GetSettings() const;

	void                   SetUserOptions(const Serialization::SStruct& options, tukk szName, tukk szLabel);
signals:
	void                   SigChanged();
private slots:
	void                   OnPropertyTreeChanged();
private:
	QPropertyTree*                   m_pPropertyTree;
	std::unique_ptr<SDisplayOptions> m_pOptions;
};

