// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxSchematyc/Utils/IString.h>

namespace DrxSchematycEditor {
namespace Utils {

void     ConstructAbsolutePath(Schematyc::IString& output, tukk szFileName);

bool     WriteToClipboard(tukk szText, tukk szPrefix = nullptr);
bool     ReadFromClipboard(string& text, tukk szPrefix = nullptr);
bool     ValidateClipboardContents(tukk szPrefix);

}
}

