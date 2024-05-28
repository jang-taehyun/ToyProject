#include "header.h"
#include "SocketClass.h"

SocketClass::SocketClass()
{
	IsCreate = false;
}

SocketClass::~SocketClass() { }

bool SocketClass::init()
{
	int FunctionResult;
	struct hostent* host;

	host = gethostbyname("serv-alb-928864148.ap-northeast-2.elb.amazonaws.com");
	if (!host)
		return false;


	FunctionResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (FunctionResult)
	{
		puts("WSAStartup() false");
		WSACleanup();

		return false;
	}

	ConnectSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == ConnectSocket)
	{
		puts("socket() error");
		WSACleanup();
		closesocket(ConnectSocket);

		return false;
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
	ServerAddress.sin_port = htons(3828);

	FunctionResult = connect(ConnectSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress));
	if (SOCKET_ERROR == FunctionResult)
	{
		puts("connect() error");
		return false;
	}

	IsCreate = true;

	return true;
}

bool SocketClass::release()
{
	closesocket(ConnectSocket);
	WSACleanup();

	IsCreate = false;

	return true;
}

bool SocketClass::DestorySocketInstance()
{
	if (!IsCreate)
	{
		puts("socket instance already released");
		return false;
	}

	return release();
}

bool SocketClass::CreateSocketInstance()
{
	if (IsCreate)
	{
		puts("Socket instance already created");
		return false;
	}

	return init();
}

bool SocketClass::SendDataToServer(const char* content)
{
	int FunctionResult, StringLength;

	if (!IsCreate)
	{
		puts("socket instance is not created");
		return false;
	}

	StringLength = send(ConnectSocket, content, strlen(content), 0);
	if (SOCKET_ERROR == StringLength)
	{
		puts("send() error");
		return false;
	}

	return true;
}

bool SocketClass::ReceiveDataFromServer(char* p)
{
	int FunctionResult, StringLength;
	char buf[BUF_SIZE];
	memset(buf, 0, sizeof(buf));

	if (!IsCreate)
	{
		puts("socket instance is not created");
		return false;
	}

	StringLength = recv(ConnectSocket, buf, sizeof(buf) - 1, 0);
	if (SOCKET_ERROR == StringLength)
	{
		puts("recv() error");
		return false;
	}

	strcpy(p, buf);

	return false;
}