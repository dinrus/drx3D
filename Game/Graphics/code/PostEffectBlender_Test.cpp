// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/Stdafx.h>
#include <drx3D/Game/PostEffectBlender.h>
#include <drx3D/Game/Testing/GameTesting.h>

#include <drx3D/Game/EngineFacade/PluggableEngineFacade.h>
#include <drx3D/Game/EngineFacade/3DEngine.h>

using namespace GameTesting;
using namespace EngineFacade;
using namespace Graphics;

DRX_TEST_FIXTURE(PostEffectBlender_TestFixture, Actor2BaseTestFixture, Actor2TestSuite)
{
public:
	virtual void SetUp()
	{
		m_engine.Use(m_test3DEngine);
		m_effectBlender.reset(new CPostEffectBlender(m_engine));
	}

protected:

	class CTest3DEngine : public CDummyEngine3DEngine
	{
	public:
		
		CTest3DEngine()
			: m_getPostEffectParameterCalled(false)
			, m_postEffectsReseted(false)
			, m_paramValue(0.0f)
		{

		}

		virtual float GetPostEffectParameter(tukk const parameterName) const
		{
			m_getPostEffectParameterCalled = true;
			return 0.0f;
		}

		virtual void SetPostProcessEffectParameter(tukk const parameterName, const float value)
		{
			m_postEffect = parameterName;
			m_paramValue = value;
		}

		virtual void ResetPostEffects()
		{
			m_postEffectsReseted = true;
		}

		bool WasGetPostEffectParameterCalled() const
		{
			return m_getPostEffectParameterCalled;
		}

		bool WasPostEffectValueUpdated(tukk postEffect) const
		{
			return (strcmp(postEffect, m_postEffect.c_str()) == 0);
		}

		float GetParamValue() const
		{
			return m_paramValue;
		}

		bool WerePostEffectReseted() const
		{
			return m_postEffectsReseted;
		}

	private:

		mutable bool m_getPostEffectParameterCalled;
		string m_postEffect;
		float m_paramValue;
		bool m_postEffectsReseted;
	};

	void AddBlendedEffect(const SBlendEffectParams& params)
	{
		m_effectBlender->AddBlendedEffect(params);
	}

	void Update()
	{
		m_effectBlender->Update(0.033f);
	}

	i32 GetRunningBlends() const
	{
		return m_effectBlender->GetRunningBlendsCount();
	}

	void Reset() 
	{
		m_effectBlender->Reset();
	}

	bool WasPostEffectValueUpdated(tukk postEffect) const
	{
		return m_test3DEngine.WasPostEffectValueUpdated(postEffect);
	}

	bool WasInitialBlendValueSet() const
	{
		return m_test3DEngine.WasGetPostEffectParameterCalled();
	}

	bool IsPostEffectValue(float value) const
	{
		return (m_test3DEngine.GetParamValue() == value);
	}

	bool WerePostEffectReseted() const
	{
		return m_test3DEngine.WerePostEffectReseted();
	}

protected:

	CDummyPluggableEngineFacade m_engine;
	CTest3DEngine m_test3DEngine;
	shared_ptr<CPostEffectBlender> m_effectBlender;
};

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_InitialConditions, PostEffectBlender_TestFixture)
{
	ASSERT_ARE_EQUAL(0, GetRunningBlends());
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_AddBlendedEffect, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	AddBlendedEffect(blendParams);

	ASSERT_ARE_EQUAL(1, GetRunningBlends());
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_GetsCurrentValueIfNotInitialValueProvided, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	AddBlendedEffect(blendParams);

	ASSERT_IS_TRUE(WasInitialBlendValueSet());
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_UpdatesPostEffectParameterValues, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	blendParams.m_postEffectName = "postEffect_1";
	AddBlendedEffect(blendParams);

	Update();

	ASSERT_IS_TRUE(WasPostEffectValueUpdated("postEffect_1"));
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_FinishesBlend, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	blendParams.m_blendTime = 0.2f;
	AddBlendedEffect(blendParams);

	for (i32 i = 0; i < 15; ++i)
	{
		Update();
	}

	ASSERT_ARE_EQUAL(0, GetRunningBlends());
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_FinishesBlend_AndSetsFinalValue, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	blendParams.m_initialValue = 0.8f;
	blendParams.m_endValue = 0.2f;
	blendParams.m_blendTime = 0.2f;
	AddBlendedEffect(blendParams);

	for (i32 i = 0; i < 15; ++i)
	{
		Update();
	}

	ASSERT_IS_TRUE(IsPostEffectValue(blendParams.m_endValue));
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_Reset, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	AddBlendedEffect(blendParams);

	Update();
	Reset();

	ASSERT_IS_TRUE(WerePostEffectReseted());
}

DRX_TEST_WITH_FIXTURE(Test_PostEffectBlender_NoRunningEffectsAfterReset, PostEffectBlender_TestFixture)
{
	SBlendEffectParams blendParams;
	AddBlendedEffect(blendParams);

	Update();
	Reset();

	ASSERT_ARE_EQUAL(0, GetRunningBlends());
}


