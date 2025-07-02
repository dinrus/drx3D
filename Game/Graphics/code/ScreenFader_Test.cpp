// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 3:06:2009   Created by Benito G.R.
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Graphics/ScreenFader.h>

#include <drx3D/Game/EngineFacade/PluggableEngineFacade.h>

using namespace GameTesting;
using namespace Graphics;
using namespace EngineFacade;

DRX_TEST_FIXTURE(ScreenFader_TestFixture, Actor2BaseTestFixture, Actor2TestSuite)
{
public:
	virtual void SetUp()
	{
		m_engine.Use(m_engineRenderer);
		m_screenFader.reset(new CScreenFader(m_engine));
	}

protected:

	class CTestTexture : public CDummyEngineTexture
	{
	public:
		CTestTexture(const string& textureName)
		{
			m_name = textureName;
		}

		virtual tukk GetName() const
		{
			return m_name.c_str();
		}

	private:
		string m_name;
	};

	class CTestRenderer : public CDummyEngineRenderer
	{
	public:
		CTestRenderer()
			: m_loadTextureCallCount(0)
			, m_drawFullScreenCallCount(0)
		{

		}

		virtual IEngineTexture::Ptr LoadTexture(const string& texture, u32 flags, byte eTextureType)
		{
			m_loadedTexture = texture;
			m_loadTextureCallCount++;

			return IEngineTexture::Ptr(new CTestTexture(texture));
		}

		virtual void DrawFullScreenImage(i32 textureID, ColorF color)
		{
			m_drawFullScreenCallCount++;
		}

		bool WasTextureLoaded(const string& texture) const
		{
			return (m_loadedTexture == texture);
		}

		i32 GetLoadTextureCallCount() const
		{
			return m_loadTextureCallCount;
		}

		i32 GetDrawFullScreenCallCount() const
		{
			return m_drawFullScreenCallCount;
		}

	private:

		string m_loadedTexture;
		i32 m_loadTextureCallCount;
		i32 m_drawFullScreenCallCount;
	};

	EngineFacade::CDummyPluggableEngineFacade m_engine;
	CTestRenderer m_engineRenderer;
	shared_ptr<CScreenFader> m_screenFader;
};

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_InitialState, ScreenFader_TestFixture)
{
	ASSERT_IS_FALSE(m_screenFader->IsFadingIn());
	ASSERT_IS_FALSE(m_screenFader->IsFadingOut());
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_IsFadingIn, ScreenFader_TestFixture)
{
	m_screenFader->FadeIn("", Col_Black, 2.0f, true);

	ASSERT_IS_TRUE(m_screenFader->IsFadingIn());
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_IsFadingOut, ScreenFader_TestFixture)
{
	m_screenFader->FadeOut("", Col_Black, 2.0f, true);

	ASSERT_IS_TRUE(m_screenFader->IsFadingOut());
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_LoadsTextureIfProvided_FadeIn, ScreenFader_TestFixture)
{
	m_screenFader->FadeIn("dummyPath/Cool_FadeIn_Texture.dds", Col_Black, 2.0f, true);

	ASSERT_IS_TRUE(m_engineRenderer.WasTextureLoaded("dummyPath/Cool_FadeIn_Texture.dds"));
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_LoadsTextureIfProvided_FadeOut, ScreenFader_TestFixture)
{
	m_screenFader->FadeOut("dummyPath/Cool_FadeOut_Texture.dds", Col_Black, 2.0f, true);

	ASSERT_IS_TRUE(m_engineRenderer.WasTextureLoaded("dummyPath/Cool_FadeOut_Texture.dds"));
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_DoesNotLoadSameTextureSeveralTimes, ScreenFader_TestFixture)
{
	m_screenFader->FadeIn("dummyPath/Cool_FadeIn_Texture.dds", Col_Black, 2.0f, true);
	m_screenFader->FadeOut("dummyPath/Cool_FadeIn_Texture.dds", Col_Black, 2.0f, true);
	m_screenFader->FadeIn("dummyPath/Cool_FadeIn_Texture.dds", Col_Black, 3.0f, true);

	ASSERT_ARE_EQUAL(1, m_engineRenderer.GetLoadTextureCallCount());
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_LoadsDifferentTextureIfNeeded, ScreenFader_TestFixture)
{
	m_screenFader->FadeIn("dummyPath/Cool_FadeIn_Texture.dds", Col_Black, 2.0f, true);
	m_screenFader->FadeOut("dummyPath/Cool_FadeOut_Texture.dds", Col_Black, 2.0f, true);
	m_screenFader->FadeIn("dummyPath/Cool_FadeOut_Texture.dds", Col_Black, 3.0f, true);

	ASSERT_ARE_EQUAL(2, m_engineRenderer.GetLoadTextureCallCount());
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_OnUpdateRendersFullScreenQuad, ScreenFader_TestFixture)
{
	m_screenFader->FadeIn("dummyPath/Cool_FadeIn_Texture.dds", Col_Black, 2.0f, true);
	m_screenFader->Update(0.033f);

	ASSERT_ARE_EQUAL(1, m_engineRenderer.GetDrawFullScreenCallCount());
}

DRX_TEST_WITH_FIXTURE(Test_ScreenFader_DoesNotRenderIfNotFading, ScreenFader_TestFixture)
{
	m_screenFader->Update(0.033f);

	ASSERT_ARE_EQUAL(0, m_engineRenderer.GetDrawFullScreenCallCount());
}



