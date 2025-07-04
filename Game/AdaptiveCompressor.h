// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ADAPTIVECOMPRESSOR_H__
#define __ADAPTIVECOMPRESSOR_H__

// Generic Huffman Stream Compressor
// Features:
//  Compresses using an adaptive Huffman based compression scheme
//  Fully streamable: No finalisation needed whatever is written in can be decompressed immediately
//  Compressed stream is endian independent (Can be decompressed on any platform and output will be endian correct)
// Nodes:
//  Intended to compress deltas although a new QuatisationObject could be trivially written to compress values directly (change Delta()/ApplyDelta()) 

class CAdaptiveCompressor
{
public:
	struct SDictNode;

	// "QuantisationObjects": Classes to use with templated Compress/Decompress methods
	class QuantisedFloatDelta; // Quantises float by custom amount and then stores integer delta from last value
	class UInt16Delta;				 // Stores integer deltas between u16s
	class ByteDelta;					 // Stores integer deltas between uint8s
	
	struct SCompressionState
	{
		SDictNode *nodes;							// Dictionary nodes
		SDictNode *lastAllocatedNode; // End of allocated area
		SDictNode *rootNode;					// Root node of dictionary (used when decompressing)
		SDictNode *freeNode;					// End of dictionary symbols/start of structural dictionary nodes
		SDictNode **hashTable;				// Hash table (Bigger = better performance/worse memory use)
		i32 numHashes;								// Size of hash table
		float quantisation;						// Current quantisation value in use (quantised=(i32)floorf(value*quantisation+0.5f))
	};

	class CBitStream
	{
	public:
		typedef u8 StreamChunk; // Use byte sized chunks to make endian independent

		ILINE void WriteBit(i32 value) { WriteBit(m_out, m_mask, value); }
		ILINE i32 ReadBit() { return ReadBit(m_out, m_mask); }
		ILINE void WriteVariableLengthValue(i32 value) { WriteVariableLengthValue(m_out, m_mask, value); }
		ILINE i32 ReadVariableLengthValue() { return ReadVariableLengthValue(m_out, m_mask); }
		ILINE u8* GetEnd() const { return (u8*)((m_mask==1)?m_out:(m_out+1)); }
	
	protected:
		static ILINE void WriteBit(StreamChunk * __restrict &ptr, StreamChunk &mask, i32 value) 
		{
			i32 d=(mask>>(sizeof(StreamChunk)*8-1));
			if (value)
				*ptr|=mask;
			mask<<=1;
			mask|=d;
			ptr+=d;
		}

		static ILINE i32 ReadBit(StreamChunk * __restrict &ptr, StreamChunk &mask)
		{
			i32 d=(mask>>(sizeof(StreamChunk)*8-1));
			i32 ret=*ptr&mask;
			mask<<=1;
			mask|=d;
			ptr+=d;
			return ret;
		}

		static ILINE void WriteVariableLengthValue(StreamChunk * __restrict &ptr, StreamChunk &mask, i32 value)
		{
			i32 absValue=abs(value);
			i32 shift=1;
			while (absValue>=(shift<<3))
			{
				shift<<=4;
			}
			while (shift)
			{
				WriteBit(ptr, mask, value&(shift<<0));
				WriteBit(ptr, mask, value&(shift<<1));
				WriteBit(ptr, mask, value&(shift<<2));
				WriteBit(ptr, mask, value&(shift<<3));
				shift>>=4;
				WriteBit(ptr, mask, shift);
			}
		}

		static ILINE i32 ReadVariableLengthValue(StreamChunk * __restrict &ptr, StreamChunk &mask)
		{
			i32 shift=1;
			i32 value=0;
			do
			{
				value<<=4;
				shift<<=4;
				value|=ReadBit(ptr, mask)?1:0;
				value|=ReadBit(ptr, mask)?2:0;
				value|=ReadBit(ptr, mask)?4:0;
				value|=ReadBit(ptr, mask)?8:0;
			} while (ReadBit(ptr, mask));
			if (value&(shift>>1)) // sign extend needed
			{
				u32 smask=(1<<31);
				while (!(value&(smask)))
				{
					value|=smask;
					smask>>=1;
				}
			}
			return value;
		}

	public:
		StreamChunk * __restrict m_out;	// Next byte that will be written to
		StreamChunk * m_end;						// Last chunk in stream (Used to check for overrun)
		StreamChunk m_mask;							// Signifies which bit will be written to next

		friend class CAdaptiveCompressor;
	};

	SCompressionState m_state;
	CBitStream m_stream;

	// CAdaptiveCompressor(outputBuffer, outputBufferSize, maxNodes, numHasBuckets)
	//  outputBuffer:     Where the compressed data will be written to
	//  outputBufferSize: Amount of space allocated to output buffer
	//  maxNodes:         Maximum dictionary nodes you need (Approximately 2 nodes are need per unique delta)
	//  numHashBuckets:   Used to increase performance of dictionary lookup during compression (Use ~1 per unique delta)
	//  bCompression:     Set this if you intend to use this as a compressor (If true allocates hashes and memsets output buffer)
	CAdaptiveCompressor(uk  __restrict outputBuffer, u32 outputBufferSize, i32 maxNodes, i32 numHashBuckets, bool bCompression);
	~CAdaptiveCompressor();

	// ResetDictionary(currentQuantisation)
	//  Causes dictionary to reset. Stream from then on will be independent from what went on before
	//  You should reset if you expect to pass in a dataset that's dissimilar to what you've already compressed
	//  Example: Passing components of a Vec3/Quat should be done without resetting in between as delta will be similar
	//  currentQuantisation: Float is passed to QuatisationObject when compressing/decompressing
	void ResetDictionary(float currentQuantisation);

	// Compress(pInput, stride, numInputs)
	//  T:         QuantisationObject ie QuantisedFloatDelta/UInt16Delta etc
	//  pInput:    Pointer to first input (QuantisationObject determines how this is interpreted)
	//  stride:    Stride between inputs in bytes
	//  numInputs: Number of inputs in stream
	template <class T>
	void Compress(uk  __restrict pInput, u32 stride, u32 numInputs);

	// Decompress(pOutput, stride, numOutputs)
	//  T:         QuantisationObject ie QuantisedFloatDelta/UInt16Delta etc
	//  pOutput:   Pointer to first output (type should match QuantisationObject::OutputType)
	//  stride:    Stride between outputs in bytes
	//  numInputs: Number of outputs that are expected
	template <class T, class S>
	void Decompress(S* pOutput, u32 stride, u32 numOutputs);

private:

	static ILINE void RebuildTree(SDictNode *cur, SDictNode *lastNode, SDictNode *endNode, SDictNode*& root, bool bFullRebuild);
};

#endif // __ADAPTIVECOMPRESSOR_H__