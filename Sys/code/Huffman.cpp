// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/******************************************************************************
** Huffman.cpp
** 19/06/2012
** Jake Turner (Ported by Rob Jessop)
******************************************************************************/

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/Huffman.h>

void HuffmanCoder::BitStreamBuilder::AddBits(u32 value, u32 numBits)
{
	if (numBits > 24)
	{
		AddBits(static_cast<u8>((value >> 24) & 0x000000ff), numBits - 24);
		numBits = 24;
	}
	if (numBits > 16)
	{
		AddBits(static_cast<u8>((value >> 16) & 0x000000ff), numBits - 16);
		numBits = 16;
	}
	if (numBits > 8)
	{
		AddBits(static_cast<u8>((value >> 8) & 0x000000ff), numBits - 8);
		numBits = 8;
	}
	AddBits(static_cast<u8>(value & 0x000000ff), numBits);
}

void HuffmanCoder::BitStreamBuilder::AddBits(u8 value, u32 numBits)
{
	if (m_mode != eM_WRITE)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Trying to write to a read only BitStreamBuilder");
		return;
	}
	u8 mask;
	mask = (u8)(1 << (numBits - 1));
	while (mask != 0)
	{
		//DrxLogAlways("mask is %u", mask);
		if (mask & value)
		{
			//DrxLogAlways("Buffer value was %u", *m_pBufferCursor.ptr);
			*(m_pBufferCursor.ptr) |= m_mask;
			//DrxLogAlways("Buffer value now %u", *m_pBufferCursor.ptr);
		}
		//DrxLogAlways("m_mask was %u", m_mask);
		m_mask = m_mask >> 1;
		//DrxLogAlways("m_mask now %u", m_mask);
		if (m_mask == 0)
		{
			if (m_pBufferCursor.ptr == m_pBufferEnd.ptr)
			{
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Bit Stream has consumed the last byte of the buffer and is requesting another. This stream will be truncated here.");
				return;
			}
			//DrxLogAlways("Buffer cursor was %u (%p)", *m_pBufferCursor.ptr, m_pBufferCursor.ptr);
			m_pBufferCursor.ptr++;
			//DrxLogAlways("Buffer cursor now %u (%p)", *m_pBufferCursor.ptr, m_pBufferCursor.ptr);
			m_mask = 0x80;
		}
		mask = mask >> 1L;
	}
}

//Returns 1 or 0 for valid values. Returns 2 if the buffer has run out or is the wrong type of builder.
u8 HuffmanCoder::BitStreamBuilder::GetBit()
{
	if (m_mode != eM_READ)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Trying to read from a write only BitStreamBuilder");
		return 2;
	}
	u8 value = 0;

	if (m_mask == 0)
	{
		if (m_pBufferCursor.const_ptr == m_pBufferEnd.const_ptr)
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Bit Stream has consumed the last byte of the buffer and is requesting another. This stream will be truncated here.");
			return 2;
		}
		//DrxLogAlways("Buffer cursor was %u (%p)", *m_pBufferCursor.const_ptr, m_pBufferCursor.const_ptr);
		m_pBufferCursor.const_ptr++;
		//DrxLogAlways("Buffer cursor now %u (%p)", *m_pBufferCursor.const_ptr, m_pBufferCursor.const_ptr);
		m_mask = 0x80;
	}
	if (m_mask & *(m_pBufferCursor.const_ptr))
	{
		value = 1;
	}
	//DrxLogAlways("m_mask was %u", m_mask);
	m_mask = m_mask >> 1;
	//DrxLogAlways("m_mask now %u", m_mask);

	return value;
}

void HuffmanCoder::Init()
{
	SAFE_DELETE_ARRAY(m_TreeNodes);
	SAFE_DELETE_ARRAY(m_Codes);
	m_Counts = new u32[MAX_NUM_SYMBOLS];
	memset(m_Counts, 0, sizeof(u32) * MAX_NUM_SYMBOLS);
	m_State = eHCS_OPEN;
}

//Adds the values of an array of chars to the counts
void HuffmanCoder::Update(u8k* const pSource, const size_t numBytes)
{
	if (m_State != eHCS_OPEN)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Trying to update a Huffman Coder that has not been initialized, or has been finalized");
		return;
	}

	size_t i;
	for (i = 0; i < numBytes; i++)
	{
		i32k symbol = pSource[i];
		m_Counts[symbol]++;
	}
}

void HuffmanCoder::Finalize()
{
	if (m_State != eHCS_OPEN)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Trying to finalize a Huffman Coder that has not been initialized, or has been finalized");
		return;
	}

	//Construct the tree
	m_TreeNodes = new HuffmanTreeNode[MAX_NUM_NODES];
	memset(m_TreeNodes, 0, sizeof(HuffmanTreeNode) * MAX_NUM_NODES);
	m_Codes = new HuffmanSymbolCode[MAX_NUM_CODES];
	memset(m_Codes, 0, sizeof(HuffmanSymbolCode) * MAX_NUM_CODES);

	ScaleCountsAndUpdateNodes();
	m_RootNode = BuildTree();
	ConvertTreeToCode(m_TreeNodes, m_Codes, 0, 0, m_RootNode);

	//Finalize the coder so that it won't accept any more strings
	m_State = eHCS_FINAL;

	//Counts are no longer needed
	SAFE_DELETE_ARRAY(m_Counts);
}

void HuffmanCoder::CompressInput(u8k* const pInput, const size_t numBytes, u8* const pOutput, size_t* const outputSize)
{
	BitStreamBuilder streamBuilder(pOutput, pOutput + (*outputSize));
	for (size_t i = 0; i < numBytes; i++)
	{
		i32k symbol = pInput[i];
		u32k value = m_Codes[symbol].value;
		u32k numBits = m_Codes[symbol].numBits;
		/*char szBits[33];
		   memset(szBits, '0', 33);
		   for( u32 j = 0; j < numBits; j++ )
		   {
		   if( (value & (u32)(1<<j)) != 0 )
		   {
		    szBits[31-j] = '1';
		   }
		   else
		   {
		    szBits[31-j] = '0';
		   }
		   }
		   szBits[32] = 0;
		   DrxLogAlways("%c - %s (%u)", value, szBits, numBits);*/
		streamBuilder.AddBits(value, numBits);
	}
	streamBuilder.AddBits(m_Codes[END_OF_STREAM].value, m_Codes[END_OF_STREAM].numBits);
	*outputSize = (streamBuilder.m_pBufferCursor.ptr - streamBuilder.m_pBufferStart.ptr) + 1;
}

size_t HuffmanCoder::UncompressInput(u8k* const pInput, const size_t numBytes, u8* const pOutput, const size_t maxOutputSize)
{
	size_t numOutputBytes = 0;
	BitStreamBuilder streamBuilder(pInput, pInput + numBytes);

	while (1)
	{
		i32 code;
		i32 node = m_RootNode;
		do
		{
			u8 bitValue = streamBuilder.GetBit();
#if 0
			DrxLogAlways("bit=%ld\n", bitValue);
#endif

			if (bitValue == 0)
			{
				node = m_TreeNodes[node].child0;
			}
			else
			{
				node = m_TreeNodes[node].child1;
			}
		}
		while (node > END_OF_STREAM);

		if (node == END_OF_STREAM)
		{
			pOutput[numOutputBytes] = '\0';
			break;
		}
		code = node;
#if 0
		{
			DrxLogAlways("%c", code);
			if (code == '\0')
			{
				DrxLogAlways("EOM");
			}
		}
#endif
		pOutput[numOutputBytes] = (char)code;
		numOutputBytes++;
		if (numOutputBytes >= maxOutputSize)
		{
			pOutput[maxOutputSize - 1] = '\0';
			break;
		}
	}

	return numOutputBytes;
}

//Private functions

void HuffmanCoder::ScaleCountsAndUpdateNodes()
{
	u64 maxCount = 0;
	i32 i;

	for (i = 0; i < MAX_NUM_SYMBOLS; i++)
	{
		const u64 count = m_Counts[i];
		if (count > maxCount)
		{
			maxCount = count;
		}
	}
	if (maxCount == 0)
	{
		m_Counts[0] = 1;
		maxCount = 1;
	}
	maxCount = maxCount / MAX_NUM_SYMBOLS;
	maxCount = maxCount + 1;
	for (i = 0; i < MAX_NUM_SYMBOLS; i++)
	{
		const u64 count = m_Counts[i];
		u32 scaledCount = (u32)(count / maxCount);
		if ((scaledCount == 0) && (count != 0))
		{
			scaledCount = 1;
		}
		m_TreeNodes[i].count = scaledCount;
		m_TreeNodes[i].child0 = END_OF_STREAM;
		m_TreeNodes[i].child1 = END_OF_STREAM;
	}
	m_TreeNodes[END_OF_STREAM].count = 1;
	m_TreeNodes[END_OF_STREAM].child0 = END_OF_STREAM;
	m_TreeNodes[END_OF_STREAM].child1 = END_OF_STREAM;
}

//Jake's file IO code. Kept in case we make the compression and table generation an offline task
/* Format is: startSymbol, stopSymbol, count0, count1, count2, ... countN, ..., 0 */
/* When finding the start, stop symbols only break out if find more than 3 0's in the counts */
/*static void outputCounts(FILE* const pFile, const HuffmanTreeNode* const pNodes)
   {
   i32 first = 0;
   i32 last;
   i32 next;

   while ((first < MAX_NUM_SYMBOLS) && (pNodes[first].count == 0))
   {
    first++;
   }
   last = first;
   next = first;
   for (; first < MAX_NUM_SYMBOLS; first = next)
   {
    i32 i;
    last = first+1;
    while (1)
    {
      for (; last < MAX_NUM_SYMBOLS; last++)
      {
        if (pNodes[last].count == 0)
        {
          break;
        }
      }
      last--;
      for (next = last+1; next < MAX_NUM_SYMBOLS; next++)
      {
        if (pNodes[next].count != 0)
        {
          break;
        }
      }
      if (next == MAX_NUM_SYMBOLS)
      {
        break;
      }
      if ((next-last) > 3)
      {
        break;
      }
      last = next;
    }
    putc(first, pFile);
    putc(last, pFile);
    for (i = first; i <= last; i++)
    {
      u32k count = pNodes[i].count;
      putc((i32)count, pFile);
    }
   }
   putc(0xFF, pFile);
   putc(0xFF, pFile);
   putc((i32)(pNodes[0xFF].count), pFile);
   }*/

/*static void inputCounts(FILE* const pFile, HuffmanTreeNode* const pNodes)
   {
   while (1)
   {
    i32 i;
    i32k first = getc(pFile);
    i32k last = getc(pFile);
    for (i = first; i <= last; i++)
    {
      i32k count = getc(pFile);
      pNodes[i].count = (size_t)count;
      pNodes[i].child0 = END_OF_STREAM;
      pNodes[i].child1 = END_OF_STREAM;
    }
    if ((first == last) && (first == 0xFF))
    {
      break;
    }
   }
   pNodes[END_OF_STREAM].count = 1;
   pNodes[END_OF_STREAM].child0 = END_OF_STREAM;
   pNodes[END_OF_STREAM].child1 = END_OF_STREAM;
   }*/

i32 HuffmanCoder::BuildTree()
{
	i32 min1;
	i32 min2;
	i32 nextFree;

	m_TreeNodes[MAX_NODE].count = 0xFFFFFFF;
	for (nextFree = END_OF_STREAM + 1;; nextFree++)
	{
		i32 i;
		min1 = MAX_NODE;
		min2 = MAX_NODE;
		for (i = 0; i < nextFree; i++)
		{
			u32k count = m_TreeNodes[i].count;
			if (count != 0)
			{
				if (count < m_TreeNodes[min1].count)
				{
					min2 = min1;
					min1 = i;
				}
				else if (count < m_TreeNodes[min2].count)
				{
					min2 = i;
				}
			}
		}
		if (min2 == MAX_NODE)
		{
			break;
		}
		m_TreeNodes[nextFree].count = m_TreeNodes[min1].count + m_TreeNodes[min2].count;

		m_TreeNodes[min1].savedCount = m_TreeNodes[min1].count;
		m_TreeNodes[min1].count = 0;

		m_TreeNodes[min2].savedCount = m_TreeNodes[min2].count;
		m_TreeNodes[min2].count = 0;

		m_TreeNodes[nextFree].child0 = min1;
		m_TreeNodes[nextFree].child1 = min2;
		m_TreeNodes[nextFree].savedCount = 0;
	}

	nextFree--;
	m_TreeNodes[nextFree].savedCount = m_TreeNodes[nextFree].count;

	return nextFree;
}

void HuffmanCoder::ConvertTreeToCode(const HuffmanTreeNode* const pNodes, HuffmanSymbolCode* const pCodes,
                                     u32k value, u32k numBits, i32k node)
{
	u32 nextValue;
	u32 nextNumBits;
	if (node <= END_OF_STREAM)
	{
		pCodes[node].value = value;
		pCodes[node].numBits = numBits;
		return;
	}
	nextValue = value << 1;
	nextNumBits = numBits + 1;
	ConvertTreeToCode(pNodes, pCodes, nextValue, nextNumBits, pNodes[node].child0);
	nextValue = nextValue | 0x1;
	ConvertTreeToCode(pNodes, pCodes, nextValue, nextNumBits, pNodes[node].child1);
}

/*static void printChar(i32k c)
   {
   if (c >= ' ' && c < 127)
   {
    printf("'%c'", c);
   }
   else
   {
    printf("0x%03X", c);
   }
   }

   static void printModel(const HuffmanTreeNode* const pNodes, const HuffmanSymbolCode* const pCodes)
   {
   i32 i;
   for (i = 0; i < MAX_NODE; i++)
   {
    u32k count = pNodes[i].savedCount;
    if (count != 0)
    {
      printf("node=");
      printChar(i);
      printf(" count=%3d", count);
      printf(" child0=");
      printChar(pNodes[i].child0);
      printf(" child1=");
      printChar(pNodes[i].child1);
      if (pCodes && (i <= END_OF_STREAM))
      {
        printf(" Huffman code=");
        binaryFilePrint(stdout, pCodes[i].value, pCodes[i].numBits);
      }
      printf("\n");
    }
   }
   }*/
