// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/AdaptiveCompressor.h>

struct CAdaptiveCompressor::SDictNode
{
	SDictNode *prev;
	SDictNode *next;
	SDictNode *left;
	SDictNode *parent;
	SDictNode *right;
	SDictNode *hashNext;
	i32 count;
	i32 symbol;
};

// Quantisation objects - For passing to Compress/Decompress
class CAdaptiveCompressor::UInt16Delta
{
public:
	typedef u16 OutputType;
	ILINE UInt16Delta(float quantisation) : i(0) {}
	ILINE void PreCache(uk input) { i=*(u16*)input; }
	ILINE i32 Quantise() { return i; }
	ILINE OutputType Dequantise(i32 q) { return (OutputType)q; }
	ILINE i32 Delta(i32 a, i32 b) { return a-b; }
	ILINE i32 ApplyDelta(i32 a, i32 b) { return a+b; }
	ILINE void Smooth(uk input, i32 stride, i32 numInputs) {}
private:
	i32 i;
};

class CAdaptiveCompressor::ByteDelta
{
public:
	typedef u8 OutputType;
	ILINE ByteDelta(float quantisation) : i(0) {}
	ILINE void PreCache(uk input) { i=*(u8*)input; }
	ILINE i32 Quantise() { return i; }
	ILINE OutputType Dequantise(i32 q) { return (OutputType)q; }
	ILINE i32 Delta(i32 a, i32 b) { return a-b; }
	ILINE i32 ApplyDelta(i32 a, i32 b) { return a+b; }
	ILINE void Smooth(uk input, i32 stride, i32 numInputs) {}
private:
	i32 i;
};

class CAdaptiveCompressor::QuantisedFloatDelta
{
public:
	typedef float OutputType;
	ILINE QuantisedFloatDelta(float _quantisation) : i(0)
	{
		quantisation=_quantisation;
		invQuantisation=1.0f/quantisation;
	}
	ILINE void PreCache(uk input)
	{
		i=(i32)floorf(quantisation*(*(float*)input)+0.5f);
	}
	ILINE i32 Quantise()
	{
		return i;
	}
	ILINE OutputType Dequantise(i32 q) { return ((float)q)*invQuantisation; }
	ILINE i32 Delta(i32 a, i32 b) { return a-b; }
	ILINE i32 ApplyDelta(i32 a, i32 b) { return a+b; }
	ILINE void Smooth(uk input, i32 stride, i32 numInputs)
	{
		// Quantisation can cause some ugly stuttering in play back
		// This smooths the results using simple string pulling
		// Nb. Increases potential maximum error to invQuantisation instead of 0.5f*invQuantisation
		uk inputStart=input;
		float halfQuantisation=0.5f*invQuantisation;
		float *pSmoothed=new float[numInputs];
		for (i32 i=0; i<numInputs; i++)
		{
			pSmoothed[i]=*(float*)input;
			input=(tuk)input+stride;
		}
		for (i32 k=0; k<8; k++) // 8 samples = Filter out to ~0.25s either side
		{
			input=(tuk)inputStart+stride;
			for (i32 i=1; i<numInputs-1; i++)
			{
				float middle=(pSmoothed[i-1]+pSmoothed[i+1])*0.5f;
				float baseLine=*(float*)input;
				float fDelta=clamp_tpl(middle-baseLine, -halfQuantisation, halfQuantisation);
				pSmoothed[i]=baseLine+fDelta;
				input=(tuk)input+stride;
			}
		}
		input=inputStart;
		for (i32 i=0; i<numInputs; i++)
		{
			*(float*)input=pSmoothed[i];
			input=(tuk)input+stride;
		}
		delete[] pSmoothed;
	}
private:
	i32 i;
	float quantisation;
	float invQuantisation;
};
// End quantisation objects

CAdaptiveCompressor::CAdaptiveCompressor(uk outputBuffer, u32 outputBufferSize, i32 maxNodes, i32 numHashBuckets, bool bCompression)
{
	m_state.nodes=new SDictNode[maxNodes];
	m_state.lastAllocatedNode=&m_state.nodes[maxNodes];
	m_state.quantisation=0;
	m_stream.m_out=(CBitStream::StreamChunk*)outputBuffer;
	m_stream.m_end=(CBitStream::StreamChunk*)((tuk)outputBuffer+outputBufferSize);
	m_stream.m_mask=1;
	if (bCompression)
	{
		m_state.hashTable=new SDictNode*[numHashBuckets];
		m_state.numHashes=numHashBuckets;
		memset(outputBuffer, 0, outputBufferSize);
		assert(!(m_state.numHashes&(m_state.numHashes-1))); // Number of hash buckets must be power of two
	}
	else
	{
		m_state.hashTable=NULL;
		m_state.numHashes=0;
	}
}

CAdaptiveCompressor::~CAdaptiveCompressor()
{
	delete[] m_state.nodes;
	delete[] m_state.hashTable;
}

void CAdaptiveCompressor::ResetDictionary(float currentQuantisation)
{
	if (m_state.hashTable)
	{
		memset(m_state.hashTable, 0, sizeof(SDictNode*)*m_state.numHashes);
	}
	m_state.freeNode=m_state.nodes;
	m_state.rootNode=NULL;

	SDictNode *newSymbolNode=m_state.freeNode++;
	SDictNode *endNode=m_state.freeNode++;
	newSymbolNode->count=0;
	newSymbolNode->parent=NULL;
	newSymbolNode->left=NULL;
	newSymbolNode->right=NULL;
	newSymbolNode->next=endNode;
	newSymbolNode->prev=endNode;
	newSymbolNode->hashNext=NULL;
	endNode->next=newSymbolNode;
	endNode->prev=newSymbolNode;

	m_state.quantisation=currentQuantisation;
}

ILINE void CAdaptiveCompressor::RebuildTree(SDictNode *cur, SDictNode *lastNode, SDictNode *endNode, SDictNode*& root, bool bFullRebuild)
{
	SDictNode *next=cur->next;
	while (next!=endNode && next->count<cur->count) // Keep symbol list sorted in increasing probability order
		next=next->next;
	if (next!=cur->next)
	{
		cur->prev->next=cur->next;
		cur->next->prev=cur->prev;
		cur->next=next;
		cur->prev=next->prev;
		next->prev->next=cur;
		next->prev=cur;
	}
	if (bFullRebuild) // If we inserted a new symbol need to rebuild tree immediately else do it periodically to improve compression
	{
		SDictNode *symHead=endNode->next;
		SDictNode *childHead=lastNode;
		SDictNode *childTail=childHead;
		while (symHead!=endNode || childTail-childHead>1)
		{
			SDictNode *children[2];
			for (i32 i=0; i<2; i++)
			{
				if (symHead!=endNode && (childTail==childHead || symHead->count<childHead->count))
				{
					children[i]=symHead;
					symHead=symHead->next;
				}
				else
				{
					children[i]=childHead++;
				}
				children[i]->parent=childTail;
			}
			childTail->count=children[0]->count+children[1]->count;
			childTail->left=children[0];
			childTail->right=children[1];
			childTail->parent=NULL;
			childTail->next=NULL;
			childTail++;
		}
		root=childTail-1;
	}
}

template <class T>
void CAdaptiveCompressor::Compress(uk  __restrict pInput, u32 stride, u32 numInputs)
{
	u32k numHashesMinus1=m_state.numHashes-1;
	SDictNode *nodes=m_state.nodes;
	SDictNode **hashTable=m_state.hashTable;
	SDictNode *newSymNode=&nodes[0];
	SDictNode *endNode=&nodes[1];
	i32 updateMask=15;
	T quantiser(m_state.quantisation);

	// Check compressor was set up with hash buckets
	assert(hashTable && m_state.numHashes);

	// Use local versions of stream pointers for efficiency
	CBitStream::StreamChunk *outPtr=m_stream.m_out;
	CBitStream::StreamChunk outMask=m_stream.m_mask;

	if (numInputs==0) // Write nothing
		return;

	quantiser.PreCache(pInput);
	pInput=(tuk)pInput+stride;
	i32 sampleAfter=quantiser.Quantise();
	if (numInputs>1)
		quantiser.PreCache(pInput);
	pInput=(tuk)pInput+stride;

	m_stream.WriteVariableLengthValue(outPtr, outMask, sampleAfter);

	if (numInputs==1) // Seed value was the only value so now exit
	{
		m_stream.m_out=outPtr;
		m_stream.m_mask=outMask;
		return;
	}

	for (u32 i=0; i<numInputs-1; i++)
	{
		i32 sample=sampleAfter;
		sampleAfter=quantiser.Quantise();
		if (i+2<numInputs)
			quantiser.PreCache(pInput);
		pInput=(tuk)pInput+stride;
		i32 delta=quantiser.Delta(sampleAfter,sample);
		i32 bucket=delta&(numHashesMinus1);
		SDictNode *cur=hashTable[bucket];
		while (cur && cur->symbol!=delta)
			cur=cur->hashNext;
		if (!cur)
		{
			SDictNode *n=m_state.freeNode++;
			n->count=1;
			n->next=endNode->next;
			n->prev=endNode;
			n->next->prev=n;
			endNode->next=n;
			n->hashNext=hashTable[bucket];
			n->symbol=delta;
			hashTable[bucket]=n;
			cur=newSymNode;
		}
		if (m_state.rootNode) // First symbol will always be NEW_SYM so it's implied (rootNode is null until dictionary is first built)
		{
			i32 bits=0;
			SDictNode *n=cur;
			SDictNode *stack[128]; // stack used to reverse tree
			SDictNode **stackPtr=stack;
			while (n->parent)
			{
				stackPtr[0]=n;
				stackPtr++;
				bits++;
				n=n->parent;
			}
			while (stackPtr>stack)
			{
				--stackPtr;
				m_stream.WriteBit(outPtr, outMask, stackPtr[0]!=stackPtr[0]->parent->left);
			}
		}
		bool bNeedRebuild=!(i&updateMask);
		if (cur==newSymNode)
		{
			m_stream.WriteVariableLengthValue(outPtr, outMask, delta);
			bNeedRebuild=true;
		}
		cur->count++;
		RebuildTree(cur, m_state.freeNode, endNode, m_state.rootNode, bNeedRebuild);
		if (m_state.rootNode>=m_state.lastAllocatedNode) // If you hit this you didn't allocate enough nodes
		{
			DrxFatalError("Failed to allocate node during compression. Data must be unusual long or complex. Try increasing number of nodes allocated\n");
		}
	}

	m_stream.m_out=outPtr;
	m_stream.m_mask=outMask;
	if (m_stream.m_out>m_stream.m_end)
	{
		DrxFatalError("Overrun maximum size of compressed buffer. Pass in a bigger buffer\n");
	}
}

template <class T, class S>
void CAdaptiveCompressor::Decompress(S* pOutput, u32 stride, u32 numOutputs)
{
	SDictNode *nodes=m_state.nodes;
	SDictNode *newSymNode=&nodes[0];
	SDictNode *endNode=&nodes[1];
	i32 updateMask=15;
	S* pOrigOutput=pOutput;
	T quantiser(m_state.quantisation);
	i32 currentValue=0;

	// Use local versions of stream pointers for efficiency
	CBitStream::StreamChunk *outPtr=m_stream.m_out;
	CBitStream::StreamChunk outMask=m_stream.m_mask;

	if (numOutputs==0)
		return;

	currentValue=m_stream.ReadVariableLengthValue(outPtr, outMask);
	pOutput[0]=quantiser.Dequantise(currentValue);
	pOutput=(typename T::OutputType*)((tuk)pOutput+stride);

	if (numOutputs==1)
	{
		m_stream.m_out=outPtr;
		m_stream.m_mask=outMask;
		return;
	}

	for (u32 i=0; i<numOutputs-1; i++)
	{
		SDictNode *cur=newSymNode;
		if (m_state.rootNode) // First symbol will always be NEW_SYM so it's implied
		{
			cur=m_state.rootNode;
			while (cur->left!=NULL)
			{
				if (m_stream.ReadBit(outPtr, outMask))
					cur=cur->right;
				else
					cur=cur->left;
			}
		}
		bool bNeedRebuild=!(i&updateMask);
		if (cur==newSymNode)
		{
			i32 newSym=m_stream.ReadVariableLengthValue(outPtr, outMask);
			SDictNode *n=m_state.freeNode++;
			n->count=1;
			n->next=endNode->next;
			n->prev=endNode;
			n->left=NULL;
			n->next->prev=n;
			endNode->next=n;
			n->symbol=newSym;
			cur=&nodes[0];
			currentValue=quantiser.ApplyDelta(currentValue, newSym);
			bNeedRebuild=true;
		}
		else
		{
			currentValue=quantiser.ApplyDelta(currentValue, cur->symbol);
		}
		pOutput[0]=quantiser.Dequantise(currentValue);
		pOutput=(typename T::OutputType*)((tuk)pOutput+stride);
		cur->count++;
		RebuildTree(cur, m_state.freeNode, endNode, m_state.rootNode, bNeedRebuild);
		if (m_state.rootNode>=m_state.lastAllocatedNode) // If you hit this you didn't allocate enough nodes
		{
			DrxFatalError("Failed to allocate node during decompress! Should not be possible without also triggering similar assert in compressor\n");
		}
	}

	quantiser.Smooth(pOrigOutput, stride, numOutputs);

	m_stream.m_out=outPtr;
	m_stream.m_mask=outMask;
	if (m_stream.m_out>m_stream.m_end)
	{
		DrxFatalError("Decompression read more from the input stream than should have been possible\n");
	}
}

template void CAdaptiveCompressor::Compress<CAdaptiveCompressor::QuantisedFloatDelta>(uk  __restrict pInput, u32 stride, u32 numInputs);
template void CAdaptiveCompressor::Compress<CAdaptiveCompressor::UInt16Delta>(uk  __restrict pInput, u32 stride, u32 numInputs);
template void CAdaptiveCompressor::Compress<CAdaptiveCompressor::ByteDelta>(uk  __restrict pInput, u32 stride, u32 numInputs);
template void CAdaptiveCompressor::Decompress<CAdaptiveCompressor::QuantisedFloatDelta>(CAdaptiveCompressor::QuantisedFloatDelta::OutputType * pOutput, u32 stride, u32 numOutputs);
template void CAdaptiveCompressor::Decompress<CAdaptiveCompressor::UInt16Delta>(CAdaptiveCompressor::UInt16Delta::OutputType * pOutput, u32 stride, u32 numOutputs);
template void CAdaptiveCompressor::Decompress<CAdaptiveCompressor::ByteDelta>(CAdaptiveCompressor::ByteDelta::OutputType * pOutput, u32 stride, u32 numOutputs);
