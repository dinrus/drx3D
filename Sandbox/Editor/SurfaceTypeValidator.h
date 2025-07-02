// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SURFACETYPEVALIDATOR_H__
#define __SURFACETYPEVALIDATOR_H__

struct pe_params_part;

class CSurfaceTypeValidator
{
public:
	void Validate();

private:
	void GetUsedSubMaterials(pe_params_part* pPart, char usedSubMaterials[]);
};

#endif //__SURFACETYPEVALIDATOR_H__

