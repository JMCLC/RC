#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
#include <fstream>
#include <ctime>
#include <math.h>

using namespace std;

typedef struct {
    int currentMove, maxErrors, currentErrors, missing;
    vector<string> wordsGuessed;
    vector<char> lettersPlayed;
    string word, hint;
} Game;


int fd, newfd, errcode, verboseMode = 1;
struct addrinfo hints,*res;
struct sockaddr_in addr;
string word_file,S_port;

//map<int,Game> gameList;

socklen_t addrlen;
ssize_t n;

string printStringArray(vector<string> array) {
    string text = "";
    for (int i = 0; i < array.size(); i++) {
        text += array[i];
        text += ' ';
    }
    return text;
}

string printCharArray(vector<char> array) {
    string text = "";
    for (int i = 0; i < array.size(); i++) {
        text += array[i];
        text += ' ';
    }
    return text;
}

void printGame(Game current) {
    cout << "Word: " << current.word << endl;
    cout << "Current Move: " << current.currentMove << endl;
    cout << "Max Errors: " << current.maxErrors << endl;
    cout << "Current Errors: " << current.currentErrors << endl;
    cout << "Missing: " << current.missing << endl;
    cout << "Words Guessed: " << printStringArray(current.wordsGuessed) << endl;
    cout << "Letters Played: ";
    for (int i = 0; i < current.lettersPlayed.size(); i++)
        cout << current.lettersPlayed[i] << " ";
    cout << endl;
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

void createGameFile(string fileName, Game game) {
    ofstream File(fileName);
    File << game.word << " " << game.hint << "\n" << game.currentMove << " " << game.maxErrors << " " << game.currentErrors << " " << game.missing << "\n" << printStringArray(game.wordsGuessed) << "\n" << printCharArray(game.lettersPlayed);
    File.close();
}

void deleteGameFile(int plId) {
    remove(("GAMES/GAME_" + to_string(plId) + ".txt").c_str());
}

void saveCurrentGame(Game game, int plId) {
    deleteGameFile(plId);
    createGameFile(("GAMES/GAME_" + to_string(plId) + ".txt"), game);
}

void saveGameScore(Game game, int plId, string time) {
    int successfulTries = game.currentMove - game.currentErrors; 
    double score = round(((successfulTries * 1.0) / (game.currentMove * 1.0)) * 100);
    ofstream File(("SCORES/" + to_string((int) score) + "_" + to_string(plId) + "_" + time + ".txt"));
    File << game.word << " " << successfulTries << " " << game.currentMove;
    File.close();
}

void archiveGame(Game game, int plId, char operation) {
    string folderName = ("GAMES/" + to_string(plId)), timeString, fileName;
    struct stat status = { 0 };
    time_t tempTime;
    char buffer[128];
    time(&tempTime);
    struct tm *currentTime = localtime(&tempTime);
    memset(buffer, 0, sizeof(buffer));
    strftime(buffer, sizeof(buffer),"%Y%m%d_%H%M%S",currentTime);
    timeString = buffer;
    fileName = (timeString + "_" + operation + ".txt");
    if(stat(folderName.c_str(), &status) == -1)
        mkdir(folderName.c_str(), 0700);
    ofstream File((folderName + "/" + fileName));
    File << game.word << " " << game.hint << "\n" << game.currentMove << " " << game.maxErrors << " " << game.currentErrors << " " << game.missing << "\n" << printStringArray(game.wordsGuessed) << "\n" << printCharArray(game.lettersPlayed);
    File.close();
    deleteGameFile(plId);
    if (operation == 'W')
        saveGameScore(game, plId, timeString);
}

Game readGameFile(int plId) {    
    vector<string> currentLine;
    int lineNumber = 0;
    string line;
    Game game;
    ifstream File(("GAMES/GAME_" + to_string(plId) + ".txt"));
    if (!File) {
        game.currentMove = -1;
        return game;
    }
    cout << "Starting to read" << endl;
    while (getline(File, line)) {
        lineNumber++;
        currentLine = stringSplitter(line);
        if (lineNumber == 1) {
            game.word = currentLine[0];
            game.hint = currentLine[1];
        } else if (lineNumber == 2) {
            game.currentMove = stoi(currentLine[0]);
            game.maxErrors = stoi(currentLine[1]);
            game.currentErrors = stoi(currentLine[2]);
            game.missing = stoi(currentLine[3]);
        } else if (lineNumber == 3) {
            for (int i = 0; i < currentLine.size(); i++)
                game.wordsGuessed.push_back(currentLine[i]);
        } else if (lineNumber == 4) {
            for (int i = 0; i < currentLine.size(); i++)
                game.lettersPlayed.push_back(currentLine[i][0]);
        }
    }
    return game;
}

string start(int plId){
    string response, word, fileName = ("GAMES/GAME_" + to_string(plId) + ".txt");
    Game newGame;
    ifstream File(fileName);
    if (File)
        return "RSG NOK\n";
    word = toUpper(random_word());
    newGame.word = word.c_str();
    newGame.hint = "PlaceHolder";
    newGame.missing = word.length();
    newGame.currentMove = 1;
    newGame.currentErrors = 0;
    newGame.wordsGuessed = {};
    newGame.lettersPlayed = {};
    if (word.length() <= 6)
        newGame.maxErrors = 7;
    else if(word.length() <= 10)
        newGame.maxErrors = 8;
    else 
        newGame.maxErrors = 9;
    if (verboseMode == 1)
        cout << "PLID=" << plId << ": new game; word = \"" << newGame.word << "\" ("<< word.length() << " letters)" << endl;
    response = ("RSG OK " + to_string(word.length()) + " " + to_string(newGame.maxErrors) + "\n");
    createGameFile(fileName, newGame);
    return response;
}

string play(int plId, string letter, int trial){
    Game game = readGameFile(plId);
    cout << trial  << game.currentMove << endl;
    if (letter.length() > 1 || game.currentMove == -1)
        return "RLG ERR\n";
    else if ((game.currentMove - 1 == trial && game.lettersPlayed.back() != letter[0]) || trial > game.currentMove || trial < game.currentMove - 1)
        return "RLG INV\n";
    for (int i = 0; i < game.lettersPlayed.size(); i++)
        if (game.lettersPlayed[i] == toupper(letter[0]))
            return "RLG DUP\n";
    game.currentMove++;
    game.lettersPlayed.push_back(toupper(letter[0]));
    string word = game.word;
    if (word.find(toupper(letter[0])) == string::npos) {
        game.currentErrors++;
        if (game.currentErrors > game.maxErrors) {
            archiveGame(game, plId, 'F');
            return "RLG OVR\n";
        } else {
            saveCurrentGame(game, plId);
            return "RLG NOK\n";
        }
    }
    vector<int> positions;
    for (int i = 0; i < word.length(); i++) {
        if (letter[0] == game.word[i]) {
            positions.push_back(i + 1);
            game.missing--;
        }
    }
    cout << "Missing: " << game.missing << " Positions: " << positions.size() << endl; 
    if (game.missing == 0) {
        archiveGame(game, plId, 'W');
        return "RLG WIN\n";
    }
    string response = ("RLG OK " + to_string(game.currentMove - 1) + " " + to_string(positions.size()) + " ");
    saveCurrentGame(game, plId);
    for (int i = 0; i < positions.size() - 1; i++)
        response.append((to_string(positions[i]) + " "));
    response.append(to_string(positions.back()));
    return response;
}

string guess(int plId, string word, int trial){
    Game game = readGameFile(plId);
    if (game.currentMove == -1)
        return "RWG ERR\n";
    else if ((game.currentMove - 1 == trial && game.wordsGuessed.back() != word) || trial > game.currentMove || trial < game.currentMove - 1)
        return "RWG INV\n";
    for (int i = 0; i < game.wordsGuessed.size(); i++)
        if (toUpper(game.wordsGuessed[i]) == toUpper(word))
            return "RWG DUP\n";
    game.currentMove++;
    game.wordsGuessed.push_back(word);
    if (game.word != word) {
        game.currentErrors++;
        if (game.currentErrors > game.maxErrors) {
            archiveGame(game, plId, 'F');
            return "RWG OVR\n";
        } else {
            saveCurrentGame(game, plId);
            return "RWG NOK\n";
        }
    }
    archiveGame(game, plId, 'W');
    return "RWG WIN\n";
}

string quit(int plId) {
    Game game = readGameFile(plId);
    if (game.currentMove == -1)
        return "RQT NOK\n";
    else {
        archiveGame(game, plId, 'Q');
        return "RQT OK\n";
    }
}

void handleGame() {
    vector<string> currentCommand;
    string response, bufferStr;
    char buffer[128]; 
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addrlen);
        bufferStr = buffer;
        bufferStr.substr(0, n);
        currentCommand = stringSplitter(bufferStr);
        cout << "Received command: " << printStringArray(currentCommand) << endl;
        if (currentCommand[0] == "SNG") {
            if (currentCommand.size() != 2) {
                cout << "Bad Formatting" << endl;
                response = "RSG ERR\n";
            } else
                response = start(stoi(currentCommand[1]));
            cout << "Received response " <<endl;
        } else if (currentCommand[0] == "PLG") { 
            if (currentCommand.size() != 4) {
                cout << "Bad Formatting" << endl;
                response = "RLG ERR\n";
            } else
                response = play(stoi(currentCommand[1]), currentCommand[2], stoi(currentCommand[3]));
        } else if(currentCommand[0] == "PWG") {
            if (currentCommand.size() != 4) {
                cout << "Bad Formatting" << endl;
                response = "RLG ERR\n";
            } else
                response = guess(stoi(currentCommand[1]), currentCommand[2], stoi(currentCommand[3]));
        } else if(currentCommand[0] == "QUT") {
            if (currentCommand.size() != 2) {
                cout << "Bad Formatting" << endl;
                response = "RQT ERR\n";
            } else
                response = quit(stoi(currentCommand[1]));
        }
        cout << "Sending command: " << response;
        n = sendto(fd, response.c_str(), response.length(), 0, (struct sockaddr*)&addr, addrlen);
    }
}

bool file_empty(std::ifstream& pFile){
     return pFile.peek() == std::ifstream::traits_type::eof();
}

// void sendScoreboard(char buffer[128]){
//     std::ifstream file("/home/gd/GS/SCORES/_top10_scores.txt");
//     if(file_empty(file) == true){
//         bzero(buffer,sizeof(buffer));
        
//     }
// }

void createTCPconnection(string S_port){
    char buffer[128];
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
        // if(code.compare("GSB") == 0){
        //     sendScoreboard(buffer);
        // }
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