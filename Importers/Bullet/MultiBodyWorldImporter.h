#ifndef DRX3D_MULTIBODY_WORLD_IMPORTER_H
#define DRX3D_MULTIBODY_WORLD_IMPORTER_H

#include "BulletWorldImporter.h"

class MultiBodyWorldImporter : public BulletWorldImporter
{
	struct MultiBodyWorldImporterInternalData* m_data;

public:
	MultiBodyWorldImporter(class MultiBodyDynamicsWorld* world);
	virtual ~
	
	MultiBodyWorldImporter();

	virtual bool convertAllObjects(bParse::BulletFile* bulletFile2);

	virtual void deleteAllData();
};

#endif  //DRX3D_MULTIBODY_WORLD_IMPORTER_H
