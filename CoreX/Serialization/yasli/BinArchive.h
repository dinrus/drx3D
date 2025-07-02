/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

// Tags are 16-bit xor-hashes, checked for uniqueness in debug.
// Block is automatic: 8, 16 or 32-bits

#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/MemoryWriter.h> 
#include <vector>

namespace yasli{

inline unsigned short calcHash(tukk str)
{
	u32 hash = 1315423911;

	while(*str)
		hash ^= (hash << 5) + *str++ + (hash >> 2);

	return static_cast<unsigned short>(hash);
}

class BinOArchive : public Archive{
public:

	YASLI_INLINE BinOArchive();
	YASLI_INLINE ~BinOArchive() {}

	YASLI_INLINE void clear();
	YASLI_INLINE size_t length() const;
	tukk buffer() const { return stream_.buffer(); }
	YASLI_INLINE bool save(tukk fileName);

	YASLI_INLINE bool operator()(bool& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(StringInterface& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(WStringInterface& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(float& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(double& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i8& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i16& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i32& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i64& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u8& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u16& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u32& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u64& value, tukk name, tukk label) override;

	YASLI_INLINE bool operator()(char& value, tukk name, tukk label) override;

	YASLI_INLINE bool operator()(const Serializer &ser, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(ContainerInterface &ser, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(PointerInterface &ptr, tukk name, tukk label) override;

	using Archive::operator();

private:
	YASLI_INLINE void openContainer(tukk name, i32 size, tukk typeName);
	YASLI_INLINE void openNode(tukk name, bool size8 = true);
	YASLI_INLINE void closeNode(tukk name, bool size8 = true);

	std::vector<u32> blockSizeOffsets_;
	MemoryWriter stream_;

#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	enum BlockType { UNDEFINED, POD, NON_POD };
	std::vector<BlockType> blockTypes_;
#endif
};

//////////////////////////////////////////////////////////////////////////

class BinIArchive : public Archive{
public:

	YASLI_INLINE BinIArchive();
	YASLI_INLINE ~BinIArchive();

	YASLI_INLINE bool load(tukk fileName);
	YASLI_INLINE bool open(tukk buffer, size_t length); // Do not copy buffer!
	YASLI_INLINE bool open(const BinOArchive& ar) { return open(ar.buffer(), ar.length()); }
	YASLI_INLINE void close();

	YASLI_INLINE bool operator()(bool& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(char& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(float& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(double& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i8& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u8& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i16& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u16& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i32& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u32& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(i64& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(u64& value, tukk name, tukk label) override;


	YASLI_INLINE bool operator()(StringInterface& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(WStringInterface& value, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(const Serializer& ser, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(ContainerInterface& ser, tukk name, tukk label) override;
	YASLI_INLINE bool operator()(PointerInterface& ptr, tukk name, tukk label) override;

	using Archive::operator();

private:
	class Block
	{
	public:
		Block(tukk data, i32 size) : 
		  begin_(data), curr_(data), end_(data + size), complex_(false) {}

		  YASLI_INLINE bool get(tukk name, Block& block);

		  void read(uk data, i32 size)
		  {
			  if(curr_ + size <= end_){
					memcpy(data, curr_, size);
					curr_ += size;	
			  }
			  else
				  YASLI_ASSERT(0);
		  }

		  template<class T>
		  void read(T& x){ read(&x, sizeof(x)); }

		  void read(string& s)
		  {
			  if(curr_ + strlen(curr_) < end_){
				s = curr_;
				curr_ += strlen(curr_) + 1;
			  }
			  else{
				  YASLI_ASSERT(0);
				  curr_ = end_;
			  }
		  }
		  void read(wstring& s)
		  {
			  // make sure that accessed wchar_t is always aligned
			  tukk strEnd = curr_;
			  const wchar_t nullChar = L'\0';
			  while (strEnd < end_) {
				  if (memcmp(strEnd, &nullChar, sizeof(nullChar)) == 0)
					  break;
				  strEnd += sizeof(wchar_t);
			  }
			  i32 len = i32((strEnd - curr_) / sizeof(wchar_t));

			  s.resize(len);
			  if (len)
				  memcpy((uk )&s[0], curr_, len * sizeof(wchar_t));

			  curr_ = curr_ + (len + 1) * sizeof(wchar_t);
			  if (curr_ > end_){
				  YASLI_ASSERT(0);
				  curr_ = end_;
			  }
		  }

		  YASLI_INLINE u32 readPackedSize();

		  void resetBegin() { begin_ = curr_; }  // Set block wrapping point to current point, after custom block data

		  bool validToClose() const { return complex_ || curr_ == end_; }    // Simple blocks must be read exactly
	
	private:
		tukk begin_;
		tukk end_;
		tukk curr_;
		bool complex_;

#ifdef YASLI_BIN_ARCHIVE_CHECK_HASH_COLLISION
		typedef std::map<unsigned short, string> HashMap;
		HashMap hashMap_;
#endif
	};

	typedef std::vector<Block> Blocks;
	Blocks blocks_;
	tukk loadedData_;
	string stringBuffer_;
	wstring wstringBuffer_;

	YASLI_INLINE bool openNode(tukk name);
	YASLI_INLINE void closeNode(tukk name, bool check = true);
	Block& currentBlock() { return blocks_.back(); }
	template<class T>
	void read(T& t) { currentBlock().read(t); }
};

}


#if YASLI_INLINE_IMPLEMENTATION
#include "BinArchiveImpl.h"
#endif