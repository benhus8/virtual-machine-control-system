#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring> //memset

using namespace std;

struct Request {
    int client_id;
    string action;
    int action_client_id;
};

void sendRequest(int sockfd, const Request& request) {
    string request_str = to_string(request.client_id) + " " + request.action;
    send(sockfd, request_str.c_str(), request_str.length(), 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <client_id>" << endl;
        return 1;
    }
    //config conenction with server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888); 
    inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }
    

    int client_id = stoi(argv[1]);
    string action;
    char buffer[1024] = {0};
    while (true) {
        cout << "\nEnter action: ";
        getline(cin, action);

        if(action == "exit") break;
        if(action == "ADD_ADMIN_ID") {
            
        }

        Request request{client_id, action};
        sendRequest(sockfd, request);
        //TODO RECEIVE RESPONSE MESSAGE
        // recv(sockfd, buffer, sizeof(buffer), 0);
        // cout << buffer << "\n";
        // memset(buffer, 0, sizeof(buffer));
    }
    
    close(sockfd);
    return 0;
}
