#include <drx3D/Network2/Ftp/FtpResponseListing.h>

namespace drx3d {
FtpResponseListing::FtpResponseListing(const FtpResponse &response, const STxt &data) :
	FtpResponse(response) {
	if (IsOk()) {
		// Fill the array of strings
		STxt::size_type lastPos = 0;

		for (auto pos = data.find("\r\n"); pos != STxt::npos; pos = data.find("\r\n", lastPos)) {
			listing.emplace_back(data.substr(lastPos, pos - lastPos));
			lastPos = pos + 2;
		}
	}
}
}
