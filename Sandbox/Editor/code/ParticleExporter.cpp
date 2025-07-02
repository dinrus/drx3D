// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "ParticleExporter.h"

#include "Util\PakFile.h"
#include "Util/DrxMemFile.h"
#include "Particles\ParticleItem.h"
#include "Particles\ParticleLibrary.h"
#include "Particles\ParticleManager.h"

//////////////////////////////////////////////////////////////////////////
// Signatures for particles.lst file.
#define PARTICLES_FILE_TYPE      2
#define PARTICLES_FILE_VERSION   4
#define PARTICLES_FILE_SIGNATURE "DRX"

#define PARTICLES_FILE           "LevelParticles.xml"

#define EFFECTS_PAK_FILE         "Effects.pak"

//////////////////////////////////////////////////////////////////////////
void CParticlesExporter::ExportParticles(const string& path, const string& levelName, CPakFile& levelPakFile)
{
	CParticleManager* pPartManager = GetIEditorImpl()->GetParticleManager();
	if (pPartManager->GetLibraryCount() == 0)
		return;

	// Effects pack.
	i32 i;
	for (i = 0; i < pPartManager->GetLibraryCount(); i++)
	{
		CParticleLibrary* pLib = (CParticleLibrary*)pPartManager->GetLibrary(i);
		if (pLib->IsLevelLibrary())
		{

			XmlNodeRef node = XmlHelpers::CreateXmlNode("ParticleLibrary");
			pLib->Serialize(node, false);
			XmlString xmlData = node->getXML();

			CDrxMemFile file;
			file.Write(xmlData.c_str(), xmlData.length());
			string filename = PathUtil::Make(path, PARTICLES_FILE);
			levelPakFile.UpdateFile(filename, file);
		}
	}

	/*
	   ISystem *pISystem = GetIEditorImpl()->GetSystem();

	   // If have more then onle library save them into shared Effects.pak
	   bool bNeedEffectsPak = pPartManager->GetLibraryCount() > 1;

	   bool bEffectsPak = true;

	   string pakFilename = EFFECTS_PAK_FILE;
	   CPakFile effectsPak;

	   if (bNeedEffectsPak)
	   {
	   // Close main game pak file if open.
	   if (!pISystem->GetIPak()->ClosePack( pakFilename ))
	   {
	    DrxLog( "Cannot close Pak file %s",(tukk)pakFilename );
	    bEffectsPak = false;
	   }

	   //////////////////////////////////////////////////////////////////////////
	   if (CFileUtil::OverwriteFile(pakFilename))
	   {
	    // Delete old pak file.
	    if (!effectsPak.Open( pakFilename,false ))
	    {
	      DrxLog( "Cannot open Pak file for Writing %s",(tukk)pakFilename );
	      bEffectsPak = false;
	    }
	   }
	   else
	    bEffectsPak = false;
	   }

	   // Effects pack.
	   i32 i;
	   for (i = 0; i < pPartManager->GetLibraryCount(); i++)
	   {
	   CParticleLibrary *pLib = (CParticleLibrary*)pPartManager->GetLibrary(i);
	   if (pLib->IsLevelLibrary())
	   {
	    CDrxMemFile file;
	    ExportParticleLib( pLib,file );
	    string filename = Path::Make( path,PARTICLES_FILE );
	    levelPakFile.UpdateFile( filename,file );
	   }
	   else
	   {
	    if (bEffectsPak)
	    {
	      CDrxMemFile file;
	      string filename = Path::Make( EFFECTS_FOLDER,pLib->GetName() + ".prt" );
	      ExportParticleLib( pLib,file );
	      effectsPak.UpdateFile( filename,file );
	    }
	   }
	   }

	   // Open Pak, which was closed before.
	   pISystem->GetIPak()->OpenPack( pakFilename );
	 */
}

