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

using namespace std;


string random_word()
{
    string line;
    vector<string> lines;
    
    srand(time(0));
    
    ifstream file("example.txt");
    
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

void startGame(int* fd,struct sockaddr_in* addr,struct addrinfo *res){
    int errcode,max_errors;
    char buffer[128];
    char code[3];
    char PLID[7];
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints;

    *fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
        if(*fd==-1) /*error*/exit(1);
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET; // IPv4
        hints.ai_socktype=SOCK_DGRAM; // UDP socket
        hints.ai_flags=AI_PASSIVE;
        errcode=getaddrinfo(NULL,"58001",&hints,&res);
        if(errcode!=0) /*error*/ exit(1);
        n = bind(*fd,res->ai_addr,res->ai_addrlen);
        if(n==-1) exit(1);
        n = recvfrom(*fd,buffer,sizeof(buffer),0,(struct sockaddr*)&addr,&addrlen);
        if(n==-1) exit(1);
        sscanf(buffer,"%s %s",code,PLID);
        if(strcmp(code,"RSG")==0){
            string rw = random_word();
            const void * rwv = rw.c_str();
            if (sizeof(rw) <= 6){
                max_errors = 7;
            }
            else if(sizeof(rw)> 6 && sizeof(rw)<=10){
                max_errors = 8;
            }
            else if(sizeof(rw) >10){
                max_errors = 9;
            }
            string m_errors = to_string(max_errors);
            const void * buffer = m_errors.c_str();
            n = sendto(*fd,buffer,n,0,(struct sockaddr*)&addr,addrlen);
            if (n==-1) exit(1);
            freeaddrinfo(res);
            close(*fd);
    }
}

int main(int argc, char** args) {
    string word_file,S_port;
    word_file = args[1];
    cout << word_file << endl;
        if(strcmp(args[2],"-p")==0){ 
                S_port = args[3];
        }
        else if(strcmp(args[2],"-v")==0){ 
                printf("58011\n");
        }
    return 0;
}
