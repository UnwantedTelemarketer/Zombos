#include "server.hpp"

Server::Server() {
    this -> socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cout << "Failure to open socket. Please verify permissions are correct to open sockets. Exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    this -> protocol = DEFAULT_PROTOCOL;
    this -> timeout = DEFAULT_TIMEOUT;
    this -> port = DEFAULT_PORT;

}