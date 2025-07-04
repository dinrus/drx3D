// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   Parser.cpp : Script parser implementations.

   Revision история:
* Created by Honich Andrey

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>

tukk kWhiteSpace = " ,";
tuk pCurCommand;

void SkipCharacters(tuk* buf, tukk toSkip)
{
	char theChar;
	tukk skip;

	while ((theChar = **buf) != 0)
	{
		if (theChar >= 0x20)
		{
			skip = (tukk)toSkip;
			while (*skip)
			{
				if (theChar == *skip)
					break;
				++skip;
			}
			if (*skip == 0)
				return;
		}
		++*buf;
	}
}
void SkipCharacters(tukk* buf, tukk toSkip)
{
	char theChar;
	tukk skip;

	while ((theChar = **buf) != 0)
	{
		if (theChar >= 0x20)
		{
			skip = (tukk)toSkip;
			while (*skip)
			{
				if (theChar == *skip)
					break;
				++skip;
			}
			if (*skip == 0)
				return;
		}
		++*buf;
	}
}

void RemoveCR(tuk pbuf)
{
	while (*pbuf)
	{
		if (*pbuf == 0xd)
			*pbuf = 0x20;
		pbuf++;
	}
}

FXMacro sStaticMacros;

bool SkipChar(u32 ch)
{
	bool res = ch <= 0x20;

	res |= (ch - 0x21) < 2;  // !"
	res |= (ch - 0x26) < 10; // &'()*+,-./
	res |= (ch - 0x3A) < 6;  // :;<=>?
	res |= ch == 0x5B;       // [
	// cppcheck-suppress badBitmaskCheck
	res |= ch == 0x5D;      // ]
	res |= (ch - 0x7B) < 3; // {|}

	return res;
}

// Determine is this preprocessor directive belongs to first pass or second one
bool fxIsFirstPass(tukk buf)
{
	char com[1024];
	char tok[256];
	fxFillCR((tuk*)&buf, com);
	tuk s = com;
	while (*s != 0)
	{
		tuk pStart = fxFillPr(&s, tok);
		if (tok[0] == '%' && tok[1] == '_')
			return false;
	}
	return true;
}

static void fxAddMacro(tukk Name, tuk Macro, FXMacro& Macros)
{
	SMacroFX pr;

	if (Name[0] == '%')
	{
		pr.m_nMask = shGetHex(&Macro[0]);
#ifdef _DEBUG
		FXMacroItor it = Macros.find(CONST_TEMP_STRING(Name));
		if (it != Macros.end())
			assert(0);
#endif
	}
	else
		pr.m_nMask = 0;
	pr.m_szMacro = Macro ? Macro : "";
	FXMacroItor it = Macros.find(CONST_TEMP_STRING(Name));
	if (it != Macros.end())
		Macros.erase(Name);
	Macros.insert(FXMacroItor::value_type(Name, pr));

	/*it = Macros.find(Name);
	   if (it != Macros.end())
	   {
	   i32 nnn = 0;
	   }*/
}

void fxParserInit(void)
{
}

void fxRegisterEnv(tukk szStr)
{
	// deprecated
}

tuk fxFillPr(tuk* buf, tuk dst)
{
	i32 n = 0;
	char ch;
	while ((ch = **buf) != 0)
	{
		if (!SkipChar(ch))
			break;
		++*buf;
	}
	tuk pStart = *buf;
	while ((ch = **buf) != 0)
	{
		if (SkipChar(ch))
			break;
		dst[n++] = ch;
		++*buf;
	}
	dst[n] = 0;
	return pStart;
}

tuk fxFillPrC(tuk* buf, tuk dst)
{
	i32 n = 0;
	char ch;
	while ((ch = **buf) != 0)
	{
		if (!SkipChar(ch))
			break;
		++*buf;
	}
	tuk pStart = *buf;
	while ((ch = **buf) != 0)
	{
		if (ch != ',' && SkipChar(ch))
			break;
		dst[n++] = ch;
		++*buf;
	}
	dst[n] = 0;
	return pStart;
}

tuk fxFillNumber(tuk* buf, tuk dst)
{
	i32 n = 0;
	char ch;
	while ((ch = **buf) != 0)
	{
		if (!SkipChar(ch))
			break;
		++*buf;
	}
	tuk pStart = *buf;
	while ((ch = **buf) != 0)
	{
		if (ch != '.' && SkipChar(ch))
			break;
		dst[n++] = ch;
		++*buf;
	}
	dst[n] = 0;
	return pStart;
}

i32 shFill(tuk* buf, tuk dst, i32 nSize)
{
	i32 n = 0;
	SkipCharacters(buf, kWhiteSpace);

	while (**buf > 0x20)
	{
		dst[n++] = **buf;
		++*buf;

		if (nSize > 0 && n == nSize)
		{
			break;
		}
	}

	dst[n] = 0;
	return n;
}
i32 fxFill(tuk* buf, tuk dst, i32 nSize)
{
	i32 n = 0;
	SkipCharacters(buf, kWhiteSpace);

	while (**buf != ';')
	{
		if (**buf == 0)
			break;
		dst[n++] = **buf;
		++*buf;

		if (nSize > 0 && n == nSize)
		{
			dst[n - 1] = 0;
			return 1;
		}
	}

	PREFAST_ASSUME(*buf);
	dst[n] = 0;
	if (**buf == ';')
		++*buf;
	return n;
}

i32 fxFillCR(tuk* buf, tuk dst)
{
	i32 n = 0;
	SkipCharacters(buf, kWhiteSpace);
	while (**buf != 0xa)
	{
		if (**buf == 0)
			break;
		dst[n++] = **buf;
		++*buf;
	}
	dst[n] = 0;
	return n;
}

//================================================================================

bool shGetBool(tukk buf)
{
	if (!buf)
		return false;

	if (!strnicmp(buf, "yes", 3))
		return true;

	if (!strnicmp(buf, "true", 4))
		return true;

	if (!strnicmp(buf, "on", 2))
		return true;

	if (!strncmp(buf, "1", 1))
		return true;

	return false;
}

float shGetFloat(tukk buf)
{
	if (!buf)
		return 0;
	float f = 0;

	i32 res = sscanf(buf, "%f", &f);
	assert(res);

	return f;
}

void shGetFloat(tukk buf, float* v1, float* v2)
{
	if (!buf)
		return;
	float f = 0, f1 = 0;

	i32 n = sscanf(buf, "%f %f", &f, &f1);
	if (n == 1)
	{
		*v1 = f;
		*v2 = f;
	}
	else
	{
		*v1 = f;
		*v2 = f1;
	}
}

i32 shGetInt(tukk buf)
{
	if (!buf)
		return 0;
	i32 i = 0;

	if (buf[0] == '0' && buf[1] == 'x')
	{
		i32 res = sscanf(&buf[2], "%x", &i);
		assert(res);
	}
	else
	{
		i32 res = sscanf(buf, "%i", &i);
		assert(res);
	}

	return i;
}

i32 shGetHex(tukk buf)
{
	if (!buf)
		return 0;
	i32 i = 0;

	i32 res = sscanf(buf, "%x", &i);
	assert(res);

	return i;
}

uint64 shGetHex64(tukk buf)
{
	if (!buf)
		return 0;
#if defined(__GNUC__)
	zu64 i = 0;
	i32 res = sscanf(buf, "%llx", &i);
	assert(res);
	return (uint64)i;
#else
	uint64 i = 0;
	i32 res = sscanf(buf, "%I64x", &i);
	assert(res);
	return i;
#endif
}

void shGetVector(tukk buf, Vec3& v)
{
	if (!buf)
		return;
	i32 res = sscanf(buf, "%f %f %f", &v[0], &v[1], &v[2]);
	assert(res);
}

void shGetVector(tukk buf, float v[3])
{
	if (!buf)
		return;
	i32 res = sscanf(buf, "%f %f %f", &v[0], &v[1], &v[2]);
	assert(res);
}

void shGetVector4(tukk buf, Vec4& v)
{
	if (!buf)
		return;
	i32 res = sscanf(buf, "%f %f %f %f", &v.x, &v.y, &v.z, &v.w);
	assert(res);
}

static struct SColAsc
{
	tuk  nam;
	ColorF col;

	SColAsc(tuk name, const ColorF& c)
	{
		nam = name;
		col = c;
	}
} sCols[] =
{
	SColAsc("Aquamarine",        Col_Aquamarine),
	SColAsc("Black",             Col_Black),
	SColAsc("Blue",              Col_Blue),
	SColAsc("BlueViolet",        Col_BlueViolet),
	SColAsc("Brown",             Col_Brown),
	SColAsc("CadetBlue",         Col_CadetBlue),
	SColAsc("Coral",             Col_Coral),
	SColAsc("CornflowerBlue",    Col_CornflowerBlue),
	SColAsc("Cyan",              Col_Cyan),
	SColAsc("DarkGray",          Col_DarkGray),
	SColAsc("DarkGrey",          Col_DarkGrey),
	SColAsc("DarkGreen",         Col_DarkGreen),
	SColAsc("DarkOliveGreen",    Col_DarkOliveGreen),
	SColAsc("DarkOrchid",        Col_DarkOrchid),
	SColAsc("DarkSlateBlue",     Col_DarkSlateBlue),
	SColAsc("DarkSlateGray",     Col_DarkSlateGray),
	SColAsc("DarkSlateGrey",     Col_DarkSlateGrey),
	SColAsc("DarkTurquoise",     Col_DarkTurquoise),
	SColAsc("DarkWood",          Col_DarkWood),
	SColAsc("DeepPink",          Col_DeepPink),
	SColAsc("DimGray",           Col_DimGray),
	SColAsc("DimGrey",           Col_DimGrey),
	SColAsc("FireBrick",         Col_FireBrick),
	SColAsc("ForestGreen",       Col_ForestGreen),
	SColAsc("Gold",              Col_Gold),
	SColAsc("Goldenrod",         Col_Goldenrod),
	SColAsc("Gray",              Col_Gray),
	SColAsc("Grey",              Col_Grey),
	SColAsc("Green",             Col_Green),
	SColAsc("GreenYellow",       Col_GreenYellow),
	SColAsc("IndianRed",         Col_IndianRed),
	SColAsc("Khaki",             Col_Khaki),
	SColAsc("LightBlue",         Col_LightBlue),
	SColAsc("LightGray",         Col_LightGray),
	SColAsc("LightGrey",         Col_LightGrey),
	SColAsc("LightSteelBlue",    Col_LightSteelBlue),
	SColAsc("LightWood",         Col_LightWood),
	SColAsc("Lime",              Col_Lime),
	SColAsc("LimeGreen",         Col_LimeGreen),
	SColAsc("Magenta",           Col_Magenta),
	SColAsc("Maroon",            Col_Maroon),
	SColAsc("MedianWood",        Col_MedianWood),
	SColAsc("MediumAquamarine",  Col_MediumAquamarine),
	SColAsc("MediumBlue",        Col_MediumBlue),
	SColAsc("MediumForestGreen", Col_MediumForestGreen),
	SColAsc("MediumGoldenrod",   Col_MediumGoldenrod),
	SColAsc("MediumOrchid",      Col_MediumOrchid),
	SColAsc("MediumSeaGreen",    Col_MediumSeaGreen),
	SColAsc("MediumSlateBlue",   Col_MediumSlateBlue),
	SColAsc("MediumSpringGreen", Col_MediumSpringGreen),
	SColAsc("MediumTurquoise",   Col_MediumTurquoise),
	SColAsc("MediumVioletRed",   Col_MediumVioletRed),
	SColAsc("MidnightBlue",      Col_MidnightBlue),
	SColAsc("Navy",              Col_Navy),
	SColAsc("NavyBlue",          Col_NavyBlue),
	SColAsc("Orange",            Col_Orange),
	SColAsc("OrangeRed",         Col_OrangeRed),
	SColAsc("Orchid",            Col_Orchid),
	SColAsc("PaleGreen",         Col_PaleGreen),
	SColAsc("Pink",              Col_Pink),
	SColAsc("Plum",              Col_Plum),
	SColAsc("Red",               Col_Red),
	SColAsc("Salmon",            Col_Salmon),
	SColAsc("SeaGreen",          Col_SeaGreen),
	SColAsc("Sienna",            Col_Sienna),
	SColAsc("SkyBlue",           Col_SkyBlue),
	SColAsc("SlateBlue",         Col_SlateBlue),
	SColAsc("SpringGreen",       Col_SpringGreen),
	SColAsc("SteelBlue",         Col_SteelBlue),
	SColAsc("Tan",               Col_Tan),
	SColAsc("Thistle",           Col_Thistle),
	SColAsc("Turquoise",         Col_Turquoise),
	SColAsc("Violet",            Col_Violet),
	SColAsc("VioletRed",         Col_VioletRed),
	SColAsc("Wheat",             Col_Wheat),
	SColAsc("White",             Col_White),
	SColAsc("Yellow",            Col_Yellow),
	SColAsc("YellowGreen",       Col_YellowGreen),

	SColAsc(NULL,                ColorF(1.0f,           1.0f, 1.0f))
};

#include <ctype.h>

void shGetColor(tukk buf, ColorF& v)
{
	char name[64];
	i32 n;

	if (!buf)
	{
		v = Col_White;
		return;
	}
	if (buf[0] == '{')
		buf++;
	if (isalpha((u8)buf[0]))
	{
		n = 0;
		float scal = 1;
		drx_strcpy(name, buf);
		char nm[64];
		if (strchr(buf, '*'))
		{
			while (buf[n] != '*')
			{
				if (buf[n] == 0x20)
					break;
				nm[n] = buf[n];
				n++;
			}
			nm[n] = 0;
			if (buf[n] == 0x20)
			{
				while (buf[n] != '*')
					n++;
			}
			n++;
			while (buf[n] == 0x20)
				n++;
			scal = shGetFloat(&buf[n]);
			drx_strcpy(name, nm);
		}
		n = 0;
		while (sCols[n].nam)
		{
			if (!stricmp(sCols[n].nam, name))
			{
				v = sCols[n].col;
				if (scal != 1)
					v.ScaleCol(scal);
				return;
			}
			n++;
		}
	}
	n = 0;
	while (true)
	{
		if (n == 4)
			break;
		char par[64];
		par[0] = 0;
		fxFillNumber((tuk*)&buf, par);
		if (!par[0])
			break;
		v[n++] = (float)atof(par);
	}

	//v.Clamp();
}

void shGetColor(tukk buf, float v[4])
{
	char name[64];

	if (!buf)
	{
		v[0] = 1.0f;
		v[1] = 1.0f;
		v[2] = 1.0f;
		v[3] = 1.0f;
		return;
	}
	if (isalpha((u8)buf[0]))
	{
		i32 n = 0;
		float scal = 1;
		drx_strcpy(name, buf);
		char nm[64];
		if (strchr(buf, '*'))
		{
			while (buf[n] != '*')
			{
				if (buf[n] == 0x20)
					break;
				nm[n] = buf[n];
				n++;
			}
			nm[n] = 0;
			if (buf[n] == 0x20)
			{
				while (buf[n] != '*')
					n++;
			}
			n++;
			while (buf[n] == 0x20)
				n++;
			scal = shGetFloat(&buf[n]);
			drx_strcpy(name, nm);
		}
		n = 0;
		while (sCols[n].nam)
		{
			if (!stricmp(sCols[n].nam, name))
			{
				v[0] = sCols[n].col[0];
				v[1] = sCols[n].col[1];
				v[2] = sCols[n].col[2];
				v[3] = sCols[n].col[3];
				if (scal != 1)
				{
					v[0] *= scal;
					v[1] *= scal;
					v[2] *= scal;
				}
				return;
			}
			n++;
		}
	}
	i32 n = sscanf(buf, "%f %f %f %f", &v[0], &v[1], &v[2], &v[3]);
	switch (n)
	{
	case 0:
		v[0] = v[1] = v[2] = v[3] = 1.0f;
		break;

	case 1:
		v[1] = v[2] = v[3] = 1.0f;
		break;

	case 2:
		v[2] = v[3] = 1.0f;
		break;

	case 3:
		v[3] = 1.0f;
		break;
	}
	//v.Clamp();
}

//=========================================================================================

tuk GetAssignmentText(tuk* buf)
{
	SkipCharacters(buf, kWhiteSpace);
	tuk result = *buf;

	char theChar;

	PREFAST_SUPPRESS_WARNING(28182)
	while ((theChar = **buf) != 0)
	{
		if (theChar == '[')
		{
			while ((theChar = **buf) != ']')
			{
				if (theChar == 0 || theChar == ';')
					break;
				++*buf;
			}
			continue;
		}
		if (theChar <= 0x20 || theChar == ';')
			break;
		++*buf;
	}

	PREFAST_ASSUME(*buf);
	**buf = 0;
	if (theChar)
		++*buf;
	return result;
}

tuk GetSubText(tuk* buf, char open, char close)
{
	if (**buf == 0 || **buf != open)
		return 0;
	++*buf;
	tuk result = *buf;

	char theChar;
	long skip = 1;

	if (open == close)
		open = 0;
	while ((theChar = **buf) != 0)
	{
		if (theChar == open)
			++skip;
		if (theChar == close)
		{
			if (--skip == 0)
			{
				**buf = 0;
				++*buf;
				break;
			}
		}
		++*buf;
	}
	return result;
}

inline static i32 IsComment(tukk buf)
{
	if (!buf)
		return 0;

	if (buf[0] == '/' && buf[1] == '/')
		return 2;

	if (buf[0] == '/' && buf[1] == '*')
		return 3;

	return 0;
}

void SkipComments(tuk* buf, bool bSkipWhiteSpace)
{
	i32 n;
	static i32 m;

	while ((n = IsComment(*buf)) != 0)
	{
		switch (n)
		{
		case 2:
			// skip comment lines.
			*buf = strchr(*buf, '\n');
			if (*buf && bSkipWhiteSpace)
				SkipCharacters(buf, kWhiteSpace);
			break;

		case 3:
			// skip comment blocks.
			m = 0;
			do
			{
				*buf = strchr(*buf, '*');
				if (!(*buf))
					break;
				if ((*buf)[-1] == '/')
				{
					*buf += 1;
					m++;
				}
				else if ((*buf)[1] == '/')
				{
					*buf += 2;
					m--;
				}
				else
					*buf += 1;
			}
			while (m);
			if (!(*buf))
			{
				iLog->Log("Warning: Comment lines aren't closed\n");
				break;
			}
			if (bSkipWhiteSpace)
				SkipCharacters(buf, kWhiteSpace);
			break;
		}
	}
}

void fxSkipTillCR(tukk* buf)
{
	char ch;
	while ((ch = **buf) != 0)
	{
		if (ch == 0xa)
			break;
		++*buf;
	}
}

bool fxCheckMacroses(tukk* str, i32 nPass)
{
	char tmpBuf[1024];
	byte bRes[64];
	byte bOr[64];
	i32 nLevel = 0;
	i32 i;
	while (true)
	{
		SkipCharacters(str, kWhiteSpace);
		if (**str == '(')
		{
			++*str;
			i32 n = 0;
			i32 nD = 0;
			while (true)
			{
				if (**str == '(')
					n++;
				else if (**str == ')')
				{
					if (!n)
					{
						tmpBuf[nD] = 0;
						++*str;
						break;
					}
					n--;
				}
				else if (**str == 0)
					return false;
				tmpBuf[nD++] = **str;
				++*str;
			}
			tukk s = &tmpBuf[0];
			bRes[nLevel] = fxCheckMacroses(&s, nPass);
			nLevel++;
			bOr[nLevel] = 255;
		}
		else
		{
			i32 n = 0;
			while (true)
			{
				if (**str == '|' || **str == '&' || **str == 0)
					break;
				if (**str <= 0x20)
					break;
				tmpBuf[n++] = **str;
				++*str;
			}
			tmpBuf[n] = 0;
			if (tmpBuf[0] != 0)
			{
				tukk s = tmpBuf;
				bool bNeg = false;
				if (s[0] == '!')
				{
					bNeg = true;
					s++;
				}
				const SMacroBinFX* pFound;
				if (isdigit((unsigned)s[0]))
				{
					if ((s[0] == '0' && s[1] == 'x') || s[0] != 0)
						pFound = (SMacroBinFX*)1;
					else
						pFound = NULL;
				}
				else
				{
					bool bKey = false;
					u32 nTok = CParserBin::NextToken(s, tmpBuf, bKey);
					if (nTok == eT_unknown)
						nTok = CParserBin::GetCRC32(tmpBuf);
					pFound = CParserBin::FindMacro(nTok, CParserBin::GetStaticMacroses());
				}
				bRes[nLevel] = (pFound) ? true : false;
				if (bNeg)
					bRes[nLevel] = !bRes[nLevel];
				nLevel++;
				bOr[nLevel] = 255;
			}
			else
				assert(0);
		}
		SkipCharacters(str, kWhiteSpace);
		if (**str == 0)
			break;
		tukk s = *str;
		if (s[0] == '|' && s[1] == '|')
		{
			bOr[nLevel] = true;
			*str = s + 2;
		}
		else if (s[0] == '&' && s[1] == '&')
		{
			bOr[nLevel] = false;
			*str = s + 2;
		}
		else
			assert(0);
	}
	byte Res = false;
	for (i = 0; i < nLevel; i++)
	{
		if (!i)
			Res = bRes[i];
		else
		{
			assert(bOr[i] != 255);
			if (bOr[i])
				Res = Res | bRes[i];
			else
				Res = Res & bRes[i];
		}
	}
	return Res != 0;
}

bool fxIgnorePreprBlock(tuk* buf)
{
	i32 nLevel = 0;
	tuk start = *buf;
	bool bEnded = false;
	SkipCharacters(buf, kWhiteSpace);
	SkipComments(buf, true);
	PREFAST_SUPPRESS_WARNING(6011)
	while (**buf != 0)
	{
		char ch;
		while ((ch = **buf) && SkipChar((unsigned)ch))
		{
			tuk b = *buf;
			while ((ch = **buf) != 0)
			{
				if (ch == '/' && IsComment(*buf))
					break;
				if (!SkipChar(ch))
					break;
				++*buf;
			}
			SkipComments(buf, true);
		}
		i32 n = 0;
		tuk posS = *buf;
		tuk st = posS;
		PREFAST_ASSUME(posS);
		if (*posS == '#')
		{
			posS++;
			if (SkipChar(posS[0]))
			{
				while ((ch = *posS) != 0)
				{
					if (!SkipChar(ch))
						break;
					posS++;
				}
			}
			if (posS[0] == 'i' && posS[1] == 'f')
			{
				nLevel++;
				*buf = posS + 2;
				continue;
			}
			if (!strncmp(posS, "endif", 5))
			{
				if (!nLevel)
				{
					*buf = st;
					bEnded = true;
					break;
				}
				nLevel--;
				*buf = posS + 4;
			}
			else if (!strncmp(posS, "else", 4) || !strncmp(posS, "elif", 4))
			{
				if (!nLevel)
				{
					*buf = st;
					break;
				}
				*buf = posS + 4;
			}
		}
		while ((ch = **buf))
		{
			if (ch == '/' && IsComment(*buf))
				break;
			if (SkipChar((unsigned)ch))
				break;
			++*buf;
		}
	}
	PREFAST_ASSUME(*buf);
	if (!**buf)
	{
		assert(0);
		Warning("Couldn't find #endif directive for associated #ifdef");
		return false;
	}

	return bEnded;
}

i32 shGetObject(tuk* buf, STokenDesc* tokens, tuk* name, tuk* data)
{
start:
	if (!*buf)
		return 0;
	SkipCharacters(buf, kWhiteSpace);
	SkipComments(buf, true);

	if (!(*buf) || !**buf)
		return -2;

	tuk b = *buf;
	if (b[0] == '#')
	{
		char nam[1024];
		bool bPrepr = false;
		if (!strncmp(b, "#if", 3))
		{
			bPrepr = true;
			fxFillPr(buf, nam);
			fxFillCR(buf, nam);
			tukk s = &nam[0];
			bool bRes = fxCheckMacroses(&s, 0);
			if (b[2] == 'n')
				bRes = !bRes;
			if (!bRes)
			{
				sfxIFDef.AddElem(false);
				fxIgnorePreprBlock(buf);
			}
			else
				sfxIFDef.AddElem(false);
		}
		else if (!strncmp(b, "#else", 5))
		{
			fxFillPr(buf, nam);
			bPrepr = true;
			i32 nLevel = sfxIFDef.Num() - 1;
			if (nLevel < 0)
			{
				assert(0);
				Warning("#else without #ifdef");
				return false;
			}
			if (sfxIFDef[nLevel] == true)
			{
				bool bEnded = fxIgnorePreprBlock(buf);
				if (!bEnded)
				{
					assert(0);
					Warning("#else or #elif after #else");
					return -1;
				}
			}
		}
		else if (!strncmp(b, "#elif", 5))
		{
			fxFillPr(buf, nam);
			bPrepr = true;
			i32 nLevel = sfxIFDef.Num() - 1;
			if (nLevel < 0)
			{
				assert(0);
				Warning("#elif without #ifdef");
				return -1;
			}
			if (sfxIFDef[nLevel] == true)
			{
				fxIgnorePreprBlock(buf);
			}
			else
			{
				fxFillCR(buf, nam);
				tukk s = &nam[0];
				bool bRes = fxCheckMacroses(&s, 0);
				if (!bRes)
					fxIgnorePreprBlock(buf);
				else
					sfxIFDef[nLevel] = true;
			}
		}
		else if (!strncmp(b, "#endif", 6))
		{
			fxFillPr(buf, nam);
			bPrepr = true;
			i32 nLevel = sfxIFDef.Num() - 1;
			if (nLevel < 0)
			{
				assert(0);
				Warning("#endif without #ifdef");
				return -1;
			}
			sfxIFDef.Remove(nLevel);
		}
		if (bPrepr)
			goto start;
	}

	STokenDesc* ptokens = tokens;
	while (tokens->id != 0)
	{
		if (!strnicmp(tokens->token, *buf, strlen(tokens->token)))
		{
			pCurCommand = *buf;
			break;
		}
		++tokens;
	}
	if (tokens->id == 0)
	{
		tuk p = strchr(*buf, '\n');

		char pp[1024];
		if (p)
		{
			drx_strcpy(pp, *buf, (size_t)(p - *buf));
			*buf = p;
		}
		else
		{
			drx_strcpy(pp, *buf);
		}

		iLog->Log("Warning: Found token '%s' which was not one of the list (Skipping).\n", pp);
		while (ptokens->id != 0)
		{
			iLog->Log("    %s\n", ptokens->token);
			ptokens++;
		}
		return 0;
	}
	*buf += strlen(tokens->token);
	SkipCharacters(buf, kWhiteSpace);

	*name = GetSubText(buf, 0x27, 0x27);
	SkipCharacters(buf, kWhiteSpace);

	if (**buf == '=')
	{
		++*buf;
		*data = GetAssignmentText(buf);
	}
	else
	{
		*data = GetSubText(buf, '(', ')');
		if (!*data)
			*data = GetSubText(buf, '{', '}');
	}

	return tokens->id;
}
