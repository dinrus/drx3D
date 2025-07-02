// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <Mannequin/Serialization.h>
#include <drx3D/CoreX/Serialization/IArchive.h>

static i32k INVALID_LAYER_INDEX = -1;

struct SProceduralParamsCopyNormalizedTime
	: public IProceduralParams
{
	SProceduralParamsCopyNormalizedTime()
		: sourceLayer(0)
		, layer(0)
	{
	}

	virtual void Serialize(Serialization::IArchive& ar)
	{
		ar(sourceLayer, "SourceLayer", "Source Layer");
		ar(layer, "Layer", "Layer");
		ar(sourceScope, "SourceScope", "Source Scope");
	}

	float sourceLayer;
	float layer;
	SProcDataCRC sourceScope;
};

class CProceduralClipCopyNormalizedTime
	: public TProceduralClip< SProceduralParamsCopyNormalizedTime >
{
public:
	CProceduralClipCopyNormalizedTime()
		: m_sourceLayer( INVALID_LAYER_INDEX )
		, m_targetLayer( INVALID_LAYER_INDEX )
	{
	}

	virtual void OnEnter( float blendTime, float duration, const SProceduralParamsCopyNormalizedTime& params )
	{
		IF_UNLIKELY ( m_charInstance == NULL )
		{
			return;
		}

		ISkeletonAnim* pSkeletonAnim = m_charInstance->GetISkeletonAnim();
		IF_UNLIKELY ( pSkeletonAnim == NULL )
		{
			return;
		}

		{
			const IScope* pSourceScope = GetSourceScope( params.sourceScope );
			if ( pSourceScope == NULL )
			{
				tukk const entityName = m_entity ? m_entity->GetName() : "";
				DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CopyNormalizedTime procedural clip setup is not valid for entity '%s'. Not able to find source scope named '%s'.", entityName, params.sourceScope.c_str() );
				return;
			}

			i32k sourceScopeLayerCount = static_cast< i32 >( pSourceScope->GetTotalLayers() );
			i32k sourceScopeLayerBase = static_cast< i32 >( pSourceScope->GetBaseLayer() );
			i32k sourceLayerParam = static_cast< i32 >( params.sourceLayer );

			IF_UNLIKELY ( sourceLayerParam < 0 || sourceScopeLayerCount <= sourceLayerParam )
			{
				tukk const entityName = m_entity ? m_entity->GetName() : "";
				DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CopyNormalizedTime procedural clip setup is not valid for entity '%s'. Source layer is %d but should be between 0 and %d.", entityName, sourceLayerParam, sourceScopeLayerCount );
				return;
			}

			m_sourceLayer = sourceScopeLayerBase + sourceLayerParam;
		}
		
		{
			i32k targetScopeLayerCount = static_cast< i32 >( m_scope->GetTotalLayers() );
			i32k targetScopeLayerBase = static_cast< i32 >( m_scope->GetBaseLayer() );
			i32k targetLayerParam = static_cast< i32 >( params.layer );

			IF_UNLIKELY ( targetLayerParam < 0 || targetScopeLayerCount <= targetLayerParam )
			{
				tukk const entityName = m_entity ? m_entity->GetName() : "";
				DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CopyNormalizedTime procedural clip setup is not valid for entity '%s'. Layer is %d but should be between 0 and %d.", entityName, targetLayerParam, targetScopeLayerCount );
				return;
			}

			m_targetLayer = targetScopeLayerBase + targetLayerParam;
		}

		IF_UNLIKELY ( m_targetLayer == m_sourceLayer )
		{
			tukk const entityName = m_entity ? m_entity->GetName() : "";
			DrxWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CopyNormalizedTime procedural clip setup is not valid for entity '%s'. Target and source layers have the same value.", entityName );
			m_sourceLayer = INVALID_LAYER_INDEX;
			m_targetLayer = INVALID_LAYER_INDEX;
			return;
		}
	}

	virtual void OnExit( float blendTime )
	{
		const bool isSetupValid = IsSetupValid();
		IF_UNLIKELY ( ! isSetupValid )
		{
			return;
		}

		DRX_ASSERT( m_charInstance );
		ISkeletonAnim* pSkeletonAnim = m_charInstance->GetISkeletonAnim();
		DRX_ASSERT( pSkeletonAnim );

		i32k targetAnimationsCount = pSkeletonAnim->GetNumAnimsInFIFO( m_targetLayer );
		for ( i32 i = 0; i < targetAnimationsCount; ++i )
		{
			CAnimation& targetAnimation = pSkeletonAnim->GetAnimFromFIFO( m_targetLayer, i );
			targetAnimation.ClearStaticFlag( CA_MANUAL_UPDATE );
		}
	}

	virtual void Update( float timePassed )
	{
		const bool isSetupValid = IsSetupValid();
		IF_UNLIKELY ( ! isSetupValid )
		{
			return;
		}

		DRX_ASSERT( m_charInstance );
		ISkeletonAnim* pSkeletonAnim = m_charInstance->GetISkeletonAnim();
		DRX_ASSERT( pSkeletonAnim );

		i32k sourceAnimationsCount = pSkeletonAnim->GetNumAnimsInFIFO( m_sourceLayer );
		IF_UNLIKELY ( sourceAnimationsCount <= 0 )
		{
			return;
		}

		const CAnimation& sourceAnimation = pSkeletonAnim->GetAnimFromFIFO( m_sourceLayer, sourceAnimationsCount - 1 );
		const float sourceNormalizedTime = sourceAnimation.GetCurrentSegmentNormalizedTime();

		i32k targetAnimationsCount = pSkeletonAnim->GetNumAnimsInFIFO( m_targetLayer );
		for ( i32 i = 0; i < targetAnimationsCount; ++i )
		{
			CAnimation& targetAnimation = pSkeletonAnim->GetAnimFromFIFO( m_targetLayer, i );
			targetAnimation.SetStaticFlag( CA_MANUAL_UPDATE );
			targetAnimation.SetCurrentSegmentNormalizedTime( sourceNormalizedTime );
		}
	}

private:

	bool IsSetupValid() const
	{
		const bool isSetupValid = ( m_sourceLayer != INVALID_LAYER_INDEX && m_targetLayer != INVALID_LAYER_INDEX );
		return isSetupValid;
	}


	const IScope* GetSourceScope( const SProcDataCRC& sourceScopeCrc ) const
	{
		if ( sourceScopeCrc.IsEmpty() )
		{
			return m_scope;
		}

		const IActionController& actionController = m_scope->GetActionController();
		const TagID sourceScopeTagId = actionController.GetContext().controllerDef.m_scopeIDs.Find( sourceScopeCrc.crc );
		IF_UNLIKELY ( sourceScopeTagId == SCOPE_ID_INVALID )
		{
			return NULL;
		}

		const IScope* pSourceScope = actionController.GetScope( sourceScopeTagId );
		DRX_ASSERT( pSourceScope );

		return pSourceScope;
	}

private:
	i32 m_sourceLayer;
	i32 m_targetLayer;
};

REGISTER_PROCEDURAL_CLIP(CProceduralClipCopyNormalizedTime, "CopyNormalizedTime");