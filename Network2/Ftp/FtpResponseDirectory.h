#pragma once

#include <drx3D/Network2/Ftp/FtpResponse.h>

namespace drx3d {
/**
 * @brief Specialization of FTP response returning a directory.
 */
class DRX3D_EXPORT FtpResponseDirectory : public FtpResponse {
public:
	/**
	 * Default constructor.
	 * @param response Source response.
	 */
	FtpResponseDirectory(const FtpResponse &response);

	/**
	 * Get the directory returned in the response.
	 * @return Directory name.
	 */
	const STxt &GetDirectory() const { return directory; }

private:
	/// Directory extracted from the response message.
	STxt directory;
};
}
