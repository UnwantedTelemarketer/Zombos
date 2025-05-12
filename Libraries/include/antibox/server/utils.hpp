/**
 * @brief Used for utility functions and type/macro definitions for server subsystem of Antibox engine. 
 */
typedef struct server_options {
    unsigned int keepAliveTimeout;
    unsigned int numOfThreads;
    bool lanOnly;
    unsigned long sizeOfSndBuffer;
    unsigned long sizeOfRcvBuffer;
    unsigned int clientLimit;
};

typedef struct listener_arguments {
    unsigned short maximum_number_of_clients; /// < for now, acts as limit to the number of threads the server can create to handle new clients. 
    short optional_timeout; /// < allows for servers to stop listening for new connections should no new clients arrive within a certain time window after the last connections. 
};