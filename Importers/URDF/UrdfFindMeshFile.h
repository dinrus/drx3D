
#ifndef URDF_FIND_MESH_FILE_H
#define URDF_FIND_MESH_FILE_H

#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/Common/b3Logging.h>

#include <string>
#include <list>

#include "UrdfParser.h"

static bool UrdfFindMeshFile(
	CommonFileIOInterface* fileIO,
	const STxt& urdf_path, STxt fn,
	const STxt& error_message_prefix,
	STxt* out_found_filename, i32* out_type)
{
	if (fn.size() <= 4)
	{
		drx3DWarning("%s: invalid mesh filename '%s'\n", error_message_prefix.c_str(), fn.c_str());
		return false;
	}

	STxt ext;
	STxt ext_ = fn.substr(fn.size() - 4);
	for (STxt::iterator i = ext_.begin(); i != ext_.end(); ++i)
	{
		ext += char(tolower(*i));
	}

	if (ext == ".dae")
	{
		*out_type = UrdfGeometry::FILE_COLLADA;
	}
	else if (ext == ".stl")
	{
		*out_type = UrdfGeometry::FILE_STL;
	}
	else if (ext == ".obj")
	{
		*out_type = UrdfGeometry::FILE_OBJ;
	}
	else if (ext == ".cdf")
	{
		*out_type = UrdfGeometry::FILE_CDF;
	}
	else if (ext == ".vtk")
	{
		*out_type = UrdfGeometry::FILE_VTK;
	}
	else
	{
		drx3DWarning("%s: invalid mesh filename extension '%s'\n", error_message_prefix.c_str(), ext.c_str());
		return false;
	}

	STxt drop_it_file = "file://";
	STxt drop_it_pack = "package://";
	STxt drop_it_model = "model://";
	if (fn.substr(0, drop_it_file.length()) == drop_it_file)
		fn = fn.substr(drop_it_file.length());
	if (fn.substr(0, drop_it_pack.length()) == drop_it_pack)
		fn = fn.substr(drop_it_pack.length());
	else if (fn.substr(0, drop_it_model.length()) == drop_it_model)
		fn = fn.substr(drop_it_model.length());

	std::list<STxt> shorter;
	shorter.push_back("../../");
	shorter.push_back("../");
	shorter.push_back("./");
	i32 cnt = urdf_path.size();
	for (i32 i = 0; i < cnt; ++i)
	{
		if (urdf_path[i] == '/' || urdf_path[i] == '\\')
		{
			shorter.push_back(urdf_path.substr(0, i) + "/");
		}
	}
	shorter.push_back("");  // no prefix
	shorter.reverse();

	STxt existing_file;

	for (std::list<STxt>::iterator x = shorter.begin(); x != shorter.end(); ++x)
	{
		STxt attempt = *x + fn;
		i32 f = fileIO->fileOpen(attempt.c_str(), "rb");
		if (f<0)
		{
			//drx3DPrintf("%s: tried '%s'", error_message_prefix.c_str(), attempt.c_str());
			continue;
		}
		fileIO->fileClose(f);
		existing_file = attempt;
		//drx3DPrintf("%s: found '%s'", error_message_prefix.c_str(), attempt.c_str());
		break;
	}

	if (existing_file.empty())
	{
		drx3DWarning("%s: cannot find '%s' in any directory in urdf path\n", error_message_prefix.c_str(), fn.c_str());
		return false;
	}
	else
	{
		*out_found_filename = existing_file;
		return true;
	}
}

#endif //URDF_FIND_MESH_FILE_H
