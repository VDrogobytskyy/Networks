#include <iostream>
#include <string>       
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>  

class server{
    private:
        int sockfd;
        struct sockaddr_in serveradr;

    public:
        void create_sockfd();
        void close_sockfd();

        void bind_socket(const int port);
        void listen_and_respond();

};

void server::close_sockfd(){

    if(close(sockfd) < 0){
        std::cerr << "Fail to close socket!" << std::endl;
        
        exit(1);
    }else{
        std::cout << "Socket was closed!" << std::endl;
    }
}

void server::create_sockfd(){

    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(socketFD < 0){
        std::cerr << "Fail to create UDP socket!" << std::endl;
        
        exit(1);
    }else{
        std::cout << "Socket was succesfully created! Descriptor: " << socketFD << std::endl;
        sockfd = socketFD;
    }
}

void server::bind_socket(const int port){
    
    memset(&serveradr, 0, sizeof(serveradr));

    serveradr.sin_family = AF_INET;
    serveradr.sin_port = htons(port);

    serveradr.sin_addr.s_addr = INADDR_ANY;

    int result = bind(sockfd, (struct sockaddr*)&serveradr, sizeof(serveradr));

    if(result < 0){
        std::cerr << "Failed to bind port to server!" << std::endl;

        exit(1);
    }else{
        std::cout << "Server binded to port: " << port << "." << std::endl;
    }
}

void server::listen_and_respond(){

    char buffer[1024];              
    struct sockaddr_in client_addr; // Сюди ОС запише адресу клієнта, який надіслав пакет

    socklen_t client_len = sizeof(client_addr); // Розмір структури адреси клієнта

    std::cout << "Server is listening on port 9999..." << std::endl;
    std::cout << "Waiting for UDP packets (Press Ctrl+C to stop)...\n" << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                          (struct sockaddr*)&client_addr, &client_len);

        if (bytes_received < 0){
            std::cerr << "Error receiving data!" << std::endl;
            continue;
        }

        buffer[bytes_received] = '\0';

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        std::cout << "[PACKET RECEIVED] From " << client_ip << ":" << client_port << std::endl;
        std::cout << "Data: \"" << buffer << "\" (" << bytes_received << " bytes)" << std::endl;
                
        std::string req(buffer);
        std::string response;

        if(req.find("CHECK_SEATS") != std::string::npos){

            response = "{\"status\":200,\"action\":\"CHECK_SEATS\",\"train_id\":\"705\",\"route\":\"Kyiv-Lviv\",\"available_seats\":42,\"price\":520}";
        } else if(req.find("GET_BOARD") != std::string::npos){

            response = "{\"status\":200,\"action\":\"GET_BOARD\",\"station\":\"Kyiv-Pas\",\"trains\":\"[705:Lviv:06:15|712:Kramatorsk:06:45]\"}";
        } else if(req.find("PING") != std::string::npos){

            response = "{\"status\":200,\"action\":\"PONG\",\"server\":\"Kyiv-Central\",\"status\":\"OK\"}";
        } else{
            
            response = "{\"status\":400,\"error\":\"Unknown action\"}";
        }

        ssize_t bytes_sent = sendto(sockfd, response.c_str(), response.length(), 0,
                                    (struct sockaddr*)&client_addr, client_len);

        if (bytes_sent < 0) {
            std::cerr << "Fail to send response to client!" << std::endl;
        } else {
            std::cout << "Response sent back to client (" << bytes_sent << " bytes).\n" << std::endl;
        }
    }
}

int main(void){
    std::cout << std::endl;

    std::cout << "Server" << std::endl;

    std::cout << std::endl;

    server serv;

    serv.create_sockfd();

    serv.bind_socket(9999);
    serv.listen_and_respond();

    serv.close_sockfd();

    std::cout << std::endl;
    return 0;
}