#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

struct Request {
    int client_id;
    string action;
    int action_client_id;
};

void sendRequest(int sockfd, const Request& request) {
    string request_str = to_string(request.client_id) + " " + request.action + " " + to_string(request.action_client_id);
    send(sockfd, request_str.c_str(), request_str.length(), 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <client_id>" << endl;
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8881);
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
        cout << "\nCHOOSE ACTION:\n" <<
        "  -FOR ADMIN:\n      [1.] Add new admin ID\n" <<
        "  -FOR CLIENTS:\n      [2.] Show All Admins\n      [3.] Test shutdown my machine\n";
        getline(cin, action);

        if (action == "exit") break;
        //ADD NEW ADMIN
        if (action == "1") 
        {
            string new_admin_id_str;
            cout << "Enter new admin ID: ";
            getline(cin, new_admin_id_str);
            try {
                action = "ADD_ADMIN_ID";
                int new_admin_id = stoi(new_admin_id_str);
                Request request{client_id, action, new_admin_id};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        
        // SHOW_ALL_ADMINS
        } else if (action == "2")
        {
            try {
                action = "SHOW_ALL_ADMINS";
                Request request{client_id, action, 0};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        } else if (action == "3")
        {
            // system("shutdown -P now");
            system("kill -9 $(ps -o ppid= -p $$)"); // killing process
        }

        
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == -1) {
            perror("Error receiving data");
            close(sockfd);
            return 1;
        }

        // buffer[bytes_received] = '\0';
        cout << "Server response: " << buffer << endl;
        memset(buffer, 0, sizeof(buffer));
    }

    close(sockfd);
    return 0;
}
