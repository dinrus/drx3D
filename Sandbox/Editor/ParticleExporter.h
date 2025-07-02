// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __particleexporter_h__
#define __particleexporter_h__
#pragma once

//  предварительные объявления.
class CPakFile;

/*! Class responsible for exporting particles to game format.
 */
/** Export brushes from specified Indoor to .bld file.
 */
class CParticlesExporter
{
public:
	void ExportParticles(const string& path, const string& levelName, CPakFile& pakFile);
private:
};

#endif // __particleexporter_h__

