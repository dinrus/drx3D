// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CSceneElementCommon;
class CSceneViewCommon;

class QMenu;
class QModelIndex;
class QPoint;

class CSceneContextMenuCommon
{
public:
	CSceneContextMenuCommon(CSceneViewCommon* pSceneView);
	virtual ~CSceneContextMenuCommon() {}

	void Attach();

	void Show(const QPoint& p);

protected:
	QMenu* GetMenu() { return m_pMenu; }

private:
	virtual void AddSceneElementSection(CSceneElementCommon* pSceneElement) {}

private:
	void AddSceneElementSectionInternal(const QModelIndex& index);
	void AddGeneralSectionInternal();

private:
	CSceneViewCommon* m_pSceneView;

	QMenu* m_pMenu;
};

