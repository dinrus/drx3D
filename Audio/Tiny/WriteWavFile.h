#ifndef D3_WRITE_WAV_FILE_H
#define D3_WRITE_WAV_FILE_H

#include <drxtypes.h>

// WriteWavFile is copied from Stk::FileWvOut/FileWrite
// See also https://github.com/thestk/stk
// by Perry R. Cook and Gary P. Scavone, 1995--2014.
#include <string>

class WriteWavFile
{
	void incrementFrame(void);
	void flush();

	struct WriteWavFileInternalData* m_data;

	void flushData(i32 bufferSize);

public:
	WriteWavFile();
	virtual ~WriteWavFile();

	bool setWavFile(STxt fileName, i32 sampleRate, i32 numChannels, bool useDoublePrecision = true);

	void closeWavFile();

	void tick(double* values, i32 numValues);
};

#endif  //D3_WRITE_WAV_FILE_H
