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
int wordSize, errors, currentMove, playerId, fd, error;
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

void printScoreboard(string text) {
    return;
}

void printGameState(string text) {
    return;
}

void sendMessageToServerTCP(string message) {
    struct addrinfo tcp_hints;
    vector<string> response;
    string currentString, stringBuffer;
    char buffer[128];
    ssize_t tcp_n;
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0), currentSize = 0, count = 0;
    tcp_hints.ai_family = AF_INET;
    tcp_hints.ai_socktype = SOCK_STREAM;
    tcp_n = connect(tcp_fd, res->ai_addr, res->ai_addrlen);
    tcp_n = write(tcp_fd, message.c_str(), sizeof(message.c_str()));
    tcp_n = read(tcp_fd, buffer, 128);
    stringBuffer = buffer;
    response = stringSplitter(stringBuffer);
    if (response[1] == "OK") {
        ofstream File(response[2]);
        for (int i = 0; i < sizeof(buffer); i++) {
            if (buffer[i] == ' ') {
                count++;
            }
            if (count == 4) {
                currentString = buffer;
                currentString = currentString.substr(i + 1, sizeof(i + 2));
                break;
            }
        }
        for (currentSize = sizeof(currentString); currentSize < stoi(response[3]); currentSize += sizeof(currentString)) {
            File << currentString;
            if (stoi(response[3]) - currentSize < 128)
                tcp_n = read(tcp_fd, buffer, stoi(response[3]) - currentSize);
            else
                tcp_n = read(tcp_fd, buffer, 128);
            currentString = buffer; 
        }
        File.close();
    }
    if (response[0] == "RSB")
        printScoreboard(response[2]);
    else if (response[0] == "RHL")
        cout << "received hint file: " << response[2] << " (" << response[3] << " bytes)" << endl;
    else if (response[0] == "RST")
        printGameState(response[2]);
    close(tcp_n);
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
        errors = stoi(response[3]);
        currentMove++;
        for (int i = 0; i < wordSize; i++)
            currentWord.append("_ ");
            cout << "New game started (max " << errors << " errors): " << currentWord << endl;
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
            cout << "Yes, " << letter << " is part of the word: " << currentWord << endl;
            currentMove++;
        } else if (response[1] == "WIN") {
            for (int i = 0; i < sizeof(currentWord); i++)
                if (currentWord[i] == '_')
                    currentWord[i] = toupper(letter[0]);
            cout << "WELL DONE! You guessed: " << currentWord << endl;
            currentMove++;
        } else if (response[1] == "DUP") {
            cout << "This letter has already been sent" << endl;
        } else if (response[1] == "NOK") {
            cout << "No, " << letter << " is not part of the word" << endl;
            errors--;
            currentMove++;
        } else if (response[1] == "OVR") {
            cout << "Game over!" << endl;
            currentMove;
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
            cout << "WELL DONE! You guessed: " << currentWord << endl;
            currentMove++;            
        } else if (response[1] == "NOK") {
            cout << "No, " << word << " is not the word";
            errors--;
            currentMove++;
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
    return;
}

void hint() {
    return;
}

void state() {
    return;
}

void quit() {
    exit(1);
    return;
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
            quit();
        else
            cout << "This is not a valid command!" << endl;
    }
}

int main(int argc, char** args) {
    if (argc > 0) {
        if (args[0] == "-n")
            server_ip = args[1];
        else if (args[0] == "-p")
            port = args[1];
    }
    if (argc > 2) {
        if (args[2] == "-n")
            server_ip = args[3];
        else if (args[2] == "-p")
            port = args[3];
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