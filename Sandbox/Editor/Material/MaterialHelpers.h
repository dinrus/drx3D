// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Material.h"
#include "IEditorImpl.h"

namespace MaterialHelpers
{
//////////////////////////////////////////////////////////////////////////
//! Get public parameters of material in variable block.
CVarBlock* GetPublicVars(SInputShaderResources& pShaderResources);

//! Sets variable block of public shader parameters.
//! VarBlock must be in same format as returned by GetPublicVars().
void SetPublicVars(CVarBlock* pPublicVars, SInputShaderResources& pInputShaderResources);
void SetPublicVars(CVarBlock* pPublicVars, SInputShaderResources& pInputShaderResources, IRenderShaderResources* pRenderShaderResources, IShader* pShader);

//////////////////////////////////////////////////////////////////////////
CVarBlock* GetShaderGenParamsVars(IShader* pShader, uint64 nShaderGenMask);
uint64     SetShaderGenParamsVars(IShader* pShader, CVarBlock* pBlock);

//////////////////////////////////////////////////////////////////////////
inline EEfResTextures FindTexSlot(tukk texName)            { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().FindTexSlot(texName); }
inline tukk    FindTexName(EEfResTextures texSlot)         { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().FindTexName(texSlot); }
inline tukk    LookupTexName(EEfResTextures texSlot)       { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().LookupTexName(texSlot); }
inline bool           IsAdjustableTexSlot(EEfResTextures texSlot) { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().IsAdjustableTexSlot(texSlot); }

//////////////////////////////////////////////////////////////////////////
inline tukk GetNameFromTextureType(u8 texType)      { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().GetNameFromTextureType(texType); }
inline u8       GetTextureTypeFromName(tukk szName) { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().GetTextureTypeFromName(szName); }
inline bool        IsTextureTypeExposed(u8 texType)        { return GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().IsTextureTypeExposed(texType); }

//////////////////////////////////////////////////////////////////////////
inline void SetTexModFromXml(SEfTexModificator& pShaderResources, const XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetTexModFromXml(pShaderResources, node); }
inline void SetXmlFromTexMod(const SEfTexModificator& pShaderResources, XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetXmlFromTexMod(pShaderResources, node); }

//////////////////////////////////////////////////////////////////////////
inline void SetTexturesFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node, tukk szBaseFileName) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetTexturesFromXml(pShaderResources, node, szBaseFileName); }
inline void SetXmlFromTextures(const SInputShaderResources& pShaderResources, XmlNodeRef& node, tukk szBaseFileName) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetXmlFromTextures(pShaderResources, node, szBaseFileName); }

//////////////////////////////////////////////////////////////////////////
inline void SetVertexDeformFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetVertexDeformFromXml(pShaderResources, node); }
inline void SetXmlFromVertexDeform(const SInputShaderResources& pShaderResources, XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetXmlFromVertexDeform(pShaderResources, node); }

//////////////////////////////////////////////////////////////////////////
inline void SetDetailDecalFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetDetailDecalFromXml(pShaderResources, node); }
inline void SetXmlFromDetailDecal(const SInputShaderResources& pShaderResources, XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetXmlFromDetailDecal(pShaderResources, node); }

//////////////////////////////////////////////////////////////////////////
inline void SetLightingFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetLightingFromXml(pShaderResources, node); }
inline void SetXmlFromLighting(const SInputShaderResources& pShaderResources, XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetXmlFromLighting(pShaderResources, node); }

//////////////////////////////////////////////////////////////////////////
inline void SetShaderParamsFromXml(SInputShaderResources& pShaderResources, const XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetShaderParamsFromXml(pShaderResources, node); }
inline void SetXmlFromShaderParams(const SInputShaderResources& pShaderResources, XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().SetXmlFromShaderParams(pShaderResources, node); }

//////////////////////////////////////////////////////////////////////////
inline void MigrateXmlLegacyData(SInputShaderResources& pShaderResources, const XmlNodeRef& node) { GetIEditorImpl()->Get3DEngine()->GetMaterialHelpers().MigrateXmlLegacyData(pShaderResources, node); }
}

