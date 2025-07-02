// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Wrapper for QViewport usable from Qt Creator's UI Designer
#include <QViewPort.h>

class QViewportWrapper : public QViewport
{
public:
	QViewportWrapper(QWidget* pParent)
		: QViewport(gEnv, pParent)
	{
		SetOrbitMode(true);
	}
};

