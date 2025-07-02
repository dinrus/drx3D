// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "animationserializer.h"
#include "DrxEditDoc.h"
#include "mission.h"
#include <DrxMovie/IMovieSystem.h>

#include "Util\PakFile.h"
#include "Util/DrxMemFile.h"

CAnimationSerializer::CAnimationSerializer(void)
{
}

CAnimationSerializer::~CAnimationSerializer(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::SaveSequence(IAnimSequence* seq, tukk szFilePath, bool bSaveEmpty)
{
	assert(seq != 0);
	XmlNodeRef sequenceNode = XmlHelpers::CreateXmlNode("Sequence");
	seq->Serialize(sequenceNode, false, false);
	XmlHelpers::SaveXmlNode(sequenceNode, szFilePath);
}

IAnimSequence* CAnimationSerializer::LoadSequence(tukk szFilePath)
{
	IAnimSequence* seq = 0;
	XmlNodeRef sequenceNode = XmlHelpers::LoadXmlFromFile(szFilePath);
	if (sequenceNode)
	{
		seq = GetIEditorImpl()->GetMovieSystem()->LoadSequence(sequenceNode);
	}
	return seq;
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::SaveAllSequences(tukk szPath, CPakFile& pakFile)
{
	IMovieSystem* movSys = GetIEditorImpl()->GetMovieSystem();
	XmlNodeRef movieNode = XmlHelpers::CreateXmlNode("MovieData");
	for (i32 i = 0; i < GetIEditorImpl()->GetDocument()->GetMissionCount(); i++)
	{
		CMission* pMission = GetIEditorImpl()->GetDocument()->GetMission(i);
		pMission->ExportAnimations(movieNode);
	}
	string sFilename = string(szPath) + "MovieData.xml";
	//XmlHelpers::SaveXmlNode(movieNode,sFilename.c_str());

	XmlString xml = movieNode->getXML();
	CDrxMemFile file;
	file.Write(xml.c_str(), xml.length());
	pakFile.UpdateFile(sFilename.c_str(), file);
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::LoadAllSequences(tukk szPath)
{
	string dir = PathUtil::AddBackslash(szPath);
	std::vector<CFileUtil::FileDesc> files;
	CFileUtil::ScanDirectory(dir, "*.seq", files, false);

	for (i32 i = 0; i < files.size(); i++)
	{
		// Construct the full filepath of the current file
		LoadSequence(dir + files[i].filename.GetString());
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::SerializeSequences(XmlNodeRef& xmlNode, bool bLoading)
{
	if (bLoading)
	{
		// Load.
		IMovieSystem* movSys = GetIEditorImpl()->GetMovieSystem();
		movSys->RemoveAllSequences();
		i32 num = xmlNode->getChildCount();
		for (i32 i = 0; i < num; i++)
		{
			XmlNodeRef seqNode = xmlNode->getChild(i);
			movSys->LoadSequence(seqNode);
		}
	}
	else
	{
		// Save.
		IMovieSystem* movSys = GetIEditorImpl()->GetMovieSystem();
		for (i32 i = 0; i < movSys->GetNumSequences(); ++i)
		{
			IAnimSequence* seq = movSys->GetSequence(i);
			XmlNodeRef seqNode = xmlNode->newChild("Sequence");
			seq->Serialize(seqNode, false);
		}
	}
}

