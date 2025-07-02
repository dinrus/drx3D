// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAdvancedTreeView.h>

class CTextureModel;
class CComboBoxDelegate;

class CTextureView : public QAdvancedTreeView
{
public:
	CTextureView(QWidget* pParent = nullptr);

	void SetModel(CTextureModel* pTextureModel);
private:
	void CreateContextMenu(const QPoint& point);

	CTextureModel*                     m_pTextureModel;
	std::unique_ptr<CComboBoxDelegate> m_pComboBoxDelegate;
};
