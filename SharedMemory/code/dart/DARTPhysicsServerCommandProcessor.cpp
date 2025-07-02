#include "DARTPhysicsServerCommandProcessor.h"

DARTPhysicsServerCommandProcessor::DARTPhysicsServerCommandProcessor()
{
}

DARTPhysicsServerCommandProcessor::~DARTPhysicsServerCommandProcessor()
{
}

bool DARTPhysicsServerCommandProcessor::connect()
{
	return false;
}

void DARTPhysicsServerCommandProcessor::disconnect()
{
}

bool DARTPhysicsServerCommandProcessor::isConnected() const
{
	return false;
}

bool DARTPhysicsServerCommandProcessor::processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	return false;
}

bool DARTPhysicsServerCommandProcessor::receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes)
{
	return false;
}
