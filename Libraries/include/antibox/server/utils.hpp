
typedef struct server_options {
    unsigned int keepAliveTimeout;
    unsigned int numOfThreads;
    bool lanOnly;
    unsigned long sizeOfSndBuffer;
    unsigned long sizeOfRcvBuffer;
    unsigned int clientLimit;
};