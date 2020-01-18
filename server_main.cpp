#include <winsock2.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <fstream>

#include "md2.cpp"

//#include <E:\my_programs\c++\cryptolib\md2.cpp>

using namespace std;

const int maxConnections = 1000;

SOCKET Connections[maxConnections];
int Counter = 0;

using client = tuple<unsigned int, string, string>;

vector<client> TableClients;

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

bool addClient (string fname, string ClientLogin, string ClientPasswd, unsigned int clientID){
    ofstream out;
    out.open(fname, ios_base::app);
    if (out.is_open()){
        //print
        out << clientID << ' ' << ClientLogin << ' ' << ClientPasswd << endl;
    }
    out.close();
    return !out.is_open();
}

bool CheckClient (client incomingClient){

    string checkingLogin, checkingPasswd;
    unsigned int checkingID;

    //client candidate = find(TableClients.begin(), TableClients.end(), incomingClient);

    for (auto cl:TableClients){
        string checkingLogin, checkingPasswd;
        unsigned int checkingID;

        std::tie(checkingID, checkingLogin, checkingPasswd) = cl;

        string icheckingLogin, icheckingPasswd;
        unsigned int icheckingID;

        std::tie(icheckingID, icheckingLogin, icheckingPasswd) = incomingClient;

        string hashedIncomingPass = applyHash(icheckingPasswd);
        cout << checkingPasswd;

        if ((checkingID == icheckingID) and (checkingLogin == icheckingLogin) and (checkingPasswd == hashedIncomingPass)){
            cout << checkingLogin << " connected!" << endl;
            return true;
        }

    }

    return false;
}

void ClientHandler(int index) {
	int msg_size;
	while(true) {

        recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *cID = new char[msg_size + 1];
		cID[msg_size] = '\0';
		recv(Connections[index], cID, msg_size, NULL);

		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *cLogin = new char[msg_size + 1];
		cLogin[msg_size] = '\0';
		recv(Connections[index], cLogin, msg_size, NULL);

		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *cPasswd = new char[msg_size + 1];
		cPasswd[msg_size] = '\0';
		recv(Connections[index], cPasswd, msg_size, NULL);

		if  (CheckClient(make_tuple(index, cLogin, cPasswd))){
            send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
			char *msg = new char[msg_size + 1];
			msg[msg_size] = '\0';
			msg = "Client connected!";
			send(Connections[index], msg, msg_size, NULL);

			cout << "Client " << cLogin << " connected!" << endl;
			delete[] msg;

		} else {
            string msg = "Wrong login or password!";
            msg_size = msg.size();
            send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
			//char *msg = new char[msg_size + 1];
			//msg[msg_size] = '\0';
			send(Connections[index], msg.c_str(), msg_size, NULL);

			cout << "Client " << cLogin << " connection failed!" << endl;
		}
		/*
		for(int i = 0; i < Counter; i++) {
			if(i == index) {
				continue;
			}
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
		}*/
		delete[] cLogin;
		delete[] cPasswd;


	}
}

void loadBase(){
   fstream in;
   in.open("Base.txt");

   unsigned int id;
   string login, passwd(16, ' ');

   while(!in.eof()){
        in >> id >> login;
        char c;
        for (int i = 0; i < 16; ++i){
            in >> c;
            passwd[i] = c;
        }

        cout << id << ' ' << login << ' ' << passwd << endl;
        TableClients.emplace_back(make_tuple(id, login, passwd));
   }

   in.close();
}

int main(int argc, char* argv[]) {

	int OperCode = 0;
	cin >> OperCode;



	if  (OperCode == 1){
        cout << "Adding new user" << endl;
        string login, passwd;
        unsigned int clientID;
        cin >> login;
        cin >> passwd;
        cin >> clientID;
        //applyHash(passwd);
        string ans = applyHash(passwd);
        string ans1 = applyHash(passwd);

        if (ans == ans1){
            cout << "DA BLYAT!" << endl;
        }

        addClient("Base.txt", login, ans, clientID);
	}   else {

        loadBase();
        cout << "Base loading!" << endl;

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

        SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
        bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
        listen(sListen, SOMAXCONN);

        SOCKET newConnection;
        for(int i = 0; i < maxConnections; i++) {
            newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

            if(newConnection == 0) {
                std::cout << "Error #2\n";
            } else {
                std::cout << "Client check started!\n";
                //std::string msg = "Hello. It`s my first network program!";
                //int msg_size = msg.size();
                //send(newConnection, (char*)&msg_size, sizeof(int), NULL);
                //send(newConnection, msg.c_str(), msg_size, NULL);

                Connections[i] = newConnection;
                Counter++;
                CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
            }
        }


        system("pause");
        return 0;
	}
}
