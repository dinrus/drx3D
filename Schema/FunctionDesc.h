// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/GUID.h>

namespace sxema
{

class CCommonFunctionDesc
{
public:

private:

	DrxGUID       m_guid;
	tukk m_szName = nullptr;
};

template<typename TYPE, TYPE FUNCTION_PTR> class CFunctionDesc : public CCommonFunctionDesc
{
};

} // sxema