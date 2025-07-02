// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IExportManager.h>

typedef std::vector<IExporter*>     TExporters;
typedef std::map<CBaseObject*, i32> TObjectMap;

class SANDBOX_API CExportManager : public IExportManager
{
public:

	CExportManager();
	virtual ~CExportManager();

	//! Register exporter
	//! return true if succeed, otherwise false
	virtual bool       RegisterExporter(IExporter* pExporter);

	virtual IExporter* GetExporterForExtension(tukk szExtension) const override;

	//! Export specified geometry
	//! return true if succeed, otherwise false
	bool Export(tukk defaultName, tukk defaultExt = "", tukk defaultPath = "", bool isSelectedObjects = true,
	            bool isSelectedRegionObjects = false, bool isTerrain = false, bool isOccluder = false, bool bAnimationExport = false);

	//! Add to Export Data geometry from selected objects
	//! return true if succeed, otherwise false
	bool AddSelectedObjects();

	bool AddSelectedEntityObjects();

	//! Add to Export Data geometry from objects inside selected region volume
	//! return true if succeed, otherwise false
	bool AddSelectedRegionObjects();

	//! Add to Export Data terrain geometry
	//! return true if succeed, otherwise false
	bool AddTerrain();

	//! Export to file collected data, using specified exporter throughout the file extension
	//! return true if succeed, otherwise false
	bool               ExportToFile(tukk filename, bool bClearDataAfterExport = true);

	bool               ImportFromFile(tukk filename);
	const SExportData& GetData() const { return m_data; }

	//! Exports the stat obj to the obj file specified
	//! returns true if succeeded, otherwise false
	virtual bool ExportSingleStatObj(IStatObj* pStatObj, tukk filename);

private:
	void AddMesh(SExportObject* pObj, const IIndexedMesh* pIndMesh, Matrix34A* pTm = 0);
	bool AddStatObj(SExportObject* pObj, IStatObj* pStatObj, Matrix34A* pTm = 0);
	bool AddCharacter(SExportObject* pObj, ICharacterInstance* pCharacter, Matrix34A* pTm = 0);
	bool AddMeshes(SExportObject* pObj);
	bool AddObject(CBaseObject* pBaseObj);
	bool AddVegetation();
	bool AddOcclusionObjects();
	bool IsNotChildOfGroup(CBaseObject* pObj);
	void SolveHierarchy();

	void AddPosRotScale(SExportObject* pObj, const CBaseObject* pBaseObj);
	void AddEntityData(SExportObject* pObj, EAnimParamType dataType, const float fValue, const float fTime);

	bool IsDuplicateObjectBeingAdded(const string& newObject);

private:

	TExporters   m_exporters;
	SExportData  m_data;
	bool         m_isPrecaching;
	bool         m_isOccluder;
	float        m_fScale;
	TObjectMap   m_objectMap;
	bool         m_bAnimationExport;

	CBaseObject* m_pBaseObj;
};

