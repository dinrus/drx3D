// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _TEXTMESSAGES_H_
#define _TEXTMESSAGES_H_

// compact buffer to store text messages for a frame and render them each frame
// (replacement for the former PodArray<text_info_struct> m_listMessages[2], cleaner/more cache friendly,less memory,faster,typesafe)
// todo: can release memory in case this is needed
class CTextMessages
{
public:
	CTextMessages() : m_dwCurrentReadPos() {}

	class CTextMessageHeader;

	const CTextMessages& operator=(const CTextMessages& rhs);
	void                 Merge(const CTextMessages& rhs);

	// iteration should not be started yet
	// Arguments
	//   vPos - WorldSpace position
	//   szText - must not be 0
	//   nDrawFlags - EDrawTextFlags
	void PushEntry_Text(const Vec3& vPos, const ColorB col, IFFont* pFont, const Vec2& fFontSize, i32k nDrawFlags, tukk szText);

	// usually called every frame
	// resets/ends iteration
	void Clear(bool posonly = false);

	// todo: improve interface
	// starts the iteration
	// Returns
	//   0 if there are no more entries
	const CTextMessageHeader* GetNextEntry();

	u32                    ComputeSizeInMemory() const;

	//
	bool empty() const { return m_TextMessageData.empty(); }

	// -------------------------------------------------------------

	struct SText;

	class CTextMessageHeader
	{
	public:
		const SText* CastTo_Text() const { return (SText*)this; }

		u16       GetSize() const     { return m_Size; }

	protected:       // ---------------------------------------------
		u16 m_Size; // including attached text
	};

	// ---------------------------------------------

	struct SText : public CTextMessageHeader
	{
		void        Init(u32k paddedSize) { assert(paddedSize < 65535); m_Size = paddedSize; }
		tukk GetText() const               { return (tuk)this + sizeof(*this); }

		Vec3   m_vPos;
		ColorB m_Color;
		Vec2  m_fFontSize;
		u32 m_nDrawFlags;      // EDrawTextFlags
		IFFont* m_pFont;
	};

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_TextMessageData);
	}
private: // ------------------------------------------------------

	// each call the former returned pointer might be invalid
	// Arguments
	//   dwBytes >0 and dividable by 4
	// Returns
	//   0 if there is not enough space
	u8* PushData(u32k dwBytes);

	// ------------------------------------------------------

	std::vector<u8> m_TextMessageData;           // consists of many 4 byte aligned STextMessageHeader+ZeroTermintedText
	u32             m_dwCurrentReadPos;          // in bytes, !=0 interation started

	DrxCriticalSection m_TextMessageLock;
};

#endif // #ifndef _TEXTMESSAGES_H_
