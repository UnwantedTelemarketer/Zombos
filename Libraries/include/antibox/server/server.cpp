#include "server.hpp"

Server::Server() {
    this -> socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cout << "Failure to open socket. Please verify permissions are correct to open sockets. Exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;

    this -> address = inet_addr(DEFAULT_ADDRESS);
    this -> protocol = DEFAULT_PROTOCOL;
    this -> timeout = DEFAULT_TIMEOUT;
    this -> port = DEFAULT_PORT;

    int option = 1;
    int keepIdle = (this -> timeout) / 2;
    int numOfKeepAliveProbes = (this -> timeout) / 10;
    int keepAliveInterval = (this -> timeout) / 6;

    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option));
    setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPALIVE, &keepIdle, sizeof(keepIdle));
    setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepAliveInterval, sizeof(keepAliveInterval));
    setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &numOfKeepAliveProbes, sizeof(numOfKeepAliveProbes));
}