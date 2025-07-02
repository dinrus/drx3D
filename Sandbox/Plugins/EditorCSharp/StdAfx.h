// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#include <drx3D/CoreX/Platform/platform.h>

//TODO : remove this, but there are many compilation errors. The particle editor should not include MFC.
#define DRX_USE_MFC
#include <drx3D/CoreX/Platform/DrxAtlMfc.h>

#include "EditorCommon.h"
#include "IEditor.h"
#include "IPlugin.h"
#include <Drx3DEngine/I3DEngine.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include "Expected.h"

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QStyleOption>
#include <QMenu>
#include <QVariant>
#include <QTextCursor>
#include <QDrag>
#include <QMimeData>
#include <QClipboard>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushbutton>
#include <QToolButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QSplitter>
#include <QFileDialog>

#include "QtUtil.h"
#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>
#include "Serialization.h"
#include "UndoStack.h"

#include <Serialization/QPropertyTree/QPropertyTree.h>
#include "IResourceSelectorHost.h"

IEditor*               GetIEditor();
