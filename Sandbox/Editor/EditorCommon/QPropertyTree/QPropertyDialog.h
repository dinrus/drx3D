// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <memory>

#include "Controls/EditorDialog.h"

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/yasli/Config.h>

namespace yasli{
class BinOArchive;
}

class QPropertyTree;
class QBoxLayout;

class EDITOR_COMMON_API QPropertyDialog : public CEditorDialog
{
	Q_OBJECT
public:
	static bool edit(Serialization::SStruct& ser, tukk title, tukk windowStateFilename, QWidget* parent);

	QPropertyDialog(QWidget* parent);
	~QPropertyDialog();

	void        setSerializer(const Serialization::SStruct& ser);
	void        setArchiveContext(Serialization::SContextLink* context);
	void        setWindowStateFilename(tukk windowStateFilename);
	void        setSizeHint(const QSize& sizeHint);
	void        setStoreContent(bool storeContent);

	void        revert();
	QBoxLayout* layout() { return m_layout; }

	void        Serialize(Serialization::IArchive& ar);
protected slots:
	void        onAccepted();
	void        onRejected();

protected:
	QSize sizeHint() const override;
	void  setVisible(bool visible) override;
private:
	QPropertyTree*                          m_propertyTree;
	QBoxLayout*                             m_layout;
	std::unique_ptr<Serialization::SStruct> m_serializer;
	std::unique_ptr<yasli::BinOArchive>     m_backup;
	yasli::string                           m_windowStateFilename;
	QSize m_sizeHint;
	bool  m_storeContent;
};

