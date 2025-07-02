// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   Parser.h : Script parser declarations.

   Revision история:
* Created by Honich Andrey

   =============================================================================*/

#ifndef PARSER_H
#define PARSER_H

struct STokenDesc
{
	i32   id;
	tuk token;
};
i32 shGetObject(tuk* buf, STokenDesc* tokens, tuk* name, tuk* data);

extern tukk kWhiteSpace;
extern FXMacro sStaticMacros;

bool   SkipChar(u32 ch);

void   fxParserInit(void);

void   SkipCharacters(tukk* buf, tukk toSkip);
void   SkipCharacters(tuk* buf, tukk toSkip);
void   RemoveCR(tuk buf);
void   SkipComments(tuk* buf, bool bSkipWhiteSpace);

bool   fxIsFirstPass(tukk buf);
void   fxRegisterEnv(tukk szStr);

i32    shFill(tuk* buf, tuk dst, i32 nSize = -1);
i32    fxFill(tuk* buf, tuk dst, i32 nSize = -1);
tuk  fxFillPr(tuk* buf, tuk dst);
tuk  fxFillPrC(tuk* buf, tuk dst);
tuk  fxFillNumber(tuk* buf, tuk dst);
i32    fxFillCR(tuk* buf, tuk dst);

bool   shGetBool(tukk buf);
float  shGetFloat(tukk buf);
void   shGetFloat(tukk buf, float* v1, float* v2);
i32    shGetInt(tukk buf);
i32    shGetHex(tukk buf);
uint64 shGetHex64(tukk buf);
void   shGetVector(tukk buf, Vec3& v);
void   shGetVector(tukk buf, float v[3]);
void   shGetVector4(tukk buf, vec4_t& v);
void   shGetColor(tukk buf, ColorF& v);
void   shGetColor(tukk buf, float v[4]);
i32    shGetVar(tukk* buf, tuk* vr, tuk* val);

#endif
