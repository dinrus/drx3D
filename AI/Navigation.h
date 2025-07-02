// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   Navigation.h
   $Id$
   Описание: interface for the CGraph class.

   -------------------------------------------------------------------------
   История:
   - ?
   - 4 May 2009   : Evgeny Adamenkov: Removed IRenderer

 *********************************************************************/

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <drx3D/AI/INavigation.h>

class CDrxBufferedFileReader;

bool ReadPolygonArea(CDrxBufferedFileReader& file, i32 version, string& name, ListPositions& pts);

class CNavigation : public INavigation
{
public:
	explicit CNavigation(ISystem* pSystem);
	~CNavigation();

	// INavigation
	virtual float  GetNearestPointOnPath(tukk szPathName, const Vec3& vPos, Vec3& vResult, bool& bLoopPath, float& totalLength) const;
	virtual void   GetPointOnPathBySegNo(tukk szPathName, Vec3& vResult, float segNo) const;
	//~INavigation

	const ShapeMap& GetDesignerPaths() const { return m_mapDesignerPaths; }

	// copies a designer path into provided list if a path of such name is found
	bool  GetDesignerPath(tukk szName, SShape& path) const;

	void  Reset(IAISystem::EResetReason reason);
	void  ShutDown();
	
	// // loads the triangulation for this level and mission
	void  LoadNavigationData(tukk szLevel, tukk szMission);

	// reads (designer paths) areas from file. clears the existing areas
	void ReadAreasFromFile(CDrxBufferedFileReader&, i32 fileVersion);

	void Update(CTimeValue currentTime, float frameTime);

	/// This is just for debugging
	tukk GetNavigationShapeName(i32 nBuildingID) const;
	bool        DoesNavigationShapeExists(tukk szName, EnumAreaType areaType, bool road = false);
	bool        CreateNavigationShape(const SNavigationShapeParams& params);
	void        DeleteNavigationShape(tukk szName);

	void FlushAllAreas();

	// Offsets all areas when segmented world shifts
	void OffsetAllAreas(const Vec3& additionalOffset);

	/// Returns nearest designer created path/shape.
	/// The devalue parameter specifies how long the path will be unusable by others after the query.
	/// If useStartNode is true the start point of the path is used to select nearest path instead of the nearest point on path.
	virtual tukk GetNearestPathOfTypeInRange(IAIObject* requester, const Vec3& pos, float range, i32 type, float devalue, bool useStartNode);

	IAISystem::ENavigationType CheckNavigationType(const Vec3& pos, i32& nBuildingID, IAISystem::tNavCapMask navCapMask) const;

	void                       GetMemoryStatistics(IDrxSizer* pSizer);
	size_t                     GetMemoryUsage() const;

#ifdef DRXAISYS_DEBUG
	void DebugDraw() const;
#endif //DRXAISYS_DEBUG

private:
	// <AI SHAPE STUFF>
	ShapeMap           m_mapDesignerPaths;
};

#endif // NAVIGATION_H
