#include <iostream>
#include <WS2tcpip.h>
#include <unordered_map>
#include <MSWSock.h>
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "WS2_32.lib")
using namespace std;
constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 200;

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

enum IOType {IO_SEND,IO_RECV,IO_ACCEPT};

class EXP_OVER {
public:
	WSAOVERLAPPED m_over;
	IOType m_Iotype;
	WSABUF	m_wsa;
	char  m_buff[BUF_SIZE];
	EXP_OVER() {
		ZeroMemory(&m_over, sizeof(m_over));
		m_wsa.buf = m_buff;
		m_wsa.len = BUF_SIZE;
	}
	EXP_OVER(IOType iot) : m_Iotype(iot)
	{
		ZeroMemory(&m_over, sizeof(m_over));
		m_wsa.buf = m_buff;
		m_wsa.len = BUF_SIZE;
	}
};

class SESSION;
unordered_map<long long, SESSION> clients;
class SESSION {
	SOCKET client;
	EXP_OVER recv_over;
	long long m_id;
public:
	SESSION() { exit(-1); }
	SESSION(int id, SOCKET so) : m_id(id), client(so)
	{
		recv_wsabuf[0].buf = recv_mess;
	}
	~SESSION()
	{
		closesocket(client);
	}
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&recv_over.m_over, 0, sizeof(recv_over.m_over));
		WSARecv(client, &recv_over.m_wsa, 1, 0, &recv_flag, & recv_over.m_over, recv_callback);
	}
	void do_send(int sender_id, int num_bytes, char* mess)
	{
		EXP_OVER* o = new EXP_OVER(IO_SEND);
		o->m_buff[0] = num_bytes + 2;
		o->m_buff[1] = sender_id;
		memcpy(o->m_buff + 2, mess, num_bytes);
		WSASend(client, &o->m_wsa, 1, 0, 0, &o->m_over, nullptr);
	}
};


{
	int client_id = static_cast<int>(reinterpret_cast<long long>(over->hEvent));
	cout << "Client[" << client_id << "] sent: " << clients[client_id].c_mess << endl;
	for (auto& cl : clients)
		cl.second.do_send(client_id, num_bytes, clients[client_id].c_mess);
	clients[client_id].do_recv();
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	EXP_OVER* o = reinterpret_cast<EXP_OVER*>(over);
	delete o;
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);
	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort((HANDLE)server, h_iocp, 0, 0);

	SOCKET client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	EXP_OVER accept_over(IO_ACCEPT);
	AcceptEx(server, client_socket, &accept_over.m_buff, 0, 
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, 
		NULL, &accept_over.m_over);


	while (true) {
		DWORD num_bytes;
		ULONG_PTR
			key;
		LPOVERLAPPED over;
		GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		if (over == nullptr) {
			error_display(L"GetQueuedCompletionStatus() failed", GetLastError());
			continue;
		}
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(over);
		switch (exp_over->m_Iotype) {
		case IO_ACCEPT:
		{
			cout << "New client connected." << endl;
			AcceptEx(server, client_socket, &accept_over.m_buff, 0,
				sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
				NULL, &accept_over.m_over);
			clients.try_emplace(client_socket, client_socket);
			clients[client_socket].do_recv();
			break;
		}
		
	}

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	for (int i = 1; ; ++i) {
		SOCKET client = WSAAccept(server,
			reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, NULL, NULL);
		clients.try_emplace(i, i, client);
		clients[i].do_recv();
	}
	closesocket(server);
	WSACleanup();
}
