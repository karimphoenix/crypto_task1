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
string salt = "GlTl";

void make_digest(char *md5str, const unsigned char *digest, int len) /* {{{ */
{
 static const char hexits[17] = "0123456789abcdef";
 int i;

 for (i = 0; i < len; i++) {
  md5str[i * 2]       = hexits[digest[i] >> 4];
  md5str[(i * 2) + 1] = hexits[digest[i] &  0x0F];
 }
 md5str[len * 2] = '\0';
}

string applyHash(string& incomingValue){
    MD2_CTX context;
    //vector<unsigned char> retHash;
    //string retHash;

    char *Hash;
    char md2str[33];
    md2str[0] = '\0';
    string retHash(16, ' ');
    unsigned char digest[16];
    unsigned int len = incomingValue.size();

    MD2Init(&context);
    MD2Update(&context, (unsigned char*)incomingValue.c_str(), len);
    MD2Final(digest, &context);
    //MD2Final(retHash.c_str(), &context);

    /*for (int i = 0; i < 16; ++i)
        retHash[i] = digest[i];
    retHash[17] = '\0';
    return retHash;*/

    make_digest(md2str, digest, 16);
    Hash = md2str;
    return string(Hash);
}


bool addClient (string fname, string ClientLogin, string ClientPasswd, unsigned int clientID){
    ofstream out;
    out.open(fname, ios_base::app);
    if (out.is_open()){
        //print\

        string outStr = to_string(clientID) + ' ' + ClientLogin + ' ' + ClientPasswd + '\n';
        out << outStr;
        //out << clientID << ' ' << ClientLogin << ' ' << ClientPasswd << endl;
    }
    out.close();
    return !out.is_open();
}

bool CheckClient (client incomingClient, int variant){

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

        if  (variant = 1){
            string hashedIncomingPass = applyHash(icheckingPasswd);
            cout << checkingID << ' ' << checkingLogin << ' ' << checkingPasswd << ' ' << checkingPasswd.size() << endl;
            cout << icheckingID << ' ' << icheckingLogin << ' ' << hashedIncomingPass << ' ' << hashedIncomingPass.size() << endl;
        }

        if (variant = 2){
            icheckingPasswd = icheckingPasswd + salt;
            string hashedIncomingPass = applyHash(icheckingPasswd);
            cout << checkingID << ' ' << checkingLogin << ' ' << checkingPasswd << ' ' << checkingPasswd.size() << endl;
            cout << icheckingID << ' ' << icheckingLogin << ' ' << hashedIncomingPass << ' ' << hashedIncomingPass.size() << endl;
        }


        if ((checkingID == icheckingID) and (checkingLogin == icheckingLogin) and (checkingPasswd == hashedIncomingPass)){
            cout << checkingLogin << " connected!" << endl;
            return true;
        }

    }

    return false;
}

void ClientHandler(int index) {
	int msg_size;
	int ret;
	do {

        ret = recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
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


		int id = atoi(cID);

		if  (CheckClient(make_tuple(id, cLogin, cPasswd))){
            send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
			char *msg = new char[msg_size + 1];
			msg[msg_size] = '\0';
			msg = "Client connected!";
			ret = send(Connections[index], msg, msg_size, NULL);

			cout << "Client " << cLogin << " connected!" << endl;
			delete[] msg;
			//break;

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


	} while (ret > 0);
	//closesocket(Connections[index]);
	//Counter--;
}

void loadBase(string fname){
   fstream in;
   in.open(fname);

   unsigned int id;
   string login, passwd/*(16, ' ')*/;

   while(!in.eof()){
        char c;
        in >> id >> login >> passwd;
        //cout << c;
        /*for (int i = 0; i < 16; ++i){
            in >> c;
            passwd[i] = c;
        }*/

        //cout << id << ' ' << login << ' ' << passwd << endl;
        TableClients.emplace_back(make_tuple(id, login, passwd));
   }

   in.close();

   //cout << TableClients.size();
}

void algorithmVar(int& variant){
    switch (variant){

    case 1:{
        loadBase("BaseHash.txt");

    }
    case 2:{
        loadBase("BaseSalt.txt")
    }

    case 3:{
        loadBase("BaseLamp.txt")
    }
    }


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

        addClient("Base.txt", login, ans, clientID);
	}   else {

        loadBase("Base.txt");
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
        while(Counter < maxConnections + 1) {
            newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

            if(newConnection == 0) {
                std::cout << "Error #2\n";
                break;
            } else {
                std::cout << "Client check started!\n";
                //std::string msg = "Hello. It`s my first network program!";
                //int msg_size = msg.size();
                //send(newConnection, (char*)&msg_size, sizeof(int), NULL);
                //send(newConnection, msg.c_str(), msg_size, NULL);

                Connections[Counter] = newConnection;
                Counter++;
                CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(Counter-1), NULL, NULL);
                //Counter--;
                cout << "Connections: " << Counter << endl;
            }
        }
	}

	WSACleanup();

    system("pause");
    return 0;
}
