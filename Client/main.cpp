#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cctype>

using namespace std;
int wordSize, errors, currentMove, playerId;

vector<string> stringSplitter(string text) {
    vector<string> res;
    int i = text.find(" ");
    while (i != -1) {
        res.push_back(text.erase(0, i));
        i = text.find(" ");
    }
    return res;
}

vector<string> sendMessageToServer(string message) {
    char buffer[128];
    ssize_t n = sendto(*fd, code, sizeof(code), 0, *res->ai_addr, *res->ai_addrlen);
    n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &sizeof(addr));
    if (n == -1)
        cout << "Time out" << endl
    return stringSplitter(string(buffer))
}

void play(string letter) {
    vector<string> response = sendMessageToServer("PLG " + string(playerId) + " " + toLower(letter) + " " + string(currentMove))
    if (response[0] == "RLG") {
        switch(response[1]) {
            case "OK":
                break;
            case "WIN":
                break;
            case "DUP":
                break;
            case "NOK":
                break;
            case "OVR":
                break;
            case "INV":
                break;
            case "ERR":
                break;
        }
    } else
        cout << "Error" << endl
}

void handleGame() {
    vector<string> command;
    string line;
    while (command[0] != "exit") {
        getline(cin, line);
        command = stringSplitter(line);
        switch(command[0]) {
            case "play":
            case "pl":
                play(command[1]);
                break;
            case "guess":
            case "gw":
                break;
            case "scoreboard":
            case "sb":
                break;
            case "hint":
            case "h":
                break;
            case "state":
            case "st":
                break;
            case "quit":
            case "exit":
                break;
        }
    }
}

void startGame(int* fd, struct sockaddr_in* addr, struct addrinfo *res, string* plid) {
    string code = "SNG ";
    char buffer[12];
    code.append(string(*plid));
    ssize_t n = sendto(*fd, code, sizeof(code), 0, *res->ai_addr, *res->ai_addrlen);
    n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &sizeof(addr));
    if (n == -1)
        cout << "Time out" << endl
    vector<string> sv_response = stringSplitter(string(buffer))
    if (sv_response[0] == "RSG" && sv_response[1] == "OK") {
        string wordSizeString;
        wordSize = stoi(sv_response[2]);
        errors = stoi(sv_response[3]);
        for (int i = 0; i < wordSize; i++)
            wordSizeString.append("_");
        cout << "New game started (max " << errors << " errors): " << wordSizeString << endl
        handleGame();
    } else {
        cout << "Error" << endl
    }
}

void main(int argc, char** args) {
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
    int i = message.find(" ");
    plid = message.erase(0, i);
    playerId = atoi(plid);
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