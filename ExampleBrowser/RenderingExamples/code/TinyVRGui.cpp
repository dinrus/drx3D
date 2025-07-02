#include "../TinyVRGui.h"
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include "../../GwenGUISupport/GraphingTexture.h"
#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include "../TimeSeriesCanvas.h"
#include "../TimeSeriesFontData.h"
#include <drx3D/Importers/MeshUtility/b3ImportMeshUtility.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

struct TestCanvasInterface2 : public Common2dCanvasInterface
{
	b3AlignedObjectArray<u8>& m_texelsRGB;
	i32 m_width;
	i32 m_height;

	TestCanvasInterface2(b3AlignedObjectArray<u8>& texelsRGB, i32 width, i32 height)
		: m_texelsRGB(texelsRGB),
		  m_width(width),
		  m_height(height)
	{
	}

	virtual ~TestCanvasInterface2()
	{
	}
	virtual i32 createCanvas(tukk canvasName, i32 width, i32 height, i32 posX, i32 posY)
	{
		return 0;
	}
	virtual void destroyCanvas(i32 canvasId)
	{
	}
	virtual void setPixel(i32 canvasId, i32 x, i32 y, u8 red, u8 green, u8 blue, u8 alpha)
	{
		if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		{
			m_texelsRGB[(x + y * m_width) * 3 + 0] = red;
			m_texelsRGB[(x + y * m_width) * 3 + 1] = green;
			m_texelsRGB[(x + y * m_width) * 3 + 2] = blue;
		}
	}
	virtual void getPixel(i32 canvasId, i32 x, i32 y, u8& red, u8& green, u8& blue, u8& alpha)
	{
		if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		{
			red = m_texelsRGB[(x + y * m_width) * 3 + 0];
			green = m_texelsRGB[(x + y * m_width) * 3 + 1];
			blue = m_texelsRGB[(x + y * m_width) * 3 + 2];
		}
	}

	virtual void refreshImageData(i32 canvasId)
	{
	}
};

struct TinyVRGuiInternalData
{
	CommonRenderInterface* m_renderer;

	b3AlignedObjectArray<u8> m_texelsRGB;
	TestCanvasInterface2* m_testCanvas;
	TimeSeriesCanvas* m_timeSeries;
	i32 m_src;
	i32 m_textureId;
	i32 m_gfxObjectId;

	TinyVRGuiInternalData()
		: m_renderer(0),
		  m_testCanvas(0),
		  m_timeSeries(0),
		  m_src(-1),
		  m_textureId(-1),
		  m_gfxObjectId(-1)
	{
	}
};

TinyVRGui::TinyVRGui(struct ComboBoxParams& params, struct CommonRenderInterface* renderer)
{
	m_data = new TinyVRGuiInternalData;
	m_data->m_renderer = renderer;
}

TinyVRGui::~TinyVRGui()
{
	delete m_data->m_timeSeries;
	delete m_data->m_testCanvas;
	delete m_data;
}

bool TinyVRGui::init()
{
	{
		i32 width = 256;
		i32 height = 256;
		m_data->m_texelsRGB.resize(width * height * 3);
		for (i32 i = 0; i < width; i++)
			for (i32 j = 0; j < height; j++)
			{
				m_data->m_texelsRGB[(i + j * width) * 3 + 0] = 155;
				m_data->m_texelsRGB[(i + j * width) * 3 + 1] = 155;
				m_data->m_texelsRGB[(i + j * width) * 3 + 2] = 255;
			}

		m_data->m_testCanvas = new TestCanvasInterface2(m_data->m_texelsRGB, width, height);
		m_data->m_timeSeries = new TimeSeriesCanvas(m_data->m_testCanvas, width, height, "time series");

		bool clearCanvas = false;

		m_data->m_timeSeries->setupTimeSeries(3, 100, 0, clearCanvas);
		m_data->m_timeSeries->addDataSource("Some sine wave", 255, 0, 0);
		m_data->m_timeSeries->addDataSource("Some cosine wave", 0, 255, 0);
		m_data->m_timeSeries->addDataSource("Delta Time (*10)", 0, 0, 255);
		m_data->m_timeSeries->addDataSource("Tan", 255, 0, 255);
		m_data->m_timeSeries->addDataSource("Some cosine wave2", 255, 255, 0);
		m_data->m_timeSeries->addDataSource("Empty source2", 255, 0, 255);

		m_data->m_textureId = m_data->m_renderer->registerTexture(&m_data->m_texelsRGB[0], width, height);

		{
			tukk fileName = "cube.obj";  //"textured_sphere_smooth.obj";
			//fileName = "cube.obj";

			i32 shapeId = -1;

			b3ImportMeshData meshData;
			DefaultFileIO fileIO;
			if (b3ImportMeshUtility::loadAndRegisterMeshFromFileInternal(fileName, meshData,&fileIO))
			{
				shapeId = m_data->m_renderer->registerShape(&meshData.m_gfxShape->m_vertices->at(0).xyzw[0],
															meshData.m_gfxShape->m_numvertices,
															&meshData.m_gfxShape->m_indices->at(0),
															meshData.m_gfxShape->m_numIndices,
															D3_GL_TRIANGLES,
															m_data->m_textureId);

				float position[4] = {0, 0, 2, 1};
				float orn[4] = {0, 0, 0, 1};
				float color[4] = {1, 1, 1, 1};
				float scaling[4] = {.1, .1, .1, 1};

				m_data->m_gfxObjectId = m_data->m_renderer->registerGraphicsInstance(shapeId, position, orn, color, scaling);
				m_data->m_renderer->writeTransforms();

				meshData.m_gfxShape->m_scaling[0] = scaling[0];
				meshData.m_gfxShape->m_scaling[1] = scaling[1];
				meshData.m_gfxShape->m_scaling[2] = scaling[2];

				delete meshData.m_gfxShape;
				if (!meshData.m_isCached)
				{
					free(meshData.m_textureImage1);
				}
			}
		}
	}

	m_data->m_renderer->writeTransforms();
	return true;
}

void TinyVRGui::tick(b3Scalar deltaTime, const b3Transform& guiWorldTransform)
{
	float time = m_data->m_timeSeries->getCurrentTime();
	float v = sinf(time);
	m_data->m_timeSeries->insertDataAtCurrentTime(v, 0, true);
	v = cosf(time);
	m_data->m_timeSeries->insertDataAtCurrentTime(v, 1, true);
	v = tanf(time);
	m_data->m_timeSeries->insertDataAtCurrentTime(v, 3, true);
	m_data->m_timeSeries->insertDataAtCurrentTime(deltaTime * 10, 2, true);

	m_data->m_timeSeries->nextTick();

	m_data->m_renderer->updateTexture(m_data->m_textureId, &m_data->m_texelsRGB[0]);
	m_data->m_renderer->writeSingleInstanceTransformToCPU(guiWorldTransform.getOrigin(), guiWorldTransform.getRotation(), m_data->m_gfxObjectId);
	m_data->m_renderer->writeTransforms();
}

void TinyVRGui::clearTextArea()
{
	i32 width = 256;
	i32 height = 50;
	for (i32 i = 0; i < width; i++)
		for (i32 j = 0; j < height; j++)
		{
			m_data->m_texelsRGB[(i + j * width) * 3 + 0] = 155;
			m_data->m_texelsRGB[(i + j * width) * 3 + 1] = 155;
			m_data->m_texelsRGB[(i + j * width) * 3 + 2] = 255;
		}
}

void TinyVRGui::grapicalPrintf(tukk str, i32 rasterposx, i32 rasterposy, u8 red, u8 green, u8 blue, u8 alpha)
{
	m_data->m_timeSeries->grapicalPrintf(str, sTimeSeriesFontData, rasterposx, rasterposy, red, green, blue, alpha);
}
