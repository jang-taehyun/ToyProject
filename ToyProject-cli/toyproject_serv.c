#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <mysql/mysql.h>
#define BUF_SIZE 1024
#define CONNECT_ABLE 50

void ErrorHandling(const char* message)
{
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}

void finish_with_error(MYSQL* con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main(int argc, char* argv[])
{
        int ServerSocket, ClientSocket;
        struct sockaddr_in ServerAddress;
        struct sockaddr_in ClientAddress[CONNECT_ABLE];
        socklen_t AddressSize;

        fd_set reads, cpy_reads;
        struct timeval timeout;

        int FileDescriptorMax, SelectResult;

        int StringLength;
        char buf[BUF_SIZE];

        int FunctionResult;
        int ClientGroup[CONNECT_ABLE];

        MYSQL_RES *res;
        MYSQL_ROW row;
	    MYSQL *con = mysql_init(NULL);

  	    if (con == NULL)
    	{
        		fprintf(stderr, "%s\n", mysql_error(con));
       		exit(1);
    	}

  	    if (mysql_real_connect(con, "localhost", "root", "test123", NULL, 0, NULL, 0) == NULL)
    	{
          		fprintf(stderr, "%s\n", mysql_error(con));
          		mysql_close(con);
          		exit(1);
  	    }

        for(int i=0; i<CONNECT_ABLE; i++)
        {
                ClientGroup[i] = -1;
        }

        ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(-1 == ServerSocket)
                ErrorHandling("socket() error");

        AddressSize = sizeof(ServerAddress);
        memset(&ServerAddress, 0, AddressSize);
        ServerAddress.sin_family = AF_INET;
        ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        ServerAddress.sin_port = htons(3828);

        FunctionResult = bind(ServerSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));
        if(-1 == FunctionResult)
                ErrorHandling("bind() error");

        FunctionResult = listen(ServerSocket, 5);
        if(-1 == FunctionResult)
                ErrorHandling("listen() error");

        FD_ZERO(&reads);
        FD_SET(ServerSocket, &reads);

        FileDescriptorMax = ServerSocket;
        while(1)
        {
                cpy_reads = reads;

                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                SelectResult = select(FileDescriptorMax + 1, &cpy_reads, 0, 0, &timeout);
                if(-1 == SelectResult)
                {
                        puts("select() error");
                        break;
                }

                if(0 == SelectResult)
                {
                        continue;
                }

                for(int i=0; i<FileDescriptorMax+1; i++)
		{
                        if(FD_ISSET(i, &cpy_reads))
                        {
                                if(i == ServerSocket)
                                {
                                        int idx = -1;
                                        for(int j=0; j<CONNECT_ABLE; j++)
                                        {
                                                if(ClientGroup[j] == -1)
                                                {
                                                        idx = j;
                                                        break;
                                                }
                                        }

                                        if(-1 == idx)
                                        {
                                                puts("Cannot connect this client");
                                                continue;
                                        }

                                        ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress[idx], &AddressSize);
                                        if(-1 == ClientSocket)
                                                ErrorHandling("accept() error");

                                        ClientGroup[idx] = ClientSocket;

                                        FD_SET(ClientSocket, &reads);

                                        if(ClientSocket > FileDescriptorMax)
                                                FileDescriptorMax = ClientSocket;
                                        printf("connected client : %d\n", ClientSocket);
                                }
                                else
                                {
                                    memset(buf, 0, sizeof(buf));

                                        StringLength = read(i, buf, BUF_SIZE);
                                        if(-1 == StringLength)
                                                ErrorHandling("read() error");

                                        if(0 == StringLength)
                                        {
                                                for(int j=0; j<CONNECT_ABLE; j++)
                                                {
                                                        if(ClientGroup[j] == i)
                                                        {
                                                                ClientGroup[j] = -1;
                                                                break;
                                                        }
                                                }

                                                FD_CLR(i, &reads);

                                                close(i);

                                                printf("closed client : %d\n", i);
                                        }
                                        else
                                        {
                                            char query[2048] = "INSERT INTO tmp VALUES('";
                                            char adder[] = "')";
                                            strcat(query, buf);
                                            strcat(query, adder);

                                                if (mysql_query(con, query)) {
                                                    puts("failed to store data");
                                                    finish_with_error(con);
                                                }

                                                for(int j=0; j<CONNECT_ABLE; j++)
                                                {
                                                        if(ClientGroup[j] != -1)
                                                        {
                                                                StringLength = write(ClientGroup[j], buf, StringLength);
                                                                if(-1 == StringLength)
                                                                        ErrorHandling("write() error");
                                                        }
                                                }
                                        }

                                }
                        }
                }
        }

        close(ServerSocket);
        mysql_close(con);

        return 0;
}
