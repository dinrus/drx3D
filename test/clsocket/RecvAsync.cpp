#include <pthread.h>
#include "PassiveSocket.h"

#ifdef WIN32
#include <windows.h>

// usually defined with #include <unistd.h>
static void sleep(u32 seconds)
{
	Sleep(seconds * 1000);
}
#endif

#define MAX_PACKET 4096
#define TEST_PACKET "Test Packet"

struct thread_data
{
	tukk pszServerAddr;
	i16 nPort;
	i32 nNumBytesToReceive;
	i32 nTotalPayloadSize;
};

uk CreateTCPEchoServer(uk param)
{
	CPassiveSocket socket;
	CActiveSocket *pClient = NULL;
	struct thread_data *pData = (struct thread_data *)param;
	i32 nBytesReceived = 0;

	socket.Initialize();
	socket.Listen(pData->pszServerAddr, pData->nPort);

	if ((pClient = socket.Accept()) != NULL)
	{
		while (nBytesReceived != pData->nTotalPayloadSize)
		{
			if (nBytesReceived += pClient->Receive(pData->nNumBytesToReceive))
			{
				pClient->Send((u8k *)pClient->GetData(), pClient->GetBytesReceived());
			}
		}

		sleep(100);

		delete pClient;
	}

	socket.Close();

	return NULL;
}

i32 main(i32 argc, char **argv)
{
	pthread_t threadId;
	struct thread_data thData;
	CActiveSocket client;
	char result[1024];

	thData.pszServerAddr = "127.0.0.1";
	thData.nPort = 6789;
	thData.nNumBytesToReceive = 1;
	thData.nTotalPayloadSize = (i32)strlen(TEST_PACKET);

	pthread_create(&threadId, 0, CreateTCPEchoServer, &thData);
	sleep(1);  // allow a second for the thread to create and listen

	client.Initialize();
	client.SetNonblocking();

	if (client.Open("127.0.0.1", 6789))
	{
		if (client.Send((u8 *)TEST_PACKET, strlen(TEST_PACKET)))
		{
			i32 numBytes = -1;
			i32 bytesReceived = 0;

			client.Select();

			while (bytesReceived != strlen(TEST_PACKET))
			{
				numBytes = client.Receive(MAX_PACKET);

				if (numBytes > 0)
				{
					bytesReceived += numBytes;
					memset(result, 0, 1024);
					memcpy(result, client.GetData(), numBytes);
					printf("received %d bytes: '%s'\n", numBytes, result);
				}
				else
				{
					printf("Received %d bytes\n", numBytes);
				}
			}
		}
	}
}
