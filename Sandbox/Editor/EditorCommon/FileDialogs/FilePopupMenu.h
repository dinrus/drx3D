// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QMenu>
#include <QAction>
#include <QFileInfo>

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

class QFileInfo;
class QWidget;

class EDITOR_COMMON_API CFilePopupMenu : public QMenu
{
	Q_OBJECT

public:
	struct SFilePopupMenuAction : public QAction
	{
		template<typename Func>
		SFilePopupMenuAction(const QString& text, QWidget* pParent, Func slot)
			: QAction(text, pParent)
		{
			connect(this, &SFilePopupMenuAction::triggered, slot);
		}
	};

public:
	explicit CFilePopupMenu(const QFileInfo& fileInfo, QWidget* pParent);

	const QFileInfo& fileInfo() const { return m_fileInfo; }

protected:
	QFileInfo m_fileInfo;
};

