// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MaterialHelpers.h
//  Version:     v1.00
//  Created:     6/6/2014 by NielsF.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2012
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MaterialHelpers_h__
#define __MaterialHelpers_h__
#pragma once

#include <drx3D/Eng3D/IMaterial.h>

// Описание:
//   Namespace "implementation", not a class "implementation", no member-variables, only const functions;
//   Used to encapsulate the material-definition/io into DinrusX3dEng (and make it plugable that way).
struct MaterialHelpers : public IMaterialHelpers
{
	//////////////////////////////////////////////////////////////////////////
	virtual EEfResTextures FindTexSlot(tukk texName) const final;
	virtual tukk    FindTexName(EEfResTextures texSlot) const final;
	virtual tukk    LookupTexName(EEfResTextures texSlot) const final;
	virtual tukk    LookupTexEnum(EEfResTextures texSlot) const final;
	virtual tukk    LookupTexSuffix(EEfResTextures texSlot) const final;
	virtual bool           IsAdjustableTexSlot(EEfResTextures texSlot) const final;

	//////////////////////////////////////////////////////////////////////////

	virtual tukk GetNameFromTextureType(u8 texType) const final;
	virtual u8       GetTextureTypeFromName(tukk szName) const final;
	virtual bool        IsTextureTypeExposed(u8 texType) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual bool SetGetMaterialParamFloat(IRenderShaderResources& pShaderResources, tukk sParamName, float& v, bool bGet) const final;
	virtual bool SetGetMaterialParamVec3(IRenderShaderResources& pShaderResources, tukk sParamName, Vec3& v, bool bGet) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetTexModFromXml(SEfTexModificator& pShaderResources, const XmlNodeRef& node) const final;
	virtual void SetXmlFromTexMod(const SEfTexModificator& pShaderResources, XmlNodeRef& node) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetTexturesFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node, tukk szBaseFileName) const final;
	virtual void SetXmlFromTextures(const SInputShaderResources& pShaderResources, XmlNodeRef& node, tukk szBaseFileName) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetVertexDeformFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const final;
	virtual void SetXmlFromVertexDeform(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetDetailDecalFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const final;
	virtual void SetXmlFromDetailDecal(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetLightingFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const final;
	virtual void SetXmlFromLighting(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void SetShaderParamsFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const final;
	virtual void SetXmlFromShaderParams(const SInputShaderResources& pShaderResources, XmlNodeRef& node) const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void MigrateXmlLegacyData(SInputShaderResources& pShaderResources, const XmlNodeRef& node) const final;
};

#endif
