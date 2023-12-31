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
    int action_1_client_id;
    int action_2_client_id;
};

void sendRequest(int sockfd, const Request& request) {
    string request_str = to_string(request.client_id) + " " + request.action + " " + to_string(request.action_1_client_id) + " " + to_string(request.action_2_client_id);
    send(sockfd, request_str.c_str(), request_str.length(), 0);
}

void* receiveThread(void* arg) {
    int sockfd = *(int*)arg;
    char buffer[1024] = {0};
    ssize_t bytes_received;

    while (true) {
        bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == -1) {
            perror("Error receiving data");
            close(sockfd);
            pthread_exit(NULL);
        }
        string server_response(buffer);
        if(server_response == "SHUTDOWN") {
            string accept_shutdown = "SUCCESS";
            send(sockfd, accept_shutdown.c_str(), accept_shutdown.length(), 0);
            sleep(1);
            system("kill -9 $(ps -o ppid= -p $$)");
            // system("shutdown -P now");
        }

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            cout << "Server response: " << buffer << endl;
            memset(buffer, 0, sizeof(buffer));
        }
    }
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

    // creating thread to listen for server commands
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receiveThread, (void*)&sockfd) != 0) {
        perror("Failed to create receive thread");
        close(sockfd);
        return 1;
    }

    Request request{client_id, "REGISTER", -1, -1};
    sendRequest(sockfd, request);
    sleep(1);
    while (true) {
        
        cout << "\nCHOOSE ACTION:\n" <<
        "  -FOR ADMIN:\n      [1.] Add new admin ID\n      [2.] Add permission for client\n      [3.] Show active clients\n      [4.] Show all admins\n" <<
        "  -FOR CLIENTS:\n      [5.] Shutdown client\n" <<
        "  [6.] Exit application\n";
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
                Request request{client_id, action, new_admin_id, -1};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        
        // ADD_PERMISSION
        } else if (action == "2")
        {
            string client_id_to_add_permission;
            cout << "Add permission for client with id: ";
            getline(cin, client_id_to_add_permission);

            string client_id_to_be_shutdowned;
            cout << "To shutdown client with id: ";
            getline(cin, client_id_to_be_shutdowned);

            try {
                action = "ADD_PERMISSION";
                int clinet_id_permission = stoi(client_id_to_add_permission);
                int clinet_id_shutdown = stoi(client_id_to_be_shutdowned);
                Request request{client_id, action, clinet_id_permission, clinet_id_shutdown};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        //SHOW_ALL_ACTIVE_CLIENTS    
        } else if (action == "3")
        {
            try {
                action = "SHOW_ALL_ACTIVE_CLIENTS";
                Request request{client_id, action, -1, -1};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        //SHOW_ALL_ADMINS    
        }else if (action == "4")
        {
            try {
                action = "SHOW_ALL_ADMINS";
                Request request{client_id, action, -1, -1};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        //SHUTDOWN_CLIENT    
        } else if (action == "5")
        {
            string client_id_to_shutdown;
            cout << "Client id to shutdown: ";
            getline(cin, client_id_to_shutdown);
            try {
                action = "SHUTDOWN_CLIENT";
                int clinet_id_to_shutdown = stoi(client_id_to_shutdown);
                Request request{client_id, action, clinet_id_to_shutdown, -1};
                sendRequest(sockfd, request);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        //EXIT
        } else if (action == "6")
        {
            try {
                action = "EXIT";
                Request request{client_id, action, -1, -1};
                sendRequest(sockfd, request);
                exit(0);
            } catch (const invalid_argument& e) {
                cerr << "Invalid input. Please enter a valid integer." << endl;
            } catch (const out_of_range& e) {
                cerr << "Input out of range for integer." << endl;
            }
        }
        sleep(1);
    }

    close(sockfd);
    return 0;
}
