#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <map>
#include <ctype.h>
#include <iterator>
#include <sstream>

using namespace std;

typedef struct{
    int currentMove = 1, maxErrors, currentErrors = 0, missing;
    vector<string> wordsGuessed;
    vector<char> lettersPlayed;
    string word;
} game;

int fd, newfd, errcode, verboseMode = 1;
struct addrinfo hints,*res;
struct sockaddr_in addr;
string word_file,S_port;
map<int,game> gameList;

socklen_t addrlen;
char buffer[128];
ssize_t n;

void printArray(vector<string> array) {
    for (int i = 0; i < array.size(); i++) {
        cout << array[i] << endl;
    }
}

string toUpper(string text) {
    for (int i = 0; i < text.length(); i++)
        text[i] = toupper(text[i]);
    return text;
}

vector<string> stringSplitter(string text) {
    stringstream text_stream(text);
    istream_iterator<string> begin(text_stream);
    istream_iterator<string> end;
    vector<string> res(begin, end);
    return res;
}

string random_word() {
    string line;
    vector<string> lines;
    srand(time(0));
    ifstream file(word_file);
    
    int total_lines=0;
    while(getline(file,line)) {
        total_lines++; 
        lines.push_back(line);  
    }
   
    int random_number=rand()%total_lines;
    string rw = lines[random_number];

    return rw;
}

string start(string plId){
    string response;
    game newGame;
    if (gameList.find(stoi(plId)) != gameList.end())
        return "RSG NOK\n";
    newGame.word = toUpper(random_word());
    newGame.missing = newGame.word.length();
    if (newGame.word.length() <= 6)
        newGame.maxErrors = 7;
    else if(newGame.word.length() <= 10)
        newGame.maxErrors = 8;
    else 
        newGame.maxErrors = 9;
    if (verboseMode == 1)
        cout << "PLID=" << plId << ": new game; word = \"" << newGame.word << "\" ("<< newGame.word.length() << " letters)" << endl;
    response = ("RSG OK " + to_string(newGame.word.length()) + " " + to_string(newGame.maxErrors) + "\n");    
    gameList[stoi(plId)] = newGame;
    return response;
}

string play(int plId, string letter, int trial){
    cout << trial  << gameList[plId].currentMove << endl;
    if (letter.length() > 1 || gameList.find(plId) == gameList.end())
        return "RLG ERR\n";
    else if ((gameList[plId].currentMove - 1 == trial && gameList[plId].lettersPlayed.back() != letter[0])|| trial > gameList[plId].currentMove || trial < gameList[plId].currentMove - 1)
        return "RLG INV\n";
    for (int i = 0; i < gameList[plId].lettersPlayed.size(); i++)
        if (gameList[plId].lettersPlayed[i] == toupper(letter[0]))
            return "RLG DUP\n";
    gameList[plId].currentMove++;
    gameList[plId].lettersPlayed.push_back(toupper(letter[0]));
    if (gameList[plId].word.find(toupper(letter[0])) == string::npos) {
        gameList[plId].currentErrors++;
        if (gameList[plId].currentErrors > gameList[plId].maxErrors) {
            gameList.erase(plId);
            return "RLG OVR\n";
        } else 
            return "RLG NOK\n";
    }
    vector<int> positions;
    for (int i = 0; i < gameList[plId].word.length(); i++) {
        if (letter[0] == gameList[plId].word[i]) {
            positions.push_back(i + 1);
            gameList[plId].missing--;
        }
    }
    cout << "Missing: " << gameList[plId].missing << " Positions: " << positions.size() << endl; 
    if (gameList[plId].missing == 0) {
        //winGame();
        return "RLG WIN\n";
    }
    string response = ("RLG OK " + to_string(gameList[plId].currentMove - 1) + " " + to_string(positions.size()) + " ");
    for (int i = 0; i < positions.size() - 1; i++)
        response.append((to_string(positions[i]) + " "));
    response.append(to_string(positions.back()));
    return response;
}

string guess(int plId, string word, int trial){
    if (gameList.find(plId) == gameList.end())
        return "RWG ERR\n";
    else if ((gameList[plId].currentMove - 1 == trial && gameList[plId].wordsGuessed.back() != word) || trial > gameList[plId].currentMove || trial < gameList[plId].currentMove - 1)
        return "RWG INV\n";
    for (int i = 0; i < gameList[plId].wordsGuessed.size(); i++)
        if (toUpper(gameList[plId].wordsGuessed[i]) == toUpper(word))
            return "RWG DUP\n";
    if (gameList[plId].word != word) {
        gameList[plId].currentErrors++;
        if (gameList[plId].currentErrors > gameList[plId].maxErrors) {
            gameList.erase(plId);
            return "RWG OVR\n";
        } else {
            gameList[plId].wordsGuessed.push_back(word);
            return "RWG NOK\n";
        }
    }
    //winGame();
    return "RWG WIN\n";
    // string response,gameData;
    //         fstream file;
    // file.open(*"/home/gd/GS/SCORES/_"+ plId + "_31/12/2022.txt",ios::out);
    // if(!file){
    //     cout << "Error in creating file" << endl;
    // }
    // gameData = (plId  + " " + gameList[plId].word + " " + to_string(gameList[plId].currentMove-gameList[plId].currentErrors) + to_string(gameList[plId].currentMove));
    // file << gameData;
    // if (file.is_open()){
    // cout << "Stream could not close!" << endl;
    // }
    // cout << "PLID=" << plId << "guess \"" + word + "\" -WIN (game ended)" << endl;
    // //winGame();
    // return "RWG WIN\n";
    // if (gameList[plId].word == word) {
    //     gameList[plId].currentErrors++;
    //     if (gameList[plId].currentErrors > gameList[plId].maxErrors) {
    //         gameList.erase(plId);
    //         return "RWG OVR\n";
    //     } else
    //         return "RWG NOK\n";
    // }
    // return "ERR\n"; 
}

string quit(int plId) {
    if (gameList.find(plId) == gameList.end())
        return "RQT NOK\n";
    else {
        gameList.erase(plId);
        return "RQT OK\n";
    }
}

void handleGame(){
    vector<string> currentCommand;
    string response, bufferStr;        
    while (1) {
        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addrlen);
        bufferStr = buffer;
        bufferStr.substr(0, n);
        currentCommand = stringSplitter(bufferStr);
        cout << "Received command: " << currentCommand[0] << endl;
        if (currentCommand[0] == "SNG")
            response = start(currentCommand[1]);
        else if (currentCommand[0] == "PLG") { 
            cout << currentCommand[1] << " " << currentCommand[2] << " " << currentCommand[3] << endl;
            if (currentCommand.size() != 4)
                response = "RLG ERR\n";
            else
                response = play(stoi(currentCommand[1]), currentCommand[2], stoi(currentCommand[3]));
        } else if(currentCommand[0] == "PWG") {
            if (currentCommand.size() != 4)
                response = "RWG ERR\n";
            else
                response = guess(stoi(currentCommand[1]), currentCommand[2], stoi(currentCommand[3]));
        } else if(currentCommand[0] == "QUT") {
            if (currentCommand.size() != 2)
                response = "RQT ERR\n";
            else
                response = quit(stoi(currentCommand[1]));
        }
        cout << "Sending command: " << response;
        n = sendto(fd, response.c_str(), response.length(), 0, (struct sockaddr*)&addr, addrlen);
    }
}

bool file_empty(std::ifstream& pFile){
     return pFile.peek() == std::ifstream::traits_type::eof();
}

void sendScoreboard(char bufffer[128]){
    std::ifstream file("/home/gd/GS/SCORES/_top10_scores.txt");
    if(file_empty(file) == true){
        bzero(buffer,sizeof(buffer));
        
    }
}

void createTCPconnection(string S_port){
    string code;
    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1) exit(1);

    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL,S_port.c_str(),&hints,&res);
    if((errcode != 0)) exit(1);

    n = bind(fd,res->ai_addr,res->ai_addrlen);
    if(n==-1) exit(1);

    if(listen(fd,100) == -1) exit(1);

    while(1){
        addrlen = sizeof(addr);
        if(newfd =accept(fd,(struct sockaddr*)&addr,&addrlen) == -1) exit(1);

        n = read(newfd,buffer,128);
        if(n == -1) exit(1);

        //sscanf(buffer,"%s",code);
        if(code.compare("GSB") == 0){
            sendScoreboard(buffer);
        }
    }
}

int main(int argc, char** args){
    string tst;
    word_file = args[1];
    if(strcmp(args[2],"-p")==0){ 
        S_port = args[3];
    } else if(strcmp(args[2],"-v")==0){ 
        printf("58011\n");
    }
    fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; // IPv4
    hints.ai_socktype=SOCK_DGRAM; // UDP socket
    hints.ai_flags=AI_PASSIVE;
    errcode=getaddrinfo(NULL, S_port.c_str(), &hints, &res);
    n = bind(fd, res->ai_addr, res->ai_addrlen);
    handleGame();
    freeaddrinfo(res);
    close(fd);
    return 0;
}