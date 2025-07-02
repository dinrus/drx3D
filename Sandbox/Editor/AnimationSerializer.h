// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __animationserializer_h__
#define __animationserializer_h__

#if _MSC_VER > 1000
	#pragma once
#endif

//  предварительные объявления.
struct IAnimSequence;
class CPakFile;

/** Used by Editor to serialize animation data.
 */
class CAnimationSerializer
{
public:
	CAnimationSerializer();
	~CAnimationSerializer();

	/** Save all animation sequences to files in given directory.
	 */
	void SerializeSequences(XmlNodeRef& xmlNode, bool bLoading);

	/** Saves single animation sequence to file in given directory.
	 */
	void SaveSequence(IAnimSequence* seq, tukk szFilePath, bool bSaveEmpty = true);

	/** Load sequence from file.
	 */
	IAnimSequence* LoadSequence(tukk szFilePath);

	/** Save all animation sequences to files in given directory.
	 */
	void SaveAllSequences(tukk szPath, CPakFile& pakFile);

	/** Load all animation sequences from given directory.
	 */
	void LoadAllSequences(tukk szPath);
};

#endif // __animationserializer_h__

