#include "header.h"
#include <thread>

char buf[BUF_SIZE];
bool IsFinish = false;

void InputData()
{
	fputs("Insert message(q to quit)\n", stdout);

	while (1)
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), stdin);
		if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
			break;

		SocketClass::GetSocketInstance()->SendDataToServer(buf);
	}
	
	IsFinish = true;
}

void ReceiveData()
{
	fd_set originRead, cpyRead;
	TIMEVAL timeout;
	int SelectResult;
	char buf[BUF_SIZE];

	FD_ZERO(&originRead);
	FD_SET(SocketClass::GetSocketInstance()->GetSocket(), &originRead);

	while (1)
	{
		if (IsFinish)
			break;

		memset(buf, 0, sizeof(buf));
		timeout.tv_sec = 0;
		timeout.tv_usec = 10;

		cpyRead = originRead;

		SelectResult = select(0, &cpyRead, 0, 0, &timeout);
		if (SOCKET_ERROR == SelectResult)
		{
			puts("select() error");
			break;
		}

		for (int i = 0; i < cpyRead.fd_count; i++)
		{
			if (FD_ISSET(cpyRead.fd_array[i], &cpyRead))
			{
				if (cpyRead.fd_array[i] == SocketClass::GetSocketInstance()->GetSocket())
				{
					SocketClass::GetSocketInstance()->ReceiveDataFromServer(buf);
					printf("%s", buf);
				}
			}
		}
	}
}

int main()
{
	if (nullptr == SocketClass::GetSocketInstance())
	{
		puts("fail to exectue program");
		return -1;
	}

	int StringLength;

	std::thread t1(InputData);
	std::thread t2(ReceiveData);
	t1.join();
	t2.join();	

	SocketClass::GetSocketInstance()->DestorySocketInstance();

	return 0;
}