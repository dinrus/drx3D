// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_LIVECREATENATIVEFILE_H_
#define _H_LIVECREATENATIVEFILE_H_

namespace LiveCreate
{
/// Native (platform related) file reader
class CLiveCreateFileReader : public IDataReadStream
{
private:
	FILE* m_handle;

public:
	CLiveCreateFileReader();

	// IDataReadStream interface
	virtual void        Delete();
	virtual void        Skip(u32k size);
	virtual void        Read(uk pData, u32k size);
	virtual void        Read8(uk pData);
	virtual void        Read4(uk pData);
	virtual void        Read2(uk pData);
	virtual void        Read1(uk pData);
	virtual ukk GetPointer();

	// Create a reader for a native platform path
	static CLiveCreateFileReader* CreateReader(tukk szNativePath);

private:
	~CLiveCreateFileReader();
};
}

#endif
