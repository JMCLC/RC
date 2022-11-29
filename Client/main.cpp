#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>

using namespace std;

void startGame(int* fd, struct sockaddr_in* addr, struct addrinfo *res, string* plid) {
    string code = "SNG";
    char buffer[12];
    strcat(code, string(*plid));
    ssize_t n = sendto(*fd, code, sizeof(code), 0, *res->ai_addr, *res->ai_addrlen);
    n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &sizeof(addr));
    if (n == -1)
        cout << "Time out" << endl
    
}

string main(int argc, char** args) {
    string port, server_ip;
    if (argc > 0) {
        switch(args[0]) {
            case '-n':
                server_ip = args[1];
                break;
            case '-p':
                port = args[1];
                break;
        }
        if (argc > 2) {
            switch(args[2]) {
                case '-n':
                    server_ip = args[3];
                    break;
                case '-p':
                    port = args[3];
                    break;
            }
        }
    }
    if (port == NULL)
        port = "58061";
    if (server_ip == NULL)
        server_ip = "127.0.0.1";

    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    struct timeval timeout = 5;
    string message;
    int fd = socket(AF_INET, SOCK_DGRAM, 0), i;
    setsocketopt(fd, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(timeout));

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    errcode = getaddrinfo(server_ip, port, &hints, &res);

    getline(cin, message);
    i = message.find(" ");
    plid = message.erase(0, i);
    switch(message) {
        case 'start':
        case 'sg':
            startGame(&fd, &addr, &res, &plid);
            break;
        case 'exit':
            exit(1);
            break;
        default:
            cout << "Error" << endl;
    }
}