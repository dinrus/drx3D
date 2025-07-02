
#include  <drx3D/SharedMemory/PhysicsClientSharedMemory2.h>
#include  <drx3D/SharedMemory/PosixSharedMemory.h>
#include  <drx3D/SharedMemory/Win32SharedMemory.h>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Common/b3Scalar.h>

#include  <drx3D/SharedMemory/SharedMemoryCommandProcessor.h>

PhysicsClientSharedMemory2::PhysicsClientSharedMemory2(SharedMemoryCommandProcessor* proc)
	: PhysicsDirect(proc, false)
{
	m_proc = proc;
}
PhysicsClientSharedMemory2::~PhysicsClientSharedMemory2()
{
}

void PhysicsClientSharedMemory2::setSharedMemoryInterface(class SharedMemoryInterface* sharedMem)
{
	if (m_proc)
	{
		m_proc->setSharedMemoryInterface(sharedMem);
	}
}
