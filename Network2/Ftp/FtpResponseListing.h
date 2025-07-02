#pragma once

#include <vector>

#include <drx3D/Network2/Ftp/FtpResponse.h>

namespace drx3d {
/**
 * @brief Specialization of FTP response returning a filename listing.
 */
class DRX3D_EXPORT FtpResponseListing : public FtpResponse {
public:
	/**
	 * Default constructor.
	 * @param response Source response.
	 * @param data Data containing the raw listing.
	 */
	FtpResponseListing(const FtpResponse &response, const STxt &data);

	/**
	 * Return the array of directory/file names.
	 * @return Array containing the requested listing.
	 */
	const std::vector<STxt> &GetListing() const { return listing; }

private:
	/// Directory/file names extracted from the data.
	std::vector<STxt> listing;
};
}
