
/***************************************************************************************************
**
** Real-Time Hierarchical Profiling for Game Programming Gems 3
**
** by Greg Hjelstrom & Byon Garrabrant
**
***************************************************************************************************/

// Credits: The Clock class was inspired by the Timer classes in
// Ogre (www.ogre3d.org).

#ifndef DRX3D_QUICK_PROF_H
#define DRX3D_QUICK_PROF_H

#include <drx3D/Maths/Linear/Scalar.h>
#define USE_DRX3D_CLOCK 1

#ifdef USE_DRX3D_CLOCK

///The Clock is a portable basic clock that measures accurate time in seconds, use for profiling.
class Clock
{
public:
	Clock();

	Clock(const Clock& other);
	Clock& operator=(const Clock& other);

	~Clock();

	/// Resets the initial reference time.
	void reset();

	/// Returns the time in ms since the last call to reset or since
	/// the Clock was created.
	zu64 getTimeMilliseconds();

	/// Returns the time in us since the last call to reset or since
	/// the Clock was created.
	zu64 getTimeMicroseconds();

	zu64 getTimeNanoseconds();

	/// Returns the time in s since the last call to reset or since
	/// the Clock was created.
	Scalar getTimeSeconds();

private:
	struct ClockData* m_data;
};

#endif  //USE_DRX3D_CLOCK

typedef void(EnterProfileZoneFunc)(tukk msg);
typedef void(LeaveProfileZoneFunc)();

EnterProfileZoneFunc* GetCurrentEnterProfileZoneFunc();
LeaveProfileZoneFunc* GetCurrentLeaveProfileZoneFunc();

void SetCustomEnterProfileZoneFunc(EnterProfileZoneFunc* enterFunc);
void SetCustomLeaveProfileZoneFunc(LeaveProfileZoneFunc* leaveFunc);

#ifndef DRX3D_ENABLE_PROFILE
#define DRX3D_NO_PROFILE 1
#endif  //DRX3D_NO_PROFILE

u32k DRX3D_QUICKPROF_MAX_THREAD_COUNT = 64;

//QuickprofGetCurrentThreadIndex will return -1 if thread index cannot be determined,
//otherwise returns thread index in range [0..maxThreads]
u32 QuickprofGetCurrentThreadIndex2();

#ifndef DRX3D_NO_PROFILE


#include <stdio.h>  //@todo remove this, backwards compatibility

#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <new>

///A node in the Profile Hierarchy Tree
class CProfileNode
{
public:
	CProfileNode(tukk name, CProfileNode* parent);
	~CProfileNode(void);

	CProfileNode* Get_Sub_Node(tukk name);

	CProfileNode* Get_Parent(void) { return Parent; }
	CProfileNode* Get_Sibling(void) { return Sibling; }
	CProfileNode* Get_Child(void) { return Child; }

	void CleanupMemory();
	void Reset(void);
	void Call(void);
	bool Return(void);

	tukk Get_Name(void) { return Name; }
	i32 Get_Total_Calls(void) { return TotalCalls; }
	float Get_Total_Time(void) { return TotalTime; }
	uk GetUserPointer() const { return m_userPtr; }
	void SetUserPointer(uk ptr) { m_userPtr = ptr; }

protected:
	tukk Name;
	i32 TotalCalls;
	float TotalTime;
	u64 StartTime;
	i32 RecursionCounter;

	CProfileNode* Parent;
	CProfileNode* Child;
	CProfileNode* Sibling;
	uk m_userPtr;
};

///An iterator to navigate through the tree
class CProfileIterator
{
public:
	// Access all the children of the current parent
	void First(void);
	void Next(void);
	bool Is_Done(void);
	bool Is_Root(void) { return (CurrentParent->Get_Parent() == 0); }

	void Enter_Child(i32 index);     // Make the given child the new parent
	void Enter_Largest_Child(void);  // Make the largest child the new parent
	void Enter_Parent(void);         // Make the current parent's parent the new parent

	// Access the current child
	tukk Get_Current_Name(void) { return CurrentChild->Get_Name(); }
	i32 Get_Current_Total_Calls(void) { return CurrentChild->Get_Total_Calls(); }
	float Get_Current_Total_Time(void) { return CurrentChild->Get_Total_Time(); }

	uk Get_Current_UserPointer(void) { return CurrentChild->GetUserPointer(); }
	void Set_Current_UserPointer(uk ptr) { CurrentChild->SetUserPointer(ptr); }
	// Access the current parent
	tukk Get_Current_Parent_Name(void) { return CurrentParent->Get_Name(); }
	i32 Get_Current_Parent_Total_Calls(void) { return CurrentParent->Get_Total_Calls(); }
	float Get_Current_Parent_Total_Time(void) { return CurrentParent->Get_Total_Time(); }

protected:
	CProfileNode* CurrentParent;
	CProfileNode* CurrentChild;

	CProfileIterator(CProfileNode* start);
	friend class CProfileManager;
};

///The Manager for the Profile system
class CProfileManager
{
public:
	static void Start_Profile(tukk name);
	static void Stop_Profile(void);

	static void CleanupMemory(void);
	//	{
	//		Root.CleanupMemory();
	//	}

	static void Reset(void);
	static void Increment_Frame_Counter(void);
	static i32 Get_Frame_Count_Since_Reset(void) { return FrameCounter; }
	static float Get_Time_Since_Reset(void);

	static CProfileIterator* Get_Iterator(void);
	//	{
	//
	//		return new CProfileIterator( &Root );
	//	}
	static void Release_Iterator(CProfileIterator* iterator) { delete (iterator); }

	static void dumpRecursive(CProfileIterator* profileIterator, i32 spacing);

	static void dumpAll();

private:
	static i32 FrameCounter;
	static u64 ResetTime;
};

#endif  //#ifndef DRX3D_NO_PROFILE

///ProfileSampleClass is a simple way to profile a function's scope
///Use the DRX3D_PROFILE macro at the start of scope to time
class CProfileSample
{
public:
	CProfileSample(tukk name);

	~CProfileSample(void);
};

#define DRX3D_PROFILE(name) CProfileSample __profile(name)

#endif  //DRX3D_QUICK_PROF_H
