// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "QTrackingTooltip.h"

#include <QVBoxLayout>

class CPreviewToolTip : public QTrackingTooltip
{
	Q_OBJECT
public:
	static bool ShowTrackingToolTip(tukk szAssetPath, QWidget* parent = nullptr);

	const QString& GetAssetPath() const { return m_assetPath; }

private:
	CPreviewToolTip(tukk szAssetPath, QWidget* pParent = nullptr);
	void SetPreviewWidget(QWidget* pPreview);

private:
	QString m_assetPath;
	QVBoxLayout* m_pLayout;
};

