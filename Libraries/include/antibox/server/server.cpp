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
    this -> port = htons(DEFAULT_PORT);

    int option = 1;
    int keepIdle = (this -> timeout) / 2;
    int numOfKeepAliveProbes = (this -> timeout) / 10;
    int keepAliveInterval = (this -> timeout) / 6;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option)) < 0) {
        std::cout << "WARNING: failed to set keep alive option at socket level. Connections may be prematurely cut." << std::endl;
        goto optionsset;
    } else {
        if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPALIVE, &keepIdle, sizeof(keepIdle)) < 0) {
            std::cout << "WARNING: failed to change amount of seconds before keep alive probes are sent." << std::endl;
        }

        if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepAliveInterval, sizeof(keepAliveInterval)) < 0) {
            std::cout << "WARNING: failed to change amount of seconds between keep alive probes." << std::endl;
        }
        
        if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &numOfKeepAliveProbes, sizeof(numOfKeepAliveProbes)) < 0) {
            std::cout << "WARNING: failed to change amount of keepalive probes sent." << std::endl;
        }
    }

optionsset:
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_ENABLE_ECN, &option, sizeof(option)) < 0) {
        std::cout << "WARNING: failed to set ECN capabilities. May increase time to recover lost packets." << std::endl;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = this -> address;
    address.sin_port = this -> port;

    if (bind(this -> socket_fd, (struct sockaddr *)&(address), sizeof(address) < 0)) {
        std::cout << "ERROR: Failure to bind port. Exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    // we now have a live socket, but it's not listening for any connections yet. We do that in start!
}

int Server::start() {
    // will implement multithreading to create asynchronous behavior and not hang game loop. 
    // HOWEVER: important to note. By DEFAULT, multithreaded behavior is enabled, which means that every client 
    // will be handled by a separate thread. This may (probably will) create a lot of contention between threads 
    // when a significant number of clients join, so the current plan is to implement just multithreading, 
    // THEN basically detect the number of CPU cores on the system and distribute the threads accordingly
    // by changing their CPU affinities. THEN, to impose a safeguard, I'll see if I can delegate new clients to already existing
    // threads. 
}