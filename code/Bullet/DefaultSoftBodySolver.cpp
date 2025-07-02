#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexArray.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>

#include <drx3D/Physics/SoftBody/DefaultSoftBodySolver.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>

DefaultSoftBodySolver::DefaultSoftBodySolver()
{
	// Initial we will clearly need to update solver constants
	// For now this is global for the cloths linked with this solver - we should probably make this body specific
	// for performance in future once we understand more clearly when constants need to be updated
	m_updateSolverConstants = true;
}

DefaultSoftBodySolver::~DefaultSoftBodySolver()
{
}

// In this case the data is already in the soft bodies so there is no need for us to do anything
void DefaultSoftBodySolver::copyBackToSoftBodies(bool bMove)
{
}

void DefaultSoftBodySolver::optimize(AlignedObjectArray<SoftBody *> &softBodies, bool forceUpdate)
{
	m_softBodySet.copyFromArray(softBodies);
}

void DefaultSoftBodySolver::updateSoftBodies()
{
	for (i32 i = 0; i < m_softBodySet.size(); i++)
	{
		SoftBody *psb = (SoftBody *)m_softBodySet[i];
		if (psb->isActive())
		{
			psb->integrateMotion();
		}
	}
}  // updateSoftBodies

bool DefaultSoftBodySolver::checkInitialized()
{
	return true;
}

void DefaultSoftBodySolver::solveConstraints(Scalar solverdt)
{
	// Solve constraints for non-solver softbodies
	for (i32 i = 0; i < m_softBodySet.size(); ++i)
	{
		SoftBody *psb = static_cast<SoftBody *>(m_softBodySet[i]);
		if (psb->isActive())
		{
			psb->solveConstraints();
		}
	}
}  // DefaultSoftBodySolver::solveConstraints

void DefaultSoftBodySolver::copySoftBodyToVertexBuffer(const SoftBody *const softBody, VertexBufferDescriptor *vertexBuffer)
{
	// Currently only support CPU output buffers
	// TODO: check for DX11 buffers. Take all offsets into the same DX11 buffer
	// and use them together on a single kernel call if possible by setting up a
	// per-cloth target buffer array for the copy kernel.

	if (vertexBuffer->getBufferType() == VertexBufferDescriptor::CPU_BUFFER)
	{
		const AlignedObjectArray<SoftBody::Node> &clothVertices(softBody->m_nodes);
		i32 numVertices = clothVertices.size();

		const CPUVertexBufferDescriptor *cpuVertexBuffer = static_cast<CPUVertexBufferDescriptor *>(vertexBuffer);
		float *basePointer = cpuVertexBuffer->getBasePointer();

		if (vertexBuffer->hasVertexPositions())
		{
			i32k vertexOffset = cpuVertexBuffer->getVertexOffset();
			i32k vertexStride = cpuVertexBuffer->getVertexStride();
			float *vertexPointer = basePointer + vertexOffset;

			for (i32 vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
			{
				Vec3 position = clothVertices[vertexIndex].m_x;
				*(vertexPointer + 0) = (float)position.getX();
				*(vertexPointer + 1) = (float)position.getY();
				*(vertexPointer + 2) = (float)position.getZ();
				vertexPointer += vertexStride;
			}
		}
		if (vertexBuffer->hasNormals())
		{
			i32k normalOffset = cpuVertexBuffer->getNormalOffset();
			i32k normalStride = cpuVertexBuffer->getNormalStride();
			float *normalPointer = basePointer + normalOffset;

			for (i32 vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
			{
				Vec3 normal = clothVertices[vertexIndex].m_n;
				*(normalPointer + 0) = (float)normal.getX();
				*(normalPointer + 1) = (float)normal.getY();
				*(normalPointer + 2) = (float)normal.getZ();
				normalPointer += normalStride;
			}
		}
	}
}  // DefaultSoftBodySolver::copySoftBodyToVertexBuffer

void DefaultSoftBodySolver::processCollision(SoftBody *softBody, SoftBody *otherSoftBody)
{
	softBody->defaultCollisionHandler(otherSoftBody);
}

// For the default solver just leave the soft body to do its collision processing
void DefaultSoftBodySolver::processCollision(SoftBody *softBody, const CollisionObject2Wrapper *collisionObjectWrap)
{
	softBody->defaultCollisionHandler(collisionObjectWrap);
}  // DefaultSoftBodySolver::processCollision

void DefaultSoftBodySolver::predictMotion(Scalar timeStep)
{
	for (i32 i = 0; i < m_softBodySet.size(); ++i)
	{
		SoftBody *psb = m_softBodySet[i];

		if (psb->isActive())
		{
			psb->predictMotion(timeStep);
		}
	}
}
