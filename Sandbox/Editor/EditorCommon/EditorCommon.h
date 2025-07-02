// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//Including most commonly used Qt headers to improve compilation times
#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QString>
#include <QTreeView>
#include <QListView>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QFileInfo>

#include <drx3D/CoreX/Math/Drx_Math.h>
#include "IEditor.h"
#include "PluginAPI.h"
/*
#include "QAdvancedTreeView.h"
#include "QAdvancedItemDelegate.h"
*/
#include "QControls.h"
#include "Controls/QMenuComboBox.h"
#include "QSearchBox.h"
#include "Controls/QuestionDialog.h"
#include "DrxIcon.h"
#include "Util/EditorUtils.h"

//Serialization
#include "IResourceSelectorHost.h"

//Asset system includes
#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetType.h"
#include "AssetSystem/AssetManager.h"
