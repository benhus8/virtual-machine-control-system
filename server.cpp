#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>  //memset
#include <mutex>

using namespace std;
namespace Colors {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m"; // something went wrong
    const std::string GREEN = "\033[32m"; //something passed
    const std::string YELLOW = "\033[33m"; // data modification request
    const std::string BLUE = "\033[34m"; //reading data request
}

void logMessage(int colorId, const string& origin, const string& message) {
    string color;
    if (colorId == 1) {
        color = Colors::RED;
    } else if (colorId == 2) {
        color = Colors::GREEN;
    } else if(colorId == 3){
        color = Colors::YELLOW;
    } else if(colorId == 4){
        color = Colors::BLUE;
    }
    std::cout << "[" << color << origin << Colors::RESET << "] " << message << std::endl;
}

struct Request {
    int client_id;
    string action;
    int action_client_id;
};

struct ThreadData {
    mutex data_mutex;
    vector<int>& admin_ids;
    int client_sock;

    ThreadData(vector<int>& admins, int sock) : admin_ids(admins), client_sock(sock) {}
};



Request parseRequest(const char* buffer) {
    istringstream iss(buffer);
    Request request;
    iss >> request.client_id >> request.action >> request.action_client_id;
    return request;
}

void addAdminId(vector<int>& admin_ids, int new_id) {
    admin_ids.push_back(new_id);
    logMessage(2, "addAdminId", " Added admin with Id: " + to_string(new_id));
}

bool isAdminIdInVector(const vector<int>& admin_ids, int target_id) {
    for (int id : admin_ids) {
        if (id == target_id) {
            return true; // id found
        }
    }
    return false; // id not found
}

void handleAddAdminRequest(int client_sock, const Request& request, ThreadData& thread_data) {
    string response_message;
        logMessage(3, "handleAddAdminRequest", " Add new admin with id: " + to_string(request.client_id) + " by admin with id: " + to_string(request.action_client_id));
    lock_guard<mutex> lock(thread_data.data_mutex);
    if (!isAdminIdInVector(thread_data.admin_ids, request.action_client_id)) {
            addAdminId(thread_data.admin_ids, request.action_client_id);   
            response_message = "ADMIN WITH ID: " + to_string(request.action_client_id) + " CREATED";
            send(client_sock, response_message.c_str(), response_message.length(), 0);
    } else {
        response_message = "ADMIN WITH ID: " + to_string(request.action_client_id) + " ALREADY EXISTS";

        //we convert string to byte array and send to client
        send(client_sock, response_message.c_str(), response_message.length(), 0);
    }
}
void handleShowAllAdminsRequest(int client_sock, const Request& request, ThreadData& thread_data) {
    string response_message = "Existing admin ids: [";
    logMessage(4, "handleShowAllAdminsRequest", "Show all admins for client with id: " + to_string(request.client_id));

    lock_guard<mutex> lock(thread_data.data_mutex);
    for(int id : thread_data.admin_ids) {
        response_message = response_message + to_string(id) + ", ";
    }
    response_message = response_message + " ]";

        //we convert string to byte array and send to client
        send(client_sock, response_message.c_str(), response_message.length(), 0);
    }

bool authenticateAdmin(int client_sock, const Request& request, ThreadData& thread_data) {
    if(isAdminIdInVector(thread_data.admin_ids, request.client_id)) { 
        logMessage(2, "authenticateAdmin", "User with Id: " + to_string(request.client_id) + " authenticated");
        return true;
        } else {
            logMessage(1, "authenticateAdmin", " User with Id: " + to_string(request.client_id) + " not authenticated");
            return false;
        }
}

// client loop
void* clientHandler(void* arg) {
    ThreadData* thread_data = (ThreadData*)arg;
    int client_sock = thread_data->client_sock;
    vector<int>& admin_ids = thread_data->admin_ids;
    string response_message;
    char buffer[1024] = {0};

    while (true) {
        recv(client_sock, buffer, sizeof(buffer), 0);
        Request request = parseRequest(buffer);
        if(request.action == "ADD_ADMIN_ID") {    
                if(authenticateAdmin(client_sock, request, *thread_data)) {
                    handleAddAdminRequest(client_sock, request, *thread_data); 
                } else {
                     response_message = "You have not permission to add new admin id";
                    send(client_sock, response_message.c_str(), response_message.length(), 0);
                }
            } else if (request.action == "SHOW_ALL_ADMINS")
            {
                handleShowAllAdminsRequest(client_sock, request, *thread_data);
            }
            
        
        
        memset(buffer, 0, sizeof(buffer));
    }
    

}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Error creating socket");
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8881);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_sock, 10) == -1) {
        perror("Listen failed");
        return 1;
    }

    cout << "[MAIN-LOG] Server listening on port 8888..." << std::endl;

    vector<pthread_t> thread_ids; // Wektor do przechowywania identyfikatorów wątków
    vector<int> admin_ids;
    admin_ids.push_back(1); // first ADMIN has id 1

    while (true) {
        cout << "[MAIN-LOG] Wait for new connection...\n";
        int client_sock = accept(server_sock, nullptr, nullptr);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }
        ThreadData* thread_data = new ThreadData{admin_ids, client_sock};

        // Tworzymy nowy wątek dla obsługi klienta
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, clientHandler, (void*)thread_data) != 0) {
            perror("Failed to create thread");
            close(client_sock);
            delete thread_data;
            continue;
        }

        // Dodajemy identyfikator wątku do wektora
        thread_ids.push_back(thread_id);
        pthread_detach(thread_id);
    }

    // Oczekiwanie na zakończenie wszystkich wątków
    for (pthread_t thread_id : thread_ids) {
        pthread_join(thread_id, NULL);
    }

    close(server_sock);
    return 0;
}
