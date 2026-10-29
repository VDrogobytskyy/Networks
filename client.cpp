#include <iostream>
#include <string>       
#include <cstring>
#include <chrono>     
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <cerrno>


class client{
    private:
        int sockfd;
        struct sockaddr_in serveradr;
        std::chrono::high_resolution_clock::time_point start_time;
    public:
        void create_sockfd();
        void close_sockfd();

        void setup_server(const std::string &ip, const int port);
        void set_timeout(const int seconds);

        void send_message(const std::string &message);
        void recieve_message();
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

    start_time = std::chrono::high_resolution_clock::now();
    ssize_t sent_bytes = sendto(sockfd, message.c_str(), message.length(), 0, 
                            (struct sockaddr*)(&serveradr), sizeof(serveradr));

    if(sent_bytes < 0){
        std::cerr << "Fail to send message" << std::endl;
    }else{
        std::cout << "Message succesfully sent, size in bytes: " << sent_bytes << std::endl;
    }
}

void client::recieve_message(){
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
    
    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    double rtt_ms = duration_us / 1000.0;
    std::cout << "[METRICS] RTT: " << rtt_ms << " ms (" << duration_us << " us)" << std::endl;

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

    int choice = -1;

    while (choice != 0) {

        std::cout << "\n--- Select Action ---" << std::endl;
        std::cout << "1. Check seats for Train #705 (CHECK_SEATS)" << std::endl;
        std::cout << "2. Get station departure board (GET_BOARD)" << std::endl;
        std::cout << "3. Diagnostics & latency check (PING)" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
            std::cout << "Invalid input!" << std::endl;
            
            continue;
        }

        switch (choice) {
            case 1:
                client_socket.send_message("{\"action\":\"CHECK_SEATS\",\"train_id\":\"705\"}");
                client_socket.recieve_message();
                break;
            case 2:
                client_socket.send_message("{\"action\":\"GET_BOARD\",\"station\":\"Kyiv-Pas\"}");
                client_socket.recieve_message();
                break;
            case 3:
                client_socket.send_message("{\"action\":\"PING\",\"client_id\":\"TERMINAL-01\"}");
                client_socket.recieve_message();
                break;
            case 0:
                std::cout << "Exiting client..." << std::endl;
                break;
            default:
                std::cout << "Invalid choice! Please select 0, 1, 2, or 3." << std::endl;
                break;
        }
    }
    
    client_socket.close_sockfd();

    std::cout << std::endl;
    return 0;
}