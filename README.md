# Simple Client and Administrator Management System

## Description

The program consists of two parts: the client and the server. It provides a simple system for managing clients and administrators, allowing communication between them.

## Client

### Request Structure:

- `client_id`: Unique client identifier.
- `action`: Action that the client wants to perform.
- `action_1_client_id` and `action_2_client_id`: Additional identifiers that may be required depending on the action.

### Client Functions:

- `sendRequest(int sockfd, const Request& request)`: Creates a string with request data and sends it through the socket.
- `receiveThread(void* arg)`: Receives messages from the server in a separate thread and reacts to them.

### User Roles:

- **Client**: Default role for clients.
    - Can shut down other clients (when client has permission).
  
- **Administrator**: Assigned to clients with `client_id=1` by default.
    - Can add a new administrator.
    - Can add permissions for clients to shutdown other clients.
    - Can view all administrators.
    - Can shut down other clients (when administrtor has permission).

### Menu and Actions:

- Adding a new administrator.
- Adding permissions for a client.
- Displaying active clients.
- Displaying all administrators.
- Exiting the application.

### Communication:

- The client connects to the server through a socket.
- Sends requests to the server using the Request structure.

## Server

### Request Structure:

- Similar to the client, the Request structure is used to pass information between the client and the server.

### Server Functions:

- `parseRequest(const char* buffer)`: Parses the buffer received from the client and converts it into a Request structure.
- `deleteClientFromActiveClientsRegistry`: Removes a client from the list of active clients.
- `addAdminId`: Adds a new administrator to the list of administrators.
- `addPermissionForUser`: Adds permissions for a client to the permissions list.
- `shutdownClient`: Sends the command to shut down a client and handles responses.

### Authentication:

- The server checks the client's permissions before performing certain actions (e.g., adding a new administrator, adding permissions).

### Messages and Logs:

- The server generates colored log messages that help track and understand the flow of actions.
- In case of failure or errors, the server informs the client about the problem.

### Multithreading:

- The server handles multiple clients simultaneously, each in a separate thread.
- Threads are created and managed dynamically.

## Summary
The program provides a simple system for managing clients and administrators, enabling communication between multiple clients and the server simultaneously.

## How to Run the Project:

To compile the code, use the following commands:

```bash
g++ server.cpp -o run-server
g++ client.cpp -o run-client
```
To run the server:
```bash
./run-server
```
To run a client with a specific client_id:
```bash
./run-client <client_id>
```
By default, the client with id=1 has administrator permissions.

