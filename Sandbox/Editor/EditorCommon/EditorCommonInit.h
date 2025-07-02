// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <functional>

struct IEditor;
struct ISystem;

class QWidget;
class QString;

namespace EditorCommon
{

//! Preferably be called after IEditor is fully initialized
void EDITOR_COMMON_API                        SetIEditor(IEditor* editor);
void EDITOR_COMMON_API                        Initialize();
void EDITOR_COMMON_API                        Deinitialize();
}

