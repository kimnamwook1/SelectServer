#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

char Map[10][10] = {
	{'0','0','0','0','0','0','0','0','0','0'},
	{'0','1','1','1','1','1','1','1','1','0'},
	{'0','1','1','1','1','1','1','1','1','0'},
	{'0','1','1','1','1','1','0','1','1','0'},
	{'0','1','1','1','1','1','1','1','1','0'},
	{'0','1','1','0','1','1','1','1','1','0'},
	{'0','1','1','1','1','1','1','1','1','0'},
	{'0','1','1','1','1','1','1','1','1','0'},
	{'0','1','1','1','1','1','1','1','1','0'},
	{'0','0','0','0','0','0','0','0','0','0'}
};

int main()
{
	int X = 1;
	int Y = 1;
	bool bRunning = true;

	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN ServerAddrIn;
	memset(&ServerAddrIn, 0, sizeof(SOCKADDR_IN));
	ServerAddrIn.sin_family = AF_INET;
	ServerAddrIn.sin_port = htons(1234);
	ServerAddrIn.sin_addr.s_addr = INADDR_ANY;

	fd_set Original; //os한테 특정 이벤트가 발생하면 나한테 알려줘 하는 구조체
	fd_set CopyReads; // 원본을 바꾸지 않고 사본을 바꾸어서 체크한다.
	timeval Timeout;
	Timeout.tv_sec = 0; // s 
	Timeout.tv_usec = 500; //ms

	FD_ZERO(&Original);
	FD_SET(ServerSocket, &Original); //서버 소켓에 무슨일이 생기면 나한테 알려줘.

	bind(ServerSocket, (SOCKADDR*)&ServerAddrIn, sizeof(SOCKADDR_IN));

	listen(ServerSocket, 0);

	while (bRunning)
	{
		CopyReads = Original;
		//polling
		int fd_num = select(0, &CopyReads, 0, 0, &Timeout);

		if (fd_num == 0)
		{
			//another process
			continue;
		}
		
		if(fd_num == SOCKET_ERROR)
		{
			bRunning = false;
			break;
		}

		for (size_t i = 0; i < Original.fd_count; i++)
		{	//등록한 소켓 리스트 중에 이벤트가 발생했음.
			if (FD_ISSET(Original.fd_array[i], &CopyReads)) //세팅 되었니?
			{
				if (Original.fd_array[i] == ServerSocket)
				{
					//서버 소켓에 이벤트가 발생함. 연결 요청
					SOCKADDR_IN ClientSockAddrIn;
					memset((char*)&ClientSockAddrIn, 0, sizeof(SOCKADDR_IN));
					int ClientSockAddrInSize = sizeof(SOCKADDR_IN);
					SOCKET ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientSockAddrIn, &ClientSockAddrInSize);
					FD_SET(ClientSocket, &Original);  //감시할 소켓을 추가 
					std::cout << "Connected Client : " << ClientSocket << std::endl;
				}
				else
				{
					SOCKET ClientSocket = Original.fd_array[i];
					//Client 소켓, Recv, Send
					char Buffer[1024] = { 0, };
	
					int RecvLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0);

					if (RecvLength == 0) //클라이언트가 연결이 해제된 것
					{
						std::cout << "Disconnected Client" << std::endl;
						FD_CLR(ClientSocket, &Original); //감지 리스트에서 빼주세요.
						closesocket(ClientSocket);
					}
					else if (RecvLength < 0)
					{
						std::cout << "Disconnected Client By Error" << GetLastError() << std::endl;
						FD_CLR(ClientSocket, &Original); //감지 리스트에서 빼주세요.
						closesocket(ClientSocket);
					}
					else 
					{
						//자료가 있으면 처리
						char KeyCode = Buffer[0];
						switch (KeyCode)
						{
						case 'w':
							if (Map[Y - 1][X] == '1')
							{
								Y--;
							}
							break;
						case 's':
							if (Map[Y + 1][X] == '1')
							{
								Y++;
							}
							break;
						case 'a':
							if (Map[Y][X - 1] == '1')
							{
								X--;
							}
							break;
						case 'd':
							if (Map[Y][X + 1] == '1')
							{
								X++;
							}
							break;
						}
						//std::cout << "(" << X << "," << Y << ")" << std::endl;
						system("cls");
						for (int MapY = 0; MapY < 10; ++MapY)
						{
							for (int MapX = 0; MapX < 10; ++MapX)
							{
								if (Map[MapY][MapX] == '0')
								{
									std::cout << "#" << " ";
								}
								else if (MapX == X && MapY == Y)
								{
									std::cout << "P" << " ";
								}
								else
								{
									std::cout << " " << " ";
								}
							}
							std::cout << std::endl;
						}
						memcpy(&Buffer[0], (char*)&X, 4);
						memcpy(&Buffer[4], (char*)&Y, 4);

						for (size_t i = 0; i < Original.fd_count; ++i)
						{
							if (Original.fd_array[i] != ServerSocket)
							{
								int SendLength = send(Original.fd_array[i], Buffer, 8, 0);
							}
						}
					}
				}
			}	
		}
	}
	closesocket(ServerSocket);

	WSACleanup();
	return 0;
}