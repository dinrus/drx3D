// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __Basemark_BenchmarkRendererSensor__
#define __Basemark_BenchmarkRendererSensor__
#pragma once

#ifdef ENABLE_BENCHMARK_SENSOR
	#include <drx3D/CoreX/Renderer/Tarray.h>
	#include <SimpleCommandQueue.h>
	#include <IBenchmarkRendererSensor.h>
	#include <BenchmarkResults.h>

class CD3D9Renderer;

namespace BenchmarkFramework
{
class IHardwareInfoProvider;
class HighPrecisionTimer;
class IRenderResultsObserver;
}

class BenchmarkRendererSensor : public BenchmarkFramework::IBenchmarkRendererSensor
{
public:
	BenchmarkRendererSensor(CD3D9Renderer* renderer);
	virtual ~BenchmarkRendererSensor();

	//virtual void setMatrixTextureResolution(i32 w, i32 h);

	//virtual void setMatrixDimensions(i32 rows, i32 columns);

	virtual void startTest() override;
	virtual void endTest() override;
	virtual void setRenderResultsObserver(BenchmarkFramework::IRenderResultsObserver* observer) override;
	virtual void afterSwapBuffers(CDrxDeviceWrapper& device, CDrxDeviceContextWrapper& context);
	virtual void update() override;
	virtual void PreStereoFrameSubmit(CTexture* left, CTexture* right) override;
	virtual void AfterStereoFrameSubmit() override;
	virtual void enableFlicker(bool enable, BenchmarkFramework::FlickerFunction func) override;

	virtual void setFrameSampling(i32 interval, i32 flags) override;
	virtual void overrideMonitorWindowSize(bool override, i32 width, i32 height, bool fullscreen);

	virtual void cleanupAfterTestRun();

protected:

	enum FrameMatrixCommand
	{
		StartTest,
		EndTest,
		//			SetMatrixTextureResolution,
		//			SetMatrixDimensions,
		SetFrameSampling,
		ForceWindowSize,
		SetupFlicker
	};

	enum FrameMatrixState
	{
		TestInactive,
		//			TestStarted,
		TestRunning,
		//			TestRunDone

	};
	typedef BenchmarkFramework::SimpleCommandQueue<FrameMatrixCommand, TArray<uint8_t>> CommandQueue;
	CommandQueue                                  m_commandQueue[RT_COMMAND_BUF_COUNT];
	TArray<BenchmarkFramework::RenderFrameSample> m_samples;
	BenchmarkFramework::HighPrecisionTimer*       m_timer;
	BenchmarkFramework::TimeStamp                 m_startedTime;
	BenchmarkFramework::RenderFrameSample         m_currentFrameSample;

	FrameMatrixState                              m_currentState;
	CD3D9Renderer*                                m_renderer;
	BenchmarkFramework::IRenderResultsObserver*   m_resultObserver;
	CDrxNameR m_shaderUniformName;
	BenchmarkFramework::FlickerFunction           m_flickerFunc;
	/*		i32 m_matrixTexHandle;
	    i32 m_matrixTexWidth;
	    i32 m_matrixTexHeight;
	    i32 m_rows;
	    i32 m_columns;
	    i32 m_currentIndex;
	 */
	uint32_t m_currentFrameCount;
	uint32_t m_frameFlickerID;

	i32      m_frameSampleInterval;
	i32      m_frameSampleFlags;

	CFullscreenPass m_copyPass;
	CStretchRectPass m_strechRectPass;

	//		bool m_matrixTexNeedsRefresh;
	//		bool m_matrixTexNeedsClear;
	void         saveFrameSample();

	void         submitResults();

	virtual void initializeTest();
	//		virtual void recreateMatrixTexture();
	//		virtual void clearMatrixTexture();

	virtual bool shouldPresentFrame();

	virtual void processCommands();

	//		void copyFrameToMatrix(CTexture* texture);
	//		void copyStereoFrameToMatrix(CTexture* left, CTexture* right);
	void        copyFrameToScreen(CTexture* texture);
	void        copyStereoFrameToScreen(CTexture* left, CTexture* right);
	void        accumulateFrameCount();

	void        applyFlicker(CTexture* left, CTexture* right, bool leftWhite, bool rightWhite);
	void        blendTextureWithColor(CTexture* tex, float r, float g, float b, float a);

	static void forceFlush(CDrxDeviceWrapper& device, CDrxDeviceContextWrapper& context);
	bool        isActive();
};

#endif //ENABLE_BENCHMARK_SENSOR

#endif
