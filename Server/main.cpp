#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>

using namespace std;


void random_word()
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

    cout<<lines[random_number] << endl;
  
}
void startGame(int* fd, struct sockaddr_in* addr, struct addrinfo *res, string* plid){
    
}


int main(int argc, char** args) {
    string word_file,S_port;
    word_file = args[1];
        if(strcmp(args[2],"-p")==0){ 
                S_port = args[3];
        }
        else if(strcmp(args[2],"-v")==0){ 
                printf("58011\n");
        }
    random_word();
    return 0;
}