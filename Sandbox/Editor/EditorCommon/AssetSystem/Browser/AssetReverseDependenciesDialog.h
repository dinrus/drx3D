// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Controls/EditorDialog.h>
#include <QVector>

class CAsset;
class QWidget;
class QAbstractItemModel;

//! A dialog that shows a list of assets that use the given assets.
class CAssetReverseDependenciesDialog : public CEditorDialog
{
public:
	CAssetReverseDependenciesDialog(
		const QVector<CAsset*>& assets, 
		const QString& assetsGroupTitle,
		const QString& dependentAssetsGroupTitle,
		const QString& dependentAssetsInfoText,
		const QString& question = QString(),
		QWidget* pParent = nullptr);
private:
	std::vector<std::unique_ptr<QAbstractItemModel>> m_models;
};

