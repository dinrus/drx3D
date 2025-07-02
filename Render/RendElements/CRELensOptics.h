// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CRELensOptics : public CRenderElement
{
public:
	CRELensOptics(void);
	~CRELensOptics(void) {}

	virtual void mfExport(struct SShaderSerializeContext& SC)                 {};
	virtual void mfImport(struct SShaderSerializeContext& SC, u32& offset) {};
};
