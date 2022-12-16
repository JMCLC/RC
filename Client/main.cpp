#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cctype>

using namespace std;
int wordSize, errors, currentMove, playerId, fd;
struct addrinfo hints, *res;
struct sockaddr_in addr;
string port, server_ip;
string currentWord;
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

void sendMessageToServerTCP(string message) {
    struct addrinfo tcp_hints;
    vector<string> response;
    string currentString;
    char buffer[128];
    ssize_t tcp_n;
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0), currentSize = 0, count = 0;
    tcp_hints.ai_family = AF_INET;
    tcp_hints.ai_socktype = SOCK_STREAM;
    tcp_n = connect(tcp_fd, res->ai_addr, res->ai_addrlen);
    tcp_n = write(tcp_fd, message, sizeof(message));
    tcp_n = read(tcp_fd, buffer, 128);
    response = stringSplitter(string(buffer));
    if (response[1] == "OK") {
        ofstream File(response[2]);
        for (int i = 0; i < sizeof(buffer); i++) {
            if (buffer[i] == " ") {
                count++;
            }
            if (count == 4) {
                currentString = string(buffer).substr(i + 1, sizeof(i + 2));
                break;
            }
        }
        for (currentSize = sizeof(currentString); currentSize < atoi(response[3]); currentSize += sizeof(currentString)) {
            File << currentString;
            if (atoi(response[3]) - currentSize < 128)
                tcp_n = read(tcp_fd, buffer, atoi(response[3]) - currentSize);
            else
                tcp_n = read(tcp_fd, buffer, 128);
            currentString = string(buffer); 
        }
        File.close();
    }
    if (response[0] == "RSB")
        printScoreboard(response[2]);
    else (response[0] == "RHL")
        cout << "received hint file: " << response[2] << " (" << response[3] << " bytes)" << endl;
    else (response[0] == "RST")
        printGameState(response[2]);
    close(tcp_n);
}

void start(string plId) {
    vector<string> response = sendMessageToServer("SNG " + plId + "\n");
    playerId = atoi(plId);
    if (response[1] == "OK") {
        wordSize = atoi(command[2]);
        errors = atoi(command[3]);
        for (int i = 0; i < wordSize; i++)
            currentWord.append("_");
            cout << "New game started (max " << errors << " errors): " << currentWord << endl
    } else
        cout << "There is already an ongoing game for this player id" << endl;
}

void play(string letter) {
    vector<string> response = sendMessageToServer("PLG " + string(playerId) + " " + toUpper(letter) + " " + string(currentMove) + "\n");
    if (response[0] == "RLG") {
        switch(response[1]) {
            case "OK":
                int numberOfHits = atoi(response[3]);
                for (int i = 0; i < numberOfHits; i++) {
                    currentWord[atoi(response[i + 4]) - 1] = toUpper(letter[0]);
                }
                cout << 'Yes, "' + letter + '" is part of the word: ' << currentWord << endl;
                currentMove++:
                break;
            case "WIN":
                for (int i = 0; i < sizeof(currentWord); i++)
                    if (currentWord[i] == "_")
                        currentWord[i] = toUpper(letter[0])
                cout << "WELL DONE! You guessed: " << currentWord << endl;
                currentMove++;
                break;
            case "DUP":
                cout << "This letter has already been sent" << endl;
                break;
            case "NOK":
                cout << 'No, "' + letter + '" is not part of the word' << endl;
                errors--;
                currentMove++;
                break;
            case "OVR":
                cout << "Game over!" << endl;
                currentMove;
                break;
            case "INV":

                break;
            case "ERR":

                break;
        }
    } else
        cout << "Error" << endl
}

void guess(string word) {
    vector<string> response = sendMessageToServer("PWG " + string(playerId) + " " + toUpper(word) + " " + string(currentMove) + "\n");
    if (response[0] == "RWG") {
        switch(response[1]) {
            case "WIN":
                currentWord = word;
                cout << "WELL DONE! You guessed: " << currentWord << endl;
                currentMove++;
                break;
            case "NOK":
                cout << 'No, "' + word + '" is not the word';
                errors--;
                currentMove++;
                break;
            case "OVR":
                cout << "Game over!" << endl;
                currentMove;
                break;
            case "INV":
                break;
            case "ERR":
                break;
        }
    }
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
                guess(command[1]);
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