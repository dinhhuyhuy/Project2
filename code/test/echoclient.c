#include "syscall.h"
#include "copyright.h"

void Client(char* buffer, char* ip, int port){
    int socketid;
    char* buffer2;
    int size, size2;
    int len = 0;
    while(buffer[len]){
        len++;
    }

    if((socketid = SocketTCP()) == -1){
        PrintString("Socket Cannot Create.\n");
    }else{
        PrintString("Socket Created.\n");
    }

    if(Connect(socketid, ip, port) == -1){
        PrintString("Socket Cannot Connect.\n");
    }else{
    PrintString("Socket Connect.\n");
    }

    size = Send(socketid, buffer, len);
    if(size == 0 || size == -1){
        PrintString("Send Error.\n");  
    }else{
        PrintString("Send Complete.\n"); 
    }

    size2 = Receive(socketid, buffer2, len);
    if(size == 0 || size == -1){
        PrintString("Receive Error.\n");  
    }else{
        PrintString("Receive Complete: "); 
        PrintString(buffer2);
        PrintString("\n");
    }

    if(SocketClose(socketid) == -1){
        PrintString("Socket Cannot Close.\n");
    }else{
        PrintString("Socket Close.\n\n");
    }
}

int main(){
    char* ip = "192.168.1.111";
    int port = 1234; 
    char* buffer1 = "helloworld";
    char* buffer2 = "hellofriend";
    char* buffer3 = "helloteacher";
    char* buffer4 = "helloworldagain";
    Client(buffer1, ip, port);
    Client(buffer2, ip, port);
    Client(buffer3, ip, port);
    Client(buffer4, ip, port);
    Halt();
}