#ifndef D3_READ_WAV_FILE_H
#define D3_READ_WAV_FILE_H

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <stdio.h>
#include <string.h>

struct b3WavTicker
{
	b3AlignedObjectArray<double> lastFrame_;
	bool finished_;
	double time_;
	double rate_;
};

class b3ReadWavFile
{
	bool byteswap_;
	bool wavFile_;
	u64 m_numFrames;
	u64 dataType_;
	double fileDataRate_;
	FILE *fd_;
	u64 dataOffset_;
	u32 channels_;
	bool m_machineIsLittleEndian;

public:
	b3ReadWavFile();
	virtual ~b3ReadWavFile();

	b3AlignedObjectArray<double> m_frames;

	bool getWavInfo(tukk fileName);

	void normalize(double peak);

	double interpolate(double frame, u32 channel) const;
	double tick(u32 channel, b3WavTicker *ticker);

	void resize();

	b3WavTicker createWavTicker(double sampleRate);

	bool read(u64 startFrame, bool doNormalize);

	i32 getNumFrames() const
	{
		return m_numFrames;
	}
};

#endif  //D3_READ_WAV_FILE_H
