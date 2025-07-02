
#include "../PhysicsServerExampleBullet2.h"
#include "../PhysicsServerExample.h"
#include <drx3D/SharedMemory/PhysicsServerCommandProcessor.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>

struct Bullet2CommandProcessorCreation : public CommandProcessorCreationInterface
{
	virtual class CommandProcessorInterface* createCommandProcessor()
	{
		PhysicsServerCommandProcessor* proc = new PhysicsServerCommandProcessor;
		return proc;
	}

	virtual void deleteCommandProcessor(CommandProcessorInterface* proc)
	{
		delete proc;
	}
};

static Bullet2CommandProcessorCreation sBullet2CommandCreator;

CommonExampleInterface* PhysicsServerCreateFuncBullet2(struct CommonExampleOptions& options)
{
	options.m_commandProcessorCreation = &sBullet2CommandCreator;

	CommonExampleInterface* example = PhysicsServerCreateFuncInternal(options);
	return example;
}

D3_STANDALONE_EXAMPLE(PhysicsServerCreateFuncBullet2)
