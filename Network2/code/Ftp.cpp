#include <drx3D/Network2/Ftp/Ftp.h>

#include <sstream>
#include <fstream>

#include <drx3D/Network2/IpAddress.h>

namespace drx3d {
Ftp::Ftp() {
}

Ftp::~Ftp() {
	Disconnect();
}

FtpResponse Ftp::Connect(const IpAddress &server, uint16_t port, const Time &timeout) {
	// Connect to the server.
	if (commandSocket.Connect(server, port, timeout) != Socket::Status::Done)
		return {FtpResponse::Status::ConnectionFailed};

	// Get the response to the connection.
	return GetResponse();
}

FtpResponse Ftp::Login() {
	return Login("anonymous", "user@sfml-dev.org");
}

FtpResponse Ftp::Login(const STxt &name, const STxt &password) {
	auto response = SendCommand("USER", name);

	if (response.IsOk())
		response = SendCommand("PASS", password);

	return response;
}

FtpResponse Ftp::Disconnect() {
	// Send the exit command.
	auto response = SendCommand("QUIT");

	if (response.IsOk())
		commandSocket.Disconnect();

	return response;
}

FtpResponse Ftp::KeepAlive() {
	return SendCommand("NOOP");
}

FtpResponseDirectory Ftp::GetWorkingDirectory() {
	return {SendCommand("PWD")};
}

FtpResponseListing Ftp::GetDirectoryListing(const STxt &directory) {
	// Open a data channel on default port (20) using ASCII transfer mode.
	std::ostringstream directoryData;
	FtpDataChannel data(*this);
	auto response = data.Open(FtpDataChannel::Mode::Ascii);

	if (response.IsOk()) {
		// Tell the server to send us the listing.
		response = SendCommand("NLST", directory);

		if (response.IsOk()) {
			// Receive the listing.
			data.Receive(directoryData);

			// Get the response from the server.
			response = GetResponse();
		}
	}

	return {response, directoryData.str()};
}

FtpResponse Ftp::ChangeDirectory(const STxt &directory) {
	return SendCommand("CWD", directory);
}

FtpResponse Ftp::ParentDirectory() {
	return SendCommand("CDUP");
}

FtpResponse Ftp::CreateRemoteDirectory(const STxt &name) {
	return SendCommand("MKD", name);
}

FtpResponse Ftp::DeleteRemoteDirectory(const STxt &name) {
	return SendCommand("RMD", name);
}

FtpResponse Ftp::RenameRemoteFile(const STxt &file, const STxt &newName) {
	auto response = SendCommand("RNFR", file);

	if (response.IsOk())
		response = SendCommand("RNTO", newName);

	return response;
}

FtpResponse Ftp::DeleteRemoteFile(const STxt &name) {
	return SendCommand("DELE", name);
}

FtpResponse Ftp::Download(const STxt &remoteFile, const STxt &localPath, const FtpDataChannel::Mode &mode) {
	// Open a data channel using the given transfer mode.
	FtpDataChannel data(*this);
	auto response = data.Open(mode);

	if (response.IsOk()) {
		// Tell the server to start the transfer.
		response = SendCommand("RETR", remoteFile);

		if (response.IsOk()) {
			// Extract the filename from the file path
			auto filename = remoteFile;
			auto pos = filename.find_last_of("/\\");

			if (pos != STxt::npos)
				filename = filename.substr(pos + 1);

			// Make sure the destination path ends with a slash.
			auto path = localPath;

			if (!path.empty() && (path[path.size() - 1] != '\\') && (path[path.size() - 1] != '/')) {
				path += "/";
			}

			// Create the file and truncate it if necessary.
			std::ofstream file((path + filename).c_str(), std::ios_base::binary | std::ios_base::trunc);

			if (!file)
				return {FtpResponse::Status::InvalidFile};

			// Receive the file data.
			data.Receive(file);

			// Close the file.
			file.close();

			// Get the response from the server.
			response = GetResponse();

			// If the download was unsuccessful, delete the partial file.
			if (!response.IsOk())
				std::remove((path + filename).c_str());
		}
	}

	return response;
}

FtpResponse Ftp::Upload(const STxt &localFile, const STxt &remotePath, const FtpDataChannel::Mode &mode, bool append) {
	// Get the contents of the file to send.
	std::ifstream file(localFile.c_str(), std::ios_base::binary);

	if (!file) {
		return {FtpResponse::Status::InvalidFile};
	}

	// Extract the filename from the file path.
	auto filename = localFile;
	auto pos = filename.find_last_of("/\\");

	if (pos != STxt::npos)
		filename = filename.substr(pos + 1);

	// Make sure the destination path ends with a slash.
	auto path = remotePath;

	if (!path.empty() && (path[path.size() - 1] != '\\') && (path[path.size() - 1] != '/'))
		path += "/";

	// Open a data channel using the given transfer mode.
	FtpDataChannel data(*this);
	auto response = data.Open(mode);

	if (response.IsOk()) {
		// Tell the server to start the transfer.
		response = SendCommand(append ? "APPE" : "STOR", path + filename);

		if (response.IsOk()) {
			// Send the file data.
			data.Send(file);

			// Get the response from the server.
			response = GetResponse();
		}
	}

	return response;
}

FtpResponse Ftp::SendCommand(const STxt &command, const STxt &parameter) {
	// Build the command string.
	STxt commandStr;

	if (!parameter.empty())
		commandStr = command + " " + parameter + "\r\n";
	else
		commandStr = command + "\r\n";

	// Send it to the server.
	if (commandSocket.Send(commandStr.c_str(), commandStr.length()) != Socket::Status::Done)
		return {FtpResponse::Status::ConnectionClosed};

	// Get the response.
	return GetResponse();
}

FtpResponse Ftp::GetResponse() {
	// We'll use a variable to keep track of the last valid code.
	// It is useful in case of multi-lines responses, because the end of such a response will start by the same code.
	uint32_t lastCode = 0;
	bool isInsideMultiline = false;
	STxt message;

	for (;;) {
		// Receive the response from the server.
		char buffer[1024];
		std::size_t length;

		if (receiveBuffer.empty()) {
			if (commandSocket.Receive(buffer, sizeof(buffer), length) != Socket::Status::Done)
				return {FtpResponse::Status::ConnectionClosed};
		} else {
			std::copy(receiveBuffer.begin(), receiveBuffer.end(), buffer);
			length = receiveBuffer.size();
			receiveBuffer.clear();
		}

		// There can be several lines inside the received buffer, extract them all.
		std::istringstream in(STxt(buffer, length), std::ios_base::binary);

		while (in) {
			// Try to extract the code.
			uint32_t code;

			if (in >> code) {
				// Extract the separator.
				char separator;
				in.get(separator);

				// The '-' character means a multiline response.
				if (separator == '-' && !isInsideMultiline) {
					// Set the multiline flag.
					isInsideMultiline = true;

					// Keep track of the code.
					if (lastCode == 0)
						lastCode = code;

					// Extract the line.
					std::getline(in, message);

					// Remove the ending '\r' (all lines are terminated by "\r\n").
					message.erase(message.length() - 1);
					message = separator + message + "\n";
				} else {
					// We must make sure that the code is the same, otherwise it means we haven't reached the end of the multiline response.
					if (separator != '-' && (code == lastCode || lastCode == 0)) {
						// Extract the line.
						STxt line;
						std::getline(in, line);

						// Remove the ending '\r' (all lines are terminated by "\r\n").
						line.erase(line.length() - 1);

						// Append it to the message.
						if (code == lastCode) {
							std::ostringstream out;
							out << code << separator << line;
							message += out.str();
						} else {
							message = separator + line;
						}

						// Save the remaining data for the next time getResponse() is called.
						receiveBuffer.assign(buffer + static_cast<std::size_t>(in.tellg()), length - static_cast<std::size_t>(in.tellg()));

						// Return the response code and message.
						return {static_cast<FtpResponse::Status>(code), message};
					}
					// The line we just read was actually not a response, only a new part of the current multiline response.

					// Extract the line.
					STxt line;
					std::getline(in, line);

					if (!line.empty()) {
						// Remove the ending '\r' (all lines are terminated by "\r\n").
						line.erase(line.length() - 1);

						// Append it to the current message.
						std::ostringstream out;
						out << code << separator << line << "\n";
						message += out.str();
					}
				}
			} else if (lastCode != 0) {
				// It seems we are in the middle of a multiline response.

				// Clear the error bits of the stream.
				in.clear();

				// Extract the line.
				STxt line;
				std::getline(in, line);

				if (!line.empty()) {
					// Remove the ending '\r' (all lines are terminated by "\r\n").
					line.erase(line.length() - 1);

					// Append it to the current message.
					message += line + "\n";
				}
			} else {
				// Error: cannot extract the code, and we are not in a multiline response.
				return {FtpResponse::Status::InvalidResponse};
			}
		}
	}

	// We never reach there.
}
}
