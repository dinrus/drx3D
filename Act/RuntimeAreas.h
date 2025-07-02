// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef RUNTIME_AREAS_H
#define RUNTIME_AREAS_H

class CRuntimeAreaUpr : public ISystemEventListener
{
public:

	CRuntimeAreaUpr();
	virtual ~CRuntimeAreaUpr();

	void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;

private:

	typedef std::vector<EntityId> TEntityArray;

	struct SXMLTags
	{
		static char const* const sMergedMeshSurfaceTypesRoot;
		static char const* const sMergedMeshSurfaceTag;
		static char const* const sAudioTag;

		static char const* const sNameAttribute;
		static char const* const sATLTriggerAttribute;
		static char const* const sATLRtpcAttribute;
	};

	void FillAudioControls();
	void DestroyAreas();
	void CreateAreas();

	TEntityArray m_areas;
	TEntityArray m_areaObjects;
};

#endif
