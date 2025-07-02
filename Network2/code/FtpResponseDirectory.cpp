#include <drx3D/Network2/Ftp/FtpResponseDirectory.h>

namespace drx3d {
FtpResponseDirectory::FtpResponseDirectory(const FtpResponse &response) :
	FtpResponse(response) {
	if (IsOk()) {
		// Extract the directory from the server response
		auto begin = GetFullMessage().find('"', 0);
		auto end = GetFullMessage().find('"', begin + 1);
		directory = GetFullMessage().substr(begin + 1, end - begin - 1);
	}
}
}
