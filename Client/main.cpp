#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cctype>

using namespace std;
int wordSize, errors, currentMove, playerId, fd;
struct addrinfo hints, *res;
struct sockaddr_in addr;
string port, server_ip;
socklen_t addrlen;
ssize_t n;

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
    n = sendto(fd, message, sizeof(message), 0, res->ai_addr, res->ai_addrlen);
    n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &sizeof(addr));
    if (n == -1)
        cout << "Time out" << endl
    return stringSplitter(string(buffer))
}

void start(string plId) {
    vector<string> response = sendMessageToServer("SNG " + plId + "\n")
    playerId = atoi(plId);
    if (response[1] == "OK") {
        string wordSizeString;
        wordSize = atoi(command[2]);
        errors = atoi(command[3]);
        for (int i = 0; i < wordSize; i++)
            wordSizeString.append("_");
            cout << "New game started (max " << errors << " errors): " << wordSizeString << endl
    } else
        cout << "There is already an ongoing game for this player id" << endl;
}

void play(string letter) {
    vector<string> response = sendMessageToServer("PLG " + string(playerId) + " " + toLower(letter) + " " + string(currentMove) + "\n")
    if (response[0] == "RLG") {
        switch(response[1]) {
            case "OK":
                cout << 'Yes, "' + letter + '" is part of the word: ' << currentState() << endl;
                break;
            case "WIN":
                cout << "WELL DONE! You guessed: " << currentState() << endl;
                break;
            case "DUP":
                cout << "This letter has already been sent" << endl;
                break;
            case "NOK":
                cout << 'No, "' + letter + '" is not part of the word: ' << currentState() << endl;
                errors -= 1;
                break;
            case "OVR":
                cout << "Game over!" << endl;
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
            case "start": case "sg":
                start(command[1]);
                break;
            case "play": case "pl":
                play(command[1]);
                break;
            case "guess": case "gw":
                break;
            case "scoreboard": case "sb":
                break;
            case "hint": case "h":
                break;
            case "state": case "st":
                break;
            case "quit": case "exit":
                break;
        }
    }
}



void main(int argc, char** args) {
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
    struct timeval timeout = 5;
    string message;
    fd = socket(AF_INET, SOCK_DGRAM, 0), i;
    setsocketopt(fd, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(timeout));
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    getaddrinfo(server_ip, port, &hints, &res);
    adddrlen = sizeof(addr);
    handleGame();
    freeaddrinfo(res);
    close(fd);
}