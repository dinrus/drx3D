#pragma once

#include <drx3D/Network2/Tcp/TcpSocket.h>
#include <drx3D/Network2/IpAddress.h>
#include <drx3D/Network2/Ftp/FtpDataChannel.h>
#include <drx3D/Network2/Ftp/FtpResponse.h>
#include <drx3D/Network2/Ftp/FtpResponseDirectory.h>
#include <drx3D/Network2/Ftp/FtpResponseListing.h>

namespace drx3d {
/**
 * @brief A very simple FTP client that allows you to communicate with a FTP server.
 * The FTP protocol allows you to manipulate a remote file system
 * (list files, upload, download, create, remove, ...).
 *
 * Using the FTP client consists of 4 parts:
 * \li Connecting to the FTP server
 * \li Logging in (either as a registered user or anonymously)
 * \li Sending commands to the server
 * \li Disconnecting (this part can be done implicitly by the destructor)
 *
 * Every command returns a FTP response, which contains the status code as well as a message from the server.
 * Some commands such as GetWorkingDirectory() and GetDirectoryListing() return additional data,
 * and use a class derived from drx3d::FtpResponse to provide this data. The most often used
 * commands are directly provided as member functions, but it is also possible
 * to use specific commands with the SendCommand() function.
 *
 * Note that response statuses >= 1000 are not part of the FTP standard,
 * they are generated by SFML when an internal error occurs.
 *
 * All commands, especially upload and download, may take some time to complete.
 * This is important to know if you don't want to block your application while
 * the server is completing the task.
 */
class DRX3D_EXPORT Ftp {
public:
	/**
	 * Default constructor.
	 */
	Ftp();
	/**
	 * Automatically closes the connection with the server if it is still opened.
	 */
	~Ftp();

	/**
	 * Connects to the specified FTP server.
	 * The port has a default value of 21, which is the standard port used by the FTP protocol.
	 * You should'nt use a different value, unless you really know what you do.
	 * This function tries to connect to the server so it may take a while to complete,
	 * especially if the server is not reachable. To avoid blocking your application for too long,
	 * you can use a timeout. The default value, 0s, means that the system timeout will be
	 * used (which is usually pretty long).
	 * @param server Name or address of the FTP server to connect to.
	 * @param port Port used for the connection.
	 * @param timeout Maximum time to wait.
	 * @return Server response to the request.
	 */
	FtpResponse Connect(const IpAddress &server, uint16_t port = 21, const Time &timeout = 0s);

	/**
	 * Close the connection with the server.
	 * @return Server response to the request.
	 */
	FtpResponse Disconnect();

	/**
	 * Log in using an anonymous account.
	 * Logging in is mandatory after connecting to the server.
	 * Users that are not logged in cannot perform any operation.
	 * @return Server response to the request.
	 */
	FtpResponse Login();

	/**
	 * Log in using a username and a password.
	 * Logging in is mandatory after connecting to the server.
	 * Users that are not logged in cannot perform any operation.
	 * @param name User name.
	 * @param password Password.
	 * @return Server response to the request.
	 */
	FtpResponse Login(const STxt &name, const STxt &password);

	/**
	 * Send a null command to keep the connection alive.
	 * This command is useful because the server may close the connection automatically if no command is sent.
	 * @return Server response to the request.
	 */
	FtpResponse KeepAlive();

	/**
	 * Get the current working directory.
	 * The working directory is the root path for subsequent operations involving directories and/or filenames.
	 * @return Server response to the request.
	 */
	FtpResponseDirectory GetWorkingDirectory();

	/**
	 * Get the contents of the given directory.
	 * This function retrieves the sub-directories and files contained in the given directory. It is not recursive.
	 * The \a directory parameter is relative to the current working directory.
	 * @param directory Directory to list.
	 * @return Server response to the request.
	 */
	FtpResponseListing GetDirectoryListing(const STxt &directory = "");

	/**
	 * Change the current working directory.
	 * The new directory must be relative to the current one.
	 * @param directory New working directory.
	 * @return Server response to the request.
	 */
	FtpResponse ChangeDirectory(const STxt &directory);

	/**
	 * Go to the parent directory of the current one.
	 * @return Server response to the request.
	 */
	FtpResponse ParentDirectory();

	/**
	 * Create a new directory.
	 * The new directory is created as a child of the current working directory.
	 * @param name Name of the directory to create.
	 * @return Server response to the request.
	 */
	FtpResponse CreateRemoteDirectory(const STxt &name);

	/**
	 * Remove an existing directory
	 * The directory to remove must be relative to the current working directory.
	 * Use this function with caution, the directory will be removed permanently!
	 * @param name Name of the directory to remove.
	 * @return Server response to the request.
	 */
	FtpResponse DeleteRemoteDirectory(const STxt &name);

	/**
	 * Rename an existing file.
	 * The filenames must be relative to the current working directory.
	 * @param file File to rename.
	 * @param newName New name of the file.
	 * @return Server response to the request.
	 */
	FtpResponse RenameRemoteFile(const STxt &file, const STxt &newName);

	/**
	 * Remove an existing file.
	 * The file name must be relative to the current working directory.
	 * Use this function with caution, the file will be removed permanently!
	 * @param name File to remove.
	 * @return Server response to the request.
	 */
	FtpResponse DeleteRemoteFile(const STxt &name);

	/**
	 * Download a file from the server.
	 * The filename of the distant file is relative to the current working directory of the server,
	 * and the local destination path is relative to the current directory of your application.
	 * If a file with the same filename as the distant file already exists in the local destination path,
	 * it will be overwritten.
	 * @param remoteFile Filename of the distant file to download.
	 * @param localPath The directory in which to put the file on the local computer.
	 * @param mode Transfer mode.
	 * @return Server response to the request.
	 */
	FtpResponse Download(const STxt &remoteFile, const STxt &localPath, const FtpDataChannel::Mode &mode = FtpDataChannel::Mode::Binary);

	/**
	 * Upload a file to the server.
	 * The name of the local file is relative to the current working directory of your application,
	 * and the remote path is relative to the current directory of the FTP server.
	 * The append parameter controls whether the remote file is appended to or overwritten if it already exists.
	 * @param localFile Path of the local file to upload.
	 * @param remotePath The directory in which to put the file on the server.
	 * @param mode Transfer mode.
	 * @param append Pass true to append to or false to overwrite the remote file if it already exists.
	 * @return Server response to the request.
	 */
	FtpResponse Upload(const STxt &localFile, const STxt &remotePath, const FtpDataChannel::Mode &mode = FtpDataChannel::Mode::Binary, bool append = false);

	/**
	 * Send a command to the FTP server.
	 * While the most often used commands are provided as member functions in the Ftp class,
	 * this method can be used to send any FTP command to the server.
	 * If the command requires one or more parameters, they can be specified in \a parameter.
	 * If the server returns information, you can extract it from the response using FtpResponse::GetFullMessage().
	 * @param command Command to send.
	 * @param parameter Command parameter.
	 * @return Server response to the request.
	 */
	FtpResponse SendCommand(const STxt &command, const STxt &parameter = "");

private:
	/**
	 * Receive a response from the server.
	 * This function must be called after each call to SendCommand that expects a response.
	 * @return Server response to the request.
	 */
	FtpResponse GetResponse();

	/// Socket holding the control connection with the server.
	TcpSocket commandSocket;
	/// Received command data that is yet to be processed.
	STxt receiveBuffer;
};
}
