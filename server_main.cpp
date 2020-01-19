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
int numberOfConnections = 0;
int OperCode = 0;

using client = tuple<unsigned int, string, string>;

vector<client> TableClients;
const string salt = "QRmx";

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

    //for (auto cl:TableClients){
    for (int i = 0; i < TableClients.size(); ++i){
        string checkingLogin, checkingPasswd;
        unsigned int checkingID;

        //std::tie(checkingID, checkingLogin, checkingPasswd) = cl;
        std::tie(checkingID, checkingLogin, checkingPasswd) = TableClients[i];

        string icheckingLogin, icheckingPasswd;
        unsigned int icheckingID;

        std::tie(icheckingID, icheckingLogin, icheckingPasswd) = incomingClient;
        string hashedIncomingPass;
        if  (variant == 1){
            hashedIncomingPass = applyHash(icheckingPasswd);
        }

        if (variant == 2){
            hashedIncomingPass = icheckingPasswd;
        }

        if (variant == 3){
            hashedIncomingPass = icheckingPasswd;
        }

        if (variant == 4){
            hashedIncomingPass = applyHash(icheckingPasswd);
            //TableClients[i] = make_tuple(checkingID, checkingLogin, icheckingPasswd);
        }


        if ((checkingID == icheckingID) and (checkingLogin == icheckingLogin) and (checkingPasswd == hashedIncomingPass)){
            cout << checkingLogin << " connected!" << endl;

            if (variant == 4)
               TableClients[i] = make_tuple(checkingID, checkingLogin, icheckingPasswd);
               // checkingPasswd = icheckingPasswd;

            return true;
        }

    }

    return false;
}

void ClientHandler(int index) {
	int msg_size;
	int ret;
	int failedAuthorizations = 0;
	do {

        ret = recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *cID = new char[msg_size + 1];
		cID[msg_size] = '\0';
		recv(Connections[index], cID, msg_size, NULL);
		cout << "Read id: " << cID << endl;

		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *cLogin = new char[msg_size + 1];
		cLogin[msg_size] = '\0';
		recv(Connections[index], cLogin, msg_size, NULL);
		cout << "Read login: " << cLogin << endl;

		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *cPasswd = new char[msg_size + 1];
		cPasswd[msg_size] = '\0';
		recv(Connections[index], cPasswd, msg_size, NULL);
		cout << "Read password: " << cPasswd << ' ' << msg_size << endl;


		int id = atoi(cID);

		if  (CheckClient(make_tuple(id, cLogin, cPasswd), OperCode)){
            string msg = "Client connected!";
            msg_size = msg.size();
            send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[index], msg.c_str(), msg_size, NULL);

			cout << "Client " << cLogin << " authorized!" << endl;
			//delete[] msg;
			if (OperCode != 4)
                break;

		} else {
            string msg = "Wrong login or password!";
            msg_size = msg.size();
            send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
			//char *msg = new char[msg_size + 1];
			//msg[msg_size] = '\0';
			send(Connections[index], msg.c_str(), msg_size, NULL);
            failedAuthorizations++;
			cout << "Client " << cLogin << " authorized failed!" << endl;
		}
		/*
		for(int i = 0; i < numberOfConnections; i++) {
			if(i == index) {
				continue;
			}
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
		}*/
		delete[] cLogin;
		delete[] cPasswd;


	} while (failedAuthorizations < 5);
	closesocket(Connections[index]);
	numberOfConnections--;
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
            break;
        }

        case 2:{
            loadBase("BaseHash.txt");
            break;
        }

        case 3:{
            loadBase("BaseSalt.txt");
            break;
        }

        case 4:{
            loadBase("BaseLamp.txt");
            break;
        }
    }


}

int main(int argc, char* argv[]) {

	//int OperCode = 0;
	cin >> OperCode;



	if  (OperCode == 5){
	}   else {

        algorithmVar(OperCode);
        //loadBase("Base.txt");
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
        //vector<HANDLE> threads;
        HANDLE threads[maxConnections];
        HANDLE hl;
        while(numberOfConnections < maxConnections + 1) {
            newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

            if(newConnection == 0) {
                std::cout << "Error invalid connections!\n";
                break;
            } else {
                std::cout << "New client connected!\n";
                //std::string msg = "Hello. It`s my first network program!";
                //int msg_size = msg.size();
                //send(newConnection, (char*)&msg_size, sizeof(int), NULL);
                //send(newConnection, msg.c_str(), msg_size, NULL);

                Connections[numberOfConnections] = newConnection;
                numberOfConnections++;
                hl = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(numberOfConnections-1), NULL, NULL);
                threads[numberOfConnections-1] = hl;
                //numberOfConnections--;
                cout << "Number of connections: " << numberOfConnections << endl;
            }
        }
        //for (auto handle: threads)
        WaitForMultipleObjects(maxConnections, threads, TRUE, INFINITE);
	}

	WSACleanup();

    system("pause");
    return 0;
}
