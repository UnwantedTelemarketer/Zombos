/*#ifdef __APPLE__ 
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
#endif

#include <vector>
#include <string>
#include <iostream>
#include <pthread.h>

#include "utils.hpp"

#define DEFAULT_PORT 5023
#define DEFAULT_ADDRESS "0.0.0.0"
#define DEFAULT_TIMEOUT 60
#define DEFAULT_PROTOCOL IPPROTO_TCP

class Server {
public:
    Server();
    Server(std::string configPath);
    Server(std::string protocol, unsigned short portNumber);
    Server(std::string protocol, unsigned short portNumber, server_options * options);

    int start();
    int start(int newPort);
    int stop();
private:
    unsigned int socket_fd;
    std::vector<int> client_sockets;
    std::string configPath;
    unsigned int clientLimit;
    unsigned int address;
    unsigned short port;
    unsigned char protocol;
    unsigned int timeout;
};  */