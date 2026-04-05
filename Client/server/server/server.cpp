#include <iostream>
#include <WS2tcpip.h>
#include <array>
#include <MSWSock.h>
#include "protocol.h"

#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "WS2_32.lib")
using namespace std;
constexpr int BUF_SIZE = 200;

enum IOType { IO_SEND, IO_RECV, IO_ACCEPT };

class EXP_OVER {
public:
	WSAOVERLAPPED m_over;
	IOType  m_iotype;
	WSABUF	m_wsa;
	char  m_buff[BUF_SIZE];
	EXP_OVER()
	{
		ZeroMemory(&m_over, sizeof(m_over));
		m_iotype = IO_RECV;
		m_wsa.buf = m_buff;
		m_wsa.len = BUF_SIZE;
	}
	EXP_OVER(IOType iot) : m_iotype(iot)
	{
		ZeroMemory(&m_over, sizeof(m_over));
		m_wsa.buf = m_buff;
		m_wsa.len = BUF_SIZE;
	}
};

void error_display(const wchar_t* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::wcout << msg;
	std::wcout << L" === 에러 " << lpMsgBuf << std::endl;
	while (true);   // 디버깅 용
	LocalFree(lpMsgBuf);
}

class SESSION;
std::array<SESSION, MAX_PLAYERS> clients;
class SESSION {
public:
	SOCKET client;
	long long m_id;
	bool is_connected;
public:
	EXP_OVER recv_over;
	SESSION() { 
		is_connected = false;
		m_id = 999;
		client = INVALID_SOCKET;
		recv_over.m_iotype = IO_RECV;
	}
	~SESSION()
	{
		if (is_connected)
			closesocket(client);
	}
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recv_over.m_over, 0, sizeof(recv_over.m_over));
		WSARecv(client, &recv_over.m_wsa, 1, 0, &recv_flag, &recv_over.m_over, nullptr);
	}
	void do_send(int sender_id, int num_bytes, char* mess)
	{
		EXP_OVER* o = new EXP_OVER(IO_SEND);
		o->m_wsa.len = num_bytes + 2;
		o->m_buff[0] = num_bytes + 2;
		o->m_buff[1] = sender_id;
		memcpy(o->m_buff + 2, mess, num_bytes);
		WSASend(client, &o->m_wsa, 1, 0, 0, &o->m_over, nullptr);
	}
};

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);
	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort((HANDLE)server, h_iocp, -1, 0);

	SOCKET client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	EXP_OVER accept_over(IO_ACCEPT);
	AcceptEx(server, client_socket, &accept_over.m_buff, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		NULL, &accept_over.m_over);

	for (int player_index = 0;;) {
		DWORD num_bytes;
		ULONG_PTR key;
		LPOVERLAPPED over;
		GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		if (over == nullptr) {
			error_display(L"GQCS Errror: ", WSAGetLastError());
			continue;
		}
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(over);
		switch (exp_over->m_iotype) {
		case IO_ACCEPT:
			cout << "Client connected." << endl;
			player_index = -1;
			for(int i =0; i < MAX_PLAYERS; ++i) {
				if (!clients[i].is_connected) {
					player_index = i;
					break;
				}
				if(player_index == -1) {
					cout << "Maximum player limit reached. Connection refused." << endl;
					send_login_fail(client_socket,"Server is full.");
					closesocket(client_socket);

				}
				else {
					CreateIoCompletionPort((HANDLE)client_socket, h_iocp, i, 0);
					clients[player_index].is_connected = true;
					if (clients[player_index].is_connected) {
						cout << "Player slot " << player_index << " is already occupied." << endl;
						closesocket(client_socket);
						break;
					}
					clients[i].do_recv();
					++player_index
				}
			}
			
			client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			AcceptEx(server, client_socket, &accept_over.m_buff, 0,
				sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
				NULL, &accept_over.m_over);
			break;
		case IO_RECV:
		{
			cout << "Received message." << endl;
			int client_id = static_cast<int>(key);
			cout << "Client[" << client_id << "] sent: "
				<< clients[client_id].recv_over.m_buff << endl;
			for (auto& cl : clients)
				cl.second.do_send(client_id, num_bytes, clients[client_id].recv_over.m_buff);
			clients[client_id].do_recv();
		}
		break;
		case IO_SEND: {
			cout << "Message sent." << endl;
			EXP_OVER* o = reinterpret_cast<EXP_OVER*>(over);
			delete o;
		}
					break;
		default:
			cout << "Unknown IO type." << endl;
			break;
		}
	}


	closesocket(server);
	WSACleanup();
}