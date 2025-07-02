// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include <drx3D/CoreX/Platform/platform_impl.inl>
#include "IPlugin.h"
#include "EquipPack/EquipPackDialog.h"
#include "EquipPack/EquipPackLib.h"

//DECLARE_PYTHON_MODULE(gamesdk);

class EditorGameSDK : public IPlugin
{
public:

	EditorGameSDK()
	{
		GetIEditor()->RegisterDeprecatedPropertyEditor(ePropertyEquip, 
			std::function<bool(const string&, string&)>([](const string& oldValue, string& newValueOut)->bool
		{
			CEquipPackDialog dlg;
			dlg.SetCurrEquipPack(oldValue.GetString());
			if (dlg.DoModal() == IDOK)
			{
				newValueOut = dlg.GetCurrEquipPack().GetString();
				return true;
			}
			return false;
		}));

		CEquipPackLib::GetRootEquipPack().LoadLibs(true);
	}

	i32       GetPluginVersion() override { return 1; }
	tukk GetPluginName() override    { return "EditorGameSDK"; }
	tukk GetPluginDescription() override { return "Game SDK specific editor extensions"; }
};

REGISTER_PLUGIN(EditorGameSDK);
