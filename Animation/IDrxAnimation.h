// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef DRXANIMATION_API
	#ifdef DRXANIMATION_EXPORTS
		#define DRXANIMATION_API DLL_EXPORT
	#else
		#define DRXANIMATION_API DLL_IMPORT
	#endif
#endif

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>

#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/CoreX/Extension/IDrxUnknown.h>

#include "DrxCharAnimationParams.h"

typedef i32 TJointId;

//! default return for invalid joint when searching through the skeleton
enum : TJointId { INVALID_JOINT_ID = -1 };

//! maximum number of LODs per one geometric model (DrxGeometry)
enum {g_nMaxGeomLodLevels = 6};

//! Flags used by ICharacterInstance::SetFlags and GetFlags.
enum ECharRenderFlags
{
	CS_FLAG_DRAW_MODEL           = 1 << 0,
	CS_FLAG_DRAW_NEAR            = 1 << 1,
	CS_FLAG_UPDATE               = 1 << 2,
	CS_FLAG_UPDATE_ALWAYS        = 1 << 3,
	CS_FLAG_COMPOUND_BASE        = 1 << 4,

	CS_FLAG_DRAW_WIREFRAME       = 1 << 5,  //!< Just for debug.
	CS_FLAG_DRAW_TANGENTS        = 1 << 6,  //!< Just for debug.
	CS_FLAG_DRAW_BINORMALS       = 1 << 7,  //!< Just for debug.
	CS_FLAG_DRAW_NORMALS         = 1 << 8,  //!< Just for debug.

	CS_FLAG_DRAW_LOCATOR         = 1 << 9,  //!< Just for debug.
	CS_FLAG_DRAW_SKELETON        = 1 << 10, //!< Just for debug.

	CS_FLAG_BIAS_SKIN_SORT_DIST  = 1 << 11,

	CS_FLAG_STREAM_HIGH_PRIORITY = 1 << 12,

	CS_FLAG_RENDER_NODE_VISIBLE  = 1 << 13, //!< Set by 3DEngine when render node owning character is potentially visible and needs rendering
};

enum CHRLOADINGFLAGS
{
	CA_IGNORE_LOD               = BIT(0),
	CA_CharEditModel            = BIT(1),
	CA_PreviewMode              = BIT(2),
	CA_DoNotStreamStaticObjects = BIT(3),
	CA_SkipSkelRecreation       = BIT(4),
	CA_DisableLogWarnings       = BIT(5),
	CA_SkipBoneRemapping        = BIT(6),
	CA_ImmediateMode            = BIT(7)
};

enum EReloadCAFResult
{
	CR_RELOAD_FAILED,
	CR_RELOAD_SUCCEED,
	CR_RELOAD_GAH_NOT_IN_ARRAY
};

enum CharacterToolFlags
{
	CA_CharacterTool      = 0x01,
	CA_DrawSocketLocation = 0x02,
	CA_BindPose           = 0x04,
	CA_AllowRedirection   = 0x08,  //!< Allow redirection in bindpose.
};

#define CHR            (0x11223344)
#define CGA            (0x55aa55aa)

#define NULL_ANIM      "null"
#define NULL_ANIM_FILE "null"

// Forward declarations
struct  IShader;
struct  SRendParams;
struct DinrusXDecalInfo;
struct ParticleParams;
struct DrxCharMorphParams;
struct IMaterial;
struct IStatObj;
struct IRenderMesh;

class CDefaultSkeleton;

struct ICharacterUpr;
struct ICharacterInstance;
struct ISkin;
struct IAttachmentSkin;
struct IDefaultSkeleton;
struct IAnimationSet;

struct ISkeletonAnim;
struct ISkeletonPose;
struct IAttachmentUpr;

struct IAttachment;
struct IAttachmentObject; // Entity, static object or character
struct IAnimEvents;
struct TFace;
struct IFacialInstance;
struct IFacialAnimation;
struct IAttachmentMerger;

class IDrxSizer;
struct DrxCharAnimationParams;

struct IAnimationPoseModifier;
typedef i32 (* CallBackFuncType)(ICharacterInstance*, uk );

struct IAnimationStreamingListener;
struct IAnimationSetListener;

struct IVertexFrames;
class CLodValue;

#include <drx3D/CoreX/Serialization/Forward.h>

struct IAnimationSerializable :
	public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IAnimationSerializable, "69b4f3ae-6197-4bee-ba70-d361b7975e69"_drx_guid);

	virtual void Serialize(Serialization::IArchive& ar) = 0;
};

DECLARE_SHARED_POINTERS(IAnimationSerializable);

struct IAnimationEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IAnimationEngineModule, "ea8faa6f-4ec9-48fb-935d-b54c09823b86"_drx_guid);
};

//! This class is the main access point for any character animation required for a program which uses DRXENGINE.
struct ICharacterUpr
{
	//! Priority when requested to load a DBA.
	enum EStreamingDBAPriority
	{
		eStreamingDBAPriority_Normal = 0,
		eStreamingDBAPriority_Urgent = 1,
	};

	//! Contains statistics about DrxCharUpr.
	struct Statistics
	{
		//! Number of character instances.
		unsigned numCharacters;

		//! Number of character models (CGF).
		unsigned numCharModels;

		//! Number of animobjects.
		unsigned numAnimObjects;

		//! Number of animobject models.
		unsigned numAnimObjectModels;
	};

	// <interfuscator:shuffle>
	virtual ~ICharacterUpr() {}

	//! Get statistics on the Animation System.
	//! Will fill the Statistics parameters with statistic on the instance of the Animation System.
	//! It is not recommended to call this function often.
	//! \param rStats Structure which holds the statistics.
	virtual void GetStatistics(Statistics& rStats) const = 0;

	//! Track memory usage.
	//! Gather the memory currently used by the animation. The information
	//! returned is classified according to the flags set in the sizer argument.
	//! \param pSizer Sizer class which will store the memory usage.
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	//! Create a new instance of a model
	//! Create a new instance for a model. Load the model file along with any animation file that might be available.
	//! \param szFilename Filename of the model to be loaded
	//! \param nFlags     Set how the model will be kept in memory after being used. Uses flags defined with EModelPersistence.
	//! \return Pointer to a ICharacterInstance class if the model could be loaded properly, or NULL if the model couldn't be loaded.
	virtual ICharacterInstance* CreateInstance(tukk szFilename, u32 nLoadingFlags = 0) = 0;
	virtual IDefaultSkeleton*   LoadModelSKEL(tukk szFilePath, u32 nLoadingFlags) = 0;
	virtual ISkin*              LoadModelSKIN(tukk szFilePath, u32 nLoadingFlags) = 0;

	//! Find and prefetch all resources for the required file character
	//! \param szFilename Filename of the character to be prefetched
	//! \param nFlags     Set how the model will be kept in memory after being used. Uses flags defined with EModelPersistence.
	//! \return Smart pointer to the placeholder representation of the character
	virtual bool LoadAndLockResources(tukk szFilePath, u32 nLoadingFlags) = 0;

	//! Ensure that render meshes are resident, and if not, begin streaming them.
	//! Each call with bKeep == true must be paired with a call with bKeep == false when no longer needed
	virtual void StreamKeepCharacterResourcesResident(tukk szFilePath, i32 nLod, bool bKeep, bool bUrgent = false) = 0;
	virtual bool StreamHasCharacterResources(tukk szFilePath, i32 nLod) = 0;

	//! Cleans up all resources.
	//! Cleans up all resources. Currently deletes all bodies and characters even if there are references on them.
	//! \param bForceCleanup - When set to true will force all models to be deleted, even if references to them still left.
	virtual void ClearResources(bool bForceCleanup) = 0;

	//! Update the Animation System.
	//! It's important to call this function at every frame. This should perform very fast.
	virtual void Update(bool bPaused) = 0;

	//! Update the character streaming.
	virtual void UpdateStreaming(i32 nFullUpdateRoundId, i32 nFastUpdateRoundId) = 0;

	//! Useful to prevent log spam: "several updates per frame...".
	//! Increment the frame counter.
	virtual void DummyUpdate() = 0;

	//! Release the Animation System.
	//! Releases any resource allocated by the Animation System and shut it down properly.
	virtual void Release() = 0;

	//! Retrieve all loaded models.
	virtual void GetLoadedModels(IDefaultSkeleton** pIDefaultSkeletons, u32& nCount) const = 0;

	//! Reloads loaded model.
	virtual void ReloadAllModels() = 0;
	virtual void ReloadAllCHRPARAMS() = 0;

	virtual void PreloadLevelModels() = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Retrieve facial animation interface.
	virtual IFacialAnimation*       GetIFacialAnimation() = 0;
	virtual const IFacialAnimation* GetIFacialAnimation() const = 0;

	virtual IAnimEvents*            GetIAnimEvents() = 0;
	virtual const IAnimEvents*      GetIAnimEvents() const = 0;
	//////////////////////////////////////////////////////////////////////////

	//! Use to synchronize all animation computations like forward kinematics, and calls
	//! all function which must occur after these like SkeletonPostProcess.
	//! Should be called only once per frame and as late as possible to prevent waiting
	//! for functions which run asynchronously.
	virtual void SyncAllAnimations() = 0;

	// Functions to load, lock and unload dba files
	virtual bool DBA_PreLoad(tukk filepath, ICharacterUpr::EStreamingDBAPriority priority) = 0;
	virtual bool DBA_LockStatus(tukk filepath, u32 status, ICharacterUpr::EStreamingDBAPriority priority) = 0;
	virtual bool DBA_Unload(tukk filepath) = 0;
	virtual bool DBA_Unload_All() = 0;

	//! Adds a runtime reference to a CAF animation; if not loaded it starts streaming it.
	virtual bool             CAF_AddRef(u32 filePathCRC) = 0;
	virtual bool             CAF_IsLoaded(u32 filePathCRC) const = 0;
	virtual bool             CAF_Release(u32 filePathCRC) = 0;
	virtual bool             CAF_LoadSynchronously(u32 filePathCRC) = 0;
	virtual bool             LMG_LoadSynchronously(u32 filePathCRC, const IAnimationSet* pAnimationSet) = 0;

	virtual EReloadCAFResult ReloadCAF(tukk szFilePathCAF) = 0;
	virtual i32              ReloadLMG(tukk szFilePathCAF) = 0;

	//! \return the DBA filename of an animation that is stored in a DBA.
	virtual tukk GetDBAFilePathByGlobalID(i32 globalID) const = 0;

	//! Set the listener which listens to events that happen during animation streaming.
	virtual void SetStreamingListener(IAnimationStreamingListener* pListener) = 0;

	//! Add nTicks to the number of Ticks spent this frame in animation functions.
	virtual void AddFrameTicks(uint64 nTicks) = 0;

	//! Add nTicks to the number of Ticks spent this frame in syncing animation jobs.
	virtual void AddFrameSyncTicks(uint64 nTicks) = 0;

	//! Reset Ticks Counter.
	virtual void ResetFrameTicks() = 0;

	//! Get number of Ticks accumulated over this frame.
	virtual uint64 NumFrameTicks() const = 0;

	//! Get number of Ticks accumulated over this frame in sync functions.
	virtual uint64 NumFrameSyncTicks() const = 0;

	//! Get the number of character instances that were processed asynchronously.
	virtual u32              NumCharacters() const = 0;

	virtual u32              GetNumInstancesPerModel(const IDefaultSkeleton& rIDefaultSkeleton) const = 0;
	virtual ICharacterInstance* GetICharInstanceFromModel(const IDefaultSkeleton& rIDefaultSkeleton, u32 num) const = 0;
	;

	virtual void                     SetAnimMemoryTracker(const SAnimMemoryTracker& amt) = 0;
	virtual SAnimMemoryTracker       GetAnimMemoryTracker() const = 0;

	virtual void                     UpdateRendererFrame() = 0;
	virtual void                     PostInit() = 0;

	virtual const IAttachmentMerger& GetIAttachmentMerger() const = 0;
	
	//! Extends the default skeleton of a character instance with skin attachments
	virtual void ExtendDefaultSkeletonWithSkinAttachments(ICharacterInstance* pCharInstance, tukk szFilepathSKEL, tukk* szSkinAttachments, u32k skinCount, u32k nLoadingFlags) = 0;
	// </interfuscator:shuffle>

#if BLENDSPACE_VISUALIZATION
	virtual void CreateDebugInstances(tukk szFilename) = 0;
	virtual void DeleteDebugInstances() = 0;
	virtual void RenderDebugInstances(const SRenderingPassInfo& passInfo) = 0;
	virtual void RenderBlendSpace(const SRenderingPassInfo& passInfo, ICharacterInstance* character, float fCharacterScale, u32 debugFlags) = 0;
	virtual bool HasDebugInstancesCreated(tukk szFilename) const = 0;
#endif
	virtual void GetMotionParameterDetails(SMotionParameterDetails& outDetails, EMotionParamID paramId) const = 0;
#ifdef EDITOR_PCDEBUGCODE
	virtual bool InjectCDF(tukk pathname, tukk content, size_t contentLength) = 0;
	virtual void ClearCDFCache() = 0;
	virtual void ClearAllKeepInMemFlags() = 0;
	virtual void InjectCHRPARAMS(tukk pathname, tukk content, size_t contentLength) = 0;
	virtual void ClearCHRPARAMSCache() = 0;
	virtual void InjectBSPACE(tukk pathname, tukk content, size_t contentLength) = 0;
	virtual void ClearBSPACECache() = 0;
#endif
};

//! \cond INTERNAL
//! This struct defines the interface for a class that listens to AnimLoaded, AnimUnloaded and AnimReloaded events
struct IAnimationStreamingListener
{
	// <interfuscator:shuffle>
	virtual ~IAnimationStreamingListener() {}

	//! Called when an animation finished loading.
	virtual void NotifyAnimLoaded(i32k globalID) = 0;

	//! Called when an animation gets unloaded.
	virtual void NotifyAnimUnloaded(i32k globalID) = 0;

	//! Called when an animation is reloaded from file.
	virtual void NotifyAnimReloaded(i32k globalID) = 0;
	// </interfuscator:shuffle>
};
//! \endcond

struct SJointProperty
{
	SJointProperty(tukk pname, float val) { name = pname; type = 0; fval = val; }
	SJointProperty(tukk pname, bool val) { name = pname; type = 1; bval = val; }
	SJointProperty(tukk pname, tukk val) { name = pname; type = 2; strval = val; }
	tukk name;
	i32         type;
	union
	{
		float       fval;
		bool        bval;
		tukk strval;
	};
};

struct SBoneShadowCapsule
{
	i32 arrJoints[2];
	float radius;
};

//////////////////////////////////////////////////////////////////////////
typedef u32 LimbIKDefinitionHandle;

//! Represents the skeleton tied to a character, and all the default properties it contains.
struct IDefaultSkeleton
{
	// <interfuscator:shuffle>
	virtual ~IDefaultSkeleton() {}
	virtual u32 GetJointCount() const = 0;

	virtual i32  GetJointParentIDByID(i32 id) const = 0;
	virtual i32  GetControllerIDByID(i32 id) const = 0;

	virtual i32  GetJointChildrenCountByID(i32 id) const = 0;

	//! Retrieves the ID of the Child Joint at index childIndex for parent joint id.
	//! If parent joint had less children than childIndex, or if the id is invalid in any way
	//! then it will return -1 to indicate child is not-found.
	virtual i32        GetJointChildIDAtIndexByID(i32 id, u32 childIndex) const = 0;

	virtual i32        GetJointIDByCRC32(u32 crc32) const = 0;
	virtual u32       GetJointCRC32ByID(i32 id) const = 0;

	virtual tukk  GetJointNameByID(i32 id) const = 0;
	//! Gets the identifier of a joint from the name specified in DCC
	//! \return A non-positive number on failure.
	//! \par Example
	//! \include DinrusXAnimation/Examples/GetJointOrientation.cpp
	virtual i32        GetJointIDByName(tukk name) const = 0;

	virtual const QuatT& GetDefaultAbsJointByID(u32 nJointIdx) const = 0;
	virtual const QuatT& GetDefaultRelJointByID(u32 nJointIdx) const = 0;

	//! \return File-path for this model.
	virtual tukk GetModelFilePath() const = 0;

	// All render-meshes will be removed from the CDefaultSkeleton-class.
	// The following functions will become deprecated.
	virtual const phys_geometry* GetJointPhysGeom(u32 jointIndex) const = 0;                 //!< just for statistics of physics proxies.
	virtual DrxBonePhysics*      GetJointPhysInfo(u32 jointIndex) = 0;
	virtual i32                GetLimbDefinitionIdx(LimbIKDefinitionHandle handle) const = 0;
	virtual void                 PrecacheMesh(bool bFullUpdate, i32 nRoundId, i32 nLod) = 0;
	virtual IRenderMesh*         GetIRenderMesh() const = 0;
	virtual Vec3                 GetRenderMeshOffset() const = 0;
	virtual u32               GetTextureMemoryUsage2(IDrxSizer* pSizer = 0) const = 0;
	virtual u32               GetMeshMemoryUsage(IDrxSizer* pSizer = 0) const = 0;
	// END: Will become deprecated.

	//! Retrieves list of shadow capsules for soft indirect shadows
	virtual const DynArray<SBoneShadowCapsule>&  GetShadowCapsules() const = 0;

	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
//! Represents a .skin mesh type
struct ISkin
{
	// <interfuscator:shuffle>
	virtual ~ISkin() {}

	//! Precache (streaming support).
	virtual void PrecacheMesh(bool bFullUpdate, i32 nRoundId, i32 nLod) = 0;

	//! Retrieve render mesh for specified lod.
	virtual IRenderMesh* GetIRenderMesh(u32 nLOD) const = 0;
	virtual tukk  GetModelFilePath() const = 0;
	virtual IMaterial*   GetIMaterial(u32 nLOD) const = 0;
	// </interfuscator:shuffle>

	virtual u32 GetNumLODs() const = 0;

#ifdef EDITOR_PCDEBUGCODE
	virtual Vec3                 GetRenderMeshOffset(u32 nLOD) const = 0;
	virtual u32               GetTextureMemoryUsage2(IDrxSizer* pSizer = 0) const = 0;
	virtual u32               GetMeshMemoryUsage(IDrxSizer* pSizer = 0) const = 0;
	virtual const IVertexFrames* GetVertexFrames() const = 0;
#endif
};

// Split this interface up into a few logical interfaces, starting with the IDrxCharModel.
struct SCharUpdateFeedback
{
	SCharUpdateFeedback() { flags = 0; pPhysHost = 0; mtxDelta.SetIdentity(); }
	i32              flags;     //!< |1 if pPhysHost is valid, |2 is mtxDelta is valid.
	IPhysicalEntity* pPhysHost; //!< tells the caller to restore this host as the main phys entity.
	Matrix34         mtxDelta;  //!< tells the caller to instantly post-multiply its matrix with this one.
};

struct SAnimationProcessParams
{
	QuatTS locationAnimation;
	bool   bOnRender;
	float  zoomAdjustedDistanceFromCamera;
	float  overrideDeltaTime;

	SAnimationProcessParams() :
		locationAnimation(IDENTITY),
		bOnRender(false),
		zoomAdjustedDistanceFromCamera(0.0f),
		overrideDeltaTime(-1.0f)
	{
	}
};

//! Interface to character animation.
//! This interface contains methods for manipulating and querying an animated character
//! instance. The methods allow to modify the animated instance, animate it, render,
//! retrieve BBox/etc, control physics, particles and skinning, transform.
struct ICharacterInstance : IMeshObj
{
	// <interfuscator:shuffle>

	//! Get the skeleton for this instance.
	//! Return a pointer of the instance of an ISkeletonAnim derived class applicable for the model.
	//! \return Pointer to an ISkeletonAnim derived class.
	virtual ISkeletonAnim*       GetISkeletonAnim() = 0;
	virtual const ISkeletonAnim* GetISkeletonAnim() const = 0;

	//! Get the skeleton for this instance.
	//! Return a pointer of the instance of an ISkeletonPose derived class applicable for the model.
	//! \return Pointer to an ISkeletonPose derived class.
	virtual ISkeletonPose*       GetISkeletonPose() = 0;
	virtual const ISkeletonPose* GetISkeletonPose() const = 0;

	//! Get the attachment manager for this instance.
	//! Return a pointer of the instance of an IAttachmentUpr derived class applicable for the model.
	//! \return Pointer to an IAttachmentUpr derived class.
	virtual IAttachmentUpr*       GetIAttachmentUpr() = 0;
	virtual const IAttachmentUpr* GetIAttachmentUpr() const = 0;

	//! \return the shared character model used by this instance.
	virtual IDefaultSkeleton&       GetIDefaultSkeleton() = 0;
	virtual const IDefaultSkeleton& GetIDefaultSkeleton() const = 0;

	//! Get the Animation Set defined for the model.
	//! Return a pointer of the instance of an IDrxAnimationSet derived class applicable for the model.
	//! \return Pointer to a IDrxAnimationSet derived class.
	virtual IAnimationSet*       GetIAnimationSet() = 0;
	virtual const IAnimationSet* GetIAnimationSet() const = 0;

	//! Get the animation event file
	//! Get the name of the file that stores the animation event definitions for this model. This
	//! is usually stored in the CHRPARAMS file
	//! \return Pointer to a null terminated char string which contains the filename of the database
	virtual tukk GetModelAnimEventDatabase() const = 0;

	//! Enables/disables StartAnimation* calls; puts warning into the log if StartAnimation* is called while disabled.
	virtual void EnableStartAnimation(bool bEnable) = 0;
	virtual void StartAnimationProcessing(const SAnimationProcessParams& params) = 0;

	//! Computes the render LOD from the wanted LOD value
	virtual CLodValue ComputeLod(i32 wantedLod, const SRenderingPassInfo& passInfo) = 0;

	//! Draw the character using specified rendering parameters.
	//! \param RendParams Rendering parameters.
	virtual void Render(const SRendParams& RendParams, const SRenderingPassInfo& passInfo) = 0;

	//! Set rendering flags defined in ECharRenderFlags for this character instance
	//! \param nFlags Rendering flags
	virtual void SetFlags(i32 nFlags) = 0;

	//! Get the enabled rendering flags. The valid flags are the ones declared in ECharRenderFlags.
	//! \return An integer value which holds the different rendering flags
	virtual i32 GetFlags() const = 0;

	//! Get the object type contained inside the character instance. It will return either the CHR or CGA constant.
	//! \return Object type constant
	virtual i32 GetObjectType() const = 0;

	//! Get the filename of the character. This is either the model path or the CDF path.
	//! \return Pointer to a null terminated char string which contain the filename of the character.
	virtual tukk GetFilePath() const = 0;

	virtual SMeshLodInfo ComputeGeometricMean() const = 0;

	virtual bool        HasVertexAnimation() const = 0;

	//! Returns material used to render this character, can be either a custom or model material.
	//! \return Pointer to an IMaterial class.
	virtual IMaterial* GetIMaterial() const = 0;

	//! Set custom instance material for this character.
	//! \param pMaterial A valid pointer to the material.
	virtual void SetIMaterial_Instance(IMaterial* pMaterial) = 0;

	//! Returns the instance specific material - if this is 0, then the default model material is in use.
	//! \return Pointer to the material, or 0.
	virtual IMaterial* GetIMaterial_Instance() const = 0;

	// Facial interface.
	virtual IFacialInstance*       GetFacialInstance() = 0;
	virtual const IFacialInstance* GetFacialInstance() const = 0;
	virtual void                   EnableFacialAnimation(bool bEnable) = 0;
	virtual void                   EnableProceduralFacialAnimation(bool bEnable) = 0;

	//! Set animation speed scale.
	//! This is the scale factor that affects the animation speed of the character.
	//! All the animations are played with the constant real-time speed multiplied by this factor.
	//! So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	virtual void   SetPlaybackScale(f32 fSpeed) = 0;
	virtual f32    GetPlaybackScale() const = 0;
	virtual u32 IsCharacterVisible() const = 0;

	// Skeleton effects interface.
	virtual void  SpawnSkeletonEffect(const AnimEventInstance& animEvent, const QuatTS &entityLoc) = 0;
	virtual void  KillAllSkeletonEffects() = 0;

	virtual void  SetViewdir(const Vec3& rViewdir) = 0;
	virtual float GetUniformScale() const = 0;

	virtual void  CopyPoseFrom(const ICharacterInstance& instance) = 0;

	// Functions for asynchronous execution of ProcessAnimationUpdate and ForwardKinematics.

	//! Makes sure all functions which must be called after forward kinematics are executed and also synchronizes the execution.
	//! Is called during GetISkeletonAnim and GetISkeletonPose if the bNeedSync flag is set to true(the default) to
	//! ensure all computations have finished when necessary.
	virtual void FinishAnimationComputations() = 0;

	// This is a hack to keep entity attachments in synch.
	virtual void SetAttachmentLocation_DEPRECATED(const QuatTS& newCharacterLocation) = 0;

	//! Called when the character is detached (if it was an attachment).
	virtual void OnDetach() = 0;

	//! Disable rendering of this render this instance.
	virtual void HideMaster(u32 h) = 0;

	//! Pushes the underlying tree of objects into the given Sizer object for statistics gathering.
	virtual void GetMemoryUsage(class IDrxSizer* pSizer) const = 0;
	virtual void Serialize(TSerialize ser) = 0;
	// </interfuscator:shuffle>

#ifdef EDITOR_PCDEBUGCODE
	virtual u32 GetResetMode() const = 0;                                             // Will be obsolete when CharEdit is removed.
	virtual void   SetResetMode(u32 rm) = 0;                                          // Will be obsolete when CharEdit is removed.
	virtual f32    GetAverageFrameTime() const = 0;
	virtual void   SetCharEditMode(u32 m) = 0;
	virtual u32 GetCharEditMode() const = 0;
	virtual void   DrawWireframeStatic(const Matrix34& m34, i32 nLOD, u32 color) = 0;

	//! Reload the animation set at any time.
	virtual void ReloadCHRPARAMS() = 0;
#endif

};

#include <drx3D/Animation/IAnimationPoseModifier.h>                                                    // <> required for Interfuscator

#ifndef SKELETON_ANIMATION_LAYER_COUNT
#define SKELETON_ANIMATION_LAYER_COUNT 32
#endif

//! Main interface to handle low-level animation processing on a character instance
struct ISkeletonAnim
{
	// <interfuscator:shuffle>
	enum
	{
		LayerCount = SKELETON_ANIMATION_LAYER_COUNT
	};

	virtual ~ISkeletonAnim() {}

	//! Enable special debug text for this skeleton
	virtual void SetDebugging(u32 flags) = 0;

	// Motion initialization
	virtual void   SetMirrorAnimation(u32 ts) = 0;
	virtual void   SetAnimationDrivenMotion(u32 ts) = 0;
	virtual u32 GetAnimationDrivenMotion() const = 0;
	virtual void   SetTrackViewExclusive(u32 i) = 0;
	virtual void   SetTrackViewMixingWeight(u32 layer, f32 weight) = 0;
	virtual u32 GetTrackViewStatus() const = 0;

	//! Starts playing back the specified animation in the layer specified in the provided parameters.
	//! \param szAnimName0 Name of the animation we want to play back, without the .caf or .i_caf suffixes.
	//! \param params Parameters that describe how the animation should be started, such as playback speed.
	//! \par Example
	//! \include DinrusXAnimation/Examples/StartAnimation.cpp
	virtual bool StartAnimation(tukk szAnimName0, const DrxCharAnimationParams& params) = 0;
	//! Starts playing back the specified animation in the layer specified in the provided parameters.
	//! \param id Unique animation identifier, useful to avoid looking up animation by name on every playback
	//! \param params Parameters that describe how the animation should be started, such as playback speed.
	virtual bool StartAnimationById(i32 id, const DrxCharAnimationParams& Params) = 0;
	//! Stops playback of the current animation in the specified layer, and specifies the time during which we will blend out
	virtual bool StopAnimationInLayer(i32 nLayer, f32 BlendOutTime) = 0;
	//! Seizes playback of animations in all layers
	virtual bool StopAnimationsAllLayers() = 0;

	//! Find an animation with a given user token.
	//! \param nUserToken User token that identifies an animation to search for.
	//! \param nLayer Layer of FIFO where to search for animation, if -1 all layers are searched.
	virtual CAnimation*       FindAnimInFIFO(u32 nUserToken, i32 nLayer = 1) = 0;
	virtual const CAnimation* FindAnimInFIFO(u32 nUserToken, i32 nLayer = 1) const = 0;

	//! Remove an animation with a given index and given layer.
	//! \param nLayer Animation layer to remove from.
	//! \param num Transition queue index to remove at.
	//! \param forceRemove Ignore special conditions and force a removal from the layer.
	virtual bool              RemoveAnimFromFIFO(u32 nLayer, u32 num, bool forceRemove = false) = 0;
	virtual i32               GetNumAnimsInFIFO(u32 nLayer) const = 0;
	virtual void              ClearFIFOLayer(u32 nLayer) = 0;
	virtual CAnimation&       GetAnimFromFIFO(u32 nLayer, u32 num) = 0;
	virtual const CAnimation& GetAnimFromFIFO(u32 nLayer, u32 num) const = 0;

	//! If manual update is set for anim, then set anim time and handle anim events.
	virtual void ManualSeekAnimationInFIFO(u32 nLayer, u32 num, float time, bool triggerAnimEvents) = 0;

	//! Makes sure there's no anim in this layer's queue that could cause a delay (useful when you want to play an
	//! animation that you want to be 100% sure is going to be transitioned to immediately).
	virtual void RemoveTransitionDelayConditions(u32 nLayer) = 0;

	virtual void SetLayerBlendWeight(i32 nLayer, f32 fMult) = 0;
	virtual void SetLayerPlaybackScale(i32 nLayer, f32 fSpeed) = 0;

	//! \note This does NOT override the overall animation speed, but it multiplies it.
	virtual f32 GetLayerPlaybackScale(u32 nLayer) const = 0;

	//! Updates the given motion parameter in order to select / blend between animations in blend spaces. Will perform clamping and clearing as needed.
	//! \par Example
	//! \include DinrusXAnimation/Examples/SetDesiredMotionParam.cpp
	virtual void SetDesiredMotionParam(EMotionParamID id, f32 value, f32 frametime) = 0;
	virtual bool GetDesiredMotionParam(EMotionParamID id, float& value) const = 0;

	//! Set the time for the specified running animation to a value in the range [0..1].
	//! When entireClip is true, set the animation normalized time.
	//! When entireClip is false, set the current segment normalized time.
	virtual void SetAnimationNormalizedTime(CAnimation* pAnimation, f32 normalizedTime, bool entireClip = true) = 0;

	//! Get the animation normalized time for the specified running animation. The return value is in the range [0..1].
	virtual f32  GetAnimationNormalizedTime(const CAnimation* pAnimation) const = 0;

	virtual void SetLayerNormalizedTime(u32 layer, f32 normalizedTime) = 0;
	virtual f32  GetLayerNormalizedTime(u32 layer) const = 0;

	//! Calculates duration of blend space up to the point where it starts to
	//! repeat, that is a Least Common Multiple of a number of segments of
	//! different examples. For instance, if segments of caf1: A B C, caf2: 1 2.
	//! The whole duration will be calculated for sequence A1 B2 C1 A2 B1 C2.
	virtual f32                            CalculateCompleteBlendSpaceDuration(const CAnimation& rAnimation) const = 0;

	virtual Vec3                           GetCurrentVelocity() const = 0;

	virtual void                           SetEventCallback(CallBackFuncType func, uk pdata) = 0;
	virtual AnimEventInstance              GetLastAnimEvent() = 0;

	virtual const QuatT&                   GetRelMovement() const = 0;

	virtual f32                            GetUserData(i32 i) const = 0;

	//! Pushes a pose modifier into the specified layer, ensuring that it will be executed next frame
	virtual bool                           PushPoseModifier(u32 layer, IAnimationPoseModifierPtr poseModifier, tukk name = NULL) = 0;

	virtual IAnimationPoseModifierSetupPtr      GetPoseModifierSetup() = 0;
	virtual IAnimationPoseModifierSetupConstPtr GetPoseModifierSetup() const = 0;

	//! This function will move outside of this interface. Use at your own risk.
	virtual QuatT CalculateRelativeMovement(const float deltaTime, const bool CurrNext = 0) const = 0;

#ifdef EDITOR_PCDEBUGCODE
	virtual bool ExportHTRAndICAF(tukk szAnimationName, tukk saveDirectory) const = 0;
	virtual bool ExportVGrid(tukk szAnimationName) const = 0;
#endif
	// </interfuscator:shuffle>
};

struct IAnimationPoseBlenderDir;

//! Interface for maintaining the physical state of a character's skeleton, for example to generate physical parts for each joint
struct ISkeletonPhysics
{
	// <interfuscator:shuffle>
	virtual ~ISkeletonPhysics() {}

	virtual void                     BuildPhysicalEntity(IPhysicalEntity* pent, f32 mass, i32 surface_idx, f32 stiffness_scale = 1.0f, i32 nLod = 0, i32 partid0 = -1, const Matrix34& mtxloc = Matrix34(IDENTITY)) = 0;
	virtual IPhysicalEntity*         CreateCharacterPhysics(IPhysicalEntity* pHost, f32 mass, i32 surface_idx, f32 stiffness_scale, i32 nLod = 0, const Matrix34& mtxloc = Matrix34(IDENTITY)) = 0;
	virtual i32                      CreateAuxilaryPhysics(IPhysicalEntity* pHost, const Matrix34& mtx, i32 nLod = 0) = 0;
	virtual IPhysicalEntity*         GetCharacterPhysics() const = 0;
	virtual IPhysicalEntity*         GetCharacterPhysics(tukk pRootBoneName) const = 0;
	virtual IPhysicalEntity*         GetCharacterPhysics(i32 iAuxPhys) const = 0;
	virtual void                     SetCharacterPhysics(IPhysicalEntity* pent) = 0;
	virtual void                     SynchronizeWithPhysicalEntity(IPhysicalEntity* pent, const Vec3& posMaster = Vec3(ZERO), const Quat& qMaster = Quat(1, 0, 0, 0)) = 0;
	virtual IPhysicalEntity*         RelinquishCharacterPhysics(const Matrix34& mtx, f32 stiffness = 0.0f, bool bCopyJointVelocities = false, const Vec3& velHost = Vec3(ZERO)) = 0;
	virtual void                     DestroyCharacterPhysics(i32 iMode = 0) = 0;
	virtual bool                     AddImpact(i32 partid, Vec3 point, Vec3 impact) = 0;
	virtual i32                      TranslatePartIdToDeadBody(i32 partid) = 0;
	virtual i32                      GetAuxPhysicsBoneId(i32 iAuxPhys, i32 iBone = 0) const = 0;

	virtual bool                     BlendFromRagdoll(QuatTS& location, IPhysicalEntity*& pPhysicalEntity, bool b3dof) = 0;

	virtual i32                      GetFallingDir() const = 0;

	virtual i32                      getBonePhysParentOrSelfIndex(i32 nBoneIndex, i32 nLod = 0) const = 0;

	virtual i32                      GetBoneSurfaceTypeId(i32 nBoneIndex, i32 nLod = 0) const = 0;

	virtual IPhysicalEntity*         GetPhysEntOnJoint(i32 nId) = 0;
	virtual const IPhysicalEntity*   GetPhysEntOnJoint(i32 nId) const = 0;

	virtual void                     SetPhysEntOnJoint(i32 nId, IPhysicalEntity* pPhysEnt) = 0;
	virtual i32                      GetPhysIdOnJoint(i32 nId) const = 0;
	virtual DynArray<SJointProperty> GetJointPhysProperties_ROPE(u32 jointIndex, i32 nLod) const = 0;
	virtual bool                     SetJointPhysProperties_ROPE(u32 jointIndex, i32 nLod, const DynArray<SJointProperty>& props) = 0;
	// </interfuscator:shuffle>
};

//! Represents the current pose of a character instance, allowing retrieval of the latest animation pose.
struct ISkeletonPose : public ISkeletonPhysics
{
	static i32k kForceSkeletonUpdatesInfinitely = 0x8000;

	// <interfuscator:shuffle>
	virtual ~ISkeletonPose() {}

	/**
	 * Retrieves location of the specified joint in model-space.
	 * The location is in the runtime pose, not the default pose. To get the location in the default pose, retrieve it from ICharacterInstance::GetIDefaultSkeleton.
	 *
	 * This function is safe to call while the animation job is running; if you do so it will simply return the previous frame's data.
	 * New data is returned only after the job has been synchronized (ICharacterInstance::FinishAnimationComputations has been called, or we are in the PostProcess callback).
	 *
	 * @param nJointID. Behavior is undefined when the nJointID is invalid.
	 * @return Model-space location of the specified joint.
	 * @see ISkeletonPose::SetPostProcessCallback
	 * @see ISkeletonPose::GetRelJointByID
	 * @see ICharacterInstance::FinishAnimationComputations
	 * @par Example
	 * @include DinrusXAnimation/Examples/GetJointOrientation.cpp
	 */
	virtual const QuatT& GetAbsJointByID(i32 nJointID) const = 0;

	/**
	 * Retrieves location of the specified joint in parent-joint space.
	 * The location is in the runtime pose, not the default pose. To get the location in the default pose, retrieve it from ICharacterInstance::GetIDefaultSkeleton.
	 *
	 * This function is safe to call while the animation job is running; if you do so it will simply return the previous frame's data.
	 * New data is returned only after the job has been synchronized (ICharacterInstance::FinishAnimationComputations has been called, or we are in the PostProcess callback).
	 *
	 * @param nJointID. Behavior is undefined when the nJointID is invalid.
	 * @return Parent-joint space location of the specified joint.
	 * @see ISkeletonPose::SetPostProcessCallback
	 * @see ISkeletonPose::GetAbsJointByID
	 * @see ICharacterInstance::FinishAnimationComputations
	 */
	virtual const QuatT& GetRelJointByID(i32 nJointID) const = 0;

	virtual Diag33       GetAbsJointScalingByID(i32 jointId) const = 0;
	virtual Diag33       GetRelJointScalingByID(i32 jointId) const = 0;

	virtual void         SetPostProcessCallback(i32 (* func)(ICharacterInstance*, uk ), uk pdata) = 0;

	/**
	 * Force updates of the skeleton pose (even when invisible).
	 * Note that this is not recommended. If possible use CA_FORCE_SKELETON_UPDATE when starting
	 * animations that need an update.
	 *
	 * @param numFrames Number of frames to force the update. Use ISkeletonPose::kForceSkeletonUpdatesInfinitely to force it on all the time.
	 * @see CA_FORCE_SKELETON_UPDATE
	 */
	virtual void             SetForceSkeletonUpdate(i32 numFrames) = 0;

	virtual void             SetDefaultPose() = 0;
	virtual void             SetStatObjOnJoint(i32 nId, IStatObj* pStatObj) = 0;
	virtual IStatObj*        GetStatObjOnJoint(i32 nId) = 0;
	virtual const IStatObj*  GetStatObjOnJoint(i32 nId) const = 0;
	virtual void             SetMaterialOnJoint(i32 nId, IMaterial* pMaterial) = 0;
	virtual IMaterial*       GetMaterialOnJoint(i32 nId) = 0;
	virtual const IMaterial* GetMaterialOnJoint(i32 nId) const = 0;
	virtual void             DrawSkeleton(const Matrix34& rRenderMat34, u32 shift = 0) = 0;

	// -------------------------------------------------------------------------
	// Pose Modifiers (soon obsolete)
	// -------------------------------------------------------------------------
	//! Gets the pose modifier used to target gun aiming at a specific world coordinate
	//! \par Example (Aim-IK)
	//! \include DinrusXAnimation/Examples/AimIK.cpp
	virtual IAnimationPoseBlenderDir*       GetIPoseBlenderAim() = 0;
	//! Gets the pose modifier used to target gun aiming at a specific world coordinate
	//! \par Example (Aim-IK)
	//! \include DinrusXAnimation/Examples/AimIK.cpp
	virtual const IAnimationPoseBlenderDir* GetIPoseBlenderAim() const = 0;
	//! Gets the pose modifier used to target look aim at a specific world coordinate
	//! \par Example (Look-IK)
	//! \include DinrusXAnimation/Examples/LookIK.cpp
	virtual IAnimationPoseBlenderDir*       GetIPoseBlenderLook() = 0;
	//! Gets the pose modifier used to target look aim at a specific world coordinate
	//! \par Example (Look-IK)
	//! \include DinrusXAnimation/Examples/LookIK.cpp
	virtual const IAnimationPoseBlenderDir* GetIPoseBlenderLook() const = 0;
	virtual void                            ApplyRecoilAnimation(f32 fDuration, f32 fKinematicImpact, f32 fKickIn, u32 arms = 3) = 0;
	virtual u32                          SetHumanLimbIK(const Vec3& wgoal, tukk limb) = 0;

	// </interfuscator:shuffle>
};

//! \cond INTERNAL
//! Holds description of a set of animations.
//! This interface holds a set of animations in which each animation is described as properties.
struct IAnimationSet
{
	// <interfuscator:shuffle>
	virtual ~IAnimationSet() {}

	//! Increase reference count of the interface.
	virtual void AddRef() = 0;

	//! Decrease reference count of the interface.
	virtual void Release() = 0;

	//! Retrieves the amount of animations.
	//! \return An integer holding the amount of animations.
	virtual u32 GetAnimationCount() const = 0;

	//! Searches for the index of an animation using its name.
	//! Returns the index of the animation in the set, -1 if there's no such animation
	//! \param szAnimationName - Null terminated string holding the name of the animation.
	//! \return An integer representing the index of the animation. In case the animation couldn't be found, -1 will be returned.
	virtual i32 GetAnimIDByName(tukk szAnimationName) const = 0;

	//! Gets the name of the specified animation.
	//! \param nAnimationId - Id of an animation.
	//! \return A null terminated string holding the name of the animation, or "!NEGATIVE ANIMATION ID!" if not found.
	virtual tukk GetNameByAnimID(i32 nAnimationId) const = 0;

	virtual i32         GetAnimIDByCRC(u32 animationCRC) const = 0;
	virtual u32      GetCRCByAnimID(i32 nAnimationId) const = 0;
	virtual u32      GetFilePathCRCByAnimID(i32 nAnimationId) const = 0;
	virtual tukk GetFilePathByName(tukk szAnimationName) const = 0;
	virtual tukk GetFilePathByID(i32 nAnimationId) const = 0;

	//! Returns the duration of the animation.
	//! \return Duration of the animation, or 0.0f when the id/referenced animation is invalid.
	//! \note Never returns a negative value.
	virtual f32    GetDuration_sec(i32 nAnimationId) const = 0;

	virtual u32 GetAnimationFlags(i32 nAnimationId) const = 0;
	virtual u32 GetAnimationSize(u32k nAnimationId) const = 0;
	virtual bool   IsAnimLoaded(i32 nAnimationId) const = 0;
	virtual bool   IsAimPose(i32 nAnimationId, const IDefaultSkeleton& defaultSkeleton) const = 0;
	virtual bool   IsLookPose(i32 nAnimationId, const IDefaultSkeleton& defaultSkeleton) const = 0;

	virtual void   AddRef(i32k nAnimationId) const = 0;
	virtual void   Release(i32k nAnimationId) const = 0;

	//! Retrieve the 'DCC world space' location of the first frame.
	//! \return true on success, false and IDENTITY when animation is invalid.
	virtual bool GetAnimationDCCWorldSpaceLocation(tukk szAnimationName, QuatT& startLocation) const = 0;

	//! Retrieve the 'DCC world space' location of the first frame.
	//! \return true on success, false and IDENTITY when animation is invalid.
	virtual bool GetAnimationDCCWorldSpaceLocation(i32 AnimID, QuatT& startLocation) const = 0;

	//! Retrieve the 'DCC world space' location of the current frame for a specific controller.
	//! \return true on success, false and IDENTITY when animation is invalid.
	virtual bool GetAnimationDCCWorldSpaceLocation(const CAnimation* pAnim, QuatT& startLocation, u32 nControllerID) const = 0;

	enum ESampleResult
	{
		eSR_Success,

		eSR_InvalidAnimationId,
		eSR_UnsupportedAssetType,
		eSR_NotInMemory,
		eSR_ControllerNotFound,
	};

	//! Sample the location of a controller at a specific time in a non-parametric animation.
	//! On success, returns eSR_Success and fills in the relativeLocationOutput parameter. The location uses relative (aka parent-local) coordinates.
	//! On failure, returns an error code and sets relativeLocationOutput to IDENTITY.
	virtual ESampleResult SampleAnimation(i32 animationId, float animationNormalizedTime, u32 controllerId, QuatT& relativeLocationOutput) const = 0;

#ifdef EDITOR_PCDEBUGCODE
	virtual void        GetSubAnimations(DynArray<i32>& animIdsOut, i32 animId) const = 0;
	virtual i32         GetNumFacialAnimations() const = 0;
	virtual tukk GetFacialAnimationPathByName(tukk szName) const = 0;                                                              //!< \return 0 if name not found.
	virtual tukk GetFacialAnimationName(i32 index) const = 0;                                                                             //!< \return 0 on invalid index.
	virtual i32       GetGlobalIDByName(tukk szAnimationName) const = 0;
	virtual i32       GetGlobalIDByAnimID(i32 nAnimationId) const = 0;
	virtual tukk GetAnimationStatus(i32 nAnimationId) const = 0;
	virtual u32      GetTotalPosKeys(u32k nAnimationId) const = 0;
	virtual u32      GetTotalRotKeys(u32k nAnimationId) const = 0;
	virtual tukk GetDBAFilePath(u32k nAnimationId) const = 0;
	virtual i32         AddAnimationByPath(tukk animationName, tukk animationPath, const IDefaultSkeleton* pIDefaultSkeleton) = 0;
	virtual void        RebuildAimHeader(tukk szAnimationName, const IDefaultSkeleton* pIDefaultSkeleton) = 0;
	virtual void        RegisterListener(IAnimationSetListener* pListener) = 0;
	virtual void        UnregisterListener(IAnimationSetListener* pListener) = 0;

	//! Queries motion parameters of specific animation examples. Used for blendspace editing.
	virtual bool GetMotionParameters(u32 nAnimationId, i32 parameterIndex, IDefaultSkeleton* pDefaultSkeleton, Vec4& outParameters) const = 0;
	virtual bool GetMotionParameterRange(u32 nAnimationId, EMotionParamID paramId, float& outMin, float& outMax) const = 0;
	virtual bool IsBlendSpace(i32 nAnimationId) const = 0;
	virtual bool IsCombinedBlendSpace(i32 nAnimationId) const = 0;
#endif

	// </interfuscator:shuffle>
};
//! \endcond

struct IAnimationSetListener
{
	IAnimationSet*    m_pIAnimationSet;
	IDefaultSkeleton* m_pIDefaultSkeleton;

	IAnimationSetListener() : m_pIAnimationSet(0), m_pIDefaultSkeleton(0) {}
	virtual ~IAnimationSetListener()
	{
#ifdef EDITOR_PCDEBUGCODE
		if (m_pIAnimationSet)
			m_pIAnimationSet->UnregisterListener(this);
#endif
	}

	virtual void OnAnimationSetAddAnimation(tukk animationPath, tukk animationName) {}
	virtual void OnAnimationSetAboutToBeReloaded()                                                {}
	virtual void OnAnimationSetReloaded()                                                         {}
};

#include <drx3D/Animation/IAttachment.h>

struct SAnimationStatistics
{
	tukk name;
	long        count;
};

struct IAnimEventList
{
	// <interfuscator:shuffle>
	virtual ~IAnimEventList() {}

	virtual u32                GetCount() const = 0;
	virtual const CAnimEventData& GetByIndex(u32 animEventIndex) const = 0;
	virtual CAnimEventData&       GetByIndex(u32 animEventIndex) = 0;
	virtual void                  Append(const CAnimEventData& animEvent) = 0;
	virtual void                  Remove(u32 animEventIndex) = 0;
	virtual void                  Clear() = 0;
	// </interfuscator:shuffle>
};

struct IAnimEvents
{
	// <interfuscator:shuffle>
	virtual ~IAnimEvents() {}

	virtual IAnimEventList*       GetAnimEventList(tukk animationFilePath) = 0;
	virtual const IAnimEventList* GetAnimEventList(tukk animationFilePath) const = 0;

	virtual bool                  SaveAnimEventToXml(const CAnimEventData& dataIn, XmlNodeRef& dataOut) = 0;
	virtual bool                  LoadAnimEventFromXml(const XmlNodeRef& dataIn, CAnimEventData& dataOut) = 0;

	virtual void                  InitializeSegmentationDataFromAnimEvents(tukk animationFilePath) = 0;

	virtual size_t                GetGlobalAnimCount() = 0;
	// </interfuscator:shuffle>
};

#ifdef __cplusplus
extern "C" {
#endif

//! Experimental way to track the interface version.
//! This value will be compared with value passed from system module
const char gAnimInterfaceVersion[64] = __TIMESTAMP__;

//! CreateDinrusXAnimation function type definition.
typedef ICharacterUpr* (* PFNCREATEDRXANIMATION)(ISystem* pSystem, tukk szInterfaceVersion);

//! Create an instance of the Animation System. Should usually be called by ISystem::InitAnimationSystem().
//! \param ISystem            Pointer to the current ISystem instance.
//! \param szInterfaceVersion String version of the build date.
//! \see ICharacterUpr, ICharacterUpr::Release
DRXANIMATION_API ICharacterUpr* CreateCharUpr(ISystem* pSystem, tukk szInterfaceVersion = gAnimInterfaceVersion);

#ifdef __cplusplus
}
#endif

#if defined(ENABLE_LW_PROFILERS)
class CAnimationLightProfileSection
{
public:
	CAnimationLightProfileSection()
		: m_nTicks(DrxGetTicks())
	{
	}

	~CAnimationLightProfileSection()
	{
		ICharacterUpr* pCharacterUpr = gEnv->pCharacterUpr;
		IF (pCharacterUpr != NULL, 1)
		{
			pCharacterUpr->AddFrameTicks(DrxGetTicks() - m_nTicks);
		}
	}
private:
	uint64 m_nTicks;
};

class CAnimationLightSyncProfileSection
{
public:
	CAnimationLightSyncProfileSection()
		: m_nTicks(DrxGetTicks())
	{}
	~CAnimationLightSyncProfileSection()
	{
		ICharacterUpr* pCharacterUpr = gEnv->pCharacterUpr;
		IF (pCharacterUpr != NULL, 1)
		{
			pCharacterUpr->AddFrameSyncTicks(DrxGetTicks() - m_nTicks);
		}
	}
private:
	uint64 m_nTicks;
};

	#define ANIMATION_LIGHT_PROFILER()      CAnimationLightProfileSection _animationLightProfileSection;
	#define ANIMATION_LIGHT_SYNC_PROFILER() CAnimationLightSyncProfileSection _animationLightSyncProfileSection;
#else
	#define ANIMATION_LIGHT_PROFILER()
	#define ANIMATION_LIGHT_SYNC_PROFILER()
#endif

//! \cond INTERNAL
//! Utility class to automatically start loading & lock a CAF file.
//! Either it is 'empty' or it holds a reference to a CAF file.
//! It asserts gEnv->pCharacterUpr exists when it isn't empty.
//! \see ICharacterUpr
struct CAutoResourceCache_CAF
{
	//! \param filePathCRC Either 0 (in which case this class refers to 'nothing') or a valid CAF filePathCRC
	explicit CAutoResourceCache_CAF(u32k filePathCRC = 0)
		: m_filePathCRC(filePathCRC)
	{
		SafeAddRef();
	}

	CAutoResourceCache_CAF(const CAutoResourceCache_CAF& rhs)
		: m_filePathCRC(rhs.m_filePathCRC)
	{
		SafeAddRef();
	}

	~CAutoResourceCache_CAF()
	{
		SafeRelease();
	}

	CAutoResourceCache_CAF& operator=(const CAutoResourceCache_CAF& anim)
	{
		if (m_filePathCRC != anim.m_filePathCRC)
		{
			SafeRelease();
			m_filePathCRC = anim.m_filePathCRC;
			SafeAddRef();
		}
		return *this;
	}

	bool IsLoaded() const
	{
		if (m_filePathCRC)
		{
			assert(gEnv->pCharacterUpr);
			PREFAST_ASSUME(gEnv->pCharacterUpr);
			return gEnv->pCharacterUpr->CAF_IsLoaded(m_filePathCRC);
		}
		else
		{
			return false;
		}
	}

	u32 GetFilePathCRC() const
	{
		return m_filePathCRC;
	}

	void Serialize(TSerialize ser)
	{
		ser.BeginGroup("AutoResourceCache_CAF");
		ser.Value("m_filePathCRC", m_filePathCRC);
		ser.EndGroup();
		if (ser.IsReading())
		{
			SafeAddRef();
		}
	}

private:
	void SafeAddRef() const
	{
		if (m_filePathCRC)
		{
			assert(gEnv->pCharacterUpr);
			PREFAST_ASSUME(gEnv->pCharacterUpr);
			gEnv->pCharacterUpr->CAF_AddRef(m_filePathCRC);
		}
	}

	void SafeRelease() const
	{
		if (m_filePathCRC)
		{
			assert(gEnv->pCharacterUpr);
			PREFAST_ASSUME(gEnv->pCharacterUpr);
			gEnv->pCharacterUpr->CAF_Release(m_filePathCRC);
		}
	}

	u32 m_filePathCRC;
};
//! \endcond

