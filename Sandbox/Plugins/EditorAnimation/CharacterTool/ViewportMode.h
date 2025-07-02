// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "QViewportConsumer.h"
#include <vector>
#include <drx3D/CoreX/Serialization/Forward.h>

struct ICharacterInstance;
class QToolBar;
class QPropertyTree;

namespace CharacterTool
{

using std::vector;
class CharacterToolForm;
class CharacterDocument;
class TransformPanel;
struct System;

struct SModeContext
{
	System*                system;
	CharacterToolForm*     window;
	CharacterDocument*     document;
	ICharacterInstance*    character;
	TransformPanel*        transformPanel;
	QToolBar*              toolbar;
	vector<QPropertyTree*> layerPropertyTrees;
};

struct IViewportMode : public QViewportConsumer
{
	virtual ~IViewportMode() {}
	virtual void Serialize(Serialization::IArchive& ar)                         {}
	virtual void EnterMode(const SModeContext& context)                         {}
	virtual void LeaveMode()                                                    {}

	virtual void GetPropertySelection(vector<ukk>* selectedItems) const {}
	virtual void SetPropertySelection(const vector<ukk>& items)         {}
};

}

