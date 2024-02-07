// C++ Standard Library headers
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// C Standard Library headers
#include <cstring>
#include <unistd.h>

// System headers
#include <netinet/in.h>
#include <sys/socket.h>


std::vector<int> client_sockets;
std::mutex mtx;
std::map<int, std::string> client_usernames;
std::set<std::string> usernames;

// A function to get the current time
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time);
    std::stringstream ss;
    ss << std::put_time(now_tm, "%H:%M");
    return ss.str();
}

void handle_client(int client_socket) {
    char buffer[256];
    const int USERNAME_MAX_LENGTH = 20;

    std::cout << getCurrentTime() << " Client " << client_socket << " connected\n" << std::endl;

    // Ask for the name of the user
    const char* response = "Insert your name: ";
    write(client_socket, response, strlen(response));
    
    // Read the username
    std::memset(buffer, 0, 256);
    int n = read(client_socket, buffer, 255);
    if (n < 0) {
        std::cerr << "Error reading username from socket" << std::endl;
        return;
    }

    std::string username = buffer;

    // Remove the newline character
    if (!username.empty() && username[username.length()-1] == '\n') {
        username.erase(username.length()-2);
    }

    // Check if the username is too long
    if (username.length() > USERNAME_MAX_LENGTH) {
        const char* response = "Maximum username length is 20 characters\n";
        n = write(client_socket, response, strlen(response));
        close(client_socket);
        mtx.lock();
        client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
        mtx.unlock();
        std::cout << getCurrentTime() << " Client " << client_socket << " disconnected\n" << std::endl;
        return;
    }

    // Check if the username is already in use
    if (usernames.count(username) > 0) {
        const char* response = "This username is already in use\n";
        n = write(client_socket, response, strlen(response));
        close(client_socket);
        mtx.lock();
        client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
        mtx.unlock();
        std::cout << getCurrentTime() << " Client " << client_socket << " disconnected\n" << std::endl;
        return;
    }


    // Add the username to the list of usernames
    client_usernames[client_socket] = username;
    usernames.insert(username);

    while (true) {
        std::memset(buffer, 0, 256);
        n = read(client_socket, buffer, 255);
        if (n <= 0) {
            if (n < 0) {
                std::cerr << "Error reading from socket" << std::endl;
            } else {
                std::cout << getCurrentTime() << " Client " << client_usernames[client_socket] << " disconnected\n" << std::endl;
                std::string username = client_usernames[client_socket];
                usernames.erase(username);
                mtx.lock();
                client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
                mtx.unlock();
            }
            break;
        }

        std::cout << getCurrentTime() << " Message received from " << client_usernames[client_socket] << ": " << buffer << std::endl;
        
        mtx.lock();

        std::string message = getCurrentTime() + " " + client_usernames[client_socket] + ": " + buffer;

        for (int other_socket : client_sockets) {
            if (other_socket != client_socket) {
                n = write(other_socket, message.c_str(), message.length());
                if (n < 0) {
                    std::cerr << "Error writing to socket" << std::endl;
                    break;
                }
            }
        }
        mtx.unlock();
    }
    close(client_socket);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return -1;
    }
    sockaddr_in serv_addr;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(12345); // Port 12345

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error on binding" << std::endl;
        return -1;
    }
    listen(sockfd, 5);
    std::cout << "Server is online and listening for connections..." << std::endl;
    while (true) {
        sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            std::cerr << "Error on accept" << std::endl;
            continue;
        }
        mtx.lock();
        client_sockets.push_back(newsockfd);
        mtx.unlock();
        std::thread client_thread(handle_client, newsockfd);
        client_thread.detach();
    }
    close(sockfd);
    return 0;
}