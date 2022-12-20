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

typedef struct{
    int currentMove = 1, maxErrors, currentErrors = 0;
    vector<char> lettersPlayed;
    string word, currentWord;
} game;

struct addrinfo hints,*res;
struct sockaddr_in addr;
string word_file,S_port;
map<int,game> gameList;
int fd, newfd, errcode, verboseMode = 1;
using namespace std;
socklen_t addrlen;
char buffer[128];
ssize_t n;

void printArray(vector<string> array) {
    for (int i = 0; i < array.size(); i++) {
        cout << array[i] << endl;
    }
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
    newGame.word = random_word();    
    for (int i = 0; i < wordSize - 1; i++)
        newGame.currentWord.append("_");
    if (newGame.word.length() <= 6)
        newGame.maxErrors = 7;
    else if(newGame.word.length() <= 10)
        newGame.maxErrors = 8;
    else 
        newGame.maxErrors = 9;
    if (verboseMode == 1)
        cout << "PLID=" << plId << ": new game; word = \"" << newGame.word << "\" ("<< newGame.word.length() << " letters)" << endl;
    response = ("RSG OK" + " " + to_string(newGame.word.length()) + " " + to_string(newGame.maxErrors) + "\n");    
    gameList[stoi(plId)] = newGame;
    return response;
}

string play(int plId, string letter, int trial){
    if (letter.length() > 1 || gameList.find(plId) == gameList.end())
        return "RWG ERR\n";
    else if ((gameList[plId].currentMove - 1 == trial && gameList[plId].lettersPlayed.back() != letter[0]) || trial > gameList[plId].currentMove || trial < gameList[plId].currentMove - 1)
        return "RWG INV\n";
    for (int i = 0; i < gameList[plId].lettersPlayed.size(); i++)
        if (gameList[plId].lettersPlayed[i] == letter[0])
            return "RWG DUP\n";
    if (gameList[plId].word.find(letter[0]) == gameList[plId].word.end()) {
        gameList[plId].currentErrors++;
        if (gameList[plId].currentErrors > gameList[plId].maxErrors) {
            gameList.erase(plId);
            return "RWG OVR\n";
        } else
            return "RWG NOK\n";
    }
    vector<int> positions;
    int missing;
    for (int i = 0; i < gameList[plId].word.length(); i++) {
        if (letter[0] == gameList[plId].word[i])
            positions.push_back(i + 1);
        if (gameList[plId].currentWord[i] == "_")
            missing++;
    }
    if (missing == positions.size()) {
        winGame();
        return "RWG WIN\n";
    }
    response = ("RWG OK " + to_string(positions.size()) + " ");
    for (int i = 0; i < positions.size() - 1; i++)
        response.append((to_string(positions[i]) + " "));
    response.append(to_string(positions.back()));
    return response;
}

string guess(string PLID,string word,string trial){
    int P_id = stoi(PLID);
    string response,gameData;
    if(word.compare(SV_Game[P_id].letter) == 0){
        SV_Game[P_id].status = "WIN";
        SV_Game[P_id].trials++;
        SV_Game[P_id].s_trials++;
        SV_Game[P_id].score = (SV_Game[P_id].s_trials*100)/SV_Game[P_id].trials;
            fstream file;
    file.open("/home/gd/GS/SCORES/_"+ PLID + "_31/12/2022.txt",ios::out);
    if(!file){
        cout << "Error in creating file" << endl;
    }
    gameData = (to_string(SV_Game[P_id].score) + PLID  + " " + SV_Game[P_id].word + " " + to_string(SV_Game[P_id].s_trials) + to_string(SV_Game[P_id].trials));
    file << gameData;
    if (file.is_open()){
    cout << "Stream could not close!" << endl;
    }
    cout << "PLID=" << PLID << "guess \"" + word + "\" - "  + SV_Game[P_id].status + " (game ended)" << endl;
    response = ("RWG " + SV_Game[P_id].status + to_string(SV_Game[P_id].trials) + "\n");
    return response;
    }
    else if(word.compare(SV_Game[P_id].letter) != 0 && SV_Game[P_id].attempts > 0){
        SV_Game[P_id].status = "NOK";
        SV_Game[P_id].trials++;
        cout << "PLID=" << PLID << "guess \"" + word + "\" - "  + SV_Game[P_id].status + " " + to_string(SV_Game[P_id].trials) + "attempts left" << endl;
        response = ("RWG " + SV_Game[P_id].status + to_string(SV_Game[P_id].trials) + "\n");
        return response;
    }
    else if(word.compare(SV_Game[P_id].letter) != 0 && SV_Game[P_id].attempts == 0){
        SV_Game[P_id].status = "OVR";
        SV_Game[P_id].trials++;
        cout << "PLID=" << PLID << "guess \"" + word + "\" - "  + SV_Game[P_id].status + "(game ended)" << endl;
        response = ("RWG " + SV_Game[P_id].status + to_string(SV_Game[P_id].trials) + "\n");
        return response;
    }
return "ERR\n"; 
} 

string quit(string PLID){
    int P_id = stoi(PLID);
    string response;
    if(SV_Game.find(P_id) == SV_Game.end()){
        SV_Game[P_id].status = "OK";
        cout << "PLID=" << PLID << "game ongoing" << endl;
        response = ("RQT " + SV_Game[P_id].status + "\n");
        return response;
    }
    else{
        SV_Game[P_id].status = "ERR";
        cout << "PLID=" << PLID << "no game found" << endl;
        response = ("RQT " + SV_Game[P_id].status + "\n");
        return response;
    }
}

string reveal(string PLID){
    int P_id = stoi(PLID);
    string response;
    cout << "PLID=" << PLID << "sending word \"" + SV_Game[P_id].word + "\" to client"<< endl;
    response =("RRV" + SV_Game[P_id].word + "\n");
    return response;

}


void handleGame(){
    vector<string> currentCommand;
    string response, bufferStr;        
    while(1){
        addrlen = sizeof(addr);
        n = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&addr,&addrlen);
        bufferStr = buffer;
        bufferStr.substr(0, n);
        currentCommand = stringSplitter(bufferStr);
        cout << "Received command: " << currentCommand[0] << endl;
        if (code == "SNG")
            response = start(currentCommand[1]);
        else if (code == "PLG")
            response = play(stoi(currentCommand[1]), currentCommand[2], currentCommand[3]);
        else if(code == "PWG")
            response = guess(stoi(currentCommand[1]), currentCommand[2], currentCommand[3]);
        else if(code == "QUT")
            response = quit(stoi(currentCommand[1]));
        else if(code == "REV")
            response = reveal(stoi(currentCommand[1]));
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
    errcode=getaddrinfo(NULL,S_port.c_str(),&hints,&res);
    n = bind(fd,res->ai_addr,res->ai_addrlen);
    handleGame();
    freeaddrinfo(res);
    close(fd);
    return 0;
}
