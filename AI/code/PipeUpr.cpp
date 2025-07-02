// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   PipeUpr.cpp
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 8:6:2005   14:17 : Created by Kirill Bulatsev

 *********************************************************************/

#include <drx3D/AI/StdAfx.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/AI/PipeUpr.h>
#include <drx3D/AI/GoalPipe.h>
#include <drx3D/CoreX/StlUtils.h>

CPipeUpr::CPipeUpr(void) :
	m_bDynamicGoalPipes(false)
{
	CreateGoalPipe("_first_", SilentlyReplaceDuplicate);
}

CPipeUpr::~CPipeUpr(void)
{
	GoalMap::iterator gi;
	for (gi = m_mapGoals.begin(); gi != m_mapGoals.end(); ++gi)
		delete gi->second;

	m_mapGoals.clear();
}

void CPipeUpr::ClearAllGoalPipes()
{
	CGoalPipe* first = NULL;

	for (GoalMap::iterator gi = m_mapGoals.begin(); gi != m_mapGoals.end(); )
	{
		if (gi->first != "_first_")
		{
			delete gi->second;
			m_mapGoals.erase(gi++);
		}
		else
		{
			first = gi->second;
			++gi;
		}
	}

	if (!first)
		CreateGoalPipe("_first_", SilentlyReplaceDuplicate);
	else
		m_bDynamicGoalPipes = false;
}

//
//-----------------------------------------------------------------------------------------------------------
IGoalPipe* CPipeUpr::CreateGoalPipe(tukk pName,
                                        const CPipeUpr::ActionToTakeWhenDuplicateFound actionToTakeWhenDuplicateFound)
{
	if (!strcmp("_first_", pName))
		m_bDynamicGoalPipes = false;

	if (!strcmp("_last_", pName))
	{
		m_bDynamicGoalPipes = true;
		return NULL;
	}

	// always create a new goal pipe
	CGoalPipe* pNewPipe = new CGoalPipe(pName, m_bDynamicGoalPipes);

	// try to insert the new element in the map
	std::pair<GoalMap::iterator, bool> insertResult = m_mapGoals.insert(GoalMap::iterator::value_type(pName, pNewPipe));

	const bool goalPipeAlreadyExists = !insertResult.second;
	if (goalPipeAlreadyExists)
	{
		if (actionToTakeWhenDuplicateFound == ReplaceDuplicateAndReportError)
		{
			DrxWarning(VALIDATOR_MODULE_AI, VALIDATOR_ERROR, "Goal pipe with name %s already exists. Replacing.", pName);
		}

		// ...destroy the old one...
		delete insertResult.first->second;
		// ...and assign the new one
		insertResult.first->second = pNewPipe;
	}

	return pNewPipe;
}

//
//-----------------------------------------------------------------------------------------------------------
IGoalPipe* CPipeUpr::OpenGoalPipe(tukk pName)
{
	if (!pName)
		return 0;
	GoalMap::iterator gi = m_mapGoals.find(CONST_TEMP_STRING(pName));
	return gi != m_mapGoals.end() ? gi->second : NULL;
}

//
//-----------------------------------------------------------------------------------------------------------
CGoalPipe* CPipeUpr::IsGoalPipe(tukk name)
{
	GoalMap::iterator gi = m_mapGoals.find(CONST_TEMP_STRING(name));
	return gi != m_mapGoals.end() ? gi->second->Clone() : NULL;
}

//
//-----------------------------------------------------------------------------------------------------------
void CPipeUpr::Serialize(TSerialize ser)
{
#ifdef SERIALIZE_DYNAMIC_GOALPIPES
	i32 counter(0);
	char nameBuffer[16];
	ser.BeginGroup("DynamicGoalPipes");
	{
		drx_sprintf(nameBuffer, "DynPipe_%d", ++counter);
		if (ser.IsWriting())
		{
			for (GoalMap::iterator gi(m_mapGoals.begin()); gi != m_mapGoals.end(); ++gi)
			{
				if (!gi->second->IsDynamic())
					continue;
				ser.BeginOptionalGroup(nameBuffer, true);
				string tempstr = gi->first;
				ser.Value("PipeName", tempstr);
				gi->second->SerializeDynamic(ser);
				ser.EndGroup();
				drx_sprintf(nameBuffer, "DynPipe_%d", ++counter);
			}
			ser.BeginOptionalGroup(nameBuffer, false);
		}
		else
		{
			while (ser.BeginOptionalGroup(nameBuffer, true))
			{
				string name;
				ser.Value("PipeName", name);
				CGoalPipe* pNewPipe = static_cast<CGoalPipe*>(CreateGoalPipe(name, SilentlyReplaceDuplicate));
				pNewPipe->SerializeDynamic(ser);
				ser.EndGroup();
				drx_sprintf(nameBuffer, "DynPipe_%d", ++counter);
			}
		}
	}
	ser.EndGroup();
#endif // SERIALIZE_DYNAMIC_GOALPIPES
}

//
//-----------------------------------------------------------------------------------------------------------
struct CheckFuncCallScanDef
{
	i32   p;    // compare iterator
	tuk str;  // function name to detect
};

struct CheckFuncCall
{
	i32                 fileId; // index to file where the function call comes from
	i32                 tok;    // index to function call def
	std::vector<string> params; // function call params
};

struct CheckGoalpipe
{
	string              name;          // name of the pipe
	std::vector<string> embeddedPipes; // pipes that are embedded into this pipe when it is instantiated
	std::vector<i32>    usedInFile;    // index to files where the pipe is used
	i32                 fileId;        // index to file where the pipe is created
};

typedef std::map<string, CheckGoalpipe*> CheckPipeMap;

//
//-----------------------------------------------------------------------------------------------------------
static void ScanFileForFunctionCalls(tukk filename, i32 fileId, std::vector<CheckFuncCall>& calls,
                                     CheckFuncCallScanDef* scan, i32 nscan)
{
	FILE* fp = fxopen(filename, "rb");
	if (!fp) return;

	if (fseek(fp, 0, SEEK_END))
	{
		return;
	}

	i32 size = ftell(fp);

	if (fseek(fp, 0, SEEK_SET))
	{
		return;
	}

	if (size < 2)
	{
		fclose(fp);
		return;
	}

	tuk data = new char[size];
	if (!data)
	{
		fclose(fp);
		return;
	}
	fread(data, size, 1, fp);

	fclose(fp);

	tuk end = data + size;
	tuk str = data;
	tuk eol = 0;
	tuk param = 0;
	i32 state = 0;

	while (str != end)
	{
		// Skip white spaces in the beginning of the line.
		while (str != end && (*str == ' ' || *str == '\t'))
			str++;

		// Find the end of the line
		eol = str;
		while (eol != end && *eol != 0x0d && *eol != 0x0a) // advance until end of line
			++eol;
		while (eol != end && (*eol == 0x0d || *eol == 0x0a)) // include the eol markers into the line
			++eol;

		// Skip the line if it is a comment
		if (str != end && (str + 1) != end && str[0] == '-' && str[1] == '-')
		{
			str = eol;
			continue;
		}

		// Parse the line
		while (str != eol)
		{
			if (state == 0)
			{
				if (*str == '.' || *str == ':')
				{
					// Reset tokens
					for (i32 i = 0; i < nscan; ++i)
						scan[i].p = 0;
					state = 1; // Check token
				}
			}
			else if (state == 1)
			{
				// Check if a token starts.
				i32 passed = 0;
				for (i32 i = 0; i < nscan; ++i)
				{
					CheckFuncCallScanDef& t = scan[i];
					if (t.p == -1) continue; // invalid
					if (t.str[t.p] == *str)
					{
						++t.p;
						++passed;
						if (t.str[t.p] == '\0') // end of token?
						{
							param = 0;
							state = 2; // parse params

							calls.resize(calls.size() + 1);
							calls.back().fileId = fileId;
							calls.back().tok = i;
						}
					}
					else
						t.p = -1;
				}
				if (!passed)
					state = 0; // not a valid token, restart.
			}
			else if (state == 2)
			{
				// Scan until open bracket
				if (*str == '(')
				{
					state = 3; // scan params
				}
			}
			else if (state == 3)
			{
				if (*str == ',')
				{
					// Next param
					*str = '\0';
					if (param)
						calls.back().params.push_back(string(param));
					param = 0;
				}
				else if (*str == ')')
				{
					// End of func call
					*str = '\0';
					if (param)
						calls.back().params.push_back(string(param));
					param = 0;
					// Restart
					state = 0;
				}
				else
				{
					if (!param) // Init param start.
						param = str;
				}
			}
			++str;
		}

		// Proceed to new line.
		str = eol;
	}

	if (state != 0)
	{
		DrxLog("bad end of line! (state = %d)", state);
	}

	delete[] data;
}

//
//-----------------------------------------------------------------------------------------------------------
static void GetScriptFiles(const string& path, std::vector<string>& files)
{
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;

	string search(path);
	search += "/*.*";
	intptr_t handle = pDrxPak->FindFirst(search.c_str(), &fd);

	string filename;

	char fext[_MAX_EXT];

	if (handle != -1)
	{
		do
		{
			// Skip back folders.
			if (fd.name[0] == '.')
				continue;

			filename = path;
			filename += "/";
			filename += fd.name;

			if (fd.attrib & _A_SUBDIR)
			{
				// Recurse into subdir
				GetScriptFiles(filename, files);
			}
			else
			{
				// Check only lua files.
				_splitpath(fd.name, 0, 0, 0, fext);
				if (stricmp(fext, ".lua") != 0) continue;
				files.push_back(filename);
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);
		pDrxPak->FindClose(handle);
	}
}

//
//-----------------------------------------------------------------------------------------------------------
static bool ParseStringParam(i32 n, const std::vector<string>& params, string& out)
{
	out.clear();

	if (n >= (i32)params.size())
		return false;

	const string& p = params[n];

	size_t first = p.find_first_of('\"');
	if (first == p.npos)
		return false;
	size_t last = p.find_first_of('\"', first + 1);
	if (last == p.npos)
		return false;

	out = p.substr(first + 1, last - (first + 1));

	return true;
}

//
//-----------------------------------------------------------------------------------------------------------
static void ParseGoalpipes(const std::vector<CheckFuncCall>& calls, const std::vector<string>& files, CheckPipeMap& pipes)
{
	CGoalPipe tmp("", false); // used for finding goalop names

	i32 type = 0;
	CheckGoalpipe* pipe = 0;

	for (unsigned i = 0, ni = calls.size(); i < ni; ++i)
	{
		const CheckFuncCall& c = calls[i];

		if (c.tok == 0 || c.tok == 1) // AI.CreateGoalPipe(), AI.BeginGoalPipe()
		{
			if (pipe)
			{
				std::pair<CheckPipeMap::iterator, bool> res = pipes.insert(std::make_pair(pipe->name, pipe));
				if (!res.second)
					delete pipe;
				pipe = 0;
			}

			pipe = new CheckGoalpipe;
			if (ParseStringParam(0, c.params, pipe->name))
			{
				type = c.tok;
				pipe->fileId = c.fileId;
			}
			else
			{
				DrxLog("Warning: Goal pipe name not a string: '%s' (maybe a variable?) - %s",
				       c.params.empty() ? "<null>" : c.params[0].c_str(),
				       files[c.fileId].c_str());
				delete pipe;
				pipe = 0;
			}
		}
		else if (c.tok == 2) // AI.PushGoal()
		{
			if (!pipe)
				continue;
			string name;
			if (type == 0)
				ParseStringParam(1, c.params, name);
			else
				ParseStringParam(0, c.params, name);

			EGoalOperations op;
			if (name.size() > 1 && name[0] == '+')
				op = tmp.GetGoalOpEnum(name.c_str() + 1);
			else
				op = tmp.GetGoalOpEnum(name.c_str());

			if (op == eGO_LAST) // embedding another pipe
				pipe->embeddedPipes.push_back(name);
		}
	}

	if (pipe)
	{
		std::pair<CheckPipeMap::iterator, bool> res = pipes.insert(std::make_pair(pipe->name, pipe));
		if (!res.second)
			delete pipe;
		pipe = 0;
	}
}

//
//-----------------------------------------------------------------------------------------------------------
static void ParsePipeUsage(const std::vector<CheckFuncCall>& calls, const std::vector<string>& files, CheckPipeMap& pipes)
{
	for (unsigned i = 0, ni = calls.size(); i < ni; ++i)
	{
		const CheckFuncCall& c = calls[i];

		// The pipe name is always the second parameter for SelectPipe() and InsertSubpipe().
		string pipeName;
		ParseStringParam(1, c.params, pipeName);

		// Not using string to call the pipe.
		if (pipeName.size() < 1)
			continue;

		// Find the pipe.
		CheckPipeMap::iterator it = pipes.find(pipeName);
		if (it == pipes.end())
		{
			// Pipe not found
			DrxLog("Error: Cannot find pipe '%s' - %s",
			       pipeName.c_str(),
			       files[c.fileId].c_str());
		}
		else
		{
			// Pipe found, mark usage.
			CheckGoalpipe* pipe = it->second;
			stl::push_back_unique(pipe->usedInFile, c.fileId);
		}
	}

}
//
//-----------------------------------------------------------------------------------------------------------
static void MarkUsedEmbeddedPipe(CheckGoalpipe* pipe, CheckPipeMap& createdPipes,
                                 const std::vector<string>& files)
{
	for (unsigned i = 0, ni = pipe->embeddedPipes.size(); i < ni; ++i)
	{
		CheckPipeMap::iterator it = createdPipes.find(pipe->embeddedPipes[i]);

		if (it == createdPipes.end())
		{
			DrxLog("Error: Trying to embed invalid pipe '%s' into pipe '%s' - %s",
			       pipe->embeddedPipes[i].c_str(),
			       pipe->name.c_str(),
			       files[pipe->fileId].c_str());
		}
		else
		{
			CheckGoalpipe* embeddedPipe = it->second;
			stl::push_back_unique(embeddedPipe->usedInFile, pipe->fileId);

			// Recurse into children.
			MarkUsedEmbeddedPipe(embeddedPipe, createdPipes, files);
		}
	}
}

//
//-----------------------------------------------------------------------------------------------------------
void CPipeUpr::CheckGoalpipes()
{
	// Find all calls to goalpipes
	string path = PathUtil::Make(gEnv->pSystem->GetRootFolder(), "Game/Scripts");

	// Collect all scrip files.
	DrxLog("- Collecting files...");
	std::vector<string> files;
	GetScriptFiles(path, files);

	// Scan used goalpipes.
	std::vector<CheckFuncCall> pipesUsedCalls;
	CheckFuncCallScanDef useFuncs[] = {
		{ 0, "SelectPipe"    },
		{ 0, "InsertSubpipe" },
	};
	DrxLog("- Scanning used pipes...");
	for (unsigned i = 0, ni = files.size(); i < ni; ++i)
		ScanFileForFunctionCalls(files[i].c_str(), (i32)i, pipesUsedCalls, useFuncs, 2);

	// Scan created goalpipes
	std::vector<CheckFuncCall> pipesCreatedCalls;
	CheckFuncCallScanDef createFuncs[] = {
		{ 0, "CreateGoalPipe" },
		{ 0, "BeginGoalPipe"  },
		{ 0, "PushGoal"       },
		{ 0, "EndGoalPipe"    },
	};
	DrxLog("- Scanning created pipes...");
	for (unsigned i = 0, ni = files.size(); i < ni; ++i)
		ScanFileForFunctionCalls(files[i].c_str(), (i32)i, pipesCreatedCalls, createFuncs, 4);

	// Parse pipes
	CheckPipeMap createdPipes;
	ParseGoalpipes(pipesCreatedCalls, files, createdPipes);

	// Parse pipe usage
	ParsePipeUsage(pipesUsedCalls, files, createdPipes);

	// Check embedded pipes
	for (CheckPipeMap::iterator it = createdPipes.begin(), end = createdPipes.end(); it != end; ++it)
	{
		CheckGoalpipe* pipe = it->second;
		if (pipe->embeddedPipes.empty()) continue;

		// The pipe is used, mark the embedded pipes as used too.
		if (!pipe->usedInFile.empty())
			MarkUsedEmbeddedPipe(pipe, createdPipes, files);
	}

	DrxLog("\n");

	// Create a list of pipes per file for more intuitive output.
	std::vector<std::vector<CheckGoalpipe*>> unusedGoalsPerFile;
	unusedGoalsPerFile.resize(files.size());

	i32 unusedCount = 0;
	for (CheckPipeMap::iterator it = createdPipes.begin(), end = createdPipes.end(); it != end; ++it)
	{
		CheckGoalpipe* pipe = it->second;
		if (pipe->usedInFile.empty())
		{
			unusedGoalsPerFile[pipe->fileId].push_back(pipe);
			unusedCount++;
		}
	}

	// Output unused golapipes.
	for (unsigned i = 0, ni = unusedGoalsPerFile.size(); i < ni; ++i)
	{
		std::vector<CheckGoalpipe*>& pipes = unusedGoalsPerFile[i];
		if (pipes.empty()) continue;
		DrxLog("%" PRISIZE_T " ununsed pipes in %s", pipes.size(), files[i].c_str());
		for (unsigned j = 0, nj = pipes.size(); j < nj; ++j)
			DrxLog("    %s", pipes[j]->name.c_str());
	}

	DrxLog("\n");
	DrxLog("Unused goalpipes: %d of %" PRISIZE_T, unusedCount, createdPipes.size());

	// Cleanup
	for (CheckPipeMap::iterator it = createdPipes.begin(), end = createdPipes.end(); it != end; ++it)
		delete it->second;
}
