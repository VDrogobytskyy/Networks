#include <iostream>
#include <string>       
#include <cstring>
#include <chrono>     
#include <sys/socket.h> // Головний заголовок: функції socket(), sendto(), recvfrom(), setsockopt()
#include <netinet/in.h> // Структура sockaddr_in, константи AF_INET, IPPROTO_UDP
#include <arpa/inet.h>  // Перетворення адрес: htons(), inet_pton()
#include <unistd.h>     
#include <cerrno>


class client{
    private:
        int sockfd;
        struct sockaddr_in serveradr;
    public:
        void create_sockfd();
        void close_sockfd();

        void setup_server(const std::string &ip, const int port);
        void set_timeout(const int seconds);

        void send_message(const std::string &message);
        void recive_message();
};


void client::create_sockfd(){

    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(socketFD < 0){
        std::cerr << "Fail to create UDP socket!" << std::endl;
        
        exit(1);
    }else{
        std::cout << "Socket was succesfully created! Descriptor: " << socketFD << std::endl;
        sockfd = socketFD;
    }
}

void client::close_sockfd(){

    if(close(sockfd) < 0){
        std::cerr << "Fail to close socket!" << std::endl;
        
        exit(1);
    }else{
        std::cout << "Socket was closed!" << std::endl;
    }
}

void client::setup_server(const std::string &ip, const int port){
    memset(&serveradr, 0, sizeof(serveradr));

    serveradr.sin_family = AF_INET; // ipv4 (127.0.0.1)
    serveradr.sin_port = htons(port);
    int res = inet_pton(AF_INET, ip.c_str(), &serveradr.sin_addr); // ip in binary

    if(res <= 0){
        std::cerr << "Invalid adress" << std::endl;

        exit(1);
    }else{
        std::cout << "Server set to ip: " << ip << ", port: " << port << std::endl;
    }
}

void client::set_timeout(const int seconds){

    struct timeval t_out;
    t_out.tv_sec = seconds;
    t_out.tv_usec = 0;

    int res = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t_out, sizeof(t_out));
    if(res < 0){
        std::cerr << "Fail to set timeout!" << std::endl;

        exit(1);
    }else{
        std::cout << "Timeout set to: " << seconds << " seconds!" << std::endl;
    }
}

void client::send_message(const std::string &message){

    ssize_t sent_bytes = sendto(sockfd, message.c_str(), message.length(), 0, 
                            (struct sockaddr*)(&serveradr), sizeof(serveradr));

    if(sent_bytes < 0){
        std::cerr << "Fail to send message" << std::endl;
    }else{
        std::cout << "Message succesfully sent, size in bytes: " << sent_bytes << std::endl;
    }
}

void client::recive_message(){
    char buffer[1024];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_recv = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                  (struct sockaddr*)&from_addr, &from_len);
    if (bytes_recv < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "[TIMEOUT ERROR] Server didn`t respond on time!" << std::endl;
        } else {
            std::cerr << "[ERROR] Error receiving data from server!" << std::endl;
        }
        return;
    }

    buffer[bytes_recv] = '\0';
    
    std::cout << "[SERVER RESPONSE] Recieved: \"" << buffer << "\" (" << bytes_recv << " bytes)" << std::endl;
}

int main(void){
    std::cout << std::endl;

    std::cout << "Program started!" << std::endl;

    std::cout << std::endl;

    client client_socket;

    client_socket.create_sockfd();
    client_socket.set_timeout(5);

    client_socket.setup_server("127.0.0.1", 9999);

    client_socket.send_message("Hello world!");
    client_socket.recive_message();
    
    client_socket.close_sockfd();

    std::cout << std::endl;
    return 0;
}