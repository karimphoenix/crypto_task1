#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#pragma warning(disable: 4996)

#include "md2.cpp"
SOCKET Connection;

using namespace std;

string applyHash(string incomingValue){
    MD2_CTX context;
    //vector<unsigned char> retHash;
    //string retHash;
    string retHash(17, ' ');
    unsigned char digest[16];
    unsigned int len = incomingValue.size();

    MD2Init(&context);
    MD2Update(&context, (unsigned char*)incomingValue.c_str(), len);
    MD2Final(digest, &context);

    for (int i = 0; i < 16; ++i)
        retHash[i] = digest[i];
    retHash[17] = '\0';
    return retHash;
}

void ClientHandler() {
	int msg_size;
	while(true) {
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char *msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << msg << std::endl;
		delete[] msg;
	}
}

int main(int argc, char* argv[]) {
	if (argc < 3){
        cout << "Not enough arguments!";
        return 1;
	}

	string sid = argv[1];
	unsigned int id = stoi(sid);
    string login, passwd;
    login = argv[2];
    passwd = argv[3];

    passwd = applyHash(passwd);

	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if(WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if(connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}
	std::cout << "Connected!\n";

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	std::string msg1;
	while(true) {
		//std::getline(std::cin, msg1);
		int msg_size = sizeof(id);
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		sid = to_string(id);
		send(Connection, sid.c_str(), msg_size, NULL);
		Sleep(1);

		msg_size = login.size();
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, login.c_str(), msg_size, NULL);
		Sleep(1);

		msg_size = passwd.size();
        cout << passwd;
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, passwd.c_str(), msg_size, NULL);
		Sleep(1);
	}

	system("pause");
	return 0;
}
