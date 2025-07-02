// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************

-------------------------------------------------------------------------
История:
- 15:05:2009   Created by Federico Rebora

*************************************************************************/

#include <drx3D/Game/Stdafx.h>

#include <drx3D/Game/ColorGradientUpr.h>

#include <drx3D/Game/EngineFacade/PluggableEngineFacade.h>

#include <drx3D/Game/Testing/TestUtilities.h>

#include <drx3D/Game/Testing/MockColorGradingController.h>

#include <drx3D/CoreX/Renderer/IColorGradingController.h>

DRX_TEST_FIXTURE(ColorGradientUpr_TestFixture, DrxUnit::ITestFixture, GameTesting::MainTestSuite)
{
private:
	class CMock3DEngine : public EngineFacade::CDummyEngine3DEngine
	{
	public:
		CMock3DEngine(EngineFacade::IEngineColorGradingController& colorGradingController)
		: m_colorGradingController(colorGradingController)
		{

		}

		virtual EngineFacade::IEngineColorGradingController& GetEngineColorGradingController()
		{
			return m_colorGradingController;
		}

	private:
		EngineFacade::IEngineColorGradingController& m_colorGradingController;
	};

public:
	ColorGradientUpr_TestFixture()
	: m_mock3DEngine(m_mockColorGradingController)
	, m_colorGradientUpr(m_engine)
	{
	}

	void SetUp()
	{
		m_engine.Use(m_mock3DEngine);
	}

protected:
	GameTesting::CMockEngineColorGradingController m_mockColorGradingController;
	CMock3DEngine m_mock3DEngine;
	EngineFacade::CDummyPluggableEngineFacade m_engine;
	Graphics::CColorGradientUpr m_colorGradientUpr;
};



DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestTriggeredFadingColorGradientLoadsOnUpdate, ColorGradientUpr_TestFixture)
{
	tukk const testPath = "testPath";

	m_colorGradientUpr.TriggerFadingColorGradient(testPath, 0.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_ARE_EQUAL(testPath, m_mockColorGradingController.GetPathOfLastLoadedColorChart());
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestNothingIsLoadedIfNothingWasTriggered, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_IS_FALSE(m_mockColorGradingController.AnyChartsWereLoaded());
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestMoreGradientsCanBeTriggeredInOneFrame, ColorGradientUpr_TestFixture)
{
	tukk const testPath1 = "testPath1";
	m_colorGradientUpr.TriggerFadingColorGradient(testPath1, 0.0f);

	tukk const testPath2 = "testPath2";
	m_colorGradientUpr.TriggerFadingColorGradient(testPath2, 0.0f);

	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_ARE_EQUAL(testPath1, m_mockColorGradingController.GetPathOfLoadedColorChart(0));
	ASSERT_ARE_EQUAL(testPath2, m_mockColorGradingController.GetPathOfLoadedColorChart(1));
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestGradientsAreLoadedOnlyOnce, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 0.0f);

	m_colorGradientUpr.UpdateForThisFrame(0.0f);
	m_mockColorGradingController.ClearLoadedCharts();
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_IS_FALSE(m_mockColorGradingController.AnyChartsWereLoaded());
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestGradientWithZeroFadingTimeIsUsedImmediatelyWithFullWeight, ColorGradientUpr_TestFixture)
{
	i32k expectedTexID = 64;
	m_mockColorGradingController.SetFakeIDForLoadedTexture(expectedTexID);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 0.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	const SColorChartLayer& theLayerSet = m_mockColorGradingController.GetCurrentlySetLayerByIndex(0);
	
	const SColorChartLayer expectedColorChartLayer(expectedTexID, 1.0f);

	ASSERT_ARE_EQUAL(expectedColorChartLayer, theLayerSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestBlendWeightHalfWayThroughFadingIn, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);

	const SColorChartLayer& theLayerSet = m_mockColorGradingController.GetCurrentlySetLayerByIndex(0);

	const SColorChartLayer expectedColorChartLayer(0, 0.5f);

	ASSERT_ARE_EQUAL(expectedColorChartLayer, theLayerSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestBlendWeightIsOneWhenFadingIsFinished, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 5.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);

	const SColorChartLayer& theLayerSet = m_mockColorGradingController.GetCurrentlySetLayerByIndex(0);

	const SColorChartLayer expectedColorChartLayer(0, 1.0f);

	ASSERT_ARE_EQUAL(expectedColorChartLayer, theLayerSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestBlendWeightIsOneAfterFadingIsFinished, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 5.0f);
	m_colorGradientUpr.UpdateForThisFrame(10.0f);

	const SColorChartLayer& theLayerSet = m_mockColorGradingController.GetCurrentlySetLayerByIndex(0);

	const SColorChartLayer expectedColorChartLayer(0, 1.0f);

	ASSERT_ARE_EQUAL(expectedColorChartLayer, theLayerSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestNullLayersAreNotSet, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_IS_FALSE(m_mockColorGradingController.WasNullPointerSetOnLayers())
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestBlendWeightIsOneAfterFadingIsFinishedInMoreUpdates, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 10.0f);

	m_colorGradientUpr.UpdateForThisFrame(2.0f);
	m_colorGradientUpr.UpdateForThisFrame(2.0f);
	m_colorGradientUpr.UpdateForThisFrame(2.0f);
	m_colorGradientUpr.UpdateForThisFrame(2.0f);
	m_colorGradientUpr.UpdateForThisFrame(2.0f);

	const SColorChartLayer& theLayerSet = m_mockColorGradingController.GetCurrentlySetLayerByIndex(0);

	const SColorChartLayer expectedColorChartLayer(0, 1.0f);

	ASSERT_ARE_EQUAL(expectedColorChartLayer, theLayerSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestTwoLayersSet, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath1", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(10.0f);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath2", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);

	u32k numberOfLayersSet = (i32) m_mockColorGradingController.GetNumberOfLayersSet();
	ASSERT_ARE_EQUAL(2u, numberOfLayersSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestLayerIntroducedWhileAnotherOneWasFadingIn, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath1", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(10.0f);

	i32k expectedTexID = 64;
	m_mockColorGradingController.SetFakeIDForLoadedTexture(expectedTexID);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath2", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);
	
	const SColorChartLayer expectedFirstColorChartLayer(0, 0.5f);
	ASSERT_ARE_EQUAL(expectedFirstColorChartLayer,  m_mockColorGradingController.GetCurrentlySetLayerByIndex(0));

	const SColorChartLayer expectedSecondColorChartLayer(expectedTexID, 0.5f);
	ASSERT_ARE_EQUAL(expectedSecondColorChartLayer,  m_mockColorGradingController.GetCurrentlySetLayerByIndex(1));
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestNewFadedChartStopsUpdatingPreviousOnes, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);

	m_colorGradientUpr.TriggerFadingColorGradient("", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);

	m_colorGradientUpr.TriggerFadingColorGradient("", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(5.0f);

	const SColorChartLayer& firstLayerSet = m_mockColorGradingController.GetCurrentlySetLayerByIndex(0);

	ASSERT_FLOAT_ARE_EQUAL(0.125f, firstLayerSet.m_blendAmount, 0.001f);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestLayerWithBlendWeightOneSetsOnlyOneLayer, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath1", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath2", 0.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	u32k numberOfLayersSet = (i32) m_mockColorGradingController.GetNumberOfLayersSet();
	ASSERT_ARE_EQUAL(1u, numberOfLayersSet);
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestLayerWithBlendWeightOneRemovesOtherLayersAndIsSetCorrectly, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath1", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	i32k expectedTexID = 64;
	m_mockColorGradingController.SetFakeIDForLoadedTexture(expectedTexID);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath2", 0.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	const SColorChartLayer expectedColorChartLayer(expectedTexID, 1.0f);
	ASSERT_ARE_EQUAL(expectedColorChartLayer, m_mockColorGradingController.GetCurrentlySetLayerByIndex(0));
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestRemovedLayerUnloadsTexture, ColorGradientUpr_TestFixture)
{
	i32k expectedTexID = 64;
	m_mockColorGradingController.SetFakeIDForLoadedTexture(expectedTexID);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath1", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	m_colorGradientUpr.TriggerFadingColorGradient("testPath2", 0.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_ARE_EQUAL(expectedTexID, m_mockColorGradingController.GetLastUnloadedTextureID());
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestTextureIsNotUnloadedIfInUse, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(10.0f);

	ASSERT_ARE_EQUAL(-1, m_mockColorGradingController.GetLastUnloadedTextureID());
}

DRX_TEST_WITH_FIXTURE(ColorGradientUpr_TestTextureIsNotUnloadedIfDeltaIsZero, ColorGradientUpr_TestFixture)
{
	m_colorGradientUpr.TriggerFadingColorGradient("testPath", 10.0f);
	m_colorGradientUpr.UpdateForThisFrame(0.0f);

	ASSERT_ARE_EQUAL(-1, m_mockColorGradingController.GetLastUnloadedTextureID());
}
