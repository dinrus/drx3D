#include <drx3D/Network2/Packet.h>

#include <cstring>
#include <cwchar>
#ifdef DRX3D_BUILD_WINDOWS
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include <drx3D/Network2/Socket.h>

namespace drx3d {
Packet::Packet() :
	isValid(true) {
}

void Packet::Append(ukk data, std::size_t sizeInBytes) {
	if (data && (sizeInBytes > 0)) {
		auto start = this->data.size();
		this->data.resize(start + sizeInBytes);
		std::memcpy(&this->data[start], data, sizeInBytes);
	}
}

void Packet::Clear() {
	data.clear();
	readPos = 0;
	isValid = true;
}

ukk Packet::GetData() const {
	return !data.empty() ? &data[0] : nullptr;
}

std::size_t Packet::GetDataSize() const {
	return data.size();
}

bool Packet::EndOfStream() const {
	return readPos >= data.size();
}

Packet::operator BoolType() const {
	return isValid ? &Packet::CheckSize : nullptr;
}

Packet &Packet::operator>>(bool &data) {
	uint8_t value;

	if (*this >> value)
		data = value != 0;

	return *this;
}

Packet &Packet::operator>>(int8_t &data) {
	if (CheckSize(sizeof(data))) {
		data = *reinterpret_cast<const int8_t *>(&this->data[readPos]);
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(uint8_t &data) {
	if (CheckSize(sizeof(data))) {
		data = *reinterpret_cast<const uint8_t *>(&this->data[readPos]);
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(int16_t &data) {
	if (CheckSize(sizeof(data))) {
		data = ntohs(*reinterpret_cast<const int16_t *>(&this->data[readPos]));
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(uint16_t &data) {
	if (CheckSize(sizeof(data))) {
		data = ntohs(*reinterpret_cast<const uint16_t *>(&this->data[readPos]));
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(int32_t &data) {
	if (CheckSize(sizeof(data))) {
		data = ntohl(*reinterpret_cast<const int32_t *>(&this->data[readPos]));
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(uint32_t &data) {
	if (CheckSize(sizeof(data))) {
		data = ntohl(*reinterpret_cast<const uint32_t *>(&this->data[readPos]));
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(int64_t &data) {
	if (CheckSize(sizeof(data))) {
		// Since ntohll is not available everywhere, we have to convert to network byte order (big endian) manually.
		auto bytes = reinterpret_cast<const uint8_t *>(&this->data[readPos]);
		data = (static_cast<int64_t>(bytes[0]) << 56) | (static_cast<int64_t>(bytes[1]) << 48) | (static_cast<int64_t>(bytes[2]) << 40) | (static_cast<int64_t>(bytes[3]) << 32)
			| (static_cast<int64_t>(bytes[4]) << 24) | (static_cast<int64_t>(bytes[5]) << 16) | (static_cast<int64_t>(bytes[6]) << 8) | (static_cast<int64_t>(bytes[7]));
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(uint64_t &data) {
	if (CheckSize(sizeof(data))) {
		// Since ntohll is not available everywhere, we have to convert to network byte order (big endian) manually.
		auto bytes = reinterpret_cast<const uint8_t *>(&this->data[readPos]);
		data = (static_cast<uint64_t>(bytes[0]) << 56) | (static_cast<uint64_t>(bytes[1]) << 48) | (static_cast<uint64_t>(bytes[2]) << 40) | (static_cast<uint64_t>(bytes[3]) << 32)
			| (static_cast<uint64_t>(bytes[4]) << 24) | (static_cast<uint64_t>(bytes[5]) << 16) | (static_cast<uint64_t>(bytes[6]) << 8) | (static_cast<uint64_t>(bytes[7]));
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(float &data) {
	if (CheckSize(sizeof(data))) {
		data = *reinterpret_cast<const float *>(&this->data[readPos]);
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(double &data) {
	if (CheckSize(sizeof(data))) {
		data = *reinterpret_cast<const double *>(&this->data[readPos]);
		readPos += sizeof(data);
	}

	return *this;
}

Packet &Packet::operator>>(char *data) {
	// First extract string length.
	uint32_t length = 0;
	*this >> length;

	if ((length > 0) && CheckSize(length)) {
		// Then extract characters.
		std::memcpy(data, &this->data[readPos], length);
		data[length] = '\0';

		// Update reading position.
		readPos += length;
	}

	return *this;
}

Packet &Packet::operator>>(STxt &data) {
	// First extract string length.
	uint32_t length = 0;
	*this >> length;

	data.clear();

	if ((length > 0) && CheckSize(length)) {
		// Then extract characters.
		data.assign(&this->data[readPos], length);

		// Update reading position.
		readPos += length;
	}

	return *this;
}

Packet &Packet::operator>>(wchar_t *data) {
	// First extract string length.
	uint32_t length = 0;
	*this >> length;

	if ((length > 0) && CheckSize(length * sizeof(uint32_t))) {
		// Then extract characters.
		for (uint32_t i = 0; i < length; ++i) {
			uint32_t character = 0;
			*this >> character;
			data[i] = static_cast<wchar_t>(character);
		}

		data[length] = L'\0';
	}

	return *this;
}

Packet &Packet::operator>>(std::wstring &data) {
	// First extract string length.
	uint32_t length = 0;
	*this >> length;

	data.clear();

	if ((length > 0) && CheckSize(length * sizeof(uint32_t))) {
		// Then extract characters.
		for (uint32_t i = 0; i < length; ++i) {
			uint32_t character = 0;
			*this >> character;
			data += static_cast<wchar_t>(character);
		}
	}

	return *this;
}

Packet &Packet::operator<<(bool data) {
	*this << static_cast<uint8_t>(data);
	return *this;
}

Packet &Packet::operator<<(int8_t data) {
	Append(&data, sizeof(data));
	return *this;
}

Packet &Packet::operator<<(uint8_t data) {
	Append(&data, sizeof(data));
	return *this;
}

Packet &Packet::operator<<(int16_t data) {
	auto toWrite = static_cast<int16_t>(htons(data));
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

Packet &Packet::operator<<(uint16_t data) {
	auto toWrite = static_cast<uint16_t>(htons(data));
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

Packet &Packet::operator<<(int32_t data) {
	auto toWrite = static_cast<int32_t>(htonl(data));
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

Packet &Packet::operator<<(uint32_t data) {
	auto toWrite = static_cast<uint32_t>(htonl(data));
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

Packet &Packet::operator<<(int64_t data) {
	// Since htonll is not available everywhere, we have to convert to network byte order (big endian) manually.
	uint8_t toWrite[8] = {
		static_cast<uint8_t>((data >> 56) & 0xFF), static_cast<uint8_t>((data >> 48) & 0xFF), static_cast<uint8_t>((data >> 40) & 0xFF),
		static_cast<uint8_t>((data >> 32) & 0xFF), static_cast<uint8_t>((data >> 24) & 0xFF), static_cast<uint8_t>((data >> 16) & 0xFF), static_cast<uint8_t>((data >> 8) & 0xFF),
		static_cast<uint8_t>((data) & 0xFF)
	};
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

Packet &Packet::operator<<(uint64_t data) {
	// Since htonll is not available everywhere, we have to convert to network byte order (big endian) manually.
	uint8_t toWrite[8] = {
		static_cast<uint8_t>((data >> 56) & 0xFF), static_cast<uint8_t>((data >> 48) & 0xFF), static_cast<uint8_t>((data >> 40) & 0xFF),
		static_cast<uint8_t>((data >> 32) & 0xFF), static_cast<uint8_t>((data >> 24) & 0xFF), static_cast<uint8_t>((data >> 16) & 0xFF), static_cast<uint8_t>((data >> 8) & 0xFF),
		static_cast<uint8_t>((data) & 0xFF)
	};
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

Packet &Packet::operator<<(float data) {
	Append(&data, sizeof(data));
	return *this;
}

Packet &Packet::operator<<(double data) {
	Append(&data, sizeof(data));
	return *this;
}

Packet &Packet::operator<<(tukk data) {
	// First insert string length.
	auto length = static_cast<uint32_t>(std::strlen(data));
	*this << length;

	// Then insert characters
	Append(data, length * sizeof(char));

	return *this;
}

Packet &Packet::operator<<(const STxt &data) {
	// First insert string length.
	auto length = static_cast<uint32_t>(data.size());
	*this << length;

	// Then insert characters.
	if (length > 0) {
		Append(data.c_str(), length * sizeof(STxt::value_type));
	}

	return *this;
}

Packet &Packet::operator<<(const wchar_t *data) {
	// First insert string length.
	auto length = static_cast<uint32_t>(std::wcslen(data));
	*this << length;

	// Then insert characters.
	for (auto c = data; *c != L'\0'; ++c)
		*this << static_cast<uint32_t>(*c);

	return *this;
}

Packet &Packet::operator<<(const std::wstring &data) {
	// First insert string length.
	auto length = static_cast<uint32_t>(data.size());
	*this << length;

	// Then insert characters.
	if (length > 0) {
		for (auto c : data)
			*this << static_cast<uint32_t>(c);
	}

	return *this;
}

std::pair<ukk , std::size_t> Packet::OnSend() {
	return {GetData(), GetDataSize()};
}

void Packet::OnReceive(ukk data, std::size_t size) {
	Append(data, size);
}

bool Packet::CheckSize(std::size_t size) {
	isValid = isValid && (readPos + size <= data.size());
	return isValid;
}
}
