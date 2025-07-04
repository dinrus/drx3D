/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "BinArchive.h"
#include <map>
#include <drx3D/CoreX/Serialization/yasli/MemoryWriter.h>
#include <drx3D/CoreX/Serialization/yasli/ClassFactory.h>

namespace yasli{

static u8k SIZE16 = 254;
static u8k SIZE32 = 255;

static u32k BIN_MAGIC = 0xb1a4c17f;

BinOArchive::BinOArchive()
: Archive(OUTPUT | BINARY)
{
	clear();
}

void BinOArchive::clear()
{
	stream_.clear();
	stream_.write((tukk)&BIN_MAGIC, sizeof(BIN_MAGIC));

#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	blockTypes_.push_back(UNDEFINED);
#endif
}

size_t BinOArchive::length() const
{ 
	return stream_.position();
}

bool BinOArchive::save(tukk filename)
{
  FILE* f = fopen(filename, "wb");
  if(!f)
		return false;

  if(fwrite(buffer(), 1, length(), f) != length())
  {
		fclose(f);
		return false;
  }

  fclose(f);
  return true;
}

inline void BinOArchive::openNode(tukk name, bool size8)
{
#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	YASLI_ASSERT_STR(blockTypes_.back() == UNDEFINED || blockTypes_.back() == (!*name ? POD : NON_POD), "Mixing empty and non-empty names is dangerous for BinArchives");
	blockTypes_.back() = (!*name ? POD : NON_POD);
	blockTypes_.push_back(UNDEFINED);
#endif

	if(!*name)
		return;

	unsigned short hash = calcHash(name);
	stream_.write(hash);

	blockSizeOffsets_.push_back(i32(stream_.position()));
	stream_.write((u8)0); 
	if(!size8)
		stream_.write((unsigned short)0); 
}

inline void BinOArchive::closeNode(tukk name, bool size8)
{
#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	blockTypes_.pop_back();
#endif

	if(!*name)
		return;

	u32 offset = blockSizeOffsets_.back();
	u32 size = (u32)(stream_.position() - offset - sizeof(u8) - (size8 ? 0 : sizeof(unsigned short)));
	blockSizeOffsets_.pop_back();
	u8* sizePtr = (u8*)(stream_.buffer() + offset);

	if(size < SIZE16){
		*sizePtr = size;
		if(!size8){
			u8* buffer = sizePtr + 3;
			memmove(buffer - 2, buffer, size);
			stream_.setPosition(stream_.position() - 2);
		}
	}
	else{
		YASLI_ASSERT(!size8);
		if(size < 0x10000){
			*sizePtr = SIZE16;
			*((unsigned short*)(sizePtr + 1)) = size;
		}
		else{
			u8* buffer = sizePtr + 3;
			stream_.write((unsigned short)0);
			*sizePtr = SIZE32;
			memmove(buffer + 2, buffer, size);
			*((u32*)(sizePtr + 1)) = size;
		}
	}
}

bool BinOArchive::operator()(bool& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(StringInterface& value, tukk name, tukk label)
{
	bool size8 = strlen(value.get()) + 1 < SIZE16;
	openNode(name, size8);
	stream_ << value.get();
	stream_.write(char(0));
	closeNode(name, size8);
	return true;
}

bool BinOArchive::operator()(WStringInterface& value, tukk name, tukk label)
{
	bool size8 = (wcslen(value.get()) + 1)*sizeof(wchar_t) < SIZE16;
	openNode(name, size8);
	stream_ << value.get();
	stream_.write(wchar_t(0));
	closeNode(name, size8);
	return true;
}

bool BinOArchive::operator()(float& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(double& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(i16& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(i8& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(u8& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(char& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(u16& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(i32& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(u32& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(i64& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(u64& value, tukk name, tukk label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(const Serializer& ser, tukk name, tukk label)
{
	openNode(name, false);
	ser(*this);
	closeNode(name, false);
	return true;
}

bool BinOArchive::operator()(ContainerInterface& ser, tukk name, tukk label)
{
	openNode(name, false);

	u32 size = (u32)ser.size();
	if(size < SIZE16)
		stream_.write((u8)size);
	else if(size < 0x10000){
		stream_.write(SIZE16);
		stream_.write((unsigned short)size);
	}
	else{
		stream_.write(SIZE32);
		stream_.write(size);
	}

	if(*name){
		if(size > 0){
			i32 i = 0;
			do {
				char buffer[16];
#ifdef _MSC_VER
				_itoa_s(i++, buffer, 10);
#else
				sprintf(buffer, "%d", i++);
#endif
				ser(*this, buffer, "");
			} while (ser.next());
		}

		closeNode(name, false);
	}
	else{
		if(size > 0)
			do 
				ser(*this, "", "");
				while (ser.next());
	}

    return true;
}

bool BinOArchive::operator()(PointerInterface& ptr, tukk name, tukk label)
{
	openNode(name, false);

	tukk typeName = ptr.registeredTypeName();
	if (!typeName)
		typeName = "";
	if (typeName[0] == '\0' && ptr.get()) {
		YASLI_ASSERT(0 && "Writing unregistered class. Use YASLI_CLASS_NAME macro for registration.");
	}

	TypeID baseType = ptr.baseType();

	if(ptr.get()){
		stream_ << typeName;
		stream_.write(char(0));
		ptr.serializer()(*this);
	}
	else
		stream_.write(char(0));

	closeNode(name, false);
	return true;
}


//////////////////////////////////////////////////////////////////////////

BinIArchive::BinIArchive()
: Archive(INPUT | BINARY)
, loadedData_(0)
{
}

BinIArchive::~BinIArchive()
{
	close();
}

bool BinIArchive::load(tukk filename)
{
	close();

	FILE* f = fopen(filename, "rb");
	if(!f)
		return false;
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(length == 0){
		fclose(f);
		return false;
	}
	loadedData_ = new char[length];
	if(fread((uk )loadedData_, 1, length, f) != length || !open(loadedData_, length)){
		close();
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool BinIArchive::open(tukk buffer, size_t size)
{
	if(!buffer)
		return false;
	if(size < sizeof(u32))
		return false;
	if(*(unsigned*)(buffer) != BIN_MAGIC)
		return false;
	buffer += sizeof(u32);
	size -= sizeof(u32);

	blocks_.push_back(Block(buffer, (u32)size));
	return true;
}

void BinIArchive::close()
{
	if(loadedData_)
		delete[] loadedData_;
	loadedData_ = 0;
}

bool BinIArchive::openNode(tukk name)
{
	Block block(0, 0);
	if(currentBlock().get(name, block)){
		blocks_.push_back(block);
		return true;
	}
	return false;
}

void BinIArchive::closeNode(tukk name, bool check) 
{
	YASLI_ASSERT(!check || currentBlock().validToClose());
	blocks_.pop_back();
}

bool BinIArchive::operator()(bool& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(StringInterface& value, tukk name, tukk label)
{
	if(!*name){
		stringBuffer_.clear();
		read(stringBuffer_);
		value.set(stringBuffer_.c_str());
		return true;
	}

	if(!openNode(name))
		return false;

	stringBuffer_.clear();
	read(stringBuffer_);
	value.set(stringBuffer_.c_str());
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(WStringInterface& value, tukk name, tukk label)
{
	if(!*name){
		wstringBuffer_.clear();
		read(wstringBuffer_);
		value.set(wstringBuffer_.c_str());
		return true;
	}

	if(!openNode(name))
		return false;

	wstringBuffer_.clear();
	read(wstringBuffer_);
	value.set(wstringBuffer_.c_str());
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(float& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(double& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(i16& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u16& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}


bool BinIArchive::operator()(i32& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u32& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(i64& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u64& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(i8& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u8& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(char& value, tukk name, tukk label)
{
	if(!*name){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(const Serializer& ser, tukk name, tukk label)
{
	if(!*name){
		ser(*this);
		return true;
	}

	if(!openNode(name))
		return false;

	ser(*this);
	closeNode(name, false);
	return true;
}

bool BinIArchive::operator()(ContainerInterface& ser, tukk name, tukk label)
{
	if(*name){
		if(!openNode(name))
			return false;

		size_t size = currentBlock().readPackedSize();
		currentBlock().resetBegin();    // name/size chunks start here, after custom metadata
		ser.resize(size);

		if(size > 0){
			i32 i = 0;
			do{
				char buffer[16];
#ifdef _MSC_VER
				_itoa_s(i++, buffer, 10);
#else
				sprintf(buffer, "%d", i++);
#endif
				ser(*this, buffer, "");
			}
			while(ser.next());
		}
		closeNode(name);
		return true;
	}
	else{
		size_t size = currentBlock().readPackedSize();
		currentBlock().resetBegin();
		ser.resize(size);
		if(size > 0){
			do
				ser(*this, "", "");
			while(ser.next());
		}
		return true;
	}
}

bool BinIArchive::operator()(PointerInterface& ptr, tukk name, tukk label)
{
	if(*name && !openNode(name))
		return false;

	string typeName;
	read(typeName);
	currentBlock().resetBegin();  // name/size chunks start here, after custom metadata
	if(ptr.get() && (typeName.empty() || strcmp(typeName.c_str(), ptr.registeredTypeName()) != 0))
		ptr.create(""); // 0

	if(!typeName.empty() && !ptr.get())
		ptr.create(typeName.c_str());

	if(Serializer ser = ptr.serializer())
		ser(*this);

	if(*name)
		closeNode(name);
	return true;
}

u32 BinIArchive::Block::readPackedSize()
{
	u8 size8;
	read(size8);
	if(size8 < SIZE16)
		return size8;
	if(size8 == SIZE16){
		unsigned short size16;
		read(size16);
		return size16;
	}
	u32 size32;
	read(size32);
	return size32;
}

bool BinIArchive::Block::get(tukk name, Block& block) 
{
	if(begin_ == end_)
		return false;
	complex_ = true;
	unsigned short hashName = calcHash(name);

#ifdef YASLI_BIN_ARCHIVE_CHECK_HASH_COLLISION
	HashMap::iterator i = hashMap_.find(hashName);
	if(i != hashMap_.end() && i->second != name)
		YASLI_ASSERT(0, "BinArchive hash colliding: %s - %s", i->second.c_str(), name);
	hashMap_[hashName] = name;
#endif

	tukk currInitial = curr_;
	bool restarted = false;
	if(curr_ >= end_){
		curr_ = begin_;
		restarted = true;
	}
	for(;;){
		unsigned short hash;
		read(hash);
		u32 size = readPackedSize();
		
		tukk currPrev = curr_;

		if((curr_ += size) >= end_){
			if(restarted)
				return false;
			curr_ = begin_;
			restarted = true;
		}

		if(hash == hashName){
			block = Block(currPrev, size);
			return true;
		}
		if(curr_ == currInitial)
			return false;
	}
}

}

