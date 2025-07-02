// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QTextBrowser>
#include <QWidget>

class QBoxLayout;

namespace Schematyc
{
	class CReportWidget : public QWidget
	{
		Q_OBJECT

	public:

		CReportWidget(QWidget* pParent);

		void Write(tukk szText);
		void WriteUri(tukk szUri, tukk szText);

	public slots:

		void OnLinkClicked(const QUrl& link);

	private:

		QBoxLayout*   m_pMainLayout;
		QTextBrowser* m_pOutput;
	};
}

