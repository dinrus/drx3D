// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/EntitySystem.h>

class CEntityLoadUpr
{
public:
	CEntityLoadUpr() = default;
	~CEntityLoadUpr();

	void Reset();

	bool LoadEntities(XmlNodeRef& entitesNode, bool bIsLoadingLevelFile, const Vec3& segmentOffset = Vec3(0, 0, 0));

	void PrepareBatchCreation(i32 nSize);
	bool CreateEntity(CEntity* pPreallocatedEntity, SEntityLoadParams& loadParams, EntityId& outUsingId, bool bIsLoadingLevellFile);
	void OnBatchCreationCompleted();

	void GetMemoryStatistics(IDrxSizer* pSizer) const
	{
		pSizer->Add(*this);
		pSizer->AddContainer(m_queuedFlowgraphs);
	}

	bool ExtractArcheTypeLoadParams(XmlNodeRef& entityNode, SEntitySpawnParams& outLoadParams) const;
	bool ExtractEntityLoadParams(XmlNodeRef& entityNode, SEntitySpawnParams& outLoadParams) const;
	bool CreateEntity(XmlNodeRef& entityNode, SEntitySpawnParams& pParams, EntityId& outUsingId);

private:
	// Loading helpers
	bool ReserveEntityIds(XmlNodeRef& entityNode);
	bool CanParseEntity(XmlNodeRef& entityNode);
	bool ParseEntities(XmlNodeRef& entityNode, bool bIsLoadingLevelFile, const Vec3& segmentOffset);

	bool ExtractCommonEntityLoadParams(XmlNodeRef& entityNode, SEntityLoadParams& outLoadParams, const Vec3& segmentOffset, bool bWarningMsg, tukk szEntityClass, tukk szEntityName) const;
	bool ExtractArcheTypeLoadParams(XmlNodeRef& entityNode, SEntityLoadParams& outLoadParams, const Vec3& segmentOffset, bool bWarningMsg) const;
	bool ExtractEntityLoadParams(XmlNodeRef& entityNode, SEntityLoadParams& outLoadParams, const Vec3& segmentOffset, bool bWarningMsg) const;

	// Batch creation helpers
	void AddQueuedFlowgraph(CEntity* pEntity, XmlNodeRef& pNode);
	void AddQueuedEntityLink(CEntity* pEntity, XmlNodeRef& pNode);

	// Flowgraph queue for Flowgraph initiation at post Entity batch creation
	struct SQueuedFlowGraph
	{
		CEntity*   pEntity;
		XmlNodeRef pNode;

		SQueuedFlowGraph() : pEntity(NULL), pNode() {}
	};
	typedef std::vector<SQueuedFlowGraph> TQueuedFlowgraphs;
	TQueuedFlowgraphs  m_queuedFlowgraphs;

	TQueuedFlowgraphs  m_queuedEntityLinks;
	std::vector<u8> m_entityPool;
};
