// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	This is the interface to access command line arguments.
                This will avoid the need to parse command line in multiple
                places, and thus, reduce unnecessary code duplication.

   -------------------------------------------------------------------------
   История:
    - 30:7:2004   12:00 : Created by Márcio Martins

*************************************************************************/

#pragma once

//! The type of command line argument.
enum ECmdLineArgType
{

	eCLAT_Normal = 0,        //!< Argument was not preceeded by anything.
	eCLAT_Pre,               //!< Argument was preceeded by a minus sign '-'.
	eCLAT_Post,              //!< Argument was preceeded by a plus sign '+'.
	eCLAT_Executable,        //!< Argument is the executable filename.
};

//! Container for a command line Argument.
class ICmdLineArg
{
public:
	// <interfuscator:shuffle>
	virtual ~ICmdLineArg(){}

	//! \return The name of the argument.
	virtual tukk GetName() const = 0;

	//! \return The value of the argument as a null-terminated string.
	virtual tukk GetValue() const = 0;

	//! \return The type of argument.
	virtual const ECmdLineArgType GetType() const = 0;

	//! \return The value of the argument as float.
	virtual const float GetFValue() const = 0;

	//! \return The value of the argument as an integer.
	virtual i32k GetIValue() const = 0;
	// </interfuscator:shuffle>
};

//! Command line interface.
class ICmdLine
{
public:
	// <interfuscator:shuffle>
	virtual ~ICmdLine(){}

	//! Returns the n'th command line argument.
	//! \param n 0 returns executable name, otherwise returns n'th argument.
	//! \return Pointer to the command line argument at index specified by idx.
	virtual const ICmdLineArg* GetArg(i32 n) const = 0;

	//! \return The number of command line arguments.
	virtual i32 GetArgCount() const = 0;

	//! Finds an argument in the command line.
	//! \param name Name of the argument to find, excluding any '+' or '-'
	//! \return Pointer to a ICmdLineArg class containing the specified argument, or NULL if the argument was not found.
	virtual const ICmdLineArg* FindArg(const ECmdLineArgType ArgType, tukk name, bool caseSensitive = false) const = 0;
	// </interfuscator:shuffle>
};
