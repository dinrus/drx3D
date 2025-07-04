// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>

class QLabel;
class QBoxLayout;
class QSlider;
class QDoubleSpinBox;
class QBoxLayout;
class QViewport;

struct SRenderContext;
#include <drx3D/CoreX/Serialization/Forward.h>

namespace CharacterTool
{

class CharacterDocument;

class BlendSpacePreview : public QWidget
{
	Q_OBJECT
public:
	BlendSpacePreview(QWidget* parent, CharacterDocument* document);

	void       IdleUpdate();
	void       Serialize(Serialization::IArchive& ar);
	QViewport* GetViewport() const { return m_viewport; }
protected slots:

	void OnRender(const SRenderContext& context);
	void OnResetView();
private:

	QBoxLayout*        m_layout;
	CharacterDocument* m_document;
	QAction*           m_actionShowGrid;
	QSlider*           m_characterScaleSlider;
	QViewport*         m_viewport;
};

}

