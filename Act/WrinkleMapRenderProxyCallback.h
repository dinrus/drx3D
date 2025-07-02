// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __WrinkleMapRenderProxyCallback_H__
#define __WrinkleMapRenderProxyCallback_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Renderer/IShaderParamCallback.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

class CWrinkleMapShaderParamCallback : public IShaderParamCallback
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IShaderParamCallback)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CWrinkleMapShaderParamCallback, "WrinkleMapShaderParamCallback", "68c7f0e0-c364-46fe-82a3-bc01b54dc7bf"_drx_guid)

	CWrinkleMapShaderParamCallback();
	virtual ~CWrinkleMapShaderParamCallback();

public:

	//////////////////////////////////////////////////////////////////////////
	//	Implement IShaderParamCallback
	//////////////////////////////////////////////////////////////////////////

	virtual void SetCharacterInstance(ICharacterInstance* pCharInstance) override
	{
		m_pCharacterInstance = pCharInstance;
	}

	virtual ICharacterInstance* GetCharacterInstance() const override
	{
		return m_pCharacterInstance;
	}

	virtual bool Init() override;
	virtual void SetupShaderParams(IShaderPublicParams* pParams, IMaterial* pMaterial) override;
	virtual void Disable(IShaderPublicParams* pParams) override;

protected:

	void SetupBoneWrinkleMapInfo();

	//////////////////////////////////////////////////////////////////////////

	ICharacterInstance* m_pCharacterInstance;

	struct SWrinkleBoneInfo
	{
		i16 m_nChannelID;
		i16 m_nJointID;
	};
	typedef std::vector<SWrinkleBoneInfo> TWrinkleBoneInfo;
	TWrinkleBoneInfo m_WrinkleBoneInfo;

	IMaterial*       m_pCachedMaterial;

	u8            m_eSemantic[3];

	bool             m_bWrinklesEnabled;
};

class CWrinkleMapShaderParamCallbackUI : public CWrinkleMapShaderParamCallback
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(CWrinkleMapShaderParamCallback)
	DRXINTERFACE_END()
		
	DRXGENERATE_CLASS_GUID(CWrinkleMapShaderParamCallbackUI, "bWrinkleMap", "1b9d4692-5918-485b-b731-2c8fb3f5b763"_drx_guid)
};

#endif //__WrinkleMapRenderProxyCallback_H__
