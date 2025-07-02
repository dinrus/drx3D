// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_TERRAINLIGHTING_H__4CAA5295_2647_42FD_8334_359F55EBA4F8__INCLUDED_)
#define AFX_TERRAINLIGHTING_H__4CAA5295_2647_42FD_8334_359F55EBA4F8__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000
// TerrainLighting.h : header file
//

#define LIGHTING_TOOL_WINDOW_NAME "Lighting Tool"

enum eLightAlgorithm              // correspond to the radio buttons in the lighting dialog
{
	//	eHemisphere = 0,
	eDP3        = 1,          // <will be removed soon>
	ePrecise    = 2,          // Sky Accessiblity, lambert sun lighting, soft shadows, baked together with diffuse color (DXT1)
	eDynamicSun = 3,          // diffuse color in (DXT1) | sky accesiblity, lambert sun lighting, vegetation shadow (R5G6B5)
};

struct LightingSettings
{
	eLightAlgorithm eAlgo;             // Algorithm to use for the lighting calculations
	bool            bTerrainShadows;   // Calculate shadows from hills (default false)
	bool            bLighting;         // Calculate lighting (default true)
	i32             iSunRotation;      // Rotation of the sun (default 240 to get sun up in east ) 0..360
	i32             iSunHeight;        // Height of the sun (default 25) 0..100
	i32             iShadowIntensity;  // 0=no shadow..100=full shadow, Intensity of the shadows on vegetation
	i32             iILApplySS;        // 0 = apply no super sampling to terrain occl
	i32             iShadowBlur;       // obsolete, keep it for backstep to sun acc.
	i32             iHemiSamplQuality; // 0=no sampling (=ambient, might be brighter), 1=low (faster) .. 10=highest(slow)
	//	i32 iSkyBrightening;				// 0..100
	i32             iLongitude; // 0..360 m_sldLongitude (0..180 = north..equator..south pole)

	i32             iDawnTime;
	i32             iDawnDuration;
	i32             iDuskTime;
	i32             iDuskDuration;

	//! constructor
	LightingSettings()
	{
		eAlgo = eDynamicSun;
		bTerrainShadows = true;
		bLighting = true;
		iSunRotation = 240;
		iSunHeight = 80;
		iShadowIntensity = 100;
		iILApplySS = 1;
		iHemiSamplQuality = 5;
		//		iSkyBrightening=0;
		iLongitude = 90;    // equator
		iShadowBlur = 0;

		iDawnTime = 355;
		iDawnDuration = 10;
		iDuskTime = 365;
		iDuskDuration = 10;
	}

	Vec3 GetSunVector() const
	{
		Vec3 v;

		v.z = -((float)iSunHeight / 100.0f);

		float r = sqrtf(1.0f - v.z * v.z);

		v.x = sinf(DEG2RAD(iSunRotation)) * r;
		v.y = cosf(DEG2RAD(iSunRotation)) * r;

		// Normalize the light vector
		return (v).GetNormalized();
	}

	void Serialize(XmlNodeRef& node, bool bLoading)
	{
		////////////////////////////////////////////////////////////////////////
		// Load or restore the layer settings from an XML
		////////////////////////////////////////////////////////////////////////
		if (bLoading)
		{
			XmlNodeRef light = node->findChild("Lighting");
			if (!light)
				return;

			i32 algo = 0;
			light->getAttr("SunRotation", iSunRotation);
			light->getAttr("SunHeight", iSunHeight);
			light->getAttr("Algorithm", algo);
			light->getAttr("Lighting", bLighting);
			light->getAttr("Shadows", bTerrainShadows);
			light->getAttr("ShadowIntensity", iShadowIntensity);
			light->getAttr("ILQuality", iILApplySS);
			light->getAttr("HemiSamplQuality", iHemiSamplQuality);
			//			light->getAttr( "SkyBrightening",iSkyBrightening );
			light->getAttr("Longitude", iLongitude);

			light->getAttr("DawnTime", iDawnTime);
			light->getAttr("DawnDuration", iDawnDuration);
			light->getAttr("DuskTime", iDuskTime);
			light->getAttr("DuskDuration", iDuskDuration);

			eAlgo = eDynamicSun;      // only supported algorithm
		}
		else
		{
			i32 algo = (i32)eAlgo;

			// Storing
			XmlNodeRef light = node->newChild("Lighting");
			light->setAttr("SunRotation", iSunRotation);
			light->setAttr("SunHeight", iSunHeight);
			light->setAttr("Algorithm", algo);
			light->setAttr("Lighting", bLighting);
			light->setAttr("Shadows", bTerrainShadows);
			light->setAttr("ShadowIntensity", iShadowIntensity);
			light->setAttr("ILQuality", iILApplySS);
			light->setAttr("HemiSamplQuality", iHemiSamplQuality);
			//			light->setAttr( "SkyBrightening",iSkyBrightening );
			light->setAttr("Longitude", iLongitude);

			light->setAttr("DawnTime", iDawnTime);
			light->setAttr("DawnDuration", iDawnDuration);
			light->setAttr("DuskTime", iDuskTime);
			light->setAttr("DuskDuration", iDuskDuration);

			Vec3 sunVector = GetSunVector();
			light->setAttr("SunVector", sunVector);
		}
	}
};

#endif // !defined(AFX_TERRAINLIGHTING_H__4CAA5295_2647_42FD_8334_359F55EBA4F8__INCLUDED_)

