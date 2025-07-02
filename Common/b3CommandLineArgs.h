#ifndef COMMAND_LINE_ARGS_H
#define COMMAND_LINE_ARGS_H

/******************************************************************************
 * Command-line parsing
 ******************************************************************************/
#include <map>
#include <algorithm>
#include <string>
#include <cstring>
#include <sstream>
class b3CommandLineArgs
{
protected:
	std::map<STxt, STxt> pairs;

public:
	// Constructor
	b3CommandLineArgs(i32 argc, char **argv)
	{
		addArgs(argc, argv);
	}

	void addArgs(i32 argc, char **argv)
	{
		for (i32 i = 1; i < argc; i++)
		{
			STxt arg = argv[i];

			if ((arg.length() < 2) || (arg[0] != '-') || (arg[1] != '-'))
			{
				continue;
			}

			STxt::size_type pos;
			STxt key, val;
			if ((pos = arg.find('=')) == STxt::npos)
			{
				key = STxt(arg, 2, arg.length() - 2);
				val = "";
			}
			else
			{
				key = STxt(arg, 2, pos - 2);
				val = STxt(arg, pos + 1, arg.length() - 1);
			}

			//only add new keys, don't replace existing
			if (pairs.find(key) == pairs.end())
			{
				pairs[key] = val;
			}
		}
	}

	bool CheckCmdLineFlag(tukk arg_name)
	{
		std::map<STxt, STxt>::iterator itr;
		if ((itr = pairs.find(arg_name)) != pairs.end())
		{
			return true;
		}
		return false;
	}

	template <typename T>
	bool GetCmdLineArgument(tukk arg_name, T &val);

	i32 ParsedArgc()
	{
		return pairs.size();
	}
};

template <typename T>
inline bool b3CommandLineArgs::GetCmdLineArgument(tukk arg_name, T &val)
{
	std::map<STxt, STxt>::iterator itr;
	if ((itr = pairs.find(arg_name)) != pairs.end())
	{
		std::istringstream strstream(itr->second);
		strstream >> val;
		return true;
	}
	return false;
}

template <>
inline bool b3CommandLineArgs::GetCmdLineArgument<char *>(tukk arg_name, char *&val)
{
	std::map<STxt, STxt>::iterator itr;
	if ((itr = pairs.find(arg_name)) != pairs.end())
	{
		STxt s = itr->second;
		val = (char *)malloc(sizeof(char) * (s.length() + 1));
		std::strcpy(val, s.c_str());
		return true;
	}
	else
	{
		val = NULL;
	}
	return false;
}

#endif  //COMMAND_LINE_ARGS_H
