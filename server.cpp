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

int main(void){
    std::cout << std::endl;

    std::cout << "Server" << std::endl;

    std::cout << std::endl;

    server serv;

    serv.create_sockfd();

    serv.bind_socket(9999);

    serv.close_sockfd();

    std::cout << std::endl;
    return 0;
}