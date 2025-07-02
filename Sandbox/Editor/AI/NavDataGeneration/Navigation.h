// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <DrxAISystem/INavigation.h>
#include <DrxAISystem/IAISystem.h>
#include "CAISystem.h"

class CNavigation
{
public:
	explicit CNavigation(ISystem* pSystem);
	~CNavigation();

	const ShapeMap& GetDesignerPaths() const { return m_mapDesignerPaths; }

	// copies a designer path into provided list if a path of such name is found
	bool GetDesignerPath(tukk szName, SShape& path) const;

	bool Init();
	void Reset(IAISystem::EResetReason reason);
	void ShutDown();
	void FlushSystemNavigation();
	// Gets called after loading the mission
	void OnMissionLoaded();
	void ValidateNavigation();

	void Serialize(TSerialize ser);

	// writes areas into file
	void WritePolygonArea(CDrxFile& file, const string& name, const ListPositions& pts);
	void WriteAreasIntoFile(tukk fileName);

	// Prompt the exporting of data from AI - these should require not-too-much processing - the volume
	// stuff requires more processing so isn't done for a normal export
	void ExportData(tukk navFileName, tukk areasFileName, tukk roadsFileName,
		tukk vertsFileName, tukk volumeFileName,
		tukk flightFileName);

	void          FlushAllAreas();
	/// Check all the forbidden areas are sensible size etc
	bool          ValidateAreas();
	
	/// Checks if navigation shape exists - called by editor
	bool        DoesNavigationShapeExists(tukk szName, EnumAreaType areaType, bool road = false);
	
	bool CreateNavigationShape(const SNavigationShapeParams& params);
	/// Deletes designer created path/shape - called by editor
	void DeleteNavigationShape(tukk szName);

	void GetMemoryStatistics(IDrxSizer* pSizer);

	/// Meant for debug draws, there shouldn't be anything else to update here.
	void Update() const;

private:

	PerceptionModifierShapeMap m_mapPerceptionModifiers;
	ShapeMap                   m_mapOcclusionPlanes;
	ShapeMap                   m_mapGenericShapes;
	ShapeMap                   m_mapDesignerPaths;

	ISystem*                   m_pSystem;
};

#endif // NAVIGATION_H

