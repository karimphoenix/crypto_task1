#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#pragma warning(disable: 4996)

#include "md2.cpp"
SOCKET Connection;

using namespace std;

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



class Client{

public:

    Client(int id, string login, string passwd, int type):id(id), login(login), passwd(passwd), type(type){}

    int run(){
        SOCKADDR_IN addr;
        int sizeofaddr = sizeof(addr);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(1111);
        addr.sin_family = AF_INET;

        Connection = socket(AF_INET, SOCK_STREAM, NULL);
        if(connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
            std::cout << "Error: failed connect to server.\n";
            system("pause");
            return 1;
        }
        //std::cout << "Connected!\n";

        //CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

        std::string msg1;
        int ret = 0;
        do {
            //std::getline(std::cin, msg1);
            string sid = to_string(id);

            //cout << sid << ' ';
            int msg_size = sizeof(sid);
            send(Connection, (char*)&msg_size, sizeof(int), NULL);
            send(Connection, sid.c_str(), msg_size, NULL);
            cout << "Sent ID: " << id << endl;
            Sleep(1);

            msg_size = login.size();
            send(Connection, (char*)&msg_size, sizeof(int), NULL);
            send(Connection, login.c_str(), msg_size, NULL);
            cout << id << " Sent Login: " << login << endl;
            Sleep(1);

            string sendingPasswd;

            switch (type){
                case 1: {
                    sendingPasswd = passwd;
                    break;
                }
                case 2:{
                    sendingPasswd = applyHash(passwd);
                    break;
                }
                case 3:{
                    string sp = passwd + salt;
                    sendingPasswd = applyHash(sp);
                    break;
                }
                case 4:{
                    sendingPasswd = passwd;
                    for (int i = 0; i < w - co; ++i){
                        sendingPasswd = applyHash(sendingPasswd);
                    }
                    break;
                }

            }

            //sendingPasswd = applyHash(passwd);
            msg_size = sendingPasswd.size();
            //cout << id << "Apply hash: " << sendingPasswd << endl;
            send(Connection, (char*)&msg_size, sizeof(int), NULL);
            send(Connection, sendingPasswd.c_str(), msg_size, NULL);
            cout << id << " Sent password: " << sendingPasswd  << ' ' << msg_size << endl;
            Sleep(1);

            bool result = ClientHandler();
            cout << id << " Client handler result: " << result << endl;;
            if  (result and (type != 4)){
                break;
            }
            if (type == 4){
                    if (result)
                        co++;
                cout << "Authorization counter: " << w - co << endl;
                ret++;
            } else {
                cout << id << " Authorized failed!" << endl;
                ret++;
            }
            //cout << ret;
        } while (ret < 5);

        closesocket(Connection);
        return 0;

    }


private:
    SOCKET Connection;

    int id;
    string login, passwd;
    int type;
    const string salt = "QRmx";
    int w = 2;
    int co = 0;

    bool ClientHandler() {
	int msg_size;
	int ret;

		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char *msg = new char[msg_size + 1];
		cout << id << " Message size: " << msg_size << endl;
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << id << " Message from server: " << msg << std::endl;
		//system("pause");


	bool result = (strcmp(msg, "Client connected!") == 0);
	delete[] msg;

	return result;

    }

};


void ClientRun(Client* inclient){
    //Client cl(id, login, passwd);

    int result = inclient->run();
    if (result != 0){
        cout << "Error!" << endl;
        //return result;
    }
}

vector<Client*> loadBase(string fname, int op){
   fstream in;
   in.open(fname);

   unsigned int id;
   string login, passwd/*(16, ' ')*/;
   vector<Client*> Clients;

   if (op == 5){
        id = 1;
        login = "login";
        passwd = "password";
        Client* cl = new Client(id, login, passwd, 1);
        Clients.push_back(cl);

        return Clients;
   }

   while(!in.eof()){
        char c;
        in >> id >> login >> passwd;
        //cout << c;
        /*for (int i = 0; i < 16; ++i){
            in >> c;
            passwd[i] = c;
        }*/

        //cout << id << ' ' << login << ' ' << passwd << endl;
        Client* cl = new Client(id, login, passwd, op);
        Clients.push_back(cl);
   }

   in.close();

   return Clients;

   //cout << TableClients.size();
}


int main(int argc, char* argv[]) {
	/*if (argc < 3){
        cout << "Not enough arguments!";
        system("pause");
        return 1;
	}*/

	string sid = argv[1];
	unsigned int op = stoi(sid);
    /*string login, passwd;
    login = argv[2];
    passwd = argv[3];
    */
    cout << op;

	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if(WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	int countOfRunning = 1;

	//vector<HANDLE> threads;
	vector<Client*> clients;
    HANDLE threads[countOfRunning];

	clients = loadBase("Base.txt", op);
	//cout << clients.size();

	for (int i = 0; i < countOfRunning; ++i){
        //Client* client = new Client(id, login, passwd);

        HANDLE handle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientRun, (LPVOID)(clients[i]), NULL, NULL);
        //clients.push_back(client);
        //threads.push_back(handle);
        threads[i] = handle;

	}

    //for (auto handle : threads){
        //WaitForSingleObject(handle, INFINITE);
    //}
    WaitForMultipleObjects(countOfRunning, threads, TRUE, INFINITE);

    for (auto client : clients){
        delete client;
    }

    WSACleanup();

	system("pause");
	return 0;
}
