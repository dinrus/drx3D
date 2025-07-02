
#ifndef EXAMPLE_ENTRIES_H
#define EXAMPLE_ENTRIES_H

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>

class ExampleEntriesAll : public ExampleEntries
{
	struct ExampleEntriesInternalData* m_data;

public:
	ExampleEntriesAll();
	virtual ~ExampleEntriesAll();

	static void registerExampleEntry(i32 menuLevel, tukk name, tukk description,
	                         CommonExampleInterface::CreateFunc* createFunc, i32 option = 0);

	virtual void initExampleEntries();

	virtual void initOpenCLExampleEntries();

	virtual i32 getNumRegisteredExamples();

	virtual CommonExampleInterface::CreateFunc* getExampleCreateFunc(i32 index);

	virtual tukk getExampleName(i32 index);

	virtual tukk getExampleDescription(i32 index);

	virtual i32 getExampleOption(i32 index);
};

#endif  //EXAMPLE_ENTRIES_H
