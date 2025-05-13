#ifdef __APPLE__ 
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
    #include <pthread.h>
#endif
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")

    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
#endif

#include <vector>
#include <string>
#include <iostream>

#include "../core/log.h"
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
    void * server_listener(void * args);

    /// -------------------------------- PRIVATE MEMBER VARIABLES --------------------------------
    #ifndef _WIN32
        unsigned int socket_fd;
    #else
        SOCKET socket_fd;
    #endif
    std::vector<int> client_sockets;
    std::string configPath;
    unsigned int clientLimit;
    unsigned int address;
    unsigned short port;
    unsigned char protocol;
    unsigned int timeout;
}; 