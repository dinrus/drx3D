// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QTextBrowser>
#include <QWidget>
#include <DrxSchematyc/Services/LogStreamName.h>
#include <DrxSchematyc/SerializationUtils/SerializationUtils.h>
#include <DrxSchematyc/Utils/ScopedConnection.h>

class QBoxLayout;
class QLineEdit;
class QPlainTextEdit;
class QAdvancedPropertyTree;
class QPushButton;
class QSplitter;

namespace Schematyc {

typedef std::vector<SLogStreamName> LogStreams;

struct SLogSettings
{
	SLogSettings();

	void Serialize(Serialization::IArchive& archive);

	string     entity;
	LogStreams streams;
	bool       bShowComments;
	bool       bShowWarnings;
	bool       bShowErrors;
	bool       bShowEntity;
	bool       bShowOrigin;
};

class CLogSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	CLogSettingsWidget(SLogSettings& settings);

	const SLogSettings& GetSettings() const;

	//protected Q_SLOTS:
	//	void OnAttachToSelectedEntityButtonClicked();

protected:
	void showEvent(QShowEvent* pEvent);

private:
	SLogSettings&          m_settings;
	QBoxLayout*            m_pLayout;
	QPushButton*           m_pAttachToSelectedEntityButton;
	QAdvancedPropertyTree* m_pPropertyTree;
};

class CLogWidget : public QWidget
{
	Q_OBJECT

	typedef std::vector<QString> PendingMessages;

public:
	CLogWidget(const SLogSettings& settings);

	void InitLayout();
	void Clear();

protected:
	void timerEvent(QTimerEvent* pTimerEvent);

	void OnLinkClicked(const QUrl& link);

private:
	EVisitStatus VisitRecordedLogMessage(const SLogMessageData& logMessageData);
	void         OnLogMessage(const SLogMessageData& logMessageData);
	void         WriteLogMessageToOutput(const SLogMessageData& logMessageData);

private:

	QBoxLayout*         m_pMainLayout;
	const SLogSettings& m_settings;
	QTextBrowser*       m_pOutput;

	PendingMessages     m_pendingMessages;
	CConnectionScope    m_connectionScope;
};
} // Schematyc

