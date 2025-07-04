#pragma once

#include <drx3D/Network2/Tcp/TcpSocket.h>

namespace drx3d {
class Ftp;
class FtpResponse;

/**
 * @brief Utility class for exchanging datas with the server on the data channel.
 */
class DRX3D_EXPORT FtpDataChannel {
public:
	/**
	 * @brief Enumeration of transfer modes.
	 */
	enum class Mode {
		/// Binary mode (file is transfered as a sequence of bytes).
		Binary,
		/// Text mode using ASCII encoding.
		Ascii,
		/// Text mode using EBCDIC encoding.
		Ebcdic
	};

	explicit FtpDataChannel(Ftp &owner);

	FtpResponse Open(Mode mode);

	void Send(std::istream &stream);

	void Receive(std::ostream &stream);

private:
	/// Reference to the owner Ftp instance.
	Ftp &ftp;
	/// Socket used for data transfers.
	TcpSocket dataSocket;
};
}
