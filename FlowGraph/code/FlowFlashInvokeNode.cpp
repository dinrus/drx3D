// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowFlashNode.h
//  Version:     v1.00
//  Created:     23/5/2005 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/FlowGraph/StdAfx.h>

#include <drx3D/Sys/IFlashPlayer.h>
#include <drx3D/Sys/ILocalizationUpr.h>
#include <drx3D/FlowGraph/IFlowBaseNode.h>

////////////////////////////////////////////////////////////////////////////////////////
// Flow node for invoking a flash method of a DTS_I_FLASHPLAYER dyntexture stored in an entity's material
////////////////////////////////////////////////////////////////////////////////////////

// this maps languages to audio channels
static tukk g_languageMapping[] =
{
	"main",     // channel 0, music and sound effects
	"english",  // channel 1
	"french",   // channel 2
	"german",   // channel 3
	"italian",  // channel 4
	"russian",  // channel 5
	"polish",   // channel 6
	"spanish",  // channel 7
	"turkish",  // channel 8
	"japanese"  // channel 9
};

static const size_t g_languageMappingCount = (DRX_ARRAY_COUNT(g_languageMapping));

class CFlowFlashInvokeNode : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowFlashInvokeNode(SActivationInfo* pActInfo)
	{
	}

	enum EInputPorts
	{
		EIP_Slot = 0,
		EIP_SubMtlId,
		EIP_TexSlot,
		EIP_Invoke,
		EIP_Method,
		EIP_Param1,
		EIP_Param2,
		EIP_Param3,
		EIP_Param4,
	};

	enum EOutputPorts
	{
		EOP_Result = 0,
	};

	static i32k MAX_PARAMS = 4;
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<i32>("Slot",      0,                                     _HELP("Material Slot")),
			InputPortConfig<i32>("SubMtlId",  0,                                     _HELP("Sub Material Id, starting at 0"),_HELP("SubMaterialId")),
			InputPortConfig<i32>("TexSlot",   0,                                     _HELP("Texture Slot")),
			InputPortConfig_Void("Invoke",    _HELP("Trigger to invoked [Method]")),
			InputPortConfig<string>("Method", _HELP("Method to be invoked")),
			InputPortConfig<string>("Param1", _HELP("Param 1")),
			InputPortConfig<string>("Param2", _HELP("Param 2")),
			InputPortConfig<string>("Param3", _HELP("Param 3")),
			InputPortConfig<string>("Param4", _HELP("Param 4")),
			{ 0 }
		};

		static const SOutputPortConfig out_config[] = {
			OutputPortConfig_AnyType("Result", _HELP("Result when successfully called")),
			{ 0 }
		};

		config.sDescription = _HELP("Invoke Flash Function");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_ADVANCED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (event != eFE_Activate || !IsPortActive(pActInfo, EIP_Invoke))
			return;

		IEntity* pEntity = pActInfo->pEntity;
		if (pEntity == 0)
			return;

		IEntityRender* pIEntityRender = pEntity->GetRenderInterface();
		if (pIEntityRender == 0)
			return;

		i32k slot = GetPortInt(pActInfo, EIP_Slot);
		IMaterial* pMtl = pIEntityRender->GetRenderMaterial(slot);
		if (pMtl == 0)
		{
			GameWarning("[flow] CFlowFlashInvokeNode: Entity '%s' [%d] has no material at slot %d", pEntity->GetName(), pEntity->GetId(), slot);
			return;
		}

		i32k& subMtlId = GetPortInt(pActInfo, EIP_SubMtlId);
		pMtl = pMtl->GetSafeSubMtl(subMtlId);
		if (pMtl == 0)
		{
			GameWarning("[flow] CFlowFlashInvokeNode: Entity '%s' [%d] has no sub-material %d at slot %d", pEntity->GetName(), pEntity->GetId(), subMtlId, slot);
			return;
		}

		const string& methodName = GetPortString(pActInfo, EIP_Method);
		if (methodName.empty())
		{
			GameWarning("[flow] CFlowFlashInvokeNode: Invalid method name on calling Entity '%s'", pEntity->GetName());
			return;
		}

		i32k texSlot = GetPortInt(pActInfo, EIP_TexSlot);
		const SShaderItem& shaderItem(pMtl->GetShaderItem());
		if (shaderItem.m_pShaderResources)
		{
			SEfResTexture* pTex = shaderItem.m_pShaderResources->GetTexture(texSlot);
			if (pTex)
			{
				IDynTextureSource* pDynTexSrc = pTex->m_Sampler.m_pDynTexSource;
				if (pDynTexSrc)
				{
					IFlashPlayer* pFlashPlayer = (IFlashPlayer*) pDynTexSrc->GetSourceTemp(IDynTextureSource::DTS_I_FLASHPLAYER);
					if (pFlashPlayer)
					{
						SFlashVarValue args[MAX_PARAMS] = { false, false, false, false }; // yes, not very good, but faster then alloca
						i32 numArgs = 0;
						for (i32 i = EIP_Param1; i < EIP_Param1 + MAX_PARAMS; ++i)
						{
							const string& v = GetPortString(pActInfo, i);
							if (v.empty())
								break;
							PREFAST_ASSUME(numArgs > 0 && numArgs < MAX_PARAMS);
							args[numArgs++] = v.c_str();
						}
						{
							tukk method = methodName.c_str();
							if (method)
							{
								if (!stricmp(method, "vPlay"))
								{
									pDynTexSrc->EnablePerFrameRendering(true);
									SetAudioChannel(pFlashPlayer);
								}
								else if (!stricmp(method, "vStop"))
								{
									pDynTexSrc->EnablePerFrameRendering(false);
								}
							}
						}
						SFlashVarValue invokeRes(SFlashVarValue::CreateUndefined());
						if (pFlashPlayer->Invoke(methodName.c_str(), args, numArgs, &invokeRes))
						{
							switch (invokeRes.GetType())
							{
							case SFlashVarValue::eBool:
								ActivateOutput(pActInfo, EOP_Result, invokeRes.GetBool());
								break;
							case SFlashVarValue::eInt:
								ActivateOutput(pActInfo, EOP_Result, invokeRes.GetInt());
								break;
							case SFlashVarValue::eUInt:
								ActivateOutput(pActInfo, EOP_Result, (i32) invokeRes.GetUInt());
								break;
							case SFlashVarValue::eDouble:
								ActivateOutput(pActInfo, EOP_Result, (float) invokeRes.GetDouble());
								break;
							case SFlashVarValue::eFloat:
								ActivateOutput(pActInfo, EOP_Result, invokeRes.GetFloat());
								break;
							case SFlashVarValue::eConstStrPtr:
								{
									string t(invokeRes.GetConstStrPtr());
									ActivateOutput(pActInfo, EOP_Result, t);
								}
							case SFlashVarValue::eConstWstrPtr:
							case SFlashVarValue::eObject:
							case SFlashVarValue::eNull:
							case SFlashVarValue::eUndefined:
							default:
								ActivateOutput(pActInfo, EOP_Result, true); // at least sth
								break;
							}
						}
						else
						{
							GameWarning("[flow] CFlowFlashInvokeNode: Error while calling '%s' on Entity '%s' [%d]", methodName.c_str(), pEntity->GetName(), pEntity->GetId());
						}
					}
					else
					{
						GameWarning("[flow] CFlowFlashInvokeNode: Entity '%s' [%d] has no FlashDynTexture at sub-material %d at slot %d at texslot %d", pEntity->GetName(), pEntity->GetId(), subMtlId, slot, texSlot);
					}
				}
				else
				{
					GameWarning("[flow] CFlowFlashInvokeNode: Entity '%s' [%d] has no dyn-texture at sub-material %d at slot %d at texslot %d", pEntity->GetName(), pEntity->GetId(), subMtlId, slot, texSlot);
				}
			}
			else
			{
				GameWarning("[flow] CFlowFlashInvokeNode: Entity '%s' [%d] has no texture at sub-material %d at slot %d at texslot %d", pEntity->GetName(), pEntity->GetId(), subMtlId, slot, texSlot);
			}
		}
	}

	void SetAudioChannel(IFlashPlayer* pFlashPlayer)
	{
		i32 languageId = 1;
		tukk language = gEnv->pSystem->GetLocalizationUpr()->GetLanguage();
		if (language != NULL && language[0] != 0)
		{
			for (i32 i = 0; i < g_languageMappingCount; ++i)
			{
				if (stricmp(g_languageMapping[i], language) == 0)
				{
					languageId = i;
					break;
				}
			}
		}

		pFlashPlayer->Invoke1("setSubAudioChannel", languageId);
	}

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}
};

REGISTER_FLOW_NODE("Entity:FlashInvoke", CFlowFlashInvokeNode)
