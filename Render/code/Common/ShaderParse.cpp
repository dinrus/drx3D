// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   ShaderParse.cpp : implementation of the Shaders parser part of shaders manager.

   Revision история:
* Created by Honich Andrey

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Eng3D/CGF/DrxHeaders.h>
#include <drx3D/Render/Textures/TextureHelpers.h>

#if DRX_PLATFORM_WINDOWS
	#include <direct.h>
	#include <io.h>
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
#endif

//============================================================
// Compile functions
//============================================================

SShaderGenBit* CShaderMan::mfCompileShaderGenProperty(tuk scr)
{
	tuk name;
	long cmd;
	tuk params;
	tuk data;

	SShaderGenBit* shgm = new SShaderGenBit;

	enum {eName = 1, eProperty, eDescription, eMask, eHidden, eRuntime, ePrecache, eDependencySet, eDependencyReset, eDependFlagSet, eDependFlagReset, eAutoPrecache, eLowSpecAutoPrecache};
	static STokenDesc commands[] =
	{
		{ eName,                "Name"                },
		{ eProperty,            "Property"            },
		{ eDescription,         "Description"         },
		{ eMask,                "Mask"                },
		{ eHidden,              "Hidden"              },
		{ ePrecache,            "Precache"            },
		{ eRuntime,             "Runtime"             },
		{ eAutoPrecache,        "AutoPrecache"        },
		{ eLowSpecAutoPrecache, "LowSpecAutoPrecache" },
		{ eDependencySet,       "DependencySet"       },
		{ eDependencyReset,     "DependencyReset"     },
		{ eDependFlagSet,       "DependFlagSet"       },
		{ eDependFlagReset,     "DependFlagReset"     },

		{ 0,                    0                     }
	};

	shgm->m_PrecacheNames.reserve(45);

	while ((cmd = shGetObject(&scr, commands, &name, &params)) > 0)
	{
		data = NULL;
		if (name)
			data = name;
		else if (params)
			data = params;

		switch (cmd)
		{
		case eName:
			shgm->m_ParamName = data;
			shgm->m_dwToken = CCrc32::Compute(data);
			break;

		case eProperty:
			if (gRenDev->IsEditorMode())
				shgm->m_ParamProp = data;
			break;

		case eDescription:
			if (gRenDev->IsEditorMode())
				shgm->m_ParamDesc = data;
			break;

		case eHidden:
			shgm->m_Flags |= SHGF_HIDDEN;
			break;

		case eRuntime:
			shgm->m_Flags |= SHGF_RUNTIME;
			break;

		case eAutoPrecache:
			shgm->m_Flags |= SHGF_AUTO_PRECACHE;
			break;
		case eLowSpecAutoPrecache:
			shgm->m_Flags |= SHGF_LOWSPEC_AUTO_PRECACHE;
			break;

		case ePrecache:
			shgm->m_PrecacheNames.push_back(CParserBin::GetCRC32(data));
			shgm->m_Flags |= SHGF_PRECACHE;
			break;

		case eDependFlagSet:
			shgm->m_DependSets.push_back(data);
			break;

		case eDependFlagReset:
			shgm->m_DependResets.push_back(data);
			break;

		case eMask:
			if (data && data[0])
			{
				if (data[0] == '0' && (data[1] == 'x' || data[1] == 'X'))
					shgm->m_Mask = shGetHex64(&data[2]);
				else
					shgm->m_Mask = shGetInt(data);
			}
			break;

		case eDependencySet:
			if (data && data[0])
			{
				i32 slot = TextureHelpers::FindTexSlot(data);
				if (slot != EFTT_UNKNOWN)
				{
					switch (slot)
					{
					case EFTT_DIFFUSE:
						break;
					case EFTT_NORMALS:
						shgm->m_nDependencySet |= SHGD_TEX_NORMALS;
						break;
					case EFTT_SPECULAR:
						shgm->m_nDependencySet |= SHGD_TEX_SPECULAR;
						break;
					case EFTT_ENV:
						shgm->m_nDependencySet |= SHGD_TEX_ENVCM;
						break;
					case EFTT_DETAIL_OVERLAY:
						shgm->m_nDependencySet |= SHGD_TEX_DETAIL;
						break;
					case EFTT_SMOOTHNESS:
						break;
					case EFTT_HEIGHT:
						shgm->m_nDependencySet |= SHGD_TEX_HEIGHT;
						break;
					case EFTT_DECAL_OVERLAY:
						shgm->m_nDependencySet |= SHGD_TEX_DECAL;
						break;
					case EFTT_SUBSURFACE:
						shgm->m_nDependencySet |= SHGD_TEX_SUBSURFACE;
						break;
					case EFTT_CUSTOM:
						shgm->m_nDependencySet |= SHGD_TEX_CUSTOM;
						break;
					case EFTT_CUSTOM_SECONDARY:
						shgm->m_nDependencySet |= SHGD_TEX_CUSTOM_SECONDARY;
						break;
					case EFTT_OPACITY:
						break;
					case EFTT_TRANSLUCENCY:
						shgm->m_nDependencySet |= SHGD_TEX_TRANSLUCENCY;
						break;
					case EFTT_EMITTANCE:
						break;
					}
					break;
				}

				if (!stricmp(data, "$HW_WaterTessellation"))
					shgm->m_nDependencySet |= SHGD_HW_WATER_TESSELLATION;
				else if (!stricmp(data, "$HW_SilhouettePom"))
					shgm->m_nDependencySet |= SHGD_HW_SILHOUETTE_POM;
				else if (!stricmp(data, "$UserEnabled"))
					shgm->m_nDependencySet |= SHGD_USER_ENABLED;
				else if (!stricmp(data, "$HW_DURANGO"))
					shgm->m_nDependencySet |= SHGD_HW_DURANGO;
				else if (!stricmp(data, "$HW_ORBIS"))
					shgm->m_nDependencySet |= SHGD_HW_ORBIS;
				else if (!stricmp(data, "$HW_DX11"))
					shgm->m_nDependencySet |= SHGD_HW_DX11;
				else if (!stricmp(data, "$HW_GL4"))
					shgm->m_nDependencySet |= SHGD_HW_GL4;
				else if (!stricmp(data, "$HW_GLES3"))
					shgm->m_nDependencySet |= SHGD_HW_GLES3;
				else if (!stricmp(data, "$HW_VULKAN"))
					shgm->m_nDependencySet |= SHGD_HW_VULKAN;

				// backwards compatible names
				else if (!stricmp(data, "$TEX_Bump") || !stricmp(data, "TM_Bump"))
					shgm->m_nDependencySet |= SHGD_TEX_NORMALS;
				else if (!stricmp(data, "$TEX_BumpHeight") || !stricmp(data, "TM_BumpHeight"))
					shgm->m_nDependencySet |= SHGD_TEX_HEIGHT;
				else if (!stricmp(data, "$TEX_BumpDif") || !stricmp(data, "TM_BumpDif"))
					shgm->m_nDependencySet |= SHGD_TEX_TRANSLUCENCY;
				else if (!stricmp(data, "$TEX_Gloss") || !stricmp(data, "TM_Gloss"))
					shgm->m_nDependencySet |= SHGD_TEX_SPECULAR;

				else
					DRX_ASSERT_TRACE(0, ("Unknown eDependencySet value %s", data));
			}
			break;

		case eDependencyReset:
			if (data && data[0])
			{
				i32 slot = TextureHelpers::FindTexSlot(data);
				if (slot != EFTT_UNKNOWN)
				{
					switch (slot)
					{
					case EFTT_DIFFUSE:
						break;
					case EFTT_NORMALS:
						shgm->m_nDependencyReset |= SHGD_TEX_NORMALS;
						break;
					case EFTT_SPECULAR:
						shgm->m_nDependencyReset |= SHGD_TEX_SPECULAR;
						break;
					case EFTT_ENV:
						shgm->m_nDependencyReset |= SHGD_TEX_ENVCM;
						break;
					case EFTT_DETAIL_OVERLAY:
						shgm->m_nDependencyReset |= SHGD_TEX_DETAIL;
						break;
					case EFTT_SMOOTHNESS:
						break;
					case EFTT_HEIGHT:
						shgm->m_nDependencyReset |= SHGD_TEX_HEIGHT;
						break;
					case EFTT_DECAL_OVERLAY:
						shgm->m_nDependencyReset |= SHGD_TEX_DECAL;
						break;
					case EFTT_SUBSURFACE:
						shgm->m_nDependencyReset |= SHGD_TEX_SUBSURFACE;
						break;
					case EFTT_CUSTOM:
						shgm->m_nDependencyReset |= SHGD_TEX_CUSTOM;
						break;
					case EFTT_CUSTOM_SECONDARY:
						shgm->m_nDependencyReset |= SHGD_TEX_CUSTOM_SECONDARY;
						break;
					case EFTT_OPACITY:
						break;
					case EFTT_TRANSLUCENCY:
						shgm->m_nDependencyReset |= SHGD_TEX_TRANSLUCENCY;
						break;
					case EFTT_EMITTANCE:
						break;
					}
					break;
				}

				if (!stricmp(data, "$HW_WaterTessellation"))
					shgm->m_nDependencyReset |= SHGD_HW_WATER_TESSELLATION;
				else if (!stricmp(data, "$HW_SilhouettePom"))
					shgm->m_nDependencyReset |= SHGD_HW_SILHOUETTE_POM;
				else if (!stricmp(data, "$UserEnabled"))
					shgm->m_nDependencyReset |= SHGD_USER_ENABLED;
				else if (!stricmp(data, "$HW_DX11"))
					shgm->m_nDependencyReset |= SHGD_HW_DX11;
				else if (!stricmp(data, "$HW_GL4"))
					shgm->m_nDependencyReset |= SHGD_HW_GL4;
				else if (!stricmp(data, "$HW_GLES3"))
					shgm->m_nDependencyReset |= SHGD_HW_GLES3;
				else if (!stricmp(data, "$HW_DURANGO"))
					shgm->m_nDependencyReset |= SHGD_HW_DURANGO;
				else if (!stricmp(data, "$HW_ORBIS"))
					shgm->m_nDependencyReset |= SHGD_HW_ORBIS;
				else if (!stricmp(data, "$HW_VULKAN"))
					shgm->m_nDependencySet |= SHGD_HW_VULKAN;

				// backwards compatible names
				else if (!stricmp(data, "$TEX_Bump") || !stricmp(data, "TM_Bump"))
					shgm->m_nDependencySet |= SHGD_TEX_NORMALS;
				else if (!stricmp(data, "$TEX_BumpHeight") || !stricmp(data, "TM_BumpHeight"))
					shgm->m_nDependencySet |= SHGD_TEX_HEIGHT;
				else if (!stricmp(data, "$TEX_BumpDif") || !stricmp(data, "TM_BumpDif"))
					shgm->m_nDependencySet |= SHGD_TEX_TRANSLUCENCY;
				else if (!stricmp(data, "$TEX_Gloss") || !stricmp(data, "TM_Gloss"))
					shgm->m_nDependencySet |= SHGD_TEX_SPECULAR;

				else
					DRX_ASSERT_TRACE(0, ("Unknown eDependencyReset value %s", data));
			}
			break;
		}
	}
	shgm->m_NameLength = strlen(shgm->m_ParamName.c_str());

	return shgm;
}

bool CShaderMan::mfCompileShaderGen(SShaderGen* shg, tuk scr)
{
	tuk name;
	long cmd;
	tuk params;
	tuk data;

	SShaderGenBit* shgm;

	enum {eProperty = 1, eVersion, eUsesCommonGlobalFlags};
	static STokenDesc commands[] =
	{
		{ eProperty,              "Property"              },
		{ eVersion,               "Version"               },
		{ eUsesCommonGlobalFlags, "UsesCommonGlobalFlags" },
		{ 0,                      0                       }
	};

	while ((cmd = shGetObject(&scr, commands, &name, &params)) > 0)
	{
		data = NULL;
		if (name)
			data = name;
		else if (params)
			data = params;

		switch (cmd)
		{
		case eProperty:
			shgm = mfCompileShaderGenProperty(params);
			if (shgm)
				shg->m_BitMask.AddElem(shgm);
			break;

		case eUsesCommonGlobalFlags:
			break;

		case eVersion:
			break;
		}
	}

	return shg->m_BitMask.Num() != 0;
}

string CShaderMan::mfGetShaderBitNamesFromMaskGen(tukk szFileName, uint64 nMaskGen)
{
	// debug/helper function: returns ShaderGen bit names string based on nMaskGen

	if (!nMaskGen)
		return "NO_FLAGS";

	string pszShaderName = PathUtil::GetFileName(szFileName);
	pszShaderName.MakeUpper();

	uint64 nMaskGenRemaped = 0;
	ShaderMapNameFlagsItor pShaderRmp = m_pShadersGlobalFlags.find(pszShaderName.c_str());
	if (pShaderRmp == m_pShadersGlobalFlags.end() || !pShaderRmp->second)
		return "NO_FLAGS";

	string pszShaderBitNames;

	// get shader bit names
	MapNameFlagsItor pIter = m_pShaderCommonGlobalFlag.begin();
	MapNameFlagsItor pEnd = m_pShaderCommonGlobalFlag.end();
	for (; pIter != pEnd; ++pIter)
	{
		if (nMaskGen & pIter->second)
			pszShaderBitNames += pIter->first.c_str();
	}

	return pszShaderBitNames;
}

void CShaderMan::mfRemapShaderGenInfoBits(tukk szName, SShaderGen* pShGen)
{
	// No data to proceed
	if (!pShGen || pShGen->m_BitMask.empty())
		return;

	// Check if shader uses common flags at all
	string pszShaderName = PathUtil::GetFileName(szName);
	pszShaderName.MakeUpper();
	if ((i32) m_pShadersRemapList.find(pszShaderName.c_str()) < 0)
		return;

	MapNameFlags* pOldShaderFlags = 0;
	if (m_pShadersGlobalFlags.find(pszShaderName.c_str()) == m_pShadersGlobalFlags.end())
	{
		pOldShaderFlags = new MapNameFlags;
		m_pShadersGlobalFlags.insert(ShaderMapNameFlagsItor::value_type(pszShaderName.c_str(), pOldShaderFlags));
	}

	u32 nBitCount = pShGen->m_BitMask.size();
	for (u32 b = 0; b < nBitCount; ++b)
	{
		SShaderGenBit* pGenBit = pShGen->m_BitMask[b];
		if (!pGenBit)
			continue;

		// Store old shader flags
		if (pOldShaderFlags)
			pOldShaderFlags->insert(MapNameFlagsItor::value_type(pGenBit->m_ParamName.c_str(), pGenBit->m_Mask));

		// lookup for name - and update mask value
		MapNameFlagsItor pRemaped = m_pShaderCommonGlobalFlag.find(pGenBit->m_ParamName.c_str());
		if (pRemaped != m_pShaderCommonGlobalFlag.end())
			pGenBit->m_Mask = pRemaped->second;

		i32 test = 2;
	}
}

bool CShaderMan::mfUsesGlobalFlags(tukk szShaderName)
{
	// Check if shader uses common flags at all
	string pszName = PathUtil::GetFileName(szShaderName);

	string pszShaderNameUC = pszName;
	pszShaderNameUC.MakeUpper();
	if ((i32) m_pShadersRemapList.find(pszShaderNameUC.c_str()) < 0)
		return false;

	return true;
}

uint64 CShaderMan::mfGetShaderGlobalMaskGenFromString(tukk szShaderGen)
{
	assert(szShaderGen);
	if (!szShaderGen || szShaderGen[0] == '\0')
		return 0;

	uint64 nMaskGen = 0;
	MapNameFlagsItor pEnd = m_pShaderCommonGlobalFlag.end();

	tuk pCurrOffset = (tuk)szShaderGen;
	while (pCurrOffset)
	{
		tuk pBeginOffset = (tuk)strstr(pCurrOffset, "%");
		assert(pBeginOffset);
		PREFAST_ASSUME(pBeginOffset);
		pCurrOffset = (tuk)strstr(pBeginOffset + 1, "%");

		char pCurrFlag[64] = "\0";
		i32 nLen = pCurrOffset ? pCurrOffset - pBeginOffset : strlen(pBeginOffset);
		memcpy(pCurrFlag, pBeginOffset, nLen);

		MapNameFlagsItor pIter = m_pShaderCommonGlobalFlag.find(pCurrFlag);
		if (pIter != pEnd)
			nMaskGen |= pIter->second;
	}

	return nMaskGen;
}

tukk CShaderMan::mfGetShaderBitNamesFromGlobalMaskGen(uint64 nMaskGen)
{
	if (!nMaskGen)
		return "\0";

	static string pszShaderBitNames;
	pszShaderBitNames = "\0";
	MapNameFlagsItor pIter = m_pShaderCommonGlobalFlag.begin();
	MapNameFlagsItor pEnd = m_pShaderCommonGlobalFlag.end();
	for (; pIter != pEnd; ++pIter)
	{
		if (nMaskGen & pIter->second)
			pszShaderBitNames += pIter->first.c_str();
	}

	return pszShaderBitNames.c_str();
}

uint64 CShaderMan::mfGetRemapedShaderMaskGen(tukk szName, uint64 nMaskGen, bool bFixup)
{
	if (!nMaskGen)
		return 0;

	uint64 nMaskGenRemaped = 0;
	string szShaderName = PathUtil::GetFileName(szName);// some shaders might be using concatenated names (eg: terrain.layer), get only first name
	szShaderName.MakeUpper();
	ShaderMapNameFlagsItor pShaderRmp = m_pShadersGlobalFlags.find(szShaderName.c_str());

	//char pszDebug[256];
	//drx_sprintf( pszDebug, "checking: %s...\n", szShaderName);
	//OutputDebugString( pszDebug );

	if (pShaderRmp == m_pShadersGlobalFlags.end() || !pShaderRmp->second)
		return nMaskGen;

	if (bFixup)
	{
		// if some flag was removed, disable in mask
		nMaskGen = nMaskGen & m_nSGFlagsFix;
		return nMaskGen;
	}

	//drx_sprintf( pszDebug, "remapping: %s\n", szShaderName);
	//OutputDebugString( pszDebug );

	// old bitmask - remap to common shared bitmasks
	for (uint64 j = 0; j < 64; ++j)
	{
		uint64 nMask = (((uint64)1) << j);
		if (nMaskGen & nMask)
		{
			MapNameFlagsItor pIter = pShaderRmp->second->begin();
			MapNameFlagsItor pEnd = pShaderRmp->second->end();
			for (; pIter != pEnd; ++pIter)
			{
				// found match - enable bit for remapped mask
				if ((pIter->second) & nMask)
				{
					tukk pFlagName = pIter->first.c_str();
					MapNameFlagsItor pMatchIter = m_pShaderCommonGlobalFlag.find(pIter->first);
					if (pMatchIter != m_pShaderCommonGlobalFlag.end())
					{
						nMaskGenRemaped |= pMatchIter->second;

						//char _debug_[256];
						//drx_sprintf( _debug_, "%s flag %s (%I64x: %I64x remapped to %I64x)\n", szName, pFlagName, nMask, pIter->second, pMatchIter->second);
						//OutputDebugString(_debug_);
					}

					break;
				}
			}
		}
	}

	//char _debug_[256];
	//drx_sprintf( _debug_, "%s shadergen %I64x remapped to %I64x)\n", szName, nMaskGen, nMaskGenRemaped);
	//OutputDebugString(_debug_);

	return nMaskGenRemaped;
}

SShaderGen* CShaderMan::mfCreateShaderGenInfo(tukk szName, bool bRuntime)
{
	CDrxNameTSCRC s = szName;
	if (!bRuntime)
	{
		ShaderExtItor it = m_ShaderExts.find(s);
		if (it != m_ShaderExts.end())
			return it->second;
	}
	SShaderGen* pShGen = NULL;

	stack_string nameFile;
	nameFile.Format("%s%s.ext", gRenDev->m_cEF.m_ShadersGameExtPath.c_str(), szName);
	FILE* fp = gEnv->pDrxPak->FOpen(nameFile.c_str(), "rb", IDrxPak::FOPEN_HINT_QUIET);
	if (!fp)
	{
		nameFile.Format("%s%s.ext", gRenDev->m_cEF.m_ShadersExtPath, szName);
		fp = gEnv->pDrxPak->FOpen(nameFile.c_str(), "rb", IDrxPak::FOPEN_HINT_QUIET);
	}
	if (fp)
	{
		pShGen = new SShaderGen;
		gEnv->pDrxPak->FSeek(fp, 0, SEEK_END);
		i32 ln = gEnv->pDrxPak->FTell(fp);
		tuk buf = new char[ln + 1];
		if (buf)
		{
			buf[ln] = 0;
			gEnv->pDrxPak->FSeek(fp, 0, SEEK_SET);
			gEnv->pDrxPak->FRead(buf, ln, fp);
			mfCompileShaderGen(pShGen, buf);
			mfRemapShaderGenInfoBits(szName, pShGen);
			delete[] buf;
		}
		gEnv->pDrxPak->FClose(fp);
	}
	if (pShGen && !bRuntime)
	{
		pShGen->m_BitMask.Shrink();
		m_ShaderExts.insert(std::pair<CDrxNameTSCRC, SShaderGen*>(s, pShGen));
	}

	return pShGen;
}
