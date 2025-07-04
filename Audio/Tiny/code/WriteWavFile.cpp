// WriteWavFile is copied from Stk::FileWvOut/FileWrite
// See also https://github.com/thestk/stk
// by Perry R. Cook and Gary P. Scavone, 1995--2014.

#include "../WriteWavFile.h"
#include <drx3D/Common/b3AlignedObjectArray.h>
#include "../SwapUtils.h"

#define D3_FLOAT32 32
#define D3_FLOAT64 64

// WAV header structure. See
// http://www-mmsp.ece.mcgill.ca/documents/audioformats/WAVE/Docs/rfc2361.txt
// for information regarding format codes.
struct b3WaveHeader
{
	char riff[4];   // "RIFF"
	i32 fileSize;   // in bytes
	char wave[4];   // "WAVE"
	char fmt[4];    // "fmt "
	i32 chunkSize;  // in bytes (16 for PCM)
	union {
		signed short formatCode;  // 1=PCM, 2=ADPCM, 3=IEEE float, 6=A-Law, 7=Mu-Law
		unsigned short uformatCode;
	};
	signed short nChannels;  // 1=mono, 2=stereo
	i32 sampleRate;
	i32 bytesPerSecond;
	signed short bytesPerSample;  // 2=16-bit mono, 4=16-bit stereo
	signed short bitsPerSample;
	signed short cbSize;     // size of extension
	signed short validBits;  // valid bits per sample
	i32 channelMask;         // speaker position mask
	char subformat[16];      // format code and GUID
	char fact[4];            // "fact"
	i32 factSize;            // fact chunk size
	i32 frames;              // sample frames
};

struct WriteWavFileInternalData
{
	FILE *m_file;
	i32 m_numChannels;
	i32 m_sampleRate;
	i32 m_dataType;  // single precision 32bit float, 64bit double
	bool m_byteswap;
	i32 m_frameCounter;
	i32 m_bufferIndex;
	i32 m_bufferSize;
	bool m_clipped;
	bool m_isMachineLittleEndian;

	b3AlignedObjectArray<float> m_floatBuffer;
	b3AlignedObjectArray<double> m_doubleBuffer;

	WriteWavFileInternalData()
		: m_file(0),
		  m_numChannels(0),
		  m_dataType(D3_FLOAT32),
		  m_byteswap(false),
		  m_frameCounter(0),
		  m_bufferIndex(0),
		  m_bufferSize(1024),
		  m_clipped(false)
	{
		m_floatBuffer.reserve(m_bufferSize);
		m_doubleBuffer.reserve(m_bufferSize);
		m_isMachineLittleEndian = b3MachineIsLittleEndian();
	}
};

WriteWavFile::WriteWavFile()
{
	m_data = new WriteWavFileInternalData();
}

WriteWavFile::~WriteWavFile()
{
	closeWavFile();
	delete m_data;
}

bool WriteWavFile::setWavFile(STxt fileName, i32 sampleRate, i32 numChannels, bool useDoublePrecision)
{
	m_data->m_numChannels = numChannels;
	m_data->m_sampleRate = sampleRate;
	if (useDoublePrecision)
	{
		m_data->m_dataType = D3_FLOAT64;
	}
	else
	{
		m_data->m_dataType = D3_FLOAT32;
	}

	if (fileName.find(".wav") == STxt::npos)
		fileName += ".wav";

	m_data->m_file = fopen(fileName.c_str(), "wb");
	if (!m_data->m_file)
	{
		return false;
	}

	struct b3WaveHeader hdr = {{'R', 'I', 'F', 'F'}, 44, {'W', 'A', 'V', 'E'}, {'f', 'm', 't', ' '}, 16, 1, 1, sampleRate, 0, 2, 16, 0, 0, 0, {'\x01', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x00', '\x80', '\x00', '\x00', '\xAA', '\x00', '\x38', '\x9B', '\x71'}, {'f', 'a', 'c', 't'}, 4, 0};
	hdr.nChannels = (signed short)m_data->m_numChannels;

	if (m_data->m_dataType == D3_FLOAT32)
	{
		hdr.formatCode = 3;
		hdr.bitsPerSample = 32;
	}
	else if (m_data->m_dataType == D3_FLOAT64)
	{
		hdr.formatCode = 3;
		hdr.bitsPerSample = 64;
	}

	hdr.bytesPerSample = (signed short)(m_data->m_numChannels * hdr.bitsPerSample / 8);
	hdr.bytesPerSecond = (i32)(hdr.sampleRate * hdr.bytesPerSample);

	u32 bytesToWrite = 36;
	if (m_data->m_numChannels > 2 || hdr.bitsPerSample > 16)
	{  // use extensible format
		bytesToWrite = 72;
		hdr.chunkSize += 24;
		hdr.uformatCode = 0xFFFE;
		hdr.cbSize = 22;
		hdr.validBits = hdr.bitsPerSample;
		signed short *subFormat = (signed short *)&hdr.subformat[0];
		if (m_data->m_dataType == D3_FLOAT32 || m_data->m_dataType == D3_FLOAT64)
			*subFormat = 3;
		else
			*subFormat = 1;
	}

	m_data->m_byteswap = false;
	if (!m_data->m_isMachineLittleEndian)
	{
		m_data->m_byteswap = true;
		b3Swap32((u8*)&hdr.chunkSize);
		b3Swap16((u8*)&hdr.formatCode);
		b3Swap16((u8*)&hdr.nChannels);
		b3Swap32((u8*)&hdr.sampleRate);
		b3Swap32((u8*)&hdr.bytesPerSecond);
		b3Swap16((u8*)&hdr.bytesPerSample);
		b3Swap16((u8*)&hdr.bitsPerSample);
		b3Swap16((u8*)&hdr.cbSize);
		b3Swap16((u8*)&hdr.validBits);
		b3Swap16((u8*)&hdr.subformat[0]);
		b3Swap32((u8*)&hdr.factSize);
	}

	char data[4] = {'d', 'a', 't', 'a'};
	i32 dataSize = 0;
	if (fwrite(&hdr, 1, bytesToWrite, m_data->m_file) != bytesToWrite)
		return false;
	if (fwrite(&data, 4, 1, m_data->m_file) != 1)
		return false;
	if (fwrite(&dataSize, 4, 1, m_data->m_file) != 1)
		return false;

	return true;
}

void WriteWavFile::closeWavFile()
{
	if (m_data->m_file == 0)
		return;

	flushData(1);

	i32 bytesPerSample = 1;
	if (m_data->m_dataType == D3_FLOAT32)
		bytesPerSample = 4;
	else if (m_data->m_dataType == D3_FLOAT64)
		bytesPerSample = 8;

	bool useExtensible = false;
	i32 dataLocation = 40;
	if (bytesPerSample > 2 || m_data->m_numChannels > 2)
	{
		useExtensible = true;
		dataLocation = 76;
	}

	i32 bytes = (i32)(m_data->m_frameCounter * m_data->m_numChannels * bytesPerSample);
	if (bytes % 2)
	{  // pad extra byte if odd
		signed char sample = 0;
		fwrite(&sample, 1, 1, m_data->m_file);
	}
#ifndef __LITTLE_ENDIAN__
	b3Swap32((u8*)&bytes);
#endif
	fseek(m_data->m_file, dataLocation, SEEK_SET);  // jump to data length
	fwrite(&bytes, 4, 1, m_data->m_file);

	bytes = (i32)(m_data->m_frameCounter * m_data->m_numChannels * bytesPerSample + 44);
	if (useExtensible) bytes += 36;
#ifndef __LITTLE_ENDIAN__
	b3Swap32((u8*)&bytes);
#endif
	fseek(m_data->m_file, 4, SEEK_SET);  // jump to file size
	fwrite(&bytes, 4, 1, m_data->m_file);

	if (useExtensible)
	{  // fill in the "fact" chunk frames value
		bytes = (i32)m_data->m_frameCounter;
#ifndef __LITTLE_ENDIAN__
		b3Swap32((u8*)&bytes);
#endif
		fseek(m_data->m_file, 68, SEEK_SET);
		fwrite(&bytes, 4, 1, m_data->m_file);
	}

	fclose(m_data->m_file);
	m_data->m_file = 0;
}

void WriteWavFile::tick(double *frames, i32 numFrames)
{
	i32 iFrames = 0;
	i32 j, nChannels = m_data->m_numChannels;

	for (i32 i = 0; i < numFrames; i++)
	{
		for (j = 0; j < nChannels; j++)
		{
			double sample = frames[iFrames++];
			if (sample < -1.)
			{
				sample = -1.;
				m_data->m_clipped = true;
			}
			if (sample > 1)
			{
				sample = 1.;
				m_data->m_clipped = true;
			}

			if (m_data->m_dataType == D3_FLOAT32)
			{
				m_data->m_floatBuffer.push_back((float)sample);
			}
			else
			{
				m_data->m_doubleBuffer.push_back(sample);
			}

			flushData(m_data->m_bufferSize);
		}

		m_data->m_frameCounter++;
	}
}

void WriteWavFile::flushData(i32 bufferSize)
{
	if (m_data->m_dataType == D3_FLOAT32)
	{
		if (m_data->m_floatBuffer.size() >= bufferSize)
		{
			fwrite(&m_data->m_floatBuffer[0], sizeof(float), m_data->m_floatBuffer.size(), m_data->m_file);
			m_data->m_floatBuffer.resize(0);
		}
	}
	else
	{
		if (m_data->m_doubleBuffer.size() >= bufferSize)
		{
			fwrite(&m_data->m_doubleBuffer[0], sizeof(double), m_data->m_doubleBuffer.size(), m_data->m_file);
			m_data->m_doubleBuffer.resize(0);
		}
	}
}
