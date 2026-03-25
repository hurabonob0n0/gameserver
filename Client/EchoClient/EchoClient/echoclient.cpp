#include <iostream>
#include <WS2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

const char* SERVER_IP = "127.0.0.1";
constexpr short SERVER_PORT = 54000;
constexpr size_t BUFFER_SIZE = 4096;

int main() 
{
	std::wcout.imbue(std::locale("korean"));
	WSAStartup(MAKEWORD(2, 2), nullptr);
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
	SOCKADDR_IN	 server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
	WSAConnect(s_socket, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr), nullptr, nullptr, nullptr, nullptr);
	for(;;)
	{
		std::string input;
		std::cout << "Input: ";
		std::getline(std::cin, input);
		if (input.empty())
			break;

		WSABUF wsa_buf{ BUFFER_SIZE, const_cast<char*>(input.c_str()) };
		WSASend(s_socket, &wsa_buf,1, static_cast<int>(input.size()), 0);
		
		char buffer[BUFFER_SIZE];
		int bytes_received = recv(s_socket, buffer, BUFFER_SIZE, 0);
		if (bytes_received > 0)
		{
			std::string response(buffer, bytes_received);
			std::cout << "Echoed: " << response << std::endl;
		}
	}
}