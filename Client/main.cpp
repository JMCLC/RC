#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cctype>
#include <cstring>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <iterator>
#include <sstream>

using namespace std;
int wordSize, maxErrors, currentErrors, currentMove, playerId, fd, error;
struct addrinfo hints, *res;
struct sockaddr_in addr;
string port, server_ip;
string currentWord;
socklen_t addrlen;
ssize_t n;

void printArray(vector<string> array) {
    for (int i = 0; i < array.size(); i++) {
        cout << array[i] << " ";
    }
    cout << endl;
}

string printCurrentWord() {
    string wordOutput = "";
    for (int i = 0; i < wordSize - 1; i++) {
        wordOutput += currentWord[i];
        wordOutput += ' ';
    }
    wordOutput += currentWord.back();
    return wordOutput;
}

vector<string> stringSplitter(string text) {
    stringstream text_stream(text);
    istream_iterator<string> begin(text_stream);
    istream_iterator<string> end;
    vector<string> res(begin, end);
    return res;
}

string toUpper(string text) {
    for (int i = 0; i < text.length(); i++)
        text[i] = toupper(text[i]);
    return text;
}

void resetGame() {
    wordSize = 0;
    maxErrors = 0;
    currentErrors = 0;
    currentMove = 0;
    playerId = 0;
    currentWord = "";
}

vector<string> sendMessageToServer(string message) {
    string response;
    char buffer[128];
    n = sendto(fd, message.c_str(), message.length(), 0, res->ai_addr, res->ai_addrlen);
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (n == -1) {
        cout << "Did not receive a response from the server in time" << endl;
        return {};
    }
    response = buffer;
    response = response.substr(0, n);
    return stringSplitter(response);
}

void sendMessageToServerTCP(string message) {
    int size = 0, bufferSize = 0, status = -1, currentSize = 0, tcpFd, readSize = -1;
    char buffer[10241], confirmation[] = "OK";
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    vector<string> command;    
    
    
    FILE *file;
    string line;
    tcpFd = socket(AF_INET, SOCK_STREAM, 0);
    connect(tcpFd, res->ai_addr, res->ai_addrlen);
    while (status < 0) {
        status = read(socket, &buffer, sizeof(buffer));
        line = buffer;
        command = stringSplitter(line);
        if (command[1] == "EMPTY") {
            cout << "No game yet has been won" << endl;
            return;
        } else if (command[1] == "NOK" && command[0] == "RHL") {
            cout << "There is no hint for this word" << endl;
            return;
        } else if (command[1] == "NOK") {
            cout << "No games found for this player" << endl;
            return;
        }
        size = stoi(command[3]);
    }
    status = -1;
    while (status < 0) {
        status = write(tcpFd, &confirmation, sizeof(int));
    }
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    file = fopen(command[2], "w");
    int bufferFd;
    fd_set fds;
    while (currentSize < size) {
        FD_ZERO(&fds);
        FD_SET(tcpFd, &fds);
        bufferFd = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
        while (read < 0)
            read = read(tcpFd, buffer, sizeof(buffer));
        fwrite(buffer, 1, read, file);
        currentSize += read;
    }
    if (command[0] == "RST")
        printState();
    fclose(file);
    close(tcpFd);
}

void start(string plId) {
    vector<string> response = sendMessageToServer("SNG " + plId + "\n");
    cout << "Received: ";
    printArray(response);
    if (response.size() == 0)
        return;
    playerId = stoi(plId);
    if (response[1] == "OK") {
        wordSize = stoi(response[2]);
        maxErrors = stoi(response[3]);
        currentMove = 1;
        currentErrors = 0;
        currentWord = "";
        for (int i = 0; i < wordSize; i++)
            currentWord.append("_");
        cout << "New game started (max " << maxErrors << " errors): " << printCurrentWord() << endl;
    } else
        cout << "There is already an ongoing game for this player id" << endl;
}

void play(string letter) {
    vector<string> response = sendMessageToServer("PLG " + to_string(playerId) + " " + toUpper(letter) + " " + to_string(currentMove) + "\n");
    cout << "Received: ";
    printArray(response);
    if (response.size() == 0)
        return;
    if (response[0] == "RLG") {
        if (response[1] == "OK") {
            for (int i = 0; i < stoi(response[3]); i++)
                currentWord[stoi(response[i + 4]) - 1] = toupper(letter[0]);
            cout << "Yes, " << toUpper(letter) << " is part of the word: " << printCurrentWord() << endl;
            currentMove++;
        } else if (response[1] == "WIN") {
            for (int i = 0; i < sizeof(currentWord); i++)
                if (currentWord[i] == '_')
                    currentWord[i] = toupper(letter[0]);
            cout << "WELL DONE! You guessed: " << toUpper(printCurrentWord()) << endl;
            currentMove++;
            resetGame();
        } else if (response[1] == "DUP") {
            cout << "This letter has already been sent" << endl;
        } else if (response[1] == "NOK") {
            cout << "No, " << toUpper(letter) << " is not part of the word" << endl;
            currentErrors++;
            currentMove++;
        } else if (response[1] == "OVR") {
            cout << "Game over!" << endl;
            currentMove++;
        } else if (response[1] == "INV") {
            return;
        } else if (response[1] == "ERR") {
            return;
        }
    } else
        cout << "Error" << endl;
}

void guess(string word) {
    vector<string> response = sendMessageToServer("PWG " + to_string(playerId) + " " + toUpper(word) + " " + to_string(currentMove) + "\n");
    cout << "Received: ";
    printArray(response);
    if (response.size() == 0)
        return;
    if (response[0] == "RWG") {
        if (response[1] == "WIN") {
            currentWord = word;
            cout << "WELL DONE! You guessed: " << toUpper(printCurrentWord()) << endl;
            currentMove++; 
            resetGame();
        } else if (response[1] == "NOK") {
            cout << "No, " << word << " is not the word" << endl;
            currentErrors++;
            currentMove++;
        } else if (response[1] == "DUP") {
            cout << "This word has already been guessed" << endl;
        } else if (response[1] == "OVR") {
            cout << "Game over!" << endl;
            currentMove;
        } else if (response[1] == "INV") {
            return;
        } else if (response[1] == "ERR") {
            return;
        }
    }
}

void scoreboard() {
    sendMessageToServerTCP("GSB");
}

void hint(string plId) {
    sendMessageToServerTCP(("GHL " + plId));
}

void state(string plId) {
    sendMessageToServerTCP(("STA " + plId));
}

void quit(string command) {
    vector<string> response = sendMessageToServer("QUT " + to_string(playerId) + "\n");
    if (command == "quit") {
        if (response[1] == "ERR")
            cout << "Error" << endl;
        else if (response[1] == "NOK")
            cout << "There is no ongoing game" << endl;
        else
            cout << "Previous game has been quit" << endl;
    }
}

void handleGame() {
    vector<string> command = {""};
    string line;
    while (command[0] != "exit") {
        getline(cin, line);
        if (line.length() == 0)
            continue;
        command = stringSplitter(line);
        if (command[0] == "start" || command[0] == "sg")
            start(command[1]);
        else if (command[0] == "play" || command[0] == "pl")
            play(command[1]);
        else if (command[0] == "guess" || command[0] == "gw") 
            guess(command[1]);
        else if (command[0] == "scoreboard" || command[0] == "sb")
            scoreboard();
        else if (command[0] == "hint" || command[0] == "h")
            hint();
        else if (command[0] == "state" || command[0] == "st")
            state();
        else if (command[0] == "quit" || command[0] == "exit")
            quit(command[0]);
        else
            cout << "This is not a valid command!" << endl;
        line = "";
    }
}

int main(int argc, char** args) {
    if (argc > 1) {
        if (args[1] == "-n")
            server_ip = args[2];
        else if (args[1] == "-p")
            port = args[2];
    }
    if (argc > 3) {
        if (args[3] == "-n")
            server_ip = args[4];
        else if (args[3] == "-p")
            port = args[4];
    }
    if (port.length() == 0)
        port = "58061";
    if (server_ip.length() == 0)
        server_ip = "127.0.0.1";
    cout << "Connecting to server: " << server_ip << ":" << port << endl;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    string message;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    error = getaddrinfo(server_ip.c_str(), port.c_str(), &hints, &res);
    if (error != 0) {
        cout << "Could not reach the specified server" << endl;
        return 0;
    }
    handleGame();
    freeaddrinfo(res);
    close(fd);
    return 1;
}