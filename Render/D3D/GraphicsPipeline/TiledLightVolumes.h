// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/ComputeRenderPass.h>
#include <drx3D/Render/FullscreenPass.h>

constexpr u32 MaxNumTileLights = 255;

// Sun area light parameters (same as in standard deferred shading)
constexpr float TiledShading_SunDistance = 10000.0f;
constexpr float TiledShading_SunSourceDiameter = 94.0f;  // atan(AngDiameterSun) * 2 * SunDistance, where AngDiameterSun=0.54deg

class CVolumetricFogStage;
class CSvoRenderer;

class CTiledLightVolumesStage : public CGraphicsPipelineStage
{
	struct TextureAtlas;
	struct STiledLightInfo;
	struct STiledLightCullInfo;
	struct STiledLightShadeInfo;

public:
	CTiledLightVolumesStage();
	~CTiledLightVolumesStage();

	u32                GetDispatchSizeX() const { return m_dispatchSizeX; }
	u32                GetDispatchSizeY() const { return m_dispatchSizeY; }

	CGpuBuffer*           GetTiledTranspLightMaskBuffer() { return &m_tileTranspLightMaskBuf; }
	CGpuBuffer*           GetTiledOpaqueLightMaskBuffer() { return &m_tileOpaqueLightMaskBuf; }
	CGpuBuffer*           GetLightCullInfoBuffer() { return &m_lightCullInfoBuf; }
	CGpuBuffer*           GetLightShadeInfoBuffer() { return &m_lightShadeInfoBuf; }
	CTexture*             GetProjectedLightAtlas() const { return m_spotTexAtlas.texArray; }
	CTexture*             GetSpecularProbeAtlas() const { return m_specularProbeAtlas.texArray; }
	CTexture*             GetDiffuseProbeAtlas() const { return m_diffuseProbeAtlas.texArray; }

	STiledLightInfo*      GetTiledLightInfo() { return m_tileLights; }
	STiledLightCullInfo*  GetTiledLightCullInfo() { return &tileLightsCull[0]; }
	STiledLightShadeInfo* GetTiledLightShadeInfo() { return &tileLightsShade[0]; }
	u32                GetValidLightCount() { return m_numValidLights; }

	bool                  IsCausticsVisible() const { return m_bApplyCaustics; }
	void                  NotifyCausticsVisible() { m_bApplyCaustics = true; }
	void                  NotifyCausticsInvisible() { m_bApplyCaustics = false; }

	void Init() final;
	void Resize(i32 renderWidth, i32 renderHeight) final;
	void Clear();
	void Destroy(bool destroyResolutionIndependentResources);
	void Update() final;

	void GenerateLightList();
	void Execute();

	bool IsSeparateVolumeListGen();

	// Cubemap Array(s) ==================================================================

	// #PFX2_TODO overly specific function. Re-implement as an algorithm.
	u32 GetLightShadeIndexBySpecularTextureId(i32 textureId) const;

private:
	// Cubemap Array(s) ==================================================================

	i32  InsertTexture(CTexture* texture, float mipFactor, TextureAtlas& atlas, i32 arrayIndex);
	void UploadTextures(TextureAtlas& atlas);
	std::pair<size_t, size_t> MeasureTextures(TextureAtlas& atlas);

	// Tiled Light Volumes ===============================================================

	void GenerateLightVolumeInfo();

	void ExecuteVolumeListGen(u32 dispatchSizeX, u32 dispatchSizeY);

	void InjectSunIntoTiledLights(uint32_t& counter);

private:
	typedef _smart_ptr<CTexture> TexSmartPtr;

	friend class CVolumetricFogStage;
	friend class CSvoRenderer;

	// Cubemap Array(s) ==================================================================

	struct AtlasItem
	{
		static constexpr u8 highestMip = 100;
		static constexpr float mipFactorMinSize = highestMip * highestMip;

		TexSmartPtr texture;
		i32         updateFrameID;
		i32         accessFrameID;
		float       mipFactorRequested;
		u8       lowestTransferedMip;
		u8       lowestRenderableMip;
		bool        invalid;

		AtlasItem() : texture(nullptr), updateFrameID(-1), accessFrameID(0),
			lowestTransferedMip(highestMip), lowestRenderableMip(highestMip),
			mipFactorRequested(mipFactorMinSize), invalid(false) {}
	};

	struct TextureAtlas
	{
		TexSmartPtr            texArray;
		std::vector<AtlasItem> items;

		TextureAtlas() : texArray(nullptr) {}
	};

	// Tiled Light Lists =================================================================
	// The following structs and constants have to match the shader code

	enum ETiledLightTypes
	{
		tlTypeProbe            = 1,
		tlTypeAmbientPoint     = 2,
		tlTypeAmbientProjector = 3,
		tlTypeAmbientArea      = 4,
		tlTypeRegularPoint     = 5,
		tlTypeRegularProjector = 6,
		tlTypeRegularPointFace = 7,
		tlTypeRegularArea      = 8,
		tlTypeSun              = 9,
	};

	struct STiledLightInfo
	{
		u32 lightType;
		u32 volumeType;
		Vec2   depthBoundsVS;

		Vec4   posRad;
		Vec4   volumeParams0;
		Vec4   volumeParams1;
		Vec4   volumeParams2;
	};

	struct STiledLightCullInfo
	{
		u32 volumeType;
		u32 miscFlag;
		Vec2   depthBounds;
		Vec4   posRad;
		Vec4   volumeParams0;
		Vec4   volumeParams1;
		Vec4   volumeParams2;
	};  // 80 bytes

	struct STiledLightShadeInfo
	{
		static constexpr u16 resNoIndex = 0xFFFF;
		static constexpr u8  resMipLimit = 0x00;

		u32   lightType;
		u16   resIndex;
		u8    resMipClamp0;
		u8    resMipClamp1;
		u32   shadowMaskIndex;
		u16   stencilID0;
		u16   stencilID1;
		Vec4     posRad;
		Vec2     attenuationParams;
		Vec2     shadowParams;
		Vec4     color;
		Matrix44 projectorMatrix;
		Matrix44 shadowMatrix;
	};  // 192 bytes

	// Tiled Light Volumes ===============================================================

	enum ETiledVolumeTypes
	{
		tlVolumeSphere = 1,
		tlVolumeCone   = 2,
		tlVolumeOBB    = 3,
		tlVolumeSun    = 4,
	};

	struct STiledLightVolumeInfo
	{
		Matrix44 worldMat;
		Vec4     volumeTypeInfo;
		Vec4     volumeParams0;
		Vec4     volumeParams1;
		Vec4     volumeParams2;
		Vec4     volumeParams3;
	};

	enum EVolumeTypes
	{
		eVolumeType_Box,
		eVolumeType_Sphere,
		eVolumeType_Cone,

		eVolumeType_Count
	};
	
	struct SVolumeGeometry
	{
		CGpuBuffer       vertexDataBuf;
		buffer_handle_t  vertexBuffer;
		buffer_handle_t  indexBuffer;
		u32           numVertices;
		u32           numIndices;
	};

private:
	u32                m_dispatchSizeX, m_dispatchSizeY;

	CGpuBuffer            m_lightCullInfoBuf;
	CGpuBuffer            m_lightShadeInfoBuf;
	CGpuBuffer            m_tileOpaqueLightMaskBuf;
	CGpuBuffer            m_tileTranspLightMaskBuf;

	u32                m_numValidLights;
	u32                m_numSkippedLights;
	u32                m_numAtlasUpdates;
	u32                m_numAtlasEvictions;

	bool                  m_bApplyCaustics;

	// Cubemap Array(s) ==================================================================

	TextureAtlas          m_specularProbeAtlas;
	TextureAtlas          m_diffuseProbeAtlas;
	TextureAtlas          m_spotTexAtlas;

	// Tiled Light Lists =================================================================

	STiledLightInfo       m_tileLights[MaxNumTileLights];

	STiledLightCullInfo*  tileLightsCull;
	STiledLightShadeInfo* tileLightsShade;

	// Tiled Light Volumes ===============================================================

	CGpuBuffer            m_lightVolumeInfoBuf;

	SVolumeGeometry       m_volumeMeshes[eVolumeType_Count];

	CFullscreenPass       m_passCopyDepth;
	CPrimitiveRenderPass  m_passLightVolumes;
	CRenderPrimitive      m_volumePasses[eVolumeType_Count * 2];  // Inside and outside of volume for each type
	u32                m_numVolumesPerPass[eVolumeType_Count * 2];
};
