#ifndef DRX3D_BULLET_WORLD_IMPORTER_H
#define DRX3D_BULLET_WORLD_IMPORTER_H

#include <drx3D/Importers/Bullet/WorldImporter.h>

class BulletFile;

namespace bParse
{
class BulletFile;

};

///The btBulletWorldImporter is a starting point to import .bullet files.
///note that not all data is converted yet. You are expected to override or modify this class.
///See drx3D/Demos/SerializeDemo for a derived class that extract btSoftBody objects too.
class BulletWorldImporter : public WorldImporter
{
public:
	BulletWorldImporter(DynamicsWorld* world = 0);

	virtual ~BulletWorldImporter();

	///if you pass a valid preSwapFilenameOut, it will save a new file with a different endianness
	///this pre-swapped file can be loaded without swapping on a target platform of different endianness
	bool loadFile(tukk fileName, tukk preSwapFilenameOut = 0);

	///the memoryBuffer might be modified (for example if endian swaps are necessary)
	bool loadFileFromMemory(tuk memoryBuffer, i32 len);

	bool loadFileFromMemory(bParse::BulletFile* file);

	//call make sure bulletFile2 has been parsed, either using btBulletFile::parse or btBulletWorldImporter::loadFileFromMemory
	virtual bool convertAllObjects(bParse::BulletFile* file);
};

#endif  //DRX3D_BULLET_WORLD_IMPORTER_H
