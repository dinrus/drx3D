// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef HOTUPDATE_H
#define HOTUPDATE_H

class CHotUpdateNotification :
	public INotificationNetworkListener
{
public:
	static CHotUpdateNotification& Instance()
	{
		static CHotUpdateNotification instance;
		return instance;
	}

	// INotificationNetworkListener
public:
	virtual void OnNotificationNetworkReceive(ukk pBuffer, size_t length)
	{
		if (!length)
			return;
		tukk file = (tukk)pBuffer;
		if (file[length - 1] != '\0')
			return;

		if (!gEnv->pSystem)
			return;

		gEnv->pLog->Log("HotUpdating file \"%s\"...", file);

		// TODO: Figure out what type of file we need to reload and call the
		// appropriate method. For now only textures are guaranteed to work.

		IRenderer* pRenderer = gEnv->pSystem->GetIRenderer();
		if (!pRenderer)
			return;

		ICVar* pPackPriority = gEnv->pConsole->GetCVar("sys_PakPriority");
		if (pPackPriority)
		{
			if (pPackPriority->GetIVal() != 0)
			{
				pPackPriority->Set(0);
			}
		}

		pRenderer->EF_ReloadFile(file);
	}
};

#endif // HOTUPDATE_H
