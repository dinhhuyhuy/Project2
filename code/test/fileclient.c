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
    int port = 1234;
    char* ip = "192.168.1.111";
    int id_1; int id_2; 
    int size; int i; 
    char* buffer; 
    char* file1;
    char* file2;
    PrintString("file 1: ");
    ReadString(file1, 255);
    id_1 = Open(file1, 0);

    PrintString("\nFile 2: ");
    ReadString(file2, 255);
    id_2 = Open(file2, 0);

    if(id_1 != -1 && id_2 != -1){
        size = Read(buffer, 255, id_1);
        Client(buffer, ip, port);  
        Seek(-1,id_2);  
        Write(buffer, size, id_2);
    }else{
        PrintString("Can't open file\n");
    }
    for(i = 0; i < 20; i++){
        Close(i);
    }
    Halt();
}