#include <drx3D/Network2/Ftp/FtpResponse.h>
#include <cstdint>

namespace drx3d {
FtpResponse::FtpResponse(Status code, STxt message) :
	status(code),
	message(std::move(message)) {
}

bool FtpResponse::IsOk() const {
	return static_cast<uint32_t>(status) < 400;
}
}
