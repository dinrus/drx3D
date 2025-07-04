// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

//Qt
#include <QWidget>
#include "QAdvancedTreeView.h"

class QDragEnterEvent;
class QDrag;
class QAdvancedTreeView;
class QAbstractButton;
class QPreviewWidget;
class QItemSelectionModel;
class QToolButton;

class CObjectBrowser : public QWidget
{
	Q_OBJECT

public:
	//Supports "Path/to/file/*.ext";"*.ext2";"*.ext3" syntax
	CObjectBrowser(tukk filters);

	//Supports same syntax as a string list
	CObjectBrowser(const QStringList& filters);

	~CObjectBrowser();

	CDrxSignal<void(tukk)>              signalOnDoubleClickFile;
	CDrxSignal<void(QDragEnterEvent*, QDrag*)> signalOnDragStarted;
	CDrxSignal<void()>                         singalOnDragEnded;

private:
	virtual void dragEnterEvent(QDragEnterEvent* event) override;

	static void  CorrectSyntax(const QStringList& filters, QStringList& extensions, QStringList& directories);
	void         Init(const QStringList& extensions, const QStringList& directories);
	void         UpdatePreviewWidget();

private:
	QAdvancedTreeView* m_treeView;
	QPreviewWidget*    m_previewWidget;
	QToolButton*       m_showPreviewButton;
	bool               m_bIsDragTracked;
	bool               m_bHasGeometryFileFilter;
	bool               m_bHasParticleFileFilter;
};

