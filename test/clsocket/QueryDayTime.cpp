
#include <string.h>
#include "ActiveSocket.h"  // Include header for active socket object definition

i32 main(i32 argc, char **argv)
{
	CActiveSocket socket;  // Instantiate active socket object (defaults to TCP).
	char time[50];

	memset(&time, 0, 50);

	//--------------------------------------------------------------------------
	// Initialize our socket object
	//--------------------------------------------------------------------------
	socket.Initialize();

	//--------------------------------------------------------------------------
	// Create a connection to the time server so that data can be sent
	// and received.
	//--------------------------------------------------------------------------
	//    if (socket.Open("time-C.timefreq.bldrdoc.gov", 13))
	if (socket.Open("192.168.86.196", 6667))
	{
		for (i32 i = 0; i < 100; i++)
		{
			//----------------------------------------------------------------------
			// Send a requtest the server requesting the current time.
			//----------------------------------------------------------------------
			char data[1024];
			sprintf(data, "%s %d", "Hello", i);
			i32 len = strlen(data);
			data[len] = 0;
			printf("Sending [%s]\n", data);
			len++;
			if (socket.Send((u8k *)data, len))
			{
				//----------------------------------------------------------------------
				// Receive response from the server.
				//----------------------------------------------------------------------
				i32 rec = socket.Receive(len);
				if (rec)
				{
					u8 *data = socket.GetData();
					memcpy(&time, data, len);
					printf("Received: [%s]\n", time);
				}
			}
		}

		//----------------------------------------------------------------------
		// Close the connection.
		//----------------------------------------------------------------------
		socket.Close();
	}

	return 1;
}
