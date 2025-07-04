#pragma once

#include <string>
#include <map>
#include <cstdint>
#include <drx3D/Export.h>

namespace drx3d {
/**
 * @brief Defines a HTTP response.
 */
class DRX3D_EXPORT HttpResponse {
	friend class Http;
public:
	/**
	 * Enumerate all the valid status codes for a response.
	 */
	enum class Status {
		/// Most common code returned when operation was successful.
		Ok = 200,
		/// The resource has successfully been created.
		Created = 201,
		/// The request has been accepted, but will be processed later by the server.
		Accepted = 202,
		/// The server didn't send any data in return.
		NoContent = 204,
		/// The server informs the client that it should clear the view (form) that caused the request to be sent.
		ResetContent = 205,
		/// The server has sent a part of the resource, as a response to a partial GET request.
		PartialContent = 206,

		// 3xx: redirection.

		/// The requested page can be accessed from several locations.
		MultipleChoices = 300,
		/// The requested page has permanently moved to a new location.
		MovedPermanently = 301,
		/// The requested page has temporarily moved to a new location.
		MovedTemporarily = 302,
		/// For conditional requests, means the requested page hasn't changed and doesn't need to be refreshed.
		NotModified = 304,

		// 4xx: client error.

		/// The server couldn't understand the request (syntax error).
		BadRequest = 400,
		/// The requested page needs an authentication to be accessed.
		Unauthorized = 401,
		/// The requested page cannot be accessed at all, even with authentication.
		Forbidden = 403,
		/// The requested page doesn't exist.
		NotFound = 404,
		/// The server can't satisfy the partial GET request (with a "Range" header field).
		RangeNotSatisfiable = 407,

		// 5xx: server error.

		/// The server encountered an unexpected error.
		InternalServerError = 500,
		/// The server doesn't implement a requested feature.
		NotImplemented = 501,
		/// The gateway server has received an error from the source server.
		BadGateway = 502,
		/// The server is temporarily unavailable (overloaded, in maintenance, ...).
		ServiceNotAvailable = 503,
		/// The gateway server couldn't receive a response from the source server.
		GatewayTimeout = 504,
		/// The server doesn't support the requested HTTP version.
		VersionNotSupported = 505,

		// 10xx: custom codes.

		/// Response is not a valid HTTP one.
		InvalidResponse = 1000,
		/// Connection with server failed.
		ConnectionFailed = 1001
	};

	/**
	 * Default constructor, constructs an empty response.
	 */
	HttpResponse();

	/**
	 * Get the value of a field.
	 * If the field \a field is not found in the response header, the empty string is returned.
	 * This function uses case-insensitive comparisons.
	 * @param field Name of the field to get.
	 * @return Value of the field, or empty string if not found.
	 */
	STxt GetField(const STxt &field) const;

	/**
	 * Get the response status code.
	 * The status code should be the first thing to be checked after receiving a response,
	 * it defines whether it is a success, a failure or anything else (see the Status enumeration).
	 * @return Status code of the response.
	 */
	Status GetStatus() const { return status; }

	/**
	 * Get the major HTTP version number of the response.
	 * @return Major HTTP version number.
	 */
	uint32_t GetMajorHttpVersion() const { return majorVersion; }

	/**
	 * Get the minor HTTP version number of the response.
	 * @return Minor HTTP version number.
	 */
	uint32_t GetMinorHttpVersion() const { return minorVersion; }

	/**
	 * Get the body of the response.
	 * The body of a response may contain:
	 * <ul>
	 * <li>The requested page (for GET requests).</li>
	 * <li>A response from the server (for POST requests).</li>
	 * <li>Nothing (for HEAD requests).</li>
	 * <li>An error message (in case of an error).</li>
	 * </ul>
	 * @return The response body.
	 */
	const STxt &GetBody() const { return body; }

private:
	using FieldTable = std::map<STxt, STxt>;

	/**
	 * Construct the header from a response string.
	 * This function is used by Http to build the response of a request.
	 * @param data Content of the response to parse.
	 */
	void Parse(const STxt &data);

	/**
	 * Read values passed in the answer header.
	 * This function is used by Http to extract values passed in the response.
	 * @param in String stream containing the header values.
	 */
	void ParseFields(std::istream &in);

	/// Fields of the header.
	FieldTable fields;
	/// Status code.
	Status status;
	/// Major HTTP version.
	uint32_t majorVersion;
	/// Minor HTTP version.
	uint32_t minorVersion;
	/// Body of the response.
	STxt body;
};
}
