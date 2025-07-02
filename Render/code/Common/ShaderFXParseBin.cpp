// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   ShaderFXParseBin.cpp : implementation of the Shaders parser using FX language.

   Revision история:
* Created by Honich Andrey

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>
#include <drx3D/CoreX/AlignmentTools.h>
#include <drx3D/Render/Textures/TextureHelpers.h>
#include <drx3D/Render/DriverD3D.h>

extern CD3D9Renderer gcpRendD3D;

static FOURCC FOURCC_SHADERBIN = MAKEFOURCC('F', 'X', 'B', '0');

namespace
{
static std::vector<SFXParam> s_tempFXParams;
}

SShaderBin SShaderBin::s_Root;
u32 SShaderBin::s_nCache = 0;
u32 SShaderBin::s_nMaxFXBinCache = MAX_FXBIN_CACHE;
CShaderManBin::CShaderManBin()
{
	m_pCEF = gRenDev ? &gRenDev->m_cEF : nullptr;
}

i32 CShaderManBin::Size()
{
	SShaderBin* pSB;
	i32 nSize = 0;
	nSize += sizeOfMapStr(m_BinPaths);
	nSize += m_BinValidCRCs.size() * sizeof(bool) * sizeof(stl::MapLikeStruct);

	for (pSB = SShaderBin::s_Root.m_Prev; pSB != &SShaderBin::s_Root; pSB = pSB->m_Prev)
	{
		nSize += pSB->Size();
	}
	return nSize;
}

void CShaderManBin::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_BinPaths);
	pSizer->AddObject(m_BinValidCRCs);

	SShaderBin* pSB;
	for (pSB = SShaderBin::s_Root.m_Prev; pSB != &SShaderBin::s_Root; pSB = pSB->m_Prev)
	{
		pSB->GetMemoryUsage(pSizer);
	}
}

void SShaderBin::CryptData()
{
	u32 i;
	u32* pData = &m_Tokens[0];
	u32 CRC32 = m_CRC32;
	for (i = 0; i < m_Tokens.size(); i++)
	{
		pData[i] ^= CRC32;
	}
}

u32 SShaderBin::ComputeCRC()
{
	if (!m_Tokens.size())
		return 0;
	u32 CRC32;
	if (CParserBin::m_bEndians)
	{
		DWORD* pT = new DWORD[m_Tokens.size()];
		memcpy(pT, &m_Tokens[0], m_Tokens.size() * sizeof(u32));
		SwapEndian(pT, (size_t)m_Tokens.size(), eBigEndian);
		CRC32 = CCrc32::Compute((uk )pT, m_Tokens.size() * sizeof(u32));
		delete[] pT;
	}
	else
		CRC32 = CCrc32::Compute((uk )&m_Tokens[0], m_Tokens.size() * sizeof(u32));
	i32 nCur = 0;
	Lock();
	while (nCur >= 0)
	{
		nCur = CParserBin::FindToken(nCur, m_Tokens.size() - 1, &m_Tokens[0], eT_include);
		if (nCur >= 0)
		{
			nCur++;
			u32 nTokName = m_Tokens[nCur];
			tukk szNameInc = CParserBin::GetString(nTokName, m_TokenTable);
			SShaderBin* pBinIncl = gRenDev->m_cEF.m_Bin.GetBinShader(szNameInc, true, 0);
			assert(pBinIncl);
			if (pBinIncl)
			{
				CRC32 += pBinIncl->ComputeCRC();
			}
		}
	}
	Unlock();

	return CRC32;
}

SShaderBin* CShaderManBin::SaveBinShader(
  u32 nSourceCRC32, tukk szName, bool bInclude, FILE* fpSrc)
{
	SShaderBin* pBin = new SShaderBin;

	CParserBin Parser(pBin);

	gEnv->pDrxPak->FSeek(fpSrc, 0, SEEK_END);
	i32 nSize = gEnv->pDrxPak->FTell(fpSrc);
	tuk buf = new char[nSize + 1];
	tuk pBuf = buf;
	buf[nSize] = 0;
	gEnv->pDrxPak->FSeek(fpSrc, 0, SEEK_SET);
	gEnv->pDrxPak->FRead(buf, nSize, fpSrc);

	RemoveCR(buf);
	tukk kWhiteSpace = " ";

	while (buf && buf[0])
	{
		SkipCharacters(&buf, kWhiteSpace);
		SkipComments(&buf, true);
		if (!buf || !buf[0])
			break;

		char com[1024];
		bool bKey = false;
		u32 dwToken = CParserBin::NextToken(buf, com, bKey);
		dwToken = Parser.NewUserToken(dwToken, com, false);
		pBin->m_Tokens.push_back(dwToken);

		SkipCharacters(&buf, kWhiteSpace);
		SkipComments(&buf, true);
		if (dwToken >= eT_float && dwToken <= eT_int)
		{

		}
		if (dwToken == eT_fetchinst)
		{
			i32 nnn = 0;
		}
		else if (dwToken == eT_include)
		{
			assert(bKey);
			SkipCharacters(&buf, kWhiteSpace);
			assert(*buf == '"' || *buf == '<');
			char brak = *buf;
			++buf;
			i32 n = 0;
			while (*buf != brak)
			{
				if (*buf <= 0x20)
				{
					assert(0);
					break;
				}
				com[n++] = *buf;
				++buf;
			}
			if (*buf == brak)
				++buf;
			com[n] = 0;

			PathUtil::RemoveExtension(com);

			SShaderBin* pBIncl = GetBinShader(com, true, 0);
			//
			//assert(pBIncl);

			dwToken = CParserBin::fxToken(com, NULL);
			dwToken = Parser.NewUserToken(dwToken, com, false);
			pBin->m_Tokens.push_back(dwToken);
		}
		else if (dwToken == eT_if || dwToken == eT_ifdef || dwToken == eT_ifndef || dwToken == eT_elif)
		{
			bool bFirst = fxIsFirstPass(buf);
			if (!bFirst)
			{
				if (dwToken == eT_if)
					dwToken = eT_if_2;
				else if (dwToken == eT_ifdef)
					dwToken = eT_ifdef_2;
				else if (dwToken == eT_elif)
					dwToken = eT_elif_2;
				else
					dwToken = eT_ifndef_2;
				pBin->m_Tokens[pBin->m_Tokens.size() - 1] = dwToken;
			}
		}
		else if (dwToken == eT_define)
		{
			shFill(&buf, com);
			if (com[0] == '%')
				pBin->m_Tokens[pBin->m_Tokens.size() - 1] = eT_define_2;
			dwToken = Parser.NewUserToken(eT_unknown, com, false);
			pBin->m_Tokens.push_back(dwToken);

			TArray<char> macro;
			while (*buf == 0x20 || *buf == 0x9)
			{
				buf++;
			}
			while (*buf != 0xa)
			{
				if (*buf == '\\')
				{
					macro.AddElem('\n');
					while (*buf != '\n')
					{
						buf++;
					}
					buf++;
					continue;
				}
				macro.AddElem(*buf);
				buf++;
			}
			macro.AddElem(0);
			i32 n = macro.Num() - 2;
			while (n >= 0 && macro[n] <= 0x20)
			{
				macro[n] = 0;
				n--;
			}
			tukk b = &macro[0];
			while (*b)
			{
				SkipCharacters(&b, kWhiteSpace);
				SkipComments((tuk*)&b, true);
				if (!b[0])
					break;
				bKey = false;
				dwToken = CParserBin::NextToken(b, com, bKey);
				dwToken = Parser.NewUserToken(dwToken, com, false);
				if (dwToken == eT_if || dwToken == eT_ifdef || dwToken == eT_ifndef || dwToken == eT_elif)
				{
					bool bFirst = fxIsFirstPass(b);
					if (!bFirst)
					{
						if (dwToken == eT_if)
							dwToken = eT_if_2;
						else if (dwToken == eT_ifdef)
							dwToken = eT_ifdef_2;
						else if (dwToken == eT_elif)
							dwToken = eT_elif_2;
						else
							dwToken = eT_ifndef_2;
					}
				}
				pBin->m_Tokens.push_back(dwToken);
			}
			pBin->m_Tokens.push_back(0);
		}
	}
	if (!pBin->m_Tokens[0])
		pBin->m_Tokens.push_back(eT_skip);

	pBin->SetCRC(pBin->ComputeCRC());
	pBin->m_bReadOnly = false;
	//pBin->CryptData();
	//pBin->CryptTable();

	char nameFile[256];
	drx_sprintf(nameFile, "%s%s.%s", m_pCEF->m_ShadersCache, szName, bInclude ? "cfib" : "cfxb");
	stack_string szDst = stack_string(m_pCEF->m_szUserPath.c_str()) + stack_string(nameFile);
	tukk szFileName = szDst;

	FILE* fpDst = gEnv->pDrxPak->FOpen(szFileName, "wb", IDrxPak::FLAGS_NEVER_IN_PAK | IDrxPak::FLAGS_PATH_REAL | IDrxPak::FOPEN_ONDISK);
	if (fpDst)
	{
		SShaderBinHeader Header;
		Header.m_nTokens = pBin->m_Tokens.size();
		Header.m_Magic = FOURCC_SHADERBIN;
		Header.m_CRC32 = pBin->m_CRC32;
		float fVersion = (float)FX_CACHE_VER;
		Header.m_VersionLow = (u16)(((float)fVersion - (float)(i32)fVersion) * 10.1f);
		Header.m_VersionHigh = (u16)fVersion;
		Header.m_nOffsetStringTable = pBin->m_Tokens.size() * sizeof(DWORD) + sizeof(Header);
		Header.m_nOffsetParamsLocal = 0;
		Header.m_nSourceCRC32 = nSourceCRC32;
		SShaderBinHeader hdTemp, * pHD;
		pHD = &Header;
		if (CParserBin::m_bEndians)
		{
			hdTemp = Header;
			SwapEndian(hdTemp, eBigEndian);
			pHD = &hdTemp;
		}
		gEnv->pDrxPak->FWrite((uk )pHD, sizeof(Header), 1, fpDst);
		if (CParserBin::m_bEndians)
		{
			DWORD* pT = new DWORD[pBin->m_Tokens.size()];
			memcpy(pT, &pBin->m_Tokens[0], pBin->m_Tokens.size() * sizeof(DWORD));
			SwapEndian(pT, (size_t)pBin->m_Tokens.size(), eBigEndian);
			gEnv->pDrxPak->FWrite((uk )pT, pBin->m_Tokens.size() * sizeof(DWORD), 1, fpDst);
			delete[] pT;
		}
		else
			gEnv->pDrxPak->FWrite(&pBin->m_Tokens[0], pBin->m_Tokens.size() * sizeof(DWORD), 1, fpDst);
		FXShaderTokenItor itor;
		for (itor = pBin->m_TokenTable.begin(); itor != pBin->m_TokenTable.end(); itor++)
		{
			STokenD T = *itor;
			if (CParserBin::m_bEndians)
				SwapEndian(T.Token, eBigEndian);
			gEnv->pDrxPak->FWrite(&T.Token, sizeof(DWORD), 1, fpDst);
			gEnv->pDrxPak->FWrite(T.SToken.c_str(), T.SToken.size() + 1, 1, fpDst);
		}
		Header.m_nOffsetParamsLocal = gEnv->pDrxPak->FTell(fpDst);
		gEnv->pDrxPak->FSeek(fpDst, 0, SEEK_SET);
		if (CParserBin::m_bEndians)
		{
			hdTemp = Header;
			SwapEndian(hdTemp, eBigEndian);
			pHD = &hdTemp;
		}
		gEnv->pDrxPak->FWrite((uk )pHD, sizeof(Header), 1, fpDst);
		gEnv->pDrxPak->FClose(fpDst);
	}
	else
	{
		iLog->LogWarning("CShaderManBin::SaveBinShader: Cannot write shader to file '%s'.", nameFile);
		pBin->m_bReadOnly = true;
	}

	SAFE_DELETE_ARRAY(pBuf);

	return pBin;
}

//=========================================================================================================================================

static void sParseCSV(string& sFlt, std::vector<string>& Filters)
{
	tukk cFlt = sFlt.c_str();
	char Flt[64];
	i32 nFlt = 0;
	while (true)
	{
		char c = *cFlt++;
		if (!c)
			break;
		if (SkipChar((u8)c))
		{
			if (nFlt)
			{
				Flt[nFlt] = 0;
				Filters.push_back(string(Flt));
				nFlt = 0;
			}
			continue;
		}
		Flt[nFlt++] = c;
	}
	if (nFlt)
	{
		Flt[nFlt] = 0;
		Filters.push_back(string(Flt));
	}
}

//===========================================================================================================================

struct FXParamsSortByName
{
	bool operator()(const SFXParam& left, const SFXParam& right) const
	{
		return left.m_dwName[0] < right.m_dwName[0];
	}
	bool operator()(u32k left, const SFXParam& right) const
	{
		return left < right.m_dwName[0];
	}
	bool operator()(const SFXParam& left, u32 right) const
	{
		return left.m_dwName[0] < right;
	}
};
struct FXSamplersSortByName
{
	bool operator()(const SFXSampler& left, const SFXSampler& right) const
	{
		return left.m_dwName[0] < right.m_dwName[0];
	}
	bool operator()(u32k left, const SFXSampler& right) const
	{
		return left < right.m_dwName[0];
	}
	bool operator()(const SFXSampler& left, u32k right) const
	{
		return left.m_dwName[0] < right;
	}
};
struct FXTexturesSortByName
{
	bool operator()(const SFXTexture& left, const SFXTexture& right) const
	{
		return left.m_dwName[0] < right.m_dwName[0];
	}
	bool operator()(u32k left, const SFXTexture& right) const
	{
		return left < right.m_dwName[0];
	}
	bool operator()(const SFXTexture& left, u32k right) const
	{
		return left.m_dwName[0] < right;
	}
};

i32 CShaderManBin::mfSizeFXParams(u32& nCount)
{
	nCount = m_ShaderFXParams.size();
	i32 nSize = sizeOfMap(m_ShaderFXParams);
	return nSize;
}

void CShaderManBin::mfReleaseFXParams()
{
	m_ShaderFXParams.clear();
}

SShaderFXParams& CShaderManBin::mfGetFXParams(CShader* pSH)
{
	CDrxNameTSCRC s = pSH->GetNameCRC();
	ShaderFXParamsItor it = m_ShaderFXParams.find(s);
	if (it != m_ShaderFXParams.end())
		return it->second;
	SShaderFXParams pr;
	std::pair<ShaderFXParams::iterator, bool> insertLocation =
	  m_ShaderFXParams.insert(std::pair<CDrxNameTSCRC, SShaderFXParams>(s, pr));
	assert(insertLocation.second);
	return insertLocation.first->second;
}

void CShaderManBin::mfRemoveFXParams(CShader* pSH)
{
	CDrxNameTSCRC s = pSH->GetNameCRC();
	ShaderFXParamsItor it = m_ShaderFXParams.find(s);
	if (it != m_ShaderFXParams.end())
		m_ShaderFXParams.erase(it);
}

SFXParam* CShaderManBin::mfAddFXParam(SShaderFXParams& FXP, const SFXParam* pParam)
{
	FXParamsIt it = std::lower_bound(FXP.m_FXParams.begin(), FXP.m_FXParams.end(), pParam->m_dwName[0], FXParamsSortByName());
	if (it != FXP.m_FXParams.end() && pParam->m_dwName[0] == (*it).m_dwName[0])
	{
		SFXParam& pr = *it;
		pr.m_nFlags = pParam->m_nFlags;
		i32 n = 6;
		SFXParam& p = *it;
		for (i32 i = 0; i < n; i++)
		{
			if (p.m_nRegister[i] == 10000)
				p.m_nRegister[i] = pParam->m_nRegister[i];
		}
		//assert(p == *pParam);
		return &(*it);
	}
	FXP.m_FXParams.insert(it, *pParam);
	it = std::lower_bound(FXP.m_FXParams.begin(), FXP.m_FXParams.end(), pParam->m_dwName[0], FXParamsSortByName());
	SFXParam* pFX = &(*it);
	if (pFX->m_Semantic.empty() && pFX->m_Values.c_str()[0] == '(')
		pFX->m_nCB = CB_PER_MATERIAL;
	FXP.m_nFlags |= FXP_PARAMS_DIRTY;

	return pFX;
}
SFXParam* CShaderManBin::mfAddFXParam(CShader* pSH, const SFXParam* pParam)
{
	if (!pParam)
		return NULL;

	SShaderFXParams& FXP = mfGetFXParams(pSH);
	return mfAddFXParam(FXP, pParam);
}
void CShaderManBin::mfAddFXSampler(CShader* pSH, const SFXSampler* pSamp)
{
	if (!pSamp)
		return;

	SShaderFXParams& FXP = mfGetFXParams(pSH);

	FXSamplersIt it = std::lower_bound(FXP.m_FXSamplers.begin(), FXP.m_FXSamplers.end(), pSamp->m_dwName[0], FXSamplersSortByName());
	if (it != FXP.m_FXSamplers.end() && pSamp->m_dwName[0] == (*it).m_dwName[0])
	{
		assert(*it == *pSamp);
		return;
	}
	FXP.m_FXSamplers.insert(it, *pSamp);
	FXP.m_nFlags |= FXP_SAMPLERS_DIRTY;
}
void CShaderManBin::mfAddFXTexture(CShader* pSH, const SFXTexture* pSamp)
{
	if (!pSamp)
		return;

	SShaderFXParams& FXP = mfGetFXParams(pSH);

	FXTexturesIt it = std::lower_bound(FXP.m_FXTextures.begin(), FXP.m_FXTextures.end(), pSamp->m_dwName[0], FXTexturesSortByName());
	if (it != FXP.m_FXTextures.end() && pSamp->m_dwName[0] == (*it).m_dwName[0])
	{
		assert(*it == *pSamp);
		return;
	}
	FXP.m_FXTextures.insert(it, *pSamp);
	FXP.m_nFlags |= FXP_TEXTURES_DIRTY;
}

void CShaderManBin::mfGeneratePublicFXParams(CShader* pSH, CParserBin& Parser)
{
	SShaderFXParams& FXP = mfGetFXParams(pSH);
	if (!(FXP.m_nFlags & FXP_PARAMS_DIRTY))
		return;
	FXP.m_nFlags &= ~FXP_PARAMS_DIRTY;

	u32 i;
	// Generate public parameters
	for (i = 0; i < FXP.m_FXParams.size(); i++)
	{
		SFXParam* pr = &FXP.m_FXParams[i];
		u32 nFlags = pr->GetParamFlags();
		if (nFlags & PF_AUTOMERGED)
			continue;
		if (nFlags & PF_TWEAKABLE_MASK)
		{
			tukk szName = Parser.GetString(pr->m_dwName[0]);
			// Avoid duplicating of public parameters
			i32 j;
			for (j = 0; j < FXP.m_PublicParams.size(); j++)
			{
				SShaderParam* p = &FXP.m_PublicParams[j];
				if (!stricmp(p->m_Name, szName))
					break;
			}
			if (j == FXP.m_PublicParams.size())
			{
				SShaderParam sp;
				drx_strcpy(sp.m_Name, szName);
				EParamType eType;
				string szWidget = pr->GetValueForName("UIWidget", eType);
				tukk szVal = pr->m_Values.c_str();
				if (szWidget == "color")
				{
					sp.m_Type = eType_FCOLOR;
					if (szVal[0] == '{')
						szVal++;
					i32 n = sscanf(szVal, "%f, %f, %f, %f", &sp.m_Value.m_Color[0], &sp.m_Value.m_Color[1], &sp.m_Value.m_Color[2], &sp.m_Value.m_Color[3]);
					assert(n == 4);
				}
				else
				{
					sp.m_Type = eType_FLOAT;
					sp.m_Value.m_Float = (float)atof(szVal);
				}

				bool bAdd = true;
				if (!pr->m_Annotations.empty() && gRenDev->IsEditorMode())
				{
					//EParamType eType;
					string sFlt = pr->GetValueForName("Filter", eType);
					bool bUseScript = true;
					if (!sFlt.empty())
					{
						std::vector<string> Filters;
						sParseCSV(sFlt, Filters);
						string strShader = Parser.m_pCurShader->GetName();
						u32 h;
						for (h = 0; h < Filters.size(); h++)
						{
							if (!strnicmp(Filters[h].c_str(), strShader.c_str(), Filters[h].size()))
								break;
						}
						if (h == Filters.size())
						{
							bUseScript = false;
							bAdd = false;
						}
					}

					if (bUseScript)
					{
						sp.m_Script = pr->m_Annotations.c_str();
					}
				}

				if (bAdd)
				{
					FXP.m_PublicParams.push_back(sp);
				}
			}
		}
	}
}

SParamCacheInfo* CShaderManBin::GetParamInfo(SShaderBin* pBin, u32 dwName, uint64 nMaskGenFX)
{
	i32k n = pBin->m_ParamsCache.size();
	for (i32 i = 0; i < n; i++)
	{
		SParamCacheInfo* pInf = &pBin->m_ParamsCache[i];
		if (pInf->m_dwName == dwName && pInf->m_nMaskGenFX == nMaskGenFX)
		{
			pBin->m_nCurParamsID = i;
			return pInf;
		}
	}
	pBin->m_nCurParamsID = -1;
	return NULL;
}

bool CShaderManBin::SaveBinShaderLocalInfo(SShaderBin* pBin, u32 dwName, uint64 nMaskGenFX, TArray<i32>& Funcs, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures)
{
	if (GetParamInfo(pBin, dwName, nMaskGenFX))
		return true;
	//return false;
	if (pBin->IsReadOnly() && !gEnv->IsEditor()) // if in the editor, allow params to be added in-memory, but not saved to disk
		return false;
	TArray<i32> EParams;
	TArray<i32> ESamplers;
	TArray<i32> ETextures;
	TArray<i32> EFuncs;
	for (u32 i = 0; i < Params.size(); i++)
	{
		SFXParam& pr = Params[i];
		assert(pr.m_dwName.size());
		if (pr.m_dwName.size())
			EParams.push_back(pr.m_dwName[0]);
	}
	for (u32 i = 0; i < Samplers.size(); i++)
	{
		SFXSampler& pr = Samplers[i];
		assert(pr.m_dwName.size());
		if (pr.m_dwName.size())
			ESamplers.push_back(pr.m_dwName[0]);
	}
	for (u32 i = 0; i < Textures.size(); i++)
	{
		SFXTexture& pr = Textures[i];
		assert(pr.m_dwName.size());
		if (pr.m_dwName.size())
			ETextures.push_back(pr.m_dwName[0]);
	}
	pBin->m_nCurParamsID = pBin->m_ParamsCache.size();
	pBin->m_ParamsCache.push_back(SParamCacheInfo());
	SParamCacheInfo& pr = pBin->m_ParamsCache.back();
	pr.m_nMaskGenFX = nMaskGenFX;
	pr.m_dwName = dwName;
	pr.m_AffectedFuncs.assign(Funcs.begin(), Funcs.end());
	pr.m_AffectedParams.assign(EParams.begin(), EParams.end());
	pr.m_AffectedSamplers.assign(ESamplers.begin(), ESamplers.end());
	pr.m_AffectedTextures.assign(ETextures.begin(), ETextures.end());
	if (pBin->IsReadOnly())
		return false;
	FILE* fpBin = gEnv->pDrxPak->FOpen(pBin->m_szName, "r+b", IDrxPak::FLAGS_NEVER_IN_PAK | IDrxPak::FLAGS_PATH_REAL | IDrxPak::FOPEN_ONDISK);
	assert(fpBin);
	if (!fpBin)
		return false;
	gEnv->pDrxPak->FSeek(fpBin, 0, SEEK_END);
	i32 nSeek = gEnv->pDrxPak->FTell(fpBin);
	assert(nSeek > 0);
	if (nSeek <= 0)
		return false;
	SShaderBinParamsHeader sd;
	i32* pFuncs = &Funcs[0];
	i32* pParams = EParams.size() ? &EParams[0] : NULL;
	i32* pSamplers = ESamplers.size() ? &ESamplers[0] : NULL;
	i32* pTextures = ETextures.size() ? &ETextures[0] : NULL;
	sd.nMask = nMaskGenFX;
	sd.nName = dwName;
	sd.nFuncs = Funcs.size();
	sd.nParams = EParams.size();
	sd.nSamplers = ESamplers.size();
	sd.nTextures = ETextures.size();
	if (CParserBin::m_bEndians)
	{
		SwapEndian(sd, eBigEndian);
		EFuncs = Funcs;
		if (EParams.size())
			SwapEndian(&EParams[0], (size_t)EParams.size(), eBigEndian);
		if (ESamplers.size())
			SwapEndian(&ESamplers[0], (size_t)ESamplers.size(), eBigEndian);
		if (ETextures.size())
			SwapEndian(&ETextures[0], (size_t)ETextures.size(), eBigEndian);
		SwapEndian(&EFuncs[0], (size_t)EFuncs.size(), eBigEndian);
		pFuncs = &EFuncs[0];
	}
	gEnv->pDrxPak->FWrite(&sd, sizeof(sd), 1, fpBin);
	if (EParams.size())
		gEnv->pDrxPak->FWrite(pParams, EParams.size(), sizeof(i32), fpBin);
	if (ESamplers.size())
		gEnv->pDrxPak->FWrite(pSamplers, ESamplers.size(), sizeof(i32), fpBin);
	if (ETextures.size())
		gEnv->pDrxPak->FWrite(pTextures, ETextures.size(), sizeof(i32), fpBin);
	gEnv->pDrxPak->FWrite(pFuncs, Funcs.size(), sizeof(i32), fpBin);
	gEnv->pDrxPak->FClose(fpBin);

	return true;
}

SShaderBin* CShaderManBin::LoadBinShader(FILE* fpBin, tukk szName, tukk szNameBin, bool bReadParams)
{
	LOADING_TIME_PROFILE_SECTION(iSystem);

	gEnv->pDrxPak->FSeek(fpBin, 0, SEEK_SET);
	SShaderBinHeader Header;
	size_t sizeRead = gEnv->pDrxPak->FReadRaw(&Header, 1, sizeof(SShaderBinHeader), fpBin);
	if (sizeRead != sizeof(SShaderBinHeader))
	{
		DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read header for %s in CShaderManBin::LoadBinShader. Expected %" PRISIZE_T ", got %" PRISIZE_T, szName, sizeof(SShaderBinHeader), sizeRead);
		return NULL;
	}

	if (CParserBin::m_bEndians)
		SwapEndian(Header, eBigEndian);
	float fVersion = (float)FX_CACHE_VER;
	u16 MinorVer = (u16)(((float)fVersion - (float)(i32)fVersion) * 10.1f);
	u16 MajorVer = (u16)fVersion;
	bool bCheckValid = CRenderer::CV_r_shadersAllowCompilation != 0;
	if (bCheckValid && (Header.m_VersionLow != MinorVer || Header.m_VersionHigh != MajorVer || Header.m_Magic != FOURCC_SHADERBIN))
		return NULL;
	if (Header.m_VersionHigh > 10)
		return NULL;
	SShaderBin* pBin = new SShaderBin;

	pBin->m_SourceCRC32 = Header.m_nSourceCRC32;
	pBin->m_nOffsetLocalInfo = Header.m_nOffsetParamsLocal;

	u32 CRC32 = Header.m_CRC32;
	pBin->m_CRC32 = CRC32;
	pBin->m_Tokens.resize(Header.m_nTokens);
	sizeRead = gEnv->pDrxPak->FReadRaw(&pBin->m_Tokens[0], sizeof(u32), Header.m_nTokens, fpBin);
	if (sizeRead != Header.m_nTokens)
	{
		DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read Tokens for %s in CShaderManBin::LoadBinShader. Expected %u, got %" PRISIZE_T, szName, Header.m_nTokens, sizeRead);
		return NULL;
	}
	if (CParserBin::m_bEndians)
		SwapEndian(&pBin->m_Tokens[0], (size_t)Header.m_nTokens, eBigEndian);

	//pBin->CryptData();
	i32 nSizeTable = Header.m_nOffsetParamsLocal - Header.m_nOffsetStringTable;
	if (nSizeTable < 0)
		return NULL;
	tuk bufTable = new char[nSizeTable];
	tuk bufT = bufTable;
	sizeRead = gEnv->pDrxPak->FReadRaw(bufTable, 1, nSizeTable, fpBin);
	if (sizeRead != nSizeTable)
	{
		DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read bufTable for %s in CShaderManBin::LoadBinShader. Expected %d, got %" PRISIZE_T, szName, nSizeTable, sizeRead);
		return NULL;
	}
	tuk bufEnd = &bufTable[nSizeTable];

	// First pass to count the tokens
	u32 nTokens(0);
	while (bufTable < bufEnd)
	{
		STokenD TD;
		LoadUnaligned(bufTable, TD.Token);
		i32 nIncr = 4 + strlen(&bufTable[4]) + 1;
		bufTable += nIncr;
		++nTokens;
	}

	pBin->m_TokenTable.reserve(nTokens);
	bufTable = bufT;
	while (bufTable < bufEnd)
	{
		STokenD TD;
		LoadUnaligned(bufTable, TD.Token);
		if (CParserBin::m_bEndians)
			SwapEndian(TD.Token, eBigEndian);
		FXShaderTokenItor itor = std::lower_bound(pBin->m_TokenTable.begin(), pBin->m_TokenTable.end(), TD.Token, SortByToken());
		assert(itor == pBin->m_TokenTable.end() || (*itor).Token != TD.Token);
		TD.SToken = &bufTable[4];
		pBin->m_TokenTable.insert(itor, TD);
		i32 nIncr = 4 + strlen(&bufTable[4]) + 1;
		bufTable += nIncr;
	}
	SAFE_DELETE_ARRAY(bufT);

	//if (CRenderer::CV_r_shadersnocompile)
	//  bReadParams = false;
	if (bReadParams)
	{
		i32 nSeek = pBin->m_nOffsetLocalInfo;
		gEnv->pDrxPak->FSeek(fpBin, nSeek, SEEK_SET);
		while (true)
		{
			SShaderBinParamsHeader sd;
			i32 nSize = gEnv->pDrxPak->FReadRaw(&sd, 1, sizeof(sd), fpBin);
			if (nSize != sizeof(sd))
			{
				break;
			}
			if (CParserBin::m_bEndians)
				SwapEndian(sd, eBigEndian);
			SParamCacheInfo pr;
			i32 n = pBin->m_ParamsCache.size();
			pBin->m_ParamsCache.push_back(pr);
			SParamCacheInfo& prc = pBin->m_ParamsCache[n];
			prc.m_dwName = sd.nName;
			prc.m_nMaskGenFX = sd.nMask;
			prc.m_AffectedParams.resize(sd.nParams);
			prc.m_AffectedSamplers.resize(sd.nSamplers);
			prc.m_AffectedTextures.resize(sd.nTextures);
			prc.m_AffectedFuncs.resize(sd.nFuncs);

			if (sd.nParams)
			{
				nSize = gEnv->pDrxPak->FReadRaw(&prc.m_AffectedParams[0], sizeof(i32), sd.nParams, fpBin);
				if (nSize != sd.nParams)
				{
					DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read m_AffectedParams for %s in CShaderManBin::LoadBinShader. Expected %d, got %" PRISIZE_T, szName, sd.nParams, sizeRead);
					return NULL;
				}
				if (CParserBin::m_bEndians)
					SwapEndian(&prc.m_AffectedParams[0], sd.nParams, eBigEndian);
			}
			if (sd.nSamplers)
			{
				nSize = gEnv->pDrxPak->FReadRaw(&prc.m_AffectedSamplers[0], sizeof(i32), sd.nSamplers, fpBin);
				if (nSize != sd.nSamplers)
				{
					DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read m_AffectedSamplers for %s in CShaderManBin::LoadBinShader. Expected %d, got %" PRISIZE_T, szName, sd.nSamplers, sizeRead);
					return NULL;
				}
				if (CParserBin::m_bEndians)
					SwapEndian(&prc.m_AffectedSamplers[0], sd.nSamplers, eBigEndian);
			}
			if (sd.nTextures)
			{
				nSize = gEnv->pDrxPak->FReadRaw(&prc.m_AffectedTextures[0], sizeof(i32), sd.nTextures, fpBin);
				if (nSize != sd.nTextures)
				{
					DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read m_AffectedTextures for %s in CShaderManBin::LoadBinShader. Expected %d, got %" PRISIZE_T, szName, sd.nTextures, sizeRead);
					return NULL;
				}
				if (CParserBin::m_bEndians)
					SwapEndian(&prc.m_AffectedTextures[0], sd.nTextures, eBigEndian);
			}

			assert(sd.nFuncs > 0);
			nSize = gEnv->pDrxPak->FReadRaw(&prc.m_AffectedFuncs[0], sizeof(i32), sd.nFuncs, fpBin);
			if (nSize != sd.nFuncs)
			{
				DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Failed to read nFuncs for %s in CShaderManBin::LoadBinShader. Expected %d, got %" PRISIZE_T, szName, sd.nFuncs, sizeRead);
				return NULL;
			}
			if (CParserBin::m_bEndians)
				SwapEndian(&prc.m_AffectedFuncs[0], sd.nFuncs, eBigEndian);

			nSeek += (sd.nFuncs) * sizeof(i32) + sizeof(sd);
		}
	}

	char nameLwr[256];
	drx_strcpy(nameLwr, szName);
	strlwr(nameLwr);
	pBin->SetName(szNameBin);
	pBin->m_dwName = CParserBin::GetCRC32(nameLwr);

	return pBin;
}

SShaderBin* CShaderManBin::SearchInCache(tukk szName, bool bInclude)
{
	char nameFile[256], nameLwr[256];
	drx_strcpy(nameLwr, szName);
	strlwr(nameLwr);
	drx_sprintf(nameFile, "%s.%s", nameLwr, bInclude ? "cfi" : "cfx");
	u32 dwName = CParserBin::GetCRC32(nameFile);

	SShaderBin* pSB;
	for (pSB = SShaderBin::s_Root.m_Prev; pSB != &SShaderBin::s_Root; pSB = pSB->m_Prev)
	{
		if (pSB->m_dwName == dwName)
		{
			pSB->Unlink();
			pSB->Link(&SShaderBin::s_Root);
			return pSB;
		}
	}
	return NULL;
}

bool CShaderManBin::AddToCache(SShaderBin* pSB, bool bInclude)
{
	if (!CRenderer::CV_r_shadersediting)
	{
		if (SShaderBin::s_nCache > SShaderBin::s_nMaxFXBinCache)
		{
			SShaderBin* pS;
			for (pS = SShaderBin::s_Root.m_Prev; pS != &SShaderBin::s_Root; pS = pS->m_Prev)
			{
				if (!pS->m_bLocked)
				{
					DeleteFromCache(pS);
					break;
				}
			}
		}
		assert(SShaderBin::s_nCache <= SShaderBin::s_nMaxFXBinCache);
	}

	pSB->m_bInclude = bInclude;
	pSB->Link(&SShaderBin::s_Root);
	SShaderBin::s_nCache++;

	return true;
}

bool CShaderManBin::DeleteFromCache(SShaderBin* pSB)
{
	assert(pSB != &SShaderBin::s_Root);
	pSB->Unlink();
	SAFE_DELETE(pSB);
	SShaderBin::s_nCache--;

	return true;
}

void CShaderManBin::InvalidateCache(bool bIncludesOnly)
{
	SShaderBin* pSB, * Next;
	for (pSB = SShaderBin::s_Root.m_Next; pSB != &SShaderBin::s_Root; pSB = Next)
	{
		Next = pSB->m_Next;
		if (bIncludesOnly && !pSB->m_bInclude)
			continue;
		DeleteFromCache(pSB);
	}
	SShaderBin::s_nMaxFXBinCache = MAX_FXBIN_CACHE;
	m_bBinaryShadersLoaded = false;
	stl::free_container(s_tempFXParams);

	g_shaderBucketAllocator.cleanup();
}

#define SEC5_FILETIME 10*1000*1000*5

SShaderBin* CShaderManBin::GetBinShader(tukk szName, bool bInclude, u32 nRefCRC, bool* pbChanged)
{
	//static float sfTotalTime = 0.0f;

	if (pbChanged)
	{
		if (gRenDev->IsEditorMode())
			*pbChanged = false;
	}

	//float fTime0 = iTimer->GetAsyncCurTime();

	SShaderBin* pSHB = SearchInCache(szName, bInclude);
	if (pSHB)
		return pSHB;
	SShaderBinHeader Header[2];
	memset(&Header, 0, 2 * sizeof(SShaderBinHeader));
	stack_string nameFile;
	string nameBin;
	FILE* fpSrc = NULL;
	u32 nSourceCRC32 = 0;

	tukk szExt = bInclude ? "cfi" : "cfx";
	// First look for source in Game folder
	nameFile.Format("%sDrxFX/%s.%s", gRenDev->m_cEF.m_ShadersGamePath.c_str(), szName, szExt);
#if !defined(_RELEASE)
	{
		fpSrc = gEnv->pDrxPak->FOpen(nameFile.c_str(), "rb");
		nSourceCRC32 = fpSrc ? gEnv->pDrxPak->ComputeCRC(nameFile) : 0;
	}
#endif
	if (!fpSrc)
	{
		// Second look in Engine folder
		nameFile.Format("%s/%sDrxFX/%s.%s", "%ENGINE%", gRenDev->m_cEF.m_ShadersPath, szName, szExt);
#if !defined(_RELEASE)
		{
			fpSrc = gEnv->pDrxPak->FOpen(nameFile.c_str(), "rb");
			nSourceCRC32 = fpSrc ? gEnv->pDrxPak->ComputeCRC(nameFile) : 0;
		}
#endif
	}
	//char szPath[1024];
	//getcwd(szPath, 1024);
	nameBin.Format("%s/%s%s.%s", "%ENGINE%", m_pCEF->m_ShadersCache, szName, bInclude ? "cfib" : "cfxb");
	FILE* fpDst = NULL;
	i32 i = 0, n = 2;

	// don't load from the shadercache.pak when in editing mode
	if (CRenderer::CV_r_shadersediting)
		i = 1;

	string szDst = m_pCEF->m_szUserPath + (nameBin.c_str() + 9); // skip '%ENGINE%/'
	byte bValid = 0;
	float fVersion = (float)FX_CACHE_VER;
	for (; i < n; i++)
	{
		if (fpDst)
			gEnv->pDrxPak->FClose(fpDst);
		if (!i)
		{
			if (n == 2)
			{
				char nameLwr[256];
				drx_sprintf(nameLwr, "%s.%s", szName, bInclude ? "cfi" : "cfx");
				strlwr(nameLwr);
				u32 dwName = CParserBin::GetCRC32(nameLwr);
				FXShaderBinValidCRCItor itor = m_BinValidCRCs.find(dwName);
				if (itor != m_BinValidCRCs.end())
				{
					assert(itor->second == false);
					continue;
				}
			}
			fpDst = gEnv->pDrxPak->FOpen(nameBin.c_str(), "rb");
		}
		else
			fpDst = gEnv->pDrxPak->FOpen(szDst.c_str(), "rb", IDrxPak::FLAGS_NEVER_IN_PAK | IDrxPak::FLAGS_PATH_REAL | IDrxPak::FOPEN_ONDISK);
		if (!fpDst)
			continue;
		else
		{
			gEnv->pDrxPak->FReadRaw(&Header[i], 1, sizeof(SShaderBinHeader), fpDst);
			if (CParserBin::m_bEndians)
				SwapEndian(Header[i], eBigEndian);

#if !defined(_RELEASE)
			// check source crc changes
			if (nSourceCRC32 && nSourceCRC32 != Header[i].m_nSourceCRC32)
			{
				bValid |= 1 << i;
			}
			else
#endif
			{
				u16 MinorVer = (u16)(((float)fVersion - (float)(i32)fVersion) * 10.1f);
				u16 MajorVer = (u16)fVersion;
				if (Header[i].m_VersionLow != MinorVer || Header[i].m_VersionHigh != MajorVer || Header[i].m_Magic != FOURCC_SHADERBIN)
					bValid |= 4 << i;
				else if (nRefCRC && Header[i].m_CRC32 != nRefCRC)
					bValid |= 0x10 << i;
			}
		}
		if (!(bValid & (0x15 << i)))
			break;
	}
	if (i == n)
	{
#if !defined(_RELEASE) && !defined(CONSOLE_CONST_CVAR_MODE)
		{
			char acTemp[512];
			if (bValid & 1)
			{
				drx_sprintf(acTemp, "WARNING: Bin FXShader '%s' source crc mismatch", nameBin.c_str());
			}
			if (bValid & 4)
			{
				drx_sprintf(acTemp, "WARNING: Bin FXShader '%s' version mismatch (Cache: %u.%u, Expected: %.1f)", nameBin.c_str(), Header[0].m_VersionHigh, Header[0].m_VersionLow, fVersion);
			}
			if (bValid & 0x10)
			{
				drx_sprintf(acTemp, "WARNING: Bin FXShader '%s' CRC mismatch", nameBin.c_str());
			}

			if (bValid & 2)
			{
				drx_sprintf(acTemp, "WARNING: Bin FXShader USER '%s' source crc mismatch", szDst.c_str());
			}
			if (bValid & 8)
			{
				drx_sprintf(acTemp, "WARNING: Bin FXShader USER '%s' version mismatch (Cache: %u.%u, Expected: %.1f)", szDst.c_str(), Header[1].m_VersionHigh, Header[1].m_VersionLow, fVersion);
			}
			if (bValid & 0x20)
			{
				drx_sprintf(acTemp, "WARNING: Bin FXShader USER '%s' CRC mismatch", szDst.c_str());
			}

			if (bValid)
			{
				LogWarningEngineOnly(acTemp);
			}

			if (fpDst)
			{
				gEnv->pDrxPak->FClose(fpDst);
				fpDst = NULL;
			}

			if (fpSrc)
			{
				// enable shader compilation again, and show big error message
				if (!CRenderer::CV_r_shadersAllowCompilation)
				{
					if (CRenderer::CV_r_shaderscompileautoactivate)
					{
						CRenderer::CV_r_shadersAllowCompilation = 1;
						CRenderer::CV_r_shadersasyncactivation = 0;

						gEnv->pLog->LogError("ERROR LOADING BIN SHADER - REACTIVATING SHADER COMPILATION !");
					}
					else
					{
						static bool bShowMessageBox = true;

						if (bShowMessageBox)
						{
							DrxMessageBox(acTemp, "Invalid ShaderCache", eMB_Error);
							bShowMessageBox = false;
							DrxSleep(33);
						}
					}
				}

				if (CRenderer::CV_r_shadersAllowCompilation)
				{
					pSHB = SaveBinShader(nSourceCRC32, szName, bInclude, fpSrc);
					assert(!pSHB->m_Next);
					if (pbChanged)
						*pbChanged = true;

					// remove the entries in the looupdata, to be sure that level and global caches have also become invalid for these shaders!
					gRenDev->m_cEF.m_ResLookupDataMan[static_cast<i32>(cacheSource::readonly)].RemoveData(Header[0].m_CRC32);
					gRenDev->m_cEF.m_ResLookupDataMan[static_cast<i32>(cacheSource::user)].RemoveData(Header[1].m_CRC32);

					// has the shader been successfully written to the dest address
					fpDst = gEnv->pDrxPak->FOpen(szDst.c_str(), "rb", IDrxPak::FLAGS_NEVER_IN_PAK | IDrxPak::FLAGS_PATH_REAL | IDrxPak::FOPEN_ONDISK);
					if (fpDst)
					{
						SAFE_DELETE(pSHB);
						i = 1;
					}
				}
			}
		}
#endif
	}
	if (fpSrc)
		gEnv->pDrxPak->FClose(fpSrc);
	fpSrc = NULL;

	if (!CRenderer::CV_r_shadersAllowCompilation)
	{
		if (pSHB == 0 && !fpDst)
		{
			//do only perform the necessary stuff
			fpDst = gEnv->pDrxPak->FOpen(nameBin, "rb");
		}
	}
	if (pSHB == 0 && fpDst)
	{
		nameFile.Format("%s.%s", szName, szExt);
		pSHB = LoadBinShader(fpDst, nameFile.c_str(), i == 0 ? nameBin.c_str() : szDst.c_str(), !bInclude);
		gEnv->pDrxPak->FClose(fpDst);
		assert(pSHB);
	}

	if (pSHB)
	{
		if (i == 0)
			pSHB->m_bReadOnly = true;
		else
			pSHB->m_bReadOnly = false;

		AddToCache(pSHB, bInclude);
		if (!bInclude)
		{
			char nm[128];
			nm[0] = '$';
			drx_strcpy(&nm[1], sizeof(nm) - 1, szName);
			CDrxNameTSCRC NM = CParserBin::GetPlatformSpecName(nm);
			FXShaderBinPathItor it = m_BinPaths.find(NM);
			if (it == m_BinPaths.end())
				m_BinPaths.insert(FXShaderBinPath::value_type(NM, i == 0 ? string(nameBin) : szDst));
			else
				it->second = (i == 0) ? string(nameBin) : szDst;
		}
	}
	else
	{
		if (fpDst)
			Warning("Error: Failed to get binary shader '%s'", nameFile.c_str());
		else
		{
			nameFile.Format("%s.%s", szName, szExt);
			tukk matName = 0;
			if (m_pCEF && m_pCEF->m_pCurInputResources)
				matName = m_pCEF->m_pCurInputResources->m_szMaterialName;
			LogWarningEngineOnly("Error: Shader \"%s\" doesn't exist (used in material \"%s\")", nameFile.c_str(), matName != 0 ? matName : "$unknown$");
		}
	}

	/*
	   sfTotalTime += iTimer->GetAsyncCurTime() - fTime0;

	   {
	   char acTmp[1024];
	   drx_sprintf(acTmp, "Parsing of bin took: %f \n", sfTotalTime);
	   OutputDebugString(acTmp);
	   }
	 */

	return pSHB;
}

void CShaderManBin::AddGenMacros(SShaderGen* shG, CParserBin& Parser, uint64 nMaskGen)
{
	if (!nMaskGen || !shG)
		return;

	u32 dwMacro = eT_1;
	for (u32 i = 0; i < shG->m_BitMask.Num(); i++)
	{
		if (shG->m_BitMask[i]->m_Mask & nMaskGen)
		{
			Parser.AddMacro(shG->m_BitMask[i]->m_dwToken, &dwMacro, 1, shG->m_BitMask[i]->m_Mask, Parser.m_Macros[1]);
		}
	}
}

bool CShaderManBin::ParseBinFX_Global_Annotations(CParserBin& Parser, SParserFrame& Frame, bool* bPublic, CDrxNameR techStart[2])
{
	bool bRes = true;

	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(ShaderType)
		FX_TOKEN(ShaderDrawType)
		FX_TOKEN(PreprType)
		FX_TOKEN(Public)
		FX_TOKEN(NoPreview)
		FX_TOKEN(LocalConstants)
		FX_TOKEN(Cull)
		FX_TOKEN(SupportsAttrInstancing)
		FX_TOKEN(SupportsConstInstancing)
		FX_TOKEN(SupportsDeferredShading)
		FX_TOKEN(SupportsFullDeferredShading)
		FX_TOKEN(Decal)
		FX_TOKEN(DecalNoDepthOffset)
		FX_TOKEN(Sky)
		FX_TOKEN(HWTessellation)
		FX_TOKEN(ZPrePass)
		FX_TOKEN(VertexColors)
		FX_TOKEN(NoChunkMerging)
		FX_TOKEN(ForceTransPass)
		FX_TOKEN(AfterHDRPostProcess)
		FX_TOKEN(AfterPostProcess)
		FX_TOKEN(ForceZpass)
		FX_TOKEN(ForceWaterPass)
		FX_TOKEN(ForceDrawLast)
		FX_TOKEN(ForceDrawFirst)
		FX_TOKEN(Hair)
		FX_TOKEN(SkinPass)
		FX_TOKEN(ForceGeneralPass)
		FX_TOKEN(ForceDrawAfterWater)
		FX_TOKEN(DepthFixup)
		FX_TOKEN(SingleLightPass)
		FX_TOKEN(Refractive)
		FX_TOKEN(ForceRefractionUpdate)
		FX_TOKEN(WaterParticle)
		FX_TOKEN(VT_DetailBending)
		FX_TOKEN(VT_DetailBendingGrass)
		FX_TOKEN(VT_WindBending)
		FX_TOKEN(AlphaBlendShadows)
		FX_TOKEN(EyeOverlay)
		FX_TOKEN(WrinkleBlending)
		FX_TOKEN(Billboard)
	FX_END_TOKENS

	i32 nIndex;
	CShader* ef = Parser.m_pCurShader;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_Public:
			if (bPublic)
				*bPublic = true;
			break;

		case eT_NoPreview:
			if (!ef)
				break;
			ef->m_Flags |= EF_NOPREVIEW;
			break;

		case eT_Decal:
			if (!ef)
				break;
			ef->m_Flags |= EF_DECAL;
			ef->m_nMDV |= MDV_DEPTH_OFFSET;
			break;

		case eT_DecalNoDepthOffset:
			if (!ef)
				break;
			ef->m_Flags |= EF_DECAL;
			break;

		case eT_Sky:
			if (!ef)
				break;
			break;

		case eT_LocalConstants:
			if (!ef)
				break;
			ef->m_Flags |= EF_LOCALCONSTANTS;
			break;

		case eT_VT_DetailBending:
			if (!ef)
				break;
			ef->m_nMDV |= MDV_DET_BENDING;
			break;
		case eT_VT_DetailBendingGrass:
			if (!ef)
				break;
			ef->m_nMDV |= MDV_DET_BENDING | MDV_DET_BENDING_GRASS;
			break;
		case eT_VT_WindBending:
			if (!ef)
				break;
			ef->m_nMDV |= MDV_WIND;
			break;

		case eT_NoChunkMerging:
			if (!ef)
				break;
			ef->m_Flags |= EF_NOCHUNKMERGING;
			break;

		case eT_SupportsAttrInstancing:
			if (!ef)
				break;
			if (gRenDev->m_bDeviceSupportsInstancing)
				ef->m_Flags |= EF_SUPPORTSINSTANCING_ATTR;
			break;
		case eT_SupportsConstInstancing:
			if (!ef)
				break;
			if (gRenDev->m_bDeviceSupportsInstancing)
				ef->m_Flags |= EF_SUPPORTSINSTANCING_CONST;
			break;

		case eT_SupportsDeferredShading:
			if (!ef)
				break;
			ef->m_Flags |= EF_SUPPORTSDEFERREDSHADING_MIXED;
			break;

		case eT_SupportsFullDeferredShading:
			if (!ef)
				break;
			ef->m_Flags |= EF_SUPPORTSDEFERREDSHADING_FULL;
			break;

		case eT_ForceTransPass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_TRANSPASS;
			break;

		case eT_AfterHDRPostProcess:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_AFTERHDRPOSTPROCESS;
			break;

		case eT_AfterPostProcess:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_AFTERPOSTPROCESS;
			break;

		case eT_ForceZpass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_ZPASS;
			break;

		case eT_ForceWaterPass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_WATERPASS;
			break;

		case eT_ForceDrawLast:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_DRAWLAST;
			break;
		case eT_ForceDrawFirst:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_DRAWFIRST;
			break;

		case eT_Hair:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_HAIR;
			break;

		case eT_ForceGeneralPass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_GENERALPASS;
			break;

		case eT_ForceDrawAfterWater:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_FORCE_DRAWAFTERWATER;
			break;
		case eT_DepthFixup:
#if !DRX_PLATFORM_ORBIS
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_DEPTH_FIXUP;
#endif
			break;
		case eT_SingleLightPass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_SINGLELIGHTPASS;
			break;
		case eT_WaterParticle:
			if (!ef)
				break;
			ef->m_Flags |= EF_WATERPARTICLE;
			break;
		case eT_Refractive:
			if (!ef)
				break;
			ef->m_Flags |= EF_REFRACTIVE;
			break;
		case eT_ForceRefractionUpdate:
			if (!ef)
				break;
			ef->m_Flags |= EF_FORCEREFRACTIONUPDATE;
			break;

		case eT_ZPrePass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_ZPREPASS;
			break;

		case eT_HWTessellation:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_HW_TESSELLATION;
			break;

		case eT_AlphaBlendShadows:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_ALPHABLENDSHADOWS;
			break;

		case eT_SkinPass:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_SKINPASS;
			break;

		case eT_EyeOverlay:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_EYE_OVERLAY;
			break;

		case eT_VertexColors:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_VERTEXCOLORS;
			break;

		case eT_Billboard:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_BILLBOARD;
			break;

		case eT_WrinkleBlending:
			if (!ef)
				break;
			ef->m_Flags2 |= EF2_WRINKLE_BLENDING;
			break;

		case eT_ShaderDrawType:
			{
				if (!ef)
					break;
				eT = Parser.GetToken(Parser.m_Data);
				if (eT == eT_Light)
					ef->m_eSHDType = eSHDT_Light;
				else if (eT == eT_Shadow)
					ef->m_eSHDType = eSHDT_Shadow;
				else if (eT == eT_Fur)
					ef->m_eSHDType = eSHDT_Fur;
				else if (eT == eT_General)
					ef->m_eSHDType = eSHDT_General;
				else if (eT == eT_Terrain)
					ef->m_eSHDType = eSHDT_Terrain;
				else if (eT == eT_Overlay)
					ef->m_eSHDType = eSHDT_Overlay;
				else if (eT == eT_NoDraw)
				{
					ef->m_eSHDType = eSHDT_NoDraw;
					ef->m_Flags |= EF_NODRAW;
				}
				else if (eT == eT_Custom)
					ef->m_eSHDType = eSHDT_CustomDraw;
				else if (eT == eT_Sky)
				{
					ef->m_eSHDType = eSHDT_Sky;
					ef->m_Flags |= EF_SKY;
				}
				else if (eT == eT_OceanShore)
					ef->m_eSHDType = eSHDT_OceanShore;
				else if (eT == eT_DebugHelper)
					ef->m_eSHDType = eSHDT_DebugHelper;
				else
				{
					Warning("Unknown shader draw type '%s'", Parser.GetString(eT));
					assert(0);
				}
			}
			break;

		case eT_ShaderType:
			{
				if (!ef)
					break;
				eT = Parser.GetToken(Parser.m_Data);
				if (eT == eT_General)
					ef->m_eShaderType = eST_General;
				else if (eT == eT_Metal)
					ef->m_eShaderType = eST_Metal;
				else if (eT == eT_Ice)
					ef->m_eShaderType = eST_Ice;
				else if (eT == eT_Shadow)
					ef->m_eShaderType = eST_Shadow;
				else if (eT == eT_Water)
					ef->m_eShaderType = eST_Water;
				else if (eT == eT_FX)
					ef->m_eShaderType = eST_FX;
				else if (eT == eT_HUD3D)
					ef->m_eShaderType = eST_HUD3D;
				else if (eT == eT_PostProcess)
					ef->m_eShaderType = eST_PostProcess;
				else if (eT == eT_HDR)
					ef->m_eShaderType = eST_HDR;
				else if (eT == eT_Sky)
					ef->m_eShaderType = eST_Sky;
				else if (eT == eT_Glass)
					ef->m_eShaderType = eST_Glass;
				else if (eT == eT_Vegetation)
					ef->m_eShaderType = eST_Vegetation;
				else if (eT == eT_Particle)
					ef->m_eShaderType = eST_Particle;
				else if (eT == eT_Terrain)
					ef->m_eShaderType = eST_Terrain;
				else if (eT == eT_Compute)
					ef->m_eShaderType = eST_Compute;
				else
				{
					Warning("Unknown shader type '%s'", Parser.GetString(eT));
					assert(0);
				}
			}
			break;

		case eT_PreprType:
			{
				if (!ef)
					break;
				eT = Parser.GetToken(Parser.m_Data);
				if (eT == eT_ScanWater)
					ef->m_Flags2 |= EF2_PREPR_SCANWATER;
				else
				{
					Warning("Unknown preprocess type '%s'", Parser.GetString(eT));
					assert(0);
				}
			}
			break;

		case eT_Cull:
			{
				if (!ef)
					break;
				eT = Parser.GetToken(Parser.m_Data);
				if (eT == eT_None || eT == eT_NONE)
					ef->m_eCull = eCULL_None;
				else if (eT == eT_CCW || eT == eT_Back)
					ef->m_eCull = eCULL_Back;
				else if (eT == eT_CW || eT == eT_Front)
					ef->m_eCull = eCULL_Front;
				else
					assert(0);
			}
			break;

		default:
			assert(0);
		}
	}

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_Global(CParserBin& Parser, SParserFrame& Frame, bool* bPublic, CDrxNameR techStart[2])
{
	bool bRes = true;

	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(string)
	FX_END_TOKENS

	i32 nIndex;
	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_string:
			{
				eT = Parser.GetToken(Parser.m_Name);
				assert(eT == eT_Script);
				bRes &= ParseBinFX_Global_Annotations(Parser, Parser.m_Data, bPublic, techStart);
			}
			break;

		default:
			assert(0);
		}
	}

	Parser.EndFrame(OldFrame);

	return bRes;
}

static ESamplerAddressMode sGetTAddress(u32 nToken)
{
	switch (nToken)
	{
	case eT_Clamp:
		return eSamplerAddressMode_Clamp;
	case eT_Border:
		return eSamplerAddressMode_Border;
	case eT_Wrap:
		return eSamplerAddressMode_Wrap;
	case eT_Mirror:
		return eSamplerAddressMode_Mirror;
	default:
		assert(0);
	}

	return eSamplerAddressMode_Clamp;
}

bool CShaderManBin::ParseBinFX_Texture_Annotations_Script(CParserBin& Parser, SParserFrame& Frame, SFXTexture* pTexture)
{
	bool bRes = true;

	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(RenderOrder)
		FX_TOKEN(ProcessOrder)
		FX_TOKEN(RenderCamera)
		FX_TOKEN(RenderType)
		FX_TOKEN(RenderFilter)
		FX_TOKEN(RenderColorTarget1)
		FX_TOKEN(RenderDepthStencilTarget)
		FX_TOKEN(ClearSetColor)
		FX_TOKEN(ClearSetDepth)
		FX_TOKEN(ClearTarget)
		FX_TOKEN(RenderTarget_IDPool)
		FX_TOKEN(RenderTarget_UpdateType)
		FX_TOKEN(RenderTarget_Width)
		FX_TOKEN(RenderTarget_Height)
		FX_TOKEN(GenerateMips)
		FX_END_TOKENS

		SHRenderTarget * pRt = new SHRenderTarget;
	i32 nIndex;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_RenderOrder:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_PreProcess)
				pRt->m_eOrder = eRO_PreProcess;
			else if (eT == eT_PostProcess)
				pRt->m_eOrder = eRO_PostProcess;
			else if (eT == eT_PreDraw)
				pRt->m_eOrder = eRO_PreDraw;
			else
			{
				assert(0);
				Warning("Unknown RenderOrder type '%s'", Parser.GetString(eT));
			}
		}
		break;

		case eT_ProcessOrder:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_WaterReflection)
				pRt->m_nProcessFlags = FSPR_SCANTEXWATER;
			else
			{
				assert(0);
				Warning("Unknown ProcessOrder type '%s'", Parser.GetString(eT));
			}
		}
		break;

		case eT_RenderCamera:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_WaterPlaneReflected)
				pRt->m_nFlags |= FRT_CAMERA_REFLECTED_WATERPLANE;
			else if (eT == eT_PlaneReflected)
				pRt->m_nFlags |= FRT_CAMERA_REFLECTED_PLANE;
			else if (eT == eT_Current)
				pRt->m_nFlags |= FRT_CAMERA_CURRENT;
			else
			{
				assert(0);
				Warning("Unknown RenderCamera type '%s'", Parser.GetString(eT));
			}
		}
		break;

		case eT_GenerateMips:
			pRt->m_nFlags |= FRT_GENERATE_MIPS;
			break;

		case eT_RenderType:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_CurObject)
				pRt->m_nFlags |= 0;
			else if (eT == eT_CurScene)
				pRt->m_nFlags |= FRT_RENDTYPE_CURSCENE;
			else if (eT == eT_RecursiveScene)
				pRt->m_nFlags |= FRT_RENDTYPE_RECURSIVECURSCENE;
			else if (eT == eT_CopyScene)
				pRt->m_nFlags |= FRT_RENDTYPE_COPYSCENE;
			else
			{
				assert(0);
				Warning("Unknown RenderType type '%s'", Parser.GetString(eT));
			}
		}
		break;

		case eT_RenderFilter:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_Refractive)
				pRt->m_nFilterFlags |= FRF_REFRACTIVE;
			else if (eT == eT_Heat)
				pRt->m_nFilterFlags |= FRF_HEAT;
			else
			{
				assert(0);
				Warning("Unknown RenderFilter type '%s'", Parser.GetString(eT));
			}
		}
		break;

		case eT_RenderDepthStencilTarget:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_DepthBuffer || eT == eT_DepthBufferTemp)
				pRt->m_bTempDepth = true;
			else if (eT == eT_DepthBufferOrig)
				pRt->m_bTempDepth = false;
			else
			{
				assert(0);
				Warning("Unknown RenderDepthStencilTarget type '%s'", Parser.GetString(eT));
			}
		}
		break;

		case eT_RenderTarget_IDPool:
			pRt->m_nIDInPool = Parser.GetInt(Parser.GetToken(Parser.m_Data));
			assert(pRt->m_nIDInPool >= 0 && pRt->m_nIDInPool < 64);
			break;

		case eT_RenderTarget_Width:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_$ScreenSize)
				pRt->m_nWidth = -1;
			else
				pRt->m_nWidth = Parser.GetInt(eT);
		}
		break;

		case eT_RenderTarget_Height:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_$ScreenSize)
				pRt->m_nHeight = -1;
			else
				pRt->m_nHeight = Parser.GetInt(eT);
		}
		break;

		case eT_RenderTarget_UpdateType:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_WaterReflect)
				pRt->m_eUpdateType = eRTUpdate_WaterReflect;
			else if (eT == eT_Allways)
				pRt->m_eUpdateType = eRTUpdate_Always;
			else
				assert(0);
		}
		break;

		case eT_ClearSetColor:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_FogColor)
				pRt->m_nFlags |= FRT_CLEAR_FOGCOLOR;
			else
			{
				string sStr = Parser.GetString(Parser.m_Data);
				shGetColor(sStr.c_str(), pRt->m_ClearColor);
			}
		}
		break;

		case eT_ClearSetDepth:
			pRt->m_fClearDepth = Parser.GetFloat(Parser.m_Data);
			break;

		case eT_ClearTarget:
		{
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_Color)
				pRt->m_nFlags |= FRT_CLEAR_COLOR;
			else if (eT == eT_Depth)
				pRt->m_nFlags |= FRT_CLEAR_DEPTH;
			else
			{
				assert(0);
				Warning("Unknown ClearTarget type '%s'", Parser.GetString(eT));
			}
		}
		break;

		default:
			assert(0);
		}
	}
	if (pRt->m_eOrder == eRO_PreProcess)
		Parser.m_pCurShader->m_Flags |= EF_PRECACHESHADER;
	pTexture->m_pTarget = pRt;
// 	pTexture->PostLoad();

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_Texture_Annotations(CParserBin& Parser, SParserFrame& Annotations, SFXTexture* pTexture)
{
	bool bRes = true;

	SParserFrame OldFrame = Parser.BeginFrame(Annotations);

	FX_BEGIN_TOKENS
		FX_TOKEN(string)
		FX_TOKEN(UIName)
		FX_TOKEN(UIDescription)
		FX_TOKEN(sRGBLookup)
		FX_TOKEN(Global)
	FX_END_TOKENS

		i32 nIndex = 0;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		SCodeFragment Fr;
		switch (eT)
		{
		case eT_string:
		{
			eT = Parser.GetToken(Parser.m_Name);
			assert(eT == eT_Script);
			bRes &= ParseBinFX_Texture_Annotations_Script(Parser, Parser.m_Data, pTexture);
		}
		break;
#if SHADER_REFLECT_TEXTURE_SLOTS
		case eT_UIName:
		{
			SParserFrame stringData = Parser.m_Data;

			if (Parser.GetToken(stringData) == eT_quote)
			{
				stringData.m_nFirstToken++;
				stringData.m_nLastToken--;
			}
			pTexture->m_szUIName = Parser.GetString(stringData);
		}
		break;
		case eT_UIDescription:
		{
			SParserFrame stringData = Parser.m_Data;

			if (Parser.GetToken(stringData) == eT_quote)
			{
				stringData.m_nFirstToken++;
				stringData.m_nLastToken--;
			}
			pTexture->m_szUIDesc = Parser.GetString(stringData);
		}
		break;
#else
		case eT_UIName:
		case eT_UIDescription:
			break;
#endif

		case eT_sRGBLookup:
			pTexture->m_bSRGBLookup = Parser.GetBool(Parser.m_Data);
			break;

		case eT_Global:
			pTexture->m_nFlags |= PF_GLOBAL;
			break;

		default:
			assert(0);
		}
	}

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_Texture(CParserBin& Parser, SParserFrame& Frame, SFXTexture& Tex, SParserFrame Annotations)
{
	bool bRes = true;

	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(Texture)

		FX_TOKEN(slot)
		FX_TOKEN(vsslot)
		FX_TOKEN(psslot)
		FX_TOKEN(hsslot)
		FX_TOKEN(dsslot)
		FX_TOKEN(gsslot)
		FX_TOKEN(csslot)
	FX_END_TOKENS

	i32 nIndex = -1;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_Texture:
			{
				Tex.m_szTexture = Parser.GetString(Parser.m_Data);
			}
			break;

		case eT_slot:
		case eT_vsslot:
		case eT_psslot:
		case eT_hsslot:
		case eT_dsslot:
		case eT_gsslot:
		case eT_csslot:
			break;

		default:
			assert(0);
		}
	}

	if (!Annotations.IsEmpty())
		bRes &= ParseBinFX_Texture_Annotations(Parser, Annotations, &Tex);

	Parser.EndFrame(OldFrame);

	return bRes;
}

void CShaderManBin::AddAffectedParameter(CParserBin& Parser, std::vector<SFXParam>& AffectedParams, TArray<i32>& AffectedFunc, SFXParam* pParam, EHWShaderClass eSHClass, u32 dwType, SShaderTechnique* pShTech)
{
	if (!CParserBin::PlatformSupportsConstantBuffers())
	{
		//if (pParam->m_Name.c_str()[0] != '_')
		{
			if (!(Parser.m_pCurShader->m_Flags & EF_LOCALCONSTANTS))
			{
				if (pParam->m_nRegister[eSHClass] >= 0 && pParam->m_nRegister[eSHClass] < 10000)
				{
					if ((pShTech->m_Flags & FHF_NOLIGHTS) && pParam->m_nCB == CB_PER_MATERIAL && !(pParam->m_nFlags & PF_TWEAKABLE_MASK))
						return;
					if (pParam->m_Semantic.empty() && pParam->m_Values.c_str()[0] == '(')
						pParam->m_nCB = CB_PER_MATERIAL;
					AffectedParams.push_back(*pParam);
					return;
				}
			}
		}
	}
	else if (pParam->m_nCB == CB_PER_MATERIAL)
	{
		if (pParam->m_nRegister[eSHClass] < 0 || pParam->m_nRegister[eSHClass] >= 1000)
			return;
	}

	i32 nFlags = pParam->GetParamFlags();
	bool bCheckAffect = CParserBin::m_bParseFX ? true : false;
	if (nFlags & PF_ALWAYS)
		AffectedParams.push_back(*pParam);
	else
	{
		if (CParserBin::m_nPlatform & (SF_D3D11 | SF_DURANGO | SF_ORBIS | SF_GL4 | SF_GLES3 | SF_VULKAN))
		{
			assert(eSHClass < eHWSC_Num);
			if (((nFlags & PF_TWEAKABLE_MASK) || pParam->m_Values.c_str()[0] == '(') && pParam->m_nRegister[eSHClass] >= 0 && pParam->m_nRegister[eSHClass] < 1000)
				bCheckAffect = false;
		}
		for (u32 j = 0; j < AffectedFunc.size(); j++)
		{
			SCodeFragment* pCF = &Parser.m_CodeFragments[AffectedFunc[j]];
			if (!bCheckAffect || Parser.FindToken(pCF->m_nFirstToken, pCF->m_nLastToken, pParam->m_dwName[0]) >= 0)
			{
				AffectedParams.push_back(*pParam);
				break;
			}
		}
	}
}

void CShaderManBin::InitShaderDependenciesList(CParserBin& Parser, SCodeFragment* pFunc, TArray<byte>& bChecked, TArray<i32>& AffectedFunc)
{
	u32 i;
	u32k numFrags = Parser.m_CodeFragments.size();

	for (i = 0; i < numFrags; ++i)
	{
		if (bChecked[i])
			continue;
		SCodeFragment* s = &Parser.m_CodeFragments[i];
		if (!s->m_dwName)
		{
			bChecked[i] = 1;
			continue;
		}
		if (s->m_eType == eFT_Function || s->m_eType == eFT_StorageClass)
		{
			if (Parser.FindToken(pFunc->m_nFirstToken, pFunc->m_nLastToken, s->m_dwName) >= 0)
			{
				bChecked[i] = 1;
				AffectedFunc.push_back(i);
				InitShaderDependenciesList(Parser, s, bChecked, AffectedFunc);
			}
		}
	}
}

void CShaderManBin::AddAffectedSampler(CParserBin& Parser, std::vector<SFXSampler>& AffectedSamplers, TArray<i32>& AffectedFunc, SFXSampler* pSampler, EHWShaderClass eSHClass, u32 dwType, SShaderTechnique* pShTech)
{
	for (u32 j = 0; j < AffectedFunc.size(); j++)
	{
		SCodeFragment* pCF = &Parser.m_CodeFragments[AffectedFunc[j]];
		if (Parser.FindToken(pCF->m_nFirstToken, pCF->m_nLastToken, pSampler->m_dwName[0]) >= 0)
		{
			AffectedSamplers.push_back(*pSampler);
			break;
		}
	}
}
void CShaderManBin::AddAffectedTexture(CParserBin& Parser, std::vector<SFXTexture>& AffectedTextures, TArray<i32>& AffectedFunc, SFXTexture* pTexture, EHWShaderClass eSHClass, u32 dwType, SShaderTechnique* pShTech)
{
	for (u32 j = 0; j < AffectedFunc.size(); j++)
	{
		SCodeFragment* pCF = &Parser.m_CodeFragments[AffectedFunc[j]];
		if (Parser.FindToken(pCF->m_nFirstToken, pCF->m_nLastToken, pTexture->m_dwName[0]) >= 0)
		{
			AffectedTextures.push_back(*pTexture);
			break;
		}
	}
}

void CShaderManBin::CheckStructuresDependencies(CParserBin& Parser, SCodeFragment* pFrag, TArray<byte>& bChecked, TArray<i32>& AffectedFunc)
{
	u32 i;
	u32k numFrags = Parser.m_CodeFragments.size();

	for (i = 0; i < numFrags; ++i)
	{
		if (bChecked[i])
			continue;
		SCodeFragment* s = &Parser.m_CodeFragments[i];
		if (s->m_eType == eFT_Structure)
		{
			if (Parser.FindToken(pFrag->m_nFirstToken, pFrag->m_nLastToken, s->m_dwName) >= 0)
			{
				bChecked[i] = 1;
				AffectedFunc.push_back(i);
				CheckStructuresDependencies(Parser, s, bChecked, AffectedFunc);
			}
		}
	}
}

void CShaderManBin::CheckFragmentsDependencies(CParserBin& Parser, TArray<byte>& bChecked, TArray<i32>& AffectedFrags)
{
	u32 i, j;
	u32k numFuncs = AffectedFrags.size();
	u32k numFrags = Parser.m_CodeFragments.size();

	for (i = 0; i < numFuncs; ++i)
	{
		i32 nFunc = AffectedFrags[i];
		SCodeFragment* pFunc = &Parser.m_CodeFragments[nFunc];
		for (j = 0; j < numFrags; j++)
		{
			SCodeFragment* s = &Parser.m_CodeFragments[j];
			if (bChecked[j])
				continue;

			if (s->m_eType == eFT_Sampler || s->m_eType == eFT_Structure)
			{
				if (Parser.FindToken(pFunc->m_nFirstToken, pFunc->m_nLastToken, s->m_dwName) >= 0)
				{
					bChecked[j] = 1;
					AffectedFrags.push_back(j);
					if (s->m_eType == eFT_Structure)
						CheckStructuresDependencies(Parser, s, bChecked, AffectedFrags);
				}
			}
			else if (s->m_eType == eFT_ConstBuffer)
			{
				// Make sure cbuffer declaration does not get stripped
				bChecked[j] = 1;
				AffectedFrags.push_back(j);
			}
		}
	}
}

//=================================================================================================

struct SFXRegisterBin
{
	i32        nReg;
	i32        nComp;
	i32        m_nCB;
	u32     m_nFlags;
	EParamType eType;
	u32     dwName;
	CDrxNameR  m_Value;

	SFXRegisterBin()
		: nReg(0)
		, nComp(0)
		, m_nCB(-1)
		, m_nFlags(0)
		, eType(eType_UNKNOWN)
		, dwName(0)
		, m_Value()
	{
	}
};

struct SFXPackedName
{
	u32 dwName[4];

	SFXPackedName()
	{
		dwName[0] = dwName[1] = dwName[2] = dwName[3] = 0;
	}
};

inline bool Compar(const SFXRegisterBin& a, const SFXRegisterBin& b)
{
	if (CParserBin::PlatformSupportsConstantBuffers())
	{
		if (a.m_nCB != b.m_nCB)
			return a.m_nCB < b.m_nCB;
	}
	if (a.nReg != b.nReg)
		return a.nReg < b.nReg;
	if (a.nComp != b.nComp)
		return a.nComp < b.nComp;
	return false;
}

static void sFlushRegs(CParserBin& Parser, i32 nReg, SFXRegisterBin* MergedRegs[4], std::vector<SFXParam>& NewParams, EHWShaderClass eSHClass, std::vector<SFXPackedName>& PackedNames)
{
	NewParams.resize(NewParams.size() + 1);
	PackedNames.resize(PackedNames.size() + 1);
	SFXParam& p = NewParams[NewParams.size() - 1];
	SFXPackedName& pN = PackedNames[PackedNames.size() - 1];
	i32 nMaxComp = -1;
	i32 j;

	i32 nCompMerged = 0;
	EParamType eType = eType_UNKNOWN;
	i32 nCB = -1;
	for (j = 0; j < 4; j++)
	{
		if (MergedRegs[j] && MergedRegs[j]->eType != eType_UNKNOWN)
		{
			if (nCB == -1)
				nCB = MergedRegs[j]->m_nCB;
			else if (nCB != MergedRegs[j]->m_nCB)
			{
				assert(0);
			}
			if (eType == eType_UNKNOWN)
				eType = MergedRegs[j]->eType;
			else if (eType != MergedRegs[j]->eType)
			{
				//assert(0);
			}
		}
	}
	for (j = 0; j < 4; j++)
	{
		char s[16];
		drx_sprintf(s, "__%d", j);
		p.m_Name = stack_string(p.m_Name.c_str()) + s;
		if (MergedRegs[j])
		{
			if (MergedRegs[j]->m_nFlags & PF_TWEAKABLE_MASK)
				p.m_nFlags |= (PF_TWEAKABLE_0 << j);
			p.m_nFlags |= MergedRegs[j]->m_nFlags & ~(PF_TWEAKABLE_MASK | PF_SCALAR | PF_SINGLE_COMP | PF_MERGE_MASK);
			nMaxComp = max(nMaxComp, j);
			pN.dwName[j] = MergedRegs[j]->dwName;
			if (nCompMerged)
			{
				p.m_Values = stack_string(p.m_Values.c_str()) += ", ";
			}
			p.m_Name = stack_string(p.m_Name.c_str()) + Parser.GetString(pN.dwName[j]);
			p.m_Values = stack_string(p.m_Values.c_str()) + MergedRegs[j]->m_Value.c_str();
			nCompMerged++;
		}
		else
		{
			if (nCompMerged)
				p.m_Values = stack_string(p.m_Values.c_str()) + ", ";
			p.m_Values = stack_string(p.m_Values.c_str()) + "NULL";
			nCompMerged++;
		}
	}
	p.m_nComps = nMaxComp + 1;
	if (eSHClass == eHWSC_Geometry && !CParserBin::PlatformSupportsGeometryShaders())
		return;
	if (eSHClass == eHWSC_Domain && !CParserBin::PlatformSupportsDomainShaders())
		return;
	if (eSHClass == eHWSC_Hull && !CParserBin::PlatformSupportsHullShaders())
		return;
	if (eSHClass == eHWSC_Compute && !CParserBin::PlatformSupportsComputeShaders())
		return;

	// Get packed name token.
	p.m_dwName.push_back(Parser.NewUserToken(eT_unknown, p.m_Name.c_str(), true)); //pass by drxsting so that string references work.
	assert(eSHClass < eHWSC_Num);
	p.m_nRegister[eSHClass] = nReg;
	if (eSHClass == eHWSC_Domain || eSHClass == eHWSC_Hull || eSHClass == eHWSC_Compute)
		p.m_nRegister[eHWSC_Vertex] = nReg;
	p.m_nParameters = 1;
	if (eType == eType_HALF)
		eType = eType_FLOAT;
	p.m_eType = eType;
	assert(eType != eType_UNKNOWN);
	p.m_nCB = nCB;
	p.m_nFlags |= PF_AUTOMERGED;
}

static EToken sCompToken[4] = { eT_x, eT_y, eT_z, eT_w };
bool CShaderManBin::ParseBinFX_Technique_Pass_PackParameters(CParserBin& Parser, std::vector<SFXParam>& AffectedParams, TArray<i32>& AffectedFunc, SCodeFragment* pFunc, EHWShaderClass eSHClass, u32 dwSHName, std::vector<SFXParam>& PackedParams, TArray<SCodeFragment>& Replaces, TArray<u32>& NewTokens, TArray<byte>& bMerged)
{
	bool bRes = true;

	u32 i, j;

	std::vector<SFXRegisterBin> Registers;
	std::vector<SFXPackedName> PackedNames;
	u32 nMergeMask = (eSHClass == eHWSC_Pixel) ? 1 : 2;
	for (i = 0; i < AffectedParams.size(); i++)
	{
		SFXParam* pr = &AffectedParams[i];
		if (pr->m_Annotations.empty())
			continue;  // Parameter doesn't have custom register definition
		if (bMerged[i] & nMergeMask)
			continue;
		bMerged[i] |= nMergeMask;
		tuk src = (tuk)pr->m_Annotations.c_str();
		char annot[256];
		while (true)
		{
			fxFill(&src, annot);
			if (!annot[0])
				break;

			tuk b = NULL;
			if (!strncmp(&annot[0], "register", 8))
			{
				b = &annot[8];
			}
			else
			{
				if (strncmp(&annot[1], "sregister", 9))
					break;
				if ((eSHClass == eHWSC_Pixel && annot[0] != 'p') || (eSHClass == eHWSC_Vertex && annot[0] != 'v')
				    || (eSHClass == eHWSC_Geometry && annot[0] != 'g') || (eSHClass == eHWSC_Hull && annot[0] != 'h') || (eSHClass == eHWSC_Domain && annot[0] != 'd') || (eSHClass == eHWSC_Compute && annot[0] != 'c'))
					continue;
				b = &annot[10];
				assert(pr->m_nCB != CB_PER_MATERIAL);
			}

			SkipCharacters(&b, kWhiteSpace);
			assert(b[0] == '=');
			b++;
			SkipCharacters(&b, kWhiteSpace);
			assert(b[0] == 'c');
			if (b[0] == 'c')
			{
				i32 nReg = atoi(&b[1]);
				b++;
				while (*b != '.')
				{
					if (*b == 0) // Vector without swizzling
						break;
					b++;
				}
				if (*b == '.')
				{
					bMerged[i] |= (nMergeMask << 4);
					b++;
					i32 nComp = -1;
					switch (b[0])
					{
					case 'x':
					case 'r':
						nComp = 0;
						break;
					case 'y':
					case 'g':
						nComp = 1;
						break;
					case 'z':
					case 'b':
						nComp = 2;
						break;
					case 'w':
					case 'a':
						nComp = 3;
						break;
					default:
						assert(0);
					}
					if (nComp >= 0)
					{
						pr->m_nFlags |= (nMergeMask << PF_MERGE_SHIFT);
						SFXRegisterBin rg;
						rg.nReg = nReg;
						rg.nComp = nComp;
						rg.dwName = pr->m_dwName[0];
						rg.m_Value = pr->m_Values;
						rg.m_nFlags = pr->m_nFlags;
						rg.eType = (EParamType)pr->m_eType;
						rg.m_nCB = pr->m_nCB;
						assert(rg.m_Value.c_str()[0]);
						Registers.push_back(rg);
					}
				}
			}
			break;
		}
	}
	if (Registers.empty())
		return false;
	std::sort(Registers.begin(), Registers.end(), Compar);
	i32 nReg = -1;
	i32 nCB = -1;
	SFXRegisterBin* MergedRegs[4];
	MergedRegs[0] = MergedRegs[1] = MergedRegs[2] = MergedRegs[3] = NULL;
	for (i = 0; i < Registers.size(); i++)
	{
		SFXRegisterBin* rg = &Registers[i];
		if ((!CParserBin::PlatformSupportsConstantBuffers() && rg->nReg != nReg) || (CParserBin::PlatformSupportsConstantBuffers() && (rg->m_nCB != nCB || rg->nReg != nReg)))
		{
			if (nReg >= 0)
				sFlushRegs(Parser, nReg, MergedRegs, PackedParams, eSHClass, PackedNames);
			nReg = rg->nReg;
			nCB = rg->m_nCB;
			MergedRegs[0] = MergedRegs[1] = MergedRegs[2] = MergedRegs[3] = NULL;
		}
		assert(!MergedRegs[rg->nComp]);
		if (MergedRegs[rg->nComp])
		{
			Warning("register c%d (comp: %d) is used by the %s shader '%s' already", rg->nReg, rg->nComp, eSHClass == eHWSC_Pixel ? "pixel" : "vertex", Parser.GetString(dwSHName));
			assert(0);
		}
		MergedRegs[rg->nComp] = rg;
	}
	if (MergedRegs[0] || MergedRegs[1] || MergedRegs[2] || MergedRegs[3])
		sFlushRegs(Parser, nReg, MergedRegs, PackedParams, eSHClass, PackedNames);

	// Replace new parameters in shader tokens
	for (u32 n = 0; n < AffectedFunc.size(); n++)
	{
		DRX_ASSERT_MESSAGE(AffectedFunc[n] < Parser.m_CodeFragments.size(), "function index is larger than number of CodeFragments!");

		SCodeFragment* st = &Parser.m_CodeFragments[AffectedFunc[n]];
		//tukk szName = Parser.GetString(st->m_dwName);

		for (i = 0; i < PackedParams.size(); i++)
		{
			SFXParam* pr = &PackedParams[i];
			SFXPackedName* pn = &PackedNames[i];
			for (j = 0; j < 4; j++)
			{
				u32 nP = pn->dwName[j];
				if (nP == 0)
					continue;
				i32 nPos = st->m_nFirstToken;
				while (true)
				{
					nPos = Parser.FindToken(nPos, st->m_nLastToken, nP);
					if (nPos < 0)
						break;
					SCodeFragment Fr;
					Fr.m_nFirstToken = Fr.m_nLastToken = nPos;
					Fr.m_dwName = n;
					Replaces.push_back(Fr);

					Fr.m_nFirstToken = NewTokens.size();
					NewTokens.push_back(pr->m_dwName[0]);
					NewTokens.push_back(eT_dot);
					NewTokens.push_back(sCompToken[j]);
					Fr.m_nLastToken = NewTokens.size() - 1;
					Replaces.push_back(Fr);
					nPos++;
				}
			}
		}
	}

	for (i = 0; i < Replaces.size(); i += 2)
	{
		SCodeFragment* pFR1 = &Replaces[i];
		for (j = i + 2; j < Replaces.size(); j += 2)
		{
			SCodeFragment* pFR2 = &Replaces[j];
			if (pFR1->m_dwName != pFR2->m_dwName)
				continue;
			if (pFR2->m_nFirstToken < pFR1->m_nFirstToken)
			{
				std::swap(Replaces[i], Replaces[j]);
				std::swap(Replaces[i + 1], Replaces[j + 1]);
			}
		}
	}

	return bRes;
}

//===================================================================================================

template<i32 N>
inline bool CompareVars(const SFXParam* a, const SFXParam* b)
{
	u16k nCB0 = a->m_nCB;
	u16k nCB1 = b->m_nCB;
	i16k nReg0 = a->m_nRegister[N];
	i16k nReg1 = b->m_nRegister[N];

	if (nCB0 != nCB1)
		return (nCB0 < nCB1);
	return (nReg0 < nReg1);
}

EToken dwNamesCB[CB_NUM] = { eT_PER_BATCH, eT_PER_MATERIAL };

void CShaderManBin::AddParameterToScript(CParserBin& Parser, SFXParam* pr, PodArray<u32>& SHData, EHWShaderClass eSHClass, i32 nCB)
{
	char str[256];
	i32 nReg = pr->m_nRegister[eSHClass];
	if (pr->m_eType == eType_BOOL)
		SHData.push_back(eT_bool);
	else if (pr->m_eType == eType_INT)
		SHData.push_back(eT_int);
	else
	{
		i32 nVal = pr->m_nParameters * 4 + pr->m_nComps;
		EToken eT = eT_unknown;
		switch (nVal)
		{
		case 4 + 1:
			eT = (pr->m_eType == eType_FLOAT) ? eT_float : eT_half;
			break;
		case 4 + 2:
			//eT = eT_float2;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float2 : eT_half2;
			break;
		case 4 + 3:
			//eT = eT_float3;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float3 : eT_half3;
			break;
		case 4 + 4:
			//eT = eT_float4;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float4 : eT_half4;
			break;
		case 2 * 4 + 4:
			//eT = eT_float2x4;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float2x4 : eT_half2x4;
			break;
		case 3 * 4 + 4:
			//eT = eT_float3x4;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float3x4 : eT_half3x4;
			break;
		case 4 * 4 + 4:
			//eT = eT_float4x4;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float4x4 : eT_half4x4;
			break;
		case 3 * 4 + 3:
			//eT = eT_float3x3;
			eT = (pr->m_eType == eType_FLOAT) ? eT_float3x3 : eT_half3x3;
			break;
		}
		assert(eT != eT_unknown);
		if (eT == eT_unknown)
			return;
		SHData.push_back(eT);
	}
	for (u32 i = 0; i < pr->m_dwName.size(); i++)
		SHData.push_back(pr->m_dwName[i]);

	if (nReg >= 0 && nReg < 10000)
	{
		SHData.push_back(eT_colon);
		if (nCB == CB_PER_MATERIAL)
			SHData.push_back(eT_packoffset);
		else
			SHData.push_back(eT_register);
		SHData.push_back(eT_br_rnd_1);
		drx_sprintf(str, "c%d", nReg);
		SHData.push_back(Parser.NewUserToken(eT_unknown, str, true));
		SHData.push_back(eT_br_rnd_2);
	}
	SHData.push_back(eT_semicolumn);
}

void CShaderManBin::AddSamplerToScript(CParserBin& Parser, SFXSampler* pr, PodArray<u32>& SHData, EHWShaderClass eSHClass)
{
	char str[256];
	i32 nReg = pr->m_nRegister[eSHClass];
	if (pr->m_eType == eSType_Sampler)
		SHData.push_back(eT_SamplerState);
	else if (pr->m_eType == eSType_SamplerComp)
		SHData.push_back(eT_SamplerComparisonState);
	else
		assert(0);
	for (u32 i = 0; i < pr->m_dwName.size(); i++)
		SHData.push_back(pr->m_dwName[i]);

	if (nReg >= 0 && nReg < 10000)
	{
		SHData.push_back(eT_colon);
		SHData.push_back(eT_register);
		SHData.push_back(eT_br_rnd_1);
		drx_sprintf(str, "s%d", nReg);
		SHData.push_back(Parser.NewUserToken(eT_unknown, str, true));
		SHData.push_back(eT_br_rnd_2);
	}
	SHData.push_back(eT_semicolumn);
}
void CShaderManBin::AddTextureToScript(CParserBin& Parser, SFXTexture* pr, PodArray<u32>& SHData, EHWShaderClass eSHClass)
{
	char str[256];
	i32 nReg = pr->m_nRegister[eSHClass];
	if (pr->m_eType == eTT_2D)
		SHData.push_back(eT_Texture2D);
	else if (pr->m_eType == eTT_3D)
		SHData.push_back(eT_Texture3D);
	else if (pr->m_eType == eTT_2DArray)
		SHData.push_back(eT_Texture2DArray);
	else if (pr->m_eType == eTT_2DMS)
		SHData.push_back(eT_Texture2DMS);
	else if (pr->m_eType == eTT_Cube)
		SHData.push_back(eT_TextureCube);
	else if (pr->m_eType == eTT_CubeArray)
		SHData.push_back(eT_TextureCubeArray);
	else
		assert(0);
	if (pr->m_Type != eT_unknown)
	{
		SHData.push_back(eT_br_tr_1);
		SHData.push_back(pr->m_Type);
		SHData.push_back(eT_br_tr_2);
	}
	for (u32 i = 0; i < pr->m_dwName.size(); i++)
		SHData.push_back(pr->m_dwName[i]);

	if (nReg >= 0 && nReg < 10000)
	{
		SHData.push_back(eT_colon);
		SHData.push_back(eT_register);
		SHData.push_back(eT_br_rnd_1);
		drx_sprintf(str, "t%d", nReg);
		SHData.push_back(Parser.NewUserToken(eT_unknown, str, true));
		SHData.push_back(eT_br_rnd_2);
	}
	SHData.push_back(eT_semicolumn);
}

bool CShaderManBin::ParseBinFX_Technique_Pass_GenerateShaderData(CParserBin& Parser, FXMacroBin& Macros, SShaderFXParams& FXParams, u32 dwSHName, EHWShaderClass eSHClass, uint64& nAffectMask, u32 dwSHType, PodArray<u32>& SHData, SShaderTechnique* pShTech)
{
	LOADING_TIME_PROFILE_SECTION(iSystem);
	assert(gRenDev->m_pRT->IsRenderThread() || gRenDev->m_pRT->IsLevelLoadingThread());

	bool bRes = true;

	std::vector<SFXParam> AffectedParams;
	std::vector<SFXSampler> AffectedSamplers;
	std::vector<SFXTexture> AffectedTextures;

	u32 i, j;
	u32 nNum;
	static TArray<i32> AffectedFragments;
	AffectedFragments.reserve(120);
	AffectedFragments.SetUse(0);

	// Skip any generation stuff for Auto-Shaders
	if (dwSHName == eT_$AutoGS_MultiRes)
		return true;

	for (nNum = 0; nNum < Parser.m_CodeFragments.size(); nNum++)
	{
		if (dwSHName == Parser.m_CodeFragments[nNum].m_dwName)
			break;
	}
	if (nNum == Parser.m_CodeFragments.size())
	{
		Warning("Couldn't find entry function '%s'", Parser.GetString(dwSHName));
		assert(0);
		return false;
	}

	SCodeFragment* pFunc = &Parser.m_CodeFragments[nNum];
	SShaderBin* pBin = Parser.m_pCurBinShader;
	SParamCacheInfo* pCache = GetParamInfo(pBin, pFunc->m_dwName, Parser.m_pCurShader->m_nMaskGenFX);
	if (pCache)
	{
		AffectedFragments.SetUse(0);
		AffectedFragments.reserve(pCache->m_AffectedFuncs.size());
		if (pCache->m_AffectedFuncs.empty() == false)
			AffectedFragments.Copy(&pCache->m_AffectedFuncs[0], pCache->m_AffectedFuncs.size());
	}
	else
	{
		AffectedFragments.push_back(nNum);
		if (CParserBin::m_bParseFX)
		{
			TArray<byte> bChecked;
			bChecked.resize(Parser.m_CodeFragments.size());
			if (bChecked.size() > 0)
				memset(&bChecked[0], 0, sizeof(byte) * bChecked.size());
			bChecked[nNum] = 1;
			InitShaderDependenciesList(Parser, pFunc, bChecked, AffectedFragments);
			CheckFragmentsDependencies(Parser, bChecked, AffectedFragments);
		}
		else
		{
			for (i = 0; i < Parser.m_CodeFragments.size(); i++)
			{
				if (i != nNum)
				{
					AffectedFragments.push_back(i);
				}
			}
		}
	}

	nAffectMask = 0;
	for (i = 0; i < AffectedFragments.size(); i++)
	{
		DRX_ASSERT_MESSAGE(AffectedFragments[i] < Parser.m_CodeFragments.size(), "fragment index is larger than number of CodeFragments!");

		SCodeFragment* s = &Parser.m_CodeFragments[AffectedFragments[i]];
		if (s->m_eType != eFT_Function && s->m_eType != eFT_Structure && s->m_eType != eFT_ConstBuffer && s->m_eType != eFT_StorageClass)
			continue;
		FXMacroBinItor itor;
		for (itor = Macros.begin(); itor != Macros.end(); ++itor)
		{
			SMacroBinFX* pr = &itor->second;
			if (!pr->m_nMask)
				continue;
			if ((pr->m_nMask & nAffectMask) == pr->m_nMask)
				continue;
			u32 dwName = itor->first;
			if (Parser.FindToken(s->m_nFirstToken, s->m_nLastToken, dwName) >= 0)
				nAffectMask |= pr->m_nMask;
		}
	}

	// Generate list of params before first preprocessor pass for affected functions
	TArray<byte> bMerged;
	bMerged.Reserve(FXParams.m_FXParams.size());
	if (pCache)
	{
		for (i = 0; i < pCache->m_AffectedParams.size(); i++)
		{
			u32 nParam = pCache->m_AffectedParams[i];
			FXParamsIt it = std::lower_bound(FXParams.m_FXParams.begin(), FXParams.m_FXParams.end(), nParam, FXParamsSortByName());
			if (it != FXParams.m_FXParams.end() && nParam == (*it).m_dwName[0])
			{
				SFXParam& pr = (*it);
				if (!(pr.GetParamFlags() & PF_AUTOMERGED))
				{
					if (pr.m_Semantic.empty() && pr.m_Values.c_str()[0] == '(')
						pr.m_nCB = CB_PER_MATERIAL;
					AffectedParams.push_back(pr);
				}
			}
		}
		for (i = 0; i < pCache->m_AffectedSamplers.size(); i++)
		{
			u32 nParam = pCache->m_AffectedSamplers[i];
			FXSamplersIt it = std::lower_bound(FXParams.m_FXSamplers.begin(), FXParams.m_FXSamplers.end(), nParam, FXSamplersSortByName());
			if (it != FXParams.m_FXSamplers.end() && nParam == (*it).m_dwName[0])
			{
				SFXSampler& pr = (*it);
				AffectedSamplers.push_back(pr);
			}
		}
		for (i = 0; i < pCache->m_AffectedTextures.size(); i++)
		{
			u32 nParam = pCache->m_AffectedTextures[i];
			FXTexturesIt it = std::lower_bound(FXParams.m_FXTextures.begin(), FXParams.m_FXTextures.end(), nParam, FXTexturesSortByName());
			if (it != FXParams.m_FXTextures.end() && nParam == (*it).m_dwName[0])
			{
				SFXTexture& pr = (*it);
				AffectedTextures.push_back(pr);
			}
		}
	}
	else
	{
		for (i = 0; i < FXParams.m_FXParams.size(); i++)
		{
			SFXParam* pr = &FXParams.m_FXParams[i];
			if (!(pr->GetParamFlags() & PF_AUTOMERGED))
				AddAffectedParameter(Parser, AffectedParams, AffectedFragments, pr, eSHClass, dwSHType, pShTech);
		}
		for (i = 0; i < FXParams.m_FXSamplers.size(); i++)
		{
			SFXSampler* pr = &FXParams.m_FXSamplers[i];
			AddAffectedSampler(Parser, AffectedSamplers, AffectedFragments, pr, eSHClass, dwSHType, pShTech);
		}
		for (i = 0; i < FXParams.m_FXTextures.size(); i++)
		{
			SFXTexture* pr = &FXParams.m_FXTextures[i];
			AddAffectedTexture(Parser, AffectedTextures, AffectedFragments, pr, eSHClass, dwSHType, pShTech);
		}
	}

	if (CParserBin::m_bParseFX)
	{
		FXMacroBinItor itor;
		for (itor = Macros.begin(); itor != Macros.end(); ++itor)
		{
			if (itor->second.m_nMask && !(itor->second.m_nMask & nAffectMask))
				continue;
			SHData.push_back(eT_define);
			SHData.push_back(itor->first);
			SHData.push_back(0);
		}
		std::vector<SFXParam> PackedParams;
		TArray<SCodeFragment> Replaces;
		TArray<u32> NewTokens;
		ParseBinFX_Technique_Pass_PackParameters(Parser, AffectedParams, AffectedFragments, pFunc, eSHClass, dwSHName, PackedParams, Replaces, NewTokens, bMerged);

		if (!pCache)
		{
			// Update new parameters in shader structures
			for (i = 0; i < PackedParams.size(); i++)
			{
				SFXParam* pr = &PackedParams[i];
				AffectedParams.push_back(*pr);
			}
			if (CRenderer::CV_r_shadersAllowCompilation)
				SaveBinShaderLocalInfo(pBin, pFunc->m_dwName, Parser.m_pCurShader->m_nMaskGenFX, AffectedFragments, AffectedParams, AffectedSamplers, AffectedTextures);
		}
		else
		{
			for (i = 0; i < pCache->m_AffectedParams.size(); i++)
			{
				i32 nParam = pCache->m_AffectedParams[i];
				for (j = 0; j < PackedParams.size(); j++)
				{
					SFXParam& pr = PackedParams[j];
					if (pr.m_dwName[0] == nParam)
					{
						AffectedParams.push_back(pr);
						break;
					}
				}
			}
		}

		// Update FX parameters
		for (i = 0; i < AffectedParams.size(); i++)
		{
			SFXParam* pr = &AffectedParams[i];
			mfAddFXParam(FXParams, pr);
		}

		// Include all affected functions/structures/parameters in the final script
		if (CParserBin::PlatformSupportsConstantBuffers()) // use CB scopes for D3D10 shaders
		{
			int8 nPrevCB = -1;
			std::vector<SFXParam*> ParamsData;

      byte bMergeMask = (eSHClass == eHWSC_Pixel) ? 1 : 2;
      bMergeMask <<= 4;
      for (i=0; i<AffectedParams.size(); i++)
      {
        SFXParam &pr = AffectedParams[i];
        if (bMerged[i] & bMergeMask)
          continue;
        if (pr.m_nCB >= 0)
          ParamsData.push_back(&pr);
      }

		if (eSHClass == eHWSC_Vertex)
			std::sort(ParamsData.begin(), ParamsData.end(), CompareVars<0>);
		else if (eSHClass == eHWSC_Pixel)
			std::sort(ParamsData.begin(), ParamsData.end(), CompareVars<1>);
		else
			std::sort(ParamsData.begin(), ParamsData.end(), CompareVars<2>);

			// First we need to declare semantic variables (in CB scopes in case of DX11)
			for (i = 0; i < ParamsData.size(); i++)
			{
				SFXParam* pPr = ParamsData[i];
				i32 nCB = pPr->m_nCB;
				if (nPrevCB != nCB)
				{
					if (nPrevCB != -1)
					{
						SHData.push_back(eT_br_cv_2);
						SHData.push_back(eT_semicolumn);
					}
					SHData.push_back(eT_cbuffer);
					SHData.push_back(dwNamesCB[nCB]);
					SHData.push_back(eT_colon);
					SHData.push_back(eT_register);
					char str[32];
					drx_sprintf(str, "b%d", nCB);
					SHData.push_back(eT_br_rnd_1);
					SHData.push_back(Parser.NewUserToken(eT_unknown, str, true));
					SHData.push_back(eT_br_rnd_2);
					SHData.push_back(eT_br_cv_1);
				}
				nPrevCB = nCB;
				AddParameterToScript(Parser, pPr, SHData, eSHClass, nCB);
			}
			if (nPrevCB >= 0)
			{
				nPrevCB = -1;
				SHData.push_back(eT_br_cv_2);
				SHData.push_back(eT_semicolumn);
			}

			for (i = 0; i < AffectedSamplers.size(); i++)
			{
				SFXSampler* pr = &AffectedSamplers[i];
				AddSamplerToScript(Parser, pr, SHData, eSHClass);
			}
			for (i = 0; i < AffectedTextures.size(); i++)
			{
				SFXTexture* pr = &AffectedTextures[i];
				AddTextureToScript(Parser, pr, SHData, eSHClass);
			}
		}
		else
		{
			// Update affected parameters in script
#ifdef _DEBUG
			for (i = 0; i < AffectedParams.size(); i++)
			{
				SFXParam* pr = &AffectedParams[i];
				for (j = i + 1; j < AffectedParams.size(); j++)
				{
					SFXParam* pr1 = &AffectedParams[j];
					if (pr->m_dwName[0] == pr1->m_dwName[0])
					{
						assert(0);
					}
				}
			}
#endif
			byte bMergeMask = (eSHClass == eHWSC_Pixel) ? 1 : 2;
			bMergeMask <<= 4;
			for (i = 0; i < AffectedParams.size(); i++)
			{
				SFXParam* pr = &AffectedParams[i];
				// Ignore parameters which where packed
				if (bMerged[i] & bMergeMask)
					continue;
				AddParameterToScript(Parser, pr, SHData, eSHClass, -1);
			}
		}

		// Generate fragment tokens
		for (i = 0; i < Parser.m_CodeFragments.size(); i++)
		{
			i32 h = -1;
			SCodeFragment* cf = &Parser.m_CodeFragments[i];
			if (cf->m_dwName)
			{
				for (h = 0; h < (i32)AffectedFragments.size(); h++)
				{
					if (AffectedFragments[h] == i)
						break;
				}
				if (h == AffectedFragments.size())
					continue;
			}

			Parser.CopyTokens(cf, SHData, Replaces, NewTokens, h);
			if (cf->m_eType == eFT_Sampler)
			{
				if (CParserBin::m_nPlatform & (SF_D3D11 | SF_DURANGO | SF_GL4 | SF_GLES3 | SF_VULKAN))
				{
					i32 nT = Parser.m_Tokens[cf->m_nLastToken - 1];
					//assert(nT >= eT_s0 && nT <= eT_s15);
					if (nT >= eT_s0 && nT <= eT_s15)
					{
						nT = nT - eT_s0 + eT_t0;
						SHData.push_back(eT_colon);
						SHData.push_back(eT_register);
						SHData.push_back(eT_br_rnd_1);
						SHData.push_back(nT);
						SHData.push_back(eT_br_rnd_2);
					}
				}
				SHData.push_back(eT_semicolumn);
			}
		}
	}
	return bRes;
}

bool CShaderManBin::ParseBinFX_Technique_Pass_LoadShader(CParserBin& Parser, FXMacroBin& Macros, SParserFrame& SHFrame, SShaderTechnique* pShTech, SShaderPass* pPass, EHWShaderClass eSHClass, SShaderFXParams& FXParams)
{
	assert(gRenDev->m_pRT->IsRenderThread() || gRenDev->m_pRT->IsLevelLoadingThread());

	LOADING_TIME_PROFILE_SECTION(iSystem);
	bool bRes = true;

	assert(!SHFrame.IsEmpty());

	u32 dwSHName;
	u32 dwSHType = 0;

	u32* pTokens = &Parser.m_Tokens[0];

	dwSHName = pTokens[SHFrame.m_nFirstToken];
	u32 nCur = SHFrame.m_nFirstToken + 1;
	u32 nTok = pTokens[nCur];
	if (nTok != eT_br_rnd_1)
	{
		nCur += 2;
		nTok = pTokens[nCur];
	}
	nCur++;
	assert(nTok == eT_br_rnd_1);
	if (nTok == eT_br_rnd_1)
	{
		nTok = pTokens[nCur];
		if (nTok != eT_br_rnd_2)
		{
			assert(!"Local function parameters aren't supported anymore");
		}
	}
	nCur++;
	if (nCur <= SHFrame.m_nLastToken)
	{
		dwSHType = pTokens[nCur];
	}

	enum { SHDATA_BUFFER_SIZE = 48000 };

	uint64 nGenMask = 0;
	PodArray<u32> SHDataBuffer(SHDATA_BUFFER_SIZE);
	tukk szName = Parser.GetString(dwSHName);
	bRes &= ParseBinFX_Technique_Pass_GenerateShaderData(Parser, Macros, FXParams, dwSHName, eSHClass, nGenMask, dwSHType, SHDataBuffer, pShTech);
#if defined(_DEBUG)
	if (SHDataBuffer.size() > SHDATA_BUFFER_SIZE)
	{
		DrxLogAlways("CShaderManBin::ParseBinFX_Technique_Pass_LoadShader: SHDataBuffer has been exceeded (buffer=%d, count=%u). Adjust buffer size to remove unnecessary allocs", SHDATA_BUFFER_SIZE, SHDataBuffer.Size());
	}
#endif
	TArray<u32> SHData(0, SHDataBuffer.Size());
	SHData.Copy(SHDataBuffer.GetElements(), SHDataBuffer.size());

	CHWShader* pSH = NULL;
	bool bValidShader = false;

	assert(Parser.m_pCurShader != nullptr);
	if (bRes && (!CParserBin::m_bParseFX || !SHData.empty() || szName[0] == '$'))
	{
		char str[1024];
		drx_sprintf(str, "%s@%s", Parser.m_pCurShader->m_NameShader.c_str(), szName);
		pSH = CHWShader::mfForName(str, Parser.m_pCurShader->m_NameFile, Parser.m_pCurShader->m_CRC32, szName, eSHClass, SHData, Parser.m_TokenTable, dwSHType, Parser.m_pCurShader, nGenMask, Parser.m_pCurShader->m_nMaskGenFX);
	}
	if (pSH)
	{
		/*uint64 nFlagsOrigShader_RT = 0;
		   uint64 nFlagsOrigShader_GL = pSH->m_nMaskGenShader;
		   u32 nFlagsOrigShader_LT = 0;
		   pSH->mfModifyFlags(Parser.m_pCurShader);*/

		bValidShader = true;

		if (eSHClass == eHWSC_Vertex)
			pPass->m_VShader = pSH;
		else if (eSHClass == eHWSC_Pixel)
			pPass->m_PShader = pSH;
		else if (GEOMETRYSHADER_SUPPORT && CParserBin::PlatformSupportsGeometryShaders() && eSHClass == eHWSC_Geometry)
			pPass->m_GShader = pSH;
		else if (CParserBin::PlatformSupportsDomainShaders() && eSHClass == eHWSC_Domain)
			pPass->m_DShader = pSH;
		else if (CParserBin::PlatformSupportsHullShaders() && eSHClass == eHWSC_Hull)
			pPass->m_HShader = pSH;
		else if (CParserBin::PlatformSupportsComputeShaders() && eSHClass == eHWSC_Compute)
			pPass->m_CShader = pSH;
		else
			DrxLog("Unsupported/unrecognised shader: %s[%d]", pSH->m_Name.c_str(), eSHClass);
	}

	return bRes;
}

bool CShaderManBin::ParseBinFX_Technique_Pass(CParserBin& Parser, SParserFrame& Frame, SShaderTechnique* pShTech)
{
	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(VertexShader)
		FX_TOKEN(PixelShader)
		FX_TOKEN(GeometryShader)
		FX_TOKEN(DomainShader)
		FX_TOKEN(HullShader)
		FX_TOKEN(ComputeShader)
		FX_TOKEN(ZEnable)
		FX_TOKEN(ZWriteEnable)
		FX_TOKEN(CullMode)
		FX_TOKEN(SrcBlend)
		FX_TOKEN(DestBlend)
		FX_TOKEN(AlphaBlendEnable)
		FX_TOKEN(AlphaFunc)
		FX_TOKEN(AlphaRef)
		FX_TOKEN(ZFunc)
		FX_TOKEN(ColorWriteEnable)
		FX_TOKEN(IgnoreMaterialState)
	FX_END_TOKENS

	bool bRes = true;
	i32 n = pShTech->m_Passes.Num();
	pShTech->m_Passes.ReserveNew(n + 1);
	SShaderPass* sm = &pShTech->m_Passes[n];
	sm->m_eCull = -1;
	sm->m_AlphaRef = ~0;

	SParserFrame VS, PS, GS, DS, HS, CS;
	FXMacroBin VSMacro, PSMacro, GSMacro, DSMacro, HSMacro, CSMacro;

	byte ZFunc = eCF_LEqual;

	byte ColorWriteMask = 0xff;

	byte AlphaFunc = eCF_Disable;
	byte AlphaRef = 0;
	i32 State = GS_DEPTHWRITE;

	i32 nMaxTMU = 0;
	signed char Cull = -1;
	i32 nIndex;
	EToken eSrcBlend = eT_unknown;
	EToken eDstBlend = eT_unknown;
	bool bBlend = false;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_VertexShader:
			VS = Parser.m_Data;
			VSMacro = Parser.m_Macros[1];
			break;
		case eT_PixelShader:
			PS = Parser.m_Data;
			PSMacro = Parser.m_Macros[1];
			break;
		case eT_GeometryShader:
			GS = Parser.m_Data;
			GSMacro = Parser.m_Macros[1];
			break;
		case eT_DomainShader:
			DS = Parser.m_Data;
			DSMacro = Parser.m_Macros[1];
			break;
		case eT_HullShader:
			HS = Parser.m_Data;
			HSMacro = Parser.m_Macros[1];
			break;
		case eT_ComputeShader:
			CS = Parser.m_Data;
			CSMacro = Parser.m_Macros[1];
			break;

		case eT_ZEnable:
			if (Parser.GetBool(Parser.m_Data))
				State &= ~GS_NODEPTHTEST;
			else
				State |= GS_NODEPTHTEST;
			break;
		case eT_ZWriteEnable:
			if (Parser.GetBool(Parser.m_Data))
				State |= GS_DEPTHWRITE;
			else
				State &= ~GS_DEPTHWRITE;
			break;
		case eT_CullMode:
			eT = Parser.GetToken(Parser.m_Data);
			if (eT == eT_None)
				Cull = eCULL_None;
			else if (eT == eT_CCW || eT == eT_Back)
				Cull = eCULL_Back;
			else if (eT == eT_CW || eT == eT_Front)
				Cull = eCULL_Front;
			else
			{
				Warning("unknown CullMode parameter '%s' (Skipping)\n", Parser.GetString(eT));
				assert(0);
			}
			break;
		case eT_AlphaFunc:
			AlphaFunc = Parser.GetCompareFunc(Parser.GetToken(Parser.m_Data));
			break;
		case eT_ColorWriteEnable:
			{
			if (ColorWriteMask == 0xff)
				ColorWriteMask = 0;
			u32 nCur = Parser.m_Data.m_nFirstToken;
			while (nCur <= Parser.m_Data.m_nLastToken)
			{
				u32 nT = Parser.m_Tokens[nCur++];
				if (nT == eT_or)
					continue;
				if (nT == eT_0)
					ColorWriteMask |= 0;
				else if (nT == eT_RED)
					ColorWriteMask |= 1;
				else if (nT == eT_GREEN)
					ColorWriteMask |= 2;
				else if (nT == eT_BLUE)
					ColorWriteMask |= 4;
				else if (nT == eT_ALPHA)
					ColorWriteMask |= 8;
				else
				{
					Warning("unknown WriteMask parameter '%s' (Skipping)\n", Parser.GetString(eT));
				}
			}
			}
			break;
		case eT_ZFunc:
			ZFunc = Parser.GetCompareFunc(Parser.GetToken(Parser.m_Data));
			sm->m_PassFlags |= SHPF_FORCEZFUNC;
			break;
		case eT_AlphaRef:
			AlphaRef = Parser.GetInt(Parser.GetToken(Parser.m_Data));
			break;
		case eT_SrcBlend:
			eSrcBlend = Parser.GetToken(Parser.m_Data);
			break;
		case eT_DestBlend:
			eDstBlend = Parser.GetToken(Parser.m_Data);
			break;
		case eT_AlphaBlendEnable:
			bBlend = Parser.GetBool(Parser.m_Data);
			break;

		case eT_IgnoreMaterialState:
			sm->m_PassFlags |= SHPF_NOMATSTATE;
			break;

		default:
			assert(0);
		}
	}

	if (bBlend && eSrcBlend && eDstBlend)
	{
		i32 nSrc = Parser.GetSrcBlend(eSrcBlend);
		i32 nDst = Parser.GetDstBlend(eDstBlend);
		if (nSrc >= 0 && nDst >= 0)
		{
			State |= nSrc;
			State |= nDst;
		}
	}
	if (ColorWriteMask != 0xff)
	{
		ColorWriteMask = (~ColorWriteMask) & 0xf;
		auto it = std::find_if(AvailableColorMasks.cbegin(), AvailableColorMasks.cend(), [ColorWriteMask](u32k &bitmask)
		{
			return (ColorWriteMask == bitmask);
		});

		if (it != AvailableColorMasks.cend())
		{
			State |= (std::distance(AvailableColorMasks.cbegin(), it) << GS_COLMASK_SHIFT);
		}
		else
		{
			iLog->LogError("Unsupported/unrecognized ColorWriteMask in shader: %s", Parser.m_pCurShader->m_NameShader.c_str());
		}
	}

	if (AlphaFunc)
	{
		// Alpha-Test is done in shader code (with a explicit function)
		State |= GS_ALPHATEST;
	}

	switch (ZFunc)
	{
	case eCF_Equal:
		State |= GS_DEPTHFUNC_EQUAL;
		break;
	case eCF_Greater:
		State |= GS_DEPTHFUNC_GREAT;
		break;
	case eCF_GEqual:
		State |= GS_DEPTHFUNC_GEQUAL;
		break;
	case eCF_Less:
		State |= GS_DEPTHFUNC_LESS;
		break;
	case eCF_NotEqual:
		State |= GS_DEPTHFUNC_NOTEQUAL;
		break;
	case eCF_LEqual:
		State |= GS_DEPTHFUNC_LEQUAL;
		break;
	default:
		assert(0);
	}

	sm->m_RenderState = State;
	sm->m_eCull = Cull;

	mfGeneratePublicFXParams(Parser.m_pCurShader, Parser);
	SShaderFXParams& FXParams = mfGetFXParams(Parser.m_pCurShader);

	if (!VS.IsEmpty())
		bRes &= ParseBinFX_Technique_Pass_LoadShader(Parser, VSMacro, VS, pShTech, sm, eHWSC_Vertex, FXParams);
	if (!PS.IsEmpty())
		bRes &= ParseBinFX_Technique_Pass_LoadShader(Parser, PSMacro, PS, pShTech, sm, eHWSC_Pixel, FXParams);
	if (CParserBin::PlatformSupportsGeometryShaders() && !GS.IsEmpty())
		bRes &= ParseBinFX_Technique_Pass_LoadShader(Parser, GSMacro, GS, pShTech, sm, eHWSC_Geometry, FXParams);
	if (CParserBin::PlatformSupportsHullShaders() && !HS.IsEmpty())
		bRes &= ParseBinFX_Technique_Pass_LoadShader(Parser, HSMacro, HS, pShTech, sm, eHWSC_Hull, FXParams);
	if (CParserBin::PlatformSupportsDomainShaders() && !DS.IsEmpty())
		bRes &= ParseBinFX_Technique_Pass_LoadShader(Parser, DSMacro, DS, pShTech, sm, eHWSC_Domain, FXParams);
	if (CParserBin::PlatformSupportsComputeShaders() && !CS.IsEmpty())
		bRes &= ParseBinFX_Technique_Pass_LoadShader(Parser, CSMacro, CS, pShTech, sm, eHWSC_Compute, FXParams);

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_LightStyle_Val(CParserBin& Parser, SParserFrame& Frame, CLightStyle* ls)
{
	i32 i;
	char str[64];
	tukk pstr1, * pstr2;
	SLightStyleKeyFrame pKeyFrame;

	ls->m_Map.Free();
	string sr = Parser.GetString(Frame);
	tukk lstr = sr.c_str();

	// First count the keyframes
	size_t nKeyFrames = 0;
	while (true)
	{
		pstr1 = strchr(lstr, '|');
		if (!pstr1)
			break;
		pstr2 = strchr(pstr1 + 1, '|');
		if (!pstr2)
			break;
		if (pstr2 - pstr1 - 1 > 0)
			++nKeyFrames;
		lstr = pstr2;
	}
	ls->m_Map.reserve(nKeyFrames);

	lstr = sr.c_str();
	i32 n = 0;
	while (true)
	{
		pstr1 = strchr(lstr, '|');
		if (!pstr1)
			break;
		pstr2 = strchr(pstr1 + 1, '|');
		if (!pstr2)
			break;
		if (pstr2 - pstr1 - 1 > 0)
		{
			strncpy(str, pstr1 + 1, pstr2 - pstr1 - 1);
			str[pstr2 - pstr1 - 1] = 0;
			i = sscanf(str, "%f %f %f %f %f %f %f", &pKeyFrame.cColor.r, &pKeyFrame.cColor.g, &pKeyFrame.cColor.b, &pKeyFrame.cColor.a,
			           &pKeyFrame.vPosOffset.x, &pKeyFrame.vPosOffset.y, &pKeyFrame.vPosOffset.z);
			switch (i)
			{
			case 1:
				{
					// Only luminance updates
					pKeyFrame.cColor.g = pKeyFrame.cColor.b = pKeyFrame.cColor.r;
					pKeyFrame.cColor.a = 1.0f;
					pKeyFrame.vPosOffset = Vec3(0);
				}
				break;

			case 3:
				{
					// No position/spec mult updates
					pKeyFrame.cColor.a = 1.0f;
					pKeyFrame.vPosOffset = Vec3(0);
				}
				break;

			case 4:
				{
					// No position
					pKeyFrame.vPosOffset = Vec3(0);
				}
				break;

			default:
				break;

			}
			ls->m_Map.AddElem(pKeyFrame);
			n++;
		}

		lstr = pstr2;
	}
	ls->m_Map.Shrink();
	assert(ls->m_Map.Num() == n);
	if (ls->m_Map.Num() == n)
		return true;
	return false;
}

bool CShaderManBin::ParseBinFX_LightStyle(CParserBin& Parser, SParserFrame& Frame, i32 nStyle)
{
	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(KeyFrameParams)
		FX_TOKEN(KeyFrameRandColor)
		FX_TOKEN(KeyFrameRandIntensity)
		FX_TOKEN(KeyFrameRandSpecMult)
		FX_TOKEN(KeyFrameRandPosOffset)
		FX_TOKEN(Speed)
	FX_END_TOKENS

	bool bRes = true;

	Parser.m_pCurShader->m_Flags |= EF_LIGHTSTYLE;
	if (CLightStyle::s_LStyles.Num() <= (u32)nStyle)
		CLightStyle::s_LStyles.ReserveNew(nStyle + 1);
	CLightStyle* ls = CLightStyle::s_LStyles[nStyle];
	if (!ls)
	{
		ls = new CLightStyle;
		ls->m_LastTime = 0;
		ls->m_Color = Col_White;
		CLightStyle::s_LStyles[nStyle] = ls;
	}
	ls->m_TimeIncr = 60;
	i32 nIndex;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_KeyFrameRandColor:
			ls->m_bRandColor = Parser.GetBool(Parser.m_Data);
			break;

		case eT_KeyFrameRandIntensity:
			ls->m_bRandIntensity = Parser.GetBool(Parser.m_Data);
			break;

		case eT_KeyFrameRandSpecMult:
			ls->m_bRandSpecMult = Parser.GetBool(Parser.m_Data);
			break;

		case eT_KeyFrameRandPosOffset:
			ls->m_bRandPosOffset = Parser.GetBool(Parser.m_Data);
			break;

		case eT_KeyFrameParams:
			bRes &= ParseBinFX_LightStyle_Val(Parser, Parser.m_Data, ls);
			break;

		case eT_Speed:
			ls->m_TimeIncr = Parser.GetFloat(Parser.m_Data);
			break;
		default:
			assert(0);
		}
	}

	if (ls->m_Map.Num() && (ls->m_bRandPosOffset || ls->m_bRandIntensity || ls->m_bRandSpecMult || ls->m_bRandColor))
	{
		i32 nCount = ls->m_Map.Num();
		for (i32 f = 0; f < nCount; ++f)
		{
			SLightStyleKeyFrame& pKeyFrame = ls->m_Map[f];
			if (ls->m_bRandPosOffset)
				pKeyFrame.vPosOffset = Vec3(drx_random(-1.0f, 1.0f), drx_random(-1.0f, 1.0f), drx_random(-1.0f, 1.0f));

			if (ls->m_bRandColor)
				pKeyFrame.cColor *= ColorF(drx_random(0.0f, 1.0f), drx_random(0.0f, 1.0f), drx_random(0.0f, 1.0f), 1.0f);

			if (ls->m_bRandIntensity)
				pKeyFrame.cColor *= drx_random(0.0f, 1.0f);

			if (ls->m_bRandSpecMult)
				pKeyFrame.cColor.a = drx_random(0.0f, 1.0f);
		}
	}

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_Technique_Annotations_String(CParserBin& Parser, SParserFrame& Frame, SShaderTechnique* pShTech, std::vector<SShaderTechParseParams>& techParams, bool* bPublic)
{
	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(Public)
		FX_TOKEN(NoLights)
		FX_TOKEN(NoMaterialState)
		FX_TOKEN(PositionInvariant)
		FX_TOKEN(TechniqueZ)
		FX_TOKEN(TechniqueZPrepass)
		FX_TOKEN(TechniqueShadowGen)
		FX_TOKEN(TechniqueMotionBlur)
		FX_TOKEN(TechniqueCustomRender)
		FX_TOKEN(TechniqueEffectLayer)
		FX_TOKEN(TechniqueDebug)
		FX_TOKEN(TechniqueWaterRefl)
		FX_TOKEN(TechniqueWaterCaustic)
		FX_TOKEN(TechniqueThickness)
	FX_END_TOKENS

	bool bRes = true;
	i32 nIndex;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_Public:
			if (pShTech)
				pShTech->m_Flags |= FHF_PUBLIC;
			else if (bPublic)
				*bPublic = true;
			break;

		case eT_PositionInvariant:
			if (pShTech)
				pShTech->m_Flags |= FHF_POSITION_INVARIANT;
			break;

		case eT_NoLights:
			if (pShTech)
				pShTech->m_Flags |= FHF_NOLIGHTS;
			break;

		case eT_NoMaterialState:
			if (Parser.m_pCurShader)
				Parser.m_pCurShader->m_Flags2 |= EF2_IGNORERESOURCESTATES;
			break;

		// Note: When adding/removing batch flags, please, update sDescList in D3DRendPipeline.cpp
		case eT_TechniqueDebug:
			{
			}

		case eT_TechniqueZ:
		case eT_TechniqueZPrepass:
		case eT_TechniqueShadowGen:
		case eT_TechniqueMotionBlur:
		case eT_TechniqueCustomRender:
		case eT_TechniqueEffectLayer:
		case eT_TechniqueWaterRefl:
		case eT_TechniqueWaterCaustic:
		case eT_TechniqueThickness:
			{
				if (!Parser.m_pCurShader)
					break;

				static u32k pTechTable[eT_TechniqueMax - eT_TechniqueZ] =
				{
					TTYPE_Z,                    //eT_TechniqueZ
					TTYPE_SHADOWGEN,            //eT_TechniqueShadowGen
					TTYPE_MOTIONBLURPASS,       //eT_TechniqueMotionBlur
					TTYPE_CUSTOMRENDERPASS,     //eT_TechniqueCustomRender
					TTYPE_EFFECTLAYER,          //eT_TechniqueEffectLayer
					TTYPE_DEBUG,                //eT_TechniqueDebug
					TTYPE_WATERREFLPASS,        //eT_TechniqueWaterRefl
					TTYPE_WATERCAUSTICPASS,     //eT_TechniqueWaterCaustic
					TTYPE_ZPREPASS,             //eT_TechniqueZPrepass
				};

				const CDrxNameR pszNameTech = Parser.GetNameString(Parser.m_Data);
				i32 idx = techParams.size() - 1;
				assert(idx >= 0);
				assert(eT - eT_TechniqueZ >= 0);
				techParams[idx].techName[pTechTable[eT - eT_TechniqueZ]] = pszNameTech;
			}
			break;

		default:
			assert(0);
		}
	}

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_Technique_Annotations(CParserBin& Parser, SParserFrame& Frame, SShaderTechnique* pShTech, std::vector<SShaderTechParseParams>& techParams, bool* bPublic)
{
	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(string)
	FX_END_TOKENS

	bool bRes = true;
	i32 nIndex;

	while (Parser.ParseObject(sCommands, nIndex))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_string:
			{
				eT = Parser.GetToken(Parser.m_Name);
				assert(eT == eT_Script);
				bRes &= ParseBinFX_Technique_Annotations_String(Parser, Parser.m_Data, pShTech, techParams, bPublic);
			}
			break;

		default:
			assert(0);
		}
	}

	Parser.EndFrame(OldFrame);

	return bRes;
}

bool CShaderManBin::ParseBinFX_Technique_CustomRE(CParserBin& Parser, SParserFrame& Frame, SParserFrame& Name, SShaderTechnique* pShTech)
{
	u32 nName = Parser.GetToken(Name);

	if (nName == eT_LensOptics)
	{
		CRELensOptics* ps = new CRELensOptics;

		{
			pShTech->m_REs.AddElem(ps);
			pShTech->m_Flags |= FHF_RE_LENSOPTICS;
			return true;
		}
	}
	else if (nName == eT_Ocean)
	{
		assert(0);
	}

	return true;
}

SShaderTechnique* CShaderManBin::ParseBinFX_Technique(CParserBin& Parser, SParserFrame& Frame, SParserFrame Annotations, std::vector<SShaderTechParseParams>& techParams, bool* bPublic)
{
	LOADING_TIME_PROFILE_SECTION(iSystem);

	SParserFrame OldFrame = Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(pass)
		FX_TOKEN(CustomRE)
		FX_TOKEN(Style)
	FX_END_TOKENS

	bool bRes = true;
	SShaderTechnique* pShTech = NULL;
	if (Parser.m_pCurShader)
		pShTech = new SShaderTechnique(Parser.m_pCurShader);

	if (Parser.m_pCurShader)
	{
		SShaderTechParseParams ps;
		techParams.push_back(ps);
	}

	if (!Annotations.IsEmpty())
		ParseBinFX_Technique_Annotations(Parser, Annotations, pShTech, techParams, bPublic);

	while (Parser.ParseObject(sCommands))
	{
		EToken eT = Parser.GetToken();
		switch (eT)
		{
		case eT_pass:
			if (pShTech)
				bRes &= ParseBinFX_Technique_Pass(Parser, Parser.m_Data, pShTech);
			break;

		case eT_Style:
			if (pShTech)
				ParseBinFX_LightStyle(Parser, Parser.m_Data, Parser.GetInt(Parser.GetToken(Parser.m_Name)));
			break;

		case eT_CustomRE:
			if (pShTech)
				bRes &= ParseBinFX_Technique_CustomRE(Parser, Parser.m_Data, Parser.m_Name, pShTech);
			break;

		default:
			assert(0);
		}
	}

	if (bRes)
	{
		if (Parser.m_pCurShader && pShTech)
		{
			Parser.m_pCurShader->m_HWTechniques.AddElem(pShTech);
		}
	}
	else
	{
		techParams.pop_back();
	}

	Parser.EndFrame(OldFrame);

	return pShTech;
}

float g_fTimeA;

bool CShaderManBin::ParseBinFX(SShaderBin* pBin, CShader* ef, uint64 nMaskGen)
{
	LOADING_TIME_PROFILE_SECTION_ARGS(pBin->m_szName);

	bool bRes = true;

	float fTimeA = iTimer->GetAsyncCurTime();

#if !defined(SHADER_NO_SOURCES)
	CParserBin Parser(pBin, ef);

	CShader* efGen = ef->m_pGenShader;

	if (efGen && efGen->m_ShaderGenParams)
		AddGenMacros(efGen->m_ShaderGenParams, Parser, nMaskGen);

	pBin->Lock();
	Parser.Preprocess(0, pBin->m_Tokens, pBin->m_TokenTable);
	ef->m_CRC32 = pBin->m_CRC32;
	ef->m_SourceCRC32 = pBin->m_SourceCRC32;
	pBin->Unlock();
#endif

#if defined(SHADER_NO_SOURCES)
	iLog->Log("ERROR: Couldn't find binary shader '%s' (0x%x)", ef->GetName(), ef->m_nMaskGenFX);
	return false;
#else
	SParserFrame Frame(0, (Parser.m_Tokens.size() > 0 ? Parser.m_Tokens.size() - 1 : 0));
	Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(static)
		FX_TOKEN(unorm)
		FX_TOKEN(snorm)
		FX_TOKEN(float)
		FX_TOKEN(float2)
		FX_TOKEN(float3)
		FX_TOKEN(float4)
		FX_TOKEN(float2x4)
		FX_TOKEN(float3x4)
		FX_TOKEN(float4x4)
		FX_TOKEN(float3x3)
		FX_TOKEN(half)
		FX_TOKEN(half2)
		FX_TOKEN(half3)
		FX_TOKEN(half4)
		FX_TOKEN(half2x4)
		FX_TOKEN(half3x4)
		FX_TOKEN(half4x4)
		FX_TOKEN(half3x3)
		FX_TOKEN(bool)
		FX_TOKEN(i32)
		FX_TOKEN(int2)
		FX_TOKEN(int3)
		FX_TOKEN(int4)
		FX_TOKEN(uint)
		FX_TOKEN(uint2)
		FX_TOKEN(uint3)
		FX_TOKEN(uint4)
		FX_TOKEN(min16float)
		FX_TOKEN(min16float2)
		FX_TOKEN(min16float3)
		FX_TOKEN(min16float4)
		FX_TOKEN(min16float4x4)
		FX_TOKEN(min16float3x4)
		FX_TOKEN(min16float2x4)
		FX_TOKEN(min16float3x3)
		FX_TOKEN(min10float)
		FX_TOKEN(min10float2)
		FX_TOKEN(min10float3)
		FX_TOKEN(min10float4)
		FX_TOKEN(min10float4x4)
		FX_TOKEN(min10float3x4)
		FX_TOKEN(min10float2x4)
		FX_TOKEN(min10float3x3)
		FX_TOKEN(min16int)
		FX_TOKEN(min16int2)
		FX_TOKEN(min16int3)
		FX_TOKEN(min16int4)
		FX_TOKEN(min12int)
		FX_TOKEN(min12int2)
		FX_TOKEN(min12int3)
		FX_TOKEN(min12int4)
		FX_TOKEN(min16uint)
		FX_TOKEN(min16uint2)
		FX_TOKEN(min16uint3)
		FX_TOKEN(min16uint4)
		FX_TOKEN(struct)
		FX_TOKEN(sampler1D)
		FX_TOKEN(sampler2D)
		FX_TOKEN(sampler3D)
		FX_TOKEN(samplerCUBE)
		FX_TOKEN(Texture2D)
		FX_TOKEN(RWTexture2D)
		FX_TOKEN(RWTexture2DArray)
		FX_TOKEN(Texture2DArray)
		FX_TOKEN(Texture2DMS)
		FX_TOKEN(TextureCube)
		FX_TOKEN(TextureCubeArray)
		FX_TOKEN(Texture3D)
		FX_TOKEN(RWTexture3D)
		FX_TOKEN(technique)
		FX_TOKEN(SamplerState)
		FX_TOKEN(SamplerComparisonState)
		FX_TOKEN(Buffer)
		FX_TOKEN(RWBuffer)
		FX_TOKEN(StructuredBuffer)
		FX_TOKEN(RWStructuredBuffer)
		FX_TOKEN(ByteAddressBuffer)
		FX_TOKEN(RWByteAddressBuffer)
		FX_TOKEN(cbuffer)
	FX_END_TOKENS

	std::vector<SShaderTechParseParams> techParams;
	CDrxNameR techStart[2];

	// From MemReplay analysis of shader params, 200 should be more than enough space
	s_tempFXParams.reserve(200);
	s_tempFXParams.clear();

	SShaderFXParams& FXP = mfGetFXParams(Parser.m_pCurShader);
	FXP.m_FXParams.swap(s_tempFXParams);

	ef->mfFree();

	assert(ef->m_HWTechniques.Num() == 0);
	i32 nInd = 0;

	ETokenStorageClass nTokenStorageClass;
	while (nTokenStorageClass = Parser.ParseObject(sCommands))
	{
		EToken eT = Parser.GetToken();
		SCodeFragment Fr;
		SFXParam P;
		string szR;
		switch (eT)
		{
		case eT_struct:
		case eT_cbuffer:
			Fr.m_nFirstToken = Parser.FirstToken();
			Fr.m_nLastToken = Parser.m_CurFrame.m_nCurToken - 1;
			Fr.m_dwName = Parser.m_Tokens[Fr.m_nFirstToken + 1];
	#ifdef _DEBUG
			//Fr.m_Name = Parser.GetString(Fr.m_dwName);
	#endif
			Fr.m_eType = (eT == eT_cbuffer) ? eFT_ConstBuffer : eFT_Structure;
			Parser.m_CodeFragments.push_back(Fr);
			break;

		case eT_SamplerState:
		case eT_SamplerComparisonState:
			{
				SFXSampler Pr;
				Parser.CopyTokens(Parser.m_Name, Pr.m_dwName);
	#ifdef _DEBUG
				tukk sampName = Parser.GetString(Parser.m_Name);
	#endif
				if (eT == eT_SamplerState)
					Pr.m_eType = eSType_Sampler;
				else if (eT == eT_SamplerComparisonState)
					Pr.m_eType = eSType_SamplerComp;
				else
					assert(0);

				Pr.PostLoad(Parser, Parser.m_Name, Parser.m_Annotations, Parser.m_Value, Parser.m_Assign);

				// SamplerState something = SS_identifiersearch;
				if (!strnicmp(Pr.m_Values.c_str(), "SM_", 3) ||
				    !strnicmp(Pr.m_Values.c_str(), "SS_", 3))
				{
					Pr.m_Semantic = Pr.m_Values;
					Pr.m_Values = "";
				}

				mfAddFXSampler(Parser.m_pCurShader, &Pr);
			}
			break;

		case eT_Texture2D:
		case eT_Texture2DMS:
		case eT_Texture2DArray:
		case eT_TextureCube:
		case eT_TextureCubeArray:
		case eT_Texture3D:
			{
				SFXTexture Pr;
				Parser.CopyTokens(Parser.m_Name, Pr.m_dwName);

				if (eT == eT_Texture2D)
					Pr.m_eType = eTT_2D;
				else if (eT == eT_Texture2DMS)
					Pr.m_eType = eTT_2DMS;
				else if (eT == eT_Texture2DArray)
					Pr.m_eType = eTT_2DArray;
				else if (eT == eT_Texture3D)
					Pr.m_eType = eTT_3D;
				else if (eT == eT_TextureCube)
					Pr.m_eType = eTT_Cube;
				else if (eT == eT_TextureCubeArray)
					Pr.m_eType = eTT_CubeArray;
				else
					assert(0);

				Pr.PostLoad(Parser, Parser.m_Name, Parser.m_Annotations, Parser.m_Value, Parser.m_Assign);

				bRes &= ParseBinFX_Texture(Parser, Parser.m_Data, Pr, Parser.m_Annotations);

				// Texture2D something = TS_identifiersearch;
				if (!strnicmp(Pr.m_Values.c_str(), "TM_", 3) ||
				    !strnicmp(Pr.m_Values.c_str(), "TS_", 3) ||
				    !strnicmp(Pr.m_Values.c_str(), "TP_", 3) ||
				    !strnicmp(Pr.m_Values.c_str(), "TSF_", 4))
				{
					Pr.m_Semantic = Pr.m_Values;
					Pr.m_szTexture = "";
					Pr.m_Values = "";
				}

				// Texture2D something = "filepathsearch";
				// Texture2D something = $databasesearch;
				if (Pr.m_Values.c_str()[0])
				{
					Pr.m_Semantic = "";
					Pr.m_szTexture = Pr.m_Values.c_str();
					Pr.m_Values = "";
				}

				mfAddFXTexture(Parser.m_pCurShader, &Pr);
			}
			break;

		case eT_int:
		case eT_bool:
		case eT_half:
		case eT_half2:
		case eT_half3:
		case eT_half4:
		case eT_half2x4:
		case eT_half3x4:
		case eT_half4x4:
		case eT_float:
		case eT_float2:
		case eT_float3:
		case eT_float4:
		case eT_float2x4:
		case eT_float3x4:
		case eT_float4x4:
			{
				SFXParam Pr;
				Parser.CopyTokens(Parser.m_Name, Pr.m_dwName);

				if (eT == eT_float2x4 || eT == eT_half2x4)
					Pr.m_nParameters = 2;
				else if (eT == eT_float3x4 || eT == eT_half3x4)
					Pr.m_nParameters = 3;
				else if (eT == eT_float4x4 || eT == eT_half4x4)
					Pr.m_nParameters = 4;
				else
					Pr.m_nParameters = 1;

				if (eT == eT_float || eT == eT_half || eT == eT_int || eT == eT_bool)
					Pr.m_nComps = 1;
				else if (eT == eT_float2 || eT == eT_half2)
					Pr.m_nComps = 2;
				else if (eT == eT_float3 || eT == eT_half3)
					Pr.m_nComps = 3;
				else if (eT == eT_float4 || eT == eT_float2x4 || eT == eT_float3x4 || eT == eT_float4x4 ||
				         eT == eT_half4 || eT == eT_half2x4 || eT == eT_half3x4 || eT == eT_half4x4)
					Pr.m_nComps = 4;

				if (eT == eT_int)
					Pr.m_eType = eType_INT;
				else if (eT == eT_bool)
					Pr.m_eType = eType_BOOL;
				else if (eT >= eT_half && eT <= eT_half3x3)
					Pr.m_eType = eType_HALF;
				else
					Pr.m_eType = eType_FLOAT;

				if (!Parser.m_Assign.IsEmpty() && Parser.GetToken(Parser.m_Assign) == eT_STANDARDSGLOBAL)
				{
					ParseBinFX_Global(Parser, Parser.m_Annotations, NULL, techStart);
				}
				else
				{
					u32 nTokAssign = 0;
					if (Parser.m_Assign.IsEmpty() && !Parser.m_Value.IsEmpty())
					{
						nTokAssign = Parser.m_Tokens[Parser.m_Value.m_nFirstToken];
						if (nTokAssign == eT_br_cv_1)
							nTokAssign = Parser.m_Tokens[Parser.m_Value.m_nFirstToken + 1];
					}
					else if (!Parser.m_Assign.IsEmpty())
						nTokAssign = Parser.m_Tokens[Parser.m_Assign.m_nFirstToken];
					if (nTokAssign)
					{
						tukk assign = Parser.GetString(nTokAssign);
						if (assign[0] && !strnicmp(assign, "PM_", 3))
							Pr.m_nCB = CB_PER_MATERIAL;
						else
							Pr.m_nCB = CB_PER_DRAW;
					}
					else if (CParserBin::m_nPlatform & (SF_D3D11 | SF_ORBIS | SF_DURANGO | SF_GL4 | SF_GLES3 | SF_VULKAN))
					{
						u32 nTokName = Parser.GetToken(Parser.m_Name);
						tukk name = Parser.GetString(nTokName);
						
						Pr.m_nCB = CB_PER_DRAW;
					}

					Pr.PostLoad(Parser, Parser.m_Name, Parser.m_Annotations, Parser.m_Value, Parser.m_Assign);

					EParamType eType;
					string szReg = Pr.GetValueForName("register", eType);
					if (!szReg.empty())
					{
						assert(szReg[0] == 'c');
						Pr.m_nRegister[eHWSC_Vertex] = atoi(&szReg[1]);
						Pr.m_nRegister[eHWSC_Pixel] = Pr.m_nRegister[eHWSC_Vertex];
						if (GEOMETRYSHADER_SUPPORT && CParserBin::PlatformSupportsGeometryShaders())
							Pr.m_nRegister[eHWSC_Geometry] = Pr.m_nRegister[eHWSC_Vertex];
						if (CParserBin::PlatformSupportsDomainShaders())
							Pr.m_nRegister[eHWSC_Domain] = Pr.m_nRegister[eHWSC_Vertex];
					}
					u32 prFlags = Pr.GetParamFlags();
					if (prFlags & PF_TWEAKABLE_MASK)
					{
						if (!(prFlags & PF_ALWAYS))
						{
							Pr.m_nCB = CB_PER_MATERIAL;
							assert(prFlags & PF_CUSTOM_BINDED);
						}
					}
					mfAddFXParam(Parser.m_pCurShader, &Pr);
				}
			}
			break;


		case eT_Buffer:
		case eT_RWBuffer:
		case eT_StructuredBuffer:
		case eT_RWStructuredBuffer:
		case eT_ByteAddressBuffer:
		case eT_RWByteAddressBuffer:
		case eT_RWTexture2D:
		case eT_RWTexture2DArray:
		case eT_RWTexture3D:
			{
				Fr.m_nFirstToken = Parser.FirstToken();
				Fr.m_nLastToken = Fr.m_nFirstToken + 1;
				if (!Parser.m_Assign.IsEmpty())
					Fr.m_nLastToken = Parser.m_Assign.m_nLastToken + 1;
				Fr.m_dwName = Parser.m_Tokens[Parser.m_Name.m_nFirstToken];
				Fr.m_eType = eFT_Structure;
				Parser.m_CodeFragments.push_back(Fr);
			}
			break;

		case eT_technique:
			{
				u32 nToken = Parser.m_Tokens[Parser.m_Name.m_nFirstToken];
				tukk szName = Parser.GetString(nToken);
				SShaderTechnique* pShTech = ParseBinFX_Technique(Parser, Parser.m_Data, Parser.m_Annotations, techParams, NULL);
				if (szName)
				{
					pShTech->m_NameStr = szName;
					pShTech->m_NameCRC = szName;
				}
			}
			break;

		default:
			//assert(0);
			break;
		}
	}

	FXP.m_FXParams.swap(s_tempFXParams);
	FXP.m_FXParams.reserve(FXP.m_FXParams.size() + s_tempFXParams.size());
	FXP.m_FXParams.insert(FXP.m_FXParams.end(), s_tempFXParams.begin(), s_tempFXParams.end());

	m_pCEF->mfPostLoadFX(ef, techParams, techStart);

	#if SHADER_REFLECT_TEXTURE_SLOTS
	for (u32 i = 0; i < min((u32)TTYPE_MAX, ef->m_HWTechniques.size()); i++)
	{
		if (!ef->GetUsedTextureSlots(i))
		{
			ef->m_ShaderTexSlots[i] = GetTextureSlots(Parser, pBin, ef, i, -1);
		}
	}

	for (u32 i = 0; i < min((u32)TTYPE_MAX, ef->m_HWTechniques.size()); i++)
	{
		if (ef->m_ShaderTexSlots[i])
		{
			SShaderTechParseParams* linkedTechs = &techParams[i];

			// look through linked techniques (e.g. general has links to zpass, shadowgen, etc)
			for (i32 j = 0; j < TTYPE_MAX; j++)
			{
				// if we have a linked technique
				if (!linkedTechs->techName[j].empty())
				{
					// find it in our technique list
					for (i32 k = 0; k < ef->m_HWTechniques.size(); k++)
					{
						if (linkedTechs->techName[j] == ef->m_HWTechniques[k]->m_NameStr)
						{
							// merge slots - any slots that are empty in the master will be filled with the overlay
							// This leaves the general/main technique authoratitve on slots, but stops slots disappearing
							// if they're still used in some sub-pass
							MergeTextureSlots(ef->m_ShaderTexSlots[i], ef->m_ShaderTexSlots[k]);
							break;
						}
					}
				}
			}
		}
	}
	#endif

	g_fTimeA += iTimer->GetAsyncCurTime() - fTimeA;

	return bRes;
#endif
}

void CShaderManBin::MergeTextureSlots(SShaderTexSlots* master, SShaderTexSlots* overlay)
{
	if (!master || !overlay)
		return;

	for (i32 i = 0; i < EFTT_MAX; i++)
	{
		// these structures are never deleted so we can safely share pointers
		// without ref counting. See below in GetTextureSlots for the allocation
		if (master->m_UsedSlots[i] == NULL && overlay->m_UsedSlots[i] != NULL)
		{
			master->m_UsedSlots[i] = overlay->m_UsedSlots[i];
		}
	}
}

SShaderTexSlots* CShaderManBin::GetTextureSlots(CParserBin& Parser, SShaderBin* pBin, CShader* ef, i32 nTech, i32 nPass)
{
#if !SHADER_REFLECT_TEXTURE_SLOTS
	return NULL;
#else
	TArray<u32> referencedSamplers;

	bool bIterPasses = false;
	i32 nMaxPasses = 1;

	if (nPass == -1)
	{
		bIterPasses = true;
		nPass = 0;
	}

	if (nTech < 0 || nPass < 0)
		return NULL;

	SShaderFXParams& FXP = mfGetFXParams(ef);

	// if the technique's pixel shader exists
	if (ef->m_HWTechniques.size() && nTech < ef->m_HWTechniques.size() &&
	    ef->m_HWTechniques[nTech]->m_Passes.size() && nPass < ef->m_HWTechniques[nTech]->m_Passes.size() &&
	    ef->m_HWTechniques[nTech]->m_Passes[nPass].m_PShader)
	{

		if (bIterPasses)
		{
			nMaxPasses = ef->m_HWTechniques[nTech]->m_Passes.size();
		}

		for (i32 nPassIter = nPass; nPassIter < nMaxPasses; nPassIter++)
		{

			u32 dwEntryFuncName = Parser.GetCRC32(ef->m_HWTechniques[nTech]->m_Passes[nPassIter].m_PShader->m_EntryFunc);

			// get the cached info for the entry func
			SParamCacheInfo* pCache = GetParamInfo(pBin, dwEntryFuncName, ef->m_nMaskGenFX);

			if (pCache)
			{
				// loop over affected fragments from this entry func
				SParamCacheInfo::AffectedFuncsVec& AffectedFragments = pCache->m_AffectedFuncs;
				for (u32 i = 0; i < AffectedFragments.size(); i++)
				{
					DRX_ASSERT_MESSAGE(AffectedFragments[i] < Parser.m_CodeFragments.size(), "fragment index is larger than number of CodeFragments!");

					SCodeFragment* s = &Parser.m_CodeFragments[AffectedFragments[i]];

					// if it's a sampler, include this sampler name CRC
					if (s->m_eType == eFT_Sampler)
					{
						referencedSamplers.AddElem(s->m_dwName);
					}
				}
				SParamCacheInfo::AffectedParamsVec& AffectedTextures = pCache->m_AffectedTextures;
				for (u32 i = 0; i < AffectedTextures.size(); i++)
				{
					u32 nParam = AffectedTextures[i];
					FXTexturesIt it = std::lower_bound(FXP.m_FXTextures.begin(), FXP.m_FXTextures.end(), nParam, FXTexturesSortByName());
					if (it != FXP.m_FXTextures.end() && nParam == (*it).m_dwName[0])
					{
						SFXTexture& pr = (*it);
						referencedSamplers.AddElem(pr.m_dwName[0]);
					}
				}
			}
			else
			{
				return NULL;
			}
		}
	}
	else
	{
		return NULL;
	}

	u32 dependencySlots = 0;

	// check the gen params
	SShaderGen* genParams = ef->GetGenerationParams();
	if (genParams)
	{
		for (u32 i = 0; i < genParams->m_BitMask.Num(); i++)
		{
			SShaderGenBit* pBit = genParams->m_BitMask[i];
			if (!pBit || (!pBit->m_nDependencySet && !pBit->m_nDependencyReset))
				continue;

			// if any dependency set/reset is allowed for a texture, we must be conservative
			// and count this slot as used
			u32 setReset = pBit->m_nDependencySet | pBit->m_nDependencyReset;

			// bitmask will fail if we have > 32 fixed texture slots
			assert(EFTT_MAX <= 32);

			if (setReset & SHGD_TEX_MASK)
			{
				if (setReset & SHGD_TEX_NORMALS)
					dependencySlots |= 1 << EFTT_NORMALS;

				if (setReset & SHGD_TEX_HEIGHT)
					dependencySlots |= 1 << EFTT_HEIGHT;

				if (setReset & SHGD_TEX_DETAIL)
					dependencySlots |= 1 << EFTT_DETAIL_OVERLAY;

				if (setReset & SHGD_TEX_TRANSLUCENCY)
					dependencySlots |= 1 << EFTT_TRANSLUCENCY;

				if (setReset & SHGD_TEX_SPECULAR)
					dependencySlots |= 1 << EFTT_SPECULAR;

				if (setReset & SHGD_TEX_ENVCM)
					dependencySlots |= 1 << EFTT_ENV;

				if (setReset & SHGD_TEX_SUBSURFACE)
					dependencySlots |= 1 << EFTT_SUBSURFACE;

				if (setReset & SHGD_TEX_DECAL)
					dependencySlots |= 1 << EFTT_DECAL_OVERLAY;

				if (setReset & SHGD_TEX_CUSTOM)
					dependencySlots |= 1 << EFTT_CUSTOM;

				if (setReset & SHGD_TEX_CUSTOM_SECONDARY)
					dependencySlots |= 1 << EFTT_CUSTOM_SECONDARY;
			}
		}
	}
	else
	{
		return NULL;
	}

	// since we might find samplers referencing a slot more than once,
	// keep track of the priority of each sampler found.
	i32 namePriority[EFTT_MAX];
	memset(namePriority, 0, sizeof(namePriority));

	// deliberately 'leaking', these are kept around permanently (this info is only gathered in the editor),
	// and cleaned up at shutdown when the app memory is released.
	SShaderTexSlots* pSlots = new SShaderTexSlots;

	// Priority order:
	// 0 = forced in because of the dependencyset as above, can be overriden if we find a better sampler
	// 1 = referenced from the shader but doesn't include a UIName
	// 2 = includes a UIName but isn't directly referenced. In the case we've forced the sampler because of dependency,
	//     this is probably more descriptive than a priority=1 sampler
	// 3 = both from above, referenced and has UIName. We shouldn't find multiple samplers like this, shader samplers
	//     should be set up so only ever one sampler using a slot has a UIName.
	enum { PRIORITY_REFERENCED = 0x1, PRIORITY_HASUINAME = 0x2 };
	
	// loop over all textures for this shader
	{
		FXTexturesIt it = FXP.m_FXTextures.begin();
		FXTexturesIt end = FXP.m_FXTextures.end();
		for (; it != end; ++it)
		{
			i32 slot = TextureHelpers::FindTexSlot(it->m_Semantic.c_str());

			// if the slot is invalid this texture refers something else, skip it
			if (slot == EFTT_UNKNOWN) continue;

			u32 dwName = Parser.GetCRC32(it->m_Name.c_str());

			// check if this sampler must be included for dependencyset/reset reasons
			bool dependency = (dependencySlots & (1 << slot)) > 0;

			// check if this sampler is referenced from the shader
			bool referenced = referencedSamplers.Find(dwName) >= 0;

			if (dependency || referenced)
			{
				// calculate priority. See above.
				i32 priority = (referenced ? PRIORITY_REFERENCED : 0) | (it->m_szUIName.length() ? PRIORITY_HASUINAME : 0);

				// if we don't have this slot filled already, create a new slot
				if (pSlots->m_UsedSlots[slot] == NULL)
				{
					// !!IMPORTANT!! - if these slots are deleted/cleaned instead of being allowed to live forever,
					// MAKE SURE you refcount or refactor MergeTextureSlots above - as it will share pointers.
					SShaderTextureSlot* pSlot = new SShaderTextureSlot;

					pSlot->m_Name = it->m_szUIName;
					pSlot->m_Description = it->m_szUIDesc;

					pSlot->m_TexType = it->m_eType;

					// store current priority
					namePriority[slot] = priority;

					pSlots->m_UsedSlots[slot] = pSlot;
				}
				else
				{
					SShaderTextureSlot* pSlot = pSlots->m_UsedSlots[slot];
					// we shouldn't encounter two samplers that are used and have a UIName for the same slot,
					// error in this case
					if (priority == (PRIORITY_REFERENCED | PRIORITY_HASUINAME) && priority == namePriority[slot])
					{
						DrxWarning(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, "Encountered two samplers with UINames referenced for same slot in shader '%s': '%s' and '%s'\n",
						           ef->GetName(), pSlot->m_Name, it->m_szUIName);
						assert(0);
					}
					// override if we have a higher priority
					else if (priority > namePriority[slot])
					{
						pSlot->m_Name = it->m_szUIName;
						pSlot->m_Description = it->m_szUIDesc;

						pSlot->m_TexType = it->m_eType;

						namePriority[slot] = priority;
					}
				}
			}
		}
	}

	return pSlots;
#endif
}

bool CShaderManBin::ParseBinFX_Dummy(SShaderBin* pBin, std::vector<string>& ShaderNames, tukk szName)
{
	bool bRes = true;
	CParserBin Parser(pBin, NULL);

	pBin->Lock();
	Parser.Preprocess(0, pBin->m_Tokens, pBin->m_TokenTable);
	pBin->Unlock();

	SParserFrame Frame(0, Parser.m_Tokens.size() - 1);
	Parser.BeginFrame(Frame);

	FX_BEGIN_TOKENS
		FX_TOKEN(static)
		FX_TOKEN(half)
		FX_TOKEN(half2)
		FX_TOKEN(half3)
		FX_TOKEN(half4)
		FX_TOKEN(half2x4)
		FX_TOKEN(half3x4)
		FX_TOKEN(half4x4)
		FX_TOKEN(float)
		FX_TOKEN(float2)
		FX_TOKEN(float3)
		FX_TOKEN(float4)
		FX_TOKEN(float2x4)
		FX_TOKEN(float3x4)
		FX_TOKEN(float4x4)
		FX_TOKEN(bool)
		FX_TOKEN(i32)
		FX_TOKEN(Buffer)
		FX_TOKEN(RWBuffer)
		FX_TOKEN(StructuredBuffer)
		FX_TOKEN(RWStructuredBuffer)
		FX_TOKEN(cbuffer)
		FX_TOKEN(struct)
		FX_TOKEN(sampler1D)
		FX_TOKEN(sampler2D)
		FX_TOKEN(sampler3D)
		FX_TOKEN(samplerCUBE)
		FX_TOKEN(technique)
		FX_TOKEN(SamplerState)
		FX_TOKEN(SamplerComparisonState)
		FX_TOKEN(Texture2D)
		FX_TOKEN(RWTexture2D)
		FX_TOKEN(RWTexture2DArray)
		FX_TOKEN(Texture2DArray)
		FX_TOKEN(Texture2DMS)
		FX_TOKEN(TextureCube)
		FX_TOKEN(TextureCubeArray)
		FX_TOKEN(Texture3D)
		FX_TOKEN(RWTexture3D)
	FX_END_TOKENS

	std::vector<SShaderTechParseParams> techParams;
	CDrxNameR techStart[2];
	bool bPublic = false;
	std::vector<string> PubTechniques;

	ETokenStorageClass nTokenStorageClass;

	while (nTokenStorageClass = Parser.ParseObject(sCommands))
	{
		EToken eT = Parser.GetToken();
		SCodeFragment Fr;
		switch (eT)
		{
		case eT_half:
		case eT_float:
			if (!Parser.m_Assign.IsEmpty() && Parser.GetToken(Parser.m_Assign) == eT_STANDARDSGLOBAL)
			{
				ParseBinFX_Global(Parser, Parser.m_Annotations, &bPublic, techStart);
			}
			break;
		case eT_Buffer:
		case eT_RWBuffer:
		case eT_StructuredBuffer:
		case eT_RWStructuredBuffer:
		case eT_cbuffer:
		case eT_struct:
		case eT_SamplerState:
		case eT_SamplerComparisonState:
		case eT_int:
		case eT_bool:

		case eT_half2:
		case eT_half3:
		case eT_half4:
		case eT_half2x4:
		case eT_half3x4:
		case eT_half4x4:

		case eT_float2:
		case eT_float3:
		case eT_float4:
		case eT_float2x4:
		case eT_float3x4:
		case eT_float4x4:

		case eT_Texture2D:
		case eT_RWTexture2D:
		case eT_Texture2DMS:
		case eT_Texture2DArray:
		case eT_RWTexture2DArray:
		case eT_TextureCube:
		case eT_TextureCubeArray:
		case eT_Texture3D:
		case eT_RWTexture3D:
		case eT_sampler1D:
		case eT_sampler2D:
		case eT_sampler3D:
		case eT_samplerCUBE:
			break;

		case eT_technique:
			{
				u32 nToken = Parser.m_Tokens[Parser.m_Name.m_nFirstToken];
				bool bPublicTechnique = false;
				SShaderTechnique* pShTech = ParseBinFX_Technique(Parser, Parser.m_Data, Parser.m_Annotations, techParams, &bPublicTechnique);
				if (bPublicTechnique)
				{
					tukk name = Parser.GetString(nToken);
					PubTechniques.push_back(name);
				}
			}
			break;

		default:
			assert(0);
		}
	}

	if (bPublic)
		ShaderNames.push_back(szName);
	if (PubTechniques.size())
	{
		u32 i;
		for (i = 0; i < PubTechniques.size(); i++)
		{
			string str = szName;
			str += ".";
			str += PubTechniques[i];
			ShaderNames.push_back(str);
		}
	}

	return bRes;
}

inline bool CompareInstParams(const SCGParam& a, const SCGParam& b)
{
	return (a.m_dwBind < b.m_dwBind);
}

void CShaderMan::mfPostLoadFX(CShader* ef, std::vector<SShaderTechParseParams>& techParams, CDrxNameR techStart[2])
{
	ef->m_HWTechniques.Shrink();

	u32 i;

	assert(techParams.size() == ef->m_HWTechniques.Num());
	for (i = 0; i < ef->m_HWTechniques.Num(); i++)
	{
		SShaderTechnique* hw = ef->m_HWTechniques[i];
		SShaderTechParseParams* ps = &techParams[i];
		u32 n;
		for (n = 0; n < TTYPE_MAX; n++)
		{
			if (ps->techName[n].c_str()[0])
			{
				if (hw->m_NameStr == ps->techName[n])
					iLog->LogWarning("WARNING: technique '%s' refers to itself as the next technique (ignored)", hw->m_NameStr.c_str());
				else
				{
					u32 j;
					for (j = 0; j < ef->m_HWTechniques.Num(); j++)
					{
						SShaderTechnique* hw2 = ef->m_HWTechniques[j];
						if (hw2->m_NameStr == ps->techName[n])
						{
							hw->m_nTechnique[n] = j;
							break;
						}
					}
					if (j == ef->m_HWTechniques.Num())
						iLog->LogWarning("WARNING: couldn't find technique '%s' in the sequence for technique '%s' (ignored)", ps->techName[n].c_str(), hw->m_NameStr.c_str());
				}
			}
		}

		SShaderTechnique* hwZWrite = (hw->m_nTechnique[TTYPE_Z] >= 0) ? ef->m_HWTechniques[hw->m_nTechnique[TTYPE_Z]] : NULL;
		hwZWrite = (hw->m_nTechnique[TTYPE_ZPREPASS] >= 0) ? ef->m_HWTechniques[hw->m_nTechnique[TTYPE_ZPREPASS]] : hwZWrite;
		if (hwZWrite && hwZWrite->m_Passes.Num())
		{
			SShaderPass* pass = &hwZWrite->m_Passes[0];
			if (pass->m_RenderState & GS_DEPTHWRITE)
				hw->m_Flags |= FHF_WASZWRITE;
		}

		bool bTransparent = true;
		for (u32 j = 0; j < hw->m_Passes.Num(); j++)
		{
			SShaderPass* pass = &hw->m_Passes[j];
			if (CParserBin::PlatformSupportsGeometryShaders() && pass->m_GShader)
				hw->m_Flags |= FHF_USE_GEOMETRY_SHADER;
			if (CParserBin::PlatformSupportsHullShaders() && pass->m_HShader)
				hw->m_Flags |= FHF_USE_HULL_SHADER;
			if (CParserBin::PlatformSupportsDomainShaders() && pass->m_DShader)
				hw->m_Flags |= FHF_USE_DOMAIN_SHADER;
			if (!(pass->m_RenderState & GS_BLEND_MASK))
				bTransparent = false;
		}
		if (bTransparent)
			hw->m_Flags |= FHF_TRANSPARENT;
	}
}

//===========================================================================================

bool STexSamplerRT::Update()
{
	if (m_pAnimInfo && m_pAnimInfo->m_Time && gRenDev->m_bPauseTimer == 0)
	{
		float time = gRenDev->GetFrameSyncTime().GetSeconds();
		assert(time >= 0);
		u32 m = (u32)(time / m_pAnimInfo->m_Time) % (m_pAnimInfo->m_NumAnimTexs);
		assert(m < (u32)m_pAnimInfo->m_TexPics.Num());

		if (m_pTex != m_pAnimInfo->m_TexPics[m])
		{
			if (m_pTex)
			{
				m_pTex->Release();
			}

			m_pTex = m_pAnimInfo->m_TexPics[m];
			m_pTex->AddRef();

			return true;
		}
	}

	return false;
}

void SFXParam::GetCompName(u32 nId, DrxFixedStringT<128>& name)
{
	if (nId > 3)
	{
		name = "";
		return;
	}

	char nm[16];
	drx_sprintf(nm, "__%u", nId);
	tukk s = strstr(m_Name.c_str(), nm);

	if (!s)
	{
		name = "";
		return;
	}

	s += 3;

	u32 n;
	for (n = 0; s[n] != 0; ++n)
	{
		if (s[n] <= 0x20 || (s[n] == '_' && s[n + 1] == '_'))
		{
			break;
		}
	}
	name.append(s, n);
}

void SFXParam::GetParamComp(u32 nOffset, DrxFixedStringT<128>& param)
{
	tukk szValue = m_Values.c_str();

	if (!szValue[0])
	{
		param = "";
		return;
	}

	if (szValue[0] == '{')
	{
		++szValue;
	}

	for (u32 n = 0; n != nOffset; ++n)
	{
		while (szValue[0] != ',' && szValue[0] != ';' && szValue[0] != '}' && szValue[0] != 0)
		{
			++szValue;
		}

		if (szValue[0] == ';' || szValue[0] == '}' || szValue[0] == 0)
		{
			param = "";
			return;
		}

		++szValue;
	}

	while (*szValue == ' ' || *szValue == 8)
	{
		++szValue;
	}

	u32 n;
	for (n = 0; szValue[n] != 0; ++n)
	{
		if (szValue[n] == ',' || szValue[n] == ';' || szValue[n] == '}')
		{
			break;
		}
	}
	param.append(szValue, n);
}

u32 SFXParam::GetComponent(EHWShaderClass eSHClass)
{
	tuk src = (tuk)m_Annotations.c_str();
	char annot[256];
	i32 nComp = 0;
	fxFill(&src, annot);

	bool bRegisterAnnotation = strncmp(&annot[0], "register", 8) == 0;
	bool bSRegisterAnnotation = false;
	if (!bRegisterAnnotation)
		bSRegisterAnnotation = strncmp(&annot[1], "sregister", 9) == 0;

	if (bRegisterAnnotation || bSRegisterAnnotation)
	{
		/*if ((eSHClass == eHWSC_Pixel && annot[0] =='v') || (eSHClass == eHWSC_Vertex && annot[0] =='p'))
		   {
		   fxFill(&src, annot);
		   if (strncmp(&annot[1], "sregister", 9))
		    return 0;
		   if ((eSHClass == eHWSC_Pixel && annot[0] =='v') || (eSHClass == eHWSC_Vertex && annot[0] =='p'))
		    return 0;
		   }*/
		tuk b = bRegisterAnnotation ? &annot[8] : &annot[10];
		SkipCharacters(&b, kWhiteSpace);
		assert(b[0] == '=');
		b++;
		SkipCharacters(&b, kWhiteSpace);
		assert(b[0] == 'c');
		if (b[0] == 'c')
		{
			i32 nReg = atoi(&b[1]);
			b++;
			while (*b != '.')
			{
				if (*b == 0) // Vector without swizzling
					break;
				b++;
			}
			if (*b == '.')
			{
				b++;
				switch (b[0])
				{
				case 'x':
				case 'r':
					nComp = 0;
					break;
				case 'y':
				case 'g':
					nComp = 1;
					break;
				case 'z':
				case 'b':
					nComp = 2;
					break;
				case 'w':
				case 'a':
					nComp = 3;
					break;
				default:
					assert(0);
				}
			}
		}
	}
	return nComp;
}

string SFXParam::GetValueForName(tukk szName, EParamType& eType)
{
	eType = eType_UNKNOWN;
	if (m_Annotations.empty())
		return "";

	char buf[256];
	char tok[128];
	tuk szA = (tuk)m_Annotations.c_str();
	SkipCharacters(&szA, kWhiteSpace);
	while (true)
	{
		if (!fxFill(&szA, buf))
			break;
		tuk b = buf;
		fxFillPr(&b, tok);
		eType = eType_UNKNOWN;
		if (!stricmp(tok, "string"))
			eType = eType_STRING;
		else if (!stricmp(tok, "float"))
			eType = eType_FLOAT;
		else if (!stricmp(tok, "half"))
			eType = eType_HALF;

		if (eType != eType_UNKNOWN)
		{
			if (!fxFillPr(&b, tok))
				continue;
			if (stricmp(tok, szName))
				continue;
			SkipCharacters(&b, kWhiteSpace);
			if (b[0] == '=')
			{
				b++;
				if (!fxFillPrC(&b, tok))
					break;
			}
			return tok;
		}
		else
		{
			if (stricmp(tok, szName))
				continue;
			eType = eType_STRING;
			if (!fxFillPr(&b, tok))
				continue;
			if (tok[0] == '=')
			{
				if (!fxFillPr(&b, tok))
					break;
			}
			return tok;
		}
	}

	return "";
}

tukk CShaderMan::mfParseFX_Parameter(const string& script, EParamType eType, tukk szName)
{
	static char sRet[128];
	i32 nLen = script.length();
	tuk pTemp = new char[nLen + 1];
	strcpy(pTemp, script.c_str());
	sRet[0] = 0;
	tuk buf = pTemp;

	tuk name;
	long cmd;
	tuk data;

	enum {eString = 1};

	static STokenDesc commands[] =
	{
		{ eString, "String" },

		{ 0,       0        }
	};

	while ((cmd = shGetObject(&buf, commands, &name, &data)) > 0)
	{
		switch (cmd)
		{
		case eString:
			{
				if (eType != eType_STRING)
					break;
				tuk szScr = data;
				if (*szScr == '"')
					szScr++;
				i32 n = 0;
				while (szScr[n] != 0)
				{
					if (szScr[n] == '"')
						szScr[n] = ' ';
					n++;
				}
				if (!stricmp(szName, name))
				{
					strcpy(sRet, data);
					break;
				}
			}
			break;
		}
	}

	SAFE_DELETE_ARRAY(pTemp);

	if (sRet[0])
		return sRet;
	else
		return NULL;
}

SFXParam* CShaderMan::mfGetFXParameter(std::vector<SFXParam>& Params, tukk param)
{
	u32 j;
	for (j = 0; j < Params.size(); j++)
	{
		SFXParam* pr = &Params[j];
		char nameParam[256];
		i32 n = 0;
		tukk szSrc = pr->m_Name.c_str();
		i32 nParams = 1;
		while (*szSrc != 0)
		{
			if (*szSrc == '[')
			{
				nParams = atoi(&szSrc[1]);
				break;
			}
			nameParam[n++] = *szSrc;
			szSrc++;
		}
		nameParam[n] = 0;
		if (!stricmp(nameParam, param))
			return pr;
	}
	return NULL;
}
SFXSampler* CShaderMan::mfGetFXSampler(std::vector<SFXSampler>& Params, tukk param)
{
	u32 j;
	for (j = 0; j < Params.size(); j++)
	{
		SFXSampler* pr = &Params[j];
		char nameParam[256];
		i32 n = 0;
		tukk szSrc = pr->m_Name.c_str();
		i32 nParams = 1;
		while (*szSrc != 0)
		{
			if (*szSrc == '[')
			{
				nParams = atoi(&szSrc[1]);
				break;
			}
			nameParam[n++] = *szSrc;
			szSrc++;
		}
		nameParam[n] = 0;
		if (!stricmp(nameParam, param))
			return pr;
	}
	return NULL;
}
SFXTexture* CShaderMan::mfGetFXTexture(std::vector<SFXTexture>& Params, tukk param)
{
	u32 j;
	for (j = 0; j < Params.size(); j++)
	{
		SFXTexture* pr = &Params[j];
		char nameParam[256];
		i32 n = 0;
		tukk szSrc = pr->m_Name.c_str();
		i32 nParams = 1;
		while (*szSrc != 0)
		{
			if (*szSrc == '[')
			{
				nParams = atoi(&szSrc[1]);
				break;
			}
			nameParam[n++] = *szSrc;
			szSrc++;
		}
		nameParam[n] = 0;
		if (!stricmp(nameParam, param))
			return pr;
	}
	return NULL;
}

// We have to parse part of the shader to enumerate public techniques
bool CShaderMan::mfAddFXShaderNames(tukk szName, std::vector<string>* ShaderNames, bool bUpdateCRC)
{
	bool bRes = true;
	SShaderBin* pBin = m_Bin.GetBinShader(szName, false, 0);
	if (!pBin)
		return false;
	if (bUpdateCRC)
	{
		u32 nCRC32 = pBin->ComputeCRC();
		if (nCRC32 != pBin->m_CRC32)
		{
			FXShaderBinValidCRCItor itor = gRenDev->m_cEF.m_Bin.m_BinValidCRCs.find(pBin->m_dwName);
			if (itor == gRenDev->m_cEF.m_Bin.m_BinValidCRCs.end())
				gRenDev->m_cEF.m_Bin.m_BinValidCRCs.insert(FXShaderBinValidCRCItor::value_type(pBin->m_dwName, false));

			m_Bin.DeleteFromCache(pBin);
			pBin = m_Bin.GetBinShader(szName, false, nCRC32);
			if (!pBin)
				return false;
		}
	}

	// Do not parse techniques for consoles
	if (ShaderNames)
		bRes &= m_Bin.ParseBinFX_Dummy(pBin, *ShaderNames, szName);

	return bRes;
}

CTexture* CShaderMan::mfParseFXTechnique_LoadShaderTexture(STexSamplerRT* smp, tukk szName, SShaderPass* pShPass, CShader* ef, i32 nIndex, byte ColorOp, byte AlphaOp, byte ColorArg, byte AlphaArg)
{
	CTexture* tp = NULL;
	if (!szName || !szName[0] || gRenDev->m_bShaderCacheGen) // Sampler without texture specified
		return NULL;
	if (szName[0] == '$')
	{
		tp = mfCheckTemplateTexName(szName, (ETEX_Type)smp->m_eTexType);
		if (tp)
			tp->AddRef();
	}
	else
		smp->m_nTexFlags |= FT_DONT_STREAM; // disable streaming for explicitly specified textures
	if (!tp)
	{
		tp = mfTryToLoadTexture(szName, smp, smp->GetTexFlags(), false);
	}

	return tp;
}
