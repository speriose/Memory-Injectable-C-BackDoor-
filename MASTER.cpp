#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>

using namespace std;

string download(int clientSocket){
    
    const char* ACK = "ACK";
    char size_buffer[1024] = { 0 };
    
    int fileSize = recv(clientSocket, size_buffer, sizeof(size_buffer), 0);
            
            if (fileSize > 0) {
                cout << "[+] File size is: " << size_buffer << endl;
                send(clientSocket, ACK, strlen(ACK), 0);
                cout << "ACK sent!" << endl;
            } 
            
            int recv_size = 0;

            std::ofstream outputFile("image.png", std::ios::binary);
            
            char file_buffer[4096];
            ssize_t bytesReceived;

            while ((bytesReceived = recv(clientSocket, file_buffer, sizeof(size_buffer), 0)) > 0) {
                outputFile.write(file_buffer, bytesReceived);
                recv_size += bytesReceived;
                cout << recv_size << endl;

                if (recv_size == atoi(size_buffer)) {
                    cout << "[+] All data recved, breaking!" << endl;
                    break;
                }
            
            }
            outputFile.close();
            return size_buffer;
}

int main() {
    // Creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding socket
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Listening
    listen(serverSocket, 5);
    cout << "[*] Listening on port 8080...\n";

    // Accepting connection
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    cout << "[+] Client connected.\n";

    char buffer[1024] = { 0 };
    

    while (true) {
        cout << "[>] Enter command: ";
        cin >> buffer;

        send(clientSocket, buffer, strlen(buffer), 0);
        cout << "[*] Command sent: " << buffer << endl;
        

        if (strcmp(buffer, "down") == 0) {
            download(clientSocket);
            
            
        } else {
            memset(buffer, 0, sizeof(buffer));
            recv(clientSocket, buffer, sizeof(buffer), 0);
            cout << "[<] Message from client: " << buffer << endl;
        }
    }

    // Closing the socket
    close(clientSocket);
    close(serverSocket);

    return 0;
}
