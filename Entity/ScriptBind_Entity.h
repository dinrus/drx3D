// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Script/IScriptSystem.h>

struct IEntity;
struct ISystem;

#include <drx3D/Phys/IPhysics.h>
#include <drx3D/CoreX/ParticleSys/ParticleParams.h>

#define ENTITYPROP_CASTSHADOWS   0x00000001
#define ENTITYPROP_DONOTCHECKVIS 0x00000002

struct GoalParams;

/*!
 *	<description>In this class are all entity-related script-functions implemented in order to expose all functionalities provided by an entity.</description>
 *	<remarks>These function will never be called from C-Code. They're script-exclusive.</remarks>
 */
class CScriptBind_Entity : public CScriptableBase
{
public:
	CScriptBind_Entity(IScriptSystem* pSS, ISystem* pSystem);

	void         DelegateCalls(IScriptTable* pInstanceTable);

	void         GoalParamsHelper(IFunctionHandler* pH, u32 index, GoalParams& node);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddContainer(m_areaPoints);

	}
protected:
	//! <code>Entity.DeleteThis()</code>
	//! <description>Deletes this entity.</description>
	i32 DeleteThis(IFunctionHandler* pH);

	//! <code>Entity.CreateRenderProxy()</code>
	//! <description>Create a proxy render object for the entity, allows entity to be rendered without loading any assets in immediately.</description>
	i32 CreateRenderProxy(IFunctionHandler* pH);

	//! <code>Entity.UpdateShaderParamCallback()</code>
	//! <description>Check all the currently set shader param callbacks on the renderproxy with the current state of the entity.</description>
	i32 CheckShaderParamCallbacks(IFunctionHandler* pH);

	//! <code>Entity.LoadObject( nSlot, sFilename )</code>
	//! <description>Load CGF geometry into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="sFilename">CGF geometry file name.</param>
	i32 LoadObject(IFunctionHandler* pH, i32 nSlot, tukk sFilename);

	//! <code>Entity.LoadObject( nSlot, sFilename, nFlags )</code>
	//! <description>Load CGF geometry into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="sFilename">CGF geometry file name.</param>
	//!		<param name="nFlags">entity load flags</param>
	i32 LoadObjectWithFlags(IFunctionHandler* pH, i32 nSlot, tukk sFilename, i32k nFlags);

	//! <code>Entity.LoadObjectLattice( nSlot )</code>
	//! <description>Load lattice into the entity slot.</description>
	//    <param name="nSlot">Slot identifier.</param>
	i32 LoadObjectLattice(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.LoadSubObject( nSlot, sFilename, sGeomName )</code>
	//! <description>Load geometry of one CGF node into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="sFilename">CGF geometry file name.</param>
	//!		<param name="sGeomName">Name of the node inside CGF geometry.</param>
	i32 LoadSubObject(IFunctionHandler* pH, i32 nSlot, tukk sFilename, tukk sGeomName);

	//! <code>Entity.LoadCharacter( nSlot, sFilename )</code>
	//! <description>Load CGF geometry into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="sFilename">CGF geometry file name.</param>
	i32 LoadCharacter(IFunctionHandler* pH, i32 nSlot, tukk sFilename);

	//! <code>Entity.LoadGeomCache( i32 nSlot,tukk sFilename )</code>
	//! <description>Load geom cache into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="sFilename">CAX file name.</param>
	i32 LoadGeomCache(IFunctionHandler* pH, i32 nSlot, tukk sFilename);

	//! <code>Entity.LoadLight( nSlot, lightTable )</code>
	//! <description>Load CGF geometry into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="lightTable">Table with all the light information.</param>
	i32 LoadLight(IFunctionHandler* pH, i32 nSlot, SmartScriptTable table);

	//! <code>Entity.SetLightColorParams( nSlot, color, specular_multiplier)</code>
	//! <description>changes the color related params of an existing light.</description>
	i32 SetLightColorParams(IFunctionHandler* pH, i32 nSlot, Vec3 color, float specular_multiplier);

	//! <code>Entity.UpdateLightClipBounds( nSlot )</code>
	//! <description>Update the clip bounds of the light from the linked entities.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 UpdateLightClipBounds(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.SetLightCasterException( nLightSlot, nGeometrySlot )</code>
	//! <description>Entity render node will be a caster exception for light loaded in nLightSlot.
	//!		<param name="nLightSlot">Slot where our light is loaded.</param>
	i32 SetSelfAsLightCasterException(IFunctionHandler* pH, i32 nLightSlot);

	//! <code>Entity.LoadCloudBlocker( nSlot, table )</code>
	//! <description>Loads the properties of cloud blocker into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="table">Table with cloud blocker properties.</param>
	i32 LoadCloudBlocker(IFunctionHandler* pH, i32 nSlot, SmartScriptTable table);

	//! <code>Entity.LoadFogVolume( nSlot, table )</code>
	//! <description>Loads the fog volume XML file into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="table">Table with fog volume properties.</param>
	i32 LoadFogVolume(IFunctionHandler* pH, i32 nSlot, SmartScriptTable table);

	//! <code>Entity.FadeGlobalDensity( nSlot, fadeTime, newGlobalDensity )</code>
	//! <description>Sets the fade global density.</description>
	//!		<param name="nSlot">nSlot identifier.</param></param>
	//!		<param name="fadeTime">.</param>
	//!		<param name="newGlobalDensity">.</param>
	i32 FadeGlobalDensity(IFunctionHandler* pH, i32 nSlot, float fadeTime, float newGlobalDensity);

	//! <code>Entity.LoadParticleEffect( nSlot, sEffectName, fPulsePeriod, bPrime, fScale )</code>
	//! <description>Loads CGF geometry into the entity slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="sEffectName">Name of the particle effect (Ex: "explosions/rocket").</param>
	//!		<param name="(optional) bPrime">Whether effect starts fully primed to equilibrium state.</param>
	//!		<param name="(optional) fPulsePeriod">Time period between particle effect restarts.</param>
	//!		<param name="(optional) fScale">Size scale to apply to particles</param>
	//!		<param name="(optional) fCountScale">Count multiplier to apply to particles</param>
	//!		<param name="(optional) bScalePerUnit">Scale size by attachment extent</param>
	//!		<param name="(optional) bCountPerUnit">Scale count by attachment extent</param>
	//!		<param name="(optional) sAttachType">string for EGeomType</param>
	//!		<param name="(optional) sAttachForm">string for EGeomForm</param>
	i32 LoadParticleEffect(IFunctionHandler* pH, i32 nSlot, tukk sEffectName, SmartScriptTable table);

	//! <code>Entity.PreLoadParticleEffect( sEffectName )</code>
	//! <description>Pre-loads a particle effect.</description>
	//!		<param name="sEffectName">Name of the particle effect (Ex: "explosions/rocket").</param>
	i32 PreLoadParticleEffect(IFunctionHandler* pH, tukk sEffectName);

	//! <code>Entity.IsSlotParticleEmitter( slot )</code>
	//! <description>Checks if the slot is a particle emitter.</description>
	//!		<param name="slot">Slot identifier.</param>
	i32 IsSlotParticleEmitter(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.IsSlotLight( slot )</code>
	//! <description>Checks if the slot is a light.</description>
	//!		<param name="slot">Slot identifier.</param>
	i32 IsSlotLight(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.IsSlotGeometry( slot )</code>
	//! <description>Checks if the slot is a geometry.</description>
	//!		<param name="slot">Slot identifier.</param>
	i32 IsSlotGeometry(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.IsSlotCharacter( slot )</code>
	//! <description>Checks if the slot is a character.</description>
	//!		<param name="slot">Slot identifier.</param>
	i32 IsSlotCharacter(IFunctionHandler* pH, i32 slot);

	//////////////////////////////////////////////////////////////////////////
	// Slots.
	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.GetSlotCount()</code>
	//! <description>Gets the count of the slots.</description>
	i32 GetSlotCount(IFunctionHandler* pH);

	//! <code>Entity.GetSlotPos( nSlot )</code>
	//! <description>Gets the slot position.</description>
	//!		<param name="nSlot">nSlot identifier.</param>
	i32 GetSlotPos(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.SetSlotPos( nSlot, v )</code>
	//! <description>Sets the slot position.</description>
	//!		<param name="nSlot">nSlot identifier.</param>
	//!		<param name="v">Position to be set.</param>
	i32 SetSlotPos(IFunctionHandler* pH, i32 slot, Vec3 v);

	//! <code>Entity.SetSlotPosAndDir( nSlot, pos, dir )</code>
	//! <description>Sets the slot position and direction.</description>
	//!		<param name="nSlot">nSlot identifier.</param>
	//!		<param name="pos">Position to be set.</param>
	//!		<param name="dir">Direction to be set.</param>
	i32 SetSlotPosAndDir(IFunctionHandler* pH, i32 nSlot, Vec3 pos, Vec3 dir);

	//! <code>Entity.GetSlotAngles( nSlot )</code>
	//! <description>Gets the slot angles.</description>
	//!		<param name="nSlot">nSlot identifier.</param>
	i32 GetSlotAngles(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.GetSlotAngles( nSlot, v )</code>
	//! <description>Sets the slot angles.</description>
	//!		<param name="nSlot">nSlot identifier.</param>
	//!		<param name="v">Angle to be set.</param>
	i32 SetSlotAngles(IFunctionHandler* pH, i32 nSlot, Ang3 v);

	//! <code>Entity.GetSlotScale( nSlot )</code>
	//! <description>Gets the slot scale amount.</description>
	//!		<param name="nSlot">nSlot identifier.</param>
	i32 GetSlotScale(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.SetSlotScale( nSlot, fScale )</code>
	//! <description>Sets the slot scale amount.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="fScale">Scale amount for the slot.</param>
	i32 SetSlotScale(IFunctionHandler* pH, i32 nSlot, float fScale);

	//! <code>Entity.IsSlotValid( nSlot )</code>
	//! <description>Checks if the slot is valid.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 IsSlotValid(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.CopySlotTM( destSlot, srcSlot, includeTranslation )</code>
	//! <description>Copies the TM (Transformation Matrix) of the slot.</description>
	//!		<param name="destSlot">Destination slot identifier.</param>
	//!		<param name="srcSlot">Source slot identifier.</param>
	//!		<param name="includeTranslation">True to include the translation, false otherwise.</param>
	i32 CopySlotTM(IFunctionHandler* pH, i32 destSlot, i32 srcSlot, bool includeTranslation);

	//! <code>Entity.MultiplyWithSlotTM( slot, pos )</code>
	//! <description>Multiplies with the TM (Transformation Matrix) of the slot.</description>
	//!		<param name="slot">Slot identifier.</param>
	//!		<param name="pos">Position vector.</param>
	i32 MultiplyWithSlotTM(IFunctionHandler* pH, i32 slot, Vec3 pos);

	//! <code>Entity.SetSlotWorldTM( nSlot, pos, dir )</code>
	//! <description>Sets the World TM (Transformation Matrix) of the slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="pos">Position vector.</param>
	//!		<param name="dir">Direction vector.</param>
	i32 SetSlotWorldTM(IFunctionHandler* pH, i32 nSlot, Vec3 pos, Vec3 dir);

	//! <code>Entity.GetSlotWorldPos( nSlot )</code>
	//! <description>Gets the World position of the slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 GetSlotWorldPos(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.GetSlotWorldDir( nSlot )</code>
	//! <description>Gets the World direction of the slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 GetSlotWorldDir(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.SetSlotHud3D( nSlot )</code>
	//! <description>Setup flags for use as 3D HUD entity.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 SetSlotHud3D(IFunctionHandler* pH, i32 nSlot);

	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.SetPos( vPos )</code>
	//! <description>Sets the position of the entity.</description>
	//!		<param name="vPos">Position vector.</param>
	i32 SetPos(IFunctionHandler* pH, Vec3 vPos);

	//! <code>Entity.GetPos()</code>
	//! <description>Gets the position of the entity.</description>
	i32 GetPos(IFunctionHandler* pH);

	//! <code>Entity.SetAngles( vAngles )</code>
	//!		<param name="vAngles">Angle vector.</param>
	//! <description>Sets the angle of the entity.</description>
	i32 SetAngles(IFunctionHandler* pH, Ang3 vAngles);

	//! <code>Entity.GetAngles()</code>
	//! <description>Gets the angle of the entity.</description>
	i32 GetAngles(IFunctionHandler* pH);

	//! <code>Entity.SetScale( fScale )</code>
	//!		<param name="fScale">Scale amount.</param>
	//! <description>Sets the scaling value for the entity.</description>
	i32 SetScale(IFunctionHandler* pH, float fScale);

	//! <code>Entity.GetScale()</code>
	//! <description>Gets the scaling value for the entity.</description>
	i32 GetScale(IFunctionHandler* pH);

	//! <code>Entity.GetCenterOfMassPos()</code>
	//! <description>Gets the position of the entity center of mass.</description>
	i32 GetCenterOfMassPos(IFunctionHandler* pH);

	//! <code>Entity.GetWorldBoundsCenter()</code>
	//! <description>Gets the world bbox center for the entity (defaults to entity position if no bbox present).</description>
	i32 GetWorldBoundsCenter(IFunctionHandler* pH);

	//////////////////////////////////////////////////////////////////////////
	//! <code>Entity.SetLocalPos( vPos )</code>
	i32 SetLocalPos(IFunctionHandler* pH, Vec3 vPos);

	//! <code>Vec3 Entity.GetLocalPos()</code>
	i32 GetLocalPos(IFunctionHandler* pH);

	//! <code>Entity.SetLocalAngles( vAngles )</code>
	i32 SetLocalAngles(IFunctionHandler* pH, Ang3 vAngles);

	//! <code>Vec3 Entity.GetLocalAngles( vAngles )</code>
	i32 GetLocalAngles(IFunctionHandler* pH);

	//! <code>Entity.SetLocalScale( fScale )</code>
	i32 SetLocalScale(IFunctionHandler* pH, float fScale);

	//! <code>float Entity.GetLocalScale()</code>
	i32 GetLocalScale(IFunctionHandler* pH);

	//////////////////////////////////////////////////////////////////////////
	//! <code>Entity.SetWorldPos( vPos )</code>
	i32 SetWorldPos(IFunctionHandler* pH, Vec3 vPos);

	//! <code>Vec3 Entity.GetWorldPos()</code>
	i32 GetWorldPos(IFunctionHandler* pH);

	//! <code>Vec3 Entity.GetWorldDir()</code>
	i32 GetWorldDir(IFunctionHandler* pH);

	//! <code>Entity.SetWorldAngles( vAngles )</code>
	i32 SetWorldAngles(IFunctionHandler* pH, Ang3 vAngles);

	//! <code>Vec3 Entity.GetWorldAngles( vAngles )</code>
	i32 GetWorldAngles(IFunctionHandler* pH);

	//! <code>Entity.SetWorldScale( fScale )</code>
	i32 SetWorldScale(IFunctionHandler* pH, float fScale);

	//! <code>float Entity.GetWorldScale()</code>
	i32 GetWorldScale(IFunctionHandler* pH);

	//! <code>float Entity.GetBoneLocal( boneName, trgDir )</code>
	i32 GetBoneLocal(IFunctionHandler* pH, tukk boneName, Vec3 trgDir);

	//! <code>Ang3 Entity.CalcWorldAnglesFromRelativeDir( dir )</code>
	i32 CalcWorldAnglesFromRelativeDir(IFunctionHandler* pH, Vec3 dir);

	//! <code>float Entity.IsEntityInside(entityId)</code>
	i32 IsEntityInside(IFunctionHandler* pH, ScriptHandle entityId);

	//////////////////////////////////////////////////////////////////////////

	//! <code>float Entity.GetDistance( entityId )</code>
	//! <returns>The distance from  entity specified with entityId/</returns>
	i32 GetDistance(IFunctionHandler* pH);

	//////////////////////////////////////////////////////////////////////////
	//! <code>Entity.DrawSlot( nSlot )</code>
	//! <description>Enables/Disables drawing of object or character at specified slot of the entity.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	//!		<param name="nEnable">1-Enable drawing, 0-Disable drawing.</param>
	i32 DrawSlot(IFunctionHandler* pH, i32 nSlot, i32 nEnable);

	//////////////////////////////////////////////////////////////////////////
	//! <code>Entity.IgnorePhysicsUpdatesOnSlot( nSlot )</code>
	//! <description>Ignore physics when it try to update the position of a slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 IgnorePhysicsUpdatesOnSlot(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.FreeSlot( nSlot )</code>
	//! <description>Delete all objects from specified slot.</description>
	//!		<param name="nSlot">Slot identifier.</param>
	i32 FreeSlot(IFunctionHandler* pH, i32 nSlot);

	//! <code>Entity.FreeAllSlots()</code>
	//! <description>Delete all objects on every slot part of the entity.</description>
	i32 FreeAllSlots(IFunctionHandler* pH);

	//! <code>Entity.GetCharacter( nSlot )</code>
	//! <description>Gets the character for the specified slot if there is any.</description>
	i32 GetCharacter(IFunctionHandler* pH, i32 nSlot);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Physics.
	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.DestroyPhysics()</code>
	i32 DestroyPhysics(IFunctionHandler* pH);

	//! <code>Entity.EnablePhysics( bEnable )</code>
	i32 EnablePhysics(IFunctionHandler* pH, bool bEnable);

	//! <code>Entity.ResetPhysics()</code>
	i32 ResetPhysics(IFunctionHandler* pH);

	//! <code>Entity.AwakePhysics( nAwake )</code>
	i32 AwakePhysics(IFunctionHandler* pH, i32 nAwake);

	//! <code>Entity.AwakeCharacterPhysics( nSlot, sRootBoneName, nAwake )</code>
	i32 AwakeCharacterPhysics(IFunctionHandler* pH, i32 nSlot, tukk sRootBoneName, i32 nAwake);

	//! <code>Entity.Physicalize( i32 nSlot,i32 nPhysicsType,table physicsParams )</code>
	//! <description>
	//!    Create physical entity from the specified entity slot.
	//!      <para>
	//!          Physics Type        Meaning
	//!          -------------       -----------
	//!          PE_NONE             No physics.
	//!          PE_STATIC           Static physical entity.
	//!          PE_LIVING           Live physical entity (Players,Monsters).
	//!          PE_RIGID            Rigid body physical entity.
	//!          PE_WHEELEDVEHICLE   Physical vechicle with wheels.
	//!          PE_PARTICLE         Particle physical entity, it only have mass and radius.
	//!          PE_ARTICULATED      Ragdolls or other articulated physical enttiies.
	//!          PE_ROPE             Physical representation of the rope.
	//!          PE_SOFT             Soft body physics, cloth simulation.
	//!          PE_AREA             Physical Area (Sphere,Box,Geometry or Shape).
	//!       </para>
	//!
	//!      <para>
	//!          Params table keys   Meaning
	//!          -----------------   -----------
	//!          mass                Object mass, only used if density is not specified or -1.
	//!          density             Object density, only used if mass is not specified or -1.
	//!          flags               Physical entity flags.
	//!          partid              Index of the articulated body part, that this new physical entity will be attached to.
	//!          stiffness_scale     Scale of character joints stiffness (Multiplied with stiffness values specified from exported model)
	//!          Particle            This table must be set when Physics Type is PE_PARTICLE.
	//!          Living              This table must be set when Physics Type is PE_LIVING.
	//!          Area                This table must be set when Physics Type is PE_AREA.
	//!       </para>
	//!
	//!      <para>
	//!          Particle table      Meaning
	//!          -----------------   -----------
	//!          mass                Particle mass.
	//!          radius              Particle pseudo radius.
	//!          thickness           Thickness when lying on a surface (if 0, radius will be used).
	//!          velocity            Velocity direction and magnitude vector.
	//!          air_resistance      Air resistance coefficient, F = kv.
	//!          water_resistance    Water resistance coefficient, F = kv.
	//!          gravity             Gravity force vector to the air.
	//!          water_gravity       Gravity force vector when in the water.
	//!          min_bounce_vel      Minimal velocity at which particle bounces of the surface.
	//!          accel_thrust        Acceleration along direction of movement.
	//!          accel_lift          Acceleration that lifts particle with the current speed.
	//!          constant_orientation (0,1) Keep constance orientation.
	//!          single_contact      (0,1) Calculate only one first contact.
	//!          no_roll             (0,1) Do not roll particle on terrain.
	//!          no_spin             (0,1) Do not spin particle in air.
	//!          no_path_alignment   (0,1) Do not align particle orientation to the movement path.
	//!       </para>
	//!      <para>
	//!          Living table        Meaning
	//!          -----------------   -----------
	//!          height              Vertical offset of collision geometry center.
	//!          size                Collision cylinder dimensions vector (x,y,z).
	//!          height_eye          Vertical offset of the camera.
	//!          height_pivot        Offset from central ground position that is considered entity center.
	//!          head_radius         Radius of the head.
	//!          height_head         Vertical offset of the head.
	//!          inertia             Inertia coefficient, the more it is, the less inertia is; 0 means no inertia
	//!          air_resistance      Air control coefficient 0..1, 1 - special value (total control of movement)
	//!          gravity             Vertical gravity magnitude.
	//!          mass                Mass of the player (in kg).
	//!          min_slide_angle     If surface slope is more than this angle, player starts sliding (In radians)
	//!          max_climb_angle     Player cannot climb surface which slope is steeper than this angle (In radians)
	//!          max_jump_angle      Player is not allowed to jump towards ground if this angle is exceeded (In radians)
	//!          min_fall_angle      Player starts falling when slope is steeper than this (In radians)
	//!          max_vel_ground      Player cannot stand on surfaces that are moving faster than this (In radians)
	//!       </para>
	//!
	//!      <para>
	//!          Area table keys     Meaning
	//!          -----------------   -----------
	//!          type                Type of the area, valid values are: AREA_SPHERE,AREA_BOX,AREA_GEOMETRY,AREA_SHAPE
	//!          radius              Radius of the area sphere, must be specified if type is AREA_SPHERE.
	//!          box_min             Min vector of bounding box, must be specified if type is AREA_BOX.
	//!          box_max             Max vector of bounding box, must be specified if type is AREA_BOX..
	//!          points              Table, indexed collection of vectors in local entity space defining 2D shape of the area, (AREA_SHAPE)
	//!          height              Height of the 2D area (AREA_SHAPE), relative to the minimal Z in the points table.
	//!          uniform             Same direction in every point or always point to the center.
	//!          falloff             ellipsoidal falloff dimensions; 0,0,0 if no falloff.
	//!          gravity             Gravity vector inside the physical area.
	//!       </para>
	//!</description>
	//!		<param name="nSlot - Slot identifier, if -1 use geometries from all slots.</param>
	//!		<param name="nPhysicsType - Type of physical entity to create.</param>
	//!		<param name="physicsParams - Table with physicalization parameters.</param>
	i32 Physicalize(IFunctionHandler* pH, i32 nSlot, i32 nPhysicsType, SmartScriptTable physicsParams);

	//! <code>Entity.SetPhysicParams()</code>
	i32 SetPhysicParams(IFunctionHandler* pH);

	//! <code>Entity.SetCharacterPhysicParams()</code>
	i32 SetCharacterPhysicParams(IFunctionHandler* pH);

	//! <code>Entity.ActivatePlayerPhysics( bEnable )</code>
	i32 ActivatePlayerPhysics(IFunctionHandler* pH, bool bEnable);

	//! <code>Entity.ReattachSoftEntityVtx( partId )</code>
	i32 ReattachSoftEntityVtx(IFunctionHandler* pH, ScriptHandle entityId, i32 partId);

	//! <code>Entity.PhysicalizeSlot( slot, physicsParams )</code>
	i32 PhysicalizeSlot(IFunctionHandler* pH, i32 slot, SmartScriptTable physicsParams);

	//! <code>Entity.UpdateSlotPhysics( slot )</code>
	i32 UpdateSlotPhysics(IFunctionHandler* pH, i32 slot);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.SetColliderMode( mode )</code>
	i32 SetColliderMode(IFunctionHandler* pH, i32 mode);

	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.SelectPipe()</code>
	i32 SelectPipe(IFunctionHandler* pH);

	//! <code>Entity.IsUsingPipe( pipename )</code>
	//! <description>Returns true if entity is running the given goalpipe or has it inserted.</description>
	//!		<param name="pipename - goalpipe name</param>
	//! <returns>
	//!   true - if entity is running the given goalpipe or has it inserted
	//!		false - otherwise
	//! </returns>
	i32 IsUsingPipe(IFunctionHandler* pH);

	//! <code>Entity.Activate( bActivate )</code>
	//! <description>
	//!		Activates or deactivates entity.
	//!		This calls ignores update policy and forces entity to activate or deactivate
	//!		All active entities will be updated every frame, having too many active entities can affect performance.
	//! </description>
	//!		<param name="bActivate - if true entity will become active, is false will deactivate and stop being updated every frame.</param>
	i32 Activate(IFunctionHandler* pH, i32 bActive);

	//! <code>Entity.IsActive( bActivate )</code>
	//! <description>Retrieve active status of entity.</description>
	//!	<returns>
	//!		true - Entity is active.
	//!		false - Entity is not active.
	//!	</returns>
	i32 IsActive(IFunctionHandler* pH);

	//! <code>Entity.SetUpdatePolicy( nUpdatePolicy )</code>
	//! <description>
	//!    Changes update policy for the entity.
	//!    Update policy controls when entity becomes active or inactive (ex. when visible, when in close proximity, etc...).
	//!    All active entities will be updated every frame, having too many active entities can affect performance.
	//!      <para>
	//!          Update Policy                    Meaning
	//!          -------------                    -----------
	//!          ENTITY_UPDATE_NEVER              Never update this entity.
	//!          ENTITY_UPDATE_IN_RANGE           Activate entity when in specified radius.
	//!          ENTITY_UPDATE_POT_VISIBLE        Activate entity when potentially visible.
	//!          ENTITY_UPDATE_VISIBLE            Activate entity when visible in frustum.
	//!          ENTITY_UPDATE_PHYSICS            Activate entity when physics awakes, deactivate when physics go to sleep.
	//!          ENTITY_UPDATE_PHYSICS_VISIBLE    Same as ENTITY_UPDATE_PHYSICS, but also activates when visible.
	//!          ENTITY_UPDATE_ALWAYS             Entity is always active and updated every frame.
	//!       </para>
	//! </description>
	//!		<param name="nUpdatePolicy">Update policy type.</param>
	//! <remarks>Use SetUpdateRadius for update policy that require a radius.</remarks>
	i32 SetUpdatePolicy(IFunctionHandler* pH, i32 nUpdatePolicy);

	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.SetLocalBBox( vMin, vMax )</code>
	i32 SetLocalBBox(IFunctionHandler* pH, Vec3 vMin, Vec3 vMax);

	//! <code>Entity.GetLocalBBox()</code>
	i32 GetLocalBBox(IFunctionHandler* pH);

	//! <code>Entity.GetWorldBBox()</code>
	i32 GetWorldBBox(IFunctionHandler* pH);

	//! <code>Entity.GetProjectedWorldBBox()</code>
	i32 GetProjectedWorldBBox(IFunctionHandler* pH);

	//! <code>Entity.SetTriggerBBox( vMin, vMax )</code>
	i32 SetTriggerBBox(IFunctionHandler* pH, Vec3 vMin, Vec3 vMax);

	//! <code>Entity.GetTriggerBBox()</code>
	i32 GetTriggerBBox(IFunctionHandler* pH);

	//! <code>Entity.InvalidateTrigger()</code>
	i32 InvalidateTrigger(IFunctionHandler* pH);

	//! <code>Entity.ForwardTriggerEventsTo( entityId )</code>
	i32 ForwardTriggerEventsTo(IFunctionHandler* pH, ScriptHandle entityId);

	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.SetUpdateRadius()</code>
	i32 SetUpdateRadius(IFunctionHandler* pH);

	//! <code>Entity.GetUpdateRadius()</code>
	i32 GetUpdateRadius(IFunctionHandler* pH);

	//! <code>Entity.TriggerEvent()</code>
	i32 TriggerEvent(IFunctionHandler* pH);

	//! <code>Entity.GetHelperPos()</code>
	i32 GetHelperPos(IFunctionHandler* pH);

	//! <code>Entity.GetHelperDir()</code>
	i32 GetHelperDir(IFunctionHandler* pH);

	//! <code>Entity.GetHelperAngles()</code>
	i32 GetHelperAngles(IFunctionHandler* pH);

	//! <code>Entity.GetSlotHelperPos( slot, helperName, objectSpace )</code>
	i32 GetSlotHelperPos(IFunctionHandler* pH, i32 slot, tukk helperName, bool objectSpace);

	//! <code>Entity.GetBonePos()</code>
	i32 GetBonePos(IFunctionHandler* pH);

	//! <code>Entity.GetBoneDir()</code>
	i32 GetBoneDir(IFunctionHandler* pH);

	//! <code>Entity.GetBoneVelocity( characterSlot, boneName )</code>
	i32 GetBoneVelocity(IFunctionHandler* pH, i32 characterSlot, tukk boneName);

	//! <code>Entity.GetBoneAngularVelocity( characterSlot, oneName )</code>
	i32 GetBoneAngularVelocity(IFunctionHandler* pH, i32 characterSlot, tukk boneName);

	//! <code>Entity.GetBoneNameFromTable()</code>
	i32 GetBoneNameFromTable(IFunctionHandler* pH);

	//! <code>Entity.SetName()</code>
	i32 SetName(IFunctionHandler* pH);

	//! <code>Entity.GetName()</code>
	i32 GetName(IFunctionHandler* pH);

	//! <code>Entity.GetRawId()</code>
	//! <description>Returns entityId in raw numeric format.</description>
	i32 GetRawId(IFunctionHandler* pH);

	//! <code>Entity.SetAIName()</code>
	i32 SetAIName(IFunctionHandler* pH);

	//! <code>Entity.GetAIName()</code>
	i32 GetAIName(IFunctionHandler* pH);

	//! <code>Entity.SetFlags( flags, mode )</code>
	//! <description>Mode: 0: or  1: and  2: xor</description>
	i32 SetFlags(IFunctionHandler* pH, i32 flags, i32 mode);

	//! <code>Entity.GetFlags()</code>
	i32 GetFlags(IFunctionHandler* pH);

	//! <code>Entity.HasFlags( flags )</code>
	i32 HasFlags(IFunctionHandler* pH, i32 flags);

	//! <code>Entity.SetFlagsExtended( flags, mode )</code>
	//! <description>Mode: 0: or  1: and  2: xor</description>
	i32 SetFlagsExtended(IFunctionHandler* pH, i32 flags, i32 mode);

	//! <code>Entity.GetFlagsExtended()</code>
	i32 GetFlagsExtended(IFunctionHandler* pH);

	//! <code>Entity.HasFlags( flags )</code>
	i32 HasFlagsExtended(IFunctionHandler* pH, i32 flags);

	//! <code>Entity.GetArchetype()</code>
	//! <description>Retrieve the archetype of the entity.</description>
	//! <returns>name of entity archetype, nil if no archetype.</returns>
	i32 GetArchetype(IFunctionHandler* pH);

	//! <code>Entity.IntersectRay( slot, rayOrigin, rayDir, maxDistance )</code>
	i32 IntersectRay(IFunctionHandler* pH, i32 slot, Vec3 rayOrigin, Vec3 rayDir, float maxDistance);

	//////////////////////////////////////////////////////////////////////////
	// Attachments
	//////////////////////////////////////////////////////////////////////////

	//! <code>Entity.AttachChild( childEntityId, flags )</code>
	i32 AttachChild(IFunctionHandler* pH, ScriptHandle childEntityId, i32 flags);

	//! <code>Entity.DetachThis()</code>
	i32 DetachThis(IFunctionHandler* pH);

	//! <code>Entity.DetachAll()</code>
	i32 DetachAll(IFunctionHandler* pH);

	//! <code>Entity.GetParent()</code>
	i32 GetParent(IFunctionHandler* pH);

	//! <code>Entity.GetChildCount()</code>
	i32 GetChildCount(IFunctionHandler* pH);

	//! <code>Entity.GetChild( i32 nIndex )</code>
	i32 GetChild(IFunctionHandler* pH, i32 nIndex);

	//! <code>Entity.EnableInheritXForm()</code>
	//! <description>Enables/Disable entity from inheriting transformation from the parent.</description>
	i32 EnableInheritXForm(IFunctionHandler* pH, bool bEnable);

	//! <code>Entity.NetPresent()</code>
	i32 NetPresent(IFunctionHandler* pH);

	//! <code>Entity.RenderShadow()</code>
	i32 RenderShadow(IFunctionHandler* pH);

	//! <code>Entity.SetRegisterInSectors()</code>
	i32 SetRegisterInSectors(IFunctionHandler* pH);

	//! <code>Entity.IsColliding()</code>
	i32 IsColliding(IFunctionHandler* pH);

	//! <code>Entity.GetDirectionVector()</code>
	i32 GetDirectionVector(IFunctionHandler* pH);

	//! <code>Entity.SetDirectionVector( direction )</code>
	i32 SetDirectionVector(IFunctionHandler* pH, Vec3 dir);

	//! <code>Entity.IsAnimationRunning( characterSlot, layer )</code>
	//! <param name="characterSlot">Index of the character slot.</param>
	//! <param name="layer">Index of the animation layer.</param>
	//! <returns>nil or not nil</returns>
	i32 IsAnimationRunning(IFunctionHandler* pH, i32 characterSlot, i32 layer);

	//! <code>Entity.AddImpulse( ipart, position, direction, linearImpulse, linearImpulseScale, angularAxis, angularImpulse, massScale )</code>
	//! <description>
	//!    Apply an impulse to the entity.
	//!    At least four parameters need to be provided for a linear impulse.
	//!    For an additional angular impulse, at least seven parameters need to be provided.
	//! </description>
	//!	<param name="ipart">The index of the part that receives the impulse.</param>
	//!	<param name="position">The point (in world coordinates) where the impulse is applied. Set this to (0, 0, 0) to ignore it.</param>
	//!	<param name="direction">The direction in which the impulse is applied.</param>
	//!	<param name="linearImpulse">The force of the linear impulse.</param>
	//!	<param name="linearImpulseScale">Scaling of the linear impulse. (Default: 1.0)</param>
	//!	<param name="angularAxis">The axis on which the angular impulse is applied.</param>
	//!	<param name="angularImpulse">The force of the the angular impulse.</param>
	//!	<param name="massScale">Mass scaling of the angular impulse. (Default: 1.0)</param>
	i32 AddImpulse(IFunctionHandler* pH);

	//! <code>Entity.AddConstraint()</code>
	i32 AddConstraint(IFunctionHandler* pH);

	//! <code>Entity.SetPublicParam()</code>
	//! <description>Set a shader parameter</description>
	//!	<param name="paramName">The name of the shader parameter.</param>
	//!	<param name="value">The new value of the parameter.</param>
	i32 SetPublicParam(IFunctionHandler* pH);

	// Audio

	//! <code>Entity.GetAllAuxAudioProxiesID()</code>
	//! <description>
	//!    Returns the ID used to address all AuxAudioProxy of the parent EntityAudioProxy.
	//! </description>
	//! <returns>Returns the ID used to address all AuxAudioProxy of the parent EntityAudioProxy.</returns>
	i32 GetAllAuxAudioProxiesID(IFunctionHandler* pH);

	//! <code>Entity.GetDefaultAuxAudioProxyID()</code>
	//! <description>
	//!    Returns the ID of the default AudioProxy of the parent EntityAudioProxy.
	//! </description>
	//! <returns>Returns the ID of the default AudioProxy of the parent EntityAudioProxy.</returns>
	i32 GetDefaultAuxAudioProxyID(IFunctionHandler* pH);

	//! <code>Entity.CreateAuxAudioProxy()</code>
	//! <description>
	//!    Creates an additional AudioProxy managed by the EntityAudioProxy.
	//!    The created AuxAudioProxy will move and rotate with the parent EntityAudioProxy.
	//! </description>
	//! <returns>Returns the ID of the additionally created AudioProxy.</returns>
	i32 CreateAuxAudioProxy(IFunctionHandler* pH);

	//! <code>Entity.RemoveAuxAudioProxy( hAudioProxyLocalID )</code>
	//! <description>
	//!    Removes the AuxAudioProxy corresponding to the passed ID from the parent EntityAudioProxy.
	//! </description>
	//!		<param name="hAudioProxyLocalID">hAudioProxyLocalID - ID of the AuxAudioProxy to be removed from the parent EntityAudioProxy.</param>
	//! <returns>nil</returns>
	i32 RemoveAuxAudioProxy(IFunctionHandler* pH, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.ExecuteAudioTrigger( hTriggerID, hAudioProxyLocalID )</code>
	//! <description>
	//!    Execute the specified audio trigger and attach it to the entity.
	//!    The created audio object will move and rotate with the entity.
	//! </description>
	//!		<param name="hTriggerID">the audio trigger ID handle</param>
	//!		<param name="hAudioProxyLocalID">ID of the AuxAudioProxy local to the EntityAudioProxy (to address the default AuxAudioProxy pass 1 to address all AuxAudioProxies pass 0)</param>
	//! <returns>nil</returns>
	i32 ExecuteAudioTrigger(IFunctionHandler* pH, ScriptHandle const hTriggerID, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.StopAudioTrigger( hTriggerID, hAudioProxyLocalID )</code>
	//! <description>Stop the audio event generated by the trigger with the specified ID on this entity.</description>
	//!		<param name="hTriggerID">the audio trigger ID handle</param>
	//!		<param name="hAudioProxyLocalID">ID of the AuxAudioProxy local to the EntityAudioProxy (to address the default AuxAudioProxy pass 1 to address all AuxAudioProxies pass 0)</param>
	//! <returns>nil</returns>
	i32 StopAudioTrigger(IFunctionHandler* pH, ScriptHandle const hTriggerID, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.SetAudioSwitchState( hSwitchID, hSwitchStateID, hAudioProxyLocalID )</code>
	//! <description>Set the specified audio switch to the specified state on the current Entity.</description>
	//!		<param name="hSwitchID">the audio switch ID handle</param>
	//!		<param name="nSwitchStateID">the switch state ID handle</param>
	//!		<param name="hAudioProxyLocalID">ID of the AuxAudioProxy local to the EntityAudioProxy (to address the default AuxAudioProxy pass 1 to address all AuxAudioProxies pass 0)</param>
	//! <returns>nil</returns>
	i32 SetAudioSwitchState(IFunctionHandler* pH, ScriptHandle const hSwitchID, ScriptHandle const hSwitchStateID, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.SetAudioObstructionCalcType( nObstructionCalcType, hAudioProxyLocalID )</code>
	//! <description>Set the Audio Obstruction/Occlusion calculation type on the underlying GameAudioObject.</description>
	//!		<param name="nObstructionCalcType">Obstruction/Occlusion calculation type;
	//!				Possible values:
	//!				0 - ignore Obstruction/Occlusion
	//!				1 - use single physics ray
	//!				2 - use multiple physics rays (currently 5 per object)</param>
	//!		<param name="hAudioProxyLocalID">ID of the AuxAudioProxy local to the EntityAudioProxy (to address the default AuxAudioProxy pass 1 to address all AuxAudioProxies pass 0)</param>
	//! <returns>nil</returns>
	i32 SetAudioObstructionCalcType(IFunctionHandler* pH, i32 const nObstructionCalcType, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.SetFadeDistance( fFadeDistance )</code>
	//! <description>Sets the distance in which this entity will execute fade calculations.</description>
	//!		<param name="fFadeDistance">fade distance in meters</param>
	//! <returns>nil</returns>
	i32 SetFadeDistance(IFunctionHandler* pH, float const fFadeDistance);

	//! <code>Entity.SetAudioProxyOffset( vOffset, hAudioProxyLocalID )</code>
	//! <description>Set offset on the AudioProxy attached to the Entity.</description>
	//!		<param name="vOffset">offset vector</param>
	//!		<param name="hAudioProxyLocalID">ID of the AuxAudioProxy local to the EntityAudioProxy (to address the default AuxAudioProxy pass 1 to address all AuxAudioProxies pass 0)</param>
	//! <returns>nil</returns>
	i32 SetAudioProxyOffset(IFunctionHandler* pH, Vec3 const vOffset, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.SetEnvironmentFadeDistance( fEnvironmentFadeDistance )</code>
	//! <description>Sets the distance over which this entity will fade the audio environment amount for all approaching entities.</description>
	//!		<param name="fEnvironmentFadeDistance">fade distance in meters</param>
	//! <returns>nil</returns>
	i32 SetEnvironmentFadeDistance(IFunctionHandler* pH, float const fEnvironmentFadeDistance);

	//! <code>Entity.SetAudioEnvironmentID( nAudioEnvironmentID )</code>
	//! <description>Sets the ID of the audio environment this entity will set for entities inside it.</description>
	//!		<param name="nAudioEnvironmentID">audio environment ID</param>
	//! <returns>nil</returns>
	i32 SetAudioEnvironmentID(IFunctionHandler* pH, ScriptHandle const hAudioEnvironmentID);

	//! <code>Entity.SetCurrentAudioEnvironments()</code>
	//! <description>Sets the correct audio environment amounts based on the entity's position in the world.</description>
	//! <returns>nil</returns>
	i32 SetCurrentAudioEnvironments(IFunctionHandler* pH);

	//! <code>Entity.SetAudioRtpcValue( hRtpcID, fValue, hAudioProxyLocalID )</code>
	//! <description>Set the specified audio RTPC to the specified value on the current Entity.</description>
	//!		<param name="hRtpcID">the audio RTPC ID handle</param>
	//!		<param name="fValue">the RTPC value</param>
	//!		<param name="hAudioProxyLocalID">ID of the AuxAudioProxy local to the EntityAudioProxy (to address the default AuxAudioProxy pass 1 to address all AuxAudioProxies pass 0)</param>
	//! <returns>nil</returns>
	i32 SetAudioRtpcValue(IFunctionHandler* pH, ScriptHandle const hRtpcID, float const fValue, ScriptHandle const hAudioProxyLocalID);

	//! <code>Entity.AuxAudioProxiesMoveWithEntity( bCanMoveWithEntity )</code>
	//! <description>Set whether AuxAudioProxies should move with the entity or not.</description>
	//!		<param name="bCanMoveWithEntity">boolean parameter to enable or disable</param>
	//! <returns>nil</returns>
	i32 AuxAudioProxiesMoveWithEntity(IFunctionHandler* pH, bool const bCanMoveWithEntity);

	// ~Audio

	//! <code> Entity.SetGeomCachePlaybackTime()</code>
	//! <description>Sets the playback time.</description>
	i32 SetGeomCachePlaybackTime(IFunctionHandler* pH, float time);

	//! <code> Entity.SetGeomCacheParams()</code>
	//! <description>Sets geometry cache parameters.</description>
	i32 SetGeomCacheParams(IFunctionHandler* pH, bool looping, tukk standIn, tukk standInMaterial, tukk firstFrameStandIn,
	                       tukk firstFrameStandInMaterial, tukk lastFrameStandIn, tukk lastFrameStandInMaterial, float standInDistance, float streamInDistance);
	//! <code> Entity.SetGeomCacheStreaming()</code>
	//! <description>Activates/deactivates geom cache streaming.</description>
	i32 SetGeomCacheStreaming(IFunctionHandler* pH, bool active, float time);

	//! <code> Entity.IsGeomCacheStreaming()</code>
	//! <returns>true if geom cache is streaming.</returns>
	i32 IsGeomCacheStreaming(IFunctionHandler* pH);

	//! <code> Entity.GetGeomCachePrecachedTime()</code>
	//! <description>Gets time delta from current playback position to last ready to play frame.</description>
	i32 GetGeomCachePrecachedTime(IFunctionHandler* pH);

	//! <code> Entity.SetGeomCacheDrawing()</code>
	//! <description>Activates/deactivates geom cache drawing.</description>
	i32 SetGeomCacheDrawing(IFunctionHandler* pH, bool active);

	//! <code>Entity.StartAnimation()</code>
	i32 StartAnimation(IFunctionHandler* pH);

	//! <code>Entity.StopAnimation( characterSlot, layer )</code>
	i32 StopAnimation(IFunctionHandler* pH, i32 characterSlot, i32 layer);

	//! <code>Entity.ResetAnimation( characterSlot, layer )</code>
	i32 ResetAnimation(IFunctionHandler* pH, i32 characterSlot, i32 layer);

	//! <code>Entity.RedirectAnimationToLayer0( characterSlot, redirect )</code>
	i32 RedirectAnimationToLayer0(IFunctionHandler* pH, i32 characterSlot, bool redirect);

	//! <code>Entity.SetAnimationBlendOut( characterSlot, layer, blendOut )</code>
	i32 SetAnimationBlendOut(IFunctionHandler* pH, i32 characterSlot, i32 layer, float blendOut);

	//! <code>Entity.EnableBoneAnimation( characterSlot, layer, boneName, status )</code>
	i32 EnableBoneAnimation(IFunctionHandler* pH, i32 characterSlot, i32 layer, tukk boneName, bool status);

	//! <code>Entity.EnableBoneAnimationAll( characterSlot, layer, status )</code>
	i32 EnableBoneAnimationAll(IFunctionHandler* pH, i32 characterSlot, i32 layer, bool status);

	//! <code>Entity.EnableProceduralFacialAnimation( enable )</code>
	i32 EnableProceduralFacialAnimation(IFunctionHandler* pH, bool enable);

	//! <code>Entity.PlayFacialAnimation( name, looping )</code>
	i32 PlayFacialAnimation(IFunctionHandler* pH, tuk name, bool looping);

	//! <code>Entity.SetAnimationEvent( nSlot, sAnimation )</code>
	i32 SetAnimationEvent(IFunctionHandler* pH, i32 nSlot, tukk sAnimation);

	//! <code>Entity.SetAnimationKeyEvent( nSlot, sAnimation, nFrameID, sEvent)</code>
	i32 SetAnimationKeyEvent(IFunctionHandler* pH);

	//! <code>Entity.DisableAnimationEvent( nSlot, sAnimation )</code>
	i32 DisableAnimationEvent(IFunctionHandler* pH, i32 nSlot, tukk sAnimation);

	//! <code> Entity.SetAnimationSpeed( characterSlot, layer, speed )</code>
	i32 SetAnimationSpeed(IFunctionHandler* pH, i32 characterSlot, i32 layer, float speed);

	//! <code> Entity.SetAnimationTime( nSlot, nLayer, fTime )</code>
	i32 SetAnimationTime(IFunctionHandler* pH, i32 nSlot, i32 nLayer, float fTime);

	//! <code> Entity.GetAnimationTime( nSlot, nLayer )</code>
	i32 GetAnimationTime(IFunctionHandler* pH, i32 nSlot, i32 nLayer);

	//! <code> Entity.GetCurAnimation()</code>
	i32 GetCurAnimation(IFunctionHandler* pH);

	//! <code> Entity.GetAnimationLength( characterSlot, animation )</code>
	i32 GetAnimationLength(IFunctionHandler* pH, i32 characterSlot, tukk animation);

	//! <code> Entity.SetAnimationFlip( characterSlot, flip )</code>
	i32 SetAnimationFlip(IFunctionHandler* pH, i32 characterSlot, Vec3 flip);

	//! <code> Entity.SetTimer()</code>
	i32 SetTimer(IFunctionHandler* pH);

	//! <code> Entity.KillTimer()</code>
	i32 KillTimer(IFunctionHandler* pH);

	//! <code> Entity.SetScriptUpdateRate( nMillis )</code>
	i32 SetScriptUpdateRate(IFunctionHandler* pH, i32 nMillis);

	//////////////////////////////////////////////////////////////////////////
	// State management.
	//////////////////////////////////////////////////////////////////////////

	//! <code> Entity.GotoState( sState )</code>
	i32 GotoState(IFunctionHandler* pH, tukk sState);

	//! <code> Entity.IsInState( sState )</code>
	i32 IsInState(IFunctionHandler* pH, tukk sState);

	//! <code> Entity.GetState()</code>
	i32 GetState(IFunctionHandler* pH);

	//////////////////////////////////////////////////////////////////////////

	//! <code> Entity.IsHidden()</code>
	i32 IsHidden(IFunctionHandler* pH);

	//! <code> Entity.GetTouchedSurfaceID()</code>
	i32 GetTouchedSurfaceID(IFunctionHandler* pH);

	//! <code> Entity.GetTouchedPoint()</code>
	//! <description>Retrieves point of collision for rigid body.</description>
	i32 GetTouchedPoint(IFunctionHandler* pH);

	//! <code> Entity.CreateBoneAttachment( characterSlot, boneName, attachmentName )</code>
	i32 CreateBoneAttachment(IFunctionHandler* pH, i32 characterSlot, tukk boneName, tukk attachmentName);

	//! <code> Entity.CreateSkinAttachment( characterSlot, attachmentName )</code>
	i32 CreateSkinAttachment(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName);

	//! <code> Entity.DestroyAttachment( characterSlot, attachmentName )</code>
	i32 DestroyAttachment(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName);

	//! <code> Entity.GetAttachmentBone( characterSlot, attachmentName )</code>
	i32 GetAttachmentBone(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName);

	//! <code> Entity.GetAttachmentCGF( characterSlot, attachmentName )</code>
	i32 GetAttachmentCGF(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName);

	//! <code> Entity.ResetAttachment( characterSlot, attachmentName )</code>
	i32 ResetAttachment(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName);

	//! <code> Entity.SetAttachmentEffect( characterSlot, attachmentName, effectName, offset, dir, scale, flags )</code>
	i32 SetAttachmentEffect(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, tukk effectName, Vec3 offset, Vec3 dir, float scale, i32 flags);

	//! <code> Entity.SetAttachmentObject( characterSlot, attachmentName, entityId, slot, flags )</code>
	i32 SetAttachmentObject(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, ScriptHandle entityId, i32 slot, i32 flags);

	//! <code> Entity.SetAttachmentCGF( characterSlot, attachmentName, filePath )</code>
	i32 SetAttachmentCGF(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, tukk filePath);

	//! <code> Entity.SetAttachmentLight( characterSlot, attachmentName, lightTable, flags )</code>
	i32 SetAttachmentLight(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, SmartScriptTable lightTable, i32 flags);

	//! <code> Entity.SetAttachmentPos( characterSlot, attachmentName, pos )</code>
	i32 SetAttachmentPos(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, Vec3 pos);

	//! <code> Entity.SetAttachmentAngles( characterSlot, attachmentName, angles )</code>
	i32 SetAttachmentAngles(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, Vec3 angles);

	//! <code> Entity.SetAttachmentDir()</code>
	i32 SetAttachmentDir(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, Vec3 dir, bool worldSpace);

	//! <code> Entity.HideAttachment( characterSlot, attachmentName, hide, hideShadow )</code>
	i32 HideAttachment(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, bool hide, bool hideShadow);

	//! <code> Entity.HideAllAttachments( characterSlot, hide, hideShadow )</code>
	i32 HideAllAttachments(IFunctionHandler* pH, i32 characterSlot, bool hide, bool hideShadow);

	//! <code> Entity.HideAttachmentMaster( characterSlot, hide )</code>
	i32 HideAttachmentMaster(IFunctionHandler* pH, i32 characterSlot, bool hide);

	//! <code> Entity.PhysicalizeAttachment( characterSlot, attachmentName, physicalize )</code>
	i32 PhysicalizeAttachment(IFunctionHandler* pH, i32 characterSlot, tukk attachmentName, bool physicalize);

	//! <code> Entity.Damage()</code>
	i32 Damage(IFunctionHandler* pH);

	//! <code> Entity.GetEntitiesInContact()</code>
	i32 GetEntitiesInContact(IFunctionHandler* pH);

	//! <code> Entity.GetExplosionObstruction()</code>
	i32 GetExplosionObstruction(IFunctionHandler* pH);

	//! <code> Entity.GetExplosionImpulse()</code>
	i32 GetExplosionImpulse(IFunctionHandler* pH);

	//! <code> Entity.SetMaterial()</code>
	i32 SetMaterial(IFunctionHandler* pH);

	//! <code> Entity.GetMaterial()</code>
	i32 GetMaterial(IFunctionHandler* pH);

	//! <code> Entity.GetEntityMaterial()</code>
	i32 GetEntityMaterial(IFunctionHandler* pH);

	//! <code> Entity.ChangeAttachmentMaterial(attachmentName, materialName)</code>
	i32 ChangeAttachmentMaterial(IFunctionHandler* pH, tukk attachmentName, tukk materialName);

	//! <code> Entity.ReplaceMaterial( slot, name, replacement )</code>
	i32 ReplaceMaterial(IFunctionHandler* pH, i32 slot, tukk name, tukk replacement);

	//! <code> Entity.ResetMaterial( slot )</code>
	i32 ResetMaterial(IFunctionHandler* pH, i32 slot);

	/*	i32 AddMaterialLayer(IFunctionHandler *pH, i32 slotId, tukk shader);
	   i32 RemoveMaterialLayer(IFunctionHandler *pH, i32 slotId, i32 id);
	   i32 RemoveAllMaterialLayers(IFunctionHandler *pH, i32 slotId);
	   i32 SetMaterialLayerParamF(IFunctionHandler *pH, i32 slotId, i32 layerId, tukk name, float value);
	   i32 SetMaterialLayerParamV(IFunctionHandler *pH, i32 slotId, i32 layerId, tukk name, Vec3 vec);
	   i32 SetMaterialLayerParamC(IFunctionHandler *pH, i32 slotId, i32 layerId, tukk name,
	    float r, float g, float b, float a);
	 */
	//! <code> Entity.EnableMaterialLayer( enable, layer )</code>
	i32 EnableMaterialLayer(IFunctionHandler* pH, bool enable, i32 layer);

	//! <code>Entity.CloneMaterial( nSlotId, sSubMaterialName )</code>
	//! <description>
	//!		Replace material on the slot with a cloned version of the material.
	//!		Cloned material can be freely changed uniquely for this entity.
	//! </description>
	//!		<param name="nSlotId">On which slot to clone material.</param>
	//!		<param name="sSubMaterialName">if non empty string only this specific sub-material is cloned.</param>
	i32 CloneMaterial(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.SetMaterialFloat( nSlotId, nSubMtlId, sParamName, fValue )</code>
	//! <description>Change material parameter.</description>
	//!		<param name="nSlot">On which slot to change material.</param>
	//!		<param name="nSubMtlId">Specify sub-material by Id.</param>
	//!		<param name="sParamName">Name of the material parameter.</param>
	//!		<param name="fValue">New material parameter value.</param>
	i32 SetMaterialFloat(IFunctionHandler* pH, i32 slot, i32 nSubMtlId, tukk sParamName, float fValue);

	//! <code>Entity.GetMaterialFloat( nSlotId, nSubMtlId, sParamName )</code>
	//! <description>Change material parameter.</description>
	//!		<param name="nSlot">On which slot to change material.</param>
	//!		<param name="nSubMtlId">Specify sub-material by Id.</param>
	//!		<param name="sParamName">Name of the material parameter.</param>
	//! <returns>Material parameter value.</returns>
	i32 GetMaterialFloat(IFunctionHandler* pH, i32 slot, i32 nSubMtlId, tukk sParamName);

	//! <code>Entity.SetMaterialVec3( nSlotId, nSubMtlId, sParamName, vVec3 )</code>
	//! <seealso cref="SetMaterialFloat">
	i32 SetMaterialVec3(IFunctionHandler* pH, i32 slot, i32 nSubMtlId, tukk sParamName, Vec3 fValue);

	//! <code>Entity.GetMaterialVec3( nSlotId, nSubMtlId, sParamName )</code>
	//! <seealso cref="GetMaterialFloat">
	i32 GetMaterialVec3(IFunctionHandler* pH, i32 slot, i32 nSubMtlId, tukk sParamName);

	//! <code>Entity.MaterialFlashInvoke()</code>
	i32 MaterialFlashInvoke(IFunctionHandler* pH);

	//! <code>Entity.ToLocal( slotId, point )</code>
	i32 ToLocal(IFunctionHandler* pH, i32 slotId, Vec3 point);

	//! <code>Entity.ToGlobal( slotId, point )</code>
	i32 ToGlobal(IFunctionHandler* pH, i32 slotId, Vec3 point);

	//! <code>Entity.VectorToLocal( slotId, dir )</code>
	i32 VectorToLocal(IFunctionHandler* pH, i32 slotId, Vec3 dir);

	//! <code>Entity.VectorToGlobal( slotId, dir )</code>
	i32 VectorToGlobal(IFunctionHandler* pH, i32 slotId, Vec3 dir);

	//! <code>Entity.CheckCollisions()</code>
	i32 CheckCollisions(IFunctionHandler* pH);

	//! <code>Entity.AwakeEnvironment()</code>
	i32 AwakeEnvironment(IFunctionHandler* pH);

	//! <code>Entity.GetTimeSinceLastSeen()</code>
	i32 GetTimeSinceLastSeen(IFunctionHandler* pH);

	//! <code>Entity.GetViewDistRatio()</code>
	i32 GetViewDistRatio(IFunctionHandler* pH);

	//! <code>Entity.SetViewDistRatio()</code>
	i32 SetViewDistRatio(IFunctionHandler* pH);

	//! <code>Entity.SetViewDistUnlimited()</code>
	i32 SetViewDistUnlimited(IFunctionHandler* pH);

	//! <code>Entity.SetLodRatio()</code>
	i32 SetLodRatio(IFunctionHandler* pH);

	//! <code>Entity.GetLodRatio()</code>
	i32 GetLodRatio(IFunctionHandler* pH);

	//! <code>Entity.SetStateClientside()</code>
	i32 SetStateClientside(IFunctionHandler* pH);

	//! <code>Entity.InsertSubpipe()</code>
	i32 InsertSubpipe(IFunctionHandler* pH);

	//! <code>Entity.CancelSubpipe()</code>
	i32 CancelSubpipe(IFunctionHandler* pH);

	//! <code>Entity.PassParamsToPipe()</code>
	i32 PassParamsToPipe(IFunctionHandler* pH);

	//! <code>Entity.SetDefaultIdleAnimations()</code>
	i32 SetDefaultIdleAnimations(IFunctionHandler* pH);

	//! <code>Entity.GetVelocity()</code>
	i32 GetVelocity(IFunctionHandler* pH);

	//! <code>Entity.GetVelocityEx()</code>
	i32 GetVelocityEx(IFunctionHandler* pH);

	//! <code>Entity.SetVelocity(velocity)</code>
	i32 SetVelocity(IFunctionHandler* pH, Vec3 velocity);

	//! <code>Entity.GetVelocityEx(velocity, angularVelocity)</code>
	i32 SetVelocityEx(IFunctionHandler* pH, Vec3 velocity, Vec3 angularVelocity);

	//! <code>Entity.GetSpeed()</code>
	i32 GetSpeed(IFunctionHandler* pH);

	//! <code>Entity.GetMass()</code>
	i32 GetMass(IFunctionHandler* pH);

	//! <code>Entity.GetVolume(slot)</code>
	i32 GetVolume(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.GetGravity()</code>
	i32 GetGravity(IFunctionHandler* pH);

	//! <code>Entity.GetSubmergedVolume( slot, planeNormal, planeOrigin )</code>
	i32 GetSubmergedVolume(IFunctionHandler* pH, i32 slot, Vec3 planeNormal, Vec3 planeOrigin);

	//! <code>Entity.CreateLink( name, targetId )</code>
	//! <description>
	//!		Creates a new outgoing link for this entity.
	//! </description>
	//!	<param name="name">Name of the link. Does not have to be unique among all the links of this entity. Multiple links with the same name can perfectly co-exist.</param>
	//!	<param name="(optional) targetId">If specified, the ID of the entity the link shall target. If not specified or 0 then the link will not target anything. Default value: 0</param>
	//! <returns>nothing</returns>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="GetLink"/>
	//! <seealso cref="CountLinks"/>
	i32 CreateLink(IFunctionHandler* pH, tukk name);

	//! <code>Entity.GetLinkName( targetId, ith )</code>
	//! <description>
	//!		Returns the name of the link that is targeting the entity with given ID.
	//! </description>
	//!	<param name="targetId">ID of the entity for which the link name shall be looked up.</param>
	//!	<param name="(optional) ith">If specified, the i'th link that targets given entity. Default value: 0 (first entity)</param>
	//! <returns>The name of the i'th link targeting given entity or nil if no such link exists.</returns>
	//! <seealso cref="CreateLinkTarget"/>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="GetLink"/>
	//! <seealso cref="CountLinks"/>
	i32 GetLinkName(IFunctionHandler* pH, ScriptHandle targetId);

	//! <code>Entity.SetLinkTarget(name, targetId, ith)</code>
	//! <description>
	//!		Specifies the entity that an existing link shall target. Use this function to change the target of an existing link.
	//! </description>
	//!	<param name="name">Name of the link that shall target given entity.</param>
	//!	<param name="targetId">The ID of the entity the link shall target. Pass in NULL_ENTITY to make the link no longer target an entity.</param>
	//!	<param name="(optional) ith">If specified, the i'th link with given name that shall target given entity. Default value: 0 (first link with given name)</param>
	//! <returns>nothing</returns>
	//! <seealso cref="CreateLink"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="GetLink"/>
	//! <seealso cref="CountLinks"/>
	i32 SetLinkTarget(IFunctionHandler* pH, tukk name, ScriptHandle targetId);

	//! <code>Entity.GetLinkTarget( name, ith )</code>
	//! <description>
	//!		Returns the ID of the entity that given link is targeting.
	//! </description>
	//!	<param name="name">Name of the link.</param>
	//!	<param name="(optional) ith">If specified, the i'th link with given name for which to look up the targeted entity. Default value: 0 (first link with given name)</param>
	//! <returns>The ID of the entity that the link is targeting or nil if no such link exists.</returns>
	//! <seealso cref="CreateLink"/>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="GetLink"/>
	//! <seealso cref="CountLinks"/>
	i32 GetLinkTarget(IFunctionHandler* pH, tukk name);

	//! <code>Entity.RemoveLink( name, ith )</code>
	//! <description>
	//!		Removes an outgoing link from the entity.
	//! </description>
	//!	<param name="name">Name of the link to remove.</param>
	//!	<param name="(optional) ith">If specified, the i'th link with given name that shall be removed. Default value: 0 (first link with given name)</param>
	//! <returns>nothing</returns>
	//! <seealso cref="CreateLink"/>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="GetLink"/>
	//! <seealso cref="CountLinks"/>
	i32 RemoveLink(IFunctionHandler* pH, tukk name);

	//! <code>Entity.RemoveAllLinks()</code>
	//! <description>
	//!		Removes all links of an entity.
	//! </description>
	//! <returns>nothing</returns>
	//! <seealso cref="CreateLink"/>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="GetLink"/>
	//! <seealso cref="CountLinks"/>
	i32 RemoveAllLinks(IFunctionHandler* pH);

	//! <code>Entity.GetLink()</code>
	//! <description>
	//!		Returns the link at given index.
	//! </description>
	//!	<param name="ith">The index of the link that shall be returned.</param>
	//! <returns>The script table of the entity that the i'th link is targeting or nil if the specified index is out of bounds.</returns>
	//! <seealso cref="CreateLink"/>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="CountLinks"/>
	i32 GetLink(IFunctionHandler* pH, i32 ith);

	//! <code>Entity.CountLinks()</code>
	//! <description>
	//!		Counts all outgoing links of the entity.
	//! </description>
	//! <returns>Number of outgoing links.</returns>
	//! <seealso cref="CreateLink"/>
	//! <seealso cref="SetLinkTarget"/>
	//! <seealso cref="GetLinkName"/>
	//! <seealso cref="GetLinkTarget"/>
	//! <seealso cref="RemoveLink"/>
	//! <seealso cref="RemoveAllLinks"/>
	//! <seealso cref="GetLink"/>
	i32 CountLinks(IFunctionHandler* pH);

	//! <code>Entity.RemoveDecals()</code>
	i32 RemoveDecals(IFunctionHandler* pH);

	//! <code>Entity.ForceCharacterUpdate( characterSlot, updateAlways )</code>
	i32 ForceCharacterUpdate(IFunctionHandler* pH, i32 characterSlot, bool updateAlways);

	//! <code>Entity.CharacterUpdateAlways( characterSlot, updateAlways )</code>
	i32 CharacterUpdateAlways(IFunctionHandler* pH, i32 characterSlot, bool updateAlways);

	//! <code>Entity.CharacterUpdateOnRender( characterSlot, bUpdateOnRender )</code>
	i32 CharacterUpdateOnRender(IFunctionHandler* pH, i32 characterSlot, bool bUpdateOnRender);

	//! <code>Entity.SetAnimateOffScreenShadow( bAnimateOffScreenShadow )</code>
	i32 SetAnimateOffScreenShadow(IFunctionHandler* pH, bool bAnimateOffScreenShadow);

	//! <code>Entity.RagDollize(slot)</code>
	i32 RagDollize(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.Hide()</code>
	i32 Hide(IFunctionHandler* pH);

	//! <code>Entity.NoExplosionCollision()</code>
	i32 NoExplosionCollision(IFunctionHandler* pH);

	//! <code>Entity.NoBulletForce( state )</code>
	i32 NoBulletForce(IFunctionHandler* pH, bool state);

	//! <code>Entity.UpdateAreas()</code>
	i32 UpdateAreas(IFunctionHandler* pH);

	//! <code>Entity.IsPointInsideArea( areaId, point )</code>
	i32 IsPointInsideArea(IFunctionHandler* pH, i32 areaId, Vec3 point);

	//! <code>Entity.IsEntityInsideArea( areaId, entityId )</code>
	i32 IsEntityInsideArea(IFunctionHandler* pH, i32 areaId, ScriptHandle entityId);

	//! <code>Entity.GetPhysicalStats()</code>
	//! <description>Some more physics related.</description>
	i32 GetPhysicalStats(IFunctionHandler* pH);

	//! <code>Entity.SetParentSlot( child, parent )</code>
	i32 SetParentSlot(IFunctionHandler* pH, i32 child, i32 parent);

	//! <code>Entity.GetParentSlot( child )</code>
	i32 GetParentSlot(IFunctionHandler* pH, i32 child);

	//! <code>Entity.BreakToPieces()</code>
	//! <description>Breaks static geometry in slot 0 into sub objects and spawn them as particles or entities.</description>
	i32 BreakToPieces(IFunctionHandler* pH, i32 nSlot, i32 nPiecesSlot, float fExplodeImp, Vec3 vHitPt, Vec3 vHitImp, float fLifeTime, bool bSurfaceEffects);

	//! <code>Entity.AttachSurfaceEffect( nSlot, effect, countPerUnit, form, typ, countScale, sizeScale )</code>
	i32 AttachSurfaceEffect(IFunctionHandler* pH, i32 nSlot, tukk effect, bool countPerUnit, tukk form, tukk typ, float countScale, float sizeScale);

	//////////////////////////////////////////////////////////////////////////
	// This method is for engine internal usage.

	//! <code>Entity.ProcessBroadcastEvent()</code>
	i32 ProcessBroadcastEvent(IFunctionHandler* pH);

	//! <code>Entity.ActivateOutput()</code>
	i32 ActivateOutput(IFunctionHandler* pH);

	//! <code>Entity.CreateCameraProxy()</code>
	//! <description>Create a proxy camera object for the entity, allows entity to serve as camera source for material assigned on the entity.</description>
	i32 CreateCameraProxy(IFunctionHandler* pH);

	//! <code>Entity.UnSeenFrames()</code>
	i32 UnSeenFrames(IFunctionHandler* pH);

	//! <code>Entity.DeleteParticleEmitter(slot)</code>
	//!		<param name="slot">slot number</param>
	//! <description>Deletes particles emitter from 3dengine.</description>
	i32 DeleteParticleEmitter(IFunctionHandler* pH, i32 slot);

	//! <code>Entity.RegisterForAreaEvents(enable)</code>
	//!		<param name="enable">0, for disable, any other value for enable.</param>
	//! <description>Registers the script proxy so that it receives area events for this entity.</description>
	i32 RegisterForAreaEvents(IFunctionHandler* pH, i32 enable);

	//! <code>Entity.RenderAlways(enable)</code>
	//!		<param name="enable">0, for disable, any other value for enable.</param>
	//! <description>Enables 'always render' on the render node, skipping any kind of culling.</description>
	i32 RenderAlways(IFunctionHandler* pH, i32 enable);

	//! <code>Entity.GetTimeOfDayHour()</code>
	//! <returns>current time of day as a float value.</returns>
	i32 GetTimeOfDayHour(IFunctionHandler* pH);

	//! <code>Entity.CreateDRSProxy()</code>
	//! <description>
	//!    Creates a Dynamic Response System Proxy
	//! </description>
	//! <returns>Returns the ID of the created proxy.</returns>
	i32 CreateDRSProxy(IFunctionHandler* pH);

private: // -------------------------------------------------------------------------------
	friend class CEntityComponentLuaScript;

	// Helper function to get IEntity pointer from IFunctionHandler
	CEntity* GetEntity(IFunctionHandler* pH);
	Vec3     GetGlobalGravity() const { return Vec3(0, 0, -9.81f); }
	i32      SetEntityPhysicParams(IFunctionHandler* pH, IPhysicalEntity* pe, i32 type, IScriptTable* pTable, ICharacterInstance* pIChar = nullptr);
	EntityId GetEntityID(IScriptTable* pEntityTable);

	ISystem* m_pISystem;

	bool ParseLightParams(IScriptTable* pLightTable, SRenderLight& light);
	bool ParseFogVolumesParams(IScriptTable* pTable, CEntity* pEntity, SFogVolumeProperties& properties);

	// Parse script table to the entity physical params table.
	bool ParsePhysicsParams(IScriptTable* pTable, SEntityPhysicalizeParams& params);

	typedef struct
	{
		Vec3  position;
		Vec3  v0, v1, v2;
		Vec2  uv0, uv1, uv2;
		Vec3  baricentric;
		float distance;
		bool  backface;
		char  material[256];

	} SIntersectionResult;

	static i32 __cdecl cmpIntersectionResult(ukk v1, ukk v2);
	i32                IStatObjRayIntersect(IStatObj* pStatObj, const Vec3& rayOrigin, const Vec3& rayDir, float maxDistance, SIntersectionResult* pOutResult, u32 maxResults);

	//////////////////////////////////////////////////////////////////////////
	// Structures used in Physicalize call.
	//////////////////////////////////////////////////////////////////////////
	pe_params_particle                       m_particleParams;
	pe_params_buoyancy                       m_buoyancyParams;
	pe_player_dimensions                     m_playerDimensions;
	pe_player_dynamics                       m_playerDynamics;
	pe_params_area                           m_areaGravityParams;
	SEntityPhysicalizeParams::AreaDefinition m_areaDefinition;
	std::vector<Vec3>                        m_areaPoints;
	pe_params_car                            m_carParams;

	//temp table used by GetPhysicalStats
	SmartScriptTable m_pStats;
};
