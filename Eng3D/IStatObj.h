// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:IStatObj.h
//  Interface for CStatObj class
//
//	История:
//	-:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#ifndef _IStatObj_H_
#define _IStatObj_H_

#include <drx3D/CoreX/smartptr.h>     // TYPEDEF_AUTOPTR

struct    IMaterial;
struct    ShadowMapFrustum;
class CRenderObject;
struct    IShader;
class IReadStream;
struct    SRenderingPassInfo;
TYPEDEF_AUTOPTR(IReadStream);

// Interface to non animated object.
struct phys_geometry;
struct IChunkFile;

// General forward declaration.
class CRenderObject;
struct SMeshLodInfo;

#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include "IMeshObj.h"
#include <drx3D/Phys/IPhysics.h>
#include <drx3D/CoreX/BitMask.h>

//! Type of static sub object.
enum EStaticSubObjectType
{
	STATIC_SUB_OBJECT_MESH,         //!< This simple geometry part of the multi-sub object geometry.
	STATIC_SUB_OBJECT_HELPER_MESH,  //!< Special helper mesh, not rendered usually, used for broken pieces.
	STATIC_SUB_OBJECT_POINT,
	STATIC_SUB_OBJECT_DUMMY,
	STATIC_SUB_OBJECT_XREF,
	STATIC_SUB_OBJECT_CAMERA,
	STATIC_SUB_OBJECT_LIGHT,
};

//! Flags that can be set on static object.
enum EStaticObjectFlags
{
	STATIC_OBJECT_HIDDEN           = BIT(0),  //!< When set static object will not be displayed.
	STATIC_OBJECT_CLONE            = BIT(1),  //!< specifies whether this object was cloned for modification.
	STATIC_OBJECT_GENERATED        = BIT(2),  //!< tells that the object was generated procedurally (breakable obj., f.i.).
	STATIC_OBJECT_CANT_BREAK       = BIT(3),  //!< StatObj has geometry unsuitable for procedural breaking.
	STATIC_OBJECT_DEFORMABLE       = BIT(4),  //!< StatObj can be procedurally smeared (using SmearStatObj).
	STATIC_OBJECT_COMPOUND         = BIT(5),  //!< StatObj has subobject meshes.
	STATIC_OBJECT_MULTIPLE_PARENTS = BIT(6),  //!< Child StatObj referenced by several parents.

	// Collisions
	STATIC_OBJECT_NO_PLAYER_COLLIDE = BIT(10),

	// Special flags.
	STATIC_OBJECT_SPAWN_ENTITY       = BIT(20),  //!< StatObj spawns entity when broken.
	STATIC_OBJECT_PICKABLE           = BIT(21),  //!< StatObj can be picked by players.
	STATIC_OBJECT_NO_AUTO_HIDEPOINTS = BIT(22),  //!< Do not generate AI auto hide points around object if it's dynamic.
	STATIC_OBJECT_DYNAMIC            = BIT(23),  //!< mesh data should be kept in system memory (and yes, the name *is* an oxymoron).

};

#define HIT_NO_HIT           (-1)
#define HIT_UNKNOWN          (-2)

#define HIT_OBJ_TYPE_BRUSH   0
#define HIT_OBJ_TYPE_TERRAIN 1
#define HIT_OBJ_TYPE_VISAREA 2

//! \cond INTERNAL
//! Used for on-CPU voxelization.
struct SRayHitTriangle
{
	SRayHitTriangle() { ZeroStruct(*this); }
	Vec3       v[3];
	Vec2       t[3];
	ColorB     c[3];
	Vec3       n;
	IMaterial* pMat;
	u8      nTriArea;
	u8      nOpacity;
	u8      nHitObjType;
};
//! \endcond

struct SRayHitInfo
{
	SRayHitInfo()
	{
		memset(this, 0, sizeof(*this));
		nHitTriID = HIT_UNKNOWN;
	}
	// Input parameters.
	Vec3  inReferencePoint;
	Ray   inRay;
	bool  bInFirstHit;
	bool  inRetTriangle;
	bool  bUseCache;
	bool  bOnlyZWrite;
	bool  bGetVertColorAndTC;
	float fMaxHitDistance;   //!< When not 0, only hits with closer distance will be registered.
	Vec3  vTri0;
	Vec3  vTri1;
	Vec3  vTri2;
	float fMinHitOpacity;

	// Output parameters.
	float                      fDistance; //!< Distance from reference point.
	Vec3                       vHitPos;
	Vec3                       vHitNormal;
	i32                        nHitMatID;     //!< Material Id that was hit.
	i32                        nHitTriID;     //!< Triangle Id that was hit.
	i32                        nHitSurfaceID; //!< Material Id that was hit.
	struct IRenderMesh*        pRenderMesh;
	struct IStatObj*           pStatObj;
	Vec2                       vHitTC;
	Vec4                       vHitColor;
	Vec4                       vHitTangent;
	Vec4                       vHitBitangent;
	PodArray<SRayHitTriangle>* pHitTris;
};

enum EFileStreamingStatus
{
	ecss_NotLoaded,
	ecss_InProgress,
	ecss_Ready
};

//! Interface for streaming of objects like CStatObj.
struct IStreamable
{
	struct SInstancePriorityInfo
	{
		i32   nRoundId;
		float fMaxImportance;
	};

	IStreamable()
	{
		ZeroStruct(m_arrUpdateStreamingPrioriryRoundInfo);
		m_eStreamingStatus = ecss_NotLoaded;
		fCurImportance = 0;
		m_nSelectedFrameId = 0;
		m_nStatsInUse = 0;
	}

	bool UpdateStreamingPrioriryLowLevel(float fImportance, i32 nRoundId, bool bFullUpdate)
	{
		bool bRegister = false;

		if (m_arrUpdateStreamingPrioriryRoundInfo[0].nRoundId != nRoundId)
		{
			if (!m_arrUpdateStreamingPrioriryRoundInfo[0].nRoundId)
				bRegister = true;

			m_arrUpdateStreamingPrioriryRoundInfo[1] = m_arrUpdateStreamingPrioriryRoundInfo[0];

			m_arrUpdateStreamingPrioriryRoundInfo[0].nRoundId = nRoundId;
			m_arrUpdateStreamingPrioriryRoundInfo[0].fMaxImportance = fImportance;
		}
		else
		{
			m_arrUpdateStreamingPrioriryRoundInfo[0].fMaxImportance = max(m_arrUpdateStreamingPrioriryRoundInfo[0].fMaxImportance, fImportance);
		}

		if (bFullUpdate)
		{
			m_arrUpdateStreamingPrioriryRoundInfo[1] = m_arrUpdateStreamingPrioriryRoundInfo[0];
			m_arrUpdateStreamingPrioriryRoundInfo[1].nRoundId--;
		}

		return bRegister;
	}

	// <interfuscator:shuffle>
	virtual ~IStreamable(){}
	virtual void   StartStreaming(bool bFinishNow, IReadStream_AutoPtr* ppStream) = 0;
	virtual i32    GetStreamableContentMemoryUsage(bool bJustForDebug = false) = 0;
	virtual void   ReleaseStreamableContent() = 0;
	virtual void   GetStreamableName(string& sName) = 0;
	virtual u32 GetLastDrawMainFrameId() = 0;
	virtual bool   IsUnloadable() const = 0;
	// </interfuscator:shuffle>

	SInstancePriorityInfo m_arrUpdateStreamingPrioriryRoundInfo[2];
	float                 fCurImportance;
	EFileStreamingStatus  m_eStreamingStatus;
	u32                m_nSelectedFrameId : 31;
	u32                m_nStatsInUse      : 1;
};

//! Represents a static object that can be rendered in the scene, represented by the .CGF format
struct IStatObj : IMeshObj, IStreamable
{
	//! Loading flags.
	enum ELoadingFlags
	{
		ELoadingFlagsPreviewMode    = BIT(0),
		ELoadingFlagsForceBreakable = BIT(1),
		ELoadingFlagsIgnoreLoDs     = BIT(2),
		ELoadingFlagsTessellate     = BIT(3), //!< If e_StatObjTessellation enabled.
		ELoadingFlagsJustGeometry   = BIT(4), //!< For streaming, to avoid parsing all chunks.
		ELoadingFlagsNoErrorIfFail  = BIT(5), //!< Don't log error message if the file is not found
	};

	struct SSubObject
	{
		SSubObject() { bShadowProxy = 0; }

		EStaticSubObjectType nType;
		string               name;
		string               properties;
		i32                  nParent;             //!< Index of the parent sub object, if there`s hierarchy between them.
		Matrix34             tm;                  //!< Transformation matrix.
		Matrix34             localTM;             //!< Local transformation matrix, relative to parent.
		IStatObj*            pStatObj;            //!< Static object for sub part of CGF.
		Vec3                 helperSize;          //!< Size of the helper (if helper).
		struct IRenderMesh*  pWeights;            //!< render mesh with a single deformation weights stream.
		struct IFoliage*     pFoliage;            //!< for bendable foliage.
		u32         bIdentityMatrix : 1; //!< True if sub object matrix is identity.
		u32         bHidden         : 1; //!< True if sub object is hidden.
		u32         bShadowProxy    : 1; //!< Child StatObj has 'shadowproxy' in name.
		u32         nBreakerJoints  : 8; //!< number of joints that can switch this part to a broken state.

		void                 GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
			pSizer->AddObject(properties);
		}
	};
	//////////////////////////////////////////////////////////////////////////

	//! Statistics information about this object.
	struct SStatistics
	{
		i32  nVertices;
		i32  nVerticesPerLod[MAX_STATOBJ_LODS_NUM];
		i32  nIndices;
		i32  nIndicesPerLod[MAX_STATOBJ_LODS_NUM];
		i32  nMeshSize;
		i32  nMeshSizeLoaded;
		i32  nPhysProxySize;
		i32  nPhysProxySizeMax;
		i32  nPhysPrimitives;
		i32  nDrawCalls;
		i32  nLods;
		i32  nSubMeshCount;
		i32  nNumRefs;
		bool bSplitLods; //!< Lods split between files.

		//! Optional texture sizer.
		IDrxSizer* pTextureSizer;
		IDrxSizer* pTextureSizer2;

		SStatistics() { Reset(); }

		void Reset()
		{
			pTextureSizer = NULL;
			pTextureSizer2 = NULL;
			nVertices = 0;
			nIndices = 0;
			nMeshSize = 0;
			nMeshSizeLoaded = 0;
			nNumRefs = 0;
			nPhysProxySize = 0;
			nPhysPrimitives = 0;
			nDrawCalls = 0;
			nLods = 0;
			nSubMeshCount = 0;
			bSplitLods = false;
			ZeroStruct(nVerticesPerLod);
			ZeroStruct(nIndicesPerLod);
		}
	};

	// <interfuscator:shuffle>
	//! Set static object flags.
	//! \param nFlags Flags to set, a combination of EStaticObjectFlags values.
	virtual void SetFlags(i32 nFlags) = 0;

	//! Retrieve flags set on the static object.
	virtual i32 GetFlags() const = 0;

	//! Retrieves the internal flag m_nVehicleOnlyPhysics.
	virtual u32 GetVehicleOnlyPhysics() = 0;

	//! Retrieves the internal flag m_nIdMaterialBreakable.
	virtual i32 GetIDMatBreakable() = 0;

	//! Retrieves the internal flag m_bBreakableByGame.
	virtual u32 GetBreakableByGame() = 0;

	//! Get the object source geometry.
	//! Provide access to the faces, vertices, texture coordinates, normals and
	//! colors of the object used later for CRenderMesh construction.
	virtual struct IIndexedMesh* GetIndexedMesh(bool bCreateIfNone = false) = 0;

	//! Create an empty indexed mesh ready to be filled with data.
	//! If an indexed mesh already exists it is returned
	//! \return An empty indexed mesh or the existing indexed mesh
	virtual struct IIndexedMesh* CreateIndexedMesh() = 0;

	//! Physicalizes StatObj's foliage
	//! Creates a skinnable object instance for foliage simulation
	//! \param pTrunk Existing physical entity of the trunk (foliage gets attached to it).
	//! \param mtxWorld Its world matrix.
	//! \param pRes Destination pointer (if the object is deleted after timeouting, it will write 0 there).
	//! \param lifeTime Idle time after which the object gets deleted.
	//! \return Number of physicalized leaves
	virtual i32 PhysicalizeFoliage(IPhysicalEntity* pTrunk, const Matrix34& mtxWorld, struct IFoliage*& pRes, float lifeTime = 0.0f, i32 iSource = 0) = 0;

	//! Updates vertices in the range [iVtx0..iVtx0+nVtx-1], vertices are in their original order (as they are physicalized).
	//! Clones the object if necessary to make the modifications
	//! \return modified IStatObj (a clone or this one, if it's already a clone)
	virtual IStatObj* UpdateVertices(strided_pointer<Vec3> pVtx, strided_pointer<Vec3> pNormals, i32 iVtx0, i32 nVtx, i32* pVtxMap = 0, float rscale = 1.f) = 0;

	//! Skins vertices based on skeleton vertices (mtxSkelToMesh[pSkelVtx[i]]).
	//! Clones the object if necessary to make the modifications
	//! \return modified IStatObj (a clone or this one, if it's already a clone)
	virtual IStatObj* SkinVertices(strided_pointer<Vec3> pSkelVtx, const Matrix34& mtxSkelToMesh) = 0;

	//! Copies foliage data to another statobj
	//! \param pObjDst Target stat obj.
	//! \param bMove true if the data needs to be removed from the original statobj.
	virtual void CopyFoliageData(IStatObj* pObjDst, bool bMove = false, IFoliage* pSrcFoliage = 0, i32* pVtxMap = 0, primitives::box* pMovedBoxes = 0, i32 nMovedBoxes = -1) = 0;

	//! Set the physics representation.
	//! Sets and replaces the physical representation of the object.
	//! \param pPhysGeom Pointer to a phys_geometry class.
	//! \param nType Pass 0 to set the physic geometry or pass 1 to set the obstruct geometry.
	virtual void SetPhysGeom(phys_geometry* pPhysGeom, i32 nType = 0) = 0;

	//! Returns a tetrahedral lattice, if any (used for breakable objects)
	virtual ITetrLattice* GetTetrLattice() = 0;

	virtual float         GetAIVegetationRadius() const = 0;
	virtual void          SetAIVegetationRadius(float radius) = 0;

	//! Set default material for the geometry.
	//! \param pMaterial Valid pointer to the material.
	virtual void SetMaterial(IMaterial* pMaterial) = 0;

	ILINE Vec3   GetBoxMin() const { return GetAABB().min; }
	ILINE Vec3   GetBoxMax() const { return GetAABB().max; }

	//! Get the center of bounding box.
	//! \return Vec3 object containing the bounding box center.
	virtual const Vec3 GetVegCenter() = 0;

	//! Set the minimum bounding box component.
	//! \param vBBoxMin Minimum bounding box component.
	virtual void SetBBoxMin(const Vec3& vBBoxMin) = 0;

	//! Set the minimum bounding box component.
	//! \param vBBoxMax Minimum bounding box component.
	virtual void SetBBoxMax(const Vec3& vBBoxMax) = 0;

	//! Reloads one or more component of the object.
	//! The possible flags are FRO_SHADERS, FRO_TEXTURES and FRO_GEOMETRY.
	//! \param nFlags One or more flag which indicate which element of the object to reload.
	virtual void Refresh(i32 nFlags) = 0;

	//! Get the LOD object, if present.
	//! \param nLodLevel Level of the LOD.
	//! \param bReturnNearest If true will return nearest available LOD to nLodLevel.
	//! \return Static object with the desired LOD. The value NULL will be return if there isn't any LOD object for the level requested.
	virtual IStatObj* GetLodObject(i32 nLodLevel, bool bReturnNearest = false) = 0;
	virtual void      SetLodObject(i32 nLodLevel, IStatObj* pLod) = 0;
	virtual IStatObj* GetLowestLod() = 0;
	virtual i32       FindNearestLoadedLOD(i32 nLodIn, bool bSearchUp = false) = 0;
	virtual i32       FindHighestLOD(i32 nBias) = 0;

	//! Returns the filename of the object.
	//! \return Null-terminated string which contain the filename of the object.
	virtual tukk GetFilePath() = 0;

	//! Set the filename of the object.
	//! \param szFileName New filename of the object.
	virtual void SetFilePath(tukk szFileName) = 0;

	//! Returns the name of the geometry.
	//! \return Null terminated string which contains the name of the geometry
	virtual tukk GetGeoName() = 0;

	//! Sets the name of the geometry.
	virtual void SetGeoName(tukk szGeoName) = 0;

	//! Compares if another object is the same.
	//! \param szFileName Filename of the object to compare.
	//! \param szGeomName Geometry name of the object to compare (optional).
	//! \return true if both object are the same, false otherwise.
	virtual bool IsSameObject(tukk szFileName, tukk szGeomName) = 0;

	//! Gets the position of a specified helper.
	//! Will return the position of the helper named in the argument. The helper should
	//! have been specified during the exporting process of the cgf file.
	//! \param szHelperName A null terminated string holding the name of the helper.
	//! \return A Vec3 object which contains the position.
	virtual Vec3 GetHelperPos(tukk szHelperName) = 0;

	//! Gets the transformation matrix of a specified helper, see GetHelperPos.
	virtual const Matrix34& GetHelperTM(tukk szHelperName) = 0;

	//! Tell us if the object is not found.
	virtual bool IsDefaultObject() = 0;

	//! Free the geometry data.
	virtual void FreeIndexedMesh() = 0;

	//! Pushes the underlying tree of objects into the given Sizer object for statistics gathering.
	virtual void GetMemoryUsage(class IDrxSizer* pSizer) const = 0;

	//! Used for sprites.
	virtual float GetRadiusVert() const = 0;

	//! Used for sprites.
	virtual float GetRadiusHors() const = 0;

	//! Determines if the object has physics capabilities.
	virtual bool IsPhysicsExist() const = 0;

	//! Return a pointer to the object.
	//! \return Pointer to the current object, which is simply done like this "return this;"
	virtual struct IStatObj* GetIStatObj() { return this; }

	//! Invalidate geometry inside IStatObj, will mark hosted IIndexedMesh as invalid.
	//! \param bPhysics If true will also recreate physics for indexed mesh.
	virtual void Invalidate(bool bPhysics = false, float tolerance = 0.05f) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Interface to the Sub Objects.
	//! Retrieve number of sub-objects.
	virtual i32 GetSubObjectCount() const = 0;

	//! Sets number of sub-objects.
	virtual void SetSubObjectCount(i32 nCount) = 0;

	//! Retrieve sub object by index, where 0 <= nIndex < GetSubObjectCount()
	virtual IStatObj::SSubObject* GetSubObject(i32 nIndex) = 0;

	//! Check if this object is sub object of another IStatObj.
	virtual bool IsSubObject() const = 0;

	//! Retrieve parent static object, only relevant when this IStatObj is Sub-object.
	virtual IStatObj* GetParentObject() const = 0;

	//! Retrieve the static object, from which this one was cloned (if that is the case)
	virtual IStatObj* GetCloneSourceObject() const = 0;

	//! Find sub-pbject by name.
	virtual IStatObj::SSubObject* FindSubObject(tukk sNodeName) = 0;

	//! Find sub-object by name (including spaces, comma and semi-colon.
	virtual IStatObj::SSubObject* FindSubObject_CGA(tukk sNodeName) = 0;

	//! Find object by full name (use all the characters)
	virtual IStatObj::SSubObject* FindSubObject_StrStr(tukk sNodeName) = 0;

	//! Remove Sub-Object.
	virtual bool RemoveSubObject(i32 nIndex) = 0;

	//! Copy Sub-Object.
	virtual bool CopySubObject(i32 nToIndex, IStatObj* pFromObj, i32 nFromIndex) = 0;

	//! Adds a new sub object.
	virtual IStatObj::SSubObject& AddSubObject(IStatObj* pStatObj) = 0;

	//! Adds subobjects to pent, meshes as parts, joint helpers as breakable joints
	virtual i32 PhysicalizeSubobjects(IPhysicalEntity* pent, const Matrix34* pMtx, float mass, float density = 0.0f, i32 id0 = 0, strided_pointer<i32> pJointsIdMap = 0, tukk szPropsOverride = 0, i32 idbodyArtic = -1) = 0;

	//! Adds all phys geometries to pent, assigns ids starting from id; takes mass and density from the StatObj properties if not set in pgp.
	//! id == -1 means that compound objects will use the 0th level in the id space (i.e. slot#==phys id), and simple objects will let the physics allocate an id
	//! for compound objects calls PhysicalizeSubobjects
	//! if >=0, idbodyArtic sets it for all parts as idbody, otherwise idbody is set to each node's phys part id
	//! \return Physical id of the last physicalized part
	virtual i32  Physicalize(IPhysicalEntity* pent, pe_geomparams* pgp, i32 id = -1, tukk szPropsOverride = 0) = 0;

	virtual bool IsDeformable() = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Save contents of static object to the CGF file.
	//! Save object to the CGF file.
	//! \param sFilename \par
	//!     Filename of the CGF file.
	//!     The function fails if pOutChunkFile is NULL and the path to the file does not exist on the drive.
	//!     You can call CFileUtil::CreatePath() before SaveToCGF() call to create all folders that do not exist yet.
	//! \param pOutChunkFile \par
	//!     Optional output parameter. If it is specified then the file will not be written to the drive but instead
	//!     the function returns a pointer to the IChunkFile interface with filled CGF chunks. Caller of the function
	//!     is responsible to call Release method of IChunkFile to release it later.
	virtual bool SaveToCGF(tukk sFilename, IChunkFile** pOutChunkFile = NULL, bool bHavePhysicalProxy = false) = 0;

	// Clone static geometry. Makes an exact copy of the Static object and the contained geometry.
	// virtual IStatObj* Clone(bool bCloneChildren=true, bool nDynamic=false) = 0;

	//! Clones static geometry, Makes an exact copy of the Static object and the contained geometry.
	virtual IStatObj* Clone(bool bCloneGeometry, bool bCloneChildren, bool bMeshesOnly) = 0;

	//! Make sure that both objects have one-to-one vertex correspondance.
	//! Sets MorphBuddy for this object's render mesh
	//! \return 0 if failed (due to objects having no vertex maps most likely).
	virtual i32 SetDeformationMorphTarget(IStatObj* pDeformed) = 0;

	//! Change the weights of the deformation morphing according to point, radius, and strength.
	//! If the object is a compound object, updates the weights of its subobjects that have deformation morphs.
	//! Clones the object if necessary, otherwise updates the weights passed as a pWeights param
	//! \note radius==0 updates all weights of all vertices.
	virtual IStatObj* DeformMorph(const Vec3& pt, float r, float strength, IRenderMesh* pWeights = 0) = 0;

	//! hides all non-physicalized geometry, clones the object if necessary
	virtual IStatObj* HideFoliage() = 0;

	// return amount of texture memory used for vegetation sprites
	// virtual i32 GetSpritesTexMemoryUsage() = 0;

	//! Serialize the StatObj's mesh into a stream.
	virtual i32 Serialize(TSerialize ser) = 0;

	//! Get object properties as loaded from CGF.
	virtual tukk GetProperties() = 0;

	//! Update object properties.
	virtual void SetProperties(tukk) = 0;

	//! Get physical properties specified for object.
	virtual bool GetPhysicalProperties(float& mass, float& density) = 0;

	//! Return the last B operand for this object as A, along with its relative scale.
	virtual IStatObj* GetLastBooleanOp(float& scale) = 0;

	//! Intersect ray with static object.
	//! Ray must be in object local space.
	virtual bool RayIntersection(SRayHitInfo& hitInfo, IMaterial* pCustomMtl = 0) = 0;

	//! Intersect lineseg with static object. Works on dedi server as well.
	//! Lineseg must be in object local space. Returns the hit position and the surface type id of the point hit.
	virtual bool LineSegIntersection(const Lineseg& lineSeg, Vec3& hitPos, i32& surfaceTypeId) = 0;

	//! Debug Draw this static object.
	//! \param nFlags - bit0 no culling, bit1 - not draw lines.
	virtual void DebugDraw(const struct SGeometryDebugDrawInfo& info) = 0;

	//! Fill statistics about the level.
	virtual void GetStatistics(SStatistics& stats) = 0;

	//! Returns initial hide mask.
	virtual hidemask GetInitialHideMask() = 0;

	//! Set the filename of the mesh of the next state (for example damaged version).
	virtual void SetStreamingDependencyFilePath(tukk szFileName) = 0;

	//! Expose the computelod function from the engine.
	virtual i32  ComputeLodFromScale(float fScale, float fLodRatioNormalized, float fEntDistance, bool bFoliage, bool bForPrecache) = 0;

	virtual SMeshLodInfo ComputeGeometricMean() const = 0;

	//! Return the distance for the first LOD switch. Used for brushes and vegetation.
	virtual float GetLodDistance() const = 0;

	//! Return the additional local-space offset used when depth sorting rendered objects.
	virtual Vec3 GetDepthSortOffset() const = 0;

	// </interfuscator:shuffle>

protected:
	virtual ~IStatObj() {}; //!< Should be never called, use Release() instead.
};

#endif // _IStatObj_H_
