//  Имя файла:
//  Version:
//  Created:
//  Компиляторы:       drux (DinrusPro's Universal Compiler),
//  Описание: Обработка отсутствующих элементов интерфейса I3DEngine.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/Eng3D/ParticleEmitter.h>
#include <drx3D/Eng3D/ParticleRender.h>
#include <drx3D/Eng3D/ParticleEffect.h>

namespace pfx2
{
	 uint GetVersion(Serialization::IArchive& ar){
     //if(ar.GetVersion() != NULL) return ar.GetVersion();
    // else => Версия у yasli::Archive отсутствует, поэтому
     return 0;
   }
 
 /*
 Определение частицы должно быть связано с перечнем:
     
 //! Определение конфигурации, зависимое от выбранной пользователем спецификации машины.
enum ESystemConfigSpec
{
	CONFIG_CUSTOM        = 0, //!< Should always be first.
	CONFIG_LOW_SPEC      = 1,
	CONFIG_MEDIUM_SPEC   = 2,
	CONFIG_HIGH_SPEC     = 3,
	CONFIG_VERYHIGH_SPEC = 4,

	CONFIG_DURANGO       = 5,
	CONFIG_ORBIS         = 6,

	//! Specialized detail config setting.
	CONFIG_DETAIL_SPEC = 7,

	END_CONFIG_SPEC_ENUM, //!< Must be the last value, used for error checking.
};

Находится в заголовочнике <drx3D/Sys/ISystem.h>.
Сама функция используется virtual bool CanMakeRuntime(CParticleEmitter* pEmitter) const
override,
который находится в class CFeatureComponentEnableByConfig : public CParticleFeature.
Замысел пока непонятен, поэтому пока будем возвращать 0, т.е. CONFIG_CUSTOM.
 */
 /////
   uint      CParticleEmitter::GetParticleSpec() const{ return 0;}
   void      CParticleEmitter::UpdateEmitGeomFromEntity(){}
   QuatTS    CParticleEmitter::GetEmitterGeometryLocation() const{};//return ;}//QuatTS(0.f, 0.f, 0.f);}
   void      CParticleEmitter::SyncUpdateParticles(){}
    bool      CParticleEmitter::UpdateParticles(){}

/////
  void CParticleRenderBase::AddToComponent(pfx2::CParticleComponent*, pfx2::SComponentParams*){}
  EFeatureType CParticleRenderBase::GetFeatureType(){return EFeatureType::EFT_Generic ; }
  float CParticleRenderBase::CullArea(float area,
                 float areaLimit, TParticleIdArray& ids, TVarArray<float> alphas,
                                                             TConstArray<float> areas){ return 0; }
                                                             /////
	//string                    CParticleEffect::GetShortName() const;
	i32                       CParticleEffect::GetEditVersion() const{ return 0; }
	CParticleComponent*       CParticleEffect::FindComponentByName(tukk name) const{return nullptr;}
	string                    CParticleEffect::MakeUniqueName(const CParticleComponent* forComponent, tukk name){return "";}
 /*
Также отсутствуют:
	
CParticleContainer::CParticleContainer()
CParticleContainer::~CParticleContainer()
CParticleContainer::ResetUsedData()
CParticleContainer::AddParticleData(DynamicEnum<pfx2::SDataInfo, u32, pfx2::SDataInfo>)
CParticleContainer::Trim()
CParticleContainer::ResetSpawnedParticles()
CParticleContainer::RemoveParticles(Array<u32, u32, NArray::Storage>, Array<u32, u32, NArray::Storage>)
CParticleContainer::AddParticles(Array<pfx2::SSpawnEntry const, u32, NArray::Storage>)
ParticleList<_smart_ptr<IStatObj>>::m_pHead
ParticleList<_smart_ptr<IStatObj>>::m_pTail
ParticleList<EmitParticleData>::m_pHead
ParticleList<EmitParticleData>::m_pTail
ParticleList<CParticleSubEmitter>::m_pHead
*/

   
}//end pfx2