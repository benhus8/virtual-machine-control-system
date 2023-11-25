#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>  //memset

using namespace std;

struct Request {
    int client_id;
    string action;
    int action_client_id;
};

struct ThreadData {
    vector<int> admin_ids;
    int client_sock;
};



Request parseRequest(const char* buffer) {
    istringstream iss(buffer);
    Request request;
    iss >> request.client_id >> request.action >> request.action_client_id;
    return request;
}

void addAdminId(vector<int>& admin_ids, int new_id) {
    admin_ids.push_back(new_id);
    cout << "Added client with ID: " << new_id << endl;
}

bool isAdminIdInVector(const vector<int>& admin_ids, int target_id) {
    for (int id : admin_ids) {
        if (id == target_id) {
            return true; // id found
        }
    }
    return false; // id not found
}

void handleAddAdminRequest(int client_sock, const Request& request, vector<int>& admin_ids) {
    string response_message;
    cout << "Received request from client " << request.client_id
         << " to perform action: " << request.action << endl;
    if (!isAdminIdInVector(admin_ids, request.action_client_id)) {
            addAdminId(admin_ids, request.action_client_id);   
            response_message = "ADMIN WITH ID: " + to_string(request.action_client_id) + "CREATED";
    } else {
        response_message = "ADMIN WITH ID: " + to_string(request.action_client_id) + "ALREADY EXISTS";

        //we convert string to byte array and send to client
        send(client_sock, response_message.c_str(), response_message.length(), 0);
    }
}

// client loop
void* clientHandler(void* arg) {
    ThreadData* thread_data = (ThreadData*)arg;
    int client_sock = thread_data->client_sock;
    vector<int>& admin_ids = thread_data->admin_ids;

    char buffer[1024] = {0};

    while (true) {
        recv(client_sock, buffer, sizeof(buffer), 0);
        Request request = parseRequest(buffer);

        if(isAdminIdInVector(admin_ids, request.client_id)) {
            if(request.action == "ADD_ADMIN_ID") {    
                handleAddAdminRequest(client_sock, request, admin_ids); 
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }
    
    // close(client_sock);
    // pthread_exit(NULL);
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
    server_addr.sin_port = htons(8888);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_sock, 10) == -1) {
        perror("Listen failed");
        return 1;
    }

    cout << "Server listening on port 8888..." << std::endl;

    vector<pthread_t> thread_ids; // Wektor do przechowywania identyfikatorów wątków
    vector<int> admin_ids;
    admin_ids.push_back(1); // first ADMIN has id 1

    while (true) {
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
