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

int fd,newfd,errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints,*res;
struct sockaddr_in addr;
char buffer[128];

using namespace std;
string word_file,S_port;

typedef struct{
    int trials;
    int s_trials;
    string word;
    string pos;
    string status;
    string letter;
    int max_errors;
    int attempts;
    int score;
    map <string,string> cpy_word;
}game;
map<int,game> SV_Game;



string random_word(string word_file)
{
    string line;
    vector<string> lines;
    
    srand(time(0));
    
    ifstream file(word_file);
    
    int total_lines=0;
    while(getline(file,line))
    {
       total_lines++; 
    lines.push_back(line);  
  }
   
    int random_number=rand()%total_lines;

    string rw = lines[random_number];

    cout<<lines[random_number] << endl;

    return rw;
  
}

string RSG(string PLID,map<int,game> SV_Game,string word_file){
    int P_id, max_errors;
    string response;
    P_id = stoi(PLID);
    game SV;
    SV.word = random_word(word_file);
    SV.trials = 1;
    SV.s_trials = 0;
    SV.score = 0;
    if (SV.word.length() <= 6){
                max_errors = 7;
            }
    else if(SV.word.length()<=10){
                max_errors = 8;
        }
    else {
                max_errors = 9;
        }
    SV.max_errors = max_errors;
    SV.attempts = max_errors;
    SV.status = "OK";
    SV.pos = "";
    SV.letter = "";
    SV.cpy_word.insert(pair<string,string>("",""));
    SV_Game[P_id] = SV;
    if(SV_Game.find(P_id) == SV_Game.end()){
        SV_Game[P_id].status = "NOK";
        response = ("RSG " + SV_Game[P_id].status +  "\n");
        n = sendto(fd,response.c_str(),n,0,(struct sockaddr*)&addr,addrlen);
    }
    cout << "PLID=" << PLID << ": new game; word = \"" + SV_Game[P_id].word + "\"(" + to_string(SV_Game[P_id].max_errors) + " letters)"<<endl; 
    response = ("RSG " + to_string(SV.word.length()) + " " + to_string(SV.max_errors) + " " +  "\n");
    return response;
}

string RLG(string PLID,string str,map<int,game> SV_Game){
    int P_id = stoi(PLID);
    int n_hits = 0;
    string letter_test,str_test,response;
    SV_Game[P_id].letter.append(str);
    for(int i =0; i < SV_Game[P_id].letter.length();i++){
        if(str.compare(SV_Game[P_id].letter) == 0){
            SV_Game[P_id].status = "DUP";
            response = ("RLG " + SV_Game[P_id].status + "\n");
            return response;
        }
    }
    for(int i =0; i<=SV_Game[P_id].word.length();i++){
        if (*str.c_str() == SV_Game[P_id].word[i]){
            SV_Game[P_id].pos.append(str);
            SV_Game[P_id].pos.append(to_string(i));
            n_hits++;
        }
        str_test[i] = *"_";
        }
    for(int i =0;i<SV_Game[P_id].pos.length();i++){
        if(isalpha(SV_Game[P_id].pos[i])){
            letter_test = SV_Game[P_id].pos[i];
        }
        else if(isdigit(SV_Game[P_id].pos[i])){
            int ipos = SV_Game[P_id].pos[i] - '0';
            str_test[ipos] = *letter_test.c_str(); 
        }
    }
    for(int i =0; i <= str_test.length();i++){
        if(*str.c_str() == str_test[i]){
            SV_Game[P_id].status = "OK";
            SV_Game[P_id].trials++;
            SV_Game[P_id].s_trials++;
            cout << "PLID=" << PLID << "play letter \"" + str + "\" - " + to_string(n_hits) + " hits;word not guessed" << endl;
            response = ("RLG " + SV_Game[P_id].status +  to_string(SV_Game[P_id].trials) + to_string(n_hits) + "\n");
            return response;
        }
        if(SV_Game[P_id].attempts > 0){
            SV_Game[P_id].attempts--;
            SV_Game[P_id].status = "NOK";
            SV_Game[P_id].trials++;
            cout << "PLID=" << PLID << "play letter \"" + str + "\" - "  + "no hits; " + to_string(SV_Game[P_id].attempts)  + "attempts left"<< endl;
            response = ("RLG " + SV_Game[P_id].status + to_string(SV_Game[P_id].trials) + "\n");
            return response;
        }
        else if(SV_Game[P_id].attempts == 0){
            SV_Game[P_id].status = "OVR";
            SV_Game[P_id].trials++;
            cout << "PLID=" << PLID << "play letter \"" + str + "\" - "  + "no hits;no attempts left"<< endl;
            response = ("RLG " + SV_Game[P_id].status +  to_string(SV_Game[P_id].trials) + "\n");
            return response;
    }
    if(str_test.compare(SV_Game[P_id].word) == 0){
        SV_Game[P_id].status = "WIN";
        SV_Game[P_id].trials++;
        SV_Game[P_id].s_trials++;
        cout << "PLID=" << PLID << "play letter \"" + str + "\" - " + to_string(n_hits) + " hits;word guessed" << endl;
        response = ("RLG " + SV_Game[P_id].status + to_string(SV_Game[P_id].trials) + "\n");
        return response;
    }
    return response;
    }
return "ERR\n";
}

string RWG(string PLID,string word,string trial,map<int,game> SV_Game){
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

string RQT(string PLID,map<int,game> SV_Game){
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

string RRV(string PLID,map<int,game> SV_Game){
    int P_id = stoi(PLID);
    string response;
    cout << "PLID=" << PLID << "sending word \"" + SV_Game[P_id].word + "\" to client"<< endl;
    response =("RRV" + SV_Game[P_id].word + "\n");
    return response;

}


void handleGame(string S_port,string word_file){
        char code[3],PLID[7],str1[31],str2[5];
        string rw,response;
        fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
        if(fd==-1) /*error*/exit(1);
        
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET; // IPv4
        hints.ai_socktype=SOCK_DGRAM; // UDP socket
        hints.ai_flags=AI_PASSIVE;
        
        errcode=getaddrinfo(NULL,S_port.c_str(),&hints,&res);
        if(errcode!=0) /*error*/ exit(1);
        
        n = bind(fd,res->ai_addr,res->ai_addrlen);
        if(n==-1) exit(1);
        
        while(1){ 
        n = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&addr,&addrlen);
        if(n==-1) exit(1);
        sscanf(buffer,"%s %s %s %s",code,PLID,str1,str2);
        if(strcmp(code,"SNG") == 0 ){
            response = RSG(PLID,SV_Game,word_file);
            n = sendto(fd,response.c_str(),n,0,(struct sockaddr*)&addr,addrlen);
            if(n==-1) exit(1);

        }
        else if(strcmp(code,"PLG") == 0){
            response = RLG(PLID,str1,SV_Game);
            n = sendto(fd,response.c_str(),n,0,(struct sockaddr*)&addr,addrlen);
            if(n==-1) exit(1);
        }
        else if(strcmp(code,"PWG") == 0){
            response = RWG(PLID,str1,str2,SV_Game);
            n = sendto(fd,response.c_str(),n,0,(struct sockaddr*)&addr,addrlen);
            if(n==-1) exit(1);
        }
        else if(strcmp(code,"QUT") == 0){
        response = RQT(PLID,SV_Game);
        n = sendto(fd,response.c_str(),n,0,(struct sockaddr*)&addr,addrlen);
        if(n==-1) exit(1);
        }
        else if(strcmp(code,"REV") == 0){
        response = RRV(PLID,SV_Game);
        n = sendto(fd,response.c_str(),n,0,(struct sockaddr*)&addr,addrlen);
        if(n==-1) exit(1);
        }
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

        sscanf(buffer,"%s",code);
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
        }
        else if(strcmp(args[2],"-v")==0){ 
                printf("58011\n");
        }
    handleGame(S_port,word_file);
    freeaddrinfo(res);
    close(fd);
    return 0;
}
