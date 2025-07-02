// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <drx3D/CoreX/Serialization/Forward.h>

class QString;
class QByteArray;
class QColor;

//Wrappers for convenience to use Qt classes directly with serialization
bool EDITOR_COMMON_API Serialize(Serialization::IArchive& ar, QString& value, tukk name, tukk label);
bool EDITOR_COMMON_API Serialize(Serialization::IArchive& ar, QByteArray& byteArray, tukk name, tukk label);
bool EDITOR_COMMON_API Serialize(Serialization::IArchive& ar, QColor& color, tukk name, tukk label);
