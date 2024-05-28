#ifndef SOCKET_CLASS
#define SOCKET_CLASS

#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

class SocketClass
{
private:
	WSADATA wsaData;
	SOCKET ConnectSocket;
	SOCKADDR_IN ServerAddress;
	bool IsCreate;

private:
	SocketClass();
	~SocketClass();

public:
	bool DestorySocketInstance();
	bool CreateSocketInstance();
	bool SendDataToServer(const char* content);
	bool ReceiveDataFromServer(char* p);

	bool GetState() { return IsCreate; }
	SOCKET GetSocket() { return ConnectSocket; }

private:
	bool init();
	bool release();

public:
	static SocketClass* GetSocketInstance()
	{
		static SocketClass inst;
		if (!inst.GetState())
			if (!inst.CreateSocketInstance())
				return nullptr;

		return &inst;
	}
};

#endif // !SOCKET_CLASS
