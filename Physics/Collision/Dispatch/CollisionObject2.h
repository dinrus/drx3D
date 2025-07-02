#ifndef DRX3D_COLLISION_OBJECT_H
#define DRX3D_COLLISION_OBJECT_H

#include <drx3D/Maths/Linear/Transform2.h>

//island management, m_activationState1
#define ACTIVE_TAG 1
#define ISLAND_SLEEPING 2
#define WANTS_DEACTIVATION 3
#define DISABLE_DEACTIVATION 4
#define DISABLE_SIMULATION 5
#define FIXED_BASE_MULTI_BODY 6

struct BroadphaseProxy;
class CollisionShape;
struct CollisionShapeData;
#include <drx3D/Maths/Linear/MotionState.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

typedef AlignedObjectArray<class CollisionObject2*> CollisionObject2Array;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define CollisionObject2Data CollisionObject2DoubleData
#define CollisionObject2DataName "CollisionObject2DoubleData"
#else
#define CollisionObject2Data CollisionObject2FloatData
#define CollisionObject2DataName "CollisionObject2FloatData"
#endif

/// CollisionObject2 can be used to manage collision detection objects.
/// CollisionObject2 maintains all information that is needed for a collision detection: Shape, Transform2 and AABB proxy.
/// They can be added to the CollisionWorld.
ATTRIBUTE_ALIGNED16(class)
CollisionObject2
{
protected:
    Transform2 m_worldTransform;

    ///m_interpolationWorldTransform is used for CCD and interpolation
    ///it can be either previous or future (predicted) transform
    Transform2 m_interpolationWorldTransform;
    //those two are experimental: just added for bullet time effect, so you can still apply impulses (directly modifying velocities)
    //without destroying the continuous interpolated motion (which uses this interpolation velocities)
    Vec3 m_interpolationLinearVelocity;
    Vec3 m_interpolationAngularVelocity;

    Vec3 m_anisotropicFriction;
    i32 m_hasAnisotropicFriction;
    Scalar m_contactProcessingThreshold;

    BroadphaseProxy* m_broadphaseHandle;
    CollisionShape* m_collisionShape;
    ///m_extensionPointer is used by some internal low-level drx3D extensions.
    uk m_extensionPointer;

    ///m_rootCollisionShape is temporarily used to store the original collision shape
    ///The m_collisionShape might be temporarily replaced by a child collision shape during collision detection purposes
    ///If it is NULL, the m_collisionShape is not temporarily replaced.
    CollisionShape* m_rootCollisionShape;

    i32 m_collisionFlags;

    i32 m_islandTag1;
    i32 m_companionId;
    i32 m_worldArrayIndex;  // index of object in world's collisionObjects array

    mutable i32 m_activationState1;
    mutable Scalar m_deactivationTime;

    Scalar m_friction;
    Scalar m_restitution;
    Scalar m_rollingFriction;   //torsional friction orthogonal to contact normal (useful to stop spheres rolling forever)
    Scalar m_spinningFriction;  // torsional friction around the contact normal (useful for grasping)
    Scalar m_contactDamping;
    Scalar m_contactStiffness;

    ///m_internalType is reserved to distinguish drx3D's CollisionObject2, RigidBody, SoftBody, GhostObject etc.
    ///do not assign your own m_internalType unless you write a new dynamics object class.
    i32 m_internalType;

    ///users can point to their objects, m_userPointer is not used by drx3D, see setUserPointer/getUserPointer

    uk m_userObjectPointer;

    i32 m_userIndex2;

    i32 m_userIndex;

    i32 m_userIndex3;

    ///time of impact calculation
    Scalar m_hitFraction;

    ///Swept sphere radius (0.0 by default), see ConvexConvexAlgorithm::
    Scalar m_ccdSweptSphereRadius;

    /// Don't do continuous collision detection if the motion (in one step) is less then m_ccdMotionThreshold
    Scalar m_ccdMotionThreshold;

    /// If some object should have elaborate collision filtering by sub-classes
    i32 m_checkCollideWith;

    AlignedObjectArray<const CollisionObject2*> m_objectsWithoutCollisionCheck;

    ///internal update revision number. It will be increased when the object changes. This allows some subsystems to perform lazy evaluation.
    i32 m_updateRevision;

    Vec3 m_customDebugColorRGB;

public:
    DRX3D_DECLARE_ALIGNED_ALLOCATOR();

    enum CollisionFlags
    {
        CF_DYNAMIC_OBJECT = 0,
        CF_STATIC_OBJECT = 1,
        CF_KINEMATIC_OBJECT = 2,
        CF_NO_CONTACT_RESPONSE = 4,
        CF_CUSTOM_MATERIAL_CALLBACK = 8,  //this allows per-triangle material (friction/restitution)
        CF_CHARACTER_OBJECT = 16,
        CF_DISABLE_VISUALIZE_OBJECT = 32,          //disable debug drawing
        CF_DISABLE_SPU_COLLISION_PROCESSING = 64,  //disable parallel/SPU processing
        CF_HAS_CONTACT_STIFFNESS_DAMPING = 128,
        CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR = 256,
        CF_HAS_FRICTION_ANCHOR = 512,
        CF_HAS_COLLISION_SOUND_TRIGGER = 1024
    };

    enum CollisionObject2Types
    {
        CO_COLLISION_OBJECT = 1,
        CO_RIGID_BODY = 2,
        ///CO_GHOST_OBJECT keeps track of all objects overlapping its AABB and that pass its collision filter
        ///It is useful for collision sensors, explosion objects, character controller etc.
        CO_GHOST_OBJECT = 4,
        CO_SOFT_BODY = 8,
        CO_HF_FLUID = 16,
        CO_USER_TYPE = 32,
        CO_FEATHERSTONE_LINK = 64
    };

    enum AnisotropicFrictionFlags
    {
        CF_ANISOTROPIC_FRICTION_DISABLED = 0,
        CF_ANISOTROPIC_FRICTION = 1,
        CF_ANISOTROPIC_ROLLING_FRICTION = 2
    };

    SIMD_FORCE_INLINE bool mergesSimulationIslands() const
    {
        ///static objects, kinematic and object without contact response don't merge islands
        return ((m_collisionFlags & (CF_STATIC_OBJECT | CF_KINEMATIC_OBJECT | CF_NO_CONTACT_RESPONSE)) == 0);
    }

    const Vec3& getAnisotropicFriction() const
    {
        return m_anisotropicFriction;
    }
    void setAnisotropicFriction(const Vec3& anisotropicFriction, i32 frictionMode = CF_ANISOTROPIC_FRICTION)
    {
        m_anisotropicFriction = anisotropicFriction;
        bool isUnity = (anisotropicFriction[0] != 1.f) || (anisotropicFriction[1] != 1.f) || (anisotropicFriction[2] != 1.f);
        m_hasAnisotropicFriction = isUnity ? frictionMode : 0;
    }
    bool hasAnisotropicFriction(i32 frictionMode = CF_ANISOTROPIC_FRICTION) const
    {
        return (m_hasAnisotropicFriction & frictionMode) != 0;
    }

    ///the constraint solver can discard solving contacts, if the distance is above this threshold. 0 by default.
    ///Note that using contacts with positive distance can improve stability. It increases, however, the chance of colliding with degerate contacts, such as 'interior' triangle edges
    void setContactProcessingThreshold(Scalar contactProcessingThreshold)
    {
        m_contactProcessingThreshold = contactProcessingThreshold;
    }
    Scalar getContactProcessingThreshold() const
    {
        return m_contactProcessingThreshold;
    }

    SIMD_FORCE_INLINE bool isStaticObject() const
    {
        return (m_collisionFlags & CF_STATIC_OBJECT) != 0;
    }

    SIMD_FORCE_INLINE bool isKinematicObject() const
    {
        return (m_collisionFlags & CF_KINEMATIC_OBJECT) != 0;
    }

    SIMD_FORCE_INLINE bool isStaticOrKinematicObject() const
    {
        return (m_collisionFlags & (CF_KINEMATIC_OBJECT | CF_STATIC_OBJECT)) != 0;
    }

    SIMD_FORCE_INLINE bool hasContactResponse() const
    {
        return (m_collisionFlags & CF_NO_CONTACT_RESPONSE) == 0;
    }

    CollisionObject2();

    virtual ~CollisionObject2();

    virtual void setCollisionShape(CollisionShape * collisionShape)
    {
        m_updateRevision++;
        m_collisionShape = collisionShape;
        m_rootCollisionShape = collisionShape;
    }

    SIMD_FORCE_INLINE const CollisionShape* getCollisionShape() const
    {
        return m_collisionShape;
    }

    SIMD_FORCE_INLINE CollisionShape* getCollisionShape()
    {
        return m_collisionShape;
    }

    void setIgnoreCollisionCheck(const CollisionObject2* co, bool ignoreCollisionCheck)
    {
        if (ignoreCollisionCheck)
        {
            //We don't check for duplicates. Is it ok to leave that up to the user of this API?
            //i32 index = m_objectsWithoutCollisionCheck.findLinearSearch(co);
            //if (index == m_objectsWithoutCollisionCheck.size())
            //{
            m_objectsWithoutCollisionCheck.push_back(co);
            //}
        }
        else
        {
            m_objectsWithoutCollisionCheck.remove(co);
        }
        m_checkCollideWith = m_objectsWithoutCollisionCheck.size() > 0;
    }

        i32 getNumObjectsWithoutCollision() const
    {
        return m_objectsWithoutCollisionCheck.size();
    }

    const CollisionObject2* getObjectWithoutCollision(i32 index)
    {
        return m_objectsWithoutCollisionCheck[index];
    }

    virtual bool checkCollideWithOverride(const CollisionObject2* co) const
    {
        i32 index = m_objectsWithoutCollisionCheck.findLinearSearch(co);
        if (index < m_objectsWithoutCollisionCheck.size())
        {
            return false;
        }
        return true;
    }

    ///Avoid using this internal API call, the extension pointer is used by some drx3D extensions.
    ///If you need to store your own user pointer, use 'setUserPointer/getUserPointer' instead.
    uk internalGetExtensionPointer() const
    {
        return m_extensionPointer;
    }
    ///Avoid using this internal API call, the extension pointer is used by some drx3D extensions
    ///If you need to store your own user pointer, use 'setUserPointer/getUserPointer' instead.
    void internalSetExtensionPointer(uk pointer)
    {
        m_extensionPointer = pointer;
    }

    SIMD_FORCE_INLINE i32 getActivationState() const { return m_activationState1; }

    void setActivationState(i32 newState) const;

    void setDeactivationTime(Scalar time)
    {
        m_deactivationTime = time;
    }
    Scalar getDeactivationTime() const
    {
        return m_deactivationTime;
    }

    void forceActivationState(i32 newState) const;

    void activate(bool forceActivation = false) const;

    SIMD_FORCE_INLINE bool isActive() const
    {
        return ((getActivationState() != FIXED_BASE_MULTI_BODY) && (getActivationState() != ISLAND_SLEEPING) && (getActivationState() != DISABLE_SIMULATION));
    }

    void setRestitution(Scalar rest)
    {
        m_updateRevision++;
        m_restitution = rest;
    }
    Scalar getRestitution() const
    {
        return m_restitution;
    }
    void setFriction(Scalar frict)
    {
        m_updateRevision++;
        m_friction = frict;
    }
    Scalar getFriction() const
    {
        return m_friction;
    }

    void setRollingFriction(Scalar frict)
    {
        m_updateRevision++;
        m_rollingFriction = frict;
    }
    Scalar getRollingFriction() const
    {
        return m_rollingFriction;
    }
    void setSpinningFriction(Scalar frict)
    {
        m_updateRevision++;
        m_spinningFriction = frict;
    }
    Scalar getSpinningFriction() const
    {
        return m_spinningFriction;
    }
    void setContactStiffnessAndDamping(Scalar stiffness, Scalar damping)
    {
        m_updateRevision++;
        m_contactStiffness = stiffness;
        m_contactDamping = damping;

        m_collisionFlags |= CF_HAS_CONTACT_STIFFNESS_DAMPING;

        //avoid divisions by zero...
        if (m_contactStiffness < SIMD_EPSILON)
        {
            m_contactStiffness = SIMD_EPSILON;
        }
    }

    Scalar getContactStiffness() const
    {
        return m_contactStiffness;
    }

    Scalar getContactDamping() const
    {
        return m_contactDamping;
    }

    ///reserved for drx3D internal usage
    i32 getInternalType() const
    {
        return m_internalType;
    }

    Transform2& getWorldTransform()
    {
        return m_worldTransform;
    }

    const Transform2& getWorldTransform() const
    {
        return m_worldTransform;
    }

    void setWorldTransform(const Transform2& worldTrans)
    {
        m_updateRevision++;
        m_worldTransform = worldTrans;
    }

    SIMD_FORCE_INLINE BroadphaseProxy* getBroadphaseHandle()
    {
        return m_broadphaseHandle;
    }

    SIMD_FORCE_INLINE const BroadphaseProxy* getBroadphaseHandle() const
    {
        return m_broadphaseHandle;
    }

    void setBroadphaseHandle(BroadphaseProxy * handle)
    {
        m_broadphaseHandle = handle;
    }

    const Transform2& getInterpolationWorldTransform() const
    {
        return m_interpolationWorldTransform;
    }

    Transform2& getInterpolationWorldTransform()
    {
        return m_interpolationWorldTransform;
    }

    void setInterpolationWorldTransform(const Transform2& trans)
    {
        m_updateRevision++;
        m_interpolationWorldTransform = trans;
    }

    void setInterpolationLinearVelocity(const Vec3& linvel)
    {
        m_updateRevision++;
        m_interpolationLinearVelocity = linvel;
    }

    void setInterpolationAngularVelocity(const Vec3& angvel)
    {
        m_updateRevision++;
        m_interpolationAngularVelocity = angvel;
    }

    const Vec3& getInterpolationLinearVelocity() const
    {
        return m_interpolationLinearVelocity;
    }

    const Vec3& getInterpolationAngularVelocity() const
    {
        return m_interpolationAngularVelocity;
    }

    SIMD_FORCE_INLINE i32 getIslandTag() const
    {
        return m_islandTag1;
    }

    void setIslandTag(i32 tag)
    {
        m_islandTag1 = tag;
    }

    SIMD_FORCE_INLINE i32 getCompanionId() const
    {
        return m_companionId;
    }

    void setCompanionId(i32 id)
    {
        m_companionId = id;
    }

    SIMD_FORCE_INLINE i32 getWorldArrayIndex() const
    {
        return m_worldArrayIndex;
    }

    // only should be called by CollisionWorld
    void setWorldArrayIndex(i32 ix)
    {
        m_worldArrayIndex = ix;
    }

    SIMD_FORCE_INLINE Scalar getHitFraction() const
    {
        return m_hitFraction;
    }

    void setHitFraction(Scalar hitFraction)
    {
        m_hitFraction = hitFraction;
    }

    SIMD_FORCE_INLINE i32 getCollisionFlags() const
    {
        return m_collisionFlags;
    }

    void setCollisionFlags(i32 flags)
    {
        m_collisionFlags = flags;
    }

    ///Swept sphere radius (0.0 by default), see ConvexConvexAlgorithm::
    Scalar getCcdSweptSphereRadius() const
    {
        return m_ccdSweptSphereRadius;
    }

    ///Swept sphere radius (0.0 by default), see ConvexConvexAlgorithm::
    void setCcdSweptSphereRadius(Scalar radius)
    {
        m_ccdSweptSphereRadius = radius;
    }

    Scalar getCcdMotionThreshold() const
    {
        return m_ccdMotionThreshold;
    }

    Scalar getCcdSquareMotionThreshold() const
    {
        return m_ccdMotionThreshold * m_ccdMotionThreshold;
    }

    /// Don't do continuous collision detection if the motion (in one step) is less then m_ccdMotionThreshold
    void setCcdMotionThreshold(Scalar ccdMotionThreshold)
    {
        m_ccdMotionThreshold = ccdMotionThreshold;
    }

    ///users can point to their objects, userPointer is not used by drx3D
    uk getUserPointer() const
    {
        return m_userObjectPointer;
    }

    i32 getUserIndex() const
    {
        return m_userIndex;
    }

    i32 getUserIndex2() const
    {
        return m_userIndex2;
    }

    i32 getUserIndex3() const
    {
        return m_userIndex3;
    }

    ///users can point to their objects, userPointer is not used by drx3D
    void setUserPointer(uk userPointer)
    {
        m_userObjectPointer = userPointer;
    }

    ///users can point to their objects, userPointer is not used by drx3D
    void setUserIndex(i32 index)
    {
        m_userIndex = index;
    }

    void setUserIndex2(i32 index)
    {
        m_userIndex2 = index;
    }

    void setUserIndex3(i32 index)
    {
        m_userIndex3 = index;
    }

    i32 getUpdateRevisionInternal() const
    {
        return m_updateRevision;
    }

    void setCustomDebugColor(const Vec3& colorRGB)
    {
        m_customDebugColorRGB = colorRGB;
        m_collisionFlags |= CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR;
    }

    void removeCustomDebugColor()
    {
        m_collisionFlags &= ~CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR;
    }

    bool getCustomDebugColor(Vec3 & colorRGB) const
    {
        bool hasCustomColor = (0 != (m_collisionFlags & CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR));
        if (hasCustomColor)
        {
            colorRGB = m_customDebugColorRGB;
        }
        return hasCustomColor;
    }

    inline bool checkCollideWith(const CollisionObject2* co) const
    {
        if (m_checkCollideWith)
            return checkCollideWithOverride(co);

        return true;
    }

    virtual i32 calculateSerializeBufferSize() const;

    ///fills the dataBuffer and returns the struct name (and 0 on failure)
    virtual tukk serialize(uk dataBuffer, class Serializer* serializer) const;

    virtual void serializeSingleObject(class Serializer * serializer) const;
};

// clang-format off

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct  CollisionObject2DoubleData
{
    void                    *m_broadphaseHandle;
    void                    *m_collisionShape;
    CollisionShapeData  *m_rootCollisionShape;
    char                    *m_name;

    Transform2DoubleData m_worldTransform;
    Transform2DoubleData m_interpolationWorldTransform;
    Vec3DoubleData       m_interpolationLinearVelocity;
    Vec3DoubleData       m_interpolationAngularVelocity;
    Vec3DoubleData       m_anisotropicFriction;
    double                  m_contactProcessingThreshold;
    double                  m_deactivationTime;
    double                  m_friction;
    double                  m_rollingFriction;
    double                  m_contactDamping;
    double                  m_contactStiffness;
    double                  m_restitution;
    double                  m_hitFraction;
    double                  m_ccdSweptSphereRadius;
    double                  m_ccdMotionThreshold;
    i32                     m_hasAnisotropicFriction;
    i32                     m_collisionFlags;
    i32                     m_islandTag1;
    i32                     m_companionId;
    i32                     m_activationState1;
    i32                     m_internalType;
    i32                     m_checkCollideWith;
    i32                     m_collisionFilterGroup;
    i32                     m_collisionFilterMask;
    i32                     m_uniqueId;//m_uniqueId is introduced for paircache. could get rid of this, by calculating the address offset etc.
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct  CollisionObject2FloatData
{
    void                    *m_broadphaseHandle;
    void                    *m_collisionShape;
    CollisionShapeData  *m_rootCollisionShape;
    char                    *m_name;

    Transform2FloatData  m_worldTransform;
    Transform2FloatData  m_interpolationWorldTransform;
    Vec3FloatData        m_interpolationLinearVelocity;
    Vec3FloatData        m_interpolationAngularVelocity;
    Vec3FloatData        m_anisotropicFriction;
    float                   m_contactProcessingThreshold;
    float                   m_deactivationTime;
    float                   m_friction;
    float                   m_rollingFriction;
    float                   m_contactDamping;
    float                   m_contactStiffness;
    float                   m_restitution;
    float                   m_hitFraction;
    float                   m_ccdSweptSphereRadius;
    float                   m_ccdMotionThreshold;
    i32                     m_hasAnisotropicFriction;
    i32                     m_collisionFlags;
    i32                     m_islandTag1;
    i32                     m_companionId;
    i32                     m_activationState1;
    i32                     m_internalType;
    i32                     m_checkCollideWith;
    i32                     m_collisionFilterGroup;
    i32                     m_collisionFilterMask;
    i32                     m_uniqueId;
};
// clang-format on

SIMD_FORCE_INLINE i32 CollisionObject2::calculateSerializeBufferSize() const
{
    return sizeof(CollisionObject2Data);
}

#endif  //DRX3D_COLLISION_OBJECT_H
