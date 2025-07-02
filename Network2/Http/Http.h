#pragma once

#include <drx3D/Network2/Tcp/TcpSocket.h>
#include <drx3D/Network2/IpAddress.h>
#include <drx3D/Network2/Http/HttpRequest.h>
#include <drx3D/Network2/Http/HttpResponse.h>

namespace drx3d {
/**
 * @brief A very simple HTTP client that allows you to communicate with a web server.
 * You can retrieve web pages, send data to an interactive resource,
 * download a remote file, etc. The HTTPS protocol is not supported.
 *
 * The HTTP client is split into 3 classes:
 * \li drx3d::HttpRequest
 * \li drx3d::HttpResponse
 * \li drx3d::Http
 *
 * drx3d::HttpRequest builds the request that will be sent to the server. A request is made of:
 * \li a method (what you want to do)
 * \li a target URI (usually the name of the web page or file)
 * \li one or more header fields (options that you can pass to the server)
 * \li an optional body (for POST requests)
 *
 * drx3d::HttpResponse parse the response from the web server and provides getters to read them.
 * The response contains:
 * \li a status code
 * \li header fields (that may be answers to the ones that you requested)
 * \li a body, which contains the contents of the requested resource
 *
 * drx3d::Http provides a simple function, SendRequest, to send a drx3d::HttpRequest and
 * return the corresponding drx3d::HttpResponse
 * from the server.
 */
class DRX3D_EXPORT Http {
public:
	/**
	 * Default constructor.
	 */
	Http();

	/**
	 * Construct the HTTP client with the target host.
	 * This is equivalent to calling setHost(host, port).
	 * The port has a default value of 0, which means that the HTTP client will use the
	 * right port according to the protocol used (80 for HTTP).
	 * You should leave it like this unless you really need a port other than the standard one, or use an unknown protocol.
	 * @param host Web server to connect to.
	 * @param port Port to use for connection.
	 */
	explicit Http(const STxt &host, uint16_t port = 0);

	/**
	 * Set the target host.
	 * This function just stores the host address and port, it doesn't actually connect to it until you send a request.
	 * The port has a default value of 0, which means that the HTTP client will use the right port according to the
	 * protocol used (80 for HTTP). You should leave it like this unless you really need a port other than the
	 * standard one, or use an unknown protocol.
	 * @param host Web server to connect to.
	 * @param port Port to use for connection.
	 */
	void SetHost(const STxt &host, uint16_t port = 0);

	/**
	 * Send a HTTP request and return the server's response.
	 * You must have a valid host before sending a request (see setHost).
	 * Any missing mandatory header field in the request will be added with an appropriate value.
	 * Warning: this function waits for the server's response and may not return instantly;
	 * use a thread if you don't want to block your application, or use a timeout to limit the time to wait.
	 * A value of 0s means that the client will use the system default timeout (which is usually pretty long).
	 * @param request Request to send.
	 * @param timeout Maximum time to wait.
	 * @return Server's response.
	 */
	HttpResponse SendRequest(const HttpRequest &request, const Time &timeout = 0s);

private:
	/// Connection to the host.
	TcpSocket connection;
	/// Web host address.
	IpAddress host;
	/// Web host name.
	STxt hostName;
	/// Port used for connection with host.
	uint16_t port = 0;
};
}
