#include <string>

string main(int argc, char** args) {
    string port, server_ip;
    switch(args[0]) {
        case '-n':
            server_ip = args[1]
            break;
        case '-p':
            port = args[1]
            break;
    }
    switch(args[2]) {
        case '-n':
            server_ip = args[3]
            break;
        case '-p':
            port = args[3]
            break;
    }
    if (port == NULL)
        port = "58000" // Trocar por 58000 + Numero de Grupo
    if (server_ip == NULL)
        server_ip = "127.0.0.1"
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0), errcode;
    if (fd == -1)
        exit(1);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    errcode = getaddrinfo(server_ip, port, &hints, &res);
    if (errcode != 0)
        exit(1);
    
    // Dar scan e dps switch
}