#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <drx3D/Export.h>

namespace drx3d {
/**
 * @brief Packets provide a safe and easy way to serialize data,
 * in order to send it over the network using sockets (drx3d::TcpSocket, drx3d::UdpSocket).
 *
 * Packets solve 2 fundamental problems that arise when transferring data over the network:
 * \li data is interpreted correctly according to the endianness
 * \li the bounds of the packet are preserved (one send == one receive)
 *
 * The drx3d::Packet class provides both input and output modes. It is designed to follow the
 * behavior of standard C++ streams, using operators >> and << to extract and insert data.
 *
 * It is recommended to use only fixed-size types (like drx3d::Int32, etc.),
 * to avoid possible differences between the sender and the receiver.
 * Indeed, the native C++ types may have different sizes on two platforms and your data may be
 * corrupted if that happens.
 */
class DRX3D_EXPORT Packet {
	friend class TcpSocket;
	friend class UdpSocket;

public:
	/// A bool-like type that cannot be converted to integer or pointer types.
	typedef bool (Packet::*BoolType)(std::size_t);

	/**
	 * Creates an empty packet.
	 */
	Packet();
	virtual ~Packet() = default;

	/**
	 * Append data to the end of the packet.
	 * @param data Pointer to the sequence of bytes to append.
	 * @param sizeInBytes Number of bytes to append.
	 */
	void Append(ukk data, std::size_t sizeInBytes);

	/**
	 * Clear the packet, after calling Clear, the packet is empty.
	 */
	void Clear();

	/**
	 * Get a pointer to the data contained in the packet.
	 * Warning: the returned pointer may become invalid after  you append data to the packet,
	 * therefore it should never be stored.
	 * The return pointer is null if the packet is empty.
	 * @return Pointer to the data.
	 */
	ukk GetData() const;

	/**
	 * Get the size of the data contained in the packet.
	 * This function returns the number of bytes pointed to by what getData returns.
	 * @return Data size, in bytes.
	 */
	std::size_t GetDataSize() const;

	/**
	 * Tell if the reading position has reached the end of the packet.
	 * This function is useful to know if there is some data left to be read, without actually reading it.
	 * @return True if all data was read, false otherwise.
	 */
	bool EndOfStream() const;

	/**
	 * Test the validity of the packet, for reading.
	 * This operator allows to test the packet as a boolean variable, to check if a reading operation was successful.
	 * A packet will be in an invalid state if it has no more data to read. This behavior is the same as standard C++ streams.
	 *
	 * Usage example:
	 * <code>
	 * float x;
	 * packet >> x;
	 * if (packet)
	 * {
	 * // ok, x was extracted successfully
	 * }
	 *
	 * // -- or --
	 *
	 * float x;
	 * if (packet >> x)
	 * {
	 * // ok, x was extracted successfully
	 * }
	 * </code>
	 *
	 * Don't focus on the return type, it's equivalent to bool but it disallows unwanted implicit conversions to integer or pointer types.
	 * @return True if last data extraction from packet was successful.
	 */
	operator BoolType() const;

	// Overload of operator >> to read data from the data stream
	Packet &operator>>(bool &data);
	Packet &operator>>(int8_t &data);
	Packet &operator>>(uint8_t &data);
	Packet &operator>>(int16_t &data);
	Packet &operator>>(uint16_t &data);
	Packet &operator>>(int32_t &data);
	Packet &operator>>(uint32_t &data);
	Packet &operator>>(int64_t &data);
	Packet &operator>>(uint64_t &data);
	Packet &operator>>(float &data);
	Packet &operator>>(double &data);
	Packet &operator>>(char *data);
	Packet &operator>>(STxt &data);
	Packet &operator>>(wchar_t *data);
	Packet &operator>>(std::wstring &data);

	// Overload of operator << to write data into the data stream
	Packet &operator<<(bool data);
	Packet &operator<<(int8_t data);
	Packet &operator<<(uint8_t data);
	Packet &operator<<(int16_t data);
	Packet &operator<<(uint16_t data);
	Packet &operator<<(int32_t data);
	Packet &operator<<(uint32_t data);
	Packet &operator<<(int64_t data);
	Packet &operator<<(uint64_t data);
	Packet &operator<<(float data);
	Packet &operator<<(double data);
	Packet &operator<<(tukk data);
	Packet &operator<<(const STxt &data);
	Packet &operator<<(const wchar_t *data);
	Packet &operator<<(const std::wstring &data);

protected:
	/**
	 * Called before the packet is sent over the network.
	 * This function can be defined by derived classes to transform the data before it is sent;
	 * this can be used for compression, encryption, etc.
	 * The function must return a pointer to the modified data, as well as the number of bytes pointed.
	 * The default implementation provides the packet's data without transforming it.
	 * @return Pointer to the array of bytes to send and the size of data to send.
	 */
	virtual std::pair<ukk , std::size_t> OnSend();

	/**
	 * Called after the packet is received over the network.
	 * This function can be defined by derived classes to transform the data after it is received;
	 * this can be used for decompression, decryption, etc.
	 * The function receives a pointer to the received data, and must fill the packet with the transformed bytes.
	 * The default implementation fills the packet directly without transforming the data.
	 * @param data Pointer to the received bytes.
	 * @param size Number of bytes.
	 */
	virtual void OnReceive(ukk data, std::size_t size);

	/**
	 * Check if the packet can extract a given number of bytes.
	 * This function updates accordingly the state of the packet.
	 * @param size Size to check.
	 * @return True if \a size bytes can be read from the packet.
	 */
	bool CheckSize(std::size_t size);

	/// Data stored in the packet.
	std::vector<char> data;
	/// Current reading position in the packet.
	std::size_t readPos = 0;
	/// Current send position in the packet (for handling partial sends).
	std::size_t sendPos = 0;
	/// Reading state of the packet.
	bool isValid;
};
}
